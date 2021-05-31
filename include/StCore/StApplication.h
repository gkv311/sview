/**
 * StCore, window system independent C++ toolkit for writing OpenGL applications.
 * Copyright © 2009-2017 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#ifndef __StApplication_h_
#define __StApplication_h_

#include <StCore/StWindow.h>
#include <StCore/StOpenInfo.h>
#include <StSettings/StEnumParam.h>
#include <StSettings/StTranslations.h>
#include <StSlots/StAction.h>

#include <map>
#include <string>

class StEventsBuffer;
class StSettings;

/**
 * This class provides basic interface for interactive application.
 */
class StApplication {

        protected:

    /**
     * Action to be performed on escape.
     */
    enum ActionOnEscape {
        ActionOnEscape_Nothing,              //!< do not exit application
        ActionOnEscape_ExitOneClick,         //!< exit on single click
        ActionOnEscape_ExitDoubleClick,      //!< exit on double click
        ActionOnEscape_ExitOneClickWindowed, //!< exit on single click in windowed mode
    };

        public:

    /**
     * Parse process arguments.
     */
    ST_CPPEXPORT static StHandle<StOpenInfo> parseProcessArguments();

    /**
     * Read default drawer from setting.
     */
    ST_CPPEXPORT static bool readDefaultDrawer(StHandle<StOpenInfo>& theInfo);

    /**
     * @return TRUE if specified drawer is set as default in settings.
     */
    ST_CPPEXPORT bool isDefaultDrawer(const StString& theDrawer) const;

    /**
     * Save the drawer as default in settings.
     */
    ST_CPPEXPORT void setDefaultDrawer(const StString& theDrawer) const;

    /**
     * Create a named parameter for setting drawer as default.
     */
    ST_CPPEXPORT StHandle<StBoolParamNamed> createDefaultDrawerParam(const StString& theDrawer,
                                                                     const StString& theTitle) const;

        public:

    /**
     * Main constructor.
     */
    ST_CPPEXPORT StApplication(const StHandle<StResourceManager>& theResMgr,
                               const StNativeWin_t                theParentWin,
                               const StHandle<StOpenInfo>&        theOpenInfo  = NULL);

    /**
     * Destructor.
     */
    ST_CPPEXPORT virtual ~StApplication();

    /**
     * Application may exit in 2 cases:
     *  - user request (close window / hot key - should be handled by inheritors)
     *  - exit() method is called
     * @return true if application is in exit state
     */
    ST_CPPEXPORT bool closingDown() const;

    /**
     * Ask application to exit.
     * Notice that callee should call processEvents.
     */
    ST_CPPEXPORT void exit(const int theExitCode);

    /**
     * Open specified file (override application arguments if any).
     * @param theOpenInfo file information
     * @return false on any critical error
     */
    ST_CPPEXPORT bool open(const StOpenInfo& theOpenInfo);

    /**
     * Create main application window and open file.
     * If file was not set directly then it will be taken from application arguments.
     * @return false on any critical error
     */
    ST_CPPEXPORT virtual bool open();

    /**
     * @return application description
     */
    ST_CPPEXPORT virtual StString getAboutString() const;

    /**
     * Process all pending events within this application.
     * @param theMessages buffer to get new messages
     */
    ST_CPPEXPORT void processEvents();

    /**
     * Enters the main event loop and wait until exit
     * (calls processEvents in loop).
     */
    ST_CPPEXPORT int exec();

    /**
     * @return file to open in another drawer
     */
    const StHandle<StOpenInfo>& getOpenFileInOtherDrawer() const { return myOpenFileOtherApp; }

    /**
     * Return active renderer and main application window.
     */
    ST_CPPEXPORT const StHandle<StWindow>& getMainWindow() const;

    /**
     * @return true if main application window is in active state
     */
    ST_CPPEXPORT bool isActive() const;

    /**
     * @return default messages queue
     */
    ST_CPPEXPORT const StHandle<StMsgQueue>& getMessagesQueue() const;

    /**
     * Register hot keys for specified actions.
     */
    ST_CPPEXPORT void registerHotKeys();

    /**
     * Return the action association for specified hot-key.
     */
    ST_CPPEXPORT StHandle<StAction> getActionForKey(unsigned int theHKey) const;

    /**
     * @return action for specified ID
     */
    ST_CPPEXPORT const StHandle<StAction>& getAction(const int theActionId);

    /**
     * Get actions map.
     */
    ST_LOCAL const std::map< int, StHandle<StAction> >& getActions() const { return myActions; }

    /**
     * Find action ID for specified Action Name.
     */
    ST_CPPEXPORT int getActionIdFromName(const StString& theActionName) const;

        protected:

    /**
     * Idle before next redraw call.
     */
    ST_CPPEXPORT virtual void beforeDraw();

    /**
     * Rendering callback.
     */
    ST_CPPEXPORT virtual void stglDraw(unsigned int theView);

    /**
     * Reset device - release GL resources in old window and re-create them in new window.
     */
    ST_CPPEXPORT virtual bool resetDevice();

    /**
     * Register renderer.
     */
    ST_CPPEXPORT void addRenderer(const StHandle<StWindow>& theRenderer);

    /**
     * Modify actions.
     */
    ST_LOCAL std::map< int, StHandle<StAction> >& changeActions() { return myActions; }

    /**
     * Register the action.
     */
    ST_CPPEXPORT void addAction(const int                 theActionId,
                                const StHandle<StAction>& theAction);

