/**
 * Copyright © 2010-2015 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#include <StGL/StGLMatrix.h>

#define __glPif 3.1415926535897932384626433832795f

namespace {
    static const GLfloat ST_IDENT_MAT[16] = {
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    };
}

StGLMatrix::StGLMatrix() {
    initIdentity();
}

StGLMatrix::StGLMatrix(const StGLMatrix& copyMat) {
    stMemCpy(matrix, copyMat.matrix, sizeof(matrix));
}

const StGLMatrix& StGLMatrix::operator=(const StGLMatrix& copyMat) {
    stMemCpy(matrix, copyMat.matrix, sizeof(matrix));
    return *this;
}

StGLMatrix::StGLMatrix(const StGLQuaternion& theQ) {
    initIdentity();

    float wx, wy, wz, xx, yy, yz, xy, xz, zz, x2, y2, z2;
    float s = 2.0f / theQ.vec4().squareModulus();
    x2 = theQ.x() * s;    y2 = theQ.y() * s;    z2 = theQ.z() * s;
    xx = theQ.x() * x2;   xy = theQ.x() * y2;   xz = theQ.x() * z2;
    yy = theQ.y() * y2;   yz = theQ.y() * z2;   zz = theQ.z() * z2;
    wx = theQ.w() * x2;   wy = theQ.w() * y2;   wz = theQ.w() * z2;

    changeValue(0, 0) = 1.0f - (yy + zz);
    changeValue(0, 1) = xy - wz;
    changeValue(0, 2) = xz + wy;

    changeValue(1, 0) = xy + wz;
    changeValue(1, 1) = 1.0f - (xx + zz);
    changeValue(1, 2) = yz - wx;

    changeValue(2, 0) = xz - wy;
    changeValue(2, 1) = yz + wx;
    changeValue(2, 2) = 1.0f - (xx + yy);
}

StGLMatrix::~StGLMatrix() {
    //
}

void StGLMatrix::initIdentity() {
    stMemCpy(matrix, ST_IDENT_MAT, sizeof(matrix));
}

void StGLMatrix::initFrustum(const StGLVolume& theFrustum) {
    // column 0
    changeValue(0, 0)  = 2.0f * theFrustum.zNear
                       / (theFrustum.xRight - theFrustum.xLeft);
    changeValue(1, 0)  = 0.0f;
    changeValue(2, 0)  = 0.0f;
    changeValue(3, 0)  = 0.0f;
    // column 1
    changeValue(0, 1)  = 0.0f;
    changeValue(1, 1)  = 2.0f * theFrustum.zNear
                       / (theFrustum.yTop - theFrustum.yBottom);
    changeValue(2, 1)  = 0.0f;
    changeValue(3, 1)  = 0.0f;
    // column 2
    changeValue(0, 2) =   (theFrustum.xRight + theFrustum.xLeft)
                        / (theFrustum.xRight - theFrustum.xLeft);
    changeValue(1, 2) =   (theFrustum.yTop + theFrustum.yBottom)
                        / (theFrustum.yTop - theFrustum.yBottom);
    changeValue(2, 2) = - (theFrustum.zFar + theFrustum.zNear)
                        / (theFrustum.zFar - theFrustum.zNear);
    changeValue(3, 2) = -1.0f;
    // column 3
    changeValue(0, 3) = 0.0f;
    changeValue(1, 3) = 0.0f;
    changeValue(2, 3) = - (2.0f * theFrustum.zFar * theFrustum.zNear)
                        / (theFrustum.zFar - theFrustum.zNear);
    changeValue(3, 3) = 0.0f;

    // additional translation
    if(theFrustum.xTranslation != 0.0f) {
        translate(StGLVec3(theFrustum.xTranslation, 0.0f, 0.0f));
    }
}

void StGLMatrix::initOrtho(const StGLVolume& theVolume) {
    // row 0
    changeValue(0, 0) = 2.0f
                        / (theVolume.xRight - theVolume.xLeft);
    changeValue(0, 1) = 0.0f;
    changeValue(0, 2) = 0.0f;
    changeValue(0, 3) = - (theVolume.xRight + theVolume.xLeft)
                        / (theVolume.xRight - theVolume.xLeft);
    // row 1
    changeValue(1, 0) = 0.0f;
    changeValue(1, 1) = 2.0f
                        / (theVolume.yTop - theVolume.yBottom);
    changeValue(1, 2) = 0.0f;
    changeValue(1, 3) = - (theVolume.yTop + theVolume.yBottom)
                        / (theVolume.yTop - theVolume.yBottom);
    // row 2
    changeValue(2, 0) = 0.0f;
    changeValue(2, 1) = 0.0f;
    changeValue(2, 2) = -2.0f
                        / (theVolume.zFar - theVolume.zNear);
    changeValue(2, 3) = - (theVolume.zFar + theVolume.zNear)
                        / (theVolume.zFar - theVolume.zNear);
    // row 3
    changeValue(3, 0) = 0.0f;
    changeValue(3, 1) = 0.0f;
    changeValue(3, 2) = 0.0f;
    changeValue(3, 3) = 1.0f;

    // additional translation
    if(theVolume.xTranslation != 0.0f) {
        translate(StGLVec3(theVolume.xTranslation, 0.0f, 0.0f));
    }
}

StGLMatrix StGLMatrix::multiply(const StGLMatrix& matrix0, const StGLMatrix& matrix1) {
    StGLMatrix stMatResult;
    size_t k;
    for(size_t i = 0; i < 16; ++i) {
        stMatResult[i] = 0.0f;
        for(k = 0; k < 4; ++k) {
            stMatResult[i] += matrix0.getValue(i % 4, k) * matrix1.getValue(k, i / 4);
        }
    }
    return stMatResult;
}

void StGLMatrix::scale(const GLfloat theSX, const GLfloat theSY, const GLfloat theSZ) {
    StGLMatrix tempMat;
    tempMat.changeValue(0, 0) = theSX;
    tempMat.changeValue(1, 1) = theSY;
    tempMat.changeValue(2, 2) = theSZ;
    *this = multiply(*this, tempMat);
}

void StGLMatrix::translate(const StGLVec3& theVec) {
    StGLMatrix tempMat;
    tempMat.setColumn(theVec, 3);
    *this = multiply(*this, tempMat);
}

void StGLMatrix::rotate(const GLfloat theAngleDegrees,
                        const StGLDir3& theLine) {
    StGLMatrix tempMat;

    GLfloat aSin = std::sin(2.0f * __glPif * theAngleDegrees / 360.0f);
    GLfloat aCos = std::cos(2.0f * __glPif * theAngleDegrees / 360.0f);

    tempMat.changeValue(0, 0) = theLine.x() * theLine.x() * (1.0f - aCos) + aCos;
    tempMat.changeValue(1, 0) = theLine.x() * theLine.y() * (1.0f - aCos) + theLine.z() * aSin;
    tempMat.changeValue(2, 0) = theLine.x() * theLine.z() * (1.0f - aCos) - theLine.y() * aSin;

    tempMat.changeValue(0, 1) = theLine.x() * theLine.y() * (1.0f - aCos) - theLine.z() * aSin;
    tempMat.changeValue(1, 1) = theLine.y() * theLine.y() * (1.0f - aCos) + aCos;
    tempMat.changeValue(2, 1) = theLine.y() * theLine.z() * (1.0f - aCos) + theLine.x() * aSin;

    tempMat.changeValue(0, 2) = theLine.x() * theLine.z() * (1.0f - aCos) + theLine.y() * aSin;
    tempMat.changeValue(1, 2) = theLine.y() * theLine.z() * (1.0f - aCos) - theLine.x() * aSin;
    tempMat.changeValue(2, 2) = theLine.z() * theLine.z() * (1.0f - aCos) + aCos;

    *this = multiply(*this, tempMat);
}

StGLVec4 StGLMatrix::operator*(const StGLVec4& theVec) const {
    return StGLVec4(getValue(0, 0) * theVec.x() + getValue(0, 1) * theVec.y() + getValue(0, 2) * theVec.z() + getValue(0, 3) * theVec.w(),
                    getValue(1, 0) * theVec.x() + getValue(1, 1) * theVec.y() + getValue(1, 2) * theVec.z() + getValue(1, 3) * theVec.w(),
                    getValue(2, 0) * theVec.x() + getValue(2, 1) * theVec.y() + getValue(2, 2) * theVec.z() + getValue(2, 3) * theVec.w(),
                    getValue(3, 0) * theVec.x() + getValue(3, 1) * theVec.y() + getValue(3, 2) * theVec.z() + getValue(3, 3) * theVec.w());
}

StGLVec4 operator*(const StGLVec4& theVec, const StGLMatrix& theMat) {
    return StGLVec4(theVec.x() * theMat.getValue(0, 0) + theVec.y() * theMat.getValue(1, 0) + theVec.z() * theMat.getValue(2, 0) + theVec.w() * theMat.getValue(3, 0),
                    theVec.x() * theMat.getValue(0, 1) + theVec.y() * theMat.getValue(1, 1) + theVec.z() * theMat.getValue(2, 1) + theVec.w() * theMat.getValue(3, 1),
                    theVec.x() * theMat.getValue(0, 2) + theVec.y() * theMat.getValue(1, 2) + theVec.z() * theMat.getValue(2, 2) + theVec.w() * theMat.getValue(3, 2),
                    theVec.x() * theMat.getValue(0, 3) + theVec.y() * theMat.getValue(1, 3) + theVec.z() * theMat.getValue(2, 3) + theVec.w() * theMat.getValue(3, 3));
}

void StGLMatrix::lookAt(const StGLVec3& theEye,
                        const StGLVec3& theCenter,
                        const StGLDir3& theUp) {
    StGLVec3 aForward = theCenter - theEye;
    aForward.normalize();

    // side = forward x up
    StGLVec3 aSide = StGLVec3::cross(aForward, theUp);
    aSide.normalize();

    // recompute up as: up = side x forward
    StGLVec3 anUp = StGLVec3::cross(aSide, aForward);

    StGLMatrix stMMatrix;
    stMMatrix.setRow(    aSide, 0);
    stMMatrix.setRow(     anUp, 1);
    stMMatrix.setRow(-aForward, 2);

    *this = multiply(*this, stMMatrix);
    translate(-theEye);
}

bool StGLMatrix::inverted(StGLMatrix& theInvOut) const {
    // intermediate
    StGLMatrix anInvMat;
    // just short-cuts
    const GLfloat* m   = matrix;
    GLfloat* inv = anInvMat.matrix;

    inv[0]  =  m[5] * m[10] * m[15] - m[5]  * m[11] * m[14] - m[9]  * m[6] * m[15]
            +  m[9] * m[7]  * m[14] + m[13] * m[6]  * m[11] - m[13] * m[7] * m[10];
    inv[4]  = -m[4] * m[10] * m[15] + m[4]  * m[11] * m[14] + m[8]  * m[6] * m[15]
            -  m[8] * m[7]  * m[14] - m[12] * m[6]  * m[11] + m[12] * m[7] * m[10];
    inv[8]  =  m[4] * m[9]  * m[15] - m[4]  * m[11] * m[13] - m[8]  * m[5] * m[15]
            +  m[8] * m[7]  * m[13] + m[12] * m[5]  * m[11] - m[12] * m[7] * m[9];
    inv[12] = -m[4] * m[9]  * m[14] + m[4]  * m[10] * m[13] + m[8]  * m[5] * m[14]
            -  m[8] * m[6]  * m[13] - m[12] * m[5]  * m[10] + m[12] * m[6] * m[9];
    inv[1]  = -m[1] * m[10] * m[15] + m[1]  * m[11] * m[14] + m[9]  * m[2] * m[15]
            -  m[9] * m[3]  * m[14] - m[13] * m[2]  * m[11] + m[13] * m[3] * m[10];
    inv[5]  =  m[0] * m[10] * m[15] - m[0]  * m[11] * m[14] - m[8]  * m[2] * m[15]
            +  m[8] * m[3]  * m[14] + m[12] * m[2]  * m[11] - m[12] * m[3] * m[10];
    inv[9]  = -m[0] * m[9]  * m[15] + m[0]  * m[11] * m[13] + m[8]  * m[1] * m[15]
            -  m[8] * m[3]  * m[13] - m[12] * m[1]  * m[11] + m[12] * m[3] * m[9];
    inv[13] =  m[0] * m[9]  * m[14] - m[0]  * m[10] * m[13] - m[8]  * m[1] * m[14]
            +  m[8] * m[2]  * m[13] + m[12] * m[1]  * m[10] - m[12] * m[2] * m[9];
    inv[2]  =  m[1] * m[6]  * m[15] - m[1]  * m[7]  * m[14] - m[5]  * m[2] * m[15]
            +  m[5] * m[3]  * m[14] + m[13] * m[2]  * m[7]  - m[13] * m[3] * m[6];
    inv[6]  = -m[0] * m[6]  * m[15] + m[0]  * m[7]  * m[14] + m[4]  * m[2] * m[15]
            -  m[4] * m[3]  * m[14] - m[12] * m[2]  * m[7]  + m[12] * m[3] * m[6];
    inv[10] =  m[0] * m[5]  * m[15] - m[0]  * m[7]  * m[13] - m[4]  * m[1] * m[15]
            +  m[4] * m[3]  * m[13] + m[12] * m[1]  * m[7]  - m[12] * m[3] * m[5];
    inv[14] = -m[0] * m[5]  * m[14] + m[0]  * m[6]  * m[13] + m[4]  * m[1] * m[14]
            -  m[4] * m[2]  * m[13] - m[12] * m[1]  * m[6]  + m[12] * m[2] * m[5];
    inv[3]  = -m[1] * m[6]  * m[11] + m[1]  * m[7]  * m[10] + m[5]  * m[2] * m[11]
            -  m[5] * m[3]  * m[10] - m[9]  * m[2]  * m[7]  + m[9]  * m[3] * m[6];
    inv[7]  =  m[0] * m[6]  * m[11] - m[0]  * m[7]  * m[10] - m[4]  * m[2] * m[11]
            +  m[4] * m[3]  * m[10] + m[8]  * m[2]  * m[7]  - m[8]  * m[3] * m[6];
    inv[11] = -m[0] * m[5]  * m[11] + m[0]  * m[7]  * m[9]  + m[4]  * m[1] * m[11]
            -  m[4] * m[3]  * m[9]  - m[8]  * m[1]  * m[7]  + m[8]  * m[3] * m[5];
    inv[15] =  m[0] * m[5]  * m[10] - m[0]  * m[6]  * m[9]  - m[4]  * m[1] * m[10]
            +  m[4] * m[2]  * m[9]  + m[8]  * m[1]  * m[6]  - m[8]  * m[2] * m[5];

    GLfloat aDet = m[0] * inv[0] + m[1] * inv[4] + m[2] * inv[8] + m[3] * inv[12];
    if(aDet == 0.0f) {
        return false;
    }
    aDet = 1.0f / aDet;

    for(size_t anId = 0; anId < 16; ++anId) {
        theInvOut.matrix[anId] = inv[anId] * aDet;
    }
    return true;
}
