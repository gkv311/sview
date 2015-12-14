/**
 * Copyright © 2009-2015 Kirill Gavrilov <kirill@sview.ru>
 *
 * StMoviePlayer program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * StMoviePlayer program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program.
 * If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __StTimeBox_h_
#define __StTimeBox_h_

#include <StGLWidgets/StGLTextureButton.h>
#include <StGLWidgets/StGLTextArea.h>
#include <StStrings/StFormatTime.h>

class ST_LOCAL StTimeBox : public StGLTextureButton {

        public:

    /**
     * Default constructor.
     */
    StTimeBox(StGLWidget*       theParent,
              const int         theLeft,
              const int         theTop,
              const StGLCorner& theCorner,
              const StGLTextArea::FontSize theSize = StGLTextArea::SIZE_NORMAL)
    : StGLTextureButton(theParent, theLeft, theTop, theCorner),
      myProgressSec(-1.0),
      myDurationSec(-1.0),
      myToShowElapsed(true),
      myIsOverlay(false) {
        myMargins.left  = myRoot->scale(8);
        myMargins.right = myRoot->scale(8);
        myTextArea = new StGLTextArea(this, 0, 0, StGLCorner(ST_VCORNER_CENTER, ST_HCORNER_LEFT),
                                      myRoot->scale(32), myRoot->scale(32), theSize);
        myTextArea->setBorder(false);
        myTextArea->setTextColor(StGLVec3(1.0f, 1.0f, 1.0f));
        myTextArea->setupAlignment(StGLTextFormatter::ST_ALIGN_X_CENTER,
                                   StGLTextFormatter::ST_ALIGN_Y_CENTER);
        myTextArea->setDrawShadow(true);
    }

    void setSwitchOnClick(const bool theToSwitch) {
        if(theToSwitch) {
            StGLWidget::signals.onMouseUnclick += stSlot(this, &StTimeBox::doSwitchElapsed);
        } else {
            StGLWidget::signals.onMouseUnclick -= stSlot(this, &StTimeBox::doSwitchElapsed);
        }
    }

    void setOverlay(const bool theValue) {
        myIsOverlay = theValue;
    }

    StGLTextArea* getTextArea() {
        return myTextArea;
    }

    virtual bool stglInit() ST_ATTR_OVERRIDE {
        bool isBtnInit = StGLTextureButton::stglInit();
        myTextArea->changeRectPx().right()  = getRectPx().width();
        myTextArea->changeRectPx().bottom() = getRectPx().height();
        const bool isOk = isBtnInit && myTextArea->stglInit();
        myTextArea->setTextWidth(-1);
        return isOk;
    }

    virtual void stglDraw(unsigned int theView) ST_ATTR_OVERRIDE {
        StGLTextureButton::stglDraw(theView);
        myTextArea->stglDraw(theView);
        myIsResized = false;
    }

    virtual bool tryClick(const StClickEvent& theEvent,
                          bool&               theIsItemClicked) ST_ATTR_OVERRIDE {
        return myIsOverlay
             ? false
             : StGLTextureButton::tryClick(theEvent, theIsItemClicked);
    }

    virtual bool tryUnClick(const StClickEvent& theEvent,
                            bool&               theIsItemUnclicked) ST_ATTR_OVERRIDE {
        return myIsOverlay
             ? false
             : StGLTextureButton::tryUnClick(theEvent, theIsItemUnclicked);
    }

    void stglUpdateTime(const double theProgressSec,
                        const double theDurationSec) {
        if(std::abs(myDurationSec - theDurationSec) > 0.1
        && (theDurationSec > 0.1 || myDurationSec < 0.0)) {
            int aWidth  = 0;
            int aHeight = 0;
            myTextArea->computeTextWidth(StFormatTime::formatSeconds(theDurationSec) + " / "
                                       + StFormatTime::formatSeconds(theDurationSec),
                                        -1.0f, aWidth, aHeight);
            const int aWidthNew = aWidth + myMargins.left + myMargins.right;
            const int aWidthOld = getRectPx().width();
            const int aToler    = myRoot->scale(4);
            if(std::abs(aWidthNew - aWidthOld) > aToler) {
                myTextArea->changeRectPx().right() = myTextArea->getRectPx().left() + aWidthNew;
                myTextArea->setTextWidth(aWidthNew);
                changeRectPx().right() = getRectPx().left() + aWidthNew;
            }
        }

        myProgressSec = theProgressSec;
        myDurationSec = theDurationSec;
        if(myToShowElapsed) {
            myTextArea->setText(StFormatTime::formatSeconds(myProgressSec) + " / "
                              + StFormatTime::formatSeconds(myDurationSec));
        } else {
            myTextArea->setText(StFormatTime::formatSeconds(myProgressSec - myDurationSec));
        }
    }

        public: //! @name callback Slots

    void doSwitchElapsed(const int ) {
        myToShowElapsed = !myToShowElapsed;
    }

        private: //! @name private fields

    StGLTextArea* myTextArea;      //!< text widget drawn over texture
    double        myProgressSec;   //!< current progress in seconds
    double        myDurationSec;   //!< overall duration in seconds
    bool          myToShowElapsed; //!< to show elapsed or remaining time
    bool          myIsOverlay;     //!< do not handle clicking

};

#endif //__StTimeBox_h_
