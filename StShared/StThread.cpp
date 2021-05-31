/**
 * Copyright © 2009-2017 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#include <StThreads/StThread.h>
#include <StStrings/StLogger.h>

#ifdef _WIN32
    #include <windows.h>
    #include <process.h>

    namespace {
        static const DWORD MS_VC_EXCEPTION = 0x406D1388;
    }

    #pragma pack(push, 8)
    struct MsWinThreadNameInfo {
        DWORD  dwType;     //!< must be 0x1000
        LPCSTR szName;     //!< pointer to name
        DWORD  dwThreadID; //!< thread ID (-1 for caller thread)
        DWORD  dwFlags;    //!< must be zero
    };
    #pragma pack(pop)

#else
    #include <sys/types.h>

    #ifdef __sun
        #include <sys/processor.h>
        #include <sys/procset.h>
    #else
        #include <sched.h>
    #endif
#endif

#include <StFile/StRawFile.h>

size_t StThread::getCurrentThreadId() {
#ifdef _WIN32
    return (size_t )GetCurrentThreadId();
#else
    return (size_t )pthread_self(); // NOT the same as 'gettid()'!
#endif
}

StThread::StThread(threadFunction_t theThreadFunc,
                   void*            theThreadParam,
                   const char*      theThreadName) {
#ifdef _WIN32
    myThread = _beginthreadex(NULL, 0, theThreadFunc, theThreadParam, 0, &myThreadId);
#else
    myHasHandle = (pthread_create(&myThread, (pthread_attr_t* )NULL, theThreadFunc, theThreadParam) == 0);
#endif
    setName(theThreadName);
}

StThread::~StThread() {
    detach();
}

void StThread::setName(const char* theName) {
    if( theName == NULL
    || *theName == '\0'
    || !isValid()) {
        return;
    }

#ifdef _WIN32
    MsWinThreadNameInfo anInfo;
    anInfo.dwType     = 0x1000;
    anInfo.szName     = theName;
    anInfo.dwThreadID = myThreadId;
    anInfo.dwFlags    = 0;

    __try {
        ::RaiseException(MS_VC_EXCEPTION, 0, sizeof(anInfo)/sizeof(ULONG_PTR), (ULONG_PTR* )&anInfo);
    } __except(EXCEPTION_EXECUTE_HANDLER) {
        //
    }
#elif defined(__APPLE__)
    (void )theName;
#else
    pthread_setname_np(myThread, theName);
#endif
}

void StThread::setCurrentThreadName(const char* theName) {
    if( theName == NULL
    || *theName == '\0') {
        return;
    }

#ifdef _WIN32
    MsWinThreadNameInfo anInfo;
    anInfo.dwType     = 0x1000;
    anInfo.szName     = theName;
    anInfo.dwThreadID = (DWORD )-1;
    anInfo.dwFlags    = 0;

    __try {
        ::RaiseException(MS_VC_EXCEPTION, 0, sizeof(anInfo)/sizeof(ULONG_PTR), (ULONG_PTR* )&anInfo);
    } __except(EXCEPTION_EXECUTE_HANDLER) {
        //
    }
#elif defined(__APPLE__)
    pthread_setname_np(theName);
#else
    pthread_setname_np(pthread_self(), theName);
#endif
}

bool StThread::wait() {
    if(!isValid()) {
        return false;
    }

#ifdef _WIN32
    if(WaitForSingleObject((HANDLE )myThread, INFINITE) == WAIT_FAILED) {
        return false;
    }
    CloseHandle((HANDLE )myThread);
    myThread = NULL;
#else
    if(pthread_join(myThread, NULL) != 0) {
        return false;
    }
    myHasHandle = false;
#endif
    return true;
}

bool StThread::wait(const int theTimeMilliseconds) {
    if(!isValid()) {
        return false;
    }

#ifdef _WIN32
    if(WaitForSingleObject((HANDLE )myThread, (DWORD )theTimeMilliseconds) == WAIT_TIMEOUT) {
        return false;
    }
    CloseHandle((HANDLE )myThread);
    myThread = NULL;
#else
    (void )theTimeMilliseconds;
    if(pthread_join(myThread, NULL) != 0) {
        return false;
    }
    myHasHandle = false;
#endif
    return true;
}

void StThread::kill() {
    if(isValid()) {
    #ifdef _WIN32
        TerminateThread((HANDLE )myThread, 0);
    #elif defined(__ANDROID__)
        ST_ERROR_LOG("StThread::kill() is unavailable on this platform!");
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

namespace {
#if defined(__ANDROID__)

    /**
     * Simple number parser.
     */
    static const char* parseNumber(int&        theResult,
                                   const char* theInput,
                                   const char* theLimit,
                                   const int   theBase = 10) {
        const char* aCharIter = theInput;
        int aValue = 0;
        while(aCharIter < theLimit) {
            int aDigit = (*aCharIter - '0');
            if((unsigned int )aDigit >= 10U) {
                aDigit = (*aCharIter - 'a');
                if((unsigned int )aDigit >= 6U) {
                    aDigit = (*aCharIter - 'A');
                }
                if((unsigned int )aDigit >= 6U) {
                    break;
                }
                aDigit += 10;
            }
            if(aDigit >= theBase) {
                break;
            }
            aValue = aValue * theBase + aDigit;
            ++aCharIter;
        }
        if(aCharIter == theInput) {
            return NULL;
        }

        theResult = aValue;
        return aCharIter;
    }

    /**
     * Read CPUs mask from sysfs.
     */
    uint32_t readCpuMask(const StCString& thePath) {
        StRawFile aFile;
        if(!aFile.readFile(thePath)) {
            return 0;
        }

        const char* aCharIter = (const char* )aFile.getBuffer();
        const char* anEnd     = aCharIter + aFile.getSize();
        uint32_t    aCpuMask  = 0;
        while(aCharIter < anEnd && *aCharIter != '\n') {
            const char* aChunkEnd = (const char* )::memchr(aCharIter, ',', anEnd - aCharIter);
            if(aChunkEnd == NULL) {
                aChunkEnd = anEnd;
            }

            // get first value
            int anIndexLower = 0;
            aCharIter = parseNumber(anIndexLower, aCharIter, aChunkEnd);
            if(aCharIter == NULL) {
                return aCpuMask;
            }

            // if we're not at the end of the item, expect a dash and and integer; extract end value.
            int anIndexUpper = anIndexLower;
            if(aCharIter < aChunkEnd && *aCharIter == '-') {
                aCharIter = parseNumber(anIndexUpper, aCharIter + 1, aChunkEnd);
                if(aCharIter == NULL) {
                    return aCpuMask;
                }
            }

            // set bits CPU list
            for(int aCpuIndex = anIndexLower; aCpuIndex <= anIndexUpper; ++aCpuIndex) {
                if((unsigned int )aCpuIndex < 32) {
                    aCpuMask |= (uint32_t )(1U << aCpuIndex);
                }
            }

            aCharIter = aChunkEnd;
            if(aCharIter < anEnd) {
                ++aCharIter;
            }
        }
        return aCpuMask;
    }

