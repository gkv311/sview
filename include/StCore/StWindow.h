/**
 * Copyright © 2007-2013 Kirill Gavrilov <kirill@sview.ru>
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

#ifndef __StWindow_h_
#define __StWindow_h_

#include <stTypes.h>
#include <StStrings/StString.h>
#include <StSlots/StSignal.h>
#include <StSettings/StParam.h>
#include <StTemplates/StRect.h>
#include <StGL/StGLEnums.h>
#include <StGL/StGLVec.h>

#include "StWinErrorCodes.h" // Header with error codes
#include "StMessageList.h"
#include "StNativeWin_t.h"
#include "StEvent.h"

// StWindow enumeration
enum {
    ST_WIN_ALL    = -1, //!< special value for swap operation
    ST_WIN_MASTER =  0,
    ST_WIN_SLAVE  =  1,
};

enum {
    ST_DEVICE_SUPPORT_IGNORE =-1,
    ST_DEVICE_SUPPORT_NONE   = 0,
    ST_DEVICE_SUPPORT_LOW    = 1,
    ST_DEVICE_SUPPORT_MIDDLE = 2,
    ST_DEVICE_SUPPORT_HIGHT  = 3,
    ST_DEVICE_SUPPORT_FULL   = 4,
    ST_DEVICE_SUPPORT_PREFER = 5,
};

/**
 * Output device identification.
 */
struct StOutDevice {

    StString PluginId; //!< renderer identifier StWindow::getRendererId()
    StString DeviceId; //!< device   identifier (should be unique within renderer)
    StString Name;     //!< device   name
    StString Desc;     //!< device   description
    int      Priority; //!< device   priority (ST_DEVICE_SUPPORT_ enumeration)

};
typedef StArrayList< StHandle<StOutDevice> > StOutDevicesList;
template<> inline void StArray< StHandle<StOutDevice> >::sort() {}

/**
 * Slave configuration.
 */
enum StWinSlave {
    StWinSlave_slaveOff,         //!< do not create slave window
    StWinSlave_slaveSync,        //!< synchronize position and dimensions with master window
    StWinSlave_slaveFlipX,       //!< flip slave window position along X axis (horizontally), keep dimensions equal to master window
    StWinSlave_slaveFlipY,       //!< flip slave window position along Y axis (vertically),   keep dimensions equal to master window
    StWinSlave_slaveHLineTop,    //!< slave is a horizontal line at top of the display where master window shown
    StWinSlave_slaveHTop2Px,     //!< slave is a horizontal line 2 pixels long at top of the display where master window shown
    StWinSlave_slaveHLineBottom, //!< slave is a horizontal line at bottom of the display where master window shown
};

/**
 * Window attributes.
 */
enum StWinAttr {
    StWinAttr_NULL = 0,            //!< NULL-termination of array of the attributes
    StWinAttr_GlQuadStereo,        //!< boolean, request OpenGL context with Quad Buffer, turned OFF by default
    StWinAttr_GlDepthSize,         //!< integer, minimum size of Depth Buffer (in bits) or 0 if not needed, 16 bit by default
    StWinAttr_ToBlockSleepSystem,  //!< boolean, prevent system  going to sleep (display could be turned off), FALSE by default
    StWinAttr_ToBlockSleepDisplay, //!< boolean, prevent display going to sleep, FALSE by default
    StWinAttr_GlobalMediaKeys,     //!< boolean, register system hot-key to capture multimedia even without window focus, FALSE by default
    StWinAttr_SlaveCfg,            //!< StWinSlave, create StWindow with slave window and specify slave window position rules, StWinSlave_slaveOff by default
    StWinAttr_SlaveMon,            //!< integer, slave window monitor id, 1 by default
};

typedef struct tagStSlaveWindowCfg {
    int idMaster;
    int idSlave;      // slave window always should be assigned to monitor
    int xAdd;         // coordinates computed with this algorithm:
    int xSub;         // xAdd*(monLeft + left) + xSub*(monRight - right)
    int yAdd;         // yAdd*(monTop + top) + ySub*(monBottom - bottom)
    int ySub;
} StSlaveWindowCfg_t;

