/**
 * StOutDistorted, class providing stereoscopic output in anamorph side by side format using StCore toolkit.
 * Copyright © 2013-2017 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#include "StOutDistorted.h"

#include "StProgramBarrel.h"
#include "StProgramFlat.h"

#include <StGL/StGLContext.h>
#include <StGL/StGLEnums.h>
#include <StGL/StGLProgram.h>
#include <StGL/StGLFrameBuffer.h>
#include <StGL/StGLArbFbo.h>
#include <StGLCore/StGLCore20.h>
#include <StSettings/StSettings.h>
#include <StSettings/StTranslations.h>
#include <StSettings/StEnumParam.h>
#include <StCore/StSearchMonitors.h>
#include <StVersion.h>
#include <StAV/StAVImage.h>

#ifdef ST_HAVE_OPENVR
    #include <openvr.h>

    #ifdef _MSC_VER
        #pragma comment(lib, "openvr_api.lib")
    #endif
#elif defined(ST_HAVE_LIBOVR)
    #include <OVR.h>
    #include <OVR_CAPI_GL.h>

    #ifdef _MSC_VER
        #pragma comment(lib, "LibOVR.lib")
    #endif
#endif

namespace {

    static const char ST_OUT_PLUGIN_NAME[]   = "StOutDistorted";

    static const char ST_SETTING_DEVICE_ID[] = "deviceId";
    static const char ST_SETTING_WINDOWPOS[] = "windowPos";
    static const char ST_SETTING_WARP_COEF[] = "warpCoef";
    static const char ST_SETTING_CHROME_AB[] = "chromeAb";

    // translation resources
    enum {
        STTR_DISTORTED_NAME     = 1000,
        STTR_DISTORTED_DESC     = 1001,
        STTR_OPENVR_NAME        = 1002,
        STTR_OPENVR_DESC        = 1003,
        STTR_S3DV_NAME          = 1004,
        STTR_S3DV_DESC          = 1005,

        // parameters
        STTR_PARAMETER_LAYOUT     = 1110,
        STTR_PARAMETER_LAYOUT_SBS        = 1111,
        STTR_PARAMETER_LAYOUT_OVERUNDER  = 1112,
        STTR_PARAMETER_LAYOUT_SBS_ANAMORPH = 1113,
        STTR_PARAMETER_LAYOUT_OVERUNDER_ANAMORPH = 1114,
        STTR_PARAMETER_DISTORTION = 1120,
        STTR_PARAMETER_DISTORTION_OFF    = 1121,
        STTR_PARAMETER_MONOCLONE         = 1123,

        // about info
        STTR_PLUGIN_TITLE       = 2000,
        STTR_VERSION_STRING     = 2001,
        STTR_PLUGIN_DESCRIPTION = 2002,
    };

#ifdef ST_HAVE_OPENVR
    static StString getVrTrackedDeviceString(vr::IVRSystem* theHmd,
                                             vr::TrackedDeviceIndex_t theDevice,
                                             vr::TrackedDeviceProperty theProperty,
                                             vr::TrackedPropertyError* theError = NULL) {
      const uint32_t aBuffLen = theHmd->GetStringTrackedDeviceProperty(theDevice, theProperty, NULL, 0, theError);
      if(aBuffLen == 0) {
          return StString();
      }

      char* aBuffer = new char[aBuffLen + 1];
      theHmd->GetStringTrackedDeviceProperty(theDevice, theProperty, aBuffer, aBuffLen, theError);
      aBuffer[aBuffLen] = '\0';
      const StString aResult(aBuffer);
      delete[] aBuffer;
      return aResult;
    }

    /**
     * Print OpenVR compositor error.
     */
    StString getVRCompositorError(vr::EVRCompositorError theVRError) {
        switch(theVRError) {
            case vr::VRCompositorError_None:
              return "None";
            case vr::VRCompositorError_RequestFailed:
              return "Compositor Error: Request Failed";
            case vr::VRCompositorError_IncompatibleVersion:
              return "Compositor Error: Incompatible Version";
            case vr::VRCompositorError_DoNotHaveFocus:
              return "Compositor Error: Do not have focus";
            case vr::VRCompositorError_InvalidTexture:
              return "Compositor Error: Invalid Texture";
            case vr::VRCompositorError_IsNotSceneApplication:
              return "Compositor Error: Is not scene application";
            case vr::VRCompositorError_TextureIsOnWrongDevice:
              return "Compositor Error: Texture is on wrong device";
            case vr::VRCompositorError_TextureUsesUnsupportedFormat:
              return "Compositor Error: Texture uses unsupported format";
            case vr::VRCompositorError_SharedTexturesNotSupported:
              return "Compositor Error: Shared textures not supported";
            case vr::VRCompositorError_IndexOutOfRange:
              return "Compositor Error: Index out of range";
            case vr::VRCompositorError_AlreadySubmitted:
              return "Compositor Error: Already submitted";
        }
        return StString("Compositor Error: UNKNOWN #") + int(theVRError);
    }
#endif

}

StAtomic<int32_t> StOutDistorted::myInstancesNb(0);

StString StOutDistorted::getRendererAbout() const {
    return myAbout;
}

const char* StOutDistorted::getRendererId() const {
    return ST_OUT_PLUGIN_NAME;
}

const char* StOutDistorted::getDeviceId() const {
    switch(myDevice) {
        case DEVICE_HMD:       return "OpenVR";
        case DEVICE_S3DV:      return "S3DV";
        case DEVICE_DISTORTED:
        default:               return "Distorted";
    }
}

bool StOutDistorted::isLostDevice() const {
    return myToResetDevice || StWindow::isLostDevice();
}

bool StOutDistorted::setDevice(const StString& theDevice) {
    if(theDevice == "OpenVR") {
        if(myDevice != DEVICE_HMD) {
            myToResetDevice = true;
        }
        myDevice = DEVICE_HMD;
    } else if(theDevice == "S3DV") {
        if(myDevice != DEVICE_S3DV) {
            myToResetDevice = true;
        }
        myDevice = DEVICE_S3DV;
    } else if(theDevice == "Distorted") {
        if(myDevice != DEVICE_DISTORTED) {
            myToResetDevice = true;
        }
        myDevice = DEVICE_DISTORTED;
    }
    return false;
}

void StOutDistorted::getDevices(StOutDevicesList& theList) const {
    for(size_t anIter = 0; anIter < myDevices.size(); ++anIter) {
        theList.add(myDevices[anIter]);
    }
}

void StOutDistorted::getOptions(StParamsList& theList) const {
    if(myDevice != DEVICE_HMD
    && myDevice != DEVICE_S3DV) {
        theList.add(params.Layout);
    }
    theList.add(params.MonoClone);
}

