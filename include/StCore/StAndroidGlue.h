/**
 * StCore, window system independent C++ toolkit for writing OpenGL applications.
 * Copyright © 2014-2017 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#if defined(__ANDROID__)

#ifndef __StAndroidGlue_h_
#define __StAndroidGlue_h_

#include "StVirtualKeys.h"

#include <StSlots/StSignal.h>
#include <StStrings/StString.h>
#include <StThreads/StMutex.h>
#include <StThreads/StThread.h>
#include <StTemplates/StQuaternion.h>

#include <android/configuration.h>
#include <android/native_activity.h>

class StAndroidGlue;
class StApplication;
class StKeysState;

/**
 * Data associated with an ALooper fd that will be returned as the "outData" when that source has data ready.
 */
struct StAndroidPollSource {
    int32_t        id;  //!< the identifier of this source, LOOPER_ID_MAIN or LOOPER_ID_INPUT
    StAndroidGlue* app; //!< associated instance

    /**
     * Function to call to perform the standard processing of data from this source.
     */
    void (*process)(struct StAndroidGlue* theApp, StAndroidPollSource* theSource);
};

/**
 * Interface for the standard glue code of a threaded application.
 */
class StAndroidGlue {

        public:

    /**
     * Identifiers from ALooper_pollOnce().
     */
    enum LooperId {
        LooperId_MAIN  = 1, //!< commands coming from the main thread
        LooperId_INPUT = 2, //!< events coming from the AInputQueue of the application's window
        LooperId_USER  = 3, //!< user-defined ALooper identifiers
    };

    /**
     * Commands from main thread.
     */
    enum CommandId {
        CommandId_InputChanged,  //!< the AInputQueue has changed
        CommandId_WindowInit,    //!< a new ANativeWindow is ready for use
        CommandId_WindowTerm,    //!< existing ANativeWindow needs to be terminated
        CommandId_WindowResize,  //!< the current ANativeWindow has been resized (and should be redrawn)
        CommandId_WindowRedraw,  //!< the system needs that the current ANativeWindow be redrawn
        CommandId_FocusGained,   //!< activity window has gained input focus
        CommandId_FocusLost,     //!< activity window has lost input focus
        CommandId_ConfigChanged, //!< the current device configuration has changed
        CommandId_LowMemory,     //!< the system is running low on memory
        CommandId_Start,         //!< activity has been started
        CommandId_Resume,        //!< activity has been resumed
        CommandId_SaveState,     //!< application should generate a new saved state, to restore from later if needed
        CommandId_Pause,         //!< activity has been paused
        CommandId_Stop,          //!< activity has been stopped
        CommandId_Destroy,       //!< activity is being destroyed, and waiting for the application thread to clean up and exit before proceeding
        CommandId_BackPressed,   //!< pressed special back button
        CommandId_WindowChanged, //!< the current ANativeWindow has been changed
    };

    /**
     * Auxiliary function to dump command id.
     */
    static StCString getCommandIdName(StAndroidGlue::CommandId theCmd);

    /**
     * Retrieve path using android.os.Environment.getExternalStoragePublicDirectory().
     */
    static StString getStoragePath(JNIEnv*     theJEnv,
                                   const char* theType);

        public: //! @name interface to implement

    /**
     * Choose and instantiate StApplication.
     */
    virtual void createApplication() = 0;

        public: //! @name public API

    /**
     * Default constructor.
     */
    ST_CPPEXPORT StAndroidGlue(ANativeActivity* theActivity,
                               void*            theSavedState,
                               size_t           theSavedStateSize);

    /**
     * Start application - onAppEntry() callback should be set before.
     */
    ST_CPPEXPORT void start();

    /**
     * Destructor.
     */
    ST_CPPEXPORT virtual ~StAndroidGlue();

    /**
     * ANativeActivity object instance that this application is running in.
     */
    ST_LOCAL ANativeActivity* getActivity() const {
        return myActivity;
    }

    /**
     * When non-NULL, this is the window surface that the application can draw in.
     */
    ST_LOCAL ANativeWindow* getWindow() const {
        return myWindow;
    }

    /**
     * ALooper associated with the application thread.
     */
    ST_LOCAL ALooper* getLooper() const {
        return myLooper;
    }

