/**
 * StCore, window system independent C++ toolkit for writing OpenGL applications.
 * Copyright © 2007-2014 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#ifndef __StWinHandles_h_
#define __StWinHandles_h_

#ifdef _WIN32
    #include <windows.h>   // header file for Windows
#elif defined(__ANDROID__)
    #include <EGL/egl.h>
    #include <android/native_window.h>
    #include <android/window.h>
#elif defined(ST_HAVE_EGL)
    #include <EGL/egl.h>
#elif defined(__APPLE__)
    #include "StCocoaView.h"
    #include "StCocoaWin.h"
#elif defined(__linux__)
    // exclude modern definitions and system-provided glext.h, should be defined before gl.h inclusion
    #ifndef GL_GLEXT_LEGACY
        #define GL_GLEXT_LEGACY
    #endif
    #ifndef GLX_GLXEXT_LEGACY
        #define GLX_GLXEXT_LEGACY
    #endif
    #include <GL/glx.h>
#endif

#include "StXDisplay.h"
#include <StStrings/StString.h>
#include <StCore/StWinErrorCodes.h>
#include <StThreads/StMutex.h>
#include <StThreads/StCondition.h>
#include <StTemplates/StRect.h>

#ifndef __APPLE__
/**
 * Wrapper over OpenGL rendering context.
 */
class StWinGlrc {

        public:

    /**
     * Create OpenGL Rendering Context for specified Device Context.
     */
#if defined(ST_HAVE_EGL) || defined(__ANDROID__)
    ST_LOCAL StWinGlrc(EGLDisplay theDisplay,
                       const bool theDebugCtx,
                       int8_t     theGlDepthSize,
                       int8_t     theGlStencilSize);
#elif defined(_WIN32)
    ST_LOCAL StWinGlrc(HDC theDC, HGLRC theRC);
#else
    ST_LOCAL StWinGlrc(StHandle<StXDisplay>& theDisplay,
                       const bool            theDebugCtx);
#endif

    /**
     * Destructor.
     */
    ST_LOCAL ~StWinGlrc();

    /**
     * @return true if handle is not NULL.
     */
    ST_LOCAL bool isValid() const {
        return myRC != NULL;
    }

    /**
     * Activate this OpenGL context within specified Device Context.
     * Device Context should have the one used on construction of this Rendering Context
     * or have the same Pixel Format.
     */
#if defined(ST_HAVE_EGL) || defined(__ANDROID__)
    ST_LOCAL bool makeCurrent(EGLSurface theSurface);
#elif defined(_WIN32)
    ST_LOCAL bool makeCurrent(HDC theDC);
    ST_LOCAL bool isCurrent  (HDC theDC) const;
#else
    ST_LOCAL bool makeCurrent(GLXDrawable theDrawable);
#endif

#if defined(ST_HAVE_EGL) || defined(__ANDROID__)
    /**
     * EGL display connection
     */
    ST_LOCAL EGLDisplay getDisplay() const { return myDisplay; }

    /**
     * EGL configuration
     */
    ST_LOCAL EGLConfig  getConfig()  const { return myConfig;  }

    /**
     * EGL rendering context.
     */
    ST_LOCAL EGLContext getRenderContext() const { return myRC; }
#elif defined(_WIN32)
    /**
     * WinAPI Rendering Context handle.
     */
    ST_LOCAL HGLRC getRenderContext() const { return myRC; }
#else
    /**
     * X-GLX rendering context.
     */
    ST_LOCAL GLXContext getRenderContext() const { return myRC; }
#endif

        private:

#if defined(ST_HAVE_EGL) || defined(__ANDROID__)
    EGLDisplay myDisplay; //!< EGL display connection
    EGLConfig  myConfig;  //!< EGL configuration
    EGLContext myRC;      //!< EGL rendering context
#elif defined(_WIN32)
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
class StWinHandles {

        public:

    /**
     * Creates NULL-handles.
     */
    ST_LOCAL StWinHandles();

