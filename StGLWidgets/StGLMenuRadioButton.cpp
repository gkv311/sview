/**
 * StGLWidgets, small C++ toolkit for writing GUI using OpenGL.
 * Copyright © 2011-2015 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#include <StGLWidgets/StGLMenuRadioButton.h>

#include <StGLWidgets/StGLRadioButton.h>
#include <StGLWidgets/StGLRadioButtonFloat32.h>
#include <StGLWidgets/StGLMenuProgram.h>
#include <StGLWidgets/StGLRootWidget.h>

StGLMenuRadioButton::StGLMenuRadioButton(StGLMenu* theParent,
                                         const StHandle<StInt32Param>& theTrackedValue,
                                         const int32_t theOnValue)
: StGLMenuItem(theParent, 0, 0, NULL),
  myRadio(NULL) {
    myRadio = new StGLRadioButton(this, theTrackedValue, theOnValue,
                                  myRoot->scale(8), 0, StGLCorner(ST_VCORNER_CENTER, ST_HCORNER_LEFT));
    myRadio->setColor(getRoot()->getColorForElement(StGLRootWidget::Color_MenuIcon));
    StGLMenuItem::signals.onItemClick.connect(this, &StGLMenuRadioButton::doItemClick);
}

StGLMenuRadioButton::StGLMenuRadioButton(StGLMenu* theParent,
                                         const StHandle<StFloat32Param>& theTrackedValue,
                                         const float theOnValue)
: StGLMenuItem(theParent, 0, 0, NULL),
  myRadio(NULL) {
    myRadio = new StGLRadioButtonFloat32(this, theTrackedValue, theOnValue,
                                         myRoot->scale(8), 0, StGLCorner(ST_VCORNER_CENTER, ST_HCORNER_LEFT));
    StGLMenuItem::signals.onItemClick.connect(this, &StGLMenuRadioButton::doItemClick);
}

void StGLMenuRadioButton::doItemClick(const size_t ) {
    myRadio->setValue();
}
