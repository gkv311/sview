/**
 * StGLWidgets, small C++ toolkit for writing GUI using OpenGL.
 * Copyright © 2011-2015 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#include <StGLWidgets/StGLCheckboxTextured.h>

StGLCheckboxTextured::StGLCheckboxTextured(StGLWidget* theParent,
                                           const StHandle<StBoolParam>& theTrackedValue,
                                           const StString& theTextureOffPath,
                                           const StString& theTextureOnPath,
                                           const int theLeft, const int theTop,
                                           const StGLCorner theCorner)
: StGLTextureButton(theParent, theLeft, theTop, theCorner, 2),
  myTrackValue(theTrackedValue),
  myFalseOpacity(0.5f),
  myTrueOpacity (1.0f) {
    StString aTextures[2] = { theTextureOffPath, theTextureOnPath };
    StGLTextureButton::setTexturePath(aTextures, 2);
    StGLTextureButton::signals.onBtnClick.connect(this, &StGLCheckboxTextured::doClick);
}

StGLCheckboxTextured::~StGLCheckboxTextured() {
    //
}

void StGLCheckboxTextured::stglUpdate(const StPointD_t& theCursorZo,
                                      bool theIsPreciseInput) {
    const bool isOn = myTrackValue->getValue();
    setFaceId(isOn ? 1 : 0);
    myOpacityScale = isOn ? myTrueOpacity : myFalseOpacity;
    StGLTextureButton::stglUpdate(theCursorZo, theIsPreciseInput);
}

void StGLCheckboxTextured::doClick(const size_t ) {
    myTrackValue->reverse();
}