    ST_LOCAL ~StWinHandles();

    /**
     * Do swap GL frame buffers (quad-buffer / double-buffer).
     */
    ST_LOCAL void glSwap();

    /**
     * Make active this GL rendering context.
     * @return true on success.
     */
    ST_LOCAL bool glMakeCurrent();

    /**
     * Create 1 or 2 GL rendering contexts (and share them)
     * for opened windows handles.
     */
    ST_LOCAL int glCreateContext(StWinHandles*    theSlave,
                                 const StRectI_t& theRect,
                                 const int        theDepthSize,
                                 const int        theStencilSize,
                                 const bool       theIsQuadStereo,
                                 const bool       theDebugCtx);

    /**
     * Close all handles.
     */
    ST_LOCAL bool close();

#ifdef _WIN32
    /**
     * Got the unique class name.
     */
    ST_LOCAL static StStringUtfWide getNewClassName();

    /**
     * Register window class with specified name.
     * @param theName class name
     * @param theProc callback function
     * @return true on success
     */
    ST_LOCAL static bool registerClass(const StStringUtfWide& theName,
                                       WNDPROC                theProc);

    /**
     * Register window classes.
     */
    ST_LOCAL bool registerClasses(StWinHandles* theSlave,
                                  WNDPROC       theProc);

    /**
     * Unregister window class with specified name.
     * @param theName class name, will be cleared on success
     * @return true on success
     */
    ST_LOCAL static bool unregisterClass(StStringUtfWide& theName);

    /**
     * Destroy window.
     */
    ST_LOCAL static bool destroyWindow(HWND& theWindow);

#elif defined(__ANDROID__)
    //
#elif defined(__linux__)
    /**
     * Fast link
     */
    ST_LOCAL Display* getDisplay() const {
        return stXDisplay.isNull() ? NULL : stXDisplay->hDisplay;
    }

    /**
     * Announce XDND support.
     */
    ST_LOCAL void setupXDND();

    /**
     * Setup empty cursor (hide cursor).
     * Call XUndefineCursor to revert changes.
     */
    ST_LOCAL void setupNoCursor();
#endif

        public:

#ifdef _WIN32
    size_t          ThreadWnd;      //!< id of the thread, in wich window was created
    StCondition     EventMsgThread; //!< special event to check message thread active state
    HWND            hWindow; // WinAPI windows' handles
    HWND            hWindowGl;
    HWND            hWinTmp; // temporary window
    StStringUtfWide ClassBase;      //!< WinAPI classes' names
    StStringUtfWide ClassGL;
    StStringUtfWide ClassTmp;
    ATOM            myMKeyStop;
    ATOM            myMKeyPlay;
    ATOM            myMKeyPrev;
    ATOM            myMKeyNext;

    StMutex         myMutex;
    size_t          ThreadGL;       //!< id of the thread, in which rendering contexts were created
    HDC             hDC; // WinAPI Device Descriptor handle
    StWinGlrcH      hRC; // WinAPI Rendering Context handle
#elif defined(__APPLE__)
    StCocoaWin*     hWindow;
    StCocoaView*    hViewGl;
#elif defined(__ANDROID__)
    ANativeWindow*  hWindowGl;
    StWinGlrcH      hRC;
#elif defined(__linux__)
    Window          hWindow; // X-window handle
    Window          hWindowGl; // X-window handle for undecorated GL window
    StXDisplayH     stXDisplay; // X-server display connection
    StWinGlrcH      hRC; // X-GLX rendering context

    Pixmap          iconImage; // icon stuff
    Pixmap          iconShape;

    Atom            xDNDRequestType; // X Drag & Drop stuff
    Window          xDNDSrcWindow;
    int             xDNDVersion;
    int             xrandrEventBase;
    bool            isRecXRandrEvents;
#endif

#if defined(ST_HAVE_EGL) || defined(__ANDROID__)
    EGLSurface      eglSurface; //!< EGL surface (window)
#endif

};

#endif // __StWinHandles_h_
