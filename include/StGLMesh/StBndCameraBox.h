/**
 * Copyright © 2011-2012 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#ifndef __StBndCameraBox_h_
#define __StBndCameraBox_h_

#include <StGL/StGLMatrix.h>
#include "StBndBox.h"

class StGLPrism;
class StGLContext;

/**
 * Special boundary container for fit all operation.
 * The main difference from StBndBox class is that
 * box computed with camera transformation.
 * This allows to find the better boundary box for fit all operation
 * however this box should be each time recomputed for new camera position.
 * Notice that most operations like isIn() will work as is - thus assuming
 * point parameter is in local coordinates system.
 */
class ST_LOCAL StBndCameraBox : public StBndBox {

        protected:

    StGLMatrix myTransf; //!< transformation matrix

        public:

    StBndCameraBox(const StGLMatrix& theTransf);

        public: //!< inheritance methods

    virtual ~StBndCameraBox();

    /**
     * Input point is in global coordinates and will be transformed.
     */
    virtual void enlarge(const StGLVec3& theNewPnt);

    /**
     * Input points set is in global coordinates and will be transformed.
     */
    virtual void enlarge(const StArray<StGLVec3>& thePoints);

    /**
     * Returns the boundary box center in global coordinates
     * (computed with reversed transformation).
     */
    StGLVec3 getCenterGlobal() const;

    /**
     * Creates the 3D prism (for debug rendering;
     * computed with reversed transformation).
     */
    void getPrism(StGLContext& theCtx,
                  StGLPrism&   thePrism) const;

};

#endif //__StBndCameraBox_h_
