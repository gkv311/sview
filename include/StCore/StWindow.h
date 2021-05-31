/**
 * StCore, window system independent C++ toolkit for writing OpenGL applications.
 * Copyright © 2007-2017 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#ifndef __StWindow_h_
#define __StWindow_h_

#include <stTypes.h>
#include <StStrings/StMsgQueue.h>
#include <StSlots/StSignal.h>
#include <StSettings/StEnumParam.h>
#include <StTemplates/StRect.h>
#include <StThreads/StResourceManager.h>
#include <StGL/StGLEnums.h>
#include <StGL/StGLVec.h>

#include "StWinErrorCodes.h" // Header with error codes
#include "StNativeWin_t.h"
#include "StEvent.h"
#include "StKeysState.h"

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
 * Splitting configuration.
 */
enum StWinSplit {
    StWinSlave_splitOff,         //!< do not split the window
    StWinSlave_splitHorizontal,  //!< split window horizontally
    StWinSlave_splitVertical,    //!< split window vertically
    StWinSlave_splitVertHdmi720, //!< split window vertically (2x 1280x720  + 30 pixels between)
    StWinSlave_splitVertHdmi1080 //!< split window vertically (2x 1920x1080 + 45 pixels between)
};

/**
 * Window attributes.
 */
enum StWinAttr {
    StWinAttr_NULL = 0,            //!< NULL-termination of array of the attributes
    StWinAttr_GlQuadStereo,        //!< boolean, request OpenGL context with Quad Buffer, turned OFF by default
    StWinAttr_GlDebug,             //!< boolean, request Debug OpenGL context, FALSE by default
    StWinAttr_GlDepthSize,         //!< integer, minimum size of Depth Buffer (in bits) or 0 if not needed, 16 bit by default
    StWinAttr_GlStencilSize,       //!< integer, size of Stencil Buffer (in bits) or 0 if not needed, 0 bit by default
    StWinAttr_ToBlockSleepSystem,  //!< boolean, prevent system  going to sleep (display could be turned off), FALSE by default
    StWinAttr_ToBlockSleepDisplay, //!< boolean, prevent display going to sleep, FALSE by default
    StWinAttr_GlobalMediaKeys,     //!< boolean, register system hot-key to capture multimedia even without window focus, FALSE by default
    StWinAttr_SlaveCfg,            //!< StWinSlave, create StWindow with slave window and specify slave window position rules, StWinSlave_slaveOff by default
    StWinAttr_SlaveMon,            //!< integer, slave window monitor id, 1 by default
    StWinAttr_SplitCfg,            //!< StWinSplit, split master window
    StWinAttr_ToAlignEven,         //!< boolean, align window position to even numbers, FALSE by default
    StWinAttr_ExclusiveFullScreen, //!< boolean, exclusive fullscreen mode, FALSE by default
};

typedef struct tagStSlaveWindowCfg {
    int idMaster;
    int idSlave;      // slave window always should be assigned to monitor
    int xAdd;         // coordinates computed with this algorithm:
    int xSub;         // xAdd*(monLeft + left) + xSub*(monRight - right)
    int yAdd;         // yAdd*(monTop + top) + ySub*(monBottom - bottom)
    int ySub;
} StSlaveWindowCfg_t;

