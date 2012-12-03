/**
 * Copyright © 2007-2012 Kirill Gavrilov <kirill@sview.ru>
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

#if(defined(_WIN32) || defined(__WIN32__))

#include "StWindowImpl.h"

#include <StStrings/StLogger.h>

#include <math.h>

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
        return DefWindowProc(in_hWnd, uMsg, wParam, lParam);
    }
}

// function create GUI window
bool StWindowImpl::stglCreate(const StWinAttributes_t* theAttributes,
                              const StNativeWin_t*     theParentWindow) {
    if(theParentWindow != NULL) {
        stMemCpy(&myParentWin, theParentWindow, sizeof(StNativeWin_t));
    }

    // parse attributes
    size_t aBytesToCopy = (theAttributes->nSize > sizeof(StWinAttributes_t)) ? sizeof(StWinAttributes_t) : theAttributes->nSize;
    stMemCpy(&myWinAttribs, theAttributes, aBytesToCopy); // copy as much as possible
    myWinAttribs.nSize = sizeof(StWinAttributes_t);       // restore own size
    updateSlaveConfig();

    myInitState = STWIN_INITNOTSTART;

    myEventInitWin.reset();
    myEventInitGl.reset();
    myMsgThread = new StThread(threadCreateWindows, (void* )this);
    // wait for thread to create window
    myEventInitWin.wait();
    if(myInitState != STWIN_INIT_SUCCESS) {
        return false;
    }
    int isGlCtx = myMaster.glCreateContext(myWinAttribs.isSlave ? &mySlave : NULL, myWinAttribs.isGlStereo);
    myEventInitGl.set();

    return (isGlCtx == STWIN_INIT_SUCCESS);
}

bool StWindowImpl::wndRegisterClass(const StStringUtfWide& theClassName) {
    HINSTANCE hInstance = GetModuleHandle(NULL); // Holds The Instance Of The Application
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
    if(myWinAttribs.isSlave) {
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
    int posLeft   = myRectNorm.left() - (!myWinAttribs.isNoDecor ? GetSystemMetrics(SM_CXSIZEFRAME) : 0);
    int posTop    = myRectNorm.top() - (!myWinAttribs.isNoDecor ? (GetSystemMetrics(SM_CYSIZEFRAME) + GetSystemMetrics(SM_CYCAPTION)) : 0);
    int winWidth  = (!myWinAttribs.isNoDecor ? 2 * GetSystemMetrics(SM_CXSIZEFRAME) : 0) + myRectNorm.width();
    int winHeight = (!myWinAttribs.isNoDecor ? (GetSystemMetrics(SM_CYCAPTION) + 2 * GetSystemMetrics(SM_CYSIZEFRAME)) : 0) + myRectNorm.height();
    HINSTANCE hInstance = GetModuleHandle(NULL); // Holds The Instance Of The Application

    if(myParentWin == NULL) {
        // WS_EX_ACCEPTFILES - Drag&Drop support
        myMaster.hWindow = CreateWindowExW(WS_EX_APPWINDOW | WS_EX_WINDOWEDGE | WS_EX_ACCEPTFILES,
                                           myMaster.className.toCString(),
                                           myWindowTitle.toUtfWide().toCString(),
                                           (!myWinAttribs.isNoDecor ? WS_OVERLAPPEDWINDOW : WS_POPUP) | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
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
        myWinAttribs.isNoDecor = true;

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

    if(myWinAttribs.isSlave) {
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
    if(myWinAttribs.isSlave && !myWinAttribs.isSlaveHide && (!isSlaveIndependent() || myMonitors.size() > 1)) {
        SetWindowPos(mySlave.hWindowGl,
                     HWND_NOTOPMOST,
                     getSlaveLeft(),  getSlaveTop(),
                     getSlaveWidth(), getSlaveHeight(),
                     SWP_SHOWWINDOW);
    }

    if(!myWinAttribs.isHide) {
        SetWindowPos(myMaster.hWindow, HWND_NOTOPMOST,
                     posLeft, posTop, winWidth, winHeight,
                     SWP_SHOWWINDOW);

        SetWindowPos(myMaster.hWindowGl, HWND_NOTOPMOST,
                     0, 0, myRectNorm.width(), myRectNorm.height(),
                     SWP_SHOWWINDOW);
    }

    myMessageList.append(StMessageList::MSG_RESIZE);

    // always wait for message thread exit before quit
    myMaster.evMsgThread.reset();

    // ========= Start callback procedure =========
    if(!myWinAttribs.isHide && myParentWin == NULL) {
        // TODO (Kirill Gavrilov#4) need we special StWindow function to make window on top?
        SetForegroundWindow(myMaster.hWindow); // make sure Master window on top and has input focus
    }
    HANDLE waitEvents[3] = {myEventQuit, myEventCursorShow, myEventCursorHide};
    for(;;) {
        switch(MsgWaitForMultipleObjects(3, waitEvents, FALSE, INFINITE, QS_ALLINPUT)) {
            case WAIT_OBJECT_0: {
                // Event 1 (myEventQuit) has been set. If the event was created as autoreset, it has also been reset
                ///ST_DEBUG_LOG("WinAPI, End of the message thread... (TID= " + StThread::getCurrentThreadId() + ")");
                mySlave.close();  // close window handles
                myMaster.close();

                mySlave.threadIdWnd  = 0;
                myMaster.threadIdWnd = 0;

                myMaster.evMsgThread.set(); // thread now exit, nothing should be after!
                return true;
            }
            case WAIT_OBJECT_0 + 1: {
                // Event 2 (myEventCursorShow) has been set. If the event was created as autoreset, it has also been reset
                // show / hide cursor - SHOULD be called from window thread...
                ShowCursor(TRUE);
                ///ST_DEBUG_LOG("ShowCursor(TRUE)");
                ResetEvent(myEventCursorShow);
                break;
            }
            case WAIT_OBJECT_0 + 2: {
                // Event 3 (myEventCursorHide) has been set. If the event was created as autoreset, it has also been reset
                // warning - this is NOT force hide / show function;
                // this call decrease or increase show counter
                ShowCursor(FALSE);
                ///ST_DEBUG_LOG("ShowCursor(FALSE)");
                ResetEvent(myEventCursorHide);
                break;
            }
            case WAIT_OBJECT_0 + 3: {
                // A thread's (window's) message(s) has arrived
                // We should process ALL messages cause MsgWaitForMultipleObjects
                // will NOT triggered for new messages already in stack!!!
                // Means - do not replace 'while' with 'if(PeekMessage(...))'.
                while(PeekMessage(&myEvent, NULL, 0U, 0U, PM_REMOVE)) {
                    TranslateMessage(&myEvent); // Translate The Message
                    DispatchMessage(&myEvent);  // Dispatch The Message
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

/**
 * Update StWindow position according to native parent position.
 */
