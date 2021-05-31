/**
 * StOutPageFlip, class providing stereoscopic output for Shutter Glasses displays using StCore toolkit.
 * Copyright © 2010-2012 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#ifndef __StGLControlED_h_
#define __StGLControlED_h_

#include "StGLDeviceControl.h"

#include <StThreads/StTimer.h>

class StGLControlED : public StGLDeviceControl {

        public:

    ST_LOCAL virtual int getSizeY() const ST_ATTR_OVERRIDE {
        return 10;
    }

    ST_LOCAL virtual int getSlaveId() const ST_ATTR_OVERRIDE {
        return SLAVE_HLINE_TOP;
    }

    ST_LOCAL StGLControlED();
    ST_LOCAL virtual ~StGLControlED();

    ST_LOCAL virtual void release(StGLContext& theCtx) ST_ATTR_OVERRIDE;

    ST_LOCAL virtual bool isActive() const ST_ATTR_OVERRIDE {
        return myTimerCode.isOn() || myTimerBlack.isOn();
    }

    ST_LOCAL virtual void setMode(int mode) ST_ATTR_OVERRIDE {
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

    ST_LOCAL virtual double quitMS() ST_ATTR_OVERRIDE {
        return 4.0 * DELAY_MS;
    }

    ST_LOCAL virtual bool stglInit(StGLContext& theCtx) ST_ATTR_OVERRIDE;
    ST_LOCAL virtual void stglDraw(StGLContext& theCtx,
                                   unsigned int theView,
                                   const int    theWinWidth,
                                   const int    theWinHeight) ST_ATTR_OVERRIDE;

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
