/**
 * StCore, window system independent C++ toolkit for writing OpenGL applications.
 * Copyright © 2013-2017 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#ifndef __StKeysState_h_
#define __StKeysState_h_

#include <StThreads/StMutex.h>

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
     * Reset map of registered keys.
     */
    ST_CPPEXPORT void resetRegisteredKeys();

    /**
     * Return map of registered keys.
     */
    ST_LOCAL const bool* getRegisteredKeys() const { return myRegKeys; }

    /**
     * Register key.
     */
    ST_CPPEXPORT void registerKey(StVirtKey theKey);

    /**
     * Unregister key.
     */
    ST_CPPEXPORT void unregisterKey(StVirtKey theKey);

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
     * @param theKey  Virtual key code to check
     * @param theTime Time when key was pressed or released
     * @return true if key is pressed
     */
    ST_CPPEXPORT bool isKeyDown(const StVirtKey theKey,
                                double&         theTime) const;

    /**
     * @param theKey Virtual key code to check
     * @return timestamp of last change
     */
    ST_CPPEXPORT double getKeyTime(const StVirtKey theKey) const;

    /**
     * @return array of boolean flags
     */
    ST_LOCAL const bool* getMap() const {
        return myKeys;
    }

        private:

    mutable StMutex myLock;              //!< mutex for thread-safe access
    double          myTimes  [ST_VK_NB]; //!< time when key was pressed or released
    bool            myKeys   [ST_VK_NB]; //!< virtual keys pressed state
    bool            myRegKeys[ST_VK_NB]; //!< registered keys (used by some hot-keys)

};

#endif // __StKeysState_h_
