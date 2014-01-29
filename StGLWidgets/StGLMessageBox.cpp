/**
 * Copyright © 2009-2014 Kirill Gavrilov <kirill@sview.ru
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#include <StGLWidgets/StGLMessageBox.h>
#include <StGLWidgets/StGLButton.h>
#include <StGLWidgets/StGLRootWidget.h>
#include <StGLWidgets/StGLScrollArea.h>

#include <StGL/StGLContext.h>
#include <StGLCore/StGLCore20.h>

#include <StCore/StEvent.h>

namespace {
    static const int OFFSET_PIXELS = 32;
};

StGLMessageBox::StGLMessageBox(StGLWidget*     theParent,
                               const StString& theTitle,
                               const StString& theText,
                               const int       theWidth,
                               const int       theHeight)
: StGLWidget(theParent, 0, 0, StGLCorner(ST_VCORNER_CENTER, ST_HCORNER_CENTER), theWidth, theHeight),
  myContent(NULL),
  myTitle(NULL),
  myBtnPanel(NULL),
  myDefaultBtn(NULL),
  myButtonsNb(0),
  myMarginLeft(0),
  myMarginRight(0),
  myMarginTop(0),
  myMarginBottom(0),
  myMinSizeY(0),
  myToAdjustY(true) {
    create(theTitle, theText, theWidth, theHeight);
}

StGLMessageBox::StGLMessageBox(StGLWidget*     theParent,
                               const StString& theTitle,
                               const StString& theText)
: StGLWidget(theParent, 0, 0, StGLCorner(ST_VCORNER_CENTER, ST_HCORNER_CENTER),
             theParent->getRoot()->scale(384),
             theParent->getRoot()->scale(200)),
  myContent(NULL),
  myTitle(NULL),
  myBtnPanel(NULL),
  myDefaultBtn(NULL),
  myButtonsNb(0),
  myMarginLeft(0),
  myMarginRight(0),
  myMarginTop(0),
  myMarginBottom(0),
  myMinSizeY(0),
  myToAdjustY(true) {
    create(theTitle, theText, myRoot->scale(384), myRoot->scale(200));
}

void StGLMessageBox::create(const StString& theTitle,
                            const StString& theText,
                            const int       theWidth,
                            const int       theHeight) {
    StGLWidget::signals.onMouseUnclick.connect(this, &StGLMessageBox::doMouseUnclick);

    myMarginLeft   = myRoot->scale(OFFSET_PIXELS);
    myMarginRight  = myRoot->scale(OFFSET_PIXELS);
    myMarginTop    = myRoot->scale(OFFSET_PIXELS);
    myMarginBottom = myRoot->scale(24 * 3);
    myMinSizeY     = myRoot->scale(200);

    int aTitleHeight = 0;
    if(!theTitle.isEmpty()) {
        myTitle = new StGLTextArea(this, 0, myMarginTop,
                                   StGLCorner(ST_VCORNER_TOP, ST_HCORNER_CENTER),
                                   theWidth - myMarginLeft - myMarginRight, theHeight - myMarginTop - myMarginBottom);
        myTitle->setupAlignment(StGLTextFormatter::ST_ALIGN_X_CENTER,
                                StGLTextFormatter::ST_ALIGN_Y_TOP);
        myTitle->setupStyle(StFTFont::Style_Bold);
        myTitle->setText(theTitle);
        myTitle->setTextColor(StGLVec3(1.0f, 1.0f, 1.0f));
        myTitle->setVisibility(true, true);
        int aWidth = 0;
        myTitle->computeTextWidth(GLfloat(myTitle->getRectPx().width()), aWidth, aTitleHeight);
        myTitle->changeRectPx().bottom() = myTitle->getRectPx().top() + aTitleHeight;
        //myTitle->stglInitAutoHeight();
    }

    const int aTitleOffset = aTitleHeight > 0 ? (aTitleHeight + myRoot->scale(24)) : 0;
    myMarginTop += aTitleOffset;
    myContent = new StGLScrollArea(this, myMarginLeft, myMarginTop,
                                   StGLCorner(ST_VCORNER_TOP, ST_HCORNER_LEFT),
                                   theWidth - myMarginLeft - myMarginRight, theHeight - myMarginTop - myMarginBottom);
    setText(theText);

    myIsTopWidget = true;
    getRoot()->setFocus(this); // take input focus

    myBtnPanel = new StGLWidget(this, 0, -myRoot->scale(24), StGLCorner(ST_VCORNER_BOTTOM, ST_HCORNER_CENTER), 0, myRoot->scale(24));
    myBtnPanel->setVisibility(true);
}

StGLMessageBox::~StGLMessageBox() {
    StGLContext& aCtx = getContext();
    myVertexBuf.release(aCtx);
    myProgram.release(aCtx);
}

void StGLMessageBox::setTitle(const StString& theTitle) {
    if(myTitle != NULL) {
        myTitle->setText(theTitle);
    }
}

void StGLMessageBox::setText(const StString& theText) {
    myContent->destroyChildren();
    if(theText.isEmpty()) {
        return;
    }

    StGLTextArea* aText = new StGLTextArea(myContent, 0, 0, StGLCorner(ST_VCORNER_TOP, ST_HCORNER_LEFT),
                                           myContent->getRectPx().width(), myContent->getRectPx().height());
    aText->setupAlignment(StGLTextFormatter::ST_ALIGN_X_CENTER,
                          StGLTextFormatter::ST_ALIGN_Y_TOP);
    aText->setText(theText);
    aText->setTextColor(StGLVec3(1.0f, 1.0f, 1.0f));
    aText->setVisibility(true, true);
}

bool StGLMessageBox::doNextButton(const int theDir) {
    if(myButtonsNb < 2) {
        return false;
    }

    StGLButton* aNewFocus = NULL;
    if(theDir > 0) {
        aNewFocus = dynamic_cast<StGLButton*>(myDefaultBtn->getNext());
        if(aNewFocus == NULL) {
            aNewFocus = dynamic_cast<StGLButton*>(myBtnPanel->getChildren()->getStart());
        }
    } else {
        aNewFocus = dynamic_cast<StGLButton*>(myDefaultBtn->getPrev());
        if(aNewFocus == NULL) {
            aNewFocus = dynamic_cast<StGLButton*>(myBtnPanel->getChildren()->getLast());
        }
    }
    if(aNewFocus == NULL) {
        return false;
    }

    myDefaultBtn->setFocus(false);
    myDefaultBtn = aNewFocus;
    myDefaultBtn->setFocus(true);
    return true;
}

bool StGLMessageBox::doKeyDown(const StKeyEvent& theEvent) {
    switch(theEvent.VKey) {
        case ST_VK_ESCAPE: {
            destroyWithDelay(this);
            return true;
        }
        case ST_VK_TAB:
            return doNextButton(1);
        case ST_VK_SPACE:
        case ST_VK_RETURN:
            if(myDefaultBtn != NULL) {
                myDefaultBtn->signals.onBtnClick(myDefaultBtn->getUserData());
            }
            return true;
        case ST_VK_UP:
            if(myContent->isScrollable()) {
                myContent->doScroll(1);
                return true;
            }
            return doNextButton(-1);
        case ST_VK_DOWN:
            if(myContent->isScrollable()) {
                myContent->doScroll(-1);
                return true;
            }
            return doNextButton(1);
        case ST_VK_LEFT:
            return doNextButton(-1);
        case ST_VK_RIGHT:
            return doNextButton(1);
        default:
            return false;
    }
}

StGLButton* StGLMessageBox::addButton(const StString& theTitle,
                                      const bool      theIsDefault,
                                      const int       theWidth) {
    int aRight = myBtnPanel->getRectPx().right();
    if(aRight > 0) {
        aRight += myRoot->scale(24);
    }

    StGLButton* aButton = new StGLButton(myBtnPanel, aRight, 0, theTitle);
    aButton->setCorner(StGLCorner(ST_VCORNER_TOP, ST_HCORNER_LEFT));
    aButton->setHeight(myRoot->scale(24));
    aButton->setVisibility(true, true);

    if(theWidth > 0) {
        aButton->setWidth(theWidth);
    }

    myBtnPanel->changeRectPx().right() += aButton->getWidth();
    if(++myButtonsNb > 1) {
        //
    }

    aButton->signals.onBtnClick += stSlot(this, &StGLMessageBox::doKillSelf);
    if(myDefaultBtn != NULL) {
        myDefaultBtn->setFocus(false);
    }
    if(theIsDefault || myDefaultBtn == NULL) {
        myDefaultBtn = aButton;
    }
    myDefaultBtn->setFocus(true);

    return aButton;
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
    if(myContent != NULL) {
        const StGLWidget* aContent = myContent->getChildren()->getStart();
        if(aContent != NULL
        && myToAdjustY) {
            // adjust message box to fit content
            const int aSizeYToFit = aContent->getRectPx().height() + myMarginTop + myMarginBottom;
            const int aNewSizeY   = stMax(myMinSizeY, stMin(aSizeYToFit, myParent->getRectPx().height() - myRoot->scale(120)));
            changeRectPx().bottom() = aNewSizeY;
            myContent->changeRectPx().bottom() = myContent->getRectPx().top() + aNewSizeY - myMarginTop - myMarginBottom;
        }
    }

    StGLWidget::stglResize();
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

    myProgram.use(aCtx, getRoot()->getScreenDispX());
    myProgram.setProjMat(aCtx, getCamera()->getProjMatrix());
    myProgram.setColor(aCtx, StGLVec4(0.06f, 0.06f, 0.06f, 1.0f), GLfloat(opacityValue) * 0.8f);

        myVertexBuf.bindVertexAttrib(aCtx, myProgram.getVVertexLoc());
        aCtx.core20fwd->glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        myVertexBuf.unBindVertexAttrib(aCtx, myProgram.getVVertexLoc());

    myProgram.unuse(aCtx);

    aCtx.core20fwd->glDisable(GL_BLEND);

    StGLBoxPx aScissorRect;
    stglScissorRect(aScissorRect);
    aCtx.stglSetScissorRect(aScissorRect, true);

    StGLWidget::stglDraw(theView); // draw children

    aCtx.stglResetScissorRect();
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
    if(isPointIn(theCursorZo)
    && StGLWidget::tryClick(theCursorZo, theMouseBtn, isItemClicked)) {
        isItemClicked = true;
        return true;
    }
    return false;
}

bool StGLMessageBox::tryUnClick(const StPointD_t& theCursorZo,
                                const int&        theMouseBtn,
                                bool&             isItemUnclicked) {
    if(isPointIn(theCursorZo)
    && StGLWidget::tryUnClick(theCursorZo, theMouseBtn, isItemUnclicked)) {
        isItemUnclicked = true;
        return true;
    }
    return false;
}

void StGLMessageBox::doKillSelf(const size_t ) {
    destroyWithDelay(this);
}

void StGLMessageBox::doMouseUnclick(const int theBtnId) {
    switch(theBtnId) {
        case ST_MOUSE_LEFT: {
            signals.onClickLeft(getUserData());
            break;
        }
        case ST_MOUSE_RIGHT: {
            signals.onClickRight(getUserData());
            break;
        }
    }
}
