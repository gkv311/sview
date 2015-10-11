/**
 * StGLWidgets, small C++ toolkit for writing GUI using OpenGL.
 * Copyright © 2015 Kirill Gavrilov <kirill@sview.ru
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#include <StGLWidgets/StGLCombobox.h>

#include <StGLWidgets/StGLMenuItem.h>
#include <StGLWidgets/StGLMessageBox.h>
#include <StGLWidgets/StGLRootWidget.h>

StGLCombobox::StGLCombobox(StGLWidget* theParent,
                           const int   theLeft,
                           const int   theTop,
                           const StHandle<StEnumParam>& theParam)
: StGLButton(theParent, theLeft, theTop, theParam->getActiveValue()),
  myParam(theParam) {
    StGLMenuItem* anItem = getMenuItem();
    if(anItem != NULL) {
        anItem->setArrowIcon(StGLMenuItem::Arrow_Bottom);
        anItem->setupAlignment(StGLTextFormatter::ST_ALIGN_X_LEFT,
                               StGLTextFormatter::ST_ALIGN_Y_CENTER);
    }

    StGLButton::signals.onBtnClick += stSlot(this, &StGLCombobox::doShowList);
}

StGLCombobox::~StGLCombobox() {
    myParam->signals.onChanged -= stSlot(this, &StGLCombobox::doValueChanged);
}

StGLCombobox::ListBuilder::ListBuilder(StGLWidget* theParent)
: myBack(NULL),
  myMenu(NULL) {
    StGLRootWidget* aRoot       = theParent->getRoot();
    StGLWidget*     aMenuParent = aRoot;
    if(aRoot->isMobile()) {
        myBack = new StGLMessageBox(aRoot, "", "",
                                    aRoot->getRectPx().width(), aRoot->getRectPx().height());
        myBack->setContextual(true);
        aMenuParent = myBack;
    }

    int aLeft = 0;
    int aTop  = 0;
    if(myBack == NULL) {
        aLeft = int(aRoot->getCursorZo().x() * aRoot->getRectPx().width());
        aTop  = int(aRoot->getCursorZo().y() * aRoot->getRectPx().height());
    }

    myMenu = new StGLMenu(aMenuParent, aLeft, aTop, StGLMenu::MENU_VERTICAL, false);
    myMenu->setOpacity(1.0f, false);
    if(myBack != NULL) {
        myMenu->setCorner(StGLCorner(ST_VCORNER_CENTER, ST_HCORNER_CENTER));
    }
    myMenu->setContextual(myBack == NULL);
}

void StGLCombobox::ListBuilder::display() {
    if(myBack != NULL) {
        myBack->stglInit();
    } else {
        StGLRootWidget* aRoot = myMenu->getRoot();
        myMenu->stglUpdateSubmenuLayout();
        StRectI_t aRect  = myMenu->getRectPxAbsolute();
        const int aRootX = aRoot->getRectPx().width();
        const int aRootY = aRoot->getRectPx().height();
        if(aRect.width()  >= aRootX) {
            myMenu->changeRectPx().moveLeftTo(0);
        } else if(aRect.right() > aRootX) {
            myMenu->changeRectPx().moveRightTo(aRootX);
        }
        if(aRect.height() >= aRootY) {
            myMenu->changeRectPx().moveTopTo(0);
        } else if(aRect.bottom() > aRootY) {
            myMenu->changeRectPx().moveBottomTo(aRootY);
        }

        for(StGLWidget* aChild = myMenu->getChildren()->getStart(); aChild != NULL; aChild = aChild->getNext()) {
            StGLMenuItem* anItem = (StGLMenuItem* )aChild;
            if(anItem->getSubMenu() == NULL) {
                continue;
            }

            const StRectI_t aSubRect = anItem->getSubMenu()->getRectPxAbsolute();
            StRectI_t& aSubRectNew = anItem->getSubMenu()->changeRectPx();
            if(aSubRect.width() >= aRootX) {
                aSubRectNew.moveLeftTo(0);
            } else if(aSubRect.right() > aRootX) {
                aSubRectNew.moveRightTo(myMenu->getRectPxAbsolute().left() + aRoot->scale(10));
            }
        }
    }
}

void StGLCombobox::doShowList(const size_t ) {
    myParam->signals.onChanged += stSlot(this, &StGLCombobox::doValueChanged);

    ListBuilder aBuilder(this);
    const StArrayList<StString>& aValues = myParam->getValues();
    for(size_t aValIter = 0; aValIter < aValues.size(); ++aValIter) {
        aBuilder.getMenu()->addItem(aValues[aValIter], myParam, int32_t(aValIter));
    }
    aBuilder.display();
}

void StGLCombobox::doValueChanged(const int32_t ) {
    setLabel(myParam->getActiveValue());
}
