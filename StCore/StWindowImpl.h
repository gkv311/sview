/**
 * StCore, window system independent C++ toolkit for writing OpenGL applications.
 * Copyright © 2007-2017 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#ifndef __StWindowImpl_h_
#define __StWindowImpl_h_

#include <StCore/StWindow.h>
#include <StCore/StSearchMonitors.h>
#include <StCore/StKeysState.h>

#include "StWinHandles.h"
#include "StEventsBuffer.h"

#if defined(__APPLE__)
    #include <StCocoa/StCocoaCoords.h>
    #include <IOKit/pwr_mgt/IOPMLib.h>
    #include <mach/mach_time.h>
#elif !defined(_WIN32)
    #include <sys/sysinfo.h>
#endif

#if defined(__ANDROID__)
    struct AInputEvent;
#endif

#ifdef __OBJC__
    @class NSOpenGLContext;
#else
    class NSOpenGLContext;
#endif

class StGLContext;
class StThread;

/**
 * This class represents implementation of
 * all main routines for GLwindow managment.
 */
class StWindowImpl {

        public: //! @name main interface

    ST_LOCAL StWindowImpl(const StHandle<StResourceManager>& theResMgr,
                          const StNativeWin_t                theParentWindow);
    ST_LOCAL ~StWindowImpl();
    ST_LOCAL void close();
    ST_LOCAL const StString& getTitle() const { return myWindowTitle; }
    ST_LOCAL void setTitle(const StString& theTitle);
    ST_LOCAL bool hasDepthBuffer() const { return attribs.GlDepthSize != 0; }
    ST_LOCAL void getAttributes(StWinAttr* theAttributes) const;
    ST_LOCAL void setAttributes(const StWinAttr* theAttributes);
    ST_LOCAL bool isActive() const { return myIsActive; }
    ST_LOCAL bool isStereoOutput() { return attribs.IsStereoOutput; }
    ST_LOCAL void setStereoOutput(bool theStereoState) { attribs.IsStereoOutput = theStereoState; }
    ST_LOCAL void show(const int );
    ST_LOCAL void hide(const int );
    ST_LOCAL void showCursor(bool toShow);
    ST_LOCAL bool isFullScreen() { return attribs.IsFullScreen; }
    ST_LOCAL void setFullScreen(bool theFullscreen);
    ST_LOCAL StRectI_t getPlacement() const { return attribs.IsFullScreen ? myRectFull : myRectNorm; }
    ST_LOCAL void setPlacement(const StRectI_t& theRect,
                               const bool       theMoveToScreen);
    ST_LOCAL StPointD_t getMousePos();
    ST_LOCAL bool isPreciseCursor() const { return myIsPreciseCursor; }
    ST_LOCAL bool create();
    ST_LOCAL void stglSwap(const int& theWinId);
    ST_LOCAL bool stglMakeCurrent(const int theWinId);
    ST_LOCAL StGLBoxPx stglViewport(const int& theWinId) const;
    ST_LOCAL void processEvents();
    ST_LOCAL void post(StEvent& theEvent);
    ST_LOCAL GLfloat getScaleFactor() const {
        return myMonitors[myWinOnMonitorId].getScale();
    }
    ST_LOCAL const StSearchMonitors& getMonitors() const {
        return myMonitors;
    }

    ST_LOCAL bool isParentOnScreen() const;

        public: //! @name clipboard

    ST_LOCAL bool toClipboard(const StString& theText);
    ST_LOCAL bool fromClipboard(StString& theText);

#if defined(__linux__)
    StString myTextToCopy;
#endif

        public: //! @name additional

    ST_LOCAL void updateChildRect();
#ifdef _WIN32
    ST_LOCAL bool wndCreateWindows(); // called from non-main thread
    ST_LOCAL LRESULT stWndProc(HWND , UINT , WPARAM , LPARAM );
#elif defined(__APPLE__)
    ST_DISABLE_DEPRECATION_WARNINGS
    ST_LOCAL void doCreateWindows(NSOpenGLContext* theGLContextMaster,
                                  NSOpenGLContext* theGLContextSlave);
    ST_ENABLE_DEPRECATION_WARNINGS
#endif

        public:

