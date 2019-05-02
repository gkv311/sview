/**
 * Copyright © 2019 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#ifndef __StGLUVCylinder_h_
#define __StGLUVCylinder_h_

#include "StGLMesh.h"

class StBndSphere;

/**
 * Class represents configurable UV cylinder with computed vertices, normales, texture coordinates.
 */
class StGLUVCylinder : public StGLMesh {

        public:

    /**
     * Defines the UV cylinder mesh with specified parameters.
     */
    ST_CPPEXPORT StGLUVCylinder(const StGLVec3& theCenter,
                                const GLfloat theHeight,
                                const GLfloat theRadius,
                                const int theRings);

    ST_CPPEXPORT virtual ~StGLUVCylinder();

    /**
     * Compute the mesh using current configuration.
     */
    ST_CPPEXPORT virtual bool computeMesh() ST_ATTR_OVERRIDE;

        private:

    StGLVec3 myCenter;
    GLfloat  myRadius;
    GLfloat  myHeight;
    int      myNbRings;

};

#endif //__StGLUVCylinder_h_