    /**
     * Should be checked by application to exit.
     */
    ST_LOCAL bool ToDestroy() const {
        return myToDestroy;
    }

    ST_LOCAL void* getSavedState() const {
       return mySavedState;
    }

    ST_LOCAL void setSavedState(void*  theSavedState,
                                size_t theSavedStateSize) {
        mySavedState     = theSavedState;
        mySavedStateSize = theSavedStateSize;
    }

    /**
     * Setup window flags (see ANativeActivity_setWindowFlags()).
     */
    ST_CPPEXPORT void setWindowFlags(const int theFlags);

    /**
     * Setup window title.
     */
    ST_CPPEXPORT void setWindowTitle(const StString& theTitle);

    /**
     * Set/release WAKE_LOCK.
     */
    ST_CPPEXPORT void setWakeLock(const StString& theTitle, bool theToLock);

    /**
     * Turn stereo output on using device-specific API.
     */
    ST_CPPEXPORT void setHardwareStereoOn(const bool theToEnable);

    /**
     * Return true if device has orientation sensor.
     */
    ST_LOCAL bool hasOrientationSensor() const { return myHasOrientSensor; }

    /**
     * Return true if orientation sensor has poor quality.
     */
    ST_LOCAL bool isPoorOrientationSensor() const { return myIsPoorOrient; }

    /**
     * Turn orientation sensor on/off.
     * Has no effect in case if sensor is unavailable.
     */
    ST_CPPEXPORT void setTrackOrientation(bool theToTrack);

    /**
     * Setup visibility of system bars.
     */
    ST_CPPEXPORT void setHideSystemBars(bool theToHideStatusBar,
                                        bool theToHideNavBar);

    /**
     * Fetch current state:
     * @param theNewFile pop onNewIntent() open file event
     * @param theQuaternion device orientation
     * @param theToSwapEyes swap left/right views
     * @param theKeys       keys state
     */
    ST_CPPEXPORT void fetchState(StString&             theNewFile,
                                 StQuaternion<double>& theQuaternion,
                                 bool&                 theToSwapEyes,
                                 const StKeysState&    theKeys);

    /**
     * Return device memory class.
     */
    ST_LOCAL int getMemoryClass() const { return myMemoryClassMiB; }

    /**
     * Open file descriptor for specified path using contentResolver, including content:// URLs.
     * @return file descriptor, which should be closed by caller, or -1 on error
     */
    ST_CPPEXPORT int openFileDescriptor(const StString& thePath);

        public:

    /**
     * Post small message.
     */
    ST_CPPEXPORT void postToast(const char* theInfo);

    /**
     * Post message.
     */
    ST_CPPEXPORT void postMessage(const char* theInfo);

    /**
     * Post exit.
     */
    ST_CPPEXPORT void postExit();

    /**
     * Low-level method to write command into queue.
     */
    ST_CPPEXPORT bool writeCommand(const int8_t theCmd);

        public:  //! @name Signals

    struct {
        /**
         * Emit callback Slot on input event.
         * At this point the event has already been pre-dispatched, and it will be finished upon return.
         * @param theEvent       (const AInputEvent* ) input event
         * @param theIsProcessed (bool& ) should be set to true if event has been handled, false for any default dispatching
         */
        StSignal<void (const AInputEvent* , bool& )> onInputEvent;

        /**
         * Callback to process main app commands (APP_CMD_*).
         * @param theCommand (int32_t ) command to process
         */
        StSignal<void (int32_t )> onAppCmd;
    } signals;

        protected: //! @name low-level implementation

    /**
     * Read the open file from currently set intent.
     * The method calls the Java method readOpenPath() of StActivity which will call setOpenPath().
     */
    ST_CPPEXPORT void readOpenPath(bool theToNullifyIntent);

    ST_CPPEXPORT void printConfig();
    ST_CPPEXPORT void freeSavedState();

    ST_CPPEXPORT void processInput();

    ST_CPPEXPORT void processCommand();

    ST_CPPEXPORT void* saveInstanceState(size_t* theOutLen);

    ST_CPPEXPORT void setInput(AInputQueue* theInputQueue);
    ST_CPPEXPORT void setWindow(ANativeWindow* theWindow);
    ST_CPPEXPORT void setActivityState(int8_t theState);

    /**
     * Update global monitors configuration.
     */
    ST_LOCAL void updateMonitors();

