/**
 * Copyright © 2010-2015 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#ifndef __StGLMatrix_h_
#define __StGLMatrix_h_

#include "StGLVec.h"

/**
 * Define the volume configuration.
 * Could be used for both - frustum (perspective projection matrix) and orthogonal projection matrix.
 * Actually defines the front rectangle and near/far Z-clipping planes.
 * The application should care how to interpret these values.
 */
struct StGLVolume {

    GLfloat xLeft;        //!< X left position
    GLfloat xRight;       //!< X right position
    GLfloat yBottom;      //!< Y bottom position
    GLfloat yTop;         //!< Y top position
    GLfloat zNear;        //!< near Z-clipping plane
    GLfloat zFar;         //!< far  Z-clipping plane
    GLfloat xTranslation; //!< additional X-translation

    /**
     * General constructor.
     */
    StGLVolume(GLfloat theLeft, GLfloat theRight,
               GLfloat theBottom, GLfloat theTop,
               GLfloat theZNear, GLfloat theZFar)
    : xLeft(theLeft),
      xRight(theRight),
      yBottom(theBottom),
      yTop(theTop),
      zNear(theZNear),
      zFar(theZFar),
      xTranslation(0.0f) {}

    /**
     * Empty constructor (creates an invalid frustum).
     */
    StGLVolume()
    : xLeft(0.0f),
      xRight(0.0f),
      yBottom(0.0f),
      yTop(0.0f),
      zNear(0.0f),
      zFar(0.0f),
      xTranslation(0.0f) {}

};

/**
 * The 4x4 matrix with generic operations defined.
 * The data stored in the native way for OpenGL (continuous values in column not the row)
 * and can be silently pushed as argument to OpenGL functions given the matrix argument.
 * Please use getValue(row, column) / changeValue(row, column) methods to avoid mistakes
 * when accessing to the matrix elements!
 */
class StGLMatrix {

        public:

    ST_CPPEXPORT virtual ~StGLMatrix();

    /**
     * Empty constructor, creates the identity matrix.
     */
    ST_CPPEXPORT StGLMatrix();

    /**
     * Copy constructor.
     */
    ST_CPPEXPORT StGLMatrix(const StGLMatrix& copyMat);

    /**
     * Construct rotation matrix from quaternion.
     */
    ST_CPPEXPORT explicit StGLMatrix(const StGLQuaternion& theQ);

    /**
     * Assignment operator.
     */
    ST_CPPEXPORT const StGLMatrix& operator=(const StGLMatrix& copyMat);

    GLfloat getValue(size_t theRow, size_t theCol) const {
        //ST_DEBUG_ASSERT(theRow < 4 && theCol < 4);
        return matrix[theCol * 4 + theRow];
    }

    GLfloat& changeValue(size_t theRow, size_t theCol) {
        //ST_DEBUG_ASSERT(theRow < 4 && theCol < 4);
        return matrix[theCol * 4 + theRow];
    }

    StGLVec4 getRow(size_t theRowId) const {
        //ST_DEBUG_ASSERT(theRowId < 4);
        return StGLVec4(getValue(theRowId, 0),
                        getValue(theRowId, 1),
                        getValue(theRowId, 2),
                        getValue(theRowId, 3));
    }

    void setRow(const StGLVec4& theRow, size_t theRowId) {
        //ST_DEBUG_ASSERT(theRowId < 4);
        changeValue(theRowId, 0) = theRow.x();
        changeValue(theRowId, 1) = theRow.y();
        changeValue(theRowId, 2) = theRow.z();
        changeValue(theRowId, 3) = theRow.w();
    }

    void setRow(const StGLVec3& theRow, size_t theRowId) {
        //ST_DEBUG_ASSERT(theRowId < 4);
        changeValue(theRowId, 0) = theRow.x();
        changeValue(theRowId, 1) = theRow.y();
        changeValue(theRowId, 2) = theRow.z();
    }

    void setRow(size_t theRowId, const StGLVec4& theRow) {
        setRow(theRow, theRowId);
    }

