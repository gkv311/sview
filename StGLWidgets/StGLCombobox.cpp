/**
 * StGLWidgets, small C++ toolkit for writing GUI using OpenGL.
 * Copyright © 2015-2020 Kirill Gavrilov <kirill@sview.ru
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#include <StGLWidgets/StGLCombobox.h>

#include <StGLWidgets/StGLMenuItem.h>
#include <StGLWidgets/StGLMessageBox.h>
#include <StGLWidgets/StGLScrollArea.h>
#include <StGLWidgets/StGLRootWidget.h>

/**
 * Auxiliary widget covering entire screen.
 */
class StGLContextBackground : public StGLMessageBox {

        public:

    StGLContextBackground(StGLRootWidget* theParent)
    : StGLMessageBox(theParent) {
        myIsContextual = true;
        const int aWidth  = myRoot->getRootFullSizeX();
        const int aHeight = myRoot->getRootFullSizeY();
        changeRectPx().right()  = aWidth;
        changeRectPx().bottom() = aHeight;
        create(StString(), StString(), aWidth, aHeight, false);
    }

};

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
        myBack = new StGLContextBackground(aRoot);
        aMenuParent = myBack->getContent();
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
    StGLRootWidget* aRoot = myMenu->getRoot();
    const int aRootX = aRoot->getRectPx().width();
    const int aRootY = aRoot->getRectPx().height();
    if(myBack != NULL) {
        myBack->stglInit();
        if(myBack->getContent()->isScrollable()) {
            myMenu->setCorner(StGLCorner(ST_VCORNER_TOP, ST_HCORNER_CENTER));
            myMenu->stglResize();
        }
    } else {
        myMenu->stglUpdateSubmenuLayout();
        StRectI_t aRect = myMenu->getRectPxAbsolute();
        if(aRect.width()  >= aRootX) {
            myMenu->changeRectPx().moveLeftTo(0);
        } else if(aRect.right() > aRoot->getRectPx().right()) {
            myMenu->changeRectPx().moveRightTo(aRootX);
        }
        if(aRect.height() >= aRootY) {
            myMenu->changeRectPx().moveTopTo(0);
        } else if(aRect.bottom() > aRoot->getRectPx().bottom()) {
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
            } else if(aSubRect.right() > aRoot->getRectPx().right()) {
                aSubRectNew.moveRightTo(myMenu->getRectPx().left() + aRoot->scale(10));
            }
        }
        aRoot->setFocus(myMenu); // take input focus
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
