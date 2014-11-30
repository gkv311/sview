/**
 * Copyright © 2011-2014 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#include <StGLWidgets/StGLRadioButton.h>
#include <StGLWidgets/StGLMenuProgram.h>
#include <StGLWidgets/StGLRootWidget.h>

#include <StGL/StGLContext.h>
#include <StGLCore/StGLCore20.h>

namespace {

    static const size_t SHARE_PROGRAM_ID = StGLRootWidget::generateShareId();

    static const StGLVec4 OUTER_COLORS[2] = {
        StGLVec4(0.0f, 0.0f, 0.0f, 0.5f), // off
        StGLVec4(0.0f, 0.0f, 0.0f, 0.5f)  // on
    };

    static const StGLVec4 INNER_COLORS[2] = {
        StGLVec4(0.0f, 0.0f, 0.0f, 0.0f), // off
        StGLVec4(1.0f, 1.0f, 1.0f, 0.5f)  // on
    };

}

StGLRadioButton::StGLRadioButton(StGLWidget* theParent,
                                 const StHandle<StInt32Param>& theTrackedValue,
                                 const int32_t theOnValue,
                                 const int theLeft, const int theTop,
                                 const StGLCorner theCorner)
: StGLWidget(theParent,
             theLeft, theTop,
             theCorner,
             theParent->getRoot()->scale(16),
             theParent->getRoot()->scale(16)), // default dimensions = 16 x 16
  myTrackValue(theTrackedValue),
  myProgram(getRoot()->getShare(SHARE_PROGRAM_ID)),
  myVertBuf(),
  myValueOn(theOnValue) {
    StGLWidget::signals.onMouseUnclick = stSlot(this, &StGLRadioButton::doMouseUnclick);
}

StGLRadioButton::~StGLRadioButton() {
    myVertBuf.release(getContext());
}

void StGLRadioButton::stglResize() {
    StGLWidget::stglResize();

    StGLContext& aCtx = getContext();

    // outer vertices
    StRectI_t aRectPx = getRectPxAbsolute();
    StArray<StGLVec2> aVertices(8);
    getRoot()->getRectGl(aRectPx, aVertices, 0);

    // inner vertices
    aRectPx.left()   += myRoot->scale(4);
    aRectPx.right()  -= myRoot->scale(4);
    aRectPx.top()    += myRoot->scale(4);
    aRectPx.bottom() -= myRoot->scale(4);
    getRoot()->getRectGl(aRectPx, aVertices, 4);
    myVertBuf.init(aCtx, aVertices);

    // update projection matrix
    if(!myProgram.isNull()) {
        myProgram->use(aCtx);
        myProgram->setProjMat(aCtx, getRoot()->getScreenProjection());
        myProgram->unuse(aCtx);
    }
}

bool StGLRadioButton::stglInit() {
    // already initialized?
    if(myVertBuf.isValid()) {
        return true;
    }

    // initialize GLSL program
    StGLContext& aCtx = getContext();
    if(myProgram.isNull()) {
        myProgram.create(getRoot()->getContextHandle(), new StGLMenuProgram());
        if(!myProgram->init(aCtx)) {
            return false;
        }
    }

    StArray<StGLVec2> aDummyVert(8);
    myVertBuf.init(aCtx, aDummyVert);

    stglResize();
    return true;
}

bool StGLRadioButton::isActiveState() const {
    return myTrackValue->getValue() == myValueOn;
}

void StGLRadioButton::stglDraw(unsigned int ST_UNUSED(theView)) {
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
    myProgram->use(aCtx, getRoot()->getScreenDispX());
    myVertBuf.bindVertexAttrib(aCtx, myProgram->getVVertexLoc());

        bool isChecked = isActiveState();
        myProgram->setColor(aCtx, OUTER_COLORS[isChecked ? 1 : 0], GLfloat(opacityValue));
        aCtx.core20fwd->glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        myProgram->setColor(aCtx, INNER_COLORS[isChecked ? 1 : 0], GLfloat(opacityValue));
        aCtx.core20fwd->glDrawArrays(GL_TRIANGLE_STRIP, 4, 4);

    myVertBuf.unBindVertexAttrib(aCtx, myProgram->getVVertexLoc());
    myProgram->unuse(aCtx);
    aCtx.core20fwd->glDisable(GL_BLEND);
}

bool StGLRadioButton::tryClick(const StPointD_t& cursorZo, const int& mouseBtn, bool& isItemClicked) {
    if(StGLWidget::tryClick(cursorZo, mouseBtn, isItemClicked)) {
        isItemClicked = true; // always clickable widget
        return true;
    }
    return false;
}

bool StGLRadioButton::tryUnClick(const StPointD_t& cursorZo, const int& mouseBtn, bool& isItemUnclicked) {
    if(StGLWidget::tryUnClick(cursorZo, mouseBtn, isItemUnclicked)) {
        isItemUnclicked = true; // always clickable widget
        return true;
    }
    return false;
}

void StGLRadioButton::setValue() {
    myTrackValue->setValue(myValueOn);
}

void StGLRadioButton::doMouseUnclick(const int theBtnId) {
    if(theBtnId == ST_MOUSE_LEFT) {
        setValue();
    }
}
