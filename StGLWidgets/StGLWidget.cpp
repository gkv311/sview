/**
 * StGLWidgets, small C++ toolkit for writing GUI using OpenGL.
 * Copyright © 2009-2015 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#include <StGLWidgets/StGLWidget.h>
#include <StGLWidgets/StGLRootWidget.h>

#include <StGL/StGLContext.h>
#include <StGLCore/StGLCore20.h>

#include <StCore/StEvent.h>
#include <StStrings/StLogger.h>
#include <StThreads/StMutex.h>

StGLWidget::StGLWidget(StGLWidget* theParent,
                       const int theLeft, const int theTop,
                       const StGLCorner theCorner,
                       const int theWidth, const int theHeight)
: myRoot(theParent != NULL ? theParent->myRoot : NULL),
  myParent(theParent),
  myChildren(),
  myPrev(NULL),
  myNext(NULL),
  userData(0),
  rectPx(theTop, theTop + theHeight, theLeft, theLeft + theWidth),
  myCorner(theCorner),
  myOpacity(1.0f),
  myIsResized(true),
  myHasFocus(false),
  myIsTopWidget(false) {
    if(myParent != NULL) {
        myParent->getChildren()->add(this);
    }
    stMemSet(mouseClicked, 0, sizeof(mouseClicked));
}

StGLWidget::~StGLWidget() {
    if(myRoot != NULL
    && myRoot->getFocus() == this) {
        myRoot->setFocus(NULL);
    }
    if(myParent != NULL) {
        // remove self from parent
        myParent->getChildren()->remove(this);
    }
    // remove own children
    destroyChildren();
}

void StGLWidget::destroyChildren() {
    // remove own children
    for(StGLWidget *child(myChildren.getStart()), *deleteChild(NULL); child != NULL;) {
        deleteChild = child;
        child = child->getNext();
        delete deleteChild;
    }
}

bool StGLWidget::isChild(StGLWidget* theWidget,
                         const bool  theIsRecursive) {
    for(StGLWidget* aChild = myChildren.getStart(); aChild != NULL; aChild = aChild->getNext()) {
        if(aChild == theWidget) {
            return true;
        } else if(theIsRecursive
               && aChild->isChild(theWidget, true)) {
            return true;
        }
    }
    return false;
}

void StGLWidget::setPrev(StGLWidget* thePrev) {
    if(myPrev == thePrev) {
        return;
    }
    if(myPrev != NULL) {
        // silently remove self
        myPrev->myNext = NULL;
    }
    myPrev = thePrev;
    if(thePrev != NULL) {
        // set self as next item
        thePrev->setNext(this);
    }
}

void StGLWidget::setNext(StGLWidget* theNext) {
    if(myNext == theNext) {
        return;
    }
    if(myNext != NULL) {
        // silently remove self
        myNext->myPrev = NULL;
    }
    myNext = theNext;
    if(theNext != NULL) {
        // set self as previous item
        theNext->setPrev(this);
    }
}

GLdouble StGLWidget::getScaleX() const {
    return myRoot->getRootScaleX();
}

GLdouble StGLWidget::getScaleY() const {
    return myRoot->getRootScaleY();
}

StRectD_t StGLWidget::getRectGl() const {
    return myRoot->getRectGl(getRectPxAbsolute());
}

/**
 * Helper function to convert relative coordinates to absolute.
 * @param theParentRect (const StRectI_t& ) - parent widget absolute position;
 * @param theChildRect  (const StRectI_t& ) - child widget position relative to parent;
 * @param theCorner     (const StGLCorner ) - corner modifier for child widget relative to parent;
 * @return child widget absolute position.
 */
inline StRectI_t computeAbsolutePos(const StRectI_t& theParentRect,
                                    const StRectI_t& theChildRect,
                                    const StGLCorner theCorner) {
    StRectI_t aRect;
    if(theCorner.h == ST_HCORNER_LEFT) {
        aRect.left() = theParentRect.left() + theChildRect.left();
    } else if(theCorner.h == ST_HCORNER_RIGHT) {
        aRect.left() = theParentRect.right() - theChildRect.width() + theChildRect.left();
    } else if(theCorner.h == ST_HCORNER_CENTER) {
        aRect.left() = theParentRect.left() + (theParentRect.width() - theChildRect.width()) / 2 + theChildRect.left();
    }
    aRect.right() = aRect.left() + theChildRect.width();

    if(theCorner.v == ST_VCORNER_TOP) {
        // general case
        aRect.top() = theParentRect.top() + theChildRect.top();
    } else if(theCorner.v == ST_VCORNER_BOTTOM) {
        // bottom
        aRect.top() = theParentRect.bottom() - theChildRect.height() + theChildRect.top();
    } else if(theCorner.v == ST_VCORNER_CENTER) {
        // vertical center
        aRect.top() = theParentRect.top() + (theParentRect.height() - theChildRect.height()) / 2 + theChildRect.top();
    }
    aRect.bottom() = aRect.top() + theChildRect.height();

    return aRect;
}

