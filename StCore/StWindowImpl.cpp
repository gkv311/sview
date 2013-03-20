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

#include "StWindowImpl.h"

#include <StStrings/StLogger.h>
#include <StSys/StSys.h>

namespace {
    static const stUtf8_t WINDOW_TITLE_DEFAULT[] = "StWindow";
#ifdef __APPLE__
    static void stDisplayChangeCallBack(CGDirectDisplayID           theDisplay,
                                        CGDisplayChangeSummaryFlags theFlags,
                                        void*                       theStWin) {
        if(theFlags & kCGDisplayBeginConfigurationFlag) {
            return; // ingore 1st calls per changed display
        }

        // notice that this callback called twice per each reconfigured display!
        StWindowImpl* aWin = (StWindowImpl* )theStWin;
        aWin->myIsDispChanged = true;
    }
#endif
};

// shared counter for fullscreen windows to detect inactive state
StAtomic<int32_t> StWindowImpl::myFullScreenWinNb(0);

StWindowImpl::StWindowImpl()
: myParentWin(StNativeWin_t(NULL)),
  myWindowTitle(WINDOW_TITLE_DEFAULT),
  myInitState(STWIN_INITNOTSTART),
  myMousePt(0.5, 0.5),
  myRectNorm(128, 512, 128, 512),
  myRectFull(128, 512, 128, 512),
  myRectNormPrev(0, 1, 0, 1),
  myMonMasterFull(-1),
  mySyncCounter(0),
  myWinOnMonitorId(0),
  myTiledCfg(TiledCfg_Separate),
  myUserDataMap(0),
  myTargetFps(0.0),
#if (defined(_WIN32) || defined(__WIN32__))
  myEventInitWin(false),
  myEventInitGl(false),
  myEventQuit(NULL),
  myEventCursorShow(NULL),
  myEventCursorHide(NULL),
  myIsVistaPlus(StSys::isVistaPlus()),
#elif (defined(__APPLE__))
  mySleepAssert(0),
#endif
  myDndCount(0),
  myDndList(NULL),
  myIsUpdated(false),
  myIsActive(false),
  myBlockSleep(BlockSleep_OFF),
  myIsDispChanged(false),
  myWinAttribs(stDefaultWinAttributes()) {
    myDndList = new StString[1];
    myMonSlave.idMaster = 0;
    myMonSlave.idSlave  = 1; // second by default
    myMonSlave.xAdd = 1;
    myMonSlave.xSub = 0;
    myMonSlave.yAdd = 1;
    myMonSlave.ySub = 0;

#if(defined(_WIN32) || defined(__WIN32__))
    // we create Win32 event directly (not StEvent) to use it with MsgWaitForMultipleObjects()
    myEventQuit       = CreateEvent(0, true, false, NULL);
    myEventCursorShow = CreateEvent(0, true, false, NULL);
    myEventCursorHide = CreateEvent(0, true, false, NULL);
#endif

    // just debug output Monitors' configuration
    myMonitors.init();
    for(size_t aMonIter = 0; aMonIter < myMonitors.size(); ++aMonIter) {
        ST_DEBUG_LOG(myMonitors[aMonIter].toString());
    }

#ifdef __APPLE__
    // register callback for display configuration changes
    // alternatively we can add method applicationDidChangeScreenParameters to application delegate
    CGDisplayRegisterReconfigurationCallback(stDisplayChangeCallBack, this);
#endif
}

void StWindowImpl::updateMonitors() {
    myMonitors.init();
    // just debug output Monitors' configuration
    for(size_t aMonIter = 0; aMonIter < myMonitors.size(); ++aMonIter) {
        ST_DEBUG_LOG(myMonitors[aMonIter].toString());
    }
    myIsDispChanged = false;
    // should we check window is not out-of-screen here or all systems will do this for us?
}

StWindowImpl::~StWindowImpl() {
    close();
    delete[] myDndList;
#if(defined(_WIN32) || defined(__WIN32__))
    CloseHandle(myEventQuit);
    CloseHandle(myEventCursorShow);
    CloseHandle(myEventCursorHide);
#endif

#ifdef __APPLE__
    CGDisplayRemoveReconfigurationCallback(stDisplayChangeCallBack, this);
#endif
}

