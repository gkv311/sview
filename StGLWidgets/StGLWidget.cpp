/**
 * StGLWidgets, small C++ toolkit for writing GUI using OpenGL.
 * Copyright © 2009-2017 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
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
  myUserData(0),
  myRectPx(theTop, theTop + theHeight, theLeft, theLeft + theWidth),
  myCorner(theCorner),
  myOpacity(1.0f),
  myIsResized(true),
  myHasFocus(false),
  myIsTopWidget(false) {
    if(myParent != NULL) {
        myParent->getChildren()->add(this);
    }
    stMemSet(myMouseClicked, 0, sizeof(myMouseClicked));
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
    for(StGLWidget* aChildIter = myChildren.getStart(); aChildIter != NULL;) {
        const StGLWidget* aDeleteChild = aChildIter;
        aChildIter = aChildIter->getNext();
        delete aDeleteChild;
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
 * @param theParentRect parent widget absolute position
 * @param theChildRect  child widget position relative to parent
 * @param theCorner     corner modifier for child widget relative to parent
 * @return child widget absolute position
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
        return myRectPx;
    }
    return computeAbsolutePos(myParent->getRectPxAbsolute(), myRectPx, myCorner);
}

StRectI_t StGLWidget::getAbsolute(const StRectI_t& theRectPx) const {
    if(myParent == NULL) {
        return theRectPx;
    }
    return computeAbsolutePos(myParent->getRectPxAbsolute(), theRectPx, myCorner);
}

void StGLWidget::stglScissorRect2d(StGLBoxPx& theScissorRect) const {
    myRoot->stglScissorRect2d(getRectPxAbsolute(), theScissorRect);
}

void StGLWidget::stglScissorRect3d(StGLBoxPx& theScissorRect) const {
    myRoot->stglScissorRect3d(getRectPxAbsolute(), theScissorRect);
}

StPointD_t StGLWidget::getPointGl(const StPointD_t& thePointZo) const {
    double oglWidth  = myRoot->getRootWorkRectGl().right() - myRoot->getRootWorkRectGl().left();
    double oglHeight = myRoot->getRootWorkRectGl().top() - myRoot->getRootWorkRectGl().bottom();
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

void StGLWidget::stglUpdate(const StPointD_t& theCursorZo,
                            bool theIsPreciseInput) {
    if(!isVisible()) {
        return;
    }

    for(StGLWidget* aChildIter = myChildren.getStart(); aChildIter != NULL;) {
        StGLWidget* aChildActive = aChildIter;
        aChildIter = aChildIter->getNext();
        aChildActive->stglUpdate(theCursorZo, theIsPreciseInput);
    }
}

void StGLWidget::stglResize() {
    for(StGLWidget* aChildIter = myChildren.getStart(); aChildIter != NULL;) {
        StGLWidget* aChildActive = aChildIter;
        aChildIter = aChildIter->getNext();
        aChildActive->stglResize();
    }
    myIsResized = false;
}

bool StGLWidget::stglInit() {
    bool isSuccess = true;
    for(StGLWidget* aChildIter = myChildren.getStart(); aChildIter != NULL;) {
        StGLWidget* aChildActive = aChildIter;
        aChildIter = aChildIter->getNext();
        isSuccess = aChildActive->stglInit() && isSuccess;
    }
    return isSuccess;
}

void StGLWidget::stglDraw(unsigned int theView) {
    if(!isVisible()) {
        return;
    }
    for(StGLWidget* aChildIter = myChildren.getStart(); aChildIter != NULL;) {
        StGLWidget* aChildActive = aChildIter;
        aChildIter = aChildIter->getNext();
        aChildActive->stglDraw(theView);
    }
}

bool StGLWidget::isClicked(int theMouseBtn) const {
    if(theMouseBtn > ST_MOUSE_MAX_ID) {
        // ignore out of range buttons
        return false;
    }
    return myMouseClicked[theMouseBtn];
}

void StGLWidget::setClicked(int theMouseBtn, bool theIsClicked) {
    if(theMouseBtn > ST_MOUSE_MAX_ID) {
        // ignore out of range buttons
        ST_DEBUG_LOG("StGLWidget, mouse button click #" + theMouseBtn + " ignored!");
        return;
    }
    myMouseClicked[theMouseBtn] = theIsClicked;
}

bool StGLWidget::tryClick(const StClickEvent& theEvent,
                          bool&               theIsItemClicked) {
    if(!isVisible()) {
        return false;
    }

    for(StGLWidget* aChildIter = myChildren.getLast(); aChildIter != NULL;) {
        StGLWidget* aChildActive = aChildIter;
        aChildIter = aChildIter->getPrev();
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

    for(StGLWidget* aChildIter = myChildren.getLast(); aChildIter != NULL;) {
        StGLWidget* aChildActive = aChildIter;
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
    for(StGLWidget* aChildIter = myChildren.getLast(); aChildIter != NULL;) {
        StGLWidget* aChildActive = aChildIter;
        aChildIter = aChildIter->getPrev();
        if(aChildActive->isVisibleAndPointIn(aPnt)
        && aChildActive->doScroll(theEvent)) {
            return true;
        }
    }
    return false;
}

const StGLProjCamera* StGLWidget::getCamera() const {
    return myRoot->StGLRootWidget::getCamera();
}

StGLProjCamera* StGLWidget::changeCamera() {
    return myRoot->StGLRootWidget::changeCamera();
}

StGLContext& StGLWidget::getContext() {
    return myRoot->StGLRootWidget::getContext();
}

void StGLWidget::destroyWithDelay(StGLWidget* theWidget) {
    myRoot->destroyWithDelay(theWidget);
}

StGLContainer::StGLContainer(StGLWidget* theParent,
                             int theLeft, int theTop,
                             StGLCorner theCorner,
                             int theWidth, int theHeight)
: StGLWidget(theParent, theLeft, theTop, theCorner, theWidth, theHeight) {}

StGLContainer::~StGLContainer() {}

bool StGLContainer::tryClick(const StClickEvent& theEvent,
                             bool&               theIsItemClicked) {
    if(!isVisible()) {
        return false;
    }
    for(StGLWidget* aChildIter = myChildren.getLast(); aChildIter != NULL;) {
        StGLWidget* aChildActive = aChildIter;
        aChildIter = aChildIter->getPrev();
        aChildActive->tryClick(theEvent, theIsItemClicked);
    }
    return false;
}

bool StGLContainer::tryUnClick(const StClickEvent& theEvent,
                               bool&               theIsItemUnclicked) {
    if(!isVisible()) {
        return false;
    }
    for(StGLWidget* aChildIter = myChildren.getLast(); aChildIter != NULL;) {
        StGLWidget* aChildActive = aChildIter;
        aChildIter = aChildIter->getPrev();
        aChildActive->tryUnClick(theEvent, theIsItemUnclicked);
    }
    return false;
}

void StGLWidget::setOpacity(const float theOpacity, bool theToSetChildren) {
    myOpacity = theOpacity;
    if(!theToSetChildren) {
        return;
    }
    for(StGLWidget* aChildIter = myChildren.getStart(); aChildIter != NULL; aChildIter = aChildIter->getNext()) {
        aChildIter->setOpacity(theOpacity, theToSetChildren);
    }
}
