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

#ifndef __StWindowImpl_h_
#define __StWindowImpl_h_

#include <StCore/StWindowInterface.h> // interface declaration
#include "StSearchMonitors.h"
#include "StWinHandles.h"             // os-specific handles class for windows
#include "StMouseClickQueue.h"

#if (defined(__APPLE__))
    #include <StCocoa/StCocoaCoords.h>
#endif

class NSOpenGLContext;
class StThread;

/**
 * This class represents implementation of
 * all main routines for GLwindow managment.
 */
class ST_LOCAL StWindowImpl : public StWindowInterface {

        public: //! @name StWindowInterface interface implementation

    virtual StWindowInterface* getLibImpl() { return this; }
    StWindowImpl();
    virtual ~StWindowImpl();
    virtual void close();
    virtual void setTitle(const StString& theTitle);
    virtual void getAttributes(StWinAttributes_t* theAttributes);
    virtual void setAttributes(const StWinAttributes_t* theAttributes);
    virtual bool isActive() const { return myIsActive; }
    virtual bool isStereoOutput() { return myWinAttribs.isStereoOutput; }
    virtual void setStereoOutput(bool theStereoState) { myWinAttribs.isStereoOutput = theStereoState; }
    virtual void show(const int& );
    virtual void hide(const int& );
    virtual void showCursor(bool toShow);
    virtual bool isFullScreen() { return myWinAttribs.isFullScreen; }
    virtual void setFullScreen(bool theFullscreen);
    virtual StRectI_t getPlacement();
    virtual void setPlacement(const StRectI_t& theRect);
    virtual StPointD_t getMousePos();
    virtual int getMouseDown(StPointD_t* thePoint);
    virtual int getMouseUp(StPointD_t* thePoint);
    virtual int getDragNDropFile(const int& id, stUtf8_t* outFile, const size_t& buffSizeBytes);
    virtual bool stglCreate(const StWinAttributes_t* theAttributes, const StNativeWin_t theParentWindow);
    virtual void stglSwap(const int& theWinId);
    virtual void stglMakeCurrent(const int& theWinId);
    virtual double stglGetTargetFps() { return myTargetFps; }
    virtual void stglSetTargetFps(const double& theFps) { myTargetFps = theFps; }
    virtual void callback(StMessage_t* theMessages);
    virtual stBool_t appendMessage(const StMessage_t& theMessage);
    virtual bool getValue(const size_t& theKey, size_t*       theValue);
    virtual void setValue(const size_t& theKey, const size_t& theValue);

        public: //! @name additional

    void updateChildRect();
#if (defined(_WIN32) || defined(__WIN32__))
    bool wndRegisterClass(const StStringUtfWide& theClassName);
    bool wndCreateWindows(); // called from non-main thread
    LRESULT stWndProc(HWND , UINT , WPARAM , LPARAM );
#elif (defined(__APPLE__))
    void doCreateWindows(NSOpenGLContext* theGLContextMaster,
                         NSOpenGLContext* theGLContextSlave);
#endif

        public:

    void updateWindowPos();
    void updateActiveState();
#if (defined(__linux__) || defined(__linux))
    void parseXDNDClientMsg();
    void parseXDNDSelectionMsg();

    static Bool stXWaitMapped(Display* theDisplay,
                              XEvent*  theEvent,
                              char*    theArg);
#endif

    void updateSlaveConfig() {
        myMonSlave.idSlave = int(myWinAttribs.slaveMonId);
        if(myWinAttribs.isSlaveXMirrow) {
            myMonSlave.xAdd = 0; myMonSlave.xSub = 1;
            myMonSlave.yAdd = 1; myMonSlave.ySub = 0;
        } else if(myWinAttribs.isSlaveYMirrow) {
            myMonSlave.xAdd = 1; myMonSlave.xSub = 0;
            myMonSlave.yAdd = 0; myMonSlave.ySub = 1;
        } else {
            myMonSlave.xAdd = 1; myMonSlave.xSub = 0;
            myMonSlave.yAdd = 1; myMonSlave.ySub = 0;
        }
    }

    int getMasterLeft() {
        return myMonitors[myMonSlave.idMaster].getVRect().left();
    }

    int getMasterTop() {
        return myMonitors[myMonSlave.idMaster].getVRect().top();
    }

