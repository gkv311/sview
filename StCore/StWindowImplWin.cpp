/**
 * StCore, window system independent C++ toolkit for writing OpenGL applications.
 * Copyright © 2007-2017 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#ifdef _WIN32

#include "StWindowImpl.h"

#include <StStrings/StLogger.h>
#include <StThreads/StThread.h>
#include <StLibrary.h>
#include <StGL/StGLContext.h>

#include <cmath>
#include <vector>

static SV_THREAD_FUNCTION threadCreateWindows(void* inStWin);

/**
 * Static proc function just do recall to StWindowImpl class method.
 */
static LRESULT CALLBACK stWndProcWrapper(HWND   theWnd,
                                         UINT   theMsg,
                                         WPARAM theParamW,
                                         LPARAM theParamL) {
    if(theMsg == WM_NCCREATE) {
        HMODULE aUser32Module = GetModuleHandleW(L"User32"); // User32 should be already loaded
        if(aUser32Module != NULL) {
            // available since Win10 (update from '2016)
            typedef BOOL (WINAPI *EnableNonClientDpiScaling_t)(HWND hwnd);
            EnableNonClientDpiScaling_t aEnableNonClientDpiScaling = (EnableNonClientDpiScaling_t )GetProcAddress(aUser32Module, "EnableNonClientDpiScaling");
            if(aEnableNonClientDpiScaling != NULL) {
                // ask Windows 10 to resize title bar... awkward thing
                BOOL aRes = aEnableNonClientDpiScaling(theWnd);
                (void )aRes;
            }
        }
    }

    if(theMsg == WM_CREATE) {
        // save pointer to our class instance (sent on window create) to window storage
        CREATESTRUCTW* aCreateInfo = (CREATESTRUCTW* )theParamL;
        ::SetWindowLongPtr(theWnd, GWLP_USERDATA, (LONG_PTR )aCreateInfo->lpCreateParams);
    }
    // get pointer to our class instance
    StWindowImpl* aThis = (StWindowImpl* )::GetWindowLongPtr(theWnd, GWLP_USERDATA);
    if(aThis != NULL) {
        return aThis->stWndProc(theWnd, theMsg, theParamW, theParamL);
    } else {
        return ::DefWindowProcW(theWnd, theMsg, theParamW, theParamL);
    }
}

void StWindowImpl::convertRectToBacking(StGLBoxPx& /*theRect*/,
                                        const int  /*theWinId*/) const {
    // should be implemented support for Windows 8.1
}

bool StWindowImpl::create() {
    if(myRegisterTouchWindow == NULL) {
        HMODULE aUser32Module = GetModuleHandleW(L"User32");
        if(aUser32Module != NULL) {
            // User32 should be already loaded
            myRegisterTouchWindow   = (RegisterTouchWindow_t   )GetProcAddress(aUser32Module, "RegisterTouchWindow");
            myUnregisterTouchWindow = (UnregisterTouchWindow_t )GetProcAddress(aUser32Module, "UnregisterTouchWindow");
            myGetTouchInputInfo     = (GetTouchInputInfo_t     )GetProcAddress(aUser32Module, "GetTouchInputInfo");
            myCloseTouchInputHandle = (CloseTouchInputHandle_t )GetProcAddress(aUser32Module, "CloseTouchInputHandle");
        }
    }

    myKeysState.reset();
    myInitState = STWIN_INITNOTSTART;

    myEventInitWin.reset();
    myEventInitGl.reset();
    myEventQuit.reset();
    myMsgThread = new StThread(threadCreateWindows, (void* )this, "StWindowImplMSG");
    // wait for thread to create window
    myEventInitWin.wait();
    if(myInitState != STWIN_INIT_SUCCESS) {
        return false;
    }

    int isGlCtx = myMaster.glCreateContext(attribs.Slave != StWinSlave_slaveOff ? &mySlave : NULL,
                                           myRectNorm,
                                           attribs.GlDepthSize,
                                           attribs.GlStencilSize,
                                           attribs.IsGlStereo,
                                           attribs.IsGlDebug);
    myEventInitGl.set();
    myGlContext = new StGLContext(myResMgr);
    if(!myGlContext->stglInit()) {
        stError("Critical error - broken GL context!\nInvalid OpenGL driver?");
        myInitState = STWIN_ERROR_WIN32_GLRC_ACTIVATE;
        return false;
    }

    return (isGlCtx == STWIN_INIT_SUCCESS);
}

// Function create windows' handles in another thread (to take events impact-less to GL-rendering)
static SV_THREAD_FUNCTION threadCreateWindows(void* inStWin) {
    StWindowImpl* stWin = (StWindowImpl* )inStWin;
    stWin->wndCreateWindows();
    return SV_THREAD_RETURN 0;
}

