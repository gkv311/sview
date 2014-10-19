/**
 * Copyright © 2010-2014 Kirill Gavrilov <kirill@sview.ru>
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

    static const StString THE_CLASS_NAME("StGLImageRegion");

    class ST_LOCAL StTrackedFloatParam : public StFloat32Param {

            private:

        StHandle<StFloat32Param> myTracked1;
        StHandle<StFloat32Param> myTracked2;

            public:

        StTrackedFloatParam(const StHandle<StFloat32Param>& theTracked1,
                            const StHandle<StFloat32Param>& theTracked2)
        : StFloat32Param(theTracked1->getValue(),
                         theTracked1->getMinValue(),
                         theTracked1->getMaxValue(),
                         theTracked1->getDefValue(),
                         theTracked1->getStep(),
                         theTracked1->getTolerance()),
          myTracked1(theTracked1),
          myTracked2(theTracked2) {}

        virtual float getValue() const {
            return myTracked1->getValue();
        }

        virtual bool setValue(const float theValue) {
            if(myTracked1->setValue(theValue)
            || myTracked2->setValue(theValue)) {
                signals.onChanged(theValue);
                return true;
            }
            return false;
        }

    };

    class ST_LOCAL StSwapLRParam : public StBoolParam {

            public:

        StSwapLRParam() : StBoolParam(false) {}

        virtual bool getValue() const {
            return !myTrackedParams.isNull()
                 && myTrackedParams->ToSwapLR;
        }

        virtual bool setValue(const bool theValue) {
            if(myTrackedParams.isNull()
            || myTrackedParams->ToSwapLR == theValue) {
                return false;
            }
            myTrackedParams->setSwapLR(theValue);
            return true;
        }

        void setTrackedHandle(const StHandle<StStereoParams>& theTrackedParams) {
            myTrackedParams = theTrackedParams;
        }

            private:

        StHandle<StStereoParams> myTrackedParams;

    };

    class ST_LOCAL StViewModeParam : public StInt32Param {

            public:

        StViewModeParam() : StInt32Param(0) {}

        virtual int32_t getValue() const {
            return myTrackedParams.isNull() ? 0 : myTrackedParams->ViewingMode;
        }

        virtual bool setValue(const int32_t theValue) {
            if(myTrackedParams.isNull()
            || myTrackedParams->ViewingMode == theValue) {
                return false;
            }
            myTrackedParams->ViewingMode = (StStereoParams::ViewMode )theValue;
            return true;
        }

        void setTrackedHandle(const StHandle<StStereoParams>& theTrackedParams) {
            myTrackedParams = theTrackedParams;
        }

            private:

        StHandle<StStereoParams> myTrackedParams;

    };

    // we use negative scale factor to show sphere inside out!
    static const GLfloat SPHERE_RADIUS = -10.0f;

}

StGLImageRegion::StGLImageRegion(StGLWidget* theParent,
                                 const StHandle<StGLTextureQueue>& theTextureQueue,
                                 const bool  theUsePanningKeys)
: StGLWidget(theParent, 0, 0, StGLCorner(ST_VCORNER_TOP, ST_HCORNER_LEFT)),
  myQuad(),
  myUVSphere(StGLVec3(0.0f, 0.0f, 0.0f), 1.0f, 64),
  myProgramFlat(),
  myProgramSphere(),
  myTextureQueue(theTextureQueue),
  myClickPntZo(0.0, 0.0),
  myDragDelayMs(0.0),
  myIsClickAborted(false),
  myIsInitialized(false),
  myHasVideoStream(false) {
    params.displayMode   = new StInt32Param(MODE_STEREO);
    params.displayRatio  = new StInt32Param(RATIO_AUTO);
    params.textureFilter = new StInt32Param(StGLImageProgram::FILTER_LINEAR);
    params.gamma      = new StTrackedFloatParam(myProgramFlat.params.gamma,
                                                myProgramSphere.params.gamma);
    params.brightness = new StTrackedFloatParam(myProgramFlat.params.brightness,
                                                myProgramSphere.params.brightness);
    params.saturation = new StTrackedFloatParam(myProgramFlat.params.saturation,
                                                myProgramSphere.params.saturation);
    params.swapLR   = new StSwapLRParam();
    params.ViewMode = new StViewModeParam();

    // create actions
    StHandle<StAction> anAction;
    anAction = new StActionIntSlot(stCString("DoParamsReset"), stSlot(this, &StGLImageRegion::doParamsReset), 0);
    anAction->setHotKey1(ST_VK_BACK);
    myActions.add(anAction);

    anAction = new StActionBool(stCString("DoParamsSwapLR"), params.swapLR);
    anAction->setHotKey1(ST_VK_W);
    myActions.add(anAction);

    anAction = new StActionIntSlot(stCString("DoParamsGammaDec"), stSlot(this, &StGLImageRegion::doParamsGamma), (size_t )-1);
    anAction->setHotKey1(ST_VK_G | ST_VF_CONTROL);
    myActions.add(anAction);

    anAction = new StActionIntSlot(stCString("DoParamsGammaInc"), stSlot(this, &StGLImageRegion::doParamsGamma), 1);
    anAction->setHotKey1(ST_VK_G | ST_VF_SHIFT);
    myActions.add(anAction);

    anAction = new StActionIntSlot(stCString("DoParamsSepXDec"), stSlot(this, &StGLImageRegion::doParamsSepX), (size_t )-1);
    anAction->setHotKey1(ST_VK_COMMA);
    anAction->setHotKey1(ST_VK_DIVIDE);
    myActions.add(anAction);

    anAction = new StActionIntSlot(stCString("DoParamsSepXInc"), stSlot(this, &StGLImageRegion::doParamsSepX), 1);
    anAction->setHotKey1(ST_VK_PERIOD);
    anAction->setHotKey2(ST_VK_MULTIPLY);
    myActions.add(anAction);

    anAction = new StActionIntSlot(stCString("DoParamsSepYDec"), stSlot(this, &StGLImageRegion::doParamsSepY), (size_t )-1);
    anAction->setHotKey1(ST_VK_COMMA  | ST_VF_CONTROL);
    anAction->setHotKey1(ST_VK_DIVIDE | ST_VF_CONTROL);
    myActions.add(anAction);

    anAction = new StActionIntSlot(stCString("DoParamsSepYInc"), stSlot(this, &StGLImageRegion::doParamsSepY), 1);
    anAction->setHotKey1(ST_VK_PERIOD   | ST_VF_CONTROL);
    anAction->setHotKey2(ST_VK_MULTIPLY | ST_VF_CONTROL);
    myActions.add(anAction);

    anAction = new StActionHoldSlot(stCString("DoParamsSepRotDec"), stSlot(this, &StGLImageRegion::doParamsSepZDec));
    anAction->setHotKey1(ST_VK_APOSTROPHE | ST_VF_CONTROL);
    myActions.add(anAction);

    anAction = new StActionHoldSlot(stCString("DoParamsSepRotInc"), stSlot(this, &StGLImageRegion::doParamsSepZInc));
    anAction->setHotKey1(ST_VK_SEMICOLON | ST_VF_CONTROL);
    myActions.add(anAction);

    anAction = new StActionIntSlot(stCString("DoParamsRotZ90Dec"), stSlot(this, &StGLImageRegion::doParamsRotZ90), (size_t )-1);
    anAction->setHotKey1(ST_VK_BRACKETLEFT);
    myActions.add(anAction);

    anAction = new StActionIntSlot(stCString("DoParamsRotZ90Inc"), stSlot(this, &StGLImageRegion::doParamsRotZ90), 1);
    anAction->setHotKey1(ST_VK_BRACKETRIGHT);
    myActions.add(anAction);

    anAction = new StActionHoldSlot(stCString("DoParamsRotZDec"), stSlot(this, &StGLImageRegion::doParamsRotZLeft));
    anAction->setHotKey1(ST_VK_BRACKETLEFT | ST_VF_CONTROL);
    myActions.add(anAction);

    anAction = new StActionHoldSlot(stCString("DoParamsRotZInc"), stSlot(this, &StGLImageRegion::doParamsRotZRight));
    anAction->setHotKey1(ST_VK_BRACKETRIGHT | ST_VF_CONTROL);
    myActions.add(anAction);

    anAction = new StActionIntSlot(stCString("DoParamsModeNext"), stSlot(this, &StGLImageRegion::doParamsModeNext), 0);
    anAction->setHotKey1(ST_VK_P);
    myActions.add(anAction);

    anAction = new StActionHoldSlot(stCString("DoParamsPanLeft"), stSlot(this, &StGLImageRegion::doParamsPanLeft));
    anAction->setHotKey1(theUsePanningKeys ? ST_VK_LEFT : 0);
    myActions.add(anAction);

    anAction = new StActionHoldSlot(stCString("DoParamsPanRight"), stSlot(this, &StGLImageRegion::doParamsPanRight));
    anAction->setHotKey1(theUsePanningKeys ? ST_VK_RIGHT : 0);
    myActions.add(anAction);

    anAction = new StActionHoldSlot(stCString("DoParamsPanUp"), stSlot(this, &StGLImageRegion::doParamsPanUp));
    anAction->setHotKey1(theUsePanningKeys ? ST_VK_UP : 0);
    myActions.add(anAction);

    anAction = new StActionHoldSlot(stCString("DoParamsPanDown"), stSlot(this, &StGLImageRegion::doParamsPanDown));
    anAction->setHotKey1(theUsePanningKeys ? ST_VK_DOWN : 0);
    myActions.add(anAction);

    anAction = new StActionHoldSlot(stCString("DoParamsScaleIn"), stSlot(this, &StGLImageRegion::doParamsScaleIn));
    anAction->setHotKey1(ST_VK_ADD);
    anAction->setHotKey2(ST_VK_OEM_PLUS);
    myActions.add(anAction);

    anAction = new StActionHoldSlot(stCString("DoParamsScaleOut"), stSlot(this, &StGLImageRegion::doParamsScaleOut));
    anAction->setHotKey1(ST_VK_SUBTRACT);
    anAction->setHotKey2(ST_VK_OEM_MINUS);
    myActions.add(anAction);
}

StGLImageRegion::~StGLImageRegion() {
    // make sure GL objects are released within GL thread
    StGLContext& aCtx = getContext();
    myTextureQueue->getQTexture().release(aCtx);
    myQuad.release(aCtx);
    myUVSphere.release(aCtx);
    myProgramFlat.release(aCtx);
    myProgramSphere.release(aCtx);
}

StHandle<StStereoParams> StGLImageRegion::getSource() {
    return params.stereoFile;
}

const StString& StGLImageRegion::getClassName() {
    return THE_CLASS_NAME;
}

void StGLImageRegion::stglUpdate(const StPointD_t& pointZo) {
    StGLWidget::stglUpdate(pointZo);
    if(myIsInitialized) {
        myHasVideoStream = myTextureQueue->stglUpdateStTextures(getContext()) || myTextureQueue->hasConnectedStream();
        params.stereoFile = myTextureQueue->getQTexture().getFront(StGLQuadTexture::LEFT_TEXTURE).getSource();
        ((StSwapLRParam*   )params.swapLR  .access())->setTrackedHandle(params.stereoFile);
        ((StViewModeParam* )params.ViewMode.access())->setTrackedHandle(params.stereoFile);
    }
}

bool StGLImageRegion::stglInit() {
    if(myIsInitialized) {
        return true;
    }

    StGLContext& aCtx = getContext();
    if(!myProgramFlat.init(aCtx, StImage::ImgColor_RGB, StImage::ImgScale_Full, StGLImageProgram::FragGetColor_Normal)) {
        return false;
    } else if(!myProgramSphere.init(aCtx, StImage::ImgColor_RGB, StImage::ImgScale_Full, StGLImageProgram::FragGetColor_Normal)) {
        return false;
    } else if(!myQuad.initScreen(aCtx)) {
        ST_DEBUG_LOG("Fail to init StGLQuad");
        return false;
    } else if(!myUVSphere.initVBOs(aCtx)) {
        ST_DEBUG_LOG("Fail to init StGLUVSphere");
    }

    // setup texture filter
    myTextureQueue->getQTexture().setMinMagFilter(aCtx, params.textureFilter->getValue() == StGLImageProgram::FILTER_NEAREST ? GL_NEAREST : GL_LINEAR);

    myIsInitialized = true;
    return myIsInitialized;
}

StGLVec2 StGLImageRegion::getMouseMoveFlat(const StPointD_t& theCursorZoFrom,
                                           const StPointD_t& theCursorZoTo) {
    return StGLVec2( 2.0f * GLfloat(theCursorZoTo.x() - theCursorZoFrom.x()),
                    -2.0f * GLfloat(theCursorZoTo.y() - theCursorZoFrom.y()));
}

StGLVec2 StGLImageRegion::getMouseMoveSphere(const StPointD_t& theCursorZoFrom,
                                             const StPointD_t& theCursorZoTo) {
    /// TODO (Kirill Gavrilov#5) these computations are invalid
    StGLVec2 stVec = getMouseMoveFlat(theCursorZoFrom, theCursorZoTo);
    GLfloat aSphereScale = SPHERE_RADIUS * getSource()->ScaleFactor;
    StRectD_t zParams;
    getCamera()->getZParams(getCamera()->getZNear(), zParams);
    stVec.x() *= -90.0f * GLfloat(zParams.right() - zParams.left()) / aSphereScale;
    stVec.y() *=  90.0f * GLfloat(zParams.bottom() - zParams.top()) / aSphereScale;
    return stVec;
}

StGLVec2 StGLImageRegion::getMouseMoveSphere() {
    return isClicked(ST_MOUSE_LEFT) ? getMouseMoveSphere(myClickPntZo, getRoot()->getCursorZo()) : StGLVec2();
}

void StGLImageRegion::stglDraw(unsigned int theView) {
    StHandle<StStereoParams> aParams = getSource();
    if(!myIsInitialized || !isVisible() || aParams.isNull()
    || !myTextureQueue->getQTexture().getFront(StGLQuadTexture::LEFT_TEXTURE).isValid()
    || !myHasVideoStream) {
        return;
    }

    if(aParams->isMono()) {
        theView = ST_DRAW_MONO;
        aParams->setSwapLR(false);
    }

    switch(params.displayMode->getValue()) {
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
}

void StGLImageRegion::stglDrawView(unsigned int theView) {

    StGLQuadTexture::LeftOrRight leftOrRight = StGLQuadTexture::LEFT_TEXTURE;
    StHandle<StStereoParams> aParams = getSource();
    if(!myIsInitialized || aParams.isNull()) {
        return;
    }

    StGLContext& aCtx = getContext();
    bool toShowRight = ( aParams->ToSwapLR && (theView == ST_DRAW_LEFT )) ||
                       (!aParams->ToSwapLR && (theView == ST_DRAW_RIGHT));
    if(aParams->isMono()) {
        leftOrRight = StGLQuadTexture::LEFT_TEXTURE;
    } else if(myTextureQueue->getQTexture().getFront(StGLQuadTexture::RIGHT_TEXTURE).isValid()) {
        if(toShowRight) {
            leftOrRight = StGLQuadTexture::RIGHT_TEXTURE;
        }
    }

    // retrieve viewport size for correct scissor rectangle computation
    GLfloat aFrustrumL = 1.0f, aFrustrumR = 1.0f, aFrustrumT = 1.0f, aFrustrumB = 1.0f;
    StRectI_t aFrameRectPx = getRectPx();

    if(!aParams->isMono()) {
        switch(params.displayMode->getValue()) {
            case MODE_PARALLEL: {
                if(theView == ST_DRAW_LEFT) {
                    aFrameRectPx.right() /= 2;
                    aFrustrumR = 3.0f;
                } else {
                    aFrameRectPx.left() += aFrameRectPx.width() / 2;
                    aFrustrumL = 3.0f;
                }
                break;
            }
            case MODE_CROSSYED: {
                if(theView == ST_DRAW_RIGHT) {
                    aFrameRectPx.right() /= 2;
                    aFrustrumR = 3.0f;
                } else {
                    aFrameRectPx.left() += aFrameRectPx.width() / 2;
                    aFrustrumL = 3.0f;
                }
                break;
            }
        }
    }

    // setup scissor box
    StGLBoxPx aScissorBox;
    const StRectI_t aFrameRectAbs = getAbsolute(aFrameRectPx);
    getRoot()->stglScissorRect(aFrameRectAbs, aScissorBox);
    aCtx.stglSetScissorRect(aScissorBox, true);

    aCtx.core20fwd->glDisable(GL_BLEND);

    StGLFrameTextures& stFrameTexture = myTextureQueue->getQTexture().getFront(leftOrRight);
    stFrameTexture.bind(aCtx);

    // our model matrix (identity)
    StGLMatrix stModelMat;

    StGLVec2 textureSizeVec(GLfloat(stFrameTexture.getPlane(0).getSizeX()),
                            GLfloat(stFrameTexture.getPlane(0).getSizeY()));
    StGLVec2 textureUVSizeVec(GLfloat(stFrameTexture.getPlane(1).getSizeX()),
                              GLfloat(stFrameTexture.getPlane(1).getSizeY()));

    // data rectangle in the texture
    StGLVec4 dataClampVec;
    StGLVec4 dataUVClampVec;
    if(params.textureFilter->getValue() == StGLImageProgram::FILTER_NEAREST
    || aParams->ViewingMode == StStereoParams::PANORAMA_SPHERE) {
        myTextureQueue->getQTexture().setMinMagFilter(aCtx, GL_NEAREST);
        //
        dataClampVec.x() = 0.0f;
        dataClampVec.y() = 0.0f;
        dataClampVec.z() = stFrameTexture.getPlane(0).getDataSize().x();
        dataClampVec.w() = stFrameTexture.getPlane(0).getDataSize().y();
        // UV
        dataUVClampVec.x() = 0.0f;
        dataUVClampVec.y() = 0.0f;
        dataUVClampVec.z() = stFrameTexture.getPlane(1).getDataSize().x();
        dataUVClampVec.w() = stFrameTexture.getPlane(1).getDataSize().y();
    } else {
        myTextureQueue->getQTexture().setMinMagFilter(aCtx, GL_LINEAR);
        //
        dataClampVec.x() = 0.5f / textureSizeVec.x();
        dataClampVec.y() = 0.5f / textureSizeVec.y();
        dataClampVec.z() = stFrameTexture.getPlane(0).getDataSize().x() - 2.0f * dataClampVec.x();
        dataClampVec.w() = stFrameTexture.getPlane(0).getDataSize().y() - 2.0f * dataClampVec.y();
        // UV
        if(textureUVSizeVec.x() > 0.0f && textureUVSizeVec.y() > 0.0f) {
            dataUVClampVec.x() = 0.5f / textureUVSizeVec.x();
            dataUVClampVec.y() = 0.5f / textureUVSizeVec.y(),
            dataUVClampVec.z() = stFrameTexture.getPlane(1).getDataSize().x() - 2.0f * dataUVClampVec.x();
            dataUVClampVec.w() = stFrameTexture.getPlane(1).getDataSize().y() - 2.0f * dataUVClampVec.y();
        }
    }

    // select (de)anaglyph color filter
    StGLVec3 aColorScale(1.0f, 1.0f, 1.0f);
    switch(aParams->StereoFormat) {
        case ST_V_SRC_ANAGLYPH_RED_CYAN: {
            if(!toShowRight) {
                aColorScale.g() = 0.0f;
                aColorScale.b() = 0.0f;
            } else {
                aColorScale.r() = 0.0f;
            }
            break;
        }
        case ST_V_SRC_ANAGLYPH_G_RB: {
            if(!toShowRight) {
                aColorScale.r() = 0.0f;
                aColorScale.b() = 0.0f;
            } else {
                aColorScale.g() = 0.0f;
            }
            break;
        }
        case ST_V_SRC_ANAGLYPH_YELLOW_BLUE: {
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
    switch(aParams->ViewingMode) {
        default:
        case StStereoParams::FLAT_IMAGE: {
            myProgramFlat.setColorScale(aColorScale); // apply de-anaglyph color filter
            StGLImageProgram::FragGetColor aColorGetter = params.textureFilter->getValue() == StGLImageProgram::FILTER_BLEND
                                                        ? StGLImageProgram::FragGetColor_Blend
                                                        : StGLImageProgram::FragGetColor_Normal;
            if(!myProgramFlat.init(aCtx, stFrameTexture.getColorModel(), stFrameTexture.getColorScale(), aColorGetter)) {
                break;
            }

            myProgramFlat.getActiveProgram()->use(aCtx);

            // setup data rectangle in the texture
            myProgramFlat.setTextureSizePx      (aCtx, textureSizeVec);
            myProgramFlat.setTextureMainDataSize(aCtx, dataClampVec);
            myProgramFlat.setTextureUVDataSize  (aCtx, dataUVClampVec);

            // lenses center correction
            const GLfloat aLestDisp = getRoot()->getLensDist() * GLfloat(getRoot()->getRectPx().ratio());
            if(theView == ST_DRAW_LEFT) {
                stModelMat.translate(StGLVec3( aLestDisp, 0.0f, 0.0f));
            } else {
                stModelMat.translate(StGLVec3(-aLestDisp, 0.0f, 0.0f));
            }

            // handle dragging timer
            if( isClicked(ST_MOUSE_LEFT)
            && !myIsClickAborted
            &&  myClickTimer.isOn()) {
                if(myClickTimer.getElapsedTimeInMilliSec() < myDragDelayMs) {
                    const StPointD_t aCurr = getRoot()->getCursorZo();
                    const int aDx = int((aCurr.x() - myClickPntZo.x()) * double(getRectPx().width()));
                    const int aDy = int((aCurr.y() - myClickPntZo.y()) * double(getRectPx().height()));
                    if(std::abs(aDx) > 1
                    || std::abs(aDy) > 1) {
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

            // apply scale
            stModelMat.scale(aParams->ScaleFactor, aParams->ScaleFactor, 1.0f);

            // apply position
            stModelMat.translate(StGLVec3(aParams->PanCenter));

            // apply rotations
            if(theView == ST_DRAW_LEFT) {
                stModelMat.rotate(aParams->getZRotate() - aParams->getSepRotation(), StGLVec3::DZ());
            } else if(theView == ST_DRAW_RIGHT) {
                stModelMat.rotate(aParams->getZRotate() + aParams->getSepRotation(), StGLVec3::DZ());
            } else {
                stModelMat.rotate(aParams->getZRotate(), StGLVec3::DZ());
            }

            /// TODO (Kirill Gavrilov#8) implement fit all for rotated image

            // check window ratio to fill whole image in normal zoom
            GLfloat dispRatio = 1.0f;
            switch(params.displayRatio->getValue()) {
                case RATIO_1_1:   dispRatio = 1.0f;  break;
                case RATIO_4_3:   dispRatio = 1.3333333333f; break;
                case RATIO_16_9:  dispRatio = 1.7777777778f; break;
                case RATIO_16_10: dispRatio = 1.6f;  break;
                case RATIO_221_1: dispRatio = 2.21f; break;
                case RATIO_5_4:   dispRatio = 1.25f; break;
                case RATIO_AUTO:
                default: dispRatio = stFrameTexture.getPlane().getDisplayRatio(); break;
            }

            GLfloat rectRatio = GLfloat(aFrameRectPx.ratio());
            StGLVec2 ratioScale = aParams->getRatioScale(rectRatio, dispRatio);
            stModelMat.scale(ratioScale.x(), ratioScale.y(), 1.0f);

            // apply separation
            GLfloat aSepDeltaX = (2.0f * aParams->getSeparationDx()) / (stFrameTexture.getPlane().getDataSize().x() * stFrameTexture.getPlane().getSizeX());
            GLfloat aSepDeltaY = (2.0f * aParams->getSeparationDy()) / (stFrameTexture.getPlane().getDataSize().y() * stFrameTexture.getPlane().getSizeY());
            if(theView == ST_DRAW_LEFT) {
                stModelMat.translate(StGLVec3(-aSepDeltaX * 0.5f, -aSepDeltaY * 0.5f, 0.0f));
            } else if(theView == ST_DRAW_RIGHT) {
                stModelMat.translate(StGLVec3( aSepDeltaX * 0.5f,  aSepDeltaY * 0.5f, 0.0f));
            }

            StGLMatrix stOrthoMat;
            stOrthoMat.initOrtho(StGLVolume(-rectRatio * aFrustrumL, rectRatio * aFrustrumR,
                                            -1.0f      * aFrustrumB, 1.0f      * aFrustrumT,
                                            -1.0f, 1.0f));
            myProgramFlat.getActiveProgram()->setProjMat (aCtx, stOrthoMat);
            myProgramFlat.getActiveProgram()->setModelMat(aCtx, stModelMat);

            myQuad.draw(aCtx, *myProgramFlat.getActiveProgram());

            myProgramFlat.getActiveProgram()->unuse(aCtx);

            // restore changed parameters
            aParams->ScaleFactor = aScaleBack;
            aParams->PanCenter   = aPanBack;
            break;
        }
        case StStereoParams::PANORAMA_SPHERE: {
            myProgramSphere.setColorScale(aColorScale); // apply de-anaglyph color filter
            StGLImageProgram::FragGetColor aColorGetter = params.textureFilter->getValue() != StGLImageProgram::FILTER_NEAREST
                                                        ? StGLImageProgram::FragGetColor_Blend
                                                        : StGLImageProgram::FragGetColor_Normal;
            if(!myProgramSphere.init(aCtx, stFrameTexture.getColorModel(), stFrameTexture.getColorScale(), aColorGetter)) {
                break;
            }

            /// TODO (Kirill Gavrilov#5) implement cross-eyed/parallel pair output
            /// TODO (Kirill Gavrilov#5) apply separation

            // perform scaling
            stModelMat.scale(SPHERE_RADIUS * aParams->ScaleFactor, SPHERE_RADIUS * aParams->ScaleFactor, SPHERE_RADIUS);

            /// TODO (Kirill Gavrilov#5) fix horizontal movement direction after upside-downing
            // apply movements
            StGLVec2 mouseMove = getMouseMoveSphere();
            stModelMat.rotate(         aParams->PanTheta + mouseMove.y(),  StGLVec3::DX());
            stModelMat.rotate(90.0f - (aParams->PanPhi   + mouseMove.x()), StGLVec3::DY());

            // apply rotations
            if(theView == ST_DRAW_LEFT) {
                stModelMat.rotate(aParams->getZRotate() - aParams->getSepRotation(), StGLVec3::DZ());
            } else if(theView == ST_DRAW_RIGHT) {
                stModelMat.rotate(aParams->getZRotate() + aParams->getSepRotation(), StGLVec3::DZ());
            } else {
                stModelMat.rotate(aParams->getZRotate(), StGLVec3::DZ());
            }

            // perform drawing
            myProgramSphere.getActiveProgram()->use(aCtx);

            // setup data rectangle in the texture
            myProgramSphere.setTextureSizePx      (aCtx, textureSizeVec);
            myProgramSphere.setTextureMainDataSize(aCtx, dataClampVec);
            myProgramSphere.setTextureUVDataSize  (aCtx, dataUVClampVec);

            myProgramSphere.getActiveProgram()->setProjMat (aCtx, getCamera()->getProjMatrixMono());
            myProgramSphere.getActiveProgram()->setModelMat(aCtx, stModelMat);

            myUVSphere.draw(aCtx, *myProgramSphere.getActiveProgram());

            myProgramSphere.getActiveProgram()->unuse(aCtx);
            break;
        }
    }

    stFrameTexture.unbind(aCtx);

    aCtx.stglResetScissorRect();
}

bool StGLImageRegion::tryClick(const StPointD_t& theCursorZo,
                               const int&        theMouseBtn,
                               bool&             isItemClicked) {
    if(StGLWidget::tryClick(theCursorZo, theMouseBtn, isItemClicked)) {
        if(theMouseBtn == ST_MOUSE_LEFT) {
            myClickPntZo = theCursorZo;
            myClickTimer.restart();
            myIsClickAborted = false;
        }
        isItemClicked = true;
        return true;
    }
    return false;
}

bool StGLImageRegion::tryUnClick(const StPointD_t& theCursorZo,
                                 const int&        theMouseBtn,
                                 bool&             isItemUnclicked) {
    StHandle<StStereoParams> aParams = getSource();
    if(!myIsInitialized || aParams.isNull()) {
        return false;
    }
    if(isClicked(ST_MOUSE_LEFT) && theMouseBtn == ST_MOUSE_LEFT) {
        // ignore out of window
        switch(aParams->ViewingMode) {
            default:
            case StStereoParams::FLAT_IMAGE: {
                if(!myIsClickAborted) {
                    aParams->moveFlat(getMouseMoveFlat(myClickPntZo, theCursorZo), GLfloat(getRectPx().ratio()));
                }
                break;
            }
            case StStereoParams::PANORAMA_SPHERE: {
                aParams->moveSphere(getMouseMoveSphere(myClickPntZo, theCursorZo));
                break;
            }

        }
        isItemUnclicked = true;
        setClicked(ST_MOUSE_LEFT, false);
        return true;
    }
    if(StGLWidget::tryUnClick(theCursorZo, theMouseBtn, isItemUnclicked)) {
        const GLfloat SCALE_STEPS = 0.16f;
        StPointD_t aCenterCursor(0.5, 0.5);
        if(theMouseBtn == ST_MOUSE_SCROLL_V_UP) {
            switch(aParams->ViewingMode) {
                default:
                case StStereoParams::FLAT_IMAGE: {
                    const StGLVec2 aVec = getMouseMoveFlat(aCenterCursor, theCursorZo) * (-SCALE_STEPS);
                    aParams->scaleIn(SCALE_STEPS);
                    aParams->moveFlat(aVec, GLfloat(getRectPx().ratio()));
                    break;
                }
                case StStereoParams::PANORAMA_SPHERE: {
                    const StGLVec2 aVec = getMouseMoveSphere(aCenterCursor, theCursorZo) * (-SCALE_STEPS);
                    aParams->scaleIn(SCALE_STEPS);
                    aParams->moveSphere(aVec);
                    break;
                }
            }
        } else if(theMouseBtn == ST_MOUSE_SCROLL_V_DOWN) {
            switch(aParams->ViewingMode) {
                default:
                case StStereoParams::FLAT_IMAGE: {
                    const StGLVec2 aVec = getMouseMoveFlat(aCenterCursor, theCursorZo) * SCALE_STEPS;
                    aParams->moveFlat(aVec, GLfloat(getRectPx().ratio()));
                    aParams->scaleOut(SCALE_STEPS);
                    break;
                }
                case StStereoParams::PANORAMA_SPHERE: {
                    const StGLVec2 aVec = getMouseMoveSphere(aCenterCursor, theCursorZo) * SCALE_STEPS;
                    aParams->moveSphere(aVec);
                    aParams->scaleOut(SCALE_STEPS);
                    break;
                }
            }
        }
        return true;
    }
    return false;
}
