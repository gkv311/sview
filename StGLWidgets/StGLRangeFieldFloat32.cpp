/**
 * StGLWidgets, small C++ toolkit for writing GUI using OpenGL.
 * Copyright © 2013-2017 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#include <StGLWidgets/StGLRangeFieldFloat32.h>

#include <StGLWidgets/StGLRootWidget.h>
#include <StGLWidgets/StGLButton.h>
#include <StCore/StEvent.h>

StGLRangeFieldFloat32::StGLRangeFieldFloat32(StGLWidget* theParent,
                                             const StHandle<StFloat32Param>& theTrackedValue,
                                             int theLeft, int theTop,
                                             StGLCorner theCorner,
                                             RangeStyle theStyle,
                                             int theMargin)
: StGLSeekBar(theParent, theTop, theMargin, theCorner),
  myTrackValue(theTrackedValue),
  myValueText(NULL),
  myFormat(stCString("%+01.3f")),
  myRangeStyle(theStyle) {
    myRectPx.left()  = theLeft;
    myRectPx.right() = theLeft + 32;

    if(theStyle == RangeStyle_Seekbar) {
        myColors[FieldColor_Default]  = StGLVec3(1.0f, 1.0f, 1.0f);
        myColors[FieldColor_Positive] = StGLVec3(1.0f, 1.0f, 1.0f);
        myColors[FieldColor_Negative] = StGLVec3(1.0f, 1.0f, 1.0f);

        signals.onSeekClick = stSlot(this, &StGLRangeFieldFloat32::doSeekClick);
    }

    myTrackValue->signals.onChanged += stSlot(this, &StGLRangeFieldFloat32::onValueChange);
}

StGLRangeFieldFloat32::~StGLRangeFieldFloat32() {
    myTrackValue->signals.onChanged -= stSlot(this, &StGLRangeFieldFloat32::onValueChange);
}

bool StGLRangeFieldFloat32::stglInit() {
    if(myValueText != NULL) {
        return true;
    }

    float aLongVal = 0.0f;
    if(myTrackValue->hasMaxValue()) {
        aLongVal = myTrackValue->getMaxValue();
    }
    if(myTrackValue->hasMinValue()
    && std::abs(myTrackValue->getMinValue()) >= aLongVal) {
        aLongVal = myTrackValue->getMinValue();
    }

    switch(myRangeStyle) {
        case RangeStyle_PlusMinus: {
            myValueText = new StGLTextArea(this, 0, 0, StGLCorner(ST_VCORNER_CENTER, ST_HCORNER_LEFT), -myRoot->scale(1), myRoot->scale(10));
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

            if(!StGLWidget::stglInit()) {
                return false;
            }
            break;
        }
        case RangeStyle_Seekbar: {
            myValueText = new StGLTextArea(this, 0, 0, StGLCorner(ST_VCORNER_TOP, ST_HCORNER_LEFT),
                                           myRectPx.width(), myRectPx.height(), StGLTextArea::SIZE_NORMAL);
            myValueText->setBorder(false);
            myValueText->setDrawShadow(true);

            onValueChange(aLongVal);
            myValueText->setTextColor(StGLVec3(1.0f, 1.0f, 1.0f));
            myValueText->setupAlignment(StGLTextFormatter::ST_ALIGN_X_CENTER, StGLTextFormatter::ST_ALIGN_Y_CENTER);
            if(!myValueText->stglInit()) {
                delete myValueText; myValueText = NULL;
                return false;
            }

            onValueChange(myTrackValue->getValue());

            if(!StGLSeekBar::stglInit()) {
                return false;
            }
            break;
        }
    }
    return true;
}

void StGLRangeFieldFloat32::stglResize() {
    if(myRangeStyle == RangeStyle_Seekbar) {
        StGLSeekBar::stglResize();
    } else {
        StGLWidget::stglResize();
    }
}

void StGLRangeFieldFloat32::stglDraw(unsigned int theView) {
    if(myRangeStyle == RangeStyle_Seekbar) {
        StGLSeekBar::stglDraw(theView);
    } else {
        StGLWidget::stglDraw(theView);
    }
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
    setProgress(myTrackValue->getNormalizedValue());
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

bool StGLRangeFieldFloat32::doScroll(const StScrollEvent& theEvent) {
    if(theEvent.StepsY >= 1) {
        myTrackValue->increment();
    } else if(theEvent.StepsY <= -1) {
        myTrackValue->decrement();
    }
    return true;
}

void StGLRangeFieldFloat32::doSeekClick(const int    theMouseBtn,
                                        const double theValue) {
    if(theMouseBtn == ST_MOUSE_LEFT) {
        myTrackValue->setNormalizedValue((float )theValue, true);
    }
}
