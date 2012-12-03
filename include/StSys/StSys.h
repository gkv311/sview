/**
 * Copyright © 2009, 2011 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#ifndef __StSys_h_
#define __StSys_h_

#if (defined(_WIN32) || defined(__WIN32__))
    #include <windows.h>
#endif

class ST_LOCAL StSys {

        public:

    enum {
        ST_SYSTEM_UNKNOWN,
        ST_SYSTEM_LINUX,
        ST_SYSTEM_WINDOWS_XP_MINUS,
        ST_SYSTEM_WINDOWS_VISTA_PLUS,
        ST_SYSTEM_MAC_OS,
    };

        private:

#if (defined(_WIN32) || defined(__WIN32__))
    static int getSystemEnumWin() {
        OSVERSIONINFOEX osvi; ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));
        osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
        if(    GetVersionEx((OSVERSIONINFO* )&osvi) != 0
            && osvi.dwPlatformId == VER_PLATFORM_WIN32_NT
            && osvi.dwMajorVersion > 5) {
            //
            // Windows Vista and newer
            return ST_SYSTEM_WINDOWS_VISTA_PLUS;
        } else {
            // Windows XP and older
            return ST_SYSTEM_WINDOWS_XP_MINUS;
        }
    }
#endif

        public:

    static int getSystemEnum() {
    #if (defined(_WIN32) || defined(__WIN32__))
        static const int winSys = getSystemEnumWin();
        return winSys;
    #elif (defined(__APPLE__))
        return ST_SYSTEM_MAC_OS;
    #elif (defined(__linux__) || defined(__linux))
        return ST_SYSTEM_LINUX;
    #else
        return ST_SYSTEM_UNKNOWN;
    #endif
    }

};

#endif // __StSys_h_
