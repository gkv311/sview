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

#ifndef __APPLE__

#include "StWinHandles.h"
#include <StThreads/StThread.h>
#include <StStrings/StLogger.h>
#include <StGL/StGLContext.h>
#include <StGL/StGLFunctions.h>

#ifdef _WIN32

static PIXELFORMATDESCRIPTOR THE_PIXELFRMT_DOUBLE = {
    sizeof(PIXELFORMATDESCRIPTOR),   // Size Of This Pixel Format Descriptor
    1,                               // Version Number
    PFD_DRAW_TO_WINDOW |             // Format Must Support Window
    PFD_SUPPORT_OPENGL |             // Format Must Support OpenGL
    PFD_DOUBLEBUFFER,                // Must Support Double Buffering
    PFD_TYPE_RGBA,                   // Request An RGBA Format
    32,                              // Select Our Color Depth
    0, 0, 0, 0, 0, 0,                // Color Bits Ignored
    0,                               // No Alpha Buffer
    0,                               // Shift Bit Ignored
    0,                               // No Accumulation Buffer
    0, 0, 0, 0,                      // Accumulation Bits Ignored
    16,                              // 16Bit Z-Buffer (Depth Buffer)
    0,                               // No Stencil Buffer
    0,                               // No Auxiliary Buffer
    PFD_MAIN_PLANE,                  // Main Drawing Layer
    0,                               // Reserved
    0, 0, 0                          // Layer Masks Ignored
};

StWinGlrc::StWinGlrc(HDC theDC, HGLRC theRC)
: myRC(theRC != NULL ? theRC : wglCreateContext(theDC)) {
    //
}

bool StWinGlrc::isCurrent(HDC theDC) const {
    return theDC == wglGetCurrentDC()
        && myRC  == wglGetCurrentContext();
}

bool StWinGlrc::makeCurrent(HDC theDC) {
    return wglMakeCurrent(theDC, myRC) == TRUE;
}

StWinGlrc::~StWinGlrc() {
    if(myRC == NULL) {
        return;
    }

    if(wglMakeCurrent(NULL, NULL) == FALSE) {
        // this is not a problem in most cases;
        // also this happens when wglMakeCurrent(NULL, NULL) called twice
        //ST_DEBUG_LOG("WinAPI, FAILED to release DC and RC contexts");
    }

    //ST_ASSERT_SLIP(wglDeleteContext(myRC) != FALSE, "WinAPI, FAILED to delete RC", return);
    if(wglDeleteContext(myRC) == FALSE) {
        ST_ERROR_LOG("WinAPI, FAILED to delete RC");
    }
}

#else

StWinGlrc::StWinGlrc(StHandle<StXDisplay>& theDisplay)
: myDisplay(theDisplay->hDisplay),
  myRC(glXCreateContext(theDisplay->hDisplay, theDisplay->hVisInfo, None, true)) {
    //
}

StWinGlrc::~StWinGlrc() {
    if(myRC == NULL) {
        return;
    }

    // release active context
    if(!glXMakeCurrent(myDisplay, None, NULL)) {
        ST_DEBUG_LOG("X, FAILED to release OpenGL context");
    }
    glXDestroyContext(myDisplay, myRC);
}

bool StWinGlrc::makeCurrent(GLXDrawable theDrawable) {
    return myRC != NULL
        && glXMakeCurrent(myDisplay, theDrawable, myRC) == True;
}

#endif

