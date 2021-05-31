/**
 * Copyright © 2012 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#ifndef __StGLArbFbo_h_
#define __StGLArbFbo_h_

#include <StGL/StGLFunctions.h>

/**
 * GL_ARB_framebuffer_object extension.
 */
struct StGLArbFbo : protected StGLFunctions {

        public: //! @name GL_ARB_framebuffer_object (added to OpenGL 3.0 core)

    using StGLFunctions::glIsRenderbuffer;
    using StGLFunctions::glBindRenderbuffer;
    using StGLFunctions::glDeleteRenderbuffers;
    using StGLFunctions::glGenRenderbuffers;
    using StGLFunctions::glRenderbufferStorage;
    using StGLFunctions::glGetRenderbufferParameteriv;
    using StGLFunctions::glIsFramebuffer;
    using StGLFunctions::glBindFramebuffer;
    using StGLFunctions::glDeleteFramebuffers;
    using StGLFunctions::glGenFramebuffers;
    using StGLFunctions::glCheckFramebufferStatus;
    using StGLFunctions::glFramebufferTexture2D;
    using StGLFunctions::glFramebufferRenderbuffer;
    using StGLFunctions::glGetFramebufferAttachmentParameteriv;
    using StGLFunctions::glGenerateMipmap;

#if !defined(GL_ES_VERSION_2_0)
    using StGLFunctions::glBlitFramebuffer;
    using StGLFunctions::glFramebufferTexture1D;
    using StGLFunctions::glFramebufferTexture3D;
    using StGLFunctions::glFramebufferTextureLayer;
    using StGLFunctions::glRenderbufferStorageMultisample;
#endif

};

#endif // __StGLArbFbo_h_
