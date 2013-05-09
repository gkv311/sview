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

        protected:

    /**
     * Process all pending events within this application.
     * @param theMessages buffer to get new messages
     */
    ST_CPPEXPORT virtual void processEvents(const StMessage_t* theEvents);

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

        protected: //! @name window events slots

    /**
     * Process device change.
     */
    ST_CPPEXPORT virtual void doChangeDevice(const int32_t theValue);

    /**
     * Process window resize.
     */
    ST_CPPEXPORT virtual void doResize(const StSizeEvent&  theEvent);

    /**
     * Process mouse button press.
     */
    ST_CPPEXPORT virtual void doMouseDown(const StClickEvent& theEvent);

    /**
     * Process mouse button release.
     */
    ST_CPPEXPORT virtual void doMouseUp(const StClickEvent& theEvent);

        public: //! @name public parameters

    struct {

        StHandle<StEnumParam> ActiveDevice;        //!< enumerated devices

    } params;

        private:

    ST_LOCAL void stApplicationInit(const StHandle<StOpenInfo>& theOpenInfo);

        protected: //! @name protected fields

    StArrayList< StHandle<StWindow> > myRenderers; //!< list of registered renderers
    StHandle<StSettings>  myGlobalSettings;        //!< global settings shared between all applications
    StHandle<StWindow>    myWindow;                //!< active renderer and main application window
    StHandle<StWindow>    mySwitchTo;              //!< new renderer to switch to
    StHandle<StOpenInfo>  myOpenFileInfo;          //!< file to open
    StNativeWin_t         myWinParent;
    StString              myTitle;                 //!< application title
    StOutDevicesList      myDevices;
    StString              myRendId;                //!< renderer ID
    StMessage_t           myMessages[StMessageList::BUFFER_SIZE + 1];
    int                   myExitCode;
    bool                  myIsOpened;              //!< application execution state
    bool                  myToQuit;                //!< request for application termination

        private: //! @name no copies, please

    ST_LOCAL StApplication(const StApplication& );
    ST_LOCAL const StApplication& operator=(const StApplication& );

};

#endif //__StApplication_h_
