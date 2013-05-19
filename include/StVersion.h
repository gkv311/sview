/**
 * This is a header for version structure in 'Ubuntu-style' (Year.Month).
 * Copyright © 2008-2009 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#ifndef __StVersion_h_
#define __StVersion_h_

#ifdef ST_HAVE_STCONFIG
    #include <stconfig.conf>
#else
    #ifndef SVIEW_SDK_VER_STATUS
        #define SVIEW_SDK_VER_STATUS RELEASE
    #endif
    #ifndef SVIEW_SDK_VERSION_AUTO
        #define SVIEW_SDK_VERSION_AUTO
    #endif
#endif

enum {
    DEV = 0,
    DEVELOPMENT_RELEASE = 0,
    ALPHA = 1,
    BETA = 2,
    RC = 3,
    RELEASE_CANDIDATE = 3,
    RELEASE = 4,
};

#define __YEAR__ ((((__DATE__ [7]-'0')*10+(__DATE__ [8]-'0'))*10+(__DATE__ [9]-'0'))*10+(__DATE__ [10]-'0'))

// Month: 0 - 11
#define __MONTH__ (__DATE__ [2] == 'n' ? (__DATE__ [1] == 'a' ? 0 : 5) \
              : __DATE__ [2] == 'b' ? 1 \
              : __DATE__ [2] == 'r' ? (__DATE__ [0] == 'M' ? 2 : 3) \
              : __DATE__ [2] == 'y' ? 4 \
              : __DATE__ [2] == 'l' ? 6 \
              : __DATE__ [2] == 'g' ? 7 \
              : __DATE__ [2] == 'p' ? 8 \
              : __DATE__ [2] == 't' ? 9 \
              : __DATE__ [2] == 'v' ? 10 : 11)

#define __DAY__ ((__DATE__ [4]==' ' ? 0 : __DATE__ [4]-'0')*10+(__DATE__[5]-'0'))

#ifdef _WIN32
    //FILEFLAGS VS_DEBUG | VS_PRIVATEBUILD | VS_SPECIALBUILD | VS_PRERELEASE | VS_PATCHED
    #ifndef __ST_DEBUG__
        #define ST_WIN32_FILEFLAGS FILEFLAGS VS_FF_DEBUG
    #else
        #define ST_WIN32_FILEFLAGS
    #endif
#endif

// just empty version here
// should be redefined in stconfig.conf
#ifndef SVIEW_SDK_VERSION
    #define SVIEW_SDK_VERSION 10, 1, 0, 0
#endif

#ifndef SVIEW_SDK_VER_STRING
    #define SVIEW_SDK_VER_STRING "10.01dev0"
#endif

#ifndef SVIEW_SDK_VER_STATUS
    #define SVIEW_SDK_VER_STATUS DEVELOPMENT_RELEASE
#endif

/**
 * Simple version structure with 'Ubuntu-style' versioning rules,
 * means Year.Month of release is version of release!
 */
struct StVersion {

    int year;
    int month;

    int rStatus;
    int rSubVer;
};

#ifndef ST_RESOURCE_COMPILER

#include <StStrings/StLogger.h>
#include <StStrings/stUtfTools.h>
#include <time.h>

class ST_LOCAL StVersionInfo {

        private:

    StVersion ver;

        private:

    enum {
        TH_SUB = 2000
    };

        public:

    static StString getSDKVersionString() {
        return getSDKVersionInfo().toString();
    }

    static StVersionInfo getSDKVersionInfo() {
        return StVersionInfo(getSDKVersion());
    }

    static StVersion getSDKVersion() {
        // TODO
        int iVer[4] = {
        #ifdef SVIEW_SDK_VERSION_AUTO
            __YEAR__,
            (__MONTH__) + 1,
            SVIEW_SDK_VER_STATUS,
            __DAY__
        #else
            SVIEW_SDK_VERSION
        #endif
        };

        ///int iVer[4] = {SVIEW_SDK_VERSION};
        StVersion stVersion;
        stVersion.year = iVer[0];
        stVersion.month = iVer[1];
        stVersion.rStatus = iVer[2];
        stVersion.rSubVer = iVer[3];
        return stVersion;
    }

    /**
     * @return true if no errors.
     */
#if(defined(__ST_TIMEBOMB__))
    static bool checkTimeBomb(const StString& ) {
        return true;
    }
#else
    static bool checkTimeBomb(const StString& theTitle) {
        StVersionInfo aVer = getSDKVersionInfo();
        if(aVer.getReleaseStatus() == DEVELOPMENT_RELEASE) {
            time_t rawtime;
            struct tm* timeinfo;
            time(&rawtime);
            timeinfo = localtime(&rawtime);
            StVersionInfo test = StVersionInfo(1900 + timeinfo->tm_year, timeinfo->tm_mon + 1, DEVELOPMENT_RELEASE, 0);
            aVer++; aVer++;
            if(test > aVer) {
                stError(StString()
                    + "You running alpha version of the '"
                    + theTitle + "'.\n"
                    + "Sorry but time for testing this build is expired.\n"
                    + "You should update program from official site (www.sview.ru)"
                );
                return false;
            }
        }
        return true;
    }
#endif // __ST_TIMEBOMB__

    /**
     * Simple version constructor.
     * @param year (const int& ) - put year here;
     * @param month (const int& ) - put month here;
     * @param rstatus (const int& ) - release state (use enum);
     * @param rsubver (const int& ) - subversion.
     */
    StVersionInfo(const int& year, const int& month, const int& rstatus, const int& rsubver) {
        setYear(year);
        setMonth(month);
        ver.rStatus = rstatus;
        ver.rSubVer = rsubver;
    }

