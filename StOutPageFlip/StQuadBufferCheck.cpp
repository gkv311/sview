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

#include "StQuadBufferCheck.h"

#include <StStrings/StLogger.h>
#include <StThreads/StCondition.h>
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
#elif !(defined(__APPLE__))
    #include <GL/glx.h>
#endif

#if !(defined(__APPLE__))

bool StQuadBufferCheck::testQuadBufferSupport() {
    // Firstly INIT core library!
#ifdef _WIN32
    HINSTANCE anAppInst = GetModuleHandleW(NULL); // Holds The Instance Of The Application
    const StStringUtfWide QUAD_TEST_CLASS = L"StTESTQuadBufferWin";
    if(!wndRegisterClass(anAppInst, QUAD_TEST_CLASS)) {
        ST_DEBUG_LOG_AT("Fail to register class");
        return false;
    }
    HWND aWindow = CreateWindowExW(WS_EX_TOOLWINDOW | WS_EX_WINDOWEDGE | WS_EX_NOACTIVATE,
                                   QUAD_TEST_CLASS.toCString(),
                                   L"GL Quad Buffer test",
                                   WS_POPUP | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
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
#elif(defined(__linux__) || defined(__linux))
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

namespace {

    static StCondition   THE_QB_INIT_EVENT(true);
    static volatile bool IS_QB_SUPPORTED = false;

    SV_THREAD_FUNCTION testQBThreadFunction(void* ) {
        IS_QB_SUPPORTED = StQuadBufferCheck::testQuadBufferSupport();
        THE_QB_INIT_EVENT.set();
        return SV_THREAD_RETURN 0;
    }

};

bool StQuadBufferCheck::isSupported() {
    THE_QB_INIT_EVENT.wait();
    return IS_QB_SUPPORTED;
}

void StQuadBufferCheck::initAsync() {
    if(!THE_QB_INIT_EVENT.check()) {
        return; // already called
    }

    // start and detach thread
    THE_QB_INIT_EVENT.reset();
    StThread aTestThread(testQBThreadFunction, NULL);
}
