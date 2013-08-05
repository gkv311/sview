/**
 * Copyright © 2013 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#include <StGLWidgets/StGLRangeFieldFloat32.h>
#include <StGLWidgets/StGLButton.h>

namespace {
    static const StString CLASS_NAME("StGLRangeFieldFloat32");
};

StGLRangeFieldFloat32::StGLRangeFieldFloat32(StGLWidget* theParent,
                                             const StHandle<StFloat32Param>& theTrackedValue,
                                             const int theLeft, const int theTop,
                                             const StGLCorner theCorner)
: StGLWidget(theParent, theLeft, theTop, theCorner),
  myTrackValue(theTrackedValue),
  myValueText(NULL),
  myFormat(stCString("%+01.3f")) {
    myTrackValue->signals.onChanged += stSlot(this, &StGLRangeFieldFloat32::onValueChange);
}

StGLRangeFieldFloat32::~StGLRangeFieldFloat32() {
    myTrackValue->signals.onChanged -= stSlot(this, &StGLRangeFieldFloat32::onValueChange);
}

const StString& StGLRangeFieldFloat32::getClassName() {
    return CLASS_NAME;
}

bool StGLRangeFieldFloat32::stglInit() {
    if(myValueText != NULL) {
        return true;
    }

    myValueText = new StGLTextArea(this, 0, 0, StGLCorner(ST_VCORNER_CENTER, ST_HCORNER_LEFT), -1, 10);
    onValueChange(0.0f);
    myValueText->setTextColor(StGLVec3(1.0f, 1.0f, 1.0f));
    myValueText->setVisibility(true, true);
    if(!myValueText->stglInitAutoHeightWidth()) {
        delete myValueText; myValueText = NULL;
        return false;
    }

    myValueText->changeRectPx().right() += 10;
    myValueText->setTextWidth(myValueText->getRectPx().width());
    myValueText->setupAlignment(StGLTextFormatter::ST_ALIGN_X_RIGHT, StGLTextFormatter::ST_ALIGN_Y_TOP);
    onValueChange(myTrackValue->getValue());
    const GLint aHeight = myValueText->getRectPx().height();

    StGLButton* aButDec = new StGLButton(this, 0, 0, "-");
    aButDec->setCorner(StGLCorner(ST_VCORNER_CENTER, ST_HCORNER_LEFT));
    aButDec->setHeight(aHeight);
    aButDec->setWidth(15);
    aButDec->setVisibility(true, true);
    aButDec->signals.onBtnClick += stSlot(this, &StGLRangeFieldFloat32::doDecrement);

    myValueText->changeRectPx().moveLeftTo(aButDec->getRectPx().right() - 5);

    StGLButton* aButInc = new StGLButton(this, myValueText->getRectPx().right() + 5, 0, "+");
    aButInc->setCorner(StGLCorner(ST_VCORNER_CENTER, ST_HCORNER_LEFT));
    aButInc->setHeight(aHeight);
    aButInc->setWidth(15);
    aButInc->setVisibility(true, true);
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
        } else if(theValue > 0.0f) {
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
