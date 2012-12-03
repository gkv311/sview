/**
 * Copyright © 2009-2012 Kirill Gavrilov <kirill@sview.ru>
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

#if(defined(_WIN32) || defined(__WIN32__))

#include <windows.h>

static LRESULT CALLBACK wndProcWrapper
        (HWND   in_hWnd,      // Handle For This Window
         UINT      uMsg,      // Message For This Window
         WPARAM  wParam,      // Additional Message Information
         LPARAM  lParam)      // Additional Message Information
{
    return DefWindowProc(in_hWnd, uMsg, wParam, lParam);
}

static bool wndRegisterClass(const StStringUtfWide& theClassName) {
    HINSTANCE hInstance = GetModuleHandle(NULL); // Holds The Instance Of The Application
    WNDCLASSW wndClass;  // Windows Class Structure
    wndClass.style         = CS_HREDRAW | CS_VREDRAW | CS_OWNDC; // Redraw On Size, And Own DC For Window.
    wndClass.lpfnWndProc   = (WNDPROC )wndProcWrapper;           // WndProc Handles Messages
    wndClass.cbClsExtra    = 0;                                  // No Extra Window Data
    wndClass.cbWndExtra    = 0;                                  // No Extra Window Data
    wndClass.hInstance     = hInstance;                          // Set The Instance
    wndClass.hIcon         = LoadIcon(hInstance, L"A");          // Load The Icon A
    wndClass.hCursor       = LoadCursor(NULL, IDC_ARROW);        // Load The Arrow Pointer
    wndClass.hbrBackground = NULL;                               // No Background Required For GL
    wndClass.lpszMenuName  = NULL;                               // No menu
    wndClass.lpszClassName = theClassName.toCString();           // Set The Class Name
    return (RegisterClassW(&wndClass) != 0);
}
#elif !(defined(__APPLE__))
    #include <GL/glx.h>
#endif

#if !(defined(__APPLE__))

bool testQuadBufferSupport() {
    // Firstly INIT core library!
#if(defined(_WIN32) || defined(__WIN32__))
    const StStringUtfWide QUAD_TEST_CLASS = L"StTESTQuadBufferWin";
    if(!wndRegisterClass(QUAD_TEST_CLASS)) {
        ST_DEBUG_LOG_AT("Fail to register class");
        return false;
    }
    HINSTANCE hInstance = GetModuleHandle(NULL); // Holds The Instance Of The Application
    HWND hTestWindowhMaster = CreateWindowExW(
            WS_EX_TOOLWINDOW | WS_EX_WINDOWEDGE | WS_EX_NOACTIVATE, // Extended Style For The Window
            QUAD_TEST_CLASS.toCString(),        // Class Name
            L"OpenGL Hardware Quad Buffer test",// Window Title
            WS_POPUP | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, // Window Style
            32, 32,                             // Left-Top position
            32, 32,                             // Width x Height
            NULL,                               // No Parent Window
            NULL,                               // No Menu
            hInstance,                          // Instance
            NULL);
    if(hTestWindowhMaster == NULL) {
        UnregisterClassW(QUAD_TEST_CLASS.toCString(), hInstance);
        return false;
    }

    HDC hDC = GetDC(hTestWindowhMaster);
    if(hDC == NULL) { // Did We Get A Device Context?
        ST_DEBUG_LOG_AT(L"WinAPI, Can't create Device Context for the entire screen");
        DestroyWindow(hTestWindowhMaster);
        UnregisterClassW(QUAD_TEST_CLASS.toCString(), hInstance);
        return false;
    }
    PIXELFORMATDESCRIPTOR pfd;
    memset(&pfd, 0, sizeof(PIXELFORMATDESCRIPTOR)); // zero out all fields
    pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_GDI | PFD_SUPPORT_OPENGL |
            PFD_DOUBLEBUFFER | PFD_STEREO;
    int pixelFormat = ChoosePixelFormat(hDC, &pfd);
    BOOL bSuccess = SetPixelFormat(hDC, pixelFormat, &pfd);
    HGLRC hRC = wglCreateContext(hDC);
    bSuccess = wglMakeCurrent(hDC, hRC);

    pixelFormat = GetPixelFormat(hDC);
    DescribePixelFormat(hDC, pixelFormat, sizeof(PIXELFORMATDESCRIPTOR), &pfd);
    bool isSupported = (pfd.dwFlags & PFD_STEREO) != 0;

    // clean up
    wglMakeCurrent(NULL, NULL);
    wglDeleteContext(hRC);
    if(hDC != NULL) {
        if(ReleaseDC(hTestWindowhMaster, hDC) == 0) {
            ST_DEBUG_LOG_AT(L"WinAPI, ReleaseDC(hTestWindowhMaster, hDC) FAILED");
        }
    }
    DestroyWindow(hTestWindowhMaster);
    UnregisterClassW(QUAD_TEST_CLASS.toCString(), hInstance);

    return isSupported;
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

SV_THREAD_FUNCTION testQBThreadFunction(void* outValue) {
    bool* outValueBool = (bool* )outValue;
    *outValueBool = testQuadBufferSupport();
    return SV_THREAD_RETURN 0;
}
