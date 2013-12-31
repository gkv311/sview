/**
 * Copyright © 2009-2013 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#include <StGLWidgets/StGLTextArea.h>
#include <StGLWidgets/StGLRootWidget.h>

#include <StGL/StGLContext.h>
#include <StGL/StGLProgram.h>
#include <StGLCore/StGLCore20.h>

#include <StFile/StFileNode.h>
#include <StThreads/StProcess.h>

class StGLTextArea::StTextProgram : public StGLProgram {

        public:

    StTextProgram()
    : StGLProgram("StGLTextArea, Text Program") {
        //
    }

    StGLVarLocation getVVertexLoc() const {
        return atrVVertexLoc;
    }

    StGLVarLocation getVTexCoordLoc() const {
        return atrVTexCoordLoc;
    }

    void setProjMat(StGLContext&      theCtx,
                    const StGLMatrix& theProjMat) {
        theCtx.core20fwd->glUniformMatrix4fv(uniProjMatLoc, 1, GL_FALSE, theProjMat);
    }

    void setModelMat(StGLContext&      theCtx,
                     const StGLMatrix& theModelMat) {
        theCtx.core20fwd->glUniformMatrix4fv(uniModelMatLoc, 1, GL_FALSE, theModelMat);
    }

    void setTextColor(StGLContext&    theCtx,
                      const StGLVec4& theTextColorVec) {
        theCtx.core20fwd->glUniform4fv(uniTextColorLoc, 1, theTextColorVec);
    }

    virtual bool init(StGLContext& theCtx) {
        const char VERTEX_SHADER[] =
           "uniform mat4 uProjMat; \
            uniform mat4 uModelMat; \
            attribute vec4 vVertex; \
            attribute vec2 vTexCoord; \
            varying vec2 fTexCoord; \
            void main(void) { \
                fTexCoord = vTexCoord; \
                gl_Position = uProjMat * uModelMat * vVertex; \
            }";

        const char FRAGMENT_GET_RED[] =
           "float getAlpha(void) { return texture2D(uTexture, fTexCoord).r; }";

        const char FRAGMENT_GET_ALPHA[] =
           "float getAlpha(void) { return texture2D(uTexture, fTexCoord).a; }";

        const char FRAGMENT_SHADER[] =
           "uniform sampler2D uTexture;"
           "uniform vec4 uTextColor;"
           "varying vec2 fTexCoord;"
           "float getAlpha(void);"
           "void main(void) {"
           "     vec4 color = uTextColor;"
           "     color.a *= getAlpha();"
           "     gl_FragColor = color;"
           "}";

        StGLVertexShader aVertexShader(StGLProgram::getTitle());
        aVertexShader.init(theCtx, VERTEX_SHADER);
        StGLAutoRelease aTmp1(theCtx, aVertexShader);

        StGLFragmentShader aFragmentShader(StGLProgram::getTitle());
        aFragmentShader.init(theCtx, FRAGMENT_SHADER,
                             theCtx.arbTexRG ? FRAGMENT_GET_RED : FRAGMENT_GET_ALPHA);
        StGLAutoRelease aTmp2(theCtx, aFragmentShader);
        if(!StGLProgram::create(theCtx)
           .attachShader(theCtx, aVertexShader)
           .attachShader(theCtx, aFragmentShader)
           .link(theCtx)) {
            return false;
        }

        uniProjMatLoc   = StGLProgram::getUniformLocation(theCtx, "uProjMat");
        uniModelMatLoc  = StGLProgram::getUniformLocation(theCtx, "uModelMat");
        uniTextColorLoc = StGLProgram::getUniformLocation(theCtx, "uTextColor");
        atrVVertexLoc   = StGLProgram::getAttribLocation(theCtx, "vVertex");
        atrVTexCoordLoc = StGLProgram::getAttribLocation(theCtx, "vTexCoord");

        StGLVarLocation uniTextureLoc = StGLProgram::getUniformLocation(theCtx, "uTexture");
        if(uniTextureLoc.isValid()) {
            StGLProgram::use(theCtx);
            theCtx.core20fwd->glUniform1i(uniTextureLoc, StGLProgram::TEXTURE_SAMPLE_0);
            StGLProgram::unuse(theCtx);
        }

        return uniProjMatLoc.isValid()
            && uniModelMatLoc.isValid()
            && uniTextColorLoc.isValid()
            && atrVVertexLoc.isValid()
            && atrVTexCoordLoc.isValid()
            && uniTextureLoc.isValid();
    }

        private:

    StGLVarLocation uniProjMatLoc;
    StGLVarLocation uniModelMatLoc;
    StGLVarLocation uniTextColorLoc;

    StGLVarLocation atrVVertexLoc;
    StGLVarLocation atrVTexCoordLoc;

};

class StGLTextArea::StBorderProgram : public StGLProgram {

        public:

    StBorderProgram()
    : StGLProgram("StGLTextArea, Border Program"),
      uniProjMatLoc(),
      uniModelMatLoc(),
      uniColorLoc(),
      atrVVertexLoc() {
        //
    }

    StGLVarLocation getVVertexLoc() const {
        return atrVVertexLoc;
    }

    void setProjMat(StGLContext&      theCtx,
                    const StGLMatrix& theProjMat) {
        theCtx.core20fwd->glUniformMatrix4fv(uniProjMatLoc, 1, GL_FALSE, theProjMat);
    }

    void setModelMat(StGLContext&      theCtx,
                     const StGLMatrix& theModelMat) {
        theCtx.core20fwd->glUniformMatrix4fv(uniModelMatLoc, 1, GL_FALSE, theModelMat);
    }

    void setColor(StGLContext&    theCtx,
                  const StGLVec4& theColorVec) {
        theCtx.core20fwd->glUniform4fv(uniColorLoc, 1, theColorVec);
    }

    virtual bool init(StGLContext& theCtx) {
        const char VERTEX_SHADER[] =
           "uniform mat4 uProjMat; \
            uniform mat4 uModelMat; \
            attribute vec4 vVertex; \
            void main(void) { \
                gl_Position = uProjMat * uModelMat * vVertex; \
            }";

        const char FRAGMENT_SHADER[] =
           "uniform vec4 uColor; \
            void main(void) { \
                gl_FragColor = uColor; \
            }";

        StGLVertexShader aVertexShader(StGLProgram::getTitle());
        aVertexShader.init(theCtx, VERTEX_SHADER);
        StGLAutoRelease aTmp1(theCtx, aVertexShader);

        StGLFragmentShader aFragmentShader(StGLProgram::getTitle());
        aFragmentShader.init(theCtx, FRAGMENT_SHADER);
        StGLAutoRelease aTmp2(theCtx, aFragmentShader);
        if(!StGLProgram::create(theCtx)
           .attachShader(theCtx, aVertexShader)
           .attachShader(theCtx, aFragmentShader)
           .link(theCtx)) {
            return false;
        }

        uniProjMatLoc  = StGLProgram::getUniformLocation(theCtx, "uProjMat");
        uniModelMatLoc = StGLProgram::getUniformLocation(theCtx, "uModelMat");
        uniColorLoc    = StGLProgram::getUniformLocation(theCtx, "uColor");
        atrVVertexLoc  = StGLProgram::getAttribLocation(theCtx, "vVertex");
        return uniProjMatLoc.isValid() && uniModelMatLoc.isValid() && uniColorLoc.isValid() && atrVVertexLoc.isValid();
    }

        private:

    StGLVarLocation uniProjMatLoc;
    StGLVarLocation uniModelMatLoc;
    StGLVarLocation uniColorLoc;

    StGLVarLocation atrVVertexLoc;

};

namespace {
    static const StString CLASS_NAME("StGLTextArea");
    static const size_t SHARE_TEXT_PROGRAM_ID   = StGLRootWidget::generateShareId();
    static const size_t SHARE_BORDER_PROGRAM_ID = StGLRootWidget::generateShareId();
};

StGLTextArea::StGLTextArea(StGLWidget* theParent,
                           const int theLeft, const int theTop,
                           const StGLCorner theCorner,
                           const int theWidth, const int theHeight,
                           const FontSize theSize)
: StGLWidget(theParent, theLeft, theTop, theCorner, theWidth, theHeight),
  myTextProgram(getRoot()->getShare(SHARE_TEXT_PROGRAM_ID)),
  myBorderProgram(getRoot()->getShare(SHARE_BORDER_PROGRAM_ID)),
  mySize(theSize),
  myTextColor(0.0f, 0.0f, 0.0f, 1.0f),
  myShadowColor(0.0f, 0.0f, 0.0f, 1.0f),
  myBackColor(0.365f, 0.722f, 1.0f, 1.0f),
  myBorderColor(0.0f, 0.0f, 0.0f, 1.0f),
  myMarginLeft(0),
  myMarginRight(0),
  myMarginTop(0),
  myMarginBottom(0),
  myTextDX(0.0f),
  myTextWidth(-1.0f),
  myToRecompute(true),
  myToShowBorder(false),
  myToDrawShadow(false),
  myIsInitialized(false) {
    myFont = getRoot()->getFontManager()->findCreate(StFTFont::Typeface_SansSerif, getFontSize());
}

StGLTextArea::~StGLTextArea() {
    StGLContext& aCtx = getContext();

    for(size_t anIter = 0; anIter < myTextVertBuf.size(); ++anIter) {
        myTextVertBuf[anIter]->release(aCtx);
        myTextTCrdBuf[anIter]->release(aCtx);
    }

    myBorderIVertBuf.release(aCtx);
    myBorderOVertBuf.release(aCtx);
}

const StString& StGLTextArea::getClassName() {
    return CLASS_NAME;
}

const StString& StGLTextArea::getText() const {
    return myText;
}

void StGLTextArea::setText(const StString& theText) {
    if(myText != theText) {
        myText = theText;
        myToRecompute = true;
    }
}

void StGLTextArea::setTextWidth(const int theWidth) {
    myTextWidth = (GLfloat )theWidth;
    myToRecompute = true;
}

bool StGLTextArea::stglInit() {
    if(myIsInitialized) {
        return true;
    }

    // initialize GL resources for the font
    StGLContext& aCtx = getContext();
    if(!myFont->wasInitialized()) {
        if( myFont->changeFont().isNull()
        ||  myFont->changeFont()->getFont().isNull()
        || !myFont->changeFont()->getFont()->isValid()) {
            return false; // critical error
        } else if(!myFont->stglInit(aCtx)) {
            ST_ERROR_LOG("Could not initialize OpenGL resources for font");
            return false;
        }
    } else if(!myFont->changeFont()->isValid()) {
        return false;
    }

    myTextWidth = (GLfloat )getRectPx().width();

    // initialize text program
    if(myTextProgram.isNull()) {
        myTextProgram.create(getRoot()->getContextHandle(), new StTextProgram());
        if(!myTextProgram->init(aCtx)) {
            return false;
        }
    } else if(!myTextProgram->isValid()) {
        return false;
    }

    // initialize border program
    if(myBorderProgram.isNull()) {
        myBorderProgram.create(getRoot()->getContextHandle(), new StBorderProgram());
        if(!myBorderProgram->init(aCtx)) {
            return false;
        }
    } else if(!myBorderProgram->isValid()) {
        return false;
    }

    myIsInitialized = true;

    myBorderIVertBuf.init(aCtx);
    myBorderOVertBuf.init(aCtx);

    if(isVisible()) {
        formatText(aCtx);
    }

    return StGLWidget::stglInit();
}

void StGLTextArea::recomputeBorder(StGLContext& theCtx) {
    GLfloat aTextAreaW = GLfloat(getRectPx().width());
    GLfloat aMarg      = GLfloat(myRoot->scale(3));
    const GLfloat quadVerticesInner[4 * 4] = {
        aMarg + aTextAreaW, myTextBndBox.top()    + aMarg, 0.0f, 1.0f, // top-right
        aMarg + aTextAreaW, myTextBndBox.bottom() - aMarg, 0.0f, 1.0f, // bottom-right
                    -aMarg, myTextBndBox.top()    + aMarg, 0.0f, 1.0f, // top-left
                    -aMarg, myTextBndBox.bottom() - aMarg, 0.0f, 1.0f  // bottom-left
    };
    aMarg += 1.0f;
    const GLfloat quadVerticesOuter[4 * 4] = {
        aMarg + aTextAreaW, myTextBndBox.top()    + aMarg, 0.0f, 1.0f, // top-right
        aMarg + aTextAreaW, myTextBndBox.bottom() - aMarg, 0.0f, 1.0f, // bottom-right
                    -aMarg, myTextBndBox.top()    + aMarg, 0.0f, 1.0f, // top-left
                    -aMarg, myTextBndBox.bottom() - aMarg, 0.0f, 1.0f  // bottom-left
    };
    myBorderIVertBuf.init(theCtx, 4, 4, quadVerticesInner);
    myBorderOVertBuf.init(theCtx, 4, 4, quadVerticesOuter);
}

void StGLTextArea::formatText(StGLContext& theCtx) {
    if(myToRecompute) {
        myFormatter.reset();
        myFormatter.append(theCtx, myText, StFTFont::Style_Regular, *myFont);
        myFormatter.format(myTextWidth, GLfloat(getRectPx().height()));
        myFormatter.getResult(theCtx, myTexturesList, myTextVertBuf, myTextTCrdBuf);
        myFormatter.getBndBox(myTextBndBox);
        if(myToShowBorder) {
            recomputeBorder(theCtx);
        }
        myToRecompute = false;
    }
}

void StGLTextArea::drawText(StGLContext& theCtx) {
    theCtx.core20fwd->glActiveTexture(GL_TEXTURE0);
    for(size_t aTextureIter = 0; aTextureIter < myTexturesList.size(); ++aTextureIter) {
        if(!myTextVertBuf[aTextureIter]->isValid() || myTextVertBuf[aTextureIter]->getElemsCount() < 1) {
            continue;
        }

        theCtx.core20fwd->glBindTexture(GL_TEXTURE_2D, myTexturesList[aTextureIter]);

        myTextVertBuf[aTextureIter]->bindVertexAttrib(theCtx, myTextProgram->getVVertexLoc());
        myTextTCrdBuf[aTextureIter]->bindVertexAttrib(theCtx, myTextProgram->getVTexCoordLoc());

        theCtx.core20fwd->glDrawArrays(GL_TRIANGLES, 0, GLsizei(myTextVertBuf[aTextureIter]->getElemsCount()));

        myTextTCrdBuf[aTextureIter]->unBindVertexAttrib(theCtx, myTextProgram->getVTexCoordLoc());
        myTextVertBuf[aTextureIter]->unBindVertexAttrib(theCtx, myTextProgram->getVVertexLoc());
    }
    theCtx.core20fwd->glBindTexture(GL_TEXTURE_2D, 0);
}

void StGLTextArea::stglDraw(unsigned int theView) {
    if(!myIsInitialized || !isVisible()) {
        return;
    }

    StGLContext& aCtx = getContext();
    myTextColor.a() = myShadowColor.a() = myBackColor.a() = myBorderColor.a() = (GLfloat )opacityValue;
    formatText(aCtx);

    StRectI_t aTextRectPx = getRectPx();
    aTextRectPx.left()   += myMarginLeft;
    aTextRectPx.right()  -= myMarginRight;
    aTextRectPx.top()    += myMarginTop;
    aTextRectPx.bottom() -= myMarginBottom;
    StRectD_t aTextRectGl = getRoot()->getRectGl(getAbsolute(aTextRectPx));

    // size corrector for FTGL
    StRectD_t zparams; getCamera()->getZParams(zparams);
    GLfloat aSizeOut = 2.0f * GLfloat(zparams.top()) / GLfloat(getRoot()->getRectPx().height());

    StGLMatrix aModelMat;
    aModelMat.translate(StGLVec3(getRoot()->getScreenDispX() + myTextDX, 0.0f, -getCamera()->getZScreen()));
    aModelMat.translate(StGLVec3(GLfloat(aTextRectGl.left()),
                                 GLfloat(aTextRectGl.top()),
                                 0.0f));
    aModelMat.scale(aSizeOut, aSizeOut, 0.0f);

    aCtx.core20fwd->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    aCtx.core20fwd->glEnable(GL_BLEND);

    // draw borders
    if(myToShowBorder) {
        if(myBorderOVertBuf.getElemsCount() == 0) {
            recomputeBorder(aCtx);
        }

        myBorderProgram->use(aCtx);
        myBorderProgram->setProjMat(aCtx, getCamera()->getProjMatrix());
        myBorderProgram->setModelMat(aCtx, aModelMat);

        myBorderProgram->setColor(aCtx, myBorderColor);
        myBorderOVertBuf.bindVertexAttrib(aCtx, myBorderProgram->getVVertexLoc());
        aCtx.core20fwd->glDrawArrays(GL_TRIANGLE_STRIP, 0, GLsizei(myBorderOVertBuf.getElemsCount()));
        myBorderOVertBuf.unBindVertexAttrib(aCtx, myBorderProgram->getVVertexLoc());

        myBorderProgram->setColor(aCtx, myBackColor);
        myBorderIVertBuf.bindVertexAttrib(aCtx, myBorderProgram->getVVertexLoc());
        aCtx.core20fwd->glDrawArrays(GL_TRIANGLE_STRIP, 0, GLsizei(myBorderIVertBuf.getElemsCount()));
        myBorderIVertBuf.unBindVertexAttrib(aCtx, myBorderProgram->getVVertexLoc());
        myBorderProgram->unuse(aCtx);
    }

    // draw text
    aCtx.core20fwd->glActiveTexture(GL_TEXTURE0); // our shader is bound to first texture unit
    myTextProgram->use(aCtx);
        myTextProgram->setProjMat(aCtx, getCamera()->getProjMatrix());
        myTextProgram->setModelMat(aCtx, aModelMat);
        myTextProgram->setTextColor(aCtx, myToDrawShadow ? myShadowColor : myTextColor);

        drawText(aCtx);

        if(myToDrawShadow) {
            aModelMat.initIdentity();
            aTextRectPx.left() -= 1;
            aTextRectPx.top()  -= 1;
            aTextRectGl = getRoot()->getRectGl(getAbsolute(aTextRectPx));
            aModelMat.translate(StGLVec3(getRoot()->getScreenDispX() + myTextDX, 0.0f, -getCamera()->getZScreen()));
            aModelMat.translate(StGLVec3(GLfloat(aTextRectGl.left()),
                                         GLfloat(aTextRectGl.top()),
                                         0.0f));
            aModelMat.scale(aSizeOut, aSizeOut, 0.0f);

            myTextProgram->setModelMat(aCtx, aModelMat);
            myTextProgram->setTextColor(aCtx, myTextColor);

            drawText(aCtx);
        }

    myTextProgram->unuse(aCtx);

    aCtx.core20fwd->glDisable(GL_BLEND);

    StGLWidget::stglDraw(theView);
}