    ST_LOCAL static SV_THREAD_FUNCTION threadEntryWrapper(void* theParam) {
        StAndroidGlue* anApp = (StAndroidGlue* )theParam;
        anApp->threadEntry();
        return SV_THREAD_RETURN 0;
    }

    ST_CPPEXPORT void threadEntry();

        public: //! @name StActivity callbacks

    /**
     * Setup new open file from Java class.
     */
    ST_CPPEXPORT void setOpenPath(const jstring  theOpenPath,
                                  const jstring  theMimeType,
                                  const jboolean theIsLaunchedFromHistory);

    /**
     * Setup surfaceChanging flag to workaround NativeActivity API issue
     * causing two events setWindow() being called instead of a one.
     */
    ST_CPPEXPORT void setChangingSurface(bool theIsChanging);

    /**
     * Define device orientation sensor.
     * @param theHasSensor flag indicating that device has orientation sensors
     * @param theIsPoor    flag indicating that available orientation sensor provides imprecise values
     */
    ST_LOCAL void defineOrientationSensor(bool theHasSensor,
                                          bool theIsPoor) {
        myHasOrientSensor = theHasSensor;
        myIsPoorOrient    = theIsPoor;
    }

    /**
     * Define device orientation by quaternion.
     */
    ST_LOCAL void setQuaternion(const StQuaternion<float>& theQ, const float theScreenRotDeg);

    /**
     * Define device orientation using deprecated Android API.
     */
    ST_LOCAL void setOrientation(float theAzimuthDeg, float thePitchDeg, float theRollDeg, float theScreenRotDeg);

    /**
     * Define device Left/Right eyes swap flag.
     */
    ST_LOCAL void setSwapEyes(bool theToSwapLR);

    /**
     * Return TRUE if key is processed by application.
     */
    ST_LOCAL bool isKeyOverridden(int theKeyCode);

        private: //! @name ANativeActivity callbacks

    ST_LOCAL static void processInputWrapper(StAndroidGlue*       theApp,
                                             StAndroidPollSource* /*theSource*/) {
        theApp->processInput();
    }

    ST_LOCAL static void processCommandWrapper(StAndroidGlue*       theApp,
                                               StAndroidPollSource* /*theSource*/) {
        theApp->processCommand();
    }

    ST_CPPEXPORT static void onDestroy(ANativeActivity* theActivity);

    ST_LOCAL static void onStart(ANativeActivity* theActivity) {
        StAndroidGlue* anApp = (StAndroidGlue* )theActivity->instance;
        anApp->setActivityState(CommandId_Start);
    }

    ST_LOCAL static void onResume(ANativeActivity* theActivity) {
        StAndroidGlue* anApp = (StAndroidGlue* )theActivity->instance;
        anApp->setActivityState(CommandId_Resume);
    }

    ST_LOCAL static void onPause(ANativeActivity* theActivity) {
        StAndroidGlue* anApp = (StAndroidGlue* )theActivity->instance;
        anApp->setActivityState(CommandId_Pause);
    }

    ST_LOCAL static void onStop(ANativeActivity* theActivity) {
        StAndroidGlue* anApp = (StAndroidGlue* )theActivity->instance;
        anApp->setActivityState(CommandId_Stop);
    }

    ST_LOCAL static void* onSaveInstanceState(ANativeActivity* theActivity,
                                              size_t*          theOutLen) {
        StAndroidGlue* anApp = (StAndroidGlue* )theActivity->instance;
        return anApp->saveInstanceState(theOutLen);
    }

    ST_LOCAL static void onConfigurationChanged(ANativeActivity* theActivity) {
        StAndroidGlue* anApp = (StAndroidGlue* )theActivity->instance;
        anApp->writeCommand(CommandId_ConfigChanged);
    }

    ST_LOCAL static void onLowMemory(ANativeActivity* theActivity) {
        StAndroidGlue* anApp = (StAndroidGlue* )theActivity->instance;
        anApp->writeCommand(CommandId_LowMemory);
    }

    ST_LOCAL static void onWindowFocusChanged(ANativeActivity* theActivity, int theIsFocused) {
        StAndroidGlue* anApp = (StAndroidGlue* )theActivity->instance;
        anApp->writeCommand(theIsFocused ? CommandId_FocusGained : CommandId_FocusLost);
    }

