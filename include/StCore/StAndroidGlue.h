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

#ifndef __StAndroidGlue_h_
#define __StAndroidGlue_h_

#include <StSlots/StSignal.h>
#include <StStrings/StString.h>
#include <StThreads/StThread.h>

#include <android/configuration.h>
#include <android/native_activity.h>

class StAndroidGlue;

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
    };

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
    ST_CPPEXPORT ~StAndroidGlue();

    /**
     * Intent data type.
     */
    ST_LOCAL const StString& getDataType() const {
        return myDataType;
    }

    /**
     * Intent data path (URL).
     */
    ST_LOCAL const StString& getDataPath() const {
        return myDataPath;
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

        public:

    void (*onAppEntry)(StAndroidGlue* theApp);

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

        private: //! @name low-level implementation

    ST_CPPEXPORT bool writeCommand(const int8_t theCmd);
    ST_CPPEXPORT void printConfig();
    ST_CPPEXPORT void freeSavedState();

    ST_CPPEXPORT void processInput();

    ST_CPPEXPORT void processCommand();

    ST_CPPEXPORT void* saveInstanceState(size_t* theOutLen);

    ST_CPPEXPORT void setInput(AInputQueue* theInputQueue);
    ST_CPPEXPORT void setWindow(ANativeWindow* theWindow);
    ST_CPPEXPORT void setActivityState(int8_t theState);

    ST_LOCAL static SV_THREAD_FUNCTION threadEntryWrapper(void* theParam) {
        StAndroidGlue* anApp = (StAndroidGlue* )theParam;
        anApp->threadEntry();
        return SV_THREAD_RETURN 0;
    }

    ST_CPPEXPORT void threadEntry();

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

        private: //! @name private fields

    ANativeActivity*    myActivity;          //!< ANativeActivity object instance that this application is running in
    AConfiguration*     myConfig;            //!< The current configuration the application is running in
    ALooper*            myLooper;            //!< ALooper associated with the application thread
    AInputQueue*        myInputQueue;        //!< input queue from which the application will receive user input events
    AInputQueue*        myInputQueuePending;
    ANativeWindow*      myWindow;            //!< native window to draw into
    ANativeWindow*      myWindowPending;
    int                 myActivityState;     //!< Current state of the app's activity (APP_CMD_START, APP_CMD_RESUME, APP_CMD_PAUSE, or APP_CMD_STOP)
    StString            myDataType;          //!< intent data type
    StString            myDataPath;          //!< intent data string

    void*               mySavedState;        //!< last instance's saved state, as provided at creation time
    size_t              mySavedStateSize;

    pthread_mutex_t     myMutex;
    pthread_cond_t      myCond;

    int                 myMsgRead;
    int                 myMsgWrite;

    StAndroidPollSource myCmdPollSource;
    StAndroidPollSource myInputPollSource;

    bool                myIsRunning;
    bool                myIsStateSaved;
    bool                myToDestroy;         //!< this is non-zero when the application's NativeActivity is being destroyed and waiting for the application thread to complete
    bool                myIsDestroyed;
};

#endif // __StAndroidGlue_h_
#endif // __ANDROID__
