/**
 * Copyright © 2009-2015 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#ifndef __StSys_h_
#define __StSys_h_

#if defined(_WIN32)
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

#if defined(_WIN32)

#ifdef _MSC_VER
    // suppress GetVersionExW is deprecated warning
    #pragma warning(push)
    #pragma warning(disable : 4996)
#endif

    static SystemVersion getSystemVersionWin() {
        OSVERSIONINFOEXW aVerInfo; ZeroMemory(&aVerInfo, sizeof(OSVERSIONINFOEXW));
        aVerInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEXW);
        if(GetVersionExW((OSVERSIONINFOW* )&aVerInfo) == 0
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

#ifdef _MSC_VER
    #pragma warning(pop)
#endif

#endif

        public:

    static SystemType getSystemEnum() {
    #if defined(_WIN32)
        return SystemType_WINDOWS;
    #elif (defined(__APPLE__))
        return SystemType_MAC_OS;
    #elif (defined(__linux__) || defined(__linux))
        return SystemType_LINUX;
    #else
        return SystemType_UNKNOWN;
    #endif
    }

#if defined(_WIN32)
    static bool isWin8Plus() {
        return getSystemVersionWin() >= SystemVersion_Win8;
    }
#endif

};

#endif // __StSys_h_
