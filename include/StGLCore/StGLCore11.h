/**
 * Copyright © 2012 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#ifndef __StGLCore11_h_
#define __StGLCore11_h_

#include <StGLCore/StGLCore11Fwd.h>

/**
 * OpenGL 1.1 core.
 * Notice that all functions within this structure are actually exported by system GL library.
 * The main puprose for these hint - to control visibility of functions per GL version
 * (global functions shouldn't be used directly to achieve this effect!).
 */
struct StGLCore11 : public StGLCore11Fwd {

#if !defined(GL_ES_VERSION_2_0)

        public: //! @name Begin/End primitive specification (removed since 3.1)

    ST_LOCAL inline
    void glBegin(GLenum theMode) {
        ::glBegin(theMode);
    }

    ST_LOCAL inline
    void glEnd() {
        ::glEnd();
    }

    ST_LOCAL inline
    void glVertex2d(GLdouble theX, GLdouble theY) {
        ::glVertex2d(theX, theY);
    }

    ST_LOCAL inline
    void glVertex2f(GLfloat  theX, GLfloat  theY) {
        ::glVertex2f(theX, theY);
    }

    ST_LOCAL inline
    void glVertex2i(GLint    theX, GLint    theY) {
        ::glVertex2i(theX, theY);
    }

    ST_LOCAL inline
    void glVertex2s(GLshort  theX, GLshort  theY) {
        ::glVertex2s(theX, theY);
    }

    ST_LOCAL inline
    void glVertex3d(GLdouble theX, GLdouble theY, GLdouble theZ) {
        ::glVertex3d(theX, theY, theZ);
    }

    ST_LOCAL inline
    void glVertex3f(GLfloat  theX, GLfloat  theY, GLfloat  theZ) {
        ::glVertex3f(theX, theY, theZ);
    }

    ST_LOCAL inline
    void glVertex3i(GLint    theX, GLint    theY, GLint    theZ) {
        ::glVertex3i(theX, theY, theZ);
    }

    ST_LOCAL inline
    void glVertex3s(GLshort  theX, GLshort  theY, GLshort  theZ) {
        ::glVertex3s(theX, theY, theZ);
    }

    ST_LOCAL inline
    void glVertex4d(GLdouble theX, GLdouble theY, GLdouble theZ, GLdouble theW) {
        ::glVertex4d(theX, theY, theZ, theW);
    }

    ST_LOCAL inline
    void glVertex4f(GLfloat  theX, GLfloat  theY, GLfloat  theZ, GLfloat  theW) {
        ::glVertex4f(theX, theY, theZ, theW);
    }

    ST_LOCAL inline
    void glVertex4i(GLint    theX, GLint    theY, GLint    theZ, GLint    theW) {
        ::glVertex4i(theX, theY, theZ, theW);
    }

    ST_LOCAL inline
    void glVertex4s(GLshort  theX, GLshort  theY, GLshort  theZ, GLshort  theW) {
        ::glVertex4s(theX, theY, theZ, theW);
    }

    ST_LOCAL inline
    void glVertex2dv(const GLdouble* theVec2) {
        ::glVertex2dv(theVec2);
    }

    ST_LOCAL inline
    void glVertex2fv(const GLfloat*  theVec2) {
        ::glVertex2fv(theVec2);
    }

    ST_LOCAL inline
    void glVertex2iv(const GLint*    theVec2) {
        ::glVertex2iv(theVec2);
    }

    ST_LOCAL inline
    void glVertex2sv(const GLshort*  theVec2) {
        ::glVertex2sv(theVec2);
    }

    ST_LOCAL inline
    void glVertex3dv(const GLdouble* theVec3) {
        ::glVertex3dv(theVec3);
    }

    ST_LOCAL inline
    void glVertex3fv(const GLfloat*  theVec3) {
        ::glVertex3fv(theVec3);
    }

    ST_LOCAL inline
    void glVertex3iv(const GLint*    theVec3) {
        ::glVertex3iv(theVec3);
    }

    ST_LOCAL inline
    void glVertex3sv(const GLshort*  theVec3) {
        ::glVertex3sv(theVec3);
    }

    ST_LOCAL inline
    void glVertex4dv(const GLdouble* theVec4) {
        ::glVertex4dv(theVec4);
    }

    ST_LOCAL inline
    void glVertex4fv(const GLfloat*  theVec4) {
        ::glVertex4fv(theVec4);
    }

    ST_LOCAL inline
    void glVertex4iv(const GLint*    theVec4) {
        ::glVertex4iv(theVec4);
    }

    ST_LOCAL inline
    void glVertex4sv(const GLshort*  theVec4) {
        ::glVertex4sv(theVec4);
    }

    ST_LOCAL inline
    void glNormal3b(GLbyte   nx, GLbyte   ny, GLbyte   nz) {
        ::glNormal3b(nx, ny, nz);
    }

    ST_LOCAL inline
    void glNormal3d(GLdouble nx, GLdouble ny, GLdouble nz) {
        ::glNormal3d(nx, ny, nz);
    }

    ST_LOCAL inline
    void glNormal3f(GLfloat  nx, GLfloat  ny, GLfloat  nz) {
        ::glNormal3f(nx, ny, nz);
    }

    ST_LOCAL inline
    void glNormal3i(GLint    nx, GLint    ny, GLint    nz) {
        ::glNormal3i(nx, ny, nz);
    }

