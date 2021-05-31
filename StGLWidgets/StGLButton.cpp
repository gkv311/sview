/**
 * StGLWidgets, small C++ toolkit for writing GUI using OpenGL.
 * Copyright © 2013-2015 Kirill Gavrilov <kirill@sview.ru
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#include <StGLWidgets/StGLButton.h>
#include <StGLWidgets/StGLMenuItem.h>
#include <StGLWidgets/StGLRootWidget.h>

#include <StGL/StGLContext.h>
#include <StGLCore/StGLCore20.h>

StGLButton::StGLButton(StGLWidget*     theParent,
                       const int       theLeft,
                       const int       theTop,
                       const StString& theText)
: StGLMenu(theParent, theLeft, theTop, StGLMenu::MENU_ZERO) {
    setShowBounds(true);

    StGLMenuItem* aBtn = addItem(theText);
    aBtn->signals.onItemClick.connect(this, &StGLButton::doItemClick);
    aBtn->setupAlignment(StGLTextFormatter::ST_ALIGN_X_CENTER,
                         StGLTextFormatter::ST_ALIGN_Y_CENTER);
    aBtn->changeMargins().left  = myRoot->scale(8);
    aBtn->changeMargins().right = myRoot->scale(8);

    myWidth = aBtn->computeTextWidth() + myRoot->scale(8) + aBtn->getMargins().left + aBtn->getMargins().right;
}

StGLButton::~StGLButton() {
    //
}

void StGLButton::setLabel(const StString& theLabel) {
    StGLMenuItem* anItem = getMenuItem();
    if(anItem != NULL) {
        anItem->setText(theLabel);
    }
}

StGLMenuItem* StGLButton::addItem(const StString& theLabel) {
    StGLMenuItem* aNewItem = new StGLPassiveMenuItem(this);
    aNewItem->setText(theLabel);
    return aNewItem;
}

void StGLButton::setFocus(const bool theValue) {
    StGLMenuItem* anItem = getMenuItem();
    if(anItem != NULL) {
        anItem->setFocus(theValue);
    }
}

int StGLButton::computeWidth(const StString& theText) {
    StGLMenuItem* anItem = getMenuItem();
    if(anItem == NULL) {
        return 0;
    }

    return anItem->computeTextWidth(theText) + myRoot->scale(8) + anItem->getMargins().left + anItem->getMargins().right;
}

int StGLButton::getWidth() const {
    return myWidth;
}

void StGLButton::setWidth(const int theWidth) {
    myWidth = theWidth;
    changeRectPx().right() = changeRectPx().left() + theWidth;
}

void StGLButton::setHeight(const int theHeight) {
    myItemHeight = theHeight;
}

void StGLButton::doItemClick(const size_t ) {
    signals.onBtnClick(getUserData());
}

bool StGLButton::stglInit() {
    const int aWidth = myWidth;
    if(!StGLMenu::stglInit()) {
        return false;
    }

    myWidth = aWidth;
    StGLMenuItem* anItem = getMenuItem();
    if(anItem == NULL) {
        return true;
    }

    anItem->changeRectPx().left()   = 0;
    anItem->changeRectPx().right()  = myWidth;
    anItem->changeRectPx().bottom() = anItem->changeRectPx().top() + myItemHeight;
    anItem->setTextWidth(myWidth - anItem->getMargins().left - anItem->getMargins().right);
    changeRectPx().right()  = getRectPx().left() + myWidth;
    changeRectPx().bottom() = getRectPx().top()  + myItemHeight;
    return true;
}

bool StGLButton::doScroll(const StScrollEvent& ) {
    return false; // make transparent to scroll events
}
