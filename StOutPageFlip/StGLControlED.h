/**
 * Copyright © 2010-2012 Kirill Gavrilov <kirill@sview.ru>
 *
 * StOutPageFlip library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * StOutPageFlip library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.
 * If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __StGLControlED_h_
#define __StGLControlED_h_

#include "StGLDeviceControl.h"

#include <StThreads/StTimer.h>

class ST_LOCAL StGLControlED : public StGLDeviceControl {

        public:

    virtual int getSizeY() const {
        return 10;
    }

    virtual int getSlaveId() const {
        return SLAVE_HLINE_TOP;
    }

    StGLControlED();
    virtual ~StGLControlED();

    virtual void release(StGLContext& theCtx);

    virtual bool isActive() const {
        return myTimerCode.isOn() || myTimerBlack.isOn();
    }

    virtual void setMode(int mode) {
        if(myTimerCode.isOn() && myTimerCode.getElapsedTimeInMilliSec() > DELAY_MS) {
            myTimerCode.stop();
            myTimerBlack.restart();
        } else if(myTimerBlack.isOn() && myTimerBlack.getElapsedTimeInMilliSec() > DELAY_MS) {
            myTimerBlack.stop();
        }
        if(!myTimerCode.isOn() && !myTimerBlack.isOn() && (mode != StGLDeviceControl::getMode())) {
            StGLDeviceControl::setMode(mode);
            myTimerCode.restart();
        }
    }

    virtual double quitMS() {
        return 4.0 * DELAY_MS;
    }

    virtual bool stglInit(StGLContext& theCtx);
    virtual void stglDraw(StGLContext& theCtx,
                          unsigned int theView,
                          const int    theWinWidth,
                          const int    theWinHeight);

        private:

    static const double DELAY_MS; // 500

    class StEDProgram;
    StEDProgram* myProgramOn;
    StEDProgram* myProgramOff;
    StEDProgram* myProgramBlack;

    StTimer      myTimerCode;  // delays timers
    StTimer      myTimerBlack; //

};

#endif //__StGLControlED_h_
