/**
 * Copyright © 2011 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#ifndef __StGLBrightnessMatrix_h_
#define __StGLBrightnessMatrix_h_

#include "StGLMatrix.h"

/**
 * Provide simple brightness matrix for color processing.
 */
class StGLBrightnessMatrix : public StGLMatrix {

        private:

    GLfloat myBrightness;

        public:

    /**
     * Create an identity matrix (brightness 1.0).
     */
    StGLBrightnessMatrix()
    : StGLMatrix(),
      myBrightness(1.0f) {
        //
    }

    /**
     * Create the brightness matrix.
     */
    StGLBrightnessMatrix(const GLfloat theBright)
    : StGLMatrix(),
      myBrightness(theBright) {
        setBrightness(theBright);
    }

    /**
     * Returns true if brightness value is 1.0
     */
    bool isIdentity() const {
        return stAreEqual(myBrightness, 1.0f, 0.0001f);
    }

    /**
     * Get the brightness value used on matrix creation.
     */
    GLfloat getBrightness() const {
        return myBrightness;
    }

    /**
     * Setup brightness matrix (current state will be reseted).
     * 1.0 - identity matrix;
     * 0.0 - black matrix;
     */
    void setBrightness(const GLfloat theBright) {
        myBrightness = (theBright > 0.0f) ? theBright : 0.0f;
        initIdentity();
        changeValue(0, 0) = changeValue(1, 1) = changeValue(2, 2) = theBright;
    }

};

#endif //__StGLBrightnessMatrix_h_