bool StWindowImpl::wndCreateWindows() {
    // register classes
    if(!myMaster.registerClasses(attribs.Slave != StWinSlave_slaveOff ? &mySlave : NULL,
                                 (WNDPROC )stWndProcWrapper)) {
        myInitState = STWIN_ERROR_WIN32_REGCLASS;
        return false;
    }
    myMaster.ThreadWnd = mySlave.ThreadWnd = StThread::getCurrentThreadId();

    // ========= At first - create windows' instances =========
    HINSTANCE hInstance = GetModuleHandleW(NULL);
    RECT aRect;
    aRect.top    = myRectNorm.top();
    aRect.bottom = myRectNorm.bottom();
    aRect.left   = myRectNorm.left();
    aRect.right  = myRectNorm.right();

    if(myParentWin == NULL) {
        // parent Master window could be decorated - we parse this situation to get true coordinates
        const DWORD aWinStyle   = (!attribs.IsNoDecor ? WS_OVERLAPPEDWINDOW : WS_POPUP) | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
        const DWORD aWinStyleEx = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE | WS_EX_ACCEPTFILES;
        const RECT aRectOrig = aRect;
        AdjustWindowRectEx(&aRect, aWinStyle, FALSE, aWinStyleEx);
        myDecMargins.left   = aRectOrig.left     - aRect.left;
        myDecMargins.right  = aRect.right   - aRectOrig.right;
        myDecMargins.top    = aRectOrig.top       - aRect.top;
        myDecMargins.bottom = aRect.bottom - aRectOrig.bottom;

        // WS_EX_ACCEPTFILES - Drag&Drop support
        myMaster.hWindow = CreateWindowExW(aWinStyleEx,
                                           myMaster.ClassBase.toCString(),
                                           myWindowTitle.toUtfWide().toCString(),
                                           aWinStyle,
                                           aRect.left, aRect.top,
                                           aRect.right - aRect.left, aRect.bottom - aRect.top,
                                           NULL, NULL, hInstance, this); // put pointer to class, getted on WM_CREATE
        if(myMaster.hWindow == NULL) {
            myMaster.close();
            stError("WinAPI: Master Window Creation Error");
            myInitState = STWIN_ERROR_WIN32_CREATE;
            return false;
        }
    } else {
        // external native parent window
        attribs.IsNoDecor = true;

        GetWindowRect(myParentWin, &aRect);
        myRectNorm.left()   = aRect.left;
        myRectNorm.right()  = aRect.right;
        myRectNorm.top()    = aRect.top;
        myRectNorm.bottom() = aRect.bottom;

        myIsUpdated = true;
    }

    myWinOnMonitorId = myMsgMonitors[myRectNorm.center()].getId();
    myWinMonScaleId  = myWinOnMonitorId;

    // use WS_EX_NOPARENTNOTIFY style to prevent to send notify on destroying our child window (NPAPI plugin -> deadlock)
    DWORD masterWindowGl_dwExStyle = (myParentWin == NULL) ? WS_EX_NOACTIVATE : (WS_EX_NOACTIVATE | WS_EX_NOPARENTNOTIFY);
    myMaster.hWindowGl = CreateWindowExW(masterWindowGl_dwExStyle,
                                         myMaster.ClassGL.toCString(),
                                         myWindowTitle.toUtfWide().toCString(),
                                         WS_VISIBLE | WS_CHILD,  // this is child sub-window!
                                         0, 0, myRectNorm.width(), myRectNorm.height(),
                                         (myParentWin == NULL) ? myMaster.hWindow : myParentWin,
                                         NULL, hInstance, this); // put pointer to class, getted on WM_CREATE
    if(myMaster.hWindowGl == NULL) {
        myMaster.close();
        stError("WinAPI: Master GL Window Creation Error");
        myInitState = STWIN_ERROR_WIN32_CREATE;
        return false;
    }

    if(attribs.Slave != StWinSlave_slaveOff) {
        mySlave.hWindowGl = CreateWindowExW(WS_EX_TOOLWINDOW | WS_EX_WINDOWEDGE | WS_EX_NOACTIVATE,
                                            mySlave.ClassGL.toCString(),
                                            L"Slave window",
                                            // slave is always disabled (hasn't input focus)!
                                            WS_POPUP | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_DISABLED,
                                            // initialize slave window at same screen as master to workaround bugs in drivers that may prevent GL context sharing
                                            myRectNorm.left(),  myRectNorm.top(),
                                            myRectNorm.width(), myRectNorm.height(),
                                            NULL, NULL, hInstance, NULL);
        if(mySlave.hWindowGl == NULL) {
            myMaster.close();
            mySlave.close();
            stError(L"WinAPI: Slave Window Creation Error");
            myInitState = STWIN_ERROR_WIN32_CREATE;
            return false;
        }
    }

    // ========= Synchronization barrier - wait until MAIN thread creates GL contexts =========
    myInitState = STWIN_INIT_SUCCESS; // 'send' it's OK for main thread
    myEventInitWin.set();
    myEventInitGl.wait();

    // ========= Now show up the windows =========
    if(attribs.Slave != StWinSlave_slaveOff && !attribs.IsSlaveHidden && (!isSlaveIndependent() || myMsgMonitors.size() > 1)) {
        SetWindowPos(mySlave.hWindowGl,
                     HWND_NOTOPMOST,
                     getSlaveLeft(),  getSlaveTop(),
                     getSlaveWidth(), getSlaveHeight(),
                     SWP_SHOWWINDOW);
    }

    if(!attribs.IsHidden) {
        if(myMaster.hWindow != NULL) {
            SetWindowPos(myMaster.hWindow, HWND_NOTOPMOST,
                         aRect.left, aRect.top, aRect.right - aRect.left, aRect.bottom - aRect.top,
                         SWP_SHOWWINDOW);
        }

        SetWindowPos(myMaster.hWindowGl, HWND_NOTOPMOST,
                     0, 0, myRectNorm.width(), myRectNorm.height(),
                     SWP_SHOWWINDOW);
    }

    // register handler to track session lock state (WM_WTSSESSION_CHANGE)
    myIsSystemLocked = false;
    typedef BOOL (WINAPI *WTSRegisterSessionNotification_t  )(HWND hWnd, DWORD dwFlags);
    typedef BOOL (WINAPI *WTSUnRegisterSessionNotification_t)(HWND hWnd);
    WTSRegisterSessionNotification_t   aSessNotifSetProc   = NULL;
    WTSUnRegisterSessionNotification_t aSessNotifUnsetProc = NULL;
    StLibrary aWtsLib;
    if(aWtsLib.loadSimple("Wtsapi32.dll")
    && aWtsLib.find("WTSRegisterSessionNotification",   aSessNotifSetProc)
    && aWtsLib.find("WTSUnRegisterSessionNotification", aSessNotifUnsetProc)) {
        #define NOTIFY_FOR_ALL_SESSIONS 1
        #define NOTIFY_FOR_THIS_SESSION 0
        aSessNotifSetProc(myMaster.hWindowGl, NOTIFY_FOR_THIS_SESSION);
    }

    // recieve WM_TOUCH events
    if(myRegisterTouchWindow != NULL) {
        //myRegisterTouchWindow(myMaster.hWindowGl, TWF_FINETOUCH);
    }

    // always wait for message thread exit before quit
    myMaster.EventMsgThread.reset();

    // ========= Start callback procedure =========
    if(!attribs.IsHidden && myParentWin == NULL) {
        SetForegroundWindow(myMaster.hWindow); // make sure Master window on top and has input focus
    }
    // register global updater - listen for WM_DISPLAYCHANGE events
    myMsgMonitors.registerUpdater(true);

    // flag to track registered global hot-keys
    bool areGlobalHotKeys = false;

    enum {
        StWntMsg_Quit = 0,
        StWntMsg_CursorShow,
        StWntMsg_CursorHide,
        StWntMsg_WINDOW
    };
    HANDLE aWaitEvents[3] = {};
    aWaitEvents[StWntMsg_Quit]       = myEventQuit      .getHandle();
    aWaitEvents[StWntMsg_CursorShow] = myEventCursorShow.getHandle();
    aWaitEvents[StWntMsg_CursorHide] = myEventCursorHide.getHandle();

    wchar_t aCharBuff[4];
    BYTE    aKeysMap[256];
    for(;;) {
        switch(::MsgWaitForMultipleObjects(3, aWaitEvents, FALSE, INFINITE, QS_ALLINPUT)) {
            case WAIT_OBJECT_0 + StWntMsg_Quit: {
                // if the event was created as autoreset, it has also been reset
                ///ST_DEBUG_LOG("WinAPI, End of the message thread... (TID= " + StThread::getCurrentThreadId() + ")");
                if(areGlobalHotKeys) {
                    UnregisterHotKey(myMaster.hWindowGl, myMaster.myMKeyStop);
                    UnregisterHotKey(myMaster.hWindowGl, myMaster.myMKeyPlay);
                    UnregisterHotKey(myMaster.hWindowGl, myMaster.myMKeyPrev);
                    UnregisterHotKey(myMaster.hWindowGl, myMaster.myMKeyNext);
                }
                if(aSessNotifUnsetProc != NULL) {
                    aSessNotifUnsetProc(myMaster.hWindowGl);
                }

                mySlave.close();  // close window handles
                myMaster.close();

                mySlave.ThreadWnd  = 0;
                myMaster.ThreadWnd = 0;

                // end of events loop - WM_DISPLAYCHANGE will not be handled by this window anymore
                myMsgMonitors.registerUpdater(false);

                myEventQuit.reset();
                myMaster.EventMsgThread.set(); // thread now exit, nothing should be after!
                return true;
            }
            case WAIT_OBJECT_0 + StWntMsg_CursorShow: {
                // show / hide cursor - SHOULD be called from window thread...
                ::ShowCursor(TRUE);
                myEventCursorShow.reset();
                break;
            }
            case WAIT_OBJECT_0 + StWntMsg_CursorHide: {
                // warning - this is NOT force hide / show function;
                // this call decrease or increase show counter
                ::ShowCursor(FALSE);
                myEventCursorHide.reset();
                break;
            }
            case WAIT_OBJECT_0 + StWntMsg_WINDOW: {
                // synchronize high-precision timer with system UpTime
                if(myEventsTimer.isResyncNeeded()) {
                    myEventsTimer.resyncUpTime();
                }

                // A thread's (window's) message(s) has arrived
                // We should process ALL messages cause MsgWaitForMultipleObjects
                // will NOT triggered for new messages already in stack!!!
                // Means - do not replace 'while' with 'if(PeekMessage(...))'.
                while(PeekMessage(&myEvent, NULL, 0U, 0U, PM_REMOVE)) {
                    // we process WM_KEYDOWN/WM_KEYUP manually - TranslateMessage is redundant
                    //TranslateMessage(&myEvent);
                    switch(myEvent.message) {
                        // keys lookup
                        case WM_KEYDOWN: {
                            myStEvent.Key.Time = getEventTime(myEvent.time);
                            myStEvent.Key.VKey = (StVirtKey )myEvent.wParam;

                            // ToUnicode needs high-order bit of a byte to be set for pressed keys...
                            //GetKeyboardState(aKeysMap);
                            for(int anIter = 0; anIter < 256; ++anIter) {
                                aKeysMap[anIter] = myKeysState.isKeyDown((StVirtKey )anIter) ? 0xFF : 0;
                            }

                            if(::ToUnicode(myStEvent.Key.VKey, HIWORD(myEvent.lParam) & 0xFF,
                                           aKeysMap, aCharBuff, 4, 0) > 0) {
                                StUtfWideIter aUIter(aCharBuff);
                                myStEvent.Key.Char = *aUIter;
                            } else {
                                myStEvent.Key.Char = 0;
                            }

                            myStEvent.Key.Flags = ST_VF_NONE;
                            postKeyDown(myStEvent);
                            break;
                        }
                        case WM_KEYUP: {
                            myStEvent.Key.VKey  = (StVirtKey )myEvent.wParam;
                            myStEvent.Key.Time  = getEventTime(myEvent.time);
                            myStEvent.Key.Flags = ST_VF_NONE;
                            postKeyUp(myStEvent);
                            break;
                        }
                        default: break;
                    }

                    DispatchMessageW(&myEvent);
                }

                // well bad place for polling since it should be rarely changed
                const bool areGlobalMKeysNew = attribs.AreGlobalMediaKeys;
                if(areGlobalHotKeys != areGlobalMKeysNew) {
                    areGlobalHotKeys = areGlobalMKeysNew;
                    if(areGlobalHotKeys) {
                        RegisterHotKey(myMaster.hWindowGl, myMaster.myMKeyStop, 0, VK_MEDIA_STOP);
                        RegisterHotKey(myMaster.hWindowGl, myMaster.myMKeyPlay, 0, VK_MEDIA_PLAY_PAUSE);
                        RegisterHotKey(myMaster.hWindowGl, myMaster.myMKeyPrev, 0, VK_MEDIA_PREV_TRACK);
                        RegisterHotKey(myMaster.hWindowGl, myMaster.myMKeyNext, 0, VK_MEDIA_NEXT_TRACK);
                    } else {
                        UnregisterHotKey(myMaster.hWindowGl, myMaster.myMKeyStop);
                        UnregisterHotKey(myMaster.hWindowGl, myMaster.myMKeyPlay);
                        UnregisterHotKey(myMaster.hWindowGl, myMaster.myMKeyPrev);
                        UnregisterHotKey(myMaster.hWindowGl, myMaster.myMKeyNext);
                    }
                }
                break; // break from switch
            }
            default: {
                // no input or messages waiting
                break;
            }
        }
    }
}

