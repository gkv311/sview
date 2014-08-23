/**
 * Copyright © 2013 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#ifndef __StGLCore44_h_
#define __StGLCore44_h_

#include <StGLCore/StGLCore43.h>

/**
 * OpenGL 4.4 definition.
 */
template<typename theBaseClass_t>
struct stglTmplCore44 : public theBaseClass_t {

#if !defined(GL_ES_VERSION_2_0)

        public: //! @name OpenGL 4.4 additives to 4.3

    using theBaseClass_t::glBufferStorage;
    using theBaseClass_t::glClearTexImage;
    using theBaseClass_t::glClearTexSubImage;
    using theBaseClass_t::glBindBuffersBase;
    using theBaseClass_t::glBindBuffersRange;
    using theBaseClass_t::glBindTextures;
    using theBaseClass_t::glBindSamplers;
    using theBaseClass_t::glBindImageTextures;
    using theBaseClass_t::glBindVertexBuffers;

#endif

};

/**
 * OpenGL 4.4 compatibility profile.
 */
typedef stglTmplCore44<StGLCore43Back> StGLCore44Back;

/**
 * OpenGL 4.4 core profile.
 */
typedef stglTmplCore44<StGLCore43>     StGLCore44;

#endif // __StGLCore44_h_
