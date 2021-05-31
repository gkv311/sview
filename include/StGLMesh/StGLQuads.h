/**
 * Copyright © 2011-2013 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#ifndef __StGLQuads_h_
#define __StGLQuads_h_

#include "StGLMesh.h"

/**
 * Class represents quads array.
 */
class StGLQuads : public StGLMesh {

        public:

    /**
     * Empty constructor. Vertices will be drawn as GL_TRIANGLES.
     */
    ST_CPPEXPORT StGLQuads();

    /**
     * Empty constructor.
     */
    ST_CPPEXPORT StGLQuads(const GLenum thePrimitives);

    /**
     * Initialize the quad that cover the whole screen (for identity projection and model view matrices).
     * Notice that created single quad will be drawn as GL_TRIANGLE_STRIP.
     */
    ST_CPPEXPORT bool initScreen(StGLContext& theCtx);

};

#endif //__StGLQuads_h_
