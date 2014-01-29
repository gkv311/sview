/**
 * Copyright © 2009-2013 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#include <StGLWidgets/StGLWidget.h>
#include <StGLWidgets/StGLRootWidget.h>

#include <StGL/StGLContext.h>
#include <StGLCore/StGLCore20.h>

#include <StStrings/StLogger.h>
#include <StThreads/StMutex.h>

namespace {
    static const StString CLASS_NAME("StGLWidget");
};

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
  opacityValue(0.0),
  opacityOnMs(2500.0),
  opacityOffMs(5000.0),
  opacityOnTimer(false),
  opacityOffTimer(true),
  isResized(true),
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

const StString& StGLWidget::getClassName() {
    return CLASS_NAME;
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

bool StGLWidget::isPointIn(const StPointD_t& thePointZo) const {
    const StRectD_t aRectGl = getRectGl();
    StPointD_t aPointGl = getPointGl(thePointZo);
    return aPointGl.x() > aRectGl.left()
        && aPointGl.x() < aRectGl.right()
        && aPointGl.y() > aRectGl.bottom()
        && aPointGl.y() < aRectGl.top();
}

StPointD_t StGLWidget::getPointIn(const StPointD_t& thePointZo) const {
    const StRectD_t aRectGl = getRectGl();
    StPointD_t aPointGl = getPointGl(thePointZo);
    return StPointD_t((aPointGl.x() - aRectGl.left()) / (aRectGl.right() - aRectGl.left()),
                      (aRectGl.top() - aPointGl.y())  / (aRectGl.top() - aRectGl.bottom()));
}

void StGLWidget::setVisibility(bool isVisible, bool isForce) {
    if(isForce) {
        opacityValue = isVisible ? 1.0 : 0.0;
    }
    if(isVisible && !opacityOnTimer.isOn()) {
        // switch from opacity DOWN to UP
        opacityOnTimer.restart(1000.0 * opacityValue * opacityOnMs);
        opacityOffTimer.stop();
    } else if(!isVisible && !opacityOffTimer.isOn()) {
        // switch from opacity UP to DOWN
        opacityOffTimer.restart(1000.0 * (opacityOffMs - opacityValue * opacityOffMs));
        opacityOnTimer.stop();
    }
    if(!isVisible && opacityValue > 0.0) {
        // opacity DOWN
        opacityValue = 1.0 - opacityOffTimer.getElapsedTimeInMilliSec() / opacityOffMs;
        if(opacityValue < 0.0) {
            opacityValue = 0.0;
        }
    } else if(isVisible && opacityValue < 1.0) {
        // opacity UP
        opacityValue = opacityOnTimer.getElapsedTimeInMilliSec() / opacityOnMs;
        if(opacityValue > 1.0) {
            opacityValue = 1.0;
        }
    }
}

void StGLWidget::stglUpdate(const StPointD_t& cursorZo) {
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
    isResized = false;
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

bool StGLWidget::tryClick(const StPointD_t& cursorZo, const int& mouseBtn, bool& isItemClicked) {
    if(!isVisible()) {
        return false; // nothing to see - nothing to click... (all children too!)
    }
    for(StGLWidget *child(myChildren.getLast()), *childActive(NULL); child != NULL;) {
        childActive = child;
        child = child->getPrev();
        childActive->tryClick(cursorZo, mouseBtn, isItemClicked);
    }
    if(!isItemClicked && isPointIn(cursorZo)) {
        setClicked(mouseBtn, true);
        isItemClicked = signals.onMouseClick(mouseBtn);
        return true;
    }
    return false;
}

bool StGLWidget::tryUnClick(const StPointD_t& cursorZo, const int& mouseBtn, bool& isItemUnclicked) {
    if(!isVisible()) {
        return false; // nothing to see - nothing to click... (all children too!)
    }
    for(StGLWidget *child(myChildren.getLast()), *childActive(NULL); child != NULL;) {
        childActive = child;
        child = child->getPrev();
        childActive->tryUnClick(cursorZo, mouseBtn, isItemUnclicked);
    }
    bool selfClicked = isClicked(mouseBtn) && isPointIn(cursorZo);
    setClicked(mouseBtn, false);
    if(!isItemUnclicked && selfClicked) {
        isItemUnclicked = signals.onMouseUnclick(mouseBtn);
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

StGLProjCamera* StGLWidget::getCamera() {
    return myRoot->StGLRootWidget::getCamera();
}

StGLContext& StGLWidget::getContext() {
    return myRoot->StGLRootWidget::getContext();
}

void StGLWidget::destroyWithDelay(StGLWidget* theWidget) {
    myRoot->destroyWithDelay(theWidget);
}