    ST_LOCAL static void onNativeWindowCreated(ANativeActivity* theActivity,
                                               ANativeWindow*   theWindow) {
        StAndroidGlue* anApp = (StAndroidGlue* )theActivity->instance;
        anApp->setWindow(theWindow);
    }

    ST_LOCAL static void onNativeWindowDestroyed(ANativeActivity* theActivity,
                                                 ANativeWindow*   theWindow) {
        StAndroidGlue* anApp = (StAndroidGlue* )theActivity->instance;
        anApp->setWindow(NULL);
    }

    ST_LOCAL static void onInputQueueCreated(ANativeActivity* theActivity,
                                             AInputQueue*     theQueue) {
        StAndroidGlue* anApp = (StAndroidGlue* )theActivity->instance;
        anApp->setInput(theQueue);
    }

    ST_LOCAL static void onInputQueueDestroyed(ANativeActivity* theActivity,
                                               AInputQueue*     theQueue) {
        StAndroidGlue* anApp = (StAndroidGlue* )theActivity->instance;;
        anApp->setInput(NULL);
    }

        protected: //! @name protected fields

    StHandle<StThread>      myThread;            //!< application thread
    StHandle<StApplication> myApp;               //!< application instance
    ANativeActivity*        myActivity;          //!< ANativeActivity object instance that this application is running in
    AConfiguration*         myConfig;            //!< The current configuration the application is running in
    ALooper*                myLooper;            //!< ALooper associated with the application thread
    AInputQueue*            myInputQueue;        //!< input queue from which the application will receive user input events
    AInputQueue*            myInputQueuePending;
    ANativeWindow*          myWindow;            //!< native window to draw into
    ANativeWindow*          myWindowPending;
    StString                myWindowTitle;       //!< window title
    StString                myWakeLockTitle;     //!< wake lock title
    bool                    myIsChangingSurface; //!< flag indicating surface changing state
    bool                    myIsWakeLockSet;     //!< flag indicating WAKE_LOCK enabled state
    int                     myWindowFlags;       //!< active window flags
    int                     myActivityState;     //!< Current state of the app's activity (APP_CMD_START, APP_CMD_RESUME, APP_CMD_PAUSE, or APP_CMD_STOP)
    int                     myMemoryClassMiB;    //!< device memory class

    void*                   mySavedState;        //!< last instance's saved state, as provided at creation time
    size_t                  mySavedStateSize;

    JavaVM*                 myJavaVM;            //!< pointer to global Java VM instance
    JNIEnv*                 myThJniEnv;          //!< Jni environment for working thread

    pthread_mutex_t         myMutex;
    pthread_cond_t          myCond;

    int                     myMsgRead;
    int                     myMsgWrite;

    StAndroidPollSource     myCmdPollSource;
    StAndroidPollSource     myInputPollSource;

    StMutex                 myFetchLock;         //!< fetch data lock
    bool                    myRegKeys[ST_VK_NB]; //!< map of registered (used in hot-key combinations) keys
    StString                myStAppClass;        //!< application class name (e.g. image, video)
    StString                myBuildModel;        //!< android.os.Build.MODEL
    StString                myBuildDevice;       //!< android.os.Build.DEVICE
    StString                myStereoApiId;       //!< stereo API identifier
    bool                    myToEnableStereoHW;  //!< on/off state of stereo API
    StString                myDndPath;           //!< intent data string
    StString                myCreatePath;        //!< intent data string used to open this activity
    StQuaternion<double>    myQuaternion;        //!< device orientation
    bool                    myHasOrientSensor;   //!< flag indicating that device has orientation sensors
    bool                    myIsPoorOrient;      //!< flag indicating that available orientation sensor provides imprecise values
    bool                    myToTrackOrient;     //!< track device orientation
    bool                    myToHideStatusBar;   //!< hide system-provided status bar
    bool                    myToHideNavBar;      //!< hide system-provided navigation bar
    bool                    myToSwapEyesHW;      //!< flag to swap LR views on external event

    bool                    myIsRunning;
    bool                    myIsStateSaved;
    bool                    myToDestroy;         //!< this is non-zero when the application's NativeActivity is being destroyed and waiting for the application thread to complete

};

#endif // __StAndroidGlue_h_
#endif // __ANDROID__
