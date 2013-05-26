/**
 * Copyright © 2009-2013 Kirill Gavrilov <kirill@sview.ru>
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

#ifndef __StQuadBufferCheck_h_
#define __StQuadBufferCheck_h_

#include <stTypes.h>

class StQuadBufferCheck {

        public:

    /**
     * Global flag should be initialized before!
     * @return true if OpenGL Quad Buffer is supported.
     */
    ST_LOCAL static bool isSupported();

    /**
     * Initialize global flag.
     */
    ST_LOCAL static void initAsync();

    /**
     * Test QB support within thin thread.
     */
    ST_LOCAL static bool testQuadBufferSupport();

};

#endif //__StQuadBufferCheck_h_
