/**
 * Copyright © 2012-2014 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
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
struct StGLCore11Fwd : protected StGLFunctions {

        public: //! @name Miscellaneous

    ST_LOCAL inline
    void glClearColor(GLclampf theRed, GLclampf theGreen, GLclampf theBlue, GLclampf theAlpha) {
        ::glClearColor(theRed, theGreen, theBlue, theAlpha);
    }

    ST_LOCAL inline
    void glClear(GLbitfield theMask) {
        ::glClear(theMask);
    }

    ST_LOCAL inline
    void glColorMask(GLboolean theRed, GLboolean theGreen, GLboolean theBlue, GLboolean theAlpha) {
        ::glColorMask(theRed, theGreen, theBlue, theAlpha);
    }

    ST_LOCAL inline
    void glBlendFunc(GLenum sfactor, GLenum dfactor) {
        ::glBlendFunc(sfactor, dfactor);
    }

    ST_LOCAL inline
    void glCullFace(GLenum theMode) {
        ::glCullFace(theMode);
    }

    ST_LOCAL inline
    void glFrontFace(GLenum theMode) {
        ::glFrontFace(theMode);
    }

    ST_LOCAL inline
    void glLineWidth(GLfloat theWidth) {
        ::glLineWidth(theWidth);
    }

    ST_LOCAL inline
    void glPolygonOffset(GLfloat theFactor, GLfloat theUnits) {
        ::glPolygonOffset(theFactor, theUnits);
    }

    ST_LOCAL inline
    void glScissor(GLint theX, GLint theY, GLsizei theWidth, GLsizei theHeight) {
        ::glScissor(theX, theY, theWidth, theHeight);
    }

    ST_LOCAL inline
    void glEnable(GLenum theCap) {
        ::glEnable(theCap);
    }

    ST_LOCAL inline
    void glDisable(GLenum theCap) {
        ::glDisable(theCap);
    }

    ST_LOCAL inline
    GLboolean glIsEnabled(GLenum theCap) {
        return ::glIsEnabled(theCap);
    }

    ST_LOCAL inline
    void glGetBooleanv(GLenum theParamName, GLboolean* theValues) {
        ::glGetBooleanv(theParamName, theValues);
    }

    ST_LOCAL inline
    void glGetFloatv(GLenum theParamName, GLfloat* theValues) {
        ::glGetFloatv(theParamName, theValues);
    }

    ST_LOCAL inline
    void glGetIntegerv(GLenum theParamName, GLint* theValues) {
        ::glGetIntegerv(theParamName, theValues);
    }

    ST_LOCAL inline
    GLenum glGetError() {
        return ::glGetError();
    }

    ST_LOCAL inline
    const GLubyte* glGetString(GLenum theName) {
        return ::glGetString(theName);
    }

    ST_LOCAL inline
    void glFinish() {
        ::glFinish();
    }

    ST_LOCAL inline
    void glFlush() {
        ::glFlush();
    }

    ST_LOCAL inline
    void glHint(GLenum theTarget, GLenum theMode) {
        ::glHint(theTarget, theMode);
    }

        public: //! @name Depth Buffer

    ST_LOCAL inline
    void glClearDepth(GLclampd theDepth) {
    #if defined(GL_ES_VERSION_2_0)
        ::glClearDepthf((GLfloat )theDepth);
    #else
        ::glClearDepth(theDepth);
    #endif
    }

    ST_LOCAL inline
    void glClearDepthf(GLfloat theDepth) {
    #if defined(GL_ES_VERSION_2_0)
        ::glClearDepthf(theDepth);
    #else
        ::glClearDepth((GLclampd )theDepth);
    #endif
    }

    ST_LOCAL inline
    void glDepthFunc(GLenum theFunc) {
        ::glDepthFunc(theFunc);
    }

    ST_LOCAL inline
    void glDepthMask(GLboolean theFlag) {
        ::glDepthMask(theFlag);
    }

    ST_LOCAL inline
    void glDepthRange(GLclampd theNearValue,
                      GLclampd theFarValue) {
    #if defined(GL_ES_VERSION_2_0)
        ::glDepthRangef((GLfloat )theNearValue, (GLfloat )theFarValue);
    #else
        ::glDepthRange(theNearValue, theFarValue);
    #endif
    }

    ST_LOCAL inline
    void glDepthRangef(GLfloat theNearValue,
                       GLfloat theFarValue) {
    #if defined(GL_ES_VERSION_2_0)
        ::glDepthRangef(theNearValue, theFarValue);
    #else
        ::glDepthRange((GLclampd )theNearValue, (GLclampd )theFarValue);
    #endif
    }

        public: //! @name Transformation

    ST_LOCAL inline
    void glViewport(GLint theX, GLint theY, GLsizei theWidth, GLsizei theHeight) {
        ::glViewport(theX, theY, theWidth, theHeight);
    }

        public: //! @name Vertex Arrays

    ST_LOCAL inline
    void glDrawArrays(GLenum theMode, GLint theFirst, GLsizei theCount) {
        ::glDrawArrays(theMode, theFirst, theCount);
    }

    ST_LOCAL inline
    void glDrawElements(GLenum theMode, GLsizei theCount, GLenum theType, const GLvoid* theIndices) {
        ::glDrawElements(theMode, theCount, theType, theIndices);
    }

        public: //! @name Raster functions

    ST_LOCAL inline
    void glPixelStorei(GLenum theParamName, GLint   theParam) {
        ::glPixelStorei(theParamName, theParam);
    }

    ST_LOCAL inline
    void glReadPixels(GLint x, GLint y,
                      GLsizei width, GLsizei height,
                      GLenum format, GLenum type,
                      GLvoid* pixels) {
        ::glReadPixels(x, y, width, height, format, type, pixels);
    }

        public: //! @name Stenciling

    ST_LOCAL inline
    void glStencilFunc(GLenum func, GLint ref, GLuint mask) {
        ::glStencilFunc(func, ref, mask);
    }

    ST_LOCAL inline
    void glStencilMask(GLuint mask) {
        ::glStencilMask(mask);
    }

    ST_LOCAL inline
    void glStencilOp(GLenum fail, GLenum zfail, GLenum zpass) {
        ::glStencilOp(fail, zfail, zpass);
    }

    ST_LOCAL inline
    void glClearStencil(GLint s) {
        ::glClearStencil(s);
    }

        public: //! @name Texture mapping

    ST_LOCAL inline
    void glTexParameterf(GLenum target, GLenum pname, GLfloat param) {
        ::glTexParameterf(target, pname, param);
    }

    ST_LOCAL inline
    void glTexParameteri(GLenum target, GLenum pname, GLint param) {
        ::glTexParameteri(target, pname, param);
    }

    ST_LOCAL inline
    void glTexParameterfv(GLenum target, GLenum pname, const GLfloat* params) {
        ::glTexParameterfv(target, pname, params);
    }

    ST_LOCAL inline
    void glTexParameteriv(GLenum target, GLenum pname, const GLint* params) {
        ::glTexParameteriv(target, pname, params);
    }

    ST_LOCAL inline
    void glGetTexParameterfv(GLenum target, GLenum pname, GLfloat* params) {
        ::glGetTexParameterfv(target, pname, params);
    }

    ST_LOCAL inline
    void glGetTexParameteriv(GLenum target, GLenum pname, GLint* params) {
        ::glGetTexParameteriv(target, pname, params);
    }

    ST_LOCAL inline
    void glTexImage2D(GLenum target, GLint level,
                      GLint internalFormat,
                      GLsizei width, GLsizei height,
                      GLint border, GLenum format, GLenum type,
                      const GLvoid* pixels) {
        ::glTexImage2D(target, level, internalFormat, width, height, border, format, type, pixels);
    }

    ST_LOCAL inline
    void glGenTextures(GLsizei n, GLuint* textures) {
        ::glGenTextures(n, textures);
    }

    ST_LOCAL inline
    void glDeleteTextures(GLsizei n, const GLuint* textures) {
        ::glDeleteTextures(n, textures);
    }

    ST_LOCAL inline
    void glBindTexture(GLenum target, GLuint texture) {
        ::glBindTexture(target, texture);
    }

    ST_LOCAL inline
    GLboolean glIsTexture(GLuint texture) {
        return ::glIsTexture(texture);
    }

    ST_LOCAL inline
    void glTexSubImage2D(GLenum target, GLint level,
                         GLint xoffset, GLint yoffset,
                         GLsizei width, GLsizei height,
                         GLenum format, GLenum type,
                         const GLvoid* pixels) {
        ::glTexSubImage2D(target, level, xoffset, yoffset, width, height, format, type, pixels);
    }

    ST_LOCAL inline
    void glCopyTexImage2D(GLenum target, GLint level,
                          GLenum internalformat,
                          GLint x, GLint y,
                          GLsizei width, GLsizei height,
                          GLint border) {
        ::glCopyTexImage2D(target, level, internalformat, x, y, width, height, border);
    }

    ST_LOCAL inline
    void glCopyTexSubImage2D(GLenum target, GLint level,
                             GLint xoffset, GLint yoffset,
                             GLint x, GLint y,
                             GLsizei width, GLsizei height) {
        ::glCopyTexSubImage2D(target, level, xoffset, yoffset, x, y, width, height);
    }

#if !defined(GL_ES_VERSION_2_0)
    ST_LOCAL inline
    void glGetTexLevelParameteriv(GLenum target, GLint level, GLenum pname, GLint* params) {
        ::glGetTexLevelParameteriv(target, level, pname, params);
    }

    ST_LOCAL inline
    void glGetTexImage(GLenum target, GLint level,
                       GLenum format, GLenum type,
                       GLvoid* pixels) {
        ::glGetTexImage(target, level, format, type, pixels);
    }

    ST_LOCAL inline
    void glDrawBuffer(GLenum theMode) {
        ::glDrawBuffer(theMode);
    }

    ST_LOCAL inline
    void glReadBuffer(GLenum theMode) {
        ::glReadBuffer(theMode);
    }
#endif

};

/**
 * OpenGL 1.1 core desktop.
 */
