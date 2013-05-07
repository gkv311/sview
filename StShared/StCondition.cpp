/**
 * Copyright © 2009-2013 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#ifdef _WIN32
    #include <windows.h> // we used global header instead Winbase.h to prevent namespaces collisions
#else
    #include <pthread.h>
    #include <unistd.h>
    #include <errno.h>
    #include <sys/time.h>
#endif

#include <StThreads/StCondition.h>

StCondition::StCondition()
#ifdef _WIN32
: myEvent((void* )CreateEvent(0, true, theIsSet, NULL))
#else
: myFlag(true)
#endif
{
#ifndef _WIN32
    pthread_mutex_init(&myMutex, 0);
    pthread_cond_init (&myCond,  0);
#endif
}

StCondition::StCondition(const bool theIsSet)
#ifdef _WIN32
: myEvent((void* )CreateEvent(0, true, theIsSet, NULL))
#else
: myFlag(theIsSet)
#endif
{
#ifndef _WIN32
    pthread_mutex_init(&myMutex, 0);
    pthread_cond_init (&myCond,  0);
#endif
}

StCondition::~StCondition() {
#ifdef _WIN32
    CloseHandle((HANDLE )myEvent);
#else
    pthread_mutex_destroy(&myMutex);
    pthread_cond_destroy(&myCond);
#endif
}

void StCondition::set() {
#ifdef _WIN32
    SetEvent((HANDLE )myEvent);
#else
    pthread_mutex_lock(&myMutex);
    myFlag = true;
    pthread_cond_broadcast(&myCond);
    pthread_mutex_unlock(&myMutex);
#endif
}

void StCondition::reset() {
#ifdef _WIN32
    ResetEvent((HANDLE )myEvent);
#else
    pthread_mutex_lock(&myMutex);
    myFlag = false;
    pthread_mutex_unlock(&myMutex);
#endif
}

void StCondition::wait() {
#ifdef _WIN32
    WaitForSingleObject((HANDLE )myEvent, INFINITE);
#else
    pthread_mutex_lock(&myMutex);
    if(!myFlag) {
        pthread_cond_wait(&myCond, &myMutex);
    }
    pthread_mutex_unlock(&myMutex);
#endif
}

bool StCondition::wait(const size_t& theTimeMilliseconds) {
#ifdef _WIN32
    return (WaitForSingleObject((HANDLE )myEvent, (DWORD )theTimeMilliseconds) != WAIT_TIMEOUT);
#else
    bool isSignalled = true;
    pthread_mutex_lock(&myMutex);
    if(!myFlag) {
        struct timeval  aNow;
        struct timespec aTimeout;
        gettimeofday(&aNow, NULL);
        size_t aSeconds = (theTimeMilliseconds / 1000);
        size_t aNanoseconds = (theTimeMilliseconds - aSeconds * 1000) * 1000;
        aTimeout.tv_sec  = aNow.tv_sec  + (time_t )aSeconds;
        aTimeout.tv_nsec = aNow.tv_usec + (long   )aNanoseconds;
        isSignalled = (pthread_cond_timedwait(&myCond, &myMutex, &aTimeout) != ETIMEDOUT);
    }
    pthread_mutex_unlock(&myMutex);
    return isSignalled;
#endif
}

bool StCondition::check() {
#ifdef _WIN32
    return (WaitForSingleObject((HANDLE )myEvent, (DWORD )0) != WAIT_TIMEOUT);
#else
    bool isSignalled = true;
    pthread_mutex_lock(&myMutex);
    if(!myFlag) {
        struct timeval  aNow;
        struct timespec aTimeout;
        gettimeofday(&aNow, NULL);
        aTimeout.tv_sec  = aNow.tv_sec;
        // TODO (Kirill Gavrilov) which values must be here and need we this block here?
        aTimeout.tv_nsec = aNow.tv_usec + 100;
        isSignalled = (pthread_cond_timedwait(&myCond, &myMutex, &aTimeout) != ETIMEDOUT);
    }
    pthread_mutex_unlock(&myMutex);
    return isSignalled;
#endif
}

bool StCondition::checkReset() {
#ifdef _WIN32
    const bool wasSignalled = (WaitForSingleObject((HANDLE )myEvent, (DWORD )0) != WAIT_TIMEOUT);
    ResetEvent((HANDLE )myEvent);
    return wasSignalled;
#else
    pthread_mutex_lock(&myMutex);
    bool wasSignalled = myFlag;
    if(!myFlag) {
        struct timeval  aNow;
        struct timespec aTimeout;
        gettimeofday(&aNow, NULL);
        aTimeout.tv_sec  = aNow.tv_sec;
        aTimeout.tv_nsec = aNow.tv_usec + 100;
        wasSignalled = (pthread_cond_timedwait(&myCond, &myMutex, &aTimeout) != ETIMEDOUT);
    }
    myFlag = false;
    pthread_mutex_unlock(&myMutex);
    return wasSignalled;
#endif
}
