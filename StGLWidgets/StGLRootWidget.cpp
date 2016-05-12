/**
 * StGLWidgets, small C++ toolkit for writing GUI using OpenGL.
 * Copyright © 2009-2016 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#include <StGLWidgets/StGLRootWidget.h>

#include <StGLWidgets/StGLMenuProgram.h>
#include <StGLWidgets/StGLMessageBox.h>
#include <StGLWidgets/StGLTextProgram.h>
#include <StGLWidgets/StGLTextBorderProgram.h>

#include <StCore/StEvent.h>
#include <StGL/StGLContext.h>
#include <StGLCore/StGLCore20.h>
#include <StFile/StFileNode.h>

namespace {

    // we do not use StAtomic<> template here to avoid static classes initialization ambiguity
    static volatile size_t ST_WIDGET_RES_COUNTER = 0;

    static const int THE_ICON_SIZES[StGLRootWidget::IconSizeNb + 1] = {
        16,
        24,
        32,
        48,
        64,
        72,
        96,
        128,
        144,
        192,
        256,
        0
    };

}

size_t StGLRootWidget::generateShareId() {
    return StAtomicOp::Increment(ST_WIDGET_RES_COUNTER);
}

StGLRootWidget::StGLRootWidget(const StHandle<StResourceManager>& theResMgr)
: StGLWidget(NULL, 0, 0, StGLCorner(ST_VCORNER_TOP, ST_HCORNER_LEFT)),
  myShareArray(new StGLSharePointer*[10]),
  myShareSize(10),
  myResMgr(theResMgr),
  myScrDispX(0.0f),
  myLensDist(0.0f),
  myScrDispXPx(0),
  myMenuProgram(new StGLMenuProgram()),
  myTextProgram(new StGLTextProgram()),
  myTextBorderProgram(new StGLTextBorderProgram()),
  myIsMobile(false),
  myScaleGlX(1.0),
  myScaleGlY(1.0),
  myScaleGUI(1.0f),
  myResolution(72),
  cursorZo(0.0, 0.0),
  myFocusWidget(NULL),
  myModalDialog(NULL),
  myIsMenuPressed(false),
  myMenuIconSize(IconSize_16),
  myClickThreshold(3) {
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
    myGlFontMgr = new StGLFontManager(myResolution);

    myColors[Color_Menu]            = StGLVec4(0.855f, 0.855f, 0.855f, 1.0f);
    myColors[Color_MenuHighlighted] = StGLVec4(0.765f, 0.765f, 0.765f, 1.0f);
    myColors[Color_MenuClicked]     = StGLVec4(0.500f, 0.500f, 0.500f, 1.0f);
    myColors[Color_MenuText]        = StGLVec4(0.000f, 0.000f, 0.000f, 1.0f);
    myColors[Color_MenuIcon]        = StGLVec4(0.000f, 0.000f, 0.000f, 0.8f);
    myColors[Color_MessageBox]      = StGLVec4(0.060f, 0.060f, 0.060f, 1.0f);
    myColors[Color_MessageText]     = StGLVec4(1.000f, 1.000f, 1.000f, 1.0f);
    myColors[Color_ScrollBar]       = StGLVec4(0.765f, 0.765f, 0.765f, 0.8f);
    myColors[Color_IconActive]      = StGLVec4(1.000f, 1.000f, 1.000f, 1.0f);

    setupTextures();
}

StGLRootWidget::~StGLRootWidget() {
    // force children destruction here to ensure shared resources no more in use
    destroyChildren();

    // release array of shared resources (handles should be released already!)
    for(size_t aResId = 0; aResId < myShareSize; ++aResId) {
        delete myShareArray[aResId];
    }
    delete[] myShareArray;
    if(!myGlCtx.isNull()) {
        myMenuProgram->release(*myGlCtx);
        myMenuProgram.nullify();
        myTextProgram->release(*myGlCtx);
        myTextProgram.nullify();
        myTextBorderProgram->release(*myGlCtx);
        myTextBorderProgram.nullify();
        if(!myCheckboxIcon.isNull()) {
            for(size_t aTexIter = 0; aTexIter < myCheckboxIcon->size(); ++aTexIter) {
                myCheckboxIcon->changeValue(aTexIter).release(*myGlCtx);
            }
            myCheckboxIcon.nullify();
        }
        if(!myRadioIcon.isNull()) {
            for(size_t aTexIter = 0; aTexIter < myRadioIcon->size(); ++aTexIter) {
                myRadioIcon->changeValue(aTexIter).release(*myGlCtx);
            }
            myRadioIcon.nullify();
        }
        myGlFontMgr->release(*myGlCtx);
        myGlFontMgr.nullify();
    }
}

StGLContext& StGLRootWidget::getContext() {
    return *myGlCtx;
}

const StHandle<StGLContext>& StGLRootWidget::getContextHandle() const {
    return myGlCtx;
}

void StGLRootWidget::setContext(const StHandle<StGLContext>& theCtx) {
    myGlCtx = theCtx;
}

void StGLRootWidget::setupTextures() {
    const IconSize anIconSize = scaleIcon(16);
    myIcons[IconImage_CheckboxOff]    = iconTexture(StString("textures" ST_FILE_SPLITTER) + "checkboxOff",    anIconSize);
    myIcons[IconImage_CheckboxOn]     = iconTexture(StString("textures" ST_FILE_SPLITTER) + "checkboxOn",     anIconSize);
    myIcons[IconImage_RadioButtonOff] = iconTexture(StString("textures" ST_FILE_SPLITTER) + "radioButtonOff", anIconSize);
    myIcons[IconImage_RadioButtonOn]  = iconTexture(StString("textures" ST_FILE_SPLITTER) + "radioButtonOn",  anIconSize);
    myIcons[IconImage_Folder]         = iconTexture(StString("textures" ST_FILE_SPLITTER) + "actionOpen",     anIconSize);
    myIcons[IconImage_File]           = iconTexture(StString("textures" ST_FILE_SPLITTER) + "actionFile",     anIconSize);
}

StMarginsI StGLRootWidget::iconMargins(StGLRootWidget::IconSize theStdSize,
                                       const int                theSize) const {
    const int aSize     = scale(theSize);
    const int anStdSize = THE_ICON_SIZES[theStdSize];
    if(aSize <= anStdSize) {
        return StMarginsI();
    }
    StMarginsI aMargins;
    aMargins.left   = (aSize - anStdSize) / 2;
    aMargins.right  =  aSize - anStdSize - aMargins.left;
    aMargins.top    = aMargins.left;
    aMargins.bottom = aMargins.right;
    return aMargins;
}

StGLRootWidget::IconSize StGLRootWidget::scaleIcon(const int theSize) const {
    const int anIconSize = scale(theSize);
    if(anIconSize < 20) {
        return IconSize_16;
    } else if(anIconSize < 30) {
        return IconSize_24;
    } else if(anIconSize < 42) {
        return IconSize_32;
    } else if(anIconSize < 62) {
        return IconSize_48;
    } else if(anIconSize < 70) {
        return IconSize_64;
    } else if(anIconSize < 94) {
        return IconSize_72;
    } else if(anIconSize < 126) {
        return IconSize_96;
    } else if(anIconSize < 140) {
        return IconSize_128;
    } else if(anIconSize < 190) {
        return IconSize_144;
    } else if(anIconSize < 240) {
        return IconSize_192;
    } else {
        return IconSize_256;
    }
}

StString StGLRootWidget::iconTexture(const StString& theName,
                                     const IconSize  theSize) const {
    for(int anIter = theSize; anIter >= 0; --anIter) {
        const StString aPath = theName + THE_ICON_SIZES[anIter] + ".png";
        if(myResMgr->isResourceExist(aPath)) {
            return aPath;
        }
    }
    for(int anIter = theSize + 1; anIter < IconSizeNb; ++anIter) {
        const StString aPath = theName + THE_ICON_SIZES[anIter] + ".png";
        if(myResMgr->isResourceExist(aPath)) {
            return aPath;
        }
    }

    const StString aPath = theName + ".png";
    if(myResMgr->isResourceExist(aPath)) {
        return aPath;
    }

    return "";
}

void StGLRootWidget::setScale(const GLfloat     theScale,
                              const ScaleAdjust theScaleAdjust) {
    GLfloat aScale = theScale;
    switch(theScaleAdjust) {
        case ScaleAdjust_Small:  aScale *= 0.8f; break;
        case ScaleAdjust_Big:    aScale *= 1.2f; break;
        default:
        case ScaleAdjust_Normal: break;
    }

    if(stAreEqual(myScaleGUI, aScale, 0.001f)) {
        return;
    }

    myScaleGUI   = aScale;
    myResolution = (unsigned int )(72.0f * aScale + 0.1f);
    myGlFontMgr->setResolution(myResolution);
    myMenuIconSize   = scaleIcon(16);
    myClickThreshold = scale(3);
    setupTextures();
}

bool StGLRootWidget::stglInit() {
    if(myGlCtx.isNull()) {
        myGlCtx = new StGLContext(myResMgr);
        if(!myGlCtx->stglInit()) {
            return false;
        }
    }

    if(!myMenuProgram->isValid()
    && !myMenuProgram->init(*myGlCtx)) {
        return false;
    } else if(!myTextProgram->isValid()
           && !myTextProgram->init(*myGlCtx)) {
        return false;
    } else if(!myTextBorderProgram->isValid()
           && !myTextBorderProgram->init(*myGlCtx)) {
        return false;
    }

    return StGLWidget::stglInit();
}

void StGLRootWidget::stglDraw(unsigned int theView) {
    myGlCtx->stglSyncState();
    myGlCtx->core20fwd->glGetIntegerv(GL_VIEWPORT, myViewport); // cache viewport

    switch(theView) {
        case ST_DRAW_LEFT:
            myScrDispX   =             myLensDist * GLfloat(0.5 * myRectGl.width());
            myScrDispXPx =  int(double(myLensDist) * 0.5 * double(rectPx.width()));
            break;
        case ST_DRAW_RIGHT:
            myScrDispX   =            -myLensDist * GLfloat(0.5 * myRectGl.width());
            myScrDispXPx = -int(double(myLensDist) * 0.5 * double(rectPx.width()));
            break;
        case ST_DRAW_MONO:
        default:
            myScrDispX   = 0.0f;
            myScrDispXPx = 0;
            break;
    }

    if(myTextProgram->isValid()) {
        myTextProgram->use(*myGlCtx);
        myTextProgram->setProjMat(*myGlCtx,       myProjCamera.getProjMatrix());
        myTextProgram->unuse(*myGlCtx);
    }
    if(myTextBorderProgram->isValid()) {
        myTextBorderProgram->use(*myGlCtx);
        myTextBorderProgram->setProjMat(*myGlCtx, myProjCamera.getProjMatrix());
        myTextBorderProgram->unuse(*myGlCtx);
    }

    StGLWidget::stglDraw(theView);
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

    theScissorRect.x() = myViewport[0] + GLint(aWidthFactor  * GLdouble(theRect.left())) + myScrDispXPx;
    theScissorRect.y() = myViewport[1] + GLint(aHeightFactor * GLdouble(aRootHeight - theRect.bottom()));

    theScissorRect.width()  = GLint(aWidthFactor  * GLdouble(theRect.width()));
    theScissorRect.height() = GLint(aHeightFactor * GLdouble(theRect.height()));
}

void StGLRootWidget::stglResize(const StGLBoxPx& theRectPx) {
    const bool isChanged = getRectPx().right()  != theRectPx.width()
                        || getRectPx().bottom() != theRectPx.height();

    myProjCamera.resize(theRectPx.width(), theRectPx.height());

    changeRectPx().right()  = theRectPx.width();  // (left, top) forced to zero point (0, 0)
    changeRectPx().bottom() = theRectPx.height();

    myProjCamera.getZParams(myRectGl);
    myScaleGlX = (myRectGl.right() - myRectGl.left()) / GLdouble(getRectPx().width());
    myScaleGlY = (myRectGl.top() - myRectGl.bottom()) / GLdouble(getRectPx().height());

    myScrProjMat = myProjCamera.getProjMatrix();
    myScrProjMat.translate(StGLVec3(0.0f, 0.0f, -myProjCamera.getZScreen()));

    if(myMenuProgram->isValid()) {
        myMenuProgram->use(*myGlCtx);
        myMenuProgram->setProjMat(*myGlCtx, getScreenProjection());
        myMenuProgram->unuse(*myGlCtx);
    }

    // update all child widgets
    if(isChanged) {
        StGLWidget::stglResize();
    }
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

bool StGLRootWidget::tryClick(const StClickEvent& theEvent,
                              bool&               theIsItemClicked) {
    const StPointD_t aCursorBack = cursorZo;
    cursorZo = StPointD_t(theEvent.PointX, theEvent.PointY);
    if(isPointIn(cursorZo)) {
        setClicked(theEvent.Button, true);
    }
    const bool aResult = StGLWidget::tryClick(theEvent, theIsItemClicked);
    cursorZo = aCursorBack;
    return aResult;
}

bool StGLRootWidget::tryUnClick(const StClickEvent& theEvent,
                                bool&               theIsItemUnclicked) {
    const StPointD_t aCursorBack = cursorZo;
    cursorZo = StPointD_t(theEvent.PointX, theEvent.PointY);
    if(isPointIn(cursorZo)) {
        setClicked(theEvent.Button, false);
    }

    const bool aResult = StGLWidget::tryUnClick(theEvent, theIsItemUnclicked);
    clearDestroyList();
    cursorZo = aCursorBack;
    return aResult;
}

bool StGLRootWidget::doScroll(const StScrollEvent& theEvent) {
    const StPointD_t aCursorBack = cursorZo;
    cursorZo = StPointD_t(theEvent.PointX, theEvent.PointY);

    const bool aResult = StGLWidget::doScroll(theEvent);
    clearDestroyList();
    cursorZo = aCursorBack;
    return aResult;
}

bool StGLRootWidget::doKeyDown(const StKeyEvent& theEvent) {
    bool isProcessed = false;
    if(myFocusWidget != NULL) {
        isProcessed = myFocusWidget->doKeyDown(theEvent);
        clearDestroyList();
    }
    return isProcessed;
}

bool StGLRootWidget::doKeyHold(const StKeyEvent& theEvent) {
    bool isProcessed = false;
    if(myFocusWidget != NULL) {
        isProcessed = myFocusWidget->doKeyHold(theEvent);
        clearDestroyList();
    }
    return isProcessed;
}

bool StGLRootWidget::doKeyUp(const StKeyEvent& theEvent) {
    bool isProcessed = false;
    if(myFocusWidget != NULL) {
        isProcessed = myFocusWidget->doKeyUp(theEvent);
        clearDestroyList();
    }
    return isProcessed;
}

void StGLRootWidget::destroyWithDelay(StGLWidget* theWidget) {
    if(theWidget == NULL) {
        return;
    }

    for(size_t anIter = 0; anIter < myDestroyList.size(); ++anIter) {
        if(theWidget == myDestroyList[anIter]) {
            return; // already appended
        }
    }
    myDestroyList.add(theWidget);
}

void StGLRootWidget::clearDestroyList() {
    for(size_t anIter = 0; anIter < myDestroyList.size(); ++anIter) {
        StGLWidget* aWidget = myDestroyList[anIter];
        delete aWidget;
    }
    myDestroyList.clear();
}

StGLWidget* StGLRootWidget::setFocus(StGLWidget* theWidget) {
    if(myFocusWidget == theWidget) {
        return myFocusWidget;
    }

    StGLWidget* aPrevWidget = myFocusWidget;
    if(aPrevWidget != NULL) {
        aPrevWidget->myHasFocus = false;
    }

    myFocusWidget = theWidget;
    if(theWidget == NULL) {
        // search for another top widget (prefer widget with greater visibility layer)
        for(StGLWidget* aChild = myChildren.getLast(); aChild != NULL; aChild = aChild->getPrev()) {
            if(aChild->isTopWidget()
            && aPrevWidget != aChild
            && aChild->isVisible()) {
                myFocusWidget = aChild;
                break;
            }
        }
    }

    if(myFocusWidget != NULL) {
        myFocusWidget->myHasFocus = true;
    }
    return aPrevWidget;
}

void StGLRootWidget::setModalDialog(StGLMessageBox* theWidget,
                                    const bool      theToReleaseOld) {
    if(myModalDialog == theWidget) {
        return;
    }

    if(theToReleaseOld && myModalDialog != NULL) {
        destroyWithDelay(myModalDialog);
    }
    myModalDialog = theWidget;
}
