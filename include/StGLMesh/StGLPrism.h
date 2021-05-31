/**
 * Copyright © 2010-2013 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#ifndef __StGLPrism_h_
#define __StGLPrism_h_

#include "StGLMesh.h"

class StBndBox;

/**
 * Class represents the prism.
 */
class StGLPrism : public StGLMesh {

        public:

    ST_CPPEXPORT StGLPrism();

    ST_CPPEXPORT void setWireframe(size_t theNbIsosX = 0,
                                   size_t theNbIsosY = 0,
                                   size_t theNbIsosZ = 0);

    ST_CPPEXPORT void setVisibilityX(bool theToShowX0, bool theToShowX1);
    ST_CPPEXPORT void setVisibilityY(bool theToShowY0, bool theToShowY1);
    ST_CPPEXPORT void setVisibilityZ(bool theToShowZ0, bool theToShowZ1);

    /**
     * Define the box.
     */
    ST_CPPEXPORT bool init(StGLContext&    theCtx,
                           const StGLVec3& theCenter,
                           const GLfloat   theDX,
                           const GLfloat   theDY,
                           const GLfloat   theDZ);

    /**
     * Define the box from boundary box.
     */
    ST_CPPEXPORT bool init(StGLContext&    theCtx,
                           const StBndBox& theBndBox);

    /**
     * Define the prism by 6 points.
     */
    ST_CPPEXPORT bool init(StGLContext&    theCtx,
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