void StWindowImpl::close() {
    hide(ST_WIN_MASTER); hide(ST_WIN_SLAVE);
    myMessageList.append(StMessageList::MSG_EXIT);

    mySlave.close(); // close GL contexts
    myMaster.close();
#if(defined(_WIN32) || defined(__WIN32__))
    SetEvent(myEventQuit);
    size_t TIME_LIMIT = 60000;
    size_t timeWait = 10000;
    while(!myMaster.evMsgThread.wait(timeWait)) {
        ST_DEBUG_LOG("WinAPI, wait for Message thread to quit " + (timeWait / 1000) + " seconds!");
        timeWait += 10000;
        if(timeWait > TIME_LIMIT) {
            ST_DEBUG_LOG("WinAPI, Message thread was killed (timeout " + (TIME_LIMIT / 1000) + " seconds exceeded!)");
            myMsgThread->kill();
            break;
        }
    }
    myMsgThread.nullify();
#endif

    // turn off display sleep blocking
    myWinAttribs.toBlockSleepSystem  = false;
    myWinAttribs.toBlockSleepDisplay = false;
    updateBlockSleep();

    myParentWin = (StNativeWin_t )NULL;

    if(myWinAttribs.isFullScreen) {
        myFullScreenWinNb.decrement();
    }
    myWinAttribs.isFullScreen = false; // just hack to return window position after closing
}

#if (!defined(__APPLE__))
void StWindowImpl::setTitle(const StString& theTitle) {
    myWindowTitle = theTitle;
#if(defined(_WIN32) || defined(__WIN32__))
    myIsUpdated = true;
#elif(defined(__linux__) || defined(__linux))
    if(myMaster.hWindow != 0){
        XTextProperty aTitleProperty;
        aTitleProperty.encoding = None;
        char* aTitle = (char* )myWindowTitle.toCString();
        Xutf8TextListToTextProperty(myMaster.getDisplay(), &aTitle, 1, XUTF8StringStyle,  &aTitleProperty);
        XSetWMName(myMaster.getDisplay(), myMaster.hWindow, &aTitleProperty);
        XSetWMProperties(myMaster.getDisplay(), myMaster.hWindow, &aTitleProperty, &aTitleProperty, NULL, 0, NULL, NULL, NULL);
    }
#endif
}
#endif // !__APPLE__

void StWindowImpl::getAttributes(StWinAttributes_t* theAttributes) {
    size_t aBytesToCopy = stMin(theAttributes->nSize, sizeof(StWinAttributes_t));
    stMemCpy(theAttributes, &myWinAttribs, aBytesToCopy); // copy as much as possible
    theAttributes->nSize = aBytesToCopy;
}

void StWindowImpl::setAttributes(const StWinAttributes_t* theAttributes) {
    size_t aBytesToCopy = stMin(theAttributes->nSize, sizeof(StWinAttributes_t));
    stMemCpy(&myWinAttribs, theAttributes, aBytesToCopy); // copy as much as possible
    myWinAttribs.nSize = sizeof(StWinAttributes_t);       // restore own size
    updateSlaveConfig();
    updateWindowPos();
}

#if (defined(_WIN32) || defined(__WIN32__))
namespace {
    static StAtomic<int32_t> ST_BLOCK_SLEEP_COUNTER(0);
};
#endif