    StGLVec4 getColumn(size_t theColId) const {
        //ST_DEBUG_ASSERT(theColId < 4);
        return StGLVec4(getValue(theColId, 0),
                        getValue(theColId, 1),
                        getValue(theColId, 2),
                        getValue(theColId, 3));
    }

    void setColumn(const StGLVec4& theColumn, size_t theColId) {
        //ST_DEBUG_ASSERT(theColId < 4);
        changeValue(0, theColId) = theColumn.x();
        changeValue(1, theColId) = theColumn.y();
        changeValue(2, theColId) = theColumn.z();
        changeValue(3, theColId) = theColumn.w();
    }

    void setColumn(const StGLVec3& theColumn, size_t theColId) {
        //ST_DEBUG_ASSERT(theColId < 4);
        changeValue(0, theColId) = theColumn.x();
        changeValue(1, theColId) = theColumn.y();
        changeValue(2, theColId) = theColumn.z();
    }

    void setColumn(size_t theColId, const StGLVec4& theColumn) {
        setColumn(theColumn, theColId);
    }

    /**
     * Initialize the identity matrix.
     */
    ST_CPPEXPORT void initIdentity();

    /**
     * Initialize the perspective projection matrix.
     */
    ST_CPPEXPORT void initFrustum(const StGLVolume& theFrustum);

    /**
     * Initialize the orthographic projection matrix.
     */
    ST_CPPEXPORT void initOrtho(const StGLVolume& theVolume);

    /**
     * Automatically return GLfloat* to be silently used as argument got OpenGL functions.
     * Note that the matrix values are stored as OpenGL need (flat columns instead of flat rows).
     */
    operator const GLfloat*() const {
        return matrix;
    }

    /**
     * Automatically return GLfloat* to be silently used as argument got OpenGL functions.
     * Note that the matrix values are stored as OpenGL need (flat columns instead of flat rows).
     */
    operator GLfloat*() {
        return matrix;
    }

    bool isRowZero(size_t row) const {
        return (getValue(row, 0) == 0.0f) &&
               (getValue(row, 1) == 0.0f) &&
               (getValue(row, 2) == 0.0f) &&
               (getValue(row, 3) == 0.0f);
    }

    bool isColumnZero(size_t column) const {
        return (getValue(0, column) == 0.0f) &&
               (getValue(1, column) == 0.0f) &&
               (getValue(2, column) == 0.0f) &&
               (getValue(3, column) == 0.0f);
    }

    /**
     * Return the multiply theMat0 * theMat1 result.
     */
    ST_CPPEXPORT static StGLMatrix multiply(const StGLMatrix& theMat0, const StGLMatrix& theMat1);

    /**
     * Scale the current matrix.
     */
    ST_CPPEXPORT void scale(const GLfloat theSX, const GLfloat theSY, const GLfloat theSZ);

    /**
     * Translate the current matrix.
     */
    ST_CPPEXPORT void translate(const StGLVec3& theVec);

    /**
     * Rotate the current matrix around specified vector at specified angle.
     */
    ST_CPPEXPORT void rotate(const GLfloat theAngleDegrees,
                             const StGLDir3& theLine);

    /**
     * Multiply: theMat * theVec.
     */
    ST_CPPEXPORT StGLVec4 operator*(const StGLVec4& theVec) const;

    /**
     * Compute the inverted matrix.
     * @return true if reversion success.
     */
    ST_CPPEXPORT bool inverted(StGLMatrix& theInvOut) const;

    /**
     * Define a viewing transformation for model view matrix.
     * @param theEye    the eye position
     * @param theCenter the center position (look target)
     * @param theUp     the direction of the up vector
     */
    ST_CPPEXPORT void lookAt(const StGLVec3& theEye,
                             const StGLVec3& theCenter,
                             const StGLDir3& theUp);

    StString toString() const {
        return StString("{\n")
             + getRow(0).toString() + '\n'
             + getRow(1).toString() + '\n'
             + getRow(2).toString() + '\n'
             + getRow(3).toString() + "\n}";
    }

        private:

    GLfloat matrix[16];

};

/**
 * Compute: theVec^T * theMat.
 */
ST_CPPEXPORT StGLVec4 operator*(const StGLVec4& theVec, const StGLMatrix& theMat);

#endif //__StGLMatrix_h_
