/**
 * Copyright © 2013 Kirill Gavrilov <kirill@sview.ru
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
    StGLWidget::signals.onMouseUnclick = stSlot(this, &StGLScrollArea::doMouseUnclick);
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
    //
}

void StGLScrollArea::stglResize(const StRectI_t& theWinRectPx) {
    stglResize();
    StGLWidget::stglResize(theWinRectPx);
}

void StGLScrollArea::stglDraw(unsigned int theView) {
    if(!isVisible()) {
        return;
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
    StGLWidget* aContent = myChildren.getStart();
    if(aContent == NULL
    || aContent->getRectPx().height() <= getRectPx().height()) {
        return;
    }

    aContent->changeRectPx().top()    += 10 * theDir;
    aContent->changeRectPx().bottom() += 10 * theDir;
    stglResize(myRoot->getRectPx());
}

void StGLScrollArea::doMouseUnclick(const int theBtnId) {
    switch(theBtnId) {
        case ST_MOUSE_SCROLL_V_UP: {
            doScroll(1);
            break;
        }
        case ST_MOUSE_SCROLL_V_DOWN: {
            doScroll(-1);
            break;
        }
    }
}