    ST_LOCAL void updateMonitors();
    ST_LOCAL void updateWindowPos();
    ST_LOCAL void updateActiveState();
    ST_LOCAL void updateBlockSleep();
    ST_LOCAL void doTouch(const StTouchEvent& theEvent);
#if defined(__ANDROID__)
    ST_LOCAL void onAndroidInput(const AInputEvent* theEvent, bool& theIsProcessed);
    ST_LOCAL void onAndroidCommand(int32_t theCommand);
    ST_LOCAL bool onAndroidInitWindow();
#elif defined(__linux__)
    ST_LOCAL void parseXDNDClientMsg();
    ST_LOCAL void parseXDNDSelectionMsg();

    ST_LOCAL static Bool stXWaitMapped(Display* theDisplay,
                                       XEvent*  theEvent,
                                       char*    theArg);
#endif

    /**
     * Swap events read/write buffers
     * and pop cached events from read buffer.
     */
    ST_LOCAL void swapEventsBuffers();

    /**
     * @return uptime in seconds for event
     */
    ST_LOCAL double getEventTime() const;

    /**
     * @return uptime in seconds (convert from 32-bit integer)
     */
    ST_LOCAL double getEventTime(const uint32_t theTime) const;

    /**
     * Setup common fields of event structure (Type, Flags)
     * and post key down event and perform post-processing.
     */
    ST_LOCAL void postKeyDown(StEvent& theEvent);

    /**
     * Setup common fields of event structure (Type, Flags)
     * and post key up event and perform post-processing.
     */
    ST_LOCAL void postKeyUp(StEvent& theEvent);

    /**
     * Tiles configuration (multiple viewports within the same window).
     */
    enum TiledCfg {
        TiledCfg_Separate,     //!< dedicated windows - default
        TiledCfg_MasterSlaveX, //!< Master at left   / Slave at right
        TiledCfg_SlaveMasterX, //!< Master at right  / Slave at left
        TiledCfg_MasterSlaveY, //!< Master at top    / Slave at bottom
        TiledCfg_SlaveMasterY, //!< Master at bottom / Slave at top
        TiledCfg_VertHdmi720,
        TiledCfg_VertHdmi1080,
    };

    ST_LOCAL void getTiledWinRect(StRectI_t& theRect) const;
    ST_LOCAL void correctTiledCursor(int& theLeft, int& theTop) const;

    ST_LOCAL void convertRectToBacking(StGLBoxPx& theRect,
                                       const int  theWinId) const;

    ST_LOCAL void updateSlaveConfig() {
        myMonSlave.idSlave = int(attribs.SlaveMonId);
        if(attribs.Slave == StWinSlave_slaveFlipX) {
            myMonSlave.xAdd = 0; myMonSlave.xSub = 1;
            myMonSlave.yAdd = 1; myMonSlave.ySub = 0;
        } else if(attribs.Slave == StWinSlave_slaveFlipY) {
            myMonSlave.xAdd = 1; myMonSlave.xSub = 0;
            myMonSlave.yAdd = 0; myMonSlave.ySub = 1;
        } else {
            myMonSlave.xAdd = 1; myMonSlave.xSub = 0;
            myMonSlave.yAdd = 1; myMonSlave.ySub = 0;
        }
    }

    ST_LOCAL int getMasterLeft() const {
        return myMonitors[myMonSlave.idMaster].getVRect().left();
    }

    ST_LOCAL int getMasterTop() const {
        return myMonitors[myMonSlave.idMaster].getVRect().top();
    }

    /**
     * @return true if slave window should be displayed on independent monitor.
     */
    ST_LOCAL bool isSlaveIndependent() const {
        return attribs.Slave == StWinSlave_slaveSync
            || attribs.Slave == StWinSlave_slaveFlipX
            || attribs.Slave == StWinSlave_slaveFlipY;
    }

    ST_LOCAL int getSlaveLeft() const {
        if(!isSlaveIndependent()) {
            return myMonitors[getPlacement().center()].getVRect().left();
        } else if(attribs.IsFullScreen) {
            return myMonitors[myMonSlave.idSlave].getVRect().left();
        } else {
            const StMonitor& aMonMaster = myMonitors[getPlacement().center()]; // detect from current location
            return myMonSlave.xAdd * (myMonitors[myMonSlave.idSlave].getVRect().left()  + myRectNorm.left()  - aMonMaster.getVRect().left())
                 + myMonSlave.xSub * (myMonitors[myMonSlave.idSlave].getVRect().right() - myRectNorm.right() + aMonMaster.getVRect().left());
        }
    }

