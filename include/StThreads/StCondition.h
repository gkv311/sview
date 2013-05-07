/**
 * Copyright © 2009-2013 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#ifndef __StCondition_h_
#define __StCondition_h_

#ifdef _WIN32
    #include <windows.h> // we used global header instead Winbase.h to prevent namespaces collisions
#else
    #include <pthread.h>
    #include <unistd.h>
    #include <errno.h>
    #include <sys/time.h>
#endif

#include <stTypes.h>

// MacOS headers currently by default define check macro for assertions
#ifdef check
    #undef check
#endif

/**
 * This is boolean flag intended for communication between threads.
 * One thread sets this flag to TRUE to indicate some event happened
 * and another thread either waits this event or checks periodically its state
 * to perform job.
 *
 * This class provides interface similar to WinAPI Event objects.
 */
class StCondition {

        public:

    inline StCondition(bool isSignalling = true) {
    #ifdef _WIN32
        myEvent = CreateEvent(0,            // default security attributes
                              true,         // manual-reset event
                              isSignalling, // initial state is signaled
                              NULL);        // no name
    #else
        pthread_mutex_init(&myMutex, 0);
        pthread_cond_init(&myCond, 0);
        myFlag = isSignalling;
    #endif
    }

    inline ~StCondition() {
    #ifdef _WIN32
        CloseHandle(myEvent);
    #else
        pthread_mutex_destroy(&myMutex);
        pthread_cond_destroy(&myCond);
    #endif
    }

    /**
     * Set event into signalling state.
     */
    inline void set() {
    #ifdef _WIN32
        SetEvent(myEvent);
    #else
        pthread_mutex_lock(&myMutex);
        myFlag = true;
        pthread_cond_broadcast(&myCond);
        pthread_mutex_unlock(&myMutex);
    #endif
    }

    /**
     * Reset event (unset signalling state)
     */
    inline void reset() {
    #ifdef _WIN32
        ResetEvent(myEvent);
    #else
        pthread_mutex_lock(&myMutex);
        myFlag = false;
        pthread_mutex_unlock(&myMutex);
    #endif
    }

    /**
     * Wait for Event (infinity).
     */
    inline void wait() {
    #ifdef _WIN32
        WaitForSingleObject(myEvent, INFINITE);
    #else
        pthread_mutex_lock(&myMutex);
        if(!myFlag) {
            pthread_cond_wait(&myCond, &myMutex);
        }
        pthread_mutex_unlock(&myMutex);
    #endif
    }

    /**
     * Wait for signal requested time.
     * @param theTimeMilliseconds (const size_t& ) - wait limit in millisecods;
     * @return true if get event.
     */
    inline bool wait(const size_t& theTimeMilliseconds) {
    #ifdef _WIN32
        return (WaitForSingleObject(myEvent, (DWORD )theTimeMilliseconds) != WAIT_TIMEOUT);
    #else
        bool isSignalled = true;
        pthread_mutex_lock(&myMutex);
        if(!myFlag) {
            struct timeval  aNow;
            struct timespec aTimeout;
            gettimeofday(&aNow, NULL);
            size_t aSeconds = (theTimeMilliseconds / 1000);
            size_t aNanoseconds = (theTimeMilliseconds - aSeconds * 1000) * 1000;
            aTimeout.tv_sec  = aNow.tv_sec  + (time_t )aSeconds;
            aTimeout.tv_nsec = aNow.tv_usec + (long   )aNanoseconds;
            isSignalled = (pthread_cond_timedwait(&myCond, &myMutex, &aTimeout) != ETIMEDOUT);
        }
        pthread_mutex_unlock(&myMutex);
        return isSignalled;
    #endif
    }

    /**
     * Do not wait for signal - just test it state.
     * @return true if get event.
     */
    inline bool check() {
    #ifdef _WIN32
        return (WaitForSingleObject(myEvent, (DWORD )0) != WAIT_TIMEOUT);
    #else
        bool isSignalled = true;
        pthread_mutex_lock(&myMutex);
        if(!myFlag) {
            struct timeval  aNow;
            struct timespec aTimeout;
            gettimeofday(&aNow, NULL);
            aTimeout.tv_sec  = aNow.tv_sec;
            // TODO (Kirill Gavrilov) wich values must be here and need we this block here?
            aTimeout.tv_nsec = aNow.tv_usec + 100;
            isSignalled = (pthread_cond_timedwait(&myCond, &myMutex, &aTimeout) != ETIMEDOUT);
        }
        pthread_mutex_unlock(&myMutex);
        return isSignalled;
    #endif
    }

    /**
     * Method perform two steps at-once - reset the event object
     * and returns true if it was in signaling state.
     * @return true if event object was in signaling state.
     */
    inline bool checkReset() {
        bool wasSignalled = check();
        reset();
        return wasSignalled;
    }

        private:

#ifdef _WIN32
    HANDLE          myEvent;
#else
    pthread_mutex_t myMutex;
    pthread_cond_t  myCond;
    bool            myFlag;
#endif

};

#endif //__StCondition_h_
