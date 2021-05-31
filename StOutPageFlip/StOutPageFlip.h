/**
 * StOutPageFlip, class providing stereoscopic output for Shutter Glasses displays using StCore toolkit.
 * Copyright © 2007-2017 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#ifndef __StOutPageFlip_h_
#define __StOutPageFlip_h_

#include <StCore/StWindow.h>
#include <StGL/StGLVec.h>
#include <StGL/StGLVertexBuffer.h>
#include <StGL/StGLTexture.h>
#include <StThreads/StFPSControl.h>
#include <StSettings/StEnumParam.h>
#include <StSettings/StTranslations.h>

#include "StDXInfo.h"

class StSettings;
class StWindow;
class StVuzixSDK;
class StGLFrameBuffer;
class StGLTextureQuad;
class StDXNVWindow;

/**
 * This class implements stereoscopic rendering on displays
 * with shutter glasses.
 */
class StOutPageFlip : public StWindow {

        public:

    /**
     * Initialize global Quad Buffer support state.
     * Notice that this check performed asynchronously in dedicated thread.
     * You should call this function as soon as possible at process start
     * to reduce initialization delay.
     */
    ST_CPPEXPORT static void initGlobalsAsync();

        public:

    /**
     * Main constructor.
     */
    ST_CPPEXPORT StOutPageFlip(const StHandle<StResourceManager>& theResMgr,
                               const StNativeWin_t                theParentWindow);

    /**
     * Destructor.
     */
    ST_CPPEXPORT virtual ~StOutPageFlip();

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
     * In addition pageflip output requires re-initialization
     * when hardware Quad-Buffer is turned on/off.
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
     * Stereo renderer.
     */
    ST_CPPEXPORT virtual void stglDraw() ST_ATTR_OVERRIDE;

    /**
     * Show/Hide mouse cursor.
     * @param theToShow true to show cursor
     */
    ST_CPPEXPORT virtual void showCursor(const bool theToShow) ST_ATTR_OVERRIDE;

    /**
     * Update strings.
     */
    ST_LOCAL virtual void doChangeLanguage() ST_ATTR_OVERRIDE;

    /**
     * Return true if 3D output requires fullscreen mode.
     */
    ST_CPPEXPORT virtual bool isStereoFullscreenOnly() const ST_ATTR_OVERRIDE;

        protected:

    ST_LOCAL void setupDevice();
    ST_LOCAL void stglDrawAggressive(unsigned int theView);

        protected:

    ST_LOCAL bool dxInit();
    ST_LOCAL void dxRelease();
    ST_LOCAL void dxActivate();
    ST_LOCAL void dxDisactivate();
    ST_LOCAL void dxDraw(unsigned int theView);

        protected:

    /**
     * Release GL resources before window closing.
     */
    ST_LOCAL virtual void releaseResources();

    /**
     * Update strings.
     */
    ST_LOCAL void updateStrings();

    /**
     * Switch quad buffer callback.
     */
    ST_LOCAL void doSetQuadBuffer(const int32_t );

    /**
     * Show/hide extra options callback.
     */
    ST_LOCAL void doShowExtra(const bool theValue);

    /**
     * Process monitor change event.
     */
    ST_LOCAL void doNewMonitor(const StSizeEvent& theEvent);

    /**
     * On/off VSync callback.
     */
    ST_LOCAL void doSwitchVSync(const int32_t theValue);

    ST_LOCAL virtual void stglDrawExtra(unsigned int theView, int theMode);

        protected:

    typedef enum tagDeviceEnum {
        DEVICE_AUTO     =-1,
        DEVICE_SHUTTERS = 0, // generic shutter glasses
        DEVICE_VUZIX    = 1, // Vuzix HMD
    } DeviceEnum;

    typedef enum tagQuadBufferEnum {
        QUADBUFFER_HARD_OPENGL = 0, //!< OpenGL hardware Quad Buffer
    #ifdef _WIN32
        QUADBUFFER_HARD_D3D_ANY,    //!< NVIDIA or AMD extension for D3D
    #endif
        QUADBUFFER_SOFT,            //!< OpenGL emulated Page Flip
    } QuadBufferEnum;

        protected:

    struct {

        StHandle<StBoolParamNamed> ToShowExtra; //!< show extra options
        StHandle<StEnumParam>      QuadBuffer;  //!< quad buffer option

    } params;

        protected:

    StOutDevicesList      myDevices;
    StHandle<StSettings>  mySettings;
    StString              myAbout;      //!< about string
    StHandle<StGLContext> myContext;
    StHandle<StVuzixSDK>  myVuzixSDK;   //!< Vuzix HMD control
    StHandle<StGLTextureQuad> myWarning;
    StTranslations        myLangMap;
    DeviceEnum            myDevice;
    StDXInfo              myDxInfo;
    StFPSControl          myFPSControl;
    bool                  myToDrawStereo;
    bool                  myToResetDevice;

    class StGLDXFrameBuffer;
    class StProgramQuad;

    struct StOutDirect3D {
        StHandle<StGLFrameBuffer>   GlBuffer;
        StHandle<StProgramQuad>     Program;
        StGLVertexBuffer            VertBuf;
        StGLVertexBuffer            TCrdBuf;
    #ifdef _WIN32
        StHandle<StDXNVWindow>      DxWindow;
        StHandle<StThread>          DxThread;
        StHandle<StGLDXFrameBuffer> WglDxBuffer;
    #endif
        GLuint GlIoBuff;
        int    ActivateStep;
        bool   IsActive;
        bool   ToUsePBO;

        ST_LOCAL StOutDirect3D();
        ST_LOCAL ~StOutDirect3D();

    #ifdef _WIN32
        static SV_THREAD_FUNCTION dxThreadFunction(void* theStOutD3d);
    #endif // _WIN32

    } myOutD3d; //!< auxiliary Direct3D stuff

    friend class StDXNVWindow;

};

#endif // __StOutPageFlip_h_
