/**
 * Copyright © 2011 Kirill Gavrilov <kirill@sview.ru>
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

#ifndef __StCoreInitializer_h_
#define __StCoreInitializer_h_

#include <StCore/StCore.h>

/**
 * Stupid wrapper to initialize/free StCore library.
 */
class StCoreInitializer {

        private:

    size_t myCounter;

        public:

    StCoreInitializer() : myCounter(0) {}
    ~StCoreInitializer() {
        while(free()) {}
    }

    bool init() {
        if(StCore::INIT() == STERROR_LIBNOERROR) {
            ++myCounter;
            return true;
        }
        return false;
    }

    bool free() {
        if(myCounter > 0) {
            --myCounter;
            StCore::FREE();
            return true;
        }
        return false;
    }

};

#endif // __StCoreInitializer_h_