    ST_LOCAL inline
    void glNormal3s(GLshort nx, GLshort ny, GLshort nz) {
        ::glNormal3s(nx, ny, nz);
    }

    ST_LOCAL inline
    void glNormal3bv(const GLbyte*   theVec) {
        ::glNormal3bv(theVec);
    }

    ST_LOCAL inline
    void glNormal3dv(const GLdouble* theVec) {
        ::glNormal3dv(theVec);
    }

    ST_LOCAL inline
    void glNormal3fv(const GLfloat*  theVec) {
        ::glNormal3fv(theVec);
    }

    ST_LOCAL inline
    void glNormal3iv(const GLint*    theVec) {
        ::glNormal3iv(theVec);
    }

    ST_LOCAL inline
    void glNormal3sv(const GLshort*  theVec) {
        ::glNormal3sv(theVec);
    }

    ST_LOCAL inline
    void glIndexd(GLdouble c) {
        ::glIndexd(c);
    }

    ST_LOCAL inline
    void glIndexf(GLfloat c) {
        ::glIndexf(c);
    }

    ST_LOCAL inline
    void glIndexi(GLint c) {
        ::glIndexi(c);
    }

    ST_LOCAL inline
    void glIndexs(GLshort c) {
        ::glIndexs(c);
    }

    ST_LOCAL inline
    void glIndexub(GLubyte c) {
        ::glIndexub(c);
    }

    ST_LOCAL inline
    void glIndexdv(const GLdouble* c) {
        ::glIndexdv(c);
    }

    ST_LOCAL inline
    void glIndexfv(const GLfloat* c) {
        ::glIndexfv(c);
    }

    ST_LOCAL inline
    void glIndexiv(const GLint* c) {
        ::glIndexiv(c);
    }

    ST_LOCAL inline
    void glIndexsv(const GLshort* c) {
        ::glIndexsv(c);
    }

    ST_LOCAL inline
    void glIndexubv(const GLubyte* c) {
        ::glIndexubv(c);
    }

    ST_LOCAL inline
    void glColor3b(GLbyte theRed, GLbyte theGreen, GLbyte theBlue) {
        ::glColor3b(theRed, theGreen, theBlue);
    }

    ST_LOCAL inline
    void glColor3d(GLdouble theRed, GLdouble theGreen, GLdouble theBlue) {
        ::glColor3d(theRed, theGreen, theBlue);
    }

    ST_LOCAL inline
    void glColor3f(GLfloat theRed, GLfloat theGreen, GLfloat theBlue) {
        ::glColor3f(theRed, theGreen, theBlue);
    }

    ST_LOCAL inline
    void glColor3i(GLint theRed, GLint theGreen, GLint theBlue) {
        ::glColor3i(theRed, theGreen, theBlue);
    }

    ST_LOCAL inline
    void glColor3s(GLshort theRed, GLshort theGreen, GLshort theBlue) {
        ::glColor3s(theRed, theGreen, theBlue);
    }

    ST_LOCAL inline
    void glColor3ub(GLubyte theRed, GLubyte theGreen, GLubyte theBlue) {
        ::glColor3ub(theRed, theGreen, theBlue);
    }

    ST_LOCAL inline
    void glColor3ui(GLuint theRed, GLuint theGreen, GLuint theBlue) {
        ::glColor3ui(theRed, theGreen, theBlue);
    }

    ST_LOCAL inline
    void glColor3us(GLushort theRed, GLushort theGreen, GLushort theBlue) {
        ::glColor3us(theRed, theGreen, theBlue);
    }

    ST_LOCAL inline
    void glColor4b(GLbyte theRed, GLbyte theGreen, GLbyte theBlue, GLbyte theAlpha) {
        ::glColor4b(theRed, theGreen, theBlue, theAlpha);
    }

    ST_LOCAL inline
    void glColor4d(GLdouble theRed, GLdouble theGreen, GLdouble theBlue, GLdouble theAlpha) {
        ::glColor4d(theRed, theGreen, theBlue, theAlpha);
    }

    ST_LOCAL inline
    void glColor4f(GLfloat theRed, GLfloat theGreen, GLfloat theBlue, GLfloat theAlpha) {
        ::glColor4f(theRed, theGreen, theBlue, theAlpha);
    }

    ST_LOCAL inline
    void glColor4i(GLint theRed, GLint theGreen, GLint theBlue, GLint theAlpha) {
        ::glColor4i(theRed, theGreen, theBlue, theAlpha);
    }

    ST_LOCAL inline
    void glColor4s(GLshort theRed, GLshort theGreen, GLshort theBlue, GLshort theAlpha) {
        ::glColor4s(theRed, theGreen, theBlue, theAlpha);
    }

    ST_LOCAL inline
    void glColor4ub(GLubyte theRed, GLubyte theGreen, GLubyte theBlue, GLubyte theAlpha) {
        ::glColor4ub(theRed, theGreen, theBlue, theAlpha);
    }

    ST_LOCAL inline
    void glColor4ui(GLuint theRed, GLuint theGreen, GLuint theBlue, GLuint theAlpha) {
        ::glColor4ui(theRed, theGreen, theBlue, theAlpha);
    }

    ST_LOCAL inline
    void glColor4us(GLushort theRed, GLushort theGreen, GLushort theBlue, GLushort theAlpha) {
        ::glColor4us(theRed, theGreen, theBlue, theAlpha);
    }

    ST_LOCAL inline
    void glColor3bv(const GLbyte*    theVec) {
        ::glColor3bv(theVec);
    }

