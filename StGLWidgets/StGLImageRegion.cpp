/**
 * StGLWidgets, small C++ toolkit for writing GUI using OpenGL.
 * Copyright © 2010-2016 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#include <StGLWidgets/StGLImageRegion.h>
#include <StGLWidgets/StGLRootWidget.h>
#include <StGL/StPlayList.h>
#include <StGLStereo/StGLQuadTexture.h>

#include <StGL/StGLContext.h>
#include <StGLCore/StGLCore20.h>

#include <StCore/StEvent.h>
#include <StSlots/StAction.h>

namespace {

    class ST_LOCAL StSwapLRParam : public StBoolParamNamed {

            public:

        StSwapLRParam(StGLImageRegion* theWidget)
        : StBoolParamNamed(false, stCString("swapLR"), stCString("SwapLR")), myWidget(theWidget) {}

        void invalidateWidget() {
            myWidget = NULL;
        }

        virtual bool getValue() const ST_ATTR_OVERRIDE {
            const StHandle<StStereoParams>& aParams = myWidget->params.stereoFile;
            return !aParams.isNull()
                 && aParams->ToSwapLR;
        }

        virtual bool setValue(const bool theValue) ST_ATTR_OVERRIDE {
            const StHandle<StStereoParams>& aParams = myWidget->params.stereoFile;
            if(aParams.isNull()
            || aParams->ToSwapLR == theValue) {
                return false;
            }
            aParams->setSwapLR(theValue);
            return true;
        }

            private:

        StGLImageRegion* myWidget;  //!< parent widget holding image parameters

    };

    class ST_LOCAL StViewModeParam : public StInt32ParamNamed {

            public:

        StViewModeParam(StGLImageRegion* theWidget)
        : StInt32ParamNamed(0, stCString("viewMode"), stCString("View Mode")), myWidget(theWidget) {}

        void invalidateWidget() {
            myWidget = NULL;
        }

        virtual int32_t getValue() const ST_ATTR_OVERRIDE {
            const StHandle<StStereoParams>& aParams = myWidget->params.stereoFile;
            return aParams.isNull() ? 0 : aParams->ViewingMode;
        }

        virtual bool setValue(const int32_t theValue) ST_ATTR_OVERRIDE {
            const StHandle<StStereoParams>& aParams = myWidget->params.stereoFile;
            if(aParams.isNull()
            || aParams->ViewingMode == theValue) {
                return false;
            }
            aParams->ViewingMode = (StStereoParams::ViewMode )theValue;
            signals.onChanged(theValue);
            return true;
        }

            private:

        StGLImageRegion* myWidget;  //!< parent widget holding image parameters

    };

    /**
     * Parameter interface tracking parameter value within active file.
     */
    class ST_LOCAL StFloat32StereoParam : public StFloat32Param {

            public:

        enum StereoParamId {
            StereoParamId_SepDX,
            StereoParamId_SepDY,
            StereoParamId_SepRot,
        };

            public:

        /**
         * Main constructor.
         */
        StFloat32StereoParam(StGLImageRegion*    theWidget,
                             const StereoParamId theParamId,
                             const StCString&    theParamKey)
        : StFloat32Param(0.0f, theParamKey), myWidget(theWidget), myParamId(theParamId) {
            const float THE_SEP_STEP_PX = 2.0f;
            switch(myParamId) {
                case StereoParamId_SepDX: {
                    setMinValue(-100.0f);
                    setMaxValue( 100.0f);
                    setStep(THE_SEP_STEP_PX);
                    setTolerance(0.5f);
                    break;
                }
                case StereoParamId_SepDY: {
                    setMinValue(-100.0f);
                    setMaxValue( 100.0f);
                    setStep(THE_SEP_STEP_PX);
                    setTolerance(0.5f);
                    break;
                }
                case StereoParamId_SepRot: {
                    setMinValue(-180.0f);
                    setMaxValue( 180.0f);
                    setStep(0.1f);
                    setTolerance(0.001f);
                    break;
                }
            }
        }

        void invalidateWidget() {
            myWidget = NULL;
        }

        virtual float getValue() const ST_ATTR_OVERRIDE {
            const StHandle<StStereoParams>& aParams = myWidget->params.stereoFile;
            if(aParams.isNull()) {
                return 0.0f;
            }
            switch(myParamId) {
                case StereoParamId_SepDX:  return (float )aParams->getSeparationDx();
                case StereoParamId_SepDY:  return (float )aParams->getSeparationDy();
                case StereoParamId_SepRot: return aParams->getSepRotation();
            }
            return 0.0f;
        }

        virtual bool setValue(const float theValue) ST_ATTR_OVERRIDE {
            const StHandle<StStereoParams>& aParams = myWidget->params.stereoFile;
            if(aParams.isNull()) {
                return false;
            }

            switch(myParamId) {
                case StereoParamId_SepDX: {
                    int32_t aValue = int32_t(theValue >= 0.0f ? (theValue + 0.5f) : (theValue - 0.5f));
                    if(aParams->getSeparationDx() == aValue) {
                        return false;
                    }
                    aParams->setSeparationDx(aValue);
                    break;
                 }
                case StereoParamId_SepDY: {
                    int32_t aValue = int32_t(theValue >= 0.0f ? (theValue + 0.5f) : (theValue - 0.5f));
                    if(aParams->getSeparationDy() == aValue) {
                        return false;
                    }
                    aParams->setSeparationDy(aValue);
                    break;
                 }
                 case StereoParamId_SepRot: {
                     float aValue = theValue;
                     while(aValue >= 360.0f) {
                        aValue -= 360.0f;
                     }
                     while(aValue <= -360.0f) {
                        aValue += 360.0f;
                     }
                     if(StFloat32Param::areEqual(aParams->getSepRotation(), aValue)) {
                        return false;
                     }
                     aParams->setSepRotation(aValue);
                 }
            }

            signals.onChanged(theValue);
            return true;
        }

            private:

        StGLImageRegion* myWidget;  //!< parent widget holding image parameters
        StereoParamId    myParamId; //!< active parameter to be tracked

    };

    // we use negative scale factor to show sphere inside out!
    static const float THE_SPHERE_RADIUS     = -10.0f;
    static const float THE_PANORAMA_DEF_ZOOM = 0.45f;

}

StGLImageRegion::StGLImageRegion(StGLWidget* theParent,
                                 const StHandle<StGLTextureQueue>& theTextureQueue,
                                 bool theUsePanningKeys)
: StGLWidget(theParent, 0, 0, StGLCorner(ST_VCORNER_TOP, ST_HCORNER_LEFT)),
  myQuad(),
  myUVSphere(StGLVec3(0.0f, 0.0f, 0.0f), 1.0f, 64),
  myTextureQueue(theTextureQueue),
  myClickPntZo(0.0, 0.0),
  myKeyFlags(ST_VF_NONE),
  myDragDelayMs(0.0),
  myRotAngle(0.0f),
  myIsClickAborted(false),