void StOutDistorted::updateStrings() {
    StTranslations aLangMap(getResourceManager(), ST_OUT_PLUGIN_NAME);

    myDevices[DEVICE_DISTORTED]->Name = aLangMap.changeValueId(STTR_DISTORTED_NAME, "TV (parallel pair)");
    myDevices[DEVICE_DISTORTED]->Desc = aLangMap.changeValueId(STTR_DISTORTED_DESC, "Distorted Output");
    myDevices[DEVICE_HMD]->Name       = aLangMap.changeValueId(STTR_OPENVR_NAME,    "OpenVR HMD");
    myDevices[DEVICE_HMD]->Desc       = aLangMap.changeValueId(STTR_OPENVR_DESC,    "Distorted Output");
    if(myDevices.size() > DEVICE_S3DV) {
        myDevices[DEVICE_S3DV] ->Name = "S3DV";             //aLangMap.changeValueId(STTR_S3DV_NAME, "S3DV");
        myDevices[DEVICE_S3DV] ->Desc = "Distorted Output"; //aLangMap.changeValueId(STTR_S3DV_DESC, "Distorted Output");
    }

    params.MonoClone->setName(aLangMap.changeValueId(STTR_PARAMETER_MONOCLONE, "Show Mono in Stereo"));

    params.Layout->setName(aLangMap.changeValueId(STTR_PARAMETER_LAYOUT, "Layout"));
    params.Layout->defineOption(LAYOUT_SIDE_BY_SIDE_ANAMORPH, aLangMap.changeValueId(STTR_PARAMETER_LAYOUT_SBS_ANAMORPH,       "Side-by-Side (Anamorph)"));
    params.Layout->defineOption(LAYOUT_OVER_UNDER_ANAMORPH,   aLangMap.changeValueId(STTR_PARAMETER_LAYOUT_OVERUNDER_ANAMORPH, "Top-and-Bottom (Anamorph)"));
    params.Layout->defineOption(LAYOUT_SIDE_BY_SIDE,          aLangMap.changeValueId(STTR_PARAMETER_LAYOUT_SBS,                "Side-by-Side"));
    params.Layout->defineOption(LAYOUT_OVER_UNDER,            aLangMap.changeValueId(STTR_PARAMETER_LAYOUT_OVERUNDER,          "Top-and-Bottom") + (myCanHdmiPack ? " [HDMI]" : ""));

    // about string
    myAboutTitle     = aLangMap.changeValueId(STTR_PLUGIN_TITLE,   "sView - Distorted Output module");
    myAboutVerString = aLangMap.changeValueId(STTR_VERSION_STRING, "version");
    myAboutDescr     = aLangMap.changeValueId(STTR_PLUGIN_DESCRIPTION,
                                              "(C) {0} Kirill Gavrilov <{1}>\nOfficial site: {2}\n\nThis library is distributed under LGPL3.0");
    updateAbout();
}

void StOutDistorted::updateAbout() {
    myAbout = myAboutTitle + '\n' + myAboutVerString + " " + StVersionInfo::getSDKVersionString() + "\n \n"
            + (!myAboutVrDevice.isEmpty() ? ("Connected hardware: " + myAboutVrDevice + "\n \n") : "")
            + myAboutDescr.format("2013-2017", "kirill@sview.ru", "www.sview.ru");
#ifdef ST_HAVE_OPENVR
    myAbout = myAbout + "\n \n"
            + "This software uses OpenVR library:\n"
            + "https://github.com/ValveSoftware/openvr\n"
            + "\xC2\xA9 2015 Valve Corporation";
#endif
}

StOutDistorted::StOutDistorted(const StHandle<StResourceManager>& theResMgr,
                               const StNativeWin_t                theParentWindow)
: StWindow(theResMgr, theParentWindow),
  mySettings(new StSettings(theResMgr, ST_OUT_PLUGIN_NAME)),
  myDevice(DEVICE_AUTO),
  myToResetDevice(false),
  myFrBuffer(new StGLFrameBuffer()),
  myCursor(new StGLTexture(GL_RGBA8)),
  myProgramFlat(new StProgramFlat()),
  myProgramBarrel(new StProgramBarrel()),
  myBarrelCoef(1.0f, 0.22f, 0.24f, 0.041f), // 7 inches
  //myBarrelCoef(1.0f, 0.18f, 0.115f, 0.0387f),
  myChromAb(0.996f, -0.004f, 1.014f, 0.0f),
  //myChromAb(1.0f, 0.0f, 1.0f, 0.0f),
  myVrMarginsTop(0.35),
  myVrMarginsBottom(0.35),
  myVrMarginsLeft(0.33),
  myVrMarginsRight(0.33),
  myVrRendSizeX(0),
  myVrRendSizeY(0),
  myVrTrackOrient(false),
#ifdef ST_HAVE_OPENVR
  myVrHmd(NULL),
  myVrTrackedPoses(new vr::TrackedDevicePose_t[vr::k_unMaxTrackedDeviceCount]),
#elif defined(ST_HAVE_LIBOVR)
  myVrHmd(NULL),
  myOvrSwapTexture(NULL),
  myOvrMirrorTexture(NULL),
  myOvrMirrorFbo(0),
#endif
  myToShowCursor(true),
  myToCompressMem(myInstancesNb.increment() > 1),
  myIsBroken(false),
  myIsStereoOn(false),
  myCanHdmiPack(false),
  myIsHdmiPack(false),
  myIsForcedFboUsage(false) {
#ifdef ST_HAVE_LIBOVR
    myOvrSwapFbo[0] = 0;
    myOvrSwapFbo[1] = 0;
#endif
    const StSearchMonitors& aMonitors = StWindow::getMonitors();

    // detect connected displays
    int aSupportOpenVr   = ST_DEVICE_SUPPORT_NONE;
    int aSupportParallel = ST_DEVICE_SUPPORT_NONE;
    int aSupportS3DV     = ST_DEVICE_SUPPORT_NONE;
    for(size_t aMonIter = 0; aMonIter < aMonitors.size(); ++aMonIter) {
        const StMonitor& aMon = aMonitors[aMonIter];
        if(aMon.getPnPId().isStartsWith(stCString("OVR"))) {
            // Oculus Rift
            aSupportOpenVr = ST_DEVICE_SUPPORT_HIGHT;
            break;
        } else if(aMon.getPnPId().isEquals(stCString("ST@S3DV"))) {
            aSupportS3DV = ST_DEVICE_SUPPORT_PREFER;
            break;
        } else if(aMon.getVRect().width()  == 1920
               && aMon.getVRect().height() == 2205) {
            aSupportParallel = ST_DEVICE_SUPPORT_HIGHT;
            myCanHdmiPack = true;
        } else if(aMon.getVRect().width()  == 1280
               && aMon.getVRect().height() == 1470) {
            aSupportParallel = ST_DEVICE_SUPPORT_HIGHT;
            myCanHdmiPack = true;
        }
    }

#ifdef ST_HAVE_OPENVR
    if(vr::VR_IsHmdPresent()) {
        aSupportOpenVr = ST_DEVICE_SUPPORT_PREFER;
    }
#elif defined(ST_HAVE_LIBOVR)
    const ovrResult anOvrRes = ovr_Initialize(NULL);
    if(!OVR_SUCCESS(anOvrRes)) {
        ST_ERROR_LOG("StOutDistorted, OVR initialization has failed!");
    } else {
        aSupportOpenVr = ST_DEVICE_SUPPORT_HIGHT;
    }
#endif

    // devices list
    StHandle<StOutDevice> aDevDistorted = new StOutDevice();
    aDevDistorted->PluginId = ST_OUT_PLUGIN_NAME;
    aDevDistorted->DeviceId = stCString("Distorted");
    aDevDistorted->Priority = aSupportParallel;
    aDevDistorted->Name     = stCString("TV (parallel pair)");
    myDevices.add(aDevDistorted);

    StHandle<StOutDevice> aDevVR = new StOutDevice();
    aDevVR->PluginId = ST_OUT_PLUGIN_NAME;
    aDevVR->DeviceId = stCString("OpenVR");
    aDevVR->Priority = aSupportOpenVr;
    aDevVR->Name     = stCString("OpenVR");
    myDevices.add(aDevVR);

    if(aSupportS3DV != ST_DEVICE_SUPPORT_NONE) {
        StHandle<StOutDevice> aDevS3dv = new StOutDevice();
        aDevS3dv->PluginId = ST_OUT_PLUGIN_NAME;
        aDevS3dv->DeviceId = stCString("S3DV");
        aDevS3dv->Priority = aSupportS3DV;
        aDevS3dv->Name     = stCString("S3DV");
        myDevices.add(aDevS3dv);
    }

    // load device settings
    mySettings->loadInt32(ST_SETTING_DEVICE_ID, myDevice);
    if(myDevice == DEVICE_AUTO) {
        myDevice = DEVICE_DISTORTED;
        if(aSupportS3DV != ST_DEVICE_SUPPORT_NONE) {
            myDevice = DEVICE_S3DV;
        }
    }

    // Distortion parameters
    params.MonoClone = new StBoolParamNamed(false, stCString("monoClone"), stCString("monoClone"));
    // Layout option
    params.Layout = new StEnumParam(myCanHdmiPack ? LAYOUT_OVER_UNDER : LAYOUT_SIDE_BY_SIDE_ANAMORPH, stCString("layout"), stCString("layout"));
    updateStrings();

    // load window position
    if(isMovable()) {
        StRect<int32_t> aRect;
        if(!mySettings->loadInt32Rect(ST_SETTING_WINDOWPOS, aRect)) {
            aRect = defaultRect();
        }
        StWindow::setPlacement(aRect, true);
    }
    mySettings->loadParam(params.MonoClone);
    mySettings->loadParam(params.Layout);
    checkHdmiPack();
    StWindow::setTitle("sView - Distorted Renderer");

    mySettings->loadFloatVec4(ST_SETTING_WARP_COEF, myBarrelCoef);
    mySettings->loadFloatVec4(ST_SETTING_CHROME_AB, myChromAb);
}