void StWindowImpl::updateBlockSleep() {
#if(defined(_WIN32) || defined(__WIN32__))
    if(myWinAttribs.toBlockSleepDisplay) {
        // prevent display sleep - call this periodically
        EXECUTION_STATE aState = ES_CONTINUOUS | ES_DISPLAY_REQUIRED | ES_SYSTEM_REQUIRED;
        if(myIsVistaPlus) {
            aState = aState | ES_AWAYMODE_REQUIRED;
        }
        SetThreadExecutionState(aState);

        if(myBlockSleep == BlockSleep_DISPLAY) {
            return;
        }

        if(ST_BLOCK_SLEEP_COUNTER.increment() == 1) {
            // block screensaver
            /*HKEY aKey = NULL;
            DWORD aDisp = 0, aData = 1;
            if(RegCreateKeyExW(HKEY_CURRENT_USER, L"Control Panel\Desktop\\", 0, NULL,
                               REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &aKey, &aDisp) != 0
            || RegSetValueExW(aKey, L"ScreenSaveActive", 0, REG_DWORD, (LPBYTE )&aData, sizeof(DWORD)) != 0) {
                ST_DEBUG_LOG("Couldn't save ScreenSaveActive parameter into register");
            }
            RegCloseKey(aKey);*/
        }
        myBlockSleep = BlockSleep_DISPLAY;
    } else if(myWinAttribs.toBlockSleepSystem) {
        // prevent system sleep - call this periodically
        EXECUTION_STATE aState = ES_CONTINUOUS | ES_SYSTEM_REQUIRED;
        if(myIsVistaPlus) {
            aState = aState | ES_AWAYMODE_REQUIRED;
        }
        SetThreadExecutionState(aState);

        if(myBlockSleep == BlockSleep_SYSTEM) {
            return;
        }

        myBlockSleep = BlockSleep_SYSTEM;
    } else if(myBlockSleep != BlockSleep_OFF) {
        if(ST_BLOCK_SLEEP_COUNTER.decrement() == 0) {
            SetThreadExecutionState(ES_CONTINUOUS);

            /*HKEY aKey = NULL;
            DWORD aDisp = 0, aData = 0;
            if(RegCreateKeyExW(HKEY_CURRENT_USER, L"Control Panel\Desktop\\", 0, NULL,
                               REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &aKey, &aDisp) != 0
            || RegSetValueExW(aKey, L"ScreenSaveActive", 0, REG_DWORD, (LPBYTE )&aData, sizeof(DWORD)) != 0) {
                ST_DEBUG_LOG("Couldn't save ScreenSaveActive parameter into register");
            }
            RegCloseKey(aKey);*/
        }
        myBlockSleep = BlockSleep_OFF;
    }
#elif(defined(__APPLE__))
    if(myWinAttribs.toBlockSleepDisplay) {
        if(myBlockSleep == BlockSleep_DISPLAY) {
            return;
        } else if(mySleepAssert != 0) {
            IOPMAssertionRelease(mySleepAssert);
            mySleepAssert = 0;
        }

        if(IOPMAssertionCreateWithName(kIOPMAssertionTypeNoDisplaySleep, kIOPMAssertionLevelOn,
                                       CFSTR("sView media playback"), &mySleepAssert) != kIOReturnSuccess) {
            ST_DEBUG_LOG("IOPMAssertionCreateWithName() call FAILed");
        }
        myBlockSleep = BlockSleep_DISPLAY;
    } else if(myWinAttribs.toBlockSleepSystem) {
        if(myBlockSleep == BlockSleep_SYSTEM) {
            return;
        } else if(mySleepAssert != 0) {
            IOPMAssertionRelease(mySleepAssert);
            mySleepAssert = 0;
        }

        if(IOPMAssertionCreateWithName(kIOPMAssertionTypeNoIdleSleep, kIOPMAssertionLevelOn,
                                       CFSTR("sView media playback"), &mySleepAssert) != kIOReturnSuccess) {
            ST_DEBUG_LOG("IOPMAssertionCreateWithName() call FAILed");
        }
        myBlockSleep = BlockSleep_SYSTEM;
    } else if(myBlockSleep != BlockSleep_OFF) {
        if(mySleepAssert != 0) {
            IOPMAssertionRelease(mySleepAssert);
            mySleepAssert = 0;
        }
        myBlockSleep = BlockSleep_OFF;
    }
#elif(defined(__linux__) || defined(__linux))
    if(myWinAttribs.toBlockSleepDisplay) { // || myWinAttribs.toBlockSleepSystem
        if(myBlockSleep == BlockSleep_DISPLAY
        || myMaster.stXDisplay.isNull()
        || myMaster.hWindow == 0) {
            return;
        }

        StArrayList<StString> anArguments(2);
        anArguments.add("suspend");
        anArguments.add(StString((size_t )myMaster.hWindow));
        if(!StProcess::execProcess("/usr/bin/xdg-screensaver", anArguments)) {
            ST_DEBUG_LOG("/usr/bin/xdg-screensaver is not found!");
        }
        myBlockSleep = BlockSleep_DISPLAY;
    } else if(myBlockSleep != BlockSleep_OFF) {
        if(myMaster.stXDisplay.isNull()
        || myMaster.hWindow == 0) {
            return;
        }

        StArrayList<StString> anArguments(2);
        anArguments.add("resume");
        anArguments.add(StString((size_t )myMaster.hWindow));
        if(!StProcess::execProcess("/usr/bin/xdg-screensaver", anArguments)) {
            //ST_DEBUG_LOG("/usr/bin/xdg-screensaver is not found!");
        }
        myBlockSleep = BlockSleep_OFF;
    }
#endif
}

