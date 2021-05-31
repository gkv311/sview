/**
 * StOutPageFlip, class providing stereoscopic output for Shutter Glasses displays using StCore toolkit.
 * Copyright © 2009-2015 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#ifdef _WIN32

#include "StDXManager.h"
#include "StDXAqbsControl.h"

#include <StCore/StMonitor.h>
#include <StThreads/StCondition.h>
#include <StThreads/StThread.h>

#include <wnt/nvapi.h>
#ifdef _MSC_VER
    #ifdef _WIN64
        #pragma comment(lib, "nvapi64.lib")
    #else
        #pragma comment(lib, "nvapi.lib")
    #endif
    #pragma comment(linker, "/NODEFAULTLIB:libcmt.lib")
#endif

namespace {
    static const DWORD ST_D3D_DEVICE_FLAGS = D3DCREATE_HARDWARE_VERTEXPROCESSING; // sets the graphic card to do the hardware vertexprocessing

#ifdef ST_DEBUG
    static StString printD3dFormat(const D3DFORMAT theD3dFormat) {
        switch(theD3dFormat) {
            case D3DFMT_R8G8B8:       return "R8G8B8      (24bit)";
            case D3DFMT_A8R8G8B8:     return "A8R8G8B8    (24bit)";
            case D3DFMT_X8R8G8B8:     return "X8R8G8B8    (24bit)";
            case D3DFMT_R5G6B5:       return "R5G6B5      (16bit)";
            case D3DFMT_A2B10G10R10:  return "A2B10G10R10 (30bit)";
            case D3DFMT_A8B8G8R8:     return "A8B8G8R8    (24bit)";
            case D3DFMT_X8B8G8R8:     return "X8B8G8R8    (24bit)";
            case D3DFMT_A2R10G10B10:  return "A2R10G10B10 (30bit)";
            case D3DFMT_A16B16G16R16: return "A16B16G16R16(48bit)";
            default: return StString("D3DFORMAT(") + theD3dFormat + ")";
        }
    }

    static StString printDisplayFormat(const UINT      theSizeX,
                                       const UINT      theSizeY,
                                       const UINT      theRefreshRate,
                                       const D3DFORMAT theD3dFormat,
                                       const bool      theIsFullscreen = false) {
        return StString("") + (theIsFullscreen ? "F" : " ") + theSizeX + "x" + theSizeY + "@" + theRefreshRate + "Hz (" + printD3dFormat(theD3dFormat) + ")";
    }

    static StString printDisplayFormat(const D3DPRESENT_PARAMETERS& theD3dParams) {
        return printDisplayFormat(theD3dParams.BackBufferWidth, theD3dParams.BackBufferHeight,
                                  theD3dParams.FullScreen_RefreshRateInHz,
                                  theD3dParams.BackBufferFormat,
                                  theD3dParams.Windowed == FALSE);
    }

    static StString printDisplayFormat(const D3DDISPLAYMODE& theDispMode) {
        return printDisplayFormat(theDispMode.Width, theDispMode.Height, theDispMode.RefreshRate, theDispMode.Format);
    }
#endif // ST_DEBUG

}

StString StDXManager::printErrorDesc(HRESULT theErrCode) {
    switch(theErrCode) {
        case D3D_OK:                     return "OK";
        case D3DERR_DEVICELOST:          return "Device lost";
        case D3DERR_DEVICEREMOVED:       return "Device removed";
        case D3DERR_DRIVERINTERNALERROR: return "Driver internal error";
        case D3DERR_OUTOFVIDEOMEMORY:    return "Out of video memory";
        case D3DERR_INVALIDCALL:         return "Invalid call";
        default:                         return StString("Error #") + int(theErrCode) + ")";
    }
}

StDXManager::StDXManager()
: myD3dLib(NULL),
  myD3dDevice(NULL),
  myRefreshRate(D3DPRESENT_RATE_DEFAULT),
  myWithAqbs(false) {
    stMemSet(&myD3dParams, 0, sizeof(myD3dParams));
    stMemSet(&myCurrMode,  0, sizeof(myCurrMode));
    myD3dParams.Windowed         = FALSE;
    myD3dParams.SwapEffect       = D3DSWAPEFFECT_DISCARD; // discards the previous frames
    myD3dParams.BackBufferFormat = D3DFMT_R5G6B5;         // display format
    myD3dParams.BackBufferCount  = 1;                     // number of back buffers
    myD3dParams.BackBufferHeight = 2;
    myD3dParams.BackBufferWidth  = 2;
    myD3dParams.AutoDepthStencilFormat     = D3DFMT_D16_LOCKABLE;
    myD3dParams.EnableAutoDepthStencil     = FALSE; // no need for our purposes!
    myD3dParams.FullScreen_RefreshRateInHz = D3DPRESENT_RATE_DEFAULT;
    myD3dParams.PresentationInterval       = D3DPRESENT_INTERVAL_DEFAULT;
}

StDXManager::~StDXManager() {
    if(myD3dDevice != NULL) {
        myD3dDevice->Release();
        myD3dDevice = NULL;
    }
    if(myD3dLib != NULL) {
        myD3dLib->Release();
        myD3dLib = NULL;
    }
}

bool StDXManager::initDxLib() {
    if(myD3dLib == NULL) {
        Direct3DCreate9Ex(D3D_SDK_VERSION, &myD3dLib);
    }
    return myD3dLib != NULL;
}

bool StDXManager::init(const HWND       theWinHandle,
                       const int        theSizeX,
                       const int        theSizeY,
                       const bool       theFullscreen,
                       const StMonitor& theMonitor,
                       const StDXAdapterClass theAdapter) {
    const StString anAdapterStr = theMonitor.getName().subString(0, 12);
    if(!initDxLib()) {
        stError("StDXManager, Direct3DCreate9 failed!");
        return false;
    }

    const UINT aD3dAdaptersNb = getAdapterCount();
    UINT anAdapterId     = UINT(-1);
    UINT anAdapterVendor = UINT(-1);
    D3DADAPTER_IDENTIFIER9 anAdapterInfo;
    for(UINT anAdapterIter = 0; anAdapterIter < aD3dAdaptersNb; ++anAdapterIter) {
        getAdapterIdentifier(anAdapterIter, 0, &anAdapterInfo);
        switch(theAdapter) {
            case ST_DX_ADAPTER_AMD: {
                if(anAdapterInfo.VendorId != ST_DX_VENDOR_AMD) {
                    continue;
                }
                anAdapterVendor = anAdapterIter;
                break;
            }
            case ST_DX_ADAPTER_NVIDIA: {
                if(anAdapterInfo.VendorId != ST_DX_VENDOR_NVIDIA) {
                    continue;
                }
                anAdapterVendor = anAdapterIter;
                break;
            }
            case ST_DX_ADAPTER_ANY:
            default:
                break;
        }
        if(anAdapterStr == StString(anAdapterInfo.DeviceName)) {
            anAdapterId = anAdapterIter;
            break;
        }
    }
    if(theAdapter != ST_DX_ADAPTER_ANY) {
        if(anAdapterId     == UINT(-1)
        && anAdapterVendor != UINT(-1)) {
            anAdapterId = anAdapterVendor;
        }
        if(anAdapterId == UINT(-1)) {
            return false;
        }
    }
    if(anAdapterId == UINT(-1)) {
        // the default adapter is the primary display adapter
        anAdapterId = D3DADAPTER_DEFAULT;
    }

    // setup the present parameters
    if(getAdapterDisplayMode(anAdapterId, &myCurrMode) == D3D_OK) {
        myD3dParams.BackBufferFormat = myCurrMode.Format;
        myRefreshRate = myCurrMode.RefreshRate;
    }
    myD3dParams.Windowed         = !theFullscreen;        // is windowed?
    myD3dParams.BackBufferWidth  = theSizeX;
    myD3dParams.BackBufferHeight = theSizeY;
    myD3dParams.hDeviceWindow    = theWinHandle;

    // create the Video Device
    myD3dDevice = createAqbsDevice(anAdapterId, theWinHandle, myD3dParams);
    if(myD3dDevice == NULL) {
        HRESULT isOK = myD3dLib->CreateDevice(anAdapterId, D3DDEVTYPE_HAL, // the HAL (hardware accelerated layer) uses your 3d accelerator card
                                              theWinHandle,
                                              ST_D3D_DEVICE_FLAGS,
                                              &myD3dParams, &myD3dDevice);
        if(isOK < 0) {
            return false;
        }
    }
    // this normalizes the normal values (this is important for how lighting effects your models)
    ///myD3dDevice->SetRenderState(D3DRS_NORMALIZENORMALS, TRUE);
    ST_DEBUG_LOG("Direct3D9, Created StDXManager device (WxH= " + theSizeX + "x"+ theSizeY + ")");
    return myD3dDevice != NULL;
}

bool StDXManager::checkAqbsSupport(const HWND theWinHandle) {
    if(!initDxLib()) {
        return false;
    }

    const UINT aD3dAdaptersNb = getAdapterCount();
    D3DADAPTER_IDENTIFIER9 anAdapterInfo;
    for(UINT anAdapterIter = 0; anAdapterIter < aD3dAdaptersNb; ++anAdapterIter) {
        getAdapterIdentifier(anAdapterIter, 0, &anAdapterInfo);
        if(anAdapterInfo.VendorId != ST_DX_VENDOR_AMD) {
            continue;
        }

        // setup the present parameters
        if(getAdapterDisplayMode(anAdapterIter, &myCurrMode) == D3D_OK) {
            myD3dParams.BackBufferFormat = myCurrMode.Format;
            myRefreshRate = myCurrMode.RefreshRate;
        }

        // create temporary video device
        myD3dParams.hDeviceWindow = theWinHandle;
        myD3dDevice = createAqbsTmpDevice(anAdapterIter, theWinHandle, myD3dParams);
        if(myD3dDevice == NULL) {
            continue;
        }

        // create a surface to be used to communicate with the driver
        StHandle<StDXAqbsControl> anAqbsControl = new StDXAqbsControl(myD3dDevice);
        if(!anAqbsControl->isValid()) {
            myD3dDevice->Release();
            myD3dDevice = NULL;
            return false;
        }

        // send the command to the driver using the temporary surface
        if(!anAqbsControl->enableStereo()) {
            anAqbsControl.nullify();
            myD3dDevice->Release();
            myD3dDevice = NULL;
            return false;
        }
        myWithAqbs = true;
        anAqbsControl.nullify();
        myD3dDevice->Release();
        myD3dDevice = NULL;
        return true;
    }
    return false;
}

IDirect3DDevice9* StDXManager::createAqbsTmpDevice(const UINT                   theAdapterId,
                                                   const HWND                   theWinHandle,
                                                   const D3DPRESENT_PARAMETERS& theD3dParams) {
    // first create a temporary windowed device
    IDirect3DDevice9* aD3dDevTmp = NULL;
    D3DPRESENT_PARAMETERS aD3dParamsTmp = theD3dParams;
    aD3dParamsTmp.Windowed = TRUE;
    aD3dParamsTmp.SwapEffect = D3DSWAPEFFECT_COPY;
    aD3dParamsTmp.BackBufferCount  = 1;
    aD3dParamsTmp.BackBufferWidth  = 2;
    aD3dParamsTmp.BackBufferHeight = 2;
    aD3dParamsTmp.EnableAutoDepthStencil = FALSE;
    aD3dParamsTmp.Flags = 0;
    aD3dParamsTmp.FullScreen_RefreshRateInHz = D3DPRESENT_RATE_DEFAULT;
    aD3dParamsTmp.PresentationInterval = D3DPRESENT_INTERVAL_DEFAULT;
    aD3dParamsTmp.MultiSampleType = D3DMULTISAMPLE_NONE;

    if(myD3dLib->CreateDevice(theAdapterId, D3DDEVTYPE_HAL, theWinHandle, ST_D3D_DEVICE_FLAGS, &aD3dParamsTmp, &aD3dDevTmp) < 0) {
        ST_DEBUG_LOG("StDXManager::createAqbsDevice(), failed to create temporary D3D device");
        return NULL;
    }
    return aD3dDevTmp;
}

IDirect3DDevice9* StDXManager::createAqbsDevice(const UINT                   theAdapterId,
                                                const HWND                   theWinHandle,
                                                const D3DPRESENT_PARAMETERS& theD3dParams) {
    // first create a temporary windowed device
    IDirect3DDevice9* aD3dDevTmp = StDXManager::createAqbsTmpDevice(theAdapterId, theWinHandle, theD3dParams);
    if(aD3dDevTmp == NULL) {
        return NULL;
    }

    // create a surface to be used to communicate with the driver
    StHandle<StDXAqbsControl> anAqbsControl = new StDXAqbsControl(aD3dDevTmp);
    if(!anAqbsControl->isValid()) {
        ST_DEBUG_LOG("StDXManager::createAqbsDevice(), fail to create AQBS sufrace");
        aD3dDevTmp->Release();
        aD3dDevTmp = NULL;
        return NULL;
    }

    // send the command to the driver using the temporary surface
    if(!anAqbsControl->enableStereo()) {
        ST_DEBUG_LOG("StDXManager::createAqbsDevice(), fail to enable stereo via AQBS sufrace");
        anAqbsControl.nullify();
        aD3dDevTmp->Release();
        aD3dDevTmp = NULL;
        return NULL;
    }
    myWithAqbs = true;

    // see what stereo modes are available
    ATIDX9GETDISPLAYMODES aDispModeParams;
    stMemSet(&aDispModeParams, 0, sizeof(ATIDX9GETDISPLAYMODES));

    // send stereo command to get the number of available stereo modes.
    if(!anAqbsControl->sendCommand(ATI_STEREO_GETDISPLAYMODES,
                                   (BYTE* )&aDispModeParams, sizeof(ATIDX9GETDISPLAYMODES))) {
        ST_DEBUG_LOG("StDXManager::createAqbsDevice(), fail to enumerate stereo modes via AQBS sufrace");
        anAqbsControl.nullify();
        aD3dDevTmp->Release();
        aD3dDevTmp = NULL;
        return NULL;
    }

    if(aDispModeParams.dwNumModes != 0) {
        // allocating memory to get the list of modes.
        aDispModeParams.pStereoModes = new D3DDISPLAYMODE[aDispModeParams.dwNumModes];

        // send stereo command to get the list of stereo modes
        if(!anAqbsControl->sendCommand(ATI_STEREO_GETDISPLAYMODES,
                                       (BYTE* )&aDispModeParams, sizeof(ATIDX9GETDISPLAYMODES))) {
            ST_DEBUG_LOG("StDXManager::createAqbsDevice(), fail to retrieve stereo modes via AQBS sufrace");
            anAqbsControl.nullify();
            aD3dDevTmp->Release();
            aD3dDevTmp = NULL;
            delete[] aDispModeParams.pStereoModes;
            return NULL;
        }
    }
    anAqbsControl.nullify();

    int aResFormatMatch = -1;
///ST_DEBUG_LOG(" DX CUDD " + printDisplayFormat(theD3dParams));
///ST_DEBUG_LOG(" DX CURR " + printDisplayFormat(myCurrMode));
    for(int aDispModeIter = int(aDispModeParams.dwNumModes - 1); aDispModeIter >= 0; --aDispModeIter) {
        const D3DDISPLAYMODE& aDispMode = aDispModeParams.pStereoModes[aDispModeIter];
///ST_DEBUG_LOG(" DX ST  " + printDisplayFormat(aDispMode));
        if(aDispMode.Width  != theD3dParams.BackBufferWidth
        || aDispMode.Height != theD3dParams.BackBufferHeight
        || aDispMode.Format != theD3dParams.BackBufferFormat) {
            continue;
        }
        aResFormatMatch = aDispModeIter;
        break;
    }

    if(aResFormatMatch < 0) {
        ST_DEBUG_LOG("StDXManager::createAqbsDevice(), stereo display format doesn't found");
        aD3dDevTmp->Release();
        aD3dDevTmp = NULL;
        delete[] aDispModeParams.pStereoModes;
        return NULL;
    }

    int aRefreshMatch = -1;
    UINT aRefreshMax = 0;
    for(int aDispModeIter = aResFormatMatch; aDispModeIter >= 0; --aDispModeIter) {
        const D3DDISPLAYMODE& aDispMode = aDispModeParams.pStereoModes[aDispModeIter];
ST_DEBUG_LOG(" DX ST  " + printDisplayFormat(aDispMode));
        if(aDispMode.Width  != theD3dParams.BackBufferWidth
        || aDispMode.Height != theD3dParams.BackBufferHeight
        || aDispMode.Format != theD3dParams.BackBufferFormat) {
            continue;
        }
        if(aDispMode.RefreshRate == myRefreshRate) {
            aRefreshMatch = aDispModeIter; // found a match with the current refresh
            break;
        } else if(aDispMode.RefreshRate > aRefreshMax) {
            aRefreshMax = aDispMode.RefreshRate;
            aRefreshMatch = aDispModeIter;
        }
    }

ST_DEBUG_LOG(" DXSSS  " + printDisplayFormat(aDispModeParams.pStereoModes[aRefreshMatch]));

    // a valid multisample value other then 0 or 1 must be set for stereo (ex 2)
    D3DPRESENT_PARAMETERS aD3dParams = theD3dParams;
    aD3dParams.MultiSampleType = D3DMULTISAMPLE_2_SAMPLES;
    aD3dParams.Flags = 0; // can't lock the back buffer
    aD3dParams.EnableAutoDepthStencil     = FALSE; // need to create a special depth buffer
    aD3dParams.FullScreen_RefreshRateInHz = aDispModeParams.pStereoModes[aRefreshMatch].RefreshRate;
    aD3dParams.BackBufferFormat           = aDispModeParams.pStereoModes[aRefreshMatch].Format;
    myRefreshRate = aDispModeParams.pStereoModes[aRefreshMatch].RefreshRate;
    delete[] aDispModeParams.pStereoModes;

    // override original parameters
    myD3dParams = aD3dParams;
    return aD3dDevTmp;
}

bool StDXManager::reset(const HWND theWinHandle,
                        const int  theSizeX,
                        const int  theSizeY,
                        const bool theFullscreen) {
    if(myD3dDevice == NULL) {
        return false;
    }

    myD3dParams.Windowed         = !theFullscreen;        // is windowed?
    myD3dParams.BackBufferWidth  = theSizeX;
    myD3dParams.BackBufferHeight = theSizeY;
    myD3dParams.hDeviceWindow    = theWinHandle;
    myD3dParams.FullScreen_RefreshRateInHz = theFullscreen ? myRefreshRate : 0;

    HRESULT isOK = myD3dDevice->Reset(&myD3dParams);
    ST_DEBUG_LOG("Direct3D9, Reset device (" + printDisplayFormat(myD3dParams) + "), " + printErrorDesc(isOK));
    return isOK == D3D_OK;
}

void StDXManager::beginRender() {
    if(myD3dDevice == NULL) {
        return;
    }

    // clear the back buffer
    myD3dDevice->Clear(0, NULL, D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(0, 0, 0), 1.0f, 0);
    myD3dDevice->BeginScene();
}

void StDXManager::endRender() {
    if(myD3dDevice != NULL) {
        myD3dDevice->EndScene();
    }
}

bool StDXManager::swap() {
    if(myD3dDevice == NULL) {
        return false;
    }

    const HRESULT isOK = myD3dDevice->Present(NULL, NULL, NULL, NULL);
    if(isOK != D3D_OK) {
        ST_DEBUG_LOG("Direct3D9, Present device failed, " + printErrorDesc(isOK));
    }
    return isOK == D3D_OK;
}

namespace {

    static LRESULT CALLBACK aDummyWinProc(HWND in_hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
        return DefWindowProcW(in_hWnd, uMsg, wParam, lParam);
    }

    static volatile bool ST_DX_HAS_CACHED_INFO = false;
    static StDXInfo      ST_DX_CACHED_INFO;

}

bool StDXManager::getInfo(StDXInfo&  theInfo,
                          const bool theForced) {
    if(!theForced) {
        if(ST_DX_HAS_CACHED_INFO) {
            theInfo = ST_DX_CACHED_INFO;
            return true;
        }
    }

    const StStringUtfWide AQBS_TEST_CLASS = L"StTESTAqbsWin";
    HINSTANCE anAppInst = GetModuleHandle(NULL);
    WNDCLASSW aWinClass;
    aWinClass.style         = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    aWinClass.lpfnWndProc   = (WNDPROC )aDummyWinProc;
    aWinClass.cbClsExtra    = 0;
    aWinClass.cbWndExtra    = 0;
    aWinClass.hInstance     = anAppInst;
    aWinClass.hIcon         = LoadIcon(anAppInst, L"A");
    aWinClass.hCursor       = LoadCursor(NULL, IDC_ARROW);
    aWinClass.hbrBackground = NULL;
    aWinClass.lpszMenuName  = NULL;
    aWinClass.lpszClassName = AQBS_TEST_CLASS.toCString(); // class name
    if(RegisterClassW(&aWinClass) == 0) {
        ST_DEBUG_LOG("Failed to RegisterClass '" + AQBS_TEST_CLASS.toUtf8() + "'");
        return false;
    }
    HWND aWinHandle = CreateWindowExW(WS_EX_TOOLWINDOW | WS_EX_WINDOWEDGE | WS_EX_NOACTIVATE,
                                      AQBS_TEST_CLASS.toCString(), L"StTESTAqbs Window",
                                      WS_POPUP | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
                                      32, 32, 32, 32, NULL, NULL, anAppInst, NULL);
    if(aWinHandle == NULL) {
        ST_DEBUG_LOG("Failed to create 'StTESTAqbsWin' window (" + int(GetLastError()) + ")");
        UnregisterClassW(AQBS_TEST_CLASS.toCString(), anAppInst);
        return false;
    }

    StHandle<StDXManager> aDXManager = new StDXManager();

    // test AQBS support
    theInfo.hasAqbsSupport = aDXManager->checkAqbsSupport(aWinHandle);

    // enumerate available adapters
    if(aDXManager->myD3dLib != NULL) {
        const UINT aD3dAdaptersNb = aDXManager->getAdapterCount();
        D3DADAPTER_IDENTIFIER9 anAdapterInfo;
        for(UINT anAdapterIter = 0; anAdapterIter < aD3dAdaptersNb; ++anAdapterIter) {
            aDXManager->getAdapterIdentifier(anAdapterIter, 0, &anAdapterInfo);
            if(anAdapterInfo.VendorId == ST_DX_VENDOR_AMD) {
                theInfo.hasAmdAdapter = true;
            } else if(anAdapterInfo.VendorId == ST_DX_VENDOR_NVIDIA) {
                theInfo.hasNvAdapter  = true;
            }
        }
    }

    // release resources
    aDXManager.nullify();
    DestroyWindow(aWinHandle);
    UnregisterClass(AQBS_TEST_CLASS.toCString(), anAppInst);

    // check NVIDIA Stereo enabled state
    if(NvAPI_Initialize() == NVAPI_OK) {
        NvU8 isStereoEnabled = FALSE;
        NvAPI_Stereo_IsEnabled(&isStereoEnabled);
        theInfo.hasNvStereoSupport = isStereoEnabled == TRUE;
    }

    //ST_DEBUG_LOG("DXInfo, AMD(" + int(theInfo.hasAmdAdapter) + "), AQBS(" + int(theInfo.hasAqbsSupport)
    //           + "); NVIDIA(" + int(theInfo.hasNvAdapter) + "), NvStereo(" + int(theInfo.hasNvStereoSupport) + ")");

    // cache the state to avoid unnecessary calls
    ST_DX_CACHED_INFO = theInfo;
    ST_DX_HAS_CACHED_INFO = true;

    return true;
}

#endif // _WIN32
