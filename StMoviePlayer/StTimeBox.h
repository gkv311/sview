/**
 * Copyright © 2009-2012 Kirill Gavrilov <kirill@sview.ru>
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
    StTimeBox(StGLWidget* theParent,
              const int   theLeft = 32,
              const int   theTop  = 32)
    : StGLTextureButton(          theParent, theLeft, theTop, StGLCorner(ST_VCORNER_TOP, ST_HCORNER_LEFT)),
      myTextArea(new StGLTextArea(theParent, theLeft, theTop, StGLCorner(ST_VCORNER_TOP, ST_HCORNER_LEFT), 32, 32)),
      myProgressSec(0.0),
      myDurationSec(0.0),
      myToShowElapsed(true) {
        myTextArea->setBorder(false);
        myTextArea->setTextColor(StGLVec3(1.0f, 1.0f, 1.0f));
        myTextArea->setupAlignment(StGLTextFormatter::ST_ALIGN_X_CENTER,
                                   StGLTextFormatter::ST_ALIGN_Y_CENTER);
        StGLWidget::signals.onMouseUnclick.connect(this, &StTimeBox::doSwitchElapsed);
    }

    virtual const StString& getClassName() {
        static const StString className("StTimeBox");
        return className;
    }

    virtual bool stglInit() {
        bool isBtnInit = StGLTextureButton::stglInit();
        myTextArea->changeRectPx() = StGLTextureButton::getRectPx();
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

    void setVisibility(bool isVisible) {
        StGLTextureButton::setVisibility(isVisible);
        myTextArea->setVisibility(isVisible);
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

};

#endif //__StTimeBox_h_
