/**
 * Copyright © 2009-2012 Kirill Gavrilov <kirill@sview.ru>
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
class ST_LOCAL StOutIZ3DShaders : public StGLResource {

        public:

    enum {
        IZ3D_TABLE_OLD = 0,
        IZ3D_TABLE_NEW = 1,
        IZ3D_CLASSIC = 2,
    };

    StOutIZ3DShaders();

    virtual ~StOutIZ3DShaders();

    virtual void release(StGLContext& theCtx);

    const int getMode() const {
        return myMode;
    }

    void setMode(const int theMode);
    virtual bool init(StGLContext& theCtx);

    StGLStereoFrameBuffer::StGLStereoProgram* master() {
        return myBack;
    }

    StGLStereoFrameBuffer::StGLStereoProgram* slave() {
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