StWinHandles::StWinHandles()
#ifdef _WIN32
: ThreadWnd(0),
  EventMsgThread(true),
  hWindow(NULL),
  hWindowGl(NULL),
  hWinTmp(NULL),
  myMKeyStop(GlobalAddAtom(MAKEINTATOM(VK_MEDIA_STOP))),
  myMKeyPlay(GlobalAddAtom(MAKEINTATOM(VK_MEDIA_PLAY_PAUSE))),
  myMKeyPrev(GlobalAddAtom(MAKEINTATOM(VK_MEDIA_PREV_TRACK))),
  myMKeyNext(GlobalAddAtom(MAKEINTATOM(VK_MEDIA_NEXT_TRACK))),
  ThreadGL(0),
  hDC(NULL) {
    //
#elif defined(__linux__)
: hWindow(0),
  hWindowGl(0),
  stXDisplay(),
  iconImage(0),
  iconShape(0),
  xDNDRequestType(None),
  xDNDSrcWindow(0),
  xDNDVersion(0),
  xrandrEventBase(0),
  isRecXRandrEvents(false) {
    //
#endif
}

StWinHandles::~StWinHandles() {
    close();
}

void StWinHandles::glSwap() {
#ifdef _WIN32
    if(hDC != NULL) {
        SwapBuffers(hDC);
    }
#elif defined(__linux__)
    if(!stXDisplay.isNull()
    && hRC->makeCurrent(hWindowGl)) { // if GL rendering context is bound to another drawable - we got BadMatch error
        glXSwapBuffers(stXDisplay->hDisplay, hWindowGl);
    }
#endif
}

bool StWinHandles::glMakeCurrent() {
#ifdef _WIN32
    if(hDC != NULL && !hRC.isNull()) {
        return hRC->isCurrent(hDC)
            || hRC->makeCurrent(hDC);
    }
#elif defined(__linux__)
    if(!stXDisplay.isNull() && !hRC.isNull()) {
        return hRC->makeCurrent(hWindowGl);
    }
#endif
    return false;
}

/**
 * Auxiliary macros.
 */
#define ST_GL_ERROR_CHECK(theTrueCondition, theErrCode, theErrDesc) \
    if(!(theTrueCondition)) { \
        stError(theErrDesc); \
        return theErrCode; \
    }

int StWinHandles::glCreateContext(StWinHandles*    theSlave,
                                  const StRectI_t& theRect,
                                  const int        theDepthSize,
                                  const bool       theIsQuadStereo,
                                  const bool       theDebugCtx) {
#ifdef _WIN32
    ThreadGL = StThread::getCurrentThreadId();
    ST_DEBUG_LOG("WinAPI, glCreateContext, ThreadGL= " + ThreadGL + ", ThreadWnd= " + ThreadWnd);
    hDC = GetDC(hWindowGl);
    ST_GL_ERROR_CHECK(hDC != NULL, STWIN_ERROR_WIN32_GLDC,
                      "WinAPI, Can't create Master GL Device Context");
    if(theSlave != NULL) {
        theSlave->ThreadGL = ThreadGL;
        theSlave->hDC      = GetDC(theSlave->hWindowGl);
        ST_GL_ERROR_CHECK(theSlave->hDC != NULL, STWIN_ERROR_WIN32_GLDC,
                          "WinAPI, Can't create Slave GL Device Context");
    }

    PIXELFORMATDESCRIPTOR aPixFrmtDesc = THE_PIXELFRMT_DOUBLE;
    aPixFrmtDesc.cDepthBits = (BYTE )theDepthSize;
    if(theIsQuadStereo) {
        aPixFrmtDesc.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_GDI | PFD_SUPPORT_OPENGL
                             | PFD_DOUBLEBUFFER | PFD_STEREO;
    }
    int aPixFrmtId = ChoosePixelFormat(hDC, &aPixFrmtDesc);
    ST_GL_ERROR_CHECK(aPixFrmtId != 0, STWIN_ERROR_WIN32_PIXELFORMATF,
                      "WinAPI, Can't find a suitable PixelFormat for Master");
    if(theSlave != NULL
    && ChoosePixelFormat(theSlave->hDC, &aPixFrmtDesc) != aPixFrmtId) {
        ST_ERROR_LOG("Slave window returns another pixel format! Try to ignore...");
    }

    if(theIsQuadStereo) {
        DescribePixelFormat(hDC, aPixFrmtId, sizeof(PIXELFORMATDESCRIPTOR), &aPixFrmtDesc);
        if((aPixFrmtDesc.dwFlags & PFD_STEREO) == 0) {
            ST_ERROR_LOG("WinAPI, Quad Buffered stereo not supported");
        } else {
            //bool isVistaPlus = StSys::isVistaPlus();
            //bool isWin8Plus  = StSys::isWin8Plus();
            ///myNeedsFullscr
        }
    }

    HMODULE aModule = GetModuleHandleW(NULL);
    hWinTmp = CreateWindowExW(WS_EX_TOOLWINDOW | WS_EX_WINDOWEDGE | WS_EX_NOACTIVATE,
                              ClassTmp.toCString(), L"TmpWnd",
                              WS_POPUP | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_DISABLED,
                              theRect.left() + 2, theRect.top() + 2, 4, 4,
                              NULL, NULL, aModule, NULL);
    ST_GL_ERROR_CHECK(hWinTmp != NULL, STWIN_ERROR_WIN32_GLDC,
                      "WinAPI, Temporary window creation error");

    HDC aDevCtxTmp = GetDC(hWinTmp);
    ST_GL_ERROR_CHECK(aPixFrmtId != 0, STWIN_ERROR_WIN32_PIXELFORMATF,
                      "WinAPI, Can't find a suitable PixelFormat for Tmp");

    ST_GL_ERROR_CHECK(SetPixelFormat(aDevCtxTmp, aPixFrmtId, &aPixFrmtDesc),
                      STWIN_ERROR_WIN32_PIXELFORMATS, "WinAPI, Can't set the PixelFormat for Master");
    StWinGlrcH aRendCtxTmp = new StWinGlrc(aDevCtxTmp, NULL);
    ST_GL_ERROR_CHECK(aRendCtxTmp->isValid(),
                      STWIN_ERROR_WIN32_GLRC_CREATE, "WinAPI, Can't create GL Rendering Context");
    ST_GL_ERROR_CHECK(aRendCtxTmp->makeCurrent(aDevCtxTmp),
                      STWIN_ERROR_WIN32_GLRC_ACTIVATE, "WinAPI, Can't activate Tmp GL Rendering Context");

    StGLContext aCtx;
    ST_GL_ERROR_CHECK(aCtx.stglInit(),
                      STWIN_ERROR_WIN32_GLRC_ACTIVATE, "WinAPI, Broken Tmp GL Rendering Context");

    if(aCtx.extAll->wglChoosePixelFormatARB != NULL) {
        const int aPixAttribs[] = {
            WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
            WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
            WGL_DOUBLE_BUFFER_ARB,  GL_TRUE,
            WGL_STEREO_ARB,         theIsQuadStereo ? GL_TRUE : GL_FALSE,
            WGL_PIXEL_TYPE_ARB,     WGL_TYPE_RGBA_ARB,
            //WGL_SAMPLE_BUFFERS_ARB, 1,
            //WGL_SAMPLES_ARB,        8,
            // WGL_CONTEXT_ROBUST_ACCESS_BIT_ARB       0x00000004
            WGL_COLOR_BITS_ARB,     32,
            WGL_DEPTH_BITS_ARB,     24,
            WGL_STENCIL_BITS_ARB,   8,
            0, 0,
        };
        unsigned int aFrmtsNb = 0;
        aCtx.extAll->wglChoosePixelFormatARB(hDC, aPixAttribs, NULL, 1, &aPixFrmtId, &aFrmtsNb);
    }
    ST_GL_ERROR_CHECK(SetPixelFormat(hDC, aPixFrmtId, &aPixFrmtDesc),
                      STWIN_ERROR_WIN32_PIXELFORMATS, "WinAPI, Can't set the PixelFormat for Master");
    ST_GL_ERROR_CHECK(theSlave == NULL || SetPixelFormat(theSlave->hDC, aPixFrmtId, &aPixFrmtDesc),
                      STWIN_ERROR_WIN32_PIXELFORMATS, "WinAPI, Can't set the PixelFormat for Slave");

    HGLRC aRendCtx = NULL;
    if(aCtx.extAll->wglCreateContextAttribsARB != NULL) {
        int aCtxAttribs[] = {
            //WGL_CONTEXT_MAJOR_VERSION_ARB, 3,
            //WGL_CONTEXT_MINOR_VERSION_ARB, 2,
            WGL_CONTEXT_PROFILE_MASK_ARB,  WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB, //WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
            WGL_CONTEXT_FLAGS_ARB,         theDebugCtx ? WGL_CONTEXT_DEBUG_BIT_ARB : 0,
            0, 0
        };

        aRendCtx = aCtx.extAll->wglCreateContextAttribsARB(hDC, NULL, aCtxAttribs);
    }

    aRendCtxTmp.nullify();
    destroyWindow(hWinTmp);

    hRC = new StWinGlrc(hDC, aRendCtx);
    ST_GL_ERROR_CHECK(hRC->isValid(),
                      STWIN_ERROR_WIN32_GLRC_CREATE, "WinAPI, Can't create GL Rendering Context");
    if(theSlave != NULL) {
        theSlave->hRC = hRC;
    }

    ST_GL_ERROR_CHECK(hRC->makeCurrent(hDC),
                      STWIN_ERROR_WIN32_GLRC_ACTIVATE, "WinAPI, Can't activate Master GL Rendering Context");
    return STWIN_INIT_SUCCESS;
#elif defined(__linux__)
    // create an OpenGL rendering context
    hRC = new StWinGlrc(stXDisplay);
    ST_GL_ERROR_CHECK(hRC->isValid(),
                      STWIN_ERROR_X_GLRC_CREATE, "GLX, could not create rendering context for Master");
    if(theSlave != NULL) {
        theSlave->hRC = hRC;

        // bind the rendering context to the window
        ST_GL_ERROR_CHECK(hRC->makeCurrent(theSlave->hWindowGl),
                          STWIN_ERROR_X_GLRC_CREATE, "GLX, Can't activate Slave GL Rendering Context");
    }

    // bind the rendering context to the window
    ST_GL_ERROR_CHECK(hRC->makeCurrent(hWindowGl),
                      STWIN_ERROR_X_GLRC_CREATE, "GLX, Can't activate Master GL Rendering Context");
    return STWIN_INIT_SUCCESS;
#endif
}

bool StWinHandles::close() {
#ifdef _WIN32
    // NOTE - destroy functions will fail if called from another thread than created
    const size_t aThreadId = StThread::getCurrentThreadId();
    myMutex.lock();
    // ========= Release OpenGL resources =========
    if(aThreadId == ThreadGL && hWindowGl != NULL) {
        ST_DEBUG_LOG("WinAPI, close, aThreadId= " + aThreadId + ", ThreadGL= " + ThreadGL + ", ThreadWnd= " + ThreadWnd);

        // release Rendering Context
        hRC.nullify();

        // Release Device Context
        if(hDC != NULL && hWindowGl != NULL) {
            if(ReleaseDC(hWindowGl, hDC) == 0) {
                ST_DEBUG_LOG("WinAPI, FAILED to release DC");
                myMutex.unlock();
                return false;
            } else {
                ST_DEBUG_LOG("WinAPI, Released DC");
                hDC = NULL;
                ThreadGL = 0;
            }
        }
    }

    // release window resources
    if(aThreadId == ThreadWnd && hDC == NULL) {
        ST_DEBUG_LOG("WinAPI, close, aThreadId= " + aThreadId + ", ThreadGL= " + ThreadGL + ", ThreadWnd= " + ThreadWnd);

        // destroy windows
        if(!destroyWindow(hWindowGl)
        || !destroyWindow(hWindow)
        || !destroyWindow(hWinTmp)) {
            myMutex.unlock();
            return false;
        }

        // unregister window classes
        if(hWindowGl == NULL && hWindow == NULL) {
            if(!unregisterClass(ClassGL)
            || !unregisterClass(ClassBase)
            || !unregisterClass(ClassTmp)) {
                myMutex.unlock();
                return false;
            }
        }
    }
    myMutex.unlock();
#elif defined(__linux__)
    // release active context
    hRC.nullify();
    if(!stXDisplay.isNull()) {
        // close x-server windows
        if(hWindowGl != 0) {
            XUnmapWindow(stXDisplay->hDisplay, hWindowGl);
            XDestroyWindow(stXDisplay->hDisplay, hWindowGl);
            hWindowGl = 0;
        }
        if(hWindow != 0) {
            XUnmapWindow(stXDisplay->hDisplay, hWindow);
            XDestroyWindow(stXDisplay->hDisplay, hWindow);
            hWindow = 0;
        }
        if(iconImage != 0) {
            XFreePixmap(stXDisplay->hDisplay, iconImage);
            iconImage = 0;
        }
        if(iconShape != 0) {
            XFreePixmap(stXDisplay->hDisplay, iconShape);
            iconShape = 0;
        }

        // close x-server connection
        stXDisplay.nullify();
    }
#endif
    return true;
}

#ifdef _WIN32

namespace {
    static StAtomic<int32_t> ST_CLASS_COUNTER(0);
};

StStringUtfWide StWinHandles::getNewClassName() {
    return StStringUtfWide(L"StWindowClass") + StStringUtfWide(ST_CLASS_COUNTER.increment());
}

bool StWinHandles::registerClass(const StStringUtfWide& theName,
                                 WNDPROC                theProc) {
    HINSTANCE aModule = GetModuleHandleW(NULL);
    WNDCLASSW aClass; stMemZero(&aClass, sizeof(aClass));
    // redraw on resize, and request own DC for window
    aClass.style         = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    aClass.lpfnWndProc   = theProc;
    aClass.cbClsExtra    = 0;
    aClass.cbWndExtra    = 0;
    aClass.hInstance     = aModule;
    aClass.hIcon         = LoadIconW(aModule, L"A");
    aClass.hCursor       = LoadCursor(NULL, IDC_ARROW);
    aClass.hbrBackground = NULL;
    aClass.lpszMenuName  = NULL;
    aClass.lpszClassName = theName.toCString();
    if(RegisterClassW(&aClass) == 0) {
        stError(StString("WinAPI: Failed to register window class '") + theName.toUtf8() + "'");
        return false;
    }
    return true;
}

bool StWinHandles::registerClasses(StWinHandles* theSlave,
                                   WNDPROC       theProc) {
    const StStringUtfWide aBase      = getNewClassName();
    const StStringUtfWide aNameGl    = aBase + StStringUtfWide(L"Gl");
    const StStringUtfWide aNameTmp   = aBase + StStringUtfWide(L"Tmp");
    const StStringUtfWide aNameSlave = aBase + StStringUtfWide(L"SGl");
    if(!registerClass(aBase, theProc)) {
        return false;
    }
    ClassBase = aBase;

    if(!registerClass(aNameGl, theProc)) {
        return false;
    }
    ClassGL = aNameGl;

    if(!registerClass(aNameTmp, theProc)) {
        return false;
    }
    ClassTmp = aNameTmp;

    if(theSlave != NULL) {
        if(!registerClass(aNameSlave, theProc)) {
            return false;
        }
        theSlave->ClassGL = aNameSlave;
    }
    return true;
}

bool StWinHandles::unregisterClass(StStringUtfWide& theName) {
    HMODULE aModule = GetModuleHandleW(NULL);
    if(!theName.isEmpty()) {
        if(UnregisterClassW(theName.toCString(), aModule) == 0) {
            ST_DEBUG_LOG("WinAPI, FAILED to unregister Class= '" + theName.toUtf8() + "'");
            return false;
        }
        ST_DEBUG_LOG("WinAPI, Unregistered Class= " + theName.toUtf8());
        theName.clear();
    }
    return true;
}

bool StWinHandles::destroyWindow(HWND& theWindow) {
    if(theWindow != NULL) {
        if(DestroyWindow(theWindow) == 0) {
            ST_DEBUG_LOG("WinAPI, FAILED to destroy the Window");
            return false;
        }
        ST_DEBUG_LOG("WinAPI, Destroyed window");
        theWindow = NULL;
    }
    return true;
}

#else

void StWinHandles::setupXDND() {
    Atom aVersion = 5;
    XChangeProperty(stXDisplay->hDisplay, hWindowGl, stXDisplay->xDNDAware, XA_ATOM, 32, PropModeReplace, (unsigned char* )&aVersion, 1);
    if(hWindow != 0) {
        XChangeProperty(stXDisplay->hDisplay, hWindow, stXDisplay->xDNDAware, XA_ATOM, 32, PropModeReplace, (unsigned char* )&aVersion, 1);
    }
}

namespace {
    static const char noPixData[] = {0, 0, 0, 0, 0, 0, 0, 0};
};

void StWinHandles::setupNoCursor() {
    Display* aDisp = getDisplay();
    if(aDisp == NULL
    || hWindowGl == 0) {
        return;
    }

    XColor black, dummy;
    Colormap aColorMap = DefaultColormap(aDisp, DefaultScreen(aDisp));
    XAllocNamedColor(aDisp, aColorMap, "black", &black, &dummy);
    Pixmap noPix = XCreateBitmapFromData(aDisp, hWindowGl, noPixData, 8, 8);
    Cursor noPtr = XCreatePixmapCursor(aDisp, noPix, noPix, &black, &black, 0, 0);
    XDefineCursor(aDisp, hWindowGl, noPtr);
    XFreeCursor(aDisp, noPtr);
    if(noPix != None) {
        XFreePixmap(aDisp, noPix);
    }
    XFreeColors(aDisp, aColorMap, &black.pixel, 1, 0);
}

#endif

#endif // !__APPLE__