void StWindowImpl::updateActiveState() {
    updateBlockSleep();

    if(myWinAttribs.isFullScreen) {
        myIsActive = true;
        return;
    }

    myIsActive = false;
    if(myFullScreenWinNb.getValue() > 0) {
        return;
    }

    for(size_t aMonIter = 0; aMonIter < myMonitors.size(); ++aMonIter) {
        if(!myMonitors[aMonIter].getVRect().isOut(myRectNorm)) {
            myIsActive = true;
            break;
        }
    }
}

#if (!defined(__APPLE__))
void StWindowImpl::show(const int& winNum) {
    if(winNum == ST_WIN_MASTER && myWinAttribs.isHide) {
    #if(defined(_WIN32) || defined(__WIN32__))
        if(myMaster.hWindow != NULL) {
            ShowWindow(myMaster.hWindow, SW_SHOW);
        } else if(myMaster.hWindowGl != NULL) {
            ShowWindow(myMaster.hWindowGl, SW_SHOW);
        }
    #elif(defined(__linux__) || defined(__linux))
        if(!myMaster.stXDisplay.isNull()) {
            if(myMaster.hWindow != 0) {
                XMapWindow(myMaster.getDisplay(), myMaster.hWindow);
                //XIfEvent(myMaster.getDisplay(), &myXEvent, stXWaitMapped, (char* )myMaster.hWindow);
            } else if(myMaster.hWindowGl != 0) {
                XMapWindow(myMaster.getDisplay(), myMaster.hWindowGl);
                //XIfEvent(myMaster.getDisplay(), &myXEvent, stXWaitMapped, (char* )myMaster.hWindowGl);
            }
        }
    #endif
        myWinAttribs.isHide = false;
        updateWindowPos();
    } else if(winNum == ST_WIN_SLAVE && myWinAttribs.isSlaveHide) {
    #if(defined(_WIN32) || defined(__WIN32__))
        if(mySlave.hWindowGl != NULL) {
            ShowWindow(mySlave.hWindowGl, SW_SHOW);
        }
    #elif(defined(__linux__) || defined(__linux))
        if(!mySlave.stXDisplay.isNull() && mySlave.hWindowGl != 0) {
            XMapWindow(mySlave.getDisplay(), mySlave.hWindowGl);
            //XIfEvent(myMaster.getDisplay(), &myXEvent, stXWaitMapped, (char* )mySlave.hWindowGl);
        }
    #endif
        myWinAttribs.isSlaveHide = false;
        updateWindowPos();
    }
}

void StWindowImpl::hide(const int& winNum) {
    if(winNum == ST_WIN_MASTER && !myWinAttribs.isHide) {
    #if(defined(_WIN32) || defined(__WIN32__))
        if(myMaster.hWindow != NULL) {
            ShowWindow(myMaster.hWindow, SW_HIDE);
        } else if(myMaster.hWindowGl != NULL) {
            ShowWindow(myMaster.hWindowGl, SW_HIDE);
        }
    #elif(defined(__linux__) || defined(__linux))
        if(!myMaster.stXDisplay.isNull()) {
            if(myMaster.hWindow != 0) {
                XUnmapWindow(myMaster.getDisplay(), myMaster.hWindow);
                myIsUpdated = true; // ?
            } else if(myMaster.hWindowGl != 0) {
                XUnmapWindow(myMaster.getDisplay(), myMaster.hWindowGl);
                myIsUpdated = true; // ?
            }
        }
    #endif
        myWinAttribs.isHide = true;
    } else if(winNum == ST_WIN_SLAVE && !myWinAttribs.isSlaveHide) {
    #if(defined(_WIN32) || defined(__WIN32__))
        if(mySlave.hWindowGl != NULL) {
            ShowWindow(mySlave.hWindowGl, SW_HIDE);
        }
    #elif(defined(__linux__) || defined(__linux))
        if(!mySlave.stXDisplay.isNull() && mySlave.hWindowGl != 0) {
            XUnmapWindow(mySlave.getDisplay(), mySlave.hWindowGl);
            myIsUpdated = true; // ?
        }
    #endif
        myWinAttribs.isSlaveHide = true;
    }
}
#endif // !__APPLE__

