/**
 * Copyright © 2010-2012 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#ifndef __StGLCircle_h_
#define __StGLCircle_h_

#include "StGLMesh.h"

class StGLCircle : public StGLMesh {

        public:

    /**
     * Initialize the invalid circle.
     */
    ST_CPPEXPORT StGLCircle();

    ST_CPPEXPORT StGLCircle(const StGLVec3& theCenter,
                            const GLfloat   theRadius,
                            const GLsizei   thePointsCount);

    ST_CPPEXPORT void create(const StGLVec3& theCenter,
                             const GLfloat   theRadius,
                             const GLsizei   thePointsCount);

    ST_CPPEXPORT void create(const StGLVec3& theCenter,
                             const GLfloat   theRadiusX,
                             const GLfloat   theRadiusY,
                             const GLsizei   thePointsCount);

    /**
     * Compute the mesh using current configuration.
     */
    ST_CPPEXPORT virtual bool computeMesh() ST_ATTR_OVERRIDE;

    /**
     * Initialize the colors array with one specified color.
     * Mesh should be already initialized before.
     * @param theColor (const StGLVec4& ) - the color to set.
     */
    ST_CPPEXPORT bool initColorsArray(const StGLVec4& theColor);

        private:

    StGLVec3 myCenter;
    GLfloat  myRadiusX;
    GLfloat  myRadiusY;
    GLsizei  myPointsCount;

};

#endif //__StGLCircle_h_