    ST_LOCAL inline
    void glColor3dv(const GLdouble*  theVec) {
        ::glColor3dv(theVec);
    }

    ST_LOCAL inline
    void glColor3fv(const GLfloat*   theVec) {
        ::glColor3fv(theVec);
    }

    ST_LOCAL inline
    void glColor3iv(const GLint*     theVec) {
        ::glColor3iv(theVec);
    }

    ST_LOCAL inline
    void glColor3sv(const GLshort*   theVec) {
        ::glColor3sv(theVec);
    }

    ST_LOCAL inline
    void glColor3ubv(const GLubyte*  theVec) {
        ::glColor3ubv(theVec);
    }

    ST_LOCAL inline
    void glColor3uiv(const GLuint*   theVec) {
        ::glColor3uiv(theVec);
    }

    ST_LOCAL inline
    void glColor3usv(const GLushort* theVec) {
        ::glColor3usv(theVec);
    }

    ST_LOCAL inline
    void glColor4bv(const GLbyte*    theVec) {
        ::glColor4bv(theVec);
    }

    ST_LOCAL inline
    void glColor4dv(const GLdouble*  theVec) {
        ::glColor4dv(theVec);
    }

    ST_LOCAL inline
    void glColor4fv(const GLfloat*   theVec) {
        ::glColor4fv(theVec);
    }

    ST_LOCAL inline
    void glColor4iv(const GLint*     theVec) {
        ::glColor4iv(theVec);
    }

    ST_LOCAL inline
    void glColor4sv(const GLshort*   theVec) {
        ::glColor4sv(theVec);
    }

    ST_LOCAL inline
    void glColor4ubv(const GLubyte*  theVec) {
        ::glColor4ubv(theVec);
    }

    ST_LOCAL inline
    void glColor4uiv(const GLuint*   theVec) {
        ::glColor4uiv(theVec);
    }

    ST_LOCAL inline
    void glColor4usv(const GLushort* theVec) {
        ::glColor4usv(theVec);
    }

    ST_LOCAL inline
    void glTexCoord1d(GLdouble s) {
        ::glTexCoord1d(s);
    }

    ST_LOCAL inline
    void glTexCoord1f(GLfloat s) {
        ::glTexCoord1f(s);
    }

    ST_LOCAL inline
    void glTexCoord1i(GLint s) {
        ::glTexCoord1i(s);
    }

    ST_LOCAL inline
    void glTexCoord1s(GLshort s) {
        ::glTexCoord1s(s);
    }

    ST_LOCAL inline
    void glTexCoord2d(GLdouble s, GLdouble t) {
        ::glTexCoord2d(s, t);
    }

    ST_LOCAL inline
    void glTexCoord2f(GLfloat s, GLfloat t) {
        ::glTexCoord2f(s, t);
    }

    ST_LOCAL inline
    void glTexCoord2i(GLint s, GLint t) {
        ::glTexCoord2i(s, t);
    }

    ST_LOCAL inline
    void glTexCoord2s(GLshort s, GLshort t) {
        ::glTexCoord2s(s, t);
    }

    ST_LOCAL inline
    void glTexCoord3d(GLdouble s, GLdouble t, GLdouble r) {
        ::glTexCoord3d(s, t, r);
    }

    ST_LOCAL inline
    void glTexCoord3f(GLfloat s, GLfloat t, GLfloat r) {
        ::glTexCoord3f(s, t, r);
    }

    ST_LOCAL inline
    void glTexCoord3i(GLint s, GLint t, GLint r) {
        ::glTexCoord3i(s, t, r);
    }

    ST_LOCAL inline
    void glTexCoord3s(GLshort s, GLshort t, GLshort r) {
        ::glTexCoord3s(s, t, r);
    }

    ST_LOCAL inline
    void glTexCoord4d(GLdouble s, GLdouble t, GLdouble r, GLdouble q) {
        ::glTexCoord4d(s, t, r, q);
    }

    ST_LOCAL inline
    void glTexCoord4f(GLfloat s, GLfloat t, GLfloat r, GLfloat q) {
        ::glTexCoord4f(s, t, r, q);
    }

    ST_LOCAL inline
    void glTexCoord4i(GLint s, GLint t, GLint r, GLint q) {
        ::glTexCoord4i(s, t, r, q);
    }

    ST_LOCAL inline
    void glTexCoord4s(GLshort s, GLshort t, GLshort r, GLshort q) {
        ::glTexCoord4s(s, t, r, q);
    }

    ST_LOCAL inline
    void glTexCoord1dv(const GLdouble* theVec1) {
        ::glTexCoord1dv(theVec1);
    }

    ST_LOCAL inline
    void glTexCoord1fv(const GLfloat*  theVec1) {
        ::glTexCoord1fv(theVec1);
    }

    ST_LOCAL inline
    void glTexCoord1iv(const GLint*    theVec1) {
        ::glTexCoord1iv(theVec1);
    }

    ST_LOCAL inline
    void glTexCoord1sv(const GLshort*  theVec1) {
        ::glTexCoord1sv(theVec1);
    }

    ST_LOCAL inline
    void glTexCoord2dv(const GLdouble* theVec2) {
        ::glTexCoord2dv(theVec2);
    }

    ST_LOCAL inline
    void glTexCoord2fv(const GLfloat*  theVec2) {
        ::glTexCoord2fv(theVec2);
    }

