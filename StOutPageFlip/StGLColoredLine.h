/**
 * StOutPageFlip, class providing stereoscopic output for Shutter Glasses displays using StCore toolkit.
 * Copyright © 2010-2012 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#ifndef __StGLColoredLine_h_
#define __StGLColoredLine_h_

#include "StGLDeviceControl.h"

class StGLColoredLine : public StGLDeviceControl {

        public:

    ST_LOCAL virtual int getSizeY() const ST_ATTR_OVERRIDE {
        return 1;
    }

    ST_LOCAL virtual int getSlaveId() const ST_ATTR_OVERRIDE {
        return SLAVE_HLINE_BOTTOM;
    }

    ST_LOCAL void setWhiteColor() {
        myLineColor.r() = 1.0f; myLineColor.g() = 1.0f; myLineColor.b() = 1.0f; myLineColor.a() = 1.0f;
    }

    ST_LOCAL void setBlueColor() {
        myLineColor.r() = 0.0f; myLineColor.g() = 0.0f; myLineColor.b() = 1.0f; myLineColor.a() = 1.0f;
    }

    ST_LOCAL StGLColoredLine();
    ST_LOCAL virtual ~StGLColoredLine();
    ST_LOCAL virtual void release (StGLContext& theCtx) ST_ATTR_OVERRIDE;
    ST_LOCAL virtual bool stglInit(StGLContext& theCtx) ST_ATTR_OVERRIDE;
    ST_LOCAL virtual void stglDraw(StGLContext& theCtx,
                                   unsigned int theView,
                                   const int    theWinWidth,
                                   const int ) ST_ATTR_OVERRIDE;

        private:

    class StColoredLineProgram;
    StColoredLineProgram* myProgram;
    StGLVec4              myLineColor;

};

#endif //__StGLColoredLine_h_
