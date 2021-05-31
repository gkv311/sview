/**
 * StOutDistorted, class providing stereoscopic output in anamorph side by side format using StCore toolkit.
 * Copyright © 2013-2020 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
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
#include <StGLMesh/StGLTextureQuad.h>
#include <StGLStereo/StGLProjCamera.h>
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
    if(myDevice != DEVICE_HMD) {
        theList.add(params.MonoClone);
    }
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
            + myAboutDescr.format("2013-2020", "kirill@sview.ru", "www.sview.ru");
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
  myVrFrequency(0),
  myVrAspectRatio(1.0f),
  myVrFieldOfView(-1.0f),
  myVrIOD(0.0f),
  myVrTrackOrient(false),
  myVrToDrawMsg(false),
#ifdef ST_HAVE_OPENVR
  myVrHmd(NULL),
  myVrTrackedPoses(new vr::TrackedDevicePose_t[vr::k_unMaxTrackedDeviceCount]),
#endif
  myToShowCursor(true),
  myToCompressMem(myInstancesNb.increment() > 1),
  myIsBroken(false),
  myIsStereoOn(false),
  myCanHdmiPack(false),
  myIsHdmiPack(false),
  myIsForcedFboUsage(false) {
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
        if(!myVrFullscreenMsg.isNull()) {
            myVrFullscreenMsg->release(*myContext);
            myVrFullscreenMsg.nullify();
        }
    #ifdef ST_HAVE_OPENVR
        if(myVrHmd != NULL) {
            vr::VR_Shutdown();
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
    myVrFrequency = 0;
    myVrFieldOfView = -1.0f;
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
    {
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
        myVrFrequency = myVrHmd->GetFloatTrackedDeviceProperty(vr::k_unTrackedDeviceIndex_Hmd, vr::Prop_DisplayFrequency_Float);
        updateVRProjectionFrustums();

        uint32_t aRenderSizeX = 0, aRenderSizeY = 0;
        myVrHmd->GetRecommendedRenderTargetSize(&aRenderSizeX, &aRenderSizeY);
        myAboutVrDevice = aVrManuf + " " + aVrDriver + "\n"
                        + aVrDisplay + " [" + aRenderSizeX + "x" + aRenderSizeY + "@" + myVrFrequency + "]";
        myVrRendSize.x() = int(aRenderSizeX);
        myVrRendSize.y() = int(aRenderSizeY);
        updateAbout();
    }
#endif

    if(myDevice == DEVICE_HMD) {
        StAVImage anImage;
        StHandle<StResource> aWarnRes = getResourceManager()->getResource(StString("textures") + SYS_FS_SPLITTER + "hmd_exit_fullscreen.png");
        uint8_t* aData     = NULL;
        int      aDataSize = 0;
        if(!aWarnRes.isNull()
        && !aWarnRes->isFile()
        &&  aWarnRes->read()) {
            aData     = (uint8_t* )aWarnRes->getData();
            aDataSize = aWarnRes->getSize();
        }
        if(anImage.load(!aWarnRes.isNull() ? aWarnRes->getPath() : StString(), StImageFile::ST_TYPE_PNG, aData, aDataSize)) {
            myVrFullscreenMsg = new StGLTextureQuad();
            if(!myVrFullscreenMsg->init(*myContext, anImage.getPlane())) {
                ST_ERROR_LOG(ST_OUT_PLUGIN_NAME + " Plugin, Texture can not be initialized!");
                myVrFullscreenMsg->release(*myContext);
                myVrFullscreenMsg.nullify();
            }
        } else {
            ST_ERROR_LOG(ST_OUT_PLUGIN_NAME + " Plugin, Texture missed: " + anImage.getState());
        }
    }

    return true;
}

void StOutDistorted::updateVRProjectionFrustums() {
#ifdef ST_HAVE_OPENVR
    StRect<float> aFrustL, aFrustR;
    myVrHmd->GetProjectionRaw(vr::Eye_Left,  &aFrustL.left(), &aFrustL.right(), &aFrustL.top(), &aFrustL.bottom());
    myVrHmd->GetProjectionRaw(vr::Eye_Right, &aFrustR.left(), &aFrustR.right(), &aFrustR.top(), &aFrustR.bottom());
    myVrFrustumL = aFrustL;
    myVrFrustumR = aFrustR;
    std::swap(myVrFrustumL.top(), myVrFrustumL.bottom());
    std::swap(myVrFrustumR.top(), myVrFrustumR.bottom());

    const StVec2<double> aTanHalfFov(StVec4<float>(-aFrustL.left(), aFrustL.right(),  -aFrustR.left(), aFrustR.right()).maxComp(),
                                     StVec4<float>(-aFrustL.top(),  aFrustL.bottom(), -aFrustR.top(),  aFrustR.bottom()).maxComp());
    myVrAspectRatio = float(aTanHalfFov.x() / aTanHalfFov.y());
    myVrFieldOfView = float(2.0 * std::atan(aTanHalfFov.y()) * 180.0 / M_PI);

    // Intra-ocular Distance can be changed in runtime
    //const vr::HmdMatrix34_t aLeftToHead  = myContext->System->GetEyeToHeadTransform (vr::Eye_Left);
    //const vr::HmdMatrix34_t aRightToHead = myContext->System->GetEyeToHeadTransform (vr::Eye_Right);
    //myVrIOD = aRightToHead.m[0][3] - aLeftToHead.m[0][3];
    myVrIOD = myVrHmd->GetFloatTrackedDeviceProperty(vr::k_unTrackedDeviceIndex_Hmd, vr::Prop_UserIpdMeters_Float);
#else
  (void )myVrIOD;
#endif
}

float StOutDistorted::getMaximumTargetFps() const {
    return myVrFrequency > 24
         ? myVrFrequency
         : StWindow::getMaximumTargetFps();
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

    double aGlLeft  = -1.0, aGlTop   = 1.0;
    double aGlSizeX =  2.0, aGlSizeY = 2.0;
    float aLensDisp = theView == ST_DRAW_LEFT ? getLensDist() : -getLensDist();
    if(isHmdOutput()) {
        aGlLeft  = -1.0 + myVrMarginsLeft * 2.0;
        aGlTop   =  1.0 - myVrMarginsTop  * 2.0;
        aGlSizeX =  2.0 - (myVrMarginsLeft + myVrMarginsRight) * 2.0;
        aGlSizeY =  2.0 - (myVrMarginsTop + myVrMarginsBottom) * 2.0;

        StGLProjCamera aProjCam;
        aProjCam.setPerspective(true);
        //aProjCam.setFOVy(myVrFieldOfView);
        aProjCam.resize(myVrAspectRatio);
        aProjCam.setCustomProjection(myVrFrustumL, myVrFrustumR);
        aProjCam.setView(theView);

        const StGLVec4 aTestProj = aProjCam.getProjMatrix() * StGLVec4(0, 0, aProjCam.getZScreen(), 1.0f);
        aLensDisp = aTestProj.x() / aTestProj.w();
    }

    // compute cursor position
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

    StArray<StGLVec4> aVerts(4);
    aVerts[0] = StGLVec4(aLensDisp + aCurLeft + aCurWidth, aCurTop - aCurHeight, 0.0f, 1.0f);
    aVerts[1] = StGLVec4(aLensDisp + aCurLeft + aCurWidth, aCurTop,              0.0f, 1.0f);
    aVerts[2] = StGLVec4(aLensDisp + aCurLeft,             aCurTop - aCurHeight, 0.0f, 1.0f);
    aVerts[3] = StGLVec4(aLensDisp + aCurLeft,             aCurTop,              0.0f, 1.0f);
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
#if defined(ST_HAVE_OPENVR)
    if(myVrHmd != NULL) {
        return true;
    }
#endif
    return StWindow::hasOrientationSensor();
}

bool StOutDistorted::toTrackOrientation() const {
#if defined(ST_HAVE_OPENVR)
    if(myVrHmd != NULL) {
        return myVrTrackOrient;
    }
#endif
    return StWindow::toTrackOrientation();
}

void StOutDistorted::setTrackOrientation(const bool theToTrack) {
#if defined(ST_HAVE_OPENVR)
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
    #if defined(ST_HAVE_OPENVR)
        if(myVrHmd != NULL) {
            return myVrOrient;
        }
    #endif
    }
    return StWindow::getDeviceOrientation();
}

bool StOutDistorted::getCustomProjection(StRectF_t& theLeft, StRectF_t& theRight) const {
    if(!isHmdOutput()) {
        return false;
    }

    theLeft  = myVrFrustumL;
    theRight = myVrFrustumR;
    return true;
}

StGLBoxPx StOutDistorted::stglViewport(const int theWinEnum) const {
    if(!isHmdOutput()) {
        return StWindow::stglViewport(theWinEnum);
    }

    StGLBoxPx aBox;
    aBox.x() = aBox.y() = 0;
    aBox.width()  = myVrRendSize.x();
    aBox.height() = myVrRendSize.y();
    return aBox;
}

StMarginsI StOutDistorted::getMargins() const {
    if(isHmdOutput()) {
        StMarginsI aMargins;
        aMargins.left   = int(double(myVrRendSize.x()) * myVrMarginsLeft);
        aMargins.right  = int(double(myVrRendSize.x()) * myVrMarginsRight);
        aMargins.top    = int(double(myVrRendSize.y()) * myVrMarginsTop);
        aMargins.bottom = int(double(myVrRendSize.y()) * myVrMarginsBottom);
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

    // TODO this is calibrated for HTC Vive; applying to HMDs with another FOV might give bad results
    return float(myVrRendSize.x()) / 1280.0f;
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

bool StOutDistorted::isStereoFullscreenOnly() const {
    return StWindow::hasFullscreenMode();
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

    // force resizing instead of lazy resize as we do not pass texture bounds
    const vr::VRTextureBounds_t* aTexBounds = NULL;
    if(!myFrBuffer->isValid()
     || myFrBuffer->getSizeX() != myVrRendSize.x()
     || myFrBuffer->getSizeY() != myVrRendSize.y()) {
        if(!myFrBuffer->init(*myContext, GL_RGBA8, myVrRendSize.x(), myVrRendSize.y(), StWindow::hasDepthBuffer())) {
            myIsBroken = true;
            myMsgQueue->pushError(stCString("OpenVR output - critical error:\nFrame Buffer Object resize failed!"));
            vr::VR_Shutdown();
            myVrHmd = NULL;
            return;
        }
    }

    // draw into virtual frame buffers (textures)
    myFrBuffer->setupViewPort(*myContext);       // we set TEXTURE sizes here
    {
        myFrBuffer->bindBuffer(*myContext);
        StWindow::signals.onRedraw(ST_DRAW_LEFT);
        stglDrawCursor(aCursorPos, ST_DRAW_LEFT);

        vr::Texture_t aVRTexture = { (void* )(size_t )myFrBuffer->getTextureColor()->getTextureId(),  vr::TextureType_OpenGL, vr::ColorSpace_Gamma };
        const vr::EVRCompositorError aVRError = vr::VRCompositor()->Submit(vr::Eye_Left, &aVRTexture, aTexBounds);
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
            updateVRProjectionFrustums();
        }
    }

    // real screen buffer
    myContext->stglBindFramebuffer(StGLFrameBuffer::NO_FRAMEBUFFER);

    if(hasComposError || myVrToDrawMsg || myVrMsgTimer.getElapsedTimeInSec() > 2.0) {
        myContext->stglResizeViewport(aVPBoth);
        myContext->stglResetScissorRect();
        myContext->core20fwd->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        if(!myVrFullscreenMsg.isNull()) {
            myVrFullscreenMsg->stglDraw(*myContext);
        }
        StWindow::stglSwap(ST_WIN_ALL);
        if(myVrToDrawMsg) {
            myVrToDrawMsg = false;
            myVrMsgTimer.restart();
        } else if(!hasComposError) {
            myVrMsgTimer.stop();
        }
    }
#endif
}

void StOutDistorted::stglDraw() {
    myFPSControl.setTargetFPS(StWindow::getTargetFps());

    const bool isStereoSource = StWindow::isStereoSource()
                             || myDevice == DEVICE_HMD
                             || params.MonoClone->getValue();
    myIsStereoOn = isStereoSource
                && StWindow::isFullScreen()
                && !myIsBroken;

    myIsForcedStereo = myIsStereoOn && (myDevice == DEVICE_HMD || params.MonoClone->getValue());

    double aForcedAspect = -1.0;
    if(isHmdOutput()) {
        //if(myVrRendSize.x() != 0 && myVrRendSize.y() != 0) { aForcedAspect = double(myVrRendSize.x()) / double(myVrRendSize.y()); }
        aForcedAspect = myVrAspectRatio;
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
        myVrToDrawMsg = true;
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

#if defined(ST_HAVE_OPENVR)
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
