/**
 * Copyright © 2013-2014 Kirill Gavrilov <kirill@sview.ru
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#include <StGLWidgets/StGLScrollArea.h>
#include <StGLWidgets/StGLRootWidget.h>

#include <StGL/StGLContext.h>
#include <StGLCore/StGLCore20.h>

StGLScrollArea::StGLScrollArea(StGLWidget*      theParent,
                               const int        theLeft,  const int theTop,
                               const StGLCorner theCorner,
                               const int        theWidth, const int theHeight)
: StGLWidget(theParent, theLeft, theTop, theCorner, theWidth, theHeight) {
    //
}

StGLScrollArea::~StGLScrollArea() {
    //
}

bool StGLScrollArea::stglInit() {
    if(!StGLWidget::stglInit()) {
        return false;
    }

    stglResize();

    // extend text area to fit whole text
    StGLTextArea* aText = dynamic_cast<StGLTextArea*> (myChildren.getStart());
    if(aText != NULL) {
        aText->changeRectPx().bottom() = aText->getRectPx().top() + aText->getTextHeight();
        if(aText->getRectPx().height() < getRectPx().height()) {
            aText->setCorner(StGLCorner(ST_VCORNER_CENTER, ST_HCORNER_LEFT));
        }
    }

    return true;
}

void StGLScrollArea::stglResize() {
    StGLWidget* aContent = myChildren.getStart();
    if(!isScrollable()
    && aContent != NULL
    && aContent->getRectPx().top() < 0
    && aContent->getCorner().v == ST_VCORNER_TOP) {
        const int aDelta = -aContent->getRectPx().top();
        aContent->changeRectPx().top()    += aDelta;
        aContent->changeRectPx().bottom() += aDelta;
    }

    StGLWidget::stglResize();
}

void StGLScrollArea::stglDraw(unsigned int theView) {
    if(!isVisible()) {
        return;
    }

    if(isClicked(ST_MOUSE_LEFT)
    && isScrollable()) {
        StPointD_t aDelta = getRoot()->getCursorZo() - myClickPntZo;
        int aDeltaY = int(aDelta.y() * getRoot()->getRectPx().height());
        myClickPntZo = getRoot()->getCursorZo();

        StGLWidget* aContent = myChildren.getStart();
        aContent->changeRectPx().top()    += aDeltaY;
        aContent->changeRectPx().bottom() += aDeltaY;
        isResized = true;
    }

    StGLContext& aCtx = getContext();
    if(isResized) {
        stglResize();
        isResized = false;
    }

    StGLBoxPx aScissorRect;
    stglScissorRect(aScissorRect);
    aCtx.stglSetScissorRect(aScissorRect, true);

    StGLWidget::stglDraw(theView); // draw children

    aCtx.stglResetScissorRect();
}

void StGLScrollArea::doScroll(const int theDir) {
    if(!isScrollable()) {
        return;
    }

    StGLWidget* aContent = myChildren.getStart();
    aContent->changeRectPx().top()    += 10 * theDir;
    aContent->changeRectPx().bottom() += 10 * theDir;
    stglResize();
}

bool StGLScrollArea::tryClick(const StPointD_t& theCursorZo,
                              const int&        theMouseBtn,
                              bool&             isItemClicked) {
    if(StGLWidget::tryClick(theCursorZo, theMouseBtn, isItemClicked)) {
        if(theMouseBtn == ST_MOUSE_LEFT) {
            myClickPntZo = theCursorZo;
        }
        isItemClicked = true;
        return true;
    }
    return false;
}

bool StGLScrollArea::tryUnClick(const StPointD_t& theCursorZo,
                                const int&        theMouseBtn,
                                bool&             isItemUnclicked) {
    if(isClicked(ST_MOUSE_LEFT)
    && theMouseBtn == ST_MOUSE_LEFT) {
        isItemUnclicked = true;
        setClicked(ST_MOUSE_LEFT, false);
        return true;
    }
    if(StGLWidget::tryUnClick(theCursorZo, theMouseBtn, isItemUnclicked)) {
        switch(theMouseBtn) {
            case ST_MOUSE_SCROLL_V_UP: {
                doScroll(1);
                break;
            }
            case ST_MOUSE_SCROLL_V_DOWN: {
                doScroll(-1);
                break;
            }
        }
        return true;
    }
    return false;
}
