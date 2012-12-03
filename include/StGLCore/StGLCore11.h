/**
 * Copyright © 2012 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
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
struct ST_LOCAL StGLCore11 : public StGLCore11Fwd {

        public: //! @name Begin/End primitive specification (removed since 3.1)

    void glBegin(GLenum theMode) {
        ::glBegin(theMode);
    }

    void glEnd() {
        ::glEnd();
    }

    void glVertex2d(GLdouble theX, GLdouble theY) {
        ::glVertex2d(theX, theY);
    }

    void glVertex2f(GLfloat  theX, GLfloat  theY) {
        ::glVertex2f(theX, theY);
    }

    void glVertex2i(GLint    theX, GLint    theY) {
        ::glVertex2i(theX, theY);
    }

    void glVertex2s(GLshort  theX, GLshort  theY) {
        ::glVertex2s(theX, theY);
    }

    void glVertex3d(GLdouble theX, GLdouble theY, GLdouble theZ) {
        ::glVertex3d(theX, theY, theZ);
    }

    void glVertex3f(GLfloat  theX, GLfloat  theY, GLfloat  theZ) {
        ::glVertex3f(theX, theY, theZ);
    }

    void glVertex3i(GLint    theX, GLint    theY, GLint    theZ) {
        ::glVertex3i(theX, theY, theZ);
    }

    void glVertex3s(GLshort  theX, GLshort  theY, GLshort  theZ) {
        ::glVertex3s(theX, theY, theZ);
    }

    void glVertex4d(GLdouble theX, GLdouble theY, GLdouble theZ, GLdouble theW) {
        ::glVertex4d(theX, theY, theZ, theW);
    }

    void glVertex4f(GLfloat  theX, GLfloat  theY, GLfloat  theZ, GLfloat  theW) {
        ::glVertex4f(theX, theY, theZ, theW);
    }

    void glVertex4i(GLint    theX, GLint    theY, GLint    theZ, GLint    theW) {
        ::glVertex4i(theX, theY, theZ, theW);
    }

    void glVertex4s(GLshort  theX, GLshort  theY, GLshort  theZ, GLshort  theW) {
        ::glVertex4s(theX, theY, theZ, theW);
    }

    void glVertex2dv(const GLdouble* theVec2) {
        ::glVertex2dv(theVec2);
    }

    void glVertex2fv(const GLfloat*  theVec2) {
        ::glVertex2fv(theVec2);
    }

    void glVertex2iv(const GLint*    theVec2) {
        ::glVertex2iv(theVec2);
    }

    void glVertex2sv(const GLshort*  theVec2) {
        ::glVertex2sv(theVec2);
    }

    void glVertex3dv(const GLdouble* theVec3) {
        ::glVertex3dv(theVec3);
    }

    void glVertex3fv(const GLfloat*  theVec3) {
        ::glVertex3fv(theVec3);
    }

    void glVertex3iv(const GLint*    theVec3) {
        ::glVertex3iv(theVec3);
    }

    void glVertex3sv(const GLshort*  theVec3) {
        ::glVertex3sv(theVec3);
    }

    void glVertex4dv(const GLdouble* theVec4) {
        ::glVertex4dv(theVec4);
    }

    void glVertex4fv(const GLfloat*  theVec4) {
        ::glVertex4fv(theVec4);
    }

    void glVertex4iv(const GLint*    theVec4) {
        ::glVertex4iv(theVec4);
    }

    void glVertex4sv(const GLshort*  theVec4) {
        ::glVertex4sv(theVec4);
    }

    void glNormal3b(GLbyte   nx, GLbyte   ny, GLbyte   nz) {
        ::glNormal3b(nx, ny, nz);
    }

    void glNormal3d(GLdouble nx, GLdouble ny, GLdouble nz) {
        ::glNormal3d(nx, ny, nz);
    }

    void glNormal3f(GLfloat  nx, GLfloat  ny, GLfloat  nz) {
        ::glNormal3f(nx, ny, nz);
    }

    void glNormal3i(GLint    nx, GLint    ny, GLint    nz) {
        ::glNormal3i(nx, ny, nz);
    }

    void glNormal3s(GLshort nx, GLshort ny, GLshort nz) {
        ::glNormal3s(nx, ny, nz);
    }

    void glNormal3bv(const GLbyte*   theVec) {
        ::glNormal3bv(theVec);
    }

    void glNormal3dv(const GLdouble* theVec) {
        ::glNormal3dv(theVec);
    }

    void glNormal3fv(const GLfloat*  theVec) {
        ::glNormal3fv(theVec);
    }

    void glNormal3iv(const GLint*    theVec) {
        ::glNormal3iv(theVec);
    }

    void glNormal3sv(const GLshort*  theVec) {
        ::glNormal3sv(theVec);
    }

    void glIndexd(GLdouble c) {
        ::glIndexd(c);
    }

    void glIndexf(GLfloat c) {
        ::glIndexf(c);
    }

    void glIndexi(GLint c) {
        ::glIndexi(c);
    }

    void glIndexs(GLshort c) {
        ::glIndexs(c);
    }

    void glIndexub(GLubyte c) {
        ::glIndexub(c);
    }

    void glIndexdv(const GLdouble* c) {
        ::glIndexdv(c);
    }

    void glIndexfv(const GLfloat* c) {
        ::glIndexfv(c);
    }

    void glIndexiv(const GLint* c) {
        ::glIndexiv(c);
    }

    void glIndexsv(const GLshort* c) {
        ::glIndexsv(c);
    }

    void glIndexubv(const GLubyte* c) {
        ::glIndexubv(c);
    }

    void glColor3b(GLbyte theRed, GLbyte theGreen, GLbyte theBlue) {
        ::glColor3b(theRed, theGreen, theBlue);
    }

    void glColor3d(GLdouble theRed, GLdouble theGreen, GLdouble theBlue) {
        ::glColor3d(theRed, theGreen, theBlue);
    }

    void glColor3f(GLfloat theRed, GLfloat theGreen, GLfloat theBlue) {
        ::glColor3f(theRed, theGreen, theBlue);
    }

    void glColor3i(GLint theRed, GLint theGreen, GLint theBlue) {
        ::glColor3i(theRed, theGreen, theBlue);
    }

    void glColor3s(GLshort theRed, GLshort theGreen, GLshort theBlue) {
        ::glColor3s(theRed, theGreen, theBlue);
    }

    void glColor3ub(GLubyte theRed, GLubyte theGreen, GLubyte theBlue) {
        ::glColor3ub(theRed, theGreen, theBlue);
    }

    void glColor3ui(GLuint theRed, GLuint theGreen, GLuint theBlue) {
        ::glColor3ui(theRed, theGreen, theBlue);
    }

    void glColor3us(GLushort theRed, GLushort theGreen, GLushort theBlue) {
        ::glColor3us(theRed, theGreen, theBlue);
    }

    void glColor4b(GLbyte theRed, GLbyte theGreen, GLbyte theBlue, GLbyte theAlpha) {
        ::glColor4b(theRed, theGreen, theBlue, theAlpha);
    }

    void glColor4d(GLdouble theRed, GLdouble theGreen, GLdouble theBlue, GLdouble theAlpha) {
        ::glColor4d(theRed, theGreen, theBlue, theAlpha);
    }

    void glColor4f(GLfloat theRed, GLfloat theGreen, GLfloat theBlue, GLfloat theAlpha) {
        ::glColor4f(theRed, theGreen, theBlue, theAlpha);
    }

    void glColor4i(GLint theRed, GLint theGreen, GLint theBlue, GLint theAlpha) {
        ::glColor4i(theRed, theGreen, theBlue, theAlpha);
    }

    void glColor4s(GLshort theRed, GLshort theGreen, GLshort theBlue, GLshort theAlpha) {
        ::glColor4s(theRed, theGreen, theBlue, theAlpha);
    }

    void glColor4ub(GLubyte theRed, GLubyte theGreen, GLubyte theBlue, GLubyte theAlpha) {
        ::glColor4ub(theRed, theGreen, theBlue, theAlpha);
    }

    void glColor4ui(GLuint theRed, GLuint theGreen, GLuint theBlue, GLuint theAlpha) {
        ::glColor4ui(theRed, theGreen, theBlue, theAlpha);
    }

    void glColor4us(GLushort theRed, GLushort theGreen, GLushort theBlue, GLushort theAlpha) {
        ::glColor4us(theRed, theGreen, theBlue, theAlpha);
    }

    void glColor3bv(const GLbyte*    theVec) {
        ::glColor3bv(theVec);
    }

    void glColor3dv(const GLdouble*  theVec) {
        ::glColor3dv(theVec);
    }

    void glColor3fv(const GLfloat*   theVec) {
        ::glColor3fv(theVec);
    }

    void glColor3iv(const GLint*     theVec) {
        ::glColor3iv(theVec);
    }

    void glColor3sv(const GLshort*   theVec) {
        ::glColor3sv(theVec);
    }

    void glColor3ubv(const GLubyte*  theVec) {
        ::glColor3ubv(theVec);
    }

    void glColor3uiv(const GLuint*   theVec) {
        ::glColor3uiv(theVec);
    }

    void glColor3usv(const GLushort* theVec) {
        ::glColor3usv(theVec);
    }

    void glColor4bv(const GLbyte*    theVec) {
        ::glColor4bv(theVec);
    }

    void glColor4dv(const GLdouble*  theVec) {
        ::glColor4dv(theVec);
    }

    void glColor4fv(const GLfloat*   theVec) {
        ::glColor4fv(theVec);
    }

    void glColor4iv(const GLint*     theVec) {
        ::glColor4iv(theVec);
    }

    void glColor4sv(const GLshort*   theVec) {
        ::glColor4sv(theVec);
    }

    void glColor4ubv(const GLubyte*  theVec) {
        ::glColor4ubv(theVec);
    }

    void glColor4uiv(const GLuint*   theVec) {
        ::glColor4uiv(theVec);
    }

    void glColor4usv(const GLushort* theVec) {
        ::glColor4usv(theVec);
    }

    void glTexCoord1d(GLdouble s) {
        ::glTexCoord1d(s);
    }

    void glTexCoord1f(GLfloat s) {
        ::glTexCoord1f(s);
    }

    void glTexCoord1i(GLint s) {
        ::glTexCoord1i(s);
    }

    void glTexCoord1s(GLshort s) {
        ::glTexCoord1s(s);
    }

    void glTexCoord2d(GLdouble s, GLdouble t) {
        ::glTexCoord2d(s, t);
    }

    void glTexCoord2f(GLfloat s, GLfloat t) {
        ::glTexCoord2f(s, t);
    }

    void glTexCoord2i(GLint s, GLint t) {
        ::glTexCoord2i(s, t);
    }

    void glTexCoord2s(GLshort s, GLshort t) {
        ::glTexCoord2s(s, t);
    }

    void glTexCoord3d(GLdouble s, GLdouble t, GLdouble r) {
        ::glTexCoord3d(s, t, r);
    }

    void glTexCoord3f(GLfloat s, GLfloat t, GLfloat r) {
        ::glTexCoord3f(s, t, r);
    }

    void glTexCoord3i(GLint s, GLint t, GLint r) {
        ::glTexCoord3i(s, t, r);
    }

    void glTexCoord3s(GLshort s, GLshort t, GLshort r) {
        ::glTexCoord3s(s, t, r);
    }

    void glTexCoord4d(GLdouble s, GLdouble t, GLdouble r, GLdouble q) {
        ::glTexCoord4d(s, t, r, q);
    }

    void glTexCoord4f(GLfloat s, GLfloat t, GLfloat r, GLfloat q) {
        ::glTexCoord4f(s, t, r, q);
    }

    void glTexCoord4i(GLint s, GLint t, GLint r, GLint q) {
        ::glTexCoord4i(s, t, r, q);
    }

    void glTexCoord4s(GLshort s, GLshort t, GLshort r, GLshort q) {
        ::glTexCoord4s(s, t, r, q);
    }

    void glTexCoord1dv(const GLdouble* theVec1) {
        ::glTexCoord1dv(theVec1);
    }

    void glTexCoord1fv(const GLfloat*  theVec1) {
        ::glTexCoord1fv(theVec1);
    }

    void glTexCoord1iv(const GLint*    theVec1) {
        ::glTexCoord1iv(theVec1);
    }

    void glTexCoord1sv(const GLshort*  theVec1) {
        ::glTexCoord1sv(theVec1);
    }

    void glTexCoord2dv(const GLdouble* theVec2) {
        ::glTexCoord2dv(theVec2);
    }

    void glTexCoord2fv(const GLfloat*  theVec2) {
        ::glTexCoord2fv(theVec2);
    }

    void glTexCoord2iv(const GLint*    theVec2) {
        ::glTexCoord2iv(theVec2);
    }

    void glTexCoord2sv(const GLshort*  theVec) {
        ::glTexCoord2sv(theVec);
    }

    void glTexCoord3dv(const GLdouble* theVec3) {
        ::glTexCoord3dv(theVec3);
    }

    void glTexCoord3fv(const GLfloat*  theVec3) {
        ::glTexCoord3fv(theVec3);
    }

    void glTexCoord3iv(const GLint*    theVec3) {
        ::glTexCoord3iv(theVec3);
    }

    void glTexCoord3sv(const GLshort*  theVec3) {
        ::glTexCoord3sv(theVec3);
    }

    void glTexCoord4dv(const GLdouble* theVec4) {
        ::glTexCoord4dv(theVec4);
    }

    void glTexCoord4fv(const GLfloat*  theVec4) {
        ::glTexCoord4fv(theVec4);
    }

    void glTexCoord4iv(const GLint*    theVec4) {
        ::glTexCoord4iv(theVec4);
    }

    void glTexCoord4sv(const GLshort*  theVec4) {
        ::glTexCoord4sv(theVec4);
    }

        public: //! @name Matrix operations (removed since 3.1)

    void glMatrixMode(GLenum theMode) {
        ::glMatrixMode(theMode);
    }

    void glOrtho(GLdouble theLeft,    GLdouble theRight,
                 GLdouble theBottom,  GLdouble theTop,
                 GLdouble theNearVal, GLdouble theFarVal) {
        ::glOrtho(theLeft, theRight, theBottom, theTop, theNearVal, theFarVal);
    }

    void glFrustum(GLdouble theLeft,    GLdouble theRight,
                   GLdouble theBottom,  GLdouble theTop,
                   GLdouble theNearVal, GLdouble theFarVal) {
        ::glFrustum(theLeft, theRight, theBottom, theTop, theNearVal, theFarVal);
    }

    void glPushMatrix() {
        ::glPushMatrix();
    }

    void glPopMatrix() {
        ::glPopMatrix();
    }

    void glLoadIdentity() {
        ::glLoadIdentity();
    }

    void glLoadMatrixd(const GLdouble* theMatrix) {
        ::glLoadMatrixd(theMatrix);
    }

    void glLoadMatrixf(const GLfloat*  theMatrix) {
        ::glLoadMatrixf(theMatrix);
    }

    void glMultMatrixd(const GLdouble* theMatrix) {
        ::glMultMatrixd(theMatrix);
    }

    void glMultMatrixf(const GLfloat*  theMatrix) {
        ::glMultMatrixf(theMatrix);
    }

    void glRotated(GLdouble theAngleDegrees,
                   GLdouble theX, GLdouble theY, GLdouble theZ) {
        ::glRotated(theAngleDegrees, theX, theY, theZ);
    }

    void glRotatef(GLfloat theAngleDegrees,
                   GLfloat theX, GLfloat theY, GLfloat theZ) {
        ::glRotatef(theAngleDegrees, theX, theY, theZ);
    }

    void glScaled(GLdouble theX, GLdouble theY, GLdouble theZ) {
        ::glScaled(theX, theY, theZ);
    }

    void glScalef(GLfloat theX, GLfloat theY, GLfloat theZ) {
        ::glScalef(theX, theY, theZ);
    }

    void glTranslated(GLdouble theX, GLdouble theY, GLdouble theZ) {
        ::glTranslated(theX, theY, theZ);
    }

    void glTranslatef(GLfloat theX, GLfloat theY, GLfloat theZ) {
        ::glTranslatef(theX, theY, theZ);
    }

        public: //! @name Line and Polygon stripple (removed since 3.1)

    //void glLineWidth(GLfloat theWidth) { ::glLineWidth(theWidth); }

    void glLineStipple(GLint theFactor, GLushort thePattern) {
        ::glLineStipple(theFactor, thePattern);
    }

    void glPolygonStipple(const GLubyte* theMask) {
        ::glPolygonStipple(theMask);
    }

    void glGetPolygonStipple(GLubyte* theMask) {
        ::glGetPolygonStipple(theMask);
    }

        public: //! @name Attribute stacks (removed since 3.1)

    void glPushAttrib(GLbitfield theMask) {
        ::glPushAttrib(theMask);
    }

    void glPopAttrib() {
        ::glPopAttrib();
    }

    void glPushClientAttrib(GLbitfield theMask) {
        ::glPushClientAttrib(theMask);
    }

    void glPopClientAttrib() {
        ::glPopClientAttrib();
    }

        public: //! @name Fixed pipeline lighting (removed since 3.1)

    void glShadeModel(GLenum theMode) {
        ::glShadeModel(theMode);
    }

    void glLightf(GLenum theLight, GLenum pname, GLfloat param) {
        ::glLightf(theLight, pname, param);
    }

    void glLighti(GLenum theLight, GLenum pname, GLint param) {
        ::glLighti(theLight, pname, param);
    }

    void glLightfv(GLenum theLight, GLenum pname, const GLfloat* params) {
        ::glLightfv(theLight, pname, params);
    }

    void glLightiv(GLenum theLight, GLenum pname, const GLint* params) {
        ::glLightiv(theLight, pname, params);
    }

    void glGetLightfv(GLenum theLight, GLenum pname, GLfloat *params) {
        ::glGetLightfv(theLight, pname, params);
    }

    void glGetLightiv(GLenum theLight, GLenum pname, GLint *params) {
        ::glGetLightiv(theLight, pname, params);
    }

    void glLightModelf(GLenum pname, GLfloat param) {
        ::glLightModelf(pname, param);
    }

    void glLightModeli(GLenum pname, GLint param) {
        ::glLightModeli(pname, param);
    }

    void glLightModelfv(GLenum pname, const GLfloat* params) {
        ::glLightModelfv(pname, params);
    }

    void glLightModeliv(GLenum pname, const GLint* params) {
        ::glLightModeliv(pname, params);
    }

    void glMaterialf(GLenum face, GLenum pname, GLfloat param) {
        ::glMaterialf(face, pname, param);
    }

    void glMateriali(GLenum face, GLenum pname, GLint param) {
        ::glMateriali(face, pname, param);
    }

    void glMaterialfv(GLenum face, GLenum pname, const GLfloat* params) {
        ::glMaterialfv(face, pname, params);
    }

    void glMaterialiv(GLenum face, GLenum pname, const GLint* params) {
        ::glMaterialiv(face, pname, params);
    }

    void glGetMaterialfv(GLenum face, GLenum pname, GLfloat* params) {
        ::glGetMaterialfv(face, pname, params);
    }

    void glGetMaterialiv(GLenum face, GLenum pname, GLint* params) {
        ::glGetMaterialiv(face, pname, params);
    }

    void glColorMaterial(GLenum face, GLenum mode) {
        ::glColorMaterial(face, mode);
    }

        public: //! @name clipping plane (removed since 3.1)

    void glClipPlane(GLenum thePlane, const GLdouble* theEquation) {
        ::glClipPlane(thePlane, theEquation);
    }

    void glGetClipPlane(GLenum thePlane, GLdouble* theEquation) {
        ::glGetClipPlane(thePlane, theEquation);
    }

        public: //! @name Display lists (removed since 3.1)

    GLboolean glIsList(GLuint theList) {
        return ::glIsList(theList);
    }

    void glDeleteLists(GLuint theList, GLsizei theRange) {
        ::glDeleteLists(theList, theRange);
    }

    GLuint glGenLists(GLsizei theRange) {
        return ::glGenLists(theRange);
    }

    void glNewList(GLuint theList, GLenum theMode) {
        ::glNewList(theList, theMode);
    }

    void glEndList() {
        ::glEndList();
    }

    void glCallList(GLuint theList) {
        ::glCallList(theList);
    }

    void glCallLists(GLsizei theNb, GLenum theType, const GLvoid* theLists) {
        ::glCallLists(theNb, theType, theLists);
    }

    void glListBase(GLuint theBase) {
        ::glListBase(theBase);
    }

        public: //! @name Current raster position and Rectangles (removed since 3.1)

    void glRasterPos2d(GLdouble x, GLdouble y) {
        ::glRasterPos2d(x, y);
    }

    void glRasterPos2f(GLfloat  x, GLfloat  y) {
        ::glRasterPos2f(x, y);
    }

    void glRasterPos2i(GLint    x, GLint    y) {
        ::glRasterPos2i(x, y);
    }

    void glRasterPos2s(GLshort  x, GLshort  y) {
        ::glRasterPos2s(x, y);
    }

    void glRasterPos3d(GLdouble x, GLdouble y, GLdouble z) {
        ::glRasterPos3d(x, y, z);
    }

    void glRasterPos3f(GLfloat  x, GLfloat  y, GLfloat  z) {
        ::glRasterPos3f(x, y, z);
    }

    void glRasterPos3i(GLint    x, GLint    y, GLint    z) {
        ::glRasterPos3i(x, y, z);
    }

    void glRasterPos3s(GLshort  x, GLshort  y, GLshort  z) {
        ::glRasterPos3s(x, y, z);
    }

    void glRasterPos4d(GLdouble x, GLdouble y, GLdouble z, GLdouble w) {
        ::glRasterPos4d(x, y, z, w);
    }

    void glRasterPos4f(GLfloat  x, GLfloat  y, GLfloat  z, GLfloat  w) {
        ::glRasterPos4f(x, y, z, w);
    }

    void glRasterPos4i(GLint    x, GLint    y, GLint    z, GLint    w) {
        ::glRasterPos4i(x, y, z, w);
    }

    void glRasterPos4s(GLshort  x, GLshort  y, GLshort  z, GLshort  w) {
        ::glRasterPos4s(x, y, z, w);
    }

    void glRasterPos2dv(const GLdouble* theVec) {
        ::glRasterPos2dv(theVec);
    }

    void glRasterPos2fv(const GLfloat*  theVec) {
        ::glRasterPos2fv(theVec);
    }

    void glRasterPos2iv(const GLint*    theVec) {
        ::glRasterPos2iv(theVec);
    }

    void glRasterPos2sv(const GLshort*  theVec) {
        ::glRasterPos2sv(theVec);
    }

    void glRasterPos3dv(const GLdouble* theVec) {
        ::glRasterPos3dv(theVec);
    }

    void glRasterPos3fv(const GLfloat*  theVec) {
        ::glRasterPos3fv(theVec);
    }

    void glRasterPos3iv(const GLint*    theVec) {
        ::glRasterPos3iv(theVec);
    }

    void glRasterPos3sv(const GLshort*  theVec) {
        ::glRasterPos3sv(theVec);
    }

    void glRasterPos4dv(const GLdouble* theVec) {
        ::glRasterPos4dv(theVec);
    }

    void glRasterPos4fv(const GLfloat*  theVec) {
        ::glRasterPos4fv(theVec);
    }

    void glRasterPos4iv(const GLint*    theVec) {
        ::glRasterPos4iv(theVec);
    }

    void glRasterPos4sv(const GLshort*  theVec) {
        ::glRasterPos4sv(theVec);
    }

    void glRectd(GLdouble x1, GLdouble y1, GLdouble x2, GLdouble y2) {
        ::glRectd(x1, y1, x2, y2);
    }

    void glRectf(GLfloat  x1, GLfloat  y1, GLfloat  x2, GLfloat  y2) {
        ::glRectf(x1, y1, x2, y2);
    }

    void glRecti(GLint    x1, GLint    y1, GLint    x2, GLint    y2) {
        ::glRecti(x1, y1, x2, y2);
    }

    void glRects(GLshort  x1, GLshort  y1, GLshort  x2, GLshort  y2) {
        ::glRects(x1, y1, x2, y2);
    }

    void glRectdv(const GLdouble* v1, const GLdouble* v2) {
        ::glRectdv(v1, v2);
    }

    void glRectfv(const GLfloat*  v1, const GLfloat*  v2) {
        ::glRectfv(v1, v2);
    }

    void glRectiv(const GLint*    v1, const GLint*    v2) {
        ::glRectiv(v1, v2);
    }

    void glRectsv(const GLshort*  v1, const GLshort*  v2) {
        ::glRectsv(v1, v2);
    }

        public: //! @name Texture mapping (removed since 3.1)

    void glTexGend(GLenum coord, GLenum pname, GLdouble param) {
        ::glTexGend(coord, pname, param);
    }

    void glTexGenf(GLenum coord, GLenum pname, GLfloat param) {
        ::glTexGenf(coord, pname, param);
    }

    void glTexGeni(GLenum coord, GLenum pname, GLint param) {
        ::glTexGeni(coord, pname, param);
    }

    void glTexGendv(GLenum coord, GLenum pname, const GLdouble* params) {
        ::glTexGendv(coord, pname, params);
    }

    void glTexGenfv(GLenum coord, GLenum pname, const GLfloat* params) {
        ::glTexGenfv(coord, pname, params);
    }

    void glTexGeniv(GLenum coord, GLenum pname, const GLint* params) {
        ::glTexGeniv(coord, pname, params);
    }

    void glGetTexGendv(GLenum coord, GLenum pname, GLdouble* params) {
        ::glGetTexGendv(coord, pname, params);
    }

    void glGetTexGenfv(GLenum coord, GLenum pname, GLfloat* params) {
        ::glGetTexGenfv(coord, pname, params);
    }

    void glGetTexGeniv(GLenum coord, GLenum pname, GLint* params) {
        ::glGetTexGeniv(coord, pname, params);
    }

        public: //! @name Rssident textures and priorities (removed since 3.1)

    void glPrioritizeTextures(GLsizei n, const GLuint* textures, const GLclampf* priorities) {
        ::glPrioritizeTextures(n, textures, priorities);
    }

    GLboolean glAreTexturesResident(GLsizei n, const GLuint* textures, GLboolean* residences) {
        return ::glAreTexturesResident(n, textures, residences);
    }

        public: //! @name Pixel copying (removed since 3.1)

    void glDrawPixels(GLsizei width, GLsizei height,
                      GLenum format, GLenum type,
                      const GLvoid* pixels) {
        ::glDrawPixels(width, height, format, type, pixels);
    }

    void glCopyPixels(GLint x, GLint y,
                      GLsizei width, GLsizei height,
                      GLenum type) {
        ::glCopyPixels(x, y, width, height, type);
    }

    void glBitmap(GLsizei width, GLsizei height,
                  GLfloat xorig, GLfloat yorig,
                  GLfloat xmove, GLfloat ymove,
                  const GLubyte* bitmap) {
        ::glBitmap(width, height, xorig, yorig, xmove, ymove, bitmap);
    }

    void glPixelZoom(GLfloat xfactor, GLfloat yfactor) {
        ::glPixelZoom(xfactor, yfactor);
    }

        public: //! @name Fog and all associated parameters (removed since 3.1)

    void glFogf(GLenum pname, GLfloat param) {
        ::glFogf(pname, param);
    }

    void glFogi(GLenum pname, GLint param) {
        ::glFogi(pname, param);
    }

    void glFogfv(GLenum pname, const GLfloat* params) {
        ::glFogfv(pname, params);
    }

    void glFogiv(GLenum pname, const GLint* params) {
        ::glFogiv(pname, params);
    }

        public: //! @name Evaluators (removed since 3.1)

    void glMap1d(GLenum target, GLdouble u1, GLdouble u2,
                 GLint stride,
                 GLint order, const GLdouble* points) {
        ::glMap1d(target, u1, u2, stride, order, points);
    }

    void glMap1f(GLenum target, GLfloat u1, GLfloat u2,
                 GLint stride,
                 GLint order, const GLfloat* points) {
         ::glMap1f(target, u1, u2, stride, order, points);
    }

    void glMap2d(GLenum target,
                 GLdouble u1, GLdouble u2, GLint ustride, GLint uorder,
                 GLdouble v1, GLdouble v2, GLint vstride, GLint vorder,
                 const GLdouble* points) {
        ::glMap2d(target, u1, u2, ustride, uorder, v1, v2, vstride, vorder, points);
    }

    void glMap2f(GLenum target,
                 GLfloat u1, GLfloat u2, GLint ustride, GLint uorder,
                 GLfloat v1, GLfloat v2, GLint vstride, GLint vorder,
                 const GLfloat* points) {
        ::glMap2f(target, u1, u2, ustride, uorder, v1, v2, vstride, vorder, points);
    }

    void glGetMapdv(GLenum target, GLenum query, GLdouble* v) {
        ::glGetMapdv(target, query, v);
    }

    void glGetMapfv(GLenum target, GLenum query, GLfloat* v) {
        ::glGetMapfv(target, query, v);
    }

    void glGetMapiv(GLenum target, GLenum query, GLint* v) {
        ::glGetMapiv(target, query, v);
    }

    void glEvalCoord1d(GLdouble u) {
        ::glEvalCoord1d(u);
    }

    void glEvalCoord1f(GLfloat u) {
        ::glEvalCoord1f(u);
    }

    void glEvalCoord1dv(const GLdouble* u) {
        ::glEvalCoord1dv(u);
    }

    void glEvalCoord1fv(const GLfloat* u) {
        ::glEvalCoord1fv(u);
    }

    void glEvalCoord2d(GLdouble u, GLdouble v) {
        ::glEvalCoord2d(u, v);
    }

    void glEvalCoord2f(GLfloat u, GLfloat v) {
        ::glEvalCoord2f(u, v);
    }

    void glEvalCoord2dv(const GLdouble* u) {
        ::glEvalCoord2dv(u);
    }
    void glEvalCoord2fv(const GLfloat* u) {
        ::glEvalCoord2fv(u);
    }

    void glMapGrid1d(GLint un, GLdouble u1, GLdouble u2) {
        ::glMapGrid1d(un, u1, u2);
    }

    void glMapGrid1f(GLint un, GLfloat u1, GLfloat u2) {
        ::glMapGrid1f(un, u1, u2);
    }

    void glMapGrid2d(GLint un, GLdouble u1, GLdouble u2,
                     GLint vn, GLdouble v1, GLdouble v2) {
        ::glMapGrid2d(un, u1, u2, vn, v1, v2);
    }

    void glMapGrid2f(GLint un, GLfloat u1, GLfloat u2,
                     GLint vn, GLfloat v1, GLfloat v2) {
        ::glMapGrid2f(un, u1, u2, vn, v1, v2);
    }

    void glEvalPoint1(GLint i) {
        ::glEvalPoint1(i);
    }

    void glEvalPoint2(GLint i, GLint j) {
        ::glEvalPoint2(i, j);
    }

    void glEvalMesh1(GLenum mode, GLint i1, GLint i2) {
        ::glEvalMesh1(mode, i1, i2);
    }

    void glEvalMesh2(GLenum mode, GLint i1, GLint i2, GLint j1, GLint j2) {
        ::glEvalMesh2(mode, i1, i2, j1, j2);
    }

        public: //! @name Selection and feedback modes (removed since 3.1)

    void glFeedbackBuffer(GLsizei theSize, GLenum theType, GLfloat* theBuffer) {
        ::glFeedbackBuffer(theSize, theType, theBuffer);
    }

    void glPassThrough(GLfloat token) {
        ::glPassThrough(token);
    }

    void glSelectBuffer(GLsizei theSize, GLuint* theBuffer) {
        ::glSelectBuffer(theSize, theBuffer);
    }

    void glInitNames() {
        ::glInitNames();
    }

    void glLoadName(GLuint theName) {
        ::glLoadName(theName);
    }

    void glPushName(GLuint theName) {
        ::glPushName(theName);
    }

    void glPopName() {
        ::glPopName();
    }

        public: //! @name Accumulation Buffer (removed since 3.1)

    void glClearAccum(GLfloat theRed, GLfloat theGreen, GLfloat theBlue, GLfloat theAlpha) {
        ::glClearAccum(theRed, theGreen, theBlue, theAlpha);
    }

    void glAccum(GLenum theOp, GLfloat theValue) {
        ::glAccum(theOp, theValue);
    }

        public: //! @name Edge flags and fixed-function vertex processing (removed since 3.1)

    void glEdgeFlag(GLboolean theFlag) {
        ::glEdgeFlag(theFlag);
    }

    void glEdgeFlagv(const GLboolean* theFlag) {
        ::glEdgeFlagv(theFlag);
    }

    void glVertexPointer(GLint theSize, GLenum theType, GLsizei theStride, const GLvoid* thePtr) {
        ::glVertexPointer(theSize, theType, theStride, thePtr);
    }

    void glNormalPointer(GLenum theType, GLsizei theStride, const GLvoid* thePtr) {

        ::glNormalPointer(theType, theStride, thePtr);
    }

    void glColorPointer(GLint theSize, GLenum theType, GLsizei theStride, const GLvoid* thePtr) {
        ::glColorPointer(theSize, theType, theStride, thePtr);
    }

    void glIndexPointer(GLenum theType, GLsizei theStride, const GLvoid* thePtr) {
        ::glIndexPointer(theType, theStride, thePtr);
    }

    void glTexCoordPointer(GLint theSize, GLenum theType, GLsizei theStride, const GLvoid* thePtr) {
        ::glTexCoordPointer(theSize, theType, theStride, thePtr);
    }

    void glEdgeFlagPointer(GLsizei theStride, const GLvoid* thePtr) {
        ::glEdgeFlagPointer(theStride, thePtr);
    }

    void glGetPointerv(GLenum pname, GLvoid** params) {
        ::glGetPointerv(pname, params);
    }

    void glInterleavedArrays(GLenum theFormat, GLsizei theStride, const GLvoid* thePointer) {
        ::glInterleavedArrays(theFormat, theStride, thePointer);
    }

    void glEnableClientState(GLenum theCap) {
        ::glEnableClientState(theCap);
    }

    void glDisableClientState(GLenum theCap) {
        ::glDisableClientState(theCap);
    }

};

#endif // __StGLCore11_h_
