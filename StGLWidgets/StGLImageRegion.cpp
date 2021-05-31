/**
 * StGLWidgets, small C++ toolkit for writing GUI using OpenGL.
 * Copyright © 2010-2020 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#include <StGLWidgets/StGLImageRegion.h>
#include <StGLWidgets/StGLRootWidget.h>
#include <StGLWidgets/StGLTextureButton.h>
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
            aParams->ViewingMode = (StViewSurface )theValue;
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
                    setMinMaxValues(-500.0f, 500.0f);
                    setEffectiveMinMaxValues(-100.0f, 100.0f);
                    setStep(THE_SEP_STEP_PX);
                    setTolerance(0.5f);
                    break;
                }
                case StereoParamId_SepDY: {
                    setMinMaxValues(-500.0f, 500.0f);
                    setEffectiveMinMaxValues(-100.0f, 100.0f);
                    setStep(THE_SEP_STEP_PX);
                    setTolerance(0.5f);
                    break;
                }
                case StereoParamId_SepRot: {
                    setMinMaxValues(-180.0f, 180.0f);
                    setEffectiveMinMaxValues(-5.0f, 5.0f);
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
    static const float THE_PANORAMA_DEF_ZOOM = 0.45f; // circa 85 degrees FOV

    static const float THE_THEATER_ANGLE = float(M_PI * 0.5);
    static const float THE_THEATER_FROM  = float(M_PI) - THE_THEATER_ANGLE * 0.5f;
}

StGLImageRegion::StGLImageRegion(StGLWidget* theParent,
                                 const StHandle<StGLTextureQueue>& theTextureQueue,
                                 bool theUsePanningKeys)
: StGLWidget(theParent, 0, 0, StGLCorner(ST_VCORNER_TOP, ST_HCORNER_LEFT)),
  myIconPrev(NULL),
  myIconNext(NULL),
  myCube(GL_TRIANGLES),
  myCubePano(StPanorama_OFF),
  myUVSphere  (StGLVec3(0.0f, 0.0f, 0.0f), 1.0f, 64, false),
  myHemisphere(StGLVec3(0.0f, 0.0f, 0.0f), 1.0f, 64, true),
  myCylinder  (StGLVec3(0.0f, 0.0f, 0.0f), 1.0f, 1.0f, 64),
  myTheater   (StGLVec3(0.0f, 0.0f, 0.0f), 1.0f, 1.0f, THE_THEATER_FROM, THE_THEATER_FROM + THE_THEATER_ANGLE, 64),
  myTextureQueue(theTextureQueue),
  myClickPntZo(0.0, 0.0),
  myKeyFlags(ST_VF_NONE),
  myDragDelayMs(0.0),
  myDragDelayTmpMs(0.0),
  mySampleRatio(1.0f),
  myRotAngle(0.0f),
  myIsClickAborted(false),
#ifdef ST_EXTRA_CONTROLS
  myToRightRotate(true),
#else
  myToRightRotate(false),
#endif
  myIsInitialized(false),
  myHasVideoStream(false) {
    myIconPrev = new StGLIcon(this,  getRoot()->scale(48), 0, StGLCorner(ST_VCORNER_CENTER, ST_HCORNER_LEFT),  1);
    myIconPrev->setOpacity(0.0f, false);
    myIconNext = new StGLIcon(this, -getRoot()->scale(48), 0, StGLCorner(ST_VCORNER_CENTER, ST_HCORNER_RIGHT), 1);
    myIconNext->setOpacity(0.0f, false);

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
    params.TextureFilter->defineOption(StGLImageProgram::FILTER_NEAREST,   stCString("Nearest"));
    params.TextureFilter->defineOption(StGLImageProgram::FILTER_LINEAR,    stCString("Linear"));
    params.TextureFilter->defineOption(StGLImageProgram::FILTER_TRILINEAR, stCString("Trilinear"));
    params.TextureFilter->defineOption(StGLImageProgram::FILTER_BLEND,     stCString("Blend"));

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
    myCube.release(aCtx);
    myUVSphere.release(aCtx);
    myHemisphere.release(aCtx);
    myCylinder.release(aCtx);
    myTheater.release(aCtx);
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

void StGLImageRegion::stglUpdate(const StPointD_t& thePointZo,
                                 bool theIsPreciseInput) {
    StGLWidget::stglUpdate(thePointZo, theIsPreciseInput);
    if(myIsInitialized) {
        myHasVideoStream = myTextureQueue->stglUpdateStTextures(getContext()) || myTextureQueue->hasConnectedStream();
        StHandle<StStereoParams> aFileParams = myTextureQueue->getQTexture().getFront(StGLQuadTexture::LEFT_TEXTURE).getSource();
        if(params.stereoFile != aFileParams) {
            params.stereoFile = aFileParams;
            myFadeTimer.stop();
            onParamsChanged();
        } else if(!myHasVideoStream) {
            myFadeTimer.stop();
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
    }/* else if(!myUVSphere.initVBOs(aCtx)) {
        ST_ERROR_LOG("Fail to init StGLUVSphere");
    }*/

    // a unit cube for cubemap rendering
    if(!stglInitCube()) {
        return false;
    }

    // setup texture filter
    myTextureQueue->getQTexture().setMinMagFilter(aCtx,
                                                  params.TextureFilter->getValue() == StGLImageProgram::FILTER_NEAREST
                                                ? GL_NEAREST : GL_LINEAR);

    myIsInitialized = true;
    return myIsInitialized && isInit;
}

