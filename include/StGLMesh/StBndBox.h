/**
 * Copyright © 2011 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#ifndef __StBndBox_h_
#define __StBndBox_h_

#include "StBndContainer.h"

/**
 * The bounding box of a finite geometric object is the box with minimal volume, that contains the object.
 * Edges of bounding box are parallel to the coordinate axes, and is thus defined by its maximum and minimum extents for all axes.
 * This is the computationally simplest of all linear bounding containers.
 */
class StBndBox : public StBndContainer {

        public: //!< inheritance methods

    ST_CPPEXPORT virtual ~StBndBox();
    ST_CPPEXPORT virtual void reset();
    ST_CPPEXPORT virtual bool isIn(const StGLVec3& thePnt) const;
    ST_CPPEXPORT virtual void enlarge(const GLfloat theTolerance);
    ST_CPPEXPORT virtual void enlarge(const StGLVec3& theNewPnt);
    ST_CPPEXPORT virtual void enlarge(const StArray<StGLVec3>& thePoints);

        public:

    /**
     * Create the empty (void) bounding box.
     */
    ST_CPPEXPORT StBndBox();

    /**
     * Define the bounding box with min / max vectors.
     */
    ST_CPPEXPORT StBndBox(const StGLVec3& theMin,
                          const StGLVec3& theMax);

    /**
     * Return the x/y/z minimal values.
     */
    const StGLVec3& getMin() const {
        return myMin;
    }

    /**
     * Return the x/y/z maximal values.
     */
    const StGLVec3& getMax() const {
        return myMax;
    }

    /**
     * Get width for the boundary box.
     */
    GLfloat getDX() const {
        return myMax.x() - myMin.x();
    }

    /**
     * Get height for the boundary box.
     */
    GLfloat getDY() const {
        return myMax.y() - myMin.y();
    }

    /**
     * Get DZ for the boundary box.
     */
    GLfloat getDZ() const {
        return myMax.z() - myMin.z();
    }

    /**
     * Compute boundary box center.
     */
    StGLVec3 getCenter() const {
        return StGLVec3::getLERP(myMin, myMax, 0.5f);
    }

    /**
     * Check that bounding boxes are disjoint.
     * @param theBndBox (const StBndBox& ) - another bounding box;
     * @return true if bounding boxes are disjoint.
     */
    ST_CPPEXPORT bool areDisjoint(const StBndBox& theBndBox) const;

    /**
     * Check that bounding boxes has intersection.
     * @param theBndBox (const StBndBox& ) - another bounding box;
     * @return true if bounding boxes has intersection.
     */
    bool areIntersect(const StBndBox& theBndBox) const {
        return !areDisjoint(theBndBox);
    }

        protected:

    StGLVec3 myMin; //!< x/y/z minimal values
    StGLVec3 myMax; //!< x/y/z maximal values

};

#endif //__StBndBox_h_
