/**
 * Copyright © 2012-2014 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#ifndef __StGLFunctions_h_
#define __StGLFunctions_h_

#ifdef ST_HAVE_STCONFIG
    #include <stconfig.conf>
#endif

// required for correct APIENTRY definition
#if(defined(_WIN32) && !defined(APIENTRY) && !defined(__CYGWIN__) && !defined(__SCITECH_SNAP__))
    #define WIN32_LEAN_AND_MEAN
    #include <windows.h>
#endif

#ifndef APIENTRY
    #define APIENTRY
#endif
#ifndef APIENTRYP
    #define APIENTRYP APIENTRY *
#endif
#ifndef GLAPI
    #define GLAPI extern
#endif

// exclude modern definitions - use own glext.h header!
#define GL_GLEXT_LEGACY

#ifndef __glext_h_
    #define __glext_h_
    #define ST_GLEXT_PRE
#endif

// include main OpenGL header provided with system
#if defined(__APPLE__)
    #define GL_SILENCE_DEPRECATION
    #include <OpenGL/gl.h>
#elif defined(ST_HAVE_GLES2) || defined(__ANDROID__)
    #include <GLES2/gl2.h>
    ///#include <GLES3/gl3.h>

    // in core since OpenGL ES 3.0
    #define GL_UNPACK_ROW_LENGTH  0x0CF2
    #define GL_UNPACK_SKIP_ROWS   0x0CF3
    #define GL_UNPACK_SKIP_PIXELS 0x0CF4
    #define GL_PACK_ROW_LENGTH    0x0D02
    #define GL_PACK_SKIP_ROWS     0x0D03
    #define GL_PACK_SKIP_PIXELS   0x0D04
    #define GL_DEPTH24_STENCIL8   0x88F0

    // in core since OpenGL ES 3.0, extension GL_EXT_texture_rg
    #define GL_RED   0x1903
    #define GL_R8    0x8229
    // in core since OpenGL ES 3.0, extension GL_OES_rgb8_rgba8
    #define GL_RGB8  0x8051
    #define GL_RGBA8 0x8058
    // GL_EXT_texture_format_BGRA8888
    #define GL_BGRA_EXT 0x80E1 // same as GL_BGRA on desktop
    // in core since OpenGL ES 3.0, extension GL_OES_required_internalformat
    #define GL_RGB10_A2 0x8059

    // debug ARB extension
    #define GL_DEBUG_OUTPUT                   0x92E0
    #define GL_DEBUG_OUTPUT_SYNCHRONOUS       0x8242
    #define GL_DEBUG_NEXT_LOGGED_MESSAGE_LENGTH 0x8243
    #define GL_DEBUG_CALLBACK_FUNCTION        0x8244
    #define GL_DEBUG_CALLBACK_USER_PARAM      0x8245
    #define GL_DEBUG_SOURCE_API               0x8246
    #define GL_DEBUG_SOURCE_WINDOW_SYSTEM     0x8247
    #define GL_DEBUG_SOURCE_SHADER_COMPILER   0x8248
    #define GL_DEBUG_SOURCE_THIRD_PARTY       0x8249
    #define GL_DEBUG_SOURCE_APPLICATION       0x824A
    #define GL_DEBUG_SOURCE_OTHER             0x824B
    #define GL_DEBUG_TYPE_ERROR               0x824C
    #define GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR 0x824D
    #define GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR  0x824E
    #define GL_DEBUG_TYPE_PORTABILITY         0x824F
    #define GL_DEBUG_TYPE_PERFORMANCE         0x8250
    #define GL_DEBUG_TYPE_OTHER               0x8251
    #define GL_MAX_DEBUG_MESSAGE_LENGTH       0x9143
    #define GL_MAX_DEBUG_LOGGED_MESSAGES      0x9144
    #define GL_DEBUG_LOGGED_MESSAGES          0x9145
    #define GL_DEBUG_SEVERITY_HIGH            0x9146
    #define GL_DEBUG_SEVERITY_MEDIUM          0x9147
    #define GL_DEBUG_SEVERITY_LOW             0x9148
#else
    #include <GL/gl.h>
#endif

#ifdef ST_GLEXT_PRE
    #undef __glext_h_
#endif

#ifndef GL_ES_VERSION_2_0
    // GL version can be defined by system gl.h header
    #undef GL_VERSION_1_2
    #undef GL_VERSION_1_3
    #undef GL_VERSION_1_4
    #undef GL_VERSION_1_5
    #undef GL_VERSION_2_0
    #undef GL_VERSION_2_1
    #undef GL_VERSION_3_0
    #undef GL_VERSION_3_1
    #undef GL_VERSION_3_2
    #undef GL_VERSION_3_3
    #undef GL_VERSION_4_0
    #undef GL_VERSION_4_1
    #undef GL_VERSION_4_2
    #undef GL_VERSION_4_3
    #undef GL_VERSION_4_4

    #include <StGL/StGLExt.h>
#endif

#include <stTypes.h>

/**
 * Mega structure defines the list of ALL OpenGL functions!
 */
struct StGLFunctions {

#if defined(GL_ES_VERSION_2_0)

        public: //! @name OpenGL ES 1.1

    ST_LOCAL inline
    void glActiveTexture(GLenum texture) {
        ::glActiveTexture(texture);
    }

    ST_LOCAL inline
    void glCompressedTexImage2D(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const GLvoid *data) {
        ::glCompressedTexImage2D(target, level, internalformat, width, height, border, imageSize, data);
    }

