/**
 * Copyright © 2011 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#ifndef __StBndContainer_h_
#define __StBndContainer_h_

#include <StGL/StGLVec.h>

/**
 * This is the common interface for bounding container object.
 */
class StBndContainer {

        private:

    bool myIsVoid; //!< indicate an undefined bounding container

        protected:

    void setDefined() {
        myIsVoid = false;
    }

        public:

    /**
     * Create the empty (void) bounding container.
     */
    StBndContainer()
    : myIsVoid(true) {
        //
    }

    virtual ~StBndContainer() {
        //
    }

    /**
     * Returns the bounding container definition is complete or not.
     * @return true if bounding container is undefined.
     */
    bool isVoid() const {
        return myIsVoid;
    }

    /**
     * Reset current bounding container (make it void).
     */
    virtual void reset() {
        myIsVoid = true;
    }

    /**
     * Check the point is inside (or touch) the bounding container.
     * @param thePnt (const StGLVec3& ) - 3D point to check;
     * @return true if point inside the bounding container.
     */
    virtual bool isIn(const StGLVec3& thePnt) const = 0;

    /**
     * Check the point is outside the bounding container.
     * @param thePnt (const StGLVec3& ) - 3D point to check;
     * @return true if point is outside the bounding container.
     */
    bool isOut(const StGLVec3& thePnt) const {
        return !isIn(thePnt);
    }

    /**
     * Smoothly enlarge bounding container with the given tolerance.
     * @param theTolerance (const GLfloat ) - the tolerance value.
     */
    virtual void enlarge(const GLfloat theTolerance) = 0;

    /**
     * Enlarge bounding container with the given 3D point.
     * @param theNewPnt (const StGLVec3& ) - new 3D point.
     */
    virtual void enlarge(const StGLVec3& theNewPnt) = 0;

    /**
     * Enlarge bounding container with 3D points set.
     * @param thePoints (const StArray<StGLVec3>&) - 3D points set.
     */
    virtual void enlarge(const StArray<StGLVec3>& thePoints) = 0;

    /**
     * Enlarge bounding container with another container.
     * @param theBnd (const StBndContainer&) - another boundary container.
     */
    ///virtual void enlarge(const StBndContainer& theBnd) = 0;

};

#endif //__StBndContainer_h_