    ST_LOCAL inline
    void glTexCoord2iv(const GLint*    theVec2) {
        ::glTexCoord2iv(theVec2);
    }

    ST_LOCAL inline
    void glTexCoord2sv(const GLshort*  theVec) {
        ::glTexCoord2sv(theVec);
    }

    ST_LOCAL inline
    void glTexCoord3dv(const GLdouble* theVec3) {
        ::glTexCoord3dv(theVec3);
    }

    ST_LOCAL inline
    void glTexCoord3fv(const GLfloat*  theVec3) {
        ::glTexCoord3fv(theVec3);
    }

    ST_LOCAL inline
    void glTexCoord3iv(const GLint*    theVec3) {
        ::glTexCoord3iv(theVec3);
    }

    ST_LOCAL inline
    void glTexCoord3sv(const GLshort*  theVec3) {
        ::glTexCoord3sv(theVec3);
    }

    ST_LOCAL inline
    void glTexCoord4dv(const GLdouble* theVec4) {
        ::glTexCoord4dv(theVec4);
    }

    ST_LOCAL inline
    void glTexCoord4fv(const GLfloat*  theVec4) {
        ::glTexCoord4fv(theVec4);
    }

    ST_LOCAL inline
    void glTexCoord4iv(const GLint*    theVec4) {
        ::glTexCoord4iv(theVec4);
    }

    ST_LOCAL inline
    void glTexCoord4sv(const GLshort*  theVec4) {
        ::glTexCoord4sv(theVec4);
    }

        public: //! @name Matrix operations (removed since 3.1)

    ST_LOCAL inline
    void glMatrixMode(GLenum theMode) {
        ::glMatrixMode(theMode);
    }

    ST_LOCAL inline
    void glOrtho(GLdouble theLeft,    GLdouble theRight,
                 GLdouble theBottom,  GLdouble theTop,
                 GLdouble theNearVal, GLdouble theFarVal) {
        ::glOrtho(theLeft, theRight, theBottom, theTop, theNearVal, theFarVal);
    }

    ST_LOCAL inline
    void glFrustum(GLdouble theLeft,    GLdouble theRight,
                   GLdouble theBottom,  GLdouble theTop,
                   GLdouble theNearVal, GLdouble theFarVal) {
        ::glFrustum(theLeft, theRight, theBottom, theTop, theNearVal, theFarVal);
    }

    ST_LOCAL inline
    void glPushMatrix() {
        ::glPushMatrix();
    }

    ST_LOCAL inline
    void glPopMatrix() {
        ::glPopMatrix();
    }

    ST_LOCAL inline
    void glLoadIdentity() {
        ::glLoadIdentity();
    }

    ST_LOCAL inline
    void glLoadMatrixd(const GLdouble* theMatrix) {
        ::glLoadMatrixd(theMatrix);
    }

    ST_LOCAL inline
    void glLoadMatrixf(const GLfloat*  theMatrix) {
        ::glLoadMatrixf(theMatrix);
    }

    ST_LOCAL inline
    void glMultMatrixd(const GLdouble* theMatrix) {
        ::glMultMatrixd(theMatrix);
    }

    ST_LOCAL inline
    void glMultMatrixf(const GLfloat*  theMatrix) {
        ::glMultMatrixf(theMatrix);
    }

    ST_LOCAL inline
    void glRotated(GLdouble theAngleDegrees,
                   GLdouble theX, GLdouble theY, GLdouble theZ) {
        ::glRotated(theAngleDegrees, theX, theY, theZ);
    }

    ST_LOCAL inline
    void glRotatef(GLfloat theAngleDegrees,
                   GLfloat theX, GLfloat theY, GLfloat theZ) {
        ::glRotatef(theAngleDegrees, theX, theY, theZ);
    }

    ST_LOCAL inline
    void glScaled(GLdouble theX, GLdouble theY, GLdouble theZ) {
        ::glScaled(theX, theY, theZ);
    }

    ST_LOCAL inline
    void glScalef(GLfloat theX, GLfloat theY, GLfloat theZ) {
        ::glScalef(theX, theY, theZ);
    }

    ST_LOCAL inline
    void glTranslated(GLdouble theX, GLdouble theY, GLdouble theZ) {
        ::glTranslated(theX, theY, theZ);
    }

    ST_LOCAL inline
    void glTranslatef(GLfloat theX, GLfloat theY, GLfloat theZ) {
        ::glTranslatef(theX, theY, theZ);
    }

        public: //! @name Line and Polygon stripple (removed since 3.1)

    //void glLineWidth(GLfloat theWidth) { ::glLineWidth(theWidth); }

    ST_LOCAL inline
    void glLineStipple(GLint theFactor, GLushort thePattern) {
        ::glLineStipple(theFactor, thePattern);
    }

    ST_LOCAL inline
    void glPolygonStipple(const GLubyte* theMask) {
        ::glPolygonStipple(theMask);
    }

    ST_LOCAL inline
    void glGetPolygonStipple(GLubyte* theMask) {
        ::glGetPolygonStipple(theMask);
    }

        public: //! @name Attribute stacks (removed since 3.1)

    ST_LOCAL inline
    void glPushAttrib(GLbitfield theMask) {
        ::glPushAttrib(theMask);
    }

    ST_LOCAL inline
    void glPopAttrib() {
        ::glPopAttrib();
    }

    ST_LOCAL inline
    void glPushClientAttrib(GLbitfield theMask) {
        ::glPushClientAttrib(theMask);
    }

