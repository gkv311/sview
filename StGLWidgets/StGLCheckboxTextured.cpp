/**
 * Copyright © 2011-2014 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#include <StGLWidgets/StGLCheckboxTextured.h>

StGLCheckboxTextured::StGLCheckboxTextured(StGLWidget* theParent,
                                           const StHandle<StBoolParam>& theTrackedValue,
                                           const StString& theTextureOffPath,
                                           const StString& theTextureOnPath,
                                           const int theLeft, const int theTop,
                                           const StGLCorner theCorner)
: StGLTextureButton(theParent, theLeft, theTop, theCorner, 2),
  myTrackValue(theTrackedValue) {
    //
    StString aTextures[2] = { theTextureOffPath, theTextureOnPath };
    StGLTextureButton::setTexturePath(aTextures, 2);
    StGLTextureButton::signals.onBtnClick.connect(this, &StGLCheckboxTextured::doClick);
}

StGLCheckboxTextured::~StGLCheckboxTextured() {
    //
}

void StGLCheckboxTextured::stglUpdate(const StPointD_t& theCursorZo) {
    setFaceId(myTrackValue->getValue() ? 1 : 0);
    StGLTextureButton::stglUpdate(theCursorZo);
}

void StGLCheckboxTextured::doClick(const size_t ) {
    myTrackValue->reverse();
}
