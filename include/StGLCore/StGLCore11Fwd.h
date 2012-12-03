/**
 * Copyright © 2012 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#ifndef __StGLCore11Fwd_h_
#define __StGLCore11Fwd_h_

#include <StGL/StGLFunctions.h>

/**
 * OpenGL 1.1 core without deprecated Fixed Pipeline entry points.
 * Notice that all functions within this structure are actually exported by system GL library.
 * The main puprose for these hint - to control visibility of functions per GL version
 * (global functions shouldn't be used directly to achieve this effect!).
 */
struct ST_LOCAL StGLCore11Fwd : protected StGLFunctions {

        public: //! @name Miscellaneous

    void glClearIndex(GLfloat c) {
        ::glClearIndex(c);
    }

    void glClearColor(GLclampf theRed, GLclampf theGreen, GLclampf theBlue, GLclampf theAlpha) {
        ::glClearColor(theRed, theGreen, theBlue, theAlpha);
    }

    void glClear(GLbitfield theMask) {
        ::glClear(theMask);
    }

    void glIndexMask(GLuint theMask) {
        ::glIndexMask(theMask);
    }

    void glColorMask(GLboolean theRed, GLboolean theGreen, GLboolean theBlue, GLboolean theAlpha) {
        ::glColorMask(theRed, theGreen, theBlue, theAlpha);
    }

    void glAlphaFunc(GLenum theFunc, GLclampf theRef) {
        ::glAlphaFunc(theFunc, theRef);
    }

    void glBlendFunc(GLenum sfactor, GLenum dfactor) {
        ::glBlendFunc(sfactor, dfactor);
    }

    void glLogicOp(GLenum opcode) {
        ::glLogicOp(opcode);
    }

    void glCullFace(GLenum theMode) {
        ::glCullFace(theMode);
    }

    void glFrontFace(GLenum theMode) {
        ::glFrontFace(theMode);
    }

    void glPointSize(GLfloat theSize) {
        ::glPointSize(theSize);
    }

    void glLineWidth(GLfloat theWidth) {
        ::glLineWidth(theWidth);
    }

    void glPolygonMode(GLenum theFace, GLenum theMode) {
        ::glPolygonMode(theFace, theMode);
    }

    void glPolygonOffset(GLfloat theFactor, GLfloat theUnits) {
        ::glPolygonOffset(theFactor, theUnits);
    }

    void glScissor(GLint theX, GLint theY, GLsizei theWidth, GLsizei theHeight) {
        ::glScissor(theX, theY, theWidth, theHeight);
    }

    void glDrawBuffer(GLenum theMode) {
        ::glDrawBuffer(theMode);
    }

    void glReadBuffer(GLenum theMode) {
        ::glReadBuffer(theMode);
    }

    void glEnable(GLenum theCap) {
        ::glEnable(theCap);
    }

    void glDisable(GLenum theCap) {
        ::glDisable(theCap);
    }

    GLboolean glIsEnabled(GLenum theCap) {
        return ::glIsEnabled(theCap);
    }

    void glGetBooleanv(GLenum theParamName, GLboolean* theValues) {
        ::glGetBooleanv(theParamName, theValues);
    }

    void glGetDoublev(GLenum theParamName, GLdouble* theValues) {
        ::glGetDoublev(theParamName, theValues);
    }

    void glGetFloatv(GLenum theParamName, GLfloat* theValues) {
        ::glGetFloatv(theParamName, theValues);
    }

    void glGetIntegerv(GLenum theParamName, GLint* theValues) {
        ::glGetIntegerv(theParamName, theValues);
    }

    GLint glRenderMode(GLenum theMode) {
        return ::glRenderMode(theMode);
    }

    GLenum glGetError() {
        return ::glGetError();
    }

    const GLubyte* glGetString(GLenum theName) {
        return ::glGetString(theName);
    }

    void glFinish() {
        ::glFinish();
    }

    void glFlush() {
        ::glFlush();
    }

    void glHint(GLenum theTarget, GLenum theMode) {
        ::glHint(theTarget, theMode);
    }

        public: //! @name Depth Buffer

    void glClearDepth(GLclampd theDepth) {
        ::glClearDepth(theDepth);
    }

    void glDepthFunc(GLenum theFunc) {
        ::glDepthFunc(theFunc);
    }

    void glDepthMask(GLboolean theFlag) {
        ::glDepthMask(theFlag);
    }

    void glDepthRange(GLclampd theNearValue, GLclampd theFarValue) {
        ::glDepthRange(theNearValue, theFarValue);
    }

        public: //! @name Transformation