#ifdef ST_EXTRA_CONTROLS
  myToRightRotate(true),
#else
  myToRightRotate(false),
#endif
  myIsInitialized(false),
  myHasVideoStream(false) {
    params.DisplayMode = new StEnumParam(MODE_STEREO, stCString("viewStereoMode"), stCString("Stereo Output"));
    params.DisplayMode->defineOption(MODE_STEREO,     stCString("Stereo"));
    params.DisplayMode->defineOption(MODE_ONLY_LEFT,  stCString("Left View"));
    params.DisplayMode->defineOption(MODE_ONLY_RIGHT, stCString("Right View"));
    params.DisplayMode->defineOption(MODE_PARALLEL,   stCString("Parallel Pair"));
    params.DisplayMode->defineOption(MODE_CROSSYED,   stCString("Cross-eyed"));

    params.DisplayRatio = new StEnumParam(RATIO_AUTO, stCString("ratio"), stCString("Display Ratio"));
    params.DisplayRatio->defineOption(RATIO_AUTO,  stCString("Auto"));
    params.DisplayRatio->defineOption(RATIO_1_1,   stCString("1:1"));
    params.DisplayRatio->defineOption(RATIO_4_3,   stCString("4:3"));
    params.DisplayRatio->defineOption(RATIO_16_9,  stCString("16:9"));
    params.DisplayRatio->defineOption(RATIO_16_10, stCString("16:10"));
    params.DisplayRatio->defineOption(RATIO_221_1, stCString("2.21:1"));
    params.DisplayRatio->defineOption(RATIO_5_4,   stCString("5:4"));

    params.ToHealAnamorphicRatio = new StBoolParamNamed(false, stCString("toHealAnamorphic"), stCString("Heal Anamorphic Ratio"));
    params.TextureFilter = new StEnumParam(StGLImageProgram::FILTER_LINEAR, stCString("viewTexFilter"), stCString("Texture Filter"));
    params.TextureFilter->defineOption(StGLImageProgram::FILTER_NEAREST, stCString("Nearest"));
    params.TextureFilter->defineOption(StGLImageProgram::FILTER_LINEAR,  stCString("Linear"));
    params.TextureFilter->defineOption(StGLImageProgram::FILTER_BLEND,   stCString("Blend"));

    params.Gamma         = myProgram.params.gamma;
    params.Brightness    = myProgram.params.brightness;
    params.Saturation    = myProgram.params.saturation;
    params.SwapLR        = new StSwapLRParam(this);
    params.ViewMode      = new StViewModeParam(this);
    params.SeparationDX  = new StFloat32StereoParam(this, StFloat32StereoParam::StereoParamId_SepDX,  stCString("sepDX"));
    params.SeparationDY  = new StFloat32StereoParam(this, StFloat32StereoParam::StereoParamId_SepDY,  stCString("sepDY"));
    params.SeparationRot = new StFloat32StereoParam(this, StFloat32StereoParam::StereoParamId_SepRot, stCString("sepRot"));

#ifdef ST_EXTRA_CONTROLS
    theUsePanningKeys = false;
#endif

    // create actions
    StHandle<StAction> anAction;
    anAction = new StActionIntSlot(stCString("DoParamsReset"), stSlot(this, &StGLImageRegion::doParamsReset), 0);
    anAction->setDefaultHotKey1(ST_VK_BACK);
    myActions.add(anAction);

    anAction = new StActionBool(stCString("DoParamsSwapLR"), params.SwapLR);
    anAction->setDefaultHotKey1(ST_VK_W);
    myActions.add(anAction);

    anAction = new StActionIntSlot(stCString("DoParamsGammaDec"), stSlot(this, &StGLImageRegion::doParamsGamma), (size_t )-1);
    anAction->setDefaultHotKey1(ST_VK_G | ST_VF_CONTROL);
    myActions.add(anAction);

    anAction = new StActionIntSlot(stCString("DoParamsGammaInc"), stSlot(this, &StGLImageRegion::doParamsGamma), 1);
    anAction->setDefaultHotKey1(ST_VK_G | ST_VF_SHIFT);
    myActions.add(anAction);

    anAction = new StActionIntSlot(stCString("DoParamsSepXDec"), stSlot(this, &StGLImageRegion::doParamsSepX), (size_t )-1);
    anAction->setDefaultHotKey1(ST_VK_COMMA);
    anAction->setDefaultHotKey2(ST_VK_DIVIDE);
#ifdef ST_EXTRA_CONTROLS
    anAction->setDefaultHotKey1(ST_VK_Q);
#endif
    myActions.add(anAction);

    anAction = new StActionIntSlot(stCString("DoParamsSepXInc"), stSlot(this, &StGLImageRegion::doParamsSepX), 1);
    anAction->setDefaultHotKey1(ST_VK_PERIOD);
    anAction->setDefaultHotKey2(ST_VK_MULTIPLY);
#ifdef ST_EXTRA_CONTROLS
    anAction->setDefaultHotKey1(ST_VK_E);
#endif
    myActions.add(anAction);

    anAction = new StActionIntSlot(stCString("DoParamsSepYDec"), stSlot(this, &StGLImageRegion::doParamsSepY), (size_t )-1);
    anAction->setDefaultHotKey1(ST_VK_COMMA  | ST_VF_CONTROL);
    anAction->setDefaultHotKey2(ST_VK_DIVIDE | ST_VF_CONTROL);
#ifdef ST_EXTRA_CONTROLS
    anAction->setDefaultHotKey1(ST_VK_Q      | ST_VF_CONTROL);
#endif
    myActions.add(anAction);

    anAction = new StActionIntSlot(stCString("DoParamsSepYInc"), stSlot(this, &StGLImageRegion::doParamsSepY), 1);
    anAction->setDefaultHotKey1(ST_VK_PERIOD   | ST_VF_CONTROL);
    anAction->setDefaultHotKey2(ST_VK_MULTIPLY | ST_VF_CONTROL);
#ifdef ST_EXTRA_CONTROLS
    anAction->setDefaultHotKey1(ST_VK_E        | ST_VF_CONTROL);
#endif
    myActions.add(anAction);

    anAction = new StActionHoldSlot(stCString("DoParamsSepRotDec"), stSlot(this, &StGLImageRegion::doParamsSepZDec));
    anAction->setDefaultHotKey1(ST_VK_APOSTROPHE | ST_VF_CONTROL);
    myActions.add(anAction);

    anAction = new StActionHoldSlot(stCString("DoParamsSepRotInc"), stSlot(this, &StGLImageRegion::doParamsSepZInc));
    anAction->setDefaultHotKey1(ST_VK_SEMICOLON | ST_VF_CONTROL);
    myActions.add(anAction);

    anAction = new StActionIntSlot(stCString("DoParamsRotZ90Dec"), stSlot(this, &StGLImageRegion::doParamsRotZ90), (size_t )-1);
    anAction->setDefaultHotKey1(ST_VK_BRACKETLEFT);
    myActions.add(anAction);

    anAction = new StActionIntSlot(stCString("DoParamsRotZ90Inc"), stSlot(this, &StGLImageRegion::doParamsRotZ90), 1);
    anAction->setDefaultHotKey1(ST_VK_BRACKETRIGHT);
    myActions.add(anAction);

    anAction = new StActionHoldSlot(stCString("DoParamsRotZDec"), stSlot(this, &StGLImageRegion::doParamsRotZLeft));
    anAction->setDefaultHotKey1(ST_VK_BRACKETLEFT | ST_VF_CONTROL);
    myActions.add(anAction);

    anAction = new StActionHoldSlot(stCString("DoParamsRotZInc"), stSlot(this, &StGLImageRegion::doParamsRotZRight));
    anAction->setDefaultHotKey1(ST_VK_BRACKETRIGHT | ST_VF_CONTROL);
    myActions.add(anAction);

    anAction = new StActionIntSlot(stCString("DoParamsModeNext"), stSlot(this, &StGLImageRegion::doParamsModeNext), 0);
    myActions.add(anAction);

    anAction = new StActionHoldSlot(stCString("DoParamsPanLeft"), stSlot(this, &StGLImageRegion::doParamsPanLeft));
    anAction->setDefaultHotKey1(theUsePanningKeys ? ST_VK_LEFT : 0);
    myActions.add(anAction);

    anAction = new StActionHoldSlot(stCString("DoParamsPanRight"), stSlot(this, &StGLImageRegion::doParamsPanRight));
    anAction->setDefaultHotKey1(theUsePanningKeys ? ST_VK_RIGHT : 0);
    myActions.add(anAction);

    anAction = new StActionHoldSlot(stCString("DoParamsPanUp"), stSlot(this, &StGLImageRegion::doParamsPanUp));
    anAction->setDefaultHotKey1(theUsePanningKeys ? ST_VK_UP : 0);
    myActions.add(anAction);

    anAction = new StActionHoldSlot(stCString("DoParamsPanDown"), stSlot(this, &StGLImageRegion::doParamsPanDown));
    anAction->setDefaultHotKey1(theUsePanningKeys ? ST_VK_DOWN : 0);
    myActions.add(anAction);

    anAction = new StActionHoldSlot(stCString("DoParamsScaleIn"), stSlot(this, &StGLImageRegion::doParamsScaleIn));
    anAction->setDefaultHotKey1(ST_VK_ADD);
    anAction->setDefaultHotKey2(ST_VK_OEM_PLUS);
    myActions.add(anAction);

    anAction = new StActionHoldSlot(stCString("DoParamsScaleOut"), stSlot(this, &StGLImageRegion::doParamsScaleOut));
    anAction->setDefaultHotKey1(ST_VK_SUBTRACT);
    anAction->setDefaultHotKey2(ST_VK_OEM_MINUS);
    myActions.add(anAction);

    anAction = new StActionHoldSlot(stCString("DoParamsRotYLeft"), stSlot(this, &StGLImageRegion::doParamsRotYLeft));
#ifdef ST_EXTRA_CONTROLS
    anAction->setDefaultHotKey1(ST_VK_LEFT);
#endif
    myActions.add(anAction);

    anAction = new StActionHoldSlot(stCString("DoParamsRotYRight"), stSlot(this, &StGLImageRegion::doParamsRotYRight));
#ifdef ST_EXTRA_CONTROLS
    anAction->setDefaultHotKey1(ST_VK_RIGHT);
#endif
    myActions.add(anAction);

    anAction = new StActionHoldSlot(stCString("DoParamsRotXUp"), stSlot(this, &StGLImageRegion::doParamsRotXUp));
#ifdef ST_EXTRA_CONTROLS
    anAction->setDefaultHotKey1(ST_VK_UP);
#endif
    myActions.add(anAction);

    anAction = new StActionHoldSlot(stCString("DoParamsRotXDown"), stSlot(this, &StGLImageRegion::doParamsRotXDown));
#ifdef ST_EXTRA_CONTROLS
    anAction->setDefaultHotKey1(ST_VK_DOWN);
#endif
    myActions.add(anAction);
}

