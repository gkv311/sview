/**
 * StGLWidgets, small C++ toolkit for writing GUI using OpenGL.
 * Copyright © 2011-2015 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#include <StGLWidgets/StGLCheckbox.h>
#include <StGLWidgets/StGLMenuProgram.h>
#include <StGLWidgets/StGLRootWidget.h>

#include <StGL/StGLContext.h>
#include <StGLCore/StGLCore20.h>

namespace {

    static const StGLVec4 OUTER_COLORS[2] = {
        StGLVec4(0.0f, 0.0f, 0.0f, 0.5f), // off
        StGLVec4(0.0f, 0.0f, 0.0f, 0.5f)  // on
    };

    static const StGLVec4 INNER_COLORS[2] = {
        StGLVec4(0.0f, 0.0f, 0.0f, 0.0f), // off
        StGLVec4(1.0f, 1.0f, 1.0f, 0.5f)  // on
    };

}

StGLCheckbox::StGLCheckbox(StGLWidget* theParent,
                           const StHandle<StBoolParam>& theTrackedValue,
                           const int theLeft, const int theTop,
                           const StGLCorner theCorner)
: StGLTextureButton(theParent,
                    theLeft, theTop,
                    theCorner,
                    0),
  myTrackValue(theTrackedValue) {
    myAnim = Anim_None;
    StGLWidget::signals.onMouseUnclick = stSlot(this, &StGLCheckbox::doMouseUnclick);
    changeRectPx().right()  = getRectPx().left() + theParent->getRoot()->scale(16);
    changeRectPx().bottom() = getRectPx().top()  + theParent->getRoot()->scale(16);

    myTextures = myRoot->getCheckboxIcon();
    if(myTextures.isNull()) {
        const StString& anIcon0 = myRoot->getIcon(StGLRootWidget::IconImage_CheckboxOff);
        const StString& anIcon1 = myRoot->getIcon(StGLRootWidget::IconImage_CheckboxOn);
        if(!anIcon0.isEmpty()
        && !anIcon1.isEmpty()) {
            myTextures = new StGLTextureArray(2);
            myTextures->changeValue(0).setName(anIcon0);
            myTextures->changeValue(1).setName(anIcon1);
            myRoot->getCheckboxIcon() = myTextures;
        }
    }
}

StGLCheckbox::~StGLCheckbox() {
    myTextures.nullify(); // will be released by StGLRootWidget
    myVertBuf.release(getContext());
}

void StGLCheckbox::stglResize() {
    if(!myTextures.isNull()) {
        StGLTextureButton::stglResize();
        return;
    }

    // outer vertices
    StArray<StGLVec2> aVertices(8);
    StGLContext& aCtx = getContext();

    StRectI_t aRectPx = getRectPxAbsolute();
    aRectPx.left()   += myMargins.left;
    aRectPx.right()  -= myMargins.right;
    aRectPx.top()    += myMargins.top;
    aRectPx.bottom() -= myMargins.bottom;
    getRoot()->getRectGl(aRectPx, aVertices, 0);

    // inner vertices
    aRectPx.left()   += myRoot->scale(4);
    aRectPx.right()  -= myRoot->scale(4);
    aRectPx.top()    += myRoot->scale(4);
    aRectPx.bottom() -= myRoot->scale(4);
    getRoot()->getRectGl(aRectPx, aVertices, 4);
    myVertBuf.init(aCtx, aVertices);
    myIsResized = false;
}

bool StGLCheckbox::stglInit() {
    if(!myTextures.isNull()
    &&  StGLTextureButton::stglInit()) {
        return true;
    }

    // already initialized?
    myTextures.nullify();
    if(myVertBuf.isValid()) {
        return true;
    }

    // initialize GLSL program
    StGLContext& aCtx = getContext();
    StArray<StGLVec2> aDummyVert(8);
    if(!myVertBuf.init(aCtx, aDummyVert)) {
        return false;
    }

    stglResize();
    return true;
}

void StGLCheckbox::stglDraw(unsigned int theView) {
    if(!isVisible()) {
        return;
    }

    myFaceId = myTrackValue->getValue() ? 1 : 0;
    if(!myTextures.isNull()) {
        StGLTextureButton::stglDraw(theView);
        return;
    }

    StGLContext&     aCtx     = getContext();
    StGLMenuProgram& aProgram = myRoot->getMenuProgram();
    if(myIsResized) {
        stglResize();
    }

    aCtx.core20fwd->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    aCtx.core20fwd->glEnable(GL_BLEND);
    aProgram.use(aCtx, getRoot()->getScreenDispX());
    myVertBuf.bindVertexAttrib  (aCtx, aProgram.getVVertexLoc());

    aProgram.setColor(aCtx, OUTER_COLORS[myFaceId], myOpacity);
    aCtx.core20fwd->glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    aProgram.setColor(aCtx, INNER_COLORS[myFaceId], myOpacity);
    aCtx.core20fwd->glDrawArrays(GL_TRIANGLE_STRIP, 4, 4);

    myVertBuf.unBindVertexAttrib(aCtx, aProgram.getVVertexLoc());
    aProgram.unuse(aCtx);
    aCtx.core20fwd->glDisable(GL_BLEND);
}

void StGLCheckbox::reverseValue() {
    myTrackValue->reverse();
}

void StGLCheckbox::doMouseUnclick(const int theBtnId) {
    if(theBtnId == ST_MOUSE_LEFT) {
        myTrackValue->reverse();
    }
}
