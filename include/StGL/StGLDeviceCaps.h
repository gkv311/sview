/**
 * Copyright © 2016-2019 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#ifndef __StGLDeviceCaps_h_
#define __StGLDeviceCaps_h_

#include <StImage/StImagePlane.h>

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
     * GL_PACK_ROW_LENGTH / GL_UNPACK_ROW_LENGTH can be used - OpenGL ES 3.0+ or any desktop.
     */
    bool hasUnpack;

    /**
     * Return TRUE if image format can be uploaded into OpenGL texture.
     */
    bool isSupportedFormat(StImagePlane::ImgFormat theFormat) const { return mySupportedFormats[theFormat]; }

    /**
     * Set if image format can be uploaded into OpenGL texture.
     */
    void setSupportedFormat(StImagePlane::ImgFormat theFormat, bool theIsSupported) { mySupportedFormats[theFormat] = theIsSupported; }

    /**
     * Empty constructor.
     */
    ST_LOCAL StGLDeviceCaps()
    : maxTexDim(0), hasUnpack(true) {
        stMemZero(mySupportedFormats, sizeof(mySupportedFormats));
    }

        private:

    bool mySupportedFormats[StImagePlane::ImgNB];

};

#endif // __StGLDeviceCaps_h_
