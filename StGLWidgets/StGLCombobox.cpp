/**
 * Copyright © 2015 Kirill Gavrilov <kirill@sview.ru
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#include <StGLWidgets/StGLCombobox.h>
#include <StGLWidgets/StGLRootWidget.h>

StGLCombobox::StGLCombobox(StGLWidget* theParent,
                           const int   theLeft,
                           const int   theTop,
                           const StHandle<StEnumParam>& theParam)
: StGLButton(theParent, theLeft, theTop, theParam->getActiveValue()),
  myParam(theParam) {
    StGLButton::signals.onBtnClick += stSlot(this, &StGLCombobox::doShowList);
}

StGLCombobox::~StGLCombobox() {
    myParam->signals.onChanged -= stSlot(this, &StGLCombobox::doValueChanged);
}

void StGLCombobox::doShowList(const size_t ) {
    myParam->signals.onChanged += stSlot(this, &StGLCombobox::doValueChanged);

    StGLMenu* aMenu = new StGLMenu(myRoot, 0, 0, StGLMenu::MENU_VERTICAL);
    aMenu->setCorner(StGLCorner(ST_VCORNER_CENTER, ST_HCORNER_CENTER));
    aMenu->setContextual(true);
    const StArrayList<StString>& aValues = myParam->getValues();
    for(size_t aValIter = 0; aValIter < aValues.size(); ++aValIter) {
        aMenu->addItem(aValues[aValIter], myParam, int32_t(aValIter));
    }
    aMenu->setVisibility(true, true);
    aMenu->stglInit();
}

void StGLCombobox::doValueChanged(const int32_t ) {
    setLabel(myParam->getActiveValue());
}
