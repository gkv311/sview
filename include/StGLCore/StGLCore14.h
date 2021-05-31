/**
 * Copyright © 2012-2014 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#ifndef __StGLCore14_h_
#define __StGLCore14_h_

#include <StGLCore/StGLCore13.h>

/**
 * OpenGL 1.4 core based on 1.3 version.
 */
template<typename theBaseClass_t>
struct stglTmplCore14 : public theBaseClass_t {

        public: //! @name OpenGL 1.4 additives to 1.3

    using theBaseClass_t::glBlendFuncSeparate;
    using theBaseClass_t::glMultiDrawElements;

#if !defined(GL_ES_VERSION_2_0)
    using theBaseClass_t::glMultiDrawArrays;
    using theBaseClass_t::glPointParameterf;
    using theBaseClass_t::glPointParameterfv;
    using theBaseClass_t::glPointParameteri;
    using theBaseClass_t::glPointParameteriv;
#endif

};

/**
 * OpenGL 1.4 core based on 1.3 version.
 */
typedef stglTmplCore14<StGLCore13>    StGLCore14;

/**
 * OpenGL 1.4 without deprecated entry points.
 */
typedef stglTmplCore14<StGLCore13Fwd> StGLCore14Fwd;

#endif // __StGLCore14_h_