class StSearchMonitors;
class StWindowImpl;

/**
 * This is an OpenGL Window implementation.
 * Notice that StWindow doesn't support nesting since all nested rendering should be implemented using OpenGL.
 * However it is possible to embed StWindow into another window.
 */
class StWindow {

        public:

    /**
     * Default constructor. Window will not be displayed until create() call.
     */
    ST_CPPEXPORT StWindow();

    /**
     * Default constructor. Window will not be displayed until create() call.
     * @param theParentWindow Parent window
     */
    ST_CPPEXPORT StWindow(const StNativeWin_t theParentWindow);

    /**
     * Destructor.
     */
    ST_CPPEXPORT virtual ~StWindow();

    /**
     * @param theTitle window title
     */
    ST_CPPEXPORT void setTitle(const StString& theTitle);

    /**
     * Request window attributes.
     * @param theAttributes NULL-terminated list of key-value pairs, values will be set to current state
     */
    ST_CPPEXPORT void getAttributes(StWinAttr* theAttributes) const;

    /**
     * @return true if OpenGL context was created with Depth Buffer (see StWinAttr_GlDepthSize)
     */
    ST_CPPEXPORT bool hasDepthBuffer() const;

    /**
     * Setup window attributes.
     * Notice that some attributes should be set BEFORE window creation:
     *  - Slave window existance
     *  - OpenGL context attributes (additional buffers)
     * @param theAttributes NULL-terminated list of key-value pairs (values should be casted to StWinAttr)
     */
    ST_CPPEXPORT void setAttributes(const StWinAttr* theAttributes);

    /**
     * Setup window attribute.
     * @param theAttribute Attribute to change
     * @param theValue     Attribute value
     */
    ST_LOCAL void setAttribute(const StWinAttr theAttribute,
                               const int       theValue) {
        const StWinAttr anAttribs[] = {
            theAttribute, (StWinAttr )theValue,
            StWinAttr_NULL
        };
        setAttributes(anAttribs);
    }

    /**
     * @return true if window content is visible
     */
    ST_CPPEXPORT bool isActive() const;

    /**
     * Show up the window.
     */
    ST_CPPEXPORT virtual void show();

    /**
     * Hide the window.
     */
    ST_CPPEXPORT virtual void hide();

    /**
     * Show/Hide mouse cursor.
     * @param theToShow true to show cursor
     */
    ST_CPPEXPORT virtual void showCursor(const bool theToShow);

    /**
     * @return true if in fullscreen state
     */
    ST_CPPEXPORT bool isFullScreen() const;

    /**
     * @param theFullScreen fullscreen state
     */
    ST_CPPEXPORT void setFullScreen(const bool theFullScreen);

    /**
     * Get GUI GL window placement
     * @return window placement on screen
     */
    ST_CPPEXPORT StRectI_t getPlacement() const;

    /**
     * Change GUI GL window placement
     * @param theRect new window position
     */
    ST_CPPEXPORT void setPlacement(const StRectI_t& theRect,
                                   const bool       theMoveToScreen = false);

    /**
     * @return point relative to window mouse position
     * (0,0) - is top left of the window and (1,1) right buttom.
     */
    ST_CPPEXPORT StPointD_t getMousePos() const;

    /**
     * Function to get Drag&Drop file list.
     * @param theId   file if in list
     * @param theFile buffer for file path;
     * @return number of files in list if theId < 0 and -1 on error
     */
    ST_CPPEXPORT int getDragNDropFile(const int theId,
                                      StString& theFile);

    /**
     * Function creates a GL window.
     * @return true on success
     */
    ST_CPPEXPORT virtual bool create();

    ST_CPPEXPORT virtual void close();

    /**
     * CallBack function.
     * @param theMessages buffer to get new messages
     */
    ST_CPPEXPORT virtual void processEvents(StMessage_t* theMessages);

    /**
     * Append message into callback list.
     * Message will be read on next callback() call.
     * @param theMessage message to append
     */
    ST_CPPEXPORT bool appendMessage(const StMessage_t& theMessage);

        public: //! @name OpenGL routines

