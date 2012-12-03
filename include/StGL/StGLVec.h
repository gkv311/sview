/**
 * Copyright © 2010-2011 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#ifndef __StGLVec_h_
#define __StGLVec_h_

#include <StStrings/StLogger.h>
#include <StTemplates/StVec2.h>
#include <StTemplates/StVec3.h>
#include <StTemplates/StVec4.h>
#include <StTemplates/StArrayList.h>

typedef StVec2<GLfloat> StGLVec2;
typedef StVec3<GLfloat> StGLVec3;
typedef StVec4<GLfloat> StGLVec4;

/**
 * Class defines the special vector with magnitude equal to 1.
 */
class ST_LOCAL StGLDir3 : public StGLVec3 {

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
