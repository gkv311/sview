/**
 * This is a header for threads creating/manipulating.
 * (redefinition for WinAPI and POSIX threads)
 * Copyright © 2008-2013 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#ifndef __StTheads_h_
#define __StTheads_h_

#include <stTypes.h>

#ifdef _WIN32
    extern "C" __declspec(dllimport) void __stdcall Sleep(unsigned long theMilliseconds);
#else
    #include <pthread.h>
    #include <unistd.h>
    #include <errno.h>
    #include <sys/time.h>
#endif

/**
 * Simple class to create/manipulate threads.
 */
class StThread {

        public:

#ifdef _WIN32
    #define SV_THREAD_FUNCTION unsigned int __stdcall
    #define SV_THREAD_RETURN
    typedef unsigned int (__stdcall* threadFunction_t)(void* );
#else
    #define SV_THREAD_FUNCTION void*
    #define SV_THREAD_RETURN (void*)
    typedef void* (*threadFunction_t)(void* );
#endif

        public:

    /**
     * Simple function to sleep thread.
     * @param theMilliseconds - time to sleep in milliseconds.
     */
    static void sleep(const int theMilliseconds) {
    #ifdef _WIN32
        Sleep(theMilliseconds);
    #else
        usleep(theMilliseconds * 1000);
    #endif
    }

    /**
     * Returns the logical processors count in system.
     * This number could be used to tune multithreading algorithms.
     * @return logical processors count.
     */
    ST_CPPEXPORT static int countLogicalProcessors();

    /**
     * Returns the CPU architecture used to build the program (may not match the system).
     */
    ST_LOCAL static const char* getArchString() {
    #if (defined(_WIN64) || defined(__WIN64__))\
     || (defined(_LP64)  || defined(__LP64__))
        return "x86_64";
    #else
        return "x86";
    #endif
    }

    /**
     * Runtime check
     */
    ST_LOCAL static bool isBigEndian() {
        union {
            int  myInt;
            char myChar[sizeof(int)];
        } aUnion;
        aUnion.myInt = 1;
        return !aUnion.myChar[0];
    }

    /**
     * Returns unique identifier for current thread.
     * Intended only fo debugging/logging purposes!
     * @return unique id for current thread
     */
    ST_CPPEXPORT static size_t getCurrentThreadId();

    /**
     * The function cannot be used by one thread to create a handle that can be used by other threads to refer to the first thread!
     * /
    static StThread getCurrentThread() {
    #if (defined(_WIN32) || defined(__WIN32__))
        return StThread(GetCurrentThread(), GetCurrentThreadId(), true);
    #else
        return StThread(pthread_self());
    #endif
    }*/

    /**
     * Create the thread and start it.
     * @param theThreadFunc  Thread function
     * @param theThreadParam Thread function argument
     */
    ST_CPPEXPORT StThread(threadFunction_t theThreadFunc,
                          void*            theThreadParam);

    /**
     * Indicates valid thread handle.
     */
    ST_LOCAL inline bool isValid() const {
    #ifdef _WIN32
        return myThread != NULL;
    #else
        return myHasHandle; // myThread != PTHREAD_NULL
    #endif
    }

    /**
     * Wait for thread return (infinity).
     */
    ST_CPPEXPORT bool wait();

    /**
     *  This is a dangerous function that should only be used in the most extreme case!
     */
    ST_CPPEXPORT void kill();

    /**
     * Function shall indicate that storage for the thread thread can be reclaimed when that thread terminates.
     * If thread has not terminated, this function shall not cause it to terminate.
     */
    ST_CPPEXPORT void detach();

    /**
     * Destructor. By default detaches from created thread.
     */
    ST_CPPEXPORT ~StThread();

        private:

#ifdef _WIN32
    uintptr_t    myThread;
    unsigned int myThreadId;
#else
    pthread_t    myThread;
    bool         myHasHandle;
#endif

};

#endif //__StTheads_h_
