/**
 * Copyright © 2011 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#ifndef __StGLSaturationMatrix_h_
#define __StGLSaturationMatrix_h_

#include "StGLMatrix.h"

/**
 * Provide classic saturation matrix for color processing.
 */
class StGLSaturationMatrix : public StGLMatrix {

        private:

    GLfloat mySaturation;

        public:

    /**
     * Create an identity matrix (saturation 1.0).
     */
    StGLSaturationMatrix()
    : StGLMatrix(),
      mySaturation(1.0f) {
        //
    }

    /**
     * Create saturation matrix.
     */
    StGLSaturationMatrix(const GLfloat theSatur)
    : StGLMatrix(),
      mySaturation(theSatur) {
        setSaturation(theSatur);
    }

    /**
     * Returns true if saturation value is 1.0
     */
    bool isIdentity() const {
        return stAreEqual(mySaturation, 1.0f, 0.0001f);
    }

    /**
     * Get the saturation value used on matrix creation.
     */
    GLfloat getSaturation() const {
        return mySaturation;
    }

    /**
     * Setup saturation matrix (current state will be reseted).
     *  1.0 - identity matrix;
     *  0.0 - convert to gray scale matrix;
     *  0.0..1.0 - interpolated;
     * -1.0 - complement the colors (special usage).
     */
    void setSaturation(const GLfloat theSatur) {
        static const StGLVec3 LUMEN_VEC(0.3086f, 0.6094f, 0.0820f);

        mySaturation = theSatur;
        initIdentity();
        changeValue(0, 0) = (1.0f - theSatur) * LUMEN_VEC.r() + theSatur;
        changeValue(1, 0) = (1.0f - theSatur) * LUMEN_VEC.r();
        changeValue(2, 0) = (1.0f - theSatur) * LUMEN_VEC.r();
        changeValue(0, 1) = (1.0f - theSatur) * LUMEN_VEC.g();
        changeValue(1, 1) = (1.0f - theSatur) * LUMEN_VEC.g() + theSatur;
        changeValue(2, 1) = (1.0f - theSatur) * LUMEN_VEC.g();
        changeValue(0, 2) = (1.0f - theSatur) * LUMEN_VEC.b();
        changeValue(1, 2) = (1.0f - theSatur) * LUMEN_VEC.b();
        changeValue(2, 2) = (1.0f - theSatur) * LUMEN_VEC.b() + theSatur;
    }

};

#endif //__StGLSaturationMatrix_h_
