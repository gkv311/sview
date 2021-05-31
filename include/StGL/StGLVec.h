/**
 * Copyright © 2010-2011 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#ifndef __StGLVec_h_
#define __StGLVec_h_

#include <StStrings/StLogger.h>
#include <StTemplates/StVec2.h>
#include <StTemplates/StVec3.h>
#include <StTemplates/StVec4.h>
#include <StTemplates/StQuaternion.h>
#include <StTemplates/StArrayList.h>

typedef StVec2<GLfloat> StGLVec2;
typedef StVec3<GLfloat> StGLVec3;
typedef StVec4<GLfloat> StGLVec4;
typedef StQuaternion<GLfloat> StGLQuaternion;

/**
 * POD structure for OpenGL rectangle in window coordinates (y coordinates is from bottom corner).
 * Intended to be used for Viewport and Scissor box definition.
 */
struct StGLBoxPx {

    GLint  x()      const { return v[0]; }
    GLint  y()      const { return v[1]; }
    GLint  width()  const { return v[2]; }
    GLint  height() const { return v[3]; }

    GLint& x()            { return v[0]; }
    GLint& y()            { return v[1]; }
    GLint& width()        { return v[2]; }
    GLint& height()       { return v[3]; }

    GLint v[4];
};

/**
 * Class defines the special vector with magnitude equal to 1.
 */
class StGLDir3 : public StGLVec3 {

        public:

    /**
     * Extract the direction from vector.
     * Zero-length vector is not applicable!
     */
    StGLDir3(const StGLVec3& theVec)
    : StGLVec3(theVec) {
        StGLVec3::normalize();
    }

    /**
     * Copy constructor.
     */
    StGLDir3(const StGLDir3& theDir)
    : StGLVec3(theDir) {
        //
    }

    /**
     * Initialize the direction by its components.
     */
    StGLDir3(const GLfloat theX, const GLfloat theY, const GLfloat theZ)
    : StGLVec3(theX, theY, theZ) {
        StGLVec3::normalize();
    }

    /**
     * Assignment operator.
     */
    const StGLDir3& operator=(const StGLDir3& theDir) {
        stMemCpy(this, &theDir, sizeof(StGLDir3));
        return *this;
    }

    /**
     * Assignment operator.
     */
    const StGLDir3& operator=(const StGLVec3& theVec) {
        stMemCpy(this, &theVec, sizeof(StGLVec3));
        StGLVec3::normalize();
        return *this;
    }

};

#endif //__StGLVec_h_
