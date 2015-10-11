/**
 * StGLWidgets, small C++ toolkit for writing GUI using OpenGL.
 * Copyright © 2009-2014 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#include <StGLWidgets/StGLDescription.h>
#include <StGLWidgets/StGLRootWidget.h>

StGLDescription::StGLDescription(StGLWidget* theParent)
: StGLTextArea(theParent, 0, 0, StGLCorner(ST_VCORNER_TOP, ST_HCORNER_LEFT),
               theParent->getRoot()->scale(256),
               theParent->getRoot()->scale(96)) {
    myFormatter.setupAlignment(StGLTextFormatter::ST_ALIGN_X_CENTER,
                               StGLTextFormatter::ST_ALIGN_Y_TOP);
    setBorder(true);
}

StGLDescription::StGLDescription(StGLWidget* theParent,
                                 const int   theWidth)
: StGLTextArea(theParent, 0, 0, StGLCorner(ST_VCORNER_TOP, ST_HCORNER_LEFT),
               theWidth,
               theParent->getRoot()->scale(96)) {
    myFormatter.setupAlignment(StGLTextFormatter::ST_ALIGN_X_CENTER,
                               StGLTextFormatter::ST_ALIGN_Y_TOP);
    setBorder(true);
}

StGLDescription::~StGLDescription() {
    //
}

void StGLDescription::setPoint(const StPointD_t& thePointZo) {
    const StRectI_t& aRootRectPx = StGLWidget::getParent()->getRectPx();
    const int aRootSizeX = aRootRectPx.width();
    const int aRootSizeY = aRootRectPx.height();
    const int aSizeX = StGLWidget::getRectPx().width();
    const int aSizeY = StGLWidget::getRectPx().height();

    StRectI_t aRect;
    aRect.left()   = int(thePointZo.x() * (double )aRootSizeX) + 16;
    aRect.right()  = aRect.left() + aSizeX;
    aRect.top()    = int(thePointZo.y() * (double )aRootSizeY) + 16;
    aRect.bottom() = aRect.top()  + aSizeY;

    StGLVCorner aCornerY = ST_VCORNER_TOP;
    StGLHCorner aCornerX = ST_HCORNER_LEFT;
    if(aRect.right() > aRootRectPx.width()) {
        aRect.left()  -= aRootSizeX + 32;
        aRect.right() -= aRootSizeX + 32;
        aCornerX = ST_HCORNER_RIGHT;
    }
    if(aRect.bottom() > aRootRectPx.height()) {
        aRect.top()    -= aRootSizeY + 32;
        aRect.bottom() -= aRootSizeY + 32;
        myFormatter.setupAlignment(StGLTextFormatter::ST_ALIGN_X_CENTER,
                                   StGLTextFormatter::ST_ALIGN_Y_BOTTOM);
        aCornerY = ST_VCORNER_BOTTOM;
    } else {
        myFormatter.setupAlignment(StGLTextFormatter::ST_ALIGN_X_CENTER,
                                   StGLTextFormatter::ST_ALIGN_Y_TOP);
    }

    myCorner = StGLCorner(aCornerY, aCornerX);
    StGLWidget::changeRectPx() = aRect;
}
