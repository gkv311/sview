/**
 * This is source code for sView
 *
 * Copyright © Kirill Gavrilov, 2011
 */

#include "StCloneDesktop.h"

#include <StSettings/StSettings.h>

#if(defined(_WIN32) || defined(__WIN32__))
    #include <windowsX.h>
#endif

namespace {
#if(defined(_WIN32) || defined(__WIN32__))
    static const StString THE_WINDOW_CLASS(ST_TEXT("StWindowFlip"));

    /**
     * Static proc function just do recall to class method.
     */
    static LRESULT CALLBACK stWndProcWrapper(HWND in_hWnd, UINT uMsg,
                                             WPARAM wParam, LPARAM lParam) {
        if(uMsg == WM_CREATE) {
            // save pointer to our class instance (sended on window create) to window storage
            LPCREATESTRUCT pCS = (LPCREATESTRUCT )lParam;
            SetWindowLongPtr(in_hWnd, int(GWLP_USERDATA), (LONG_PTR )pCS->lpCreateParams);
        }
        // get pointer to our class instance
        StCloneDesktop* pThis = (StCloneDesktop* )GetWindowLongPtr(in_hWnd, int(GWLP_USERDATA));
        if(pThis != NULL) {
            return pThis->stWndProc(in_hWnd, uMsg, wParam, lParam);
        } else {
            return DefWindowProc(in_hWnd, uMsg, wParam, lParam);
        }
    }
#endif

    // look up to the StOutDual plugin settings
    static const StString ST_OUT_PLUGIN_NAME   = ST_STRING("StOutDual");
    static const StString ST_SETTING_DEVICE_ID = ST_STRING("deviceId");
    static const StString ST_SETTING_WINDOWPOS = ST_STRING("windowPos");
    static const StString ST_SETTING_SLAVE_ID  = ST_STRING("slaveId");

    enum {
        DUALMODE_SIMPLE   = 0, // no mirroring
        DUALMODE_XMIRROW  = 1, // mirror on X SLAVE  window
        DUALMODE_YMIRROW  = 2, // mirror on Y SLAVE  window
    };
};

StCloneDesktop::StCloneDesktop()
: myWindowH(NULL),
  myFPSControl(),
  myMonMaster(),
  myMonSlave(),
  myIsFlipX(false),
  myIsFlipY(false) {
    //
    StSearchMonitors aMonitors;
    aMonitors.init();
    if(aMonitors.size() == 1) {
        // dummy for debug
        aMonitors.add(aMonitors[0]);
        StRectI_t& aRect0 = aMonitors[0].changeVRect();
        int aWidth = aRect0.width();
        aRect0.right() = aRect0.left() + aWidth / 2;
        StRectI_t& aRect1 = aMonitors[1].changeVRect();
        aRect1.left()  = aRect0.right();
        aRect1.right() = aRect1.left() + aWidth / 2;
    }
    for(size_t m = 0; m < aMonitors.size(); ++m) {
        ST_DEBUG_LOG(aMonitors[m].toString());
    }

    // load settings from StOutDual plugin
    if(StSettings::INIT() == STERROR_LIBNOERROR) {
        StSettings* aSettings = new StSettings(ST_OUT_PLUGIN_NAME);

        // get master monitor position using saved window position
        StRect<int32_t> aLoadedRect(256, 768, 256, 1024);
        aSettings->loadInt32Rect(ST_SETTING_WINDOWPOS, aLoadedRect);
        myMonMaster = aMonitors[aLoadedRect.center()];

        // load slave monitor position
        int32_t aMonSlaveId = 1;
        aSettings->loadInt32(ST_SETTING_SLAVE_ID, aMonSlaveId);
        myMonSlave = aMonitors[aMonSlaveId];

        // load flip option
        int32_t aDeviceId = DUALMODE_SIMPLE;
        aSettings->loadInt32(ST_SETTING_DEVICE_ID, aDeviceId);
        myIsFlipX = aDeviceId == DUALMODE_XMIRROW;
        myIsFlipY = aDeviceId == DUALMODE_YMIRROW;

        // release resources
        delete aSettings;
        StSettings::FREE();
    } else {
        myMonMaster = aMonitors[0];
        myMonSlave  = aMonitors[1];
    }
}

StCloneDesktop::~StCloneDesktop() {
    //
}

