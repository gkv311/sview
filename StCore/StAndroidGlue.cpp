/**
 * Copyright © 2014-2015 Kirill Gavrilov <kirill@sview.ru>
 *
 * StCore library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * StCore library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.
 * If not, see <http://www.gnu.org/licenses/>.
 */

#if defined(__ANDROID__)

#include <StCore/StAndroidGlue.h>

#include <StCore/StApplication.h>
#include <StCore/StSearchMonitors.h>
#include <StTemplates/StHandle.h>
#include <StThreads/StThread.h>
#include <StStrings/StLogger.h>

#include <jni.h>
#define jexp extern "C" JNIEXPORT

StCString StAndroidGlue::getCommandIdName(StAndroidGlue::CommandId theCmd) {
    switch(theCmd) {
        case StAndroidGlue::CommandId_InputChanged:  return stCString("InputChanged");
        case StAndroidGlue::CommandId_WindowInit:    return stCString("WindowInit");
        case StAndroidGlue::CommandId_WindowTerm:    return stCString("WindowTerm");
        case StAndroidGlue::CommandId_WindowResize:  return stCString("WindowResize");
        case StAndroidGlue::CommandId_WindowRedraw:  return stCString("WindowRedraw");
        case StAndroidGlue::CommandId_FocusGained:   return stCString("FocusGained");
        case StAndroidGlue::CommandId_FocusLost:     return stCString("FocusLost");
        case StAndroidGlue::CommandId_ConfigChanged: return stCString("ConfigChanged");
        case StAndroidGlue::CommandId_LowMemory:     return stCString("LowMemory");
        case StAndroidGlue::CommandId_Start:         return stCString("Start");
        case StAndroidGlue::CommandId_Resume:        return stCString("Resume");
        case StAndroidGlue::CommandId_SaveState:     return stCString("SaveState");
        case StAndroidGlue::CommandId_Pause:         return stCString("Pause");
        case StAndroidGlue::CommandId_Stop:          return stCString("Stop");
        case StAndroidGlue::CommandId_Destroy:       return stCString("Destroy");
    }
    return stCString("UNKNOWN");
}

StAndroidGlue::StAndroidGlue(ANativeActivity* theActivity,
                             void*            theSavedState,
                             size_t           theSavedStateSize)