    ST_LOCAL inline
    void glCompressedTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const GLvoid *data) {
        ::glCompressedTexSubImage2D(target, level, xoffset, yoffset, width, height, format, imageSize, data);
    }

        public: //! @name OpenGL ES 2.0

    ST_LOCAL inline
    void glBlendColor(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha) {
        ::glBlendColor(red, green, blue, alpha);
    }

    ST_LOCAL inline
    void glBlendEquation(GLenum mode) {
        ::glBlendEquation(mode);
    }

    ST_LOCAL inline
    void glBlendFuncSeparate(GLenum sfactorRGB, GLenum dfactorRGB, GLenum sfactorAlpha, GLenum dfactorAlpha) {
        ::glBlendFuncSeparate(sfactorRGB, dfactorRGB, sfactorAlpha, dfactorAlpha);
    }

    ST_LOCAL inline
    void glBlendEquationSeparate(GLenum modeRGB, GLenum modeAlpha) {
        ::glBlendEquationSeparate(modeRGB, modeAlpha);
    }

    ST_LOCAL inline
    void glStencilOpSeparate(GLenum face, GLenum sfail, GLenum dpfail, GLenum dppass) {
        ::glStencilOpSeparate(face, sfail, dpfail, dppass);
    }

    ST_LOCAL inline
    void glStencilFuncSeparate(GLenum face, GLenum func, GLint ref, GLuint mask) {
        ::glStencilFuncSeparate(face, func, ref, mask);
    }

    ST_LOCAL inline
    void glStencilMaskSeparate(GLenum face, GLuint mask) {
        ::glStencilMaskSeparate(face, mask);
    }

    ST_LOCAL inline
    void glAttachShader(GLuint program, GLuint shader) {
        ::glAttachShader(program, shader);
    }

    ST_LOCAL inline
    void glBindAttribLocation(GLuint program, GLuint index, const GLchar *name) {
        ::glBindAttribLocation(program, index, name);
    }

    ST_LOCAL inline
    void glBindBuffer(GLenum target, GLuint buffer) {
        ::glBindBuffer(target, buffer);
    }

    ST_LOCAL inline
    void glBindFramebuffer(GLenum target, GLuint framebuffer) {
        ::glBindFramebuffer(target, framebuffer);
    }

    ST_LOCAL inline
    void glBindRenderbuffer(GLenum target, GLuint renderbuffer) {
        ::glBindRenderbuffer(target, renderbuffer);
    }

    ST_LOCAL inline
    void glBufferData(GLenum target, GLsizeiptr size, const void* data, GLenum usage) {
        ::glBufferData(target, size, data, usage);
    }

    ST_LOCAL inline
    void glBufferSubData(GLenum target, GLintptr offset, GLsizeiptr size, const void* data) {
        ::glBufferSubData(target, offset, size, data);
    }

    ST_LOCAL inline
    GLenum glCheckFramebufferStatus(GLenum target) {
        return ::glCheckFramebufferStatus(target);
    }

    ST_LOCAL inline
    void glCompileShader(GLuint shader) {
        ::glCompileShader(shader);
    }

    ST_LOCAL inline
    GLuint glCreateProgram() {
        return ::glCreateProgram();
    }

    ST_LOCAL inline
    GLuint glCreateShader(GLenum type) {
        return ::glCreateShader(type);
    }

    ST_LOCAL inline
    void glDeleteBuffers(GLsizei n, const GLuint *buffers) {
        ::glDeleteBuffers(n, buffers);
    }

    ST_LOCAL inline
    void glDeleteFramebuffers(GLsizei n, const GLuint *framebuffers) {
        ::glDeleteFramebuffers(n, framebuffers);
    }

    ST_LOCAL inline
    void glDeleteProgram(GLuint program) {
        ::glDeleteProgram(program);
    }

    ST_LOCAL inline
    void glDeleteRenderbuffers(GLsizei n, const GLuint *renderbuffers) {
        ::glDeleteRenderbuffers(n, renderbuffers);
    }

    ST_LOCAL inline
    void glDeleteShader(GLuint shader) {
        ::glDeleteShader(shader);
    }

    ST_LOCAL inline
    void glDeleteTextures(GLsizei n, const GLuint *textures) {
        ::glDeleteTextures(n, textures);
    }

    ST_LOCAL inline
    void glDepthFunc(GLenum func) {
        ::glDepthFunc(func);
    }

    ST_LOCAL inline
    void glDepthMask(GLboolean flag) {
        ::glDepthMask(flag);
    }

    ST_LOCAL inline
    void glDepthRangef(GLfloat n, GLfloat f) {
        ::glDepthRangef(n, f);
    }

    ST_LOCAL inline
    void glDetachShader(GLuint program, GLuint shader) {
        ::glDetachShader(program, shader);
    }

    ST_LOCAL inline
    void glDisableVertexAttribArray(GLuint index) {
        ::glDisableVertexAttribArray(index);
    }

    ST_LOCAL inline
    void glEnableVertexAttribArray(GLuint index) {
        ::glEnableVertexAttribArray(index);
    }

    ST_LOCAL inline
    void glFramebufferRenderbuffer(GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer) {
        ::glFramebufferRenderbuffer(target, attachment, renderbuffertarget, renderbuffer);
    }

    ST_LOCAL inline
    void glFramebufferTexture2D(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level) {
        ::glFramebufferTexture2D(target, attachment, textarget, texture, level);
    }

    ST_LOCAL inline
    void glGenBuffers(GLsizei n, GLuint *buffers) {
        ::glGenBuffers(n, buffers);
    }

    ST_LOCAL inline
    void glGenerateMipmap(GLenum target) {
        ::glGenerateMipmap(target);
    }

    ST_LOCAL inline
    void glGenFramebuffers(GLsizei n, GLuint *framebuffers) {
        ::glGenFramebuffers(n, framebuffers);
    }

    ST_LOCAL inline
    void glGenRenderbuffers(GLsizei n, GLuint *renderbuffers) {
        ::glGenRenderbuffers(n, renderbuffers);
    }

    ST_LOCAL inline
    void glGenTextures(GLsizei n, GLuint *textures) {
        ::glGenTextures(n, textures);
    }

    ST_LOCAL inline
    void glGetActiveAttrib(GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLint* size, GLenum *type, GLchar *name) {
        ::glGetActiveAttrib(program, index, bufSize, length, size, type, name);
    }

    ST_LOCAL inline
    void glGetActiveUniform(GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLint* size, GLenum *type, GLchar *name) {
        ::glGetActiveUniform(program, index, bufSize, length, size, type, name);
    }

    ST_LOCAL inline
    void glGetAttachedShaders(GLuint program, GLsizei maxCount, GLsizei *count, GLuint *shaders) {
        ::glGetAttachedShaders(program, maxCount, count, shaders);
    }

    ST_LOCAL inline
    GLint glGetAttribLocation(GLuint program, const GLchar *name) {
        return ::glGetAttribLocation(program, name);
    }

    ST_LOCAL inline
    void glGetBufferParameteriv(GLenum target, GLenum pname, GLint* params) {
        ::glGetBufferParameteriv(target, pname, params);
    }

    ST_LOCAL inline
    void glGetFramebufferAttachmentParameteriv(GLenum target, GLenum attachment, GLenum pname, GLint* params) {
        ::glGetFramebufferAttachmentParameteriv(target, attachment, pname, params);
    }

    ST_LOCAL inline
    void glGetProgramiv(GLuint program, GLenum pname, GLint* params) {
        ::glGetProgramiv(program, pname, params);
    }

    ST_LOCAL inline
    void glGetProgramInfoLog(GLuint program, GLsizei bufSize, GLsizei *length, GLchar *infoLog) {
        ::glGetProgramInfoLog(program, bufSize, length, infoLog);
    }

    ST_LOCAL inline
    void glGetRenderbufferParameteriv(GLenum target, GLenum pname, GLint* params) {
        ::glGetRenderbufferParameteriv(target, pname, params);
    }

    ST_LOCAL inline
    void glGetShaderiv(GLuint shader, GLenum pname, GLint* params) {
        ::glGetShaderiv(shader, pname, params);
    }

    ST_LOCAL inline
    void glGetShaderInfoLog(GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *infoLog) {
        ::glGetShaderInfoLog(shader, bufSize, length, infoLog);
    }

    ST_LOCAL inline
    void glGetShaderPrecisionFormat(GLenum shadertype, GLenum precisiontype, GLint* range, GLint* precision) {
        ::glGetShaderPrecisionFormat(shadertype, precisiontype, range, precision);
    }

    ST_LOCAL inline
    void glGetShaderSource(GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *source) {
        ::glGetShaderSource(shader, bufSize, length, source);
    }

    ST_LOCAL inline
    void glGetUniformfv(GLuint program, GLint location, GLfloat* params) {
        ::glGetUniformfv(program, location, params);
    }

    ST_LOCAL inline
    void glGetUniformiv(GLuint program, GLint location, GLint* params) {
        ::glGetUniformiv(program, location, params);
    }

    GLint glGetUniformLocation(GLuint program, const GLchar *name) {
        return ::glGetUniformLocation(program, name);
    }

    ST_LOCAL inline
    void glGetVertexAttribfv(GLuint index, GLenum pname, GLfloat* params) {
        ::glGetVertexAttribfv(index, pname, params);
    }

    ST_LOCAL inline
    void glGetVertexAttribiv(GLuint index, GLenum pname, GLint* params) {
        ::glGetVertexAttribiv(index, pname, params);
    }

    ST_LOCAL inline
    void glGetVertexAttribPointerv(GLuint index, GLenum pname, void* *pointer) {
        ::glGetVertexAttribPointerv(index, pname, pointer);
    }

    ST_LOCAL inline
    GLboolean glIsBuffer(GLuint buffer) {
        return ::glIsBuffer(buffer);
    }

    ST_LOCAL inline
    GLboolean glIsFramebuffer(GLuint framebuffer) {
        return ::glIsFramebuffer(framebuffer);
    }

    ST_LOCAL inline
    GLboolean glIsProgram(GLuint program) {
        return ::glIsProgram(program);
    }

    ST_LOCAL inline
    GLboolean glIsRenderbuffer(GLuint renderbuffer) {
        return ::glIsRenderbuffer(renderbuffer);
    }

    ST_LOCAL inline
    GLboolean glIsShader(GLuint shader) {
        return ::glIsShader(shader);
    }

    ST_LOCAL inline
    void glLinkProgram(GLuint program) {
        ::glLinkProgram(program);
    }

    ST_LOCAL inline
    void glReleaseShaderCompiler() {
        ::glReleaseShaderCompiler();
    }

    ST_LOCAL inline
    void glRenderbufferStorage(GLenum target, GLenum internalformat, GLsizei width, GLsizei height) {
        ::glRenderbufferStorage(target, internalformat, width, height);
    }

    ST_LOCAL inline
    void glSampleCoverage(GLfloat value, GLboolean invert) {
        ::glSampleCoverage(value, invert);
    }

    ST_LOCAL inline
    void glShaderBinary(GLsizei count, const GLuint *shaders, GLenum binaryformat, const void* binary, GLsizei length) {
        ::glShaderBinary(count, shaders, binaryformat, binary, length);
    }

    ST_LOCAL inline
    void glShaderSource(GLuint shader, GLsizei count, const GLchar** string, const GLint* length) {
        ::glShaderSource(shader, count, string, length);
    }

    ST_LOCAL inline
    void glUniform1f(GLint location, GLfloat v0) {
        ::glUniform1f(location, v0);
    }

    ST_LOCAL inline
    void glUniform1fv(GLint location, GLsizei count, const GLfloat* value) {
        ::glUniform1fv(location, count, value);
    }

    ST_LOCAL inline
    void glUniform1i(GLint location, GLint v0) {
        ::glUniform1i(location, v0);
    }

    ST_LOCAL inline
    void glUniform1iv(GLint location, GLsizei count, const GLint* value) {
        ::glUniform1iv(location, count, value);
    }

    ST_LOCAL inline
    void glUniform2f(GLint location, GLfloat v0, GLfloat v1) {
        ::glUniform2f(location, v0, v1);
    }

    ST_LOCAL inline
    void glUniform2fv(GLint location, GLsizei count, const GLfloat* value) {
        ::glUniform2fv(location, count, value);
    }

    ST_LOCAL inline
    void glUniform2i(GLint location, GLint v0, GLint v1) {
        ::glUniform2i(location, v0, v1);
    }

    ST_LOCAL inline
    void glUniform2iv(GLint location, GLsizei count, const GLint* value) {
        ::glUniform2iv(location, count, value);
    }

    ST_LOCAL inline
    void glUniform3f(GLint location, GLfloat v0, GLfloat v1, GLfloat v2) {
        ::glUniform3f(location, v0, v1, v2);
    }

    ST_LOCAL inline
    void glUniform3fv(GLint location, GLsizei count, const GLfloat* value) {
        ::glUniform3fv(location, count, value);
    }

    ST_LOCAL inline
    void glUniform3i(GLint location, GLint v0, GLint v1, GLint v2) {
        ::glUniform3i(location, v0, v1, v2);
    }

    ST_LOCAL inline
    void glUniform3iv(GLint location, GLsizei count, const GLint* value) {
        ::glUniform3iv(location, count, value);
    }

    ST_LOCAL inline
    void glUniform4f(GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3) {
        ::glUniform4f(location, v0, v1, v2, v3);
    }

    ST_LOCAL inline
    void glUniform4fv(GLint location, GLsizei count, const GLfloat* value) {
        ::glUniform4fv(location, count, value);
    }

    ST_LOCAL inline
    void glUniform4i(GLint location, GLint v0, GLint v1, GLint v2, GLint v3) {
        ::glUniform4i(location, v0, v1, v2, v3);
    }

    ST_LOCAL inline
    void glUniform4iv(GLint location, GLsizei count, const GLint* value) {
        ::glUniform4iv(location, count, value);
    }

    ST_LOCAL inline
    void glUniformMatrix2fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value) {
        ::glUniformMatrix2fv(location, count, transpose, value);
    }

    ST_LOCAL inline
    void glUniformMatrix3fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value) {
        ::glUniformMatrix3fv(location, count, transpose, value);
    }

    ST_LOCAL inline
    void glUniformMatrix4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value) {
        ::glUniformMatrix4fv(location, count, transpose, value);
    }

    ST_LOCAL inline
    void glUseProgram(GLuint program) {
        ::glUseProgram(program);
    }

    ST_LOCAL inline
    void glValidateProgram(GLuint program) {
        ::glValidateProgram(program);
    }

    ST_LOCAL inline
    void glVertexAttrib1f(GLuint index, GLfloat x) {
        ::glVertexAttrib1f(index, x);
    }

    ST_LOCAL inline
    void glVertexAttrib1fv(GLuint index, const GLfloat* v) {
        ::glVertexAttrib1fv(index, v);
    }

    ST_LOCAL inline
    void glVertexAttrib2f(GLuint index, GLfloat x, GLfloat y) {
        ::glVertexAttrib2f(index, x, y);
    }

    ST_LOCAL inline
    void glVertexAttrib2fv(GLuint index, const GLfloat* v) {
        ::glVertexAttrib2fv(index, v);
    }

    ST_LOCAL inline
    void glVertexAttrib3f(GLuint index, GLfloat x, GLfloat y, GLfloat z) {
        ::glVertexAttrib3f(index, x, y, z);
    }

    ST_LOCAL inline
    void glVertexAttrib3fv(GLuint index, const GLfloat* v) {
        ::glVertexAttrib3fv(index, v);
    }

    ST_LOCAL inline
    void glVertexAttrib4f(GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w) {
        ::glVertexAttrib4f(index, x, y, z, w);
    }

    ST_LOCAL inline
    void glVertexAttrib4fv(GLuint index, const GLfloat* v) {
        ::glVertexAttrib4fv(index, v);
    }

    ST_LOCAL inline
    void glVertexAttribPointer(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void* pointer) {
        ::glVertexAttribPointer(index, size, type, normalized, stride, pointer);
    }

    ST_LOCAL inline
    void glMultiDrawElements(GLenum theMode, const GLsizei* theCount, GLenum theType, const void* const* theIndices, GLsizei theDrawCount) {
        if(theCount   == NULL
        || theIndices == NULL) {
            return;
        }

        for(GLsizei aBatchIter = 0; aBatchIter < theDrawCount; ++aBatchIter) {
            ::glDrawElements(theMode, theCount[aBatchIter], theType, theIndices[aBatchIter]);
        }
    }

        public: //! @name GL_KHR_debug (optional)

    typedef void   (APIENTRY  *GLDEBUGPROCARB)(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam);

    typedef void   (APIENTRYP glDebugMessageControl_t ) (GLenum source, GLenum type, GLenum severity, GLsizei count, const GLuint* ids, GLboolean enabled);
    typedef void   (APIENTRYP glDebugMessageInsert_t  ) (GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* buf);
    typedef void   (APIENTRYP glDebugMessageCallback_t) (GLDEBUGPROCARB callback, const void* userParam);
    typedef GLuint (APIENTRYP glGetDebugMessageLog_t  ) (GLuint   count,
                                                         GLsizei  bufSize,
                                                         GLenum*  sources,
                                                         GLenum*  types,
                                                         GLuint*  ids,
                                                         GLenum*  severities,
                                                         GLsizei* lengths,
                                                         GLchar*  messageLog);

    glDebugMessageControl_t  glDebugMessageControl;
    glDebugMessageInsert_t   glDebugMessageInsert;
    glDebugMessageCallback_t glDebugMessageCallback;
    glGetDebugMessageLog_t   glGetDebugMessageLog;