    /**
     * Register the action.
     */
    ST_CPPEXPORT void addAction(const int           theActionId,
                                StHandle<StAction>& theAction,
                                const unsigned int  theHotKey1,
                                const unsigned int  theHotKey2 = 0);

    /**
     * Invoke action from another thread.
     */
    ST_CPPEXPORT void invokeAction(const int    theActionId,
                                   const double theProgress = 0.0);

    /**
     * Language change slot.
     */
    ST_CPPEXPORT virtual void doChangeLanguage(const int32_t );

        protected: //! @name window events slots

    /**
     * Process device change.
     */
    ST_CPPEXPORT virtual void doChangeDevice(const int32_t theValue);

    /**
     * Process window close event.
     * Default implementation just redirect to StApplication::exit(0);
     */
    ST_CPPEXPORT virtual void doClose(const StCloseEvent& theEvent);

    /**
     * Process window pause event - application can be closed at any moment.
     * Implementation should save the state to avoid data loss.
     */
    ST_CPPEXPORT virtual void doPause(const StPauseEvent& theEvent);

    /**
     * Process window resize.
     */
    ST_CPPEXPORT virtual void doResize(const StSizeEvent& theEvent);

    /**
     * Process queued application event.
     */
    ST_CPPEXPORT virtual void doAction(const StActionEvent& theEvent);

    /**
     * Process keyboard key press.
     * Should be processed by text input fields (Char field contains Unicode code point)
     * or to control on/off state with emphasis on event happens time.
     */
    ST_CPPEXPORT virtual void doKeyDown(const StKeyEvent& theEvent);

    /**
     * Process keyboard key release.
     * In most cases should not be processed at all.
     */
    ST_CPPEXPORT virtual void doKeyUp(const StKeyEvent& theEvent);

    /**
     * Process holded keyboard key.
     * Should be processed to alter continuous properties
     * with emphasis on duration not on time when event happend.
     */
    ST_CPPEXPORT virtual void doKeyHold(const StKeyEvent& theEvent);

    /**
     * Process mouse button press.
     */
    ST_CPPEXPORT virtual void doMouseDown(const StClickEvent& theEvent);

    /**
     * Process mouse button release.
     */
    ST_CPPEXPORT virtual void doMouseUp(const StClickEvent& theEvent);

    /**
     * Process touch.
     */
    ST_CPPEXPORT virtual void doTouch(const StTouchEvent& theEvent);

    /**
     * Process gesture.
     */
    ST_CPPEXPORT virtual void doGesture(const StGestureEvent& theEvent);

    /**
     * Process scrolling.
     */
    ST_CPPEXPORT virtual void doScroll (const StScrollEvent& theEvent);

    /**
     * Process file Drag & Drop event.
     */
    ST_CPPEXPORT virtual void doFileDrop(const StDNDropEvent& theEvent);

    /**
     * Navigation event.
     */
    ST_CPPEXPORT virtual void doNavigate(const StNavigEvent& theEvent);

    /**
     * Exit on escape.
     */
    ST_CPPEXPORT bool doExitOnEscape(StApplication::ActionOnEscape theAction);

        public:

    /**
     * Find translation for the string with specified id.
     */
    ST_LOCAL const StString& tr(const size_t theId) const {
        return myLangMap->getValue(theId);
    }

        public: //! @name public parameters

    struct {

        StHandle<StEnumParam> ActiveDevice; //!< enumerated devices
        StHandle<StEnumParam> VSyncMode;    //!< VSync mode from StGLContext::VSync_Mode enumeration (shared between renderers)

    } params;

        private:

    ST_LOCAL void stApplicationInit(const StHandle<StOpenInfo>& theOpenInfo);
    ST_LOCAL void doDrawProxy(unsigned int theView);

        protected: //! @name protected fields

    StArrayList< StHandle<StWindow> > myRenderers; //!< list of registered renderers
    StHandle<StResourceManager>       myResMgr;    //!< resources manager
    StHandle<StTranslations>          myLangMap;   //!< translated strings map
    StHandle<StMsgQueue>  myMsgQueue;              //!< messages queue
    StHandle<StWindow>    myWindow;                //!< active renderer and main application window
    StHandle<StWindow>    mySwitchTo;              //!< new renderer to switch to
    StHandle<StOpenInfo>  myOpenFileInfo;          //!< file to open
    StHandle<StOpenInfo>  myOpenFileOtherApp;      //!< file to open in another drawer
    std::map< int, StHandle<StAction> >
                          myActions;               //!< ID -> Action map
    std::map< std::string, int >
                          myActionLookup;          //!< lookup map ActionName -> ActionID
    std::map< unsigned int, StHandle<StAction> >
                          myKeyActions;            //!< Hot Key -> Action map
    StHandle<StEventsBuffer>
                          myEventsBuffer;          //!< extra buffer for application-specific queued events

    StNativeWin_t         myWinParent;
    StString              myTitle;                 //!< application title
    StOutDevicesList      myDevices;
    StString              myRendId;                //!< renderer ID
    int                   myExitCode;
    bool                  myGlDebug;               //!< request debug OpenGL context
    bool                  myIsOpened;              //!< application execution state
    bool                  myToQuit;                //!< request for application termination
    bool                  myToRecreateMenu;        //!< flag to recreate the menu
    StTimer               myExitTimer;             //!< double click exit timer

        private: //! @name no copies, please

    ST_LOCAL StApplication(const StApplication& );
    ST_LOCAL const StApplication& operator=(const StApplication& );

};

#endif //__StApplication_h_