: myActivity(theActivity),
  myConfig(NULL),
  myLooper(NULL),
  myInputQueue(NULL),
  myInputQueuePending(NULL),
  myWindow(NULL),
  myWindowPending(NULL),
  myActivityState(0),
  myMemoryClassMiB(0),
  mySavedState(NULL),
  mySavedStateSize(0),
  myJavaVM(NULL),
  myThJniEnv(NULL),
  myMsgRead(0),
  myMsgWrite(0),
  myHasOrientSensor(false),
  myIsPoorOrient(false),
  myToTrackOrient(false),
  myIsRunning(false),
  myIsStateSaved(false),
  myToDestroy(false) {
    theActivity->instance = this;
    theActivity->env->GetJavaVM(&myJavaVM);

    JNIEnv* aJniEnv = myActivity->env;

    jclass    aJClass_Activity       = aJniEnv->GetObjectClass(myActivity->clazz);
    jmethodID aJMet_getSystemService = aJniEnv->GetMethodID(aJClass_Activity, "getSystemService", "(Ljava/lang/String;)Ljava/lang/Object;");
    jstring   aJStr_activity         = aJniEnv->NewStringUTF("activity");
    jobject   aJActivityMgr          = aJniEnv->CallObjectMethod(myActivity->clazz, aJMet_getSystemService, aJStr_activity);
    aJniEnv->DeleteLocalRef(aJStr_activity);
    if(aJActivityMgr != NULL) {
        // getLargeMemoryClass()
        jclass aJClass_ActivityManager = aJniEnv->GetObjectClass(aJActivityMgr);
        jmethodID aJMet_getMemoryClass = aJniEnv->GetMethodID(aJClass_ActivityManager, "getMemoryClass", "()I");
        myMemoryClassMiB = aJniEnv->CallIntMethod(aJActivityMgr, aJMet_getMemoryClass);
    }

    readOpenPath();

    myCmdPollSource.id        = LooperId_MAIN;
    myCmdPollSource.app       = this;
    myCmdPollSource.process   = StAndroidGlue::processCommandWrapper;
    myInputPollSource.id      = LooperId_INPUT;
    myInputPollSource.app     = this;
    myInputPollSource.process = StAndroidGlue::processInputWrapper;

    theActivity->callbacks->onDestroy               = StAndroidGlue::onDestroy;
    theActivity->callbacks->onStart                 = StAndroidGlue::onStart;
    theActivity->callbacks->onResume                = StAndroidGlue::onResume;
    theActivity->callbacks->onSaveInstanceState     = StAndroidGlue::onSaveInstanceState;
    theActivity->callbacks->onPause                 = StAndroidGlue::onPause;
    theActivity->callbacks->onStop                  = StAndroidGlue::onStop;
    theActivity->callbacks->onConfigurationChanged  = StAndroidGlue::onConfigurationChanged;
    theActivity->callbacks->onLowMemory             = StAndroidGlue::onLowMemory;
    theActivity->callbacks->onWindowFocusChanged    = StAndroidGlue::onWindowFocusChanged;
    theActivity->callbacks->onNativeWindowCreated   = StAndroidGlue::onNativeWindowCreated;
    theActivity->callbacks->onNativeWindowDestroyed = StAndroidGlue::onNativeWindowDestroyed;
    theActivity->callbacks->onInputQueueCreated     = StAndroidGlue::onInputQueueCreated;
    theActivity->callbacks->onInputQueueDestroyed   = StAndroidGlue::onInputQueueDestroyed;

    pthread_mutex_init(&myMutex, NULL);
    pthread_cond_init (&myCond,  NULL);

    if(theSavedState != NULL) {
        mySavedState     = ::malloc(theSavedStateSize);
        mySavedStateSize = theSavedStateSize;
        memcpy(mySavedState, theSavedState, theSavedStateSize);
    }

    int aMsgPipe[2];
    if(::pipe(aMsgPipe)) {
        ST_ERROR_LOG("could not create pipe: " + strerror(errno));
        return;
    }
    myMsgRead  = aMsgPipe[0];
    myMsgWrite = aMsgPipe[1];
}

void StAndroidGlue::readOpenPath() {
    JNIEnv* aJniEnv = myActivity->env;

    jclass      aJClassActivity = aJniEnv->GetObjectClass(myActivity->clazz);
    jmethodID   aJMet_getIntent = aJniEnv->GetMethodID(aJClassActivity, "getIntent", "()Landroid/content/Intent;");
    jmethodID   aJMet_setIntent = aJniEnv->GetMethodID(aJClassActivity, "setIntent", "(Landroid/content/Intent;)V");
    jobject     aJIntent        = aJniEnv->CallObjectMethod(myActivity->clazz, aJMet_getIntent);
    if(aJIntent == NULL) {
        return;
    }

    jclass      aJClassIntent       = aJniEnv->GetObjectClass(aJIntent);
    jmethodID   aJMet_getDataString = aJniEnv->GetMethodID(aJClassIntent, "getDataString", "()Ljava/lang/String;");
    jmethodID   aJMet_getType       = aJniEnv->GetMethodID(aJClassIntent, "getType",       "()Ljava/lang/String;");

    // retrieve data path
    jstring     aJString      = (jstring )aJniEnv->CallObjectMethod(aJIntent, aJMet_getDataString);
    const char* aJStringStr   = aJniEnv->GetStringUTFChars(aJString, 0);
    StString aDataPath = aJStringStr;
    aJniEnv->ReleaseStringUTFChars(aJString, aJStringStr);

    // retrieve data type
    aJString      = (jstring )aJniEnv->CallObjectMethod(aJIntent, aJMet_getType);
    aJStringStr   = aJniEnv->GetStringUTFChars(aJString, 0);
    const StString aDataType = aJStringStr;
    aJniEnv->ReleaseStringUTFChars(aJString, aJStringStr);

    // reset intent in Activity
    aJniEnv->CallVoidMethod(myActivity->clazz, aJMet_setIntent, NULL);

    const StString ST_FILE_PROTOCOL("file://");
    if(aDataPath.isStartsWith(ST_FILE_PROTOCOL)) {
        const size_t   aCutFrom = ST_FILE_PROTOCOL.getLength();
        const StString aPath    = aDataPath.subString(aCutFrom, (size_t )-1);
        aDataPath.fromUrl(aPath);
    }

    StMutexAuto aLock(myFetchLock);
    myDndPath = aDataPath;
}

