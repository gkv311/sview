/**
 * Copyright © 2012-2013 Kirill Gavrilov <kirill@sview.ru>
 *
 * StActiveX plugin is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * StActiveX plugin is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this library.
 * If not, see <http://www.gnu.org/licenses/>.
 */

#include "StActiveXModule.h"
#include "StActiveXCtrl.h"

#include "StBrPluginInfo.h"

#ifdef _MSC_VER

#include <StCore/StApplication.h>
#include <StGLStereo/StFormatEnum.h>
#include <StStrings/StLogger.h>
#include <StVersion.h>
#include <StFile/StFolder.h>
#include <StThreads/StThread.h>

#include "../StImageViewer/StImageViewer.h"

namespace {
    // Interface IDs
    static const IID IID_DStActiveXPlugin =       // 7138d6a9-b899-4dc3-b39c-dfd09c5dd5a8
        { 0x7138d6a9, 0xb899, 0x4dc3, { 0xb3, 0x9c, 0xdf, 0xd0, 0x9c, 0x5d, 0xd5, 0xa8 } };
    static const IID IID_DStActiveXPluginEvents = // bcc7f80a-c5b8-431e-be41-40d15b89469e
        { 0xbcc7f80a, 0xc5b8, 0x431e, { 0xbe, 0x41, 0x40, 0xd1, 0x5b, 0x89, 0x46, 0x9e } };

    // Control type information
    static const DWORD ST_OCX_OLE_MISC =
        OLEMISC_ACTIVATEWHENVISIBLE
      | OLEMISC_SETCLIENTSITEFIRST
      | OLEMISC_INSIDEOUT
      | OLEMISC_CANTLINKINSIDE
      | OLEMISC_RECOMPOSEONRESIZE;
}

StActiveXCtrl::StActiveXCtrl()
: myResMgr(new StResourceManager()),
  myParentWin((StNativeWin_t )NULL),
  myOpenEvent(false),
  myHasPreview(false),
  myToBlockMsg(false),
  myIsActive(false),
  myToQuit(false) {
    InitializeIIDs(&IID_DStActiveXPlugin, &IID_DStActiveXPluginEvents);
}

StActiveXCtrl::~StActiveXCtrl() {
    // all stuff should be done in OnDestroy() method
}

void StActiveXCtrl::OnDraw(CDC*         /*theDevCtx*/,
                           const CRect& /*theRectBounds*/,
                           const CRect& /*theRectInvalid*/) {
    // we draw into child window using OpenGL
}

void StActiveXCtrl::DoPropExchange(CPropExchange* thePropEx) {
    ExchangeVersion(thePropEx, MAKELONG(StActiveXModule::VER_MINOR, StActiveXModule::VER_MAJOR));
    COleControl::DoPropExchange(thePropEx);
    PX_String(thePropEx, _T("src"),          myUrlFull);
    PX_String(thePropEx, _T("data-prv-url"), myUrlPreview);
    PX_String(thePropEx, _T("type"),         myMimeType);

    myHasPreview = !myUrlPreview.IsEmpty()
                && myUrlPreview != myUrlFull;
}

void StActiveXCtrl::OnResetState() {
    COleControl::OnResetState();
}

StString StActiveXCtrl::loadURL(const CString& theUrl) {
    if(theUrl.IsEmpty()) {
        // invalid URL
        return StString();
    }

    // obtaining the full url
    CComBSTR aURLTemp(theUrl);
    CW2CT aFullUrl(aURLTemp);

    // load file and return path to cache
    IBindHost* aHost = NULL;
    wchar_t aFilePath[ST_MAX_PATH];
    this->GetClientSite()->QueryInterface(IID_IBindHost, (void** )&aHost);
    if(URLDownloadToCacheFileW(aHost, aFullUrl, aFilePath, ST_MAX_PATH, 0, NULL) == S_OK) {
        return StString(aFilePath);
    }

    return StString();
}