#else

        public: //! @name OpenGL 1.2

    PFNGLBLENDCOLORPROC               glBlendColor;
    PFNGLBLENDEQUATIONPROC            glBlendEquation;
    PFNGLDRAWRANGEELEMENTSPROC        glDrawRangeElements;
    PFNGLTEXIMAGE3DPROC               glTexImage3D;
    PFNGLTEXSUBIMAGE3DPROC            glTexSubImage3D;
    PFNGLCOPYTEXSUBIMAGE3DPROC        glCopyTexSubImage3D;

        public: //! @name OpenGL 1.3

    PFNGLACTIVETEXTUREPROC            glActiveTexture;
    PFNGLSAMPLECOVERAGEPROC           glSampleCoverage;
    PFNGLCOMPRESSEDTEXIMAGE3DPROC     glCompressedTexImage3D;
    PFNGLCOMPRESSEDTEXIMAGE2DPROC     glCompressedTexImage2D;
    PFNGLCOMPRESSEDTEXIMAGE1DPROC     glCompressedTexImage1D;
    PFNGLCOMPRESSEDTEXSUBIMAGE3DPROC  glCompressedTexSubImage3D;
    PFNGLCOMPRESSEDTEXSUBIMAGE2DPROC  glCompressedTexSubImage2D;
    PFNGLCOMPRESSEDTEXSUBIMAGE1DPROC  glCompressedTexSubImage1D;
    PFNGLGETCOMPRESSEDTEXIMAGEPROC    glGetCompressedTexImage;

    PFNGLCLIENTACTIVETEXTUREPROC      glClientActiveTexture;
    PFNGLMULTITEXCOORD1DPROC          glMultiTexCoord1d;
    PFNGLMULTITEXCOORD1DVPROC         glMultiTexCoord1dv;
    PFNGLMULTITEXCOORD1FPROC          glMultiTexCoord1f;
    PFNGLMULTITEXCOORD1FVPROC         glMultiTexCoord1fv;
    PFNGLMULTITEXCOORD1IPROC          glMultiTexCoord1i;
    PFNGLMULTITEXCOORD1IVPROC         glMultiTexCoord1iv;
    PFNGLMULTITEXCOORD1SPROC          glMultiTexCoord1s;
    PFNGLMULTITEXCOORD1SVPROC         glMultiTexCoord1sv;
    PFNGLMULTITEXCOORD2DPROC          glMultiTexCoord2d;
    PFNGLMULTITEXCOORD2DVPROC         glMultiTexCoord2dv;
    PFNGLMULTITEXCOORD2FPROC          glMultiTexCoord2f;
    PFNGLMULTITEXCOORD2FVPROC         glMultiTexCoord2fv;
    PFNGLMULTITEXCOORD2IPROC          glMultiTexCoord2i;
    PFNGLMULTITEXCOORD2IVPROC         glMultiTexCoord2iv;
    PFNGLMULTITEXCOORD2SPROC          glMultiTexCoord2s;
    PFNGLMULTITEXCOORD2SVPROC         glMultiTexCoord2sv;
    PFNGLMULTITEXCOORD3DPROC          glMultiTexCoord3d;
    PFNGLMULTITEXCOORD3DVPROC         glMultiTexCoord3dv;
    PFNGLMULTITEXCOORD3FPROC          glMultiTexCoord3f;
    PFNGLMULTITEXCOORD3FVPROC         glMultiTexCoord3fv;
    PFNGLMULTITEXCOORD3IPROC          glMultiTexCoord3i;
    PFNGLMULTITEXCOORD3IVPROC         glMultiTexCoord3iv;
    PFNGLMULTITEXCOORD3SPROC          glMultiTexCoord3s;
    PFNGLMULTITEXCOORD3SVPROC         glMultiTexCoord3sv;
    PFNGLMULTITEXCOORD4DPROC          glMultiTexCoord4d;
    PFNGLMULTITEXCOORD4DVPROC         glMultiTexCoord4dv;
    PFNGLMULTITEXCOORD4FPROC          glMultiTexCoord4f;
    PFNGLMULTITEXCOORD4FVPROC         glMultiTexCoord4fv;
    PFNGLMULTITEXCOORD4IPROC          glMultiTexCoord4i;
    PFNGLMULTITEXCOORD4IVPROC         glMultiTexCoord4iv;
    PFNGLMULTITEXCOORD4SPROC          glMultiTexCoord4s;
    PFNGLMULTITEXCOORD4SVPROC         glMultiTexCoord4sv;
    PFNGLLOADTRANSPOSEMATRIXFPROC     glLoadTransposeMatrixf;
    PFNGLLOADTRANSPOSEMATRIXDPROC     glLoadTransposeMatrixd;
    PFNGLMULTTRANSPOSEMATRIXFPROC     glMultTransposeMatrixf;
    PFNGLMULTTRANSPOSEMATRIXDPROC     glMultTransposeMatrixd;

        public: //! @name OpenGL 1.4

    PFNGLBLENDFUNCSEPARATEPROC        glBlendFuncSeparate;
    PFNGLMULTIDRAWARRAYSPROC          glMultiDrawArrays;
    PFNGLMULTIDRAWELEMENTSPROC        glMultiDrawElements;
    PFNGLPOINTPARAMETERFPROC          glPointParameterf;
    PFNGLPOINTPARAMETERFVPROC         glPointParameterfv;
    PFNGLPOINTPARAMETERIPROC          glPointParameteri;
    PFNGLPOINTPARAMETERIVPROC         glPointParameteriv;

        public: //! @name OpenGL 1.5

    PFNGLGENQUERIESPROC               glGenQueries;
    PFNGLDELETEQUERIESPROC            glDeleteQueries;
    PFNGLISQUERYPROC                  glIsQuery;
    PFNGLBEGINQUERYPROC               glBeginQuery;
    PFNGLENDQUERYPROC                 glEndQuery;
    PFNGLGETQUERYIVPROC               glGetQueryiv;
    PFNGLGETQUERYOBJECTIVPROC         glGetQueryObjectiv;
    PFNGLGETQUERYOBJECTUIVPROC        glGetQueryObjectuiv;
    PFNGLBINDBUFFERPROC               glBindBuffer;
    PFNGLDELETEBUFFERSPROC            glDeleteBuffers;
    PFNGLGENBUFFERSPROC               glGenBuffers;
    PFNGLISBUFFERPROC                 glIsBuffer;
    PFNGLBUFFERDATAPROC               glBufferData;
    PFNGLBUFFERSUBDATAPROC            glBufferSubData;
    PFNGLGETBUFFERSUBDATAPROC         glGetBufferSubData;
    PFNGLMAPBUFFERPROC                glMapBuffer;
    PFNGLUNMAPBUFFERPROC              glUnmapBuffer;
    PFNGLGETBUFFERPARAMETERIVPROC     glGetBufferParameteriv;
    PFNGLGETBUFFERPOINTERVPROC        glGetBufferPointerv;

        public: //! @name OpenGL 2.0

    PFNGLBLENDEQUATIONSEPARATEPROC    glBlendEquationSeparate;
    PFNGLDRAWBUFFERSPROC              glDrawBuffers;
    PFNGLSTENCILOPSEPARATEPROC        glStencilOpSeparate;
    PFNGLSTENCILFUNCSEPARATEPROC      glStencilFuncSeparate;
    PFNGLSTENCILMASKSEPARATEPROC      glStencilMaskSeparate;
    PFNGLATTACHSHADERPROC             glAttachShader;
    PFNGLBINDATTRIBLOCATIONPROC       glBindAttribLocation;
    PFNGLCOMPILESHADERPROC            glCompileShader;
    PFNGLCREATEPROGRAMPROC            glCreateProgram;
    PFNGLCREATESHADERPROC             glCreateShader;
    PFNGLDELETEPROGRAMPROC            glDeleteProgram;
    PFNGLDELETESHADERPROC             glDeleteShader;
    PFNGLDETACHSHADERPROC             glDetachShader;
    PFNGLDISABLEVERTEXATTRIBARRAYPROC glDisableVertexAttribArray;
    PFNGLENABLEVERTEXATTRIBARRAYPROC  glEnableVertexAttribArray;
    PFNGLGETACTIVEATTRIBPROC          glGetActiveAttrib;
    PFNGLGETACTIVEUNIFORMPROC         glGetActiveUniform;
    PFNGLGETATTACHEDSHADERSPROC       glGetAttachedShaders;
    PFNGLGETATTRIBLOCATIONPROC        glGetAttribLocation;
    PFNGLGETPROGRAMIVPROC             glGetProgramiv;
    PFNGLGETPROGRAMINFOLOGPROC        glGetProgramInfoLog;
    PFNGLGETSHADERIVPROC              glGetShaderiv;
    PFNGLGETSHADERINFOLOGPROC         glGetShaderInfoLog;
    PFNGLGETSHADERSOURCEPROC          glGetShaderSource;
    PFNGLGETUNIFORMLOCATIONPROC       glGetUniformLocation;
    PFNGLGETUNIFORMFVPROC             glGetUniformfv;
    PFNGLGETUNIFORMIVPROC             glGetUniformiv;
    PFNGLGETVERTEXATTRIBDVPROC        glGetVertexAttribdv;
    PFNGLGETVERTEXATTRIBFVPROC        glGetVertexAttribfv;
    PFNGLGETVERTEXATTRIBIVPROC        glGetVertexAttribiv;
    PFNGLGETVERTEXATTRIBPOINTERVPROC  glGetVertexAttribPointerv;
    PFNGLISPROGRAMPROC                glIsProgram;
    PFNGLISSHADERPROC                 glIsShader;
    PFNGLLINKPROGRAMPROC              glLinkProgram;
    PFNGLSHADERSOURCEPROC             glShaderSource;
    PFNGLUSEPROGRAMPROC               glUseProgram;
    PFNGLUNIFORM1FPROC                glUniform1f;
    PFNGLUNIFORM2FPROC                glUniform2f;
    PFNGLUNIFORM3FPROC                glUniform3f;
    PFNGLUNIFORM4FPROC                glUniform4f;
    PFNGLUNIFORM1IPROC                glUniform1i;
    PFNGLUNIFORM2IPROC                glUniform2i;
    PFNGLUNIFORM3IPROC                glUniform3i;
    PFNGLUNIFORM4IPROC                glUniform4i;
    PFNGLUNIFORM1FVPROC               glUniform1fv;
    PFNGLUNIFORM2FVPROC               glUniform2fv;
    PFNGLUNIFORM3FVPROC               glUniform3fv;
    PFNGLUNIFORM4FVPROC               glUniform4fv;
    PFNGLUNIFORM1IVPROC               glUniform1iv;
    PFNGLUNIFORM2IVPROC               glUniform2iv;
    PFNGLUNIFORM3IVPROC               glUniform3iv;
    PFNGLUNIFORM4IVPROC               glUniform4iv;
    PFNGLUNIFORMMATRIX2FVPROC         glUniformMatrix2fv;
    PFNGLUNIFORMMATRIX3FVPROC         glUniformMatrix3fv;
    PFNGLUNIFORMMATRIX4FVPROC         glUniformMatrix4fv;
    PFNGLVALIDATEPROGRAMPROC          glValidateProgram;
    PFNGLVERTEXATTRIB1DPROC           glVertexAttrib1d;
    PFNGLVERTEXATTRIB1DVPROC          glVertexAttrib1dv;
    PFNGLVERTEXATTRIB1FPROC           glVertexAttrib1f;
    PFNGLVERTEXATTRIB1FVPROC          glVertexAttrib1fv;
    PFNGLVERTEXATTRIB1SPROC           glVertexAttrib1s;
    PFNGLVERTEXATTRIB1SVPROC          glVertexAttrib1sv;
    PFNGLVERTEXATTRIB2DPROC           glVertexAttrib2d;
    PFNGLVERTEXATTRIB2DVPROC          glVertexAttrib2dv;
    PFNGLVERTEXATTRIB2FPROC           glVertexAttrib2f;
    PFNGLVERTEXATTRIB2FVPROC          glVertexAttrib2fv;
    PFNGLVERTEXATTRIB2SPROC           glVertexAttrib2s;
    PFNGLVERTEXATTRIB2SVPROC          glVertexAttrib2sv;
    PFNGLVERTEXATTRIB3DPROC           glVertexAttrib3d;
    PFNGLVERTEXATTRIB3DVPROC          glVertexAttrib3dv;
    PFNGLVERTEXATTRIB3FPROC           glVertexAttrib3f;
    PFNGLVERTEXATTRIB3FVPROC          glVertexAttrib3fv;
    PFNGLVERTEXATTRIB3SPROC           glVertexAttrib3s;
    PFNGLVERTEXATTRIB3SVPROC          glVertexAttrib3sv;
    PFNGLVERTEXATTRIB4NBVPROC         glVertexAttrib4Nbv;
    PFNGLVERTEXATTRIB4NIVPROC         glVertexAttrib4Niv;
    PFNGLVERTEXATTRIB4NSVPROC         glVertexAttrib4Nsv;
    PFNGLVERTEXATTRIB4NUBPROC         glVertexAttrib4Nub;
    PFNGLVERTEXATTRIB4NUBVPROC        glVertexAttrib4Nubv;
    PFNGLVERTEXATTRIB4NUIVPROC        glVertexAttrib4Nuiv;
    PFNGLVERTEXATTRIB4NUSVPROC        glVertexAttrib4Nusv;
    PFNGLVERTEXATTRIB4BVPROC          glVertexAttrib4bv;
    PFNGLVERTEXATTRIB4DPROC           glVertexAttrib4d;
    PFNGLVERTEXATTRIB4DVPROC          glVertexAttrib4dv;
    PFNGLVERTEXATTRIB4FPROC           glVertexAttrib4f;
    PFNGLVERTEXATTRIB4FVPROC          glVertexAttrib4fv;
    PFNGLVERTEXATTRIB4IVPROC          glVertexAttrib4iv;
    PFNGLVERTEXATTRIB4SPROC           glVertexAttrib4s;
    PFNGLVERTEXATTRIB4SVPROC          glVertexAttrib4sv;
    PFNGLVERTEXATTRIB4UBVPROC         glVertexAttrib4ubv;
    PFNGLVERTEXATTRIB4UIVPROC         glVertexAttrib4uiv;
    PFNGLVERTEXATTRIB4USVPROC         glVertexAttrib4usv;
    PFNGLVERTEXATTRIBPOINTERPROC      glVertexAttribPointer;

        public: //! @name OpenGL 2.1

    PFNGLUNIFORMMATRIX2X3FVPROC       glUniformMatrix2x3fv;
    PFNGLUNIFORMMATRIX3X2FVPROC       glUniformMatrix3x2fv;
    PFNGLUNIFORMMATRIX2X4FVPROC       glUniformMatrix2x4fv;
    PFNGLUNIFORMMATRIX4X2FVPROC       glUniformMatrix4x2fv;
    PFNGLUNIFORMMATRIX3X4FVPROC       glUniformMatrix3x4fv;
    PFNGLUNIFORMMATRIX4X3FVPROC       glUniformMatrix4x3fv;

        public: //! @name GL_ARB_framebuffer_object (added to OpenGL 3.0 core)

    PFNGLISRENDERBUFFERPROC                      glIsRenderbuffer;
    PFNGLBINDRENDERBUFFERPROC                    glBindRenderbuffer;
    PFNGLDELETERENDERBUFFERSPROC                 glDeleteRenderbuffers;
    PFNGLGENRENDERBUFFERSPROC                    glGenRenderbuffers;
    PFNGLRENDERBUFFERSTORAGEPROC                 glRenderbufferStorage;
    PFNGLGETRENDERBUFFERPARAMETERIVPROC          glGetRenderbufferParameteriv;
    PFNGLISFRAMEBUFFERPROC                       glIsFramebuffer;
    PFNGLBINDFRAMEBUFFERPROC                     glBindFramebuffer;
    PFNGLDELETEFRAMEBUFFERSPROC                  glDeleteFramebuffers;
    PFNGLGENFRAMEBUFFERSPROC                     glGenFramebuffers;
    PFNGLCHECKFRAMEBUFFERSTATUSPROC              glCheckFramebufferStatus;
    PFNGLFRAMEBUFFERTEXTURE1DPROC                glFramebufferTexture1D;
    PFNGLFRAMEBUFFERTEXTURE2DPROC                glFramebufferTexture2D;
    PFNGLFRAMEBUFFERTEXTURE3DPROC                glFramebufferTexture3D;
    PFNGLFRAMEBUFFERRENDERBUFFERPROC             glFramebufferRenderbuffer;
    PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVPROC glGetFramebufferAttachmentParameteriv;
    PFNGLGENERATEMIPMAPPROC                      glGenerateMipmap;
    PFNGLBLITFRAMEBUFFERPROC                     glBlitFramebuffer;
    PFNGLRENDERBUFFERSTORAGEMULTISAMPLEPROC      glRenderbufferStorageMultisample;
    PFNGLFRAMEBUFFERTEXTURELAYERPROC             glFramebufferTextureLayer;

        public: //! @name GL_ARB_vertex_array_object (added to OpenGL 3.0 core)

    PFNGLBINDVERTEXARRAYPROC             glBindVertexArray;
    PFNGLDELETEVERTEXARRAYSPROC          glDeleteVertexArrays;
    PFNGLGENVERTEXARRAYSPROC             glGenVertexArrays;
    PFNGLISVERTEXARRAYPROC               glIsVertexArray;

        public: //! @name GL_ARB_map_buffer_range (added to OpenGL 3.0 core)

    PFNGLMAPBUFFERRANGEPROC              glMapBufferRange;
    PFNGLFLUSHMAPPEDBUFFERRANGEPROC      glFlushMappedBufferRange;

        public: //! @name OpenGL 3.0

    PFNGLCOLORMASKIPROC                  glColorMaski;
    PFNGLGETBOOLEANI_VPROC               glGetBooleani_v;
    PFNGLGETINTEGERI_VPROC               glGetIntegeri_v;
    PFNGLENABLEIPROC                     glEnablei;
    PFNGLDISABLEIPROC                    glDisablei;
    PFNGLISENABLEDIPROC                  glIsEnabledi;
    PFNGLBEGINTRANSFORMFEEDBACKPROC      glBeginTransformFeedback;
    PFNGLENDTRANSFORMFEEDBACKPROC        glEndTransformFeedback;
    PFNGLBINDBUFFERRANGEPROC             glBindBufferRange;
    PFNGLBINDBUFFERBASEPROC              glBindBufferBase;
    PFNGLTRANSFORMFEEDBACKVARYINGSPROC   glTransformFeedbackVaryings;
    PFNGLGETTRANSFORMFEEDBACKVARYINGPROC glGetTransformFeedbackVarying;
    PFNGLCLAMPCOLORPROC                  glClampColor;
    PFNGLBEGINCONDITIONALRENDERPROC      glBeginConditionalRender;
    PFNGLENDCONDITIONALRENDERPROC        glEndConditionalRender;
    PFNGLVERTEXATTRIBIPOINTERPROC        glVertexAttribIPointer;
    PFNGLGETVERTEXATTRIBIIVPROC          glGetVertexAttribIiv;
    PFNGLGETVERTEXATTRIBIUIVPROC         glGetVertexAttribIuiv;
    PFNGLVERTEXATTRIBI1IPROC             glVertexAttribI1i;
    PFNGLVERTEXATTRIBI2IPROC             glVertexAttribI2i;
    PFNGLVERTEXATTRIBI3IPROC             glVertexAttribI3i;
    PFNGLVERTEXATTRIBI4IPROC             glVertexAttribI4i;
    PFNGLVERTEXATTRIBI1UIPROC            glVertexAttribI1ui;
    PFNGLVERTEXATTRIBI2UIPROC            glVertexAttribI2ui;
    PFNGLVERTEXATTRIBI3UIPROC            glVertexAttribI3ui;
    PFNGLVERTEXATTRIBI4UIPROC            glVertexAttribI4ui;
    PFNGLVERTEXATTRIBI1IVPROC            glVertexAttribI1iv;
    PFNGLVERTEXATTRIBI2IVPROC            glVertexAttribI2iv;
    PFNGLVERTEXATTRIBI3IVPROC            glVertexAttribI3iv;
    PFNGLVERTEXATTRIBI4IVPROC            glVertexAttribI4iv;
    PFNGLVERTEXATTRIBI1UIVPROC           glVertexAttribI1uiv;
    PFNGLVERTEXATTRIBI2UIVPROC           glVertexAttribI2uiv;
    PFNGLVERTEXATTRIBI3UIVPROC           glVertexAttribI3uiv;
    PFNGLVERTEXATTRIBI4UIVPROC           glVertexAttribI4uiv;
    PFNGLVERTEXATTRIBI4BVPROC            glVertexAttribI4bv;
    PFNGLVERTEXATTRIBI4SVPROC            glVertexAttribI4sv;
    PFNGLVERTEXATTRIBI4UBVPROC           glVertexAttribI4ubv;
    PFNGLVERTEXATTRIBI4USVPROC           glVertexAttribI4usv;
    PFNGLGETUNIFORMUIVPROC               glGetUniformuiv;
    PFNGLBINDFRAGDATALOCATIONPROC        glBindFragDataLocation;
    PFNGLGETFRAGDATALOCATIONPROC         glGetFragDataLocation;
    PFNGLUNIFORM1UIPROC                  glUniform1ui;
    PFNGLUNIFORM2UIPROC                  glUniform2ui;
    PFNGLUNIFORM3UIPROC                  glUniform3ui;
    PFNGLUNIFORM4UIPROC                  glUniform4ui;
    PFNGLUNIFORM1UIVPROC                 glUniform1uiv;
    PFNGLUNIFORM2UIVPROC                 glUniform2uiv;
    PFNGLUNIFORM3UIVPROC                 glUniform3uiv;
    PFNGLUNIFORM4UIVPROC                 glUniform4uiv;
    PFNGLTEXPARAMETERIIVPROC             glTexParameterIiv;
    PFNGLTEXPARAMETERIUIVPROC            glTexParameterIuiv;
    PFNGLGETTEXPARAMETERIIVPROC          glGetTexParameterIiv;
    PFNGLGETTEXPARAMETERIUIVPROC         glGetTexParameterIuiv;
    PFNGLCLEARBUFFERIVPROC               glClearBufferiv;
    PFNGLCLEARBUFFERUIVPROC              glClearBufferuiv;
    PFNGLCLEARBUFFERFVPROC               glClearBufferfv;
    PFNGLCLEARBUFFERFIPROC               glClearBufferfi;
    PFNGLGETSTRINGIPROC                  glGetStringi;

        public: //! @name GL_ARB_uniform_buffer_object (added to OpenGL 3.1 core)

    PFNGLGETUNIFORMINDICESPROC           glGetUniformIndices;
    PFNGLGETACTIVEUNIFORMSIVPROC         glGetActiveUniformsiv;
    PFNGLGETACTIVEUNIFORMNAMEPROC        glGetActiveUniformName;
    PFNGLGETUNIFORMBLOCKINDEXPROC        glGetUniformBlockIndex;
    PFNGLGETACTIVEUNIFORMBLOCKIVPROC     glGetActiveUniformBlockiv;
    PFNGLGETACTIVEUNIFORMBLOCKNAMEPROC   glGetActiveUniformBlockName;
    PFNGLUNIFORMBLOCKBINDINGPROC         glUniformBlockBinding;

        public: //! @name GL_ARB_copy_buffer (added to OpenGL 3.1 core)

    PFNGLCOPYBUFFERSUBDATAPROC           glCopyBufferSubData;

        public: //! @name OpenGL 3.1

    PFNGLDRAWARRAYSINSTANCEDPROC         glDrawArraysInstanced;
    PFNGLDRAWELEMENTSINSTANCEDPROC       glDrawElementsInstanced;
    PFNGLTEXBUFFERPROC                   glTexBuffer;
    PFNGLPRIMITIVERESTARTINDEXPROC       glPrimitiveRestartIndex;

        public: //! @name GL_ARB_draw_elements_base_vertex (added to OpenGL 3.2 core)

    PFNGLDRAWELEMENTSBASEVERTEXPROC      glDrawElementsBaseVertex;
    PFNGLDRAWRANGEELEMENTSBASEVERTEXPROC glDrawRangeElementsBaseVertex;
    PFNGLDRAWELEMENTSINSTANCEDBASEVERTEXPROC glDrawElementsInstancedBaseVertex;
    PFNGLMULTIDRAWELEMENTSBASEVERTEXPROC glMultiDrawElementsBaseVertex;

        public: //! @name GL_ARB_provoking_vertex (added to OpenGL 3.2 core)

    PFNGLPROVOKINGVERTEXPROC             glProvokingVertex;

        public: //! @name GL_ARB_sync (added to OpenGL 3.2 core)

    PFNGLFENCESYNCPROC                   glFenceSync;
    PFNGLISSYNCPROC                      glIsSync;
    PFNGLDELETESYNCPROC                  glDeleteSync;
    PFNGLCLIENTWAITSYNCPROC              glClientWaitSync;
    PFNGLWAITSYNCPROC                    glWaitSync;
    PFNGLGETINTEGER64VPROC               glGetInteger64v;
    PFNGLGETSYNCIVPROC                   glGetSynciv;

        public: //! @name GL_ARB_texture_multisample (added to OpenGL 3.2 core)

    PFNGLTEXIMAGE2DMULTISAMPLEPROC       glTexImage2DMultisample;
    PFNGLTEXIMAGE3DMULTISAMPLEPROC       glTexImage3DMultisample;
    PFNGLGETMULTISAMPLEFVPROC            glGetMultisamplefv;
    PFNGLSAMPLEMASKIPROC                 glSampleMaski;

        public: //! @name OpenGL 3.2

    PFNGLGETINTEGER64I_VPROC             glGetInteger64i_v;
    PFNGLGETBUFFERPARAMETERI64VPROC      glGetBufferParameteri64v;
    PFNGLFRAMEBUFFERTEXTUREPROC          glFramebufferTexture;

        public: //! @name GL_ARB_blend_func_extended (added to OpenGL 3.3 core)

    PFNGLBINDFRAGDATALOCATIONINDEXEDPROC glBindFragDataLocationIndexed;
    PFNGLGETFRAGDATAINDEXPROC            glGetFragDataIndex;

        public: //! @name GL_ARB_sampler_objects (added to OpenGL 3.3 core)

    PFNGLGENSAMPLERSPROC                 glGenSamplers;
    PFNGLDELETESAMPLERSPROC              glDeleteSamplers;
    PFNGLISSAMPLERPROC                   glIsSampler;
    PFNGLBINDSAMPLERPROC                 glBindSampler;
    PFNGLSAMPLERPARAMETERIPROC           glSamplerParameteri;
    PFNGLSAMPLERPARAMETERIVPROC          glSamplerParameteriv;
    PFNGLSAMPLERPARAMETERFPROC           glSamplerParameterf;
    PFNGLSAMPLERPARAMETERFVPROC          glSamplerParameterfv;
    PFNGLSAMPLERPARAMETERIIVPROC         glSamplerParameterIiv;
    PFNGLSAMPLERPARAMETERIUIVPROC        glSamplerParameterIuiv;
    PFNGLGETSAMPLERPARAMETERIVPROC       glGetSamplerParameteriv;
    PFNGLGETSAMPLERPARAMETERIIVPROC      glGetSamplerParameterIiv;
    PFNGLGETSAMPLERPARAMETERFVPROC       glGetSamplerParameterfv;
    PFNGLGETSAMPLERPARAMETERIUIVPROC     glGetSamplerParameterIuiv;

        public: //! @name GL_ARB_timer_query (added to OpenGL 3.3 core)

    PFNGLQUERYCOUNTERPROC                glQueryCounter;
    PFNGLGETQUERYOBJECTI64VPROC          glGetQueryObjecti64v;
    PFNGLGETQUERYOBJECTUI64VPROC         glGetQueryObjectui64v;

        public: //! @name GL_ARB_vertex_type_2_10_10_10_rev (added to OpenGL 3.3 core)

    PFNGLVERTEXP2UIPROC                  glVertexP2ui;
    PFNGLVERTEXP2UIVPROC                 glVertexP2uiv;
    PFNGLVERTEXP3UIPROC                  glVertexP3ui;
    PFNGLVERTEXP3UIVPROC                 glVertexP3uiv;
    PFNGLVERTEXP4UIPROC                  glVertexP4ui;
    PFNGLVERTEXP4UIVPROC                 glVertexP4uiv;
    PFNGLTEXCOORDP1UIPROC                glTexCoordP1ui;
    PFNGLTEXCOORDP1UIVPROC               glTexCoordP1uiv;
    PFNGLTEXCOORDP2UIPROC                glTexCoordP2ui;
    PFNGLTEXCOORDP2UIVPROC               glTexCoordP2uiv;
    PFNGLTEXCOORDP3UIPROC                glTexCoordP3ui;
    PFNGLTEXCOORDP3UIVPROC               glTexCoordP3uiv;
    PFNGLTEXCOORDP4UIPROC                glTexCoordP4ui;
    PFNGLTEXCOORDP4UIVPROC               glTexCoordP4uiv;
    PFNGLMULTITEXCOORDP1UIPROC           glMultiTexCoordP1ui;
    PFNGLMULTITEXCOORDP1UIVPROC          glMultiTexCoordP1uiv;
    PFNGLMULTITEXCOORDP2UIPROC           glMultiTexCoordP2ui;
    PFNGLMULTITEXCOORDP2UIVPROC          glMultiTexCoordP2uiv;
    PFNGLMULTITEXCOORDP3UIPROC           glMultiTexCoordP3ui;
    PFNGLMULTITEXCOORDP3UIVPROC          glMultiTexCoordP3uiv;
    PFNGLMULTITEXCOORDP4UIPROC           glMultiTexCoordP4ui;
    PFNGLMULTITEXCOORDP4UIVPROC          glMultiTexCoordP4uiv;
    PFNGLNORMALP3UIPROC                  glNormalP3ui;
    PFNGLNORMALP3UIVPROC                 glNormalP3uiv;
    PFNGLCOLORP3UIPROC                   glColorP3ui;
    PFNGLCOLORP3UIVPROC                  glColorP3uiv;
    PFNGLCOLORP4UIPROC                   glColorP4ui;
    PFNGLCOLORP4UIVPROC                  glColorP4uiv;
    PFNGLSECONDARYCOLORP3UIPROC          glSecondaryColorP3ui;
    PFNGLSECONDARYCOLORP3UIVPROC         glSecondaryColorP3uiv;
    PFNGLVERTEXATTRIBP1UIPROC            glVertexAttribP1ui;
    PFNGLVERTEXATTRIBP1UIVPROC           glVertexAttribP1uiv;
    PFNGLVERTEXATTRIBP2UIPROC            glVertexAttribP2ui;
    PFNGLVERTEXATTRIBP2UIVPROC           glVertexAttribP2uiv;
    PFNGLVERTEXATTRIBP3UIPROC            glVertexAttribP3ui;
    PFNGLVERTEXATTRIBP3UIVPROC           glVertexAttribP3uiv;
    PFNGLVERTEXATTRIBP4UIPROC            glVertexAttribP4ui;
    PFNGLVERTEXATTRIBP4UIVPROC           glVertexAttribP4uiv;

        public: //! @name OpenGL 3.3

    PFNGLVERTEXATTRIBDIVISORPROC         glVertexAttribDivisor;

        public: //! @name GL_ARB_draw_indirect (added to OpenGL 4.0 core)

    PFNGLDRAWARRAYSINDIRECTPROC          glDrawArraysIndirect;
    PFNGLDRAWELEMENTSINDIRECTPROC        glDrawElementsIndirect;

        public: //! @name GL_ARB_gpu_shader_fp64 (added to OpenGL 4.0 core)

    PFNGLUNIFORM1DPROC                   glUniform1d;
    PFNGLUNIFORM2DPROC                   glUniform2d;
    PFNGLUNIFORM3DPROC                   glUniform3d;
    PFNGLUNIFORM4DPROC                   glUniform4d;
    PFNGLUNIFORM1DVPROC                  glUniform1dv;
    PFNGLUNIFORM2DVPROC                  glUniform2dv;
    PFNGLUNIFORM3DVPROC                  glUniform3dv;
    PFNGLUNIFORM4DVPROC                  glUniform4dv;
    PFNGLUNIFORMMATRIX2DVPROC            glUniformMatrix2dv;
    PFNGLUNIFORMMATRIX3DVPROC            glUniformMatrix3dv;
    PFNGLUNIFORMMATRIX4DVPROC            glUniformMatrix4dv;
    PFNGLUNIFORMMATRIX2X3DVPROC          glUniformMatrix2x3dv;
    PFNGLUNIFORMMATRIX2X4DVPROC          glUniformMatrix2x4dv;
    PFNGLUNIFORMMATRIX3X2DVPROC          glUniformMatrix3x2dv;
    PFNGLUNIFORMMATRIX3X4DVPROC          glUniformMatrix3x4dv;
    PFNGLUNIFORMMATRIX4X2DVPROC          glUniformMatrix4x2dv;
    PFNGLUNIFORMMATRIX4X3DVPROC          glUniformMatrix4x3dv;
    PFNGLGETUNIFORMDVPROC                glGetUniformdv;

        public: //! @name GL_ARB_shader_subroutine (added to OpenGL 4.0 core)

    PFNGLGETSUBROUTINEUNIFORMLOCATIONPROC   glGetSubroutineUniformLocation;
    PFNGLGETSUBROUTINEINDEXPROC             glGetSubroutineIndex;
    PFNGLGETACTIVESUBROUTINEUNIFORMIVPROC   glGetActiveSubroutineUniformiv;
    PFNGLGETACTIVESUBROUTINEUNIFORMNAMEPROC glGetActiveSubroutineUniformName;
    PFNGLGETACTIVESUBROUTINENAMEPROC        glGetActiveSubroutineName;
    PFNGLUNIFORMSUBROUTINESUIVPROC          glUniformSubroutinesuiv;
    PFNGLGETUNIFORMSUBROUTINEUIVPROC        glGetUniformSubroutineuiv;
    PFNGLGETPROGRAMSTAGEIVPROC              glGetProgramStageiv;

        public: //! @name GL_ARB_tessellation_shader (added to OpenGL 4.0 core)

    PFNGLPATCHPARAMETERIPROC             glPatchParameteri;
    PFNGLPATCHPARAMETERFVPROC            glPatchParameterfv;

        public: //! @name GL_ARB_transform_feedback2 (added to OpenGL 4.0 core)

    PFNGLBINDTRANSFORMFEEDBACKPROC       glBindTransformFeedback;
    PFNGLDELETETRANSFORMFEEDBACKSPROC    glDeleteTransformFeedbacks;
    PFNGLGENTRANSFORMFEEDBACKSPROC       glGenTransformFeedbacks;
    PFNGLISTRANSFORMFEEDBACKPROC         glIsTransformFeedback;
    PFNGLPAUSETRANSFORMFEEDBACKPROC      glPauseTransformFeedback;
    PFNGLRESUMETRANSFORMFEEDBACKPROC     glResumeTransformFeedback;
    PFNGLDRAWTRANSFORMFEEDBACKPROC       glDrawTransformFeedback;

        public: //! @name GL_ARB_transform_feedback3 (added to OpenGL 4.0 core)

    PFNGLDRAWTRANSFORMFEEDBACKSTREAMPROC glDrawTransformFeedbackStream;
    PFNGLBEGINQUERYINDEXEDPROC           glBeginQueryIndexed;
    PFNGLENDQUERYINDEXEDPROC             glEndQueryIndexed;
    PFNGLGETQUERYINDEXEDIVPROC           glGetQueryIndexediv;

        public: //! @name OpenGL 4.0

    PFNGLMINSAMPLESHADINGPROC            glMinSampleShading;
    PFNGLBLENDEQUATIONIPROC              glBlendEquationi;
    PFNGLBLENDEQUATIONSEPARATEIPROC      glBlendEquationSeparatei;
    PFNGLBLENDFUNCIPROC                  glBlendFunci;
    PFNGLBLENDFUNCSEPARATEIPROC          glBlendFuncSeparatei;

        public: //! @name GL_ARB_ES2_compatibility (added to OpenGL 4.1 core)

    PFNGLRELEASESHADERCOMPILERPROC       glReleaseShaderCompiler;
    PFNGLSHADERBINARYPROC                glShaderBinary;
    PFNGLGETSHADERPRECISIONFORMATPROC    glGetShaderPrecisionFormat;
    PFNGLDEPTHRANGEFPROC                 glDepthRangef;
    PFNGLCLEARDEPTHFPROC                 glClearDepthf;

        public: //! @name GL_ARB_get_program_binary (added to OpenGL 4.1 core)

    PFNGLGETPROGRAMBINARYPROC            glGetProgramBinary;
    PFNGLPROGRAMBINARYPROC               glProgramBinary;
    PFNGLPROGRAMPARAMETERIPROC           glProgramParameteri;

        public: //! @name GL_ARB_separate_shader_objects (added to OpenGL 4.1 core)

    PFNGLUSEPROGRAMSTAGESPROC            glUseProgramStages;
    PFNGLACTIVESHADERPROGRAMPROC         glActiveShaderProgram;
    PFNGLCREATESHADERPROGRAMVPROC        glCreateShaderProgramv;
    PFNGLBINDPROGRAMPIPELINEPROC         glBindProgramPipeline;
    PFNGLDELETEPROGRAMPIPELINESPROC      glDeleteProgramPipelines;
    PFNGLGENPROGRAMPIPELINESPROC         glGenProgramPipelines;
    PFNGLISPROGRAMPIPELINEPROC           glIsProgramPipeline;
    PFNGLGETPROGRAMPIPELINEIVPROC        glGetProgramPipelineiv;
    PFNGLPROGRAMUNIFORM1IPROC            glProgramUniform1i;
    PFNGLPROGRAMUNIFORM1IVPROC           glProgramUniform1iv;
    PFNGLPROGRAMUNIFORM1FPROC            glProgramUniform1f;
    PFNGLPROGRAMUNIFORM1FVPROC           glProgramUniform1fv;
    PFNGLPROGRAMUNIFORM1DPROC            glProgramUniform1d;
    PFNGLPROGRAMUNIFORM1DVPROC           glProgramUniform1dv;
    PFNGLPROGRAMUNIFORM1UIPROC           glProgramUniform1ui;
    PFNGLPROGRAMUNIFORM1UIVPROC          glProgramUniform1uiv;
    PFNGLPROGRAMUNIFORM2IPROC            glProgramUniform2i;
    PFNGLPROGRAMUNIFORM2IVPROC           glProgramUniform2iv;
    PFNGLPROGRAMUNIFORM2FPROC            glProgramUniform2f;
    PFNGLPROGRAMUNIFORM2FVPROC           glProgramUniform2fv;
    PFNGLPROGRAMUNIFORM2DPROC            glProgramUniform2d;
    PFNGLPROGRAMUNIFORM2DVPROC           glProgramUniform2dv;
    PFNGLPROGRAMUNIFORM2UIPROC           glProgramUniform2ui;
    PFNGLPROGRAMUNIFORM2UIVPROC          glProgramUniform2uiv;
    PFNGLPROGRAMUNIFORM3IPROC            glProgramUniform3i;
    PFNGLPROGRAMUNIFORM3IVPROC           glProgramUniform3iv;
    PFNGLPROGRAMUNIFORM3FPROC            glProgramUniform3f;
    PFNGLPROGRAMUNIFORM3FVPROC           glProgramUniform3fv;
    PFNGLPROGRAMUNIFORM3DPROC            glProgramUniform3d;
    PFNGLPROGRAMUNIFORM3DVPROC           glProgramUniform3dv;
    PFNGLPROGRAMUNIFORM3UIPROC           glProgramUniform3ui;
    PFNGLPROGRAMUNIFORM3UIVPROC          glProgramUniform3uiv;
    PFNGLPROGRAMUNIFORM4IPROC            glProgramUniform4i;
    PFNGLPROGRAMUNIFORM4IVPROC           glProgramUniform4iv;
    PFNGLPROGRAMUNIFORM4FPROC            glProgramUniform4f;
    PFNGLPROGRAMUNIFORM4FVPROC           glProgramUniform4fv;
    PFNGLPROGRAMUNIFORM4DPROC            glProgramUniform4d;
    PFNGLPROGRAMUNIFORM4DVPROC           glProgramUniform4dv;
    PFNGLPROGRAMUNIFORM4UIPROC           glProgramUniform4ui;
    PFNGLPROGRAMUNIFORM4UIVPROC          glProgramUniform4uiv;
    PFNGLPROGRAMUNIFORMMATRIX2FVPROC     glProgramUniformMatrix2fv;
    PFNGLPROGRAMUNIFORMMATRIX3FVPROC     glProgramUniformMatrix3fv;
    PFNGLPROGRAMUNIFORMMATRIX4FVPROC     glProgramUniformMatrix4fv;
    PFNGLPROGRAMUNIFORMMATRIX2DVPROC     glProgramUniformMatrix2dv;
    PFNGLPROGRAMUNIFORMMATRIX3DVPROC     glProgramUniformMatrix3dv;
    PFNGLPROGRAMUNIFORMMATRIX4DVPROC     glProgramUniformMatrix4dv;
    PFNGLPROGRAMUNIFORMMATRIX2X3FVPROC   glProgramUniformMatrix2x3fv;
    PFNGLPROGRAMUNIFORMMATRIX3X2FVPROC   glProgramUniformMatrix3x2fv;
    PFNGLPROGRAMUNIFORMMATRIX2X4FVPROC   glProgramUniformMatrix2x4fv;
    PFNGLPROGRAMUNIFORMMATRIX4X2FVPROC   glProgramUniformMatrix4x2fv;
    PFNGLPROGRAMUNIFORMMATRIX3X4FVPROC   glProgramUniformMatrix3x4fv;
    PFNGLPROGRAMUNIFORMMATRIX4X3FVPROC   glProgramUniformMatrix4x3fv;
    PFNGLPROGRAMUNIFORMMATRIX2X3DVPROC   glProgramUniformMatrix2x3dv;
    PFNGLPROGRAMUNIFORMMATRIX3X2DVPROC   glProgramUniformMatrix3x2dv;
    PFNGLPROGRAMUNIFORMMATRIX2X4DVPROC   glProgramUniformMatrix2x4dv;
    PFNGLPROGRAMUNIFORMMATRIX4X2DVPROC   glProgramUniformMatrix4x2dv;
    PFNGLPROGRAMUNIFORMMATRIX3X4DVPROC   glProgramUniformMatrix3x4dv;
    PFNGLPROGRAMUNIFORMMATRIX4X3DVPROC   glProgramUniformMatrix4x3dv;
    PFNGLVALIDATEPROGRAMPIPELINEPROC     glValidateProgramPipeline;
    PFNGLGETPROGRAMPIPELINEINFOLOGPROC   glGetProgramPipelineInfoLog;

        public: //! @name GL_ARB_vertex_attrib_64bit (added to OpenGL 4.1 core)

    PFNGLVERTEXATTRIBL1DPROC             glVertexAttribL1d;
    PFNGLVERTEXATTRIBL2DPROC             glVertexAttribL2d;
    PFNGLVERTEXATTRIBL3DPROC             glVertexAttribL3d;
    PFNGLVERTEXATTRIBL4DPROC             glVertexAttribL4d;
    PFNGLVERTEXATTRIBL1DVPROC            glVertexAttribL1dv;
    PFNGLVERTEXATTRIBL2DVPROC            glVertexAttribL2dv;
    PFNGLVERTEXATTRIBL3DVPROC            glVertexAttribL3dv;
    PFNGLVERTEXATTRIBL4DVPROC            glVertexAttribL4dv;
    PFNGLVERTEXATTRIBLPOINTERPROC        glVertexAttribLPointer;
    PFNGLGETVERTEXATTRIBLDVPROC          glGetVertexAttribLdv;

        public: //! @name GL_ARB_viewport_array (added to OpenGL 4.1 core)

    PFNGLVIEWPORTARRAYVPROC              glViewportArrayv;
    PFNGLVIEWPORTINDEXEDFPROC            glViewportIndexedf;
    PFNGLVIEWPORTINDEXEDFVPROC           glViewportIndexedfv;
    PFNGLSCISSORARRAYVPROC               glScissorArrayv;
    PFNGLSCISSORINDEXEDPROC              glScissorIndexed;
    PFNGLSCISSORINDEXEDVPROC             glScissorIndexedv;
    PFNGLDEPTHRANGEARRAYVPROC            glDepthRangeArrayv;
    PFNGLDEPTHRANGEINDEXEDPROC           glDepthRangeIndexed;
    PFNGLGETFLOATI_VPROC                 glGetFloati_v;
    PFNGLGETDOUBLEI_VPROC                glGetDoublei_v;

        public: //! @name OpenGL 4.1

    //

        public: //! @name GL_ARB_base_instance (added to OpenGL 4.2 core)

    PFNGLDRAWARRAYSINSTANCEDBASEINSTANCEPROC             glDrawArraysInstancedBaseInstance;
    PFNGLDRAWELEMENTSINSTANCEDBASEINSTANCEPROC           glDrawElementsInstancedBaseInstance;
    PFNGLDRAWELEMENTSINSTANCEDBASEVERTEXBASEINSTANCEPROC glDrawElementsInstancedBaseVertexBaseInstance;

        public: //! @name GL_ARB_transform_feedback_instanced (added to OpenGL 4.2 core)

    PFNGLDRAWTRANSFORMFEEDBACKINSTANCEDPROC              glDrawTransformFeedbackInstanced;
    PFNGLDRAWTRANSFORMFEEDBACKSTREAMINSTANCEDPROC        glDrawTransformFeedbackStreamInstanced;

        public: //! @name GL_ARB_internalformat_query (added to OpenGL 4.2 core)

    PFNGLGETINTERNALFORMATIVPROC                         glGetInternalformativ;

        public: //! @name GL_ARB_shader_atomic_counters (added to OpenGL 4.2 core)

    PFNGLGETACTIVEATOMICCOUNTERBUFFERIVPROC              glGetActiveAtomicCounterBufferiv;

        public: //! @name GL_ARB_shader_image_load_store (added to OpenGL 4.2 core)

    PFNGLBINDIMAGETEXTUREPROC                            glBindImageTexture;
    PFNGLMEMORYBARRIERPROC                               glMemoryBarrier;

        public: //! @name GL_ARB_texture_storage (added to OpenGL 4.2 core)

    PFNGLTEXSTORAGE1DPROC                                glTexStorage1D;
    PFNGLTEXSTORAGE2DPROC                                glTexStorage2D;
    PFNGLTEXSTORAGE3DPROC                                glTexStorage3D;
    PFNGLTEXTURESTORAGE1DEXTPROC                         glTextureStorage1DEXT;
    PFNGLTEXTURESTORAGE2DEXTPROC                         glTextureStorage2DEXT;
    PFNGLTEXTURESTORAGE3DEXTPROC                         glTextureStorage3DEXT;

        public: //! @name OpenGL 4.2

        public: //! @name OpenGL 4.3

    PFNGLCLEARBUFFERDATAPROC                 glClearBufferData;
    PFNGLCLEARBUFFERSUBDATAPROC              glClearBufferSubData;
    PFNGLDISPATCHCOMPUTEPROC                 glDispatchCompute;
    PFNGLDISPATCHCOMPUTEINDIRECTPROC         glDispatchComputeIndirect;
    PFNGLCOPYIMAGESUBDATAPROC                glCopyImageSubData;
    PFNGLFRAMEBUFFERPARAMETERIPROC           glFramebufferParameteri;
    PFNGLGETFRAMEBUFFERPARAMETERIVPROC       glGetFramebufferParameteriv;
    PFNGLGETINTERNALFORMATI64VPROC           glGetInternalformati64v;
    PFNGLINVALIDATETEXSUBIMAGEPROC           glInvalidateTexSubImage;
    PFNGLINVALIDATETEXIMAGEPROC              glInvalidateTexImage;
    PFNGLINVALIDATEBUFFERSUBDATAPROC         glInvalidateBufferSubData;
    PFNGLINVALIDATEBUFFERDATAPROC            glInvalidateBufferData;
    PFNGLINVALIDATEFRAMEBUFFERPROC           glInvalidateFramebuffer;
    PFNGLINVALIDATESUBFRAMEBUFFERPROC        glInvalidateSubFramebuffer;
    PFNGLMULTIDRAWARRAYSINDIRECTPROC         glMultiDrawArraysIndirect;
    PFNGLMULTIDRAWELEMENTSINDIRECTPROC       glMultiDrawElementsIndirect;
    PFNGLGETPROGRAMINTERFACEIVPROC           glGetProgramInterfaceiv;
    PFNGLGETPROGRAMRESOURCEINDEXPROC         glGetProgramResourceIndex;
    PFNGLGETPROGRAMRESOURCENAMEPROC          glGetProgramResourceName;
    PFNGLGETPROGRAMRESOURCEIVPROC            glGetProgramResourceiv;
    PFNGLGETPROGRAMRESOURCELOCATIONPROC      glGetProgramResourceLocation;
    PFNGLGETPROGRAMRESOURCELOCATIONINDEXPROC glGetProgramResourceLocationIndex;
    PFNGLSHADERSTORAGEBLOCKBINDINGPROC       glShaderStorageBlockBinding;
    PFNGLTEXBUFFERRANGEPROC                  glTexBufferRange;
    PFNGLTEXSTORAGE2DMULTISAMPLEPROC         glTexStorage2DMultisample;
    PFNGLTEXSTORAGE3DMULTISAMPLEPROC         glTexStorage3DMultisample;
    PFNGLTEXTUREVIEWPROC                     glTextureView;
    PFNGLBINDVERTEXBUFFERPROC                glBindVertexBuffer;
    PFNGLVERTEXATTRIBFORMATPROC              glVertexAttribFormat;
    PFNGLVERTEXATTRIBIFORMATPROC             glVertexAttribIFormat;
    PFNGLVERTEXATTRIBLFORMATPROC             glVertexAttribLFormat;
    PFNGLVERTEXATTRIBBINDINGPROC             glVertexAttribBinding;
    PFNGLVERTEXBINDINGDIVISORPROC            glVertexBindingDivisor;
    PFNGLDEBUGMESSAGECONTROLPROC             glDebugMessageControl;
    PFNGLDEBUGMESSAGEINSERTPROC              glDebugMessageInsert;
    PFNGLDEBUGMESSAGECALLBACKPROC            glDebugMessageCallback;
    PFNGLGETDEBUGMESSAGELOGPROC              glGetDebugMessageLog;
    PFNGLPUSHDEBUGGROUPPROC                  glPushDebugGroup;
    PFNGLPOPDEBUGGROUPPROC                   glPopDebugGroup;
    PFNGLOBJECTLABELPROC                     glObjectLabel;
    PFNGLGETOBJECTLABELPROC                  glGetObjectLabel;
    PFNGLOBJECTPTRLABELPROC                  glObjectPtrLabel;
    PFNGLGETOBJECTPTRLABELPROC               glGetObjectPtrLabel;

        public: //! @name OpenGL 4.4

    PFNGLBUFFERSTORAGEPROC     glBufferStorage;
    PFNGLCLEARTEXIMAGEPROC     glClearTexImage;
    PFNGLCLEARTEXSUBIMAGEPROC  glClearTexSubImage;
    PFNGLBINDBUFFERSBASEPROC   glBindBuffersBase;
    PFNGLBINDBUFFERSRANGEPROC  glBindBuffersRange;
    PFNGLBINDTEXTURESPROC      glBindTextures;
    PFNGLBINDSAMPLERSPROC      glBindSamplers;
    PFNGLBINDIMAGETEXTURESPROC glBindImageTextures;
    PFNGLBINDVERTEXBUFFERSPROC glBindVertexBuffers;

