/**
 * Copyright © 2008-2013 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#ifndef __StTimer_h_
#define __StTimer_h_

#include <stTypes.h>

#if (defined(_WIN32) || defined(__WIN32__))
    #include <windows.h>
#else
    #include <sys/time.h>
    #include <stdlib.h> // just for NULL declaration
#endif

/**
 * High Resolution Timer.
 * This timer is able to measure the elapsed time with 1 micro-second accuracy
 * in both Windows, Linux and Unix systems.
 */
class StTimer {

        public:

    /**
     * @param isStart (bool ) - start timer on create flag.
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
        stMemSet(&myCounterStart, 0, sizeof(myCounterStart));
        stMemSet(&myCounterEnd,   0, sizeof(myCounterEnd));
        myIsPaused = false;

        fillCounter(myCounterStart);
    }

    /**
     * Clean start.
     * @param startTime (double ) - initial time in micro second.
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
        // increment our timer value
        myTimeInMicroSec += getElapsedTimeFromLastStartInMicroSec();
        // set timer paused flag
        myIsPaused = true;
    }

    /**
     * Stop the timer (and clean current timestamp).
     */
    void stop() {
        pause();
        reset();
    }

    /**
     * @return seconds (double ) - timer in seconds.
     */
    double getElapsedTime() {
        return this->getElapsedTimeInSec();
    }

    /**
     * @return seconds (double ) - timer in seconds.
     */
    double getElapsedTimeInSec() {
        return this->getElapsedTimeInMicroSec() * 0.000001;
    }

    /**
     * @return milliSeconds (double ) - timer in milli-seconds.
     */
    double getElapsedTimeInMilliSec() {
        return this->getElapsedTimeInMicroSec() * 0.001;
    }

    /**
     * Main function.
     * @return microSeconds (double ) - timer in micro-seconds.
     */
    double getElapsedTimeInMicroSec() {
        return myTimeInMicroSec + getElapsedTimeFromLastStartInMicroSec();
    }

    /**
     * Main function - return time, elapsed from last start.
     * @return microSeconds (double ) - micro-seconds from start.
     */
    double getElapsedTimeFromLastStartInMicroSec() {
        return myIsPaused ? 0.0 : timeFromStart();
    }

        private:

#if (defined(_WIN32) || defined(__WIN32__))
    typedef LARGE_INTEGER stTimeCounter_t;
#else
    typedef timeval       stTimeCounter_t;
#endif

        private:

    void fillCounter(stTimeCounter_t& theCounter) {
    #if (defined(_WIN32) || defined(__WIN32__))
        QueryPerformanceCounter(&theCounter);
    #else
        gettimeofday(&theCounter, NULL);
    #endif
    }

#if (defined(_WIN32) || defined(__WIN32__))
    static double winInvFrequency() {
        // according to MSDN frequency cannot change while the system is running
        LARGE_INTEGER aFrequency; // ticks per second
        QueryPerformanceFrequency(&aFrequency);
        return (1000000.0 / double(aFrequency.QuadPart));
    }
#endif

    double timeFromStart() {
        fillCounter(myCounterEnd);
    #if (defined(_WIN32) || defined(__WIN32__))
        static const double INV_FREQ = winInvFrequency();
        return double(myCounterEnd.QuadPart - myCounterStart.QuadPart) * INV_FREQ;
    #else
        return (double(myCounterEnd.tv_sec  - myCounterStart.tv_sec) * 1000000.0)
              + double(myCounterEnd.tv_usec - myCounterStart.tv_usec);
    #endif
    }

    /**
     * Reset current timestamp to zero.
     */
    void reset() {
        stMemSet(&myCounterStart, 0, sizeof(myCounterStart));
        stMemSet(&myCounterEnd,   0, sizeof(myCounterEnd));
        myTimeInMicroSec = 0.0;
    }

        private:

    double          myTimeInMicroSec; //!< cumulative elapsed time
    stTimeCounter_t myCounterStart;
    stTimeCounter_t myCounterEnd;
    bool            myIsPaused;       //!< pause flag

};

#endif // __StTimer_h_
