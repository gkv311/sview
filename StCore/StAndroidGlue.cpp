/**
 * Copyright © 2014 Kirill Gavrilov <kirill@sview.ru>
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

#include <StCore/StSearchMonitors.h>
#include <StTemplates/StHandle.h>
#include <StThreads/StThread.h>
#include <StStrings/StLogger.h>

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
: onAppEntry(NULL),
  myActivity(theActivity),
  myConfig(NULL),
  myLooper(NULL),
  myInputQueue(NULL),
  myInputQueuePending(NULL),
  myWindow(NULL),
  myWindowPending(NULL),
  myActivityState(0),
  mySavedState(NULL),
  mySavedStateSize(0),
  myJavaVM(NULL),
  myThJniEnv(NULL),
  myMsgRead(0),
  myMsgWrite(0),
  myIsRunning(false),
  myIsStateSaved(false),
  myToDestroy(false),
  myIsDestroyed(false) {
    theActivity->instance = this;

    JNIEnv* aJniEnv = theActivity->env;
    aJniEnv->GetJavaVM(&myJavaVM);

    jclass      anActivityJClass = aJniEnv->GetObjectClass(theActivity->clazz);
    jmethodID   anActivityJMetId = aJniEnv->GetMethodID(anActivityJClass, "getIntent", "()Landroid/content/Intent;");
    jobject     aJIntent         = aJniEnv->CallObjectMethod(theActivity->clazz, anActivityJMetId);
    jclass      aJIntentClass    = aJniEnv->GetObjectClass(aJIntent);

    // retrieve data path
    jmethodID   aJIntentMetId = aJniEnv->GetMethodID(aJIntentClass, "getDataString", "()Ljava/lang/String;");
    jstring     aJString      = (jstring )aJniEnv->CallObjectMethod(aJIntent, aJIntentMetId);
    const char* aJStringStr   = aJniEnv->GetStringUTFChars(aJString, 0);
    myDataPath = aJStringStr;
    aJniEnv->ReleaseStringUTFChars(aJString, aJStringStr);

    // retrieve data type
    aJIntentMetId = aJniEnv->GetMethodID(aJIntentClass, "getType", "()Ljava/lang/String;");
    aJString      = (jstring )aJniEnv->CallObjectMethod(aJIntent, aJIntentMetId);
    aJStringStr   = aJniEnv->GetStringUTFChars(aJString, 0);
    myDataType = aJStringStr;
    aJniEnv->ReleaseStringUTFChars(aJString, aJStringStr);

    const StString ST_FILE_PROTOCOL("file://");
    if(myDataPath.isStartsWith(ST_FILE_PROTOCOL)) {
        const size_t   aCutFrom = ST_FILE_PROTOCOL.getLength();
        const StString aPath    = myDataPath.subString(aCutFrom, (size_t )-1);
        myDataPath.fromUrl(aPath);
    }

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

void StAndroidGlue::start() {
    StHandle<StThread> aThread = new StThread(threadEntryWrapper, this);
    aThread->detach();

    // Wait for thread to start
    pthread_mutex_lock(&myMutex);
    while(!myIsRunning) {
        pthread_cond_wait(&myCond, &myMutex);
    }
    pthread_mutex_unlock(&myMutex);
}

StAndroidGlue::~StAndroidGlue() {
    pthread_mutex_lock(&myMutex);
    writeCommand(CommandId_Destroy);
    while(!myIsDestroyed) {
        pthread_cond_wait(&myCond, &myMutex);
    }
    pthread_mutex_unlock(&myMutex);

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
    if(onAppEntry == NULL) {
        return;
    } else if(myJavaVM->AttachCurrentThread(&myThJniEnv, NULL) < 0) {
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

    onAppEntry(this);

    // application is done but we are waiting for destroying event...
    for(; !myToDestroy; ) {
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
    myIsDestroyed = true;
    pthread_cond_broadcast(&myCond);
    pthread_mutex_unlock(&myMutex);

    myThJniEnv = NULL;
    StMessageBox::setCallback(NULL);
    THE_ANDROID_GLUE = NULL;
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

#endif // __ANDROID__