#if defined(_WIN32)
        public: //! @name wgl extensions

    typedef const char* (WINAPI *wglGetExtensionsStringARB_t)(HDC theDeviceContext);
    wglGetExtensionsStringARB_t wglGetExtensionsStringARB;

    typedef BOOL        (WINAPI *wglSwapIntervalEXT_t)(int theInterval);
    wglSwapIntervalEXT_t wglSwapIntervalEXT;

        // WGL_ARB_pixel_format

#ifndef WGL_NUMBER_PIXEL_FORMATS_ARB
    #define WGL_NUMBER_PIXEL_FORMATS_ARB            0x2000
    #define WGL_DRAW_TO_WINDOW_ARB                  0x2001
    #define WGL_DRAW_TO_BITMAP_ARB                  0x2002
    #define WGL_ACCELERATION_ARB                    0x2003
    #define WGL_NEED_PALETTE_ARB                    0x2004
    #define WGL_NEED_SYSTEM_PALETTE_ARB             0x2005
    #define WGL_SWAP_LAYER_BUFFERS_ARB              0x2006
    #define WGL_SWAP_METHOD_ARB                     0x2007
    #define WGL_NUMBER_OVERLAYS_ARB                 0x2008
    #define WGL_NUMBER_UNDERLAYS_ARB                0x2009
    #define WGL_TRANSPARENT_ARB                     0x200A
    #define WGL_TRANSPARENT_RED_VALUE_ARB           0x2037
    #define WGL_TRANSPARENT_GREEN_VALUE_ARB         0x2038
    #define WGL_TRANSPARENT_BLUE_VALUE_ARB          0x2039
    #define WGL_TRANSPARENT_ALPHA_VALUE_ARB         0x203A
    #define WGL_TRANSPARENT_INDEX_VALUE_ARB         0x203B
    #define WGL_SHARE_DEPTH_ARB                     0x200C
    #define WGL_SHARE_STENCIL_ARB                   0x200D
    #define WGL_SHARE_ACCUM_ARB                     0x200E
    #define WGL_SUPPORT_GDI_ARB                     0x200F
    #define WGL_SUPPORT_OPENGL_ARB                  0x2010
    #define WGL_DOUBLE_BUFFER_ARB                   0x2011
    #define WGL_STEREO_ARB                          0x2012
    #define WGL_PIXEL_TYPE_ARB                      0x2013
    #define WGL_COLOR_BITS_ARB                      0x2014
    #define WGL_RED_BITS_ARB                        0x2015
    #define WGL_RED_SHIFT_ARB                       0x2016
    #define WGL_GREEN_BITS_ARB                      0x2017
    #define WGL_GREEN_SHIFT_ARB                     0x2018
    #define WGL_BLUE_BITS_ARB                       0x2019
    #define WGL_BLUE_SHIFT_ARB                      0x201A
    #define WGL_ALPHA_BITS_ARB                      0x201B
    #define WGL_ALPHA_SHIFT_ARB                     0x201C
    #define WGL_ACCUM_BITS_ARB                      0x201D
    #define WGL_ACCUM_RED_BITS_ARB                  0x201E
    #define WGL_ACCUM_GREEN_BITS_ARB                0x201F
    #define WGL_ACCUM_BLUE_BITS_ARB                 0x2020
    #define WGL_ACCUM_ALPHA_BITS_ARB                0x2021
    #define WGL_DEPTH_BITS_ARB                      0x2022
    #define WGL_STENCIL_BITS_ARB                    0x2023
    #define WGL_AUX_BUFFERS_ARB                     0x2024

    #define WGL_NO_ACCELERATION_ARB                 0x2025
    #define WGL_GENERIC_ACCELERATION_ARB            0x2026
    #define WGL_FULL_ACCELERATION_ARB               0x2027

    #define WGL_SWAP_EXCHANGE_ARB                   0x2028
    #define WGL_SWAP_COPY_ARB                       0x2029
    #define WGL_SWAP_UNDEFINED_ARB                  0x202A

    #define WGL_TYPE_RGBA_ARB                       0x202B
    #define WGL_TYPE_COLORINDEX_ARB                 0x202C