void StAndroidGlue::fetchState(StString&             theNewFile,
                               StQuaternion<double>& theQuaternion) {
    StMutexAuto aLock(myFetchLock);
    theQuaternion = myQuaternion;
    if(!myDndPath.isEmpty()) {
        theNewFile = myDndPath;
        myDndPath.clear();
    }
}

void StAndroidGlue::start() {
    // workaround NativeActivity design issues - notify Java StActivity class about C++ pointer to StAndroidGlue instance
    JNIEnv* aJniEnv = myActivity->env;
    jclass    aJClassActivity   = aJniEnv->GetObjectClass(myActivity->clazz);
    jmethodID aJMet_setInstance = aJniEnv->GetMethodID(aJClassActivity, "setCppInstance", "(J)V");
    aJniEnv->CallVoidMethod(myActivity->clazz, aJMet_setInstance, (jlong )this);

    myThread = new StThread(threadEntryWrapper, this, "StAndroidGlue");

    // Wait for thread to start
    pthread_mutex_lock(&myMutex);
    while(!myIsRunning) {
        pthread_cond_wait(&myCond, &myMutex);
    }
    pthread_mutex_unlock(&myMutex);
}

StAndroidGlue::~StAndroidGlue() {
    // workaround NativeActivity design issues - notify Java StActivity class about C++ pointer to StAndroidGlue instance
    if(myActivity != NULL
    && myActivity->env != NULL) {
        JNIEnv* aJniEnv = myActivity->env;
        jclass    aJClassActivity   = aJniEnv->GetObjectClass(myActivity->clazz);
        jmethodID aJMet_setInstance = aJniEnv->GetMethodID(aJClassActivity, "setCppInstance", "(J)V");
        aJniEnv->CallVoidMethod(myActivity->clazz, aJMet_setInstance, (jlong )0);
    }

    pthread_mutex_lock(&myMutex);
    writeCommand(CommandId_Destroy);
    pthread_mutex_unlock(&myMutex);

    if(!myThread.isNull()) {
        myThread->wait();
    }

    ::close(myMsgRead);
    ::close(myMsgWrite);
    pthread_cond_destroy (&myCond);
    pthread_mutex_destroy(&myMutex);
    myActivity->instance = NULL;
}

void StAndroidGlue::updateMonitors() {
    if(myConfig == NULL) {
        return;
    }

    const int32_t aWidthDp  = AConfiguration_getScreenWidthDp (myConfig);
    const int32_t aHeightDp = AConfiguration_getScreenHeightDp(myConfig);
    const int32_t aDpi      = AConfiguration_getDensity       (myConfig);

    StMonitor aMon;
    aMon.setId(0);
    aMon.changeVRect().top()    = 0;
    aMon.changeVRect().left()   = 0;
    aMon.changeVRect().right()  = (int )(0.5 + double(aWidthDp ) * (double(aDpi) / 160.0));
    aMon.changeVRect().bottom() = (int )(0.5 + double(aHeightDp) * (double(aDpi) / 160.0));

    aMon.setOrientation(AConfiguration_getOrientation(myConfig) == ACONFIGURATION_ORIENTATION_PORT
                      ? StMonitor::Orientation_Portrait : StMonitor::Orientation_Landscape);
    aMon.setScale(float(double(aDpi) / 160.0));
    StSearchMonitors::setupGlobalDisplay(aMon);
}

void StAndroidGlue::postToast(const char* theInfo) {
    if(myThJniEnv == NULL) {
        return;
    }

    jclass  aJClassAct = myThJniEnv->GetObjectClass(myActivity->clazz);
    if(aJClassAct == NULL) {
        ST_ERROR_LOG("StAndroidGlue::postToast() - class is unavailable!");
        return;
    }

    jmethodID aJMetId = myThJniEnv->GetMethodID(aJClassAct, "postToast", "(Ljava/lang/String;)V");
    if(aJMetId == NULL) {
        ST_ERROR_LOG("StAndroidGlue::postToast() - method is unavailable!");
        return;
    }

    jstring aJStr = myThJniEnv->NewStringUTF(theInfo);
    myThJniEnv->CallVoidMethod(myActivity->clazz, aJMetId, aJStr);
    myThJniEnv->DeleteLocalRef (aJStr);
}

