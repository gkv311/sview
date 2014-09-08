/**
 * Copyright © 2009-2013 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#include <StGLWidgets/StGLTextureButton.h>
#include <StGLWidgets/StGLRootWidget.h>

#include <StGL/StGLProgram.h>
#include <StGL/StGLResources.h>
#include <StGL/StGLContext.h>
#include <StGLCore/StGLCore20.h>

#include <StImage/StImageFile.h>
#include <StThreads/StProcess.h>

class StGLTextureButton::StButtonProgram : public StGLProgram {

        public:

    StButtonProgram()
    : StGLProgram("StGLTextureButton"),
      myDispX(0.0f) {}

    StGLVarLocation getVVertexLoc()   const { return StGLVarLocation(0); }
    StGLVarLocation getVTexCoordLoc() const { return StGLVarLocation(1); }

    void setProjMat(StGLContext&      theCtx,
                    const StGLMatrix& theProjMat) {
        theCtx.core20fwd->glUniformMatrix4fv(uniProjMatLoc, 1, GL_FALSE, theProjMat);
    }

    virtual bool init(StGLContext& theCtx) {
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
           "        v.z = v.z + sin(uTime * v.x + uTime) * cos(v.y + uTime) * 0.25;\n"
           "    }\n"
           "    gl_Position = uProjMat * v;\n"
           "}\n";

        const char FRAG_SHADER[] =
           "uniform sampler2D uTexture;\n"
           "uniform vec3 uParams;\n"
           "varying vec2 fTexCoord;\n"
           "void main(void) {\n"
           "    vec4 aColor = texture2D(uTexture, fTexCoord);\n"
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

        StGLVertexShader aVertexShader(StGLProgram::getTitle());
        aVertexShader.init(theCtx, VERT_SHADER);
        StGLAutoRelease aTmp1(theCtx, aVertexShader);

        StGLFragmentShader aFragmentShader(StGLProgram::getTitle());
        aFragmentShader.init(theCtx, FRAG_SHADER);
        StGLAutoRelease aTmp2(theCtx, aFragmentShader);
        if(!StGLProgram::create(theCtx)
           .attachShader(theCtx, aVertexShader)
           .attachShader(theCtx, aFragmentShader)
           .bindAttribLocation(theCtx, "vVertex",   getVVertexLoc())
           .bindAttribLocation(theCtx, "vTexCoord", getVTexCoordLoc())
           .link(theCtx)) {
            return false;
        }

        uniProjMatLoc   = StGLProgram::getUniformLocation(theCtx, "uProjMat");
        uniDispLoc      = StGLProgram::getUniformLocation(theCtx, "uDisp");
        uniTimeLoc      = StGLProgram::getUniformLocation(theCtx, "uTime");
        uniClickedLoc   = StGLProgram::getUniformLocation(theCtx, "uClicked");
        uniParamsLoc    = StGLProgram::getUniformLocation(theCtx, "uParams");

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

    void use(StGLContext&  theCtx,
             const double  theTime,
             const double  theLightX,
             const double  theLightY,
             const double  theOpacity,
             const bool    theIsClicked,
             const GLfloat theDispX) {
        StGLProgram::use(theCtx);
        theCtx.core20fwd->glUniform1f(uniTimeLoc, GLfloat(theTime));
        theCtx.core20fwd->glUniform1i(uniClickedLoc, (theIsClicked ? 20 : 2));
        theCtx.core20fwd->glUniform3f(uniParamsLoc, GLfloat(theLightX), GLfloat(theLightY), GLfloat(theOpacity));
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

};

namespace {
    static const StString CLASS_NAME("StGLTextureButton");
    static const size_t SHARE_PROGRAM_ID = StGLRootWidget::generateShareId();
};

StGLTextureButton::StGLTextureButton(StGLWidget*      theParent,
                                     const int        theLeft,
                                     const int        theTop,
                                     const StGLCorner theCorner,
                                     const size_t     theFacesCount)
: StGLWidget(theParent, theLeft, theTop, theCorner),
  myFaceId(0),
  myFacesCount(theFacesCount),
  myAnim(Anim_Wave),
  myTextures(theFacesCount),
  myTexturesPaths(theFacesCount),
  myProgram(getRoot()->getShare(SHARE_PROGRAM_ID)),
  myWaveTimer(false) {
    StGLWidget::signals.onMouseUnclick = stSlot(this, &StGLTextureButton::doMouseUnclick);
}

StGLTextureButton::~StGLTextureButton() {
    StGLContext& aCtx = getContext();
    myVertBuf.release(aCtx);
    myTCrdBuf.release(aCtx);
    for(size_t anIter = 0; anIter < myTextures.size(); ++anIter) {
        myTextures[anIter].release(aCtx);
    }
}

const StString& StGLTextureButton::getClassName() {
    return CLASS_NAME;
}

void StGLTextureButton::glWaveTimerControl() {
    if(isPointIn(getRoot()->getCursorZo())) {
        if(!myWaveTimer.isOn()) {
            myWaveTimer.restart();
        }
    } else {
        myWaveTimer.stop();
    }
}

void StGLTextureButton::setTexturePath(const StString* theTexturesPaths,
                                       const size_t    theCount) {
    size_t minCount = (theCount > myFacesCount) ? myFacesCount : theCount;
#ifdef __ST_DEBUG__
    if(theCount != myFacesCount) {
        ST_DEBUG_LOG_AT("WARNING, Not inaf textures paths for StGLTextureButton!");
    }
#endif
    for(size_t t = 0; t < minCount; ++t) {
        myTexturesPaths.changeValue(t) = theTexturesPaths[t];
    }
}

void StGLTextureButton::stglResize() {
    StGLWidget::stglResize();
    StGLContext& aCtx = getContext();

    // update vertices
    StArray<StGLVec2> aVertices(4);
    getRectGl(aVertices);
    myVertBuf.init(aCtx, aVertices);

    // update projection matrix
    if(!myProgram.isNull()) {
        myProgram->useTemp(aCtx);
        myProgram->setProjMat(aCtx, getRoot()->getScreenProjection());
        myProgram->unuse(aCtx);
    }
}

bool StGLTextureButton::stglInit() {
    bool result = true;
    StHandle<StImageFile> stImage = StImageFile::create();
    StGLContext& aCtx = getContext();
    if(!stImage.isNull()) {
        for(size_t t = 0; t < myFacesCount; ++t) {
            if(myTexturesPaths[t].isEmpty()) {
                ST_DEBUG_LOG("StGLTextureButton, texture for face " + t + " not set");
                continue;
            }
            if(!stImage->load(myTexturesPaths[t], StImageFile::ST_TYPE_PNG)) {
                ST_DEBUG_LOG(stImage->getState());
                continue;
            }
            changeRectPx().right()  = getRectPx().left() + (int )stImage->getSizeX();
            changeRectPx().bottom() = getRectPx().top()  + (int )stImage->getSizeY();
            myTextures[t].init(aCtx, stImage->getPlane());
        }
        stImage.nullify();
    }

    if(myProgram.isNull()) {
        myProgram.create(getRoot()->getContextHandle(), new StButtonProgram());
        myProgram->init(aCtx);
    }

    if(!myProgram->isValid()) {
        return false;
    }

    StArray<StGLVec2> aDummyVert(4);
    StArray<StGLVec2> aTexCoords(4);
    aTexCoords[0] = StGLVec2(1.0f, 0.0f);
    aTexCoords[1] = StGLVec2(1.0f, 1.0f);
    aTexCoords[2] = StGLVec2(0.0f, 0.0f);
    aTexCoords[3] = StGLVec2(0.0f, 1.0f);

    myVertBuf.init(aCtx, aDummyVert);
    myTCrdBuf.init(aCtx, aTexCoords);
    return result;
}

void StGLTextureButton::stglDraw(unsigned int ) {
    if(!isVisible()) {
        return;
    }

    StGLContext& aCtx = getContext();
    aCtx.core20fwd->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    aCtx.core20fwd->glEnable(GL_BLEND);
    myTextures[myFaceId].bind(aCtx);

    StRectD_t butRectGl = getRectGl();
    GLdouble butWGl = butRectGl.right() - butRectGl.left();
    GLdouble butHGl = butRectGl.top() - butRectGl.bottom();

    const StPointD_t aMouseGl = getPointGl(getRoot()->getCursorZo());;
    if(myAnim == Anim_Wave) {
        glWaveTimerControl();
    }

    myProgram->use(aCtx,
                   myWaveTimer.getElapsedTimeInSec(),
                  (aMouseGl.x() - butRectGl.left()) / butWGl,
                  (butRectGl.top()  - aMouseGl.y()) / butHGl,
                   opacityValue,
                   isClicked(ST_MOUSE_LEFT),
                   getRoot()->getScreenDispX());

    myVertBuf.bindVertexAttrib(aCtx, myProgram->getVVertexLoc());
    myTCrdBuf.bindVertexAttrib(aCtx, myProgram->getVTexCoordLoc());

    aCtx.core20fwd->glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    myTCrdBuf.unBindVertexAttrib(aCtx, myProgram->getVTexCoordLoc());
    myVertBuf.unBindVertexAttrib(aCtx, myProgram->getVVertexLoc());

    myProgram->unuse(aCtx);
    myTextures[myFaceId].unbind(aCtx);
    aCtx.core20fwd->glDisable(GL_BLEND);
}

bool StGLTextureButton::tryClick(const StPointD_t& cursorZo, const int& mouseBtn, bool& isItemClicked) {
    if(StGLWidget::tryClick(cursorZo, mouseBtn, isItemClicked)) {
        isItemClicked = true;
        return true;
    }
    return false;
}

bool StGLTextureButton::tryUnClick(const StPointD_t& cursorZo, const int& mouseBtn, bool& isItemUnclicked) {
    if(StGLWidget::tryUnClick(cursorZo, mouseBtn, isItemUnclicked)) {
        isItemUnclicked = true;
        return true;
    }
    return false;
}

void StGLTextureButton::doMouseUnclick(const int theBtnId) {
    if(theBtnId == ST_MOUSE_LEFT) {
        signals.onBtnClick(getUserData());
    }
}

StGLIcon::StGLIcon(StGLWidget*      theParent,
                   const int        theLeft,
                   const int        theTop,
                   const StGLCorner theCorner,
                   const size_t     theFacesCount)
: StGLTextureButton(theParent, theLeft, theTop, theCorner, theFacesCount) {
    myAnim = Anim_None;
}

bool StGLIcon::tryClick(const StPointD_t& , const int& , bool& ) {
    return false;
}

bool StGLIcon::tryUnClick(const StPointD_t& , const int& , bool& ) {
    return false;
}
