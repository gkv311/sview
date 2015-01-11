/**
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
    }

    StGLButton::signals.onBtnClick += stSlot(this, &StGLCombobox::doShowList);
}

StGLCombobox::~StGLCombobox() {
    myParam->signals.onChanged -= stSlot(this, &StGLCombobox::doValueChanged);
}

void StGLCombobox::doShowList(const size_t ) {
    myParam->signals.onChanged += stSlot(this, &StGLCombobox::doValueChanged);

    StGLMessageBox* aBack   = NULL;
    StGLWidget*     aParent = myRoot;
    if(myRoot->isMobile()) {
        aBack = new StGLMessageBox(myRoot, "", "",
                                   myRoot->getRectPx().width(), myRoot->getRectPx().height());
        aBack->setContextual(true);
        aParent = aBack;
    }

    const StRectI_t aRect = getRectPxAbsolute();
    int aLeft = aBack != NULL ? 0 : aRect.left();
    int aTop  = aBack != NULL ? 0 : aRect.bottom();

    StGLMenu* aMenu = new StGLMenu(aParent, aLeft, aTop, StGLMenu::MENU_VERTICAL);
    if(aBack != NULL) {
        aMenu->setCorner(StGLCorner(ST_VCORNER_CENTER, ST_HCORNER_CENTER));
    }
    aMenu->setContextual(aBack == NULL);
    const StArrayList<StString>& aValues = myParam->getValues();
    for(size_t aValIter = 0; aValIter < aValues.size(); ++aValIter) {
        aMenu->addItem(aValues[aValIter], myParam, int32_t(aValIter));
    }

    if(aBack != NULL) {
        aBack->setVisibility(true, true);
        aBack->stglInit();
    } else {
        aMenu->setVisibility(true, true);
        aMenu->stglInit();
    }
}

void StGLCombobox::doValueChanged(const int32_t ) {
    setLabel(myParam->getActiveValue());
}
