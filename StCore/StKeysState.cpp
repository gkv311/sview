/**
 * StCore, window system independent C++ toolkit for writing OpenGL applications.
 * Copyright © 2013-2017 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#include <StCore/StKeysState.h>

StKeysState::StKeysState() {
    stMemZero(myKeys,    sizeof(myKeys));
    stMemZero(myRegKeys, sizeof(myRegKeys));
}

StKeysState::~StKeysState() {
    //
}

void StKeysState::resetRegisteredKeys() {
    stMemZero(myRegKeys, sizeof(myRegKeys));
}

void StKeysState::registerKey(StVirtKey theKey) {
    myRegKeys[theKey] = true;
}

void StKeysState::unregisterKey(StVirtKey theKey) {
    myRegKeys[theKey] = false;
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
