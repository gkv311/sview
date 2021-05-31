/**
 * Copyright © 1998-2010 Geometric Tools, LLC
 * Copyright © 2011 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#ifndef __StBndSphere_h_
#define __StBndSphere_h_

#include "StBndContainer.h"

class StBndSphere : public StBndContainer {

        protected:

    /**
     * Default initializer. Find initial sphere center and radius
     * however doesn't ensures that all points are inside this sphere.
     * This protected method is called from ::enlarge() method
     * for void boundary sphere.
     * @param thePoints the points set
     */
    ST_CPPEXPORT virtual void init(const StArray<StGLVec3>& thePoints);

    /**
     * Find a fast approximation of the bounding sphere for a point set.
     * Computed sphere is NOT the smallest one!
     * @param thePoints (const StArray<StGLVec3>& ) - the points set.
     */
    ST_CPPEXPORT void initFast(const StArray<StGLVec3>& thePoints);

    /**
     * Compute the minimum volume sphere containing the input set of points using Welzl's algorithm.
     * @param thePoints (const StArray<StGLVec3>& ) - the points set.
     */
    ST_CPPEXPORT void initWelzl(const StArray<StGLVec3>& thePoints);

        public: //!< inheritance methods

    ST_CPPEXPORT virtual ~StBndSphere();
    ST_CPPEXPORT virtual void reset();
    ST_CPPEXPORT virtual bool isIn(const StGLVec3& thePnt) const;
    ST_CPPEXPORT virtual void enlarge(const GLfloat theTolerance);
    ST_CPPEXPORT virtual void enlarge(const StGLVec3& theNewPnt);
    ST_CPPEXPORT virtual void enlarge(const StArray<StGLVec3>& thePoints);

        public:

    ST_CPPEXPORT StBndSphere();

    /**
     * Return the center of bounding sphere.
     */
    const StGLVec3& getCenter() const {
        return myCenter;
    }

    /**
     * Return the radius of bounding sphere.
     */
    GLfloat getRadius() const {
        return myRadius;
    }

    /**
     * (Re)define the bounding sphere using known values.
     */
    void define(const StGLVec3& theCenter, GLfloat theRadius) {
        myCenter = theCenter;
        myRadius = theRadius;
        StBndContainer::setDefined();
    }

        private:

    StGLVec3 myCenter; //!< sphere center
    GLfloat  myRadius; //!< sphere radius

};

#endif //__StBndSphere_h_
