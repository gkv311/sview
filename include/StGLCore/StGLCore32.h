/**
 * Copyright © 2012 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#ifndef __StGLCore32_h_
#define __StGLCore32_h_

#include <StGLCore/StGLCore31.h>

/**
 * OpenGL 3.2 definition.
 */
template<typename theBaseClass_t>
struct stglTmplCore32 : public theBaseClass_t {

#if !defined(GL_ES_VERSION_2_0)

        public: //! @name GL_ARB_draw_elements_base_vertex (added to OpenGL 3.2 core)

    using theBaseClass_t::glDrawElementsBaseVertex;
    using theBaseClass_t::glDrawRangeElementsBaseVertex;
    using theBaseClass_t::glDrawElementsInstancedBaseVertex;
    using theBaseClass_t::glMultiDrawElementsBaseVertex;

        public: //! @name GL_ARB_provoking_vertex (added to OpenGL 3.2 core)

    using theBaseClass_t::glProvokingVertex;

        public: //! @name GL_ARB_sync (added to OpenGL 3.2 core)

    using theBaseClass_t::glFenceSync;
    using theBaseClass_t::glIsSync;
    using theBaseClass_t::glDeleteSync;
    using theBaseClass_t::glClientWaitSync;
    using theBaseClass_t::glWaitSync;
    using theBaseClass_t::glGetInteger64v;
    using theBaseClass_t::glGetSynciv;

        public: //! @name GL_ARB_texture_multisample (added to OpenGL 3.2 core)

    using theBaseClass_t::glTexImage2DMultisample;
    using theBaseClass_t::glTexImage3DMultisample;
    using theBaseClass_t::glGetMultisamplefv;
    using theBaseClass_t::glSampleMaski;

        public: //! @name OpenGL 3.2 additives to 3.1

    using theBaseClass_t::glGetInteger64i_v;
    using theBaseClass_t::glGetBufferParameteri64v;
    using theBaseClass_t::glFramebufferTexture;

#endif

};

/**
 * OpenGL 3.2 compatibility profile.
 */
typedef stglTmplCore32<StGLCore31Back> StGLCore32Back;

/**
 * OpenGL 3.2 core profile.
 */
typedef stglTmplCore32<StGLCore31>     StGLCore32;

#endif // __StGLCore32_h_
