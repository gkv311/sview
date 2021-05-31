/**
 * StGLWidgets, small C++ toolkit for writing GUI using OpenGL.
 * Copyright © 2014-2017 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#include <StGLWidgets/StGLTable.h>

#include <StGLWidgets/StGLCombobox.h>
#include <StGLWidgets/StGLCheckbox.h>
#include <StGLWidgets/StGLMenuItem.h>
#include <StGLWidgets/StGLRangeFieldFloat32.h>
#include <StGLWidgets/StGLRootWidget.h>

#include <StGL/StGLContext.h>
#include <StGLCore/StGLCore20.h>

#include <StStrings/StDictionary.h>
#include <StSettings/StEnumParam.h>
#include <StSettings/StFloat32Param.h>

#include <stAssert.h>

StGLTableItem::StGLTableItem(StGLTable* theParent)
: StGLWidget(theParent,
             theParent->getItemMargins().left, theParent->getItemMargins().top,
             StGLCorner(ST_VCORNER_TOP, ST_HCORNER_LEFT),
             theParent->getRoot()->scale(32),
             theParent->getRoot()->scale(32)),
  myColSpan(1),
  myRowSpan(1) {
    //
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
  myIsInitialized(false) {
    myItemMargins.left   = myRoot->scale(5);
    myItemMargins.right  = myRoot->scale(5);
    myItemMargins.top    = myRoot->scale(2);
    myItemMargins.bottom = myRoot->scale(2);
}

StGLTable::~StGLTable() {
    //
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
    const int aCol1MaxWidth = theCol1MaxWidth - (myItemMargins.left + myItemMargins.right);
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
    int aCol2MaxWidth = theMaxWidth - aCol1Width - 2 * (myItemMargins.left + myItemMargins.right);
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
        aText->stglInitAutoHeightWidth(aCol2MaxWidth);
    }

    updateLayout();
}

void StGLTable::fillFromParams(const StParamsList& theParams,
                               const StGLVec3&     theTextColor,
                               const int           theMaxWidth,
                               const int           theRowId,
                               const int           theColId) {
    ST_ASSERT_SLIP(theRowId >= 0 && theColId >= 0,
                   "StGLTable::fillFromParams() out of range",
                   return);
    const int aRowsNb = theRowId + (int )theParams.size();
    const int aColsNb = theColId + 2;
    if(aRowsNb > (int )myRowBottoms.size()
    || aColsNb > (int )myColRights.size()) {
        setupTable(aRowsNb, aColsNb);
    }

    // fill second column with values
    int aCol2Width = 0;
    StHandle<StBoolParamNamed> aBool;
    StHandle<StEnumParam>      anEnum;
    StHandle<StFloat32Param>   aFloat32;
    const int anIconMargin = myRoot->scale(8);
    const int anIconWidth  = anIconMargin * 2 + myRoot->scale(16);
    StMarginsI aCheckMargins;
    aCheckMargins.left   = anIconMargin;
    aCheckMargins.right  = anIconMargin + myRoot->scale(32);
    aCheckMargins.top    = anIconMargin;
    aCheckMargins.bottom = anIconMargin;
    for(size_t anIter = 0; anIter < theParams.size(); ++anIter) {
        const StHandle<StParamBase>& aParam = theParams[anIter];
        StGLTableItem&               anItem = changeElement(theRowId + (int )anIter, theColId + 1);
        anItem.setRowSpan(1);
        anItem.setColSpan(1);
        if(aBool.downcastFrom(aParam)) {
            StGLCheckbox* aCheckBox = new StGLCheckbox(&anItem, aBool,
                                                       0, 0, StGLCorner(ST_VCORNER_CENTER, ST_HCORNER_LEFT));
            aCheckBox->changeRectPx().right()  = aCheckBox->getRectPx().left() + anIconWidth;
            aCheckBox->changeRectPx().bottom() = aCheckBox->getRectPx().top()  + anIconWidth;
            aCheckBox->changeMargins() = aCheckMargins;
            aCol2Width = stMax(aCol2Width, aCheckBox->getRectPx().width());
        } else if(anEnum.downcastFrom(aParam)) {
            StGLCombobox* aButton = new StGLCombobox(&anItem, 0, 0, anEnum);
            aButton->setCorner(StGLCorner(ST_VCORNER_CENTER, ST_HCORNER_CENTER));
            aButton->setHeight(myRoot->scale(24));

            aCol2Width = stMax(aCol2Width, aButton->getRectPx().width());
            const StArrayList<StString>& aValues = anEnum->getValues();
            for(size_t aValIter = 0; aValIter < aValues.size(); ++aValIter) {
                aCol2Width = stMax(aCol2Width, aButton->computeWidth(aValues[aValIter]));
            }
        } else if(aFloat32.downcastFrom(aParam)) {
            StGLRangeFieldFloat32* aRange = new StGLRangeFieldFloat32(&anItem, aFloat32,
                                                                      0, 0, StGLCorner(ST_VCORNER_CENTER, ST_HCORNER_RIGHT),
                                                                      StGLRangeFieldFloat32::RangeStyle_Seekbar, myRoot->scale(18));
            aRange->changeRectPx().right() = myRoot->scale(50);
            aRange->changeMargins().left   = myRoot->scale(8);
            aRange->changeMargins().right  = myRoot->scale(8);
            aRange->setCorner(StGLCorner(ST_VCORNER_TOP, ST_HCORNER_LEFT));
            if(!aFloat32->getFormat().isEmpty()) {
                aRange->setFormat(aFloat32->getFormat());
            }
            aCol2Width = stMax(aCol2Width, aRange->getRectPx().width());
        } else {
            // skip
        }
    }

    // adjust width of all elements in second column
    const int aCol2MaxWidth = 2 * (theMaxWidth / 3);
    aCol2Width = stMin(aCol2Width, aCol2MaxWidth);
    for(size_t anIter = 0; anIter < theParams.size(); ++anIter) {
        StGLTableItem& anItem  = changeElement(theRowId + (int )anIter, theColId + 1);
        StGLWidget*    aWidget = anItem.getItem();
        if(aWidget == NULL) {
            continue;
        }

        if(StGLCombobox* aButton = dynamic_cast<StGLCombobox* >(anItem.getItem())) {
            anItem.getItem()->changeRectPx().right() = aWidget->getRectPx().left() + aCol2Width;
            aButton->setWidth(aCol2Width);
        } else if(dynamic_cast<StGLRangeFieldFloat32* >(anItem.getItem()) != NULL) {
            anItem.getItem()->changeRectPx().right() = aWidget->getRectPx().left() + aCol2Width;
        }
    }

    // fill first column with labels
    int aCol1MaxWidth = theMaxWidth - aCol2Width - 2 * (myItemMargins.left + myItemMargins.right);
    for(size_t anIter = 0; anIter < theParams.size(); ++anIter) {
        const StHandle<StParamBase>& aParam = theParams[anIter];
        StGLTableItem&               anItem = changeElement(theRowId + (int )anIter, theColId);
        anItem.setRowSpan(1);
        anItem.setColSpan(1);

        StString aLabelText;
        if(aBool.downcastFrom(aParam)) {
            aLabelText = aBool->getName();
        } else if(anEnum.downcastFrom(aParam)) {
            aLabelText = anEnum->getName();
        } else if(aFloat32.downcastFrom(aParam)) {
            aLabelText = aFloat32->getName();
        } else {
            // skip
        }

        StGLTextArea* aText = new StGLTextArea(&anItem, 0, 0, StGLCorner(ST_VCORNER_CENTER, ST_HCORNER_LEFT));
        aText->setupAlignment(StGLTextFormatter::ST_ALIGN_X_LEFT,
                              StGLTextFormatter::ST_ALIGN_Y_TOP);
        aText->setText(aLabelText);
        aText->setTextColor(theTextColor);
        aText->setupStyle(StFTFont::Style_Bold);
        aText->stglInitAutoHeightWidth(aCol1MaxWidth);
    }

    updateLayout();
}

void StGLTable::fillFromHotKeys(const std::map< int, StHandle<StAction> >&      theActions,
                                const StLangMap&                                theLangMap,
                                const StHandle< StSlot<void (const size_t )> >& theHKeySlot1,
                                const StHandle< StSlot<void (const size_t )> >& theHKeySlot2,
                                int       theMaxWidth,
                                const int theRowId,
                                const int theColId) {
    const StGLVec4 THE_COLOR_BACK   (0.0f, 0.0f, 0.0f, 0.0f);
    const StGLVec3 THE_COLOR_WHITE  (1.0f, 1.0f, 1.0f);
    const StGLVec4 THE_COLOR_HILIGHT(0.5f, 0.5f, 0.5f, 1.0f);

    if(theMaxWidth < 1) {
        theMaxWidth = getParent()->getRectPx().width();
    }

    // fill table
    int aCol2Width    = 0;
    int aCol1MaxWidth = theMaxWidth / 2 - myItemMargins.left - myItemMargins.right;
    const StGLCorner aCorner(ST_VCORNER_CENTER, ST_HCORNER_LEFT);
    size_t aRowIter = 0;
    for(std::map< int, StHandle<StAction> >::const_iterator anActionIter = theActions.begin();
        anActionIter != theActions.end(); ++anActionIter, ++aRowIter) {
        const StHandle<StAction>& anAction   = anActionIter->second;
        StGLTableItem&            anItemLab  = changeElement(theRowId + (int )aRowIter, theColId);
        StGLTableItem&            anItemKey1 = changeElement(theRowId + (int )aRowIter, theColId + 1);
        StGLTableItem&            anItemKey2 = changeElement(theRowId + (int )aRowIter, theColId + 2);
        anItemLab .setRowSpan(1);
        anItemLab .setColSpan(1);
        anItemKey1.setRowSpan(1);
        anItemKey1.setColSpan(1);
        anItemKey2.setRowSpan(1);
        anItemKey2.setColSpan(1);

        StGLTextArea* aTextLab = new StGLTextArea(&anItemLab, 0, 0, aCorner);
        aTextLab->setupAlignment(StGLTextFormatter::ST_ALIGN_X_LEFT,
                                 StGLTextFormatter::ST_ALIGN_Y_TOP);
        StString aDesc = theLangMap.getValue(anAction->getName());
        if(aDesc.isEmpty()) {
            aDesc = anAction->getName();
        }
        aTextLab->setText(aDesc);
        aTextLab->setTextColor(THE_COLOR_WHITE);
        aTextLab->setupStyle(StFTFont::Style_Bold);
        aTextLab->stglInitAutoHeightWidth(aCol1MaxWidth);

        StGLButton* aTextKey1 = new StGLButton(&anItemKey1, 0, 0, encodeHotKey(anAction->getHotKey1()));
        aTextKey1->getMenuItem()->setHilightText();
        aTextKey1->getMenuItem()->setHilightColor(THE_COLOR_HILIGHT);
        aTextKey1->getMenuItem()->setTextColor   (THE_COLOR_WHITE);
        aTextKey1->setColor(THE_COLOR_BACK);
        aTextKey1->setUserData(anActionIter->first);
        aTextKey1->signals.onBtnClick.connectExtra(theHKeySlot1);
        aCol2Width = stMax(aCol2Width, aTextKey1->getWidth());

        StGLButton* aTextKey2 = new StGLButton(&anItemKey2, 0, 0, encodeHotKey(anAction->getHotKey2()));
        aTextKey2->getMenuItem()->setHilightText();
        aTextKey2->getMenuItem()->setHilightColor(THE_COLOR_HILIGHT);
        aTextKey2->getMenuItem()->setTextColor   (THE_COLOR_WHITE);
        aTextKey2->setColor(THE_COLOR_BACK);
        aTextKey2->setUserData(anActionIter->first);
        aTextKey2->signals.onBtnClick.connectExtra(theHKeySlot2);
        aCol2Width = stMax(aCol2Width, aTextKey2->getWidth());
    }

    // adjust width of all elements
    const size_t aRowLast = theActions.size();
    for(aRowIter = 0; aRowIter < aRowLast; ++aRowIter) {
        StGLTableItem& anItemLab = changeElement(theRowId + (int )aRowIter, theColId);
        anItemLab.getItem()->changeRectPx().right() = anItemLab.getItem()->getRectPx().left() + aCol1MaxWidth;

        StGLTableItem& anItemKey1 = changeElement(theRowId + (int )aRowIter, theColId + 1);
        StGLTableItem& anItemKey2 = changeElement(theRowId + (int )aRowIter, theColId + 2);
        ((StGLButton* )anItemKey1.getItem())->setWidth(aCol2Width);
        ((StGLButton* )anItemKey2.getItem())->setWidth(aCol2Width);
    }
    updateLayout();
}

void StGLTable::updateHotKeys(const std::map< int, StHandle<StAction> >& theActions,
                              const int theRowId,
                              const int theColId) {
    const size_t aRowLast = theActions.size();
    for(size_t aRowIter = 0; aRowIter < aRowLast; ++aRowIter) {
        StGLTableItem& anItemKey1 = changeElement(theRowId + (int )aRowIter, theColId + 1);
        StGLTableItem& anItemKey2 = changeElement(theRowId + (int )aRowIter, theColId + 2);
        StGLButton* aKey1 = dynamic_cast<StGLButton* >(anItemKey1.getItem());
        StGLButton* aKey2 = dynamic_cast<StGLButton* >(anItemKey2.getItem());
        ST_ASSERT_SLIP(aKey1 != NULL && aKey2 != NULL,
                       "StGLTable::updateHotKeys() NULL button",
                       return);

        std::map< int, StHandle<StAction> >::const_iterator anAction = theActions.find((int )aKey1->getUserData());
        ST_ASSERT_SLIP(anAction != theActions.end(),
                       "StGLTable::updateHotKeys() NULL action",
                       return);

        aKey1->setLabel(encodeHotKey(anAction->second->getHotKey1()));
        aKey2->setLabel(encodeHotKey(anAction->second->getHotKey2()));
    }
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
                          + myItemMargins.top + myItemMargins.bottom);
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
                         + myItemMargins.left + myItemMargins.right);
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
            anItem->changeRectPx().top()    = aTop  + myItemMargins.top;
            anItem->changeRectPx().left()   = aLeft + myItemMargins.left;
            anItem->changeRectPx().bottom() = myRowBottoms.changeValue(aBotRowId)   - myItemMargins.bottom;
            anItem->changeRectPx().right()  = myColRights .changeValue(aRightColId) - myItemMargins.right;
        }
    }
}

void StGLTable::stglDraw(unsigned int theView) {
    if(!myIsInitialized || !isVisible()) {
        return;
    }

    if(myIsResized) {
        stglResize();
    }

    StGLWidget::stglDraw(theView);
}
