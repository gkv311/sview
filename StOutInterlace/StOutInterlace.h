/**
 * StOutInterlace, class providing stereoscopic output in row interlaced format using StCore toolkit.
 * Copyright © 2009-2017 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#ifndef __StOutInterlace_h_
#define __StOutInterlace_h_

#include <StCore/StWindow.h>
#include <StCore/StMonitor.h>
#include <StThreads/StFPSControl.h>
#include <StGL/StGLProgram.h>
#include <StGL/StGLFrameBuffer.h>
#include <StGL/StGLVertexBuffer.h>

class StSettings;

/**
 * Simple GLSL program.
 */
class StProgramFB : public StGLProgram {

        public:

    ST_LOCAL StProgramFB(const StString& theTitle);
    ST_LOCAL virtual bool link(StGLContext& theCtx) ST_ATTR_OVERRIDE;

};

/**
 * This class implements stereoscopic rendering on Interlaced monitors.
 */
class StOutInterlace : public StWindow {

        public:

    /**
     * Main constructor.
     */
    ST_CPPEXPORT StOutInterlace(const StHandle<StResourceManager>& theResMgr,
                                const StNativeWin_t                theParentWindow);

    /**
     * Destructor.
     */
    ST_CPPEXPORT virtual ~StOutInterlace();

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
     * Show up the window.
     */
    ST_CPPEXPORT virtual void show() ST_ATTR_OVERRIDE;

    /**
     * Process callback.
     */
    ST_CPPEXPORT virtual void processEvents() ST_ATTR_OVERRIDE;

    /**
     * Stereo renderer.
     */
    ST_CPPEXPORT virtual void stglDraw() ST_ATTR_OVERRIDE;

    /**
     * Update strings.
     */
    ST_LOCAL virtual void doChangeLanguage() ST_ATTR_OVERRIDE { updateStrings(); }

        private:

    enum {
        DEVICE_AUTO                =-1,
        DEVICE_ROW_INTERLACED      = 0, //!< interlace (horizontal 1xPixel lines, full color from R or L)
        DEVICE_COL_INTERLACED      = 1, //!< interlace (vertical 1xPixel lines, full color from R or L)
        DEVICE_CHESSBOARD          = 2, //!< 1xPixel chessboard (some DLP devices)
        DEVICE_ROW_INTERLACED_ED   = 3, //!< interlace (horizontal 1xPixel lines) + EDimensional onscreen codes
        DEVICE_COL_INTERLACED_MI3D = 4, //!< interlace with barrier

        DEVICE_NB,
    };

        private:

    /**
     * Look for interlaced monitors within the given list.
     */
    ST_LOCAL static StHandle<StMonitor> getInterlacedMonitor(const StArrayList<StMonitor>& theMonitors,
                                                             bool& theIsReversed,
                                                             bool& theIsRowInterlaced);

    ST_LOCAL void stglDrawEDCodes();

    /**
     * Initialize texture mask.
     */
    ST_LOCAL bool initTextureMask(int  theDevice,
                                  bool theToReverse,
                                  int  theSizeX,
                                  int  theSizeY);

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

    /**
     * Bind to monitor callback.
     */
    ST_LOCAL void doSetBindToMonitor(const bool theValue);

    /**
     * Process monitor change event.
     */
    ST_LOCAL void doNewMonitor(const StSizeEvent& theEvent);

        private:

    static StAtomic<int32_t>  myInstancesNb;              //!< shared counter for all instances

        private:

    struct {

        StHandle<StBoolParamNamed> ToReverse; //!< configurable flag to reverse rows order
        StHandle<StBoolParamNamed> BindToMon; //!< flag to bind to monitor
        StHandle<StBoolParamNamed> ToUseMask; //!< use mask texture instead of straightforward discard shader

    } params;

    /**
     * Parallax barrier state.
     */
    enum BarrierState {
        BarrierState_Unknown = -1,
        BarrierState_Off     = 0,
        BarrierState_Landscape,
        BarrierState_Portrait,
    };

    /**
     * Initialize parallax barrier state.
     */
    void setBarrierState(BarrierState theBarrierState);

        private:

    StOutDevicesList          myDevices;
    StHandle<StSettings>      mySettings;
    StString                  myAbout;                    //!< about string
    StHandle<StGLContext>     myContext;
    StHandle<StGLFrameBuffer> myFrmBuffer;                //!< OpenGL frame buffer object
    StHandle<StProgramFB>     myGlPrograms[DEVICE_NB];    //!< GLSL programs
    StHandle<StProgramFB>     myGlProgramsRev[DEVICE_NB]; //!< GLSL programs with reversed left/right condition

    StHandle<StProgramFB>     myGlProgramMask;            //!< universal GLSL program which uses mask texture
    StHandle<StGLTexture>     myTextureMask;              //!< texture holding mask for discarding pixels
    int                       myTexMaskDevice;            //!< texture mask device
    bool                      myTexMaskReversed;          //!< texture mask is initialized in reversed state

    StGLVertexBuffer          myQuadVertBuf;
    StGLVertexBuffer          myQuadTexCoordBuf;
    int                       myDevice;
    StHandle<StMonitor>       myMonitor;                  //!< current monitor
    BarrierState              myBarrierState;

    StRectI_t                 myWinRect;
    StRectI_t                 myEDRect;
    StTimer                   myEDTimer;                  //!< EDimensional activator/disactivator timer
    StHandle<StGLProgram>     myEDIntelaceOn;             //!< ED interlace activate program
    StHandle<StGLProgram>     myEDOff;                    //!< ED disactivate program
    GLsizei                   myVpSizeY;                  //!< VIewPort Y size
    StGLVarLocation           myVpSizeYOnLoc;             //!< helper shader variables
    StGLVarLocation           myVpSizeYOffLoc;

    StFPSControl              myFPSControl;
    bool                      myIsMonReversed;            //!< indicates (known) monitor model with reversed rows order
    bool                      myIsMonPortrait;            //!< monitor is in portrait mode or not
    bool                      myIsStereo;
    bool                      myIsEDactive;
    bool                      myIsEDCodeFinished;
    bool                      myToCompressMem;            //!< reduce memory usage
    bool                      myIsFirstDraw;              //!< first frame draw
    bool                      myIsBroken;                 //!< special flag for broke state - when FBO can not be allocated

};

#endif //__StOutInterlace_h_
