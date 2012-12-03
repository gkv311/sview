/**
 * For more information and updates: http://www.firstpr.com.au/dsp/rand31/
 * Copyright © 2005 Robin Whittle <rw@firstpr.com.au>
 * Copyright © 2011 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#include "stTypes.h"

/**
 * 31 bit Pseudo Random Number Generator based on Park Miller
 */
class StMinGen {

        private:

    uint32_t mySeed31; //!< the sole item of state - a 32 bit integer

        public:

    /**
     * Default constructor.
     */
    StMinGen();

    /**
     * Set seed with a 31 bit unsigned integer between
     * 1 and 0x7FFFFFFE inclusive. Don't use 0!
     */
    void setSeed(uint32_t theSeed) {
        mySeed31 = (theSeed == 0) ? 1 : theSeed;
    }

    /**
     * Provides the next pseudorandom integer number (31 bits).
     */
    uint32_t nextInt();

    /**
     * Return next pseudo-random value as a floating point value.
     */
    double next();

};
