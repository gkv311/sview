/**
 * Copyright © 2009-2013 Kirill Gavrilov <kirill@sview.ru>
 *
 * StOutPageFlip library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * StOutPageFlip library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.
 * If not, see <http://www.gnu.org/licenses/>.
 */

#ifdef _WIN32
#ifndef __StDXNVWindow_h_
#define __StDXNVWindow_h_

#include "StDXManager.h"
#include "StDXNVSurface.h"

#include <StCore/StMonitor.h>
#include <StCore/StWindow.h>
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
        WaitForSingleObject(hEventReady, INFINITE);
    }

    /**
     * Request Direct3D window to show.
     */
    void show() {
        ResetEvent(hEventHide);
        SetEvent(hEventShow);
    }

    /**
     * Request Direct3D window to hide.
     */
    void hide() {
        ResetEvent(hEventShow);
        SetEvent(hEventHide);
    }

    /**
     * Request Direct3D window to update stereo buffer.
     */
    void update() {
        SetEvent(hEventUpdate);
    }

    bool isInUpdate() {
        return (WaitForSingleObject(hEventUpdate, (DWORD )0) != WAIT_TIMEOUT);
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
     * Set termination event.
     */
    void quit() {
        SetEvent(hEventQuit);
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

        private:

    StHandle<StMsgQueue>    myMsgQueue; //!< messages queue
    unsigned char*          myBufferL;
    unsigned char*          myBufferR;
    size_t                  myFboSizeX;
    size_t                  myFboSizeY;

    HWND                    myWinD3d;
    StStringUtfWide         myWinClass;

    StHandle<StDXManager>   myDxManager;
    StHandle<StDXNVSurface> myDxSurface;

    StMonitor               myMonitor;
    StOutPageFlip*          myStWin;

    StMutex                 myMutex;

    HANDLE                  hEventReady;
    HANDLE                  hEventQuit; // quit message thread event
    HANDLE                  hEventShow;
    HANDLE                  hEventHide;
    HANDLE                  hEventUpdate;

    bool myMouseState[ST_MOUSE_MAX_ID + 1];

};

#endif //__StDXNVWindow_h_
#endif //_WIN32
