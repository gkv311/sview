/**
 * StGLWidgets, small C++ toolkit for writing GUI using OpenGL.
 * Copyright © 2013-2015 Kirill Gavrilov <kirill@sview.ru
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#include <StGLWidgets/StGLPlayList.h>

#include <StGLWidgets/StGLMenuItem.h>
#include <StGLWidgets/StGLMenuProgram.h>
#include <StGLWidgets/StGLRootWidget.h>

#include <StCore/StEvent.h>
#include <StGL/StGLContext.h>
#include <StGLCore/StGLCore20.h>

StGLPlayList::StGLPlayList(StGLWidget*                 theParent,
                           const StHandle<StPlayList>& theList)
: StGLWidget(theParent, -theParent->getRoot()->scale(32), 0),
  myMenu(NULL),
  myBarColor(getRoot()->getColorForElement(StGLRootWidget::Color_ScrollBar)),
  myList(theList),
  myFromId(0),
  myItemsNb(0),
  myToResetList(false),
  myToUpdateList(false),
  myIsLeftClick(false),
  myDragDone(0),
  myFlingAccel((double )myRoot->scale(200)),
  myFlingYSpeed(0.0) {
    myMenu = new StGLMenu(this, 0, 0, StGLMenu::MENU_VERTICAL_COMPACT);

    myMenu->setItemWidth(myRoot->scale(250));
    myMenu->setColor(StGLVec4(0.2f, 0.2f, 0.2f, 0.5f));

    StGLWidget::signals.onMouseUnclick = stSlot(this, &StGLPlayList::doMouseUnclick);
    myList->signals.onPlaylistChange  += stSlot(this, &StGLPlayList::doResetList);
    myList->signals.onTitleChange     += stSlot(this, &StGLPlayList::doChangeItem);

    changeRectPx().right()  = getRectPx().left() + myMenu->getRectPx().width();
    changeRectPx().bottom() = getRectPx().top()  + myMenu->getRectPx().height();
}

StGLPlayList::~StGLPlayList() {
    myBarVertBuf.release(getContext());
    myList->signals.onPlaylistChange  -= stSlot(this, &StGLPlayList::doResetList);
    myList->signals.onTitleChange     -= stSlot(this, &StGLPlayList::doChangeItem);
}

StGLMenuItem* StGLPlayList::addItem() {
    StGLMenuItem* aNewItem = new StGLPassiveMenuItem(myMenu);
    return aNewItem;
}

void StGLPlayList::doItemClick(const size_t theItem) {
    if(myList->walkToPosition(myFromId + theItem)) {
        signals.onOpenItem();
    }
}

void StGLPlayList::updateList() {
    StArrayList<StString> aList;
    myList->getSubList(aList, myFromId, myFromId + myItemsNb);
    const size_t aCurrent     = myList->getCurrentId() - myFromId;
    const size_t anUpperLimit = aList.size();

    int anIter = 0;
    for(StGLWidget* aChild = myMenu->getChildren()->getStart();
        aChild != NULL && anIter < myItemsNb; ++anIter, aChild = aChild->getNext()) {
        StGLMenuItem* anItem = dynamic_cast<StGLMenuItem*>(aChild);
        anItem->setClicked(ST_MOUSE_LEFT, false);
        if(size_t(anIter) < anUpperLimit) {
            anItem->setText(aList.getValue(anIter));
            anItem->setOpacity(1.0f, false);
            anItem->setFocus(size_t(anIter) == aCurrent);
            anItem->changeRectPx().right() = anItem->getRectPx().left() + myMenu->getItemWidth();
        } else {
            anItem->setText("");
            anItem->setOpacity(0.0f, false);
            //anItem->changeRectPx().right() = anItem->getRectPx().left();
        }
    }
    stglInitMenu();
}

void StGLPlayList::doResetList() {
    myToResetList = true;
}

void StGLPlayList::doChangeItem(const size_t ) {
    myToUpdateList = true;
}

void StGLPlayList::doMouseClick(const int theBtnId) {
    const bool toAbort = myFlingTimer.isOn();
    myDragTimer .stop();
    myFlingTimer.stop();
    myClickPntZo = myRoot->getCursorZo();
    myFlingPntZo = myRoot->getCursorZo();
    myDragDone   = 0;
    if(theBtnId != ST_MOUSE_LEFT) {
        myIsLeftClick = false;
        return;
    }

    myIsLeftClick = true;
    for(StGLWidget* aChild = myMenu->getChildren()->getStart(); aChild != NULL; aChild = aChild->getNext()) {
        StGLMenuItem* anItem = dynamic_cast<StGLMenuItem*>(aChild);
        if(anItem != NULL
        //&& anItem->isClicked(theBtnId)) {
        && anItem->isPointIn(myClickPntZo)) {
            // stick to the center
            const StRectI_t aRect = anItem->getRectPxAbsolute();
            myClickPntZo.y() = double(aRect.top() + aRect.height() / 2 - myRoot->getRectPx().top()) / double(myRoot->getRectPx().height());
            if(toAbort) {
                anItem->setClicked(theBtnId, false);
            }
            break;
        }
    }
}

void StGLPlayList::doMouseUnclick(const int theBtnId) {
    if(theBtnId == ST_MOUSE_LEFT) {
        myIsLeftClick = false;
    }
}

bool StGLPlayList::doScroll(const StScrollEvent& theEvent) {
    if(theEvent.StepsY >= 1) {
        if(myFromId == 0) {
            return true;
        }
        --myFromId;
        updateList();
    } else if(theEvent.StepsY <= -1) {
        if(myFromId + myItemsNb >= myList->getItemsCount()) {
            return true;
        }
        ++myFromId;

        updateList();
    }
    return true;
}

void StGLPlayList::stglResize() {
    const int aNewHeight = myParent->getRectPx().height() - myFitMargins.top - myFitMargins.bottom - myMargins.top - myMargins.bottom;
    const int anItemsOld = myItemsNb;
    myItemsNb = stMax(aNewHeight / myMenu->getItemHeight(), 0);

    StArrayList<StString> aList;
    myList->getSubList(aList, myFromId, myFromId + myItemsNb);
    const size_t anUpperLimit = aList.size();

    for(int anIter = anItemsOld; anIter > myItemsNb; --anIter) {
        StGLWidget* aChild = myMenu->getChildren()->getLast();
        if(aChild != NULL) {
            delete aChild;
        }
    }

    for(int anIter = anItemsOld; anIter < myItemsNb; ++anIter) {
        StGLMenuItem* anItem = addItem();
        anItem->setUserData(anIter);
        anItem->signals.onItemClick = stSlot(this, &StGLPlayList::doItemClick);
        anItem->StGLWidget::signals.onMouseClick   += stSlot(this, &StGLPlayList::doMouseClick);
        anItem->StGLWidget::signals.onMouseUnclick += stSlot(this, &StGLPlayList::doMouseUnclick);

        anItem->setHilightText();
        anItem->setHilightColor(StGLVec4(0.5f, 0.5f, 0.5f, 1.0f));

        if(size_t(anIter) < anUpperLimit) {
            anItem->setText(aList.getValue(anIter));
            anItem->setOpacity(1.0f, false);
        }
    }

    if(myItemsNb != anItemsOld) {
        stglInitMenu();
    }

    int anItemWidth = myRoot->getRectPx().width() / 4;
    anItemWidth = stMin(anItemWidth, myRoot->scale(400));
    anItemWidth = stMax(anItemWidth, myRoot->scale(250));
    myMenu->setItemWidth(anItemWidth);
    resizeWidth();

    StGLWidget::stglResize();
}

void StGLPlayList::resizeWidth() {
    changeRectPx().top()    = myFitMargins.top;
    changeRectPx().bottom() = myParent->getRectPx().height() - myFitMargins.bottom;
    for(StGLWidget* aChild = myMenu->getChildren()->getStart(); aChild != NULL; aChild = aChild->getNext()) {
        StGLMenuItem* anItem = dynamic_cast<StGLMenuItem*>(aChild);
        if(anItem != NULL) {
            anItem->setTextWidth(-1);
            anItem->changeRectPx().right() = anItem->getRectPx().left() + myMenu->getItemWidth();
        }
    }

    myMenu->changeRectPx().right() = myMenu->getRectPx().left() + myMenu->getItemWidth();
    myMenu->changeRectPx().bottom() = getRectPx().height();
    changeRectPx().right() = getRectPx().left() + myMenu->getRectPx().width();
}

bool StGLPlayList::stglInitMenu() {
    changeRectPx().top()    = myFitMargins.top;
    changeRectPx().bottom() = myParent->getRectPx().height() - myFitMargins.bottom;
    const int aWidth = myMenu->getItemWidth();
    if(!myMenu->stglInit()) {
        return false;
    }

    myMenu->setItemWidth(aWidth);
    //myMenu->changeRectPx().bottom() += myMargins.top + myMargins.bottom;
    myMenu->changeRectPx().bottom() = getRectPx().height();
    changeRectPx().right()  = getRectPx().left() + myMenu->getRectPx().width();
    resizeWidth();
    return true;
}

bool StGLPlayList::stglInit() {
    bool isOk = stglInitMenu();
    for(StGLWidget *aChild(myChildren.getStart()), *aChildActive(NULL); aChild != NULL;) {
        aChildActive = aChild;
        aChild       = aChild->getNext();
        if(aChild == myMenu) {
            continue;
        }
        isOk = aChildActive->stglInit() && isOk;
    }
    return isOk;
}

void StGLPlayList::stglDrawScrollBar(unsigned int theView) {
    StGLContext& aCtx = getContext();
    StGLMenuProgram& aProgram = myRoot->getMenuProgram();
    if((size_t )myItemsNb > myList->getItemsCount()
    || !aProgram.isValid()) {
        return;
    }

    if(theView != ST_DRAW_RIGHT) {
        const size_t aSizeY       = stMax(myItemsNb * myMenu->getItemHeight(), 1);
        const size_t aContSizeY   = myList->getItemsCount() * myMenu->getItemHeight();
        const double aScaleY      = double(aSizeY) / double(aContSizeY);
        const int    aScrollSizeY = stMax(int(aScaleY * (double )aSizeY), myRoot->scale(4));
        const double aPosY        = double(myFromId * myMenu->getItemHeight()) / double(aContSizeY - aSizeY);

        StArray<StGLVec2> aVertices(4);
        StRectI_t aRectPx = getRectPxAbsolute();
        aRectPx.left()   =  aRectPx.right() - myRoot->scale(2);
        aRectPx.top()    += int(aPosY * double(aSizeY - aScrollSizeY));
        aRectPx.bottom() =  aRectPx.top() + aScrollSizeY;

        myRoot->getRectGl(aRectPx, aVertices);
        myBarVertBuf.init(aCtx, aVertices);
    }
    if(!myBarVertBuf.isValid()) {
        return;
    }

    aCtx.core20fwd->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    aCtx.core20fwd->glEnable(GL_BLEND);
    aProgram.use(aCtx, myRoot->getScreenDispX());
    myBarVertBuf.bindVertexAttrib  (aCtx, aProgram.getVVertexLoc());

    aProgram.setColor(aCtx, myBarColor, myOpacity);
    aCtx.core20fwd->glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    myBarVertBuf.unBindVertexAttrib(aCtx, aProgram.getVVertexLoc());
    aProgram.unuse(aCtx);
    aCtx.core20fwd->glDisable(GL_BLEND);
}

void StGLPlayList::stglUpdate(const StPointD_t& theCursorZo,
                              bool theIsPreciseInput) {
    if(!isVisible()) {
        StGLWidget::stglUpdate(theCursorZo, theIsPreciseInput);
        return;
    }

    if(myIsLeftClick
    && !myRoot->isClicked(ST_MOUSE_LEFT)) {
        // handle global unclick to start inertial scrolling
        myIsLeftClick = false;
        myDragDone = 0;
        myDragTimer.stop();
        if(std::abs(myFlingYSpeed) > 0.0000001) {
            myFlingTimer.restart();
        }
    }

    if((myIsLeftClick || myFlingTimer.isOn())) {
        StPointD_t aDelta;
        if(myFlingTimer.isOn()) {
            double aTime   = myFlingTimer.getElapsedTime();
            double anA     = (myFlingYSpeed > 0.0 ? -1.0 : 1.0) * (myFlingAccel / double(myRoot->getRectPx().height()));
            double aDeltaY = myFlingYSpeed * aTime + anA * aTime * aTime;
            aDelta = myFlingPntZo - myClickPntZo;
            aDelta.y() += aDeltaY;
        } else {
            double aTime = myDragTimer.getElapsedTime();
            if(aTime > 0.0000001) {
                myFlingYSpeed = (myRoot->getCursorZo().y() - myFlingPntZo.y()) / aTime;
            }
            myFlingPntZo = myRoot->getCursorZo();
            myDragTimer.restart();
            aDelta = myFlingPntZo - myClickPntZo;
        }
        int64_t aPrevDone = myDragDone;
        for(;;) {
            double aDeltaY = aDelta.y() * myRoot->getRectPx().height() + double(myDragDone * myMenu->getItemHeight());
            if(aDeltaY > (double(myMenu->getItemHeight() / 2))) {
                if(myFlingTimer.isOn()
                && myFlingYSpeed < 0.0) {
                    myFlingTimer.stop();
                    break;
                } else if(myFromId == 0
                       || aPrevDone < myDragDone) {
                    break;
                }
                --myFromId;
                --myDragDone;
                myToUpdateList = true;
            } else if(aDeltaY < (double(-myMenu->getItemHeight() / 2))) {
                if(myFlingTimer.isOn()
                && myFlingYSpeed > 0.0) {
                    myFlingTimer.stop();
                    break;
                } else if(myFromId + myItemsNb >= myList->getItemsCount()
                       || aPrevDone > myDragDone) {
                    break;
                }
                ++myFromId;
                ++myDragDone;
                myToUpdateList = true;
            } else {
                break;
            }
        }
    }

    StGLWidget::stglUpdate(theCursorZo, theIsPreciseInput);
}

void StGLPlayList::stglDraw(unsigned int theView) {
    if(!isVisible()) {
        return;
    }

    if((myToUpdateList || myToResetList)
    && theView != ST_DRAW_RIGHT) {
        if(myToResetList) {
            myFromId = 0;
        }
        myToUpdateList = false;
        myToResetList  = false;
        updateList();
    }

    const size_t aCurrent = myList->getCurrentId() - myFromId;
    int anIter = 0;
    for(StGLWidget* aChild = myMenu->getChildren()->getStart();
        aChild != NULL; ++anIter, aChild = aChild->getNext()) {
        StGLMenuItem* anItem = dynamic_cast<StGLMenuItem*>(aChild);
        if(anItem != NULL) {
            anItem->setFocus(size_t(anIter) == aCurrent);
        }
    }

    StGLContext& aCtx = getContext();
    StGLBoxPx aScissorRect;
    stglScissorRect2d(aScissorRect);
    aCtx.stglSetScissorRect(aScissorRect, true);

    StGLWidget::stglDraw(theView);
    stglDrawScrollBar(theView);

    aCtx.stglResetScissorRect();
}
