/**
 * StGLWidgets, small C++ toolkit for writing GUI using OpenGL.
 * Copyright © 2011-2015 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#include <StGLWidgets/StGLSwitchTextured.h>

#include <StGLWidgets/StGLRadioButtonTextured.h>
#include <StCore/StEvent.h>

StGLSwitchTextured::StGLSwitchTextured(StGLWidget* theParent,
                                       const StHandle<StInt32Param>& theTrackedValue,
                                       const int theLeft, const int theTop,
                                       const StGLCorner theCorner)
: StGLWidget(theParent, theLeft, theTop, theCorner),
  myTrackValue(theTrackedValue),
  mySkipValues() {
    //
}

StGLSwitchTextured::~StGLSwitchTextured() {
    //
}

bool StGLSwitchTextured::stglInit() {
    bool isOK = StGLWidget::stglInit();
    StGLWidget* aChild = getChildren()->getStart();
    if(aChild != NULL) {
        // just upsacle to fit children
        changeRectPx().right()  = getRectPx().left() + aChild->getRectPx().width();
        changeRectPx().bottom() = getRectPx().top()  + aChild->getRectPx().height();
    }
    return isOK;
}

void StGLSwitchTextured::stglDraw(unsigned int theView) {
    if(!isVisible()) {
        return;
    }
    int32_t anActiveValue = myTrackValue->getValue();
    StGLRadioButtonTextured* aRadioBtn = NULL;
    for(StGLWidget* aChild = getChildren()->getStart(); aChild != NULL;) {
        /// TODO (Kirill Gavrilov#9) - adding children with another type is not allowed
        ///                            hovewer not protected thus this cast is not thread-safe!
        aRadioBtn = (StGLRadioButtonTextured* )aChild;
        aChild = aChild->getNext();

        // show only active item
        if(anActiveValue == aRadioBtn->getValueOn()) {
            aRadioBtn->stglDraw(theView);
            return;
        }
    }
    // show first item anyway
    StGLWidget* aChild = getChildren()->getStart();
    if(aChild != NULL) {
        aChild->stglDraw(theView);
    }
}


bool StGLSwitchTextured::tryClick(const StClickEvent& theEvent,
                                  bool&               theIsItemClicked) {
    if(!isVisible()) {
        return false; // nothing to see - nothing to click...
    }

    if(!theIsItemClicked && isPointIn(StPointD_t(theEvent.PointX, theEvent.PointY))) {
        setClicked(theEvent.Button, true);
        theIsItemClicked = true;
        return true;
    }
    return false;
}

bool StGLSwitchTextured::tryUnClick(const StClickEvent& theEvent,
                                    bool&               theIsItemUnclicked) {
    if(!isVisible()) {
        return false; // nothing to see - nothing to click...
    }
    bool isSelfClicked = isClicked(theEvent.Button)
                      && isPointIn(StPointD_t(theEvent.PointX, theEvent.PointY));
    setClicked(theEvent.Button, false);
    if(!theIsItemUnclicked && isSelfClicked) {
        theIsItemUnclicked = true;

        int32_t anActiveValue = myTrackValue->getValue();
        StGLRadioButtonTextured* aRadioBtn = NULL;
        for(StGLWidget* aChild = getChildren()->getStart(); aChild != NULL;) {
            /// TODO (Kirill Gavrilov#9) - adding children with another type is not allowed
            ///                            however not protected thus this cast is not thread-safe!
            aRadioBtn = (StGLRadioButtonTextured* )aChild;
            aChild = aChild->getNext();

            if(anActiveValue == aRadioBtn->getValueOn()) {
                // switch to next
                while(aChild != NULL) {
                    aRadioBtn = (StGLRadioButtonTextured* )aChild;
                    if(!mySkipValues.contains(aRadioBtn->getValueOn())) {
                        myTrackValue->setValue(aRadioBtn->getValueOn());
                        break;
                    }
                    aChild = aChild->getNext();
                }
                if(aChild == NULL) {
                    // switch to first
                    aRadioBtn = (StGLRadioButtonTextured* )getChildren()->getStart();
                    myTrackValue->setValue(aRadioBtn->getValueOn());
                }
                break;
            }
        }
        return true;
    }
    return false;
}

void StGLSwitchTextured::addItem(const int32_t   theValueOn,
                                 const StString& theTexturePath,
                                 const bool      theToSkip) {
    StGLRadioButtonTextured* aNewItem = new StGLRadioButtonTextured(this, myTrackValue, theValueOn, theTexturePath, 0, 0);
    aNewItem->changeMargins() = myMargins;
    if(theToSkip) {
        mySkipValues.add(theValueOn);
    }
}