bool StWindowImpl::isParentOnScreen() const {
    if(myParentWin == NULL) {
        return false;
    }

    RECT aRect;
    StRectI_t aRectSt;
    GetClientRect (myParentWin, &aRect);
    ClientToScreen(myParentWin,  (POINT* )&aRect);
    ClientToScreen(myParentWin, ((POINT* )&aRect) + 1);
    if(aRect.right  - aRect.left < 10
    || aRect.bottom - aRect.top  < 10) {
        return false; // window is not yet positioned on the screen
    }

    aRectSt.left()   = aRect.left;
    aRectSt.top()    = aRect.top;
    aRectSt.right()  = aRect.right;
    aRectSt.bottom() = aRect.bottom;

    for(size_t aMonIter = 0; aMonIter < myMonitors.size(); ++aMonIter) {
        if(!myMonitors[aMonIter].getVRect().isOut(aRectSt)) {
            return true;
        }
    }
    return false;
}

/**
 * Update StWindow position according to native parent position.
 */
void StWindowImpl::updateChildRect() {
    if(!attribs.IsFullScreen && myParentWin != NULL) {
        myIsUpdated = true;
        RECT aRect;
        GetClientRect(myParentWin, &aRect);
        ClientToScreen(myParentWin, (POINT* )&aRect);
        ClientToScreen(myParentWin, ((POINT* )&aRect) + 1);

        // TODO (Kirill Gavrilov#2) not thread safe assignment!
        myRectNorm.left()   = aRect.left;
        myRectNorm.top()    = aRect.top;
        myRectNorm.right()  = aRect.right;
        myRectNorm.bottom() = aRect.bottom;

        myStEventAux.Size.init(getEventTime(), myRectNorm.width(), myRectNorm.height(), myForcedAspect);
        signals.onResize->emit(myStEventAux.Size);
    }
}

