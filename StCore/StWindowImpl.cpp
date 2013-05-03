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

StWindowImpl::StWindowImpl(const StNativeWin_t theParentWindow)
: myParentWin(theParentWindow),
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
#ifdef _WIN32
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
  myIsDispChanged(false) {
    stMemZero(&attribs, sizeof(attribs));
    attribs.IsNoDecor      = false;
    attribs.IsStereoOutput = false;
    attribs.IsGlStereo     = false;
    attribs.GlDepthSize    = 16;
    attribs.IsFullScreen   = false;
    attribs.IsHidden       = false;
    attribs.ToHideCursor   = false;
    attribs.ToBlockSleepSystem  = false;
    attribs.ToBlockSleepDisplay = false;
    attribs.AreGlobalMediaKeys  = false;
    attribs.Slave      = StWinSlave_slaveOff;
    attribs.SlaveMonId = 1;

    myDndList = new StString[1];
    myMonSlave.idMaster = 0;
    myMonSlave.idSlave  = 1; // second by default
    myMonSlave.xAdd = 1;
    myMonSlave.xSub = 0;
    myMonSlave.yAdd = 1;
    myMonSlave.ySub = 0;

#ifdef _WIN32
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
#ifdef _WIN32
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
#ifdef _WIN32
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
    const bool toBlockSleepSystem  = attribs.ToBlockSleepSystem;
    const bool toBlockSleepDisplay = attribs.ToBlockSleepDisplay;
    attribs.ToBlockSleepSystem  = false;
    attribs.ToBlockSleepDisplay = false;
    updateBlockSleep();
    attribs.ToBlockSleepSystem  = toBlockSleepSystem;
    attribs.ToBlockSleepDisplay = toBlockSleepDisplay;

    myParentWin = (StNativeWin_t )NULL;

    if(attribs.IsFullScreen) {
        myFullScreenWinNb.decrement();
    }
    attribs.IsFullScreen = false; // just hack to return window position after closing
}

