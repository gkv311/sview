/**
 * Copyright © 2019 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#ifndef __StJNIEnv_h__
#define __StJNIEnv_h__

#include <StStrings/StString.h>

//#include <jni.h>

struct _JNIEnv;
struct _JavaVM;
typedef _JNIEnv JNIEnv;
typedef _JavaVM JavaVM;

/**
 * The sentry class for attaching the current thread to JavaVM.
 */
class StJNIEnv {

        public:

    /**
     * Main constructor, tries to attach JavaVM to the current thread.
     */
    ST_CPPEXPORT StJNIEnv(JavaVM* theJavaVM);

    /**
     * Destructor, automatically detaches JavaVM from current thread.
     * Has no effect if JavaVM has been attached to the thread before creating this sentry.
     */
    ST_CPPEXPORT ~StJNIEnv();

    /**
     * Detach from current thread right now.
     * Has no effect if JavaVM has been attached to the thread before creating this sentry.
     */
    ST_CPPEXPORT void detach();

    /**
     * Cast to actual JNIEnv instance.
     */
    ST_LOCAL JNIEnv* operator->() const {
        return myJniEnv;
    }

    /**
     * Return true if JNI environment is NULL.
     */
    ST_LOCAL bool isNull() const { return myJniEnv == NULL; }

        private:

    JavaVM* myJavaVM;   //!< pointer to global Java VM instance
    JNIEnv* myJniEnv;   //!< JNI environment for working thread
    size_t  myThreadId; //!< attached thread id
    bool    myToDetach; //!< flag to detach

};

#endif //__StJNIEnv_h__