#endif
}


int StThread::countLogicalProcessors() {
#ifdef _WIN32
    static int aNumLogicalProcessors = 0;
    if(aNumLogicalProcessors != 0) {
        return aNumLogicalProcessors;
    }

    SYSTEM_INFO aSysInfo;
    ::GetNativeSystemInfo(&aSysInfo);
    aNumLogicalProcessors = aSysInfo.dwNumberOfProcessors;
    return aNumLogicalProcessors;
#else

    #if defined(__ANDROID__)
    uint32_t aCpuMaskPresent  = readCpuMask(stCString("/sys/devices/system/cpu/present"));
    uint32_t aCpuMaskPossible = readCpuMask(stCString("/sys/devices/system/cpu/possible"));
    aCpuMaskPresent &= aCpuMaskPossible;
    const int aNbCores = __builtin_popcount(aCpuMaskPresent);
    if(aNbCores >= 1) {
        return aNbCores;
    }
    #endif
    // These are the choices. We'll check number of processors online.
    // _SC_NPROCESSORS_CONF   Number of processors configured
    // _SC_NPROCESSORS_MAX    Max number of processors supported by platform
    // _SC_NPROCESSORS_ONLN   Number of processors online
    return (int )sysconf(_SC_NPROCESSORS_ONLN);
#endif
}
