/**
 * Copyright © 2011-2015 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#include <StGLWidgets/StGLMenuCheckbox.h>

#include <StGLWidgets/StGLCheckbox.h>
#include <StGLWidgets/StGLMenuProgram.h>
#include <StGLWidgets/StGLRootWidget.h>

StGLMenuCheckbox::StGLMenuCheckbox(StGLMenu* theParent,
                                   const StHandle<StBoolParam>& theTrackedValue)
: StGLMenuItem(theParent, 0, 0, NULL),
  myCheckbox(NULL) {
    myCheckbox = new StGLCheckbox(this, theTrackedValue,
                                  myRoot->scale(8), 0, StGLCorner(ST_VCORNER_CENTER, ST_HCORNER_LEFT));
    myCheckbox->setColor(getRoot()->getColorForElement(StGLRootWidget::Color_MenuIcon));
    StGLMenuItem::signals.onItemClick.connect(this, &StGLMenuCheckbox::doItemClick);
}

void StGLMenuCheckbox::doItemClick(const size_t ) {
    myCheckbox->reverseValue();
}
