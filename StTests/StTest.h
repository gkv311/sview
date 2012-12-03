/**
 * Copyright © 2012 Kirill Gavrilov <kirill@sview.ru>
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

#ifndef __StTest_h_
#define __StTest_h_

#include <StThreads/StTimer.h>

/**
 * Interface for tests.
 */
class ST_LOCAL StTest {

        public:

    virtual void perform() = 0;

    virtual ~StTest() {}

        protected:

    StTimer myTimer;

};

#endif // __StTest_h_