StGLImageRegion::~StGLImageRegion() {
    // make sure GL objects are released within GL thread
    StGLContext& aCtx = getContext();
    myTextureQueue->getQTexture().release(aCtx);
    myQuad.release(aCtx);
    myUVSphere.release(aCtx);
    myProgram.release(aCtx);

    // simplify debugging - nullify pointer to this widget
    ((StSwapLRParam*        )params.SwapLR       .access())->invalidateWidget();
    ((StViewModeParam*      )params.ViewMode     .access())->invalidateWidget();
    ((StFloat32StereoParam* )params.SeparationDX .access())->invalidateWidget();
    ((StFloat32StereoParam* )params.SeparationDY .access())->invalidateWidget();
    ((StFloat32StereoParam* )params.SeparationRot.access())->invalidateWidget();
}

StHandle<StStereoParams> StGLImageRegion::getSource() {
    return params.stereoFile;
}

void StGLImageRegion::stglSkipFrames() {
    myTextureQueue->stglUpdateStTextures(getContext());
}

void StGLImageRegion::stglUpdate(const StPointD_t& pointZo) {
    StGLWidget::stglUpdate(pointZo);
    if(myIsInitialized) {
        myHasVideoStream = myTextureQueue->stglUpdateStTextures(getContext()) || myTextureQueue->hasConnectedStream();
        StHandle<StStereoParams> aFileParams = myTextureQueue->getQTexture().getFront(StGLQuadTexture::LEFT_TEXTURE).getSource();
        if(params.stereoFile != aFileParams) {
            params.stereoFile = aFileParams;
            onParamsChanged();
        }
    }
}

bool StGLImageRegion::stglInit() {
    bool isInit = StGLWidget::stglInit();
    if(myIsInitialized) {
        return isInit;
    }

    StGLContext& aCtx = getContext();
    if(!myProgram.init(aCtx, StImage::ImgColor_RGB, StImage::ImgScale_Full, StGLImageProgram::FragGetColor_Normal)) {
        return false;
    } else if(!myQuad.initScreen(aCtx)) {
        aCtx.pushError(StString("Fail to init StGLQuad"));
        ST_ERROR_LOG("Fail to init StGLQuad");
        return false;
    } else if(!myUVSphere.initVBOs(aCtx)) {
        ST_ERROR_LOG("Fail to init StGLUVSphere");
    }

    // setup texture filter
    myTextureQueue->getQTexture().setMinMagFilter(aCtx, params.TextureFilter->getValue() == StGLImageProgram::FILTER_NEAREST ? GL_NEAREST : GL_LINEAR);

    myIsInitialized = true;
    return myIsInitialized && isInit;
}

