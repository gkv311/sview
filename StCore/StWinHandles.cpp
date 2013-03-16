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
#include <StThreads/StThreads.h>
#include <StStrings/StLogger.h>

#if (defined(_WIN32) || defined(__WIN32__))

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

StWinGlrc::StWinGlrc(HDC theDC)
: myRC(wglCreateContext(theDC)) {
    //
}

StWinGlrc::~StWinGlrc() {
    if(myRC != NULL) {
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
}

bool StWinGlrc::makeCurrent(HDC theDC) {
    return wglMakeCurrent(theDC, myRC) == TRUE;
}

#endif

StWinHandles::StWinHandles()
#if (defined(_WIN32) || defined(__WIN32__))
: threadIdWnd(0),
  evMsgThread(true),
  hWindow(NULL),
  hWindowGl(NULL),
  myMKeyStop(GlobalAddAtom(MAKEINTATOM(VK_MEDIA_STOP))),
  myMKeyPlay(GlobalAddAtom(MAKEINTATOM(VK_MEDIA_PLAY_PAUSE))),
  myMKeyPrev(GlobalAddAtom(MAKEINTATOM(VK_MEDIA_PREV_TRACK))),
  myMKeyNext(GlobalAddAtom(MAKEINTATOM(VK_MEDIA_NEXT_TRACK))),
  threadIdOgl(0),
  hDC(NULL) {
    //
#elif (defined(__linux__) || defined(__linux))
: hWindow(0),
  hWindowGl(0),
  stXDisplay(),
  hRC(NULL),
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
#if (defined(_WIN32) || defined(__WIN32__))
    if(hDC != NULL) {
        SwapBuffers(hDC);
    }
#elif (defined(__linux__) || defined(__linux))
    if(!stXDisplay.isNull()) {
        glXSwapBuffers(stXDisplay->hDisplay, hWindowGl);
    }
#endif
}

bool StWinHandles::glMakeCurrent() {
#if (defined(_WIN32) || defined(__WIN32__))
    if(hDC != NULL && !hRC.isNull()) {
        return hRC->makeCurrent(hDC);
    }
#elif (defined(__linux__) || defined(__linux))
    if(!stXDisplay.isNull()) {
        return glXMakeCurrent(stXDisplay->hDisplay, hWindowGl, hRC) == True;
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

int StWinHandles::glCreateContext(StWinHandles* theSlave, bool isQuadStereo) {
#if (defined(_WIN32) || defined(__WIN32__))
    threadIdOgl = StThread::getCurrentThreadId();
    ST_DEBUG_LOG("WinAPI, glCreateContext, threadIdOgl= " + threadIdOgl + ", threadIdWnd= " + threadIdWnd);
    hDC = GetDC(hWindowGl);
    ST_GL_ERROR_CHECK(hDC != NULL, STWIN_ERROR_WIN32_GLDC,
                      "WinAPI, Can't create Master GL Device Context");
    if(theSlave != NULL) {
        theSlave->threadIdOgl = threadIdOgl;
        theSlave->hDC = GetDC(theSlave->hWindowGl);
        ST_GL_ERROR_CHECK(theSlave->hDC != NULL, STWIN_ERROR_WIN32_GLDC,
                          "WinAPI, Can't create Slave GL Device Context");
    }

    PIXELFORMATDESCRIPTOR aPixelFormatDesc = THE_PIXELFRMT_DOUBLE;
    if(isQuadStereo) {
        aPixelFormatDesc.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_GDI | PFD_SUPPORT_OPENGL
                                 | PFD_DOUBLEBUFFER | PFD_STEREO;
    }
    int aPixelFormatId = ChoosePixelFormat(hDC, &aPixelFormatDesc);
    ST_GL_ERROR_CHECK(aPixelFormatId != 0, STWIN_ERROR_WIN32_PIXELFORMATF,
                      "WinAPI, Can't find a suitable PixelFormat for Master");
    if(theSlave != NULL
    && ChoosePixelFormat(theSlave->hDC, &aPixelFormatDesc) != aPixelFormatId) {
        ST_ERROR_LOG("Slave window returns another pixel format! Try to ignore...");
    }

    if(isQuadStereo) {
        DescribePixelFormat(hDC, aPixelFormatId, sizeof(PIXELFORMATDESCRIPTOR), &aPixelFormatDesc);
        if((aPixelFormatDesc.dwFlags & PFD_STEREO) == 0) {
            stError("WinAPI, Quad Buffered stereo not supported");
        } else {
            //bool isVistaPlus = StSys::isVistaPlus();
            //bool isWin8Plus  = StSys::isWin8Plus();
            ///myNeedsFullscr
        }
    }

    ST_GL_ERROR_CHECK(SetPixelFormat(hDC, aPixelFormatId, &aPixelFormatDesc),
                      STWIN_ERROR_WIN32_PIXELFORMATS, "WinAPI, Can't set the PixelFormat for Master");
    ST_GL_ERROR_CHECK(theSlave == NULL || SetPixelFormat(theSlave->hDC, aPixelFormatId, &aPixelFormatDesc),
                      STWIN_ERROR_WIN32_PIXELFORMATS, "WinAPI, Can't set the PixelFormat for Slave");

    hRC = new StWinGlrc(hDC);
    ST_GL_ERROR_CHECK(hRC->isValid(),
                      STWIN_ERROR_WIN32_GLRC_CREATE, "WinAPI, Can't create GL Rendering Context");
    if(theSlave != NULL) {
        theSlave->hRC = hRC;
    }

    ST_GL_ERROR_CHECK(hRC->makeCurrent(hDC),
                      STWIN_ERROR_WIN32_GLRC_ACTIVATE, "WinAPI, Can't activate Master GL Rendering Context");
    return STWIN_INIT_SUCCESS;
#elif (defined(__linux__) || defined(__linux))
    // create an OpenGL rendering context
    hRC = glXCreateContext(stXDisplay->hDisplay, stXDisplay->hVisInfo,
                           None, true); // direct rendering if possible
    if(hRC == NULL) {
        stError("X, could not create rendering context for Master");
        return STWIN_ERROR_X_GLRC_CREATE;
    }

    if(theSlave != NULL) {
        theSlave->hRC = glXCreateContext(theSlave->stXDisplay->hDisplay, stXDisplay->hVisInfo,
                                         hRC, true); // shared GL contexts and direct rendering if possible
        if(theSlave->hRC == NULL) {
            stError("X, could not create rendering context for Slave");
            return STWIN_ERROR_X_GLRC_CREATE;
        }
    }

    // bind the rendering context to the window
    glXMakeCurrent(stXDisplay->hDisplay, hWindowGl, hRC);
    return STWIN_INIT_SUCCESS;
#endif
}

bool StWinHandles::close() {
#if(defined(_WIN32) || defined(__WIN32__))
    // NOTE - destroy functions will fail if called from another thread than created
    size_t currThreadId = StThread::getCurrentThreadId();
    stMutex.lock();
    // ========= Release OpenGL resources =========
    if(currThreadId == threadIdOgl && hWindowGl != NULL) {
        ST_DEBUG_LOG("WinAPI, close, currThreadId= " + currThreadId + ", threadIdOgl= " + threadIdOgl + ", threadIdWnd= " + threadIdWnd);

        // release Rendering Context
        hRC.nullify();

        // Release Device Context
        if(hDC != NULL && hWindowGl != NULL) {
            if(ReleaseDC(hWindowGl, hDC) == 0) {
                ST_DEBUG_LOG("WinAPI, FAILED to release DC");
                stMutex.unlock();
                return false;
            } else {
                ST_DEBUG_LOG("WinAPI, Released DC");
                hDC = NULL;
                threadIdOgl = 0;
            }
        }
    }

    // ========= Release window resources =========
    if(currThreadId == threadIdWnd && hDC == NULL) {
        ST_DEBUG_LOG("WinAPI, close, currThreadId= " + currThreadId + ", threadIdOgl= " + threadIdOgl + ", threadIdWnd= " + threadIdWnd);

        // are we able to Destroy the Window?
        if(hWindowGl != NULL) {
            if(DestroyWindow(hWindowGl) == 0) {
                ST_DEBUG_LOG("WinAPI, FAILED to destroy the Window");
                stMutex.unlock();
                return false;
            } else {
                ST_DEBUG_LOG("WinAPI, Destroyed GLwindow");
                hWindowGl = NULL;
            }
        }
        if(hWindow != NULL) {
            if(DestroyWindow(hWindow) == 0) {
                ST_DEBUG_LOG("WinAPI, FAILED to destroy the Window");
                stMutex.unlock();
                return false;
            } else {
                ST_DEBUG_LOG("WinAPI, Destroyed window");
                hWindow = NULL;
            }
        }

        // are we able to unregister Class
        if(hWindowGl == NULL && hWindow == NULL && !classNameGl.isEmpty()) {
            if(UnregisterClassW(classNameGl.toCString(), GetModuleHandle(NULL)) == 0) {
                ST_DEBUG_LOG("WinAPI, FAILED to unregister Class= '" + classNameGl.toUtf8() + "'");
                stMutex.unlock();
                return false;
            } else {
                ST_DEBUG_LOG("WinAPI, Unregistered Class= " + classNameGl.toUtf8());
                classNameGl.clear();
            }
        }
        if(hWindowGl == NULL && hWindow == NULL && !className.isEmpty()) {
            if(UnregisterClassW(className.toCString(), GetModuleHandle(NULL)) == 0) {
                ST_DEBUG_LOG("WinAPI, FAILED to unregister Class= '" + className.toUtf8() + "'");
                stMutex.unlock();
                return false;
            } else {
                ST_DEBUG_LOG("WinAPI, Unregistered Class= " + className.toUtf8());
                className.clear();
            }
        }
    }
    stMutex.unlock();
#elif (defined(__linux__) || defined(__linux))
    if(!stXDisplay.isNull()) {
        // release active context
        if(!glXMakeCurrent(stXDisplay->hDisplay, None, NULL)) {
            ST_DEBUG_LOG("X, FAILED to release OpenGL context");
        }
        if(hRC != NULL) {
            glXDestroyContext(stXDisplay->hDisplay, hRC);
            hRC = NULL;
        }
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

#if (defined(_WIN32) || defined(__WIN32__))
namespace {
    static const StStringUtfWide ST_WINDOW_CLASSNAME = L"StWindowClass";
    static StAtomic<int32_t> ST_CLASS_COUNTER(0);
};

StStringUtfWide StWinHandles::getNewClassName() {
    return ST_WINDOW_CLASSNAME + StStringUtfWide(ST_CLASS_COUNTER.increment());
}

#else
void StWinHandles::setupXDND() {
    Atom aVersion = 5;
    XChangeProperty(stXDisplay->hDisplay, hWindowGl, stXDisplay->xDNDAware, XA_ATOM, 32, PropModeReplace, (unsigned char* )&aVersion, 1);
    if(hWindow != 0) {
        XChangeProperty(stXDisplay->hDisplay, hWindow, stXDisplay->xDNDAware, XA_ATOM, 32, PropModeReplace, (unsigned char* )&aVersion, 1);
    }
}
#endif

#endif // !__APPLE__