LRESULT StWindowImpl::stWndProc(HWND theWin, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch(uMsg) {                 // Check For Windows Messages
        case WM_ACTIVATE: {        // Watch For Window Activate Message
            if(LOWORD(wParam) == WA_INACTIVE) {
              // input focus loss - release pressed keys cached state
              myKeysState.reset();
            }
            /*if(!HIWORD(wParam)) {
                //active = true;   // Check Minimization State
            } else {
                //active = false;  // Program Is No Longer Active
                //active = true;
            }*/
            return 0;              // Return To The Message Loop
        }
        case WM_SYSCOMMAND: {
            switch(wParam) {
                /*case 0xF032: // double click on title
                case SC_MAXIMIZE: {
                    setFullScreen(true);
                    myIsUpdated = true;
                    return 0;
                }*/
                case SC_SCREENSAVE:     // Screensaver Trying To Start?
                case SC_MONITORPOWER: { // Monitor Trying To Enter Powersave?
                    if(attribs.ToBlockSleepDisplay) {
                        return 0; // Prevent From Happening
                    }
                    break;
                }
            }
            break;
        }
        case WM_WTSSESSION_CHANGE: {
            if(wParam == WTS_SESSION_LOCK) {
                myIsSystemLocked = true;
            } else if(wParam == WTS_SESSION_UNLOCK) {
                myIsSystemLocked = false;
            }
            break;
        }
        case WM_CLOSE: {
            myStEvent.Type       = stEvent_Close;
            myStEvent.Close.Time = getEventTime(myEvent.time);
            myEventsBuffer.append(myStEvent);
            return 0; // do nothing - window close action should be performed by application
        }

        case WM_DISPLAYCHANGE: {
            myMsgMonitors.init(true);
            myIsDispChanged = true;
            return 0;
        }
        case WM_DROPFILES: {
            HDROP aDrops = (HDROP )wParam;
            const UINT aFilesCount = DragQueryFileW(aDrops, 0xFFFFFFFF, NULL, 0);
            stUtfWide_t aFileBuff[MAX_PATH];
            if(aFilesCount < 1) {
                DragFinish(aDrops);
                break;
            }

            // convert to UTF-8
            std::vector<StString> aPaths;
            for(UINT aFileId = 0; aFileId < aFilesCount; ++aFileId) {
                if(DragQueryFileW(aDrops, aFileId, aFileBuff, MAX_PATH) > 0) {
                    aPaths.push_back(StString(aFileBuff));
                }
            }
            DragFinish(aDrops);

            std::vector<const char*> aDndList;
            for(std::vector<StString>::const_iterator aFileIter = aPaths.begin(); aFileIter != aPaths.end(); ++aFileIter) {
                aDndList.push_back(aFileIter->toCString());
            }
            if(!aDndList.empty()) {
                myStEvent.Type = stEvent_FileDrop;
                myStEvent.DNDrop.Time    = getEventTime(myEvent.time);
                myStEvent.DNDrop.NbFiles = (uint32_t )aDndList.size();
                myStEvent.DNDrop.Files   = &aDndList[0];
                myEventsBuffer.append(myStEvent);
            }
            break;
        }
        case WM_WINDOWPOSCHANGED: {
            if(attribs.IsFullScreen || myParentWin != NULL) {
                //return 0;
                break;
            } else if(theWin != myMaster.hWindow) {
                break;
            }

            RECT aRect;
            GetClientRect(theWin, &aRect);
            ClientToScreen(theWin,  (POINT* )&aRect);
            ClientToScreen(theWin, ((POINT* )&aRect) + 1);

            // update decoration margins - AdjustWindowRectEx() does not work with per-monitor DPI and EnableNonClientDpiScaling()
            WINDOWPOS* aPos = (WINDOWPOS* )lParam;
            myDecMargins.left   = aRect.left - aPos->x;
            myDecMargins.right  = (aPos->x + aPos->cx) - aRect.right;
            myDecMargins.top    = aRect.top  - aPos->y;
            myDecMargins.bottom = (aPos->y + aPos->cy) - aRect.bottom;

            // WM_WINDOWPOSCHANGED is no good for checking client size changes
            // just ask DefWindowProcW() to generate WM_MOVE and WM_SIZE events
            break;

            /*StRectI_t aNewRect;
            aNewRect.left()   = aRect.left;
            aNewRect.top()    = aRect.top;
            aNewRect.right()  = aRect.right;
            aNewRect.bottom() = aRect.bottom;

            const bool isMoved = aNewRect.left()   != myRectNorm.left()
                              || aNewRect.top()    != myRectNorm.top();
            const bool isSized = aNewRect.width()  != myRectNorm.width()
                              || aNewRect.height() != myRectNorm.height();
            if(!isMoved && !isSized) {
                // nothing has changed
                return 0;
            }

            myRectNorm = aNewRect;
            myIsUpdated = true;
            if(isSized) {
                myStEvent.Size.init(getEventTime(myEvent.time), myRectNorm.width(), myRectNorm.height(), myForcedAspect);
                myEventsBuffer.append(myStEvent);
            }
            return 0; // we don't want to receive also WM_MOVE and WM_SIZE events generated by DefWindowProcW()
            */
        }
        // WM_WINDOWPOSCHANGING can be used only within WM_WINDOWPOSCHANGED
        /*case WM_WINDOWPOSCHANGING: {
            if(attribs.IsFullScreen || myParentWin != NULL) {
                return 0;
            } else if(theWin != myMaster.hWindow) {
                break;
            }

            WINDOWPOS* aDragRect = (WINDOWPOS* )lParam;
            StRectI_t aNewRect;
            aNewRect.left()   = aDragRect->x                 + myDecMargins.left;
            aNewRect.right()  = aDragRect->x + aDragRect->cx - myDecMargins.right;
            aNewRect.top()    = aDragRect->y                 + myDecMargins.top;
            aNewRect.bottom() = aDragRect->y + aDragRect->cy - myDecMargins.bottom;
            const StRectI_t aPrevRect = myRectNorm;
            if(attribs.ToAlignEven) {
                // adjust window position to ensure alignment
                const int aDL = getDirNorm(aPrevRect.left(),   aNewRect.left());
                const int aDR = getDirNorm(aPrevRect.right(),  aNewRect.right());
                const int aDT = getDirNorm(aPrevRect.top(),    aNewRect.top());
                const int aDB = getDirNorm(aPrevRect.bottom(), aNewRect.bottom());
                if(isOddNumber(aNewRect.left())) {
                    aNewRect.left() += aDL;
                    aDragRect->x    += aDL;
                    aDragRect->cx   -= aDL;
                }
                if(isEvenNumber(aNewRect.right())) {
                    aNewRect.right()  += aDR;
                    aDragRect->cx     += aDR;
                }
                if(isOddNumber(aNewRect.top())) {
                    aNewRect.top()    += aDT;
                    aDragRect->y      += aDT;
                    aDragRect->cy     -= aDT;
                }
                if(isEvenNumber(aNewRect.bottom())) {
                    aNewRect.bottom() += aDB;
                    aDragRect->cy     += aDB;
                }
            }

            // determine monitor scale factor change
            const bool isMoved   = myRectNorm.left()   != aNewRect.left()
                                || myRectNorm.top()    != aNewRect.top();
            const bool isResized = myRectNorm.width()  != aNewRect.width()
                                || myRectNorm.height() != aNewRect.height();
            myRectNorm = aNewRect;
            if(isMoved || isResized) {
                myIsUpdated = true;
            }
            if(isResized) {
                myStEvent.Size.init(getEventTime(myEvent.time), myRectNorm.width(), myRectNorm.height(), myForcedAspect);
                myEventsBuffer.append(myStEvent);
            }
            return 0;
        }*/
        case 0x02E0: { // WM_DPICHANGED
            if(attribs.IsFullScreen || myParentWin != NULL) {
                break;
            } else if(theWin != myMaster.hWindow) {
                break;
            }

            RECT* aNewRect = (RECT* )lParam;
            SetWindowPos(theWin, NULL,
                         aNewRect->left, aNewRect ->top, aNewRect->right - aNewRect->left, aNewRect->bottom - aNewRect->top,
                         SWP_NOZORDER | SWP_NOACTIVATE);
            break;
        }
        case WM_MOVE: {
            if(attribs.IsFullScreen || myParentWin != NULL) {
                break;
            } else if(theWin == myMaster.hWindow) {
                StRectI_t aNewRect = myRectNorm;
                const int aWidth   = aNewRect.width();
                const int aHeight  = aNewRect.height();
                aNewRect.left()   = (int )(short )LOWORD(lParam);
                aNewRect.top()    = (int )(short )HIWORD(lParam);
                aNewRect.right()  = aNewRect.left() + aWidth;
                aNewRect.bottom() = aNewRect.top() + aHeight;
                if(myRectNorm.left() != aNewRect.left()
                || myRectNorm.top()  != aNewRect.top()) {
                    myRectNorm = aNewRect;
                    myIsUpdated = true;
                }
                break;
            }
            // ignore GL subwindow resize messages!
            break;
        }
        case WM_SIZE: {
            if(attribs.IsFullScreen || myParentWin != NULL) {
                break;
            } else if(theWin == myMaster.hWindow) {
                const int aNewWidth  = LOWORD(lParam);
                const int aNewHeight = HIWORD(lParam);
                if(aNewWidth  == myRectNorm.width()
                && aNewHeight == myRectNorm.height()) {
                    // nothing has changed
                    break;
                }

                myRectNorm.right()  = myRectNorm.left() + aNewWidth;
                myRectNorm.bottom() = myRectNorm.top()  + aNewHeight;

                myIsUpdated = true;
                myStEvent.Size.init(getEventTime(myEvent.time), myRectNorm.width(), myRectNorm.height(), myForcedAspect);
                myEventsBuffer.append(myStEvent);
                break;
            }
            break;
        }
        case WM_MOVING:
        case WM_SIZING: {
            if(attribs.IsFullScreen || myParentWin != NULL) {
                break;
            } else if(theWin != myMaster.hWindow) {
                break;
            }

            RECT* aDragRect = (RECT* )lParam;
            StRectI_t aNewRect;
            aNewRect.left()   = aDragRect->left   + myDecMargins.left;
            aNewRect.right()  = aDragRect->right  - myDecMargins.right;
            aNewRect.top()    = aDragRect->top    + myDecMargins.top;
            aNewRect.bottom() = aDragRect->bottom - myDecMargins.bottom;
            const StRectI_t aPrevRect = myRectNorm;
            if(attribs.ToAlignEven) {
                // revert extra shift applied on the previous step
                aNewRect.left()   -= myAlignDL;
                aDragRect->left   -= myAlignDL;
                aNewRect.right()  -= myAlignDR;
                aDragRect->right  -= myAlignDR;
                aNewRect.top()    -= myAlignDT;
                aDragRect->top    -= myAlignDT;
                aNewRect.bottom() -= myAlignDB;
                aDragRect->bottom -= myAlignDB;
                myAlignDL = 0;
                myAlignDR = 0;
                myAlignDT = 0;
                myAlignDB = 0;

                // adjust window position to ensure alignment
                if(!isOddNumber(aNewRect.bottom())) {
                    if(isOddNumber(aPrevRect.bottom())) {
                        myAlignDB = 1;
                    }
                    aNewRect.bottom() += 1;
                    aDragRect->bottom += 1;
                }
                if(uMsg == WM_MOVING) {
                    int aNewTop = aNewRect.bottom() - aPrevRect.height();
                    myAlignDT = aNewTop - aNewRect.top();
                    aNewRect.top() += myAlignDT;
                    aDragRect->top += myAlignDT;
                } else if(!isEvenNumber(aNewRect.top())) {
                    if(isEvenNumber(aPrevRect.top())) {
                        myAlignDT = 1;
                    }
                    aNewRect.top() += 1;
                    aDragRect->top += 1;
                }

                if(!isEvenNumber(aNewRect.left())) {
                    if(isEvenNumber(aPrevRect.left())) {
                        myAlignDL = 1;
                    }
                    aNewRect.left() += 1;
                    aDragRect->left += 1;
                }
                if(uMsg == WM_MOVING) {
                    int aNewRight = aNewRect.left() + aPrevRect.width();
                    myAlignDR = aNewRight - aNewRect.right();
                    aNewRect.right() += myAlignDR;
                    aDragRect->right += myAlignDR;
                } else if(!isOddNumber(aNewRect.right())) {
                    if(isOddNumber(aPrevRect.right())) {
                        myAlignDR = 1;
                    }
                    aNewRect.right() += 1;
                    aDragRect->right += 1;
                }
            }

            // determine monitor scale factor change
            const bool isMoved   = myRectNorm.left()   != aNewRect.left()
                                || myRectNorm.top()    != aNewRect.top();
            const bool isResized = myRectNorm.width()  != aNewRect.width()
                                || myRectNorm.height() != aNewRect.height();
            myRectNorm = aNewRect;
            if(isMoved || isResized) {
                myIsUpdated = true;
            }
            if(isResized) {
                myStEvent.Size.init(getEventTime(myEvent.time), myRectNorm.width(), myRectNorm.height(), myForcedAspect);
                myEventsBuffer.append(myStEvent);
            }
            break;
        }
        case WM_HOTKEY: {
            // notice - unpress event will NOT be generated!
            myStEvent.Type      = stEvent_KeyDown;
            myStEvent.Key.Time  = getEventTime(myEvent.time);
            myStEvent.Key.Flags = ST_VF_NONE;
            if(wParam == myMaster.myMKeyStop) {
                //myKeysState.keyDown(ST_VK_MEDIA_STOP, aTime);
                myStEvent.Key.VKey = ST_VK_MEDIA_STOP;
                myEventsBuffer.append(myStEvent);
            } else if(wParam == myMaster.myMKeyPlay) {
                //myKeysState.keyDown(ST_VK_MEDIA_PLAY_PAUSE, aTime);
                myStEvent.Key.VKey = ST_VK_MEDIA_PLAY_PAUSE;
                myEventsBuffer.append(myStEvent);
            } else if(wParam == myMaster.myMKeyPrev) {
                //myKeysState.keyDown(ST_VK_MEDIA_PREV_TRACK, aTime);
                myStEvent.Key.VKey = ST_VK_MEDIA_PREV_TRACK;
                myEventsBuffer.append(myStEvent);
            } else if(wParam == myMaster.myMKeyNext) {
                //myKeysState.keyDown(ST_VK_MEDIA_NEXT_TRACK, aTime);
                myStEvent.Key.VKey = ST_VK_MEDIA_NEXT_TRACK;
                myEventsBuffer.append(myStEvent);
            }
            break;
        }
        // mouse lookup
        //case WM_LBUTTONDBLCLK: // left double click
        //case WM_MBUTTONDBLCLK: // right double click
        case WM_LBUTTONUP:
        case WM_RBUTTONUP:
        case WM_MBUTTONUP:
        case WM_XBUTTONUP:
        case WM_LBUTTONDOWN:
        case WM_RBUTTONDOWN:
        case WM_MBUTTONDOWN:
        case WM_XBUTTONDOWN: {
            int mouseXPx = int(short(LOWORD(lParam)));
            int mouseYPx = int(short(HIWORD(lParam)));
            const StRectI_t aWinRect = getPlacement();
            switch(myTiledCfg) {
                case TiledCfg_SlaveMasterX: {
                    mouseXPx -= aWinRect.width();
                    break;
                }
                case TiledCfg_SlaveMasterY: {
                    mouseYPx -= aWinRect.height();
                    break;
                }
                case TiledCfg_MasterSlaveX:
                case TiledCfg_MasterSlaveY:
                case TiledCfg_Separate:
                default: {
                    break;
                }
            }

            StPointD_t aPnt(double(mouseXPx) / double(aWinRect.width()),
                            double(mouseYPx) / double(aWinRect.height()));
            StVirtButton aBtnId = ST_NOMOUSE;
            switch(uMsg) {
                case WM_LBUTTONUP:
                case WM_LBUTTONDOWN: aBtnId = ST_MOUSE_LEFT; break;
                case WM_RBUTTONUP:
                case WM_RBUTTONDOWN: aBtnId = ST_MOUSE_RIGHT; break;
                case WM_MBUTTONUP:
                case WM_MBUTTONDOWN: aBtnId = ST_MOUSE_MIDDLE; break;
                case WM_XBUTTONUP:
                case WM_XBUTTONDOWN: aBtnId = (HIWORD(wParam) == XBUTTON1) ? ST_MOUSE_X1 : ST_MOUSE_X2; break;
            }

            myStEvent.Button.Time    = getEventTime(myEvent.time);
            myStEvent.Button.Button  = aBtnId;
            myStEvent.Button.Buttons = 0;
            myStEvent.Button.PointX  = aPnt.x();
            myStEvent.Button.PointY  = aPnt.y();

            switch(uMsg) {
                case WM_LBUTTONUP:
                case WM_RBUTTONUP:
                case WM_MBUTTONUP:
                case WM_XBUTTONUP: {
                    // TODO (Kirill Gavrilov#9) what if we have some another not unclicked mouse button?
                    ReleaseCapture();
                    myStEvent.Type = stEvent_MouseUp;
                    myEventsBuffer.append(myStEvent);
                    break;
                }
                case WM_LBUTTONDOWN:
                case WM_RBUTTONDOWN:
                case WM_MBUTTONDOWN:
                case WM_XBUTTONDOWN: {
                    // to receive out-of-window unclick message
                    SetFocus  (theWin);
                    SetCapture(theWin);

                    myStEvent.Type = stEvent_MouseDown;
                    myEventsBuffer.append(myStEvent);
                    break;
                }
            }
            return 0;
        }
        case WM_MOUSEWHEEL:    // vertical wheel
        case WM_MOUSEHWHEEL:   // horizontal wheel (only Vista+)
        {
            const StRectI_t aWinRect = getPlacement();
            int aMouseXPx = int(short(LOWORD(lParam))) - aWinRect.left();
            int aMouseYPx = int(short(HIWORD(lParam))) - aWinRect.top();

            const bool  isVert   = (uMsg == WM_MOUSEWHEEL);
            const int   aZDelta  = GET_WHEEL_DELTA_WPARAM(wParam);
            const float aDeltaSt = float(aZDelta) / float(WHEEL_DELTA);

            myStEvent.Scroll.init(getEventTime(myEvent.time),
                                  double(aMouseXPx) / double(aWinRect.width()),
                                  double(aMouseYPx) / double(aWinRect.height()),
                                  !isVert ? aDeltaSt : 0.0f,
                                   isVert ? aDeltaSt : 0.0f,
                                  false);
            if((myStEvent.Scroll.Time - myScrollAcc.Time) > 0.1
             || std::abs(aMouseXPx - (int)myScrollAcc.PointX) > 10
             || std::abs(aMouseYPx - (int)myScrollAcc.PointY) > 10) {
                myScrollAcc.reset();
            }
            myScrollAcc.Time = myStEvent.Scroll.Time;
            myScrollAcc.PointX = aMouseXPx;
            myScrollAcc.PointY = aMouseYPx;
            myStEvent.Scroll.StepsX = myScrollAcc.accumulateStepsX(!isVert ? aZDelta : 0, WHEEL_DELTA);
            myStEvent.Scroll.StepsY = myScrollAcc.accumulateStepsY( isVert ? aZDelta : 0, WHEEL_DELTA);
            myEventsBuffer.append(myStEvent);
            return 0;
        }
        case WM_TOUCH: {
            int aNbTouches = LOWORD(wParam);
            //ST_DEBUG_LOG(" @@ WM_TOUCH " + aNbTouches);
            if(aNbTouches < 1
            || myGetTouchInputInfo == NULL) {
                break;
            }

            if(aNbTouches > myNbTmpTouches) {
                stMemFree(myTmpTouches);
                myNbTmpTouches = stMax(aNbTouches, 8);
                myTmpTouches   = stMemAlloc<TOUCHINPUT*>(sizeof(TOUCHINPUT) * myNbTmpTouches);
            }
            if(myTmpTouches == NULL) {
                break;
            }

            if(!myGetTouchInputInfo((HTOUCHINPUT )lParam, aNbTouches, myTmpTouches, sizeof(TOUCHINPUT))) {
                break;
            }

            const StRectI_t aWinRect  = getPlacement();
            const float     aWinSizeX = float(aWinRect.width());
            const float     aWinSizeY = float(aWinRect.height());
            myStEvent.Touch.NbTouches = 0;
            myStEvent.Type = stEvent_TouchMove;
            for(size_t aTouchIter = 0; aTouchIter < ST_MAX_TOUCHES; ++aTouchIter) {
                StTouch& aTouch = myStEvent.Touch.Touches[aTouchIter];
                aTouch = StTouch::Empty();
                if(aTouchIter >= size_t(aNbTouches)) {
                    continue;
                }

                const TOUCHINPUT& aTouchSrc = myTmpTouches[aTouchIter];
                if((aTouchSrc.dwFlags & TOUCHEVENTF_UP) == TOUCHEVENTF_UP) {
                    myStEvent.Type = stEvent_TouchUp;
                    continue;
                } else if((aTouchSrc.dwFlags & TOUCHEVENTF_DOWN) == TOUCHEVENTF_DOWN) {
                    myStEvent.Type = stEvent_TouchDown;
                }

                ++myStEvent.Touch.NbTouches;
                aTouch.Id       = aTouchSrc.dwID;
                aTouch.DeviceId = (size_t )aTouchSrc.hSource;
                aTouch.OnScreen = true; // how to test?

                const float aPosX = float(aTouchSrc.x) * 0.01f;
                const float aPosY = float(aTouchSrc.y) * 0.01f;
                aTouch.PointX = (aPosX - float(aWinRect.left())) / aWinSizeX;
                aTouch.PointY = (aPosY - float(aWinRect.top()))  / aWinSizeY;
            }

            myCloseTouchInputHandle((HTOUCHINPUT )lParam);
            myEventsBuffer.append(myStEvent);
            return 0;
        }
        case WM_GESTURE: {
            //ST_DEBUG_LOG("WM_GESTURE");
            break;
        }
    }
    // Pass All Unhandled Messages To DefWindowProc
    return DefWindowProcW(theWin, uMsg, wParam, lParam);
}

