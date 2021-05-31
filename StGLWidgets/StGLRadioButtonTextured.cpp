/**
 * StGLWidgets, small C++ toolkit for writing GUI using OpenGL.
 * Copyright © 2011-2014 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#include <StGLWidgets/StGLRadioButtonTextured.h>

StGLRadioButtonTextured::StGLRadioButtonTextured(StGLWidget* theParent,
                                                 const StHandle<StInt32Param>& theTrackedValue, const int32_t theValueOn,
                                                 const StString& theTexturePath,
                                                 const int theLeft, const int theTop,
                                                 const StGLCorner theCorner)
: StGLTextureButton(theParent, theLeft, theTop, theCorner, 1),
  myTrackValue(theTrackedValue),
  myValueOn(theValueOn) {
    //
    StGLTextureButton::setTexturePath(&theTexturePath, 1);
    StGLTextureButton::signals.onBtnClick.connect(this, &StGLRadioButtonTextured::doClick);
}

StGLRadioButtonTextured::~StGLRadioButtonTextured() {
    //
}

void StGLRadioButtonTextured::doClick(const size_t ) {
    myTrackValue->setValue(myValueOn);
}
