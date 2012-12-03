/**
 * Copyright © 2012 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#ifndef __StGLCore12_h_
#define __StGLCore12_h_

#include <StGLCore/StGLCore11.h>

/**
 * OpenGL 1.2 core based on 1.1 version.
 */
template<typename theBaseClass_t>
struct ST_LOCAL stglTmplCore12 : public theBaseClass_t {

        public: //! @name OpenGL 1.2 additives to 1.1

    using theBaseClass_t::glBlendColor;
    using theBaseClass_t::glBlendEquation;
    using theBaseClass_t::glDrawRangeElements;
    using theBaseClass_t::glTexImage3D;
    using theBaseClass_t::glTexSubImage3D;
    using theBaseClass_t::glCopyTexSubImage3D;

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