/**
 * In this function we fullscreen only Master window,
 * Slave window resized in update procedure.
 */
void StWindowImpl::setFullScreen(bool theFullscreen) {
    if(attribs.IsFullScreen != theFullscreen) {
        attribs.IsFullScreen = theFullscreen;
        if(attribs.IsFullScreen) {
            myFullScreenWinNb.increment();
        } else {
            myFullScreenWinNb.decrement();
        }
    }

    if(myMaster.hWindow != NULL) {
        ShowWindow(myMaster.hWindow, SW_HIDE);
    }
    if(attribs.IsFullScreen) {
        HWND aWin = myMaster.hWindow;
        if(myParentWin != NULL) {
            // embedded
            aWin = myMaster.hWindowGl;
            SetParent(myMaster.hWindowGl, NULL);
            SetFocus (myMaster.hWindowGl);
        }

        // generic
        const StMonitor& stMon = (myMonMasterFull == -1) ? myMonitors[myRectNorm.center()] : myMonitors[myMonMasterFull];
        myRectFull = stMon.getVRect();
        SetWindowLongPtr(aWin, GWL_STYLE, WS_POPUP);
        SetWindowPos(aWin, HWND_TOP,
                     myRectFull.left(),  myRectFull.top(),
                     myRectFull.width(), myRectFull.height(),
                     !attribs.IsHidden ? SWP_SHOWWINDOW : SWP_NOACTIVATE); // show window

        // use tiled Master+Slave layout within single window if possible
        if(attribs.Slave != StWinSlave_slaveOff && isSlaveIndependent()) {
            StRectI_t aRectSlave;
            aRectSlave.left()   = getSlaveLeft();
            aRectSlave.top()    = getSlaveTop();
            aRectSlave.right()  = aRectSlave.left() + myRectFull.width();
            aRectSlave.bottom() = aRectSlave.top()  + myRectFull.height();
            myTiledCfg = TiledCfg_Separate;
            if(myRectFull.top()   == aRectSlave.top()) {
                if(myRectFull.right() == aRectSlave.left()) {
                    myTiledCfg = TiledCfg_MasterSlaveX;
                } else if(myRectFull.left() == aRectSlave.right()) {
                    myTiledCfg = TiledCfg_SlaveMasterX;
                }
            } else if(myRectFull.left() == aRectSlave.left()) {
                if(myRectFull.bottom() == aRectSlave.top()) {
                    myTiledCfg = TiledCfg_MasterSlaveY;
                } else if(myRectFull.top() == aRectSlave.bottom()) {
                    myTiledCfg = TiledCfg_SlaveMasterY;
                }
            }
        } else if(attribs.Split == StWinSlave_splitHorizontal) {
            myTiledCfg = TiledCfg_MasterSlaveX;
            myRectFull.right() -= myRectFull.width() / 2;
        } else if(attribs.Split == StWinSlave_splitVertical) {
            myTiledCfg = TiledCfg_MasterSlaveY;
            myRectFull.bottom() -= myRectFull.height() / 2;
        }

        if(!attribs.IsHidden
        && myParentWin == NULL) {
            SetFocus(myMaster.hWindow);
        }
    } else {
        if(myParentWin != NULL) {
            // embedded
            SetParent(myMaster.hWindowGl, myParentWin);
        } else if(!attribs.IsNoDecor) {
            SetWindowLongPtr(myMaster.hWindow, GWL_STYLE, WS_OVERLAPPEDWINDOW);
        }

        RECT aRect;
        aRect.top    = myRectNorm.top();
        aRect.bottom = myRectNorm.bottom();
        aRect.left   = myRectNorm.left();
        aRect.right  = myRectNorm.right();

        // parent Master window could be decorated - we parse this situation to get true coordinates
        const DWORD aWinStyle   = (!attribs.IsNoDecor ? WS_OVERLAPPEDWINDOW : WS_POPUP) | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
        const DWORD aWinStyleEx = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE | WS_EX_ACCEPTFILES;
        ::AdjustWindowRectEx(&aRect, aWinStyle, FALSE, aWinStyleEx);

        if(myMaster.hWindow != NULL) {
            SetWindowPos(myMaster.hWindow, HWND_NOTOPMOST,
                         aRect.left, aRect.top, aRect.right - aRect.left, aRect.bottom - aRect.top,
                         !attribs.IsHidden ? SWP_SHOWWINDOW : SWP_NOACTIVATE);
            if(!attribs.IsHidden
            && myParentWin == NULL) {
                SetFocus(myMaster.hWindow);
            }
        }
    }

    const StRectI_t& aRect = attribs.IsFullScreen ? myRectFull : myRectNorm;
    StEvent anEvent;
    anEvent.Size.init(getEventTime(), aRect.width(), aRect.height(), myForcedAspect);
    if(StThread::getCurrentThreadId() == myMaster.ThreadGL) {
        updateWindowPos();
        signals.onResize->emit(anEvent.Size);
    } else {
        // in general setFullScreen should be called only within StWindow thread
        // but if not - prevent access to OpenGL context from wrong thread
        myIsUpdated = true;
        myEventsBuffer.append(anEvent);
    }
}