    ST_LOCAL inline
    void glPopClientAttrib() {
        ::glPopClientAttrib();
    }

        public: //! @name Fixed pipeline lighting (removed since 3.1)

    ST_LOCAL inline
    void glShadeModel(GLenum theMode) {
        ::glShadeModel(theMode);
    }

    ST_LOCAL inline
    void glLightf(GLenum theLight, GLenum pname, GLfloat param) {
        ::glLightf(theLight, pname, param);
    }

    ST_LOCAL inline
    void glLighti(GLenum theLight, GLenum pname, GLint param) {
        ::glLighti(theLight, pname, param);
    }

    ST_LOCAL inline
    void glLightfv(GLenum theLight, GLenum pname, const GLfloat* params) {
        ::glLightfv(theLight, pname, params);
    }

    ST_LOCAL inline
    void glLightiv(GLenum theLight, GLenum pname, const GLint* params) {
        ::glLightiv(theLight, pname, params);
    }

    ST_LOCAL inline
    void glGetLightfv(GLenum theLight, GLenum pname, GLfloat *params) {
        ::glGetLightfv(theLight, pname, params);
    }

    ST_LOCAL inline
    void glGetLightiv(GLenum theLight, GLenum pname, GLint *params) {
        ::glGetLightiv(theLight, pname, params);
    }

    ST_LOCAL inline
    void glLightModelf(GLenum pname, GLfloat param) {
        ::glLightModelf(pname, param);
    }

    ST_LOCAL inline
    void glLightModeli(GLenum pname, GLint param) {
        ::glLightModeli(pname, param);
    }

    ST_LOCAL inline
    void glLightModelfv(GLenum pname, const GLfloat* params) {
        ::glLightModelfv(pname, params);
    }

    ST_LOCAL inline
    void glLightModeliv(GLenum pname, const GLint* params) {
        ::glLightModeliv(pname, params);
    }

    ST_LOCAL inline
    void glMaterialf(GLenum face, GLenum pname, GLfloat param) {
        ::glMaterialf(face, pname, param);
    }

    ST_LOCAL inline
    void glMateriali(GLenum face, GLenum pname, GLint param) {
        ::glMateriali(face, pname, param);
    }

    ST_LOCAL inline
    void glMaterialfv(GLenum face, GLenum pname, const GLfloat* params) {
        ::glMaterialfv(face, pname, params);
    }

    ST_LOCAL inline
    void glMaterialiv(GLenum face, GLenum pname, const GLint* params) {
        ::glMaterialiv(face, pname, params);
    }

    ST_LOCAL inline
    void glGetMaterialfv(GLenum face, GLenum pname, GLfloat* params) {
        ::glGetMaterialfv(face, pname, params);
    }

    ST_LOCAL inline
    void glGetMaterialiv(GLenum face, GLenum pname, GLint* params) {
        ::glGetMaterialiv(face, pname, params);
    }

    ST_LOCAL inline
    void glColorMaterial(GLenum face, GLenum mode) {
        ::glColorMaterial(face, mode);
    }

        public: //! @name clipping plane (removed since 3.1)

    ST_LOCAL inline
    void glClipPlane(GLenum thePlane, const GLdouble* theEquation) {
        ::glClipPlane(thePlane, theEquation);
    }

    ST_LOCAL inline
    void glGetClipPlane(GLenum thePlane, GLdouble* theEquation) {
        ::glGetClipPlane(thePlane, theEquation);
    }

        public: //! @name Display lists (removed since 3.1)

    ST_LOCAL inline
    GLboolean glIsList(GLuint theList) {
        return ::glIsList(theList);
    }

    ST_LOCAL inline
    void glDeleteLists(GLuint theList, GLsizei theRange) {
        ::glDeleteLists(theList, theRange);
    }

    ST_LOCAL inline
    GLuint glGenLists(GLsizei theRange) {
        return ::glGenLists(theRange);
    }

    ST_LOCAL inline
    void glNewList(GLuint theList, GLenum theMode) {
        ::glNewList(theList, theMode);
    }

    ST_LOCAL inline
    void glEndList() {
        ::glEndList();
    }

    ST_LOCAL inline
    void glCallList(GLuint theList) {
        ::glCallList(theList);
    }

    ST_LOCAL inline
    void glCallLists(GLsizei theNb, GLenum theType, const GLvoid* theLists) {
        ::glCallLists(theNb, theType, theLists);
    }

    ST_LOCAL inline
    void glListBase(GLuint theBase) {
        ::glListBase(theBase);
    }

        public: //! @name Current raster position and Rectangles (removed since 3.1)

    ST_LOCAL inline
    void glRasterPos2d(GLdouble x, GLdouble y) {
        ::glRasterPos2d(x, y);
    }

    ST_LOCAL inline
    void glRasterPos2f(GLfloat  x, GLfloat  y) {
        ::glRasterPos2f(x, y);
    }

    ST_LOCAL inline
    void glRasterPos2i(GLint    x, GLint    y) {
        ::glRasterPos2i(x, y);
    }

    ST_LOCAL inline
    void glRasterPos2s(GLshort  x, GLshort  y) {
        ::glRasterPos2s(x, y);
    }

    ST_LOCAL inline
    void glRasterPos3d(GLdouble x, GLdouble y, GLdouble z) {
        ::glRasterPos3d(x, y, z);
    }