#endif // WGL_NUMBER_PIXEL_FORMATS_ARB

        // WGL_ARB_multisample
#ifndef WGL_SAMPLE_BUFFERS_ARB
    #define WGL_SAMPLE_BUFFERS_ARB               0x2041
    #define WGL_SAMPLES_ARB                      0x2042
#endif

        // WGL_ARB_create_context_robustness
#ifndef WGL_CONTEXT_ROBUST_ACCESS_BIT_ARB
    #define WGL_CONTEXT_ROBUST_ACCESS_BIT_ARB 0x00000004
    #define WGL_CONTEXT_RESET_NOTIFICATION_STRATEGY_ARB 0x8256
    #define WGL_NO_RESET_NOTIFICATION_ARB               0x8261
    #define WGL_LOSE_CONTEXT_ON_RESET_ARB               0x8252
#endif

    typedef BOOL (WINAPI *wglChoosePixelFormatARB_t)
        (HDC           theDevCtx,
         const int*    theIntAttribs,
         const float*  theFloatAttribs,
         unsigned int  theMaxFormats,
         int*          theFormatsOut,
         unsigned int* theNumFormatsOut);
    wglChoosePixelFormatARB_t wglChoosePixelFormatARB;

        // WGL_ARB_create_context_profile

#ifndef WGL_CONTEXT_MAJOR_VERSION_ARB
    #define WGL_CONTEXT_MAJOR_VERSION_ARB           0x2091
    #define WGL_CONTEXT_MINOR_VERSION_ARB           0x2092
    #define WGL_CONTEXT_LAYER_PLANE_ARB             0x2093
    #define WGL_CONTEXT_FLAGS_ARB                   0x2094
    #define WGL_CONTEXT_PROFILE_MASK_ARB            0x9126

    // WGL_CONTEXT_FLAGS bits
    #define WGL_CONTEXT_DEBUG_BIT_ARB               0x0001
    #define WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB  0x0002

    // WGL_CONTEXT_PROFILE_MASK_ARB bits
    #define WGL_CONTEXT_CORE_PROFILE_BIT_ARB          0x00000001
    #define WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB 0x00000002
