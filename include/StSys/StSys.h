/**
 * Copyright © 2009-2013 Kirill Gavrilov <kirill@sview.ru>
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

    enum SystemType {
        SystemType_UNKNOWN,
        SystemType_LINUX,
        SystemType_WINDOWS,
        SystemType_MAC_OS,
    };

    enum SystemVersion {
    #if (defined(_WIN32) || defined(__WIN32__))
        SystemVersion_WinAncient,
        SystemVersion_XP,
        SystemVersion_Vista,
        SystemVersion_Win7,
        SystemVersion_Win8,
        SystemVersion_WinX,
    #endif
    };

        private:

#if (defined(_WIN32) || defined(__WIN32__))
    static SystemVersion getSystemVersionWin() {
        OSVERSIONINFOEX aVerInfo; ZeroMemory(&aVerInfo, sizeof(OSVERSIONINFOEX));
        aVerInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
        if(GetVersionEx((OSVERSIONINFO* )&aVerInfo) == 0
        || aVerInfo.dwPlatformId != VER_PLATFORM_WIN32_NT
        || aVerInfo.dwMajorVersion < 5) {
            return SystemVersion_WinAncient;
        }

        switch(aVerInfo.dwMajorVersion) {
            case 5: {
                // 5.0 = 2k
                // 5.1 = XP
                // 5.2 = Server 2003 / XP x86_64
                return SystemVersion_XP;
            }
            case 6: {
                switch(aVerInfo.dwMinorVersion) {
                    case 0:  return SystemVersion_Vista;
                    case 1:  return SystemVersion_Win7;
                    case 2:  return SystemVersion_Win8;
                    default: return SystemVersion_WinX;
                }
            }
            default: return SystemVersion_WinX;
        }
    }
#endif

        public:

    static SystemType getSystemEnum() {
    #if (defined(_WIN32) || defined(__WIN32__))
        return SystemType_WINDOWS;
    #elif (defined(__APPLE__))
        return SystemType_MAC_OS;
    #elif (defined(__linux__) || defined(__linux))
        return SystemType_LINUX;
    #else
        return SystemType_UNKNOWN;
    #endif
    }

#if (defined(_WIN32) || defined(__WIN32__))
    static bool isVistaPlus() {
        return getSystemVersionWin() >= SystemVersion_Vista;
    }

    static bool isWin8Plus() {
        return getSystemVersionWin() >= SystemVersion_Win8;
    }
#endif

};

#endif // __StSys_h_
