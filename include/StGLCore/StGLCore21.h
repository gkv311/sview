/**
 * Copyright © 2012-2014 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#ifndef __StGLCore21_h_
#define __StGLCore21_h_

#include <StGLCore/StGLCore20.h>

/**
 * OpenGL 2.1 core based on 2.0 version.
 */
template<typename theBaseClass_t>
struct stglTmplCore21 : public theBaseClass_t {

        public: //! @name OpenGL 2.1 additives to 2.0

#if defined(GL_ES_VERSION_2_0) && !defined(GL_ES_VERSION_3_0)
    using theBaseClass_t::glUniformMatrix2x3fv;
    using theBaseClass_t::glUniformMatrix3x2fv;
    using theBaseClass_t::glUniformMatrix2x4fv;
    using theBaseClass_t::glUniformMatrix4x2fv;
    using theBaseClass_t::glUniformMatrix3x4fv;
    using theBaseClass_t::glUniformMatrix4x3fv;
#endif

};

/**
 * OpenGL 2.1 core based on 2.0 version.
 */
typedef stglTmplCore21<StGLCore20>    StGLCore21;

/**
 * OpenGL 2.1 without deprecated entry points.
 */
typedef stglTmplCore21<StGLCore20Fwd> StGLCore21Fwd;

#endif // __StGLCore21_h_
