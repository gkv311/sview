/**
 * StGLWidgets, small C++ toolkit for writing GUI using OpenGL.
 * Copyright © 2009-2017 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#include <StGLWidgets/StGLTextureButton.h>
#include <StGLWidgets/StGLRootWidget.h>

#include <StGL/StGLProgramMatrix.h>
#include <StGL/StGLResources.h>
#include <StGL/StGLContext.h>
#include <StGLCore/StGLCore20.h>

#include <StAV/StAVImage.h>
#include <StSlots/StAction.h>
#include <StThreads/StProcess.h>

/**
 * This class defines GLSL program for textured button widget.
 */
class StGLTextureButton::Program : public StGLProgram {

        public:

    Program(const StString& theTitle)
    : StGLProgram(theTitle),
      myDispX(0.0f) {}

    StGLVarLocation getVVertexLoc()   const { return StGLVarLocation(0); }
    StGLVarLocation getVTexCoordLoc() const { return StGLVarLocation(1); }

    void setProjMat(StGLContext&      theCtx,
                    const StGLMatrix& theProjMat) {
        theCtx.core20fwd->glUniformMatrix4fv(uniProjMatLoc, 1, GL_FALSE, theProjMat);
    }

    virtual bool link(StGLContext& theCtx) ST_ATTR_OVERRIDE {
        if(!isValid()) {
            return false;
        }

        bindAttribLocation(theCtx, "vVertex",   getVVertexLoc());
        bindAttribLocation(theCtx, "vTexCoord", getVTexCoordLoc());
        if(!StGLProgram::link(theCtx)) {
            return false;
        }

        uniProjMatLoc   = StGLProgram::getUniformLocation(theCtx, "uProjMat");
        uniDispLoc      = StGLProgram::getUniformLocation(theCtx, "uDisp");
        uniTimeLoc      = StGLProgram::getUniformLocation(theCtx, "uTime");
        uniClickedLoc   = StGLProgram::getUniformLocation(theCtx, "uClicked");
        uniParamsLoc    = StGLProgram::getUniformLocation(theCtx, "uParams");
        uniColorLoc     = StGLProgram::getUniformLocation(theCtx, "uColor");

        StGLVarLocation uniTextureLoc = StGLProgram::getUniformLocation(theCtx, "uTexture");
        if(uniTextureLoc.isValid()) {
            StGLProgram::use(theCtx);
            theCtx.core20fwd->glUniform1i(uniTextureLoc, StGLProgram::TEXTURE_SAMPLE_0);
            StGLProgram::unuse(theCtx);
        }

        return uniProjMatLoc.isValid()
            && uniTimeLoc.isValid()
            && uniClickedLoc.isValid()
            && uniParamsLoc.isValid()
            && uniTextureLoc.isValid();
    }

    void useTemp(StGLContext& theCtx) {
        StGLProgram::use(theCtx);
    }

    void setColor(StGLContext&    theCtx,
                  const StGLVec4& theColor) {
        theCtx.core20fwd->glUniform4fv(uniColorLoc, 1, theColor);
    }

    using StGLProgram::use;
    void use(StGLContext&    theCtx,
             const StGLVec4& theColor,
             const GLfloat   theTime,
             const double    theLightX,
             const double    theLightY,
             const GLfloat   theOpacity,
             const bool      theIsClicked,
             const GLfloat   theDispX) {
        StGLProgram::use(theCtx);
        theCtx.core20fwd->glUniform4fv(uniColorLoc, 1, theColor);
        theCtx.core20fwd->glUniform1f(uniTimeLoc, theTime);
        theCtx.core20fwd->glUniform1i(uniClickedLoc, (theIsClicked ? 20 : 2));
        theCtx.core20fwd->glUniform3f(uniParamsLoc, GLfloat(theLightX), GLfloat(theLightY), theOpacity);
        if(!stAreEqual(myDispX, theDispX, 0.0001f)) {
            myDispX = theDispX;
            theCtx.core20fwd->glUniform4fv(uniDispLoc,  1, StGLVec4(theDispX, 0.0f, 0.0f, 0.0f));
        }
    }

        private:

    GLfloat         myDispX;

    StGLVarLocation uniProjMatLoc;
    StGLVarLocation uniDispLoc;

    StGLVarLocation uniTimeLoc;
    StGLVarLocation uniClickedLoc;
    StGLVarLocation uniParamsLoc;

    StGLVarLocation uniColorLoc;

};

/**
 * This class defines matrix of GLSL programs for textured button widget.
 */
class StGLTextureButton::ButtonPrograms : public StGLProgramMatrix<1, 2, StGLTextureButton::Program> {