void StOutDistorted::releaseResources() {
    if(!myContext.isNull()) {
    #ifdef ST_HAVE_OPENVR
        if(myVrHmd != NULL) {
            vr::VR_Shutdown();
            myVrHmd = NULL;
        }
    #elif defined(ST_HAVE_LIBOVR)
        if(myOvrSwapFbo[0] != 0) {
            myContext->arbFbo->glDeleteFramebuffers(2, myOvrSwapFbo);
            myOvrSwapFbo[0] = 0;
            myOvrSwapFbo[1] = 0;
        }
        if(myOvrSwapTexture != NULL) {
            ovr_DestroySwapTextureSet(myVrHmd, myOvrSwapTexture);
            myOvrSwapTexture = NULL;
        }
        if(myOvrMirrorFbo != 0) {
            myContext->arbFbo->glDeleteFramebuffers(1, &myOvrMirrorFbo);
            myOvrMirrorFbo = NULL;
        }
        if(myOvrMirrorTexture != NULL) {
            ovr_DestroyMirrorTexture(myVrHmd, &myOvrMirrorTexture->Texture);
            myOvrMirrorTexture = NULL;
        }
        if(myVrHmd != NULL) {
            ovr_Destroy(myVrHmd);
            myVrHmd = NULL;
        }
    #endif

        myProgramFlat->release(*myContext);
        myProgramBarrel->release(*myContext);
        myFrVertsBuf .release(*myContext);
        myFrTCrdsBuf .release(*myContext);
        myCurVertsBuf.release(*myContext);
        myCurTCrdsBuf.release(*myContext);
        myFrBuffer->release(*myContext);
        myCursor->release(*myContext);
    }
    myContext.nullify();

    // read windowed placement
    StWindow::hide();
    if(isMovable()) {
        StWindow::setFullScreen(false);
    }
}

void StOutDistorted::beforeClose() {
    if(isMovable() && myWasUsed) {
        mySettings->saveInt32Rect(ST_SETTING_WINDOWPOS, StWindow::getWindowedPlacement());
    }

    mySettings->saveParam(params.Layout);
    mySettings->saveParam(params.MonoClone);
    mySettings->saveFloatVec4(ST_SETTING_WARP_COEF, myBarrelCoef);
    mySettings->saveFloatVec4(ST_SETTING_CHROME_AB, myChromAb);
    if(myWasUsed) {
        mySettings->saveInt32(ST_SETTING_DEVICE_ID, myDevice);
    }
    mySettings->flush();
}

StOutDistorted::~StOutDistorted() {
    myInstancesNb.decrement();
    releaseResources();

#ifdef ST_HAVE_OPENVR
    delete[] myVrTrackedPoses;
#elif defined(ST_HAVE_LIBOVR)
    ovr_Shutdown();
#endif
}

void StOutDistorted::close() {
    beforeClose();
    StWindow::params.VSyncMode->signals.onChanged -= stSlot(this, &StOutDistorted::doSwitchVSync);
    myToResetDevice = false;
    releaseResources();
    StWindow::close();
}