void StWindowImpl::updateChildRect() {
    if(!myWinAttribs.isFullScreen && myParentWin != NULL) {
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

        myMessageList.append(StMessageList::MSG_RESIZE);
    }
}

LRESULT StWindowImpl::stWndProc(HWND theWin, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch(uMsg) {                 // Check For Windows Messages
        case WM_ACTIVATE: {        // Watch For Window Activate Message
            if(!HIWORD(wParam)) {
                //active = true;   // Check Minimization State
            } else {
                //active = false;  // Program Is No Longer Active
                //active = true;
            }
            return 0;              // Return To The Message Loop
        }

        case WM_SYSCOMMAND: {      // Intercept System Commands
            switch(wParam) {       // Check System Calls
            case 0xF032: // TODO (Kirill Gavrilov#7#) parse double click on title in 'right' way
            case SC_MAXIMIZE:
                setFullScreen(true);
                myIsUpdated = true;
                myMessageList.append(StMessageList::MSG_RESIZE);
            case SC_SCREENSAVE:    // Screensaver Trying To Start?
            case SC_MONITORPOWER:  // Monitor Trying To Enter Powersave?
                return 0;          // Prevent From Happening
            }
            break;                 // Exit
        }

        case WM_CLOSE: {           // Did We Receive A Close Message?
            myMessageList.append(StMessageList::MSG_CLOSE);
            return 0;              // Jump Back
        }

        case WM_DROPFILES: {
            HDROP aDrops = (HDROP )wParam;
            UINT aFilesCount = DragQueryFileW(aDrops, 0xFFFFFFFF, NULL, 0);
            stUtfWide_t aFileBuff[MAX_PATH];
            myDndMutex.lock();
            if(aFilesCount < 1) {
                DragFinish(aDrops);
                break;
            }
            myDndCount = aFilesCount;
            delete[] myDndList;
            myDndList = new StString[myDndCount];
            for(UINT aFileId = 0; aFileId < aFilesCount; ++aFileId) {
                if(DragQueryFileW(aDrops, aFileId, aFileBuff, MAX_PATH) > 0) {
                    myDndList[aFileId] = StString(aFileBuff);
                }
            }
            myDndMutex.unlock();
            DragFinish(aDrops); // do not forget
            myMessageList.append(StMessageList::MSG_DRAGNDROP_IN);
            break;
        }
        case WM_MOVE: {
            if(myWinAttribs.isFullScreen || myParentWin != NULL) {
                break;
            } else if(theWin == myMaster.hWindow) {
                myIsUpdated = true;
                // TODO (Kirill GAvrilov#2) not thread safe assignment!
                myRectNorm.left() = (int )(short )LOWORD(lParam);
                myRectNorm.top()  = (int )(short )HIWORD(lParam);
                myMessageList.append(StMessageList::MSG_RESIZE);
                //ST_DEBUG_LOG("WM_MOVE, XxY= " + x + "x" + y);
                break;
            }
            // ignore GL subwindow resize messages!
            break;
        }
        case WM_SIZE: {
            if(myWinAttribs.isFullScreen || myParentWin != NULL) {
                break;
            } else if(theWin == myMaster.hWindow) {
                myIsUpdated = true;
                int w = LOWORD(lParam);
                int h = HIWORD(lParam);
                // TODO (Kirill GAvrilov#2) not thread safe assignment!
                myRectNorm.right()  = myRectNorm.left() + w;
                myRectNorm.bottom() = myRectNorm.top()  + h;
                myMessageList.append(StMessageList::MSG_RESIZE);
                //ST_DEBUG_LOG("WM_SIZE, WxH= " + w + "x" + h);
                break;
            }
            break;
        }
        case WM_MOVING:
        case WM_SIZING: {
            if(myWinAttribs.isFullScreen || myParentWin != NULL) {
                break;
            } else if(theWin == myMaster.hWindow) {
                RECT* aRect = (RECT* )(LPARAM )lParam;
                // TODO (Kirill GAvrilov#2) not thread safe assignment!
                myRectNorm.left()   = aRect->left   + GetSystemMetrics(SM_CXSIZEFRAME);
                myRectNorm.right()  = aRect->right  - GetSystemMetrics(SM_CXSIZEFRAME);
                myRectNorm.top()    = aRect->top    + GetSystemMetrics(SM_CYSIZEFRAME) + GetSystemMetrics(SM_CYCAPTION);
                myRectNorm.bottom() = aRect->bottom - GetSystemMetrics(SM_CYSIZEFRAME);
                myIsUpdated = true;
                myMessageList.append(StMessageList::MSG_RESIZE);
                break;
            }
            break;
        }
        // keys lookup
        case WM_KEYDOWN: {
            myMessageList.getKeysMap()[wParam] = true; break;
        }
        case WM_KEYUP: {         // has a key been released?
            myMessageList.getKeysMap()[wParam] = false; break;
        }
        // mouse lookup
        // TODO (Kirill Gavrilov#6#) parse double click mouse messages
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
            if(uMsg == WM_MOUSEWHEEL) {
                // special case - WinAPI give us position relative to screen!
                mouseXPx -= getPlacement().left();
                mouseYPx -= getPlacement().top();
            }
            StPointD_t point(double(mouseXPx) / double(getPlacement().width()),
                             double(mouseYPx) / double(getPlacement().height()));
            int mbtn = ST_NOMOUSE;
            switch(uMsg) {
                case WM_LBUTTONUP:
                case WM_LBUTTONDOWN: mbtn = ST_MOUSE_LEFT; break;
                case WM_RBUTTONUP:
                case WM_RBUTTONDOWN: mbtn = ST_MOUSE_RIGHT; break;
                case WM_MBUTTONUP:
                case WM_MBUTTONDOWN: mbtn = ST_MOUSE_MIDDLE; break;
                case WM_XBUTTONUP:
                case WM_XBUTTONDOWN: mbtn = (HIWORD(wParam) == XBUTTON1) ? ST_MOUSE_X1 : ST_MOUSE_X2; break;
                case WM_MOUSEWHEEL: {
                    int zDelta = GET_WHEEL_DELTA_WPARAM(wParam);
                    mbtn = (zDelta > 0) ? ST_MOUSE_SCROLL_V_UP : ST_MOUSE_SCROLL_V_DOWN;
                    break;
                }
            }
            switch(uMsg) {
                case WM_LBUTTONUP:
                case WM_RBUTTONUP:
                case WM_MBUTTONUP:
                case WM_XBUTTONUP: {
                    // TODO (Kirill Gavrilov#9) what if we have some another not unclicked mouse button?
                    ReleaseCapture();

                    myMUpQueue.push(point, mbtn);
                    myMessageList.append(StMessageList::MSG_MOUSE_UP);
                    break;
                }
                case WM_LBUTTONDOWN:
                case WM_RBUTTONDOWN:
                case WM_MBUTTONDOWN:
                case WM_XBUTTONDOWN: {
                    // to receive out-of-window unclick message
                    SetFocus  (theWin);
                    SetCapture(theWin);

                    myMDownQueue.push(point, mbtn);
                    myMessageList.append(StMessageList::MSG_MOUSE_DOWN);
                    break;
                }
                case WM_MOUSEWHEEL: {
                    // TODO (Kirill Gavrilov#9#) delta ignored
                    myMDownQueue.push(point, mbtn);
                    myMessageList.append(StMessageList::MSG_MOUSE_DOWN);
                    myMUpQueue.push(point, mbtn);
                    myMessageList.append(StMessageList::MSG_MOUSE_UP);
                    break;
                }
            }
            break;
        }
    }
    // Pass All Unhandled Messages To DefWindowProc
    return DefWindowProc(theWin, uMsg, wParam, lParam);
}

