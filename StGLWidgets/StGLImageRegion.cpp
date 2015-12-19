/**
 * StGLWidgets, small C++ toolkit for writing GUI using OpenGL.
 * Copyright © 2010-2015 Kirill Gavrilov <kirill@sview.ru>
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
            signals.onChanged(theValue);
            return true;
        }

        void setTrackedHandle(const StHandle<StStereoParams>& theTrackedParams) {
            myTrackedParams = theTrackedParams;
        }

            private:

        StHandle<StStereoParams> myTrackedParams;

    };

    // we use negative scale factor to show sphere inside out!
    static const GLfloat SPHERE_RADIUS     = -10.0f;
    static const GLfloat PANORAMA_DEF_ZOOM = 0.45f;

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
  myRotAngle(0.5f),
  myIsClickAborted(false),
#ifdef ST_EXTRA_CONTROLS
  myToRightRotate(true),
#else
  myToRightRotate(false),
#endif
  myIsInitialized(false),
  myHasVideoStream(false) {
    params.displayMode = new StEnumParam(MODE_STEREO, "Stereo Output");
    params.displayMode->changeValues().add("Stereo");        // MODE_STEREO
    params.displayMode->changeValues().add("Left View");     // MODE_ONLY_LEFT
    params.displayMode->changeValues().add("Right View");    // MODE_ONLY_RIGHT
    params.displayMode->changeValues().add("Parallel Pair"); // MODE_PARALLEL
    params.displayMode->changeValues().add("Cross-eyed");    // MODE_CROSSYED

    params.displayRatio  = new StInt32Param(RATIO_AUTO);
    params.ToHealAnamorphicRatio = new StBoolParam(false);
    params.textureFilter = new StInt32Param(StGLImageProgram::FILTER_LINEAR);
    params.gamma      = myProgram.params.gamma;
    params.brightness = myProgram.params.brightness;
    params.saturation = myProgram.params.saturation;
    params.swapLR   = new StSwapLRParam();
    params.ViewMode = new StViewModeParam();

#ifdef ST_EXTRA_CONTROLS
    theUsePanningKeys = false;
#endif

    // create actions
    StHandle<StAction> anAction;
    anAction = new StActionIntSlot(stCString("DoParamsReset"), stSlot(this, &StGLImageRegion::doParamsReset), 0);
    anAction->setDefaultHotKey1(ST_VK_BACK);
    myActions.add(anAction);

    anAction = new StActionBool(stCString("DoParamsSwapLR"), params.swapLR);
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
    if(!myProgram.init(aCtx, StImage::ImgColor_RGB, StImage::ImgScale_Full, StGLImageProgram::FragGetColor_Normal)) {
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
    StGLVec2 aVec = getMouseMoveFlat(theCursorZoFrom, theCursorZoTo);
    GLfloat aSphereScale = SPHERE_RADIUS * PANORAMA_DEF_ZOOM * getSource()->ScaleFactor;
    StRectD_t aZParams;
    getCamera()->getZParams(getCamera()->getZNear(), aZParams);
    aVec.x() *= -90.0f * GLfloat(aZParams.right() - aZParams.left()) / aSphereScale;
    aVec.y() *=  90.0f * GLfloat(aZParams.bottom() - aZParams.top()) / aSphereScale;
    return aVec;
}

StGLVec2 StGLImageRegion::getMouseMoveSphere() {
    return isClicked(ST_MOUSE_LEFT)
         ? getMouseMoveSphere(myClickPntZo, getRoot()->getCursorZo())
         : StGLVec2();
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

    StGLFrameTextures& aTextures = myTextureQueue->getQTexture().getFront(aLeftOrRight);
    aTextures.bind(aCtx);

    StGLVec2 aTextureSize  (GLfloat(aTextures.getPlane(0).getSizeX()),
                            GLfloat(aTextures.getPlane(0).getSizeY()));
    StGLVec2 aTextureUVSize(GLfloat(aTextures.getPlane(1).getSizeX()),
                            GLfloat(aTextures.getPlane(1).getSizeY()));
    StGLMatrix aModelMat;
    // data rectangle in the texture
    StGLVec4 aClampVec, aClampUV;
    if(params.textureFilter->getValue() == StGLImageProgram::FILTER_NEAREST) {
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

    StStereoParams::ViewMode aViewMode = aParams->ViewingMode;
    if(aTextures.getPlane(0).getTarget() == GL_TEXTURE_CUBE_MAP) {
        aViewMode = StStereoParams::PANORAMA_CUBEMAP;
    } else if(aViewMode == StStereoParams::PANORAMA_CUBEMAP) {
        aViewMode = StStereoParams::FLAT_IMAGE;
    }

    myProgram.setColorScale(aColorScale); // apply de-anaglyph color filter
    StGLImageProgram::FragGetColor aColorGetter = params.textureFilter->getValue() == StGLImageProgram::FILTER_BLEND
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

            // lenses center correction
            const GLfloat aLestDisp = getRoot()->getLensDist() * GLfloat(getRoot()->getRectPx().ratio());
            if(theView == ST_DRAW_LEFT) {
                aModelMat.translate(StGLVec3( aLestDisp, 0.0f, 0.0f));
            } else {
                aModelMat.translate(StGLVec3(-aLestDisp, 0.0f, 0.0f));
            }

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
            aModelMat.scale(aParams->ScaleFactor, aParams->ScaleFactor, 1.0f);

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
            switch(params.displayRatio->getValue()) {
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

            const GLfloat aScale = aParams->ScaleFactor * PANORAMA_DEF_ZOOM;
            aModelMat.scale(aScale, aScale, 1.0f);

            // compute orientation
            StGLVec2 aMouseMove = getMouseMoveSphere();
            float aYaw   = -stToRadians(aParams->PanPhi   + aMouseMove.x()) + stToRadians(90.0f);
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

            aModelMat = StGLMatrix::multiply(aModelMat, StGLMatrix(anOri));

            StGLMatrix aMatModelInv, aMatProjInv;
            aModelMat.inverted(aMatModelInv);
            getCamera()->getProjMatrixMono().inverted(aMatProjInv);
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
            const GLfloat aScale = SPHERE_RADIUS * PANORAMA_DEF_ZOOM * aParams->ScaleFactor;
            aModelMat.scale(aScale, aScale, SPHERE_RADIUS);

            // compute orientation
            StGLVec2 aMouseMove = getMouseMoveSphere();
            float aYaw   = -stToRadians(aParams->PanPhi   + aMouseMove.x()) + stToRadians(90.0f);
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

            aModelMat = StGLMatrix::multiply(aModelMat, StGLMatrix(anOri));

            // perform drawing
            myProgram.getActiveProgram()->use(aCtx);

            // setup data rectangle in the texture
            myProgram.setTextureSizePx      (aCtx, aTextureSize);
            myProgram.setTextureMainDataSize(aCtx, aClampVec);
            myProgram.setTextureUVDataSize  (aCtx, aClampUV);

            myProgram.getActiveProgram()->setProjMat (aCtx, getCamera()->getProjMatrixMono());
            myProgram.getActiveProgram()->setModelMat(aCtx, aModelMat);

            myUVSphere.draw(aCtx, *myProgram.getActiveProgram());

            myProgram.getActiveProgram()->unuse(aCtx);
            break;
        }
    }

    aTextures.unbind(aCtx);

    aCtx.stglResetScissorRect();
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
    ||  aParams.isNull()) {
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
        case stEvent_Gesture2Rotate: {
            myRotAngle += theEvent.Value;
            if(myRotAngle >= M_PI * 0.3) {
                doParamsRotZ90(1);
                myRotAngle -= M_PI * 0.3;
            } else if(myRotAngle <= -M_PI * 0.3) {
                doParamsRotZ90(size_t(-1));
                myRotAngle += M_PI * 0.3;
            }
            return true;
        }
        case stEvent_Gesture2Move: {
            if(!theEvent.OnScreen) {
                // this gesture conflicts with scrolling on OS X
                return true;
            }
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
