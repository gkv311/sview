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

#ifndef __StGLColoredLine_h_
#define __StGLColoredLine_h_

#include "StGLDeviceControl.h"

class ST_LOCAL StGLColoredLine : public StGLDeviceControl {

        public:

    virtual int getSizeY() const {
        return 1;
    }

    virtual int getSlaveId() const {
        return SLAVE_HLINE_BOTTOM;
    }

    void setWhiteColor() {
        myLineColor.r() = 1.0f; myLineColor.g() = 1.0f; myLineColor.b() = 1.0f; myLineColor.a() = 1.0f;
    }

    void setBlueColor() {
        myLineColor.r() = 0.0f; myLineColor.g() = 0.0f; myLineColor.b() = 1.0f; myLineColor.a() = 1.0f;
    }

    StGLColoredLine();
    virtual ~StGLColoredLine();
    virtual void release(StGLContext& theCtx);
    virtual bool stglInit(StGLContext& theCtx);
    virtual void stglDraw(StGLContext& theCtx,
                          unsigned int theView,
                          const int    theWinWidth,
                          const int );

        private:

    class StColoredLineProgram;
    StColoredLineProgram* myProgram;
    StGLVec4              myLineColor;

};

#endif //__StGLColoredLine_h_
