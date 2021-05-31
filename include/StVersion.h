/**
 * This is a header for version structure in 'Ubuntu-style' (Year.Month).
 * Copyright © 2008-2015 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#ifndef __StVersion_h_
#define __StVersion_h_

#define ST_DEVELOPMENT_RELEASE 0
#define ST_ALPHA               1
#define ST_BETA                2
#define ST_RELEASE_CANDIDATE   3
#define ST_RELEASE             4

#if defined(ST_HAVE_STCONFIG) || defined(RC_INVOKED)
    #include <stconfig.conf>
#else
    #if !defined(SVIEW_SDK_VER_STATUS) && !defined(ST_DEBUG)
        #define SVIEW_SDK_VER_STATUS ST_RELEASE
    #endif
    #ifndef SVIEW_SDK_VERSION_AUTO
        #define SVIEW_SDK_VERSION_AUTO
    #endif
#endif

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


// just empty version here
// should be redefined in stconfig.conf
#ifndef SVIEW_SDK_VERSION
    #define SVIEW_SDK_VERSION 10, 1, 0, 0
#endif

#ifndef SVIEW_SDK_VER_STRING
    #define SVIEW_SDK_VER_STRING "10.01dev0"
#endif

#ifndef SVIEW_SDK_VER_STATUS
    #define SVIEW_SDK_VER_STATUS ST_DEVELOPMENT_RELEASE
#endif

#ifdef RC_INVOKED
    #if defined(ST_DEBUG) || (SVIEW_SDK_VER_STATUS == 0)
        #define ST_WIN32_FILEFLAGS FILEFLAGS VS_FF_DEBUG
    #elif (SVIEW_SDK_VER_STATUS != 4)
        #define ST_WIN32_FILEFLAGS FILEFLAGS VS_FF_PRERELEASE
    #else
        #define ST_WIN32_FILEFLAGS
    #endif
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

#ifndef RC_INVOKED

#include <StStrings/StLogger.h>
#include <StStrings/stUtfTools.h>
#include <StThreads/StThread.h>

#include <time.h>

class ST_LOCAL StVersionInfo {

        public:

    static StString getSDKVersionString() {
        return getSDKVersionInfo().toString();
    }

    static StVersionInfo getSDKVersionInfo() {
        return StVersionInfo(getSDKVersion());
    }

    static StVersion getSDKVersion() {
        const int aVerInt[4] = {
        #ifdef SVIEW_SDK_VERSION_AUTO
            __YEAR__,
            (__MONTH__) + 1,
            SVIEW_SDK_VER_STATUS,
            __DAY__
        #else
            SVIEW_SDK_VERSION
        #endif
        };

        StVersion aVer;
        aVer.year    = aVerInt[0];
        aVer.month   = aVerInt[1];
        aVer.rStatus = aVerInt[2];
        aVer.rSubVer = aVerInt[3];
        return aVer;
    }

    /**
     * @return true if no errors.
     */
#ifdef ST_TIMEBOMB
    static bool checkTimeBomb(const StString& ) {
        return true;
    }
#else
    static bool checkTimeBomb(const StString& theTitle) {
        StVersionInfo aVer = getSDKVersionInfo();
        if(aVer.getReleaseStatus() == ST_DEVELOPMENT_RELEASE) {
            time_t aRawTime;
            struct tm* aTimeInfo;
            time(&aRawTime);
            aTimeInfo = localtime(&aRawTime);
            StVersionInfo aTest = StVersionInfo(1900 + aTimeInfo->tm_year, aTimeInfo->tm_mon + 1, ST_DEVELOPMENT_RELEASE, 0);
            aVer++; aVer++;
            if(aTest > aVer) {
                stError(StString()
                    + "You are running alpha version of the '"
                    + theTitle + "'.\n"
                    + "Sorry but time for testing this build is expired.\n"
                    + "You should update program from official site (www.sview.ru)"
                );
                return false;
            }
        }
        return true;
    }
