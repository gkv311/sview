/**
 * StGLWidgets, small C++ toolkit for writing GUI using OpenGL.
 * Copyright © 2009-2015 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#ifndef __StGLTextBorderProgram_h_
#define __StGLTextBorderProgram_h_

#include <StGL/StGLProgram.h>
#include <StGL/StGLVec.h>

class StGLMatrix;

/**
 * GLSL program for rendering border around 2D text.
 */
class StGLTextBorderProgram : public StGLProgram {

        public:

    /**
     * Empty constructor.
     */
    ST_CPPEXPORT StGLTextBorderProgram();

    /**
     * Destructor.
     */
    ST_CPPEXPORT virtual ~StGLTextBorderProgram();

    /**
     * Return location of vertex attribute defining position.
     */
    ST_LOCAL StGLVarLocation getVVertexLoc() const { return StGLVarLocation(0); }

    /**
     * Setup projection matrix.
     * @param theCtx     active GL context
     * @param theProjMat projection matrix
     */
    ST_CPPEXPORT void setProjMat(StGLContext&      theCtx,
                                 const StGLMatrix& theProjMat);

    /**
     * Setup model-view matrix.
     * @param theCtx      active GL context
     * @param theModelMat model-view matrix
     */
    ST_CPPEXPORT void setModelMat(StGLContext&      theCtx,
                                  const StGLMatrix& theModelMat);

    /**
     * Setup text color.
     * @param theCtx   active GL context
     * @param theColor color
     */
    ST_CPPEXPORT void setColor(StGLContext&    theCtx,
                               const StGLVec4& theColor);

    /**
     * Initialize program.
     * @param theCtx active GL context
     * @return true if no error
     */
    ST_CPPEXPORT virtual bool init(StGLContext& theCtx);

        private:

    StGLVarLocation myUniformProjMat;  //!< location of uniform variable of projection matrix
    StGLVarLocation myUniformModelMat; //!< location of uniform variable of model view matrix
    StGLVarLocation myUniformColor;    //!< location of uniform variable of color value

};

#endif // __StGLTextBorderProgram_h_
