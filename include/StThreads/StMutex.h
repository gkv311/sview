/**
 * Copyright © 2009-2012 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#ifndef __StMutex_h_
#define __StMutex_h_

#if (defined(_WIN32) || defined(__WIN32__))
    #include <windows.h> // we used global header instead Winbase.h to prevent namespaces collisions
#else
    #include <pthread.h>
    #include <unistd.h>
    #include <errno.h>
    #include <sys/time.h>
#endif

#include <stTypes.h> // common types and defines

/**
 * StMutex is a simple Mutex class-wrapper
 * with behaviour similar to WinAPI mutex object.
 * Class implemented as inline.
 */
class ST_LOCAL StMutex {

        public:

    /**
     * Create unnamed mutex in system
     */
    StMutex() {
    #if (defined(_WIN32) || defined(__WIN32__))
        myMutex = CreateMutex(NULL,  // means mutex can not be inherited by child processes
                              false, // If this value is TRUE and the caller created the mutex,
                                     // the calling thread obtains initial ownership of the mutex object.
                                     // Otherwise, the calling thread does not obtain ownership of the mutex.
                                     // To determine if the caller created the mutex, see the Return Values section.
                              NULL); // the mutex object is created without a name.
    #else
        // We create recursive POSIX mutex to get behaviour like WinAPI mutex object
        pthread_mutexattr_t anAttr;
        pthread_mutexattr_init(&anAttr);
        pthread_mutexattr_settype(&anAttr, PTHREAD_MUTEX_RECURSIVE);
        pthread_mutex_init(&myMutex, &anAttr);
    #endif
    }

    /**
     * WAIT (for release from other thread) and lock mutex.
     * Note: recursive mutex object is used, means
     * if thread locked mutex several times
     * it must call unlock mutex SAME times to release.
     * @return true if success.
     */
    bool lock() {
    #if (defined(_WIN32) || defined(__WIN32__))
        return (WaitForSingleObject(myMutex, INFINITE) != WAIT_FAILED);
    #else
        return (pthread_mutex_lock(&myMutex) == 0);
    #endif
    }

    /**
     * TRY to lock mutex (WITHOUT waiting for release from other thread).
     * @return true if success.
     */
    bool tryLock() {
    #if (defined(_WIN32) || defined(__WIN32__))
        return (WaitForSingleObject(myMutex, (DWORD )0) != WAIT_TIMEOUT);
    #else
        return (pthread_mutex_trylock(&myMutex) == 0);
    #endif
    }

    /**
     * Release mutex.
     * @return true if success
     */
    bool unlock() {
    #if (defined(_WIN32) || defined(__WIN32__))
        return (ReleaseMutex(myMutex) != 0);
    #else
        return (pthread_mutex_unlock(&myMutex) == 0);
    #endif
    }

    /**
     * Just remove mutex from system.
     */
    ~StMutex() {
    #if (defined(_WIN32) || defined(__WIN32__))
        if(lock()) {
            CloseHandle(myMutex);
        }
    #else
        pthread_mutex_destroy(&myMutex);
    #endif
    }

        private:

#if (defined(_WIN32) || defined(__WIN32__))
    HANDLE          myMutex;
#else
    pthread_mutex_t myMutex;
#endif

};

/**
 * Class implements auto-lock mutex on construction
 * and auto-unlock it on destruction.
 * This object designed to use memory scope rules
 * to automatically release mutex. Thus it helps
 * for multiple branching return code.
 * Do not use it for several relative mutexes - this will
 * be unsafe!
 */
class ST_LOCAL StMutexAuto {

        public:

    StMutexAuto(StMutex* theMutex)
    : myMutex(theMutex) {
        myMutex->lock();
    }

    StMutexAuto(StMutex& theMutex)
    : myMutex(&theMutex) {
        myMutex->lock();
    }

    ~StMutexAuto() {
        myMutex->unlock();
    }

        private:

    StMutex* myMutex;

};

#endif //__StMutex_h_