    void glViewport(GLint theX, GLint theY, GLsizei theWidth, GLsizei theHeight) {
        ::glViewport(theX, theY, theWidth, theHeight);
    }

        public: //! @name Vertex Arrays

    void glArrayElement(GLint i) {
        ::glArrayElement(i);
    }

    void glDrawArrays(GLenum theMode, GLint theFirst, GLsizei theCount) {
        ::glDrawArrays(theMode, theFirst, theCount);
    }

    void glDrawElements(GLenum theMode, GLsizei theCount, GLenum theType, const GLvoid* theIndices) {
        ::glDrawElements(theMode, theCount, theType, theIndices);
    }

        public: //! @name Raster functions

    void glPixelStoref(GLenum theParamName, GLfloat theParam) {
        ::glPixelStoref(theParamName, theParam);
    }

    void glPixelStorei(GLenum theParamName, GLint   theParam) {
        ::glPixelStorei(theParamName, theParam);
    }

    void glPixelTransferf(GLenum theParamName, GLfloat theParam) {
        ::glPixelTransferf(theParamName, theParam);
    }

    void glPixelTransferi(GLenum theParamName, GLint   theParam) {
        ::glPixelTransferi(theParamName, theParam);
    }

    void glPixelMapfv (GLenum map, GLsizei mapsize, const GLfloat*  values) {
        ::glPixelMapfv (map, mapsize, values);
    }

    void glPixelMapuiv(GLenum map, GLsizei mapsize, const GLuint*   values) {
        ::glPixelMapuiv(map, mapsize, values);
    }

    void glPixelMapusv(GLenum map, GLsizei mapsize, const GLushort* values) {
        ::glPixelMapusv(map, mapsize, values);
    }

    void glGetPixelMapfv (GLenum map, GLfloat*  values) {
        ::glGetPixelMapfv (map, values);
    }

    void glGetPixelMapuiv(GLenum map, GLuint*   values) {
        ::glGetPixelMapuiv(map, values);
    }

    void glGetPixelMapusv(GLenum map, GLushort* values) {
        ::glGetPixelMapusv(map, values);
    }

    void glReadPixels(GLint x, GLint y,
                      GLsizei width, GLsizei height,
                      GLenum format, GLenum type,
                      GLvoid* pixels) {
        ::glReadPixels(x, y, width, height, format, type, pixels);
    }

        public: //! @name Stenciling

    void glStencilFunc(GLenum func, GLint ref, GLuint mask) {
        ::glStencilFunc(func, ref, mask);
    }

    void glStencilMask(GLuint mask) {
        ::glStencilMask(mask);
    }

    void glStencilOp(GLenum fail, GLenum zfail, GLenum zpass) {
        ::glStencilOp(fail, zfail, zpass);
    }

    void glClearStencil(GLint s) {
        ::glClearStencil(s);
    }

        public: //! @name Texture mapping

    void glTexEnvf(GLenum target, GLenum pname, GLfloat param) {
        ::glTexEnvf(target, pname, param);
    }

    void glTexEnvi(GLenum target, GLenum pname, GLint param) {
        ::glTexEnvi(target, pname, param);
    }

    void glTexEnvfv(GLenum target, GLenum pname, const GLfloat* params) {
        ::glTexEnvfv(target, pname, params);
    }

    void glTexEnviv(GLenum target, GLenum pname, const GLint* params) {
        ::glTexEnviv(target, pname, params);
    }

    void glGetTexEnvfv(GLenum target, GLenum pname, GLfloat* params) {
        ::glGetTexEnvfv(target, pname, params);
    }

    void glGetTexEnviv(GLenum target, GLenum pname, GLint* params) {
        ::glGetTexEnviv(target, pname, params);
    }

    void glTexParameterf(GLenum target, GLenum pname, GLfloat param) {
        ::glTexParameterf(target, pname, param);
    }

    void glTexParameteri(GLenum target, GLenum pname, GLint param) {
        ::glTexParameteri(target, pname, param);
    }

    void glTexParameterfv(GLenum target, GLenum pname, const GLfloat* params) {
        ::glTexParameterfv(target, pname, params);
    }

    void glTexParameteriv(GLenum target, GLenum pname, const GLint* params) {
        ::glTexParameteriv(target, pname, params);
    }

    void glGetTexParameterfv(GLenum target, GLenum pname, GLfloat* params) {
        ::glGetTexParameterfv(target, pname, params);
    }

    void glGetTexParameteriv(GLenum target, GLenum pname, GLint* params) {
        ::glGetTexParameteriv(target, pname, params);
    }