        public:

    /**
     * Color conversion options in GLSL Fragment Shader.
     */
    enum FragSection {
        FragSection_Main = 0, //!< section with main() function
        FragSection_GetColor, //!< read color values from textures
        FragSection_NB
    };

    /**
     * Color getter options in GLSL Fragment Shader.
     */
    enum FragGetColor {
        FragGetColor_RGB = 0,
        FragGetColor_Alpha,
        FragGetColor_NB
    };

        public:

    /**
     * Access program.
     */
    StHandle<StGLTextureButton::Program>& getProgram(const StGLTextureButton::ProgramIndex theIndex) { return myPrograms[theIndex]; }

    /**
     * Main constructor.
     */
    ButtonPrograms() {
        myTitle = "StGLTextureButton";
        registerFragmentShaderPart(FragSection_GetColor, FragGetColor_RGB,
            "uniform sampler2D uTexture;\n"
            "vec4 getColor(in vec2 theTexCoord) {\n"
            "    return texture2D(uTexture, theTexCoord);\n"
            "}\n\n");

        const char VERT_SHADER[] =
           "uniform mat4  uProjMat;\n"
           "uniform vec4  uDisp;\n"
           "uniform float uTime;\n"
           "uniform int   uClicked;\n"
            // per-vertex input
           "attribute vec4 vVertex;\n"
           "attribute vec2 vTexCoord;\n"
            // outs to fragment shader
           "varying vec2 fTexCoord;\n"

           "void main(void) {\n"
           "    fTexCoord = vTexCoord;\n"
           "    vec4 v = vVertex + uDisp;\n"
           "    if(uClicked > 10) {\n"
           "        v.z = v.z - 0.25;\n"
           "    } else {\n"
           "        v.z = v.z + sin(uTime * v.x + uTime) * cos(v.y + uTime) * 0.1;\n"
           "    }\n"
           "    gl_Position = uProjMat * v;\n"
           "}\n";

        const char FRAG_SHADER[] =
           "uniform vec3 uParams;\n"
           "varying vec2 fTexCoord;\n"
           "vec4 getColor(in vec2 theTexCoord);\n"
           "void main(void) {\n"
           "    vec4 aColor = getColor(fTexCoord);\n"
           "    float ups = 0.0;\n"
           "        float upsx = (uParams.x - fTexCoord.x);\n"
           "        upsx *= upsx;\n"
           "        float upsy = (uParams.y - fTexCoord.y);\n"
           "        upsy *= upsy;\n"
           "        ups = upsx + upsy;\n"
           "        ups = -ups * 0.2;\n"
           "        if(ups < -0.1) {\n"
           "            ups = -0.1;\n"
           "        }\n"
           "    aColor.rgb += 0.1 + ups;\n"
           "    aColor.a *= uParams.z;\n"
           "    gl_FragColor = aColor;\n"
           "}\n";

        registerVertexShaderPart  (0,                0, VERT_SHADER);
        registerFragmentShaderPart(FragSection_Main, 0, FRAG_SHADER);
    }

    /**
     * Release OpenGL resources.
     */
    ST_LOCAL virtual void release(StGLContext& theCtx) ST_ATTR_OVERRIDE {
        StGLProgramMatrix<1, 2, StGLTextureButton::Program>::release(theCtx);
        for(int aProgIter = 0; aProgIter < StGLTextureButton::ProgramIndex_NB; ++aProgIter) {
            StHandle<StGLTextureButton::Program>& aProgram = myPrograms[aProgIter];
            if(!aProgram.isNull()) {
                aProgram->release(theCtx);
                aProgram.nullify();
            }
        }
    }

    /**
     * Initialize all programs.
     */
    bool init(StGLContext& theCtx) {
        const char* FRAGMENT_GET_ALPHA = theCtx.arbTexRG
                                       ? "#define stTextureAlpha(theSampler, theCoords) texture2D(theSampler, theCoords).r\n"
                                       : "#define stTextureAlpha(theSampler, theCoords) texture2D(theSampler, theCoords).a\n";
        const char FRAGMENT_GET_COLOR[] =
            "uniform sampler2D uTexture;\n"
            "uniform vec4      uColor;\n"
            "vec4 getColor(in vec2 theTexCoord) {\n"
            "     vec4 aColor = uColor;\n"
            "     aColor.a *= 1.0 - stTextureAlpha(uTexture, theTexCoord);\n"
            "     return aColor;\n"
            "}\n\n";

        registerFragmentShaderPart(FragSection_GetColor, FragGetColor_Alpha,
                                   StString(FRAGMENT_GET_ALPHA) + FRAGMENT_GET_COLOR);

        setFragmentShaderPart(theCtx, FragSection_GetColor, FragGetColor_RGB);
        if(!initProgram(theCtx)) {
            release(theCtx);
            return false;
        }
        myPrograms[StGLTextureButton::ProgramIndex_WaveRGB] = myActiveProgram;
        myActiveProgram.nullify();

        setFragmentShaderPart(theCtx, FragSection_GetColor, FragGetColor_Alpha);
        if(!initProgram(theCtx)) {
            release(theCtx);
            return false;
        }
        myPrograms[StGLTextureButton::ProgramIndex_WaveAlpha] = myActiveProgram;
        return true;
    }