bool StOutDistorted::create() {
    StWindow::show();
    if(!StWindow::create()) {
        return false;
    }

    // initialize GL context
    myContext = StWindow::getContext();
    myContext->setMessagesQueue(myMsgQueue);
    myIsBroken = false;
    if(!myContext->isGlGreaterEqual(2, 0)) {
        myMsgQueue->pushError(stCString("OpenGL 2.0 is required by Distorted Output"));
        myIsBroken = true;
        return true;
    }

    StWindow::stglMakeCurrent(ST_WIN_MASTER);
    myContext->stglSetVSync((StGLContext::VSync_Mode )StWindow::params.VSyncMode->getValue());
    StWindow::params.VSyncMode->signals.onChanged += stSlot(this, &StOutDistorted::doSwitchVSync);

    if(!myProgramFlat  ->init(*myContext)
    || !myProgramBarrel->init(*myContext)) {
        myMsgQueue->pushError(stCString("Distorted output - critical error:\nShaders initialization failed!"));
        myIsBroken = true;
        return true;
    }
    myProgramBarrel->setupCoeff (*myContext, myBarrelCoef);
    myProgramBarrel->setupChrome(*myContext, myChromAb);

    // create vertices buffers to draw simple textured quad
    const GLfloat QUAD_VERTICES[4 * 4] = {
         1.0f, -1.0f, 0.0f, 1.0f, // top-right
         1.0f,  1.0f, 0.0f, 1.0f, // bottom-right
        -1.0f, -1.0f, 0.0f, 1.0f, // top-left
        -1.0f,  1.0f, 0.0f, 1.0f  // bottom-left
    };

    const GLfloat QUAD_TEXCOORD[2 * 4] = {
        1.0f, 1.0f,
        1.0f, 0.0f,
        0.0f, 1.0f,
        0.0f, 0.0f
    };

    myFrVertsBuf .init(*myContext, 4, 4, QUAD_VERTICES);
    myFrTCrdsBuf .init(*myContext, 2, 4, QUAD_TEXCOORD);
    myCurVertsBuf.init(*myContext, 4, 4, QUAD_VERTICES);
    myCurTCrdsBuf.init(*myContext, 2, 4, QUAD_TEXCOORD);

    // cursor texture
    StAVImage aCursorImg;
    StHandle<StResource> aCursorRes = getResourceManager()->getResource(StString("textures") + SYS_FS_SPLITTER + "cursor.png");
    uint8_t* aData     = NULL;
    int      aDataSize = 0;
    if(!aCursorRes.isNull()
    && !aCursorRes->isFile()
    &&  aCursorRes->read()) {
        aData     = (uint8_t* )aCursorRes->getData();
        aDataSize = aCursorRes->getSize();
    }
    if(aCursorImg.load(!aCursorRes.isNull() ? aCursorRes->getPath() : StString(), StImageFile::ST_TYPE_PNG, aData, aDataSize)) {
        myCursor->init(*myContext, aCursorImg.getPlane());
    }

#ifdef ST_HAVE_OPENVR
    if(myDevice == DEVICE_HMD) {
        vr::EVRInitError aVrError = vr::VRInitError_None;
        myVrHmd = vr::VR_Init(&aVrError, vr::VRApplication_Scene);
        if(aVrError != vr::VRInitError_None) {
            myVrHmd = NULL;
            myMsgQueue->pushError(StString("Unable to init VR runtime: ") + vr::VR_GetVRInitErrorAsEnglishDescription(aVrError));
        }

        if(myVrHmd != NULL) {
            /*vr::IVRRenderModels* aRenderModels = (vr::IVRRenderModels* )vr::VR_GetGenericInterface(vr::IVRRenderModels_Version, &aVrError);
            if(aRenderModels == NULL) {
                myMsgQueue->pushError(StString("Unable to get render model interface: ") + vr::VR_GetVRInitErrorAsEnglishDescription(aVrError));
            }*/
        }
    }

    myAboutVrDevice.clear();
    if(myVrHmd != NULL) {
        const StString aVrManuf   = getVrTrackedDeviceString(myVrHmd, vr::k_unTrackedDeviceIndex_Hmd, vr::Prop_ManufacturerName_String);
        const StString aVrDriver  = getVrTrackedDeviceString(myVrHmd, vr::k_unTrackedDeviceIndex_Hmd, vr::Prop_TrackingSystemName_String);
        const StString aVrDisplay = getVrTrackedDeviceString(myVrHmd, vr::k_unTrackedDeviceIndex_Hmd, vr::Prop_SerialNumber_String);

        uint32_t aRenderSizeX = 0;
        uint32_t aRenderSizeY = 0;
        myVrHmd->GetRecommendedRenderTargetSize(&aRenderSizeX, &aRenderSizeY);
        myAboutVrDevice = aVrManuf + " " + aVrDriver + "\n"
                        + aVrDisplay + " [" + aRenderSizeX + "x" + aRenderSizeY + "]";
        myVrRendSizeX = int(aRenderSizeX);
        myVrRendSizeY = int(aRenderSizeY);
        updateAbout();
    }

#elif defined(ST_HAVE_LIBOVR)
    if(myDevice == DEVICE_HMD) {
        ovrGraphicsLuid aLuid;
        const ovrResult anOvrRes = ovr_Create(&myVrHmd, &aLuid);
        if(myVrHmd == NULL
        || !OVR_SUCCESS(anOvrRes)) {
            myMsgQueue->pushError(stCString("StOutDistorted, Oculus Rift is not connected!"));
            myVrHmd = NULL;
            return true;
        }
    }

    if(myVrHmd != NULL) {
        ovrHmdDesc anHmdDesc = ovr_GetHmdDesc(myVrHmd);
        ovrSizei aWinSize = { anHmdDesc.Resolution.w / 2, anHmdDesc.Resolution.h / 2 };

        ST_DEBUG_LOG("libOVR Resolution: " + anHmdDesc.Resolution.w + "x" + anHmdDesc.Resolution.h);
        if(isMovable()) {
            StRect<int32_t> aRect = StWindow::getPlacement();
            aRect.right()  = aRect.left() + aWinSize.w;
            aRect.bottom() = aRect.top()  + aWinSize.h;
            StWindow::setPlacement(aRect, false);
        }

        ovrResult anOvrRes = ovr_CreateMirrorTextureGL(myVrHmd, GL_SRGB8_ALPHA8, aWinSize.w, aWinSize.h, (ovrTexture** )&myOvrMirrorTexture);
        if(!OVR_SUCCESS(anOvrRes)) {
            myMsgQueue->pushError(stCString("StOutDistorted, Failed to create mirror texture!"));
            myIsBroken = true;
            return true;
        }

        const GLuint anFboReadBack = myContext->stglFramebufferRead();
        myContext->arbFbo->glGenFramebuffers(1, &myOvrMirrorFbo);
        myContext->stglBindFramebufferRead(myOvrMirrorFbo);
        myContext->arbFbo->glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, myOvrMirrorTexture->OGL.TexId, 0);
        myContext->stglBindFramebufferRead(anFboReadBack);

        ovrSizei anEyeSizes[2] = {
            ovr_GetFovTextureSize(myVrHmd, ovrEye_Left,  anHmdDesc.DefaultEyeFov[ovrEye_Left],  1),
            ovr_GetFovTextureSize(myVrHmd, ovrEye_Right, anHmdDesc.DefaultEyeFov[ovrEye_Right], 1)
        };
        myVrRendSizeX = stMax(anEyeSizes[0].w, anEyeSizes[1].w);
        myVrRendSizeY = stMax(anEyeSizes[0].h, anEyeSizes[1].h);
        anOvrRes = ovr_CreateSwapTextureSetGL(myVrHmd, GL_SRGB8_ALPHA8, myVrRendSizeX * 2, myVrRendSizeY, &myOvrSwapTexture);
        if(!OVR_SUCCESS(anOvrRes)
         || myOvrSwapTexture->TextureCount < 2) {
            myMsgQueue->pushError(stCString("StOutDistorted, Failed to create swap texture!"));
            myIsBroken = true;
            return true;
        }

        myOvrSwapTexture->CurrentIndex = 0;
        myContext->arbFbo->glGenFramebuffers(2, myOvrSwapFbo);
        myContext->stglBindFramebufferRead(myOvrSwapFbo[0]);
        myContext->arbFbo->glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                                                  ((ovrGLTexture* )&myOvrSwapTexture->Textures[0])->OGL.TexId, 0);
        myContext->stglBindFramebufferRead(myOvrSwapFbo[1]);
        myContext->arbFbo->glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                                                  ((ovrGLTexture* )&myOvrSwapTexture->Textures[1])->OGL.TexId, 0);
        myContext->stglBindFramebufferRead(anFboReadBack);
    }
#endif
    return true;
}

void StOutDistorted::processEvents() {
    StWindow::processEvents();

#ifdef ST_HAVE_OPENVR
    if(myVrHmd == NULL) {
        return;
    }

    // process OpenVR events
    for(vr::VREvent_t aVREvent; myVrHmd->PollNextEvent(&aVREvent, sizeof(aVREvent));) {
        switch(aVREvent.eventType) {
            case vr::VREvent_TrackedDeviceActivated: {
                // setupVrRenderModel(aVREvent.trackedDeviceIndex);
                ST_DEBUG_LOG("Device " + aVREvent.trackedDeviceIndex + " attached. Setting up render model")
                break;
            }
            case vr::VREvent_TrackedDeviceDeactivated: {
                ST_DEBUG_LOG("Device " + aVREvent.trackedDeviceIndex + " detached")
                break;
            }
            case vr::VREvent_TrackedDeviceUpdated: {
                ST_DEBUG_LOG("Device " + aVREvent.trackedDeviceIndex + " updated")
                break;
            }
        }
    }

    // process OpenVR controller state
    for(vr::TrackedDeviceIndex_t aDevIter = 0; aDevIter < vr::k_unMaxTrackedDeviceCount; ++aDevIter) {
        vr::VRControllerState_t aCtrlState;
        if(myVrHmd->GetControllerState(aDevIter, &aCtrlState, sizeof(aCtrlState))) {
            // aCtrlState.ulButtonPressed == 0;
        }
    }
#endif
}

