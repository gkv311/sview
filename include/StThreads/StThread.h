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
    #include <windows.h> // we used global header instead Winbase.h to prevent namespaces collisions
#else
    #include <pthread.h>
    #include <unistd.h>
    #include <errno.h>
    #include <sys/time.h>
#endif

#ifdef _WIN32
    #include <process.h>
#else
    #include <sys/types.h>

    #ifdef __sun
        #include <sys/processor.h>
        #include <sys/procset.h>
    #else
        #include <sched.h>
    #endif
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
    static const char* getArchString() {
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
    static bool isBigEndian() {
        union {
            int  myInt;
            char myChar[sizeof(int)];
        } aUnion;
        aUnion.myInt = 1;
        return !aUnion.myChar[0];
    }

    /**
     * Returns unique identifier for current thread.
     * @return unique id for current thread.
     */
    static size_t getCurrentThreadId() {
    #ifdef _WIN32
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
    #ifdef _WIN32
        myThread = (HANDLE )_beginthreadex(NULL, 0, theThreadFunc, theThreadParam, 0, (unsigned int* )&myThreadId);
    #else
        myHasHandle = (pthread_create(&myThread, (pthread_attr_t* )NULL, theThreadFunc, theThreadParam) == 0);
    #endif
    }

    /**
     * Indicates valid thread handle.
     */
    bool isValid() const {
    #ifdef _WIN32
        return myThread != NULL;
    #else
        return myHasHandle; // myThread != PTHREAD_NULL
    #endif
    }

    /**
     * Wait for thread return (infinity).
     */
    bool wait() {
    #ifdef _WIN32
        return isValid() && (WaitForSingleObject(myThread, INFINITE) != WAIT_FAILED);
    #else
        return isValid() && (pthread_join(myThread, NULL) == 0);
    #endif
    }

    /**
     *  This is a dangerous function that should only be used in the most extreme case!
     */
    void kill() {
        if(isValid()) {
        #ifdef _WIN32
            TerminateThread(myThread, 0);
        #else
            pthread_cancel(myThread);
        #endif
        }
    }

    /**
     * Function shall indicate that storage for the thread thread can be reclaimed when that thread terminates.
     * If thread has not terminated, this function shall not cause it to terminate.
     */
    void detach() {
        if(isValid()) {
        #ifdef _WIN32
            CloseHandle(myThread);
            myThread = NULL;
        #else
            pthread_detach(myThread);
            myHasHandle = false;
        #endif
        }
    }

    /**
     * Destructor. By default detaches from created thread.
     */
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

#ifdef _WIN32
    HANDLE    myThread;
    DWORD     myThreadId;
#else
    pthread_t myThread;
    bool      myHasHandle;
#endif

};

#endif //__StTheads_h_
