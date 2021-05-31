/**
 * StGLWidgets, small C++ toolkit for writing GUI using OpenGL.
 * Copyright © 2009-2016 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#include <StGLWidgets/StGLSeekBar.h>

#include <StGL/StGLProgram.h>
#include <StGL/StGLContext.h>
#include <StGLCore/StGLCore20.h>
#include <StGLWidgets/StGLRootWidget.h>
#include <StCore/StEvent.h>

class StGLSeekBar::StProgramSB : public StGLProgram {

        public:

    StProgramSB() : StGLProgram("StGLSeekBar") {}

    StGLVarLocation getVVertexLoc() const { return StGLVarLocation(0); }
    StGLVarLocation getVColorLoc()  const { return StGLVarLocation(1); }

    void setProjMat(StGLContext&      theCtx,
                    const StGLMatrix& theProjMat) {
        theCtx.core20fwd->glUniformMatrix4fv(uniProjMatLoc, 1, GL_FALSE, theProjMat);
    }

    using StGLProgram::use;
    void use(StGLContext& theCtx) {
        StGLProgram::use(theCtx);
    }

    void use(StGLContext&  theCtx,
             const GLfloat theOpacityValue,
             const GLfloat theDispX) {
        StGLProgram::use(theCtx);
        theCtx.core20fwd->glUniform1f(uniOpacityLoc, theOpacityValue);
        if(!stAreEqual(myDispX, theDispX, 0.0001f)) {
            myDispX = theDispX;
            theCtx.core20fwd->glUniform4fv(uniDispLoc,  1, StGLVec4(theDispX, 0.0f, 0.0f, 0.0f));
        }
    }

    virtual bool init(StGLContext& theCtx) ST_ATTR_OVERRIDE {
        const char VERTEX_SHADER[] =
           "uniform mat4  uProjMatrix;\n"
           "uniform vec4  uDisp;\n"
           "uniform float uOpacity;\n"
           "attribute vec4 vVertex;\n"
           "attribute vec4 vColor;\n"
           "varying vec4 fColor;\n"
           "void main(void) {\n"
           "    fColor = vec4(vColor.rgb, vColor.a * uOpacity);\n"
           "    gl_Position = uProjMatrix * (vVertex + uDisp);\n"
           "}\n";

        const char FRAGMENT_SHADER[] =
           "varying vec4 fColor;\n"
           "void main(void) {\n"
           "    gl_FragColor = fColor;\n"
           "}\n";

        StGLVertexShader aVertexShader(StGLProgram::getTitle());
        StGLAutoRelease aTmp1(theCtx, aVertexShader);
        aVertexShader.init(theCtx, VERTEX_SHADER);

        StGLFragmentShader aFragmentShader(StGLProgram::getTitle());
        StGLAutoRelease aTmp2(theCtx, aFragmentShader);
        aFragmentShader.init(theCtx, FRAGMENT_SHADER);
        if(!StGLProgram::create(theCtx)
           .attachShader(theCtx, aVertexShader)
           .attachShader(theCtx, aFragmentShader)
           .bindAttribLocation(theCtx, "vVertex", getVVertexLoc())
           .bindAttribLocation(theCtx, "vColor",  getVColorLoc())
           .link(theCtx)) {
            return false;
        }

        uniProjMatLoc = StGLProgram::getUniformLocation(theCtx, "uProjMatrix");
        uniDispLoc    = StGLProgram::getUniformLocation(theCtx, "uDisp");
        uniOpacityLoc = StGLProgram::getUniformLocation(theCtx, "uOpacity");
        return uniProjMatLoc.isValid() && uniOpacityLoc.isValid();
    }

        private:

    GLfloat         myDispX;
    StGLVarLocation uniProjMatLoc;
    StGLVarLocation uniDispLoc;
    StGLVarLocation uniOpacityLoc;

};

StGLSeekBar::StGLSeekBar(StGLWidget* theParent,
                         int theTop,
                         int theMargin,
                         StGLCorner theCorner)
: StGLWidget(theParent, 0, theTop, theCorner,
             theParent->getRoot()->scale(512),
             theParent->getRoot()->scale(12) + theMargin * 2),
  myProgram(new StProgramSB()),
  myProgress(0.0f),
  myProgressPx(0),
  myClickPos(-1),
  myMoveTolerPx(0) {
    StGLWidget::signals.onMouseClick  .connect(this, &StGLSeekBar::doMouseClick);
    StGLWidget::signals.onMouseUnclick.connect(this, &StGLSeekBar::doMouseUnclick);
    myMargins.top    = theMargin;
    myMargins.bottom = theMargin;
}

StGLSeekBar::~StGLSeekBar() {
    StGLContext& aCtx = getContext();
    if(!myProgram.isNull()) {
        myProgram->release(aCtx);
    }
    myVertices.release(aCtx);
    myColors.release(aCtx);
}

void StGLSeekBar::stglResize() {
    StGLWidget::stglResize();
    StGLContext& aCtx = getContext();

    stglUpdateVertices();

    // update projection matrix
    if(!myProgram.isNull()) {
        myProgram->use(aCtx);
        myProgram->setProjMat(aCtx, getRoot()->getScreenProjection());
        myProgram->unuse(aCtx);
    }
}

void StGLSeekBar::stglUpdateVertices() {
    StArray<StGLVec2> aVertices(12);

    // black border quad
    StRectI_t aRectPx(getRectPxAbsolute());
    aRectPx.top()    += myMargins.top;
    aRectPx.bottom() -= myMargins.bottom;
    aRectPx.left()   += myMargins.left;
    aRectPx.right()  -= myMargins.right;

    myRoot->getRectGl(aRectPx, aVertices, 0);

    // inner empty quad
    aRectPx.top()    += 1;
    aRectPx.bottom() -= 1;
    aRectPx.left()   += 1;
    aRectPx.right()  -= 1;
    myRoot->getRectGl(aRectPx, aVertices, 4);

    // inner filled quad
    myProgressPx = int(myProgress * GLfloat(aRectPx.width()));
    myProgressPx = stClamp(myProgressPx, 0, aRectPx.width());
    aRectPx.right() = aRectPx.left() + myProgressPx;
    myRoot->getRectGl(aRectPx, aVertices, 8);

    myVertices.init(getContext(), aVertices);
    myIsResized = false;
}

bool StGLSeekBar::stglInit() {
    StGLContext& aCtx = getContext();
    const GLfloat COLORS[4 * 12] = {
        // black border colors
        0.0f, 0.0f, 0.0f, 1.0f, // quad's top-right
        0.0f, 0.0f, 0.0f, 1.0f, // quad's bottom-right
        0.0f, 0.0f, 0.0f, 1.0f, // quad's top-left
        0.0f, 0.0f, 0.0f, 1.0f, // quad's bottom-left
        // empty color
        0.3f, 0.3f, 0.3f, 1.0f, // quad's top-right
        0.3f, 0.3f, 0.3f, 1.0f, // quad's bottom-right
        0.3f, 0.3f, 0.3f, 1.0f, // quad's top-left
        0.3f, 0.3f, 0.3f, 1.0f, // quad's bottom-left
        // fill color
        0.13f, 0.35f, 0.49f, 1.0f, // quad's top-right
        0.13f, 0.35f, 0.49f, 1.0f, // quad's bottom-right
        0.13f, 0.35f, 0.49f, 1.0f, // quad's top-left
        0.13f, 0.35f, 0.49f, 1.0f  // quad's bottom-left
    };

    myVertices.init(aCtx); // just generate buffer
    myColors.init(aCtx, 4, 12, COLORS);

    stglUpdateVertices();

    return myProgram->init(aCtx)
        && StGLWidget::stglInit();
}

void StGLSeekBar::stglDraw(unsigned int theView) {
    StGLContext& aCtx = getContext();

    // need to update vertices buffer?
    if(myIsResized
    || myProgressPx != int(myProgress * GLfloat((getRectPx().width() - myMargins.left - myMargins.right - 2)))) {
        stglUpdateVertices();
    }

    aCtx.core20fwd->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    aCtx.core20fwd->glEnable(GL_BLEND);
    myProgram->use(aCtx, myOpacity, myRoot->getScreenDispX());

    myVertices.bindVertexAttrib(aCtx, myProgram->getVVertexLoc());
    myColors  .bindVertexAttrib(aCtx, myProgram->getVColorLoc());

    aCtx.core20fwd->glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    aCtx.core20fwd->glDrawArrays(GL_TRIANGLE_STRIP, 4, 4);

    if(myProgressPx >= 1) {
        aCtx.core20fwd->glDrawArrays(GL_TRIANGLE_STRIP, 8, 4);
    }

    myColors  .unBindVertexAttrib(aCtx, myProgram->getVColorLoc());
    myVertices.unBindVertexAttrib(aCtx, myProgram->getVVertexLoc());

    myProgram->unuse(aCtx);
    aCtx.core20fwd->glDisable(GL_BLEND);

    StGLWidget::stglDraw(theView);
}

void StGLSeekBar::stglUpdate(const StPointD_t& theCursor,
                             bool theIsPreciseInput) {
    StGLWidget::stglUpdate(theCursor, theIsPreciseInput);
    if(!isClicked(ST_MOUSE_LEFT)) {
        return;
    }

    const int aRoundTol = myRoot->scale(1);
    const int aMaxPosPx = getRectPx().width();
    double aPos   = stMin(stMax(getPointInEx(theCursor), 0.0), 1.0);
    int    aPosPx = int(aPos * double(aMaxPosPx));
    const int aMoveTolerPx = myMoveTolerPx > 0 ? myMoveTolerPx : myRoot->scale(theIsPreciseInput ? 1 : 2);
    if(std::abs(aPosPx - 0) <= aRoundTol) {
        aPos   = 0.0;
        aPosPx = 0;
    } else if(std::abs(aPosPx - aMaxPosPx) <= aRoundTol) {
        aPos   = 1.0;
        aPosPx = aMaxPosPx;
    }

    if(myClickPos >= 0) {
        if(myClickPos == aPosPx) {
            return;
        }

        if(aPosPx != 0
        && aPosPx != aMaxPosPx
        && std::abs(aPosPx - myClickPos) < aMoveTolerPx) {
            return;
        }
    }

    myClickPos = aPosPx;
    signals.onSeekClick(ST_MOUSE_LEFT, aPos);
}

double StGLSeekBar::getPointInEx(const StPointD_t& thePointZo) const {
    StRectI_t aRectPx = getRectPxAbsolute();
    aRectPx.left()  += myMargins.left;
    aRectPx.right() -= myMargins.right;
    const StRectD_t  aRectGl  = myRoot->getRectGl(aRectPx);;
    const StPointD_t aPointGl = getPointGl(thePointZo);
    return (aPointGl.x() - aRectGl.left()) / (aRectGl.right() - aRectGl.left());
}

void StGLSeekBar::doMouseClick(const int ) {
    //
}

void StGLSeekBar::doMouseUnclick(const int mouseBtn) {
    const int aTolerance = myRoot->scale(1);
    const int aMaxPosPx  = getRectPx().width();
    double aPos   = stMin(stMax(getPointInEx(myRoot->getCursorZo()), 0.0), 1.0);
    int    aPosPx = int(aPos * double(aMaxPosPx));
    if(std::abs(aPosPx - 0) <= aTolerance) {
        aPos   = 0.0;
        aPosPx = 0;
    } else if(std::abs(aPosPx - aMaxPosPx) <= aTolerance) {
        aPos   = 1.0;
        aPosPx = aMaxPosPx;
    }

    if(myClickPos >= 0
    && std::abs(aPosPx - myClickPos) < aTolerance) {
        myClickPos = -1;
        return;
    }

    myClickPos = -1;
    signals.onSeekClick(mouseBtn, aPos);
}

bool StGLSeekBar::doScroll(const StScrollEvent& theEvent) {
    if(theEvent.StepsY >= 1) {
        signals.onSeekScroll(1.0);
    } else if(theEvent.StepsY <= -1) {
        signals.onSeekScroll(-1.0);
    }
    return true;
}