StGLVec2 StGLImageRegion::getMouseMoveFlat(const StPointD_t& theCursorZoFrom,
                                           const StPointD_t& theCursorZoTo) const {
    // apply scale factor in case of working area margins
    const double aVrScale = 1.0 / myRoot->getVrZoomOut();
    const double aScaleX  = double(myRoot->getRectPx().width())  / double(myRoot->getRootFullSizeX());
    const double aScaleY  = double(myRoot->getRectPx().height()) / double(myRoot->getRootFullSizeY());
    return StGLVec2(float( 2.0 * double(theCursorZoTo.x() - theCursorZoFrom.x()) * aScaleX * aVrScale),
                    float(-2.0 * double(theCursorZoTo.y() - theCursorZoFrom.y()) * aScaleY * aVrScale));
}

StGLVec2 StGLImageRegion::getMouseMoveSphere(const StPointD_t& theCursorZoFrom,
                                             const StPointD_t& theCursorZoTo) const {
    StGLVec2 aVec = getMouseMoveFlat(theCursorZoFrom, theCursorZoTo);
    float aSphereScale = THE_SPHERE_RADIUS * THE_PANORAMA_DEF_ZOOM * params.stereoFile->ScaleFactor;
    StRectD_t aZParams;
    getCamera()->getZParams(getCamera()->getZNear(), aZParams);
    aVec.x() *= -90.0f * float(aZParams.right() - aZParams.left()) / aSphereScale;
    aVec.y() *=  90.0f * float(aZParams.bottom() - aZParams.top()) / aSphereScale;
    return aVec;
}

StGLVec2 StGLImageRegion::getMouseMoveSphere() const {
    return isClicked(ST_MOUSE_LEFT)
         ? getMouseMoveSphere(myClickPntZo, myRoot->getCursorZo())
         : StGLVec2();
}

StGLQuaternion StGLImageRegion::getHeadOrientation(unsigned int theView,
                                                   const bool theToApplyDefShift) const {
    StHandle<StStereoParams> aParams = params.stereoFile;
    if(aParams.isNull()) {
        return StGLQuaternion();
    }

    const float aYawShift = theToApplyDefShift ? stToRadians(90.0f) : 0.0f;
    StGLVec2 aMouseMove = getMouseMoveSphere();
    float aYaw   = -stToRadians(aParams->PanPhi   + aMouseMove.x()) + aYawShift;
    float aPitch =  stToRadians(StStereoParams::clipPitch(aParams->PanTheta + aMouseMove.y()));
    float aRoll  =  stToRadians(aParams->getZRotate());

    // apply separation
    const float aSepDeltaX = GLfloat(aParams->getSeparationDx()) * 0.05f;
    const float aSepDeltaY = GLfloat(aParams->getSeparationDy()) * 0.05f;
    if(theView == ST_DRAW_LEFT) {
        aYaw   +=  stToRadians(aSepDeltaX);
        aPitch += -stToRadians(aSepDeltaY);
        aRoll  += -stToRadians(aParams->getSepRotation());
    } else if(theView == ST_DRAW_RIGHT) {
        aYaw   += -stToRadians(aSepDeltaX);
        aPitch +=  stToRadians(aSepDeltaY);
        aRoll  +=  stToRadians(aParams->getSepRotation());
    }

    const StGLQuaternion anOriYaw   = StGLQuaternion(StGLVec3::DY(), aYaw);
    const StGLQuaternion anOriPitch = StGLQuaternion(StGLVec3::DX(), aPitch);
    const StGLQuaternion anOriRoll  = StGLQuaternion(StGLVec3::DZ(), aRoll);
    StGLQuaternion anOri = StGLQuaternion::multiply(anOriPitch, anOriYaw);
    anOri = StGLQuaternion::multiply(anOriRoll,    anOri);
    anOri = StGLQuaternion::multiply(myDeviceQuat, anOri);
    return anOri;
}

void StGLImageRegion::stglDraw(unsigned int theView) {
    StHandle<StStereoParams> aParams = getSource();
    if(!myIsInitialized || !isVisible() || aParams.isNull()
    || !myTextureQueue->getQTexture().getFront(StGLQuadTexture::LEFT_TEXTURE).isValid()
    || !myHasVideoStream) {
        StGLWidget::stglDraw(theView);
        return;
    }

    if(aParams->isMono()) {
        theView = ST_DRAW_MONO;
        aParams->setSwapLR(false);
    }

    switch(params.DisplayMode->getValue()) {
        case MODE_PARALLEL:
        case MODE_CROSSYED:
            stglDrawView(ST_DRAW_LEFT);
            stglDrawView(ST_DRAW_RIGHT);
            break;
        case MODE_ONLY_LEFT:
            stglDrawView(ST_DRAW_LEFT);
            break;
        case MODE_ONLY_RIGHT:
            stglDrawView(ST_DRAW_RIGHT);
            break;
        default:
            stglDrawView(theView);
            break;
    }
    StGLWidget::stglDraw(theView);
}