/**
 * Cubemap sides.
 */
enum StCubeSide {
    StCubeSide_PX = 0,
    StCubeSide_NX,
    StCubeSide_PY,
    StCubeSide_NY,
    StCubeSide_PZ,
    StCubeSide_NZ,
};
typedef StVec4<int> StIVec4;

bool StGLImageRegion::stglInitCube(const StGLVec4& theClamp,
                                   const StPanorama thePano) {
    myCubeClamp = theClamp;
    myCubePano = thePano;

    // a unit cube for cubemap rendering
    const StGLVec3 THE_VERTS[8] = {
        StGLVec3(-1.0f,-1.0f, 1.0f),
        StGLVec3( 1.0f,-1.0f, 1.0f),
        StGLVec3(-1.0f, 1.0f, 1.0f),
        StGLVec3( 1.0f, 1.0f, 1.0f),
        StGLVec3(-1.0f,-1.0f,-1.0f),
        StGLVec3( 1.0f,-1.0f,-1.0f),
        StGLVec3(-1.0f, 1.0f,-1.0f),
        StGLVec3( 1.0f, 1.0f,-1.0f)
    };

    const StIVec4 THE_SIDES[6] = {
        StIVec4(3, 7, 5, 1), // px
        StIVec4(6, 2, 0, 4), // nx
        StIVec4(2, 3, 7, 6), // py
        StIVec4(0, 1, 5, 4), // ny
        StIVec4(0, 1, 3, 2), // pz
        StIVec4(5, 4, 6, 7), // nz
    };

    StArrayList<StGLVec3>& aVertArr  = myCube.changeVertices();
    StArrayList<StGLVec3>& aCoordArr = myCube.changeNormals();
    StArrayList<StGLVec4>& aClampArr = myCube.changeColors();
    aVertArr.initArray(6 * 6);
    aCoordArr.initArray(aVertArr.size());
    aClampArr.initArray(aVertArr.size());
    int aVert = 0;
    for(int aSideIter = 0; aSideIter < 6; ++aSideIter, aVert += 6) {
        StIVec4 aSide = THE_SIDES[aSideIter];
        aVertArr[aVert + 0] = THE_VERTS[aSide[0]];
        aVertArr[aVert + 1] = THE_VERTS[aSide[1]];
        aVertArr[aVert + 2] = THE_VERTS[aSide[2]];

        aVertArr[aVert + 3] = THE_VERTS[aSide[0]];
        aVertArr[aVert + 4] = THE_VERTS[aSide[2]];
        aVertArr[aVert + 5] = THE_VERTS[aSide[3]];

        // rotate EAC cube sides
        if(thePano == StPanorama_Cubemap3_2ytb) {
            if(aSideIter == StCubeSide_NY) {
                const StIVec4 aCopy = aSide;
                for(int aSubIter = 0; aSubIter < 4; ++aSubIter) {
                    aSide[aSubIter] = aCopy[aSubIter > 0 ? aSubIter - 1 : 3];
                }
            }
            if(aSideIter == StCubeSide_NZ || aSideIter == StCubeSide_PY) {
                const StIVec4 aCopy = aSide;
                for(int aSubIter = 0; aSubIter < 4; ++aSubIter) {
                    aSide[aSubIter] = aCopy[aSubIter < 3 ? aSubIter + 1 : 0];
                }
            }
        } else if(thePano == StPanorama_Cubemap2_3ytb) {
            if(aSideIter == StCubeSide_PX
            || aSideIter == StCubeSide_NX) {
                const StIVec4 aCopy = aSide;
                for(int aSubIter = 0; aSubIter < 4; ++aSubIter) {
                    aSide[aSubIter] = aCopy[aSubIter < 3 ? aSubIter + 1 : 0];
                }
            }
            if(aSideIter == StCubeSide_PZ) {
                const StIVec4 aCopy = aSide;
                for(int aSubIter = 0; aSubIter < 4; ++aSubIter) {
                    aSide[aSubIter] = aCopy[aSubIter > 0 ? aSubIter - 1 : 3];
                }
            }
        }

        aCoordArr[aVert + 0] = THE_VERTS[aSide[0]];
        aCoordArr[aVert + 1] = THE_VERTS[aSide[1]];
        aCoordArr[aVert + 2] = THE_VERTS[aSide[2]];

        aCoordArr[aVert + 3] = THE_VERTS[aSide[0]];
        aCoordArr[aVert + 4] = THE_VERTS[aSide[2]];
        aCoordArr[aVert + 5] = THE_VERTS[aSide[3]];
        for(int aSubIter = 0; aSubIter < 6; ++aSubIter) {
            StGLVec4& aClamp = aClampArr[aVert + aSubIter];
            aClamp = StGLVec4(0.0f, 0.0f, 0.0f, theClamp.x());
            switch(aSideIter) {
                case StCubeSide_PX: {
                    aClamp.x() = 1.0f;
                    aClamp.y() = -theClamp.w();
                    aClamp.z() = -theClamp.z();
                    break;
                }
                case StCubeSide_NX: {
                    aClamp.x() = -1.0f;
                    aClamp.y() = -theClamp.w();
                    aClamp.z() = theClamp.z();
                    break;
                }
                case StCubeSide_PZ: {
                    aClamp.x() = -theClamp.z();
                    aClamp.y() = -theClamp.w();
                    aClamp.z() = 1.0f;
                    break;
                }
                case StCubeSide_NZ: {
                    aClamp.x() = theClamp.z();
                    aClamp.y() = -theClamp.w();
                    aClamp.z() = -1.0f;
                    break;
                }
                case StCubeSide_PY: {
                    aClamp.x() = theClamp.z();
                    aClamp.y() = 1.0f;
                    aClamp.z() = theClamp.w();
                    break;
                }
                case StCubeSide_NY: {
                    aClamp.x() = theClamp.z();
                    aClamp.y() = -1.0f;
                    aClamp.z() = -theClamp.w();
                    break;
                }
            }
        }
    }

    // triangle strip initialization
    /*aCubeVerts.initArray(8);
    for(int aVertIter = 0; aVertIter < 8; ++aVertIter) {
      aCubeVerts[aVertIter] = THE_VERTS[aVertIter];
    }
    const GLuint THE_BOX_TRISTRIP[14] = { 0, 1, 2, 3, 7, 1, 5, 4, 7, 6, 2, 4, 0, 1 };
    StArrayList<GLuint>& aCubeInd = myCube.changeIndices();
    aCubeInd.initArray(14);
    for(unsigned int aVertIter = 0; aVertIter < 14; ++aVertIter) {
        aCubeInd[aVertIter] = THE_BOX_TRISTRIP[aVertIter];
    }*/

    StGLContext& aCtx = getContext();
    if(!myCube.initVBOs(aCtx)) {
        aCtx.pushError(StString("Fail to init Cube Mesh"));
        ST_ERROR_LOG("Fail to init Cube Mesh");
        return false;
    }
    return true;
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

bool StGLImageRegion::getHeadOrientation(StGLQuaternion& theOrient,
                                         unsigned int theView,
                                         const bool theToApplyDefShift) const {
    StHandle<StStereoParams> aParams = params.stereoFile;
    if(aParams.isNull()
    || aParams->ViewingMode == StViewSurface_Plain) {
        theOrient = StGLQuaternion();
        return false;
    }

    const float aYawShift = theToApplyDefShift ? stToRadians(90.0f) : 0.0f;
    StGLVec2 aMouseMove = getMouseMoveSphere();
    float aYaw   = -stToRadians(aParams->getPanYaw() + aMouseMove.x()) + aYawShift;
    float aPitch =  stToRadians(StStereoParams::clipPitch(aParams->getPanPitch() + aMouseMove.y()));
    float aRoll  =  stToRadians(aParams->getZRotate());
    if(myProjCam.isCustomProjection()) {
        aPitch = 0.0f; // ignore pitch for HMD
    }

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
    theOrient = anOri;
    return true;
}

void StGLImageRegion::stglDraw(unsigned int theView) {
    myIconPrev->setOpacity(0.0f, false);
    myIconNext->setOpacity(0.0f, false);
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
    mySampleRatio = 1.0f;
    myFrameSize = StVec2<int>(0, 0);
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

    // set 85 degrees FOV as 1.0x zoom for panorama rendering
    myProjCam = *getCamera();
    myProjCam.resize(aCameraAspect);
    myProjCam.setFOVy(85.0f);

    aCtx.core20fwd->glDisable(GL_BLEND);

    StGLFrameTextures& aTextures = myTextureQueue->getQTexture().getFront(aLeftOrRight);

    StGLVec2 aTextureSize  (GLfloat(aTextures.getPlane(0).getSizeX()),
                            GLfloat(aTextures.getPlane(0).getSizeY()));
    StGLVec2 aTextureUVSize(GLfloat(aTextures.getPlane(1).getSizeX()),
                            GLfloat(aTextures.getPlane(1).getSizeY()));
    StGLVec2 aTextureASize (GLfloat(aTextures.getPlane(3).getSizeX()),
                            GLfloat(aTextures.getPlane(3).getSizeY()));
    StGLMatrix aModelMat;
    // data rectangle in the texture
    StGLVec4 aClampVec(0.0f, 0.0f, 1.0f, 1.0f), aClampUV(0.0f, 0.0f, 1.0f, 1.0f), aClampA(0.0f, 0.0f, 1.0f, 1.0f);
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
        // Alpha
        aClampA.x() = 0.0f;
        aClampA.y() = 0.0f;
        aClampA.z() = aTextures.getPlane(3).getDataSize().x();
        aClampA.w() = aTextures.getPlane(3).getDataSize().y();
    } else {
        if(params.TextureFilter->getValue() == StGLImageProgram::FILTER_TRILINEAR) {
            myTextureQueue->getQTexture().setMinMagFilter(aCtx, GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR);
        } else {
            myTextureQueue->getQTexture().setMinMagFilter(aCtx, GL_LINEAR);
        }

        // clamping is undesired for full-range cubemap to allow smooth seamless cube boundaries,
        // but EAC video normally defines non-square cube sides requiring clamping
        const bool isCubemapFull = aTextures.getPlane().getTarget() == GL_TEXTURE_CUBE_MAP
                                && aTextures.getPlane().getPackedPanorama() != StPanorama_Cubemap3_2ytb
                                && aTextures.getPlane().getPackedPanorama() != StPanorama_Cubemap2_3ytb;
        if(!isCubemapFull
         || aTextures.getPlane(0).getDataSize().x() != aTextures.getPlane(0).getDataSize().y()
         || aTextures.getPlane(0).getDataSize().x() != 1.0f) {
            aClampVec.x() = 0.5f / aTextureSize.x();
            aClampVec.y() = 0.5f / aTextureSize.y();
            aClampVec.z() = aTextures.getPlane(0).getDataSize().x() - 2.0f * aClampVec.x();
            aClampVec.w() = aTextures.getPlane(0).getDataSize().y() - 2.0f * aClampVec.y();
        }
        // UV
        if(aTextureUVSize.x() > 0.0f
        && aTextureUVSize.y() > 0.0f) {
            if(!isCubemapFull
             || aTextures.getPlane(1).getDataSize().x() != aTextures.getPlane(1).getDataSize().y()
             || aTextures.getPlane(1).getDataSize().x() != 1.0f) {
                aClampUV.x() = 0.5f / aTextureUVSize.x();
                aClampUV.y() = 0.5f / aTextureUVSize.y(),
                aClampUV.z() = aTextures.getPlane(1).getDataSize().x() - 2.0f * aClampUV.x();
                aClampUV.w() = aTextures.getPlane(1).getDataSize().y() - 2.0f * aClampUV.y();
            }
        }
        // Alpha
        if(aTextureASize.x() > 0.0f
        && aTextureASize.y() > 0.0f) {
            if(!isCubemapFull
             || aTextures.getPlane(3).getDataSize().x() != aTextures.getPlane(3).getDataSize().y()
             || aTextures.getPlane(3).getDataSize().x() != 1.0f) {
                aClampA.x() = 0.5f / aTextureASize.x();
                aClampA.y() = 0.5f / aTextureASize.y(),
                aClampA.z() = aTextures.getPlane(3).getDataSize().x() - 2.0f * aClampA.x();
                aClampA.w() = aTextures.getPlane(3).getDataSize().y() - 2.0f * aClampA.y();
            }
        }
    }
    aTextures.bind(aCtx);

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

    StViewSurface aViewMode = aParams->ViewingMode;
    if(aTextures.getPlane(0).getTarget() == GL_TEXTURE_CUBE_MAP) {
        if(aViewMode != StViewSurface_Cubemap && aViewMode != StViewSurface_CubemapEAC) {
            aViewMode = aTextures.getPlane().getPackedPanorama() == StPanorama_Cubemap3_2ytb
                     || aTextures.getPlane().getPackedPanorama() == StPanorama_Cubemap2_3ytb
                      ? StViewSurface_CubemapEAC
                      : StViewSurface_Cubemap;
        }
    } else if(aViewMode == StViewSurface_Cubemap || aViewMode == StViewSurface_CubemapEAC) {
        aViewMode = StViewSurface_Plain;
    }

    // setup scissor box
    StGLBoxPx aScissorBox;
    const StRectI_t aFrameRectAbs = getAbsolute(aFrameRectPx);
    if(aViewMode == StViewSurface_Plain) {
        getRoot()->stglScissorRect2d(aFrameRectAbs, aScissorBox);
    } else {
        getRoot()->stglScissorRect3d(aFrameRectAbs, aScissorBox);
    }
    aCtx.stglSetScissorRect(aScissorBox, true);
    aCtx.stglResizeViewport(aScissorBox);

    myFrameSize.x() = int(double(aTextures.getPlane().getDataSize().x()) * double(aTextures.getPlane().getSizeX()));
    myFrameSize.y() = int(double(aTextures.getPlane().getDataSize().y()) * double(aTextures.getPlane().getSizeY()));
    mySampleRatio = aTextures.getPlane().getPixelRatio();

    myProgram.setColorScale(aColorScale); // apply de-anaglyph color filter
    StGLImageProgram::FragGetColor aColorGetter = params.TextureFilter->getValue() == StGLImageProgram::FILTER_BLEND
                                                ? StGLImageProgram::FragGetColor_Blend
                                                : StGLImageProgram::FragGetColor_Normal;
    const bool toDragImageInSwipe = getRectPx().ratio() < 0.75; // drag image only in portrait mode while swiping
    switch(aViewMode) {
        case StViewSurface_Plain: {
            const float aBrightnessBack = params.Brightness->getValue();
            const double aDragDelayMs = myDragDelayMs > 0.0 ? myDragDelayMs : myDragDelayTmpMs;
            if(isClicked(ST_MOUSE_LEFT)) {
                // handle dragging timer
                if(!myIsClickAborted
                &&  myClickTimer.isOn()) {
                    if(myClickTimer.getElapsedTimeInMilliSec() < aDragDelayMs) {
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
                if(!myIsClickAborted
                && !myClickTimer.isOn()) {
                    // panning
                    const GLfloat aRectRatio = GLfloat(getRectPx().ratio());
                    aParams->moveFlat(getMouseMoveFlat(myClickPntZo, getRoot()->getCursorZo()), aRectRatio);
                    if(aDragDelayMs > 1.0) {
                        const GLfloat    aScaleSteps = 0.1f;
                        const StPointD_t aCenterCursor(0.5, 0.5);
                        const StGLVec2   aVec = getMouseMoveFlat(aCenterCursor, getRoot()->getCursorZo()) * (-aScaleSteps);
                        aParams->scaleIn(aScaleSteps);
                        aParams->moveFlat(aVec, aRectRatio);
                    }
                } else if(!myList.isNull()) {
                    // previous / next swipe gesture
                    const double anIntensMult = toDragImageInSwipe ? 1.0 : 3.0;
                    const StPlayList::CurrentPosition aPos = myList->getCurrentPosition();
                    const double aMouseDX = myClickPntZo.x() - getRoot()->getCursorZo().x();
                    const float aScaleSteps = (float )stMin(std::abs(aMouseDX) * anIntensMult, 1.0);
                    if(aMouseDX < 0.0) {
                        // previous
                        if(aPos == StPlayList::CurrentPosition_Middle
                        || aPos == StPlayList::CurrentPosition_Last) {
                            //params.Brightness->setValue(aBrightnessBack - aScaleSteps);
                            //aParams->scaleIn(-aScaleSteps);
                            if(toDragImageInSwipe) {
                                StGLVec2 aVec = getMouseMoveFlat(myClickPntZo, getRoot()->getCursorZo());
                                aVec.y() = 0.0;
                                aParams->moveFlat(aVec, float(getRectPx().ratio()));
                            }
                            myIconPrev->setOpacity(aScaleSteps, false);
                        }
                    } else {
                        // next
                        if(aPos == StPlayList::CurrentPosition_Middle
                        || aPos == StPlayList::CurrentPosition_First) {
                            if(toDragImageInSwipe) {
                                StGLVec2 aVec = getMouseMoveFlat(myClickPntZo, getRoot()->getCursorZo());
                                aVec.y() = 0.0;
                                aParams->moveFlat(aVec, float(getRectPx().ratio()));
                            }
                            myIconNext->setOpacity(aScaleSteps, false);
                        }
                    }
                }
            } else if(myFadeTimer.isOn()) {
                static const double THE_FADE_ANIM_MS = 1000.0;
                const double aProgress  = myFadeTimer.getElapsedTimeInMilliSec() / THE_FADE_ANIM_MS;
                const double aFadeClamp = stMin(1.0, aProgress);
                const bool isNext = myFadeFrom.x() < myClickPntZo.x();
                if(toDragImageInSwipe) {
                    StPointD_t aFadeTo = myFadeFrom;
                    aFadeTo.x() += isNext ? -aFadeClamp : aFadeClamp;

                    StGLVec2 aVec = getMouseMoveFlat(myClickPntZo, aFadeTo);
                    aVec.y() = 0.0;
                    aParams->moveFlat(aVec, float(getRectPx().ratio()));
                }
                params.Brightness->setValue(aBrightnessBack - (float )aFadeClamp * 3.0f);

                // animate icon opacity in loop
                const double anIntensMult  = toDragImageInSwipe ? 1.0 : 3.0;
                const double anOpacityFrom = stMin(std::abs(myClickPntZo.x() - myFadeFrom.x()) * anIntensMult, 1.0);
                double anIconProgress = anOpacityFrom + aProgress;
                const bool isEven = (int(anIconProgress) % 2) == 0;
                anIconProgress -= int(anIconProgress);
                if(!isEven) { anIconProgress = 1.0 - anIconProgress; }
                StGLIcon* anIcon = isNext ? myIconNext : myIconPrev;
                anIcon->setOpacity((float )anIconProgress, false);
            }

            GLfloat anXRotate = aParams->getXRotate();
            GLfloat anYRotate = aParams->getYRotate();
            if(isClicked(ST_MOUSE_RIGHT)
            && (myToRightRotate || (myKeyFlags & ST_VF_CONTROL) == ST_VF_CONTROL)) {
                anXRotate += 180.0f * GLfloat(myRoot->getCursorZo().y() - myClickPntZo.y());
                anYRotate += 180.0f * GLfloat(myRoot->getCursorZo().x() - myClickPntZo.x());
            }

            // apply scale
            const float aVrScale = float(myRoot->getVrZoomOut());
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
                                    mySampleRatio *= 2.0f;
                                }
                                break;
                             }
                             case StFormat_TopBottom_LR:
                             case StFormat_TopBottom_RL: {
                                if(aDispRatio >= 3.5 && aDispRatio <= 4.8) {
                                    aDispRatio *= 0.5;
                                    mySampleRatio *= 0.5f;
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

            if(myProgram.init(aCtx, aTextures.getColorModel(), aTextures.getColorScale(), aColorGetter)) {
                myProgram.getActiveProgram()->use(aCtx);

                // setup data rectangle in the texture
                myProgram.setTextureSizePx      (aCtx, aTextureSize);
                myProgram.setTextureMainDataSize(aCtx, aClampVec);
                myProgram.setTextureUVDataSize  (aCtx, aClampUV);
                myProgram.setTextureADataSize   (aCtx, aClampA);

                StGLMatrix anOrthoMat;
                anOrthoMat.initOrtho(StGLVolume(-aRectRatio * aFrustrumL, aRectRatio * aFrustrumR,
                                                -1.0f       * aFrustrumB, 1.0f       * aFrustrumT,
                                                -1.0f, 1.0f));
                myProgram.getActiveProgram()->setProjMat (aCtx, anOrthoMat);
                myProgram.getActiveProgram()->setModelMat(aCtx, aModelMat);

                myQuad.draw(aCtx, *myProgram.getActiveProgram());

                myProgram.getActiveProgram()->unuse(aCtx);
            }

            // restore changed parameters
            aParams->ScaleFactor = aScaleBack;
            aParams->PanCenter   = aPanBack;
            params.Brightness->setValue(aBrightnessBack);
            break;
        }
        case StViewSurface_Cubemap:
        case StViewSurface_CubemapEAC: {
            // Clamping range is passed through vertex attributes.
            // To avoid extra vertex attributes, the smallest clamping range is used across planes.
            // This trick is possible, because cubemap texture lazy resizing is not used,
            // hence all planes should have same proportions.
            // The result will be broken for rare formats like YUV422P/YUV411P/YUV440P.
            StGLVec4 aClamVecMin = aClampVec;
            if(aTextureUVSize.x() > 0.0f
            && aTextureUVSize.y() > 0.0f) {
                aClamVecMin.x() = stMax(aClamVecMin.x(), aClampUV.x());
                aClamVecMin.y() = stMax(aClamVecMin.y(), aClampUV.y());
                aClamVecMin.z() = stMin(aClamVecMin.z(), aClampUV.z());
                aClamVecMin.w() = stMin(aClamVecMin.w(), aClampUV.w());
            }
            if(aTextureASize.x() > 0.0f
            && aTextureASize.y() > 0.0f) {
                aClamVecMin.x() = stMax(aClamVecMin.x(), aClampA.x());
                aClamVecMin.y() = stMax(aClamVecMin.y(), aClampA.y());
                aClamVecMin.z() = stMin(aClamVecMin.z(), aClampA.z());
                aClamVecMin.w() = stMin(aClamVecMin.w(), aClampA.w());
            }

            if(myCubeClamp != aClamVecMin
            || myCubePano != aTextures.getPlane().getPackedPanorama()) {
                stglInitCube(aClamVecMin, aTextures.getPlane().getPackedPanorama());
            }

            if(!myProgram.init(aCtx, aTextures.getColorModel(), aTextures.getColorScale(),
                               StGLImageProgram::FragGetColor_Cubemap,
                               aViewMode == StViewSurface_CubemapEAC ? StGLImageProgram::FragTexEAC_On : StGLImageProgram::FragTexEAC_Off)) {
                break;
            }

            myProgram.getActiveProgram()->use(aCtx);

            // setup data rectangle in the texture
            myProgram.setTextureSizePx      (aCtx, aTextureSize);
            myProgram.setTextureMainDataSize(aCtx, aClampVec);
            myProgram.setTextureUVDataSize  (aCtx, aClampUV);
            myProgram.setTextureADataSize   (aCtx, aClampA);
            myProgram.setCubeTextureFlipZ   (aCtx, aParams->ToFlipCubeZ);

            const float aScale = aParams->ScaleFactor;
            aModelMat.scale(aScale, aScale, 1.0f);

            // compute orientation
            const StGLQuaternion anOri = getHeadOrientation(theView, true);
            aModelMat = StGLMatrix::multiply(aModelMat, StGLMatrix(anOri));
            myProgram.getActiveProgram()->setModelMat(aCtx, aModelMat);

            if(myProjCam.isCustomProjection()) {
                myProgram.getActiveProgram()->setProjMat (aCtx, myProjCam.getProjMatrix());
            } else {
                myProgram.getActiveProgram()->setProjMat (aCtx, myProjCam.getProjMatrixMono());
            }

        #if !defined(GL_ES_VERSION_2_0)
            // Issues with seamless cubemap filtering:
            // - Desktop, available since OpenGL 3.2+ (or ARB_seamless_cube_map) and DISABLED by default;
            //   some old implementations might be buggy;
            //   some very old implementation might even switch to software fallback.
            // - Mobile, OpenGL ES 3.0 requires cubemap filtering to be always ENABLED - no API for disabling;
            //   OpenGL ES 3.0 implementations might not support seamless filtering without a way to detect it.
            // - Seamless filtering works is desired for a proper cubemap definition,
            //   but undesired for EAC video frames with non-square cube sides and has rotated sides.
            if(aCtx.isGlGreaterEqual(3, 2)) {
                ///glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
            }
        #endif
            myCube.draw(aCtx, *myProgram.getActiveProgram());

            myProgram.getActiveProgram()->unuse(aCtx);

            // restore changed parameters
            aParams->ScaleFactor = aScaleBack;
            aParams->PanCenter   = aPanBack;
            break;
        }
        case StViewSurface_Cylinder:
        case StViewSurface_Theater:
        case StViewSurface_Hemisphere:
        case StViewSurface_Sphere: {
            if(!myProgram.init(aCtx, aTextures.getColorModel(), aTextures.getColorScale(), aColorGetter)) {
                break;
            }

            StGLMesh* aMesh = &myUVSphere;
            if(aViewMode == StViewSurface_Hemisphere) {
                aMesh = &myHemisphere;
            } else if(aViewMode == StViewSurface_Cylinder) {
                aMesh = &myCylinder;
                const StGLVec2 aSrcSize (aTextures.getPlane().getDataSize().x() * (float )aTextures.getPlane().getSizeX(),
                                         aTextures.getPlane().getDataSize().y() * (float )aTextures.getPlane().getSizeY());
                float aCylHeight = float(2.0 * M_PI * aSrcSize.y()) / aSrcSize.x();
                //aVertScale *= aTextures.getPlane().getDisplayRatio();
                if(myCylinder.getHeight() != aCylHeight) {
                    myCylinder.setHeight(aCylHeight);
                    myCylinder.release(aCtx);
                }
            } else if(aViewMode == StViewSurface_Theater) {
                aMesh = &myTheater;
                const float aCylWidth = float(M_PI * myTheater.getRadius() / myTheater.getAngle());
                const float aCylHeight = (aCylWidth / aTextures.getPlane().getDisplayRatio()) * 0.75f;
                if(myTheater.getHeight() != aCylHeight) {
                    myTheater.setHeight(aCylHeight);
                    myTheater.release(aCtx);
                }
            }
            if(!aMesh->changeVBO(ST_VBO_VERTEX)->isValid()) {
                if(!aMesh->initVBOs(aCtx)) {
                    aCtx.pushError(StString("Fail to init mesh data"));
                    ST_ERROR_LOG("Fail to init mesh data");
                    break;
                }
            }

            // perform scaling
            const float aScale = THE_SPHERE_RADIUS * aParams->ScaleFactor;
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
            myProgram.setTextureADataSize   (aCtx, aClampA);

            myProgram.getActiveProgram()->setModelMat(aCtx, aModelMat);
            if(myProjCam.isCustomProjection()) {
                myProgram.getActiveProgram()->setProjMat (aCtx, myProjCam.getProjMatrix());
            } else {
                myProgram.getActiveProgram()->setProjMat (aCtx, myProjCam.getProjMatrixMono());
            }

            aMesh->draw(aCtx, *myProgram.getActiveProgram());

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
     || aParams->ViewingMode != StViewSurface_Plain) {
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
    const bool hasImage = !aParams.isNull()
                        && myHasVideoStream
                        && myTextureQueue->getQTexture().getFront(StGLQuadTexture::LEFT_TEXTURE).isValid();
    if(!myIsInitialized || !hasImage) {
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

            myDragDelayTmpMs = 0.0f;
            if(aParams->ViewingMode == StViewSurface_Plain
            && fabs(aParams->ScaleFactor - 1.0f) < 0.00001f
            && fabs(aParams->PanCenter.x()) < 0.00001f
            && fabs(aParams->PanCenter.y()) < 0.00001f) {
                myDragDelayTmpMs = 250.0;
            }
        }
        theIsItemClicked = true;
        return true;
    }
    return false;
}

bool StGLImageRegion::tryUnClick(const StClickEvent& theEvent,
                                 bool&               theIsItemUnclicked) {
    StHandle<StStereoParams> aParams = getSource();
    const bool hasImage = !aParams.isNull()
                        && myHasVideoStream
                        && myTextureQueue->getQTexture().getFront(StGLQuadTexture::LEFT_TEXTURE).isValid();
    if(!myIsInitialized || !hasImage) {
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
            case StViewSurface_Plain: {
                if(!myIsClickAborted) {
                    aParams->moveFlat(getMouseMoveFlat(myClickPntZo, aCursor), GLfloat(getRectPx().ratio()));
                } else if(!myList.isNull()) {
                    // previous / next swipe gesture
                    StPlayList::CurrentPosition aPos = myList->getCurrentPosition();
                    const double aMouseDX = myClickPntZo.x() - getRoot()->getCursorZo().x();
                    if(std::abs(aMouseDX) >= 0.25) {
                        if(aMouseDX < 0.0) {
                            if(aPos == StPlayList::CurrentPosition_Middle
                            || aPos == StPlayList::CurrentPosition_Last) {
                                if(myList->walkToPrev()) {
                                    signals.onOpenItem();
                                    myFadeTimer.restart();
                                    myFadeFrom = getRoot()->getCursorZo();
                                }
                            }
                        } else {
                            if(aPos == StPlayList::CurrentPosition_Middle
                            || aPos == StPlayList::CurrentPosition_First) {
                                if(myList->walkToNext()) {
                                    signals.onOpenItem();
                                    myFadeTimer.restart();
                                    myFadeFrom = getRoot()->getCursorZo();
                                }
                            }
                        }
                    }
                }
                break;
            }
            case StViewSurface_Cubemap:
            case StViewSurface_CubemapEAC:
            case StViewSurface_Cylinder:
            case StViewSurface_Theater:
            case StViewSurface_Hemisphere:
            case StViewSurface_Sphere: {
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

    if((myKeyFlags & ST_VF_CONTROL) == ST_VF_CONTROL) {
        if(theEvent.StepsY == 0) {
            return false;
        }
        if((myKeyFlags & ST_VF_SHIFT) == ST_VF_SHIFT) {
            if(theEvent.StepsY > 0) {
                doParamsSepZDec(0.01);
            } else {
                doParamsSepZInc(0.01);
            }
        } else {
            doParamsSepX(theEvent.StepsY > 0 ? size_t(-1) : size_t(1));
        }
        return true;
    } else if((myKeyFlags & ST_VF_SHIFT) == ST_VF_SHIFT) {
        if(theEvent.StepsY == 0) {
            return false;
        }
        doParamsSepY(theEvent.StepsY > 0 ? size_t(-1) : size_t(1));
        return true;
    }

    const GLfloat SCALE_STEPS = fabs(theEvent.DeltaY) * 0.1f;
    if(theEvent.DeltaY > 0.0001f) {
        scaleAt(aCursor, SCALE_STEPS);
    } else if(theEvent.DeltaY < -0.0001f) {
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
        case StViewSurface_Plain: {
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
        case StViewSurface_Cubemap:
        case StViewSurface_CubemapEAC:
        case StViewSurface_Cylinder:
        case StViewSurface_Theater:
        case StViewSurface_Hemisphere:
        case StViewSurface_Sphere: {
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
            if(aParams->ViewingMode == StViewSurface_Plain) {
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
    || params.stereoFile->ViewingMode != StViewSurface_Plain) {
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
    || params.stereoFile->ViewingMode != StViewSurface_Plain) {
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
    || params.stereoFile->ViewingMode != StViewSurface_Plain) {
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
    || params.stereoFile->ViewingMode != StViewSurface_Plain) {
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