    void glGetTexLevelParameterfv(GLenum target, GLint level, GLenum pname, GLfloat* params) {
        ::glGetTexLevelParameterfv(target, level, pname, params);
    }

    void glGetTexLevelParameteriv(GLenum target, GLint level, GLenum pname, GLint* params) {
        ::glGetTexLevelParameteriv(target, level, pname, params);
    }

    void glTexImage1D(GLenum target, GLint level,
                      GLint internalFormat,
                      GLsizei width, GLint border,
                      GLenum format, GLenum type,
                      const GLvoid* pixels) {
        ::glTexImage1D(target, level, internalFormat, width, border, format, type, pixels);
    }

    void glTexImage2D(GLenum target, GLint level,
                      GLint internalFormat,
                      GLsizei width, GLsizei height,
                      GLint border, GLenum format, GLenum type,
                      const GLvoid* pixels) {
        ::glTexImage2D(target, level, internalFormat, width, height, border, format, type, pixels);
    }

    void glGetTexImage(GLenum target, GLint level,
                       GLenum format, GLenum type,
                       GLvoid* pixels) {
        ::glGetTexImage(target, level, format, type, pixels);
    }

    void glGenTextures(GLsizei n, GLuint* textures) {
        ::glGenTextures(n, textures);
    }

    void glDeleteTextures(GLsizei n, const GLuint* textures) {
        ::glDeleteTextures(n, textures);
    }

    void glBindTexture(GLenum target, GLuint texture) {
        ::glBindTexture(target, texture);
    }

    GLboolean glIsTexture(GLuint texture) {
        return ::glIsTexture(texture);
    }

    void glTexSubImage1D(GLenum target, GLint level,
                         GLint xoffset,
                         GLsizei width, GLenum format,
                         GLenum type, const GLvoid* pixels) {
        ::glTexSubImage1D(target, level, xoffset, width, format, type, pixels);
    }


    void glTexSubImage2D(GLenum target, GLint level,
                         GLint xoffset, GLint yoffset,
                         GLsizei width, GLsizei height,
                         GLenum format, GLenum type,
                         const GLvoid* pixels) {
        ::glTexSubImage2D(target, level, xoffset, yoffset, width, height, format, type, pixels);
    }

    void glCopyTexImage1D(GLenum target, GLint level,
                          GLenum internalformat,
                          GLint x, GLint y,
                          GLsizei width, GLint border) {
        ::glCopyTexImage1D(target, level, internalformat, x, y, width, border);
    }

    void glCopyTexImage2D(GLenum target, GLint level,
                          GLenum internalformat,
                          GLint x, GLint y,
                          GLsizei width, GLsizei height,
                          GLint border) {
        ::glCopyTexImage2D(target, level, internalformat, x, y, width, height, border);
    }


    void glCopyTexSubImage1D(GLenum target, GLint level,
                             GLint xoffset, GLint x, GLint y,
                             GLsizei width) {
        ::glCopyTexSubImage1D(target, level, xoffset, x, y, width);
    }

    void glCopyTexSubImage2D(GLenum target, GLint level,
                             GLint xoffset, GLint yoffset,
                             GLint x, GLint y,
                             GLsizei width, GLsizei height) {
        ::glCopyTexSubImage2D(target, level, xoffset, yoffset, x, y, width, height);
    }

};

/*#undef GL_LINE_STIPPLE
#undef GL_POINT_SMOOTH
#undef GL_POINT_SPRITE

#undef GL_QUADS
#undef GL_QUAD_STRIP
#undef GL_POLYGON
#undef GL_POLYGON_STIPPLE

#undef GL_LIGHTING
#undef GL_LIGHT0
#undef GL_LIGHT1
#undef GL_LIGHT2
#undef GL_LIGHT3
#undef GL_LIGHT4
#undef GL_LIGHT5
#undef GL_LIGHT6
#undef GL_LIGHT7
#undef GL_COLOR_MATERIAL
#undef GL_VERTEX_PROGRAM_TWO_SIDE

#undef GL_BITMAP
#undef GL_ALPHA
#undef GL_LUMINANCE
#undef GL_LUMINANCE_ALPHA
#undef GL_INTENSITY

#undef GL_GENERATE_MIPMAP

#undef GL_TEXTURE_PRIORITY
#undef GL_TEXTURE_ENV
#undef GL_TEXTURE_FILTER_CONTROL
#undef GL_TEXTURE_LOD_BIAS

#undef GL_ACCUM_BUFFER_BIT*/

#endif // __StGLCore11Fwd_h_
