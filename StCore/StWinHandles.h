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

#ifndef __StWinHandles_h_
#define __StWinHandles_h_

#if (defined(_WIN32) || defined(__WIN32__))
    #include <windows.h>   // header file for Windows
#elif (defined(__APPLE__))
    #include "StCocoaView.h"
    #include "StCocoaWin.h"
#elif (defined(__linux__) || defined(__linux))
    #include <GL/glx.h>
#endif

#include "StXDisplay.h"
#include <StStrings/StString.h>
#include <StCore/StWinErrorCodes.h>
#include <StThreads/StMutex.h>
#include <StThreads/StEvent.h>

#ifndef __APPLE__
/**
 * Wrapper over OpenGL rendering context.
 */
class ST_LOCAL StWinGlrc {

        public:

    /**
     * Create OpenGL Rendering Context for specified Device Context.
     */
#ifdef _WIN32
    StWinGlrc(HDC theDC);
#else
    StWinGlrc(StHandle<StXDisplay>& theDisplay);
#endif

    /**
     * Destructor.
     */
    ~StWinGlrc();

    /**
     * @return true if handle is not NULL.
     */
    bool isValid() const {
        return myRC != NULL;
    }

    /**
     * Activate this OpenGL context within specified Device Context.
     * Device Context should have the one used on construction of this Rendering Context
     * or have the same Pixel Format.
     */
#ifdef _WIN32
    bool makeCurrent(HDC theDC);
#else
    bool makeCurrent(GLXDrawable theDrawable);
#endif

        private:

#ifdef _WIN32
    HGLRC      myRC;      //!< WinAPI Rendering Context handle
#else
    Display*   myDisplay; //!< display connection
    GLXContext myRC;      //!< X-GLX rendering context
#endif
};

// just short typedef for handle
typedef StHandle<StWinGlrc> StWinGlrcH;

#endif

/**
 * This class represent cumulative system-dependent
 * windows' and GL rendering contexts' handles.
 * Class DO NOT represents methods for creating
 * those handles, but give full access for them.
 */
class ST_LOCAL StWinHandles {

        public:

#if (defined(_WIN32) || defined(__WIN32__))
    size_t      threadIdWnd; // id of the thread, in wich window was created
    StEvent     evMsgThread; // special event to check message thread active state
    HWND            hWindow; // WinAPI windows' handles
    HWND          hWindowGl;
    StStringUtfWide   className; // WinAPI classes' names
    StStringUtfWide classNameGl;
    ATOM         myMKeyStop;
    ATOM         myMKeyPlay;
    ATOM         myMKeyPrev;
    ATOM         myMKeyNext;
        private:
    StMutex         stMutex;
    size_t      threadIdOgl; // id of the thread, in wich rendering context was created
    HDC                 hDC; // WinAPI Device Descriptor handle
    StWinGlrcH          hRC; // WinAPI Rendering Context handle
#elif (defined(__APPLE__))
    StCocoaWin*     hWindow;
    StCocoaView*    hViewGl;
#elif (defined(__linux__) || defined(__linux))
    Window          hWindow; // X-window handle
    Window        hWindowGl; // X-window handle for undecorated GL window
    StXDisplayH  stXDisplay; // X-server display connection
    StWinGlrcH          hRC; // X-GLX rendering context

    Pixmap        iconImage; // icon stuff
    Pixmap        iconShape;

    Atom    xDNDRequestType; // X Drag & Drop stuff
    Window    xDNDSrcWindow;
    int         xDNDVersion;
    int     xrandrEventBase;
    bool  isRecXRandrEvents;
#endif

        public:

    /**
     * Creates NULL-handles.
     */
    StWinHandles();

    ~StWinHandles();

    /**
     * Do swap GL frame buffers (quad-buffer / double-buffer).
     */
    void glSwap();

    /**
     * Make active this GL rendering context.
     * @return true on success.
     */
    bool glMakeCurrent();

    /**
     * Create 1 or 2 GL rendering contexts (and share them)
     * for opened windows handles.
     */
    int glCreateContext(StWinHandles* theSlave, bool theIsQuadStereo);

    /**
     * Close all handles.
     */
    bool close();

#if(defined(_WIN32) || defined(__WIN32__))
    /**
     * Got the unique class name.
     */
    static StStringUtfWide getNewClassName();

#elif(defined(__linux__) || defined(__linux))
    /**
     * Fast link
     */
    Display* getDisplay() const {
        return stXDisplay.isNull() ? NULL : stXDisplay->hDisplay;
    }

    /**
     * Announce XDND support.
     */
    void setupXDND();
#endif

};

#endif //__StWinHandles_h_
