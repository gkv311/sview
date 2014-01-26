/**
 * Copyright © 2014 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#include <StGLWidgets/StGLTable.h>
#include <StGLWidgets/StGLRootWidget.h>

#include <StThreads/StProcess.h>

#include <StGL/StGLContext.h>
#include <StGLCore/StGLCore20.h>

#include <stAssert.h>

namespace {
    static const StString CLASS_NAME("StGLTable");
};

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
    myTable.clear();
    for(int aRowIter = 0; aRowIter < theNbRows; ++aRowIter) {
        myTable.add(StArrayList<StGLTableItem>());
        myTable.changeLast().initArray(theNbColumns);
    }
    myRowBottoms.initArray(theNbRows);
    myColRights .initArray(theNbColumns);
    stMemZero(&myRowBottoms.changeFirst(), sizeof(int) * myRowBottoms.size());
    stMemZero(&myColRights .changeFirst(), sizeof(int) * myColRights .size());
}

void StGLTable::setElement(const int   theRowId,
                           const int   theColId,
                           StGLWidget* theItem,
                           const int   theRowSpan,
                           const int   theColSpan) {
    ST_ASSERT_SLIP(theRowId >= 0
                && theColId >= 0
                && theRowId < (int )myRowBottoms.size()
                && theColId < (int )myColRights.size(),
                   "StGLTable::setElement() out of range",
                   return);

    StGLTableItem& anItem = myTable.changeValue(theRowId).changeValue(theColId);
    anItem.Item    = theItem;
    anItem.RowSpan = theRowSpan;
    anItem.ColSpan = theColSpan;
}

void StGLTable::fillFromMap(const StArgumentsMap& theMap,
                            const StGLVec3&       theTextColor,
                            const int             theMaxWidth,
                            const int             theCol1MaxWidth,
                            const int             theRowId,
                            const int             theColId) {
    ST_ASSERT_SLIP(theRowId >= 0 && theColId >= 0,
                   "StGLTable::fillFromMap() out of range",
                   return);
    const int aRowsNb = theRowId + theMap.size();
    const int aColsNb = theColId + 2;
    if(aRowsNb > (int )myRowBottoms.size()
    || aColsNb > (int )myColRights.size()) {
        setupTable(aRowsNb, aColsNb);
    }

    // fill first column with keys
    int aCol1Width = 0;
    for(size_t anIter = 0; anIter < theMap.size(); ++anIter) {
        const StArgument& aPair = theMap.getValue(anIter);
        StGLTextArea* aText = new StGLTextArea(this, 0, 0, StGLCorner(ST_VCORNER_TOP, ST_HCORNER_LEFT));
        aText->setupAlignment(StGLTextFormatter::ST_ALIGN_X_RIGHT,
                              StGLTextFormatter::ST_ALIGN_Y_TOP);
        aText->setText(aPair.getKey());
        aText->setTextColor(theTextColor);
        aText->setupStyle(StFTFont::Style_Bold);
        aText->setVisibility(true, true);
        aText->stglInitAutoHeightWidth(theCol1MaxWidth);
        aCol1Width = stMax(aCol1Width, aText->getRectPx().width());
        setElement(theRowId + anIter, theColId, aText);
    }

    // adjust width of all elements in first column
    // (alternatively we might adjust right corner)
    for(size_t anIter = 0; anIter < theMap.size(); ++anIter) {
        StGLTableItem& anItem = changeElement(theRowId + anIter, theColId);
        anItem.Item->changeRectPx().right() = anItem.Item->getRectPx().left() + aCol1Width;
        ((StGLTextArea* )anItem.Item)->setTextWidth(aCol1Width);
    }

    // fill second column with values
    int aCol2MaxWidth = theMaxWidth - aCol1Width;
    for(size_t anIter = 0; anIter < theMap.size(); ++anIter) {
        const StArgument& aPair = theMap.getValue(anIter);
        StGLTextArea* aText = new StGLTextArea(this, 0, 0, StGLCorner(ST_VCORNER_TOP, ST_HCORNER_LEFT));
        aText->setupAlignment(StGLTextFormatter::ST_ALIGN_X_LEFT,
                              StGLTextFormatter::ST_ALIGN_Y_TOP);
        aText->setText(aPair.getValue());
        aText->setTextColor(theTextColor);
        aText->setVisibility(true, true);
        aText->stglInitAutoHeightWidth(aCol2MaxWidth);
        setElement(theRowId + anIter, theColId + 1, aText);
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
    for(size_t aRowIter = 0; aRowIter < myRowBottoms.size(); ++aRowIter) {
        StArrayList<StGLTableItem>& aRow = myTable.changeValue(aRowIter);
        for(size_t aColIter = 0; aColIter < myColRights.size(); ++aColIter) {
            StGLTableItem& anItem = aRow.changeValue(aColIter);
            if(anItem.Item == NULL) {
                continue;
            }

            const int aBefore = aRowIter != 0
                              ? myRowBottoms.changeValue(aRowIter - 1)
                              : 0;
            size_t aBotRowId = aRowIter + anItem.RowSpan - 1;
            int&   aBottom   = myRowBottoms.changeValue(aBotRowId);
            aBottom = stMax(aBottom,
                            aBefore + anItem.Item->getRectPx().height()
                          + myMarginTop + myMarginBottom);
        }
    }

    // determine columns widths
    for(size_t aColIter = 0; aColIter < myColRights.size(); ++aColIter) {
        for(size_t aRowIter = 0; aRowIter < myRowBottoms.size(); ++aRowIter) {
            StGLTableItem& anItem = myTable.changeValue(aRowIter).changeValue(aColIter);
            if(anItem.Item == NULL) {
                continue;
            }

            const int aBefore = aColIter != 0
                              ? myColRights.changeValue(aColIter - 1)
                              : 0;
            size_t aRightColId = aColIter + anItem.ColSpan - 1;
            int&   aRight      = myColRights.changeValue(aRightColId);
            aRight = stMax(aRight,
                           aBefore + anItem.Item->getRectPx().width()
                         + myMarginLeft + myMarginRight);
        }
    }
    changeRectPx().right()  = getRectPx().left() + myColRights.getLast();
    changeRectPx().bottom() = getRectPx().top()  + myRowBottoms.getLast();

    // adjust table elements positions
    int aTop = 0;
    for(size_t aRowIter = 0; aRowIter < myRowBottoms.size(); aTop = myRowBottoms.getValue(aRowIter++)) {
        StArrayList<StGLTableItem>& aRow = myTable.changeValue(aRowIter);
        int aLeft = 0;
        for(size_t aColIter = 0; aColIter < myColRights.size(); aLeft = myColRights.getValue(aColIter++)) {
            StGLTableItem& anItem = aRow.changeValue(aColIter);
            if(anItem.Item == NULL) {
                continue;
            }

            //size_t aBotRowId   = aRowIter + anItem.RowSpan - 1;
            //size_t aRightColId = aColIter + anItem.ColSpan - 1;
            anItem.Item->changeRectPx().moveTopTo(aTop + myMarginTop);
            anItem.Item->changeRectPx().moveLeftTo(aLeft + myMarginLeft);
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
