/**
 * Copyright © 2009-2013 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#include <StGLWidgets/StGLRootWidget.h>

#include <StGL/StGLContext.h>
#include <StGLCore/StGLCore20.h>

namespace {
    // we do not use StAtomic<> template here to avoid static classes initialization ambiguity
    static volatile size_t ST_WIDGET_RES_COUNTER = 0;
    static const StString CLASS_NAME("StGLRootWidget");
};

size_t StGLRootWidget::generateShareId() {
    return StAtomicOp::Increment(ST_WIDGET_RES_COUNTER);
}

StGLRootWidget::StGLRootWidget()
: StGLWidget(NULL, 0, 0, StGLCorner(ST_VCORNER_TOP, ST_HCORNER_LEFT)),
  myShareArray(new StGLSharePointer*[10]),
  myShareSize(10),
  myProjCamera(),
  myScrProjMat(),
  myRectGl(),
  myScaleGlX(1.0),
  myScaleGlY(1.0),
  cursorZo(0.0, 0.0) {
    // unify access
    StGLWidget::myRoot = this;
    myViewport[0] = 0;
    myViewport[1] = 0;
    myViewport[2] = 1;
    myViewport[3] = 1;

    // allocate shared resources array
    for(size_t aResId = 0; aResId < myShareSize; ++aResId) {
        myShareArray[aResId] = new StGLSharePointer();
    }
}

StGLRootWidget::~StGLRootWidget() {
    // force children destruction here to ensure shared resources no more in use
    destroyChildren();

    // release array of shared resources (handles should be released already!)
    for(size_t aResId = 0; aResId < myShareSize; ++aResId) {
        delete myShareArray[aResId];
    }
    delete[] myShareArray;
}

StGLContext& StGLRootWidget::getContext() {
    return *myGlCtx;
}

const StHandle<StGLContext>& StGLRootWidget::getContextHandle() {
    return myGlCtx;
}

void StGLRootWidget::setContext(const StHandle<StGLContext>& theCtx) {
    myGlCtx = theCtx;
}

bool StGLRootWidget::stglInit() {
    if(myGlCtx.isNull()) {
        myGlCtx = new StGLContext();
        if(!myGlCtx->stglInit()) {
            return false;
        }
    }
    return StGLWidget::stglInit();
}

void StGLRootWidget::stglDraw(unsigned int theView) {
    myGlCtx->stglSyncState();
    myGlCtx->core20fwd->glGetIntegerv(GL_VIEWPORT, myViewport); // cache viewport
    StGLWidget::stglDraw(theView);
}

const StString& StGLRootWidget::getClassName() {
    return CLASS_NAME;
}

StGLSharePointer* StGLRootWidget::getShare(const size_t theResId) {
    if(theResId >= myShareSize) {
        size_t aSizeNew = theResId + 10;
        StGLSharePointer** anArrayNew = new StGLSharePointer*[aSizeNew];
        stMemCpy(anArrayNew, myShareArray, myShareSize * sizeof(StGLSharePointer*));
        delete[] myShareArray;
        for(size_t aResId = myShareSize; aResId < aSizeNew; ++aResId) {
            anArrayNew[aResId] = new StGLSharePointer();
        }
        myShareArray = anArrayNew;
        myShareSize = aSizeNew;
    }
    return myShareArray[theResId];
}

void StGLRootWidget::stglUpdate(const StPointD_t& theCursorZo) {
    cursorZo = theCursorZo;
    StGLWidget::stglUpdate(theCursorZo);
}

void StGLRootWidget::stglScissorRect(const StRectI_t& theRect,
                                     StGLBoxPx&       theScissorRect) const {
    const GLint aVPortWidth  = myViewport[2];
    const GLint aVPortHeight = myViewport[3];
    const GLint aRootWidth   = getRectPx().width();
    const GLint aRootHeight  = getRectPx().height();
    if(aRootWidth <= 0 || aRootHeight <= 0) {
        // just prevent division by zero - should never happen
        stMemZero(&theScissorRect, sizeof(StGLBoxPx));
        return;
    }

    // viewport could have different size in case of rendering to FBO
    const GLdouble aWidthFactor  = GLdouble(aVPortWidth)  / GLdouble(aRootWidth);
    const GLdouble aHeightFactor = GLdouble(aVPortHeight) / GLdouble(aRootHeight);

    theScissorRect.x() = myViewport[0] + GLint(aWidthFactor  * GLdouble(theRect.left()));
    theScissorRect.y() = myViewport[1] + GLint(aHeightFactor * GLdouble(aRootHeight - theRect.bottom()));

    theScissorRect.width()  = GLint(aWidthFactor  * GLdouble(theRect.width()));
    theScissorRect.height() = GLint(aHeightFactor * GLdouble(theRect.height()));
}

void StGLRootWidget::stglResize(const StRectI_t& theWinRectPx) {
    myProjCamera.resize(*myGlCtx, theWinRectPx.width(), theWinRectPx.height());

    changeRectPx().right()  = theWinRectPx.width();  // (left, top) forced to zero point (0, 0)
    changeRectPx().bottom() = theWinRectPx.height();

    myProjCamera.getZParams(myRectGl);
    myScaleGlX = (myRectGl.right() - myRectGl.left()) / GLdouble(getRectPx().width());
    myScaleGlY = (myRectGl.top() - myRectGl.bottom()) / GLdouble(getRectPx().height());

    myScrProjMat = myProjCamera.getProjMatrix();
    myScrProjMat.translate(StGLVec3(0.0f, 0.0f, -myProjCamera.getZScreen()));

    // update all child widgets
    StGLWidget::stglResize(theWinRectPx);
}

StRectD_t StGLRootWidget::getRectGl(const StRectI_t& theRectPx) const {
    StRectD_t aRectGl;
    aRectGl.left()   = myRectGl.left() + myScaleGlX * theRectPx.left();
    aRectGl.right()  =  aRectGl.left() + myScaleGlX * theRectPx.width();
    aRectGl.top()    = myRectGl.top()  - myScaleGlY * theRectPx.top();
    aRectGl.bottom() =  aRectGl.top()  - myScaleGlY * theRectPx.height();
    return aRectGl;
}

void StGLRootWidget::getRectGl(const StRectI_t& theRectPx,
                               StArray<StGLVec2>& theVertices,
                               const size_t theFromId) const {
    StRectD_t aRectGl = getRectGl(theRectPx);
    theVertices[theFromId + 0] = StGLVec2(GLfloat(aRectGl.right()), GLfloat(aRectGl.top()));
    theVertices[theFromId + 1] = StGLVec2(GLfloat(aRectGl.right()), GLfloat(aRectGl.bottom()));
    theVertices[theFromId + 2] = StGLVec2(GLfloat(aRectGl.left()),  GLfloat(aRectGl.top()));
    theVertices[theFromId + 3] = StGLVec2(GLfloat(aRectGl.left()),  GLfloat(aRectGl.bottom()));
}