    ST_LOCAL inline
    void glRasterPos3f(GLfloat  x, GLfloat  y, GLfloat  z) {
        ::glRasterPos3f(x, y, z);
    }

    ST_LOCAL inline
    void glRasterPos3i(GLint    x, GLint    y, GLint    z) {
        ::glRasterPos3i(x, y, z);
    }

    ST_LOCAL inline
    void glRasterPos3s(GLshort  x, GLshort  y, GLshort  z) {
        ::glRasterPos3s(x, y, z);
    }

    ST_LOCAL inline
    void glRasterPos4d(GLdouble x, GLdouble y, GLdouble z, GLdouble w) {
        ::glRasterPos4d(x, y, z, w);
    }

    ST_LOCAL inline
    void glRasterPos4f(GLfloat  x, GLfloat  y, GLfloat  z, GLfloat  w) {
        ::glRasterPos4f(x, y, z, w);
    }

    ST_LOCAL inline
    void glRasterPos4i(GLint    x, GLint    y, GLint    z, GLint    w) {
        ::glRasterPos4i(x, y, z, w);
    }

    ST_LOCAL inline
    void glRasterPos4s(GLshort  x, GLshort  y, GLshort  z, GLshort  w) {
        ::glRasterPos4s(x, y, z, w);
    }

    ST_LOCAL inline
    void glRasterPos2dv(const GLdouble* theVec) {
        ::glRasterPos2dv(theVec);
    }

    ST_LOCAL inline
    void glRasterPos2fv(const GLfloat*  theVec) {
        ::glRasterPos2fv(theVec);
    }

    ST_LOCAL inline
    void glRasterPos2iv(const GLint*    theVec) {
        ::glRasterPos2iv(theVec);
    }

    ST_LOCAL inline
    void glRasterPos2sv(const GLshort*  theVec) {
        ::glRasterPos2sv(theVec);
    }

    ST_LOCAL inline
    void glRasterPos3dv(const GLdouble* theVec) {
        ::glRasterPos3dv(theVec);
    }

    ST_LOCAL inline
    void glRasterPos3fv(const GLfloat*  theVec) {
        ::glRasterPos3fv(theVec);
    }

    ST_LOCAL inline
    void glRasterPos3iv(const GLint*    theVec) {
        ::glRasterPos3iv(theVec);
    }

    ST_LOCAL inline
    void glRasterPos3sv(const GLshort*  theVec) {
        ::glRasterPos3sv(theVec);
    }

    ST_LOCAL inline
    void glRasterPos4dv(const GLdouble* theVec) {
        ::glRasterPos4dv(theVec);
    }

    ST_LOCAL inline
    void glRasterPos4fv(const GLfloat*  theVec) {
        ::glRasterPos4fv(theVec);
    }

    ST_LOCAL inline
    void glRasterPos4iv(const GLint*    theVec) {
        ::glRasterPos4iv(theVec);
    }

    ST_LOCAL inline
    void glRasterPos4sv(const GLshort*  theVec) {
        ::glRasterPos4sv(theVec);
    }

    ST_LOCAL inline
    void glRectd(GLdouble x1, GLdouble y1, GLdouble x2, GLdouble y2) {
        ::glRectd(x1, y1, x2, y2);
    }

    ST_LOCAL inline
    void glRectf(GLfloat  x1, GLfloat  y1, GLfloat  x2, GLfloat  y2) {
        ::glRectf(x1, y1, x2, y2);
    }

    ST_LOCAL inline
    void glRecti(GLint    x1, GLint    y1, GLint    x2, GLint    y2) {
        ::glRecti(x1, y1, x2, y2);
    }

    ST_LOCAL inline
    void glRects(GLshort  x1, GLshort  y1, GLshort  x2, GLshort  y2) {
        ::glRects(x1, y1, x2, y2);
    }

    ST_LOCAL inline
    void glRectdv(const GLdouble* v1, const GLdouble* v2) {
        ::glRectdv(v1, v2);
    }

    ST_LOCAL inline
    void glRectfv(const GLfloat*  v1, const GLfloat*  v2) {
        ::glRectfv(v1, v2);
    }

    ST_LOCAL inline
    void glRectiv(const GLint*    v1, const GLint*    v2) {
        ::glRectiv(v1, v2);
    }

    ST_LOCAL inline
    void glRectsv(const GLshort*  v1, const GLshort*  v2) {
        ::glRectsv(v1, v2);
    }

        public: //! @name Texture mapping (removed since 3.1)

    ST_LOCAL inline
    void glTexGend(GLenum coord, GLenum pname, GLdouble param) {
        ::glTexGend(coord, pname, param);
    }

    ST_LOCAL inline
    void glTexGenf(GLenum coord, GLenum pname, GLfloat param) {
        ::glTexGenf(coord, pname, param);
    }

    ST_LOCAL inline
    void glTexGeni(GLenum coord, GLenum pname, GLint param) {
        ::glTexGeni(coord, pname, param);
    }

    ST_LOCAL inline
    void glTexGendv(GLenum coord, GLenum pname, const GLdouble* params) {
        ::glTexGendv(coord, pname, params);
    }

    ST_LOCAL inline
    void glTexGenfv(GLenum coord, GLenum pname, const GLfloat* params) {
        ::glTexGenfv(coord, pname, params);
    }

    ST_LOCAL inline
    void glTexGeniv(GLenum coord, GLenum pname, const GLint* params) {
        ::glTexGeniv(coord, pname, params);
    }

