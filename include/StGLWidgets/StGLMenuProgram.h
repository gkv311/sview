/**
 * StGLWidgets, small C++ toolkit for writing GUI using OpenGL.
 * Copyright © 2009-2013 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#ifndef __StGLMenuProgram_h_
#define __StGLMenuProgram_h_

#include <StGL/StGLProgram.h>
#include <StGL/StGLVec.h>

class StGLMatrix;

/**
 * Simple GLSL program for GUI elements.
 */
class StGLMenuProgram : public StGLProgram {

        public:

    /**
     * Empty constructor.
     */
    ST_CPPEXPORT StGLMenuProgram();

    /**
     * Initialize program.
     * @param theCtx active GL context
     * @return true if no error
     */
    ST_CPPEXPORT virtual bool init(StGLContext& theCtx);

    /**
     * @return vertex attribute location
     */
    ST_LOCAL inline StGLVarLocation getVVertexLoc() const {
        return StGLVarLocation(0);
    }

    /**
     * Setup projection matrix.
     * @param theCtx     active GL context
     * @param theProjMat projection matrix
     */
    ST_CPPEXPORT void setProjMat(StGLContext&      theCtx,
                                 const StGLMatrix& theProjMat);

    /**
     * Use the program and setup uniforms.
     * @param theCtx          active GL context
     * @param theColor        color
     * @param theOpacityValue opacity coefficient
     * @param theDispX        displacement
     */
    ST_CPPEXPORT void use(StGLContext&    theCtx,
                          const StGLVec4& theColor,
                          const GLfloat   theOpacityValue,
                          const GLfloat   theDispX);

    /**
     * Use the program and setup uniforms.
     * @param theCtx   active GL context
     * @param theDispX displacement
     */
    ST_CPPEXPORT void use(StGLContext&  theCtx,
                          const GLfloat theDispX);

    ST_LOCAL inline void use(StGLContext& theCtx) const {
        StGLProgram::use(theCtx);
    }

    /**
     * Setup color.
     * @param theCtx          active GL context
     * @param theColor        color
     * @param theOpacityValue opacity coefficient
     */
    ST_CPPEXPORT void setColor(StGLContext&    theCtx,
                               const StGLVec4& theColor,
                               const GLfloat   theOpacityValue);

        private:

    GLfloat         myDispX;       //!< vertex displacement along X direction
    StGLVarLocation uniProjMatLoc; //!< location of uniform variable of projection matrix
    StGLVarLocation uniDispLoc;    //!< location of uniform variable of displacement vector
    StGLVarLocation uniColorLoc;   //!< location of uniform variable of color value

};

#endif // __StGLMenuProgram_h_
