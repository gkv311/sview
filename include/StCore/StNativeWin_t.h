/**
 * Copyright © 2009 Kirill Gavrilov <kirill@sview.ru>
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

#ifndef __StNativeWin_t_h_
#define __StNativeWin_t_h_

#if(defined(_WIN32) || defined(__WIN32__))
    #include <windows.h>
    typedef HWND    StNativeWin_t;
#elif(defined(__APPLE__))
    #ifdef __OBJC__
        @class NSView;
    #else
        struct NSView;
    #endif
    typedef NSView* StNativeWin_t;
#else
    typedef void*   StNativeWin_t; // Window
#endif

#endif //__StNativeWin_t_h_
