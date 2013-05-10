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

#include <StCore/StKeysState.h>

StKeysState::StKeysState() {
    stMemZero(myKeys, sizeof(myKeys));
}

StKeysState::~StKeysState() {
    //
}

void StKeysState::reset() {
    StMutexAuto aLock(myLock);
    stMemZero(myKeys, sizeof(myKeys));
}

void StKeysState::keyDown(const StVirtKey theKey,
                          const double    theTime) {
    if(!myKeys[theKey]) {
        StMutexAuto aLock(myLock);
        myKeys[theKey]  = true;
        myTimes[theKey] = theTime;
    }
}

void StKeysState::keyUp(const StVirtKey theKey,
                        const double    theTime) {
    if(myKeys[theKey]) {
        StMutexAuto aLock(myLock);
        myKeys[theKey]  = false;
        myTimes[theKey] = theTime;
    }
}

bool StKeysState::isKeyDown(const StVirtKey theKey,
                            double&         theTime) const {
    StMutexAuto aLock(myLock);
    theTime = myTimes[theKey];
    const bool isPressed = myKeys[theKey];
    return isPressed;
}

double StKeysState::getKeyTime(const StVirtKey theKey) const {
    StMutexAuto aLock(myLock);
    const double aResult = myTimes[theKey];
    return aResult;
}
