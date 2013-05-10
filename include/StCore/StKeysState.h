/**
 * Copyright © 2013 Kirill Gavrilov <kirill@sview.ru>
 *
 * StCore library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * StCore library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.
 * If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __StKeysState_h_
#define __StKeysState_h_

#include <stTypes.h>
#include "StVirtualKeys.h"

/**
 * This class reflects keyboard state.
 */
class StKeysState {

        public:

    /**
     * Empty constructor.
     */
    ST_CPPEXPORT StKeysState();

    /**
     * Destructor.
     */
    ST_CPPEXPORT ~StKeysState();

    /**
     * Release all pressed keys (window lost input focus etc.).
     */
    ST_CPPEXPORT void reset();

    /**
     * Press key.
     * @param theKey  Virtual key code to press
     * @param theTime Event timestamp
     */
    ST_CPPEXPORT void keyDown(const StVirtKey theKey,
                              const double    theTime);

    /**
     * Release key.
     * @param theKey  Virtual key code to release
     * @param theTime Event timestamp
     */
    ST_CPPEXPORT void keyUp(const StVirtKey theKey,
                            const double    theTime);

    /**
     * @param theKey Virtual key code to check
     * @return true if key is pressed
     */
    ST_LOCAL bool isKeyDown(const StVirtKey theKey) const {
        return myKeys[theKey];
    }

    /**
     * @param theKey Virtual key code to check
     * @return timestamp of last change
     */
    ST_LOCAL double getKeyTime(const StVirtKey theKey) const {
        return myTimes[theKey];
    }

    /**
     * @return array of boolean flags
     */
    ST_LOCAL const bool* getMap() const {
        return myKeys;
    }

        private:

    bool   myKeys[256];  //!< virtual keys pressed state
    double myTimes[256];

};

#endif // __StKeysState_h_
