/**
 * Copyright © 2011 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#include <StGLWidgets/StGLMenuRadioButton.h>

#include <StGLWidgets/StGLRadioButton.h>
#include <StGLWidgets/StGLRadioButtonFloat32.h>
#include <StGLWidgets/StGLMenuProgram.h>

namespace {
    static const StString CLASS_NAME("StGLMenuRadioButton");
};

const StString& StGLMenuRadioButton::getClassName() {
    return CLASS_NAME;
}

StGLMenuRadioButton::StGLMenuRadioButton(StGLMenu* theParent,
                                         const StHandle<StInt32Param>& theTrackedValue,
                                         const int32_t theOnValue)
: StGLMenuItem(theParent, 0, 0, NULL),
  myRadio(NULL) {
    //
    myRadio = new StGLRadioButton(this, theTrackedValue, theOnValue,
                                  8, 0, StGLCorner(ST_VCORNER_CENTER, ST_HCORNER_LEFT));
    StGLMenuItem::signals.onItemClick.connect(this, &StGLMenuRadioButton::doItemClick);
}

StGLMenuRadioButton::StGLMenuRadioButton(StGLMenu* theParent,
                                         const StHandle<StFloat32Param>& theTrackedValue,
                                         const float theOnValue)
: StGLMenuItem(theParent, 0, 0, NULL),
  myRadio(NULL) {
    //
    myRadio = new StGLRadioButtonFloat32(this, theTrackedValue, theOnValue,
                                         8, 0, StGLCorner(ST_VCORNER_CENTER, ST_HCORNER_LEFT));
    StGLMenuItem::signals.onItemClick.connect(this, &StGLMenuRadioButton::doItemClick);
}

void StGLMenuRadioButton::setVisibility(bool isVisible, bool isForce) {
    StGLWidget::setVisibility(isVisible, isForce);
    myRadio->setVisibility   (isVisible, isForce);
}

void StGLMenuRadioButton::doItemClick(const size_t ) {
    myRadio->setValue();
}