void StOutDistorted::showCursor(const bool theToShow) {
    myToShowCursor = theToShow;
    if(myDevice == DEVICE_S3DV) {
        StWindow::showCursor(theToShow);
    }
}

void StOutDistorted::stglDrawCursor(const StPointD_t&  theCursorPos,
                                    const unsigned int theView) {
    if(myDevice == DEVICE_S3DV) {
        return;
    }

    StWindow::showCursor(false);
    if(!myToShowCursor
    || !myCursor->isValid()) {
        return;
    }

    const float aLensDisp = getLensDist() * 0.5f;
    double aGlLeft  = -1.0;
    double aGlTop   =  1.0;
    double aGlSizeX =  2.0;
    double aGlSizeY =  2.0;
    if(isHmdOutput()) {
        aGlLeft  = -1.0 + myVrMarginsLeft * 2.0;
        aGlTop   =  1.0 - myVrMarginsTop  * 2.0;
        aGlSizeX =  2.0 - (myVrMarginsLeft + myVrMarginsRight) * 2.0;
        aGlSizeY =  2.0 - (myVrMarginsTop + myVrMarginsBottom) * 2.0;
    }

    // compute cursor position
    StArray<StGLVec4> aVerts(4);
    const float aCurLeft = float(aGlLeft + theCursorPos.x() * aGlSizeX);
    const float aCurTop  = float(aGlTop  - theCursorPos.y() * aGlSizeY);
    if(isHmdOutput()) {
        if(aCurLeft < float(aGlLeft) || aCurLeft > float( 1.0 - myVrMarginsRight  * 2.0)
        || aCurTop  > float(aGlTop)  || aCurTop  < float(-1.0 + myVrMarginsBottom * 2.0)) {
            return;
        }
    }

    const int aVPSizeX = myContext->stglViewport().width();
    const int aVPSizeY = myContext->stglViewport().height();

    float aCurWidth  = 2.0f * float(myCursor->getSizeX()) / float(aVPSizeX);
    float aCurHeight = 2.0f * float(myCursor->getSizeY()) / float(aVPSizeY);
    if(myDevice != DEVICE_HMD) {
        switch(getPairLayout()) {
            case LAYOUT_SIDE_BY_SIDE_ANAMORPH:
                aCurWidth  *= 0.5;
                break;
            case LAYOUT_OVER_UNDER_ANAMORPH:
                aCurHeight *= 0.5;
                break;
            case LAYOUT_SIDE_BY_SIDE:
            case LAYOUT_OVER_UNDER:
                break;
        }
    }
    if(theView == ST_DRAW_LEFT) {
        aVerts[0] = StGLVec4( 2.0f * aLensDisp + aCurLeft + aCurWidth, aCurTop - aCurHeight, 0.0f, 1.0f);
        aVerts[1] = StGLVec4( 2.0f * aLensDisp + aCurLeft + aCurWidth, aCurTop,              0.0f, 1.0f);
        aVerts[2] = StGLVec4( 2.0f * aLensDisp + aCurLeft,             aCurTop - aCurHeight, 0.0f, 1.0f);
        aVerts[3] = StGLVec4( 2.0f * aLensDisp + aCurLeft,             aCurTop,              0.0f, 1.0f);
    } else {
        aVerts[0] = StGLVec4(-2.0f * aLensDisp + aCurLeft + aCurWidth, aCurTop - aCurHeight, 0.0f, 1.0f);
        aVerts[1] = StGLVec4(-2.0f * aLensDisp + aCurLeft + aCurWidth, aCurTop,              0.0f, 1.0f);
        aVerts[2] = StGLVec4(-2.0f * aLensDisp + aCurLeft,             aCurTop - aCurHeight, 0.0f, 1.0f);
        aVerts[3] = StGLVec4(-2.0f * aLensDisp + aCurLeft,             aCurTop,              0.0f, 1.0f);
    }
    myCurVertsBuf.init(*myContext, aVerts);

    myContext->core20fwd->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    myContext->core20fwd->glEnable(GL_BLEND);

    myCursor->bind(*myContext);
    myProgramFlat->use(*myContext);
        myCurVertsBuf.bindVertexAttrib(*myContext, myProgramFlat->getVVertexLoc());
        myCurTCrdsBuf.bindVertexAttrib(*myContext, myProgramFlat->getVTexCoordLoc());

        myContext->core20fwd->glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        myCurTCrdsBuf.unBindVertexAttrib(*myContext, myProgramFlat->getVTexCoordLoc());
        myCurVertsBuf.unBindVertexAttrib(*myContext, myProgramFlat->getVVertexLoc());
    myProgramFlat->unuse(*myContext);
    myCursor->unbind(*myContext);

    myContext->core20fwd->glDisable(GL_BLEND);
}

bool StOutDistorted::hasOrientationSensor() const {
#if defined(ST_HAVE_OPENVR) || defined(ST_HAVE_LIBOVR)
    if(myVrHmd != NULL) {
        return true;
    }
#endif
    return StWindow::hasOrientationSensor();
}

bool StOutDistorted::toTrackOrientation() const {
#if defined(ST_HAVE_OPENVR) || defined(ST_HAVE_LIBOVR)
    if(myVrHmd != NULL) {
        return myVrTrackOrient;
    }
#endif
    return StWindow::toTrackOrientation();
}

void StOutDistorted::setTrackOrientation(const bool theToTrack) {
#if defined(ST_HAVE_OPENVR) || defined(ST_HAVE_LIBOVR)
    if(myVrHmd != NULL) {
        myVrTrackOrient = theToTrack;
        return;
    }
#endif

    myVrTrackOrient = false;
    StWindow::setTrackOrientation(theToTrack);
}

StQuaternion<double> StOutDistorted::getDeviceOrientation() const {
    if(myVrTrackOrient
    && !myIsBroken) {
    #if defined(ST_HAVE_OPENVR) || defined(ST_HAVE_LIBOVR)
        if(myVrHmd != NULL) {
            return myVrOrient;
        }
    #endif
    }
    return StWindow::getDeviceOrientation();
}

StMarginsI StOutDistorted::getMargins() const {
    if(isHmdOutput()) {
        const StGLBoxPx aViewPort = StWindow::stglViewport(ST_WIN_MASTER);
        const int aSizeX = aViewPort.width();
        const int aSizeY = aViewPort.height();
        StMarginsI aMargins;
        aMargins.left   = int(double(aSizeX) * myVrMarginsLeft);
        aMargins.right  = int(double(aSizeX) * myVrMarginsRight);
        aMargins.top    = int(double(aSizeY) * myVrMarginsTop);
        aMargins.bottom = int(double(aSizeY) * myVrMarginsBottom);
        return aMargins;
    }
    return StMarginsI();
}

