/**
 * Copyright © 2012-2013 Kirill Gavrilov <kirill@sview.ru>
 *
 * StTests program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * StTests program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program.
 * If not, see <http://www.gnu.org/licenses/>.
 */

#include "StTestEmbed.h"

#include <StCore/StApplication.h>
#include <StCore/StWindow.h>

#include <StStrings/stConsole.h>
#include <StTemplates/StHandle.h>

#if(defined(__linux__) || defined(__linux))
    #include <X11/X.h>
    #include <X11/Xlib.h>
    #include <X11/Xatom.h>
    #include <X11/Xutil.h>
#endif

namespace {

#if(defined(_WIN32) || defined(__WIN32__))
    LRESULT WINAPI embedWindowProc(HWND   theWinHandle,
                                   UINT   theMsg,
                                   WPARAM theWParam,
                                   LPARAM theLParam) {
        switch(theMsg) {
            case WM_CLOSE: {
                exit(0);
            }
        }
        return DefWindowProc(theWinHandle, theMsg, theWParam, theLParam);
    }
#endif

};

#ifndef __APPLE__
bool StTestEmbed::createNative() {
    myParent = (StNativeWin_t )NULL;
    StRectI_t aRect;
    aRect.top()    = 128;
    aRect.bottom() = 128 + 400;
    aRect.left()   = 128;
    aRect.right()  = 128 + 400;
#if(defined(_WIN32) || defined(__WIN32__))
    WNDCLASSW aWinClass;
    stMemSet(&aWinClass, 0, sizeof(aWinClass));
    HINSTANCE anAppInst = GetModuleHandle(NULL);
    aWinClass.lpfnWndProc   = (WNDPROC )embedWindowProc;
    aWinClass.hInstance     = anAppInst;
    aWinClass.hIcon         = LoadIcon(NULL, IDI_WINLOGO);
    aWinClass.hCursor       = LoadCursor(NULL, IDC_ARROW);
    aWinClass.lpszClassName = L"DummyClass";
    if(!RegisterClassW(&aWinClass)) {
        st::cout << stostream_text("RegisterClass() failed:\nCannot register window class 'DummyClass'.\n");
        return false;
    }
    HWND aWin = CreateWindowW(L"DummyClass", L"DummyWindow",
                              WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
                              aRect.left(), aRect.top(), aRect.width(), aRect.height(), NULL, NULL, anAppInst, NULL);
    ShowWindow(aWin, TRUE);
    myParent = aWin;
    return true;
#elif(defined(__linux__) || defined(__linux))
    // open a connection to the X server
    Display* aDisplay = XOpenDisplay(NULL);
    if(aDisplay == NULL) {
        st::cout << stostream_text("XOpenDisplay() failed!\n");
        return false;
    }

    Window aWin = XCreateSimpleWindow(aDisplay, RootWindow(aDisplay, DefaultScreen(aDisplay)),
                                      aRect.left(), aRect.top(), aRect.width(), aRect.height(),
                                      0, 0, BlackPixel(aDisplay, DefaultScreen(aDisplay)));
    if(aWin == 0) {
        st::cout << stostream_text("XCreateSimpleWindow() failed!\n");
        XCloseDisplay(aDisplay);
        return false;
    }

    XSetStandardProperties(aDisplay, aWin, "DummyWindow", "DummyWindow",
                           None, NULL, 0, NULL);
    XSelectInput(aDisplay, aWin,
                 KeyPressMask | KeyReleaseMask | ButtonPressMask | ButtonReleaseMask | StructureNotifyMask);

    // handle close window event
    //XSetWMProtocols(aDisplay, aWin, &(stXDisplay->wndDestroyAtom), 1);

    // request the X window to be displayed on the screen
    XMapWindow(aDisplay, aWin);

    // flushes the output buffer
    XFlush(aDisplay);
    myParent = (void* )aWin;
    myDisplay = aDisplay;
    return true;
#else
    st::cout << stostream_text("StTestEmbed::createNative() not implemented on this platform!\n");
    return false;
#endif
}
#endif // __APPLE__

SV_THREAD_FUNCTION StTestEmbed::embedAppThread(void* thePtr) {
    StTestEmbed* aTestEmbed = static_cast<StTestEmbed* >(thePtr);
    aTestEmbed->embedAppLoop();
    return SV_THREAD_RETURN 0;
}

void StTestEmbed::embedAppLoop() {
    StHandle<StApplication> anApp = new StApplication(myParent);
    if(!anApp->open()) {
        return;
    }

    anApp->exec();
}

#ifndef __APPLE__
void StTestEmbed::nativeLoop() {
#if(defined(_WIN32) || defined(__WIN32__))
    MSG aMsg;
    while(GetMessage(&aMsg, NULL, 0, 0)) {
        TranslateMessage(&aMsg);
        DispatchMessage(&aMsg);
    }
    DestroyWindow(myParent);
#elif(defined(__linux__) || defined(__linux))
    XEvent anEvent;
    for(;;) {
        XNextEvent((Display* )myDisplay, &anEvent);
    }
    XUnmapWindow  ((Display* )myDisplay, (Window )myParent);
    XDestroyWindow((Display* )myDisplay, (Window )myParent);
#else
    st::cout << stostream_text("StTestEmbed::nativeLoop() not implemented on this platform!\n");
#endif
}
#endif // __APPLE__

void StTestEmbed::perform() {
    if(!createNative()) {
        return;
    }

    StThread(embedAppThread, this);
    nativeLoop();
}