    ST_LOCAL int getSlaveWidth() const {
        if(attribs.Slave == StWinSlave_slaveHTop2Px) {
            return 2;
        } else if(attribs.Slave == StWinSlave_slaveHLineTop
               || attribs.Slave == StWinSlave_slaveHLineBottom) {
            return myMonitors[getPlacement().center()].getVRect().width();
        } else if(attribs.IsFullScreen) {
            return myMonitors[myMonSlave.idSlave].getVRect().width();
        } else {
            return myRectNorm.width();
        }
    }

    ST_LOCAL int getSlaveTop() const {
        if(attribs.Slave == StWinSlave_slaveHLineBottom) {
            return myMonitors[getPlacement().center()].getVRect().bottom() - 1;
        } else if(attribs.Slave == StWinSlave_slaveHLineTop
               || attribs.Slave == StWinSlave_slaveHTop2Px) {
            return myMonitors[getPlacement().center()].getVRect().top();
        } else if(attribs.IsFullScreen) {
            return myMonitors[myMonSlave.idSlave].getVRect().top();
        } else {
            const StMonitor& aMonMaster = myMonitors[getPlacement().center()]; // detect from current location
            return myMonSlave.yAdd * (myMonitors[myMonSlave.idSlave].getVRect().top()    + myRectNorm.top()    - aMonMaster.getVRect().top())
                 + myMonSlave.ySub * (myMonitors[myMonSlave.idSlave].getVRect().bottom() - myRectNorm.bottom() + aMonMaster.getVRect().top());
        }
    }

    ST_LOCAL int getSlaveHeight() const {
        if(attribs.Slave == StWinSlave_slaveHLineBottom
        || attribs.Slave == StWinSlave_slaveHTop2Px) {
            return 1;
        } else if(attribs.Slave == StWinSlave_slaveHLineTop) {
            return 10;
        } else if(attribs.IsFullScreen) {
            return myMonitors[myMonSlave.idSlave].getVRect().height();
        } else {
            return myRectNorm.height();
        }
    }

        public: //! @name fields

    enum BlockSleep {
        BlockSleep_OFF,     //!< do not block sleeping
        BlockSleep_SYSTEM,  //!< block system to sleep but not display
        BlockSleep_DISPLAY, //!< block display to sleep
    };

    static StAtomic<int32_t> myFullScreenWinNb; //!< shared counter for fullscreen windows to detect inactive state

    StHandle<StResourceManager> myResMgr; //!< file resources manager
    StHandle<StGLContext> myGlContext;
    StWinHandles       myMaster;          //!< master window
    StWinHandles       mySlave;           //!< slave  window (optional)
    StNativeWin_t      myParentWin;       //!< parent window (optional, for embedding)

    StString           myWindowTitle;     //!< window caption
    int                myInitState;       //!< initialization error code
    StString           myStatistics;      //!< extra statistics
    bool               myToEnableStereoHW;//!< hardware stereo enable state
    StQuaternion<double> myQuaternion;    //!< device orientation
    bool               myHasOrientSensor; //!< flag indicating that device has orientation sensors
    bool               myIsPoorOrient;    //!< flag indicating that available orientation sensor provides imprecise values
    bool               myToTrackOrient;   //!< track device orientation
    bool               myToHideStatusBar; //!< hide system-provided status bar
    bool               myToHideNavBar;    //!< hide system-provided navigation bar
    bool               myToSwapEyesHW;    //!< flag to swap LR views on external event

    StPointD_t         myMousePt;         //!< mouse coordinates to track activity
    bool               myIsPreciseCursor; //!< flag indicating that last mouse cursor position was updated by precise input device
    StTouchEvent       myTouches;         //!< current state of touch screen
    StTouchEvent       myTap1Touch;       //!< previous tap touch
    StTouch            myTap0Touch;       //!< started tap
    int                myNbTouchesMax;    //!< maximum touches within current sequence
    StRectI_t          myRectNorm;        //!< master window coordinates in normal     state
    StRectI_t          myRectFull;        //!< master window coordinates in fullscreen state
    StRectI_t          myRectFullInit;    //!< fullscreen rectangle before applying tiling
    StRectI_t          myRectNormPrev;    //!< window rectangle to track changes
    StMarginsI         myDecMargins;      //!< decoration margins

