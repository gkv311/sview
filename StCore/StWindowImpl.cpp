/**
 * StCore, window system independent C++ toolkit for writing OpenGL applications.
 * Copyright © 2007-2017 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#include "StWindowImpl.h"

#include <StStrings/StLogger.h>
#include <StSys/StSys.h>
#include <StThreads/StProcess.h>
#include <StThreads/StThread.h>
#include <StGL/StGLContext.h>

#ifdef __APPLE__
    #include <sys/sysctl.h>
#elif defined(__ANDROID__)
    #include <StCore/StAndroidGlue.h>
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
}

// shared counter for fullscreen windows to detect inactive state
StAtomic<int32_t> StWindowImpl::myFullScreenWinNb(0);

StWindowImpl::StWindowImpl(const StHandle<StResourceManager>& theResMgr,
                           const StNativeWin_t                theParentWindow)
: myResMgr(theResMgr),
  myParentWin(theParentWindow),
  myWindowTitle(WINDOW_TITLE_DEFAULT),
  myInitState(STWIN_INITNOTSTART),
  myToEnableStereoHW(false),
  myHasOrientSensor(false),
  myIsPoorOrient(false),
  myToTrackOrient(false),
  myToHideStatusBar(true),
  myToHideNavBar(true),
  myToSwapEyesHW(false),
  myMousePt(0.5, 0.5),
  myIsPreciseCursor(true),
  myNbTouchesMax(0),
  myRectNorm(128, 512, 128, 512),
  myRectFull(128, 512, 128, 512),
  myRectNormPrev(0, 1, 0, 1),
  myMonMasterFull(-1),
  mySyncCounter(0),
  myWinOnMonitorId(0),
  myWinMonScaleId(0),
  myTiledCfg(TiledCfg_Separate),
  myForcedAspect(-1.0),
#ifdef _WIN32
  myRegisterTouchWindow(NULL),
  myUnregisterTouchWindow(NULL),
  myGetTouchInputInfo(NULL),
  myCloseTouchInputHandle(NULL),
  myTmpTouches(NULL),
  myNbTmpTouches(0),
  myEventInitWin(false),
  myEventInitGl(false),
  myEventQuit(false),
  myEventCursorShow(false),
  myEventCursorHide(false),
#elif (defined(__APPLE__))
  mySleepAssert(0),
#endif
  myToResetDevice(false),
  myIsUpdated(false),
  myIsActive(false),
  myIsPaused(false),
  myBlockSleep(BlockSleep_OFF),
  myIsSystemLocked(false),
  myIsDispChanged(false),
  myAlignDL(0),
  myAlignDR(0),
  myAlignDT(0),
  myAlignDB(0),
  myLastEventsTime(0.0),
  myEventsThreaded(false),
  myIsMouseMoved(false) {
    stMemZero(&attribs, sizeof(attribs));
    stMemZero(&signals, sizeof(signals));
    myStEvent   .Type = stEvent_None;
    myStEvent2  .Type = stEvent_None;
    myStEventAux.Type = stEvent_None;
    myScrollAcc.reset();
    attribs.IsNoDecor      = false;
    attribs.IsStereoOutput = false;
    attribs.IsGlStereo     = false;
    attribs.IsGlDebug      = false;
    attribs.GlDepthSize    = 16;
    attribs.GlStencilSize  = 0;
#if defined(__ANDROID__)
    attribs.IsFullScreen   = true;
#else
    attribs.IsFullScreen   = false;
#endif
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

    myTouches.Type = stEvent_TouchCancel;
    myTouches.Time = 0.0;
    myTouches.clearTouches();

    myTap0Touch = StTouch::Empty();
    myTap1Touch.Type = stEvent_TouchCancel;
    myTap1Touch.Time = 0.0;
    myTap1Touch.clearTouches();

    myMonSlave.idMaster = 0;
    myMonSlave.idSlave  = 1; // second by default
    myMonSlave.xAdd = 1;
    myMonSlave.xSub = 0;
    myMonSlave.yAdd = 1;
    myMonSlave.ySub = 0;

#ifdef _WIN32
    myEventsThreaded = true; // events loop is always performed in dedicated thread

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
    myMsgMonitors.init();
#endif

    myMonitors.init();
#ifdef __APPLE__
    // register callback for display configuration changes
    // alternatively we can add method applicationDidChangeScreenParameters to application delegate
    CGDisplayRegisterReconfigurationCallback(stDisplayChangeCallBack, this);
    myMonitors.registerUpdater(true);
#endif

    myEventsTimer.initUpTime();
}

void StWindowImpl::StSyncTimer::initUpTime() {
#if defined(_WIN32)
    // Spin waiting for a change in system time (should take up to 15 ms on modern systems).
    // Alternatively timeGetTime() might be used instead which handle time in 1 ms precision
    // when used in combination with timeBeginPeriod(1).
    const uint64_t anUptime0 = GetTickCount64();
    uint64_t anUptime1 = 0;
    do {
        anUptime1 = GetTickCount64();
        fillCounter(myCounterStart);
    } while(anUptime0 == anUptime1);
    myTimeInMicroSec = anUptime1 * 1000.0; // convert to microseconds
#elif defined(__APPLE__)
    const uint64_t anUpTimeNano = machUptimeInNanoseconds();
    myTimeInMicroSec = double(anUpTimeNano / 1000); // convert to microseconds
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
    const uint64_t anUptime0 = GetTickCount64();
    uint64_t anUptime1 = 0;
    double aTimerValue = 0.0;
    do {
        anUptime1   = GetTickCount64();
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
    // restore timer adjustments
    TIMECAPS aTimeCaps = {0, 0};
    if(timeGetDevCaps(&aTimeCaps, sizeof(aTimeCaps)) == TIMERR_NOERROR) {
        timeEndPeriod(aTimeCaps.wPeriodMin);
    } else {
        timeEndPeriod(1);
    }
    stMemFree(myTmpTouches);
    myTmpTouches = NULL;
#endif

#ifdef __APPLE__
    myMonitors.registerUpdater(false);
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
    myEventQuit.set();
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
#elif defined(__ANDROID__)
    if(myParentWin != NULL) {
        myParentWin->signals.onInputEvent -= stSlot(this, &StWindowImpl::onAndroidInput);
        myParentWin->signals.onAppCmd     -= stSlot(this, &StWindowImpl::onAndroidCommand);
    }
#endif

#if defined(__linux__) && !defined(__ANDROID__)
    // window will no more receive Xlib events - unregister global updater
    myMonitors.registerUpdater(false);
#endif

    // turn off display sleep blocking
    const bool toBlockSleepSystem  = attribs.ToBlockSleepSystem;
    const bool toBlockSleepDisplay = attribs.ToBlockSleepDisplay;
    attribs.ToBlockSleepSystem  = false;
    attribs.ToBlockSleepDisplay = false;
    updateBlockSleep();
    attribs.ToBlockSleepSystem  = toBlockSleepSystem;
    attribs.ToBlockSleepDisplay = toBlockSleepDisplay;

    //myParentWin = (StNativeWin_t )NULL;

    if(attribs.IsFullScreen) {
        myFullScreenWinNb.decrement();
    }
#if !defined(__ANDROID__)
    attribs.IsFullScreen = false; // just hack to return window position after closing
#endif
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

#if !defined(__APPLE__)
void StWindowImpl::setTitle(const StString& theTitle) {
    myWindowTitle = theTitle;
#ifdef _WIN32
    myIsUpdated = true;
#elif defined(__ANDROID__)
    //
#elif defined(__linux__)
    if(myMaster.hWindow != 0) {
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
            case StWinAttr_GlStencilSize:
                anIter[1] = (StWinAttr )attribs.GlStencilSize;
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
            case StWinAttr_ExclusiveFullScreen:
                anIter[1] = (StWinAttr)attribs.IsExclusiveFullScr;
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
            case StWinAttr_GlStencilSize:
                attribs.GlStencilSize = (int8_t )anIter[1];
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
                    switch(attribs.Split) {
                        case StWinSlave_splitHorizontal:
                        case StWinSlave_splitVertical:
                        case StWinSlave_splitVertHdmi720:
                        case StWinSlave_splitVertHdmi1080: {
                            if(myRectFullInit.width() != 0) {
                                // restore rectangle
                                myRectFull = myRectFullInit;
                            }
                            break;
                        }
                        case StWinSlave_splitOff: {
                            // remember rectangle
                            myRectFullInit = myRectFull;
                            break;
                        }
                    }

                    switch((StWinSplit )anIter[1]) {
                        case StWinSlave_splitHorizontal: {
                            myTiledCfg = TiledCfg_MasterSlaveX;
                            if(myForcedAspect > 0.0) {
                                const int aNewSizeY = int(double(myRectFull.width()) / myForcedAspect);
                                const int aDY       = aNewSizeY - myRectFull.height();
                                if(aDY <= 0) {
                                    myRectFull.bottom() += aDY;
                                } else {
                                    const int aNewSizeX = int(double(myRectFull.height()) * myForcedAspect);
                                    const int aDX       = aNewSizeX - myRectFull.width();
                                    myRectFull.right() += aDX;
                                }
                            } else {
                                myRectFull.right() -= myRectFull.width() / 2;
                            }
                            break;
                        }
                        case StWinSlave_splitVertical: {
                            myTiledCfg = TiledCfg_MasterSlaveY;
                            myRectFull.bottom() -= myRectFull.height() / 2;
                            break;
                        }
                        case StWinSlave_splitVertHdmi720: {
                            myTiledCfg = TiledCfg_VertHdmi720;
                            myRectFull.bottom() -= (720 + 30);
                            break;
                        }
                        case StWinSlave_splitVertHdmi1080: {
                            myTiledCfg = TiledCfg_VertHdmi1080;
                            myRectFull.bottom() -= (1080 + 45);
                            break;
                        }
                        default: {
                            myTiledCfg = TiledCfg_Separate;
                            break;
                        }
                    }
                    myStEventAux.Size.init(getEventTime(), myRectFull.width(), myRectFull.height(), myForcedAspect);
                    signals.onResize->emit(myStEventAux.Size);
                }
                attribs.Split = (StWinSplit )anIter[1];
                break;
            case StWinAttr_ToAlignEven:
                attribs.ToAlignEven = (anIter[1] == 1);
                break;
            case StWinAttr_ExclusiveFullScreen:
                attribs.IsExclusiveFullScr = (anIter[1] == 1);
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
    if(attribs.ToBlockSleepDisplay
    && !myIsSystemLocked) {
        // prevent display sleep - call this periodically
        EXECUTION_STATE aState = ES_CONTINUOUS | ES_DISPLAY_REQUIRED | ES_SYSTEM_REQUIRED
                               | ES_AWAYMODE_REQUIRED;
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
        EXECUTION_STATE aState = ES_CONTINUOUS | ES_SYSTEM_REQUIRED
                               | ES_AWAYMODE_REQUIRED;
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
#elif defined(__ANDROID__)
    int aFlags = 0;
    bool toWakeLock = false;
    if(attribs.ToBlockSleepDisplay) {
        aFlags |= AWINDOW_FLAG_KEEP_SCREEN_ON;
        myBlockSleep = BlockSleep_DISPLAY;
    } else if(attribs.ToBlockSleepSystem) {
        myBlockSleep = BlockSleep_SYSTEM;
        toWakeLock = true;
    } else {
        myBlockSleep = BlockSleep_OFF;
    }
    if(myParentWin != NULL) {
        myParentWin->setWindowFlags(aFlags);
        myParentWin->setWakeLock(myWindowTitle, toWakeLock);
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

/*#if defined(__ANDROID__)
    myIsActive = (myMaster.eglSurface != EGL_NO_SURFACE);
#else*/
    if(myIsSystemLocked) {
        myIsActive = false;
        return;
    } else if(attribs.IsFullScreen) {
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
//#endif
}