void StWindowImpl::showCursor(bool toShow) {
    if(myWinAttribs.isHideCursor != toShow) {
        return; // nothing to update
    }
#if(defined(_WIN32) || defined(__WIN32__))
    // native show / hide function should be called from
    // window-message thread -> we set events to do that
    if(toShow) {
        SetEvent(myEventCursorShow);
    } else {
        SetEvent(myEventCursorHide);
    }
#elif(defined(__linux__) || defined(__linux))
    if(toShow) {
        XUndefineCursor(myMaster.getDisplay(), myMaster.hWindowGl);
    } else {
        static const char noPixData[] = {0, 0, 0, 0, 0, 0, 0, 0};
        XColor black, dummy;
        Colormap cmap = DefaultColormap(myMaster.getDisplay(), DefaultScreen(myMaster.getDisplay()));
        XAllocNamedColor(myMaster.getDisplay(), cmap, "black", &black, &dummy);
        Pixmap noPix = XCreateBitmapFromData(myMaster.getDisplay(), myMaster.hWindowGl, noPixData, 8, 8);
        Cursor noPtr = XCreatePixmapCursor(myMaster.getDisplay(), noPix, noPix, &black, &black, 0, 0);
        XDefineCursor(myMaster.getDisplay(), myMaster.hWindowGl, noPtr);
        XFreeCursor(myMaster.getDisplay(), noPtr);
        if(noPix != None) {
            XFreePixmap(myMaster.getDisplay(), noPix);
        }
        XFreeColors(myMaster.getDisplay(), cmap, &black.pixel, 1, 0);
    }
#endif
    myWinAttribs.isHideCursor = !toShow;
}

#if (!defined(__APPLE__))
void StWindowImpl::setPlacement(const StRectI_t& theRect) {
    myRectNorm  = theRect;
    myIsUpdated = true;
#if(defined(_WIN32) || defined(__WIN32__))
    if(myMaster.hWindow != NULL && !myWinAttribs.isFullScreen) {
        int posLeft   = myRectNorm.left() - (!myWinAttribs.isNoDecor ?  GetSystemMetrics(SM_CXSIZEFRAME) : 0);
        int posTop    = myRectNorm.top()  - (!myWinAttribs.isNoDecor ? (GetSystemMetrics(SM_CYSIZEFRAME) + GetSystemMetrics(SM_CYCAPTION)) : 0);
        int winWidth  = (!myWinAttribs.isNoDecor ? 2 * GetSystemMetrics(SM_CXSIZEFRAME) : 0) + myRectNorm.width();
        int winHeight = (!myWinAttribs.isNoDecor ? (GetSystemMetrics(SM_CYCAPTION) + 2 * GetSystemMetrics(SM_CYSIZEFRAME)) : 0) + myRectNorm.height();
        SetWindowPos(myMaster.hWindow, HWND_NOTOPMOST,
                     posLeft, posTop, winWidth, winHeight,
                     SWP_NOACTIVATE);
    }
#elif(defined(__linux__) || defined(__linux))
    if(!myMaster.stXDisplay.isNull() && !myWinAttribs.isFullScreen && myMaster.hWindow != 0) {
        XMoveResizeWindow(myMaster.getDisplay(), myMaster.hWindow,
                          myRectNorm.left(),  myRectNorm.top(),
                          myRectNorm.width(), myRectNorm.height());
        XFlush(myMaster.getDisplay());
    }
#endif
}
#endif // !__APPLE__

StRectI_t StWindowImpl::getPlacement() {
    if(myWinAttribs.isFullScreen) {
        return myRectFull;
    } else {
        return myRectNorm;
    }
}

void StWindowImpl::getTiledWinRect(StRectI_t& theRect) const {
    switch(myTiledCfg) {
        case TiledCfg_MasterSlaveX: {
            theRect.right()  += theRect.width();
            return;
        }
        case TiledCfg_SlaveMasterX: {
            theRect.left()   -= theRect.width();
            return;
        }
        case TiledCfg_MasterSlaveY: {
            theRect.bottom() += theRect.height();
            return;
        }
        case TiledCfg_SlaveMasterY: {
            theRect.top()    -= theRect.height();
            return;
        }
        case TiledCfg_Separate:
        default: {
            return;
        }
    }
}

