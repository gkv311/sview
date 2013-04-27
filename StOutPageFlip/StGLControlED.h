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

class StGLControlED : public StGLDeviceControl {

        public:

    ST_LOCAL virtual int getSizeY() const {
        return 10;
    }

    ST_LOCAL virtual int getSlaveId() const {
        return SLAVE_HLINE_TOP;
    }

    ST_LOCAL StGLControlED();
    ST_LOCAL virtual ~StGLControlED();

    ST_LOCAL virtual void release(StGLContext& theCtx);

    ST_LOCAL virtual bool isActive() const {
        return myTimerCode.isOn() || myTimerBlack.isOn();
    }

    ST_LOCAL virtual void setMode(int mode) {
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

    ST_LOCAL virtual double quitMS() {
        return 4.0 * DELAY_MS;
    }

    ST_LOCAL virtual bool stglInit(StGLContext& theCtx);
    ST_LOCAL virtual void stglDraw(StGLContext& theCtx,
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
