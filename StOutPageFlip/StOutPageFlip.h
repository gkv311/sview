/**
 * Copyright © 2007-2012 Kirill Gavrilov <kirill@sview.ru>
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

#include <StCore/StCore.h> // header for Stereo Output Core
#include <StCore/StWindowInterface.h>
#include <StGL/StGLVec.h>
#include <StGL/StGLVertexBuffer.h>
#include <StGL/StGLTexture.h>
#include <StThreads/StFPSControl.h>

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
    STTR_PARAMETER_SHOW_FPS     = 1101,
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

class ST_LOCAL StOutPageFlip : public StRendererInterface {

        protected:

    typedef enum tagDeviceEnum {
        DEVICE_AUTO     =-1,
        DEVICE_SHUTTERS = 0, // generic shutter glasses
        DEVICE_VUZIX    = 1, // Vuzix HMD
    } DeviceEnum;

    enum {
        DEVICE_OPTION_SHOWFPS    = 0,
        DEVICE_OPTION_EXTRA      = 1,
        DEVICE_OPTION_QUADBUFFER = 2,
    } DeviceOption;

    typedef enum tagQuadBufferEnum {
        QUADBUFFER_AUTO            =-1, // autodetection
        QUADBUFFER_HARD_OPENGL     = 0, // OpenGL hardware Quad Buffer
        QUADBUFFER_HARD_D3D_ANY    = 1, // NVIDIA or AMD
        QUADBUFFER_SOFT            = 2, // OpenGL emulated Quad Buffer
    } QuadBufferEnum;

        protected:

    StHandle<StCore>      myStCore;
    StHandle<StSettings>  mySettings;
    StString              myPluginPath;
    StWinAttributes_t     myWinAttribs;
    StSDOptionsList_t*    myOptions;
    StHandle<StGLContext> myContext;
    StHandle<StVuzixSDK>  myVuzixSDK; // Vuzix HMD control
    DeviceEnum            myDevice;
    int                   myDeviceOptionsNb;
    QuadBufferEnum        myQuadBuffer;
    QuadBufferEnum        myQuadBufferMax;
    StDXInfo              myDxInfo;
    StFPSControl          myFPSControl;
    bool                  myToSavePlacement;
    bool                  myToDrawStereo;
    bool                  myToShowFPS;

    struct StOutDirect3D {
        StHandle<StGLFrameBuffer> myGLBuffer;
    #if(defined(_WIN32) || defined(__WIN32__))
        StHandle<StDXNVWindow>    myDxWindow;
        StHandle<StThread>        myDxThread;
        StHandle<StGLTexture>     myWarning;
    #endif
        GLuint myGLIoBuff;
        int    myActivateStep;
        bool   myIsActive;
        bool   myToUsePBO;

        StOutDirect3D();

    #if(defined(_WIN32) || defined(__WIN32__))
        static SV_THREAD_FUNCTION dxThreadFunction(void* theStOutD3d);
    #endif // _WIN32

    } myOutD3d; //!< auxiliary Direct3D stuff

        protected:

    void setupDevice();
    void stglDrawAggressive(unsigned int theView);

    bool dxInit();
    void dxRelease();
    void dxActivate();
    void dxDisactivate();
    void dxDraw(unsigned int theView);

        protected:

    virtual void optionsStructAlloc();
    virtual void updateOptions(const StSDOptionsList_t* theOptions,
                               StMessage_t&             theMsg);
    virtual void stglDrawExtra(unsigned int theView, int theMode);
    virtual void stglResize(const StRectI_t& theWinRect);
    virtual void parseKeys(bool* theKeysMap);

        public:

    StOutPageFlip(const StHandle<StSettings>& theSettings);
    virtual ~StOutPageFlip();
    virtual StRendererInterface* getLibImpl() { return this; }
    virtual StWindowInterface* getStWindow() { return myStCore->getStWindow(); }
    virtual bool init(const StString& , const int& , const StNativeWin_t );
    virtual bool open(const StOpenInfo& theOpenInfo) { return myStCore->open(theOpenInfo); }
    virtual void callback(StMessage_t* );
    virtual void stglDraw(unsigned int );

};

#endif //__StOutPageFlip_h_
