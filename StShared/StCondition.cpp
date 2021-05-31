/**
 * Copyright © 2009-2015 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#ifdef _WIN32
    #include <windows.h>
#else
    #include <pthread.h>
    #include <unistd.h>
    #include <errno.h>
    #include <sys/time.h>
#endif

#include <StThreads/StCondition.h>
#include <StThreads/StTimer.h>

namespace {

#ifndef _WIN32
    /**
     * clock_gettime() wrapper.
     */
    inline void stGetRealTime(struct timespec& theTime) {
    #if defined(__APPLE__)
        struct timeval aTime;
        gettimeofday(&aTime, NULL);
        theTime.tv_sec  = aTime.tv_sec;
        theTime.tv_nsec = aTime.tv_usec * 1000;
    #else
        clock_gettime(CLOCK_REALTIME, &theTime);
    #endif
    }
#endif

}

StCondition::StCondition()
#ifdef _WIN32
: myEvent((void* )CreateEvent(0, true, true, NULL))
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

bool StCondition::wait(const size_t theTimeMilliseconds) {
#ifdef _WIN32
    return (WaitForSingleObject((HANDLE )myEvent, (DWORD )theTimeMilliseconds) != WAIT_TIMEOUT);
#else
    bool isSignalled = true;
    pthread_mutex_lock(&myMutex);
    if(!myFlag) {
        struct timespec aNow;
        struct timespec aTimeout;
        stGetRealTime(aNow);
        aTimeout.tv_sec  = (theTimeMilliseconds / 1000);
        aTimeout.tv_nsec = (theTimeMilliseconds - aTimeout.tv_sec * 1000) * 1000000;
        if(aTimeout.tv_nsec > 1000000000) {
            aTimeout.tv_sec  += 1;
            aTimeout.tv_nsec -= 1000000000;
        }
        aTimeout.tv_sec  += aNow.tv_sec;
        aTimeout.tv_nsec += aNow.tv_nsec;
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
        struct timespec aNow;
        struct timespec aTimeout;
        stGetRealTime(aNow);
        aTimeout.tv_sec  = aNow.tv_sec;
        aTimeout.tv_nsec = aNow.tv_nsec + 100;
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
        struct timespec aNow;
        struct timespec aTimeout;
        stGetRealTime(aNow);
        aTimeout.tv_sec  = aNow.tv_sec;
        aTimeout.tv_nsec = aNow.tv_nsec + 100;
        wasSignalled = (pthread_cond_timedwait(&myCond, &myMutex, &aTimeout) != ETIMEDOUT);
    }
    myFlag = false;
    pthread_mutex_unlock(&myMutex);
    return wasSignalled;
#endif
}
