/**
 * Copyright © 2012-2014 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#ifndef __StGLCore20_h_
#define __StGLCore20_h_

#include <StGLCore/StGLCore15.h>

/**
 * OpenGL 2.0 core based on 1.5 version.
 */
template<typename theBaseClass_t>
struct stglTmplCore20 : public theBaseClass_t {

        public: //! @name OpenGL 2.0 additives to 1.5

    using theBaseClass_t::glBlendEquationSeparate;
    using theBaseClass_t::glStencilOpSeparate;
    using theBaseClass_t::glStencilFuncSeparate;
    using theBaseClass_t::glStencilMaskSeparate;
    using theBaseClass_t::glAttachShader;
    using theBaseClass_t::glBindAttribLocation;
    using theBaseClass_t::glCompileShader;
    using theBaseClass_t::glCreateProgram;
    using theBaseClass_t::glCreateShader;
    using theBaseClass_t::glDeleteProgram;
    using theBaseClass_t::glDeleteShader;
    using theBaseClass_t::glDetachShader;
    using theBaseClass_t::glDisableVertexAttribArray;
    using theBaseClass_t::glEnableVertexAttribArray;
    using theBaseClass_t::glGetActiveAttrib;
    using theBaseClass_t::glGetActiveUniform;
    using theBaseClass_t::glGetAttachedShaders;
    using theBaseClass_t::glGetAttribLocation;
    using theBaseClass_t::glGetProgramiv;
    using theBaseClass_t::glGetProgramInfoLog;
    using theBaseClass_t::glGetShaderiv;
    using theBaseClass_t::glGetShaderInfoLog;
    using theBaseClass_t::glGetShaderSource;
    using theBaseClass_t::glGetUniformLocation;
    using theBaseClass_t::glGetUniformfv;
    using theBaseClass_t::glGetUniformiv;
    using theBaseClass_t::glGetVertexAttribfv;
    using theBaseClass_t::glGetVertexAttribiv;
    using theBaseClass_t::glGetVertexAttribPointerv;
    using theBaseClass_t::glIsProgram;
    using theBaseClass_t::glIsShader;
    using theBaseClass_t::glLinkProgram;
    using theBaseClass_t::glShaderSource;
    using theBaseClass_t::glUseProgram;
    using theBaseClass_t::glUniform1f;
    using theBaseClass_t::glUniform2f;
    using theBaseClass_t::glUniform3f;
    using theBaseClass_t::glUniform4f;
    using theBaseClass_t::glUniform1i;
    using theBaseClass_t::glUniform2i;
    using theBaseClass_t::glUniform3i;
    using theBaseClass_t::glUniform4i;
    using theBaseClass_t::glUniform1fv;
    using theBaseClass_t::glUniform2fv;
    using theBaseClass_t::glUniform3fv;
    using theBaseClass_t::glUniform4fv;
    using theBaseClass_t::glUniform1iv;
    using theBaseClass_t::glUniform2iv;
    using theBaseClass_t::glUniform3iv;
    using theBaseClass_t::glUniform4iv;
    using theBaseClass_t::glUniformMatrix2fv;
    using theBaseClass_t::glUniformMatrix3fv;
    using theBaseClass_t::glUniformMatrix4fv;
    using theBaseClass_t::glValidateProgram;
    using theBaseClass_t::glVertexAttrib1f;
    using theBaseClass_t::glVertexAttrib1fv;
    using theBaseClass_t::glVertexAttrib2f;
    using theBaseClass_t::glVertexAttrib2fv;
    using theBaseClass_t::glVertexAttrib3f;
    using theBaseClass_t::glVertexAttrib3fv;
    using theBaseClass_t::glVertexAttrib4f;
    using theBaseClass_t::glVertexAttrib4fv;
    using theBaseClass_t::glVertexAttribPointer;

#if !defined(GL_ES_VERSION_2_0)
    using theBaseClass_t::glDrawBuffers;
    using theBaseClass_t::glGetVertexAttribdv;
    using theBaseClass_t::glVertexAttrib1d;
    using theBaseClass_t::glVertexAttrib1dv;
    using theBaseClass_t::glVertexAttrib2d;
    using theBaseClass_t::glVertexAttrib2dv;
    using theBaseClass_t::glVertexAttrib3d;
    using theBaseClass_t::glVertexAttrib3dv;
    using theBaseClass_t::glVertexAttrib4d;
    using theBaseClass_t::glVertexAttrib4dv;
    using theBaseClass_t::glVertexAttrib1s;
    using theBaseClass_t::glVertexAttrib1sv;
    using theBaseClass_t::glVertexAttrib2s;
    using theBaseClass_t::glVertexAttrib2sv;
    using theBaseClass_t::glVertexAttrib3s;
    using theBaseClass_t::glVertexAttrib3sv;
    using theBaseClass_t::glVertexAttrib4s;
    using theBaseClass_t::glVertexAttrib4sv;
    using theBaseClass_t::glVertexAttrib4iv;
    using theBaseClass_t::glVertexAttrib4Nbv;
    using theBaseClass_t::glVertexAttrib4Niv;
    using theBaseClass_t::glVertexAttrib4Nsv;
    using theBaseClass_t::glVertexAttrib4Nub;
    using theBaseClass_t::glVertexAttrib4Nubv;
    using theBaseClass_t::glVertexAttrib4Nuiv;
    using theBaseClass_t::glVertexAttrib4Nusv;
    using theBaseClass_t::glVertexAttrib4bv;
    using theBaseClass_t::glVertexAttrib4ubv;
    using theBaseClass_t::glVertexAttrib4uiv;
    using theBaseClass_t::glVertexAttrib4usv;
#endif

};

/**
 * OpenGL 2.0 core based on 1.5 version.
 */
typedef stglTmplCore20<StGLCore15>    StGLCore20;

/**
 * OpenGL 2.0 without deprecated entry points.
 */
typedef stglTmplCore20<StGLCore15Fwd> StGLCore20Fwd;

#endif // __StGLCore20_h_