#endif // WGL_CONTEXT_MAJOR_VERSION_ARB

    typedef HGLRC (WINAPI *wglCreateContextAttribsARB_t)(HDC        theDevCtx,
                                                         HGLRC      theShareContext,
                                                         const int* theAttribs);
    wglCreateContextAttribsARB_t wglCreateContextAttribsARB;

        // WGL_ARB_context_flush_control

    #define WGL_CONTEXT_RELEASE_BEHAVIOR_ARB       0x2097
    #define WGL_CONTEXT_RELEASE_BEHAVIOR_NONE_ARB  0x0000
    #define WGL_CONTEXT_RELEASE_BEHAVIOR_FLUSH_ARB 0x2098

        // WGL_NV_DX_interop

    typedef BOOL   (WINAPI *wglDXSetResourceShareHandleNV_t)(void* theObjectD3d, HANDLE theShareHandle);
    typedef HANDLE (WINAPI *wglDXOpenDeviceNV_t      )(void*   theDeviceD3d);
    typedef BOOL   (WINAPI *wglDXCloseDeviceNV_t     )(HANDLE  theDeviceIOP);
    typedef HANDLE (WINAPI *wglDXRegisterObjectNV_t  )(HANDLE  theDeviceIOP,
                                                       void*   theObjectD3d,
                                                       GLuint  theName,
                                                       GLenum  theType,
                                                       GLenum  theAccess);
    typedef BOOL   (WINAPI *wglDXUnregisterObjectNV_t)(HANDLE  theDeviceIOP,
                                                       HANDLE  theObject);
    typedef BOOL   (WINAPI *wglDXObjectAccessNV_t    )(HANDLE  theObject,
                                                       GLenum  theAccess);
    typedef BOOL   (WINAPI *wglDXLockObjectsNV_t     )(HANDLE  theDeviceIOP,
                                                       GLint   theCount,
                                                       HANDLE* theObjects);
    typedef BOOL   (WINAPI *wglDXUnlockObjectsNV_t   )(HANDLE  theDeviceIOP,
                                                       GLint   theCount,
                                                       HANDLE* theObjects);

    wglDXSetResourceShareHandleNV_t wglDXSetResourceShareHandleNV;
    wglDXOpenDeviceNV_t       wglDXOpenDeviceNV;
    wglDXCloseDeviceNV_t      wglDXCloseDeviceNV;
    wglDXRegisterObjectNV_t   wglDXRegisterObjectNV;
    wglDXUnregisterObjectNV_t wglDXUnregisterObjectNV;
    wglDXObjectAccessNV_t     wglDXObjectAccessNV;
    wglDXLockObjectsNV_t      wglDXLockObjectsNV;
    wglDXUnlockObjectsNV_t    wglDXUnlockObjectsNV;

