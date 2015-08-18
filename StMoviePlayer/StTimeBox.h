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
      myProgressSec(0.0),
      myDurationSec(0.0),
      myToShowElapsed(true),
      myIsOverlay(false) {
        myTextArea = new StGLTextArea(this, 0, 0, StGLCorner(ST_VCORNER_CENTER, ST_HCORNER_CENTER),
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

    virtual bool stglInit() {
        bool isBtnInit = StGLTextureButton::stglInit();
        myTextArea->changeRectPx().right()  = getRectPx().width();
        myTextArea->changeRectPx().bottom() = getRectPx().height();
        return isBtnInit && myTextArea->stglInit();
    }

    virtual void stglDraw(unsigned int theView) {
        if(myToShowElapsed) {
            myTextArea->setText(StFormatTime::formatSeconds(myProgressSec) + " / "
                              + StFormatTime::formatSeconds(myDurationSec));
        } else {
            myTextArea->setText(StFormatTime::formatSeconds(myProgressSec - myDurationSec));
        }
        StGLTextureButton::stglDraw(theView);
        myTextArea->stglDraw(theView);
    }

    virtual bool tryClick(const StPointD_t& theCursor,
                          const int&        theMouseBtn,
                          bool&             theIsItemClicked) {
        return myIsOverlay
             ? false
             : StGLTextureButton::tryClick(theCursor, theMouseBtn, theIsItemClicked);
    }

    virtual bool tryUnClick(const StPointD_t& theCursor,
                            const int&        theMouseBtn,
                            bool&             theIsItemUnclicked) {
        return myIsOverlay
             ? false
             : StGLTextureButton::tryUnClick(theCursor, theMouseBtn, theIsItemUnclicked);
    }

    virtual void setVisibility(bool isVisible, bool isForce) {
        StGLTextureButton::setVisibility(isVisible, isForce);
        myTextArea->setVisibility(isVisible, isForce);
    }

    void setTime(const double theProgressSec,
                 const double theDurationSec) {
        myProgressSec = theProgressSec;
        myDurationSec = theDurationSec;
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
