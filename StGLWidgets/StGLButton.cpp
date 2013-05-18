/**
 * Copyright © 2013 Kirill Gavrilov <kirill@sview.ru
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#include <StGLWidgets/StGLButton.h>
#include <StGLWidgets/StGLMenuItem.h>

#include <StGL/StGLContext.h>
#include <StGLCore/StGLCore20.h>

StGLButton::StGLButton(StGLWidget*     theParent,
                       const int       theLeft,
                       const int       theTop,
                       const StString& theText)
: StGLMenu(theParent, theLeft, theTop, StGLMenu::MENU_ZERO) {
    StGLMenuItem* aBtn = addItem(theText);
    aBtn->signals.onItemClick.connect(this, &StGLButton::doItemClick);
    aBtn->setupAlignment(StGLTextFormatter::ST_ALIGN_X_CENTER,
                         StGLTextFormatter::ST_ALIGN_Y_CENTER);
}

StGLButton::~StGLButton() {
    //
}

void StGLButton::setWidth(const int theWidth) {
    myWidth = theWidth;
}

void StGLButton::setHeight(const int theHeight) {
    myItemHeight = theHeight;
}

void StGLButton::doItemClick(const size_t ) {
    signals.onBtnClick(getUserData());
}

bool StGLButton::stglInit() {
    const size_t aWidth = myWidth;
    if(!StGLMenu::stglInit()) {
        return false;
    }

    myWidth = aWidth;
    StGLMenuItem* anItem = (StGLMenuItem* )getChildren()->getStart();
    if(anItem == NULL) {
        return true;
    }

    if(myWidth == 0) {
        myWidth = anItem->computeTextWidth() + 16;
    }

    anItem->changeRectPx().left()   = 0;
    anItem->changeRectPx().right()  = myWidth;
    anItem->changeRectPx().bottom() = anItem->changeRectPx().top() + myItemHeight;
    anItem->setTextWidth(myWidth);
    changeRectPx().right()  = getRectPx().left() + myWidth;
    changeRectPx().bottom() = getRectPx().top()  + myItemHeight;
    return true;
}