#ifndef WGL_ACCESS_READ_WRITE_NV
    #define WGL_ACCESS_READ_ONLY_NV     0x0000
    #define WGL_ACCESS_READ_WRITE_NV    0x0001
    #define WGL_ACCESS_WRITE_DISCARD_NV 0x0002
#endif

        // WGL_AMD_gpu_association

#ifndef WGL_GPU_VENDOR_AMD
    #define WGL_GPU_VENDOR_AMD                 0x1F00
    #define WGL_GPU_RENDERER_STRING_AMD        0x1F01
    #define WGL_GPU_OPENGL_VERSION_STRING_AMD  0x1F02
    #define WGL_GPU_FASTEST_TARGET_GPUS_AMD    0x21A2
    #define WGL_GPU_RAM_AMD                    0x21A3
    #define WGL_GPU_CLOCK_AMD                  0x21A4
    #define WGL_GPU_NUM_PIPES_AMD              0x21A5
    #define WGL_GPU_NUM_SIMD_AMD               0x21A6
    #define WGL_GPU_NUM_RB_AMD                 0x21A7
    #define WGL_GPU_NUM_SPI_AMD                0x21A8
#endif

    typedef UINT (WINAPI *wglGetGPUIDsAMD_t       )(UINT theMaxCount, UINT* theIds);
    typedef INT  (WINAPI *wglGetGPUInfoAMD_t      )(UINT theId, INT theProperty, GLenum theDataType, UINT theSize, void* theData);
    typedef UINT (WINAPI *wglGetContextGPUIDAMD_t )(HGLRC theHglrc);
    wglGetGPUIDsAMD_t       wglGetGPUIDsAMD;
    wglGetGPUInfoAMD_t      wglGetGPUInfoAMD;
    wglGetContextGPUIDAMD_t wglGetContextGPUIDAMD;