    StSearchMonitors   myMonitors;        //!< available monitors
    int                myMonMasterFull;   //!< monitor for fullscreen (-1 - use monitor where window currently placed)
    StSlaveWindowCfg_t myMonSlave;        //!< slave window options
    size_t             mySyncCounter;
    int                myWinOnMonitorId;  //!< monitor id where window is placed
    int                myWinMonScaleId;   //!< monitor id from which scale factor is applied
    TiledCfg           myTiledCfg;        //!< tiles configuration (multiple viewports within the same window)
    double             myForcedAspect;    //!< forced window aspect ratio (negative number if not forced)

#ifdef _WIN32
    // available since Win7 (not in Vista!)
    typedef BOOL (WINAPI *RegisterTouchWindow_t)(HWND hwnd, ULONG ulFlags);
    typedef BOOL (WINAPI *UnregisterTouchWindow_t)(HWND hwnd);
    typedef BOOL (WINAPI *GetTouchInputInfo_t)(HTOUCHINPUT hTouchInput,
                                               UINT cInputs,
                                               PTOUCHINPUT pInputs,
                                               int         cbSize);
    typedef BOOL (WINAPI *CloseTouchInputHandle_t)(HTOUCHINPUT hTouchInput);
    RegisterTouchWindow_t   myRegisterTouchWindow;
    UnregisterTouchWindow_t myUnregisterTouchWindow;
    GetTouchInputInfo_t     myGetTouchInputInfo;
    CloseTouchInputHandle_t myCloseTouchInputHandle;
    TOUCHINPUT*             myTmpTouches;
    int                     myNbTmpTouches;

    StSearchMonitors   myMsgMonitors;     //!< available monitors, accessed from message thread
    POINT              myPointTest;       //!< temporary point object to verify cached window position
    StHandle<StThread> myMsgThread;       //!< dedicated thread for window message loop
    StCondition        myEventInitWin;    //!< special event waited from createWindows() thread
    StCondition        myEventInitGl;
    StCondition        myEventQuit;       //!< quit message thread event
    StCondition        myEventCursorShow;
    StCondition        myEventCursorHide;
    MSG                myEvent;           //!< message for windows' message loop
#elif defined(__APPLE__)
    StCocoaCoords      myCocoaCoords;
    IOPMAssertionLevel mySleepAssert;     //!< prevent system going to sleep
#elif defined(__ANDROID__)
    //
#else
    XEvent             myXEvent;
    char               myXInputBuff[32];
#endif

    bool               myToResetDevice;   //!< indicate device lost state
    bool               myIsUpdated;       //!< helper flag on window movements updates
    bool               myIsActive;        //!< window visible state
    bool               myIsPaused;        //!< window is in background
    BlockSleep         myBlockSleep;      //!< indicates that display sleep was blocked or not
    bool               myIsSystemLocked;   //!< flag indicating that user session is in locked state
    volatile bool      myIsDispChanged;   //!< monitors reconfiguration event

    /**
     * Window attributes structure for internal use.
     * Notice that some options could not be changed after window was created!
     */
    struct {
        bool       IsNoDecor;          //!< to decorate master window or not (will be ignored in case of embedded and fullscreen)
        bool       IsStereoOutput;     //!< indicate stereoscopic output on / off (used for interconnection between modules)
        bool       IsGlStereo;         //!< request OpenGL hardware accelerated QuadBuffer
        bool       IsGlDebug;          //!< request OpenGL debug context
        int8_t     GlDepthSize;        //!< OpenGL Depth Buffer size
        int8_t     GlStencilSize;      //!< OpenGL Stencil Buffer size
        bool       IsFullScreen;       //!< to show in fullscreen mode
        bool       IsExclusiveFullScr; //!< use exclusive fullscreen mode (improve performance, prevent other applications)
        bool       IsHidden;           //!< to hide the window
        bool       IsSlaveHidden;      //!< to hide the only slave window
        bool       ToHideCursor;       //!< to hide cursor
        bool       ToBlockSleepSystem; //!< prevent system  going to sleep (display could be turned off)
        bool       ToBlockSleepDisplay;//!< prevent display going to sleep
        bool       AreGlobalMediaKeys; //!< register system hot-key to capture multimedia even without window focus
        StWinSlave Slave;              //!< slave configuration
        int8_t     SlaveMonId;         //!< on which monitor show slave window (1 by default)
        StWinSplit Split;              //!< split window configuration
        bool       ToAlignEven;        //!< align window position to even numbers
    } attribs;

