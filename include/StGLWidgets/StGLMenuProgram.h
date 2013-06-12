/**
 * Copyright © 2009-2013 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
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
     * Setup color.
     * @param theCtx          active GL context
     * @param theColor        color
     * @param theOpacityValue opacity coefficient
     */
    ST_CPPEXPORT void setColor(StGLContext&    theCtx,
                               const StGLVec4& theColor,
                               const GLfloat   theOpacityValue);

    /**
     * Initialize program.
     * @param theCtx active GL context
     * @return true if no error
     */
    ST_CPPEXPORT virtual bool init(StGLContext& theCtx);

        private:

    StGLVarLocation uniProjMatLoc; //!< location of uniform variable of projection matrix
    StGLVarLocation uniColorLoc;   //!< location of uniform variable of color value

};

#endif // __StGLMenuProgram_h_
