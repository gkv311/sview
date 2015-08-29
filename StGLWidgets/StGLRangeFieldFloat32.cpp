/**
 * Copyright © 2013-2015 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#include <StGLWidgets/StGLRangeFieldFloat32.h>
#include <StGLWidgets/StGLRootWidget.h>
#include <StGLWidgets/StGLButton.h>

StGLRangeFieldFloat32::StGLRangeFieldFloat32(StGLWidget* theParent,
                                             const StHandle<StFloat32Param>& theTrackedValue,
                                             const int theLeft, const int theTop,
                                             const StGLCorner theCorner)
: StGLWidget(theParent, theLeft, theTop, theCorner),
  myTrackValue(theTrackedValue),
  myValueText(NULL),
  myFormat(stCString("%+01.3f")) {
    myTrackValue->signals.onChanged   += stSlot(this, &StGLRangeFieldFloat32::onValueChange);
    StGLWidget::signals.onMouseUnclick = stSlot(this, &StGLRangeFieldFloat32::doMouseUnclick);
}

StGLRangeFieldFloat32::~StGLRangeFieldFloat32() {
    myTrackValue->signals.onChanged -= stSlot(this, &StGLRangeFieldFloat32::onValueChange);
}

bool StGLRangeFieldFloat32::stglInit() {
    if(myValueText != NULL) {
        return true;
    }

    myValueText = new StGLTextArea(this, 0, 0, StGLCorner(ST_VCORNER_CENTER, ST_HCORNER_LEFT), -myRoot->scale(1), myRoot->scale(10));
    float aLongVal = 0.0f;
    if(myTrackValue->hasMaxValue()) {
        aLongVal = myTrackValue->getMaxValue();
    }
    if(myTrackValue->hasMinValue()
    && std::abs(myTrackValue->getMinValue()) >= aLongVal) {
        aLongVal = myTrackValue->getMinValue();
    }
    onValueChange(aLongVal);
    myValueText->setTextColor(StGLVec3(1.0f, 1.0f, 1.0f));
    if(!myValueText->stglInitAutoHeightWidth()) {
        delete myValueText; myValueText = NULL;
        return false;
    }

    myValueText->changeRectPx().right() += myRoot->scale(10);
    myValueText->setTextWidth(myValueText->getRectPx().width());
    myValueText->setupAlignment(StGLTextFormatter::ST_ALIGN_X_RIGHT, StGLTextFormatter::ST_ALIGN_Y_TOP);
    onValueChange(myTrackValue->getValue());
    const GLint aHeight = myValueText->getRectPx().height();

    StGLButton* aButDec = new StGLButton(this, 0, 0, "-");
    aButDec->setCorner(StGLCorner(ST_VCORNER_CENTER, ST_HCORNER_LEFT));
    aButDec->setHeight(aHeight);
    aButDec->setWidth(myRoot->scale(15));
    aButDec->signals.onBtnClick += stSlot(this, &StGLRangeFieldFloat32::doDecrement);

    myValueText->changeRectPx().moveLeftTo(aButDec->getRectPx().right() - myRoot->scale(5));

    StGLButton* aButInc = new StGLButton(this, myValueText->getRectPx().right() + myRoot->scale(5), 0, "+");
    aButInc->setCorner(StGLCorner(ST_VCORNER_CENTER, ST_HCORNER_LEFT));
    aButInc->setHeight(aHeight);
    aButInc->setWidth(myRoot->scale(15));
    aButInc->signals.onBtnClick += stSlot(this, &StGLRangeFieldFloat32::doIncrement);

    changeRectPx().right()  = getRectPx().left() + aButInc->getRectPx().right();
    changeRectPx().bottom() = getRectPx().top()  + aHeight;

    return StGLWidget::stglInit();
}

void StGLRangeFieldFloat32::onValueChange(const float theValue) {
    if(myValueText != NULL) {
        char aBuff[128];
        stsprintf(aBuff, 128, myFormat.toCString(), theValue);
        myValueText->setText(aBuff);
        if(myTrackValue->isDefaultValue()) {
            myValueText->setTextColor(myColors[FieldColor_Default]);
        } else if(theValue > myTrackValue->getDefValue()) {
            myValueText->setTextColor(myColors[FieldColor_Positive]);
        } else {
            myValueText->setTextColor(myColors[FieldColor_Negative]);
        }
    }
}

void StGLRangeFieldFloat32::doResetValue(const size_t ) {
    myTrackValue->reset();
}

void StGLRangeFieldFloat32::doDecrement(const size_t ) {
    myTrackValue->decrement();
}

void StGLRangeFieldFloat32::doIncrement(const size_t ) {
    myTrackValue->increment();
}

void StGLRangeFieldFloat32::doMouseUnclick(const int theBtnId) {
    if(theBtnId == ST_MOUSE_SCROLL_V_UP) {
        myTrackValue->increment();
    } else if(theBtnId == ST_MOUSE_SCROLL_V_DOWN) {
        myTrackValue->decrement();
    }
}