#if (!defined(__APPLE__))
void StWindowImpl::setTitle(const StString& theTitle) {
    myWindowTitle = theTitle;
#ifdef _WIN32
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

void StWindowImpl::getAttributes(StWinAttr* theAttributes) const {
    if(theAttributes == NULL) {
        return;
    }

    for(StWinAttr* anIter = theAttributes; *anIter != StWinAttr_NULL; anIter += 2) {
        switch(anIter[0]) {
            case StWinAttr_GlQuadStereo:
                anIter[1] = (StWinAttr )attribs.IsGlStereo;
                break;
            case StWinAttr_GlDepthSize:
                anIter[1] = (StWinAttr )attribs.GlDepthSize;
                break;
            case StWinAttr_ToBlockSleepSystem:
                anIter[1] = (StWinAttr )attribs.ToBlockSleepSystem;
                break;
            case StWinAttr_ToBlockSleepDisplay:
                anIter[1] = (StWinAttr )attribs.ToBlockSleepDisplay;
                break;
            case StWinAttr_GlobalMediaKeys:
                anIter[1] = (StWinAttr )attribs.AreGlobalMediaKeys;
                break;
            case StWinAttr_SlaveCfg:
                anIter[1] = (StWinAttr )attribs.Slave;
                break;
            case StWinAttr_SlaveMon:
                anIter[1] = (StWinAttr )attribs.SlaveMonId;
                break;
            default:
                ST_DEBUG_LOG("UNKNOWN window attribute #" + anIter[0] + " requested");
                break;
        }
    }
}

void StWindowImpl::setAttributes(const StWinAttr* theAttributes) {
    if(theAttributes == NULL) {
        return;
    }

    bool hasSlaveChanges = false;
    for(const StWinAttr* anIter = theAttributes; anIter[0] != StWinAttr_NULL; anIter += 2) {
        switch(anIter[0]) {
            case StWinAttr_GlQuadStereo:
                attribs.IsGlStereo = (anIter[1] == 1);
                break;
            case StWinAttr_GlDepthSize:
                attribs.GlDepthSize = (int8_t )anIter[1];
                break;
            case StWinAttr_ToBlockSleepSystem:
                attribs.ToBlockSleepSystem = (anIter[1] == 1);
                break;
            case StWinAttr_ToBlockSleepDisplay:
                attribs.ToBlockSleepDisplay = (anIter[1] == 1);
                break;
            case StWinAttr_GlobalMediaKeys:
                attribs.AreGlobalMediaKeys = (anIter[1] == 1);
                break;
            case StWinAttr_SlaveCfg:
                hasSlaveChanges = hasSlaveChanges || (attribs.Slave != (StWinSlave )anIter[1]);
                attribs.Slave = (StWinSlave )anIter[1];
                break;
            case StWinAttr_SlaveMon:
                hasSlaveChanges = hasSlaveChanges || (attribs.SlaveMonId != anIter[1]);
                attribs.SlaveMonId = (int8_t )anIter[1];
                break;
            default:
                ST_DEBUG_LOG("UNKNOWN window attribute #" + anIter[0] + " requested");
                break;
        }
    }

    if(hasSlaveChanges) {
        updateSlaveConfig();
        updateWindowPos();
    }
}

#ifdef _WIN32
namespace {
    static StAtomic<int32_t> ST_BLOCK_SLEEP_COUNTER(0);
};
#endif

void StWindowImpl::updateBlockSleep() {
#ifdef _WIN32
    #ifndef ES_AWAYMODE_REQUIRED // for old MinGW
        #define ES_AWAYMODE_REQUIRED ((DWORD)0x00000040)
    #endif
    if(attribs.ToBlockSleepDisplay) {
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
    } else if(attribs.ToBlockSleepSystem) {
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
    if(attribs.ToBlockSleepDisplay) {
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
    } else if(attribs.ToBlockSleepSystem) {
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
    if(attribs.ToBlockSleepDisplay) { // || attribs.ToBlockSleepSystem
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

    if(attribs.IsFullScreen) {
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
void StWindowImpl::show(const int theWinNum) {
    if((theWinNum == ST_WIN_MASTER || theWinNum == ST_WIN_ALL)
     && attribs.IsHidden) {
    #ifdef _WIN32
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
        attribs.IsHidden = false;
        updateWindowPos();
    }
    if((theWinNum == ST_WIN_SLAVE || theWinNum == ST_WIN_ALL)
     && attribs.IsSlaveHidden) {
    #ifdef _WIN32
        if(mySlave.hWindowGl != NULL) {
            ShowWindow(mySlave.hWindowGl, SW_SHOW);
        }
    #elif(defined(__linux__) || defined(__linux))
        if(!mySlave.stXDisplay.isNull() && mySlave.hWindowGl != 0) {
            XMapWindow(mySlave.getDisplay(), mySlave.hWindowGl);
            //XIfEvent(myMaster.getDisplay(), &myXEvent, stXWaitMapped, (char* )mySlave.hWindowGl);
        }
    #endif
        attribs.IsSlaveHidden = false;
        updateWindowPos();
    }
}

void StWindowImpl::hide(const int theWinNum) {
    if((theWinNum == ST_WIN_MASTER || theWinNum == ST_WIN_ALL)
    && !attribs.IsHidden) {
    #ifdef _WIN32
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
        attribs.IsHidden = true;
    }
    if((theWinNum == ST_WIN_SLAVE || theWinNum == ST_WIN_ALL)
    && !attribs.IsSlaveHidden) {
    #ifdef _WIN32
        if(mySlave.hWindowGl != NULL) {
            ShowWindow(mySlave.hWindowGl, SW_HIDE);
        }
    #elif(defined(__linux__) || defined(__linux))
        if(!mySlave.stXDisplay.isNull() && mySlave.hWindowGl != 0) {
            XUnmapWindow(mySlave.getDisplay(), mySlave.hWindowGl);
            myIsUpdated = true; // ?
        }
    #endif
        attribs.IsSlaveHidden = true;
    }
}
#endif // !__APPLE__

void StWindowImpl::showCursor(bool toShow) {
    if(attribs.ToHideCursor != toShow) {
        return; // nothing to update
    }
#ifdef _WIN32
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
        myMaster.setupNoCursor();
    }
#endif
    attribs.ToHideCursor = !toShow;
}

#if (!defined(__APPLE__))
void StWindowImpl::setPlacement(const StRectI_t& theRect,
                                const bool       theMoveToScreen) {
    if(theMoveToScreen) {
        const StPointI_t aCenter = theRect.center();
        const StMonitor& aMon = myMonitors[aCenter];
        if(!aMon.getVRect().isPointIn(aCenter)) {
            ST_DEBUG_LOG("Warning, window position is out of the monitor(" + aMon.getId() + ")!" + theRect.toString());
            const int aWidth  = theRect.width();
            const int aHeight = theRect.height();
            StRectI_t aRect;
            aRect.left()   = aMon.getVRect().left() + 256;
            aRect.right()  = aRect.left() + aWidth;
            aRect.top()    = aMon.getVRect().top() + 256;
            aRect.bottom() = aRect.top() + aHeight;
            myRectNorm = aRect;
        } else {
            myRectNorm = theRect;
        }
    } else {
        myRectNorm = theRect;
    }
    myIsUpdated = true;
#ifdef _WIN32
    if(myMaster.hWindow != NULL && !attribs.IsFullScreen) {
        int posLeft   = myRectNorm.left() - (!attribs.IsNoDecor ?  GetSystemMetrics(SM_CXSIZEFRAME) : 0);
        int posTop    = myRectNorm.top()  - (!attribs.IsNoDecor ? (GetSystemMetrics(SM_CYSIZEFRAME) + GetSystemMetrics(SM_CYCAPTION)) : 0);
        int winWidth  = (!attribs.IsNoDecor ? 2 * GetSystemMetrics(SM_CXSIZEFRAME) : 0) + myRectNorm.width();
        int winHeight = (!attribs.IsNoDecor ? (GetSystemMetrics(SM_CYCAPTION) + 2 * GetSystemMetrics(SM_CYSIZEFRAME)) : 0) + myRectNorm.height();
        SetWindowPos(myMaster.hWindow, HWND_NOTOPMOST,
                     posLeft, posTop, winWidth, winHeight,
                     SWP_NOACTIVATE);
    }
#elif(defined(__linux__) || defined(__linux))
    if(!myMaster.stXDisplay.isNull() && !attribs.IsFullScreen && myMaster.hWindow != 0) {
        XMoveResizeWindow(myMaster.getDisplay(), myMaster.hWindow,
                          myRectNorm.left(),  myRectNorm.top(),
                          myRectNorm.width(), myRectNorm.height());
        XFlush(myMaster.getDisplay());
    }
#endif
}
#endif // !__APPLE__

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

void StWindowImpl::correctTiledCursor(int& theLeft, int& theTop) const {
    const StRectI_t& aWinRect = attribs.IsFullScreen ? myRectFull : myRectNorm;
    switch(myTiledCfg) {
        case TiledCfg_SlaveMasterX: {
            theLeft -= aWinRect.width();
            return;
        }
        case TiledCfg_SlaveMasterY: {
            theTop -= aWinRect.height();
            return;
        }
        case TiledCfg_MasterSlaveX:
        case TiledCfg_MasterSlaveY:
        case TiledCfg_Separate:
        default: {
            return;
        }
    }
}

StGLBoxPx StWindowImpl::stglViewport(const int& theWinId) const {
    const StRectI_t& aWinRect = attribs.IsFullScreen ? myRectFull : myRectNorm;
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

int StWindowImpl::getMouseDown(StPointD_t& thePoint) {
    return myMDownQueue.pop(thePoint);
}

int StWindowImpl::getMouseUp(StPointD_t& thePoint) {
    return myMUpQueue.pop(thePoint);
}

StPointD_t StWindowImpl::getMousePos() {
    StRectI_t aWinRect = (attribs.IsFullScreen) ? myRectFull : myRectNorm;
#ifdef _WIN32
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

        correctTiledCursor(winX, winY);
        return StPointD_t((double )winX / (double )aWinRect.width(),
                          (double )winY / (double )aWinRect.height());
    }
#endif
    // undefined
    return StPointD_t(0.0, 0.0);
}

int StWindowImpl::getDragNDropFile(const int theIndex,
                                   StString& theFile) {
    myDndMutex.lock();
    if(theIndex < 0) {
        size_t aCount = myDndCount;
        myDndMutex.unlock();
        return (int )aCount; // returns files' count
    }
    if(theIndex >= (int )myDndCount) {
        myDndMutex.unlock();
        return -1; // returns error
    }

    theFile = myDndList[theIndex];
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