    /**
     * @return true if slave window should be displayed on independent monitor.
     */
    bool isSlaveIndependent() {
        return !myWinAttribs.isSlaveHLineTop && !myWinAttribs.isSlaveHTop2Px && !myWinAttribs.isSlaveHLineBottom;
    }

    int getSlaveLeft() {
        if(!isSlaveIndependent()) {
            return myMonitors[getPlacement().center()].getVRect().left();
        } else if(myWinAttribs.isFullScreen) {
            return myMonitors[myMonSlave.idSlave].getVRect().left();
        } else {
            StMonitor& aMonMaster = myMonitors[getPlacement().center()]; // detect from current location
            return myMonSlave.xAdd * (myMonitors[myMonSlave.idSlave].getVRect().left()  + myRectNorm.left()  - aMonMaster.getVRect().left())
                 + myMonSlave.xSub * (myMonitors[myMonSlave.idSlave].getVRect().right() - myRectNorm.right() + aMonMaster.getVRect().left());
        }
    }

    int getSlaveWidth() {
        if(myWinAttribs.isSlaveHTop2Px) {
            return 2;
        } else if(myWinAttribs.isSlaveHLineTop || myWinAttribs.isSlaveHLineBottom) {
            return myMonitors[getPlacement().center()].getVRect().width();
        } else if(myWinAttribs.isFullScreen) {
            return myMonitors[myMonSlave.idSlave].getVRect().width();
        } else {
            return myRectNorm.width();
        }
    }

    int getSlaveTop() {
        if(myWinAttribs.isSlaveHLineBottom) {
            return myMonitors[getPlacement().center()].getVRect().bottom() - 1;
        } else if(myWinAttribs.isSlaveHLineTop || myWinAttribs.isSlaveHTop2Px) {
            return myMonitors[getPlacement().center()].getVRect().top();
        } else if(myWinAttribs.isFullScreen) {
            return myMonitors[myMonSlave.idSlave].getVRect().top();
        } else {
            StMonitor& aMonMaster = myMonitors[getPlacement().center()]; // detect from current location
            return myMonSlave.yAdd * (myMonitors[myMonSlave.idSlave].getVRect().top()    + myRectNorm.top()    - aMonMaster.getVRect().top())
                 + myMonSlave.ySub * (myMonitors[myMonSlave.idSlave].getVRect().bottom() - myRectNorm.bottom() + aMonMaster.getVRect().top());
        }
    }

    int getSlaveHeight() {
        if(myWinAttribs.isSlaveHLineBottom || myWinAttribs.isSlaveHTop2Px) {
            return 1;
        } else if(myWinAttribs.isSlaveHLineTop) {
            return 10;
        } else if(myWinAttribs.isFullScreen) {
            return myMonitors[myMonSlave.idSlave].getVRect().height();
        } else {
            return myRectNorm.height();
        }
    }

        public: //! @name fields

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

    size_t             myUserDataMap;     //!< user data
    double             myTargetFps;       //!< user data

#if (defined(_WIN32) || defined(__WIN32__))
    POINT              myPointTest;       //!< temporary point object to verify cached window position
    StHandle<StThread> myMsgThread;       //!< dedicated thread for window message loop
    StEvent            myEventInitWin;    //!< special event waited from createWindows() thread
    StEvent            myEventInitGl;
    HANDLE             myEventQuit;       //!< quit message thread event
    HANDLE             myEventCursorShow;
    HANDLE             myEventCursorHide;
    MSG                myEvent;           //!< message for windows' message loop
#elif (defined(__APPLE__))
    StCocoaCoords      myCocoaCoords;
#elif (defined(__linux__) || defined(__linux))
    bool               myReparentHackX;   //!< hack variable ST_REPARENT_HACK
    XEvent             myXEvent;
#endif

    StMutex            myDndMutex;        //!< access mustex for Drag&Drop file list
    size_t             myDndCount;        //!< files' count
    StString*          myDndList;         //!< Drag&Drop list

    StMouseClickQueue  myMDownQueue;
    StMouseClickQueue  myMUpQueue;
    StMessageList      myMessageList;     //!< callback list
    bool               myIsUpdated;       //!< helper flag on window movements updates
    bool               myIsActive;        //!< window visible state

    StWinAttributes_t  myWinAttribs;

};

#endif //__StWindowImpl_h_
