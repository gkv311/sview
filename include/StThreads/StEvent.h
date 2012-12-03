/**
 * Copyright © 2009-2012 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#ifndef __StEvent_h_
#define __StEvent_h_

#if (defined(_WIN32) || defined(__WIN32__))
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
 * This is simple Event class that helps to talk threads between themselfs.
 * Object is similar to WinAPI Event.
 */
class ST_LOCAL StEvent {

        public:

    StEvent(bool isSignalling = true) {
    #if (defined(_WIN32) || defined(__WIN32__))
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

    ~StEvent() {
    #if (defined(_WIN32) || defined(__WIN32__))
        CloseHandle(myEvent);
    #else
        pthread_mutex_destroy(&myMutex);
        pthread_cond_destroy(&myCond);
    #endif
    }

    /**
     * Set event into signalling state.
     */
    void set() {
    #if (defined(_WIN32) || defined(__WIN32__))
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
    void reset() {
    #if (defined(_WIN32) || defined(__WIN32__))
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
    void wait() {
    #if (defined(_WIN32) || defined(__WIN32__))
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
    bool wait(const size_t& theTimeMilliseconds) {
    #if (defined(_WIN32) || defined(__WIN32__))
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
    bool check() {
    #if (defined(_WIN32) || defined(__WIN32__))
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
    bool checkReset() {
        bool wasSignalled = check();
        reset();
        return wasSignalled;
    }

        private:

#if (defined(_WIN32) || defined(__WIN32__))
    HANDLE          myEvent;
#else
    pthread_mutex_t myMutex;
    pthread_cond_t  myCond;
    bool            myFlag;
#endif

};

#endif //__StEvent_h_