    struct {
        StSignal<void (const StCloseEvent&  )>* onClose;
        StSignal<void (const StPauseEvent&  )>* onPause;
        StSignal<void (const StSizeEvent&   )>* onResize;
        StSignal<void (const StSizeEvent&   )>* onAnotherMonitor;
        StSignal<void (const StKeyEvent&    )>* onKeyUp;
        StSignal<void (const StKeyEvent&    )>* onKeyDown;
        StSignal<void (const StKeyEvent&    )>* onKeyHold;
        StSignal<void (const StClickEvent&  )>* onMouseUp;
        StSignal<void (const StClickEvent&  )>* onMouseDown;
        StSignal<void (const StTouchEvent&  )>* onTouch;
        StSignal<void (const StGestureEvent&)>* onGesture;
        StSignal<void (const StScrollEvent& )>* onScroll;
        StSignal<void (const StDNDropEvent& )>* onFileDrop;
        StSignal<void (const StNavigEvent&  )>* onNavigate;
        StSignal<void (const StActionEvent& )>* onAction;
    } signals;

    class StSyncTimer : public StTimer {

            public:

        /**
         * Constructor.
         */
        StSyncTimer()
        : StTimer(),
          myLastSyncMicroSec(0.0),
          mySyncMicroSec(0.0f) {
        #if defined(__APPLE__)
            (void )::mach_timebase_info(&myTimebaseInfo);
        #endif
        }

        /**
         * Initialize timer from current UpTime using system API.
         */
        ST_LOCAL void initUpTime();

        /**
         * @return UpTime computed with this timer
         */
        ST_LOCAL double getUpTime() const {
            return getElapsedTime() + double(mySyncMicroSec) * 0.000001;
        }

        /**
         * @return true each 2 minutes
         */
        ST_LOCAL bool isResyncNeeded() const {
            return (getElapsedTimeInMicroSec() - myLastSyncMicroSec) > 120000000.0;
        }

        /**
         * Compute correction for high-performance timer value
         * relative to real system UpTime.
         */
        ST_LOCAL void resyncUpTime();

    #if defined(__APPLE__)
        /**
         * Replacement for deprecated AbsoluteToNanoseconds(UpTime()).
         */
        ST_LOCAL uint64_t machUptimeInNanoseconds() const {
            //const Nanoseconds anUpTimeNano = ::AbsoluteToNanoseconds(::UpTime());
            //sreturn *(uint64_t* )&anUpTimeNano;
            // Convert to nanoseconds.
            // We hope that the multiplication doesn't overflow; the price you pay for working in fixed point.
            const uint64_t anElapsed     = ::mach_absolute_time();
            const uint64_t anElapsedNano = anElapsed * myTimebaseInfo.numer / myTimebaseInfo.denom;
            return anElapsedNano;
        }
    #endif

        /**
         * Retrieve UpTime using system API.
         */
        ST_LOCAL double getUpTimeFromSystem() const {
        #ifdef _WIN32
            const uint64_t anUptime = GetTickCount64();
            return double(anUptime) * 0.001;
        #elif defined(__APPLE__)
            // use function from CoreServices to retrieve system uptime
            const uint64_t anUpTimeNano = machUptimeInNanoseconds();
            return double(anUpTimeNano / 1000) * 0.000001;
        #else
            // read system uptime (in seconds)
            struct sysinfo aSysInfo;
            ::sysinfo(&aSysInfo);
            return double(aSysInfo.uptime);
        #endif
        }

    #if defined(__APPLE__)
        mach_timebase_info_data_t myTimebaseInfo;
    #endif
        double             myLastSyncMicroSec; //!< timestamp of last synchronization
        float              mySyncMicroSec;     //!< should be replaced by double with atomic accessors

    };

    StKeysState    myKeysState;        //!< cached keyboard state
    StSyncTimer    myEventsTimer;
    StEventsBuffer myEventsBuffer;     //!< window events double buffer
    StEvent        myStEvent;          //!< temporary event object (to be used in message loop thread)
    StEvent        myStEvent2;         //!< temporary event object (to be used in message loop thread)
    StEvent        myStEventAux;       //!< extra temporary event object (to be used in StWindow creation thread)
    StScrollEvent  myScrollAcc;        //!< extra temporary event object accumulating mouse scroll events
    StTimer        myDoubleClickTimer; //!< timer for detecting double click
    StPointD_t     myDoubleClickPnt;   //!< double click point
    int            myAlignDL;          //!< extra window shift applied for alignment (left)
    int            myAlignDR;          //!< extra window shift applied for alignment (right)
    int            myAlignDT;          //!< extra window shift applied for alignment (top)
    int            myAlignDB;          //!< extra window shift applied for alignment (bottom)
    double         myLastEventsTime;   //!< time when processEvents() was last called
    bool           myEventsThreaded;
    bool           myIsMouseMoved;

};

#endif // __StWindowImpl_h_
