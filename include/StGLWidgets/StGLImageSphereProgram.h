/**
 * Copyright © 2010-2012 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#ifndef __StGLImageSphereProgram_h_
#define __StGLImageSphereProgram_h_

#include "StGLImageProgram.h"

class ST_LOCAL StGLImageSphereProgram : public StGLImageProgram {

        public:

    StGLImageSphereProgram();

    /**
     * Initialize shaders and perform link.
     */
    virtual bool init(StGLContext& theCtx);

    virtual void release(StGLContext& theCtx);

    void setSmoothFilter(StGLContext&        theCtx,
                         const TextureFilter theTextureFilter);

        private:

    StGLFragmentShader* fGetColorPtr;
    StGLFragmentShader  fGetColorLinear;

};

#endif //__StGLImageSphereProgram_h_
