/**
 * StOutDistorted, class providing stereoscopic output in anamorph side by side format using StCore toolkit.
 * Copyright © 2013-2016 Kirill Gavrilov <kirill@sview.ru>
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

#ifdef ST_HAVE_LIBOVR

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
    static const char ST_SETTING_LAYOUT[]    = "layout";
    static const char ST_SETTING_MONOCLONE[] = "monoClone";
    static const char ST_SETTING_MARGINS[]   = "margins";
    static const char ST_SETTING_WARP_COEF[] = "warpCoef";
    static const char ST_SETTING_CHROME_AB[] = "chromeAb";

    // translation resources
    enum {
        STTR_DISTORTED_NAME     = 1000,
        STTR_DISTORTED_DESC     = 1001,
        STTR_OCULUS_NAME        = 1002,
        STTR_OCULUS_DESC        = 1003,
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
        case DEVICE_OCULUS:    return "Oculus";
        case DEVICE_S3DV:      return "S3DV";
        case DEVICE_DISTORTED:
        default:               return "Distorted";
    }
}

bool StOutDistorted::isLostDevice() const {
    return myToResetDevice || StWindow::isLostDevice();
}

bool StOutDistorted::setDevice(const StString& theDevice) {
    if(theDevice == "Oculus") {
        if(myDevice != DEVICE_OCULUS) {
            myToResetDevice = true;
        }
        myDevice = DEVICE_OCULUS;
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
    if(myDevice != DEVICE_DISTORTED
    && myDevice != DEVICE_S3DV) {
        theList.add(params.Layout);
    }
    theList.add(params.MonoClone);
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
  myOvrHmd(NULL),
  myOvrSizeX(0),
  myOvrSizeY(0),
#ifdef ST_HAVE_LIBOVR
  myOvrSwapTexture(NULL),
  myOvrMirrorTexture(NULL),
  myOvrMirrorFbo(0),
#endif
  myToReduceGui(false),
  myToShowCursor(true),
  myToCompressMem(myInstancesNb.increment() > 1),
  myIsBroken(false),
  myIsStereoOn(false),
  myIsHdmiPack(false),
  myIsForcedFboUsage(false) {
#ifdef ST_HAVE_LIBOVR
    myOvrSwapFbo[0] = 0;
    myOvrSwapFbo[1] = 0;
#endif
    const StSearchMonitors& aMonitors = StWindow::getMonitors();
    StTranslations aLangMap(getResourceManager(), ST_OUT_PLUGIN_NAME);

    // about string
    StString& aTitle     = aLangMap.changeValueId(STTR_PLUGIN_TITLE,   "sView - Distorted Output module");
    StString& aVerString = aLangMap.changeValueId(STTR_VERSION_STRING, "version");
    StString& aDescr     = aLangMap.changeValueId(STTR_PLUGIN_DESCRIPTION,
        "(C) {0} Kirill Gavrilov <{1}>\nOfficial site: {2}\n\nThis library is distributed under LGPL3.0");
    myAbout = aTitle + '\n' + aVerString + " " + StVersionInfo::getSDKVersionString() + "\n \n"
            + aDescr.format("2013-2016", "kirill@sview.ru", "www.sview.ru");

    // detect connected displays
    int aSupportOculus   = ST_DEVICE_SUPPORT_NONE;
    int aSupportParallel = ST_DEVICE_SUPPORT_NONE;
    int aSupportS3DV     = ST_DEVICE_SUPPORT_NONE;
    bool isHdmiPack = false;
    for(size_t aMonIter = 0; aMonIter < aMonitors.size(); ++aMonIter) {
        const StMonitor& aMon = aMonitors[aMonIter];
        if(aMon.getPnPId().isStartsWith(stCString("OVR"))) {
            // Oculus Rift
            aSupportOculus = ST_DEVICE_SUPPORT_HIGHT;
            break;
        } else if(aMon.getPnPId().isEquals(stCString("ST@S3DV"))) {
            aSupportS3DV = ST_DEVICE_SUPPORT_PREFER;
            break;
        } else if(aMon.getVRect().width()  == 1920
               && aMon.getVRect().height() == 2205) {
            aSupportParallel = ST_DEVICE_SUPPORT_HIGHT;
            isHdmiPack = true;
        } else if(aMon.getVRect().width()  == 1280
               && aMon.getVRect().height() == 1470) {
            aSupportParallel = ST_DEVICE_SUPPORT_HIGHT;
            isHdmiPack = true;
        }
    }

#ifdef ST_HAVE_LIBOVR
    const ovrResult anOvrRes = ovr_Initialize(NULL);
    if(!OVR_SUCCESS(anOvrRes)) {
        ST_ERROR_LOG("StOutDistorted, OVR initialization has failed!");
    } else {
        aSupportOculus = ST_DEVICE_SUPPORT_HIGHT;
    }
#endif

    // devices list
    StHandle<StOutDevice> aDevDistorted = new StOutDevice();
    aDevDistorted->PluginId = ST_OUT_PLUGIN_NAME;
    aDevDistorted->DeviceId = "Distorted";
    aDevDistorted->Priority = aSupportParallel;
    aDevDistorted->Name     = aLangMap.changeValueId(STTR_DISTORTED_NAME, "TV (parallel pair)");
    aDevDistorted->Desc     = aLangMap.changeValueId(STTR_DISTORTED_DESC, "Distorted Output");
    myDevices.add(aDevDistorted);

    StHandle<StOutDevice> aDevOculus = new StOutDevice();
    aDevOculus->PluginId = ST_OUT_PLUGIN_NAME;
    aDevOculus->DeviceId = "Oculus";
    aDevOculus->Priority = aSupportOculus;
    aDevOculus->Name     = aLangMap.changeValueId(STTR_OCULUS_NAME, "Oculus Rift");
    aDevOculus->Desc     = aLangMap.changeValueId(STTR_OCULUS_DESC, "Distorted Output");
    myDevices.add(aDevOculus);

    if(aSupportS3DV != ST_DEVICE_SUPPORT_NONE) {
        StHandle<StOutDevice> aDevS3dv = new StOutDevice();
        aDevS3dv->PluginId = ST_OUT_PLUGIN_NAME;
        aDevS3dv->DeviceId = "S3DV";
        aDevS3dv->Priority = aSupportS3DV;
        aDevS3dv->Name     = "S3DV";             //aLangMap.changeValueId(STTR_S3DV_NAME, "S3DV");
        aDevS3dv->Desc     = "Distorted Output"; //aLangMap.changeValueId(STTR_S3DV_DESC, "Distorted Output");
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
    params.MonoClone = new StBoolParamNamed(false, aLangMap.changeValueId(STTR_PARAMETER_MONOCLONE, "Show Mono in Stereo"));
    mySettings->loadParam(ST_SETTING_MONOCLONE, params.MonoClone);

    // Layout option
    StHandle<StEnumParam> aLayoutParam = new StEnumParam(isHdmiPack ? LAYOUT_OVER_UNDER : LAYOUT_SIDE_BY_SIDE_ANAMORPH,
                                                         aLangMap.changeValueId(STTR_PARAMETER_LAYOUT, "Layout"));
    aLayoutParam->changeValues().add(aLangMap.changeValueId(STTR_PARAMETER_LAYOUT_SBS_ANAMORPH,       "Side-by-Side (Anamorph)"));
    aLayoutParam->changeValues().add(aLangMap.changeValueId(STTR_PARAMETER_LAYOUT_OVERUNDER_ANAMORPH, "Top-and-Bottom (Anamorph)"));
    aLayoutParam->changeValues().add(aLangMap.changeValueId(STTR_PARAMETER_LAYOUT_SBS,                "Side-by-Side"));
    aLayoutParam->changeValues().add(aLangMap.changeValueId(STTR_PARAMETER_LAYOUT_OVERUNDER,          "Top-and-Bottom") + (isHdmiPack ? " [HDMI]" : ""));
    params.Layout = aLayoutParam;
    mySettings->loadParam(ST_SETTING_LAYOUT, params.Layout);

    // load window position
    if(isMovable()) {
        StRect<int32_t> aRect;
        if(!mySettings->loadInt32Rect(ST_SETTING_WINDOWPOS, aRect)) {
            aRect = defaultRect();
        }
        StWindow::setPlacement(aRect, true);
    }
    checkHdmiPack();
    StWindow::setTitle("sView - Distorted Renderer");

    StRectI_t aMargins;
    aMargins.left()   = 64;
    aMargins.right()  = 64;
    aMargins.top()    = 160;
    aMargins.bottom() = 160;
    mySettings->loadInt32Rect(ST_SETTING_MARGINS,   aMargins);
    mySettings->loadFloatVec4(ST_SETTING_WARP_COEF, myBarrelCoef);
    mySettings->loadFloatVec4(ST_SETTING_CHROME_AB, myChromAb);

    myBarMargins.left   = aMargins.left();
    myBarMargins.right  = aMargins.right();
    myBarMargins.top    = aMargins.top();
    myBarMargins.bottom = aMargins.bottom();
}

void StOutDistorted::releaseResources() {
    if(!myContext.isNull()) {
    #ifdef ST_HAVE_LIBOVR
        if(myOvrSwapFbo[0] != 0) {
            myContext->arbFbo->glDeleteFramebuffers(2, myOvrSwapFbo);
            myOvrSwapFbo[0] = 0;
            myOvrSwapFbo[1] = 0;
        }
        if(myOvrSwapTexture != NULL) {
            ovr_DestroySwapTextureSet(myOvrHmd, myOvrSwapTexture);
            myOvrSwapTexture = NULL;
        }
        if(myOvrMirrorFbo != 0) {
            myContext->arbFbo->glDeleteFramebuffers(1, &myOvrMirrorFbo);
            myOvrMirrorFbo = NULL;
        }
        if(myOvrMirrorTexture != NULL) {
            ovr_DestroyMirrorTexture(myOvrHmd, &myOvrMirrorTexture->Texture);
            myOvrMirrorTexture = NULL;
        }
        if(myOvrHmd != NULL) {
            ovr_Destroy(myOvrHmd);
            myOvrHmd = NULL;
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

    StRectI_t aMargins;
    aMargins.left()   = myBarMargins.left;
    aMargins.right()  = myBarMargins.right;
    aMargins.top()    = myBarMargins.top;
    aMargins.bottom() = myBarMargins.bottom;

    mySettings->saveParam(ST_SETTING_LAYOUT,     params.Layout);
    mySettings->saveParam(ST_SETTING_MONOCLONE,  params.MonoClone);
    mySettings->saveInt32Rect(ST_SETTING_MARGINS,   aMargins);
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

#ifdef ST_HAVE_LIBOVR
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
        //myCursor->setMinMagFilter(*myContext, GL_NEAREST);
        myCursor->init(*myContext, aCursorImg.getPlane());
    }

#ifdef ST_HAVE_LIBOVR
    if(myDevice == DEVICE_OCULUS) {
        ovrGraphicsLuid aLuid;
        const ovrResult anOvrRes = ovr_Create(&myOvrHmd, &aLuid);
        if(myOvrHmd == NULL
        || !OVR_SUCCESS(anOvrRes)) {
            myMsgQueue->pushError(stCString("StOutDistorted, Oculus Rift is not connected!"));
            myOvrHmd = NULL;
            return true;
        }
    }

    if(myOvrHmd != NULL) {
        ovrHmdDesc anHmdDesc = ovr_GetHmdDesc(myOvrHmd);
        ovrSizei aWinSize = { anHmdDesc.Resolution.w / 2, anHmdDesc.Resolution.h / 2 };

        ST_DEBUG_LOG("libOVR Resolution: " + anHmdDesc.Resolution.w + "x" + anHmdDesc.Resolution.h);
        if(isMovable()) {
            StRect<int32_t> aRect = StWindow::getPlacement();
            aRect.right()  = aRect.left() + aWinSize.w;
            aRect.bottom() = aRect.top()  + aWinSize.h;
            StWindow::setPlacement(aRect, false);
        }

        ovrResult anOvrRes = ovr_CreateMirrorTextureGL(myOvrHmd, GL_SRGB8_ALPHA8, aWinSize.w, aWinSize.h, (ovrTexture** )&myOvrMirrorTexture);
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
            ovr_GetFovTextureSize(myOvrHmd, ovrEye_Left,  anHmdDesc.DefaultEyeFov[ovrEye_Left],  1),
            ovr_GetFovTextureSize(myOvrHmd, ovrEye_Right, anHmdDesc.DefaultEyeFov[ovrEye_Right], 1)
        };
        myOvrSizeX = stMax(anEyeSizes[0].w, anEyeSizes[1].w);
        myOvrSizeY = stMax(anEyeSizes[0].h, anEyeSizes[1].h);
        anOvrRes = ovr_CreateSwapTextureSetGL(myOvrHmd, GL_SRGB8_ALPHA8, myOvrSizeX * 2, myOvrSizeY, &myOvrSwapTexture);
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
}

void StOutDistorted::showCursor(const bool theToShow) {
    myToShowCursor = theToShow;
}

void StOutDistorted::stglDrawCursor(const StPointD_t&  theCursorPos,
                                    const unsigned int theView) {
    StWindow::showCursor(false);
    if(!myToShowCursor
    || !myCursor->isValid()) {
        return;
    }

    const GLfloat aLensDisp = getLensDist() * 0.5f;

    // compute cursor position
    StArray<StGLVec4> aVerts(4);
    const GLfloat aCurLeft = GLfloat(-1.0 + theCursorPos.x() * 2.0);
    const GLfloat aCurTop  = GLfloat( 1.0 - theCursorPos.y() * 2.0);
    const int aVPSizeX = myContext->stglViewport().width();
    const int aVPSizeY = myContext->stglViewport().height();

    GLfloat aCurWidth  = 2.0f * GLfloat(myCursor->getSizeX()) / GLfloat(aVPSizeX);
    GLfloat aCurHeight = 2.0f * GLfloat(myCursor->getSizeY()) / GLfloat(aVPSizeY);
    if(myDevice != DEVICE_OCULUS) {
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
    if(myOvrHmd != NULL) {
        return true;
    }
    return StWindow::hasOrientationSensor();
}

StQuaternion<double> StOutDistorted::getDeviceOrientation() const {
    if(StWindow::toTrackOrientation()
    && myOvrHmd != NULL
    && !myIsBroken) {
        return myOvrOrient;
    }
    return StWindow::getDeviceOrientation();
}

GLfloat StOutDistorted::getLensDist() const {
    return (myIsStereoOn && myDevice == DEVICE_OCULUS) ? 0.1453f : 0.0f;
}

GLfloat StOutDistorted::getScaleFactor() const {
    if(!myToReduceGui
    || !myIsStereoOn
    ||  myDevice != DEVICE_OCULUS) {
        return StWindow::getScaleFactor();
    }

    return 0.8f;
}

void StOutDistorted::checkHdmiPack() {
    myIsHdmiPack = false;
    if(!StWindow::isFullScreen()
    || myDevice == DEVICE_OCULUS) {
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
        myMargins.left   = 0;
        myMargins.right  = 0;
        myMargins.top    = 0;
        myMargins.bottom = 0;
    }
    StWindow::setFullScreen(theFullScreen);
    if(!wasFullscreen) {
        checkHdmiPack();
    }
}

void StOutDistorted::stglDrawLibOVR() {
#ifdef ST_HAVE_LIBOVR
    const StGLBoxPx  aVPBoth    = StWindow::stglViewport(ST_WIN_ALL);
    const StPointD_t aCursorPos = StWindow::getMousePos();
    if(myOvrHmd == NULL
    || myIsBroken) {
        return;
    }

    myToReduceGui = true;
    ovrHmdDesc anHmdDesc = ovr_GetHmdDesc(myOvrHmd);
    ovrEyeRenderDesc anEyeRenderDesc[2] = {
        ovr_GetRenderDesc(myOvrHmd, ovrEye_Left,  anHmdDesc.DefaultEyeFov[0]),
        ovr_GetRenderDesc(myOvrHmd, ovrEye_Right, anHmdDesc.DefaultEyeFov[1])
    };
    ovrViewScaleDesc aViewScaleDesc;
    aViewScaleDesc.HmdSpaceToWorldScaleInMeters = 1.0f;
    aViewScaleDesc.HmdToEyeViewOffset[0] = anEyeRenderDesc[0].HmdToEyeViewOffset;
    aViewScaleDesc.HmdToEyeViewOffset[1] = anEyeRenderDesc[1].HmdToEyeViewOffset;

    const StGLBoxPx aViewPortL = {{ 0, 0,
                                    myOvrSizeX, myOvrSizeY }};
    const StGLBoxPx aViewPortR = {{ myOvrSizeX, 0,
                                    myOvrSizeX, myOvrSizeY }};

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
    const double aPredictedTime = ovr_GetPredictedDisplayTime(myOvrHmd, 0);
    ovrTrackingState anHmdState = ovr_GetTrackingState(myOvrHmd, aPredictedTime, ovrTrue);
    ovr_CalcEyePoses(anHmdState.HeadPose.ThePose, aViewScaleDesc.HmdToEyeViewOffset, aLayerFov.RenderPose);
    myOvrOrient = StQuaternion<double>((double )aLayerFov.RenderPose[0].Orientation.x,
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
    ovrResult anOvrRes = ovr_SubmitFrame(myOvrHmd, 0, &aViewScaleDesc, &aLayers, 1);
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
#ifdef ST_HAVE_LIBOVR
    if(myOvrHmd != NULL) {
        myIsStereoOn = isStereoSource
                    && !myIsBroken;
    }
#endif

    myIsForcedStereo = myIsStereoOn && params.MonoClone->getValue();
    if(myIsStereoOn
    && myDevice == DEVICE_OCULUS) {
        myMargins = myBarMargins;
    } else {
        myMargins.left   = 0;
        myMargins.right  = 0;
        myMargins.top    = 0;
        myMargins.bottom = 0;
    }

    StWinSplit aWinSplit = StWinSlave_splitOff;
    const Layout aPairLayout = getPairLayout();
    if(myIsStereoOn
    && (myDevice == DEVICE_OCULUS
     || aPairLayout == LAYOUT_SIDE_BY_SIDE
     || aPairLayout == LAYOUT_OVER_UNDER)) {
        aWinSplit = aPairLayout == LAYOUT_OVER_UNDER && myDevice != DEVICE_OCULUS
                  ? StWinSlave_splitVertical
                  : StWinSlave_splitHorizontal;
        if(myDevice != DEVICE_OCULUS
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

#ifdef ST_HAVE_LIBOVR
    if(myOvrHmd != NULL
    && !myIsBroken) {
        stglDrawLibOVR();
        return;
    }
#endif

    const StGLBoxPx  aVPSlave   = StWindow::stglViewport(ST_WIN_SLAVE);
    const StGLBoxPx  aVPBoth    = StWindow::stglViewport(ST_WIN_ALL);
    const StPointD_t aCursorPos = StWindow::getMousePos();
    StGLBoxPx aViewPortL = aVPMaster;
    StGLBoxPx aViewPortR = aVPSlave;
    if(myDevice != DEVICE_OCULUS) {
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
    if(myDevice != DEVICE_OCULUS
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
    if(myDevice == DEVICE_OCULUS) {
        myToReduceGui = aFrSizeX <= 640;
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
    if(myDevice == DEVICE_OCULUS) {
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
