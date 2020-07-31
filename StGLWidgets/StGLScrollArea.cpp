/**
 * StGLWidgets, small C++ toolkit for writing GUI using OpenGL.
 * Copyright © 2013-2015 Kirill Gavrilov <kirill@sview.ru
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#include <StGLWidgets/StGLScrollArea.h>

#include <StGLWidgets/StGLMenuProgram.h>
#include <StGLWidgets/StGLRootWidget.h>
#include <StGL/StGLContext.h>
#include <StGLCore/StGLCore20.h>
#include <StCore/StEvent.h>

StGLScrollArea::StGLScrollArea(StGLWidget*      theParent,
                               const int        theLeft,  const int theTop,
                               const StGLCorner theCorner,
                               const int        theWidth, const int theHeight)
: StGLWidget(theParent, theLeft, theTop, theCorner, theWidth, theHeight),
  myBarColor(getRoot()->getColorForElement(StGLRootWidget::Color_ScrollBar)),
  myIsLeftClick(false),
  myHasDragged(false),
  myDragYDelta(0.0),
  myDragYCumul(0),
  myFlingAccel((double )myRoot->scale(200)),
  myFlingYSpeed(0.0),
  myFlingYDone(0),
  myScrollYAccum(0.0f) {
    //
}

StGLScrollArea::~StGLScrollArea() {
    myBarVertBuf.release(getContext());
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
    StGLContext& aCtx = getContext();
    if(!isScrollable()
    && aContent != NULL
    && aContent->getRectPx().top() < 0
    && aContent->getCorner().v == ST_VCORNER_TOP) {
        const int aDelta = -aContent->getRectPx().top();
        aContent->changeRectPx().top()    += aDelta;
        aContent->changeRectPx().bottom() += aDelta;
    }

    if(isScrollable()
    && aContent != NULL) {
        const int    aSizeY       = stMax(getRectPx().height(), 1);
        const int    aContSizeY   = aContent->getRectPx().height();
        const double aScaleY      = double(aSizeY) / double(aContSizeY);
        const int    aScrollSizeY = stMax(int(aScaleY * (double )aSizeY), myRoot->scale(4));
        const double aPosY        = double(-aContent->getRectPx().top()) / double(aContSizeY - aSizeY);

        StArray<StGLVec2> aVertices(4);
        StRectI_t aRectPx = getRectPxAbsolute();
        aRectPx.left()   =  aRectPx.right() - myRoot->scale(2);
        aRectPx.top()    += int(aPosY * double(aSizeY - aScrollSizeY));
        aRectPx.bottom() =  aRectPx.top() + aScrollSizeY;

        myRoot->getRectGl(aRectPx, aVertices);
        myBarVertBuf.init(aCtx, aVertices);
    } else {
        myBarVertBuf.release(aCtx);
    }

    StGLWidget::stglResize();
}

template<typename T1, typename T2>
inline bool haveSameSign(const T1 theVal1, const T2 theVal2) {
    return (theVal1 >= T1(0) && theVal2 > T2(0))
        || (theVal1 <= T1(0) && theVal2 < T2(0));
}

inline void setClickedWithChildren(StGLWidgetList& theList, const int theMouseBtn, bool isClicked) {
    for(StGLWidget* aChild = theList.getStart(); aChild != NULL; aChild = aChild->getNext()) {
        aChild->setClicked(theMouseBtn, isClicked);
        setClickedWithChildren(*aChild->getChildren(), theMouseBtn, isClicked);
    }
}

void StGLScrollArea::stglUpdate(const StPointD_t& theCursorZo,
                                bool theIsPreciseInput) {
    if(!isVisible()) {
        StGLWidget::stglUpdate(theCursorZo, theIsPreciseInput);
        return;
    }

    if(myIsLeftClick
    && isScrollable()) {
        StPointD_t aDelta = myRoot->getCursorZo() - myClickPntZo;
        double aDeltaY = aDelta.y() * myRoot->getRectPx().height();
        myClickPntZo = myRoot->getCursorZo();

        double aTime = myDragTimer.getElapsedTime();
        if(myDragTimer.isOn()) {
            if(aTime > 0.1) {
                myFlingYSpeed = 0.0;
                myDragYDelta  = 0.0;
                myDragTimer.restart();
            }
            else if(aTime > 0.0000001
                 && std::abs(aDeltaY) >= double(myRoot->scale(2))) {
                if(std::abs(myDragYDelta) < 0.001
                || haveSameSign(myDragYDelta, aDeltaY)) {
                    myFlingYSpeed = (myDragYDelta + aDeltaY) / aTime;
                }
                myDragYDelta = 0.0;
                myDragTimer.restart();
            } else {
                myDragYDelta += aDeltaY;
            }
        } else {
            myFlingYSpeed = 0.0;
            myDragYDelta  = 0.0;
            myDragTimer.restart();
        }
        doScroll((int )aDeltaY);
    } else if(myFlingTimer.isOn()) {
        double aTime = myFlingTimer.getElapsedTime();
        double anA   = (myFlingYSpeed > 0.0 ? -1.0 : 1.0) * myFlingAccel;
        int aFullDeltaY = int(myFlingYSpeed * aTime + anA * aTime * aTime);
        int aDeltaY     = aFullDeltaY - myFlingYDone;
        if(aDeltaY == 0) {
            // ignore zero update
        } else if(!haveSameSign(myFlingYSpeed, aDeltaY)) {
            myFlingTimer.stop();
        } else  {
            myFlingYDone += aDeltaY;
            doScroll(aDeltaY, true);
        }
    }

    StGLWidget::stglUpdate(theCursorZo, theIsPreciseInput);
}

void StGLScrollArea::stglDraw(unsigned int theView) {
    if(!isVisible()) {
        return;
    }

    StGLContext& aCtx = getContext();
    if(myIsResized) {
        stglResize();
    }

    StGLBoxPx aScissorRect;
    stglScissorRect2d(aScissorRect);
    aCtx.stglSetScissorRect(aScissorRect, true);

    StGLWidget::stglDraw(theView); // draw children

    aCtx.stglResetScissorRect();

    StGLMenuProgram& aProgram = myRoot->getMenuProgram();
    if(!aProgram.isValid()
    || !myBarVertBuf.isValid()) {
        return;
    }
    aCtx.core20fwd->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    aCtx.core20fwd->glEnable(GL_BLEND);
    aProgram.use(aCtx, myRoot->getScreenDispX());
    myBarVertBuf.bindVertexAttrib(aCtx, aProgram.getVVertexLoc());

    aProgram.setColor(aCtx, myBarColor, myOpacity);
    aCtx.core20fwd->glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    myBarVertBuf.unBindVertexAttrib(aCtx, aProgram.getVVertexLoc());
    aProgram.unuse(aCtx);
    aCtx.core20fwd->glDisable(GL_BLEND);
}

bool StGLScrollArea::doScroll(const int  theDelta,
                              const bool theIsFling) {
    if(!theIsFling) {
        myDragYDelta = 0.0;
        myFlingTimer.stop();
    }
    if(!isScrollable()) {
        return false;
    }

    StGLWidget* aContent = myChildren.getStart();
    const int aMinLim = 0;
    const int aMaxLim = getRectPx().height() - aContent->getRectPx().height();
    const int aTopOld = aContent->getRectPx().top();
    const int aTopNew = stMax(stMin(aMinLim, aTopOld + theDelta), aMaxLim);
    const int aDelta  = aTopNew - aTopOld;
    if(aDelta == 0) {
        if(theIsFling) {
            myFlingTimer.stop();
        }
        return false;
    }

    aContent->changeRectPx().top()    += aDelta;
    aContent->changeRectPx().bottom() += aDelta;

    if(myIsLeftClick) {
        if(!theIsFling) {
            myDragYCumul += aDelta;
            if(std::abs(myDragYCumul) > myRoot->getClickThreshold()
            && !myHasDragged) {
                setClickedWithChildren(myChildren, ST_MOUSE_LEFT, false);
                myHasDragged = true;
            }
        } else {
            myDragYCumul = 0;
        }
    }

    myIsResized = true;
    return true;
}

bool StGLScrollArea::tryClick(const StClickEvent& theEvent,
                              bool&               theIsItemClicked) {
    if(!isVisibleAndPointIn(StPointD_t(theEvent.PointX, theEvent.PointY))) {
        return false;
    }

    if( theEvent.Button == ST_MOUSE_LEFT
    && !theIsItemClicked
    &&  isScrollable()) {
        myIsLeftClick = true;
        myHasDragged  = false;
        myClickPntZo  = StPointD_t(theEvent.PointX, theEvent.PointY);
        myDragYCumul  = 0;

        // abort flinging
        if(myFlingTimer.isOn()) {
            const double aTime  = myFlingTimer.getElapsedTime();
            const double anA    = (myFlingYSpeed > 0.0 ? -1.0 : 1.0) * myFlingAccel;
            const double aSpeed = myFlingYSpeed + anA * aTime;
            if(std::abs(aSpeed) > myRoot->scale(300)) {
                setClickedWithChildren(myChildren, ST_MOUSE_LEFT, false);
                theIsItemClicked = true;
                return true;
            }
        }
    } else {
        myIsLeftClick = false;
        myHasDragged  = false;
    }

    if(StGLWidget::tryClick(theEvent, theIsItemClicked)) {
        theIsItemClicked = true;
        return true;
    }
    return false;
}

bool StGLScrollArea::tryUnClick(const StClickEvent& theEvent,
                                bool&               theIsItemUnclicked) {
    bool toCancel = false;
    if(myIsLeftClick
    && theEvent.Button == ST_MOUSE_LEFT) {
        if(myHasDragged) {
            toCancel = true;
        }
        myIsLeftClick = false;
        myHasDragged  = false;
        if(myDragTimer.isOn()) {
            myDragYDelta = 0.0;
            myDragYCumul = 0;
            myDragTimer.stop();
            myFlingYDone = 0;
            if(std::abs(myFlingYSpeed) > 0.00001) {
                myFlingTimer.restart();
            } else {
                myFlingTimer.stop();
            }
        }
    }
    if(toCancel) {
        StClickEvent anEvent = theEvent;
        anEvent.Type = stEvent_MouseCancel;
        return StGLWidget::tryUnClick(anEvent, theIsItemUnclicked);
    }
    return StGLWidget::tryUnClick(theEvent, theIsItemUnclicked);
}

bool StGLScrollArea::doScroll(const StScrollEvent& theEvent) {
    if(StGLWidget::doScroll(theEvent)) {
        return true;
    }

    myScrollYAccum += theEvent.DeltaY * 20.0f;
    const int aDeltaY = (int )myScrollYAccum;
    if(aDeltaY != 0) {
        myScrollYAccum -= float(aDeltaY);
        const int aDeltaScaled = myRoot->scale(std::abs(aDeltaY));
        doScroll(aDeltaY > 0 ? aDeltaScaled : -aDeltaScaled);
    }
    return true;
}