        private:

    StHandle<StGLTextureButton::Program> myPrograms[StGLTextureButton::ProgramIndex_NB];

};

namespace {
    static const size_t SHARE_PROGRAM_ID = StGLRootWidget::generateShareId();
}

StGLTextureButton::StGLTextureButton(StGLWidget*      theParent,
                                     const int        theLeft,
                                     const int        theTop,
                                     const StGLCorner theCorner,
                                     const size_t     theFacesCount)
: StGLWidget(theParent, theLeft, theTop, theCorner),
  myColor(getRoot()->getColorForElement(StGLRootWidget::Color_IconActive)),
  myShadowColor(0.0f, 0.0f, 0.0f, 1.0f),
  myFaceId(0),
  myOpacityScale(1.0f),
  myProgram(getRoot()->getShare(SHARE_PROGRAM_ID)),
  myProgramIndex(StGLTextureButton::ProgramIndex_WaveRGB),
  myHoldTimer(false),
  myHoldDuration(0.0),
  myWaveTimer(false),
  myAnimTime(0.0f),
  myAnim(Anim_Wave),
  myToDrawShadow(false) {
    if(theFacesCount != 0) {
        myTextures = new StGLTextureArray(theFacesCount);
    }
    StGLWidget::signals.onMouseUnclick = stSlot(this, &StGLTextureButton::doMouseUnclick);
}

StGLTextureButton::~StGLTextureButton() {
    StGLContext& aCtx = getContext();
    myVertBuf.release(aCtx);
    myTCrdBuf.release(aCtx);
    if(!myTextures.isNull()) {
        for(size_t anIter = 0; anIter < myTextures->size(); ++anIter) {
            myTextures->changeValue(anIter).release(aCtx);
        }
    }
}

void StGLTextureButton::setAction(const StHandle<StAction>& theAction) {
    myAction = theAction;
}

void StGLTextureButton::setTexturePath(const StString* theTexturesPaths,
                                       const size_t    theCount) {
    if(myTextures.isNull()) {
        if(theCount != 0) {
            myTextures = new StGLTextureArray(theCount);
        } else {
        #ifdef ST_DEBUG
            ST_DEBUG_LOG_AT("WARNING, Attempt to set an empty list of textures to StGLTextureButton!");
        #endif
            return;
        }
    }
    const size_t aNbTextures = (theCount > myTextures->size()) ? myTextures->size() : theCount;
#ifdef ST_DEBUG
    if(theCount != myTextures->size()) {
        ST_DEBUG_LOG_AT("WARNING, Not enough textures paths for StGLTextureButton!");
    }
#endif
    for(size_t aTexIter = 0; aTexIter < aNbTextures; ++aTexIter) {
        myTextures->changeValue(aTexIter).setName(theTexturesPaths[aTexIter]);
    }
}

void StGLTextureButton::setFaceId(const size_t theId) {
    if(myFaceId == theId
    || theId >= myTextures->size()) {
        return;
    }

    myFaceId = theId;
    const StGLNamedTexture& aTexture = myTextures->getValue(myFaceId);
    myProgramIndex = StGLTexture::isAlphaFormat(aTexture.getTextureFormat())
                   ? StGLTextureButton::ProgramIndex_WaveAlpha
                   : StGLTextureButton::ProgramIndex_WaveRGB;
}

void StGLTextureButton::stglResize() {
    StGLWidget::stglResize();
    StGLContext& aCtx = getContext();

    // update vertices
    StArray<StGLVec2> aVertices(myToDrawShadow ? 8 : 4);
    StRectI_t aRect = getRectPxAbsolute();
    aRect.left()   += myMargins.left;
    aRect.right()  -= myMargins.right;
    aRect.top()    += myMargins.top;
    aRect.bottom() -= myMargins.bottom;
    myRoot->getRectGl(aRect, aVertices, 0);
    if(myToDrawShadow) {
        aRect.move(StVec2<int>(1, 1));
        myRoot->getRectGl(aRect, aVertices, 4);
    }
    myVertBuf.init(aCtx, aVertices);

    // update projection matrix
    if(myProgram.isNull()) {
        return;
    }

    StHandle<StGLTextureButton::Program>& aProgram = myProgram->getProgram(myProgramIndex);
    if(aProgram.isNull()) {
        return;
    }

    aProgram->useTemp(aCtx);
    aProgram->setProjMat(aCtx, getRoot()->getScreenProjection());
    aProgram->unuse(aCtx);
}

