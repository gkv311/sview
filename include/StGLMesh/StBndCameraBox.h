/**
 * Copyright © 2011-2012 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
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
class StBndCameraBox : public StBndBox {

        public:

    ST_CPPEXPORT StBndCameraBox(const StGLMatrix& theTransf);

        public: //!< inheritance methods

    ST_CPPEXPORT virtual ~StBndCameraBox();

    /**
     * Input point is in global coordinates and will be transformed.
     */
    ST_CPPEXPORT virtual void enlarge(const StGLVec3& theNewPnt);

    /**
     * Input points set is in global coordinates and will be transformed.
     */
    ST_CPPEXPORT virtual void enlarge(const StArray<StGLVec3>& thePoints);

    /**
     * Returns the boundary box center in global coordinates
     * (computed with reversed transformation).
     */
    ST_CPPEXPORT StGLVec3 getCenterGlobal() const;

    /**
     * Creates the 3D prism (for debug rendering;
     * computed with reversed transformation).
     */
    ST_CPPEXPORT void getPrism(StGLContext& theCtx,
                               StGLPrism&   thePrism) const;

        protected:

    StGLMatrix myTransf; //!< transformation matrix

};

#endif //__StBndCameraBox_h_
