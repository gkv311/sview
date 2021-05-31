/**
 * StGLWidgets, small C++ toolkit for writing GUI using OpenGL.
 * Copyright © 2009-2014 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#include <StGLWidgets/StGLDescription.h>
#include <StGLWidgets/StGLRootWidget.h>

StGLDescription::StGLDescription(StGLRootWidget* theParent)
: StGLTextArea(theParent, 0, 0, StGLCorner(ST_VCORNER_TOP, ST_HCORNER_LEFT),
               theParent->getRoot()->scale(256),
               theParent->getRoot()->scale(96)) {
    myFormatter.setupAlignment(StGLTextFormatter::ST_ALIGN_X_CENTER,
                               StGLTextFormatter::ST_ALIGN_Y_TOP);
    setBorder(true);
}

StGLDescription::StGLDescription(StGLRootWidget* theParent,
                                 const int theWidth)
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
    const StRectI_t& aRootRectPx = myRoot->getRectPx();
    const int aSizeX = StGLWidget::getRectPx().width();
    const int aSizeY = StGLWidget::getRectPx().height();

    const int aPntInRootX = int(thePointZo.x() * (double )myRoot->getRectPx().width());
    const int aPntInRootY = int(thePointZo.y() * (double )myRoot->getRectPx().height());

    StRectI_t aRect;
    aRect.left()   = aPntInRootX + 16;
    aRect.right()  = aRect.left() + aSizeX;
    aRect.top()    = aPntInRootY + 16;
    aRect.bottom() = aRect.top()  + aSizeY;

    StGLVCorner aCornerY = ST_VCORNER_TOP;
    StGLHCorner aCornerX = ST_HCORNER_LEFT;
    if(aRect.right() > aRootRectPx.width()) {
        aRect.left()  -= aRootRectPx.width() + 32;
        aRect.right() -= aRootRectPx.width() + 32;
        aCornerX = ST_HCORNER_RIGHT;
    }
    if(aRect.bottom() > aRootRectPx.height()) {
        aRect.top()    -= aRootRectPx.height() + 32;
        aRect.bottom() -= aRootRectPx.height() + 32;
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
