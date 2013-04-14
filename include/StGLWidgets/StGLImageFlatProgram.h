/**
 * Copyright © 2010-2013 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#ifndef __StGLImageFlatProgram_h_
#define __StGLImageFlatProgram_h_

#include "StGLImageProgram.h"

class StGLImageFlatProgram : public StGLImageProgram {

        public:

    ST_CPPEXPORT StGLImageFlatProgram();

    /**
     * Initialize shaders and perform link.
     */
    ST_CPPEXPORT virtual bool init(StGLContext& theCtx);

    ST_CPPEXPORT void setSmoothFilter(StGLContext&        theCtx,
                                      const TextureFilter theTextureFilter);

        private:

    StGLFragmentShader* fGetColorPtr;

};

#endif //__StGLImageFlatProgram_h_
