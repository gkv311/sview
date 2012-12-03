/**
 * Copyright © 2010-2012 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#ifndef __StGLUVSphere_h_
#define __StGLUVSphere_h_

#include "StGLMesh.h"

class StBndSphere;

/**
 * Class represents configurable UV sphere with computed vertices, noramles, texture coordinates
 * and indices to faster (and correct) draw using one glMultiDrawElements() call.
 */
class ST_LOCAL StGLUVSphere : public StGLMesh {

        public:

    /**
     * Defines the UV sphere mesh with specified parameters.
     */
    StGLUVSphere(const StGLVec3& theCenter,
                 const GLfloat   theRadius,
                 const size_t    theRings);

    /**
     * Defines the UV sphere from boundary sphere.
     */
    StGLUVSphere(const StBndSphere& theBndSphere,
                 const size_t       theRings);

    virtual ~StGLUVSphere();

    /**
     * Compute the mesh using current configuration.
     */
    virtual bool computeMesh();

        protected:

    /**
     * Draw the primitives itself.
     */
    virtual void drawKernel(StGLContext& theCtx) const;

        private:

    StArrayList<GLsizei> myPrimCounts;
    StArrayList<void*>   myIndPointers;
    StGLVec3             myCenter;
    GLfloat              myRadius;
    size_t               myRings;

};

#endif //__StGLUVSphere_h_
