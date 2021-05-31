/**
 * StCore, window system independent C++ toolkit for writing OpenGL applications.
 * Copyright © 2009-2014 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#ifndef __StNativeWin_t_h_
#define __StNativeWin_t_h_

#if defined(_WIN32)
    #include <windows.h>
    typedef HWND    StNativeWin_t;
#elif defined(__APPLE__)
    #ifdef __OBJC__
        @class NSView;
    #else
        struct NSView;
    #endif
    typedef NSView* StNativeWin_t;
#elif defined(__ANDROID__)
    //struct ANativeWindow;
    //typedef ANativeWindow* StNativeWin_t;
    class StAndroidGlue;
    typedef StAndroidGlue* StNativeWin_t;
#else
    typedef void*   StNativeWin_t; // Window
#endif

#endif //__StNativeWin_t_h_