/*struct StGLCore11FwdDesktop : protected StGLCore11Fwd {

    ST_LOCAL inline
    void glClearIndex(GLfloat c) {
        ::glClearIndex(c);
    }

    ST_LOCAL inline
    void glIndexMask(GLuint theMask) {
        ::glIndexMask(theMask);
    }

    ST_LOCAL inline
    void glAlphaFunc(GLenum theFunc, GLclampf theRef) {
        ::glAlphaFunc(theFunc, theRef);
    }

    ST_LOCAL inline
    void glLogicOp(GLenum opcode) {
        ::glLogicOp(opcode);
    }

    ST_LOCAL inline
    void glPointSize(GLfloat theSize) {
        ::glPointSize(theSize);
    }

    ST_LOCAL inline
    void glPolygonMode(GLenum theFace, GLenum theMode) {
        ::glPolygonMode(theFace, theMode);
    }

    ST_LOCAL inline
    void glGetDoublev(GLenum theParamName, GLdouble* theValues) {
        ::glGetDoublev(theParamName, theValues);
    }

    ST_LOCAL inline
    GLint glRenderMode(GLenum theMode) {
        return ::glRenderMode(theMode);
    }

    ST_LOCAL inline
    void glArrayElement(GLint i) {
        ::glArrayElement(i);
    }

    ST_LOCAL inline
    void glPixelStoref(GLenum theParamName, GLfloat theParam) {
        ::glPixelStoref(theParamName, theParam);
    }

    ST_LOCAL inline
    void glPixelTransferf(GLenum theParamName, GLfloat theParam) {
        ::glPixelTransferf(theParamName, theParam);
    }

    ST_LOCAL inline
    void glPixelTransferi(GLenum theParamName, GLint   theParam) {
        ::glPixelTransferi(theParamName, theParam);
    }

    ST_LOCAL inline
    void glPixelMapfv (GLenum map, GLsizei mapsize, const GLfloat*  values) {
        ::glPixelMapfv (map, mapsize, values);
    }

    ST_LOCAL inline
    void glPixelMapuiv(GLenum map, GLsizei mapsize, const GLuint*   values) {
        ::glPixelMapuiv(map, mapsize, values);
    }

    ST_LOCAL inline
    void glPixelMapusv(GLenum map, GLsizei mapsize, const GLushort* values) {
        ::glPixelMapusv(map, mapsize, values);
    }

    ST_LOCAL inline
    void glGetPixelMapfv (GLenum map, GLfloat*  values) {
        ::glGetPixelMapfv (map, values);
    }

    ST_LOCAL inline
    void glGetPixelMapuiv(GLenum map, GLuint*   values) {
        ::glGetPixelMapuiv(map, values);
    }

    ST_LOCAL inline
    void glGetPixelMapusv(GLenum map, GLushort* values) {
        ::glGetPixelMapusv(map, values);
    }

    ST_LOCAL inline
    void glTexEnvf(GLenum target, GLenum pname, GLfloat param) {
        ::glTexEnvf(target, pname, param);
    }

    ST_LOCAL inline
    void glTexEnvi(GLenum target, GLenum pname, GLint param) {
        ::glTexEnvi(target, pname, param);
    }

    ST_LOCAL inline
    void glTexEnvfv(GLenum target, GLenum pname, const GLfloat* params) {
        ::glTexEnvfv(target, pname, params);
    }

    ST_LOCAL inline
    void glTexEnviv(GLenum target, GLenum pname, const GLint* params) {
        ::glTexEnviv(target, pname, params);
    }

    ST_LOCAL inline
    void glGetTexEnvfv(GLenum target, GLenum pname, GLfloat* params) {
        ::glGetTexEnvfv(target, pname, params);
    }

    ST_LOCAL inline
    void glGetTexEnviv(GLenum target, GLenum pname, GLint* params) {
        ::glGetTexEnviv(target, pname, params);
    }

    ST_LOCAL inline
    void glGetTexLevelParameterfv(GLenum target, GLint level, GLenum pname, GLfloat* params) {
        ::glGetTexLevelParameterfv(target, level, pname, params);
    }

    ST_LOCAL inline
    void glTexImage1D(GLenum target, GLint level,
                      GLint internalFormat,
                      GLsizei width, GLint border,
                      GLenum format, GLenum type,
                      const GLvoid* pixels) {
        ::glTexImage1D(target, level, internalFormat, width, border, format, type, pixels);
    }

    ST_LOCAL inline
    void glTexSubImage1D(GLenum target, GLint level,
                         GLint xoffset,
                         GLsizei width, GLenum format,
                         GLenum type, const GLvoid* pixels) {
        ::glTexSubImage1D(target, level, xoffset, width, format, type, pixels);
    }

    ST_LOCAL inline
    void glCopyTexImage1D(GLenum target, GLint level,
                          GLenum internalformat,
                          GLint x, GLint y,
                          GLsizei width, GLint border) {
        ::glCopyTexImage1D(target, level, internalformat, x, y, width, border);
    }

    ST_LOCAL inline
    void glCopyTexSubImage1D(GLenum target, GLint level,
                             GLint xoffset, GLint x, GLint y,
                             GLsizei width) {
        ::glCopyTexSubImage1D(target, level, xoffset, x, y, width);
    }

};*/

#endif // __StGLCore11Fwd_h_