void StWindowImpl::updateWindowPos() {
    if(myMaster.hWindowGl == NULL) {
        return;
    }

    if(attribs.Slave != StWinSlave_slaveOff && !attribs.IsSlaveHidden && (!isSlaveIndependent() || myMonitors.size() > 1)) {
        HWND afterHWND = myParentWin != NULL ? myParentWin : myMaster.hWindow;
        UINT aFlags    = SWP_NOACTIVATE;
        if(attribs.Slave == StWinSlave_slaveHLineTop
        || attribs.Slave == StWinSlave_slaveHTop2Px
        || attribs.Slave == StWinSlave_slaveHLineBottom) {
            afterHWND = HWND_TOPMOST;
        }

        if(!attribs.IsFullScreen
        && myTiledCfg != TiledCfg_Separate) {
            myTiledCfg = TiledCfg_Separate;
            if(!attribs.IsHidden) {
                aFlags = SWP_SHOWWINDOW;
            }
        }

        // resize Slave GL-window
        if(!attribs.IsFullScreen || myTiledCfg == TiledCfg_Separate) {
            SetWindowPos(mySlave.hWindowGl, afterHWND,
                         getSlaveLeft(),  getSlaveTop(),
                         getSlaveWidth(), getSlaveHeight(),
                         aFlags);
        }
    }

    if(!attribs.IsHidden) {
        if(attribs.IsFullScreen && myTiledCfg != TiledCfg_Separate) {
            ShowWindow(mySlave.hWindowGl, SW_HIDE);
            StRectI_t aRect = myRectFull;
            getTiledWinRect(aRect);

            if(myMaster.hWindow != NULL) {
                SetWindowPos(myMaster.hWindow,
                             HWND_TOP,
                             aRect.left(),  aRect.top(),
                             aRect.width(), aRect.height(),
                             SWP_NOACTIVATE);

                // resize Master GL-subwindow
                SetWindowPos(myMaster.hWindowGl, HWND_TOP,
                             0, 0, aRect.width(), aRect.height(),
                             SWP_NOACTIVATE);
            } else {
                // embedded
                SetWindowPos(myMaster.hWindowGl, HWND_TOPMOST,
                             aRect.left(),  aRect.top(),
                             aRect.width(), aRect.height(),
                             SWP_NOACTIVATE);
            }
        } else if(attribs.IsFullScreen
               && myParentWin != NULL) {
            // embedded
            SetWindowPos(myMaster.hWindowGl, HWND_TOPMOST,
                         myRectFull.left(),  myRectFull.top(),
                         myRectFull.width(), myRectFull.height(),
                         SWP_NOACTIVATE);
        } else {
            // resize Master GL-subwindow
            myTiledCfg = TiledCfg_Separate;
            int aTop   = 0;
            int aSizeX = (attribs.IsFullScreen) ? myRectFull.width()  : myRectNorm.width();
            int aSizeY = (attribs.IsFullScreen) ? myRectFull.height() : myRectNorm.height();
            if (attribs.IsFullScreen
            && !attribs.IsExclusiveFullScr)
            {
              // workaround slow switching into fullscreen mode and back on Windows 10
              // due to OpenGL driver implicitly activating an exclusive GPU usage mode
              aTop   -= 2;
              aSizeY += 2;
            }
            SetWindowPos(myMaster.hWindowGl, HWND_TOP,
                         0, aTop, aSizeX, aSizeY,
                         SWP_NOACTIVATE);
        }
    }

    // detect when window moved to another monitor
    if(!attribs.IsFullScreen && myMonitors.size() > 1) {
        const StMonitor& aMonTo    = myMonitors[myRectNorm.center()];
        const int        aNewMonId = aMonTo.getId();
        if(myWinOnMonitorId != aNewMonId) {
            myStEventAux.Size.init(getEventTime(), myRectNorm.width(), myRectNorm.height(), myForcedAspect);
            myStEventAux.Type = stEvent_NewMonitor;
            myWinOnMonitorId = aNewMonId;
            signals.onAnotherMonitor->emit(myStEventAux.Size);
        }
    }
}

