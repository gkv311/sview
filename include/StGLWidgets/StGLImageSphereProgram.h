/**
 * Copyright © 2010-2013 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#ifndef __StGLImageSphereProgram_h_
#define __StGLImageSphereProgram_h_

#include "StGLImageProgram.h"

class StGLImageSphereProgram : public StGLImageProgram {

        public:

    ST_CPPEXPORT StGLImageSphereProgram();

    /**
     * Initialize shaders and perform link.
     */
    ST_CPPEXPORT virtual bool init(StGLContext& theCtx);

    ST_CPPEXPORT virtual void release(StGLContext& theCtx);

    ST_CPPEXPORT void setSmoothFilter(StGLContext&        theCtx,
                                      const TextureFilter theTextureFilter);

        private:

    StGLFragmentShader* fGetColorPtr;
    StGLFragmentShader  fGetColorLinear;

};

#endif //__StGLImageSphereProgram_h_
