/**
 * Copyright © 2016 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#ifndef __StGLDeviceCaps_h_
#define __StGLDeviceCaps_h_

#include <stTypes.h>

/**
 * Structure defining OpenGL device capabilities
 * which can be considered within data preparation.
 */
struct StGLDeviceCaps {
    /**
     * Texture size limits.
     */
    int maxTexDim;

    /**
     * Device support data transferring with row unpack size specified.
     */
    bool hasUnpack;

    /**
     * Empty constructor.
     */
    ST_LOCAL StGLDeviceCaps() : maxTexDim(0), hasUnpack(true) {}
};

#endif // __StGLDeviceCaps_h_
