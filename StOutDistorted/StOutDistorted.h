/**
 * StOutDistorted, class providing stereoscopic output in anamorph side by side format using StCore toolkit.
 * Copyright © 2013-2020 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#ifndef __StOutDistorted_h_
#define __StOutDistorted_h_

#include <StCore/StWindow.h>
#include <StGL/StGLVertexBuffer.h>
#include <StThreads/StFPSControl.h>

//#define ST_HAVE_OPENVR

class StSettings;
class StProgramBarrel;
class StProgramFlat;
class StGLFrameBuffer;
class StGLTexture;
class StGLTextureQuad;

namespace vr {
    class IVRSystem;
    struct TrackedDevicePose_t;
}

/**
 * This class implements stereoscopic rendering on displays
 * which require software distortion correction.
 */
class StOutDistorted : public StWindow {

        public:

    /**
     * Main constructor.
     */
    ST_CPPEXPORT StOutDistorted(const StHandle<StResourceManager>& theResMgr,
                                const StNativeWin_t                theParentWindow);

    /**
     * Destructor.
     */
    ST_CPPEXPORT virtual ~StOutDistorted();

    /**
     * Use FBO even when rendering can be done by simple viewport adjustment.
     */
    ST_LOCAL void setForcedFboUsage(const bool theIsForcesFboUsage) {
        myIsForcedFboUsage = theIsForcesFboUsage;
    }

    /**
     * Renderer about string.
     */
    ST_CPPEXPORT virtual StString getRendererAbout() const ST_ATTR_OVERRIDE;

    /**
     * Renderer id.
     */
    ST_CPPEXPORT virtual const char* getRendererId() const ST_ATTR_OVERRIDE;

    /**
     * Active Device id.
     */
    ST_CPPEXPORT virtual const char* getDeviceId() const ST_ATTR_OVERRIDE;

    /**
     * This methods returns device lost state.
     * @return true if rendering device requires reinitialization
     */
    ST_CPPEXPORT virtual bool isLostDevice() const ST_ATTR_OVERRIDE;

    /**
     * Activate Device.
     */
    ST_CPPEXPORT virtual bool setDevice(const StString& theDevice) ST_ATTR_OVERRIDE;

    /**
     * Devices list.
     */
    ST_CPPEXPORT virtual void getDevices(StOutDevicesList& theList) const ST_ATTR_OVERRIDE;

    /**
     * Retrieve options list.
     */
    ST_CPPEXPORT virtual void getOptions(StParamsList& theList) const ST_ATTR_OVERRIDE;

    /**
     * Create and show window.
     * @return false if any critical error appeared
     */
    ST_CPPEXPORT virtual bool create() ST_ATTR_OVERRIDE;

    /**
     * Close the window.
     */
    ST_CPPEXPORT virtual void close() ST_ATTR_OVERRIDE;

    /**
     * Extra routines to be processed before window close.
     */
    ST_CPPEXPORT virtual void beforeClose() ST_ATTR_OVERRIDE;

    /**
     * Process callback.
     */
    ST_CPPEXPORT virtual void processEvents() ST_ATTR_OVERRIDE;

    /**
     * @param theFullScreen fullscreen state
     */
    ST_CPPEXPORT virtual void setFullScreen(const bool theFullScreen) ST_ATTR_OVERRIDE;

    /**
     * Return true if 3D output requires fullscreen mode.
     */
    ST_CPPEXPORT virtual bool isStereoFullscreenOnly() const ST_ATTR_OVERRIDE;

    /**
     * Stereo renderer.
     */
    ST_CPPEXPORT virtual void stglDraw() ST_ATTR_OVERRIDE;

    /**
    * Get viewport for specified subwindow.
    */
    ST_CPPEXPORT virtual StGLBoxPx stglViewport(const int theWinEnum) const ST_ATTR_OVERRIDE;

    /**
     * Show/Hide mouse cursor.
     * @param theToShow true to show cursor
     */
    ST_CPPEXPORT virtual void showCursor(const bool theToShow) ST_ATTR_OVERRIDE;

    /**
     * Return true if device has orientation sensor.
     */
    ST_CPPEXPORT virtual bool hasOrientationSensor() const ST_ATTR_OVERRIDE;

    /**
     * Return TRUE if orientation tracking has been activated.
     */
    ST_CPPEXPORT virtual bool toTrackOrientation() const ST_ATTR_OVERRIDE;

    /**
     * Setup flag indicating that orientation tracking should be enabled
     */
    ST_CPPEXPORT virtual void setTrackOrientation(const bool theToTrack) ST_ATTR_OVERRIDE;

    /**
     * Get head orientation.
     */
    ST_CPPEXPORT virtual StQuaternion<double> getDeviceOrientation() const ST_ATTR_OVERRIDE;

    /**
     * Return custom stereo projection frustums.
     */
    ST_CPPEXPORT virtual bool getCustomProjection(StRectF_t& theLeft, StRectF_t& theRight) const ST_ATTR_OVERRIDE;

    /**
     * Return margins for working area.
     */
    ST_CPPEXPORT virtual StMarginsI getMargins() const ST_ATTR_OVERRIDE;

    ST_CPPEXPORT virtual GLfloat getLensDist() const ST_ATTR_OVERRIDE;

    /**
     * Downscale GUI for low-resolution Oculus Rift prototype
     */
    ST_CPPEXPORT virtual GLfloat getScaleFactor() const ST_ATTR_OVERRIDE;

    /**
     * Update strings.
     */
    ST_LOCAL virtual void doChangeLanguage() ST_ATTR_OVERRIDE { updateStrings(); }

