/**
 * Copyright © 2011-2012 Kirill Gavrilov <kirill@sview.ru>
 *
 * StTests program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * StTests program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program.
 * If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __StTestGlBand_h_
#define __StTestGlBand_h_

#include "StTest.h"

class StGLContext;

/**
 * Tests CPU <-> GPU memory transfer speed.
 */
class ST_LOCAL StTestGlBand : public StTest {

        public:

    virtual void perform() ST_ATTR_OVERRIDE;

        private:

    void testTextureFill(StGLContext&  theCtx,
                         const GLsizei theFrameSizeX,
                         const GLsizei theFrameSizeY);

    void testTextureRead(StGLContext&  theCtx,
                         const GLsizei theFrameSizeX,
                         const GLsizei theFrameSizeY);

    void testFrameCopyRAM(const GLsizei theFrameSizeX,
                          const GLsizei theFrameSizeY);

};

#endif // __StTestGlBand_h_