StGLBoxPx StWindowImpl::stglViewport(const int& theWinId) const {
    const StRectI_t& aWinRect = myWinAttribs.isFullScreen ? myRectFull : myRectNorm;
    const int aWidth  = aWinRect.width();
    const int aHeight = aWinRect.height();
    StGLBoxPx aRect = {{ 0, 0, aWidth, aHeight }};
    switch(myTiledCfg) {
        case TiledCfg_MasterSlaveX: {
            if(theWinId == ST_WIN_SLAVE) {
                aRect.x() += aWidth;
            }
            return aRect;
        }
        case TiledCfg_SlaveMasterX: {
            if(theWinId == ST_WIN_MASTER) {
                aRect.x() += aWidth;
            }
            return aRect;
        }
        case TiledCfg_MasterSlaveY: {
            if(theWinId == ST_WIN_MASTER) {
                aRect.y() += aHeight;
            }
            return aRect;
        }
        case TiledCfg_SlaveMasterY: {
            if(theWinId == ST_WIN_SLAVE) {
                aRect.y() += aHeight;
            }
            return aRect;
        }
        case TiledCfg_Separate:
        default: {
            return aRect;
        }
    }
}

int StWindowImpl::getMouseDown(StPointD_t* point) {
    return myMDownQueue.pop(*point);
}

int StWindowImpl::getMouseUp(StPointD_t* point) {
    return myMUpQueue.pop(*point);
}

StPointD_t StWindowImpl::getMousePos() {
    StRectI_t aWinRect = (myWinAttribs.isFullScreen) ? myRectFull : myRectNorm;
#if (defined(_WIN32) || defined(__WIN32__))
    if(myMaster.hWindowGl != NULL) {
        CURSORINFO aCursor;
        aCursor.cbSize = sizeof(aCursor);
        if(GetCursorInfo(&aCursor) != FALSE) {
            return StPointD_t((double(aCursor.ptScreenPos.x) - double(aWinRect.left())) / double(aWinRect.width()),
                              (double(aCursor.ptScreenPos.y) - double(aWinRect.top()))  / double(aWinRect.height()));
        }
    }
#elif (defined(__APPLE__))
    CGEventRef anEvent = CGEventCreate(NULL);
    CGPoint aCursor = CGEventGetLocation(anEvent);
    CFRelease(anEvent);
    return StPointD_t((aCursor.x - double(aWinRect.left())) / double(aWinRect.width()),
                      (aCursor.y - double(aWinRect.top()))  / double(aWinRect.height()));
#elif (defined(__linux__) || defined(__linux))
    if(myMaster.hWindowGl != 0) {
        Window childReturn;
        Window rootReturn = DefaultRootWindow(myMaster.getDisplay());
        int rootX = 0;
        int rootY = 0;
        int winX = 0;
        int winY = 0;
        unsigned int maskReturn;
        XQueryPointer(myMaster.getDisplay(), myMaster.hWindowGl,
                      &rootReturn, &childReturn,
                      &rootX, &rootY, &winX, &winY, &maskReturn);
        return StPointD_t((double )winX / (double )aWinRect.width(),
                          (double )winY / (double )aWinRect.height());
    }
#endif
    // undefined
    return StPointD_t(0.0, 0.0);
}

int StWindowImpl::getDragNDropFile(const int& theIndex, stUtf8_t* theOutFile, const size_t& theBuffSizeBytes) {
    myDndMutex.lock();
    if(theIndex < 0 || theOutFile == NULL || theBuffSizeBytes == 0) {
        size_t aCount = myDndCount;
        myDndMutex.unlock();
        return (int )aCount; // returns files' count
    }
    if(theIndex >= (int )myDndCount) {
        myDndMutex.unlock();
        return -1; // returns error
    }
    size_t aSizeBytes = myDndList[theIndex].getSize() + 1;
    if(aSizeBytes > theBuffSizeBytes) {
        myDndMutex.unlock();
        return (int )aSizeBytes; // returns needed buffer size
    }
    stMemCpy(theOutFile, myDndList[theIndex].toCString(), aSizeBytes);
    myDndMutex.unlock();
    return 0; // returns success
}

