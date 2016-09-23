/**
 * StOutPageFlip, class providing stereoscopic output for Shutter Glasses displays using StCore toolkit.
 * Copyright © 2009-2016 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#ifndef __StQuadBufferCheck_h_
#define __StQuadBufferCheck_h_

#include <stTypes.h>

class StQuadBufferCheck {

        public:

    /**
     * Test QB support within thin thread.
     */
    ST_LOCAL static bool testQuadBufferSupport();

};

#endif //__StQuadBufferCheck_h_