#if !defined(__APPLE__)
void StWindowImpl::show(const int theWinNum) {
    if((theWinNum == ST_WIN_MASTER || theWinNum == ST_WIN_ALL)
     && attribs.IsHidden) {
    #ifdef _WIN32
        if(myMaster.hWindow != NULL) {
            ::ShowWindow(myMaster.hWindow, SW_SHOW);
            ::SetForegroundWindow(myMaster.hWindow); // make sure Master window on top and has input focus
        } else if(myMaster.hWindowGl != NULL) {
            ::ShowWindow(myMaster.hWindowGl, SW_SHOW);
        }
    #elif defined(__ANDROID__)
        ///
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
    #elif defined(__ANDROID__)
        ///
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
    #elif defined(__ANDROID__)
        ///
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
    #elif defined(__ANDROID__)
        ///
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
        myEventCursorShow.set();
    } else {
        myEventCursorHide.set();
    }
#elif defined(__ANDROID__)
    ///
#elif defined(__linux__)
    if(toShow) {
        XUndefineCursor(myMaster.getDisplay(), myMaster.hWindowGl);
    } else {
        myMaster.setupNoCursor();
    }
#endif
    attribs.ToHideCursor = !toShow;
}

#if defined(__ANDROID__)
void StWindowImpl::setPlacement(const StRectI_t& ,
                                const bool       ) {
    //
}
#else
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
#if defined(_WIN32)
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
#endif // !__ANDROID__
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
        case TiledCfg_VertHdmi720:
        case TiledCfg_VertHdmi1080: {
            // should not be used in this context
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
        case TiledCfg_VertHdmi720:
        case TiledCfg_VertHdmi1080:
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
        case TiledCfg_VertHdmi720: {
            if(theWinId == ST_WIN_MASTER) {
                aRect.y()      += 720 + 30;
            } else if(theWinId == ST_WIN_ALL) {
                aRect.height() += 720 + 30;
            }
            convertRectToBacking(aRect, ST_WIN_MASTER);
            return aRect;
        }
        case TiledCfg_VertHdmi1080: {
            if(theWinId == ST_WIN_MASTER) {
                aRect.y()      += 1080 + 45;
            } else if(theWinId == ST_WIN_ALL) {
                aRect.height() += 1080 + 45;
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
            if (attribs.IsFullScreen
            && !attribs.IsExclusiveFullScr
            &&  myTiledCfg == TiledCfg_Separate
            &&  myParentWin == NULL) {
                // workaround for non-exclusive fullscreen mode
                aCursor.ptScreenPos.y += 2;
            }

            return StPointD_t((double(aCursor.ptScreenPos.x) - double(aWinRect.left())) / double(aWinRect.width()),
                              (double(aCursor.ptScreenPos.y) - double(aWinRect.top()))  / double(aWinRect.height()));
        }
    }
#elif defined(__APPLE__)
    CGEventRef anEvent = CGEventCreate(NULL);
    CGPoint aCursor = CGEventGetLocation(anEvent);
    CFRelease(anEvent);
    return StPointD_t((aCursor.x - double(aWinRect.left())) / double(aWinRect.width()),
                      (aCursor.y - double(aWinRect.top()))  / double(aWinRect.height()));
#elif defined(__ANDROID__)
    (void )aWinRect;
    if(myMaster.hWindowGl != NULL) {
        return myMousePt;
    }
#elif defined(__linux__)
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
bool StWindowImpl::stglMakeCurrent(const int winNum){
    switch(winNum) {
          case ST_WIN_MASTER: {
            if(myMaster.glMakeCurrent()) {
                if(!myGlContext.isNull()) {
                    myGlContext->stglResetErrors();
                }
                return true;
            }
            return false;
        } case ST_WIN_SLAVE: {
            if(myTiledCfg == TiledCfg_Separate) {
                return mySlave.glMakeCurrent();
            } else {
                return myMaster.glMakeCurrent();
            }
        }
    }
    return false;
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

/**
 * Gesture threshold.
 */
struct StGestThreshold {
    float FromIdle;   //!< threshold to start gesture
    float Update;     //!< threshold to update this gesture
    float BreakOther; //!< threshold to start gesture breaking another
};

namespace {
    static const StGestThreshold THE_THRESHOLD_TAP = {
        20.0f,
        20.0f,
        20.0f
    };
    static const StGestThreshold THE_THRESHOLD_PAN = {
         4.0f,
         1.0f,
        20.0f
    };
    static const StGestThreshold THE_THRESHOLD_ZROT = {
        float( 2.0 * M_PI / 180.0),
        float( 2.0 * M_PI / 180.0),
        float(20.0 * M_PI / 180.0)
    };
    static const StGestThreshold THE_THRESHOLD_ZOOM = {
         6.0f,
         1.0f,
        20.0f
    };
    static const StGestThreshold THE_THRESHOLD_SWIPE = {
        150.0f,
        150.0f,
        150.0f
    };
}

/**
 * Auxiliary method to start new gesture.
 */
inline bool startGesture(const StTouchEvent&    theTouches,
                         const StEventType      theGesture,
                         const StGestThreshold& theThreshold,
                         const float            theValue) {
    if(theTouches.Type == stEvent_TouchCancel) {
        return theValue >= theThreshold.FromIdle;
    } else if(theTouches.Type == theGesture) {
        return theValue >= theThreshold.Update;
    }
    return theValue >= theThreshold.BreakOther;
}

void StWindowImpl::doTouch(const StTouchEvent& theTouches) {
    const double aTime = theTouches.Time;
    signals.onTouch->emit(theTouches);

    // scale factor for conversion into dp (dencity-independent pixels) units
    const StRectI_t  aWinRect = attribs.IsFullScreen ? myRectFull : myRectNorm;
    const StMonitor& aMon     = myMonitors[aWinRect.center()];
    StGLVec2 aScale((float )aWinRect.width() / aMon.getScale(), (float )aWinRect.height() / aMon.getScale());

    switch(theTouches.Type) {
        case stEvent_TouchDown: {
            myTouches.Type = stEvent_TouchCancel;
            myTouches.clearTouches();
            for(int aTouchIter = 0; aTouchIter < theTouches.NbTouches; ++aTouchIter) {
                myTouches.addTouch(theTouches.Touches[aTouchIter]);
            }
            if(myNbTouchesMax == 0) {
                myTouches.Time = aTime;
                myTap0Touch = theTouches.Touches[0];
            }
            myNbTouchesMax = stMax(myNbTouchesMax, theTouches.NbTouches);

            myStEvent2.Type = stEvent_GestureCancel;
            myStEvent2.Gesture.clearGesture();
            myStEvent2.Gesture.Time = aTime;
            signals.onGesture->emit(myStEvent2.Gesture);
            break;
        }
        case stEvent_TouchUp: {
            StTouchEvent aCopy = myTouches;
            myTouches.Type = stEvent_TouchCancel;
            myTouches.clearTouches();
            for(int aTouchIter = 0; aTouchIter < aCopy.NbTouches; ++aTouchIter) {
                if(theTouches.findTouchById(aCopy.Touches[aTouchIter].Id).isDefined()) {
                    myTouches.addTouch(aCopy.Touches[aTouchIter]);
                }
            }

            myStEvent2.Type = stEvent_GestureCancel;
            myStEvent2.Gesture.clearGesture();
            myStEvent2.Gesture.Time = aTime;
            signals.onGesture->emit(myStEvent2.Gesture);

            if(theTouches.NbTouches == 0) {
                if(myNbTouchesMax == 1) {
                    if(myTap1Touch.Type != stEvent_TouchCancel
                    && (aTime - myTap1Touch.Time) < 0.5) {
                        StGLVec2 aDelta(myTap1Touch.Touches[0].PointX - aCopy.Touches[0].PointX,
                                        myTap1Touch.Touches[0].PointY - aCopy.Touches[0].PointY);
                        aDelta *= aScale;
                        if(aDelta.modulus() <= THE_THRESHOLD_TAP.FromIdle) {
                            myStEvent2.Type = stEvent_Gesture1DoubleTap;
                            myStEvent2.Gesture.clearGesture();
                            myStEvent2.Gesture.Time = aTime;
                            myStEvent2.Gesture.Point1X = aCopy.Touches[0].PointX;
                            myStEvent2.Gesture.Point1Y = aCopy.Touches[0].PointY;
                            signals.onGesture->emit(myStEvent2.Gesture);
                        }
                        myTap1Touch.Type = stEvent_TouchCancel;
                    } else if(aTime - aCopy.Time < 0.5) {
                        myTap1Touch = aCopy;
                        myTap1Touch.Type = stEvent_TouchUp;
                        myTap1Touch.Time = aTime;
                        StGLVec2 aDelta(myTap0Touch.PointX - aCopy.Touches[0].PointX,
                                        myTap0Touch.PointY - aCopy.Touches[0].PointY);
                        aDelta *= aScale;
                        if(aDelta.modulus() <= THE_THRESHOLD_TAP.FromIdle) {
                            myStEvent2.Type = stEvent_Gesture1Tap;
                            myStEvent2.Gesture.clearGesture();
                            myStEvent2.Gesture.Time = aTime;
                            myStEvent2.Gesture.Point1X = aCopy.Touches[0].PointX;
                            myStEvent2.Gesture.Point1Y = aCopy.Touches[0].PointY;
                            signals.onGesture->emit(myStEvent2.Gesture);
                        }
                    }  else {
                        myTap1Touch.Type = stEvent_TouchCancel;
                    }
                } else {
                    myTap1Touch.Type = stEvent_TouchCancel;
                }
                myNbTouchesMax = 0;
            }
            break;
        }
        case stEvent_TouchCancel: {
            myTouches.Type = stEvent_TouchCancel;
            myTouches.clearTouches();

            myNbTouchesMax = 0;
            myStEvent2.Type = stEvent_GestureCancel;
            myStEvent2.Gesture.clearGesture();
            myStEvent2.Gesture.Time = aTime;
            signals.onGesture->emit(myStEvent2.Gesture);
            break;
        }
        case stEvent_TouchMove: {
            break;
        }
        default:
            break;
    }

    if(theTouches.Type != stEvent_TouchMove) {
        return;
    }

    bool toUpdate = false;
    if(myTouches .NbTouches == 1
    && theTouches.NbTouches == 1) {
        StTouch aTFrom = myTouches.Touches[0];
        StTouch aTTo = theTouches.findTouchById(aTFrom.Id);
        if(!aTTo.isDefined()) {
            return;
        }
        myTouches.Touches[0] = aTTo;
    } else if(myTouches .NbTouches == 2
           && theTouches.NbTouches == 2) {
        StTouch aTFrom[2] = {
            myTouches.Touches[0],
            myTouches.Touches[1]
        };
        StTouch aTTo[2] = {
            theTouches.findTouchById(aTFrom[0].Id),
            theTouches.findTouchById(aTFrom[1].Id)
        };

        if(!aTTo[0].isDefined()
        || !aTTo[1].isDefined()) {
            return;
        }

        StGLVec2 aFrom[2] = {
            StGLVec2(aTFrom[0].PointX, aTFrom[0].PointY) * aScale,
            StGLVec2(aTFrom[1].PointX, aTFrom[1].PointY) * aScale
        };
        StGLVec2 aTo[2] = {
            StGLVec2(aTTo[0].PointX, aTTo[0].PointY) * aScale,
            StGLVec2(aTTo[1].PointX, aTTo[1].PointY) * aScale
        };

        float aDistFrom  = (aFrom[1] - aFrom[0]).modulus();
        float aDistTo    = (aTo  [1] - aTo  [0]).modulus();
        float aDistDelta = aDistTo - aDistFrom;
        float aRotAngle  = 0.0f;
        if(std::abs(aDistFrom) > 50.0f) {
            const float A1 = aFrom[0].y() - aFrom[1].y();
            const float B1 = aFrom[1].x() - aFrom[0].x();

            const float A2 =   aTo[0].y() - aTo[1].y();
            const float B2 =   aTo[1].x() - aTo[0].x();

            const float aDenom = A1 * A2 + B1 * B2;
            if(aDenom >= 0.00001f) {
                const float aNumerator = A1 * B2 - A2 * B1;
                aRotAngle = std::atan(aNumerator / aDenom);
            }
        }

        StGLVec2 aCenterFrom  = (aFrom[0] + aFrom[1]) * 0.5;
        StGLVec2 aCenterTo    = (  aTo[0] +   aTo[1]) * 0.5;
        StGLVec2 aCenterDelta = aCenterTo - aCenterFrom;

        // take minimum delta between fingers
        StGLVec2 aDeltas[2] = {
            aTo[0] - aFrom[0],
            aTo[1] - aFrom[1]
        };
        StGLVec2 aMoveDelta = aDeltas[0];
        if(aDeltas[1].squareModulus()   < aMoveDelta.squareModulus()) {
            aMoveDelta = aDeltas[1];
        }
        if(aCenterDelta.squareModulus() < aMoveDelta.squareModulus()) {
            aMoveDelta = aCenterDelta;
        }

        myStEvent.Type = stEvent_None;
        myStEvent.Gesture.clearGesture();
        myStEvent.Gesture.Time = aTime;
        myStEvent.Gesture.OnScreen = aTTo[0].OnScreen;
        myStEvent.Gesture.Point1X  = (aTFrom[0].PointX + aTFrom[1].PointX) * 0.5f;
        myStEvent.Gesture.Point1Y  = (aTFrom[0].PointY + aTFrom[1].PointY) * 0.5f;
        myStEvent.Gesture.Point2X  = (  aTTo[0].PointX +   aTTo[1].PointX) * 0.5f;
        myStEvent.Gesture.Point2Y  = (  aTTo[0].PointY +   aTTo[1].PointY) * 0.5f;

        if(startGesture(myTouches, stEvent_Gesture2Rotate,
                        THE_THRESHOLD_ZROT, std::abs(aRotAngle))) {
            if(myTouches.Type != stEvent_GestureCancel
            && myTouches.Type != stEvent_Gesture2Rotate) {
                myStEvent2.Type = stEvent_GestureCancel;
                myStEvent2.Gesture.clearGesture();
                myStEvent2.Gesture.Time = aTime;
                signals.onGesture->emit(myStEvent2.Gesture);
            }
            myTouches.Type = stEvent_Gesture2Rotate;
            myStEvent.Type = stEvent_Gesture2Rotate;
            myStEvent.Gesture.Value = aRotAngle;
            signals.onGesture->emit(myStEvent.Gesture);
            toUpdate = true;
        } else if(//std::abs(aRotAngle) < 5.0f * M_PI / 180.0f &&
                  startGesture(myTouches, stEvent_Gesture2Pinch,
                               THE_THRESHOLD_ZOOM, std::abs(aDistDelta))) {
            if(myTouches.Type != stEvent_GestureCancel
            && myTouches.Type != stEvent_Gesture2Pinch) {
                myStEvent2.Type = stEvent_GestureCancel;
                myStEvent2.Gesture.clearGesture();
                myStEvent2.Gesture.Time = aTime;
                signals.onGesture->emit(myStEvent2.Gesture);
            }
            myTouches.Type = stEvent_Gesture2Pinch;
            myStEvent.Type = stEvent_Gesture2Pinch;
            myStEvent.Gesture.Value = aDistDelta;
            signals.onGesture->emit(myStEvent.Gesture);
            toUpdate = true;
        } else if(startGesture(myTouches, stEvent_Gesture2Move,
                               THE_THRESHOLD_PAN, aMoveDelta.modulus())) {
            if(myTouches.Type != stEvent_GestureCancel
            && myTouches.Type != stEvent_Gesture2Move) {
                myStEvent2.Type = stEvent_GestureCancel;
                myStEvent2.Gesture.clearGesture();
                myStEvent2.Gesture.Time = aTime;
                signals.onGesture->emit(myStEvent2.Gesture);
            }
            myTouches.Type = stEvent_Gesture2Move;
            myStEvent.Type = stEvent_Gesture2Move;
            signals.onGesture->emit(myStEvent.Gesture);
            toUpdate = true;
        }

        if(toUpdate) {
            myTouches.Touches[0] = aTTo[0];
            myTouches.Touches[1] = aTTo[1];
        }
    } else if(myTouches .NbTouches == 3
           && theTouches.NbTouches == 3) {
        StTouch aTFrom[3] = {
            myTouches.Touches[0],
            myTouches.Touches[1],
            myTouches.Touches[2]
        };
        StTouch aTTo[3] = {
            theTouches.findTouchById(aTFrom[0].Id),
            theTouches.findTouchById(aTFrom[1].Id),
            theTouches.findTouchById(aTFrom[2].Id)
        };

        if(!aTTo[0].isDefined()
        || !aTTo[1].isDefined()
        || !aTTo[2].isDefined()) {
            return;
        }

        StGLVec2 aFrom[3] = {
            StGLVec2(aTFrom[0].PointX, aTFrom[0].PointY) * aScale,
            StGLVec2(aTFrom[1].PointX, aTFrom[1].PointY) * aScale,
            StGLVec2(aTFrom[2].PointX, aTFrom[2].PointY) * aScale
        };
        StGLVec2 aTo[3] = {
            StGLVec2(aTTo[0].PointX, aTTo[0].PointY) * aScale,
            StGLVec2(aTTo[1].PointX, aTTo[1].PointY) * aScale,
            StGLVec2(aTTo[2].PointX, aTTo[2].PointY) * aScale
        };
        StGLVec2 aCenterFrom  = (aFrom[0] + aFrom[1] + aFrom[2]) * (1.0f / 3.0f);
        StGLVec2 aCenterTo    = (  aTo[0] +   aTo[1] +   aTo[2]) * (1.0f / 3.0f);
        StGLVec2 aCenterDelta = aCenterTo - aCenterFrom;
        if(aCenterDelta.y() < 50.0f
        && std::abs(aCenterDelta.x()) >= THE_THRESHOLD_SWIPE.FromIdle) {
            myStEvent.Type = stEvent_Navigate;
            myStEvent.Navigate.Time = aTime;
            myStEvent.Navigate.Target = aCenterDelta.x() < 0.0
                                      ? stNavigate_Backward
                                      : stNavigate_Forward;
            signals.onNavigate->emit(myStEvent.Navigate);
            toUpdate = true;
        } else if(aCenterDelta.x() < 50.0f
               && std::abs(aCenterDelta.y()) >= THE_THRESHOLD_SWIPE.FromIdle) {
            myStEvent.Type = stEvent_Navigate;
            myStEvent.Navigate.Time = aTime;
            myStEvent.Navigate.Target = aCenterDelta.y() < 0.0
                                      ? stNavigate_Top
                                      : stNavigate_Bottom;
            signals.onNavigate->emit(myStEvent.Navigate);
            toUpdate = true;
        }

        if(toUpdate) {
            myTouches.Touches[0] = aTTo[0];
            myTouches.Touches[1] = aTTo[1];
            myTouches.Touches[2] = aTTo[2];
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
            case stEvent_Pause:
                signals.onPause->emit(anEvent.Pause);
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
            case stEvent_MouseUp: {
                signals.onMouseUp->emit(anEvent.Button);
                if(anEvent.Button.Button != ST_MOUSE_LEFT) {
                    myDoubleClickTimer.stop();
                    break;
                }

                // check double click
                const StRectI_t& aWinRect = attribs.IsFullScreen ? myRectFull : myRectNorm;
                const StPointD_t aNewPnt(anEvent.Button.PointX, anEvent.Button.PointY);
                const StPointD_t aDeltaPx = (myDoubleClickPnt - aNewPnt) * StPointD_t(aWinRect.width(), aWinRect.height());
                if(myDoubleClickTimer.isOn()
                && myDoubleClickTimer.getElapsedTime() < 0.4
                && aDeltaPx.cwiseAbs().maxComp() < 3) {
                    myStEvent2.Type = stEvent_Gesture1DoubleTap;
                    myStEvent2.Gesture.clearGesture();
                    myStEvent2.Gesture.Time = anEvent.Button.Time;
                    myStEvent2.Gesture.Point1X = (float )anEvent.Button.PointX;
                    myStEvent2.Gesture.Point1Y = (float )anEvent.Button.PointY;
                    myDoubleClickTimer.stop();
                    signals.onGesture->emit(myStEvent2.Gesture);
                } else {
                    myDoubleClickTimer.restart();
                }
                myDoubleClickPnt = aNewPnt;
                break;
            }
            case stEvent_TouchDown:
            case stEvent_TouchUp:
            case stEvent_TouchMove:
            case stEvent_TouchCancel:
                doTouch(anEvent.Touch);
                break;
            case stEvent_Scroll:
                signals.onScroll->emit(anEvent.Scroll);
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
            if(myKeysState.isKeyDown(ST_VK_MENU)) {
                aHoldEvent.Flags = StVirtFlags(aHoldEvent.Flags | ST_VF_MENU);
            }
            if(myKeysState.isKeyDown(ST_VK_COMMAND)) {
                aHoldEvent.Flags = StVirtFlags(aHoldEvent.Flags | ST_VF_COMMAND);
            }
            if(myKeysState.isKeyDown(ST_VK_FUNCTION)) {
                aHoldEvent.Flags = StVirtFlags(aHoldEvent.Flags | ST_VF_FUNCTION);
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
    if(myKeysState.isKeyDown(ST_VK_MENU)) {
        theEvent.Key.Flags = StVirtFlags(theEvent.Key.Flags | ST_VF_MENU);
    }
    if(myKeysState.isKeyDown(ST_VK_COMMAND)) {
        theEvent.Key.Flags = StVirtFlags(theEvent.Key.Flags | ST_VF_COMMAND);
    }
#ifdef __APPLE__
    if(myKeysState.isKeyDown(ST_VK_FUNCTION)
    && theEvent.Key.VKey != ST_VK_DELETE
    && theEvent.Key.VKey != ST_VK_HOME
    && theEvent.Key.VKey != ST_VK_END
    && theEvent.Key.VKey != ST_VK_PRIOR
    && theEvent.Key.VKey != ST_VK_NEXT) {
#else
    if(myKeysState.isKeyDown(ST_VK_FUNCTION)) {
#endif
        theEvent.Key.Flags = StVirtFlags(theEvent.Key.Flags | ST_VF_FUNCTION);
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
    if(myKeysState.isKeyDown(ST_VK_MENU)) {
        theEvent.Key.Flags = StVirtFlags(theEvent.Key.Flags | ST_VF_MENU);
    }
    if(myKeysState.isKeyDown(ST_VK_COMMAND)) {
        theEvent.Key.Flags = StVirtFlags(theEvent.Key.Flags | ST_VF_COMMAND);
    }
#ifdef __APPLE__
    if(myKeysState.isKeyDown(ST_VK_FUNCTION)
    && theEvent.Key.VKey != ST_VK_DELETE
    && theEvent.Key.VKey != ST_VK_HOME
    && theEvent.Key.VKey != ST_VK_END
    && theEvent.Key.VKey != ST_VK_PRIOR
    && theEvent.Key.VKey != ST_VK_NEXT) {
#else
    if(myKeysState.isKeyDown(ST_VK_FUNCTION)) {
#endif
        theEvent.Key.Flags = StVirtFlags(theEvent.Key.Flags | ST_VF_FUNCTION);
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
