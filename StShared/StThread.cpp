/**
 * Copyright © 2009-2017 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#include <StThreads/StThread.h>
#include <StStrings/StLogger.h>

#include <StThreads/StMutex.h>

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

#if defined(__APPLE__)
    #import <TargetConditionals.h>
#endif

#if defined(__EMSCRIPTEN__)
    #include <emscripten/emscripten.h>
#elif defined(__ANDROID__)
    //#include <unwind.h>
#elif defined(__QNX__)
    //#include <backtrace.h> // requires linking to libbacktrace
#elif !defined(_WIN32) && !(defined(TARGET_OS_IPHONE) && TARGET_OS_IPHONE)
    #include <execinfo.h>
#elif defined(_WIN32)

#ifdef _MSC_VER
#pragma warning(disable : 4091)
#endif

#include <dbghelp.h>
#ifdef _MSC_VER
#pragma warning(default : 4091)
#endif

/**
 * This is a wrapper of DbgHelp library loaded dynamically.
 * DbgHelp is coming with Windows SDK, so that technically it is always available.
 * However, it's usage requires extra steps:
 * - .pdb files are necessary to resolve function names;
 *   Normal release DLLs without PDBs will show no much useful info.
 * - DbgHelp.dll and friends (SymSrv.dll, SrcSrv.dll) should be packaged with application;
 *   DbgHelp.dll coming with system might be of other incompatible version
 *   (some applications load it dynamically to avoid packaging extra DLL,
 *    with a extra hacks determining library version)
 */
class StDbgHelper {
        public: // dbgHelp.dll function types

    typedef BOOL(WINAPI *SYMINITIALIZEPROC) (HANDLE, PCSTR, BOOL);
    typedef BOOL(WINAPI *STACKWALK64PROC) (DWORD, HANDLE, HANDLE, LPSTACKFRAME64,
                                           PVOID, PREAD_PROCESS_MEMORY_ROUTINE64,
                                           PFUNCTION_TABLE_ACCESS_ROUTINE64,
                                           PGET_MODULE_BASE_ROUTINE64, PTRANSLATE_ADDRESS_ROUTINE64);
    typedef BOOL(WINAPI *SYMCLEANUPPROC) (HANDLE);
    typedef BOOL(WINAPI *SYMFROMADDRPROC) (HANDLE, DWORD64, PDWORD64, PSYMBOL_INFO);

        public:

    /** Return global instance. */
    static StDbgHelper& getInstance() {
        static StDbgHelper aDbgHelper;
        return aDbgHelper;
    }

    /** Return global mutex. */
    static StMutex& getMutex() {
        static StMutex aMutex;
        return aMutex;
    }

        public:

    SYMINITIALIZEPROC                SymInitialize;
    SYMCLEANUPPROC                   SymCleanup;
    STACKWALK64PROC                  StackWalk64;
    PFUNCTION_TABLE_ACCESS_ROUTINE64 SymFunctionTableAccess64;
    PGET_MODULE_BASE_ROUTINE64       SymGetModuleBase64;
    SYMFROMADDRPROC                  SymFromAddr;

    /** Return TRUE if library has been loaded. */
    bool isLoaded() const { return myDbgHelpLib != NULL; }

    /** Return loading error message. */
    const char* getErrorMessage() const { return myError; }

        private:

    /** Main constructor. */
    StDbgHelper()
    : SymInitialize(NULL),
      SymCleanup(NULL),
      StackWalk64(NULL),
      SymFunctionTableAccess64(NULL),
      SymGetModuleBase64(NULL),
      SymFromAddr(NULL),
      myDbgHelpLib(LoadLibraryW(L"DbgHelp.dll")),
      myError(NULL) {
        if (myDbgHelpLib == NULL) {
            myError = "Failed to load DbgHelp.dll";
            return;
        }

        if ((SymInitialize = (SYMINITIALIZEPROC)GetProcAddress(myDbgHelpLib, "SymInitialize")) == NULL) {
            myError = "Function not found in DbgHelp.dll: SymInitialize";
            unload();
            return;
        }
        if ((SymCleanup = (SYMCLEANUPPROC)GetProcAddress(myDbgHelpLib, "SymCleanup")) == NULL) {
            myError = "Function not found in DbgHelp.dll: SymCleanup";
            unload();
            return;
        }
        if ((StackWalk64 = (STACKWALK64PROC)GetProcAddress(myDbgHelpLib, "StackWalk64")) == NULL) {
            myError = "Function not found in DbgHelp.dll: StackWalk64";
            unload();
            return;
        }
        if ((SymFunctionTableAccess64 = (PFUNCTION_TABLE_ACCESS_ROUTINE64)GetProcAddress(myDbgHelpLib, "SymFunctionTableAccess64")) == NULL) {
            myError = "Function not found in DbgHelp.dll: SymFunctionTableAccess64";
            unload();
            return;
        }
        if ((SymGetModuleBase64 = (PGET_MODULE_BASE_ROUTINE64)GetProcAddress(myDbgHelpLib, "SymGetModuleBase64")) == NULL) {
            myError = "Function not found in DbgHelp.dll: SymGetModuleBase64";
            unload();
            return;
        }
        if ((SymFromAddr = (SYMFROMADDRPROC)GetProcAddress(myDbgHelpLib, "SymFromAddr")) == NULL) {
            myError = "Function not found in DbgHelp.dll: SymFromAddr";
            unload();
            return;
        }
    }

    /** Destructor. */
    ~StDbgHelper() {
        // unloading library makes not sense for singletone
        //unload();
    }

    /** Unload library. */
    void unload() {
        if (myDbgHelpLib != NULL) {
            FreeLibrary(myDbgHelpLib);
            myDbgHelpLib = NULL;
        }
    }

private:
    StDbgHelper(const StDbgHelper&);
    StDbgHelper& operator= (const StDbgHelper&);
private:
    HMODULE     myDbgHelpLib; //!< handle to DbgHelp
    const char* myError;      //!< loading error message
};

#endif