// Function set to argument-buffer given events and return events number
void StWindowImpl::processEvents() {
    if(myIsDispChanged) {
        updateMonitors();
    }

    if(myMaster.hWindowGl == NULL) {
        // window is closed!
        return;
    }

    // detect embedded window was moved
    if(myParentWin != NULL && myMaster.hWindowGl != NULL && !attribs.IsFullScreen) {
        myPointTest.x = 0;
        myPointTest.y = 0;
        if(ClientToScreen(myMaster.hWindowGl, &myPointTest) != FALSE
        && (myPointTest.x != myRectNorm.left() || myPointTest.y != myRectNorm.top())) {
            updateChildRect();
        }
    }

    if(myIsUpdated) {
        if(myMaster.hWindow != NULL) {
            SetWindowTextW(myMaster.hWindow, myWindowTitle.toUtfWide().toCString());
        }
        updateWindowPos();
        myIsUpdated = false;
    }
    updateActiveState();

    // get callback for current thread - just null cycle...
    if(PeekMessage(&myEvent, NULL, 0, 0, PM_REMOVE)) {
        if(myEvent.message == WM_QUIT) {
            //isOut = true;
        } else {
            TranslateMessage(&myEvent);
            DispatchMessageW(&myEvent);
        }
    }

    StPointD_t aNewMousePt = getMousePos();
    myIsMouseMoved = false;
    if(aNewMousePt.x() >= 0.0 && aNewMousePt.x() <= 1.0 && aNewMousePt.y() >= 0.0 && aNewMousePt.y() <= 1.0) {
        StPointD_t aDspl = aNewMousePt - myMousePt;
        if(std::abs(aDspl.x()) >= 0.0008 || std::abs(aDspl.y()) >= 0.0008) {
            myIsMouseMoved = true;
        }
    }
    myMousePt = aNewMousePt;

    swapEventsBuffers();
}

