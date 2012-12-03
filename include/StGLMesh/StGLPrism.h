/**
 * Copyright © 2010-2012 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#ifndef __StGLPrism_h_
#define __StGLPrism_h_

#include "StGLMesh.h"

class StBndBox;

/**
 * Class represents the prism.
 */
class ST_LOCAL StGLPrism : public StGLMesh {

        public:

    StGLPrism();

    void setWireframe(size_t theNbIsosX = 0,
                      size_t theNbIsosY = 0,
                      size_t theNbIsosZ = 0);

    void setVisibilityX(bool theToShowX0, bool theToShowX1);
    void setVisibilityY(bool theToShowY0, bool theToShowY1);
    void setVisibilityZ(bool theToShowZ0, bool theToShowZ1);

    /**
     * Define the box.
     */
    bool init(StGLContext&    theCtx,
              const StGLVec3& theCenter,
              const GLfloat   theDX,
              const GLfloat   theDY,
              const GLfloat   theDZ);

    /**
     * Define the box from boundary box.
     */
    bool init(StGLContext&    theCtx,
              const StBndBox& theBndBox);

    /**
     * Define the prism by 6 points.
     */
    bool init(StGLContext&    theCtx,
              const StGLVec3& theX0Y0Z0, // rectangle in near Z plane
              const StGLVec3& theX1Y0Z0,
              const StGLVec3& theX1Y1Z0,
              const StGLVec3& theX0Y1Z0,
              const StGLVec3& theX0Y0Z1, // rectangle in far Z plane
              const StGLVec3& theX1Y0Z1,
              const StGLVec3& theX1Y1Z1,
              const StGLVec3& theX0Y1Z1);

        private:

    size_t myNbIsosX;
    size_t myNbIsosY;
    size_t myNbIsosZ;

    bool   myToShowZ0;
    bool   myToShowZ1;
    bool   myToShowX0;
    bool   myToShowX1;
    bool   myToShowY0;
    bool   myToShowY1;

};

#endif //__StGLPrism_h_
