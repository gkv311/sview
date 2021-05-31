/**
 * Copyright © 2012-2014 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#ifndef __StGLCore12_h_
#define __StGLCore12_h_

#include <StGLCore/StGLCore11.h>

/**
 * OpenGL 1.2 core based on 1.1 version.
 */
template<typename theBaseClass_t>
struct stglTmplCore12 : public theBaseClass_t {

        public: //! @name OpenGL 1.2 additives to 1.1

    using theBaseClass_t::glBlendColor;
    using theBaseClass_t::glBlendEquation;

#if !defined(GL_ES_VERSION_2_0)
    using theBaseClass_t::glDrawRangeElements;
    using theBaseClass_t::glTexImage3D;
    using theBaseClass_t::glTexSubImage3D;
    using theBaseClass_t::glCopyTexSubImage3D;
#endif

};

/**
 * OpenGL 1.2 core based on 1.1 version.
 */
typedef stglTmplCore12<StGLCore11>    StGLCore12;

/**
 * OpenGL 1.2 without deprecated entry points.
 */
typedef stglTmplCore12<StGLCore11Fwd> StGLCore12Fwd;

#endif // __StGLCore12_h_