GLfloat StOutDistorted::getLensDist() const {
    return isHmdOutput() ? 0.1453f : 0.0f;
}

GLfloat StOutDistorted::getScaleFactor() const {
    if(!isHmdOutput()) {
        return StWindow::getScaleFactor();
    }

    // within direct rendering mode HMD is not visible to the system and thus is not registered as StMonitor,
    // therefore we need to override scale factor here to avoid incorrect scaling
    return 0.8f;
}

void StOutDistorted::checkHdmiPack() {
    myIsHdmiPack = false;
    if(!StWindow::isFullScreen()
    || myDevice == DEVICE_HMD) {
        return;
    }

    const StRectI_t aRect = StWindow::getPlacement();
    if(aRect.width()  == 1920
    && aRect.height() == 2205) {
        myIsHdmiPack = true;
    } else if(aRect.width()  == 1280
           && aRect.height() == 1470) {
        myIsHdmiPack = true;
    }
}

void StOutDistorted::setFullScreen(const bool theFullScreen) {
    bool wasFullscreen = StWindow::isFullScreen();
    if(!theFullScreen) {
        StWindow::setForcedAspect(-1.0);
        myIsStereoOn = false;
    }
    StWindow::setFullScreen(theFullScreen);
    if(!wasFullscreen) {
        checkHdmiPack();
    }
}

void StOutDistorted::stglDrawVR() {
#ifdef ST_HAVE_OPENVR
    const StGLBoxPx  aVPBoth    = StWindow::stglViewport(ST_WIN_ALL);
    const StPointD_t aCursorPos = getMousePos();
    bool hasComposError = false;
    if(myVrHmd == NULL
    || myIsBroken) {
        return;
    }

    if(!myFrBuffer->initLazy(*myContext, GL_RGBA8, myVrRendSizeX, myVrRendSizeY, StWindow::hasDepthBuffer())) {
        myIsBroken = true;
        myMsgQueue->pushError(stCString("OpenVR output - critical error:\nFrame Buffer Object resize failed!"));
        vr::VR_Shutdown();
        myVrHmd = NULL;
        return;
    }

    // draw into virtual frame buffers (textures)
    myFrBuffer->setupViewPort(*myContext);       // we set TEXTURE sizes here
    {
        myFrBuffer->bindBuffer(*myContext);
        StWindow::signals.onRedraw(ST_DRAW_LEFT);
        stglDrawCursor(aCursorPos, ST_DRAW_LEFT);

        vr::Texture_t aVRTexture = { (void* )(size_t )myFrBuffer->getTextureColor()->getTextureId(),  vr::TextureType_OpenGL, vr::ColorSpace_Gamma };
        const vr::EVRCompositorError aVRError = vr::VRCompositor()->Submit(vr::Eye_Left, &aVRTexture);
        if(aVRError != vr::VRCompositorError_None) {
            //myMsgQueue->pushError(getVRCompositorError(aVRError));
            ST_ERROR_LOG(getVRCompositorError(aVRError));
            hasComposError = true;
        }
        myFrBuffer->unbindBuffer(*myContext);
    }
    {
        myFrBuffer->bindBuffer(*myContext);
        StWindow::signals.onRedraw(ST_DRAW_RIGHT);
        stglDrawCursor(aCursorPos, ST_DRAW_RIGHT);

        vr::Texture_t aVRTexture = { (void* )(size_t )myFrBuffer->getTextureColor()->getTextureId(),  vr::TextureType_OpenGL, vr::ColorSpace_Gamma };
        const vr::EVRCompositorError aVRError = vr::VRCompositor()->Submit(vr::Eye_Right, &aVRTexture);
        if(aVRError != vr::VRCompositorError_None) {
            //myMsgQueue->pushError(getVRCompositorError(aVRError));
            ST_ERROR_LOG(getVRCompositorError(aVRError));
            hasComposError = true;
        }
        myFrBuffer->unbindBuffer(*myContext);
    }
    glFinish();

    {
        const vr::EVRCompositorError aVRError = vr::VRCompositor()->WaitGetPoses(myVrTrackedPoses, vr::k_unMaxTrackedDeviceCount, NULL, 0 );
        if(aVRError != vr::VRCompositorError_None) {
            //myMsgQueue->pushError(getVRCompositorError(aVRError));
            ST_ERROR_LOG(getVRCompositorError(aVRError));
            hasComposError = true;
        }

	      if(myVrTrackedPoses[vr::k_unTrackedDeviceIndex_Hmd].bPoseIsValid) {
            const vr::HmdMatrix34_t& aHeadPos = myVrTrackedPoses[vr::k_unTrackedDeviceIndex_Hmd].mDeviceToAbsoluteTracking;
            double aRotMat[3][3];
            for(int aRow = 0; aRow < 3; ++aRow) {
                for(int aCol = 0; aCol < 3; ++aCol) {
                    aRotMat[aCol][aRow] = aHeadPos.m[aCol][aRow];
                }
            }
            myVrOrient.setMatrix(aRotMat);
	      }
    }

    // real screen buffer
    myContext->stglBindFramebuffer(StGLFrameBuffer::NO_FRAMEBUFFER);

    if(hasComposError) {
        myContext->stglResizeViewport(aVPBoth);
        myContext->stglResetScissorRect();
        myContext->core20fwd->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        myFPSControl.sleepToTarget(); // decrease FPS to target by thread sleeps
        StWindow::stglSwap(ST_WIN_ALL);
        ++myFPSControl;
    }
#elif defined(ST_HAVE_LIBOVR)
    const StGLBoxPx  aVPBoth    = StWindow::stglViewport(ST_WIN_ALL);
    const StPointD_t aCursorPos = getMousePos();
    if(myVrHmd == NULL
    || myIsBroken) {
        return;
    }

    ovrHmdDesc anHmdDesc = ovr_GetHmdDesc(myVrHmd);
    ovrEyeRenderDesc anEyeRenderDesc[2] = {
        ovr_GetRenderDesc(myVrHmd, ovrEye_Left,  anHmdDesc.DefaultEyeFov[0]),
        ovr_GetRenderDesc(myVrHmd, ovrEye_Right, anHmdDesc.DefaultEyeFov[1])
    };
    ovrViewScaleDesc aViewScaleDesc;
    aViewScaleDesc.HmdSpaceToWorldScaleInMeters = 1.0f;
    aViewScaleDesc.HmdToEyeViewOffset[0] = anEyeRenderDesc[0].HmdToEyeViewOffset;
    aViewScaleDesc.HmdToEyeViewOffset[1] = anEyeRenderDesc[1].HmdToEyeViewOffset;

    const StGLBoxPx aViewPortL = {{ 0, 0,
                                    myVrRendSizeX, myVrRendSizeY }};
    const StGLBoxPx aViewPortR = {{ myVrRendSizeX, 0,
                                    myVrRendSizeX, myVrRendSizeY }};

    ovrLayerEyeFov aLayerFov;
    aLayerFov.Header.Type       = ovrLayerType_EyeFov;
    aLayerFov.Header.Flags      = ovrLayerFlag_TextureOriginAtBottomLeft;
    aLayerFov.ColorTexture[0]   = myOvrSwapTexture;
    aLayerFov.ColorTexture[1]   = NULL;
    aLayerFov.Viewport[0].Pos.x = aViewPortL.x();
    aLayerFov.Viewport[0].Pos.y = aViewPortL.y();
    aLayerFov.Viewport[0].Size.w= aViewPortL.width();
    aLayerFov.Viewport[0].Size.h= aViewPortL.height();
    aLayerFov.Viewport[1].Pos.x = aViewPortR.x();
    aLayerFov.Viewport[1].Pos.y = aViewPortR.y();
    aLayerFov.Viewport[1].Size.w= aViewPortR.width();
    aLayerFov.Viewport[1].Size.h= aViewPortR.height();
    aLayerFov.Fov[0]            = anHmdDesc.DefaultEyeFov[0];
    aLayerFov.Fov[1]            = anHmdDesc.DefaultEyeFov[1];
    aLayerFov.SensorSampleTime  = ovr_GetTimeInSeconds();
    const double aPredictedTime = ovr_GetPredictedDisplayTime(myVrHmd, 0);
    ovrTrackingState anHmdState = ovr_GetTrackingState(myVrHmd, aPredictedTime, ovrTrue);
    ovr_CalcEyePoses(anHmdState.HeadPose.ThePose, aViewScaleDesc.HmdToEyeViewOffset, aLayerFov.RenderPose);
    myVrOrient = StQuaternion<double>((double )aLayerFov.RenderPose[0].Orientation.x,
                                      (double )aLayerFov.RenderPose[0].Orientation.y,
                                      (double )aLayerFov.RenderPose[0].Orientation.z,
                                      (double )aLayerFov.RenderPose[0].Orientation.w);

    // draw Left View into virtual frame buffer
    myContext->stglResizeViewport(aViewPortL);
    myContext->stglSetScissorRect(aViewPortL, false);
    myContext->stglBindFramebuffer(myOvrSwapFbo[myOvrSwapTexture->CurrentIndex]);
    StWindow::signals.onRedraw(ST_DRAW_LEFT);
    stglDrawCursor(aCursorPos, ST_DRAW_LEFT);

    // draw Right View into virtual frame buffer
    myContext->stglResizeViewport(aViewPortR);
    myContext->stglSetScissorRect(aViewPortR, false);
    StWindow::signals.onRedraw(ST_DRAW_RIGHT);
    stglDrawCursor(aCursorPos, ST_DRAW_RIGHT);
    myContext->stglBindFramebuffer(StGLFrameBuffer::NO_FRAMEBUFFER);

    ovrLayerHeader* aLayers = &aLayerFov.Header;
    ovrResult anOvrRes = ovr_SubmitFrame(myVrHmd, 0, &aViewScaleDesc, &aLayers, 1);
    myOvrSwapTexture->CurrentIndex = myOvrSwapTexture->CurrentIndex == 0 ? 1 : 0;
    if(!OVR_SUCCESS(anOvrRes)) {
        myMsgQueue->pushError(stCString("StOutDistorted, Failed to submit swap texture!"));
        myIsBroken = true;
    }

    myContext->stglResizeViewport(aVPBoth);
    myContext->stglResetScissorRect();
    myContext->core20fwd->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    myContext->stglBindFramebufferRead(myOvrMirrorFbo);
    myContext->stglBindFramebufferDraw(StGLFrameBuffer::NO_FRAMEBUFFER);
    GLint aSrcSizeX = myOvrMirrorTexture->OGL.Header.TextureSize.w;
    GLint aSrcSizeY = myOvrMirrorTexture->OGL.Header.TextureSize.h;
    myContext->arbFbo->glBlitFramebuffer(0, aSrcSizeY, aSrcSizeX, 0,
                                         0, 0, aVPBoth.width(), aVPBoth.height(),
                                         GL_COLOR_BUFFER_BIT, GL_NEAREST);
    myContext->stglBindFramebufferRead(StGLFrameBuffer::NO_FRAMEBUFFER);

    myFPSControl.sleepToTarget(); // decrease FPS to target by thread sleeps
    StWindow::stglSwap(ST_WIN_ALL);
    ++myFPSControl;
#endif
}

