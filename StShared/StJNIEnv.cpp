/**
 * Copyright © 2019 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#include <StJNI/StJNIEnv.h>

#include <StStrings/StLogger.h>
#include <StThreads/StThread.h>

#if defined(__ANDROID__)
    #include <jni.h>
#endif

StJNIEnv::StJNIEnv(JavaVM* theJavaVM)
: myJavaVM(theJavaVM),
  myJniEnv(NULL),
  myThreadId(0),
  myToDetach(false) {
    if(myJavaVM == NULL) {
        return;
    }

#if defined(__ANDROID__)
    void* aJniEnv = NULL;
    switch(myJavaVM->GetEnv(&aJniEnv, JNI_VERSION_1_6)) {
        case JNI_EDETACHED: {
            if(myJavaVM->AttachCurrentThread(&myJniEnv, NULL) < 0) {
                myJniEnv = NULL;
                ST_ERROR_LOG("Failed to attach working thread to Java VM");
                return;
            }
            myToDetach = true;
            myThreadId = StThread::getCurrentThreadId();
            break;
        }
        case JNI_OK: {
            myJniEnv   = (JNIEnv* )aJniEnv;
            myToDetach = false;
            break;
        }
        case JNI_EVERSION: {
            ST_ERROR_LOG("Failed to attach working thread to Java VM - JNI version is not supported");
            break;
        }
        default: {
            ST_ERROR_LOG("Failed to attach working thread to Java VM");
            break;
        }
    }
#endif
}

StJNIEnv::~StJNIEnv() {
    detach();
}

void StJNIEnv::detach() {
    if(myJavaVM != NULL
    && myJniEnv != NULL
    && myToDetach) {
        if(myThreadId != StThread::getCurrentThreadId()) {
            ST_ERROR_LOG("Internal error, StJNIEnv::detach() - attempt to detach from another thread");
        }
    #if defined(__ANDROID__)
        myJavaVM->DetachCurrentThread();
    #endif
    }

    myJniEnv   = NULL;
    myToDetach = false;
}
