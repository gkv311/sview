/**
 * Copyright © 2012 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#ifndef __StGLCore31_h_
#define __StGLCore31_h_

#include <StGLCore/StGLCore30.h>

/**
 * OpenGL 3.1 definition.
 */
template<typename theBaseClass_t>
struct stglTmplCore31 : public theBaseClass_t {

#if !defined(GL_ES_VERSION_2_0)

        public: //! @name GL_ARB_uniform_buffer_object (added to OpenGL 3.1 core)

    using theBaseClass_t::glGetUniformIndices;
    using theBaseClass_t::glGetActiveUniformsiv;
    using theBaseClass_t::glGetActiveUniformName;
    using theBaseClass_t::glGetUniformBlockIndex;
    using theBaseClass_t::glGetActiveUniformBlockiv;
    using theBaseClass_t::glGetActiveUniformBlockName;
    using theBaseClass_t::glUniformBlockBinding;

        public: //! @name GL_ARB_copy_buffer (added to OpenGL 3.1 core)

    using theBaseClass_t::glCopyBufferSubData;

        public: //! @name OpenGL 3.1 additives to 3.0

    using theBaseClass_t::glDrawArraysInstanced;
    using theBaseClass_t::glDrawElementsInstanced;
    using theBaseClass_t::glTexBuffer;
    using theBaseClass_t::glPrimitiveRestartIndex;

#endif

};

/**
 * OpenGL 3.1 compatibility profile.
 */
typedef stglTmplCore31<StGLCore30>    StGLCore31Back;

/**
 * OpenGL 3.1 core profile (without removed entry points marked as deprecated in 3.0).
 * Notice that GLSL versions 1.10 and 1.20 also removed in 3.1!
 */
typedef stglTmplCore31<StGLCore30Fwd> StGLCore31;

#endif // __StGLCore31_h_