bool StCloneDesktop::create() {
#if(defined(_WIN32) || defined(__WIN32__))
    HINSTANCE hInstance = GetModuleHandle(NULL);

    // register the class
    WNDCLASS aWinClass;
    RtlZeroMemory(&aWinClass, sizeof(aWinClass));
    aWinClass.style         = CS_HREDRAW | CS_VREDRAW;
    aWinClass.cbClsExtra    = 0;
    aWinClass.cbWndExtra    = 0;
    aWinClass.lpfnWndProc   = stWndProcWrapper;
    aWinClass.hInstance     = hInstance;
    aWinClass.lpszClassName = THE_WINDOW_CLASS.utfText();
    aWinClass.hIcon         = LoadIcon(hInstance, L"A");
    aWinClass.hCursor       = LoadCursor(NULL, IDC_ARROW);

    // register window class
    if(!RegisterClass(&aWinClass)) {
        stError(ST_TEXT("Cannot register window class"));
        return false;
    }

    // create the window
    const StRectI_t& aRect = myMonSlave.getVRect();
    myWindowH = CreateWindowExW(WS_EX_TOOLWINDOW | WS_EX_WINDOWEDGE | WS_EX_NOACTIVATE,
                                THE_WINDOW_CLASS.utfText(),
                                L"sView - Flip Desktop",
                                WS_POPUP | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_DISABLED,
                                aRect.left(),  aRect.top(),
                                aRect.width(), aRect.height(),
                                NULL, NULL,
                                hInstance, this);
    if(myWindowH == NULL) {
        stError(ST_TEXT("Cannot create window"));
        return false;
    }
    // show window
    SetWindowPos(myWindowH,
                 HWND_BOTTOM,
                 aRect.left(),  aRect.top(),
                 aRect.width(), aRect.height(),
                 SWP_SHOWWINDOW);
    return true;
#else
    return false;
#endif
}

void StCloneDesktop::mainLoop() {
#if(defined(_WIN32) || defined(__WIN32__))
    MSG aMessage;
    StTimer myTimer(true);
    for(;;) {
        if(PeekMessage(&aMessage, 0, 0, 0, PM_REMOVE)) {
            if(aMessage.message == WM_QUIT) {
                break;
            }
            TranslateMessage(&aMessage);
            DispatchMessage(&aMessage);
        } else {
            // tell we need update snapshot
            InvalidateRect(myWindowH, 0, FALSE);
            StThread::sleep(10);
            if(myTimer.getElapsedTime() > 5.0) {
                ST_DEBUG_LOG("FPS= " + myFPSControl.getAverage());
                myTimer.restart();
            }
        }
    }
#endif
}

#if(defined(_WIN32) || defined(__WIN32__))
LRESULT StCloneDesktop::stWndProc(HWND theWinH, UINT theMessage,
                                  WPARAM wParam, LPARAM lParam) {
    switch(theMessage) {
        case WM_ERASEBKGND: {
            // tell that we erase background...
            return 1;
        }
        case WM_PAINT: {
            const StRectI_t& aRect = myMonMaster.getVRect();

            // if we need to flip image we setup negative wisth/height here
            int aTrgLeft   = myIsFlipX ?  aRect.width()  : 0;
            int aTrgWidth  = myIsFlipX ? -aRect.width()  : aRect.width();
            int aTrgTop    = myIsFlipY ?  aRect.height() : 0;
            int aTrgHeight = myIsFlipY ? -aRect.height() : aRect.height();

            PAINTSTRUCT aPaintStruct;
            BeginPaint(theWinH, &aPaintStruct);
            HDC aScreenDC = GetDC(0);

            if(!myIsFlipX && !myIsFlipY) {
                // BitBlt may be faster than StretchBlt... or not?
                BitBlt(aPaintStruct.hdc,
                       aTrgLeft,  aTrgTop,
                       aTrgWidth, aTrgHeight,
                       aScreenDC,
                       aRect.left(),  aRect.top(),
                       SRCCOPY/** | CAPTUREBLT*/);
            } else {
                StretchBlt(aPaintStruct.hdc,
                           aTrgLeft,  aTrgTop,
                           aTrgWidth, aTrgHeight,
                           aScreenDC,
                           aRect.left(),  aRect.top(),
                           aRect.width(), aRect.height(),
                           SRCCOPY/** | CAPTUREBLT*/);
            }
            ReleaseDC(0, aScreenDC);
            EndPaint(theWinH, &aPaintStruct);
            myFPSControl.nextFrame();
            return 0;
        }
        case WM_EXITSIZEMOVE: {
            // detect message to move our window to bottom
            const StRectI_t& aRect = myMonSlave.getVRect();
            SetWindowPos(theWinH,
                         HWND_BOTTOM,
                         aRect.left(),  aRect.top(),
                         aRect.width(), aRect.height(),
                         SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOOWNERZORDER | SWP_NOACTIVATE);
            return 0;
        }
        case WM_ACTIVATE: {
            // detect activate message to move our window to bottom
            if(wParam & 0x0000FFFF) {
                const StRectI_t& aRect = myMonSlave.getVRect();
                SetWindowPos(theWinH,
                             HWND_BOTTOM,
                             aRect.left(),  aRect.top(),
                             aRect.width(), aRect.height(),
                             SWP_NOACTIVATE | SWP_NOOWNERZORDER | SWP_NOACTIVATE);
                return 0;
            }
        }
        case WM_KEYDOWN: {
            // dummy (doesn't work for inactive window)
            if(wParam == ST_VK_ESCAPE) {
                DestroyWindow(theWinH);
                return 0;
            }
            break;
        }
        case WM_CLOSE: {
            DestroyWindow(theWinH);
            return 0;
        }
        case WM_DESTROY: {
            PostQuitMessage(0);
            break;
        }
    }
    return DefWindowProc(theWinH, theMessage, wParam, lParam);
}
#endif
