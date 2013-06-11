/**
 * Copyright © 2007-2013 Kirill Gavrilov <kirill@sview.ru>
 *
 * StCore library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * StCore library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.
 * If not, see <http://www.gnu.org/licenses/>.
 */

#ifdef _WIN32

#include "StWindowImpl.h"

#include <StStrings/StLogger.h>
#include <StThreads/StThread.h>

#include <cmath>

static SV_THREAD_FUNCTION threadCreateWindows(void* inStWin);

/**
 * Static proc function just do recall to StWindowImpl class method.
 */
static LRESULT CALLBACK stWndProcWrapper(HWND in_hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    if(uMsg == WM_CREATE) {
        // save pointer to our class instance (sended on window create) to window storage
        LPCREATESTRUCT pCS = (LPCREATESTRUCT )lParam;
        SetWindowLongPtr(in_hWnd, int(GWLP_USERDATA), (LONG_PTR )pCS->lpCreateParams);
    }
    // get pointer to our class instance
    StWindowImpl* pThis = (StWindowImpl* )GetWindowLongPtr(in_hWnd, int(GWLP_USERDATA));
    if(pThis != NULL) {
        return pThis->stWndProc(in_hWnd, uMsg, wParam, lParam);
    } else {
        return DefWindowProcW(in_hWnd, uMsg, wParam, lParam);
    }
}

bool StWindowImpl::create() {
    myKeysState.reset();
    myInitState = STWIN_INITNOTSTART;

    myEventInitWin.reset();
    myEventInitGl.reset();
    ResetEvent(myEventQuit);
    myMsgThread = new StThread(threadCreateWindows, (void* )this);
    // wait for thread to create window
    myEventInitWin.wait();
    if(myInitState != STWIN_INIT_SUCCESS) {
        return false;
    }

    int isGlCtx = myMaster.glCreateContext(attribs.Slave != StWinSlave_slaveOff ? &mySlave : NULL,
                                           attribs.GlDepthSize,
                                           attribs.IsGlStereo);
    myEventInitGl.set();

    return (isGlCtx == STWIN_INIT_SUCCESS);
}

bool StWindowImpl::wndRegisterClass(const StStringUtfWide& theClassName) {
    HINSTANCE hInstance = GetModuleHandleW(NULL);
    WNDCLASSW wndClass;                                           // Windows Class Structure
    wndClass.style         = CS_HREDRAW | CS_VREDRAW | CS_OWNDC; // Redraw On Size, And Own DC For Window.
    wndClass.lpfnWndProc   = (WNDPROC )stWndProcWrapper;         // WndProc Handles Messages
    wndClass.cbClsExtra    = 0;                                  // No Extra Window Data
    wndClass.cbWndExtra    = 0;                                  // No Extra Window Data
    wndClass.hInstance     = hInstance;                          // Set The Instance
    // TODO (Kirill Gavrilov) change icon setup
    wndClass.hIcon         = LoadIcon(hInstance, L"A");          // Load The Icon A
    wndClass.hCursor       = LoadCursor(NULL, IDC_ARROW);        // Load The Arrow Pointer
    wndClass.hbrBackground = NULL;                               // No Background Required For GL
    wndClass.lpszMenuName  = NULL;                               // No menu
    wndClass.lpszClassName = theClassName.toCString();           // Set The Class Name
    return (RegisterClassW(&wndClass) != 0);
}

// Function create windows' handles in another thread (to take events impact-less to GL-rendering)
static SV_THREAD_FUNCTION threadCreateWindows(void* inStWin) {
    StWindowImpl* stWin = (StWindowImpl* )inStWin;
    stWin->wndCreateWindows();
    return SV_THREAD_RETURN 0;
}