void StOutDistorted::stglDraw() {
    myFPSControl.setTargetFPS(StWindow::getTargetFps());

    const bool isStereoSource = (StWindow::isStereoSource() || params.MonoClone->getValue());
    myIsStereoOn = isStereoSource
                && StWindow::isFullScreen()
                && !myIsBroken;

    myIsForcedStereo = myIsStereoOn && params.MonoClone->getValue();

    double aForcedAspect = -1.0;
    if(isHmdOutput()) {
        if(myVrRendSizeX != 0 && myVrRendSizeY != 0) {
            aForcedAspect = double(myVrRendSizeX) / double(myVrRendSizeY);
        }
    }

    StWinSplit aWinSplit = StWinSlave_splitOff;
    const Layout aPairLayout = getPairLayout();
    if(myIsStereoOn
    && (myDevice == DEVICE_HMD
     || aPairLayout == LAYOUT_SIDE_BY_SIDE
     || aPairLayout == LAYOUT_OVER_UNDER)) {
        aWinSplit = aPairLayout == LAYOUT_OVER_UNDER && myDevice != DEVICE_HMD
                  ? StWinSlave_splitVertical
                  : StWinSlave_splitHorizontal;
        if(myDevice != DEVICE_HMD
        && aPairLayout == LAYOUT_OVER_UNDER
        && myIsHdmiPack) {
            // detect special HDMI 3D modes
            const StRectI_t aRect = StWindow::getPlacement();
            if(aRect.width() == 1920) {
                aWinSplit = StWinSlave_splitVertHdmi1080;
            } else if(aRect.width() == 1280) {
                aWinSplit = StWinSlave_splitVertHdmi720;
            }
        }
    }
    StWindow::setForcedAspect(aForcedAspect);
    StWindow::setAttribute(StWinAttr_SplitCfg, aWinSplit);
    if(myDevice == DEVICE_S3DV) {
        StWindow::setHardwareStereoOn(myIsStereoOn);
    }

    if(!StWindow::stglMakeCurrent(ST_WIN_MASTER)) {
        StWindow::signals.onRedraw(ST_DRAW_MONO);
        StThread::sleep(10);
        return;
    }

    const StGLBoxPx aVPMaster = StWindow::stglViewport(ST_WIN_MASTER);
    if(!myIsStereoOn) {
        if(myToCompressMem) {
            myFrBuffer->release(*myContext);
        }

        myContext->stglResizeViewport(aVPMaster);
        myContext->stglSetScissorRect(aVPMaster, false);
        StWindow::showCursor(myToShowCursor);
        StWindow::signals.onRedraw(ST_DRAW_LEFT);
        myContext->stglResetScissorRect();

        myFPSControl.sleepToTarget(); // decrease FPS to target by thread sleeps
        StWindow::stglSwap(ST_WIN_ALL);
        ++myFPSControl;
        return;
    }

#if defined(ST_HAVE_OPENVR) || defined(ST_HAVE_LIBOVR)
    if(myVrHmd != NULL
    && !myIsBroken) {
        stglDrawVR();
        return;
    }
#endif

    const StGLBoxPx  aVPSlave   = StWindow::stglViewport(ST_WIN_SLAVE);
    const StGLBoxPx  aVPBoth    = StWindow::stglViewport(ST_WIN_ALL);
    const StPointD_t aCursorPos = getMousePos();
    StGLBoxPx aViewPortL = aVPMaster;
    StGLBoxPx aViewPortR = aVPSlave;
    if(myDevice != DEVICE_HMD) {
        switch(aPairLayout) {
            case LAYOUT_OVER_UNDER_ANAMORPH: {
                aViewPortR.height() /= 2;
                aViewPortL.height() = aViewPortR.height();
                aViewPortL.y() += aViewPortR.height();
                break;
            }
            case LAYOUT_SIDE_BY_SIDE_ANAMORPH: {
                aViewPortL.width() /= 2;
                aViewPortR.width() = aViewPortL.width();
                aViewPortR.x() += aViewPortL.width();
                break;
            }
            case LAYOUT_SIDE_BY_SIDE:
            case LAYOUT_OVER_UNDER: {
                break;
            }
        }
    }

    // simple rendering without FBO
    if(myDevice != DEVICE_HMD
    && !myIsForcedFboUsage) {
        myContext->stglResizeViewport(aVPBoth);
        myContext->stglSetScissorRect(aVPBoth, false);
        myContext->core20fwd->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        myContext->stglResizeViewport(aViewPortL);
        myContext->stglSetScissorRect(aViewPortL, false);
        StWindow::signals.onRedraw(ST_DRAW_LEFT);
        stglDrawCursor(aCursorPos, ST_DRAW_LEFT);
        myContext->stglResetScissorRect();

        myContext->stglResizeViewport(aViewPortR);
        myContext->stglSetScissorRect(aViewPortR, false);
        StWindow::signals.onRedraw(ST_DRAW_RIGHT);
        stglDrawCursor(aCursorPos, ST_DRAW_RIGHT);
        myContext->stglResetScissorRect();

        myFPSControl.sleepToTarget();
        StWindow::stglSwap(ST_WIN_ALL);
        ++myFPSControl;
        return;
    }

    // resize FBO
    GLint aFrSizeX = aViewPortL.width();
    GLint aFrSizeY = aViewPortL.height();
    if(myDevice == DEVICE_HMD) {
        aFrSizeX = int(std::ceil(double(aFrSizeX) * 1.25) + 0.5);
        aFrSizeY = int(std::ceil(double(aFrSizeY) * 1.25) + 0.5);
    }

    if(!myFrBuffer->initLazy(*myContext, GL_RGBA8, aFrSizeX, aFrSizeY, StWindow::hasDepthBuffer())) {
        myMsgQueue->pushError(stCString("Distorted output - critical error:\nFrame Buffer Object resize failed!"));
        myIsBroken = true;
        return;
    }

    // reduce viewport to avoid additional aliasing of narrow lines
    const GLfloat aDX = GLfloat(myFrBuffer->getVPSizeX()) / GLfloat(myFrBuffer->getSizeX());
    const GLfloat aDY = GLfloat(myFrBuffer->getVPSizeY()) / GLfloat(myFrBuffer->getSizeY());
    StArray<StGLVec2> aTCoords(4);
    aTCoords[0] = StGLVec2(aDX,  0.0f);
    aTCoords[1] = StGLVec2(aDX,  aDY);
    aTCoords[2] = StGLVec2(0.0f, 0.0f);
    aTCoords[3] = StGLVec2(0.0f, aDY);
    myFrTCrdsBuf.init(*myContext, aTCoords);

    const GLfloat aLensDisp = getLensDist() * 0.5f;

    // draw Left View into virtual frame buffer
    myFrBuffer->setupViewPort(*myContext); // we set TEXTURE sizes here
    myFrBuffer->bindBuffer(*myContext);
        StWindow::signals.onRedraw(ST_DRAW_LEFT);
        stglDrawCursor(aCursorPos, ST_DRAW_LEFT);
    myFrBuffer->unbindBuffer(*myContext);

    // now draw to real screen buffer
    // clear the screen and the depth buffer
    myContext->stglResizeViewport(aVPBoth);
    myContext->stglSetScissorRect(aVPBoth, false);
    myContext->core20fwd->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // draw Left view
    myContext->stglResizeViewport(aViewPortL);
    myContext->stglSetScissorRect(aViewPortL, false);

    StGLProgram*    aProgram   = myProgramFlat.access();
    StGLVarLocation aVertexLoc = myProgramFlat->getVVertexLoc();
    StGLVarLocation aTexCrdLoc = myProgramFlat->getVTexCoordLoc();
    if(myDevice == DEVICE_HMD) {
        aProgram   = myProgramBarrel.access();
        aVertexLoc = myProgramBarrel->getVVertexLoc();
        aTexCrdLoc = myProgramBarrel->getVTexCoordLoc();
        myProgramBarrel->setScaleIn(*myContext, StGLVec2(2.0f / aDX, 2.0f / aDY));
        myProgramBarrel->setScale  (*myContext, StGLVec2(0.4f * aDX, 0.4f * aDY));
    }

    myFrBuffer->bindTexture(*myContext);
    if(aProgram == myProgramBarrel.access()) {
        myProgramBarrel->setLensCenter(*myContext, StGLVec2((0.5f + aLensDisp) * aDX, 0.5f * aDY));
    }
    aProgram->use(*myContext);
        myFrVertsBuf.bindVertexAttrib(*myContext, aVertexLoc);
        myFrTCrdsBuf.bindVertexAttrib(*myContext, aTexCrdLoc);

        myContext->core20fwd->glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        myFrTCrdsBuf.unBindVertexAttrib(*myContext, aTexCrdLoc);
        myFrVertsBuf.unBindVertexAttrib(*myContext, aVertexLoc);
    aProgram->unuse(*myContext);
    myFrBuffer->unbindTexture(*myContext);
    myContext->stglResetScissorRect();

    myFrBuffer->setupViewPort(*myContext); // we set TEXTURE sizes here
    myFrBuffer->bindBuffer(*myContext);
        StWindow::signals.onRedraw(ST_DRAW_RIGHT);
        stglDrawCursor(aCursorPos, ST_DRAW_RIGHT);
    myFrBuffer->unbindBuffer(*myContext);

    // draw Right view
    myContext->stglResizeViewport(aViewPortR);
    myContext->stglSetScissorRect(aViewPortR, false);

    myFrBuffer->bindTexture(*myContext);
    if(aProgram == myProgramBarrel.access()) {
        myProgramBarrel->setLensCenter(*myContext, StGLVec2((0.5f - aLensDisp) * aDX, 0.5f * aDY));
    }
    aProgram->use(*myContext);
    myFrVertsBuf.bindVertexAttrib(*myContext, aVertexLoc);
    myFrTCrdsBuf.bindVertexAttrib(*myContext, aTexCrdLoc);

    myContext->core20fwd->glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    myFrTCrdsBuf.unBindVertexAttrib(*myContext, aTexCrdLoc);
    myFrVertsBuf.unBindVertexAttrib(*myContext, aVertexLoc);

    aProgram->unuse(*myContext);
    myFrBuffer->unbindTexture(*myContext);
    myContext->stglResetScissorRect();

    myFPSControl.sleepToTarget(); // decrease FPS to target by thread sleeps
    StWindow::stglSwap(ST_WIN_ALL);
    ++myFPSControl;
}

void StOutDistorted::doSwitchVSync(const int32_t theValue) {
    if(myContext.isNull()) {
        return;
    }

    StWindow::stglMakeCurrent(ST_WIN_MASTER);
    myContext->stglSetVSync((StGLContext::VSync_Mode )theValue);
}
