/**
 * Copyright © 2007-2013 Kirill Gavrilov <kirill@sview.ru>
 *
 * StOutPageFlip library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * StOutPageFlip library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.
 * If not, see <http://www.gnu.org/licenses/>.
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
class StDXNVWindow;

// translation resources
enum {
    STTR_PAGEFLIP_NAME = 1000,
    STTR_PAGEFLIP_DESC = 1001,
    STTR_VUZIX_NAME    = 1002,
    STTR_VUZIX_DESC    = 1003,

    // parameters
    STTR_PARAMETER_QBUFFER_TYPE = 1102,
    STTR_PARAMETER_CONTROL_CODE = 1103,

    STTR_PARAMETER_QB_EMULATED      = 1120,
    STTR_PARAMETER_QB_HARDWARE      = 1122,
    STTR_PARAMETER_QB_D3D_ANY       = 1123,
    STTR_PARAMETER_QB_D3D_OFF       = 1124,
    STTR_PARAMETER_QB_D3D_AMD       = 1125,
    STTR_PARAMETER_QB_D3D_AMD_OFF   = 1126,
    STTR_PARAMETER_QB_D3D_NV        = 1127,
    STTR_PARAMETER_QB_D3D_NV_OFF    = 1128,

    STTR_PARAMETER_CONTROL_NO        = 1130,
    STTR_PARAMETER_CONTROL_BLUELINE  = 1131,
    STTR_PARAMETER_CONTROL_WHITELINE = 1132,
    STTR_PARAMETER_CONTROL_ED        = 1134,

    // about info
    STTR_PLUGIN_TITLE       = 2000,
    STTR_VERSION_STRING     = 2001,
    STTR_PLUGIN_DESCRIPTION = 2002,
};

/**
 * This class implements stereoscopic rendering on displays
 * with shutter glasses.
 */
class StOutPageFlip : public StWindow {

        public:

    /**
     * Main constructor.
     */
    ST_CPPEXPORT StOutPageFlip(const StNativeWin_t theParentWindow);

    /**
     * Destructor.
     */
    ST_CPPEXPORT virtual ~StOutPageFlip();

    /**
     * Renderer about string.
     */
    ST_CPPEXPORT virtual StString getRendererAbout() const;

    /**
     * Renderer id.
     */
    ST_CPPEXPORT virtual const char* getRendererId() const;

    /**
     * Active Device id.
     */
    ST_CPPEXPORT virtual const char* getDeviceId() const;

    /**
     * This methods returns device lost state.
     * In addition pageflip output requires re-initialization
     * when hardware Quad-Buffer is turned on/off.
     * @return true if rendering device requires reinitialization
     */
    ST_CPPEXPORT virtual bool isLostDevice() const;

    /**
     * Activate Device.
     */
    ST_CPPEXPORT virtual bool setDevice(const StString& theDevice);

    /**
     * Devices list.
     */
    ST_CPPEXPORT virtual void getDevices(StOutDevicesList& theList) const;

    /**
     * Retrieve options list.
     */
    ST_CPPEXPORT virtual void getOptions(StParamsList& theList) const;

    /**
     * Create and show window.
     * @return false if any critical error appeared
     */
    ST_CPPEXPORT virtual bool create();

    /**
     * Close the window.
     */
    ST_CPPEXPORT virtual void close();

    /**
     * Process callback.
     */
    ST_CPPEXPORT virtual void processEvents(StMessage_t* theMessages);

    /**
     * Stereo renderer.
     */
    ST_CPPEXPORT virtual void stglDraw();

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
     * Switch quad buffer callback.
     */
    ST_LOCAL void doSetQuadBuffer(const int32_t );

    /**
     * Show/hide extra options callback.
     */
    ST_LOCAL void doShowExtra(const bool theValue);

    ST_LOCAL         void stglDrawWarning();
    ST_LOCAL virtual void stglDrawExtra(unsigned int theView, int theMode);
    ST_LOCAL virtual void stglResize(const StRectI_t& theWinRect);

        protected:

    typedef enum tagDeviceEnum {
        DEVICE_AUTO     =-1,
        DEVICE_SHUTTERS = 0, // generic shutter glasses
        DEVICE_VUZIX    = 1, // Vuzix HMD
    } DeviceEnum;

    typedef enum tagQuadBufferEnum {
        QUADBUFFER_AUTO            =-1, // autodetection
        QUADBUFFER_HARD_OPENGL     = 0, // OpenGL hardware Quad Buffer
        QUADBUFFER_HARD_D3D_ANY    = 1, // NVIDIA or AMD
        QUADBUFFER_SOFT            = 2, // OpenGL emulated Quad Buffer
    } QuadBufferEnum;

        protected:

    struct {

        StHandle<StBoolParam> ToShowExtra; //!< show extra options
        StHandle<StEnumParam> QuadBuffer;  //!< quad buffer option

    } params;

        protected:

    StOutDevicesList      myDevices;
    StHandle<StSettings>  mySettings;
    StString              myAbout;      //!< about string
    StHandle<StGLContext> myContext;
    StHandle<StVuzixSDK>  myVuzixSDK;   //!< Vuzix HMD control
    StHandle<StGLTexture> myWarning;
    StTranslations        myLangMap;
    DeviceEnum            myDevice;
    StDXInfo              myDxInfo;
    StFPSControl          myFPSControl;
    bool                  myToSavePlacement;
    bool                  myToDrawStereo;
#ifdef _WIN32
    bool                  myIsVistaPlus;
#endif
    bool                  myToResetDevice;

    struct StOutDirect3D {
        StHandle<StGLFrameBuffer> myGLBuffer;
    #ifdef _WIN32
        StHandle<StDXNVWindow>    myDxWindow;
        StHandle<StThread>        myDxThread;
    #endif
        GLuint myGLIoBuff;
        int    myActivateStep;
        bool   myIsActive;
        bool   myToUsePBO;

        ST_LOCAL StOutDirect3D();

    #ifdef _WIN32
        static SV_THREAD_FUNCTION dxThreadFunction(void* theStOutD3d);
    #endif // _WIN32

    } myOutD3d; //!< auxiliary Direct3D stuff

};

#endif //__StOutPageFlip_h_
