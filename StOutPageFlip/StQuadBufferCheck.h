/**
 * StOutPageFlip, class providing stereoscopic output for Shutter Glasses displays using StCore toolkit.
 * Copyright © 2009-2016 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
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