void StActiveXCtrl::stWindowLoop() {
    // do not load plugin until it is placed on screen
    StWindow aParentWin(myResMgr, myParentWin);
    for(;;) {
        if(aParentWin.isParentOnScreen()) {
            break;
        }

        StThread::sleep(10);
        if(myToQuit) {
            return;
        }
    }

    myStApp = new StImageViewer(myResMgr, myParentWin, new StOpenInfo());
    if(!myStApp->open()) {
        myStApp.nullify();
        return;
    }

    bool isFullscreen = false;
    myIsActive = true;
    for(;;) {
        if(myStApp->closingDown()) {
            myStApp.nullify();
            myIsActive = false;
            return;
        }

        myIsActive = myStApp->isActive();
        if(myToQuit) {
            myStApp->exit(0);
        } else if(myOpenEvent.check()
               && myStApp->isActive()) {
            // load the image
            myStApp->open(myOpenInfo);
            myOpenEvent.reset();
        }

        StHandle<StWindow> aWin = myStApp->getMainWindow();
        if(myIsActive) {
            aWin->show();
        } else {
            aWin->hide();
        }
        myStApp->processEvents();

        if(aWin->isFullScreen()) {
            if(!isFullscreen) {
                PostMessage(WM_TIMER, 1);
                isFullscreen = true;
            }
        } else if(isFullscreen) {
            PostMessage(WM_TIMER, 0);
            isFullscreen = false;
        }
    }
}

static SV_THREAD_FUNCTION stThreadFunction(void* theParam) {
    StActiveXCtrl* aBrPlugin = (StActiveXCtrl* )theParam;
    aBrPlugin->stWindowLoop();
    return SV_THREAD_RETURN 0;
}

int StActiveXCtrl::OnCreate(LPCREATESTRUCT theCreateStruct) {
    myBackBrush.CreateSolidBrush(RGB(0, 0, 0));
    if(COleControl::OnCreate(theCreateStruct) == -1) {
        return -1;
    }

    const StString ST_ASTERIX = '*';
    StMIME aMime(StString(myMimeType), ST_ASTERIX, ST_ASTERIX);
    myOpenInfo.setMIME(aMime);

    StArgumentsMap aDrawerArgs;
    const StString ST_SETTING_SRCFORMAT    = stCString("srcFormat");
    const StString ST_SETTING_COMPRESS     = stCString("toCompress");
    const StString ST_SETTING_ESCAPENOQUIT = stCString("escNoQuit");
    const StMIME ST_MIME_X_JPS("image/x-jps", ST_ASTERIX, ST_ASTERIX);
    const StMIME ST_MIME_JPS  ("image/jps",   ST_ASTERIX, ST_ASTERIX);
    const StMIME ST_MIME_X_PNS("image/x-pns", ST_ASTERIX, ST_ASTERIX);
    const StMIME ST_MIME_PNS  ("image/pns",   ST_ASTERIX, ST_ASTERIX);
    StArgument anArgSrcFormat = aDrawerArgs[ST_SETTING_SRCFORMAT];
    if(!anArgSrcFormat.isValid()) {
        anArgSrcFormat.setKey(ST_SETTING_SRCFORMAT);
        if(aMime == ST_MIME_X_JPS
        || aMime == ST_MIME_JPS
        || aMime == ST_MIME_X_PNS
        || aMime == ST_MIME_PNS) {
            anArgSrcFormat.setValue(st::formatToString(StFormat_SideBySide_RL));
            aDrawerArgs.add(anArgSrcFormat);
        }
    }
    aDrawerArgs.add(StArgument(ST_SETTING_COMPRESS,     "true")); // optimize memory usage
    aDrawerArgs.add(StArgument(ST_SETTING_ESCAPENOQUIT, "true")); // do not close plugin instance by Escape key
    myOpenInfo.setArgumentsMap(aDrawerArgs);

    // set window
    myParentWin = m_hWnd;

    // starts out plugin main loop in another thread
    myThread = new StThread(stThreadFunction, (void* )this, "StActiveXCtrl");

    // load URL
    StString aFilePath = loadURL(myHasPreview ? myUrlPreview : myUrlFull);
    if(aFilePath.isEmpty()) {
        if(!myHasPreview) {
            return 0;
        }
        if(myHasPreview) {
            // if we have 2 URLs - try to load another one
            aFilePath = loadURL(myUrlFull);
            if(aFilePath.isEmpty()) {
                return 0;
            }
            myHasPreview = false;
        }
    }

    myOpenInfo.setPath(aFilePath);
    myOpenEvent.set();
    return 0;
}

void StActiveXCtrl::AboutBox() {
    stInfo(StString("This is sView ") + SVIEW_SDK_VER_STRING + " ActiveX Control for Internet Explorer");
}

void StActiveXCtrl::Dispose() {
    if(!myThread.isNull()) {
        myToQuit = true;
        myThread->wait();
        myThread.nullify();
    }
}

void StActiveXCtrl::OnDestroy() {
    if(myToBlockMsg) {
        return;
    }

    if(!myThread.isNull()) {
        myToQuit = true;
        myThread->wait();
        myThread.nullify();
    }

    myToBlockMsg = true;
    COleControl::OnDestroy();
    myToBlockMsg = false;
}

