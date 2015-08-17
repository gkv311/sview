/**
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

#include <StGL/StGLContext.h>
#include <StGLCore/StGLCore20.h>

namespace {
    static const size_t SHARE_PROGRAM_ID = StGLRootWidget::generateShareId();
}

StGLPlayList::StGLPlayList(StGLWidget*                 theParent,
                           const StHandle<StPlayList>& theList)
: StGLMenu(theParent, -theParent->getRoot()->scale(32), 0, StGLMenu::MENU_VERTICAL),
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
    myWidth = myRoot->scale(250);
    StGLWidget::signals.onMouseUnclick = stSlot(this, &StGLPlayList::doMouseUnclick);
    myList->signals.onPlaylistChange  += stSlot(this, &StGLPlayList::doResetList);
    myList->signals.onTitleChange     += stSlot(this, &StGLPlayList::doChangeItem);

    myColorVec = StGLVec4(0.2f, 0.2f, 0.2f, 0.5f);
}

StGLPlayList::~StGLPlayList() {
    myBarVertBuf.release(getContext());
    myList->signals.onPlaylistChange  -= stSlot(this, &StGLPlayList::doResetList);
    myList->signals.onTitleChange     -= stSlot(this, &StGLPlayList::doChangeItem);
}

StGLMenuItem* StGLPlayList::addItem() {
    StGLMenuItem* aNewItem = new StGLPassiveMenuItem(this);
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
    for(StGLWidget* aChild = getChildren()->getStart();
        aChild != NULL && anIter < myItemsNb; ++anIter, aChild = aChild->getNext()) {
        StGLMenuItem* anItem = dynamic_cast<StGLMenuItem*>(aChild);
        anItem->setClicked(ST_MOUSE_LEFT, false);
        if(size_t(anIter) < anUpperLimit) {
            anItem->setText(aList.getValue(anIter));
            anItem->setVisibility(true, true);
            anItem->setFocus(size_t(anIter) == aCurrent);
            anItem->changeRectPx().right() = anItem->getRectPx().left() + myWidth;
        } else {
            anItem->setText("");
            anItem->setVisibility(false, true);
            //anItem->changeRectPx().right() = anItem->getRectPx().left();
        }
    }
    stglInit();
}

void StGLPlayList::doResetList() {
    myToResetList = true;
}

void StGLPlayList::doChangeItem(const size_t ) {
    myToUpdateList = true;
}

void StGLPlayList::doMouseClick(const int theBtnId) {
    if(theBtnId != ST_MOUSE_LEFT) {
        return;
    }

    myIsLeftClick = true;
    myClickPntZo  = myRoot->getCursorZo();
    myFlingPntZo  = myRoot->getCursorZo();
    myDragDone    = 0;
    myDragTimer .stop();
    myFlingTimer.stop();
    for(StGLWidget* aChild = getChildren()->getStart(); aChild != NULL; aChild = aChild->getNext()) {
        StGLMenuItem* anItem = dynamic_cast<StGLMenuItem*>(aChild);
        if(anItem != NULL
        && anItem->isPointIn(myClickPntZo)) {
            // stick to the center
            const StRectI_t aRect = anItem->getRectPxAbsolute();
            myClickPntZo.y() = double(aRect.top() + aRect.height() / 2) / double(myRoot->getRectPx().height());
            break;
        }
    }
}

void StGLPlayList::doMouseUnclick(const int theBtnId) {
    switch(theBtnId) {
        case ST_MOUSE_LEFT: {
            myIsLeftClick = false;
            return;
        }
        case ST_MOUSE_SCROLL_V_UP: {
            if(myFromId == 0) {
                return;
            }
            --myFromId;
            updateList();
            return;
        }
        case ST_MOUSE_SCROLL_V_DOWN: {
            if(myFromId + myItemsNb >= myList->getItemsCount()) {
                return;
            }
            ++myFromId;

            updateList();
            return;
        }
        default: return;
    }
}

void StGLPlayList::stglResize() {
    const int aNewHeight = myRoot->getRectPx().height();
    const int anItemsOld = myItemsNb;
    myItemsNb = stMax(aNewHeight / myItemHeight - 6, 0);

    StArrayList<StString> aList;
    myList->getSubList(aList, myFromId, myFromId + myItemsNb);
    const size_t anUpperLimit = aList.size();

    for(int anIter = anItemsOld; anIter > myItemsNb; --anIter) {
        StGLWidget* aChild = getChildren()->getLast();
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
            anItem->setVisibility(true, true);
        }
    }

    if(myItemsNb != anItemsOld) {
        stglInit();
    }

    myWidth = myRoot->getRectPx().width() / 4;
    myWidth = stMin(myWidth, myRoot->scale(400));
    myWidth = stMax(myWidth, myRoot->scale(250));
    resizeWidth();

    StGLMenu::stglResize();
}

void StGLPlayList::resizeWidth() {
    for(StGLWidget* aChild = getChildren()->getStart(); aChild != NULL; aChild = aChild->getNext()) {
        StGLMenuItem* anItem = dynamic_cast<StGLMenuItem*>(aChild);
        if(anItem != NULL) {
            anItem->setTextWidth(-1);
            anItem->changeRectPx().right() = anItem->getRectPx().left() + myWidth;
        }
    }

    changeRectPx().right() = getRectPx().left() + myWidth;
}

bool StGLPlayList::stglInit() {
    const int aWidth = myWidth;
    if(!StGLMenu::stglInit()) {
        return false;
    }

    myWidth = aWidth;
    resizeWidth();
    return true;
}

void StGLPlayList::stglDrawScrollBar(unsigned int theView) {
    StGLContext& aCtx = getContext();
    if((size_t )myItemsNb > myList->getItemsCount()
    ||  myProgram.isNull()
    || !myProgram->isValid()) {
        return;
    }

    if(theView != ST_DRAW_RIGHT) {
        const int    aSizeY       = stMax(getRectPx().height(), 1);
        const int    aContSizeY   = myList->getItemsCount() * myItemHeight;
        const double aScaleY      = double(aSizeY) / double(aContSizeY);
        const int    aScrollSizeY = stMax(int(aScaleY * (double )aSizeY), myRoot->scale(4));
        const double aPosY        = double(myFromId * myItemHeight) / double(aContSizeY - aSizeY);

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
    myProgram->use(aCtx, myRoot->getScreenDispX());
    myBarVertBuf.bindVertexAttrib(aCtx, myProgram->getVVertexLoc());

    myProgram->setColor(aCtx, myBarColor, GLfloat(opacityValue));
    aCtx.core20fwd->glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    myBarVertBuf.unBindVertexAttrib(aCtx, myProgram->getVVertexLoc());
    myProgram->unuse(aCtx);
    aCtx.core20fwd->glDisable(GL_BLEND);
}

void StGLPlayList::stglUpdate(const StPointD_t& theCursorZo) {
    if(!isVisible()) {
        StGLMenu::stglUpdate(theCursorZo);
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
            double aDeltaY = aDelta.y() * myRoot->getRectPx().height() + double(myDragDone * myItemHeight);
            if(aDeltaY > (double(myItemHeight / 2))) {
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
            } else if(aDeltaY < (double(-myItemHeight / 2))) {
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

    StGLMenu::stglUpdate(theCursorZo);
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
    for(StGLWidget* aChild = getChildren()->getStart();
        aChild != NULL; ++anIter, aChild = aChild->getNext()) {
        StGLMenuItem* anItem = dynamic_cast<StGLMenuItem*>(aChild);
        if(anItem != NULL) {
            anItem->setFocus(size_t(anIter) == aCurrent);
        }
    }

    StGLContext& aCtx = getContext();
    StGLBoxPx aScissorRect;
    stglScissorRect(aScissorRect);
    aCtx.stglSetScissorRect(aScissorRect, true);

    StGLMenu::stglDraw(theView);

    aCtx.stglResetScissorRect();

    stglDrawScrollBar(theView);
}