    ST_LOCAL inline
    void glGetTexGendv(GLenum coord, GLenum pname, GLdouble* params) {
        ::glGetTexGendv(coord, pname, params);
    }

    ST_LOCAL inline
    void glGetTexGenfv(GLenum coord, GLenum pname, GLfloat* params) {
        ::glGetTexGenfv(coord, pname, params);
    }

    ST_LOCAL inline
    void glGetTexGeniv(GLenum coord, GLenum pname, GLint* params) {
        ::glGetTexGeniv(coord, pname, params);
    }

        public: //! @name Rssident textures and priorities (removed since 3.1)

    ST_LOCAL inline
    void glPrioritizeTextures(GLsizei n, const GLuint* textures, const GLclampf* priorities) {
        ::glPrioritizeTextures(n, textures, priorities);
    }

    ST_LOCAL inline
    GLboolean glAreTexturesResident(GLsizei n, const GLuint* textures, GLboolean* residences) {
        return ::glAreTexturesResident(n, textures, residences);
    }

        public: //! @name Pixel copying (removed since 3.1)

    ST_LOCAL inline
    void glDrawPixels(GLsizei width, GLsizei height,
                      GLenum format, GLenum type,
                      const GLvoid* pixels) {
        ::glDrawPixels(width, height, format, type, pixels);
    }

    ST_LOCAL inline
    void glCopyPixels(GLint x, GLint y,
                      GLsizei width, GLsizei height,
                      GLenum type) {
        ::glCopyPixels(x, y, width, height, type);
    }

    ST_LOCAL inline
    void glBitmap(GLsizei width, GLsizei height,
                  GLfloat xorig, GLfloat yorig,
                  GLfloat xmove, GLfloat ymove,
                  const GLubyte* bitmap) {
        ::glBitmap(width, height, xorig, yorig, xmove, ymove, bitmap);
    }

    ST_LOCAL inline
    void glPixelZoom(GLfloat xfactor, GLfloat yfactor) {
        ::glPixelZoom(xfactor, yfactor);
    }

        public: //! @name Fog and all associated parameters (removed since 3.1)

    ST_LOCAL inline
    void glFogf(GLenum pname, GLfloat param) {
        ::glFogf(pname, param);
    }

    ST_LOCAL inline
    void glFogi(GLenum pname, GLint param) {
        ::glFogi(pname, param);
    }

    ST_LOCAL inline
    void glFogfv(GLenum pname, const GLfloat* params) {
        ::glFogfv(pname, params);
    }

    ST_LOCAL inline
    void glFogiv(GLenum pname, const GLint* params) {
        ::glFogiv(pname, params);
    }

        public: //! @name Evaluators (removed since 3.1)

    ST_LOCAL inline
    void glMap1d(GLenum target, GLdouble u1, GLdouble u2,
                 GLint stride,
                 GLint order, const GLdouble* points) {
        ::glMap1d(target, u1, u2, stride, order, points);
    }

    ST_LOCAL inline
    void glMap1f(GLenum target, GLfloat u1, GLfloat u2,
                 GLint stride,
                 GLint order, const GLfloat* points) {
         ::glMap1f(target, u1, u2, stride, order, points);
    }

    ST_LOCAL inline
    void glMap2d(GLenum target,
                 GLdouble u1, GLdouble u2, GLint ustride, GLint uorder,
                 GLdouble v1, GLdouble v2, GLint vstride, GLint vorder,
                 const GLdouble* points) {
        ::glMap2d(target, u1, u2, ustride, uorder, v1, v2, vstride, vorder, points);
    }

    ST_LOCAL inline
    void glMap2f(GLenum target,
                 GLfloat u1, GLfloat u2, GLint ustride, GLint uorder,
                 GLfloat v1, GLfloat v2, GLint vstride, GLint vorder,
                 const GLfloat* points) {
        ::glMap2f(target, u1, u2, ustride, uorder, v1, v2, vstride, vorder, points);
    }

    ST_LOCAL inline
    void glGetMapdv(GLenum target, GLenum query, GLdouble* v) {
        ::glGetMapdv(target, query, v);
    }

    ST_LOCAL inline
    void glGetMapfv(GLenum target, GLenum query, GLfloat* v) {
        ::glGetMapfv(target, query, v);
    }

    ST_LOCAL inline
    void glGetMapiv(GLenum target, GLenum query, GLint* v) {
        ::glGetMapiv(target, query, v);
    }

    ST_LOCAL inline
    void glEvalCoord1d(GLdouble u) {
        ::glEvalCoord1d(u);
    }

    ST_LOCAL inline
    void glEvalCoord1f(GLfloat u) {
        ::glEvalCoord1f(u);
    }

    ST_LOCAL inline
    void glEvalCoord1dv(const GLdouble* u) {
        ::glEvalCoord1dv(u);
    }

    ST_LOCAL inline
    void glEvalCoord1fv(const GLfloat* u) {
        ::glEvalCoord1fv(u);
    }

    ST_LOCAL inline
    void glEvalCoord2d(GLdouble u, GLdouble v) {
        ::glEvalCoord2d(u, v);
    }

    ST_LOCAL inline
    void glEvalCoord2f(GLfloat u, GLfloat v) {
        ::glEvalCoord2f(u, v);
    }

    ST_LOCAL inline
    void glEvalCoord2dv(const GLdouble* u) {
        ::glEvalCoord2dv(u);
    }