void StGLImageRegion::stglDrawView(unsigned int theView) {
    StGLQuadTexture::LeftOrRight aLeftOrRight = StGLQuadTexture::LEFT_TEXTURE;
    StHandle<StStereoParams> aParams = getSource();
    if(!myIsInitialized || aParams.isNull()) {
        return;
    }

    StGLContext& aCtx = getContext();
    bool toShowRight = ( aParams->ToSwapLR && (theView == ST_DRAW_LEFT ))
                    || (!aParams->ToSwapLR && (theView == ST_DRAW_RIGHT));
    if(aParams->isMono()) {
        aLeftOrRight = StGLQuadTexture::LEFT_TEXTURE;
    } else if(myTextureQueue->getQTexture().getFront(StGLQuadTexture::RIGHT_TEXTURE).isValid()) {
        if(toShowRight) {
            aLeftOrRight = StGLQuadTexture::RIGHT_TEXTURE;
        }
    }

    // retrieve viewport size for correct scissor rectangle computation
    GLfloat aFrustrumL = 1.0f, aFrustrumR = 1.0f, aFrustrumT = 1.0f, aFrustrumB = 1.0f;
    StRectI_t aFrameRectPx = getRectPx();
    float aCameraAspect = getCamera()->getAspect();
    const StGLBoxPx aViewportBack = aCtx.stglViewport();
    if(!aParams->isMono()) {
        switch(params.DisplayMode->getValue()) {
            case MODE_PARALLEL: {
                if(theView == ST_DRAW_LEFT) {
                    aFrameRectPx.right() /= 2;
                } else {
                    aFrameRectPx.left() += aFrameRectPx.width() / 2;
                }
                aCameraAspect *= 0.5f;
                break;
            }
            case MODE_CROSSYED: {
                if(theView == ST_DRAW_RIGHT) {
                    aFrameRectPx.right() /= 2;
                } else {
                    aFrameRectPx.left() += aFrameRectPx.width() / 2;
                }
                aCameraAspect *= 0.5f;
                break;
            }
        }
    }

    // setup scissor box
    StGLBoxPx aScissorBox;
    const StRectI_t aFrameRectAbs = getAbsolute(aFrameRectPx);
    getRoot()->stglScissorRect(aFrameRectAbs, aScissorBox);
    aCtx.stglSetScissorRect(aScissorBox, true);
    aCtx.stglResizeViewport(aScissorBox);
    myProjCam.resize(aCameraAspect);

    aCtx.core20fwd->glDisable(GL_BLEND);

    StGLFrameTextures& aTextures = myTextureQueue->getQTexture().getFront(aLeftOrRight);
    aTextures.bind(aCtx);

    StGLVec2 aTextureSize  (GLfloat(aTextures.getPlane(0).getSizeX()),
                            GLfloat(aTextures.getPlane(0).getSizeY()));
    StGLVec2 aTextureUVSize(GLfloat(aTextures.getPlane(1).getSizeX()),
                            GLfloat(aTextures.getPlane(1).getSizeY()));
    StGLMatrix aModelMat;
    // data rectangle in the texture
    StGLVec4 aClampVec, aClampUV;
    if(params.TextureFilter->getValue() == StGLImageProgram::FILTER_NEAREST) {
        myTextureQueue->getQTexture().setMinMagFilter(aCtx, GL_NEAREST);
        //
        aClampVec.x() = 0.0f;
        aClampVec.y() = 0.0f;
        aClampVec.z() = aTextures.getPlane(0).getDataSize().x();
        aClampVec.w() = aTextures.getPlane(0).getDataSize().y();
        // UV
        aClampUV.x() = 0.0f;
        aClampUV.y() = 0.0f;
        aClampUV.z() = aTextures.getPlane(1).getDataSize().x();
        aClampUV.w() = aTextures.getPlane(1).getDataSize().y();
    } else {
        myTextureQueue->getQTexture().setMinMagFilter(aCtx, GL_LINEAR);
        //
        aClampVec.x() = 0.5f / aTextureSize.x();
        aClampVec.y() = 0.5f / aTextureSize.y();
        aClampVec.z() = aTextures.getPlane(0).getDataSize().x() - 2.0f * aClampVec.x();
        aClampVec.w() = aTextures.getPlane(0).getDataSize().y() - 2.0f * aClampVec.y();
        // UV
        if(aTextureUVSize.x() > 0.0f
        && aTextureUVSize.y() > 0.0f) {
            aClampUV.x() = 0.5f / aTextureUVSize.x();
            aClampUV.y() = 0.5f / aTextureUVSize.y(),
            aClampUV.z() = aTextures.getPlane(1).getDataSize().x() - 2.0f * aClampUV.x();
            aClampUV.w() = aTextures.getPlane(1).getDataSize().y() - 2.0f * aClampUV.y();
        }
    }

    // select (de)anaglyph color filter
    StGLVec3 aColorScale(1.0f, 1.0f, 1.0f);
    switch(aParams->StereoFormat) {
        case StFormat_AnaglyphRedCyan: {
            if(!toShowRight) {
                aColorScale.g() = 0.0f;
                aColorScale.b() = 0.0f;
            } else {
                aColorScale.r() = 0.0f;
            }
            break;
        }
        case StFormat_AnaglyphGreenMagenta: {
            if(!toShowRight) {
                aColorScale.r() = 0.0f;
                aColorScale.b() = 0.0f;
            } else {
                aColorScale.g() = 0.0f;
            }
            break;
        }
        case StFormat_AnaglyphYellowBlue: {
            if(!toShowRight) {
                aColorScale.b() = 0.0f;
            } else {
                aColorScale.r() = 0.0f;
                aColorScale.g() = 0.0f;
            }
            break;
        }
        default: break;
    }

    // remember parameters to restore
    const GLfloat  aScaleBack = aParams->ScaleFactor;
    const StGLVec2 aPanBack   = aParams->PanCenter;

    const float aVrScale = float(myRoot->getVrZoomOut());

    StStereoParams::ViewMode aViewMode = aParams->ViewingMode;
    if(aTextures.getPlane(0).getTarget() == GL_TEXTURE_CUBE_MAP) {
        aViewMode = StStereoParams::PANORAMA_CUBEMAP;
    } else if(aViewMode == StStereoParams::PANORAMA_CUBEMAP) {
        aViewMode = StStereoParams::FLAT_IMAGE;
    }

    myProgram.setColorScale(aColorScale); // apply de-anaglyph color filter
    StGLImageProgram::FragGetColor aColorGetter = params.TextureFilter->getValue() == StGLImageProgram::FILTER_BLEND
                                                ? StGLImageProgram::FragGetColor_Blend
                                                : StGLImageProgram::FragGetColor_Normal;
    switch(aViewMode) {
        default:
        case StStereoParams::FLAT_IMAGE: {
            if(!myProgram.init(aCtx, aTextures.getColorModel(), aTextures.getColorScale(), aColorGetter)) {
                break;
            }

            myProgram.getActiveProgram()->use(aCtx);

            // setup data rectangle in the texture
            myProgram.setTextureSizePx      (aCtx, aTextureSize);
            myProgram.setTextureMainDataSize(aCtx, aClampVec);
            myProgram.setTextureUVDataSize  (aCtx, aClampUV);

            // handle dragging timer
            if( isClicked(ST_MOUSE_LEFT)
            && !myIsClickAborted
            &&  myClickTimer.isOn()) {
                if(myClickTimer.getElapsedTimeInMilliSec() < myDragDelayMs) {
                    const StPointD_t aCurr = getRoot()->getCursorZo();
                    const int aDx = int((aCurr.x() - myClickPntZo.x()) * double(getRectPx().width()));
                    const int aDy = int((aCurr.y() - myClickPntZo.y()) * double(getRectPx().height()));
                    if(std::abs(aDx) > myRoot->getClickThreshold()
                    || std::abs(aDy) > myRoot->getClickThreshold()) {
                        myIsClickAborted = true;
                        myClickTimer.stop();
                    }
                } else {
                    myClickTimer.stop();
                }
            }

            // handle dragging
            if( isClicked(ST_MOUSE_LEFT)
            && !myIsClickAborted
            && !myClickTimer.isOn()) {
                const GLfloat aRectRatio = GLfloat(getRectPx().ratio());
                aParams->moveFlat(getMouseMoveFlat(myClickPntZo, getRoot()->getCursorZo()), aRectRatio);
                if(myDragDelayMs > 1.0) {
                    const GLfloat    aScaleSteps = 0.1f;
                    const StPointD_t aCenterCursor(0.5, 0.5);
                    const StGLVec2   aVec = getMouseMoveFlat(aCenterCursor, getRoot()->getCursorZo()) * (-aScaleSteps);
                    aParams->scaleIn(aScaleSteps);
                    aParams->moveFlat(aVec, aRectRatio);
                }
            }

            GLfloat anXRotate = aParams->getXRotate();
            GLfloat anYRotate = aParams->getYRotate();
            if(isClicked(ST_MOUSE_RIGHT)
            && (myToRightRotate || (myKeyFlags & ST_VF_CONTROL) == ST_VF_CONTROL)) {
                anXRotate += 180.0f * GLfloat(myRoot->getCursorZo().y() - myClickPntZo.y());
                anYRotate += 180.0f * GLfloat(myRoot->getCursorZo().x() - myClickPntZo.x());
            }

            // apply scale
            aModelMat.scale(aParams->ScaleFactor * aVrScale, aParams->ScaleFactor * aVrScale, 1.0f);

            // apply position
            aModelMat.translate(StGLVec3(aParams->PanCenter));

            // apply rotations
            if(theView == ST_DRAW_LEFT) {
                aModelMat.rotate(aParams->getZRotate() - aParams->getSepRotation(), StGLVec3::DZ());
            } else if(theView == ST_DRAW_RIGHT) {
                aModelMat.rotate(aParams->getZRotate() + aParams->getSepRotation(), StGLVec3::DZ());
            } else {
                aModelMat.rotate(aParams->getZRotate(), StGLVec3::DZ());
            }
            aModelMat.rotate(anXRotate, StGLVec3::DX());
            aModelMat.rotate(anYRotate, StGLVec3::DY());

            // check window ratio to fill whole image in normal zoom
            GLfloat aDispRatio = 1.0f;
            switch(params.DisplayRatio->getValue()) {
                case RATIO_1_1:   aDispRatio = 1.0f;  break;
                case RATIO_4_3:   aDispRatio = 1.3333333333f; break;
                case RATIO_16_9:  aDispRatio = 1.7777777778f; break;
                case RATIO_16_10: aDispRatio = 1.6f;  break;
                case RATIO_221_1: aDispRatio = 2.21f; break;
                case RATIO_5_4:   aDispRatio = 1.25f; break;
                case RATIO_AUTO:
                default: {
                    aDispRatio = aTextures.getPlane().getDisplayRatio();
                    if(params.ToHealAnamorphicRatio->getValue()
                    && aParams->Src2SizeX == 0 && aParams->Src2SizeY == 0
                    && ((aParams->Src1SizeX == 1920 && aParams->Src1SizeY >= 800 && aParams->Src1SizeY <= 1088)
                     || (aParams->Src1SizeX == 1280 && aParams->Src1SizeY >= 530 && aParams->Src1SizeY <= 720))) {
                        switch(aParams->StereoFormat) {
                             case StFormat_SideBySide_LR:
                             case StFormat_SideBySide_RL: {
                                if(aDispRatio >= 0.85 && aDispRatio <= 1.18) {
                                    aDispRatio *= 2.0;
                                }
                                break;
                             }
                             case StFormat_TopBottom_LR:
                             case StFormat_TopBottom_RL: {
                                if(aDispRatio >= 3.5 && aDispRatio <= 4.8) {
                                    aDispRatio *= 0.5;
                                }
                                break;
                             }
                             default: break;
                        }
                    }
                    break;
                }
            }

            GLfloat  aRectRatio  = GLfloat(aFrameRectPx.ratio());
            StGLVec2 aRatioScale = aParams->getRatioScale(aRectRatio, aDispRatio);
            aModelMat.scale(aRatioScale.x(), aRatioScale.y(), 1.0f);

            // apply separation
            GLfloat aSepDeltaX = (2.0f * aParams->getSeparationDx()) / (aTextures.getPlane().getDataSize().x() * aTextures.getPlane().getSizeX());
            GLfloat aSepDeltaY = (2.0f * aParams->getSeparationDy()) / (aTextures.getPlane().getDataSize().y() * aTextures.getPlane().getSizeY());
            if(theView == ST_DRAW_LEFT) {
                aModelMat.translate(StGLVec3(-aSepDeltaX * 0.5f, -aSepDeltaY * 0.5f, 0.0f));
            } else if(theView == ST_DRAW_RIGHT) {
                aModelMat.translate(StGLVec3( aSepDeltaX * 0.5f,  aSepDeltaY * 0.5f, 0.0f));
            }

            StGLMatrix anOrthoMat;
            anOrthoMat.initOrtho(StGLVolume(-aRectRatio * aFrustrumL, aRectRatio * aFrustrumR,
                                            -1.0f       * aFrustrumB, 1.0f       * aFrustrumT,
                                            -1.0f, 1.0f));
            myProgram.getActiveProgram()->setProjMat (aCtx, anOrthoMat);
            myProgram.getActiveProgram()->setModelMat(aCtx, aModelMat);

            myQuad.draw(aCtx, *myProgram.getActiveProgram());

            myProgram.getActiveProgram()->unuse(aCtx);

            // restore changed parameters
            aParams->ScaleFactor = aScaleBack;
            aParams->PanCenter   = aPanBack;
            break;
        }
        case StStereoParams::PANORAMA_CUBEMAP: {
            if(!myProgram.init(aCtx, aTextures.getColorModel(), aTextures.getColorScale(), StGLImageProgram::FragGetColor_Cubemap)) {
                break;
            }

            myProgram.getActiveProgram()->use(aCtx);

            // setup data rectangle in the texture
            myProgram.setTextureSizePx      (aCtx, aTextureSize);
            myProgram.setTextureMainDataSize(aCtx, aClampVec);
            myProgram.setTextureUVDataSize  (aCtx, aClampUV);

            const float aScale = THE_PANORAMA_DEF_ZOOM * aParams->ScaleFactor * aVrScale;
            aModelMat.scale(aScale, aScale, 1.0f);

            // compute orientation
            const StGLQuaternion anOri = getHeadOrientation(theView, true);
            aModelMat = StGLMatrix::multiply(aModelMat, StGLMatrix(anOri));

            StGLMatrix aMatModelInv, aMatProjInv;
            aModelMat.inverted(aMatModelInv);
            myProjCam.getProjMatrixMono().inverted(aMatProjInv);
            myProgram.getActiveProgram()->setProjMat (aCtx, StGLMatrix::multiply(aMatModelInv, aMatProjInv));
            myProgram.getActiveProgram()->setModelMat(aCtx, aModelMat);

            ///glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

            myQuad.draw(aCtx, *myProgram.getActiveProgram());

            myProgram.getActiveProgram()->unuse(aCtx);

            // restore changed parameters
            aParams->ScaleFactor = aScaleBack;
            aParams->PanCenter   = aPanBack;
            break;
        }
        case StStereoParams::PANORAMA_SPHERE: {
            if(!myProgram.init(aCtx, aTextures.getColorModel(), aTextures.getColorScale(), aColorGetter)) {
                break;
            }

            // perform scaling
            const float aScale = THE_SPHERE_RADIUS * THE_PANORAMA_DEF_ZOOM * aParams->ScaleFactor * aVrScale;
            aModelMat.scale(aScale, aScale, THE_SPHERE_RADIUS);

            // compute orientation
            const StGLQuaternion anOri = getHeadOrientation(theView, true);
            aModelMat = StGLMatrix::multiply(aModelMat, StGLMatrix(anOri));

            // perform drawing
            myProgram.getActiveProgram()->use(aCtx);

            // setup data rectangle in the texture
            myProgram.setTextureSizePx      (aCtx, aTextureSize);
            myProgram.setTextureMainDataSize(aCtx, aClampVec);
            myProgram.setTextureUVDataSize  (aCtx, aClampUV);

            myProgram.getActiveProgram()->setProjMat (aCtx, myProjCam.getProjMatrixMono());
            myProgram.getActiveProgram()->setModelMat(aCtx, aModelMat);

            myUVSphere.draw(aCtx, *myProgram.getActiveProgram());

            myProgram.getActiveProgram()->unuse(aCtx);
            break;
        }
    }

    aTextures.unbind(aCtx);

    aCtx.stglResetScissorRect();
    aCtx.stglResizeViewport(aViewportBack);
}