bool StThread::addStackTrace(char* theBuffer,
                             const int theBufferSize,
                             const int theNbTraces,
                             void* theContext,
                             const int theNbTopSkip) {
    (void)theContext;
    if (theBufferSize < 1
     || theNbTraces < 1
     || theBuffer == NULL
     || theNbTopSkip < 0) {
        return false;
    }

#if defined(__EMSCRIPTEN__)
    return emscripten_get_callstack(EM_LOG_C_STACK | EM_LOG_DEMANGLE | EM_LOG_NO_PATHS | EM_LOG_FUNC_PARAMS,
                                    theBuffer, theBufferSize) > 0;
#elif defined(__ANDROID__)
    // not implemented
    return false;
#elif defined(__QNX__)
    // bt_get_backtrace()
    // not implemented
    return false;
#elif defined(_WIN32)
    // Each CPU architecture requires manual stack frame setup,
    // and 32-bit version requires also additional hacks to retrieve current context;
    // this implementation currently covers only x86_64 architecture.
#if defined(_M_X64)
    int aNbTraces = theNbTraces;
    const HANDLE anHProcess = GetCurrentProcess();
    const HANDLE anHThread = GetCurrentThread();
    CONTEXT aCtx = {};
    if (theContext != NULL) {
        memcpy(&aCtx, theContext, sizeof(aCtx));
    } else {
        ++aNbTraces;
        memset(&aCtx, 0, sizeof(aCtx));
        aCtx.ContextFlags = CONTEXT_FULL;
        RtlCaptureContext(&aCtx);
    }

    // DbgHelp is not thread-safe library, hence global lock is used for serial access
    StMutexAuto aLock(StDbgHelper::getMutex());
    StDbgHelper& aDbgHelp = StDbgHelper::getInstance();
    if (!aDbgHelp.isLoaded()) {
        strcat_s(theBuffer, theBufferSize, "\n==Backtrace==\n");
        strcat_s(theBuffer, theBufferSize, aDbgHelp.getErrorMessage());
        strcat_s(theBuffer, theBufferSize, "\n=============");
        return false;
    }

    aDbgHelp.SymInitialize(anHProcess, NULL, TRUE);

    STACKFRAME64 aStackFrame;
    memset(&aStackFrame, 0, sizeof(aStackFrame));

    DWORD anImage = IMAGE_FILE_MACHINE_AMD64;
    aStackFrame.AddrPC.Offset = aCtx.Rip;
    aStackFrame.AddrPC.Mode = AddrModeFlat;
    aStackFrame.AddrFrame.Offset = aCtx.Rsp;
    aStackFrame.AddrFrame.Mode = AddrModeFlat;
    aStackFrame.AddrStack.Offset = aCtx.Rsp;
    aStackFrame.AddrStack.Mode = AddrModeFlat;

    char aModBuffer[MAX_PATH] = {};
    char aSymBuffer[sizeof(SYMBOL_INFO) + MAX_SYM_NAME * sizeof(CHAR)];
    SYMBOL_INFO* aSymbol = (SYMBOL_INFO*)aSymBuffer;
    aSymbol->SizeOfStruct = sizeof(SYMBOL_INFO);
    aSymbol->MaxNameLen = MAX_SYM_NAME;

    int aTopSkip = theNbTopSkip + 1; // skip this function call and specified extra number
    strcat_s(theBuffer, theBufferSize, "\n==Backtrace==");
    for (int aLineIter = 0; aLineIter < aNbTraces; ++aLineIter) {
        BOOL aRes = aDbgHelp.StackWalk64(anImage, anHProcess, anHThread,
                                         &aStackFrame, &aCtx, NULL,
                                         aDbgHelp.SymFunctionTableAccess64, aDbgHelp.SymGetModuleBase64, NULL);
        if (!aRes) {
            break;
        }

        if (theContext == NULL && aTopSkip > 0) {
            --aTopSkip;
            continue;
        }
        if (aStackFrame.AddrPC.Offset == 0) {
            break;
        }

        strcat_s(theBuffer, theBufferSize, "\n");

        const DWORD64 aModuleBase = aDbgHelp.SymGetModuleBase64(anHProcess, aStackFrame.AddrPC.Offset);
        if (aModuleBase != 0
         && GetModuleFileNameA((HINSTANCE)aModuleBase, aModBuffer, MAX_PATH)) {
            strcat_s(theBuffer, theBufferSize, aModBuffer);
        }

        DWORD64 aDisp = 0;
        strcat_s(theBuffer, theBufferSize, "(");
        if (aDbgHelp.SymFromAddr(anHProcess, aStackFrame.AddrPC.Offset, &aDisp, aSymbol)) {
            strcat_s(theBuffer, theBufferSize, aSymbol->Name);
        } else {
            strcat_s(theBuffer, theBufferSize, "???");
        }
        strcat_s(theBuffer, theBufferSize, ")");
    }
    strcat_s(theBuffer, theBufferSize, "\n=============");

    aDbgHelp.SymCleanup(anHProcess);
    return true;
#else
    // not implemented for this CPU architecture
    return false;
#endif
#else
    const int aTopSkip = theNbTopSkip + 1; // skip this function call and specified extra number
    int aNbTraces = theNbTraces + aTopSkip;
    void** aStackArr = (void**)alloca(sizeof(void*) * aNbTraces);
    if (aStackArr == NULL) {
        return false;
    }

    aNbTraces = ::backtrace(aStackArr, aNbTraces);
    if (aNbTraces <= 1) {
        return false;
    }

    aNbTraces -= aTopSkip;
    char** aStrings = ::backtrace_symbols(aStackArr + aTopSkip, aNbTraces);
    if (aStrings == NULL) {
        return false;
    }

    const size_t aLenInit = strlen(theBuffer);
    size_t aLimit = (size_t)theBufferSize - aLenInit - 1;
    if (aLimit > 14) {
        strcat(theBuffer, "\n==Backtrace==");
        aLimit -= 14;
    }
    for (int aLineIter = 0; aLineIter < aNbTraces; ++aLineIter) {
        const size_t aLen = strlen(aStrings[aLineIter]);
        if (aLen + 1 >= aLimit) {
            break;
        }

        strcat(theBuffer, "\n");
        strcat(theBuffer, aStrings[aLineIter]);
        aLimit -= aLen + 1;
    }
    free(aStrings);
    if (aLimit > 14) {
        strcat(theBuffer, "\n=============");
    }
    return true;
#endif
}