    /**
     * Simple version constructor.
     * @param version (Version ) - pur Version structure;
     */
    StVersionInfo(const StVersion& version) {
        setVersion(version);
    }

    StVersion& getVersion() {
        return ver;
    }

    void setVersion(const StVersion& version) {
        ver = version;
        setYear(version.year);
        setMonth(version.month);
    }

    int getYear() const {
        return ver.year;
    }

    void setYear(const int& year) {
        ver.year = (year > TH_SUB) ? year : (TH_SUB + year);
    }

    int getMonth() const {
        return ver.month;
    }

    void setMonth(const int& month) {
        ver.month = (month < 1) ? 12 : ( (month > 12) ? 1 : month );
    }

    int getReleaseStatus() const {
        return ver.rStatus;
    }

    void setReleaseStatus(const int& rstatus) {
        ver.rStatus = rstatus;
    }

    int getSubVersion() const {
        return ver.rSubVer;
    }

    void setSubVersion(const int& rsubver) {
        ver.rSubVer = rsubver;
    }

    StString toString() const {
        StString subVersion;
        switch (ver.rStatus){
            case RELEASE:
                subVersion = ' '; break;
            case RELEASE_CANDIDATE:
                subVersion = StString("RC")    + ver.rSubVer; break;
            case BETA:
                subVersion = StString("beta")  + ver.rSubVer; break;
            case ALPHA:
                subVersion = StString("alpha") + ver.rSubVer; break;
            default:
                subVersion = StString("dev")   + ver.rSubVer; break;
        }
        stUtf8_t aBuff[256];
        stsprintf(aBuff, 256, "%d.%02d", (ver.year - TH_SUB), ver.month);
        return (StString(aBuff) + subVersion);
    }

    StVersionInfo& operator++() {
        // prefix ++
        setMonth(getMonth() + 1);
        setYear((getMonth() == 1) ? (getYear() + 1) : getYear());
        return (*this);
    }

    StVersionInfo& operator--() {
        // prefix --
        setMonth(getMonth() - 1);
        setYear((getMonth() == 12) ? (getYear() - 1) : getYear());
        return (*this);
    }

    StVersionInfo operator++(int ) {
        // postfix ++
        StVersionInfo verInfo = *this;
        ++(*this);
        return verInfo;
    }

    StVersionInfo operator--(int ) {
        // postfix --
        StVersionInfo verInfo = *this;
        --(*this);
        return verInfo;
    }

    bool operator>(const StVersionInfo& compareVer) {
        return ( getYear() > compareVer.getYear()
            || ( getYear() == compareVer.getYear() && getMonth() > compareVer.getMonth())
            || ( getYear() == compareVer.getYear() && getMonth() == compareVer.getMonth()
                        && getReleaseStatus() > compareVer.getReleaseStatus())
            || ( getYear() == compareVer.getYear() && getMonth() == compareVer.getMonth()
                        && getReleaseStatus() == compareVer.getReleaseStatus()
                        && getSubVersion() > compareVer.getSubVersion()) );
    }

    bool operator<(const StVersionInfo& compareVer) {
        return ( getYear() < compareVer.getYear()
            || ( getYear() == compareVer.getYear() && getMonth() < compareVer.getMonth())
            || ( getYear() == compareVer.getYear() && getMonth() == compareVer.getMonth()
                        && getReleaseStatus() < compareVer.getReleaseStatus())
            || ( getYear() == compareVer.getYear() && getMonth() == compareVer.getMonth()
                        && getReleaseStatus() == compareVer.getReleaseStatus()
                        && getSubVersion() < compareVer.getSubVersion()) );
    }

    bool operator>=(const StVersionInfo& compareVer) {
        return ( getYear() > compareVer.getYear()
            || ( getYear() == compareVer.getYear() && getMonth() > compareVer.getMonth())
            || ( getYear() == compareVer.getYear() && getMonth() == compareVer.getMonth()
                        && getReleaseStatus() > compareVer.getReleaseStatus())
            || ( getYear() == compareVer.getYear() && getMonth() == compareVer.getMonth()
                        && getReleaseStatus() == compareVer.getReleaseStatus()
                        && getSubVersion() >= compareVer.getSubVersion()) );
    }

    bool operator<=(const StVersionInfo& compareVer) {
        return ( getYear() < compareVer.getYear()
            || ( getYear() == compareVer.getYear() && getMonth() < compareVer.getMonth())
            || ( getYear() == compareVer.getYear() && getMonth() == compareVer.getMonth()
                        && getReleaseStatus() < compareVer.getReleaseStatus())
            || ( getYear() == compareVer.getYear() && getMonth() == compareVer.getMonth()
                        && getReleaseStatus() == compareVer.getReleaseStatus()
                        && getSubVersion() <= compareVer.getSubVersion()) );
    }

    bool operator==(const StVersionInfo& compareVer) {
        return ( getYear() == compareVer.getYear() && getMonth() == compareVer.getMonth()
                 && getReleaseStatus() == compareVer.getReleaseStatus()
                 && getSubVersion() == compareVer.getSubVersion() );
    }

    bool operator!=(const StVersionInfo& compareVer) {
        return ( getYear() != compareVer.getYear() || getMonth() != compareVer.getMonth()
                 || getReleaseStatus() != compareVer.getReleaseStatus()
                 || getSubVersion() != compareVer.getSubVersion() );
    }

};

#endif // ST_RESOURCE_COMPILER


#endif //__StVersion_h_
