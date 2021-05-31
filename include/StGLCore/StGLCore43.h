/**
 * Copyright © 2013 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#ifndef __StGLCore43_h_
#define __StGLCore43_h_

#include <StGLCore/StGLCore42.h>

/**
 * OpenGL 4.3 definition.
 */
template<typename theBaseClass_t>
struct stglTmplCore43 : public theBaseClass_t {

#if !defined(GL_ES_VERSION_2_0)

        public: //! @name OpenGL 4.3 additives to 4.2

    using theBaseClass_t::glClearBufferData;
    using theBaseClass_t::glClearBufferSubData;
    using theBaseClass_t::glDispatchCompute;
    using theBaseClass_t::glDispatchComputeIndirect;
    using theBaseClass_t::glCopyImageSubData;
    using theBaseClass_t::glFramebufferParameteri;
    using theBaseClass_t::glGetFramebufferParameteriv;
    using theBaseClass_t::glGetInternalformati64v;
    using theBaseClass_t::glInvalidateTexSubImage;
    using theBaseClass_t::glInvalidateTexImage;
    using theBaseClass_t::glInvalidateBufferSubData;
    using theBaseClass_t::glInvalidateBufferData;
    using theBaseClass_t::glInvalidateFramebuffer;
    using theBaseClass_t::glInvalidateSubFramebuffer;
    using theBaseClass_t::glMultiDrawArraysIndirect;
    using theBaseClass_t::glMultiDrawElementsIndirect;
    using theBaseClass_t::glGetProgramInterfaceiv;
    using theBaseClass_t::glGetProgramResourceIndex;
    using theBaseClass_t::glGetProgramResourceName;
    using theBaseClass_t::glGetProgramResourceiv;
    using theBaseClass_t::glGetProgramResourceLocation;
    using theBaseClass_t::glGetProgramResourceLocationIndex;
    using theBaseClass_t::glShaderStorageBlockBinding;
    using theBaseClass_t::glTexBufferRange;
    using theBaseClass_t::glTexStorage2DMultisample;
    using theBaseClass_t::glTexStorage3DMultisample;
    using theBaseClass_t::glTextureView;
    using theBaseClass_t::glBindVertexBuffer;
    using theBaseClass_t::glVertexAttribFormat;
    using theBaseClass_t::glVertexAttribIFormat;
    using theBaseClass_t::glVertexAttribLFormat;
    using theBaseClass_t::glVertexAttribBinding;
    using theBaseClass_t::glVertexBindingDivisor;
    using theBaseClass_t::glDebugMessageControl;
    using theBaseClass_t::glDebugMessageInsert;
    using theBaseClass_t::glDebugMessageCallback;
    using theBaseClass_t::glGetDebugMessageLog;
    using theBaseClass_t::glPushDebugGroup;
    using theBaseClass_t::glPopDebugGroup;
    using theBaseClass_t::glObjectLabel;
    using theBaseClass_t::glGetObjectLabel;
    using theBaseClass_t::glObjectPtrLabel;
    using theBaseClass_t::glGetObjectPtrLabel;

#endif

};

/**
 * OpenGL 4.3 compatibility profile.
 */
typedef stglTmplCore43<StGLCore42Back> StGLCore43Back;

/**
 * OpenGL 4.3 core profile.
 */
typedef stglTmplCore43<StGLCore42>     StGLCore43;

#endif // __StGLCore43_h_