    /**
     * Perform rendering of single frame.
     * Notice that application should assing onRedraw callback to render something to the window.
     * Stereoscopic renderers should override this method to perform stereo rendering.
     */
    ST_CPPEXPORT virtual void stglDraw();

    /**
     * Make GL context active in current thread.
     */
    ST_CPPEXPORT void stglMakeCurrent();

    /**
     * Swap dual-buffered GL context. Notice that stglDraw will automatically call it.
     */
    ST_CPPEXPORT void stglSwap();

    /**
     * Get viewport for specified subwindow (to handle tiled presentation).
     * @param theWinEnum subwindow ID
     * @return rectangle within window from bottom-left corner (ready for OpenGL calls)
     */
    ST_CPPEXPORT StGLBoxPx stglViewport(const int theWinEnum) const;

        public: //! @name renderer properties

    /**
     * Renderer about string.
     */
    ST_CPPEXPORT virtual StString getRendererAbout() const;

    /**
     * This method returns renderer identifier.
     * @return rendrer id
     */
    ST_CPPEXPORT virtual const char* getRendererId() const;

    /**
     * Active Device id.
     */
    ST_CPPEXPORT virtual const char* getDeviceId() const;

    /**
     * This methods returns device lost state.
     * To reset device you should call close() -> open() sequence and re-initialize all GPU resources.
     * @return true if rendering device requires reinitialization
     */
    ST_CPPEXPORT virtual bool isLostDevice() const;

    /**
     * Activate Device.
     * @return true if renderer should be re-created to take effect.
     */
    ST_CPPEXPORT virtual bool setDevice(const StString& theDevice);

    /**
     * Devices list.
     */
    ST_CPPEXPORT virtual void getDevices(StOutDevicesList& theList) const;

    /**
     * @return true if stereo output enabled
     */
    ST_CPPEXPORT bool isStereoOutput() const;

    /**
     * @param theStereoState enable/disable stereooutput
     */
    ST_CPPEXPORT void setStereoOutput(bool theStereoState);

    /**
     * @return FPS control rule
     */
    ST_CPPEXPORT double getTargetFps() const;

    /**
     * Setup automatic FPS control rule:
     *  - if theFPS >  0 then renderer will try to reach specified limit
     *  - if theFPS =  0 then renderer will try to reduce CPU load with maximum FPS
     *  - if theFPS = -1 then FPS control is turned off
     */
    ST_CPPEXPORT void setTargetFps(const double theFPS);

    /**
     * Retrieve options list.
     */
    ST_CPPEXPORT virtual void getOptions(StParamsList& theList) const;

        public: //! @name signals

    struct {
        /**
         * Emit callback Slot on redraw.
         * @param theView view id to redraw
         */
        StSignal<void (const unsigned int )>  onRedraw;

        StSignal<void (const StClickEvent& )> onMouseUp;
        StSignal<void (const StClickEvent& )> onMouseDown;


    } signals;

        protected: //! @name advanced methods

    /**
     * Show up window.
     * @param theWinEnum subwindow
     */
    ST_CPPEXPORT void show(const int theWinEnum);

    /**
     * Hide window.
     * @param theWinEnum subwindow
     */
    ST_CPPEXPORT void hide(const int theWinEnum);

    /**
     * Make GL context for specified window active in current thread.
     * @param theWinEnum subwindow to activate GL context
     */
    ST_CPPEXPORT void stglMakeCurrent(const int theWinEnum);

    /**
     * Swap dual-buffered GL context for specified window.
     * @param theWinEnum subwindow to swap
     */
    ST_CPPEXPORT void stglSwap(const int theWinEnum);

    /**
     * Access list of connected monitors.
     */
    ST_CPPEXPORT const StSearchMonitors& getMonitors() const;

        private: //! @name private fields

    StWindowImpl* myWin;       //!< window implementation class - we hide implementation details since them too platform-specific
    double        myTargetFps; //!< user data

        private: //! @name no copies, please

    ST_LOCAL void copySignals();

    ST_LOCAL StWindow(const StWindow& );
    ST_LOCAL const StWindow& operator=(const StWindow& );

};
template<> inline void StArray< StHandle<StWindow> >::sort() {}

#endif //__StWindow_h_
