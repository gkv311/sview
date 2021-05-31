/**
 * Copyright © 2008-2014 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#ifndef __StTimer_h_
#define __StTimer_h_

#include <stTypes.h>

#ifdef _WIN32
    #include <windows.h>
#else
    #include <sys/time.h>
    #include <stdlib.h> // just for NULL declaration

    #if (!defined(__APPLE__) && defined(_POSIX_MONOTONIC_CLOCK) && (_POSIX_MONOTONIC_CLOCK > 0)) || defined(__ANDROID__)
        #define ST_HAVE_MONOTONIC_CLOCK
    #endif

#endif

/**
 * High Resolution Timer.
 * This timer is able to measure the elapsed time with 1 micro-second accuracy
 * in both Windows, Linux and Unix systems.
 */
class StTimer {

        public:

    /**
     * @param isStart Flag to start timer
     */
    StTimer(bool isStart = false)
    : myTimeInMicroSec(0.0),
      myIsPaused(true) {
        if(isStart) {
            restart(0.0);
        } else {
            reset();
        }
    }

    /**
     * Just start the timer again.
     */
    void resume() {
        if(myIsPaused) {
            stMemSet(&myCounterStart, 0, sizeof(myCounterStart));
            myIsPaused = false;
            fillCounter(myCounterStart);
        }
    }

    /**
     * Clean start.
     * @param theStartFromMicroSec Initial time in micro second
     */
    void restart(double theStartFromMicroSec = 0.0) {
        reset();
        myTimeInMicroSec = theStartFromMicroSec;
        myIsPaused = false;

        fillCounter(myCounterStart);
    }

    bool isOn() const {
        return !myIsPaused;
    }

    /**
     * Pause the timer (freeze current timestamp).
     */
    void pause() {
        if(!myIsPaused) {
            // increment our timer value
            myTimeInMicroSec += getElapsedTimeFromLastStartInMicroSec();
            // set timer paused flag
            myIsPaused = true;
        }
    }

    /**
     * Stop the timer (and clean current timestamp).
     */
    void stop() {
        pause();
        reset();
    }

    /**
     * @return Timer value in seconds
     */
    double getElapsedTime() const {
        return this->getElapsedTimeInSec();
    }

    /**
     * @return Timer value in seconds
     */
    double getElapsedTimeInSec() const {
        return this->getElapsedTimeInMicroSec() * 0.000001;
    }

    /**
     * @return Timer value in milli-seconds
     */
    double getElapsedTimeInMilliSec() const {
        return this->getElapsedTimeInMicroSec() * 0.001;
    }

    /**
     * Main function.
     * @return Timer value in micro-seconds
     */
    double getElapsedTimeInMicroSec() const {
        return myTimeInMicroSec + getElapsedTimeFromLastStartInMicroSec();
    }

    /**
     * Main function - return time, elapsed from last start.
     * @return micro-seconds from start
     */
    double getElapsedTimeFromLastStartInMicroSec() const {
        return myIsPaused ? 0.0 : timeFromStart();
    }

        protected:

#ifdef _WIN32
    typedef LARGE_INTEGER stTimeCounter_t;
#elif defined(ST_HAVE_MONOTONIC_CLOCK)
    typedef timespec      stTimeCounter_t;
#else
    typedef timeval       stTimeCounter_t;
#endif

        protected:

    static void fillCounter(stTimeCounter_t& theCounter) {
    #ifdef _WIN32
        QueryPerformanceCounter(&theCounter);
    #elif defined(ST_HAVE_MONOTONIC_CLOCK)
        clock_gettime(CLOCK_MONOTONIC, &theCounter);
    #else
        gettimeofday(&theCounter, NULL);
    #endif
    }

#ifdef _WIN32
    static double winInvFrequency() {
        // according to MSDN frequency cannot change while the system is running
        LARGE_INTEGER aFrequency; // ticks per second
        QueryPerformanceFrequency(&aFrequency);
        return (1000000.0 / double(aFrequency.QuadPart));
    }
#endif

    double timeFromStart() const {
        stTimeCounter_t aCounterEnd;
        fillCounter(aCounterEnd);
    #ifdef _WIN32
        static const double INV_FREQ = winInvFrequency();
        return double(aCounterEnd.QuadPart - myCounterStart.QuadPart) * INV_FREQ;
    #elif defined(ST_HAVE_MONOTONIC_CLOCK)
        return (double(aCounterEnd.tv_sec  - myCounterStart.tv_sec ) * 1000000.0)
             + (double(aCounterEnd.tv_nsec - myCounterStart.tv_nsec) * 0.001);
    #else
        return (double(aCounterEnd.tv_sec  - myCounterStart.tv_sec) * 1000000.0)
              + double(aCounterEnd.tv_usec - myCounterStart.tv_usec);
    #endif
    }

    /**
     * Reset current timestamp to zero.
     */
    void reset() {
        stMemZero(&myCounterStart, sizeof(myCounterStart));
        myTimeInMicroSec = 0.0;
    }

        protected:

    double          myTimeInMicroSec; //!< cumulative elapsed time
    stTimeCounter_t myCounterStart;
    bool            myIsPaused;       //!< pause flag

        protected:

    friend class StCondition;

};

#endif // __StTimer_h_