class StGLContext;
class StMonitor;
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
     * @param theResMgr       resources manager
     * @param theParentWindow Parent window
     */
    ST_CPPEXPORT StWindow(const StHandle<StResourceManager>& theResMgr,
                          const StNativeWin_t                theParentWindow);

    /**
     * Destructor.
     */
    ST_CPPEXPORT virtual ~StWindow();

    /**
     * Return native parent window.
     */
    ST_CPPEXPORT StNativeWin_t getParentWindow() const;

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
     *  - Slave window existence
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
     * Return true if window is in paused state (e.g. it is hidden and can be closed by system at any moment).
     * Similar to !isActive() but more aggressive - application should consider destroying itself.
     */
    ST_CPPEXPORT bool isPaused() const;

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
    ST_CPPEXPORT virtual void setFullScreen(const bool theFullScreen);

    /**
     * Get GUI GL window placement
     * @return window placement on screen
     */
    ST_CPPEXPORT StRectI_t getPlacement() const;

    /**
     * Return GUI GL window placement in windowed state (ignores active full-screen placement).
     */
    ST_CPPEXPORT StRectI_t getWindowedPlacement() const;

    /**
     * Return false if window position or dimensions can not be changes
     * (systems supports only full-screen mode, or window is embedded).
     */
    ST_CPPEXPORT bool isMovable() const;

    /**
     * Return true if window is displayed on mobile device.
     */
    ST_CPPEXPORT static bool isMobile();

    /**
     * Return false if window can be switched into fullscreen mode.
     * Fullscreen mode can be unavailable (e.g. no windowed mode) on mobile platforms.
     */
    ST_CPPEXPORT bool hasFullscreenMode() const;

    /**
     * Change GUI GL window placement
     * @param theRect new window position
     */
    ST_CPPEXPORT void setPlacement(const StRectI_t& theRect,
                                   const bool       theMoveToScreen = false);

    /**
     * @return GUI margins
     */
    ST_CPPEXPORT virtual StMarginsI getMargins() const;

    /**
     * @return point relative to window mouse position
     * (0,0) - is top left of the window and (1,1) right bottom.
     */
    ST_CPPEXPORT StPointD_t getMousePos() const;

    /**
     * @return TRUE if last mouse cursor position was updated by precise input device (e.g. mouse)
     */
    ST_CPPEXPORT bool isPreciseCursor() const;

    /**
     * Function creates a GL window.
     * @return true on success
     */
    ST_CPPEXPORT virtual bool create();

    /**
     * Close window and all destroy associated resources.
     */
    ST_CPPEXPORT virtual void close();

    /**
     * Extra routines to be processed before window close.
     */
    ST_CPPEXPORT virtual void beforeClose();

    /**
     * CallBack function.
     * @param theMessages buffer to get new messages
     */
    ST_CPPEXPORT virtual void processEvents();

    /**
     * Append message into callback list.
     * Message will be read on next callback() call.
     * @param theEvent message to append
     */
    ST_CPPEXPORT void post(StEvent& theEvent);

    /**
     * @return cached keyboard keys state for this window
     */
    ST_CPPEXPORT const StKeysState& getKeysState() const;

    /**
     * @return true if cursor position has been changed since previous processEvents().
     */
    ST_CPPEXPORT bool isMouseMoved() const;

    /**
     * Resources manager.
     */
    ST_CPPEXPORT const StHandle<StResourceManager>& getResourceManager() const;

    /**
     * Messages queue.
     */
    ST_CPPEXPORT const StHandle<StMsgQueue>& getMessagesQueue() const;

    ST_CPPEXPORT void setMessagesQueue(const StHandle<StMsgQueue>& theQueue);

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
    ST_CPPEXPORT bool stglMakeCurrent();

    /**
     * Swap dual-buffered GL context. Notice that stglDraw will automatically call it.
     */
    ST_CPPEXPORT void stglSwap();

    /**
     * Return window aspect ratio (width / height in normal case).
     */
    ST_CPPEXPORT double stglAspectRatio() const;

    /**
     * Get viewport for specified subwindow (to handle tiled presentation).
     * @param theWinEnum subwindow ID
     * @return rectangle within window from bottom-left corner (ready for OpenGL calls)
     */
    ST_CPPEXPORT virtual StGLBoxPx stglViewport(const int theWinEnum) const;

    /**
     * @return GUI scale factor for compatibility with low-resolution and high-resolution displays
     */
    ST_CPPEXPORT virtual GLfloat getScaleFactor() const;

    /**
     * @return OpenGL context
     */
    ST_CPPEXPORT const StHandle<StGLContext>& getContext() const;

        public: //! @name renderer properties

    /**
     * Handle language change (update strings).
     */
    ST_CPPEXPORT virtual void doChangeLanguage();

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
     * Return true if 3D output requires fullscreen mode.
     */
    ST_CPPEXPORT virtual bool isStereoFullscreenOnly() const;

    /**
     * @return true if stereo output enabled
     */
    ST_CPPEXPORT bool isStereoOutput() const;

    /**
     * @param theStereoState enable/disable stereooutput
     */
    ST_CPPEXPORT void setStereoOutput(bool theStereoState);

    /**
    * Return maximum display update framerate.
    */
    ST_CPPEXPORT virtual float getMaximumTargetFps() const;

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
     * Return optional statistics for verbose output.
     */
    ST_CPPEXPORT const StString& getStatistics() const;

    /**
     * Turn hardware stereo on/off, when appropriate API is available.
     */
    ST_CPPEXPORT virtual void setHardwareStereoOn(const bool theToEnable);

    /**
     * Return true if device has orientation sensor.
     */
    ST_CPPEXPORT virtual bool hasOrientationSensor() const;

    /**
     * Return true if orientation sensor has poor quality.
     */
    ST_CPPEXPORT virtual bool isPoorOrientationSensor() const;

    /**
     * Return true if orientation sensor has been enabled.
     */
    ST_CPPEXPORT virtual bool toTrackOrientation() const;

    /**
     * Turn orientation sensor on/off.
     * Has no effect in case if sensor is unavailable.
     */
    ST_CPPEXPORT virtual void setTrackOrientation(const bool theToTrack);

    /**
     * Setup visibility of system bars.
     */
    ST_CPPEXPORT void setHideSystemBars(bool theToHideStatusBar,
                                        bool theToHideNavBar);

    /**
     * Return device orientation (for head-tracking) within right-handed coordinate system:
     * - Y is positive in up direction (towards the sky, perpendicular to the ground).
     * - X is positive to the right (tangential to the ground).
     * - Z is positive heading backwards (tangential to the ground).
     */
    ST_CPPEXPORT virtual StQuaternion<double> getDeviceOrientation() const;

    /**
     * Return custom stereo projection frustums.
     */
    ST_CPPEXPORT virtual bool getCustomProjection(StRectF_t& theLeft, StRectF_t& theRight) const;

    /**
     * Return TRUE if Left/Right eyes should be swapped by external event.
     */
    ST_CPPEXPORT virtual bool toSwapEyesHW() const;

    /**
     * Retrieve options list.
     */
    ST_CPPEXPORT virtual void getOptions(StParamsList& theList) const;

    /**
     * Access list of connected monitors.
     */
    ST_CPPEXPORT const StSearchMonitors& getMonitors() const;

    /**
     * Check placement of the parent window.
     */
    ST_CPPEXPORT bool isParentOnScreen() const;

    ST_CPPEXPORT virtual GLfloat getLensDist() const;

        public: //! @name clipboard

    /**
     * Put text into clipboard.
     */
    ST_CPPEXPORT void toClipboard(const StString& theText);

    /**
     * Retrieve text from clipboard.
     */
    ST_CPPEXPORT bool fromClipboard(StString& theText);

        public: //! @name signals

    struct {
        /**
         * Emit callback Slot on redraw.
         * @param theView view id to redraw
         */
        StSignal<void (const unsigned int   )> onRedraw;

        StSignal<void (const StCloseEvent&  )> onClose;
        StSignal<void (const StPauseEvent&  )> onPause;
        StSignal<void (const StSizeEvent&   )> onResize;
        StSignal<void (const StSizeEvent&   )> onAnotherMonitor;
        StSignal<void (const StKeyEvent&    )> onKeyUp;
        StSignal<void (const StKeyEvent&    )> onKeyDown;
        StSignal<void (const StKeyEvent&    )> onKeyHold;
        StSignal<void (const StClickEvent&  )> onMouseUp;
        StSignal<void (const StClickEvent&  )> onMouseDown;
        StSignal<void (const StTouchEvent&  )> onTouch;
        StSignal<void (const StGestureEvent&)> onGesture;
        StSignal<void (const StScrollEvent& )> onScroll;
        StSignal<void (const StDNDropEvent& )> onFileDrop;
        StSignal<void (const StNavigEvent&  )> onNavigate;
        StSignal<void (const StActionEvent& )> onAction;

    } signals;

        public: //! @name Properties

    struct {

        StHandle<StEnumParam> VSyncMode; //!< VSync mode from StGLContext::VSync_Mode enumeration (shared between renderers)

    } params;

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
    ST_CPPEXPORT bool stglMakeCurrent(const int theWinEnum);

    /**
     * Swap dual-buffered GL context for specified window.
     * @param theWinEnum subwindow to swap
     */
    ST_CPPEXPORT void stglSwap(const int theWinEnum);

    /**
     * @return upload time in seconds
     */
    ST_CPPEXPORT double getEventTime(const uint32_t theTime) const;

    /**
     * Convert rectangle from desktop units into backing store units (pixels).
     */
    ST_CPPEXPORT void convertRectToBacking(StGLBoxPx& theRect,
                                           const int  theWinId);

    /**
     * @return stereo output flag specified by application (means - have stereo data)
     */
    ST_CPPEXPORT bool isStereoSource() const;

    /**
     * Setup default window placement.
     */
    ST_CPPEXPORT StRectI_t defaultRect(const StMonitor* theMon = NULL) const;

    /**
     * Setup forced window aspect ratio.
     * When negative value is given (default is -1), aspect ratio will be automatically computed as window (width/height).
     */
    ST_CPPEXPORT void setForcedAspect(double theAspect);

        public:

    /**
     * This method should be called only by inheritors
     * to override keyboard input logic.
     * @return cached keyboard keys state for this window
     */
    ST_CPPEXPORT StKeysState& changeKeysState();

    ST_CPPEXPORT void* getNativeOglWin() const;
    ST_CPPEXPORT void* getNativeOglDC()  const;
    ST_CPPEXPORT void* getNativeOglRC()  const;

        private: //! @name private fields

    StWindowImpl*               myWin;            //!< window implementation class - we hide implementation details since them too platform-specific
    double                      myTargetFps;      //!< user data

        protected:

    StHandle<StMsgQueue>        myMsgQueue;       //!< messages queue
    bool                        myWasUsed;        //!< flag indicates that window has been actually used (created)
    bool                        myIsForcedStereo; //!< flag to force stereo output for mono sources

        private: //! @name no copies, please

    ST_LOCAL void copySignals();

    ST_LOCAL StWindow(const StWindow& );
    ST_LOCAL const StWindow& operator=(const StWindow& );

};
template<> inline void StArray< StHandle<StWindow> >::sort() {}

#endif // __StWindow_h_