/**
 * In this function we fullscreen only Master window,
 * Slave window resized in update procedure.
 */
void StWindowImpl::setFullScreen(bool theFullscreen) {
    if(myWinAttribs.isFullScreen != theFullscreen) {
        myWinAttribs.isFullScreen = theFullscreen;
        if(myWinAttribs.isFullScreen) {
            myFullScreenWinNb.increment();
        } else {
            myFullScreenWinNb.decrement();
        }
    }

    ShowWindow(myMaster.hWindow, SW_HIDE);
    if(myWinAttribs.isFullScreen) {
        // embedded
        if(myParentWin != NULL) {
            // TODO (Kirill Gavrilov#6) check safity
            SetParent(myMaster.hWindowGl, NULL);
            SetFocus (myMaster.hWindowGl);
        }

        // generic
        const StMonitor& stMon = (myMonMasterFull == -1) ? myMonitors[myRectNorm.center()] : myMonitors[myMonMasterFull];
        myRectFull = stMon.getVRect();
        SetWindowLongPtr(myMaster.hWindow, GWL_STYLE, WS_POPUP);
        SetWindowPos(myMaster.hWindow, // window handle
                     HWND_TOP,         // top status
                     myRectFull.left(),  myRectFull.top(),
                     myRectFull.width(), myRectFull.height(),
                     !myWinAttribs.isHide ? SWP_SHOWWINDOW : SWP_NOACTIVATE); // show window
        if(!myWinAttribs.isHide) {
            SetFocus(myMaster.hWindow);
        }
    } else {
        // embedded
        if(myParentWin != NULL) {
            SetParent(myMaster.hWindowGl, (HWND )myParentWin);
        }

        // generic
        if(!myWinAttribs.isNoDecor) {
            SetWindowLongPtr(myMaster.hWindow, GWL_STYLE, WS_OVERLAPPEDWINDOW);
        }
        SetWindowPos(myMaster.hWindow, // window handle
                     HWND_NOTOPMOST,   // top status
                     myRectNorm.left() - GetSystemMetrics(SM_CXSIZEFRAME), // left position
                     myRectNorm.top()  - GetSystemMetrics(SM_CYSIZEFRAME) - GetSystemMetrics(SM_CYCAPTION), // top position
                     2 * GetSystemMetrics(SM_CXSIZEFRAME) + myRectNorm.width(), // width
                     GetSystemMetrics(SM_CYCAPTION) + 2* GetSystemMetrics(SM_CYSIZEFRAME) + myRectNorm.height(), // height
                     !myWinAttribs.isHide ? SWP_SHOWWINDOW : SWP_NOACTIVATE); // show window
        if(!myWinAttribs.isHide) {
            SetFocus(myMaster.hWindow);
        }
    }
    myIsUpdated = true;
    myMessageList.append(StMessageList::MSG_RESIZE);
    myMessageList.append(StMessageList::MSG_FULLSCREEN_SWITCH);
}

