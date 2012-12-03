/**
 * Copyright © 2010-2012 Kirill Gavrilov <kirill@sview.ru>
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
            return myTracked1->setValue(theValue)
                || myTracked2->setValue(theValue);
        }

    };

    class ST_LOCAL StSwapLRParam : public StBoolParam {

            private:

        StHandle<StStereoParams> myTrackedParams;

            public:

        StSwapLRParam()
        : StBoolParam(false),
          myTrackedParams() {}

        virtual bool getValue() const {
            return myTrackedParams.isNull() ? false : myTrackedParams->isSwapLR();
        }

        virtual bool setValue(const bool theValue) {
            if(myTrackedParams.isNull() || myTrackedParams->isSwapLR() == theValue) {
                return false;
            }
            myTrackedParams->setSwapLR(theValue);
            return true;
        }

        void setTrackedHandle(const StHandle<StStereoParams>& theTrackedParams) {
            myTrackedParams = theTrackedParams;
        }

    };

    // we use negative scale factor to show sphere inside out!
    static const GLfloat SPHERE_RADIUS = -10.0f;

};

StGLImageRegion::StGLImageRegion(StGLWidget* theParent, size_t theTextureQueueSizeMax)
: StGLWidget(theParent, 0, 0, StGLCorner(ST_VCORNER_TOP, ST_HCORNER_LEFT)),
  myQuad(),
  myUVSphere(StGLVec3(0.0f, 0.0f, 0.0f), 1.0f, 64),
  myProgramFlat(),
  myProgramSphere(),
  myTextureQueue(new StGLTextureQueue(theTextureQueueSizeMax)),
  myClickPntZo(0.0, 0.0),
  myIsInitialized(false),
  myHasVideoStream(false) {
    //
    params.displayMode   = new StInt32Param(MODE_STEREO);
    params.displayRatio  = new StInt32Param(RATIO_AUTO);
    params.textureFilter = new StInt32Param(StGLImageProgram::FILTER_LINEAR);
    params.textureFilter->signals.onChanged.connect(this, &StGLImageRegion::doChangeTexFilter);
    params.gamma      = new StTrackedFloatParam(myProgramFlat.params.gamma,
                                                myProgramSphere.params.gamma);
    params.brightness = new StTrackedFloatParam(myProgramFlat.params.brightness,
                                                myProgramSphere.params.brightness);
    params.saturation = new StTrackedFloatParam(myProgramFlat.params.saturation,
                                                myProgramSphere.params.saturation);
    params.swapLR = new StSwapLRParam();
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
        myHasVideoStream = myTextureQueue->stglUpdateStTextures(getContext()) || myHasVideoStream;
        params.stereoFile = myTextureQueue->getQTexture().getFront(StGLQuadTexture::LEFT_TEXTURE).getSource();
        ((StSwapLRParam* )params.swapLR.access())->setTrackedHandle(params.stereoFile);
    }
}

bool StGLImageRegion::stglInit() {
    if(myIsInitialized) {
        return true;
    }

    StGLContext& aCtx = getContext();
    myProgramFlat.setContext(getRoot()->getContextHandle());
    myProgramSphere.setContext(getRoot()->getContextHandle());
    if(!myProgramFlat.init(aCtx)) {
        return false;
    } else if(!myProgramSphere.init(aCtx)) {
        return false;
    } else if(!myQuad.initScreen(aCtx)) {
        ST_DEBUG_LOG("Fail to init StGLQuad");
        return false;
    } else if(!myUVSphere.initVBOs(aCtx)) {
        ST_DEBUG_LOG("Fail to init StGLUVSphere");
    }

    // setup texture filter
    myTextureQueue->getQTexture().setMinMagFilter(aCtx, params.textureFilter->getValue() == StGLImageProgram::FILTER_NEAREST ? GL_NEAREST : GL_LINEAR);
    myProgramFlat.setSmoothFilter  (aCtx, StGLImageProgram::TextureFilter(params.textureFilter->getValue()));
    myProgramSphere.setSmoothFilter(aCtx, StGLImageProgram::TextureFilter(params.textureFilter->getValue()));

    myIsInitialized = true;
    return myIsInitialized;
}

StGLVec2 StGLImageRegion::getMouseMoveFlat(const StPointD_t& theCursorZoFrom,
                                           const StPointD_t& theCursorZoTo) {
    return StGLVec2( 2.0f * GLfloat(theCursorZoTo.x() - theCursorZoFrom.x()),
                    -2.0f * GLfloat(theCursorZoTo.y() - theCursorZoFrom.y()));
}

StGLVec2 StGLImageRegion::getMouseMoveFlat() {
    return isClicked(ST_MOUSE_LEFT) ? getMouseMoveFlat(myClickPntZo, getRoot()->getCursorZo()) : StGLVec2();
}

StGLVec2 StGLImageRegion::getMouseMoveSphere(const StPointD_t& theCursorZoFrom,
                                             const StPointD_t& theCursorZoTo) {
    /// TODO (Kirill Gavrilov#5) these computations are invalid
    StGLVec2 stVec = getMouseMoveFlat(theCursorZoFrom, theCursorZoTo);
    GLfloat sphereScale = SPHERE_RADIUS * getSource()->getScale();
    StRectD_t zParams;
    getCamera()->getZParams(getCamera()->getZNear(), zParams);
    stVec.x() *= -90.0f * GLfloat(zParams.right() - zParams.left()) / sphereScale;
    stVec.y() *=  90.0f * GLfloat(zParams.bottom() - zParams.top()) / sphereScale;
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
    bool toShowRight = ( aParams->isSwapLR() && (theView == ST_DRAW_LEFT )) ||
                       (!aParams->isSwapLR() && (theView == ST_DRAW_RIGHT));
    if(aParams->isMono()) {
        leftOrRight = StGLQuadTexture::LEFT_TEXTURE;
    } else if(myTextureQueue->getQTexture().getFront(StGLQuadTexture::RIGHT_TEXTURE).isValid()) {
        if(toShowRight) {
            leftOrRight = StGLQuadTexture::RIGHT_TEXTURE;
        }
    }

    // retrieve viewport size for correct scissor rectangle computation
    GLint viewPort[4]; aCtx.core20fwd->glGetIntegerv(GL_VIEWPORT, viewPort);
    StRectI_t rootRectPx = getRoot()->getRectPx();
    StGLVec2 vpScale(GLfloat(viewPort[2]) / GLfloat(rootRectPx.width()),
                     GLfloat(viewPort[3]) / GLfloat(rootRectPx.height()));
    GLfloat frustrumL = 1.0f;
    GLfloat frustrumR = 1.0f;
    StRectI_t frameRectPx = getRectPx();

    if(!aParams->isMono() && params.displayMode->getValue() == MODE_PARALLEL) {
        if(theView == ST_DRAW_LEFT) {
            frameRectPx.right() /= 2;
            frustrumR = 3.0f;
        } else {
            frameRectPx.left() += frameRectPx.width() / 2;
            frustrumL = 3.0f;
        }
    } else if(!aParams->isMono() && params.displayMode->getValue() == MODE_CROSSYED) {
        if(theView == ST_DRAW_RIGHT) {
            frameRectPx.right() /= 2;
            frustrumR = 3.0f;
        } else {
            frameRectPx.left() += frameRectPx.width() / 2;
            frustrumL = 3.0f;
        }
    }

    aCtx.core20fwd->glEnable(GL_SCISSOR_TEST);
    aCtx.core20fwd->glScissor(GLsizei(vpScale.x() * GLfloat(frameRectPx.left())),
                              GLsizei(vpScale.y() * GLfloat(rootRectPx.height() - frameRectPx.bottom())),
                              GLsizei(vpScale.x() * GLfloat(frameRectPx.width())),
                              GLsizei(vpScale.y() * GLfloat(frameRectPx.height())));

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
    || aParams->getViewMode() == StStereoParams::PANORAMA_SPHERE) {
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
    bool isAnaglyph = false;
    switch(aParams->getSrcFormat()) {
        case ST_V_SRC_ANAGLYPH_RED_CYAN: {
            if(!toShowRight) {
                aColorScale.g() = 0.0f;
                aColorScale.b() = 0.0f;
            } else {
                aColorScale.r() = 0.0f;
            }
            isAnaglyph = true;
            break;
        }
        case ST_V_SRC_ANAGLYPH_G_RB: {
            if(!toShowRight) {
                aColorScale.r() = 0.0f;
                aColorScale.b() = 0.0f;
            } else {
                aColorScale.g() = 0.0f;
            }
            isAnaglyph = true;
            break;
        }
        case ST_V_SRC_ANAGLYPH_YELLOW_BLUE: {
            if(!toShowRight) {
                aColorScale.b() = 0.0f;
            } else {
                aColorScale.r() = 0.0f;
                aColorScale.g() = 0.0f;
            }
            isAnaglyph = true;
            break;
        }
        default: break;
    }

    switch(aParams->getViewMode()) {
        default:
        case StStereoParams::FLAT_IMAGE: {
            // apply (de)anaglyph color filter
            if(isAnaglyph) {
                myProgramFlat.setColorScale(aCtx, aColorScale);
            } else {
                myProgramFlat.resetColorScale(aCtx);
            }

            myProgramFlat.setupSrcColorShader(aCtx, stFrameTexture.getColorModel());
            myProgramFlat.use(aCtx);

            // setup data rectangle in the texture
            myProgramFlat.setTextureSizePx(aCtx, textureSizeVec);
            myProgramFlat.setTextureMainDataSize(aCtx, dataClampVec);
            myProgramFlat.setTextureUVDataSize(aCtx, dataUVClampVec);

            // apply scale
            stModelMat.scale(aParams->getScale(), aParams->getScale(), 1.0f);

            // apply position
            stModelMat.translate(StGLVec3(aParams->getCenter()));

            GLfloat rectRatio = GLfloat(getRectPx().ratio());
            StGLVec2 aMouseMove = aParams->moveFlatDelta(getMouseMoveFlat(), rectRatio);
            stModelMat.translate(StGLVec3(aMouseMove));

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

            rectRatio = GLfloat(frameRectPx.ratio());
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
            stOrthoMat.initOrtho(StGLVolume(-rectRatio * frustrumL, rectRatio * frustrumR,
                                            -1.0f, 1.0f,
                                            -1.0f, 1.0f));
            myProgramFlat.setProjMat(aCtx, stOrthoMat);
            myProgramFlat.setModelMat(aCtx, stModelMat);

            myQuad.draw(aCtx, myProgramFlat);

            myProgramFlat.unuse(aCtx);
            break;
        }
        case StStereoParams::PANORAMA_SPHERE: {
            // apply (de)anaglyph color filter
            if(isAnaglyph) {
                myProgramSphere.setColorScale(aCtx, aColorScale);
            } else {
                myProgramSphere.resetColorScale(aCtx);
            }

            /// TODO (Kirill Gavrilov#5) implement cross-eyed/parallel pair output
            /// TODO (Kirill Gavrilov#5) apply separation

            // perform scaling
            stModelMat.scale(SPHERE_RADIUS * aParams->getScale(), SPHERE_RADIUS * aParams->getScale(), SPHERE_RADIUS);

            /// TODO (Kirill Gavrilov#5) fix horizontal movement direction after upside-downing
            // apply movements
            StGLVec2 mouseMove = getMouseMoveSphere();
            stModelMat.rotate(         aParams->getTheta() + mouseMove.y(),  StGLVec3::DX());
            stModelMat.rotate(90.0f - (aParams->getPhi()   + mouseMove.x()), StGLVec3::DY());

            // apply rotations
            if(theView == ST_DRAW_LEFT) {
                stModelMat.rotate(aParams->getZRotate() - aParams->getSepRotation(), StGLVec3::DZ());
            } else if(theView == ST_DRAW_RIGHT) {
                stModelMat.rotate(aParams->getZRotate() + aParams->getSepRotation(), StGLVec3::DZ());
            } else {
                stModelMat.rotate(aParams->getZRotate(), StGLVec3::DZ());
            }

            // perform drawing
            myProgramSphere.setupSrcColorShader(aCtx, stFrameTexture.getColorModel());
            myProgramSphere.use(aCtx);

            // setup data rectangle in the texture
            myProgramSphere.setTextureSizePx(aCtx, textureSizeVec);
            myProgramSphere.setTextureMainDataSize(aCtx, dataClampVec);
            myProgramSphere.setTextureUVDataSize(aCtx, dataUVClampVec);

            myProgramSphere.setProjMat(aCtx, getCamera()->getProjMatrixMono());
            myProgramSphere.setModelMat(aCtx, stModelMat);

            myUVSphere.draw(aCtx, myProgramSphere);

            myProgramSphere.unuse(aCtx);
            break;
        }
    }

    stFrameTexture.unbind(aCtx);

    aCtx.core20fwd->glDisable(GL_SCISSOR_TEST);
}

bool StGLImageRegion::tryClick(const StPointD_t& theCursorZo, const int& theMouseBtn, bool& isItemClicked) {
    if(StGLWidget::tryClick(theCursorZo, theMouseBtn, isItemClicked)) {
        myClickPntZo = theCursorZo;
        isItemClicked = true;
        return true;
    }
    return false;
}

bool StGLImageRegion::tryUnClick(const StPointD_t& theCursorZo, const int& theMouseBtn, bool& isItemUnclicked) {
    StHandle<StStereoParams> aParams = getSource();
    if(!myIsInitialized || aParams.isNull()) {
        return false;
    }
    if(isClicked(ST_MOUSE_LEFT) && theMouseBtn == ST_MOUSE_LEFT) {
        // ignore out of window
        switch(aParams->getViewMode()) {
            default:
            case StStereoParams::FLAT_IMAGE: {
                aParams->moveFlat(getMouseMoveFlat(myClickPntZo, theCursorZo), GLfloat(getRectPx().ratio()));
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
        const GLfloat SCALE_STEPS = 8.0f;
        StPointD_t centerCursor(0.5, 0.5);
        if(theMouseBtn == ST_MOUSE_SCROLL_V_UP) {
            switch(aParams->getViewMode()) {
                default:
                case StStereoParams::FLAT_IMAGE: {
                    StGLVec2 stVec = getMouseMoveFlat(centerCursor, theCursorZo) * (-SCALE_STEPS * aParams->getScaleStep());
                    aParams->scaleIn(SCALE_STEPS);
                    aParams->moveFlat(stVec, GLfloat(getRectPx().ratio()));
                    break;
                }
                case StStereoParams::PANORAMA_SPHERE: {
                    StGLVec2 stVec = getMouseMoveSphere(centerCursor, theCursorZo) * (-SCALE_STEPS * aParams->getScaleStep());
                    aParams->scaleIn(SCALE_STEPS);
                    aParams->moveSphere(stVec);
                    break;
                }
            }
        } else if(theMouseBtn == ST_MOUSE_SCROLL_V_DOWN) {
            switch(aParams->getViewMode()) {
                default:
                case StStereoParams::FLAT_IMAGE: {
                    StGLVec2 stVec = getMouseMoveFlat(centerCursor, theCursorZo) * (SCALE_STEPS * aParams->getScaleStep());
                    aParams->moveFlat(stVec, GLfloat(getRectPx().ratio()));
                    aParams->scaleOut(SCALE_STEPS);
                    break;
                }
                case StStereoParams::PANORAMA_SPHERE: {
                    StGLVec2 stVec = getMouseMoveSphere(centerCursor, theCursorZo) * (SCALE_STEPS * aParams->getScaleStep());
                    aParams->moveSphere(stVec);
                    aParams->scaleOut(SCALE_STEPS);
                    break;
                }
            }
        }
        return true;
    }
    return false;
}

void StGLImageRegion::doChangeTexFilter(const int32_t theTextureFilter) {
    if(!myIsInitialized) {
        return;
    }

    StGLContext& aCtx = getContext();
    myProgramFlat  .setSmoothFilter(aCtx, StGLImageProgram::TextureFilter(theTextureFilter));
    myProgramSphere.setSmoothFilter(aCtx, StGLImageProgram::TextureFilter(theTextureFilter));
}
