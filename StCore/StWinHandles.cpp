/**
 * StCore, window system independent C++ toolkit for writing OpenGL applications.
 * Copyright © 2007-2014 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#ifndef __APPLE__

#include <StGL/StGLFunctions.h>
#include <StGL/StGLContext.h>
#include <StGLCore/StGLCore11Fwd.h>

#include "StWinHandles.h"
#include <StThreads/StThread.h>
#include <StStrings/StLogger.h>

#if defined(ST_HAVE_EGL) || defined(__ANDROID__)

StWinGlrc::StWinGlrc(EGLDisplay theDisplay,
                     const bool theDebugCtx,
                     int8_t     theGlDepthSize,
                     int8_t     theGlStencilSize)
: myDisplay(theDisplay),
  myConfig(NULL),
  myRC(EGL_NO_CONTEXT) {
    if(theDisplay == EGL_NO_DISPLAY) {
        return;
    }

    EGLint aVerMajor = 0; EGLint aVerMinor = 0;
    if(eglInitialize(myDisplay, &aVerMajor, &aVerMinor) != EGL_TRUE) {
        ST_ERROR_LOG("EGL, FAILED to initialize Display");
        return;
    }

    ST_DEBUG_LOG("EGL info\n"
               + "  Version:     " + aVerMajor + "." + aVerMinor + " (" + eglQueryString(myDisplay, EGL_VERSION) + ")\n"
               + "  Vendor:      " + eglQueryString(myDisplay, EGL_VENDOR) + "\n"
               + "  Client APIs: " + eglQueryString(myDisplay, EGL_CLIENT_APIS) + "\n"
               + "  Extensions:  " + eglQueryString(myDisplay, EGL_EXTENSIONS));

    EGLint aConfigAttribs[] = {
        EGL_RED_SIZE,   8,
        EGL_GREEN_SIZE, 8,
        EGL_BLUE_SIZE,  8,
        EGL_ALPHA_SIZE, 0,
        EGL_DEPTH_SIZE,   theGlDepthSize,
        EGL_STENCIL_SIZE, theGlStencilSize,
    #if defined(GL_ES_VERSION_2_0)
        EGL_CONFORMANT,      EGL_OPENGL_ES2_BIT,
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
    #else
        EGL_CONFORMANT,      EGL_OPENGL_BIT,
        EGL_RENDERABLE_TYPE, EGL_OPENGL_BIT,
    #endif
        EGL_NONE
    };

    EGLint aNbConfigs = 0;
    if(eglChooseConfig(myDisplay, aConfigAttribs, &myConfig, 1, &aNbConfigs) != EGL_TRUE
    || myConfig == NULL) {
        if(theGlDepthSize <= 16) {
            ST_ERROR_LOG("EGL, eglChooseConfig FAILED");
            return;
        }

        eglGetError();
        aConfigAttribs[4 * 2 + 1] = 16; // try config with smaller depth buffer
        aConfigAttribs[5 * 2 + 1] = 0;
        if(eglChooseConfig(myDisplay, aConfigAttribs, &myConfig, 1, &aNbConfigs) != EGL_TRUE
        || myConfig == NULL) {
            ST_ERROR_LOG("EGL, eglChooseConfig FAILED");
            return;
        }
    }

    /*EGLenum aEglApi = eglQueryAPI();
    switch(aEglApi) {
        case EGL_OPENGL_ES_API: ST_DEBUG_LOG("EGL API: OpenGL ES\n"); break;
        case EGL_OPENGL_API:    ST_DEBUG_LOG("EGL API: OpenGL\n");    break;
        case EGL_OPENVG_API:    ST_DEBUG_LOG("EGL API: OpenNVG\n");   break;
        case EGL_NONE:          ST_DEBUG_LOG("EGL API: NONE\n");      break;
    }*/

#if defined(GL_ES_VERSION_2_0)
    if(eglBindAPI(EGL_OPENGL_ES_API) != EGL_TRUE) {
        ST_ERROR_LOG("EGL, EGL_OPENGL_ES_API is unavailable!");
        return;
    }
