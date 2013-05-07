/**
 * Copyright © 2009-2013 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#include <StThreads/StThread.h>

#ifdef _WIN32
    #include <windows.h>
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

size_t StThread::getCurrentThreadId() {
#ifdef _WIN32
    return (size_t )GetCurrentThreadId();
#else
    return (size_t )pthread_self(); // NOT the same as 'gettid()'!
#endif
}

StThread::StThread(threadFunction_t theThreadFunc,
                   void*            theThreadParam) {
#ifdef _WIN32
    myThread = _beginthreadex(NULL, 0, theThreadFunc, theThreadParam, 0, &myThreadId);
#else
    myHasHandle = (pthread_create(&myThread, (pthread_attr_t* )NULL, theThreadFunc, theThreadParam) == 0);
#endif
}

StThread::~StThread() {
    detach();
}

bool StThread::wait() {
#ifdef _WIN32
    return isValid() && (WaitForSingleObject((HANDLE )myThread, INFINITE) != WAIT_FAILED);
#else
    return isValid() && (pthread_join(myThread, NULL) == 0);
#endif
}

void StThread::kill() {
    if(isValid()) {
    #ifdef _WIN32
        TerminateThread((HANDLE )myThread, 0);
    #else
        pthread_cancel(myThread);
    #endif
    }
}

void StThread::detach() {
    if(isValid()) {
    #ifdef _WIN32
        CloseHandle((HANDLE )myThread);
        myThread = NULL;
    #else
        pthread_detach(myThread);
        myHasHandle = false;
    #endif
    }
}

#ifdef _WIN32
namespace {
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
}
#endif

int StThread::countLogicalProcessors() {
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