#elif defined(__APPLE__)
        public: //! @name CGL extensions

#else
        public: //! @name GLX extensions

    // GLX_EXT_swap_control
    //typedef int         (*glXSwapIntervalEXT_t)(Display* theDisplay, GLXDrawable theDrawable, int theInterval);
    typedef int         (*glXSwapIntervalEXT_t)();
    glXSwapIntervalEXT_t glXSwapIntervalEXT;

    typedef int         (*glXSwapIntervalSGI_t)(int theInterval);
    glXSwapIntervalSGI_t glXSwapIntervalSGI;

        public: //! @name GLX_ARB_context_flush_control

#ifndef GLX_CONTEXT_RELEASE_BEHAVIOR_ARB
    #define GLX_CONTEXT_RELEASE_BEHAVIOR_ARB       0x2097
    #define GLX_CONTEXT_RELEASE_BEHAVIOR_NONE_ARB  0x0000
    #define GLX_CONTEXT_RELEASE_BEHAVIOR_FLUSH_ARB 0x2098
#endif

        // GLX_MESA_query_renderer

#ifndef GLX_RENDERER_VENDOR_ID_MESA
    // for glXQueryRendererIntegerMESA() and glXQueryCurrentRendererIntegerMESA()
    #define GLX_RENDERER_VENDOR_ID_MESA                      0x8183
    #define GLX_RENDERER_DEVICE_ID_MESA                      0x8184
    #define GLX_RENDERER_VERSION_MESA                        0x8185
    #define GLX_RENDERER_ACCELERATED_MESA                    0x8186
    #define GLX_RENDERER_VIDEO_MEMORY_MESA                   0x8187
    #define GLX_RENDERER_UNIFIED_MEMORY_ARCHITECTURE_MESA    0x8188
    #define GLX_RENDERER_PREFERRED_PROFILE_MESA              0x8189
    #define GLX_RENDERER_OPENGL_CORE_PROFILE_VERSION_MESA    0x818A
    #define GLX_RENDERER_OPENGL_COMPATIBILITY_PROFILE_VERSION_MESA 0x818B
    #define GLX_RENDERER_OPENGL_ES_PROFILE_VERSION_MESA      0x818C
    #define GLX_RENDERER_OPENGL_ES2_PROFILE_VERSION_MESA     0x818D

    #define GLX_RENDERER_ID_MESA                             0x818E
#endif // GLX_RENDERER_VENDOR_ID_MESA

    typedef void* stglxDisplay_t; // Display*
    typedef int   stglxBool;      // Bool
    typedef stglxBool (*glXQueryRendererIntegerMESA_t)(stglxDisplay_t dpy, int screen,
                                                       int renderer, int attribute,
                                                       unsigned int *value);
    typedef stglxBool (*glXQueryCurrentRendererIntegerMESA_t)(int attribute, unsigned int *value);
    typedef const char* (*glXQueryRendererStringMESA_t)(stglxDisplay_t dpy, int screen,
                                                        int renderer, int attribute);
    typedef const char* (*glXQueryCurrentRendererStringMESA_t)(int attribute);

    glXQueryRendererIntegerMESA_t        glXQueryRendererIntegerMESA;
    glXQueryCurrentRendererIntegerMESA_t glXQueryCurrentRendererIntegerMESA;
    glXQueryRendererStringMESA_t         glXQueryRendererStringMESA;
    glXQueryCurrentRendererStringMESA_t  glXQueryCurrentRendererStringMESA;

#endif

#endif // OpenGL desktop or ES

};

#endif // __StGLFunctions_h_
