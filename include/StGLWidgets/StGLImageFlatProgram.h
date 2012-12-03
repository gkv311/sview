/**
 * Copyright © 2010-2012 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#ifndef __StGLImageFlatProgram_h_
#define __StGLImageFlatProgram_h_

#include "StGLImageProgram.h"

class ST_LOCAL StGLImageFlatProgram : public StGLImageProgram {

        private:

    StGLFragmentShader* fGetColorPtr;

        public:

    StGLImageFlatProgram();

    /**
     * Initialize shaders and perform link.
     */
    virtual bool init(StGLContext& theCtx);

    void setSmoothFilter(StGLContext&        theCtx,
                         const TextureFilter theTextureFilter);

};

#endif //__StGLImageFlatProgram_h_