// Change active GL context in current thread
void StWindowImpl::stglMakeCurrent(const int& winNum){
    switch(winNum) {
          case ST_WIN_MASTER: {
            myMaster.glMakeCurrent();
            break;
        } case ST_WIN_SLAVE: {
            if(myTiledCfg == TiledCfg_Separate) {
                mySlave.glMakeCurrent();
            } else {
                myMaster.glMakeCurrent();
            }
            break;
        }
    }
}

// Swap Buffers (Double Buffering)
void StWindowImpl::stglSwap(const int& theWinId) {
    if(!myIsActive) {
        return;
    }

    switch(theWinId) {
        case ST_WIN_ALL: {
            if(myTiledCfg == TiledCfg_Separate) {
                myMaster.glSwap();
                mySlave.glSwap();
            } else {
                myMaster.glSwap();
            }
            break;
        }
        case ST_WIN_MASTER: {
            myMaster.glSwap();
            break;
        } case ST_WIN_SLAVE: {
            if(myTiledCfg == TiledCfg_Separate) {
                mySlave.glSwap();
            } else {
                myMaster.glSwap();
            }
            break;
        }
    }
}

stBool_t StWindowImpl::appendMessage(const StMessage_t& stMessage) {
    switch(stMessage.uin) {
        case StMessageList::MSG_MOUSE_DOWN_APPEND: {
            myMUpQueue.clear();
            myMDownQueue.clear();
            StMouseMessage_t* mouseData = (StMouseMessage_t* )stMessage.data;
            myMDownQueue.push(mouseData->point, mouseData->button);
            return myMessageList.append(StMessageList::MSG_MOUSE_DOWN);
        }
        case StMessageList::MSG_MOUSE_UP_APPEND: {
            StMouseMessage_t* mouseData = (StMouseMessage_t* )stMessage.data;
            myMUpQueue.push(mouseData->point, mouseData->button);
            return myMessageList.append(StMessageList::MSG_MOUSE_UP);
        }
        case StMessageList::MSG_KEY_DOWN_APPEND: {
            myMessageList.getKeysMap()[(size_t )stMessage.data] = true;
            return true;
        }
        case StMessageList::MSG_KEY_UP_APPEND: {
            myMessageList.getKeysMap()[(size_t )stMessage.data] = false;
            return true;
        }
        default: return myMessageList.append(stMessage);
    }
}

bool StWindowImpl::getValue(const size_t& key, size_t* value) {
    if(key == ST_WIN_DATAKEYS_RENDERER && myUserDataMap != 0) {
        *value = myUserDataMap;
        return true;
    }
    return false;
}

void StWindowImpl::setValue(const size_t& key, const size_t& value) {
    if(key == ST_WIN_DATAKEYS_RENDERER) {
        myUserDataMap = value;
    } else {
        ST_DEBUG_LOG_AT("NOT IMPLEMENTED!");
    }
}

// Exported class-methods wpappers.
ST_EXPORT StWindowInterface* StWindow_new() {
    return new StWindowImpl();
}

ST_EXPORT void StWindow_del(StWindowInterface* inst) {
    delete (StWindowImpl* )inst;
}

ST_EXPORT void StWindow_close(StWindowInterface* inst) {
    ((StWindowImpl* )inst)->close();
}

ST_EXPORT void StWindow_setTitle(StWindowInterface* inst, const stUtf8_t* theTitle) {
    ((StWindowImpl* )inst)->setTitle(theTitle);
}

ST_EXPORT void StWindow_getAttributes(StWindowInterface* inst, StWinAttributes_t* inOutAttributes) {
    ((StWindowImpl* )inst)->getAttributes(inOutAttributes);
}

ST_EXPORT void StWindow_setAttributes(StWindowInterface* inst, const StWinAttributes_t* inAttributes) {
    ((StWindowImpl* )inst)->setAttributes(inAttributes);
}

ST_EXPORT stBool_t StWindow_isActive(StWindowInterface* inst) {
    return ((StWindowImpl* )inst)->isActive();
}

ST_EXPORT stBool_t StWindow_isStereoOutput(StWindowInterface* inst) {
    return ((StWindowImpl* )inst)->isStereoOutput();
}

ST_EXPORT void StWindow_setStereoOutput(StWindowInterface* inst, stBool_t stereoState) {
    ((StWindowImpl* )inst)->setStereoOutput(stereoState);
}

