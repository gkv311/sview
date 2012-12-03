/**
 * Copyright © 1998-2010 Geometric Tools, LLC
 * Copyright © 2011 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#ifndef __StBndSphere_h_
#define __StBndSphere_h_

#include "StBndContainer.h"

class ST_LOCAL StBndSphere : public StBndContainer {

        private:

    StGLVec3 myCenter; //!< sphere center
    GLfloat  myRadius; //!< sphere radius

        protected:

    /**
     * Default initializer. Find initial sphere center and radius
     * however doesn't ensures that all points are inside this sphere.
     * This protected method is called from ::enlarge() method
     * for void boundary sphere.
     * @param thePoints (const StArray<StGLVec3>& ) - the points set.
     */
    virtual void init(const StArray<StGLVec3>& thePoints) {
        initWelzl(thePoints);
    }

    /**
     * Find a fast approximation of the bounding sphere for a point set.
     * Computed sphere is NOT the smallest one!
     * @param thePoints (const StArray<StGLVec3>& ) - the points set.
     */
    void initFast(const StArray<StGLVec3>& thePoints);

    /**
     * Compute the minimum volume sphere containing the input set of points using Welzl's algorithm.
     * @param thePoints (const StArray<StGLVec3>& ) - the points set.
     */
    void initWelzl(const StArray<StGLVec3>& thePoints);

        public: //!< inheritance methods

    virtual ~StBndSphere();
    virtual void reset();
    virtual bool isIn(const StGLVec3& thePnt) const;
    virtual void enlarge(const GLfloat theTolerance);
    virtual void enlarge(const StGLVec3& theNewPnt);
    virtual void enlarge(const StArray<StGLVec3>& thePoints);

        public:

    StBndSphere();

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

};

#endif //__StBndSphere_h_
