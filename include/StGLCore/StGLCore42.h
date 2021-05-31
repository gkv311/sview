/**
 * Copyright © 2012 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#ifndef __StGLCore42_h_
#define __StGLCore42_h_

#include <StGLCore/StGLCore41.h>

/**
 * OpenGL 4.2 definition.
 */
template<typename theBaseClass_t>
struct stglTmplCore42 : public theBaseClass_t {

#if !defined(GL_ES_VERSION_2_0)

        public: //! @name GL_ARB_base_instance (added to OpenGL 4.2 core)

    using theBaseClass_t::glDrawArraysInstancedBaseInstance;
    using theBaseClass_t::glDrawElementsInstancedBaseInstance;
    using theBaseClass_t::glDrawElementsInstancedBaseVertexBaseInstance;

        public: //! @name GL_ARB_transform_feedback_instanced (added to OpenGL 4.2 core)

    using theBaseClass_t::glDrawTransformFeedbackInstanced;
    using theBaseClass_t::glDrawTransformFeedbackStreamInstanced;

        public: //! @name GL_ARB_internalformat_query (added to OpenGL 4.2 core)

    using theBaseClass_t::glGetInternalformativ;

        public: //! @name GL_ARB_shader_atomic_counters (added to OpenGL 4.2 core)

    using theBaseClass_t::glGetActiveAtomicCounterBufferiv;

        public: //! @name GL_ARB_shader_image_load_store (added to OpenGL 4.2 core)

    using theBaseClass_t::glBindImageTexture;
    using theBaseClass_t::glMemoryBarrier;

        public: //! @name GL_ARB_texture_storage (added to OpenGL 4.2 core)

    using theBaseClass_t::glTexStorage1D;
    using theBaseClass_t::glTexStorage2D;
    using theBaseClass_t::glTexStorage3D;
    using theBaseClass_t::glTextureStorage1DEXT;
    using theBaseClass_t::glTextureStorage2DEXT;
    using theBaseClass_t::glTextureStorage3DEXT;

        public: //! @name OpenGL 4.2 additives to 4.1

#endif

};

/**
 * OpenGL 4.2 compatibility profile.
 */
typedef stglTmplCore42<StGLCore41Back> StGLCore42Back;

/**
 * OpenGL 4.2 core profile.
 */
typedef stglTmplCore42<StGLCore41>     StGLCore42;


#endif // __StGLCore42_h_
