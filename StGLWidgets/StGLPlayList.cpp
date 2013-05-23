/**
 * Copyright © 2013 Kirill Gavrilov <kirill@sview.ru
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#include <StGLWidgets/StGLPlayList.h>
#include <StGLWidgets/StGLMenuItem.h>

#include <StGL/StGLContext.h>
#include <StGLCore/StGLCore20.h>

StGLPlayList::StGLPlayList(StGLWidget*                 theParent,
                           const StHandle<StPlayList>& theList)
: StGLMenu(theParent, -32, 0, StGLMenu::MENU_VERTICAL),
  myList(theList),
  myFromId(0),
  myItemsNb(0),
  myToUpdateList(false) {
    myWidth = 250;
    StGLWidget::signals.onMouseUnclick = stSlot(this, &StGLPlayList::doMouseUnclick);
    myList->signals.onPlaylistChange  += stSlot(this, &StGLPlayList::doResetList);
}

StGLPlayList::~StGLPlayList() {
    //
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
        if(size_t(anIter) < anUpperLimit) {
            anItem->setText(aList.getValue(anIter));
            anItem->setVisibility(true, true);
            anItem->setFocus(size_t(anIter) == aCurrent);
        } else {
            anItem->setVisibility(false, true);
        }
    }
    stglInit();
}

void StGLPlayList::doResetList() {
    myToUpdateList = true;
}

void StGLPlayList::doMouseUnclick(const int theBtnId) {
    switch(theBtnId) {
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

void StGLPlayList::stglResize(const StRectI_t& theWinRectPx) {
    const int aNewHeight = theWinRectPx.height();
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
        StGLMenuItem* anItem = addItem("");
        anItem->setUserData(anIter);
        anItem->signals.onItemClick = stSlot(this, &StGLPlayList::doItemClick);
        anItem->StGLWidget::signals.onMouseUnclick += stSlot(this, &StGLPlayList::doMouseUnclick);
        if(size_t(anIter) < anUpperLimit) {
            anItem->setText(aList.getValue(anIter));
            anItem->setVisibility(true, true);
        }
    }

    if(myItemsNb != anItemsOld) {
        stglInit();
    }

    myWidth = theWinRectPx.width() / 4;
    myWidth = stMin(myWidth, 400);
    myWidth = stMax(myWidth, 250);

    changeRectPx().right() = getRectPx().left() + myWidth;
    StGLMenu::stglResize(theWinRectPx);
}

bool StGLPlayList::stglInit() {
    const int aWidth = myWidth;
    if(!StGLMenu::stglInit()) {
        return false;
    }

    myWidth = aWidth;
    for(StGLWidget* aChild = getChildren()->getStart(); aChild != NULL; aChild = aChild->getNext()) {
        StGLMenuItem* anItem = dynamic_cast<StGLMenuItem*>(aChild);
        if(anItem != NULL) {
            anItem->setTextWidth(-1);
        }
    }

    changeRectPx().right() = getRectPx().left() + myWidth;

    return true;
}

void StGLPlayList::stglDraw(unsigned int theView) {
    if(!isVisible()) {
        return;
    }

    if(myToUpdateList
    && theView != ST_DRAW_RIGHT) {
        myToUpdateList = false;
        myFromId = 0;
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
}
