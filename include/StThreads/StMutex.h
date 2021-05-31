/**
 * Copyright © 2009-2013 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#ifndef __StMutex_h_
#define __StMutex_h_

#include <stTypes.h> // common types and defines

#ifndef _WIN32
    #include <pthread.h>
#endif

/**
 * StMutex is a simple Mutex class-wrapper
 * with behaviour similar to WinAPI mutex object.
 * Class implemented as inline.
 */
class StMutex {

        public:

    /**
     * Create unnamed mutex in system
     */
    ST_CPPEXPORT StMutex();

    /**
     * Just remove mutex from system.
     */
    ST_CPPEXPORT ~StMutex();

    /**
     * WAIT (for release from other thread) and lock mutex.
     * Note: recursive mutex object is used, means
     * if thread locked mutex several times
     * it must call unlock mutex SAME times to release.
     * @return true if success.
     */
    ST_CPPEXPORT bool lock();

    /**
     * TRY to lock mutex (WITHOUT waiting for release from other thread).
     * @return true if success.
     */
    ST_CPPEXPORT bool tryLock();

    /**
     * Release mutex.
     * @return true if success
     */
    ST_CPPEXPORT bool unlock();

        private:

#ifdef _WIN32
    void*           myMutex;
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
class StMutexAuto {

        public:

    ST_LOCAL inline StMutexAuto(StMutex* theMutex)
    : myMutex(theMutex) {
        myMutex->lock();
    }

    ST_LOCAL inline StMutexAuto(StMutex& theMutex)
    : myMutex(&theMutex) {
        myMutex->lock();
    }

    ST_LOCAL inline ~StMutexAuto() {
        if(myMutex != NULL) {
            myMutex->unlock();
        }
    }

    /**
     * Unlock mutex before destruction.
     */
    ST_LOCAL inline void unlock() {
        if(myMutex != NULL) {
            myMutex->unlock();
            myMutex = NULL;
        }
    }

        private:

    StMutex* myMutex;

};

#endif //__StMutex_h_
