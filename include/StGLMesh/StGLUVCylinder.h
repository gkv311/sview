/**
 * Copyright © 2019-2020 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
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
                                const float theHeight,
                                const float theRadius,
                                const int theRings);

    /**
     * Defines the UV cylinder mesh with specified parameters.
     */
    ST_CPPEXPORT StGLUVCylinder(const StGLVec3& theCenter,
                                const float theHeight,
                                const float theRadius,
                                const float theAngleFrom,
                                const float theAngleTo,
                                const int theRings);

    ST_CPPEXPORT virtual ~StGLUVCylinder();

    /**
     * Compute the mesh using current configuration.
     */
    ST_CPPEXPORT virtual bool computeMesh() ST_ATTR_OVERRIDE;

    ST_LOCAL float getRadius()    const { return myRadius; }
    ST_LOCAL void  setRadius(float theRadius) { myRadius = theRadius; }
    ST_LOCAL float getHeight()    const { return myHeight; }
    ST_LOCAL void  setHeight(float theHeight) { myHeight = theHeight; }
    ST_LOCAL float getAngleFrom() const { return myAngleFrom; }
    ST_LOCAL void  setAngleFrom(float theAngle) { myAngleFrom = theAngle; }
    ST_LOCAL float getAngleTo()   const { return myAngleTo; }
    ST_LOCAL void  setAngleTo(float theAngle) { myAngleTo = theAngle; }
    ST_LOCAL float getAngle()     const { return myAngleTo - myAngleFrom; }

        private:

    StGLVec3 myCenter;
    float    myRadius;
    float    myHeight;
    float    myAngleFrom;
    float    myAngleTo;
    int      myNbRings;

};

#endif //__StGLUVCylinder_h_
