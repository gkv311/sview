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

#ifndef __StWindowImpl_h_
#define __StWindowImpl_h_

#include <StCore/StWindow.h>
#include <StCore/StSearchMonitors.h>

#include "StWinHandles.h"
#include "StEventsBuffer.h"

#if (defined(__APPLE__))
    #include <StCocoa/StCocoaCoords.h>
    #include <IOKit/pwr_mgt/IOPMLib.h>
#endif

class NSOpenGLContext;
class StThread;

/**
 * This class represents implementation of
 * all main routines for GLwindow managment.
 */
class StWindowImpl {

        public: //! @name main interface

    ST_LOCAL StWindowImpl(const StNativeWin_t theParentWindow);
    ST_LOCAL ~StWindowImpl();
    ST_LOCAL void close();
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
    ST_LOCAL int getDragNDropFile(const int theId, StString& theFile);
    ST_LOCAL bool create();
    ST_LOCAL void stglSwap(const int& theWinId);
    ST_LOCAL void stglMakeCurrent(const int& theWinId);
    ST_LOCAL StGLBoxPx stglViewport(const int& theWinId) const;
    ST_LOCAL void processEvents(StMessage_t* theMessages);
    ST_LOCAL stBool_t appendMessage(const StMessage_t& theMessage);
    ST_LOCAL const StSearchMonitors& getMonitors() const {
        return myMonitors;
    }

        public: //! @name additional

    ST_LOCAL void updateChildRect();
#ifdef _WIN32
    ST_LOCAL bool wndRegisterClass(const StStringUtfWide& theClassName);
    ST_LOCAL bool wndCreateWindows(); // called from non-main thread
    ST_LOCAL LRESULT stWndProc(HWND , UINT , WPARAM , LPARAM );
#elif (defined(__APPLE__))
    ST_LOCAL void doCreateWindows(NSOpenGLContext* theGLContextMaster,
                                  NSOpenGLContext* theGLContextSlave);
#endif

        public:

    ST_LOCAL void updateMonitors();
    ST_LOCAL void updateWindowPos();
    ST_LOCAL void updateActiveState();
    ST_LOCAL void updateBlockSleep();
#if (defined(__linux__) || defined(__linux))
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
     * Tiles configuration (multiple viewports within the same window).
     */
    enum TiledCfg {
        TiledCfg_Separate,     //!< dedicated windows - default
        TiledCfg_MasterSlaveX, //!< Master at left   / Slave at right
        TiledCfg_SlaveMasterX, //!< Master at right  / Slave at left
        TiledCfg_MasterSlaveY, //!< Master at top    / Slave at bottom
        TiledCfg_SlaveMasterY, //!< Master at bottom / Slave at top
    };

    ST_LOCAL void getTiledWinRect(StRectI_t& theRect) const;
    ST_LOCAL void correctTiledCursor(int& theLeft, int& theTop) const;

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

    StWinHandles       myMaster;          //!< master window
    StWinHandles       mySlave;           //!< slave  window (optional)
    StNativeWin_t      myParentWin;       //!< parent window (optional, for embedding)

    StString           myWindowTitle;     //!< window caption
    int                myInitState;       //!< initialization error code

    StPointD_t         myMousePt;         //!< mouse coordinates to track activity
    StRectI_t          myRectNorm;        //!< master window coordinates in normal     state
    StRectI_t          myRectFull;        //!< master window coordinates in fullscreen state
    StRectI_t          myRectNormPrev;    //!< window rectangle to track changes

    StSearchMonitors   myMonitors;        //!< available monitors
    int                myMonMasterFull;   //!< monitor for fullscreen (-1 - use monitor where window currently placed)
    StSlaveWindowCfg_t myMonSlave;        //!< slave window options
    size_t             mySyncCounter;
    int                myWinOnMonitorId;  //!< monitor id where window is placed
    TiledCfg           myTiledCfg;        //!< tiles configuration (multiple viewports within the same window)

#ifdef _WIN32
    POINT              myPointTest;       //!< temporary point object to verify cached window position
    StHandle<StThread> myMsgThread;       //!< dedicated thread for window message loop
    StCondition        myEventInitWin;    //!< special event waited from createWindows() thread
    StCondition        myEventInitGl;
    HANDLE             myEventQuit;       //!< quit message thread event
    HANDLE             myEventCursorShow;
    HANDLE             myEventCursorHide;
    MSG                myEvent;           //!< message for windows' message loop
    bool               myIsVistaPlus;     //!< system is Vista+
#elif (defined(__APPLE__))
    StCocoaCoords      myCocoaCoords;
    IOPMAssertionLevel mySleepAssert;     //!< prevent system going to sleep
#elif (defined(__linux__) || defined(__linux))
    XEvent             myXEvent;
#endif

    StMutex            myDndMutex;        //!< access mustex for Drag&Drop file list
    size_t             myDndCount;        //!< files' count
    StString*          myDndList;         //!< Drag&Drop list

    StMessageList      myMessageList;     //!< callback list
    bool               myIsUpdated;       //!< helper flag on window movements updates
    bool               myIsActive;        //!< window visible state
    BlockSleep         myBlockSleep;      //!< indicates that display sleep was blocked or not
    volatile bool      myIsDispChanged;   //!< monitors reconfiguration event

    /**
     * Window attributes structure for internal use.
     * Notice that some options couln't be changed after window was created!
     */
    struct {
        bool       IsNoDecor;          //!< to decorate master window or not (will be ignored in case of embedded and fullscreen)
        bool       IsStereoOutput;     //!< indicate stereoscopic output on / off (used for interconnection between modules)
        bool       IsGlStereo;         //!< request OpenGL hardware accelerated QuadBuffer
        int8_t     GlDepthSize;        //!< OpenGL Depth Buffer size
        bool       IsFullScreen;       //!< to show in fullscreen mode
        bool       IsHidden;           //!< to hide the window
        bool       IsSlaveHidden;      //!< to hide the only slave window
        bool       ToHideCursor;       //!< to hide cursor
        bool       ToBlockSleepSystem; //!< prevent system  going to sleep (display could be turned off)
        bool       ToBlockSleepDisplay;//!< prevent display going to sleep
        bool       AreGlobalMediaKeys; //!< register system hot-key to capture multimedia even without window focus
        StWinSlave Slave;              //!< slave configuration
        int8_t     SlaveMonId;         //!< on which monitor show slave window (1 by default)
    } attribs;

    struct {
        StSignal<void (const StClickEvent& )>* onMouseUp;
        StSignal<void (const StClickEvent& )>* onMouseDown;
    } signals;

    StTimer        myEventsTimer;
    StEventsBuffer myEventsBuffer; //!< window events double buffer
    StEvent myStEvent;
};

#endif //__StWindowImpl_h_