void StGLWidget::getRectGl(StArray<StGLVec2>& theVertices) const {
    myRoot->getRectGl(getRectPxAbsolute(), theVertices);
}

StRectI_t StGLWidget::getRectPxAbsolute() const {
    if(myParent == NULL) {
        return rectPx;
    }
    return computeAbsolutePos(myParent->getRectPxAbsolute(), rectPx, myCorner);
}

StRectI_t StGLWidget::getAbsolute(const StRectI_t& theRectPx) const {
    if(myParent == NULL) {
        return theRectPx;
    }
    return computeAbsolutePos(myParent->getRectPxAbsolute(), theRectPx, myCorner);
}

void StGLWidget::stglScissorRect(StGLBoxPx& theScissorRect) const {
    myRoot->stglScissorRect(getRectPxAbsolute(), theScissorRect);
}

StPointD_t StGLWidget::getPointGl(const StPointD_t& thePointZo) const {
    double oglWidth  = myRoot->getRootRectGl().right() - myRoot->getRootRectGl().left();
    double oglHeight = myRoot->getRootRectGl().top() - myRoot->getRootRectGl().bottom();
    return StPointD_t((thePointZo.x() - 0.5) * oglWidth,
                      (0.5 - thePointZo.y()) * oglHeight);
}

double StGLAnimationLerp::perform(bool theDirUp, bool theToForce) {
    if(theToForce) {
        myValue = theDirUp ? 1.0 : 0.0;
    }
    if(theDirUp && !myOnTimer.isOn()) {
        // switch from value DOWN to UP
        myOnTimer.restart(1000.0 * myValue * myOnMs);
        myOffTimer.stop();
    } else if(!theDirUp && !myOffTimer.isOn()) {
        // switch from value UP to DOWN
        myOffTimer.restart(1000.0 * (myOffMs - myValue * myOffMs));
        myOnTimer.stop();
    }
    if(!theDirUp && myValue > 0.0) {
        // value DOWN
        myValue = 1.0 - myOffTimer.getElapsedTimeInMilliSec() / myOffMs;
        if(myValue < 0.0) {
            myValue = 0.0;
        }
    } else if(theDirUp && myValue < 1.0) {
        // value UP
        myValue = myOnTimer.getElapsedTimeInMilliSec() / myOnMs;
        if(myValue > 1.0) {
            myValue = 1.0;
        }
    }
    return myValue;
}

void StGLWidget::stglUpdate(const StPointD_t& cursorZo) {
    if(!isVisible()) {
        return;
    }

    for(StGLWidget *child(myChildren.getStart()), *childActive(NULL); child != NULL;) {
        childActive = child;
        child = child->getNext();
        childActive->stglUpdate(cursorZo);
    }
}

void StGLWidget::stglResize() {
    for(StGLWidget *child(myChildren.getStart()), *childActive(NULL); child != NULL;) {
        childActive = child;
        child = child->getNext();
        childActive->stglResize();
    }
    myIsResized = false;
}

bool StGLWidget::stglInit() {
    bool success = true;
    for(StGLWidget *child(myChildren.getStart()), *childActive(NULL); child != NULL;) {
        childActive = child;
        child = child->getNext();
        success = childActive->stglInit() && success;
    }
    return success;
}

void StGLWidget::stglDraw(unsigned int view) {
    if(!isVisible()) {
        return;
    }
    for(StGLWidget *child(myChildren.getStart()), *childActive(NULL); child != NULL;) {
        childActive = child;
        child = child->getNext();
        childActive->stglDraw(view);
    }
}

bool StGLWidget::isClicked(const int& mouseBtn) const {
    if(mouseBtn > ST_MOUSE_MAX_ID) {
        // ignore out of range buttons
        return false;
    }
    return mouseClicked[mouseBtn];
}

void StGLWidget::setClicked(const int& mouseBtn, bool isClicked) {
    if(mouseBtn > ST_MOUSE_MAX_ID) {
        // ignore out of range buttons
        ST_DEBUG_LOG("StGLWidget, mouse button click #" + mouseBtn + " ignored!");
        return;
    }
    mouseClicked[mouseBtn] = isClicked;
}

