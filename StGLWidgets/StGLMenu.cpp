/**
 * Copyright © 2009-2013 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#include <StGLWidgets/StGLMenu.h>
#include <StGLWidgets/StGLMenuCheckbox.h>
#include <StGLWidgets/StGLMenuItem.h>
#include <StGLWidgets/StGLMenuProgram.h>
#include <StGLWidgets/StGLMenuRadioButton.h>
#include <StGLWidgets/StGLRootWidget.h>

#include <StGL/StGLContext.h>
#include <StGLCore/StGLCore20.h>

namespace {
    static const StString CLASS_NAME("StGLMenu");
    static const size_t SHARE_PROGRAM_ID = StGLRootWidget::generateShareId();
};

void StGLMenu::DeleteWithSubMenus(StGLMenu* theMenu) {
    if(theMenu == NULL) {
        return;
    }
    for(StGLWidget* aChild = theMenu->getChildren()->getStart(); aChild != NULL; aChild = aChild->getNext()) {
        StGLMenuItem* anItem = (StGLMenuItem* )aChild;
        if(anItem->getSubMenu() != NULL) {
            DeleteWithSubMenus(anItem->getSubMenu());
        }
    }
    delete theMenu;
}

StGLMenu::StGLMenu(StGLWidget* theParent,
                   const int   theLeft,
                   const int   theTop,
                   const int   theOrient,
                   const bool  theIsRootMenu)
: StGLWidget(theParent,
             theLeft, theTop,
             StGLCorner(ST_VCORNER_TOP, ST_HCORNER_LEFT),
             32, 32),
  myProgram(getRoot()->getShare(SHARE_PROGRAM_ID)),
  myColorVec(0.855f, 0.855f, 0.855f, 1.0f),
  myOrient(theOrient),
  myItemHeight(32),
  myWidth(0),
  myIsRootMenu(theIsRootMenu),
  myIsActive(!theIsRootMenu),
  myKeepActive(false),
  myIsInitialized(false) {
    //
}

StGLMenu::~StGLMenu() {
    myVertexBuf.release(getContext());
}

const StString& StGLMenu::getClassName() {
    return CLASS_NAME;
}

void StGLMenu::setVisibility(bool isVisible, bool isForce) {
    StGLWidget::setVisibility(isVisible, isForce);
    for(StGLWidget* aChild = getChildren()->getStart(); aChild != NULL; aChild = aChild->getNext()) {
        aChild->setVisibility(isVisible, isForce);
        if(!isVisible) {
            ((StGLMenuItem* )aChild)->setSelected(false);
        }
    }
}

void StGLMenu::stglResize() {
    StGLContext& aCtx = getContext();

    StArray<StGLVec2> aVertices(4);
    getRectGl(aVertices);
    myVertexBuf.init(aCtx, aVertices);

    if(!myProgram.isNull()) {
        myProgram->use(aCtx);
        myProgram->setProjMat(aCtx, getRoot()->getScreenProjection());
        myProgram->unuse(aCtx);
    }
}

void StGLMenu::stglResize(const StRectI_t& winRectPx) {
    StGLWidget::stglResize(winRectPx);
    stglResize();
}

void StGLMenu::stglUpdateSubmenuLayout() {
    stglInit();
    for(StGLWidget* aChild = getChildren()->getStart(); aChild != NULL; aChild = aChild->getNext()) {
        StGLMenuItem* aMenuItem = (StGLMenuItem* )aChild;
        if(aMenuItem->getSubMenu() != NULL) {
            aMenuItem->getSubMenu()->stglUpdateSubmenuLayout();
        }
    }
}

bool StGLMenu::stglInit() {
    myWidth = 0;
    myIsInitialized = StGLWidget::stglInit();
    if(!myIsInitialized) {
        return false;
    }
    for(StGLWidget* aChild = getChildren()->getStart(); aChild != NULL; aChild = aChild->getNext()) {
        StGLMenuItem* anItem = (StGLMenuItem* )aChild;
        int anItemW = anItem->computeTextWidth();
        if(myOrient == MENU_HORIZONTAL) {
            anItemW += 16; // extra margin at right
            anItem->changeRectPx().moveLeftTo(myWidth);
            anItem->changeRectPx().right() = anItem->getRectPx().left() + anItemW;
            anItem->setTextWidth(anItemW);
            myWidth += anItemW;
        } else {
            myWidth = (anItemW > myWidth) ? anItemW : myWidth;
        }
        if(anItem->getSubMenu() != NULL) {
            if(myOrient == MENU_HORIZONTAL) {
                anItem->getSubMenu()->changeRectPx().moveTopLeftTo(anItem->getRectPxAbsolute().left(), anItem->getRectPxAbsolute().bottom());
            } else if(myOrient == MENU_VERTICAL) {
                anItem->getSubMenu()->changeRectPx().moveTopLeftTo(anItem->getRectPxAbsolute().right() - 10, anItem->getRectPxAbsolute().top());
            }
        }
    }
    StGLWidget* aChildLast = getChildren()->getLast();
    if(aChildLast != NULL) {
        changeRectPx().right()  = getRectPx().left() + aChildLast->getRectPx().right();
        changeRectPx().bottom() = getRectPx().top()  + aChildLast->getRectPx().bottom();
    }
    if(myOrient == MENU_VERTICAL) {
        myWidth += 32 + 16; // icon offset + extra margin at right
        changeRectPx().right() = getRectPx().left() + myWidth;
        int anItemCount = 0;
        for(StGLWidget* aChild = getChildren()->getStart(); aChild != NULL; aChild = aChild->getNext(), ++anItemCount) {
            StGLMenuItem* anItem = (StGLMenuItem* )aChild;
            anItem->changeRectPx().moveTopTo(anItemCount * myItemHeight);
            anItem->changeRectPx().right() = anItem->getRectPx().left() + myWidth;
            anItem->setTextWidth(myWidth);
            if(anItem->getSubMenu() != NULL) {
                anItem->getSubMenu()->changeRectPx().moveTopLeftTo(getRectPxAbsolute().right() - 10,
                                                                   anItem->getRectPxAbsolute().top());
            }
        }
        changeRectPx().bottom() = getRectPx().top() + anItemCount * myItemHeight;
    }

    // already initialized?
    if(myVertexBuf.isValid()) {
        return true;
    }

    // initialize GLSL program
    StGLContext& aCtx = getContext();
    if(myProgram.isNull()) {
        myProgram.create(getRoot()->getContextHandle(), new StGLMenuProgram());
        if(!myProgram->init(aCtx)) {
            myIsInitialized = false;
            return myIsInitialized;
        }
    }

    StArray<StGLVec2> aDummyVert(4);
    myVertexBuf.init(aCtx, aDummyVert);
    stglResize();

    return myIsInitialized;
}

void StGLMenu::stglDraw(unsigned int theView) {
    if(!myIsInitialized || !isVisible()) {
        return;
    }

    if(isResized) {
        stglResize();
        isResized = false;
    }

    StGLContext& aCtx = getContext();
    aCtx.core20fwd->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    aCtx.core20fwd->glEnable(GL_BLEND);
    myProgram->use(aCtx);
    myProgram->setColor(aCtx, myColorVec, GLfloat(opacityValue));

    myVertexBuf.bindVertexAttrib(aCtx, myProgram->getVVertexLoc());
    aCtx.core20fwd->glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    myVertexBuf.unBindVertexAttrib(aCtx, myProgram->getVVertexLoc());

    myProgram->unuse(aCtx);
    aCtx.core20fwd->glDisable(GL_BLEND);

    StGLWidget::stglDraw(theView);
}

bool StGLMenu::tryUnClick(const StPointD_t& theCursorZo,
                          const int&        theMouseBtn,
                          bool&             theIsItemUnclicked) {
    myKeepActive = false;
    bool wasSomeClickedBefore = theIsItemUnclicked;
    bool isSelfClicked = StGLWidget::tryUnClick(theCursorZo, theMouseBtn, theIsItemUnclicked);
    bool isSelfItemClicked = !wasSomeClickedBefore && theIsItemUnclicked;
    if(myKeepActive) {
        return isSelfClicked;
    }

    for(StGLWidget* aChild = getChildren()->getStart(); aChild != NULL; aChild = aChild->getNext()) {
        StGLMenuItem* anItem = (StGLMenuItem* )aChild;
        if(anItem->hasSubMenu()
        && anItem->getSubMenu()->myKeepActive) {
            myKeepActive = true;
            return isSelfClicked;
        }
        anItem->setSelected(false);
    }

    if(isRootMenu() && !isSelfItemClicked) {
        setActive(false); // disactivate root menu
    }

    return isSelfClicked;
}

StGLMenuItem* StGLMenu::addItem(const StString& theLabel,
                                const size_t    theUserData) {
    StGLMenuItem* aNewItem = new StGLMenuItem(this, 0, 0);
    aNewItem->setText(theLabel);
    aNewItem->setUserData(theUserData);
    return aNewItem;
}

StGLMenuItem* StGLMenu::addItem(const StString& theLabel,
                                StGLMenu*       theSubMenu) {
    StGLMenuItem* aNewItem = new StGLMenuItem(this, 0, 0, theSubMenu);
    aNewItem->setText(theLabel);
    return aNewItem;
}

StGLMenuItem* StGLMenu::addItem(const StString&              theLabel,
                                const StHandle<StBoolParam>& theTrackedValue) {
    StGLMenuItem* aNewItem = new StGLMenuCheckbox(this, theTrackedValue);
    aNewItem->setText(theLabel);
    return aNewItem;
}

StGLMenuItem* StGLMenu::addItem(const StString&               theLabel,
                                const StHandle<StInt32Param>& theTrackedValue,
                                const int32_t                 theOnValue) {
    StGLMenuItem* aNewItem = new StGLMenuRadioButton(this, theTrackedValue, theOnValue);
    aNewItem->setText(theLabel);
    return aNewItem;
}

StGLMenuItem* StGLMenu::addItem(const StString&                 theLabel,
                                const StHandle<StFloat32Param>& theTrackedValue,
                                const float                     theOnValue) {
    StGLMenuItem* aNewItem = new StGLMenuRadioButton(this, theTrackedValue, theOnValue);
    aNewItem->setText(theLabel);
    return aNewItem;
}
