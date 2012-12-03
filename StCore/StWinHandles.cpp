/**
 * Copyright © 2007-2011 Kirill Gavrilov <kirill@sview.ru>
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

static PIXELFORMATDESCRIPTOR pfdDouble = {
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
#endif

StWinHandles::StWinHandles()
#if (defined(_WIN32) || defined(__WIN32__))
: threadIdWnd(0),
  evMsgThread(true),
  hWindow(NULL),
  hWindowGl(NULL),
  className(),
  classNameGl(),
  stMutex(),
  threadIdOgl(0),
  hDC(NULL),
  hRC(NULL) {
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
    if(hDC != NULL && hRC != NULL) {
        return wglMakeCurrent(hDC, hRC) == TRUE;
    }
#elif (defined(__linux__) || defined(__linux))
    if(!stXDisplay.isNull()) {
        return glXMakeCurrent(stXDisplay->hDisplay, hWindowGl, hRC) == True;
    }
#endif
    return false;
}

int StWinHandles::glCreateContext(StWinHandles* theSlave, bool isQuadStereo) {
#if (defined(_WIN32) || defined(__WIN32__))
    threadIdOgl = StThread::getCurrentThreadId();
    ST_DEBUG_LOG("WinAPI, glCreateContext, threadIdOgl= " + threadIdOgl + ", threadIdWnd= " + threadIdWnd);
    int pixelFormat; // Holds The Results After Searching For A Match
    hDC = GetDC(hWindowGl);
    if(hDC == NULL) { // Did We Get A Device Context?
        stError("WinAPI, Can't create Master GL Device Context");
        return STWIN_ERROR_WIN32_GLDC;
    }

    if(theSlave != NULL) {
        theSlave->threadIdOgl = threadIdOgl;
        theSlave->hDC = GetDC(theSlave->hWindowGl);
        if(theSlave->hDC == NULL) {
            stError("WinAPI, Can't create Slave GL Device Context");
            return STWIN_ERROR_WIN32_GLDC;
        }
    }

    PIXELFORMATDESCRIPTOR pfd = pfdDouble;
    memcpy(&pfd, &pfdDouble, sizeof(PIXELFORMATDESCRIPTOR));
    if(isQuadStereo) {
        pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_GDI | PFD_SUPPORT_OPENGL |
                PFD_DOUBLEBUFFER | PFD_STEREO;
    }
    pixelFormat = ChoosePixelFormat(hDC, &pfd);
    if(pixelFormat == 0) {   // Did Windows Find A Matching Pixel Format?
        stError("WinAPI, Can't find a suitable PixelFormat for Master");
        return STWIN_ERROR_WIN32_PIXELFORMATF;
    }

    if(theSlave != NULL) {
        pixelFormat = ChoosePixelFormat(theSlave->hDC, &pfd);
        if(pixelFormat == 0) {
            stError("WinAPI, Can't find a suitable PixelFormat for Slave");
            return STWIN_ERROR_WIN32_PIXELFORMATF;
        }
    }

    if(!SetPixelFormat(hDC, pixelFormat, &pfd)) { // Are We Able To Set The Pixel Format?
        stError("WinAPI, Can't set the PixelFormat for Master");
        return STWIN_ERROR_WIN32_PIXELFORMATS;
    }

    if(theSlave != NULL) {
        if(!SetPixelFormat(theSlave->hDC, pixelFormat, &pfd)) {
            stError("WinAPI, Can't set the PixelFormat for Slave");
            return STWIN_ERROR_WIN32_PIXELFORMATS;
        }
    }

    hRC = wglCreateContext(hDC);
    if(hRC == NULL) { // Are We Able To Get A Rendering Context?
        stError("WinAPI, Can't create GL Rendering Context for Master");
        return STWIN_ERROR_WIN32_GLRC_CREATE;
    }

    if(theSlave != NULL) {
        theSlave->hRC = wglCreateContext(theSlave->hDC);
        if(theSlave->hRC == NULL) {
            stError("WinAPI, Can't create GL Rendering Context for Slave");
            return STWIN_ERROR_WIN32_GLRC_CREATE;
        }
    }

    if(isQuadStereo) {
        pixelFormat = GetPixelFormat(hDC);
        DescribePixelFormat(hDC, pixelFormat, sizeof(PIXELFORMATDESCRIPTOR), &pfd);
        if((pfd.dwFlags & PFD_STEREO) == 0) {
            stError("WinAPI, Quad Buffered stereo not supported");
        }
    }

    // ========= Now we share GL contexts =========
    // TODO (Kirill Gavrilov) choose better
    if(theSlave != NULL) {
        if(!wglShareLists(theSlave->hRC, hRC)) {
        //if(!wglShareLists(hRC, theSlave->hRC)) {
            stError("WinAPI, Can't share GL Rendering Contexts");
            return STWIN_ERROR_WIN32_GLRC_SHARE;
        }
    }

    if(!wglMakeCurrent(hDC, hRC)) { // Try To Activate The Rendering Context
        stError("WinAPI, Can't activate Master GL Rendering Context");
        return STWIN_ERROR_WIN32_GLRC_ACTIVATE;
    }
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

        // do we have a Rendering Context?
        if(hRC != NULL) {
            // are we able to release DC and RC Contexts?
            if(wglMakeCurrent(NULL, NULL) == FALSE) {
                // this is not a problem in most cases;
                // also this happens when wglMakeCurrent(NULL, NULL) called twice
                ///ST_DEBUG_LOG("WinAPI, FAILED to release DC and RC contexts");
            }

            // are We Able To Delete The RC?
            if(wglDeleteContext(hRC) == FALSE) {
                ST_DEBUG_LOG("WinAPI, FAILED to delete RC");
                stMutex.unlock();
                return false;
            } else {
                ST_DEBUG_LOG("WinAPI, Deleted RC");
                hRC = NULL;
            }
        }

        // are we able to Release DC
        if(hRC == NULL && hDC != NULL && hWindowGl != NULL) {
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
#endif

#endif // !__APPLE__