void StGLImageRegion::doRightUnclick(const StPointD_t& theCursorZo) {
    StHandle<StStereoParams> aParams = getSource();
    if(!myIsInitialized || aParams.isNull()
     || aParams->ViewingMode != StStereoParams::FLAT_IMAGE) {
        return;
    }

    GLfloat anXRotate = aParams->getXRotate() + 180.0f * GLfloat(theCursorZo.y() - myClickPntZo.y());
    GLfloat anYRotate = aParams->getYRotate() + 180.0f * GLfloat(theCursorZo.x() - myClickPntZo.x());
    for(; anXRotate > 360.0f;) {
        anXRotate -= 360.0f;
    }
    for(; anXRotate < 0.0f;) {
        anXRotate += 360.0f;
    }
    for(; anYRotate > 360.0f;) {
        anYRotate -= 360.0f;
    }
    for(; anYRotate < 0.0f;) {
        anYRotate += 360.0f;
    }
    aParams->setXRotate(anXRotate);
    aParams->setYRotate(anYRotate);
}

bool StGLImageRegion::tryClick(const StClickEvent& theEvent,
                               bool&               theIsItemClicked) {
    StHandle<StStereoParams> aParams = getSource();
    if(!myIsInitialized || aParams.isNull()) {
        return false;
    }

    if(StGLWidget::tryClick(theEvent, theIsItemClicked)) {
        if(theEvent.Button == ST_MOUSE_RIGHT
        && (myToRightRotate || (myKeyFlags & ST_VF_CONTROL) == ST_VF_CONTROL)) {
            myClickPntZo = StPointD_t(theEvent.PointX, theEvent.PointY);
            myIsClickAborted = true;
        } else if(theEvent.Button == ST_MOUSE_LEFT) {
            myClickPntZo = StPointD_t(theEvent.PointX, theEvent.PointY);
            myClickTimer.restart();
            myIsClickAborted = false;
        }
        theIsItemClicked = true;
        return true;
    }
    return false;
}