void StWindowImpl::updateWindowPos() {
    if(myWinAttribs.isSlave && !myWinAttribs.isSlaveHide && (!isSlaveIndependent() || myMonitors.size() > 1)) {
        HWND afterHWND = myMaster.hWindow;
        if(myWinAttribs.isSlaveHLineBottom || myWinAttribs.isSlaveHTop2Px || myWinAttribs.isSlaveHLineTop) {
            afterHWND = HWND_TOPMOST;
        }

        // resize Slave GL-window
        SetWindowPos(mySlave.hWindowGl, afterHWND,
                     getSlaveLeft(),  getSlaveTop(),
                     getSlaveWidth(), getSlaveHeight(),
                     SWP_NOACTIVATE);
    }

    if(!myWinAttribs.isHide) {
        // resize Master GL-subwindow
        GLsizei sizeX = (myWinAttribs.isFullScreen) ? myRectFull.width()  : myRectNorm.width();
        GLsizei sizeY = (myWinAttribs.isFullScreen) ? myRectFull.height() : myRectNorm.height();
        SetWindowPos(myMaster.hWindowGl,
                     (myParentWin != NULL && myWinAttribs.isFullScreen) ? HWND_TOPMOST : HWND_TOP,
                     0, 0, sizeX, sizeY,
                     SWP_NOACTIVATE);
    }

    // detect when window moved to another monitor
    if(!myWinAttribs.isFullScreen && myMonitors.size() > 1) {
        int aNewMonId = myMonitors[myRectNorm.center()].getId();
        if(myWinOnMonitorId != aNewMonId) {
            myMessageList.append(StMessageList::MSG_WIN_ON_NEW_MONITOR);
            myWinOnMonitorId = aNewMonId;
        }
    }
}

// Function set to argument-buffer given events and return events number
void StWindowImpl::callback(StMessage_t* theMessages) {
    // detect embedded window was moved
    if(myParentWin != NULL && myMaster.hWindowGl != NULL && !myWinAttribs.isFullScreen) {
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
            DispatchMessage (&myEvent);
        }
    }

    StPointD_t aNewMousePt = getMousePos();
    if(aNewMousePt.x() >= 0.0 && aNewMousePt.x() <= 1.0 && aNewMousePt.y() >= 0.0 && aNewMousePt.y() <= 1.0) {
        StPointD_t aDspl = aNewMousePt - myMousePt;
        if(std::abs(aDspl.x()) >= 0.0008 || std::abs(aDspl.y()) >= 0.0008) {
            myMessageList.append(StMessageList::MSG_MOUSE_MOVE);
        }
    }
    myMousePt = aNewMousePt;

    // TODO (Kirill Gavrilov#5#) parse multimedia keys
    //if(GetAsyncKeyState(VK_MEDIA_NEXT_TRACK ) != 0) {
    //
    //}
    myMessageList.popList(theMessages);
}

#endif