bool StGLWidget::tryClick(const StClickEvent& theEvent,
                          bool&               theIsItemClicked) {
    if(!isVisible()) {
        return false;
    }

    for(StGLWidget *aChildIter(myChildren.getLast()), *aChildActive(NULL); aChildIter != NULL;) {
        aChildActive = aChildIter;
        aChildIter   = aChildIter->getPrev();
        aChildActive->tryClick(theEvent, theIsItemClicked);
    }
    if(!theIsItemClicked && isPointIn(StPointD_t(theEvent.PointX, theEvent.PointY))) {
        setClicked(theEvent.Button, true);
        theIsItemClicked = signals.onMouseClick(theEvent.Button);
        return true;
    }
    return false;
}

bool StGLWidget::tryUnClick(const StClickEvent& theEvent,
                            bool&               theIsItemUnclicked) {
    if(!isVisible()) {
        return false;
    }

    for(StGLWidget *aChildIter(myChildren.getLast()), *aChildActive(NULL); aChildIter != NULL;) {
        aChildActive = aChildIter;
        aChildIter   = aChildIter->getPrev();
        aChildActive->tryUnClick(theEvent, theIsItemUnclicked);
    }
    bool selfClicked = isClicked(theEvent.Button)
                    && isPointIn(StPointD_t(theEvent.PointX, theEvent.PointY));
    setClicked(theEvent.Button, false);
    if(!theIsItemUnclicked && selfClicked) {
        if(theEvent.Type != stEvent_MouseCancel) {
            theIsItemUnclicked = signals.onMouseUnclick(theEvent.Button);
        }
        return true;
    }
    return false;
}

bool StGLWidget::doKeyDown(const StKeyEvent& ) {
    return false;
}

bool StGLWidget::doKeyHold(const StKeyEvent& ) {
    return false;
}

bool StGLWidget::doKeyUp(const StKeyEvent& ) {
    return false;
}

bool StGLWidget::doScroll(const StScrollEvent& theEvent) {
    if(!isVisible()) {
        return false;
    }

    StPointD_t aPnt(theEvent.PointX, theEvent.PointY);
    for(StGLWidget *aChildIter(myChildren.getLast()), *aChildActive(NULL); aChildIter != NULL;) {
        aChildActive = aChildIter;
        aChildIter   = aChildIter->getPrev();
        if(aChildActive->isVisibleAndPointIn(aPnt)
        && aChildActive->doScroll(theEvent)) {
            return true;
        }
    }
    return false;
}

StGLProjCamera* StGLWidget::getCamera() {
    return myRoot->StGLRootWidget::getCamera();
}

StGLContext& StGLWidget::getContext() {
    return myRoot->StGLRootWidget::getContext();
}

void StGLWidget::destroyWithDelay(StGLWidget* theWidget) {
    myRoot->destroyWithDelay(theWidget);
}

StGLContainer::StGLContainer(StGLWidget* theParent,
                             const int theLeft,  const int theTop,
                             const StGLCorner theCorner,
                             const int theWidth, const int theHeight)
: StGLWidget(theParent, theLeft, theTop, theCorner, theWidth, theHeight) {}

StGLContainer::~StGLContainer() {}

bool StGLContainer::tryClick(const StClickEvent& theEvent,
                             bool&               theIsItemClicked) {
    if(!isVisible()) {
        return false;
    }
    for(StGLWidget *aChildIter(myChildren.getLast()), *aChildActive(NULL); aChildIter != NULL;) {
        aChildActive = aChildIter;
        aChildIter   = aChildIter->getPrev();
        aChildActive->tryClick(theEvent, theIsItemClicked);
    }
    return false;
}

bool StGLContainer::tryUnClick(const StClickEvent& theEvent,
                               bool&               theIsItemUnclicked) {
    if(!isVisible()) {
        return false;
    }
    for(StGLWidget *aChildIter(myChildren.getLast()), *aChildActive(NULL); aChildIter != NULL;) {
        aChildActive = aChildIter;
        aChildIter   = aChildIter->getPrev();
        aChildActive->tryUnClick(theEvent, theIsItemUnclicked);
    }
    return false;
}

void StGLWidget::setOpacity(const float theOpacity, bool theToSetChildren) {
    myOpacity = theOpacity;
    if(!theToSetChildren) {
        return;
    }
    for(StGLWidget *aChildIter(myChildren.getStart()); aChildIter != NULL; aChildIter = aChildIter->getNext()) {
        aChildIter->setOpacity(theOpacity, theToSetChildren);
    }
}
