/**
 * Copyright © 2012 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#ifndef __StGLCore33_h_
#define __StGLCore33_h_

#include <StGLCore/StGLCore32.h>

/**
 * OpenGL 3.3 definition.
 */
template<typename theBaseClass_t>
struct stglTmplCore33 : public theBaseClass_t {

#if !defined(GL_ES_VERSION_2_0)

        public: //! @name GL_ARB_blend_func_extended (added to OpenGL 3.3 core)

    using theBaseClass_t::glBindFragDataLocationIndexed;
    using theBaseClass_t::glGetFragDataIndex;

        public: //! @name GL_ARB_sampler_objects (added to OpenGL 3.3 core)

    using theBaseClass_t::glGenSamplers;
    using theBaseClass_t::glDeleteSamplers;
    using theBaseClass_t::glIsSampler;
    using theBaseClass_t::glBindSampler;
    using theBaseClass_t::glSamplerParameteri;
    using theBaseClass_t::glSamplerParameteriv;
    using theBaseClass_t::glSamplerParameterf;
    using theBaseClass_t::glSamplerParameterfv;
    using theBaseClass_t::glSamplerParameterIiv;
    using theBaseClass_t::glSamplerParameterIuiv;
    using theBaseClass_t::glGetSamplerParameteriv;
    using theBaseClass_t::glGetSamplerParameterIiv;
    using theBaseClass_t::glGetSamplerParameterfv;
    using theBaseClass_t::glGetSamplerParameterIuiv;

        public: //! @name GL_ARB_timer_query (added to OpenGL 3.3 core)

    using theBaseClass_t::glQueryCounter;
    using theBaseClass_t::glGetQueryObjecti64v;
    using theBaseClass_t::glGetQueryObjectui64v;

        public: //! @name GL_ARB_vertex_type_2_10_10_10_rev (added to OpenGL 3.3 core)

    using theBaseClass_t::glVertexP2ui;
    using theBaseClass_t::glVertexP2uiv;
    using theBaseClass_t::glVertexP3ui;
    using theBaseClass_t::glVertexP3uiv;
    using theBaseClass_t::glVertexP4ui;
    using theBaseClass_t::glVertexP4uiv;
    using theBaseClass_t::glTexCoordP1ui;
    using theBaseClass_t::glTexCoordP1uiv;
    using theBaseClass_t::glTexCoordP2ui;
    using theBaseClass_t::glTexCoordP2uiv;
    using theBaseClass_t::glTexCoordP3ui;
    using theBaseClass_t::glTexCoordP3uiv;
    using theBaseClass_t::glTexCoordP4ui;
    using theBaseClass_t::glTexCoordP4uiv;
    using theBaseClass_t::glMultiTexCoordP1ui;
    using theBaseClass_t::glMultiTexCoordP1uiv;
    using theBaseClass_t::glMultiTexCoordP2ui;
    using theBaseClass_t::glMultiTexCoordP2uiv;
    using theBaseClass_t::glMultiTexCoordP3ui;
    using theBaseClass_t::glMultiTexCoordP3uiv;
    using theBaseClass_t::glMultiTexCoordP4ui;
    using theBaseClass_t::glMultiTexCoordP4uiv;
    using theBaseClass_t::glNormalP3ui;
    using theBaseClass_t::glNormalP3uiv;
    using theBaseClass_t::glColorP3ui;
    using theBaseClass_t::glColorP3uiv;
    using theBaseClass_t::glColorP4ui;
    using theBaseClass_t::glColorP4uiv;
    using theBaseClass_t::glSecondaryColorP3ui;
    using theBaseClass_t::glSecondaryColorP3uiv;
    using theBaseClass_t::glVertexAttribP1ui;
    using theBaseClass_t::glVertexAttribP1uiv;
    using theBaseClass_t::glVertexAttribP2ui;
    using theBaseClass_t::glVertexAttribP2uiv;
    using theBaseClass_t::glVertexAttribP3ui;
    using theBaseClass_t::glVertexAttribP3uiv;
    using theBaseClass_t::glVertexAttribP4ui;
    using theBaseClass_t::glVertexAttribP4uiv;

        public: //! @name OpenGL 3.3 additives to 3.2

    using theBaseClass_t::glVertexAttribDivisor;

#endif

};

/**
 * OpenGL 3.3 compatibility profile.
 */
typedef stglTmplCore33<StGLCore32Back> StGLCore33Back;

/**
 * OpenGL 3.3 core profile.
 */
typedef stglTmplCore33<StGLCore32>     StGLCore33;

#endif // __StGLCore33_h_
