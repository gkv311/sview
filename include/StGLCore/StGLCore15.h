/**
 * Copyright © 2012-2014 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#ifndef __StGLCore15_h_
#define __StGLCore15_h_

#include <StGLCore/StGLCore14.h>

/**
 * OpenGL 1.5 core based on 1.4 version.
 */
template<typename theBaseClass_t>
struct stglTmplCore15 : public theBaseClass_t {

        public: //! @name OpenGL 1.5 additives to 1.4

#if !defined(GL_ES_VERSION_2_0)
    using theBaseClass_t::glGenQueries;
    using theBaseClass_t::glDeleteQueries;
    using theBaseClass_t::glIsQuery;
    using theBaseClass_t::glBeginQuery;
    using theBaseClass_t::glEndQuery;
    using theBaseClass_t::glGetQueryiv;
    using theBaseClass_t::glGetQueryObjectiv;
    using theBaseClass_t::glGetQueryObjectuiv;

    using theBaseClass_t::glMapBuffer;
    using theBaseClass_t::glUnmapBuffer;
    using theBaseClass_t::glGetBufferSubData;
    using theBaseClass_t::glGetBufferPointerv;
#endif

    using theBaseClass_t::glBindBuffer;
    using theBaseClass_t::glDeleteBuffers;
    using theBaseClass_t::glGenBuffers;
    using theBaseClass_t::glIsBuffer;
    using theBaseClass_t::glBufferData;
    using theBaseClass_t::glBufferSubData;
    using theBaseClass_t::glGetBufferParameteriv;

};

/**
 * OpenGL 1.5 core based on 1.4 version.
 */
typedef stglTmplCore15<StGLCore14>    StGLCore15;

/**
 * OpenGL 1.5 without deprecated entry points.
 */
typedef stglTmplCore15<StGLCore14Fwd> StGLCore15Fwd;

#endif // __StGLCore15_h_