bool StGLTextureButton::stglInit() {
    if(myTextures.isNull()) {
        return false;
    }

    StGLContext& aCtx = getContext();
    const StHandle<StResourceManager>& aResMgr = getRoot()->getResourceManager();
    for(size_t aFaceIter = 0; aFaceIter < myTextures->size(); ++aFaceIter) {
        StGLNamedTexture& aTexture = myTextures->changeValue(aFaceIter);
        if(aTexture.isValid()) {
            continue;
        }

        if(aTexture.getName().isEmpty()) {
            ST_DEBUG_LOG("StGLTextureButton, texture for face " + aFaceIter + " not set");
            continue;
        }

        StHandle<StResource> aRes = aResMgr->getResource(aTexture.getName());
        if(aRes.isNull()) {
            ST_DEBUG_LOG("StGLTextureButton, texture '" + aTexture.getName() + "' not found");
            continue;
        }

        uint8_t* aData     = NULL;
        int      aDataSize = 0;
        if(!aRes->isFile()
         && aRes->read()) {
            aData     = (uint8_t* )aRes->getData();
            aDataSize = aRes->getSize();
        }

        StAVImage anImage;
        if(!anImage.load(aRes->getPath(), StImageFile::ST_TYPE_PNG, aData, aDataSize)) {
            ST_DEBUG_LOG(anImage.getState());
            continue;
        }

        GLint anInternalFormat = GL_RGB;
        if(!StGLTexture::getInternalFormat(aCtx, anImage.getPlane().getFormat(), anInternalFormat)) {
            ST_ERROR_LOG("StGLTextureButton, texture '" + aTexture.getName() + "' has unsupported format!");
            continue;
        }

        aTexture.setTextureFormat(anInternalFormat);
        aTexture.init(aCtx, anImage.getPlane());
    }

    const StGLNamedTexture& aTexture = myTextures->getValue(myFaceId);
    if(aTexture.isValid()) {
        changeRectPx().right()  = getRectPx().left() + aTexture.getSizeX() + myMargins.left + myMargins.right;
        changeRectPx().bottom() = getRectPx().top()  + aTexture.getSizeY() + myMargins.top  + myMargins.bottom;
    }
    myProgramIndex = StGLTexture::isAlphaFormat(aTexture.getTextureFormat())
                   ? StGLTextureButton::ProgramIndex_WaveAlpha
                   : StGLTextureButton::ProgramIndex_WaveRGB;

    if(myProgram.isNull()) {
        myProgram.create(getRoot()->getContextHandle(), new ButtonPrograms());
        myProgram->init(aCtx);
    }

    if(!myProgram->isValid()) {
        return false;
    }

    StArray<StGLVec2> aDummyVert(myToDrawShadow ? 8 : 4);
    StArray<StGLVec2> aTexCoords(myToDrawShadow ? 8 : 4);
    aTexCoords[0] = StGLVec2(1.0f, 0.0f);
    aTexCoords[1] = StGLVec2(1.0f, 1.0f);
    aTexCoords[2] = StGLVec2(0.0f, 0.0f);
    aTexCoords[3] = StGLVec2(0.0f, 1.0f);
    if(myToDrawShadow) {
        aTexCoords[4] = aTexCoords[0];
        aTexCoords[5] = aTexCoords[1];
        aTexCoords[6] = aTexCoords[2];
        aTexCoords[7] = aTexCoords[3];
    }

    myVertBuf.init(aCtx, aDummyVert);
    myTCrdBuf.init(aCtx, aTexCoords);
    return true;
}

