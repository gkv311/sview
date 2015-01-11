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

StGLCombobox::ListBuilder::ListBuilder(StGLWidget* theParent)
: myBack(NULL),
  myMenu(NULL) {
    StGLWidget* aMenuParent = theParent->getRoot();
    if(theParent->getRoot()->isMobile()) {
        myBack = new StGLMessageBox(theParent->getRoot(), "", "",
                                   theParent->getRoot()->getRectPx().width(), theParent->getRoot()->getRectPx().height());
        myBack->setContextual(true);
        aMenuParent = myBack;
    }

    const StRectI_t aRect = theParent->getRectPxAbsolute();
    int aLeft = myBack != NULL ? 0 : aRect.left();
    int aTop  = myBack != NULL ? 0 : aRect.bottom();

    myMenu = new StGLMenu(aMenuParent, aLeft, aTop, StGLMenu::MENU_VERTICAL);
    if(myBack != NULL) {
        myMenu->setCorner(StGLCorner(ST_VCORNER_CENTER, ST_HCORNER_CENTER));
    }
    myMenu->setContextual(myBack == NULL);
}

void StGLCombobox::ListBuilder::display() {
    if(myBack != NULL) {
        myBack->setVisibility(true, true);
        myBack->stglInit();
    } else {
        myMenu->setVisibility(true, true);
        myMenu->stglInit();
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
