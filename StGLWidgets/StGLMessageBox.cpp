/**
 * Copyright © 2009-2012 Kirill Gavrilov <kirill@sview.ru
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#include <StGLWidgets/StGLMessageBox.h>

#include <StGL/StGLContext.h>
#include <StGLCore/StGLCore20.h>

StGLMessageBox::StGLMessageBox(StGLWidget*     theParent,
                               const StString& theText,
                               const int       theWidth,
                               const int       theHeight)
: StGLWidget(theParent, 32, 32, StGLCorner(ST_VCORNER_TOP, ST_HCORNER_LEFT), theWidth, theHeight),
  myTextArea(NULL) {
    StGLWidget::signals.onMouseUnclick.connect(this, &StGLMessageBox::doMouseUnclick);

    const int OFFSET_PIXELS = 32;
    int anOffsetX = theWidth  > 2 * OFFSET_PIXELS ? OFFSET_PIXELS : 0;
    int anOffsetY = theHeight > 2 * OFFSET_PIXELS ? OFFSET_PIXELS : 0;
    myTextArea = new StGLTextArea(this, anOffsetX, anOffsetY, StGLCorner(ST_VCORNER_TOP, ST_HCORNER_LEFT),
                                  theWidth - 2 * anOffsetX, theHeight - 2 * anOffsetY, true);
    myTextArea->setupAlignment(StGLTextFormatter::ST_ALIGN_X_CENTER,
                               StGLTextFormatter::ST_ALIGN_Y_CENTER);

    myTextArea->setText(theText);
    myTextArea->setBorder(false);
    myTextArea->setTextColor(StGLVec3(1.0f, 1.0f, 1.0f));
}

StGLMessageBox::~StGLMessageBox() {
    StGLContext& aCtx = getContext();
    myVertexBuf.release(aCtx);
    myProgram.release(aCtx);
}

bool StGLMessageBox::stglInit() {
    if(!StGLWidget::stglInit()) {
        return false;
    }

    StGLContext& aCtx = getContext();
    const GLfloat QUAD_VERTICES[4 * 4] = {
         1.0f, -1.0f, 0.0f, 1.0f, // top-right
         1.0f,  1.0f, 0.0f, 1.0f, // bottom-right
        -1.0f, -1.0f, 0.0f, 1.0f, // top-left
        -1.0f,  1.0f, 0.0f, 1.0f  // bottom-left
    };
    myVertexBuf.init(aCtx, 4, 4, QUAD_VERTICES);
    stglResize();

    if(!myProgram.init(aCtx)) {
        return false;
    }
    return true;
}

void StGLMessageBox::stglResize() {
    // move to the center
    const int aCurrW = getRectPx().width();
    const int aCurrH = getRectPx().height();
    const int aParentW = getParent()->getRectPx().width();
    const int aParentH = getParent()->getRectPx().height();
    changeRectPx().left()   = aParentW / 2 - aCurrW / 2;
    changeRectPx().right()  = getRectPx().left() + aCurrW;
    changeRectPx().top()    = aParentH / 2 - aCurrH / 2;
    changeRectPx().bottom() = getRectPx().top() + aCurrH;

    GLfloat toZScreen = -getCamera()->getZScreen();

    StRectD_t aRectGl = getRectGl();
    const GLfloat aQuadVertices[4 * 4] = {
        GLfloat(aRectGl.right()), GLfloat(aRectGl.top()),    toZScreen, 1.0f, // top-right
        GLfloat(aRectGl.right()), GLfloat(aRectGl.bottom()), toZScreen, 1.0f, // bottom-right
        GLfloat(aRectGl.left()),  GLfloat(aRectGl.top()),    toZScreen, 1.0f, // top-left
        GLfloat(aRectGl.left()),  GLfloat(aRectGl.bottom()), toZScreen, 1.0f  // bottom-left
    };
    myVertexBuf.init(getContext(), 4, 4, aQuadVertices);
}

void StGLMessageBox::stglResize(const StRectI_t& theWinRectPx) {
    StGLWidget::stglResize(theWinRectPx);
    stglResize();
}

void StGLMessageBox::stglDraw(unsigned int theView) {
    if(!isVisible()) {
        return;
    }

    StGLContext& aCtx = getContext();
    if(isResized) {
        stglResize();
        isResized = false;
    }

    aCtx.core20fwd->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    aCtx.core20fwd->glEnable(GL_BLEND);

    myProgram.use(aCtx);
    myProgram.setProjMat(aCtx, getCamera()->getProjMatrix());
    myProgram.setColor(aCtx, StGLVec4(0.06f, 0.06f, 0.06f, 1.0f), GLfloat(opacityValue) * 0.8f);

        myVertexBuf.bindVertexAttrib(aCtx, myProgram.getVVertexLoc());
        aCtx.core20fwd->glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        myVertexBuf.unBindVertexAttrib(aCtx, myProgram.getVVertexLoc());

    myProgram.unuse(aCtx);

    aCtx.core20fwd->glDisable(GL_BLEND);

    StGLWidget::stglDraw(theView); // draw children
}

void StGLMessageBox::setVisibility(bool isVisible, bool isForce) {
    StGLWidget::setVisibility(isVisible, isForce);
    for(StGLWidget* aChildIter = getChildren()->getStart(); aChildIter != NULL; aChildIter = aChildIter->getNext()) {
        aChildIter->setVisibility(isVisible, isForce);
    }
}

bool StGLMessageBox::tryClick(const StPointD_t& theCursorZo,
                              const int&        theMouseBtn,
                              bool&             isItemClicked) {
    if(StGLWidget::tryClick(theCursorZo, theMouseBtn, isItemClicked)) {
        isItemClicked = true;
        return true;
    }
    return false;
}

bool StGLMessageBox::tryUnClick(const StPointD_t& theCursorZo,
                                const int&        theMouseBtn,
                                bool&             isItemUnclicked) {
    if(StGLWidget::tryUnClick(theCursorZo, theMouseBtn, isItemUnclicked)) {
        isItemUnclicked = true;
        return true;
    }
    return false;
}

void StGLMessageBox::doKillSelf(const size_t ) {
    delete this; /// TODO (Kirill Gavrilov#2) danger call!!!
}

void StGLMessageBox::doMouseUnclick(const int theBtnId) {
    if(theBtnId == ST_MOUSE_LEFT) {
        signals.onClickLeft(getUserData());
    } else if(theBtnId == ST_MOUSE_RIGHT) {
        signals.onClickRight(getUserData());
    }
}