void StAndroidGlue::postMessage(const char* theInfo) {
    if(myThJniEnv == NULL) {
        return;
    }

    jclass  aJClassAct = myThJniEnv->GetObjectClass(myActivity->clazz);
    if(aJClassAct == NULL) {
        ST_ERROR_LOG("StAndroidGlue::postMessage() - class is unavailable!");
        return;
    }

    jmethodID aJMetId = myThJniEnv->GetMethodID(aJClassAct, "postMessage", "(Ljava/lang/String;)V");
    if(aJMetId == NULL) {
        ST_ERROR_LOG("StAndroidGlue::postMessage() - method is unavailable!");
        return;
    }

    jstring aJStr = myThJniEnv->NewStringUTF(theInfo);
    myThJniEnv->CallVoidMethod(myActivity->clazz, aJMetId, aJStr);
    myThJniEnv->DeleteLocalRef (aJStr);
}

void StAndroidGlue::postExit() {
    if(myThJniEnv == NULL) {
        return;
    }

    jclass  aJClassAct = myThJniEnv->GetObjectClass(myActivity->clazz);
    if(aJClassAct == NULL) {
        ST_ERROR_LOG("StAndroidGlue::postExit() - class is unavailable!");
        return;
    }

    jmethodID aJMetId = myThJniEnv->GetMethodID(aJClassAct, "postExit", "()V");
    if(aJMetId == NULL) {
        ST_ERROR_LOG("StAndroidGlue::postExit() - method is unavailable!");
        return;
    }

    myThJniEnv->CallVoidMethod(myActivity->clazz, aJMetId);
}

namespace {

    static StAndroidGlue* THE_ANDROID_GLUE = NULL;

    static bool msgBoxCallback(StMessageBox::MsgType theType,
                               const char*           theMessage) {
        if(THE_ANDROID_GLUE == NULL) {
            return false;
        }

        switch(theType) {
            case StMessageBox::MsgType_Info: {
                THE_ANDROID_GLUE->postToast(theMessage);
                break;
            }
            case StMessageBox::MsgType_Warning:
            case StMessageBox::MsgType_Error:
            case StMessageBox::MsgType_Question: {
                THE_ANDROID_GLUE->postMessage(theMessage);
                break;
            }
        }
        return false;
    }

}

