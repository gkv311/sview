/**
 * Copyright © 2011 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#ifndef __StMutexSlim_h_
#define __StMutexSlim_h_

#include "StMutex.h"

#include <StTemplates/StAtomic.h>

/**
 * This is a special mutex object intended to protect short code blocks.
 * It uses atomic operations within counter before obtaining real mutex
 * to avoid useless expensive calls to the kernel in case of low real contention
 * between multiple threads (single-thread access or very short code block).
 *
 * This class does not provide trylock() functionality due to
 * undefined behaviour may occur with it.
 * This mutex provides a recursive behaviour however its speed up
 * benefits may be lost in this case.
 *
 * Notice that on Windows CriticalSection object is used which seems
 * provide the behaviour described above (however limits its usage to intra-process calls).
 *
 * On Linux just general StMutex object is used that already work like described above.
 * It seem pthreads library or even Linux kernel already apply this optimization (according to tests).
 */
class ST_LOCAL StMutexSlim {

        public:

    /**
     * Create an unlocked mutex object.
     */
    StMutexSlim()
#if (defined(_WIN32) || defined(__WIN32__))
    {
        // create the critical section with spin count 1024
        if(!InitializeCriticalSectionAndSpinCount(&myCritSection, 0x00000400)) {
            // error
        }
    }
#else
    : myMutex() {}
#endif

    /**
     * Destructor.
     */
    ~StMutexSlim() {
    #if (defined(_WIN32) || defined(__WIN32__))
        DeleteCriticalSection(&myCritSection);
    #endif
    }

    /**
     * Lock the mutex.
     */
    void lock() {
    #if (defined(_WIN32) || defined(__WIN32__))
        EnterCriticalSection(&myCritSection);
    #else
        myMutex.lock();
    #endif
    }

    /**
     * Unlock the mutex.
     */
    void unlock() {
    #if (defined(_WIN32) || defined(__WIN32__))
        LeaveCriticalSection(&myCritSection);
    #else
        myMutex.unlock();
    #endif
    }

        private:

#if (defined(_WIN32) || defined(__WIN32__))
    CRITICAL_SECTION myCritSection;
#else
    StMutex myMutex;
#endif

};

#endif //__StMutexSlim_h_