bool StGLImageRegion::tryUnClick(const StClickEvent& theEvent,
                                 bool&               theIsItemUnclicked) {
    StHandle<StStereoParams> aParams = getSource();
    if(!myIsInitialized || aParams.isNull()) {
        if(isClicked(theEvent.Button)) {
            theIsItemUnclicked = true;
            setClicked(theEvent.Button, false);
            return true;
        }
        return false;
    }

    StPointD_t aCursor(theEvent.PointX, theEvent.PointY);
    if(isClicked(ST_MOUSE_RIGHT)
    && theEvent.Button == ST_MOUSE_RIGHT
    && (myToRightRotate || (myKeyFlags & ST_VF_CONTROL) == ST_VF_CONTROL)) {
        doRightUnclick(aCursor);
    } else if(isClicked(ST_MOUSE_LEFT) && theEvent.Button == ST_MOUSE_LEFT) {
        // ignore out of window
        switch(aParams->ViewingMode) {
            default:
            case StStereoParams::FLAT_IMAGE: {
                if(!myIsClickAborted) {
                    aParams->moveFlat(getMouseMoveFlat(myClickPntZo, aCursor), GLfloat(getRectPx().ratio()));
                }
                break;
            }
            case StStereoParams::PANORAMA_CUBEMAP:
            case StStereoParams::PANORAMA_SPHERE: {
                aParams->moveSphere(getMouseMoveSphere(myClickPntZo, aCursor));
                break;
            }

        }
        theIsItemUnclicked = true;
        setClicked(ST_MOUSE_LEFT, false);
        return true;
    }
    return StGLWidget::tryUnClick(theEvent, theIsItemUnclicked);
}

bool StGLImageRegion::doScroll(const StScrollEvent& theEvent) {
    StHandle<StStereoParams> aParams = getSource();
    StPointD_t aCursor(theEvent.PointX, theEvent.PointY);
    if(!myIsInitialized
    ||  aParams.isNull()
    ||  theEvent.IsFromMultiTouch) {
        return false;
    }

    const GLfloat SCALE_STEPS = fabs(theEvent.DeltaY) * 0.01f;
    if(theEvent.DeltaY > 0.001f) {
        if((myKeyFlags & ST_VF_CONTROL) == ST_VF_CONTROL) {
            if((myKeyFlags & ST_VF_SHIFT) == ST_VF_SHIFT) {
                doParamsSepZDec(0.01);
            } else {
                doParamsSepX(size_t(-1));
            }
            return true;
        } else if((myKeyFlags & ST_VF_SHIFT) == ST_VF_SHIFT) {
            doParamsSepY(size_t(-1));
            return true;
        }

        scaleAt(aCursor, SCALE_STEPS);
    } else if(theEvent.DeltaY < -0.001f) {
        if((myKeyFlags & ST_VF_CONTROL) == ST_VF_CONTROL) {
            if((myKeyFlags & ST_VF_SHIFT) == ST_VF_SHIFT) {
                doParamsSepZInc(0.01);
            } else {
                doParamsSepX(size_t(1));
            }
            return true;
        } else if((myKeyFlags & ST_VF_SHIFT) == ST_VF_SHIFT) {
            doParamsSepY(size_t(1));
            return true;
        }

        scaleAt(aCursor, -SCALE_STEPS);
    }
    return true;
}