    /**
     * Return HMD framerate.
     */
    ST_LOCAL virtual float getMaximumTargetFps() const ST_ATTR_OVERRIDE;

        private:

    /**
     * Detect frame pack HMDI mode.
     */
    ST_LOCAL void checkHdmiPack();

    /**
     * Release GL resources before window closing.
     */
    ST_LOCAL void releaseResources();

    /**
     * Update strings.
     */
    ST_LOCAL void updateStrings();

    /**
     * Update about text.
     */
    ST_LOCAL void updateAbout();

    /**
     * On/off VSync callback.
     */
    ST_LOCAL void doSwitchVSync(const int32_t theValue);

    ST_LOCAL void stglDrawCursor(const StPointD_t&  theCursorPos,
                                 const unsigned int theView);

    /**
     * Draw stereo-pair for VR.
     * To be called from stglDraw().
     */
    ST_LOCAL void stglDrawVR();

    ST_LOCAL bool isHmdOutput() const {
        return myIsStereoOn
            && myDevice == DEVICE_HMD;
    }

    /**
     * Retrieve active head position.
     */
    ST_LOCAL void updateVRProjectionFrustums();

        private:

    static StAtomic<int32_t> myInstancesNb; //!< shared counter for all instances

        private:

    enum {
        DEVICE_AUTO         =-1,
        DEVICE_DISTORTED    = 0, //!< general output
        DEVICE_HMD          = 1, //!< VR HMD
        DEVICE_S3DV         = 2, //!< S3DV
        DEVICE_NB,
    };

    enum Layout {
        LAYOUT_SIDE_BY_SIDE_ANAMORPH = 0, //!< anamorph  side by side
        LAYOUT_OVER_UNDER_ANAMORPH   = 1, //!< anamorph  over under
        LAYOUT_SIDE_BY_SIDE          = 2, //!< full-size side by side
        LAYOUT_OVER_UNDER            = 3, //!< full-size over under
    };

    /**
     * Return current layout.
     */
    Layout getPairLayout() const {
        switch(myDevice) {
            case DEVICE_HMD:
                return LAYOUT_SIDE_BY_SIDE;
            case DEVICE_S3DV:
                return LAYOUT_SIDE_BY_SIDE_ANAMORPH;
            default:
            case DEVICE_DISTORTED:
                return (Layout )params.Layout->getValue();
        }
    }

    struct {

        StHandle<StEnumParam>      Layout;   //!< pair layout
        StHandle<StBoolParamNamed> MonoClone;//!< display mono in stereo

    } params;

        private:

    StOutDevicesList          myDevices;
    StHandle<StSettings>      mySettings;
    StString                  myAbout;           //!< about string
    StString                  myAboutTitle;      //!< title for About dialog
    StString                  myAboutVerString;  //!< version string for About dialog
    StString                  myAboutDescr;      //!< description for About dialog
    StString                  myAboutVrDevice;   //!< HMD info for About dialog

    int                       myDevice;          //!< currently active device
    bool                      myToResetDevice;
    StHandle<StGLContext>     myContext;
    StHandle<StGLFrameBuffer> myFrBuffer;        //!< OpenGL frame buffer object
    StHandle<StGLTexture>     myCursor;          //!< cursor texture - we can not use normal cursor due to distortions
    StHandle<StProgramFlat>   myProgramFlat;
    StHandle<StProgramBarrel> myProgramBarrel;
    StFPSControl              myFPSControl;
    StGLVertexBuffer          myFrVertsBuf;      //!< buffers to draw simple fullsreen quad
    StGLVertexBuffer          myFrTCrdsBuf;
    StGLVertexBuffer          myCurVertsBuf;
    StGLVertexBuffer          myCurTCrdsBuf;
    StGLVec4                  myBarrelCoef;      //!< Barrel distortion coefficients
    StGLVec4                  myChromAb;         //!< chrome coefficients

    StQuaternion<double>      myVrOrient;        //!< HMD (head) orientation, excluding translation vector
    double                    myVrMarginsTop;
    double                    myVrMarginsBottom;
    double                    myVrMarginsLeft;
    double                    myVrMarginsRight;
    StRectF_t                 myVrFrustumL;      //!< projection frustum for the left eye
    StRectF_t                 myVrFrustumR;      //!< projection frustum for the right eye
    StVec2<int>               myVrRendSize;      //!< FBO width x height for rendering into VR (can be greater then actual HMD resolution to compensate distortion)
    float                     myVrFrequency;     //!< display frequency
    float                     myVrAspectRatio;   //!< aspect ratio
    float                     myVrFieldOfView;   //!< field of view
    float                     myVrIOD;           //!< intra-ocular distance in meters
    bool                      myVrTrackOrient;   //!< track orientation flag
    bool                      myVrToDrawMsg;
    StHandle<StGLTextureQuad> myVrFullscreenMsg;
    StTimer                   myVrMsgTimer;
#ifdef ST_HAVE_OPENVR
    vr::IVRSystem*            myVrHmd;           //!< OpenVR session object
    vr::TrackedDevicePose_t*  myVrTrackedPoses;  //!< array of tracked devices poses
#endif

    bool                      myToShowCursor;    //!< cursor visibility flag
    bool                      myToCompressMem;   //!< reduce memory usage
    bool                      myIsBroken;        //!< special flag for broke state - when FBO can not be allocated
    bool                      myIsStereoOn;
    bool                      myCanHdmiPack;
    bool                      myIsHdmiPack;      //!< "frame packed" mode in HDMI 1.4a
    bool                      myIsForcedFboUsage;//!< use FBO even when rendering can be done by simple viewport adjustment

};

#endif // __StOutDistorted_h_
