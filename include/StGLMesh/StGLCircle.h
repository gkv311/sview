/**
 * Copyright © 2010-2012 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#ifndef __StGLCircle_h_
#define __StGLCircle_h_

#include "StGLMesh.h"

class ST_LOCAL StGLCircle : public StGLMesh {

        public:

    /**
     * Initialize the invalid circle.
     */
    StGLCircle();

    StGLCircle(const StGLVec3& theCenter,
               const GLfloat   theRadius,
               const GLsizei   thePointsCount);

    void create(const StGLVec3& theCenter,
                const GLfloat   theRadius,
                const GLsizei   thePointsCount);

    void create(const StGLVec3& theCenter,
                const GLfloat   theRadiusX,
                const GLfloat   theRadiusY,
                const GLsizei   thePointsCount);

    /**
     * Compute the mesh using current configuration.
     */
    virtual bool computeMesh();

    /**
     * Initialize the colors array with one specified color.
     * Mesh should be already initialized before.
     * @param theColor (const StGLVec4& ) - the color to set.
     */
    bool initColorsArray(const StGLVec4& theColor);

        private:

    StGLVec3 myCenter;
    GLfloat  myRadiusX;
    GLfloat  myRadiusY;
    GLsizei  myPointsCount;

};

#endif //__StGLCircle_h_