#else
    if(eglBindAPI(EGL_OPENGL_API) != EGL_TRUE) {
        ST_ERROR_LOG("EGL, EGL_OPENGL_API is unavailable!");
        return;
    }
#endif

    #define ST_EGL_CONTEXT_MAJOR_VERSION_KHR                      0x3098
    #define ST_EGL_CONTEXT_MINOR_VERSION_KHR                      0x30FB
    #define ST_EGL_CONTEXT_FLAGS_KHR                              0x30FC
    #define ST_EGL_CONTEXT_OPENGL_PROFILE_MASK_KHR                0x30FD
    #define ST_EGL_CONTEXT_OPENGL_RESET_NOTIFICATION_STRATEGY_KHR 0x31BD

    // for EGL_CONTEXT_OPENGL_RESET_NOTIFICATION_STRATEGY_KHR
    #define ST_EGL_NO_RESET_NOTIFICATION_KHR 0x31BE
    #define ST_EGL_LOSE_CONTEXT_ON_RESET_KHR 0x31BF

    // for EGL_CONTEXT_FLAGS_KHR
    #define ST_EGL_CONTEXT_OPENGL_DEBUG_BIT_KHR                 0x00000001
    #define ST_EGL_CONTEXT_OPENGL_FORWARD_COMPATIBLE_BIT_KHR    0x00000002
    #define ST_EGL_CONTEXT_OPENGL_ROBUST_ACCESS_BIT_KHR         0x00000004

    // for EGL_CONTEXT_OPENGL_PROFILE_MASK_KHR
    #define ST_EGL_CONTEXT_OPENGL_CORE_PROFILE_BIT_KHR          0x00000001
    #define ST_EGL_CONTEXT_OPENGL_COMPATIBILITY_PROFILE_BIT_KHR 0x00000002

    // for EGL_KHR_context_flush_control
    #define ST_EGL_CONTEXT_RELEASE_BEHAVIOR_KHR       0x2097
    #define ST_EGL_CONTEXT_RELEASE_BEHAVIOR_NONE_KHR  0x0000
    #define ST_EGL_CONTEXT_RELEASE_BEHAVIOR_FLUSH_KHR 0x2098

    const char* anEglExts = eglQueryString(myDisplay, EGL_EXTENSIONS);
    if(StGLContext::stglCheckExtension(anEglExts, "EGL_KHR_create_context")) {
        const bool khrFlushControl = StGLContext::stglCheckExtension(anEglExts, "EGL_KHR_context_flush_control");
        const EGLint anEglCtxAttribs[] = {
            ST_EGL_CONTEXT_FLAGS_KHR, theDebugCtx ? ST_EGL_CONTEXT_OPENGL_DEBUG_BIT_KHR : 0,
        #if defined(GL_ES_VERSION_2_0)
            EGL_CONTEXT_CLIENT_VERSION, 2,
        #endif
            //EGL_CONTEXT_OPENGL_PROFILE_MASK_KHR, ST_EGL_CONTEXT_OPENGL_COMPATIBILITY_PROFILE_BIT_KHR,
            khrFlushControl ? ST_EGL_CONTEXT_RELEASE_BEHAVIOR_KHR : 0, khrFlushControl ? ST_EGL_CONTEXT_RELEASE_BEHAVIOR_NONE_KHR : 0,
            EGL_NONE
        };

        myRC = eglCreateContext(myDisplay, myConfig, EGL_NO_CONTEXT, anEglCtxAttribs);
    }

    if(myRC == EGL_NO_CONTEXT) {
    #if defined(GL_ES_VERSION_2_0)
        EGLint anEglCtxAttribs[] = {
            EGL_CONTEXT_CLIENT_VERSION, 2,
            EGL_NONE
        };
    #else
        EGLint* anEglCtxAttribs = NULL;
    #endif
        myRC = eglCreateContext(myDisplay, myConfig, EGL_NO_CONTEXT, anEglCtxAttribs);
    /*#if defined(GL_ES_VERSION_2_0)
        if(myRC == EGL_NO_CONTEXT) {
            myRC = eglCreateContext(myDisplay, myConfig, EGL_NO_CONTEXT, NULL);
            if(myRC != EGL_NO_CONTEXT) {
                ST_ERROR_LOG("EGL, eglCreateContext FAILED when ES 2.0 is requested!");
            }
        }
    #endif*/
    }

    if(myRC == EGL_NO_CONTEXT) {
        ST_ERROR_LOG("EGL, eglCreateContext FAILED");
    }
}

