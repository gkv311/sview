/**
 * Copyright © 2009-2013 Kirill Gavrilov <kirill@sview.ru>
 *
 * StOutIZ3D library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * StOutIZ3D library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.
 * If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __StOutIZ3DShaders_h_
#define __StOutIZ3DShaders_h_

#include <StGLStereo/StGLStereoFrameBuffer.h>

/**
 * Helper class to access IZ3D shaders array.
 */
class StOutIZ3DShaders : public StGLResource {

        public:

    enum {
        IZ3D_TABLE_OLD = 0,
        IZ3D_TABLE_NEW = 1,
        IZ3D_CLASSIC = 2,
    };

    ST_LOCAL StOutIZ3DShaders();

    ST_LOCAL virtual ~StOutIZ3DShaders();

    ST_LOCAL virtual void release(StGLContext& theCtx);

    ST_LOCAL const int getMode() const {
        return myMode;
    }

    ST_LOCAL void doSetMode(const int32_t theValue) {
        setMode(theValue);
    }

    ST_LOCAL void setMode(const int theMode);
    ST_LOCAL virtual bool init(StGLContext& theCtx);

    ST_LOCAL StGLStereoFrameBuffer::StGLStereoProgram* master() {
        return myBack;
    }

    ST_LOCAL StGLStereoFrameBuffer::StGLStereoProgram* slave() {
        return myFront;
    }

        private:

    StGLStereoFrameBuffer::StGLStereoProgram* myBack;         //!< program for master window
    StGLStereoFrameBuffer::StGLStereoProgram* myFront;        //!< program for slave window

    StGLStereoFrameBuffer::StGLStereoProgram  myBackClassic;  //!< classic shaders - computations
    StGLStereoFrameBuffer::StGLStereoProgram  myFrontClassic;
    StGLStereoFrameBuffer::StGLStereoProgram  myBackTable;    //!< new shaders - values getted from optimized tables
    StGLStereoFrameBuffer::StGLStereoProgram  myFrontTable;

    int myMode; // switch old classes table / new glasses table / classic computations

};

#endif //__StOutIZ3DShaders_h_
