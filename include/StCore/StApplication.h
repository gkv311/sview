/**
 * Copyright © 2009-2013 Kirill Gavrilov <kirill@sview.ru>
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

#ifndef __StApplication_h_
#define __StApplication_h_

#include <StCore/StWindow.h>
#include <StCore/StOpenInfo.h>
#include <StSettings/StEnumParam.h>
#include <StSlots/StAction.h>

#include <map>

class StEventsBuffer;
class StSettings;

/**
 * This class provides basic interface for interactive application.
 */
class StApplication {

         public:

    /**
     * Parse process arguments.
     */
    ST_CPPEXPORT static StHandle<StOpenInfo> parseProcessArguments();

         public:

    /**
     * Main constructor.
     */
    ST_CPPEXPORT StApplication(const StNativeWin_t         theParentWin,
                               const StHandle<StOpenInfo>& theOpenInfo  = NULL);

    /**
     * Empty constructor.
     */
    ST_CPPEXPORT StApplication();

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
     * Register hot keys for specified actions.
     */
    ST_CPPEXPORT void registerHotKeys();

    /**
     * @return action for specified ID
     */
    ST_CPPEXPORT const StHandle<StAction>& getAction(const int theActionId);

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
     * Process file Drag & Drop event.
     */
    ST_CPPEXPORT virtual void doFileDrop(const StDNDropEvent& theEvent);

    /**
     * Navigation event.
     */
    ST_CPPEXPORT virtual void doNavigate(const StNavigEvent& theEvent);

        public: //! @name public parameters

    struct {

        StHandle<StEnumParam> ActiveDevice; //!< enumerated devices
        StHandle<StEnumParam> VSyncMode;    //!< VSync mode from StGLContext::VSync_Mode enumeration (shared between renderers)

    } params;

        private:

    ST_LOCAL void stApplicationInit(const StHandle<StOpenInfo>& theOpenInfo);

        protected: //! @name protected fields

    StArrayList< StHandle<StWindow> > myRenderers; //!< list of registered renderers
    StHandle<StMsgQueue>  myMsgQueue;              //!< messages queue
    StHandle<StWindow>    myWindow;                //!< active renderer and main application window
    StHandle<StWindow>    mySwitchTo;              //!< new renderer to switch to
    StHandle<StOpenInfo>  myOpenFileInfo;          //!< file to open
    std::map< int, StHandle<StAction> >
                          myActions;               //!< ID -> Action map
    std::map< unsigned int, StHandle<StAction> >
                          myKeyActions;            //!< Hot Key -> Action map
    StHandle<StEventsBuffer>
                          myEventsBuffer;          //!< extra buffer for application-specific queued events

    StNativeWin_t         myWinParent;
    StString              myTitle;                 //!< application title
    StOutDevicesList      myDevices;
    StString              myRendId;                //!< renderer ID
    int                   myExitCode;
    bool                  myIsOpened;              //!< application execution state
    bool                  myToQuit;                //!< request for application termination

        private: //! @name no copies, please

    ST_LOCAL StApplication(const StApplication& );
    ST_LOCAL const StApplication& operator=(const StApplication& );

};

#endif //__StApplication_h_
