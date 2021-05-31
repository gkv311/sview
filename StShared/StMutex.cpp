/**
 * Copyright © 2009-2013 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#ifdef _WIN32
    #include <windows.h>
#else
    #include <unistd.h>
    #include <errno.h>
    #include <sys/time.h>
#endif

#include <StThreads/StMutex.h>

StMutex::StMutex() {
#ifdef _WIN32
    myMutex = (void* )CreateMutex(NULL, false, NULL);
#else
    // We create recursive POSIX mutex to get behaviour like WinAPI mutex object
    pthread_mutexattr_t anAttr;
    pthread_mutexattr_init(&anAttr);
    pthread_mutexattr_settype(&anAttr, PTHREAD_MUTEX_RECURSIVE);
    pthread_mutex_init(&myMutex, &anAttr);
#endif
}

StMutex::~StMutex() {
#ifdef _WIN32
    if(lock()) {
        CloseHandle((HANDLE )myMutex);
    }
#else
    pthread_mutex_destroy(&myMutex);
#endif
}

bool StMutex::lock() {
#ifdef _WIN32
    return (WaitForSingleObject((HANDLE )myMutex, INFINITE) != WAIT_FAILED);
#else
    return (pthread_mutex_lock(&myMutex) == 0);
#endif
}

bool StMutex::tryLock() {
#ifdef _WIN32
    return (WaitForSingleObject((HANDLE )myMutex, (DWORD )0) != WAIT_TIMEOUT);
#else
    return (pthread_mutex_trylock(&myMutex) == 0);
#endif
}

bool StMutex::unlock() {
#ifdef _WIN32
    return (ReleaseMutex((HANDLE )myMutex) != 0);
#else
    return (pthread_mutex_unlock(&myMutex) == 0);
#endif
}
