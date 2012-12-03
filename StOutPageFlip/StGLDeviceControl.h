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

#ifndef __StGLDeviceControl_h_
#define __StGLDeviceControl_h_

#include <StGL/StGLEnums.h>
#include <StGL/StGLVec.h>
#include <StGL/StGLVertexBuffer.h>

class ST_LOCAL StGLDeviceControl : public StGLResource {

        public:

    enum {
        OUT_UNDEFINED,
        OUT_MONO,
        OUT_STEREO,
    };

    enum {
        SLAVE_HLINE_BOTTOM,
        SLAVE_HTOP2PX,
        SLAVE_HLINE_TOP,
    };

    StGLDeviceControl()
    : myOutMode(OUT_UNDEFINED) {
        //
    }

    virtual ~StGLDeviceControl() {}

    virtual int getSlaveId() const = 0;

    virtual int getSizeY() const = 0;

    virtual bool isActive() const {
        return isStereo();
    }

    int getMode() const {
        return myOutMode;
    }

    virtual bool isStereo() const {
        return myOutMode == OUT_STEREO;
    }

    virtual void setMode(int theMode) {
        myOutMode = theMode;
    }

    virtual double quitMS() {
        setMode(OUT_UNDEFINED);
        return 0.0;
    }

    virtual bool stglInit(StGLContext& theCtx) = 0;
    virtual void stglDraw(StGLContext& theCtx,
                          unsigned int theView,
                          const int    theWinWidth,
                          const int    theWinHeight) = 0;

        protected:

    StGLVertexBuffer myVertexBuf;
    int              myOutMode;

};

#endif //__StGLDeviceControl_h_
