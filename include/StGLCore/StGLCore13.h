/**
 * Copyright © 2012 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#ifndef __StGLCore13_h_
#define __StGLCore13_h_

#include <StGLCore/StGLCore12.h>

/**
 * OpenGL 1.3 without deprecated entry points.
 */
struct StGLCore13Fwd : public StGLCore12Fwd {

        public: //! @name OpenGL 1.3 additives to 1.2

#if !defined(GL_ES_VERSION_2_0)
    using StGLFunctions::glCompressedTexImage3D;
    using StGLFunctions::glCompressedTexImage1D;
    using StGLFunctions::glCompressedTexSubImage3D;
    using StGLFunctions::glCompressedTexSubImage1D;
    using StGLFunctions::glGetCompressedTexImage;
#endif

    using StGLFunctions::glActiveTexture;
    using StGLFunctions::glSampleCoverage;
    using StGLFunctions::glCompressedTexImage2D;
    using StGLFunctions::glCompressedTexSubImage2D;

};

/**
 * OpenGL 1.3 core based on 1.2 version.
 */
struct StGLCore13 : public StGLCore12 {

#if !defined(GL_ES_VERSION_2_0)

        public: //! @name OpenGL 1.3 additives to 1.2

    using StGLFunctions::glActiveTexture;
    using StGLFunctions::glSampleCoverage;
    using StGLFunctions::glCompressedTexImage3D;
    using StGLFunctions::glCompressedTexImage2D;
    using StGLFunctions::glCompressedTexImage1D;
    using StGLFunctions::glCompressedTexSubImage3D;
    using StGLFunctions::glCompressedTexSubImage2D;
    using StGLFunctions::glCompressedTexSubImage1D;
    using StGLFunctions::glGetCompressedTexImage;

        public: //! @name Begin/End primitive specification (removed since 3.1)

    using StGLFunctions::glMultiTexCoord1d;
    using StGLFunctions::glMultiTexCoord1dv;
    using StGLFunctions::glMultiTexCoord1f;
    using StGLFunctions::glMultiTexCoord1fv;
    using StGLFunctions::glMultiTexCoord1i;
    using StGLFunctions::glMultiTexCoord1iv;
    using StGLFunctions::glMultiTexCoord1s;
    using StGLFunctions::glMultiTexCoord1sv;
    using StGLFunctions::glMultiTexCoord2d;
    using StGLFunctions::glMultiTexCoord2dv;
    using StGLFunctions::glMultiTexCoord2f;
    using StGLFunctions::glMultiTexCoord2fv;
    using StGLFunctions::glMultiTexCoord2i;
    using StGLFunctions::glMultiTexCoord2iv;
    using StGLFunctions::glMultiTexCoord2s;
    using StGLFunctions::glMultiTexCoord2sv;
    using StGLFunctions::glMultiTexCoord3d;
    using StGLFunctions::glMultiTexCoord3dv;
    using StGLFunctions::glMultiTexCoord3f;
    using StGLFunctions::glMultiTexCoord3fv;
    using StGLFunctions::glMultiTexCoord3i;
    using StGLFunctions::glMultiTexCoord3iv;
    using StGLFunctions::glMultiTexCoord3s;
    using StGLFunctions::glMultiTexCoord3sv;
    using StGLFunctions::glMultiTexCoord4d;
    using StGLFunctions::glMultiTexCoord4dv;
    using StGLFunctions::glMultiTexCoord4f;
    using StGLFunctions::glMultiTexCoord4fv;
    using StGLFunctions::glMultiTexCoord4i;
    using StGLFunctions::glMultiTexCoord4iv;
    using StGLFunctions::glMultiTexCoord4s;
    using StGLFunctions::glMultiTexCoord4sv;

    using StGLFunctions::glClientActiveTexture;

        public: //! @name Matrix operations (removed since 3.1)

    using StGLFunctions::glLoadTransposeMatrixf;
    using StGLFunctions::glLoadTransposeMatrixd;
    using StGLFunctions::glMultTransposeMatrixf;
    using StGLFunctions::glMultTransposeMatrixd;

#endif

};

#endif // __StGLCore13_h_