bool StWindowImpl::toClipboard(const StString& theText) {
    const StStringUtfWide aWideText = theText.toUtfWide();
    HGLOBAL aMem = ::GlobalAlloc(GMEM_MOVEABLE, aWideText.Size + sizeof(wchar_t));
    if(aMem == NULL) {
        return false;
    }

    stMemCpy(::GlobalLock(aMem), aWideText.String, aWideText.Size + sizeof(wchar_t));
    ::GlobalUnlock(aMem);
    if(!::OpenClipboard(NULL)) {
        return false;
    }

    ::EmptyClipboard();
    if(!::SetClipboardData(CF_UNICODETEXT, aMem)) {
        ::CloseClipboard();
        return false;
    }
    ::CloseClipboard();
    return true;
}

bool StWindowImpl::fromClipboard(StString& theText) {
    if(!::IsClipboardFormatAvailable(CF_UNICODETEXT)
    || !::OpenClipboard(NULL)) {
        return false;
    }

    HANDLE aClipData = ::GetClipboardData(CF_UNICODETEXT);
    bool hasText = false;
    if(aClipData != NULL) {
        const void* aStrData = ::GlobalLock(aClipData);
        if(aStrData != NULL) {
            theText.fromUnicode(reinterpret_cast<const wchar_t* >(aStrData));
            ::GlobalUnlock(aClipData);
            hasText = !theText.isEmpty();
        }
    }
    ::CloseClipboard();
    return hasText;
}

#endif