ST_EXPORT void StWindow_show(StWindowInterface* inst, const int& win, const stBool_t& show) {
    if(show) {
        ((StWindowImpl* )inst)->show(win);
    } else {
        ((StWindowImpl* )inst)->hide(win);
    }
}

ST_EXPORT void StWindow_showCursor(StWindowInterface* inst, stBool_t toShow) {
    ((StWindowImpl* )inst)->showCursor(toShow);
}

ST_EXPORT stBool_t StWindow_isFullScreen(StWindowInterface* inst) {
    return ((StWindowImpl* )inst)->isFullScreen();
}

ST_EXPORT void StWindow_setFullScreen(StWindowInterface* inst, stBool_t fullscreen) {
    ((StWindowImpl* )inst)->setFullScreen(fullscreen);
}

ST_EXPORT void StWindow_getPlacement(StWindowInterface* inst, StRectI_t* rect) {
    *rect = ((StWindowImpl* )inst)->getPlacement();
}

ST_EXPORT void StWindow_setPlacement(StWindowInterface* inst, const StRectI_t* rect) {
    ((StWindowImpl* )inst)->setPlacement(*rect);
}

ST_EXPORT void StWindow_stglViewport(StWindowInterface* inst, const int& theWinId, StGLBoxPx* theRect) {
    *theRect = ((StWindowImpl* )inst)->stglViewport(theWinId);
}

ST_EXPORT void StWindow_getMousePos(StWindowInterface* inst, StPointD_t* point) {
    StPointD_t p = ((StWindowImpl* )inst)->getMousePos();
    stMemCpy(point, &p, sizeof(StPointD_t));
}

ST_EXPORT int StWindow_getMouseDown(StWindowInterface* inst, StPointD_t* point) {
    return ((StWindowImpl* )inst)->getMouseDown(point);
}

ST_EXPORT int StWindow_getMouseUp(StWindowInterface* inst, StPointD_t* point) {
    return ((StWindowImpl* )inst)->getMouseUp(point);
}

ST_EXPORT int StWindow_getDragNDropFile(StWindowInterface* inst, const int& theIndex, stUtf8_t* theOutFile, const size_t& theBuffSizeBytes) {
    return ((StWindowImpl* )inst)->getDragNDropFile(theIndex, theOutFile, theBuffSizeBytes);
}

ST_EXPORT void StWindow_stglSwap(StWindowInterface* inst, const int& value) {
    ((StWindowImpl* )inst)->stglSwap(value);
}

ST_EXPORT void StWindow_stglMakeCurrent(StWindowInterface* inst, const int& value) {
    ((StWindowImpl* )inst)->stglMakeCurrent(value);
}

ST_EXPORT stBool_t StWindow_stglCreate(StWindowInterface*       inst,
                                       const StWinAttributes_t* theAttributes,
                                       const StNativeWin_t      theNativeParentWindow) {
    return ((StWindowImpl* )inst)->stglCreate(theAttributes, theNativeParentWindow);
}

ST_EXPORT double StWindow_stglGetTargetFps(StWindowInterface* inst) {
    return ((StWindowImpl* )inst)->stglGetTargetFps();
}

ST_EXPORT void StWindow_stglSetTargetFps(StWindowInterface* inst, const double& fps) {
    ((StWindowImpl* )inst)->stglSetTargetFps(fps);
}

ST_EXPORT void StWindow_callback(StWindowInterface* inst, StMessage_t* theMessages) {
    ((StWindowImpl* )inst)->callback(theMessages);
}

ST_EXPORT stBool_t StWindow_appendMessage(StWindowInterface* inst, const StMessage_t& stMessage) {
    return ((StWindowImpl* )inst)->appendMessage(stMessage);
}

ST_EXPORT stBool_t StWindow_getValue(StWindowInterface* inst, const size_t& key, size_t* value) {
    return ((StWindowImpl* )inst)->getValue(key, value);
}

ST_EXPORT void StWindow_setValue(StWindowInterface* inst, const size_t& key, const size_t& value) {
    ((StWindowImpl* )inst)->setValue(key, value);
}

ST_EXPORT void* StWindow_memAlloc(const size_t& bytes) {
    return stMemAlloc(bytes);
}

ST_EXPORT void StWindow_memFree(void* ptr) {
    stMemFree(ptr);
}