void StAndroidGlue::threadEntry() {
    if(myJavaVM->AttachCurrentThread(&myThJniEnv, NULL) < 0) {
        ST_ERROR_LOG("Failed to attach working thread to Java VM");
        return;
    }

    THE_ANDROID_GLUE = this;
    StMessageBox::setCallback(msgBoxCallback);

    myConfig = AConfiguration_new();
    AConfiguration_fromAssetManager(myConfig, myActivity->assetManager);
    updateMonitors();
    printConfig();

    ALooper* aLooper = ALooper_prepare(ALOOPER_PREPARE_ALLOW_NON_CALLBACKS);
    ALooper_addFd(aLooper, myMsgRead, LooperId_MAIN, ALOOPER_EVENT_INPUT, NULL, &myCmdPollSource);
    myLooper = aLooper;

    pthread_mutex_lock(&myMutex);
    myIsRunning = true;
    pthread_cond_broadcast(&myCond);
    pthread_mutex_unlock(&myMutex);

    // try to load stereo APIs
    /**jclass aJClass_Real3D = myThJniEnv->FindClass("com/lge/real3d/Real3D");
    if(aJClass_Real3D != NULL) {
        jmethodID aJMet_isStereoDisplayAvailable = myThJniEnv->GetStaticMethodID(aJClass_Real3D, "isStereoDisplayAvailable", "(Landroid/content/Contex;)Z");
        postMessage("com.lge.real3d.Real3D !!!");
    }

    jclass aJClass_HTC = myThJniEnv->FindClass("com/htc/view/DisplaySetting");
    if(aJClass_HTC != NULL) {
        jmethodID aJMet_isStereoDisplayAvailable = myThJniEnv->GetStaticMethodID(aJClass_HTC, "setStereoscopic3DFormat", "(Landroid/view/Surface;I)Z");
        postMessage("com.htc.view.DisplaySetting !!!");
    }

    jclass aJClass_Sharp = myThJniEnv->FindClass("jp/co/sharp/android/stereo3dlcd/SurfaceController");
    if(aJClass_Sharp != NULL) {
        jmethodID aJMet_setStereoView = myThJniEnv->GetMethodID(aJClass_Sharp, "setStereoView", "(Z)V");
        postMessage("jp.co.sharp.android.stereo3dlcd !!!");
    }*/

    createApplication();
    if(!myApp.isNull()) {
        if(!myApp->open()) {
            stError("Error: application can not be executed!");
        }
        myApp->exec();
    } else {
        stError("Error: no application to execute!");
    }
    myApp.nullify();

    // application is done but we are waiting for destroying event...
    bool isFirstWait = true;
    for(; !myToDestroy; ) {
        if(isFirstWait) {
            postExit();
            isFirstWait = false;
        }

        StAndroidPollSource* aSource = NULL;
        int aNbEvents = 0;
        ALooper_pollAll(-1, NULL, &aNbEvents, (void** )&aSource);
        if(aSource != NULL) {
            aSource->process(this, aSource);
        }
    }

    freeSavedState();
    pthread_mutex_lock(&myMutex);
    if(myInputQueue != NULL) {
        AInputQueue_detachLooper(myInputQueue);
    }
    AConfiguration_delete(myConfig);
    pthread_cond_broadcast(&myCond);
    pthread_mutex_unlock(&myMutex);

    myThJniEnv = NULL;
    StMessageBox::setCallback(NULL);
    THE_ANDROID_GLUE = NULL;

    myJavaVM->DetachCurrentThread();
}

bool StAndroidGlue::writeCommand(const int8_t theCmd) {
    if(::write(myMsgWrite, &theCmd, sizeof(theCmd)) != sizeof(theCmd)) {
        ST_ERROR_LOG("Failure writing StAndroidGlue cmd: " + strerror(errno));
        return false;
    }
    return true;
}

void StAndroidGlue::freeSavedState() {
    pthread_mutex_lock(&myMutex);
    if(mySavedState != NULL) {
        ::free(mySavedState);
        mySavedState     = NULL;
        mySavedStateSize = 0;
    }
    pthread_mutex_unlock(&myMutex);
}

void StAndroidGlue::printConfig() {
    char aLang[3], aCountry[3];
    AConfiguration_getLanguage(myConfig, aLang);
    AConfiguration_getCountry (myConfig, aCountry);
    aLang[2]    = '\0';
    aCountry[2] = '\0';
    ST_DEBUG_LOG("Config:"
               + " mcc="       + AConfiguration_getMcc(myConfig)
               + " mnc="       + AConfiguration_getMnc(myConfig)
               + " lang="      + aLang + " cnt=" + aCountry
               + " orien="     + AConfiguration_getOrientation(myConfig)
               + " touch="     + AConfiguration_getTouchscreen(myConfig)
               + " dens="      + AConfiguration_getDensity    (myConfig)
               + " keys="      + AConfiguration_getKeyboard   (myConfig)
               + " nav="       + AConfiguration_getNavigation (myConfig)
               + " keysHid="   + AConfiguration_getKeysHidden (myConfig)
               + " navHid="    + AConfiguration_getNavHidden  (myConfig)
               + " sdk="       + AConfiguration_getSdkVersion (myConfig)
               + " size="      + AConfiguration_getScreenSize (myConfig)
               + " long="      + AConfiguration_getScreenLong (myConfig)
               + " modetype="  + AConfiguration_getUiModeType (myConfig)
               + " modenight=" + AConfiguration_getUiModeNight(myConfig));
}

void StAndroidGlue::processInput() {
    for(AInputEvent* anEvent = NULL; AInputQueue_getEvent(myInputQueue, &anEvent) >= 0;) {
        if(AInputQueue_preDispatchEvent(myInputQueue, anEvent)) {
            continue;
        }

        bool isHandled = false;
        signals.onInputEvent(anEvent, isHandled);
        AInputQueue_finishEvent(myInputQueue, anEvent, isHandled ? 1 : 0);
    }
}