bool StWindowImpl::wndCreateWindows() {
    StStringUtfWide aNewClassName = StWinHandles::getNewClassName();

    // need we register class for Master window?
    if(myParentWin == NULL) {
        if(!wndRegisterClass(aNewClassName)) {
            stError("WinAPI: Failed to register Master window class");
            myInitState = STWIN_ERROR_WIN32_REGCLASS;
            return false;
        } else {
            myMaster.className = aNewClassName;
        }
    }

    aNewClassName += "Gl";
    if(!wndRegisterClass(aNewClassName)) {
        stError("WinAPI: Failed to register Master window class");
        myInitState = STWIN_ERROR_WIN32_REGCLASS;
        return false;
    } else {
        myMaster.classNameGl = aNewClassName;
    }
    if(attribs.Slave != StWinSlave_slaveOff) {
        aNewClassName = StWinHandles::getNewClassName() + "Gl";
        if(!wndRegisterClass(aNewClassName)) {
            stError("WinAPI: Failed to register Slave window class");
            myInitState = STWIN_ERROR_WIN32_REGCLASS;
            return false;
        } else {
            mySlave.classNameGl = aNewClassName;
        }
    }
    myMaster.threadIdWnd = mySlave.threadIdWnd = StThread::getCurrentThreadId();

    // ========= At first - create windows' instances =========
    // parent Master window could be decorated - we parse this situation to get true coordinates
    int posLeft   = myRectNorm.left() - (!attribs.IsNoDecor ? GetSystemMetrics(SM_CXSIZEFRAME) : 0);
    int posTop    = myRectNorm.top() - (!attribs.IsNoDecor ? (GetSystemMetrics(SM_CYSIZEFRAME) + GetSystemMetrics(SM_CYCAPTION)) : 0);
    int winWidth  = (!attribs.IsNoDecor ? 2 * GetSystemMetrics(SM_CXSIZEFRAME) : 0) + myRectNorm.width();
    int winHeight = (!attribs.IsNoDecor ? (GetSystemMetrics(SM_CYCAPTION) + 2 * GetSystemMetrics(SM_CYSIZEFRAME)) : 0) + myRectNorm.height();
    HINSTANCE hInstance = GetModuleHandleW(NULL);

    if(myParentWin == NULL) {
        // WS_EX_ACCEPTFILES - Drag&Drop support
        myMaster.hWindow = CreateWindowExW(WS_EX_APPWINDOW | WS_EX_WINDOWEDGE | WS_EX_ACCEPTFILES,
                                           myMaster.className.toCString(),
                                           myWindowTitle.toUtfWide().toCString(),
                                           (!attribs.IsNoDecor ? WS_OVERLAPPEDWINDOW : WS_POPUP) | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
                                           posLeft, posTop,
                                           winWidth, winHeight,
                                           NULL, NULL, hInstance, this); // put pointer to class, getted on WM_CREATE
        if(myMaster.hWindow == NULL) {
            myMaster.close();
            stError("WinAPI: Master Window Creation Error");
            myInitState = STWIN_ERROR_WIN32_CREATE;
            return false;
        }
    } else {
        // we have external native parent window
        attribs.IsNoDecor = true;

        RECT aRect;
        GetWindowRect(myParentWin, &aRect);

        myRectNorm.left()   = aRect.left;
        myRectNorm.right()  = aRect.right;
        myRectNorm.top()    = aRect.top;
        myRectNorm.bottom() = aRect.bottom;

        posLeft   = myRectNorm.left();
        posTop    = myRectNorm.top();
        winWidth  = myRectNorm.width();
        winHeight = myRectNorm.height();
        myIsUpdated = true;
    }

    // we use WS_EX_NOPARENTNOTIFY style to prevent to send notify on destroing our child window (NPAPI plugin -> deadlock)
    DWORD masterWindowGl_dwExStyle = (myParentWin == NULL) ? WS_EX_NOACTIVATE : (WS_EX_NOACTIVATE | WS_EX_NOPARENTNOTIFY);
    myMaster.hWindowGl = CreateWindowExW(masterWindowGl_dwExStyle,
                                         myMaster.classNameGl.toCString(),
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
                                            mySlave.classNameGl.toCString(),
                                            L"Slave window",
                                            WS_POPUP | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_DISABLED, // slave always disabled (hasn't input focus)!
                                            posLeft, posTop, // initialize slave window at same screen as master to workaround bugs in drivers that may prevent GL context sharing
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

    // ========= Synchronization barrier - wait until MAIN thread create GL contexts =========
    myInitState = STWIN_INIT_SUCCESS; // 'send' it's OK for main thread
    myEventInitWin.set();
    myEventInitGl.wait();

    // ========= Now show up the windows =========
    if(attribs.Slave != StWinSlave_slaveOff && !attribs.IsSlaveHidden && (!isSlaveIndependent() || myMonitors.size() > 1)) {
        SetWindowPos(mySlave.hWindowGl,
                     HWND_NOTOPMOST,
                     getSlaveLeft(),  getSlaveTop(),
                     getSlaveWidth(), getSlaveHeight(),
                     SWP_SHOWWINDOW);
    }

    if(!attribs.IsHidden) {
        SetWindowPos(myMaster.hWindow, HWND_NOTOPMOST,
                     posLeft, posTop, winWidth, winHeight,
                     SWP_SHOWWINDOW);

        SetWindowPos(myMaster.hWindowGl, HWND_NOTOPMOST,
                     0, 0, myRectNorm.width(), myRectNorm.height(),
                     SWP_SHOWWINDOW);
    }

    // always wait for message thread exit before quit
    myMaster.evMsgThread.reset();

    // ========= Start callback procedure =========
    if(!attribs.IsHidden && myParentWin == NULL) {
        // TODO (Kirill Gavrilov#4) need we special StWindow function to make window on top?
        SetForegroundWindow(myMaster.hWindow); // make sure Master window on top and has input focus
    }

    // flag to track registered global hot-keys
    bool areGlobalHotKeys = false;

    wchar_t aCharBuff[4];
    BYTE    aKeysMap[256];
    HANDLE  waitEvents[3] = {myEventQuit, myEventCursorShow, myEventCursorHide};
    for(;;) {
        switch(MsgWaitForMultipleObjects(3, waitEvents, FALSE, INFINITE, QS_ALLINPUT)) {
            case WAIT_OBJECT_0: {
                // Event 1 (myEventQuit) has been set. If the event was created as autoreset, it has also been reset
                ///ST_DEBUG_LOG("WinAPI, End of the message thread... (TID= " + StThread::getCurrentThreadId() + ")");
                if(areGlobalHotKeys) {
                    UnregisterHotKey(myMaster.hWindowGl, myMaster.myMKeyStop);
                    UnregisterHotKey(myMaster.hWindowGl, myMaster.myMKeyPlay);
                    UnregisterHotKey(myMaster.hWindowGl, myMaster.myMKeyPrev);
                    UnregisterHotKey(myMaster.hWindowGl, myMaster.myMKeyNext);
                }

                mySlave.close();  // close window handles
                myMaster.close();

                mySlave.threadIdWnd  = 0;
                myMaster.threadIdWnd = 0;

                ResetEvent(myEventQuit);
                myMaster.evMsgThread.set(); // thread now exit, nothing should be after!
                return true;
            }
            case WAIT_OBJECT_0 + 1: {
                // Event 2 (myEventCursorShow) has been set. If the event was created as autoreset, it has also been reset
                // show / hide cursor - SHOULD be called from window thread...
                ShowCursor(TRUE);
                ResetEvent(myEventCursorShow);
                break;
            }
            case WAIT_OBJECT_0 + 2: {
                // Event 3 (myEventCursorHide) has been set. If the event was created as autoreset, it has also been reset
                // warning - this is NOT force hide / show function;
                // this call decrease or increase show counter
                ShowCursor(FALSE);
                ResetEvent(myEventCursorHide);
                break;
            }
            case WAIT_OBJECT_0 + 3: {
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

        myStEventAux.Type       = stEvent_Size;
        myStEventAux.Size.Time  = getEventTime();
        myStEventAux.Size.SizeX = myRectNorm.width();
        myStEventAux.Size.SizeY = myRectNorm.height();
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

        case WM_CLOSE: {
            myStEvent.Type       = stEvent_Close;
            myStEvent.Close.Time = getEventTime(myEvent.time);
            myEventsBuffer.append(myStEvent);
            return 0; // do nothing - window close action should be performed by application
        }

        case WM_DISPLAYCHANGE: {
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

            for(UINT aFileId = 0; aFileId < aFilesCount; ++aFileId) {
                if(DragQueryFileW(aDrops, aFileId, aFileBuff, MAX_PATH) > 0) {
                    const StString aFile(aFileBuff);
                    myStEvent.Type = stEvent_FileDrop;
                    myStEvent.DNDrop.Time = getEventTime(myEvent.time);
                    myStEvent.DNDrop.File = aFile.toCString();
                    myEventsBuffer.append(myStEvent);
                }
            }
            DragFinish(aDrops); // do not forget
            break;
        }
        case WM_MOVE: {
            if(attribs.IsFullScreen || myParentWin != NULL) {
                break;
            } else if(theWin == myMaster.hWindow) {
                myIsUpdated = true;
                myRectNorm.left() = (int )(short )LOWORD(lParam);
                myRectNorm.top()  = (int )(short )HIWORD(lParam);
                break;
            }
            // ignore GL subwindow resize messages!
            break;
        }
        case WM_SIZE: {
            if(attribs.IsFullScreen || myParentWin != NULL) {
                break;
            } else if(theWin == myMaster.hWindow) {
                myIsUpdated = true;
                int w = LOWORD(lParam);
                int h = HIWORD(lParam);
                myRectNorm.right()  = myRectNorm.left() + w;
                myRectNorm.bottom() = myRectNorm.top()  + h;

                myStEvent.Type       = stEvent_Size;
                myStEvent.Size.Time  = getEventTime(myEvent.time);
                myStEvent.Size.SizeX = myRectNorm.width();
                myStEvent.Size.SizeY = myRectNorm.height();
                myEventsBuffer.append(myStEvent);
                break;
            }
            break;
        }
        case WM_MOVING:
        case WM_SIZING: {
            if(attribs.IsFullScreen || myParentWin != NULL) {
                break;
            } else if(theWin == myMaster.hWindow) {
                RECT* aRect = (RECT* )(LPARAM )lParam;
                myRectNorm.left()   = aRect->left   + GetSystemMetrics(SM_CXSIZEFRAME);
                myRectNorm.right()  = aRect->right  - GetSystemMetrics(SM_CXSIZEFRAME);
                myRectNorm.top()    = aRect->top    + GetSystemMetrics(SM_CYSIZEFRAME) + GetSystemMetrics(SM_CYCAPTION);
                myRectNorm.bottom() = aRect->bottom - GetSystemMetrics(SM_CYSIZEFRAME);
                myIsUpdated = true;

                myStEvent.Type       = stEvent_Size;
                myStEvent.Size.Time  = getEventTime(myEvent.time);
                myStEvent.Size.SizeX = myRectNorm.width();
                myStEvent.Size.SizeY = myRectNorm.height();
                myEventsBuffer.append(myStEvent);
                break;
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
        case WM_MOUSEWHEEL:    // vertical wheel
        //case WM_MOUSEHWHEEL:   // horizontal wheel (only Vista+)
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
            if(uMsg == WM_MOUSEWHEEL) {
                // special case - WinAPI give us position relative to screen!
                mouseXPx -= aWinRect.left();
                mouseYPx -= aWinRect.top();
            } else {
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
                case WM_MOUSEWHEEL: {
                    int zDelta = GET_WHEEL_DELTA_WPARAM(wParam);
                    //if(GET_X_LPARAM(lParam) != 0)
                    aBtnId = (zDelta > 0) ? ST_MOUSE_SCROLL_V_UP : ST_MOUSE_SCROLL_V_DOWN;
                    break;
                }
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
                case WM_MOUSEWHEEL: {
                    // TODO (Kirill Gavrilov#9#) delta ignored
                    //GET_WHEEL_DELTA_WPARAM(wParam) / WHEEL_DELTA;
                    myStEvent.Type = stEvent_MouseDown;
                    myEventsBuffer.append(myStEvent);
                    myStEvent.Type = stEvent_MouseUp;
                    myEventsBuffer.append(myStEvent);
                    break;
                }
            }
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

    ShowWindow(myMaster.hWindow, SW_HIDE);
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

        SetWindowPos(myMaster.hWindow, HWND_NOTOPMOST,
                     myRectNorm.left() - GetSystemMetrics(SM_CXSIZEFRAME),
                     myRectNorm.top()  - GetSystemMetrics(SM_CYSIZEFRAME) - GetSystemMetrics(SM_CYCAPTION),
                     2 * GetSystemMetrics(SM_CXSIZEFRAME) + myRectNorm.width(),
                     GetSystemMetrics(SM_CYCAPTION) + 2* GetSystemMetrics(SM_CYSIZEFRAME) + myRectNorm.height(),
                     !attribs.IsHidden ? SWP_SHOWWINDOW : SWP_NOACTIVATE);
        if(!attribs.IsHidden
        && myParentWin == NULL) {
            SetFocus(myMaster.hWindow);
        }
    }

    const StRectI_t& aRect = attribs.IsFullScreen ? myRectFull : myRectNorm;
    StEvent anEvent;
    anEvent.Type       = stEvent_Size;
    anEvent.Size.Time  = getEventTime();
    anEvent.Size.SizeX = aRect.width();
    anEvent.Size.SizeY = aRect.height();
    if(StThread::getCurrentThreadId() == myMaster.threadIdOgl) {
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
        HWND afterHWND = myMaster.hWindow;
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
            const GLsizei aSizeX = (attribs.IsFullScreen) ? myRectFull.width()  : myRectNorm.width();
            const GLsizei aSizeY = (attribs.IsFullScreen) ? myRectFull.height() : myRectNorm.height();
            SetWindowPos(myMaster.hWindowGl, HWND_TOP,
                         0, 0, aSizeX, aSizeY,
                         SWP_NOACTIVATE);
        }
    }

    // detect when window moved to another monitor
    if(!attribs.IsFullScreen && myMonitors.size() > 1) {
        int aNewMonId = myMonitors[myRectNorm.center()].getId();
        if(myWinOnMonitorId != aNewMonId) {
            myStEventAux.Type  = stEvent_NewMonitor;
            myStEventAux.Size.Time  = getEventTime();
            myStEventAux.Size.SizeX = myRectNorm.width();
            myStEventAux.Size.SizeY = myRectNorm.height();
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
        SetWindowTextW(myMaster.hWindow, myWindowTitle.toUtfWide().toCString());
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

#endif