void StGLTextureButton::stglUpdate(const StPointD_t& theCursorZo,
                                   bool theIsPreciseInput) {
    if(!isVisible()) {
        return;
    }

    // handle hold button event
    if(myHoldTimer.isOn()) {
        const double anElapsed = myHoldTimer.getElapsedTime();
        const double aProgress = anElapsed - myHoldDuration;
        myHoldDuration = anElapsed;
        signals.onBtnHold(getUserData(), aProgress);
        if(!isClicked(ST_MOUSE_LEFT)) {
            myHoldTimer.stop();
        }
    } else if(isClicked(ST_MOUSE_LEFT)) {
        myHoldTimer.restart();
        myHoldDuration = 0.0;
    }

    // update animation
    if(myAnim == Anim_Wave) {
        if(isPointIn(theCursorZo)) {
            if(!myWaveTimer.isOn()) {
                myWaveTimer.restart();
            }
            myAnimTime = (float )myWaveTimer.getElapsedTimeInSec();
        } else {
            myWaveTimer.stop();
            myAnimTime = 0.0f;
        }
    }
    StGLWidget::stglUpdate(theCursorZo, theIsPreciseInput);
}

void StGLTextureButton::stglDraw(unsigned int ) {
    if(!isVisible()) {
        return;
    }

    StHandle<StGLTextureButton::Program>& aProgram = myProgram->getProgram(myProgramIndex);
    StGLNamedTexture& aTexture = myTextures->changeValue(myFaceId);
    const bool hasShadow = myToDrawShadow
                        && myProgramIndex == ProgramIndex_WaveAlpha;
    if( aProgram.isNull()
    || !aTexture.isValid()) {
        return;
    }

    StGLContext& aCtx = getContext();
    aCtx.core20fwd->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    aCtx.core20fwd->glEnable(GL_BLEND);
    aTexture.bind(aCtx);

    const StRectD_t  aRectGl  = getRectGl();
    const StPointD_t aMouseGl = getPointGl(getRoot()->getCursorZo());;
    bool toShiftZ = myAnim == Anim_Wave
                 && isClicked(ST_MOUSE_LEFT);
    aProgram->use( aCtx,
                   hasShadow ? myShadowColor : myColor,
                   myAnimTime,
                  (aMouseGl.x() - aRectGl.left()) /  aRectGl.width(),
                  (aRectGl.top()  - aMouseGl.y()) / -aRectGl.height(),
                   myOpacity * myOpacityScale,
                   toShiftZ,
                   getRoot()->getScreenDispX());

    myTCrdBuf.bindVertexAttrib(aCtx, aProgram->getVTexCoordLoc());
    myVertBuf.bindVertexAttrib(aCtx, aProgram->getVVertexLoc());
    aCtx.core20fwd->glDrawArrays(GL_TRIANGLE_STRIP, hasShadow ? 4 : 0, 4);
    if(hasShadow) {
        aProgram->setColor(aCtx, myColor);
        myVertBuf.bindVertexAttrib(aCtx, aProgram->getVVertexLoc());
        aCtx.core20fwd->glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    }
    myVertBuf.unBindVertexAttrib(aCtx, aProgram->getVVertexLoc());
    myTCrdBuf.unBindVertexAttrib(aCtx, aProgram->getVTexCoordLoc());

    aProgram->unuse(aCtx);
    aTexture.unbind(aCtx);
    aCtx.core20fwd->glDisable(GL_BLEND);
}

bool StGLTextureButton::tryClick(const StClickEvent& theEvent,
                                 bool&               theIsItemClicked) {
    if(StGLWidget::tryClick(theEvent, theIsItemClicked)) {
        theIsItemClicked = true;
        return true;
    }
    return false;
}

bool StGLTextureButton::tryUnClick(const StClickEvent& theEvent,
                                   bool&               theIsItemUnclicked) {
    if(StGLWidget::tryUnClick(theEvent, theIsItemUnclicked)) {
        theIsItemUnclicked = true;
        return true;
    }
    return false;
}

void StGLTextureButton::doMouseUnclick(const int theBtnId) {
    if(theBtnId == ST_MOUSE_LEFT) {
        if(!myAction.isNull()) {
            myAction->doTrigger(NULL);
        }
        signals.onBtnClick(getUserData());
    }
}

bool StGLTextureButton::doScroll(const StScrollEvent& theEvent) {
    StGLWidget::doScroll(theEvent);
    return true; // do not pass event further
}

StGLIcon::StGLIcon(StGLWidget*      theParent,
                   const int        theLeft,
                   const int        theTop,
                   const StGLCorner theCorner,
                   const size_t     theFacesCount)
: StGLTextureButton(theParent, theLeft, theTop, theCorner, theFacesCount),
  myIsExternalTexture(false) {
    myAnim = Anim_None;
}

StGLIcon::~StGLIcon() {
    if(myIsExternalTexture) {
        myTextures.nullify();
    }
}

bool StGLIcon::tryClick(const StClickEvent& , bool& ) {
    return false;
}

bool StGLIcon::tryUnClick(const StClickEvent& , bool& ) {
    return false;
}
