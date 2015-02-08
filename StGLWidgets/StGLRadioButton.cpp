/**
 * Copyright © 2011-2015 Kirill Gavrilov <kirill@sview.ru>
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
: StGLTextureButton(theParent,
                    theLeft, theTop,
                    theCorner,
                    0),
  myTrackValue(theTrackedValue),
  myProgram(getRoot()->getShare(SHARE_PROGRAM_ID)),
  myValueOn(theOnValue) {
    myAnim = Anim_None;
    StGLWidget::signals.onMouseUnclick = stSlot(this, &StGLRadioButton::doMouseUnclick);

    changeRectPx().right()  = getRectPx().left() + myRoot->scale(16);
    changeRectPx().bottom() = getRectPx().top()  + myRoot->scale(16);

    myTextures = myRoot->getRadioIcon();
    if(myTextures.isNull()) {
        const StString& anIcon0 = myRoot->getIcon(StGLRootWidget::IconImage_RadioButtonOff);
        const StString& anIcon1 = myRoot->getIcon(StGLRootWidget::IconImage_RadioButtonOn);
        if(!anIcon0.isEmpty()
        && !anIcon1.isEmpty()) {
            myTextures = new StGLTextureArray(2);
            myTextures->changeValue(0).setName(anIcon0);
            myTextures->changeValue(1).setName(anIcon1);
            myRoot->getRadioIcon() = myTextures;
        }
    }
}

StGLRadioButton::~StGLRadioButton() {
    myTextures.nullify(); // will be released by StGLRootWidget
    myVertBuf.release(getContext());
}

void StGLRadioButton::stglResize() {
    if(myProgram.isNull()) {
        StGLTextureButton::stglResize();
        return;
    }

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
    if(!myTextures.isNull()
     && StGLTextureButton::stglInit()) {
        return true;
    }

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

void StGLRadioButton::stglDraw(unsigned int theView) {
    if(!isVisible()) {
        return;
    }

    myFaceId = isActiveState() ? 1 : 0;
    if(myProgram.isNull()) {
        StGLTextureButton::stglDraw(theView);
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

        myProgram->setColor(aCtx, OUTER_COLORS[myFaceId], GLfloat(opacityValue));
        aCtx.core20fwd->glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        myProgram->setColor(aCtx, INNER_COLORS[myFaceId], GLfloat(opacityValue));
        aCtx.core20fwd->glDrawArrays(GL_TRIANGLE_STRIP, 4, 4);

    myVertBuf.unBindVertexAttrib(aCtx, myProgram->getVVertexLoc());
    myProgram->unuse(aCtx);
    aCtx.core20fwd->glDisable(GL_BLEND);
}

void StGLRadioButton::setValue() {
    myTrackValue->setValue(myValueOn);
}

void StGLRadioButton::doMouseUnclick(const int theBtnId) {
    if(theBtnId == ST_MOUSE_LEFT) {
        setValue();
    }
}
