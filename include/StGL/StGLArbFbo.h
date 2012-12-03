/**
 * Copyright © 2012 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#ifndef __StGLArbFbo_h_
#define __StGLArbFbo_h_

#include <StGL/StGLFunctions.h>

/**
 * GL_ARB_framebuffer_object extension.
 */
struct ST_LOCAL StGLArbFbo : protected StGLFunctions {

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
    using StGLFunctions::glFramebufferTexture1D;
    using StGLFunctions::glFramebufferTexture2D;
    using StGLFunctions::glFramebufferTexture3D;
    using StGLFunctions::glFramebufferRenderbuffer;
    using StGLFunctions::glGetFramebufferAttachmentParameteriv;
    using StGLFunctions::glGenerateMipmap;
    using StGLFunctions::glBlitFramebuffer;
    using StGLFunctions::glRenderbufferStorageMultisample;
    using StGLFunctions::glFramebufferTextureLayer;

};

#endif // __StGLArbFbo_h_
