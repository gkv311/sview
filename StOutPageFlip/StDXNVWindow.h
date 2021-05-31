/**
 * StOutPageFlip, class providing stereoscopic output for Shutter Glasses displays using StCore toolkit.
 * Copyright © 2009-2015 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#ifdef _WIN32
#ifndef __StDXNVWindow_h_
#define __StDXNVWindow_h_

#include "StDXManager.h"
#include "StDXNVSurface.h"

#include <StCore/StMonitor.h>
#include <StCore/StWindow.h>
#include <StThreads/StCondition.h>
#include <StThreads/StMutex.h>
#include <StThreads/StThread.h>

class StOutPageFlip;

/**
 * Direct3D window.
 */
class StDXNVWindow {

        public:

    /**
     * Default constructor.
     */
    StDXNVWindow(const StHandle<StMsgQueue>& theMsgQueue,
                 const size_t     theFboSizeX,
                 const size_t     theFboSizeY,
                 const StMonitor& theMonitor,
                 StOutPageFlip*   theStWin);

    /**
     * Destructor.
     */
    ~StDXNVWindow();

    /**
     * Post message to hide the cursor.
     */
    void showCursor(bool theToShow);

    /**
     * Specify if WGL <-> D3D interoperation extension should be used.
     */
    bool toUseWglDxInterop() const {
        return myHasWglDx;
    }

    /**
     * Specify if WGL <-> D3D interoperation extension should be used.
     * Should be set before initialization.
     */
    void setWglDxInterop(const bool theHasWglDx) {
        myHasWglDx = theHasWglDx;
    }

    /**
     * Specify if D3D context should be created and managed within window message thread.
     */
    bool isThreadedDx() const {
        return myIsThreadedDx;
    }

    /**
     * Specify if D3D context should be created and managed within window message thread.
     * Should be set before initialization.
     */
    void setThreadedDx(const bool theIsThreadedDx) {
        myIsThreadedDx = theIsThreadedDx;
    }

    const StHandle<StDXManager>& getD3dManager() const {
        return myDxManager;
    }

    const StHandle<StDXNVSurface>& getD3dSurface() const {
        return myDxSurface;
    }

    size_t getD3dSizeX() const {
        return myMonitor.getVRect().width();
    }

    size_t getD3dSizeY() const {
        return myMonitor.getVRect().height();
    }

    /**
     * @return FBO width.
     */
    size_t getFboSizeX() const {
        return myFboSizeX;
    }

    /**
     * @return FBO height.
     */
    size_t getFboSizeY() const {
        return myFboSizeY;
    }

    /**
     * Wait until Direct3D window not initialized in its dedicated thread.
     */
    void waitReady() {
        myEventReady.wait();
    }

    /**
     * Request Direct3D window to show.
     */
    void show() {
        dxShow();
        myEventHide.reset();
        myEventShow.set();
    }

    /**
     * Request Direct3D window to hide.
     */
    void hide() {
        dxHide();
        myEventShow.reset();
        myEventHide.set();
    }

    /**
     * Request Direct3D window to update stereo buffer.
     */
    void update() {
        if(myIsThreadedDx) {
            myEventUpdate.set();
            return;
        }
        dxUpdate();
    }

    bool isInUpdate() {
        return myEventUpdate.check();
    }

    /**
     * Lock mutex for buffers access.
     */
    void lockLRBuffers() {
        myMutex.lock();
    }

    /**
     * Unlock buffers mutex.
     */
    void unlockLRBuffers() {
        myMutex.unlock();
    }

    /**
     * You should lock access using lockLRBuffers() before!
     * @return BGRA buffer for left view.
     */
    unsigned char* getBuffLeft() {
        return myBufferL;
    }

    /**
     * You should lock access using lockLRBuffers() before!
     * @return BGRA buffer for right view.
     */
    unsigned char* getBuffRight() {
        return myBufferR;
    }

    /**
     * Allocate buffers.
     * You should lock access using lockLRBuffers() before!
     */
    bool allocateBuffers();

    /**
     * Release buffers.
     * You should lock access using lockLRBuffers() before!
     */
    void releaseBuffers();

    /**
     * Main rendering loop for Direct3D window.
     */
    void dxLoop();

    /**
     * Create D3D manager.
     */
    bool dxInitManager();

    /**
     * Release D3D manager.
     */
    void dxReleaseManager();

    /**
     * Set termination event.
     */
    void quit() {
        myEventQuit.set();
    }

        private:

    bool initWinAPIWindow();

    static LRESULT CALLBACK wndProcWrapper(HWND   theWnd,
                                           UINT   theMsg,
                                           WPARAM theParamW,
                                           LPARAM theParamL);

    LRESULT wndProcFunction(HWND   theWnd,
                            UINT   theMsg,
                            WPARAM theParamW,
                            LPARAM theParamL);

    void updateMouseBtn(const int btnId, bool newState);

    /**
     * Unregister window class.
     */
    bool unregisterClass(StStringUtfWide& theName);

    /**
     * Process show event.
     */
    void dxShow();

    /**
     * Process hide event.
     */
    void dxHide();

    /**
     * Process update event within D3D message loop thread.
     */
    void dxUpdate();

    /**
     * Peek new window messages within D3D message loop thread.
     */
    void peekMessages();

        private:

    StHandle<StMsgQueue>    myMsgQueue;     //!< messages queue
    unsigned char*          myBufferL;
    unsigned char*          myBufferR;
    size_t                  myFboSizeX;
    size_t                  myFboSizeY;
    bool                    myHasWglDx;
    bool                    myIsThreadedDx; //!< option to create D3D in window message loop

    HWND                    myWinD3d;
    StStringUtfWide         myWinClass;

    StHandle<StDXManager>   myDxManager;
    StHandle<StDXNVSurface> myDxSurface;

    StMonitor               myMonitor;
    StOutPageFlip*          myStWin;        //!< link to StWindow

    StMutex                 myMutex;        //!< lock for communication between StWindow and D3D threads
    bool                    myShowState;    //!< D3D window show state
    MSG                     myWinMsg;       //!< message for windows' message loop
    StEvent                 myKeyEvent;     //!< key event
    BYTE                    myKeysMap[256]; //!< pressed keys map
    wchar_t                 myCharBuff[4];  //!< buffer for key event
    bool myMouseState[ST_MOUSE_MAX_ID + 1];

    StCondition             myEventReady;
    StCondition             myEventQuit;    //!< quit message thread event
    StCondition             myEventShow;
    StCondition             myEventHide;
    StCondition             myEventUpdate;
    StCondition             myEventCursorShow; //!< event to show cursor
    StCondition             myEventCursorHide; //!< event to hide cursor
    bool                    myToHideCursor;

};

#endif //__StDXNVWindow_h_
#endif //_WIN32