void StGLImageRegion::scaleAt(const StPointD_t& thePoint,
                              const float       theStep) {
    StHandle<StStereoParams> aParams = getSource();
    if(!myIsInitialized
    ||  aParams.isNull()) {
        return;
    }

    const StPointD_t aCenterCursor(0.5, 0.5);
    switch(aParams->ViewingMode) {
        default:
        case StStereoParams::FLAT_IMAGE: {
            if(theStep < 0.0f
            && aParams->ScaleFactor <= 0.05f) {
                break;
            }

            const StGLVec2 aVec = getMouseMoveFlat(aCenterCursor, thePoint) * (-theStep);
            if(theStep > 0.0f) {
                aParams->scaleIn(theStep);
            } else {
                aParams->scaleOut(std::abs(theStep));
            }
            aParams->moveFlat(aVec, GLfloat(getRectPx().ratio()));
            break;
        }
        case StStereoParams::PANORAMA_CUBEMAP:
        case StStereoParams::PANORAMA_SPHERE: {
            if(theStep < 0.0f
            && aParams->ScaleFactor <= 0.24f) {
                break;
            }

            const StGLVec2 aVec = getMouseMoveSphere(aCenterCursor, thePoint) * (-theStep);
            if(theStep > 0.0f) {
                aParams->scaleIn(theStep);
            } else {
                aParams->scaleOut(std::abs(theStep));
            }
            aParams->moveSphere(aVec);
            break;
        }
    }
}

bool StGLImageRegion::doGesture(const StGestureEvent& theEvent) {
    StHandle<StStereoParams> aParams = getSource();
    if(!myIsInitialized || aParams.isNull()) {
        return false;
    }

    switch(theEvent.Type) {
        case stEvent_GestureCancel: {
            myRotAngle = 0.0f;
            return false;
        }
        case stEvent_Gesture1DoubleTap: {
            doParamsReset(0);
            //aParams->ScaleFactor = 1.0f;
            return true;
        }
        case stEvent_Gesture2Rotate: {
            myRotAngle += theEvent.Value;
            if(myRotAngle >= 0.3f * M_PI) {
                doParamsRotZ90(1);
                myRotAngle -= 0.3f * float(M_PI);
            } else if(myRotAngle <= -0.3f * float(M_PI)) {
                doParamsRotZ90(size_t(-1));
                myRotAngle += 0.3f * float(M_PI);
            }
            return true;
        }
        case stEvent_Gesture2Move: {
            //if(!theEvent.OnScreen) {
                // this gesture conflicts with scrolling on OS X
                //return true;
            //}
            if(aParams->ViewingMode == StStereoParams::FLAT_IMAGE) {
                StPointD_t aPntFrom(theEvent.Point1X, theEvent.Point1Y);
                StPointD_t aPntTo  (theEvent.Point2X, theEvent.Point2Y);
                aParams->moveFlat(getMouseMoveFlat(aPntFrom, aPntTo), GLfloat(getRectPx().ratio()));
            }
            return true;
        }
        case stEvent_Gesture2Pinch: {
            StPointD_t aCursor((theEvent.Point1X + theEvent.Point2X) * 0.5,
                               (theEvent.Point1Y + theEvent.Point2Y) * 0.5);
            if(!theEvent.OnScreen) {
                aCursor = myRoot->getCursorZo();
            }
            scaleAt(aCursor, theEvent.Value * 0.01f);
            return true;
        }
        default: {
            return false;
        }
    }
}

bool StGLImageRegion::doKeyDown(const StKeyEvent& theEvent) {
    if(theEvent.VKey == ST_VK_CONTROL) {
        if((myKeyFlags & ST_VF_CONTROL) == ST_VF_CONTROL) {
            return false;
        }
        myKeyFlags = StVirtFlags(myKeyFlags | ST_VF_CONTROL);

        if(isClicked(ST_MOUSE_RIGHT) && !myToRightRotate) {
            myClickPntZo = myRoot->getCursorZo();
        }
    } else if(theEvent.VKey == ST_VK_SHIFT) {
        myKeyFlags = StVirtFlags(myKeyFlags | ST_VF_SHIFT);
    }
    return false;
}

bool StGLImageRegion::doKeyUp(const StKeyEvent& theEvent) {
    if(theEvent.VKey == ST_VK_CONTROL) {
        if((myKeyFlags & ST_VF_CONTROL) == 0) {
            return false;
        }

        myKeyFlags = StVirtFlags(myKeyFlags & ~ST_VF_CONTROL);

        if(isClicked(ST_MOUSE_RIGHT) && !myToRightRotate) {
            doRightUnclick(myRoot->getCursorZo());
        }
    } else if(theEvent.VKey == ST_VK_SHIFT) {
        myKeyFlags = StVirtFlags(myKeyFlags & ~ST_VF_SHIFT);
    }
    return false;
}

void StGLImageRegion::doParamsRotYLeft(const double ) {
    if(params.stereoFile.isNull()
    || params.stereoFile->ViewingMode != StStereoParams::FLAT_IMAGE) {
        return;
    }

    GLfloat anYRotate = params.stereoFile->getYRotate() + 1.0f;
    for(; anYRotate > 360.0f;) {
        anYRotate -= 360.0f;
    }
    params.stereoFile->setYRotate(anYRotate);
}

void StGLImageRegion::doParamsRotYRight(const double ) {
    if(params.stereoFile.isNull()
    || params.stereoFile->ViewingMode != StStereoParams::FLAT_IMAGE) {
        return;
    }

    GLfloat anYRotate = params.stereoFile->getYRotate() - 1.0f;
    for(; anYRotate < 0.0f;) {
        anYRotate += 360.0f;
    }
    params.stereoFile->setYRotate(anYRotate);
}

void StGLImageRegion::doParamsRotXUp(const double ) {
    if(params.stereoFile.isNull()
    || params.stereoFile->ViewingMode != StStereoParams::FLAT_IMAGE) {
        return;
    }

    GLfloat anXRotate = params.stereoFile->getXRotate() + 1.0f;
    for(; anXRotate > 360.0f;) {
        anXRotate -= 360.0f;
    }
    params.stereoFile->setXRotate(anXRotate);
}

void StGLImageRegion::doParamsRotXDown(const double ) {
    if(params.stereoFile.isNull()
    || params.stereoFile->ViewingMode != StStereoParams::FLAT_IMAGE) {
        return;
    }

    GLfloat anXRotate = params.stereoFile->getXRotate() - 1.0f;
    for(; anXRotate < 0.0f;) {
        anXRotate += 360.0f;
    }
    params.stereoFile->setXRotate(anXRotate);
}

void StGLImageRegion::doParamsReset(const size_t ) {
    if(!params.stereoFile.isNull()) {
        params.stereoFile->reset();
        onParamsChanged();
    }
}

void StGLImageRegion::onParamsChanged() {
    params.SwapLR       ->signals.onChanged(params.SwapLR->getValue());
    params.ViewMode     ->signals.onChanged(params.ViewMode->getValue());
    params.SeparationDX ->signals.onChanged(params.SeparationDX->getValue());
    params.SeparationDY ->signals.onChanged(params.SeparationDY->getValue());
    params.SeparationRot->signals.onChanged(params.SeparationRot->getValue());
}