LRESULT StActiveXCtrl::WindowProc(UINT theMsg, WPARAM theParamW, LPARAM theParamL) {
    // Special case for WM_INITMENUPOPUP event.
    // Standard implementation of COleControl::WindowProc does not find handlers for popup menu items created
    // by plugin and disables them
    if(theMsg == WM_INITMENUPOPUP) {
        return 0;
    } else if(theMsg == WM_ERASEBKGND) {
        return 1;
    } else if(theMsg == WM_PAINT) {
        if(!myIsActive) {
            PAINTSTRUCT aPaintStruct;
            CDC* aDevCtx = BeginPaint(&aPaintStruct);
            FillRect(aDevCtx->GetSafeHdc(), &aPaintStruct.rcPaint, myBackBrush);
            EndPaint(&aPaintStruct);
        }
        return 0;
    }

    // use fake WM_TIMER event to switch between fullscreen / windowed image in correct thread
    if(theMsg == WM_TIMER && myHasPreview) {
        if(theParamW == 1) {
            const StString aFilePath = loadURL(myUrlFull);
            if(!aFilePath.isEmpty()) {
                myOpenInfo.setPath(aFilePath);
                myOpenEvent.set();
            } else {
                // don't try to load broken URL anymore
                myHasPreview = false;
            }
        } else if(theParamW == 0) {
            const StString aFilePath = loadURL(myUrlPreview);
            if(!aFilePath.isEmpty()) {
                myOpenInfo.setPath(aFilePath);
                myOpenEvent.set();
            } else {
                // don't try to load broken URL anymore
                myHasPreview = false;
            }
        }
    }

    return COleControl::WindowProc(theMsg, theParamW, theParamL);
}

IMPLEMENT_OLECTLTYPE(StActiveXCtrl, ST_OCX_RESTXT_NAME, ST_OCX_OLE_MISC)
IMPLEMENT_OLECREATE_EX(StActiveXCtrl, "STACTIVEX.StActiveXCtrl.1", // Initialize class factory and guid
                       0x027792d0, 0x5136, 0x4ea3, 0x9b, 0xec, 0x34, 0x27, 0x6d, 0xfe, 0x43, 0x62)
// Type library ID and version
IMPLEMENT_OLETYPELIB(StActiveXCtrl, StActiveXModule::TYPELIB_GUID, StActiveXModule::VER_MAJOR, StActiveXModule::VER_MINOR)

IMPLEMENT_DYNCREATE(StActiveXCtrl, COleControl)

// Message map
BEGIN_MESSAGE_MAP(StActiveXCtrl, COleControl)
    ON_OLEVERB(AFX_IDS_VERB_PROPERTIES, OnProperties)
    ON_WM_CREATE()
    ON_WM_DESTROY()
END_MESSAGE_MAP()

BEGIN_INTERFACE_MAP(StActiveXCtrl, COleControl)
    INTERFACE_PART(StActiveXCtrl, IID_IObjectSafety, ObjectSafety)
END_INTERFACE_MAP()

// Dispatch map
BEGIN_DISPATCH_MAP(StActiveXCtrl, COleControl)
    DISP_FUNCTION_ID(StActiveXCtrl, "AboutBox", DISPID_ABOUTBOX,   AboutBox, VT_EMPTY, VTS_NONE)
    DISP_FUNCTION_ID(StActiveXCtrl, "Dispose",  ST_DISPID_DISPOSE, Dispose,  VT_EMPTY, VTS_NONE)
END_DISPATCH_MAP()

// Event map
BEGIN_EVENT_MAP(StActiveXCtrl, COleControl)
END_EVENT_MAP()

BOOL StActiveXCtrl::StActiveXCtrlFactory::UpdateRegistry(BOOL theToRegister) {
    if(!theToRegister) {
        return AfxOleUnregisterClass(m_clsid, m_lpszProgID);
    }

    return AfxOleRegisterControlClass(AfxGetInstanceHandle(),
                                      m_clsid, m_lpszProgID,
                                      ST_OCX_RESTXT_NAME,
                                      ST_OCX_RESBMP_ICON,
                                      afxRegApartmentThreading,
                                      ST_OCX_OLE_MISC,
                                      StActiveXModule::TYPELIB_GUID,
                                      StActiveXModule::VER_MAJOR,
                                      StActiveXModule::VER_MINOR);
}

// Object safety
ULONG StActiveXCtrl::XObjectSafety::Release() {
    METHOD_PROLOGUE_EX_(StActiveXCtrl, ObjectSafety)
    return (ULONG )pThis->ExternalRelease();
}

