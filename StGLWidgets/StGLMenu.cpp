/**
 * StGLWidgets, small C++ toolkit for writing GUI using OpenGL.
 * Copyright © 2009-2016 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#include <StGLWidgets/StGLMenu.h>

#include <StGLWidgets/StGLMenuCheckbox.h>
#include <StGLWidgets/StGLMenuItem.h>
#include <StGLWidgets/StGLMenuProgram.h>
#include <StGLWidgets/StGLMenuRadioButton.h>
#include <StGLWidgets/StGLRootWidget.h>

#include <StGL/StGLContext.h>
#include <StGLCore/StGLCore20.h>

#include <StCore/StEvent.h>
#include <StSettings/StEnumParam.h>
#include <StSlots/StAction.h>
#include <stAssert.h>

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
             theParent->getRoot()->scale(32),
             theParent->getRoot()->scale(32)),
  myColorVec(getRoot()->getColorForElement(StGLRootWidget::Color_Menu)),
  myOrient(theOrient),
  myItemHeight(theParent->getRoot()->scale(theParent->getRoot()->isMobile() ? 40 : 32)),
  myWidthMin(0),
  myWidth(0),
  myIsRootMenu(theIsRootMenu),
  myIsContextual(false),
  myIsActive(!theIsRootMenu),
  myKeepActive(false),
  myIsInitialized(false),
  myToDrawBounds(false) {
    myOpacity = theIsRootMenu || (myOrient == StGLMenu::MENU_ZERO)
              ? 1.0f : 0.0f;
}

StGLMenu::~StGLMenu() {
    myVertexBuf   .release(getContext());
    myVertexBndBuf.release(getContext());
}

void StGLMenu::setOpacity(const float theOpacity, bool theToSetChildren) {
    bool wasVisible = StGLMenu::isVisible();
    StGLWidget::setOpacity(theOpacity, theToSetChildren);
    if(!StGLMenu::isVisible()
     && wasVisible) {
        for(StGLWidget* aChild = getChildren()->getStart(); aChild != NULL; aChild = aChild->getNext()) {
            ((StGLMenuItem* )aChild)->setSelected(false);
        }
    }
}

void StGLMenu::setContextual(const bool theValue) {
    myIsContextual = theValue;
    myIsTopWidget  = theValue;
}

void StGLMenu::stglResize() {
    // Since all children should be StGLMenuItem implementing delayed resize,
    // just postpone resize until items will be actually rendered.
    //StGLWidget::stglResize();
    for(StGLWidget* aChild = getChildren()->getStart(); aChild != NULL; aChild = aChild->getNext()) {
        ((StGLMenuItem* )aChild)->changeRectPx();
    }

    StGLContext& aCtx = getContext();

    StArray<StGLVec2> aVertices(4);
    getRectGl(aVertices);
    myVertexBuf.init(aCtx, aVertices);

    if(myToDrawBounds) {
        StRectI_t aRectBnd = getRectPxAbsolute();
        aRectBnd.left()   -= 1;
        aRectBnd.right()  += 1;
        aRectBnd.top()    -= 1;
        aRectBnd.bottom() += 1;
        myRoot->getRectGl(aRectBnd, aVertices);
        myVertexBndBuf.init(aCtx, aVertices);
    }
    myIsResized = false;
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
    int aMarginLeft = 0;
    for(StGLWidget* aChild = getChildren()->getStart(); aChild != NULL; aChild = aChild->getNext()) {
        StGLMenuItem* anItem = (StGLMenuItem* )aChild;
        aMarginLeft = stMax(aMarginLeft, anItem->getMargins().left);
        int anItemW = anItem->getMargins().left + anItem->computeTextWidth() + anItem->getMargins().right;
        if(myOrient == MENU_HORIZONTAL) {
            anItem->changeRectPx().moveLeftTo(myWidth);
            anItem->changeRectPx().right() = anItem->getRectPx().left() + anItemW;
            anItem->setTextWidth(anItemW - anItem->getMargins().left);
            myWidth += anItemW;
        } else {
            myWidth = stMax(myWidth, anItemW);
        }
        if(StGLMenu* aSubMenu = anItem->getSubMenu()) {
            if(myOrient == MENU_HORIZONTAL) {
                aSubMenu->changeRectPx().moveTopLeftTo(anItem->getRectPxAbsolute().left()   - myRoot->getRectPx().left(),
                                                       anItem->getRectPxAbsolute().bottom() - myRoot->getRectPx().top());
            } else if(myOrient == MENU_VERTICAL
                   || myOrient == MENU_VERTICAL_COMPACT) {
                aSubMenu->changeRectPx().moveTopLeftTo(anItem->getRectPxAbsolute().right() - myRoot->scale(10) - myRoot->getRectPx().left(),
                                                       anItem->getRectPxAbsolute().top() - myRoot->getRectPx().top());
            }
        }
    }
    StGLWidget* aChildLast = getChildren()->getLast();
    if(aChildLast != NULL) {
        changeRectPx().right()  = getRectPx().left() + aChildLast->getRectPx().right();
        changeRectPx().bottom() = getRectPx().top()  + aChildLast->getRectPx().bottom();
    }
    int aWidth = stMax(myWidthMin, myWidth);
    if(myOrient == MENU_VERTICAL
    || myOrient == MENU_VERTICAL_COMPACT) {
        changeRectPx().right() = getRectPx().left() + aWidth;
        int anItemCount = 0;
        for(StGLWidget* aChild = getChildren()->getStart(); aChild != NULL; aChild = aChild->getNext(), ++anItemCount) {
            StGLMenuItem* anItem = (StGLMenuItem* )aChild;
            anItem->changeRectPx().moveTopTo(anItemCount * myItemHeight);
            anItem->changeRectPx().right() = anItem->getRectPx().left() + aWidth;
            anItem->setTextWidth(aWidth);
            if(anItem->getSubMenu() != NULL) {
                anItem->getSubMenu()->changeRectPx().moveTopLeftTo(getRectPxAbsolute().right() - myRoot->scale(10) - myRoot->getRectPx().left(),
                                                                   anItem->getRectPxAbsolute().top() - myRoot->getRectPx().top());
            }
        }
        changeRectPx().bottom() = getRectPx().top() + anItemCount * myItemHeight;
    }

    // already initialized?
    if(myVertexBuf.isValid()) {
        // synchronize menu items visibility
        setOpacity(myOpacity, true);
        return true;
    }

    stglResize();
    return myIsInitialized;
}

void StGLMenu::stglDraw(unsigned int theView) {
    if(!myIsInitialized || !isVisible()) {
        return;
    }

    if(myIsResized) {
        stglResize();
    }

    StGLContext& aCtx = getContext();
    aCtx.core20fwd->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    aCtx.core20fwd->glEnable(GL_BLEND);

    StGLMenuProgram& aProgram = myRoot->getMenuProgram();
    if(myVertexBndBuf.isValid()) {
        aProgram.use(aCtx, StGLVec4(0.0f, 0.0f, 0.0f, 1.0f), myOpacity, myRoot->getScreenDispX());
        myVertexBndBuf.bindVertexAttrib  (aCtx, aProgram.getVVertexLoc());
        aCtx.core20fwd->glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        myVertexBndBuf.unBindVertexAttrib(aCtx, aProgram.getVVertexLoc());
    }

    aProgram.use(aCtx, myColorVec, myOpacity, myRoot->getScreenDispX());

    myVertexBuf.bindVertexAttrib  (aCtx, aProgram.getVVertexLoc());
    aCtx.core20fwd->glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    myVertexBuf.unBindVertexAttrib(aCtx, aProgram.getVVertexLoc());

    aProgram.unuse(aCtx);
    aCtx.core20fwd->glDisable(GL_BLEND);

    StGLWidget::stglDraw(theView);
}

bool StGLMenu::doKeyDown(const StKeyEvent& theEvent) {
    switch(theEvent.VKey) {
        case ST_VK_ESCAPE: {
            destroyWithDelay(this);
            return true;
        }
        default:
            return false;
    }
}

bool StGLMenu::tryUnClick(const StClickEvent& theEvent,
                          bool&               theIsItemUnclicked) {
    myKeepActive = false;
    bool wasSomeClickedBefore = theIsItemUnclicked;
    bool isSelfClicked = StGLWidget::tryUnClick(theEvent, theIsItemUnclicked);
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

    if(myIsRootMenu && !isSelfItemClicked) {
        setActive(false); // deactivate root menu
    }

    if(myIsContextual) {
        myRoot->destroyWithDelay(this);
    }

    return isSelfClicked;
}

bool StGLMenu::doScroll(const StScrollEvent& theEvent) {
    StGLWidget::doScroll(theEvent);
    return true; // do not pass event further
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

StGLMenuItem* StGLMenu::addItem(const StHandle<StBoolParamNamed>& theTrackedValue) {
    StGLMenuItem* aNewItem = new StGLMenuCheckbox(this, theTrackedValue);
    aNewItem->setText(theTrackedValue->getName());
    return aNewItem;
}

StGLMenuItem* StGLMenu::addItem(const StString&               theLabel,
                                const StHandle<StInt32Param>& theTrackedValue,
                                const int32_t                 theOnValue) {
    StGLMenuItem* aNewItem = new StGLMenuRadioButton(this, theTrackedValue, theOnValue);
    aNewItem->setText(theLabel);
    return aNewItem;
}

StGLMenuItem* StGLMenu::addItem(const StHandle<StEnumParam>& theTrackedValue,
                                const int32_t                theOnValue) {
    StGLMenuItem* aNewItem = new StGLMenuRadioButton(this, theTrackedValue, theOnValue);
    aNewItem->setText(theTrackedValue->getOptionLabel(theOnValue));
    return aNewItem;
}

StGLMenuItem* StGLMenu::addItem(const StString&                 theLabel,
                                const StHandle<StFloat32Param>& theTrackedValue,
                                const float                     theOnValue) {
    StGLMenuItem* aNewItem = new StGLMenuRadioButton(this, theTrackedValue, theOnValue);
    aNewItem->setText(theLabel);
    return aNewItem;
}

/**
 * Simple menu item widget with bound action.
 */
class StGLMenuActionItem : public StGLMenuItem {

        public:

    ST_LOCAL StGLMenuActionItem(StGLMenu*                 theParent,
                                const StHandle<StAction>& theAction,
                                StGLMenu*                 theSubMenu)
    : StGLMenuItem(theParent, 0, 0, theSubMenu),
      myAction(theAction) {
        ST_ASSERT(!theAction.isNull(), "StGLMenuActionItem - Unexpected empty action makes no sense!");
        StGLMenuItem::signals.onItemClick.connect(this, &StGLMenuActionItem::doItemClick);
    }

    ST_LOCAL virtual ~StGLMenuActionItem() {}

        private:

    ST_LOCAL void doItemClick(const size_t ) {
        if(!myAction.isNull()) {
            myAction->doTrigger(NULL);
        }
    }

        private:

    StHandle<StAction> myAction;

};

StGLMenuItem* StGLMenu::addItem(const StString&           theLabel,
                                const StHandle<StAction>& theAction,
                                StGLMenu*                 theSubMenu) {
    StGLMenuItem* aNewItem = new StGLMenuActionItem(this, theAction, theSubMenu);
    aNewItem->setText(theLabel);
    return aNewItem;
}