void StAndroidGlue::processCommand() {
    int8_t aCmd = -1;
    if(::read(myMsgRead, &aCmd, sizeof(aCmd)) == sizeof(aCmd)) {
        switch(aCmd) {
            case CommandId_SaveState:
                freeSavedState();
                break;
        }
    } else {
        ST_ERROR_LOG("No data on command pipe!");
        return;
    }

    // preprocessing
    switch(aCmd) {
        case CommandId_InputChanged: {
            pthread_mutex_lock(&myMutex);
            if(myInputQueue != NULL) {
                AInputQueue_detachLooper(myInputQueue);
            }
            myInputQueue = myInputQueuePending;
            if(myInputQueue != NULL) {
                AInputQueue_attachLooper(myInputQueue,
                                         myLooper, LooperId_INPUT, NULL,
                                         &myInputPollSource);
            }
            pthread_cond_broadcast(&myCond);
            pthread_mutex_unlock(&myMutex);
            break;
        }
        case CommandId_WindowInit: {
            pthread_mutex_lock(&myMutex);
            myWindow = myWindowPending;
            pthread_cond_broadcast(&myCond);
            pthread_mutex_unlock(&myMutex);
            break;
        }
        case CommandId_WindowTerm: {
            pthread_cond_broadcast(&myCond);
            break;
        }
        case CommandId_Resume:
        case CommandId_Start:
        case CommandId_Pause:
        case CommandId_Stop: {
            pthread_mutex_lock(&myMutex);
            myActivityState = aCmd;
            pthread_cond_broadcast(&myCond);
            pthread_mutex_unlock(&myMutex);
            break;
        }
        case CommandId_ConfigChanged: {
            AConfiguration_fromAssetManager(myConfig, myActivity->assetManager);
            updateMonitors();
            printConfig();
            break;
        }
        case CommandId_Destroy: {
            myToDestroy = true;
            break;
        }
    }

    signals.onAppCmd(aCmd);

    // post-processing
    switch(aCmd) {
        case CommandId_WindowTerm: {
            pthread_mutex_lock(&myMutex);
            myWindow = NULL;
            pthread_cond_broadcast(&myCond);
            pthread_mutex_unlock(&myMutex);
            break;
        }
        case CommandId_SaveState: {
            pthread_mutex_lock(&myMutex);
            myIsStateSaved = true;
            pthread_cond_broadcast(&myCond);
            pthread_mutex_unlock(&myMutex);
            break;
        }
        case CommandId_Resume: {
            freeSavedState();
            break;
        }
    }
}

void StAndroidGlue::setInput(AInputQueue* theInputQueue) {
    pthread_mutex_lock(&myMutex);
    myInputQueuePending = theInputQueue;
    writeCommand(CommandId_InputChanged);
    while(myInputQueue != myInputQueuePending) {
        pthread_cond_wait(&myCond, &myMutex);
    }
    pthread_mutex_unlock(&myMutex);
}

void StAndroidGlue::setWindow(ANativeWindow* theWindow) {
    pthread_mutex_lock(&myMutex);
    if(myWindowPending != NULL) {
        writeCommand(CommandId_WindowTerm);
    }
    myWindowPending = theWindow;
    if(theWindow != NULL) {
        writeCommand(CommandId_WindowInit);
    }
    while(myWindow != myWindowPending) {
        pthread_cond_wait(&myCond, &myMutex);
    }
    pthread_mutex_unlock(&myMutex);
}

void StAndroidGlue::setActivityState(int8_t theState) {
    if(theState == StAndroidGlue::CommandId_Resume) {
        readOpenPath();
    }

    pthread_mutex_lock(&myMutex);
    writeCommand(theState);
    while(myActivityState != theState) {
        pthread_cond_wait(&myCond, &myMutex);
    }
    pthread_mutex_unlock(&myMutex);
}

void StAndroidGlue::onDestroy(ANativeActivity* theActivity) {
    StAndroidGlue* anApp = (StAndroidGlue* )theActivity->instance;
    delete anApp;
}

