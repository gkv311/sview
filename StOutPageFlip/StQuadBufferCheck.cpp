/**
 * StOutPageFlip, class providing stereoscopic output for Shutter Glasses displays using StCore toolkit.
 * Copyright © 2009-2016 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#include "StQuadBufferCheck.h"

#include <StStrings/StLogger.h>
#include <StThreads/StThread.h>

#ifdef _WIN32

#include <windows.h>

static LRESULT CALLBACK wndProcWrapper(HWND theWnd, UINT theMsg, WPARAM theParamW, LPARAM theParamL) {
    return DefWindowProcW(theWnd, theMsg, theParamW, theParamL);
}

static bool wndRegisterClass(HINSTANCE              theInstance,
                             const StStringUtfWide& theClassName) {
    WNDCLASSW aClass;
    aClass.style         = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    aClass.lpfnWndProc   = (WNDPROC )wndProcWrapper;
    aClass.cbClsExtra    = 0;
    aClass.cbWndExtra    = 0;
    aClass.hInstance     = theInstance;
    aClass.hIcon         = LoadIconW(theInstance, L"A");
    aClass.hCursor       = LoadCursor(NULL, IDC_ARROW);
    aClass.hbrBackground = NULL;
    aClass.lpszMenuName  = NULL;
    aClass.lpszClassName = theClassName.toCString();
    return (RegisterClassW(&aClass) != 0);
}
#elif !defined(__APPLE__) && !defined(ST_HAVE_EGL)
    // exclude modern definitions and system-provided glext.h, should be defined before gl.h inclusion
    #ifndef GL_GLEXT_LEGACY
        #define GL_GLEXT_LEGACY
    #endif
    #ifndef GLX_GLXEXT_LEGACY
        #define GLX_GLXEXT_LEGACY
    #endif
    #include <GL/glx.h>
#endif

#if !defined(__APPLE__)

bool StQuadBufferCheck::testQuadBufferSupport() {
#ifdef ST_HAVE_EGL
    return false; // unsupported at all!
#elif defined(_WIN32)
    HINSTANCE anAppInst = GetModuleHandleW(NULL); // Holds The Instance Of The Application
    const StStringUtfWide QUAD_TEST_CLASS = L"StTESTQuadBufferWin";
    if(!wndRegisterClass(anAppInst, QUAD_TEST_CLASS)) {
        ST_DEBUG_LOG_AT("Fail to register class");
        return false;
    }
    HWND aWindow = CreateWindowExW(WS_EX_TOOLWINDOW | WS_EX_WINDOWEDGE | WS_EX_NOACTIVATE,
                                   QUAD_TEST_CLASS.toCString(),
                                   L"GL Quad Buffer test",
                                   WS_POPUP | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_DISABLED,
                                   32, 32, 32, 32,
                                   NULL, NULL, anAppInst, NULL);
    if(aWindow == NULL) {
        UnregisterClassW(QUAD_TEST_CLASS.toCString(), anAppInst);
        return false;
    }

    HDC aDevCtx = GetDC(aWindow);
    if(aDevCtx == NULL) { // Did We Get A Device Context?
        ST_DEBUG_LOG_AT(L"WinAPI, Can't create Device Context for the entire screen");
        DestroyWindow(aWindow);
        UnregisterClassW(QUAD_TEST_CLASS.toCString(), anAppInst);
        return false;
    }
    PIXELFORMATDESCRIPTOR aPixelFormat;
    memset(&aPixelFormat, 0, sizeof(PIXELFORMATDESCRIPTOR)); // zero out all fields
    aPixelFormat.nSize = sizeof(PIXELFORMATDESCRIPTOR);
    aPixelFormat.nVersion = 1;
    aPixelFormat.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_GDI | PFD_SUPPORT_OPENGL
                         | PFD_DOUBLEBUFFER   | PFD_STEREO;
    const int aPixelFormatId = ChoosePixelFormat(aDevCtx, &aPixelFormat);
    DescribePixelFormat(aDevCtx, aPixelFormatId, sizeof(PIXELFORMATDESCRIPTOR), &aPixelFormat);

    // clean up
    if(ReleaseDC(aWindow, aDevCtx) == 0) {
        ST_DEBUG_LOG_AT(L"WinAPI, ReleaseDC(aWindow, aDevCtx) FAILED");
    }
    DestroyWindow(aWindow);
    UnregisterClassW(QUAD_TEST_CLASS.toCString(), anAppInst);

    return (aPixelFormat.dwFlags & PFD_STEREO) != 0;
#elif defined(__linux__)
    Display* hDisplay = XOpenDisplay(NULL); // get first display on server from DISPLAY in env
    if(hDisplay == NULL) {
        ST_DEBUG_LOG_AT("X: could not open display");
        return false;
    }

    // make sure OpenGL's GLX extension supported
    int dummy = 0;
    if(!glXQueryExtension(hDisplay, &dummy, &dummy)) {
        ST_DEBUG_LOG_AT("X: server has no OpenGL GLX extension");
        return false;
    }

    static int quadBuff[] = {
        GLX_RGBA,
        GLX_DEPTH_SIZE, 16,
        GLX_DOUBLEBUFFER,
        GLX_STEREO,
        None
    };

    // find an appropriate visual
    XVisualInfo* vi = glXChooseVisual(hDisplay, DefaultScreen(hDisplay), quadBuff);
    return vi != NULL;
#endif
}

#endif // !__APPLE__
