/**
 * StOutDistorted, class providing stereoscopic output in anamorph side by side format using StCore toolkit.
 * Copyright © 2016 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#ifndef __StProgramBarrel_h_
#define __StProgramBarrel_h_

#include <StGL/StGLProgram.h>
#include <StGL/StGLVec.h>

/**
 * Distortion GLSL program.
 */
class StProgramBarrel : public StGLProgram {

        public:

    /**
     * Empty constructor.
     */
    ST_LOCAL StProgramBarrel();

    /**
     * Position vertex attribute location.
     */
    ST_LOCAL StGLVarLocation getVVertexLoc()   const { return StGLVarLocation(0); }

    /**
     * Texture vertex attribute location.
     */
    ST_LOCAL StGLVarLocation getVTexCoordLoc() const { return StGLVarLocation(1); }

    /**
     * Initialize the program.
     */
    ST_LOCAL virtual bool init(StGLContext& theCtx) ST_ATTR_OVERRIDE;

    /**
     * Setup distortion coefficients.
     */
    ST_LOCAL void setupCoeff(StGLContext&    theCtx,
                             const StGLVec4& theVec);

    /**
     * Setup Chrome coefficients.
     */
    ST_LOCAL void setupChrome(StGLContext&    theCtx,
                              const StGLVec4& theVec);

    ST_LOCAL void setLensCenter(StGLContext&    theCtx,
                                const StGLVec2& theVec);

    ST_LOCAL void setScale(StGLContext&    theCtx,
                           const StGLVec2& theVec);

    ST_LOCAL void setScaleIn(StGLContext&    theCtx,
                             const StGLVec2& theVec);

        private:

    StGLVarLocation uniChromAbLoc;
    StGLVarLocation uniWarpCoeffLoc;
    StGLVarLocation uniLensCenterLoc;
    StGLVarLocation uniScaleLoc;
    StGLVarLocation uniScaleInLoc;

};

#endif // __StProgramBarrel_h_
