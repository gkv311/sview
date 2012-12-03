/**
 * This is a header for threads creating/manipulating.
 * (redefinition for WinAPI and POSIX threads)
 * Copyright © 2008-2012 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#ifndef __StTheads_h_
#define __StTheads_h_

#include "StTimer.h"
#include "StMutex.h"
#include "StEvent.h"
#include "StProcess.h"

#ifndef _WIN32
    #ifndef __USE_GNU
        #define __USE_GNU
    #endif
    #include <unistd.h>
    #include <sys/types.h>

    #ifdef sun
        #include <sys/processor.h>
        #include <sys/procset.h>
    #else
        #include <sched.h>
    #endif
#endif

/**
 * Simple class to create/manipulate threads.
 */
class ST_LOCAL StThread {

        public:

#if (defined(_WIN32) || defined(__WIN32__))
    #define SV_THREAD_FUNCTION DWORD WINAPI
    #define SV_THREAD_RETURN
    typedef LPTHREAD_START_ROUTINE threadFunction_t;
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
        #if (defined(_WIN32) || defined(__WIN32__))
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
    static int countLogicalProcessors() {
        static int aNumLogicalProcessors = 0;
        if(aNumLogicalProcessors != 0) {
            return aNumLogicalProcessors;
        }
    #ifdef _WIN32
        // GetSystemInfo() will return the number of processors in a data field in a SYSTEM_INFO structure.
        SYSTEM_INFO aSysInfo;
        if(isWow64()) {
            typedef BOOL (WINAPI *LPFN_GSI)(LPSYSTEM_INFO );
            HMODULE aKern32Module = GetModuleHandleW(L"kernel32");
            LPFN_GSI aFuncSysInfo = (aKern32Module != NULL)
                ? (LPFN_GSI )GetProcAddress(aKern32Module, "GetNativeSystemInfo") : NULL;
            // So, they suggest 32-bit apps should call this instead of the other in WOW64
            if(aFuncSysInfo) {
                aFuncSysInfo(&aSysInfo);
            } else {
                GetSystemInfo(&aSysInfo);
            }
        } else {
            GetSystemInfo(&aSysInfo);
        }
        aNumLogicalProcessors = aSysInfo.dwNumberOfProcessors;
    #else
        // These are the choices. We'll check number of processors online.
        // _SC_NPROCESSORS_CONF   Number of processors configured
        // _SC_NPROCESSORS_MAX    Max number of processors supported by platform
        // _SC_NPROCESSORS_ONLN   Number of processors online
        aNumLogicalProcessors = (int )sysconf(_SC_NPROCESSORS_ONLN);
    #endif
        return aNumLogicalProcessors;
    }

    /**
     * Returns the CPU architecture used to build the program (may not match the system).
     */
    static StString getArchString() {
    #if (defined(_WIN64) || defined(__WIN64__))\
     || (defined(_LP64)  || defined(__LP64__))
        return StString("x86_64");
    #else
        return StString("x86");
    #endif
    }

    /**
     * Runtime check
     */
    static bool isBigEndian() {
        union {
            int  myInt;
            char myChar[sizeof(int)];
        } aUnion;
        aUnion.myInt = 1;
        return !aUnion.myChar[0];
    }

    /**
     * Returns unique identificator for current thread.
     * @return unique id for current thread.
     */
    static size_t getCurrentThreadId() {
    #if (defined(_WIN32) || defined(__WIN32__))
        return (size_t )GetCurrentThreadId();
    #else
        return (size_t )pthread_self(); // NOT the same as 'gettid()'!
    #endif
    }

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
     */
    StThread(threadFunction_t theThreadFunc,
             void*            theThreadParam) {
    #if (defined(_WIN32) || defined(__WIN32__))
        myThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE )theThreadFunc, theThreadParam, CREATE_SUSPENDED, &myThreadId);
        if(myThread == NULL) {
            myCreateStatus = false;
        } else {
            myCreateStatus = true;
            SetThreadPriority(myThread, THREAD_PRIORITY_BELOW_NORMAL); // Set thread priority
            ResumeThread(myThread); // Run thread
        }
    #else
        myCreateStatus = (pthread_create(&myThread, (pthread_attr_t* )NULL, theThreadFunc, theThreadParam) == 0);
    #endif
    }

    /**
     * Wait for thread return (infinity).
     */
    bool wait() {
    #if (defined(_WIN32) || defined(__WIN32__))
        return (WaitForSingleObject(myThread, INFINITE) != WAIT_FAILED);
    #else
        return (pthread_join(myThread, NULL) == 0);
    #endif
    }

    /**
     *  This is a dangerous function that should only be used in the most extreme case!
     */
    void kill() {
    #if (defined(_WIN32) || defined(__WIN32__))
        TerminateThread(myThread, 0);
    #else
        pthread_cancel(myThread);
    #endif
    }

    /**
     * Function shall indicate that storage for the thread thread can be reclaimed when that thread terminates.
     * If thread has not terminated, this function shall not cause it to terminate.
     */
    void detach() {
    #if (defined(_WIN32) || defined(__WIN32__))
        if(myThread != NULL) {
            CloseHandle(myThread);
            myThread = NULL;
        }
    #else
        /// TODO (Kirill Gavrilov#1) check for PTHREAD_NULL ?
        pthread_detach(myThread);
    #endif
    }

    ~StThread() {
        detach();
    }

        private:

#ifdef _WIN32
    // for a 64-bit app running under 64-bit Windows, this is FALSE
    static bool isWow64() {
        typedef BOOL (WINAPI *LPFN_ISWOW64PROCESS) (HANDLE , PBOOL );
        BOOL bIsWow64 = FALSE;
        HMODULE aKern32Module = GetModuleHandleW(L"kernel32");
        LPFN_ISWOW64PROCESS aFunIsWow64 = (aKern32Module == NULL) ? (LPFN_ISWOW64PROCESS )NULL
                                        : (LPFN_ISWOW64PROCESS )GetProcAddress(GetModuleHandle(L"kernel32"), "IsWow64Process");
        return aFunIsWow64 != NULL
            && aFunIsWow64(GetCurrentProcess(), &bIsWow64)
            && bIsWow64 != FALSE;
    }
#endif

        private:

#if (defined(_WIN32) || defined(__WIN32__))
    HANDLE    myThread;
    DWORD     myThreadId;
#else
    pthread_t myThread;
#endif
    bool      myCreateStatus;

};

#endif //__StTheads_h_