ULONG StActiveXCtrl::XObjectSafety::AddRef() {
    METHOD_PROLOGUE_EX_(StActiveXCtrl, ObjectSafety)
    return (ULONG )pThis->ExternalAddRef();
}

STDMETHODIMP StActiveXCtrl::XObjectSafety::QueryInterface(REFIID theRefIID, LPVOID* ppvObj) {
    METHOD_PROLOGUE_EX_(StActiveXCtrl, ObjectSafety)
    return (HRESULT)pThis->ExternalQueryInterface(&theRefIID, ppvObj);
}

HRESULT StActiveXCtrl::XObjectSafety::GetInterfaceSafetyOptions(REFIID theRefIID,
                                                                DWORD* theSupportedOptions,
                                                                DWORD* theEnabledOptions) {
    METHOD_PROLOGUE_EX(StActiveXCtrl, ObjectSafety);
    if(theSupportedOptions == NULL || theEnabledOptions == NULL) {
        return E_POINTER;
    }

    *theSupportedOptions = INTERFACESAFE_FOR_UNTRUSTED_CALLER | INTERFACESAFE_FOR_UNTRUSTED_DATA;
    *theEnabledOptions   = 0;

    if(pThis->GetInterface(&theRefIID) == NULL) {
        OLECHAR aGUID[39];
        StringFromGUID2(theRefIID, aGUID, 39);
        ST_DEBUG_LOG("Requested interface '" + aGUID + "' is not supported.");
        return E_NOINTERFACE;
    }

    if(theRefIID == IID_IDispatch) {
        // Client wants to know if object is safe for scripting
        *theEnabledOptions = INTERFACESAFE_FOR_UNTRUSTED_CALLER;
        return S_OK;
    } else if(theRefIID == IID_IPersistPropertyBag
           || theRefIID == IID_IPersistStreamInit
           || theRefIID == IID_IPersistStorage
           || theRefIID == IID_IPersistMemory) {
        // Those are the persistence interfaces COleControl derived controls support
        // Client wants to know if object is safe for initializing from persistent data
        *theEnabledOptions = INTERFACESAFE_FOR_UNTRUSTED_DATA;
        return S_OK;
    }

    // Find out what interface this is, and decide what options to enable
    ST_DEBUG_LOG("We didn't account for the safety of this interface, and it's one we support...");
    return E_NOINTERFACE;
}

HRESULT StActiveXCtrl::XObjectSafety::SetInterfaceSafetyOptions(REFIID theRefIID,
                                                                DWORD  theOptionSetMask,
                                                                DWORD  theEnabledOptions) {
    METHOD_PROLOGUE_EX(StActiveXCtrl, ObjectSafety);
    if(theOptionSetMask  == 0
    && theEnabledOptions == 0) {
        // the control certainly supports NO requests through the specified interface
        // so it's safe to return S_OK even if the interface isn't supported.
        return S_OK;
    }

    // Do we support the specified interface?
    OLECHAR aGUID[39];
    StringFromGUID2(theRefIID, aGUID, 39);
    if(pThis->GetInterface(&theRefIID) == NULL) {
        ST_DEBUG_LOG("Interface '" + aGUID + "' is not supported.");
        return E_FAIL;
    }

    if(theRefIID == IID_IDispatch) {
        ST_DEBUG_LOG("Client asking if it's safe to call through IDispatch.");
        ST_DEBUG_LOG("In other words, is the control safe for scripting?");
        if(theOptionSetMask  == INTERFACESAFE_FOR_UNTRUSTED_CALLER
        && theEnabledOptions == INTERFACESAFE_FOR_UNTRUSTED_CALLER) {
            return S_OK;
        }
        return E_FAIL;
    } else if(theRefIID == IID_IPersistPropertyBag
           || theRefIID == IID_IPersistStreamInit
           || theRefIID == IID_IPersistStorage
           || theRefIID == IID_IPersistMemory) {
        ST_DEBUG_LOG("Client asking if it's safe to call through IPersist*.");
        ST_DEBUG_LOG("In other words, is the control safe for initializing from persistent data?");
        if(theOptionSetMask  == INTERFACESAFE_FOR_UNTRUSTED_DATA
        && theEnabledOptions == INTERFACESAFE_FOR_UNTRUSTED_DATA) {
            return NOERROR;
        }
        return E_FAIL;
    }

    ST_DEBUG_LOG("We didn't account for the safety of '" + aGUID + "', and it's one we support...");
    return E_FAIL;
}

#endif // _MSC_VER
