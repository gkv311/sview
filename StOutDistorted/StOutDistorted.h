/**
 * StOutDistorted, class providing stereoscopic output in anamorph side by side format using StCore toolkit.
 * Copyright © 2013-2016 Kirill Gavrilov <kirill@sview.ru>
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

class StSettings;
class StProgramBarrel;
class StProgramFlat;
class StGLFrameBuffer;
class StGLTexture;

typedef struct ovrHmdStruct* ovrSession;
typedef union ovrGLTexture_s ovrGLTexture;
typedef struct ovrSwapTextureSet_ ovrSwapTextureSet;

/**
 * This class implements stereoscopic rendering on displays
 * wich require software distortion correction.
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
     * Stereo renderer.
     */
    ST_CPPEXPORT virtual void stglDraw() ST_ATTR_OVERRIDE;

    /**
     * Show/Hide mouse cursor.
     * @param theToShow true to show cursor
     */
    ST_CPPEXPORT virtual void showCursor(const bool theToShow);

    /**
     * Return true if device has orientation sensor.
     */
    ST_CPPEXPORT virtual bool hasOrientationSensor() const ST_ATTR_OVERRIDE;

    /**
     * Get head orientation.
     */
    ST_CPPEXPORT virtual StQuaternion<double> getDeviceOrientation() const ST_ATTR_OVERRIDE;

    ST_CPPEXPORT virtual GLfloat getLensDist() const ST_ATTR_OVERRIDE;

    /**
     * Downscale GUI for low-resolution Oculus Rift prototype
     */
    ST_CPPEXPORT virtual GLfloat getScaleFactor() const ST_ATTR_OVERRIDE;

    /**
     * Update strings.
     */
    ST_LOCAL virtual void doChangeLanguage() ST_ATTR_OVERRIDE { updateStrings(); }

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
     * On/off VSync callback.
     */
    ST_LOCAL void doSwitchVSync(const int32_t theValue);

    ST_LOCAL void stglDrawCursor(const StPointD_t&  theCursorPos,
                                 const unsigned int theView);

    /**
     * Draw stereo-pair using libOVR.
     * To be called from stglDraw().
     */
    ST_LOCAL void stglDrawLibOVR();

        private:

    static StAtomic<int32_t> myInstancesNb; //!< shared counter for all instances

        private:

    enum {
        DEVICE_AUTO         =-1,
        DEVICE_DISTORTED    = 0, //!< general output
        DEVICE_OCULUS       = 1, //!< Oculus Rift
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
            case DEVICE_OCULUS:
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

    StMarginsI                myBarMargins;      //!< GUI margins

    ovrSession                myOvrHmd;
    int                       myOvrSizeX;
    int                       myOvrSizeY;
    StQuaternion<double>      myOvrOrient;
#ifdef ST_HAVE_LIBOVR
    ovrSwapTextureSet*        myOvrSwapTexture;
    GLuint                    myOvrSwapFbo[2];
    ovrGLTexture*             myOvrMirrorTexture;
    GLuint                    myOvrMirrorFbo;
#endif

    bool                      myToReduceGui;     //!< scale down GUI
    bool                      myToShowCursor;    //!< cursor visibility flag
    bool                      myToCompressMem;   //!< reduce memory usage
    bool                      myIsBroken;        //!< special flag for broke state - when FBO can not be allocated
    bool                      myIsStereoOn;
    bool                      myCanHdmiPack;
    bool                      myIsHdmiPack;      //!< "frame packed" mode in HDMI 1.4a
    bool                      myIsForcedFboUsage;//!< use FBO even when rendering can be done by simple viewport adjustment

};

#endif // __StOutDistorted_h_