StWinGlrc::~StWinGlrc() {
    if(myRC != EGL_NO_CONTEXT) {
        if(eglMakeCurrent(myDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT) != EGL_TRUE) {
            ST_DEBUG_LOG("EGL, FAILED to release OpenGL context");
        }
        eglDestroyContext(myDisplay, myRC);
        myRC = EGL_NO_CONTEXT;
    }

    if(myDisplay != EGL_NO_DISPLAY) {
        if(eglTerminate(myDisplay) != EGL_TRUE) {
            ST_ERROR_LOG("EGL, eglTerminate FAILED");
        }
        myDisplay = EGL_NO_DISPLAY;
    }
}

bool StWinGlrc::makeCurrent(EGLSurface theSurface) {
    if(myRC == EGL_NO_CONTEXT) {
        return false;
    }

    // some drivers crash when trying to make current EGL_NO_SURFACE within proper EGLContext
    EGLContext aCtx = theSurface != EGL_NO_SURFACE
                    ? myRC
                    : EGL_NO_CONTEXT;
    return eglMakeCurrent(myDisplay, theSurface, theSurface, aCtx) == EGL_TRUE;
}

#elif defined(_WIN32)

static PIXELFORMATDESCRIPTOR THE_PIXELFRMT_DOUBLE = {
    sizeof(PIXELFORMATDESCRIPTOR),   // Size Of This Pixel Format Descriptor
    1,                               // Version Number
    PFD_DRAW_TO_WINDOW |             // Format Must Support Window
    PFD_SUPPORT_OPENGL |             // Format Must Support OpenGL
    PFD_DOUBLEBUFFER,                // Must Support Double Buffering
    PFD_TYPE_RGBA,                   // Request An RGBA Format
    24,                              // Select Our Color Depth
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

  // GLX_ARB_create_context
#ifndef GLX_CONTEXT_MAJOR_VERSION_ARB
    #define GLX_CONTEXT_DEBUG_BIT_ARB         0x00000001
    #define GLX_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB 0x00000002
    #define GLX_CONTEXT_MAJOR_VERSION_ARB     0x2091
    #define GLX_CONTEXT_MINOR_VERSION_ARB     0x2092
    #define GLX_CONTEXT_FLAGS_ARB             0x2094

    // GLX_ARB_create_context_profile
    #define GLX_CONTEXT_CORE_PROFILE_BIT_ARB  0x00000001
    #define GLX_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB 0x00000002
    #define GLX_CONTEXT_PROFILE_MASK_ARB      0x9126
#endif

typedef GLXContext (*glXCreateContextAttribsARB_t)(Display* , GLXFBConfig , GLXContext , Bool , const int* );

StWinGlrc::StWinGlrc(StHandle<StXDisplay>& theDisplay,
                     const bool            theDebugCtx)
: myDisplay(theDisplay->hDisplay),
  myRC(NULL) {
    const char* aGlxExts = glXQueryExtensionsString(theDisplay->hDisplay, DefaultScreen(theDisplay->hDisplay));
    if(StGLContext::stglCheckExtension(aGlxExts, "GLX_ARB_create_context_profile")) {
        const bool khrFlushControl = StGLContext::stglCheckExtension(aGlxExts, "GLX_ARB_create_context_profile");
        glXCreateContextAttribsARB_t aCreateCtxFunc = (glXCreateContextAttribsARB_t )glXGetProcAddress((const GLubyte* )"glXCreateContextAttribsARB");

        int aCtxAttribs[] = {
            //GLX_CONTEXT_MAJOR_VERSION_ARB, 2,
            //GLX_CONTEXT_MINOR_VERSION_ARB, 0,
            //GLX_CONTEXT_PROFILE_MASK_ARB, GLX_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB,
            GLX_CONTEXT_FLAGS_ARB, theDebugCtx ? GLX_CONTEXT_DEBUG_BIT_ARB : 0,
            khrFlushControl ? GLX_CONTEXT_RELEASE_BEHAVIOR_ARB : 0, khrFlushControl ? GLX_CONTEXT_RELEASE_BEHAVIOR_NONE_ARB : 0,
            None
        };

        myRC = aCreateCtxFunc(theDisplay->hDisplay, theDisplay->FBCfg, 0, True, aCtxAttribs);
    }

    if(myRC == NULL) {
        // fallback compatibility mode
        myRC = glXCreateContext(theDisplay->hDisplay, theDisplay->hVisInfo, None, true);
    }
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
  hDC(NULL)
#elif defined(__ANDROID__)
: hWindowGl(NULL)
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
  isRecXRandrEvents(false)
#endif
#if defined(ST_HAVE_EGL) || defined(__ANDROID__)
  ,
  eglSurface(EGL_NO_SURFACE)
#endif
{
    //
}

StWinHandles::~StWinHandles() {
    close();
}

void StWinHandles::glSwap() {
#if defined(ST_HAVE_EGL) || defined(__ANDROID__)
    if(eglSurface != EGL_NO_SURFACE) {
        eglSwapBuffers(hRC->getDisplay(), eglSurface);
    }
#elif defined(_WIN32)
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
#if defined(ST_HAVE_EGL) || defined(__ANDROID__)
    if(eglSurface != EGL_NO_SURFACE
    && !hRC.isNull()) {
        return hRC->makeCurrent(eglSurface);
    }
#elif defined(_WIN32)
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
                                  const int        theStencilSize,
                                  const bool       theIsQuadStereo,
                                  const bool       theDebugCtx) {
    (void )theRect;
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

    HGLRC aRendCtx = NULL;
    {
      PIXELFORMATDESCRIPTOR aPixFrmtDesc = THE_PIXELFRMT_DOUBLE;
      aPixFrmtDesc.cDepthBits   = (BYTE )theDepthSize;
      aPixFrmtDesc.cStencilBits = (BYTE )theStencilSize;
      if(theIsQuadStereo) {
          aPixFrmtDesc.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_GDI | PFD_SUPPORT_OPENGL
                               | PFD_DOUBLEBUFFER | PFD_STEREO;
      }

      HMODULE aModule = GetModuleHandleW(NULL);
      hWinTmp = CreateWindowExW(WS_EX_TOOLWINDOW | WS_EX_WINDOWEDGE | WS_EX_NOACTIVATE,
                                ClassTmp.toCString(), L"TmpWnd",
                                WS_POPUP | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_DISABLED,
                                // always create temporary window on main screen
                                // to workaround sporadic bugs (access violation) in AMD Catalyst drivers
                                2, 2, 4, 4, //theRect.left() + 2, theRect.top() + 2, 4, 4,
                                NULL, NULL, aModule, NULL);
      ST_GL_ERROR_CHECK(hWinTmp != NULL, STWIN_ERROR_WIN32_GLDC,
                        "WinAPI, Temporary window creation error");

      HDC aDevCtxTmp = GetDC(hWinTmp);
      int aPixFrmtIdTmp = ChoosePixelFormat(aDevCtxTmp, &aPixFrmtDesc);
      ST_GL_ERROR_CHECK(aPixFrmtIdTmp != 0, STWIN_ERROR_WIN32_PIXELFORMATF,
                        "WinAPI, Can't find a suitable PixelFormat for Tmp");

      ST_GL_ERROR_CHECK(SetPixelFormat(aDevCtxTmp, aPixFrmtIdTmp, &aPixFrmtDesc),
                        STWIN_ERROR_WIN32_PIXELFORMATS, "WinAPI, Can't set the PixelFormat for Master");
      StWinGlrcH aRendCtxTmp = new StWinGlrc(aDevCtxTmp, NULL);
      ST_GL_ERROR_CHECK(aRendCtxTmp->isValid(),
                        STWIN_ERROR_WIN32_GLRC_CREATE, "WinAPI, Can't create GL Rendering Context");
      ST_GL_ERROR_CHECK(aRendCtxTmp->makeCurrent(aDevCtxTmp),
                        STWIN_ERROR_WIN32_GLRC_ACTIVATE, "WinAPI, Can't activate Tmp GL Rendering Context");

      StGLContext aCtx(false);
      ST_GL_ERROR_CHECK(aCtx.stglInit(),
                        STWIN_ERROR_WIN32_GLRC_ACTIVATE, "WinAPI, Broken Tmp GL Rendering Context");

      int aPixFrmtId = 0;
      if(aCtx.extAll->wglChoosePixelFormatARB != NULL) {
          for(bool toTryQuadBuffer = theIsQuadStereo;;) {
              const int aPixAttribs[] = {
                  WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
                  WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
                  WGL_DOUBLE_BUFFER_ARB,  GL_TRUE,
                  WGL_STEREO_ARB,         toTryQuadBuffer ? GL_TRUE : GL_FALSE,
                  WGL_PIXEL_TYPE_ARB,     WGL_TYPE_RGBA_ARB,
                  //WGL_SAMPLE_BUFFERS_ARB, 1,
                  //WGL_SAMPLES_ARB,        8,
                  // WGL_CONTEXT_ROBUST_ACCESS_BIT_ARB       0x00000004
                  WGL_COLOR_BITS_ARB,     24,
                  WGL_DEPTH_BITS_ARB,     theDepthSize,
                  WGL_STENCIL_BITS_ARB,   theStencilSize,
                  0, 0,
              };
              unsigned int aFrmtsNb = 0;
              aCtx.extAll->wglChoosePixelFormatARB(hDC, aPixAttribs, NULL, 1, &aPixFrmtId, &aFrmtsNb);
              if(toTryQuadBuffer && aPixFrmtId == 0) {
                  toTryQuadBuffer = false;
                  continue;
              }
              if(theSlave != NULL) {
                  int aPixFrmtIdSlave = 0;
                  aCtx.extAll->wglChoosePixelFormatARB(theSlave->hDC, aPixAttribs, NULL, 1, &aPixFrmtIdSlave, &aFrmtsNb);
                  if(aPixFrmtIdSlave != aPixFrmtId) {
                      ST_ERROR_LOG("Slave window returns another pixel format! Try to ignore...");
                  }
              }
              break;
          }
      } else {
          aPixFrmtId = ChoosePixelFormat(hDC, &aPixFrmtDesc);
          if(theSlave != NULL
          && ChoosePixelFormat(theSlave->hDC, &aPixFrmtDesc) != aPixFrmtId) {
              ST_ERROR_LOG("Slave window returns another pixel format! Try to ignore...");
          }
      }
      ST_GL_ERROR_CHECK(aPixFrmtId != 0, STWIN_ERROR_WIN32_PIXELFORMATF,
                        "WinAPI, Can't find a suitable PixelFormat for Master");
      DescribePixelFormat(hDC, aPixFrmtId, sizeof(PIXELFORMATDESCRIPTOR), &aPixFrmtDesc);
      if(theIsQuadStereo) {
          if((aPixFrmtDesc.dwFlags & PFD_STEREO) == 0) {
              ST_ERROR_LOG("WinAPI, Quad Buffered stereo is not supported");
          }
      }
      ST_GL_ERROR_CHECK(SetPixelFormat(hDC, aPixFrmtId, &aPixFrmtDesc),
                        STWIN_ERROR_WIN32_PIXELFORMATS, "WinAPI, Can't set the PixelFormat for Master");
      ST_GL_ERROR_CHECK(theSlave == NULL || SetPixelFormat(theSlave->hDC, aPixFrmtId, &aPixFrmtDesc),
                        STWIN_ERROR_WIN32_PIXELFORMATS, "WinAPI, Can't set the PixelFormat for Slave");
      if(aCtx.extAll->wglCreateContextAttribsARB != NULL) {
          // Beware! NVIDIA drivers reject context creation when WGL_CONTEXT_PROFILE_MASK_ARB are specified
          // but not WGL_CONTEXT_MAJOR_VERSION_ARB/WGL_CONTEXT_MINOR_VERSION_ARB
          const int aCtxAttribs[] = {
              //WGL_CONTEXT_MAJOR_VERSION_ARB, 3,
              //WGL_CONTEXT_MINOR_VERSION_ARB, 2,
              //WGL_CONTEXT_PROFILE_MASK_ARB,  WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB, //WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
              WGL_CONTEXT_FLAGS_ARB, theDebugCtx ? WGL_CONTEXT_DEBUG_BIT_ARB : 0,
              aCtx.khrFlushControl ? WGL_CONTEXT_RELEASE_BEHAVIOR_ARB : 0, aCtx.khrFlushControl ? WGL_CONTEXT_RELEASE_BEHAVIOR_NONE_ARB : 0,
              0, 0
          };

          aRendCtx = aCtx.extAll->wglCreateContextAttribsARB(hDC, NULL, aCtxAttribs);
      }

      aRendCtxTmp.nullify();
      destroyWindow(hWinTmp);
    }

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
#if defined(ST_HAVE_EGL) || defined(__ANDROID__)
    // GL context is created beforehand for EGL
    ST_GL_ERROR_CHECK(!hRC.isNull() && hRC->isValid(),
                      STWIN_ERROR_X_GLRC_CREATE, "EGL, could not create rendering context for Master");

#if defined(__ANDROID__)
    EGLint aFormat = 0;
    eglGetConfigAttrib(hRC->getDisplay(), hRC->getConfig(), EGL_NATIVE_VISUAL_ID, &aFormat);
    ANativeWindow_setBuffersGeometry(hWindowGl, 0, 0, aFormat);
#endif

    eglSurface = eglCreateWindowSurface(hRC->getDisplay(), hRC->getConfig(), hWindowGl, NULL);
    if(theSlave != NULL) {
        theSlave->hRC = hRC;
        theSlave->eglSurface = eglCreateWindowSurface(hRC->getDisplay(), hRC->getConfig(), theSlave->hWindowGl, NULL);

        // bind the rendering context to the window
        ST_GL_ERROR_CHECK(hRC->makeCurrent(theSlave->eglSurface),
                          STWIN_ERROR_X_GLRC_CREATE, "EGL, Can't activate Slave GL Rendering Context");
    }

    // bind the rendering context to the window
    ST_GL_ERROR_CHECK(hRC->makeCurrent(eglSurface),
                      STWIN_ERROR_X_GLRC_CREATE, "EGL, Can't activate Master GL Rendering Context");
    return STWIN_INIT_SUCCESS;
#else // GLX
    hRC = new StWinGlrc(stXDisplay, theDebugCtx);
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
#endif // GLX or EGL
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

#if defined(ST_HAVE_EGL) || defined(__ANDROID__)
    if(!hRC.isNull()) {
        if(eglSurface != EGL_NO_SURFACE) {
            hRC->makeCurrent(EGL_NO_SURFACE);
            eglDestroySurface(hRC->getDisplay(), eglSurface);
            eglSurface = EGL_NO_SURFACE;
        }
    }
#endif

    // release active context
    hRC.nullify();

#if defined(__ANDROID__)
    //
#else
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

#endif
    return true;
}

#ifdef _WIN32

namespace {
    static StAtomic<int32_t> ST_CLASS_COUNTER(0);
    static LRESULT CALLBACK wndProcDummy(HWND theWin, UINT theMsg, WPARAM theParamW, LPARAM theParamL) {
        return DefWindowProcW(theWin, theMsg, theParamW, theParamL);
    }
}

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

    if(!registerClass(aNameTmp, wndProcDummy)) {
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

#elif defined(__ANDROID__)
    //
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
