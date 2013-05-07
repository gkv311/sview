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

#ifndef __StTestMutex_h_
#define __StTestMutex_h_

#include "StTest.h"
#include <StThreads/StThread.h>

/**
 * Tests mutex object performance.
 */
class ST_LOCAL StTestMutex : public StTest {

        public:

    virtual void perform();

        private:

    /**
     * Lock usual mutex in loop.
     */
    static SV_THREAD_FUNCTION lockLoop(void* );

    /**
     * Lock slim mutex in loop.
     */
    static SV_THREAD_FUNCTION slimLockLoop(void* );

};

#endif // __StTestMutex_h_
