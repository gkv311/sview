/**
 * Copyright © 2007-2014 Kirill Gavrilov <kirill@sview.ru>
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
#include <StThreads/StProcess.h>
#include <StThreads/StThread.h>
#include <StGL/StGLContext.h>

#ifdef __APPLE__
    #include <sys/sysctl.h>
#endif

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
  myWinMonScaleId(0),
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
  myIsUpdated(false),
  myIsActive(false),
  myBlockSleep(BlockSleep_OFF),
  myIsDispChanged(false),
  myLastEventsTime(0.0),
  myEventsThreaded(false),
  myIsMouseMoved(false) {
    stMemZero(&attribs, sizeof(attribs));
    stMemZero(&signals, sizeof(signals));
    attribs.IsNoDecor      = false;
    attribs.IsStereoOutput = false;
    attribs.IsGlStereo     = false;
    attribs.IsGlDebug      = false;
    attribs.GlDepthSize    = 16;
    attribs.IsFullScreen   = false;
    attribs.IsExclusiveFullScr = false;
    attribs.IsHidden       = false;
    attribs.ToHideCursor   = false;
    attribs.ToBlockSleepSystem  = false;
    attribs.ToBlockSleepDisplay = false;
    attribs.AreGlobalMediaKeys  = false;
    attribs.Slave      = StWinSlave_slaveOff;
    attribs.SlaveMonId = 1;
    attribs.Split      = StWinSlave_splitOff;
    attribs.ToAlignEven = false;

    myMonSlave.idMaster = 0;
    myMonSlave.idSlave  = 1; // second by default
    myMonSlave.xAdd = 1;
    myMonSlave.xSub = 0;
    myMonSlave.yAdd = 1;
    myMonSlave.ySub = 0;

#ifdef _WIN32
    myEventsThreaded  = true; // events loop is always performed in dedicated thread

    // we create Win32 event directly (not StCondition) to use it with MsgWaitForMultipleObjects()
    myEventQuit       = CreateEvent(0, true, false, NULL);
    myEventCursorShow = CreateEvent(0, true, false, NULL);
    myEventCursorHide = CreateEvent(0, true, false, NULL);

    // Adjust system timer
    // By default Windows2K+ timer has ugly precision
    // Thus - Sleep(1) may be long 14ms!
    // We force best available precision to make Sleep() more adequate
    // This affect whole system while running application!
    TIMECAPS aTimeCaps = {0, 0};
    if(timeGetDevCaps(&aTimeCaps, sizeof(aTimeCaps)) == TIMERR_NOERROR) {
        timeBeginPeriod(aTimeCaps.wPeriodMin);
    } else {
        timeBeginPeriod(1);
    }
#endif

    myMonitors.init();
#ifdef __APPLE__
    // register callback for display configuration changes
    // alternatively we can add method applicationDidChangeScreenParameters to application delegate
    CGDisplayRegisterReconfigurationCallback(stDisplayChangeCallBack, this);
#endif

    myEventsTimer.initUpTime();
}

void StWindowImpl::StSyncTimer::initUpTime() {
#if defined(_WIN32)
    myGetTick64 = NULL;
    if(StSys::isVistaPlus()) {
        HMODULE aKern32 = GetModuleHandleW(L"kernel32");
        myGetTick64 = (GetTickCount64_t )GetProcAddress(aKern32, "GetTickCount64");
    }

    // Spin waiting for a change in system time (should take up to 15 ms on modern systems).
    // Alternatively timeGetTime() might be used instead which handle time in 1 ms precision
    // when used in combination with timeBeginPeriod(1).
    const uint64_t anUptime0 = (myGetTick64 != NULL) ? myGetTick64() : (uint64_t )GetTickCount();
    uint64_t anUptime1 = 0;
    do {
        anUptime1 = (myGetTick64 != NULL) ? myGetTick64() : (uint64_t )GetTickCount();
        fillCounter(myCounterStart);
    } while(anUptime0 == anUptime1);
    myTimeInMicroSec = anUptime1 * 1000.0; // convert to microseconds
#elif defined(__APPLE__)
    // use function from CoreServices to retrieve system uptime
    const Nanoseconds anUpTimeNano = AbsoluteToNanoseconds(UpTime());
    myTimeInMicroSec = double((*(uint64_t* )&anUpTimeNano) / 1000); // convert to microseconds
    fillCounter(myCounterStart);
#else
    myTimeInMicroSec = getUpTimeFromSystem() * 1000000.0;
    fillCounter(myCounterStart);
#endif
    myIsPaused = false;
    myLastSyncMicroSec = myTimeInMicroSec;
}

void StWindowImpl::StSyncTimer::resyncUpTime() {
#if defined(_WIN32)
    // spin waiting for a change in system time (should take up to 15 ms on modern systems)
    const uint64_t anUptime0 = (myGetTick64 != NULL) ? myGetTick64() : (uint64_t )GetTickCount();
    uint64_t anUptime1 = 0;
    double aTimerValue = 0.0;
    do {
        anUptime1   = (myGetTick64 != NULL) ? myGetTick64() : (uint64_t )GetTickCount();
        aTimerValue = getElapsedTimeInMicroSec();
    } while(anUptime0 == anUptime1);
    mySyncMicroSec     = float(anUptime1 * 1000.0 - aTimerValue);
    myLastSyncMicroSec = aTimerValue;
    //ST_DEBUG_LOG("resyncUpTime()= " + double(mySyncMicroSec) * 0.001
    //           + " msec (" + getElapsedTimeFromLastStartInMicroSec() + " after app start)");
#elif !defined(__APPLE__)
    struct sysinfo aSysInfo;
    ::sysinfo(&aSysInfo);
    const uint64_t anUptime0 = (uint64_t )aSysInfo.uptime;
    uint64_t anUptime1 = 0;
    double aTimerValue = 0.0;
    do {
        ::sysinfo(&aSysInfo);
        anUptime1 = (uint64_t )aSysInfo.uptime;
        aTimerValue = getElapsedTimeInMicroSec();
    } while(anUptime0 == anUptime1);
    mySyncMicroSec     = float(anUptime1 * 1000000.0 - aTimerValue);
    myLastSyncMicroSec = aTimerValue;
#endif
}

void StWindowImpl::updateMonitors() {
    myMonitors.init(true); // force update of cached state
    // just debug output Monitors' configuration
    for(size_t aMonIter = 0; aMonIter < myMonitors.size(); ++aMonIter) {
        ST_DEBUG_LOG(myMonitors[aMonIter].toString());
    }
    myIsDispChanged = false;
    // should we check window is not out-of-screen here or all systems will do this for us?
}

StWindowImpl::~StWindowImpl() {
    close();
#ifdef _WIN32
    CloseHandle(myEventQuit);
    CloseHandle(myEventCursorShow);
    CloseHandle(myEventCursorHide);

    // restore timer adjustments
    TIMECAPS aTimeCaps = {0, 0};
    if(timeGetDevCaps(&aTimeCaps, sizeof(aTimeCaps)) == TIMERR_NOERROR) {
        timeEndPeriod(aTimeCaps.wPeriodMin);
    } else {
        timeEndPeriod(1);
    }
#endif

#ifdef __APPLE__
    CGDisplayRemoveReconfigurationCallback(stDisplayChangeCallBack, this);
#endif
}

void StWindowImpl::close() {
    hide(ST_WIN_MASTER);
    hide(ST_WIN_SLAVE);

    // close GL contexts
    myGlContext.nullify();
    mySlave.close();
    myMaster.close();
#ifdef _WIN32
    SetEvent(myEventQuit);
    const size_t TIME_LIMIT = 60000;
    size_t       aTimeWait  = 10000;
    while(!myMaster.EventMsgThread.wait(aTimeWait)) {
        ST_DEBUG_LOG("WinAPI, wait for Message thread to quit " + (aTimeWait / 1000) + " seconds!");
        aTimeWait += 10000;
        if(aTimeWait > TIME_LIMIT) {
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

double StWindowImpl::getEventTime() const {
#if defined(__APPLE__)
    // read UpTimer directly from system - it is precise enough
    return myEventsTimer.getUpTimeFromSystem();
#else
    return myEventsTimer.getUpTime();
#endif
}

double StWindowImpl::getEventTime(const uint32_t theTime) const {
    double aTime = double(theTime) * 0.001; // system upload time in milliseconds

    // workaround integer overflows each 49 days
    const double aTimeSys = myEventsTimer.getUpTime() - 7200.0; // current time - 2 hours
    for(; aTime < aTimeSys; ) {
        aTime += double(uint32_t(-1)) * 0.001;
    }

    return aTime;
}

#if (!defined(__APPLE__))
void StWindowImpl::setTitle(const StString& theTitle) {
    myWindowTitle = theTitle;
#ifdef _WIN32
    myIsUpdated = true;
#elif defined(__linux__)
    if(myMaster.hWindow != 0){
        XTextProperty aTitleProperty = {NULL, 0, 0, 0};
        aTitleProperty.encoding = None;
        char* aTitle = (char* )myWindowTitle.toCString();
        Xutf8TextListToTextProperty(myMaster.getDisplay(), &aTitle, 1, XUTF8StringStyle,  &aTitleProperty);
        XSetWMName(myMaster.getDisplay(), myMaster.hWindow, &aTitleProperty);
        XSetWMProperties(myMaster.getDisplay(), myMaster.hWindow, &aTitleProperty, &aTitleProperty, NULL, 0, NULL, NULL, NULL);
        XFree(aTitleProperty.value);
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
            case StWinAttr_GlDebug:
                anIter[1] = (StWinAttr )attribs.IsGlDebug;
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
            case StWinAttr_SplitCfg:
                anIter[1] = (StWinAttr )attribs.Split;
                break;
            case StWinAttr_ToAlignEven:
                anIter[1] = (StWinAttr )attribs.ToAlignEven;
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
            case StWinAttr_GlDebug:
                attribs.IsGlDebug  = (anIter[1] == 1);
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
            case StWinAttr_SplitCfg:
                if(attribs.Split != (StWinSplit )anIter[1]
                && attribs.IsFullScreen) {
                    if(attribs.Split == StWinSlave_splitHorizontal) {
                        myRectFull.right() += myRectFull.width();
                    } else if(attribs.Split == StWinSlave_splitVertical) {
                        myRectFull.bottom() += myRectFull.height();
                    }
                    if((StWinSplit )anIter[1] == StWinSlave_splitHorizontal) {
                        myTiledCfg = TiledCfg_MasterSlaveX;
                        myRectFull.right() -= myRectFull.width() / 2;
                    } else if((StWinSplit )anIter[1] == StWinSlave_splitVertical) {
                        myTiledCfg = TiledCfg_MasterSlaveY;
                        myRectFull.bottom() -= myRectFull.height() / 2;
                    } else {
                        myTiledCfg = TiledCfg_Separate;
                    }
                    myStEventAux.Type       = stEvent_Size;
                    myStEventAux.Size.Time  = getEventTime();
                    myStEventAux.Size.SizeX = myRectFull.width();
                    myStEventAux.Size.SizeY = myRectFull.height();
                    signals.onResize->emit(myStEventAux.Size);
                }
                attribs.Split = (StWinSplit )anIter[1];
                break;
            case StWinAttr_ToAlignEven:
                attribs.ToAlignEven = (anIter[1] == 1);
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
#elif defined(__APPLE__)
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
#elif defined(__linux__)
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

#ifndef _WIN32
bool StWindowImpl::isParentOnScreen() const {
    if(myParentWin == (StNativeWin_t )NULL) {
        return false;
    }

    return true; // not implemented
}
#endif

void StWindowImpl::updateActiveState() {
    updateBlockSleep();

    if(attribs.IsFullScreen) {
        myIsActive = true;
        return;
    }

    myIsActive = false;
    if(myFullScreenWinNb.getValue() > 0
    || myRectNorm.width()  < 10
    || myRectNorm.height() < 10) {
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
    #elif defined(__linux__)
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
    #elif defined(__linux__)
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
    #elif defined(__linux__)
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
    #elif defined(__linux__)
        if(!mySlave.stXDisplay.isNull() && mySlave.hWindowGl != 0) {
            XUnmapWindow(mySlave.getDisplay(), mySlave.hWindowGl);
            myIsUpdated = true; // ?
        }
    #endif
        attribs.IsSlaveHidden = true;
    }
}

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
#elif defined(__linux__)
    if(toShow) {
        XUndefineCursor(myMaster.getDisplay(), myMaster.hWindowGl);
    } else {
        myMaster.setupNoCursor();
    }
#endif
    attribs.ToHideCursor = !toShow;
}

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
    if(myMaster.hWindow != NULL
    && !attribs.IsFullScreen) {
        RECT aRect;
        aRect.top    = myRectNorm.top();
        aRect.bottom = myRectNorm.bottom();
        aRect.left   = myRectNorm.left();
        aRect.right  = myRectNorm.right();
        // take into account decorations
        const DWORD aWinStyle   = (!attribs.IsNoDecor ? WS_OVERLAPPEDWINDOW : WS_POPUP) | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
        const DWORD aWinStyleEx = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE | WS_EX_ACCEPTFILES;
        AdjustWindowRectEx(&aRect, aWinStyle, FALSE, aWinStyleEx);
        SetWindowPos(myMaster.hWindow, HWND_NOTOPMOST,
                     aRect.left, aRect.top, aRect.right - aRect.left, aRect.bottom - aRect.top,
                     SWP_NOACTIVATE);
    }
#elif defined(__linux__)
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
            } else if(theWinId == ST_WIN_ALL) {
                aRect.width() += aWidth;
            }
            convertRectToBacking(aRect, ST_WIN_MASTER);
            return aRect;
        }
        case TiledCfg_SlaveMasterX: {
            if(theWinId == ST_WIN_MASTER) {
                aRect.x() += aWidth;
            } else if(theWinId == ST_WIN_ALL) {
                aRect.width() += aWidth;
            }
            convertRectToBacking(aRect, ST_WIN_MASTER);
            return aRect;
        }
        case TiledCfg_MasterSlaveY: {
            if(theWinId == ST_WIN_MASTER) {
                aRect.y() += aHeight;
            } else if(theWinId == ST_WIN_ALL) {
                aRect.height() += aHeight;
            }
            convertRectToBacking(aRect, ST_WIN_MASTER);
            return aRect;
        }
        case TiledCfg_SlaveMasterY: {
            if(theWinId == ST_WIN_SLAVE) {
                aRect.y() += aHeight;
            } else if(theWinId == ST_WIN_ALL) {
                aRect.height() += aHeight;
            }
            convertRectToBacking(aRect, ST_WIN_MASTER);
            return aRect;
        }
        case TiledCfg_Separate:
        default: {
            convertRectToBacking(aRect, theWinId);
            return aRect;
        }
    }
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

void StWindowImpl::swapEventsBuffers() {
    myEventsBuffer.swapBuffers();
    for(size_t anEventIter = 0; anEventIter < myEventsBuffer.getSize(); ++anEventIter) {
        StEvent& anEvent = myEventsBuffer.changeEvent(anEventIter);
        switch(anEvent.Type) {
            case stEvent_Close:
                signals.onClose->emit(anEvent.Close);
                break;
            case stEvent_Size:
                signals.onResize->emit(anEvent.Size);
                break;
            case stEvent_NewMonitor:
                signals.onAnotherMonitor->emit(anEvent.Size);
                break;
            case stEvent_KeyDown:
                signals.onKeyDown->emit(anEvent.Key);
                break;
            case stEvent_KeyUp: {
                // reconstruct duration event
                anEvent.Key.Progress = stMin(anEvent.Key.Time - myLastEventsTime, anEvent.Key.Duration);
                if(anEvent.Key.Progress > 1.e-7) {
                    anEvent.Type = stEvent_KeyHold;
                    signals.onKeyHold->emit(anEvent.Key);
                }
                anEvent.Type = stEvent_KeyUp;
                anEvent.Key.Progress = 0.0;
                signals.onKeyUp->emit(anEvent.Key);
                break;
            }
            case stEvent_MouseDown:
                signals.onMouseDown->emit(anEvent.Button);
                break;
            case stEvent_MouseUp:
               signals.onMouseUp->emit(anEvent.Button);
               break;
            case stEvent_FileDrop:
                signals.onFileDrop->emit(anEvent.DNDrop);
                break;
            case stEvent_Navigate:
                signals.onNavigate->emit(anEvent.Navigate);
                break;
            case stEvent_Action:
                signals.onAction->emit(anEvent.Action);
                break;
            default: break;
        }
    }

    // post key hold events
    const double aCurrTime = getEventTime();
    StKeyEvent aHoldEvent;
    aHoldEvent.Type = stEvent_KeyHold;
    aHoldEvent.Time = aCurrTime;
    aHoldEvent.Char = 0;
    double aKeyTime = 0.0;
    for(int aKeyIter = 0; aKeyIter < 256; ++aKeyIter) {
        if(myKeysState.isKeyDown((StVirtKey )aKeyIter, aKeyTime)) {
            aHoldEvent.VKey     = (StVirtKey )aKeyIter;
            aHoldEvent.Duration = aHoldEvent.Time - aKeyTime;
            aHoldEvent.Progress = stMin(aHoldEvent.Time - myLastEventsTime, aHoldEvent.Duration);
            aHoldEvent.Flags = ST_VF_NONE;
            if(myKeysState.isKeyDown(ST_VK_SHIFT)) {
                aHoldEvent.Flags = StVirtFlags(aHoldEvent.Flags | ST_VF_SHIFT);
            }
            if(myKeysState.isKeyDown(ST_VK_CONTROL)) {
                aHoldEvent.Flags = StVirtFlags(aHoldEvent.Flags | ST_VF_CONTROL);
            }
            if(aHoldEvent.Progress > 1.e-7) {
                signals.onKeyHold->emit(aHoldEvent);
            }
        }
    }
    myLastEventsTime = aCurrTime;
}

void StWindowImpl::postKeyDown(StEvent& theEvent) {
    theEvent.Type         = stEvent_KeyDown;
    theEvent.Key.Duration = 0.0;
    theEvent.Key.Progress = 0.0;
    myKeysState.keyDown(theEvent.Key.VKey, theEvent.Key.Time);
    theEvent.Key.Flags = ST_VF_NONE;
    if(myKeysState.isKeyDown(ST_VK_SHIFT)) {
        theEvent.Key.Flags = StVirtFlags(theEvent.Key.Flags | ST_VF_SHIFT);
    }
    if(myKeysState.isKeyDown(ST_VK_CONTROL)) {
        theEvent.Key.Flags = StVirtFlags(theEvent.Key.Flags | ST_VF_CONTROL);
    }

    if(myEventsThreaded) {
        myEventsBuffer.append(theEvent);
    } else {
        signals.onKeyDown->emit(theEvent.Key);
    }
}

void StWindowImpl::postKeyUp(StEvent& theEvent) {
    double aKeyTime = 0.0;
    if(!myKeysState.isKeyDown(theEvent.Key.VKey, aKeyTime)) {
        return; // should never happen
    }
    myKeysState.keyUp(theEvent.Key.VKey, theEvent.Key.Time);

    theEvent.Key.Duration = theEvent.Key.Time - aKeyTime;
    theEvent.Key.Flags = ST_VF_NONE;
    if(myKeysState.isKeyDown(ST_VK_SHIFT)) {
        theEvent.Key.Flags = StVirtFlags(theEvent.Key.Flags | ST_VF_SHIFT);
    }
    if(myKeysState.isKeyDown(ST_VK_CONTROL)) {
        theEvent.Key.Flags = StVirtFlags(theEvent.Key.Flags | ST_VF_CONTROL);
    }

    if(myEventsThreaded) {
        theEvent.Type         = stEvent_KeyUp; // hold event will be reconstructed by swapEventsBuffers()
        theEvent.Key.Progress = 0.0;
        myEventsBuffer.append(theEvent);
    } else {
        theEvent.Key.Progress = stMin(theEvent.Key.Time - myLastEventsTime, theEvent.Key.Duration);
        if(theEvent.Key.Progress > 1.e-7) {
            theEvent.Type = stEvent_KeyHold;
            signals.onKeyHold->emit(theEvent.Key);
        }
        theEvent.Type         = stEvent_KeyUp;
        theEvent.Key.Progress = 0.0;
        signals.onKeyUp->emit(theEvent.Key);
    }
}

void StWindowImpl::post(StEvent& theEvent) {
    switch(theEvent.Type) {
        case stEvent_KeyDown: postKeyDown(theEvent);           break;
        case stEvent_KeyUp:   postKeyUp  (theEvent);           break;
        default:              myEventsBuffer.append(theEvent); break;
    }
}