    ST_LOCAL inline
    void glEvalCoord2fv(const GLfloat* u) {
        ::glEvalCoord2fv(u);
    }

    ST_LOCAL inline
    void glMapGrid1d(GLint un, GLdouble u1, GLdouble u2) {
        ::glMapGrid1d(un, u1, u2);
    }

    ST_LOCAL inline
    void glMapGrid1f(GLint un, GLfloat u1, GLfloat u2) {
        ::glMapGrid1f(un, u1, u2);
    }

    ST_LOCAL inline
    void glMapGrid2d(GLint un, GLdouble u1, GLdouble u2,
                     GLint vn, GLdouble v1, GLdouble v2) {
        ::glMapGrid2d(un, u1, u2, vn, v1, v2);
    }

    ST_LOCAL inline
    void glMapGrid2f(GLint un, GLfloat u1, GLfloat u2,
                     GLint vn, GLfloat v1, GLfloat v2) {
        ::glMapGrid2f(un, u1, u2, vn, v1, v2);
    }

    ST_LOCAL inline
    void glEvalPoint1(GLint i) {
        ::glEvalPoint1(i);
    }

    ST_LOCAL inline
    void glEvalPoint2(GLint i, GLint j) {
        ::glEvalPoint2(i, j);
    }

    ST_LOCAL inline
    void glEvalMesh1(GLenum mode, GLint i1, GLint i2) {
        ::glEvalMesh1(mode, i1, i2);
    }

    ST_LOCAL inline
    void glEvalMesh2(GLenum mode, GLint i1, GLint i2, GLint j1, GLint j2) {
        ::glEvalMesh2(mode, i1, i2, j1, j2);
    }

        public: //! @name Selection and feedback modes (removed since 3.1)

    ST_LOCAL inline
    void glFeedbackBuffer(GLsizei theSize, GLenum theType, GLfloat* theBuffer) {
        ::glFeedbackBuffer(theSize, theType, theBuffer);
    }

    ST_LOCAL inline
    void glPassThrough(GLfloat token) {
        ::glPassThrough(token);
    }

    ST_LOCAL inline
    void glSelectBuffer(GLsizei theSize, GLuint* theBuffer) {
        ::glSelectBuffer(theSize, theBuffer);
    }

    ST_LOCAL inline
    void glInitNames() {
        ::glInitNames();
    }

    ST_LOCAL inline
    void glLoadName(GLuint theName) {
        ::glLoadName(theName);
    }

    ST_LOCAL inline
    void glPushName(GLuint theName) {
        ::glPushName(theName);
    }

    ST_LOCAL inline
    void glPopName() {
        ::glPopName();
    }

        public: //! @name Accumulation Buffer (removed since 3.1)

    ST_LOCAL inline
    void glClearAccum(GLfloat theRed, GLfloat theGreen, GLfloat theBlue, GLfloat theAlpha) {
        ::glClearAccum(theRed, theGreen, theBlue, theAlpha);
    }

    ST_LOCAL inline
    void glAccum(GLenum theOp, GLfloat theValue) {
        ::glAccum(theOp, theValue);
    }

        public: //! @name Edge flags and fixed-function vertex processing (removed since 3.1)

    ST_LOCAL inline
    void glEdgeFlag(GLboolean theFlag) {
        ::glEdgeFlag(theFlag);
    }

    ST_LOCAL inline
    void glEdgeFlagv(const GLboolean* theFlag) {
        ::glEdgeFlagv(theFlag);
    }

    ST_LOCAL inline
    void glVertexPointer(GLint theSize, GLenum theType, GLsizei theStride, const GLvoid* thePtr) {
        ::glVertexPointer(theSize, theType, theStride, thePtr);
    }

    ST_LOCAL inline
    void glNormalPointer(GLenum theType, GLsizei theStride, const GLvoid* thePtr) {

        ::glNormalPointer(theType, theStride, thePtr);
    }

    ST_LOCAL inline
    void glColorPointer(GLint theSize, GLenum theType, GLsizei theStride, const GLvoid* thePtr) {
        ::glColorPointer(theSize, theType, theStride, thePtr);
    }

    ST_LOCAL inline
    void glIndexPointer(GLenum theType, GLsizei theStride, const GLvoid* thePtr) {
        ::glIndexPointer(theType, theStride, thePtr);
    }

    ST_LOCAL inline
    void glTexCoordPointer(GLint theSize, GLenum theType, GLsizei theStride, const GLvoid* thePtr) {
        ::glTexCoordPointer(theSize, theType, theStride, thePtr);
    }

    ST_LOCAL inline
    void glEdgeFlagPointer(GLsizei theStride, const GLvoid* thePtr) {
        ::glEdgeFlagPointer(theStride, thePtr);
    }

    ST_LOCAL inline
    void glGetPointerv(GLenum pname, GLvoid** params) {
        ::glGetPointerv(pname, params);
    }

    ST_LOCAL inline
    void glInterleavedArrays(GLenum theFormat, GLsizei theStride, const GLvoid* thePointer) {
        ::glInterleavedArrays(theFormat, theStride, thePointer);
    }

    ST_LOCAL inline
    void glEnableClientState(GLenum theCap) {
        ::glEnableClientState(theCap);
    }

    ST_LOCAL inline
    void glDisableClientState(GLenum theCap) {
        ::glDisableClientState(theCap);
    }

#endif

};

#endif // __StGLCore11_h_
