/**
 * Copyright © 2014 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#include <StGLWidgets/StGLTable.h>
#include <StGLWidgets/StGLRootWidget.h>

#include <StGL/StGLContext.h>
#include <StGLCore/StGLCore20.h>

#include <StStrings/StDictionary.h>

#include <stAssert.h>

namespace {
    static const StString CLASS_NAME("StGLTable");
};

StGLTableItem::StGLTableItem(StGLTable* theParent)
: StGLWidget(theParent,
             theParent->getMarginLeft(), theParent->getMarginTop(),
             StGLCorner(ST_VCORNER_TOP, ST_HCORNER_LEFT),
             theParent->getRoot()->scale(32),
             theParent->getRoot()->scale(32)),
  myColSpan(1),
  myRowSpan(1) {
    setVisibility(true, true);
}

StGLTableItem::~StGLTableItem() {
    //
}

StGLTable::StGLTable(StGLWidget* theParent,
                     const int   theLeft,
                     const int   theTop,
                     StGLCorner  theCorner)
: StGLWidget(theParent,
             theLeft, theTop,
             theCorner,
             theParent->getRoot()->scale(32),
             theParent->getRoot()->scale(32)),
  myMarginLeft   (theParent->getRoot()->scale(5)),
  myMarginRight  (theParent->getRoot()->scale(5)),
  myMarginTop    (theParent->getRoot()->scale(2)),
  myMarginBottom (theParent->getRoot()->scale(2)),
  myIsInitialized(false) {
    //
}

StGLTable::~StGLTable() {
    //
}

const StString& StGLTable::getClassName() {
    return CLASS_NAME;
}

void StGLTable::setupTable(const int theNbRows,
                           const int theNbColumns) {
    // destroy old content
    for(size_t aRowIter = 0; aRowIter < myTable.size(); ++aRowIter) {
        StArrayList<StGLTableItem*>& aRow = myTable.changeValue(aRowIter);
        for(size_t aColIter = 0; aColIter < aRow.size(); ++aColIter) {
            StGLTableItem* anItem = aRow.changeValue(aColIter);
            delete anItem;
        }
    }
    myTable.clear();

    // initialize new empty content
    for(int aRowIter = 0; aRowIter < theNbRows; ++aRowIter) {
        myTable.add(StArrayList<StGLTableItem*>());
        StArrayList<StGLTableItem*>& aRow = myTable.changeLast();
        aRow.initArray(theNbColumns);
        for(size_t aColIter = 0; aColIter < aRow.size(); ++aColIter) {
            aRow.changeValue(aColIter) = new StGLTableItem(this);
        }
    }
    myRowBottoms.initArray(theNbRows);
    myColRights .initArray(theNbColumns);
    stMemZero(&myRowBottoms.changeFirst(), sizeof(int) * myRowBottoms.size());
    stMemZero(&myColRights .changeFirst(), sizeof(int) * myColRights .size());
}

void StGLTable::fillFromMap(const StDictionary& theMap,
                            const StGLVec3&     theTextColor,
                            const int           theMaxWidth,
                            const int           theCol1MaxWidth,
                            const int           theRowId,
                            const int           theColId) {
    ST_ASSERT_SLIP(theRowId >= 0 && theColId >= 0,
                   "StGLTable::fillFromMap() out of range",
                   return);
    const int aRowsNb = theRowId + (int )theMap.size();
    const int aColsNb = theColId + 2;
    if(aRowsNb > (int )myRowBottoms.size()
    || aColsNb > (int )myColRights.size()) {
        setupTable(aRowsNb, aColsNb);
    }

    // fill first column with keys
    const int aCol1MaxWidth = theCol1MaxWidth - (myMarginLeft + myMarginRight);
    int       aCol1Width    = 0;
    for(size_t anIter = 0; anIter < theMap.size(); ++anIter) {
        const StDictEntry& aPair  = theMap.getValue(anIter);
        StGLTableItem&     anItem = changeElement(theRowId + (int )anIter, theColId);
        anItem.setRowSpan(1);
        anItem.setColSpan(1);

        StGLTextArea* aText = new StGLTextArea(&anItem, 0, 0, StGLCorner(ST_VCORNER_TOP, ST_HCORNER_LEFT));
        aText->setupAlignment(StGLTextFormatter::ST_ALIGN_X_RIGHT,
                              StGLTextFormatter::ST_ALIGN_Y_TOP);
        aText->setText(aPair.getName().isEmpty() ? aPair.getKey() : aPair.getName());
        aText->setTextColor(theTextColor);
        aText->setupStyle(StFTFont::Style_Bold);
        aText->setVisibility(true, true);
        aText->stglInitAutoHeightWidth(aCol1MaxWidth);
        aCol1Width = stMax(aCol1Width, aText->getRectPx().width());
    }

    // adjust width of all elements in first column
    // (alternatively we might adjust right corner)
    for(size_t anIter = 0; anIter < theMap.size(); ++anIter) {
        StGLTableItem& anItem = changeElement(theRowId + (int )anIter, theColId);
        anItem.getItem()->changeRectPx().right() = anItem.getItem()->getRectPx().left() + aCol1Width;
        ((StGLTextArea* )anItem.getItem())->setTextWidth(aCol1Width);
    }

    // fill second column with values
    int aCol2MaxWidth = theMaxWidth - aCol1Width - 2 * (myMarginLeft + myMarginRight);
    for(size_t anIter = 0; anIter < theMap.size(); ++anIter) {
        const StDictEntry& aPair  = theMap.getValue(anIter);
        StGLTableItem&     anItem = changeElement(theRowId + (int )anIter, theColId + 1);
        anItem.setRowSpan(1);
        anItem.setColSpan(1);

        StGLTextArea* aText = new StGLTextArea(&anItem, 0, 0, StGLCorner(ST_VCORNER_TOP, ST_HCORNER_LEFT));
        aText->setupAlignment(StGLTextFormatter::ST_ALIGN_X_LEFT,
                              StGLTextFormatter::ST_ALIGN_Y_TOP);
        aText->setText(aPair.getValue());
        aText->setTextColor(theTextColor);
        aText->setVisibility(true, true);
        aText->stglInitAutoHeightWidth(aCol2MaxWidth);
    }

    updateLayout();
}

void StGLTable::stglResize() {
    StGLWidget::stglResize();
}

bool StGLTable::stglInit() {
    myIsInitialized = StGLWidget::stglInit();
    if(!myIsInitialized) {
        return false;
    }

    updateLayout();
    return true;
}

void StGLTable::updateLayout() {
    if(myRowBottoms.isEmpty()
    || myColRights .isEmpty()) {
        return;
    }

    // determine rows heights
    int aBottomPrev = 0;
    for(size_t aRowIter = 0; aRowIter < myRowBottoms.size(); ++aRowIter) {
        StArrayList<StGLTableItem*>& aRow = myTable.changeValue(aRowIter);
        for(size_t aColIter = 0; aColIter < myColRights.size(); ++aColIter) {
            StGLTableItem* anItem = aRow.changeValue(aColIter);
            if(anItem->getItem() == NULL) {
                continue;
            }

            const int aBefore = aRowIter != 0
                              ? myRowBottoms.changeValue(aRowIter - 1)
                              : 0;
            size_t aBotRowId = aRowIter + anItem->getRowSpan() - 1;
            int&   aBottom   = myRowBottoms.changeValue(aBotRowId);
            aBottom = stMax(aBottom,
                            aBefore + anItem->getItem()->getRectPx().height()
                          + myMarginTop + myMarginBottom);
        }
        int& aBottom = myRowBottoms.changeValue(aRowIter);
        aBottom      = stMax(aBottom, aBottomPrev);
        aBottomPrev  = aBottom;
    }

    // determine columns widths
    int aRightPrev = 0;
    for(size_t aColIter = 0; aColIter < myColRights.size(); ++aColIter) {
        for(size_t aRowIter = 0; aRowIter < myRowBottoms.size(); ++aRowIter) {
            StGLTableItem* anItem = myTable.changeValue(aRowIter).changeValue(aColIter);
            if(anItem->getItem() == NULL) {
                continue;
            }

            const int aBefore = aColIter != 0
                              ? myColRights.changeValue(aColIter - 1)
                              : 0;
            size_t aRightColId = aColIter + anItem->getColSpan() - 1;
            int&   aRight      = myColRights.changeValue(aRightColId);
            aRight = stMax(aRight,
                           aBefore + anItem->getItem()->getRectPx().width()
                         + myMarginLeft + myMarginRight);
        }
        int& aRight = myColRights.changeValue(aColIter);
        aRight      = stMax(aRight, aRightPrev);
        aRightPrev  = aRight;
    }
    changeRectPx().right()  = getRectPx().left() + myColRights.getLast();
    changeRectPx().bottom() = getRectPx().top()  + myRowBottoms.getLast();

    // adjust table elements positions
    int aTop = 0;
    for(size_t aRowIter = 0; aRowIter < myRowBottoms.size(); aTop = myRowBottoms.getValue(aRowIter++)) {
        StArrayList<StGLTableItem*>& aRow = myTable.changeValue(aRowIter);
        int aLeft = 0;
        for(size_t aColIter = 0; aColIter < myColRights.size(); aLeft = myColRights.getValue(aColIter++)) {
            StGLTableItem* anItem = aRow.changeValue(aColIter);
            const size_t aBotRowId   = aRowIter + anItem->getRowSpan() - 1;
            const size_t aRightColId = aColIter + anItem->getColSpan() - 1;
            anItem->changeRectPx().top()    = aTop  + myMarginTop;
            anItem->changeRectPx().left()   = aLeft + myMarginLeft;
            anItem->changeRectPx().bottom() = myRowBottoms.changeValue(aBotRowId)   - myMarginBottom;
            anItem->changeRectPx().right()  = myColRights .changeValue(aRightColId) - myMarginRight;
        }
    }
}

void StGLTable::stglDraw(unsigned int theView) {
    if(!myIsInitialized || !isVisible()) {
        return;
    }

    if(isResized) {
        stglResize();
        isResized = false;
    }

    StGLWidget::stglDraw(theView);
}