#endif // ST_TIMEBOMB

    /**
     * Simple version constructor.
     * @param theYear    put year  here
     * @param theMonth   put month here
     * @param theRStatus release state (use enum)
     * @param theRSubVer subversion
     */
    StVersionInfo(const int theYear,
                  const int theMonth,
                  const int theRStatus,
                  const int theRSubVer) {
        setYear (theYear);
        setMonth(theMonth);
        myVer.rStatus = theRStatus;
        myVer.rSubVer = theRSubVer;
    }

    /**
     * Simple version constructor.
     * @param version the version structure
     */
    StVersionInfo(const StVersion& theVersion) {
        setVersion(theVersion);
    }

    StVersion& getVersion() {
        return myVer;
    }

    void setVersion(const StVersion& theVersion) {
        myVer = theVersion;
        setYear (theVersion.year);
        setMonth(theVersion.month);
    }

    int getYear() const {
        return myVer.year;
    }

    void setYear(const int theYear) {
        myVer.year = (theYear > TH_SUB) ? theYear : (TH_SUB + theYear);
    }

    int getMonth() const {
        return myVer.month;
    }

    void setMonth(const int theMonth) {
        myVer.month = (theMonth < 1) ? 12 : ( (theMonth > 12) ? 1 : theMonth );
    }

    int getReleaseStatus() const {
        return myVer.rStatus;
    }

    void setReleaseStatus(const int theRStatus) {
        myVer.rStatus = theRStatus;
    }

    int getSubVersion() const {
        return myVer.rSubVer;
    }

    void setSubVersion(const int theRSubVer) {
        myVer.rSubVer = theRSubVer;
    }

    /**
     * Return string representation of the version.
     */
    StString toString() const {
        const int aYear  =  __YEAR__;
        const int aMonth = (__MONTH__) + 1;
        const int aDay   =  __DAY__;

        StString aState;
        switch(myVer.rStatus) {
            case ST_RELEASE:           break;
            case ST_RELEASE_CANDIDATE: aState = StString("RC")    + myVer.rSubVer; break;
            case ST_BETA:              aState = StString("beta")  + myVer.rSubVer; break;
            case ST_ALPHA:             aState = StString("alpha") + myVer.rSubVer; break;
            default:                   aState = StString("dev")   + myVer.rSubVer; break;
        }
        stUtf8_t aBuff[256];
        stsprintf(aBuff, 256, "%d.%02d", (myVer.year - TH_SUB), myVer.month);
        return StString(aBuff) + aState
             + " " + StThread::getArchString()
             + " [build " + aYear
                          + "-" + (aMonth < 10 ? "0" : "") + aMonth
                          + "-" + (aDay   < 10 ? "0" : "") + aDay + "]";
    }

    /**
     * Prefix ++
     */
    StVersionInfo& operator++() {
        setMonth(getMonth() +  1);
        setYear((getMonth() == 1) ? (getYear() + 1) : getYear());
        return *this;
    }

    /**
     * Prefix --
     */
    StVersionInfo& operator--() {
        setMonth(getMonth() -  1);
        setYear((getMonth() == 12) ? (getYear() - 1) : getYear());
        return *this;
    }

    /**
     * Postfix ++
     */
    StVersionInfo operator++(int ) {
        StVersionInfo aCopy = *this;
        ++(*this);
        return aCopy;
    }

    /**
     * Postfix --
     */
    StVersionInfo operator--(int ) {
        StVersionInfo aCopy = *this;
        --(*this);
        return aCopy;
    }

    bool operator>(const StVersionInfo& theOther) const {
        return   getYear() >  theOther.getYear()
            || ( getYear() == theOther.getYear() && getMonth() >  theOther.getMonth())
            || ( getYear() == theOther.getYear() && getMonth() == theOther.getMonth()
                        && getReleaseStatus() > theOther.getReleaseStatus())
            || ( getYear() == theOther.getYear() && getMonth() == theOther.getMonth()
                        && getReleaseStatus() == theOther.getReleaseStatus()
                        && getSubVersion()    >  theOther.getSubVersion());
    }

    bool operator<(const StVersionInfo& theOther) const {
        return   getYear() <  theOther.getYear()
            || ( getYear() == theOther.getYear() && getMonth() <  theOther.getMonth())
            || ( getYear() == theOther.getYear() && getMonth() == theOther.getMonth()
                        && getReleaseStatus() < theOther.getReleaseStatus())
            || ( getYear() == theOther.getYear() && getMonth() == theOther.getMonth()
                        && getReleaseStatus() == theOther.getReleaseStatus()
                        && getSubVersion()    <  theOther.getSubVersion());
    }

    bool operator>=(const StVersionInfo& theOther) const {
        return   getYear() >  theOther.getYear()
            || ( getYear() == theOther.getYear() && getMonth() >  theOther.getMonth())
            || ( getYear() == theOther.getYear() && getMonth() == theOther.getMonth()
                        && getReleaseStatus() > theOther.getReleaseStatus())
            || ( getYear() == theOther.getYear() && getMonth() == theOther.getMonth()
                        && getReleaseStatus() == theOther.getReleaseStatus()
                        && getSubVersion()    >= theOther.getSubVersion());
    }

    bool operator<=(const StVersionInfo& theOther) const {
        return   getYear() < theOther.getYear()
            || ( getYear() == theOther.getYear() && getMonth() <  theOther.getMonth())
            || ( getYear() == theOther.getYear() && getMonth() == theOther.getMonth()
                        && getReleaseStatus() < theOther.getReleaseStatus())
            || ( getYear() == theOther.getYear() && getMonth() == theOther.getMonth()
                        && getReleaseStatus() == theOther.getReleaseStatus()
                        && getSubVersion()    <= theOther.getSubVersion());
    }

    bool operator==(const StVersionInfo& theOther) const {
        return getYear()          == theOther.getYear()
            && getMonth()         == theOther.getMonth()
            && getReleaseStatus() == theOther.getReleaseStatus()
            && getSubVersion()    == theOther.getSubVersion();
    }

    bool operator!=(const StVersionInfo& theOther) const {
        return getYear()          != theOther.getYear()
            || getMonth()         != theOther.getMonth()
            || getReleaseStatus() != theOther.getReleaseStatus()
            || getSubVersion()    != theOther.getSubVersion();
    }

        private:

    enum {
        TH_SUB = 2000
    };

        private:

    StVersion myVer;

};

#endif // RC_INVOKED

#endif //__StVersion_h_
