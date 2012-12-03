/**
 * Copyright © 2009-2010 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#ifndef __StGLDescription_h_
#define __StGLDescription_h_

#include <StGLWidgets/StGLTextArea.h>

class ST_LOCAL StGLDescription : public StGLTextArea {

        public:

    StGLDescription(StGLWidget* parent, const int& width = 256)
    : StGLTextArea(parent, 0, 0, StGLCorner(ST_VCORNER_TOP, ST_HCORNER_LEFT), width, 96) {
        myFormatter.setupAlignment(StGLTextFormatter::ST_ALIGN_X_CENTER,
                                   StGLTextFormatter::ST_ALIGN_Y_TOP);
        setBorder(true);
    }

    virtual ~StGLDescription() {
        //
    }

    virtual const StString& getClassName() {
        static const StString className("StGLDescription");
        return className;
    }

    /**
     * Make sure you put point in RootWidget!
     * Mouse cursor putted here in most cases, and RootWidget must be window rectangle in this case.
     * @param pointZo (const StPointD_t& ) - point in RootWidget;
     */
    void setPoint(const StPointD_t& pointZo) {
        const StRectI_t& aRootRectPx = StGLWidget::getParent()->getRectPx();
        const int aRootSizeX = aRootRectPx.width();
        const int aRootSizeY = aRootRectPx.height();
        const int aSizeX = StGLWidget::getRectPx().width();
        const int aSizeY = StGLWidget::getRectPx().height();

        StRectI_t aRect;
        aRect.left()   = int(pointZo.x() * (double )aRootSizeX) + 16;
        aRect.right()  = aRect.left() + aSizeX;
        aRect.top()    = int(pointZo.y() * (double )aRootSizeY) + 16;
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

};

#endif //__StGLDescription_h_
