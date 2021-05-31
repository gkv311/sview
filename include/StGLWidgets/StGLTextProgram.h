/**
 * StGLWidgets, small C++ toolkit for writing GUI using OpenGL.
 * Copyright © 2009-2020 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#ifndef __StGLTextProgram_h_
#define __StGLTextProgram_h_

#include <StGL/StGLProgram.h>
#include <StGL/StGLVec.h>

class StGLMatrix;

/**
 * GLSL program for rendering textured text.
 * Font texture is expected to be in alpha format
 * (either GL_ALPHA8 or GL_R8 when StGLContext::arbTexRG is available).
 */
class StGLTextProgram : public StGLProgram {

        public:

    /**
     * Empty constructor.
     */
    ST_CPPEXPORT StGLTextProgram();

    /**
     * Destructor.
     */
    ST_CPPEXPORT virtual ~StGLTextProgram();

    /**
     * Return vertex position attribute location.
     */
    ST_LOCAL StGLVarLocation getVVertexLoc()   const { return StGLVarLocation(0); }

    /**
     * Return vertex texture coordinates attribute location.
     */
    ST_LOCAL StGLVarLocation getVTexCoordLoc() const { return StGLVarLocation(1); }

    /**
     * Setup projection matrix.
     * @param theCtx     active GL context
     * @param theProjMat projection matrix
     */
    ST_CPPEXPORT void setProjMat(StGLContext&      theCtx,
                                 const StGLMatrix& theProjMat);

    /**
     * Setup components of model-view matrix.
     * @param theCtx   active GL context
     * @param theDisp  translation vector
     * @param theScale scale factor
     */
    ST_CPPEXPORT void setDisplacement(StGLContext& theCtx,
                                      const StGLVec3& theDisp,
                                      const float theScale);

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
    ST_CPPEXPORT virtual bool init(StGLContext& theCtx) ST_ATTR_OVERRIDE;

        private:

    StGLVarLocation myUniformProjMat;  //!< location of uniform variable of projection matrix
    StGLVarLocation myUniformDispl;    //!< location of uniform variable of displacement vector
    StGLVarLocation myUniformColor;    //!< location of uniform variable of color value

};

#endif // __StGLTextProgram_h_