void* StAndroidGlue::saveInstanceState(size_t* theOutLen) {
    pthread_mutex_lock(&myMutex);
    myIsStateSaved = false;
    writeCommand(CommandId_SaveState);
    while(!myIsStateSaved) {
        pthread_cond_wait(&myCond, &myMutex);
    }

    void* aSavedState = NULL;
    if(mySavedState != NULL) {
        aSavedState = mySavedState;
        *theOutLen  = mySavedStateSize;
        mySavedState     = NULL;
        mySavedStateSize = 0;
    }

    pthread_mutex_unlock(&myMutex);
    return aSavedState;
}

void StAndroidGlue::setTrackOrientation(const bool theToTrack) {
    if(myToTrackOrient == theToTrack
    || myActivity == NULL
    || myThJniEnv == NULL) {
        return;
    }

    jclass    aJClassActivity = myThJniEnv->GetObjectClass(myActivity->clazz);
    jmethodID aJMet           = myThJniEnv->GetMethodID(aJClassActivity, "setTrackOrientation", "(Z)V");
    myThJniEnv->CallVoidMethod(myActivity->clazz, aJMet, (jboolean )(theToTrack ? JNI_TRUE : JNI_FALSE));
    myToTrackOrient = theToTrack;
}

void StAndroidGlue::setQuaternion(const StQuaternion<float>& theQ, const float theScreenRotDeg) {
    // do the magic - convert quaternion from Android coordinate system to sView coordinate system
    const StQuaternion<double> anOriPitch = StQuaternion<double>(StVec3<double>::DX(), stToRadians(90.0));
    const StQuaternion<double> anOriRoll  = StQuaternion<double>(StVec3<double>::DZ(), stToRadians(-270.0 + (double )theScreenRotDeg));
    const StQuaternion<double> anOriAnd((double )theQ.y(), (double )theQ.z(), (double )theQ.x(), (double )theQ.w());
    StQuaternion<double> anOri = StQuaternion<double>::multiply(anOriAnd, anOriPitch);
    anOri = StQuaternion<double>::multiply(anOriRoll, anOri);

    StMutexAuto aLock(myFetchLock);
    myQuaternion = anOri;
}

void StAndroidGlue::setOrientation(float theAzimuthDeg, float thePitchDeg, float theRollDeg, float theScreenRotDeg) {
    const StQuaternion<double> anOriYaw   = StQuaternion<double>(StVec3<double>::DY(), stToRadians((double )theAzimuthDeg));
    const StQuaternion<double> anOriPitch = StQuaternion<double>(StVec3<double>::DX(), stToRadians(90.0 + (double )thePitchDeg));
    const StQuaternion<double> anOriRoll  = StQuaternion<double>(StVec3<double>::DZ(), stToRadians((double )-theRollDeg + (double )theScreenRotDeg));
    StQuaternion<double> anOri = StQuaternion<double>::multiply(anOriPitch, anOriYaw);
    anOri = StQuaternion<double>::multiply(anOriRoll, anOri);

    StMutexAuto aLock(myFetchLock);
    myQuaternion = anOri;
}

jexp void JNICALL Java_com_sview_StActivity_cppDefineOrientationSensor(JNIEnv* theEnv, jobject theObj, jlong theCppPtr,
                                                                       jboolean theHasOri, jboolean theIsPoorOri) {
    ((StAndroidGlue* )theCppPtr)->defineOrientationSensor(theHasOri == JNI_TRUE, theIsPoorOri == JNI_TRUE);
}

jexp void JNICALL Java_com_sview_StActivity_cppSetQuaternion(JNIEnv* theEnv, jobject theObj, jlong theCppPtr,
                                                             jfloat theX, jfloat theY, jfloat theZ, jfloat theW,
                                                             jfloat theScreenRotDeg) {
    ((StAndroidGlue* )theCppPtr)->setQuaternion(StQuaternion<float>(theX, theY, theZ, theW), theScreenRotDeg);
}

jexp void JNICALL Java_com_sview_StActivity_cppSetOrientation(JNIEnv* theEnv, jobject theObj, jlong theCppPtr,
                                                              jfloat theAzimuthDeg, jfloat thePitchDeg, jfloat theRollDeg,
                                                              jfloat theScreenRotDeg) {
    ((StAndroidGlue* )theCppPtr)->setOrientation(theAzimuthDeg, thePitchDeg, theRollDeg, theScreenRotDeg);
}

#endif // __ANDROID__
