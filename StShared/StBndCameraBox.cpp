/**
 * Copyright © 2011-2012 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#include <StGLMesh/StBndCameraBox.h>
#include <StGLMesh/StGLPrism.h>

StBndCameraBox::StBndCameraBox(const StGLMatrix& theTransf)
: StBndBox(),
  myTransf(theTransf) {
    //
}

StBndCameraBox::~StBndCameraBox() {
    //
}

StGLVec3 StBndCameraBox::getCenterGlobal() const {
    StGLVec3 aCenter = getCenter();
    StGLMatrix aMatInv;
    if(!myTransf.inverted(aMatInv)) {
        return aCenter;
    }
    StGLVec4 aPnt = aMatInv * StGLVec4(aCenter, 1.0f);
    return aPnt.xyz();
}

void StBndCameraBox::enlarge(const StGLVec3& theNewPnt) {
    StArray<StGLVec3> aPoints(1);
    aPoints[0] = theNewPnt;
    StBndCameraBox::enlarge(aPoints);
}

void StBndCameraBox::enlarge(const StArray<StGLVec3>& thePoints) {
    if(thePoints.size() == 0) {
        return;
    }
    // translate the points using transformation matrix
    StArray<StGLVec3> aPoints(thePoints.size());
    StGLVec4 aPnt;
    for(size_t aPntId = 0; aPntId < aPoints.size(); ++aPntId) {
        aPnt = myTransf * StGLVec4(thePoints.getValue(aPntId), 1.0f);
        aPoints.changeValue(aPntId) = aPnt.xyz();
    }
    StBndBox::enlarge(aPoints);
}

void StBndCameraBox::getPrism(StGLContext& theCtx,
                              StGLPrism&   thePrism) const {
    if(isVoid()) {
        return;
    }
    const GLfloat aDX = getDX();
    const GLfloat aDY = getDY();
    const GLfloat aDZ = getDZ();
    StGLVec3 aCenter = getCenter();
    StGLMatrix aMatInv;
    if(!myTransf.inverted(aMatInv)) {
        return;
    }
    StGLVec3 aV0(aCenter.x() - aDX * 0.5f, aCenter.y() - aDY * 0.5f, aCenter.z() - aDZ * 0.5f);
    StGLVec3 aV1(aCenter.x() + aDX * 0.5f, aCenter.y() - aDY * 0.5f, aCenter.z() - aDZ * 0.5f);
    StGLVec3 aV2(aCenter.x() + aDX * 0.5f, aCenter.y() + aDY * 0.5f, aCenter.z() - aDZ * 0.5f);
    StGLVec3 aV3(aCenter.x() - aDX * 0.5f, aCenter.y() + aDY * 0.5f, aCenter.z() - aDZ * 0.5f);
    StGLVec3 aV4(aCenter.x() - aDX * 0.5f, aCenter.y() - aDY * 0.5f, aCenter.z() + aDZ * 0.5f);
    StGLVec3 aV5(aCenter.x() + aDX * 0.5f, aCenter.y() - aDY * 0.5f, aCenter.z() + aDZ * 0.5f);
    StGLVec3 aV6(aCenter.x() + aDX * 0.5f, aCenter.y() + aDY * 0.5f, aCenter.z() + aDZ * 0.5f);
    StGLVec3 aV7(aCenter.x() - aDX * 0.5f, aCenter.y() + aDY * 0.5f, aCenter.z() + aDZ * 0.5f);
    StGLVec4 aPnt;
    aPnt = aMatInv * StGLVec4(aV0, 1.0f); aV0 = aPnt.xyz();
    aPnt = aMatInv * StGLVec4(aV1, 1.0f); aV1 = aPnt.xyz();
    aPnt = aMatInv * StGLVec4(aV2, 1.0f); aV2 = aPnt.xyz();
    aPnt = aMatInv * StGLVec4(aV3, 1.0f); aV3 = aPnt.xyz();
    aPnt = aMatInv * StGLVec4(aV4, 1.0f); aV4 = aPnt.xyz();
    aPnt = aMatInv * StGLVec4(aV5, 1.0f); aV5 = aPnt.xyz();
    aPnt = aMatInv * StGLVec4(aV6, 1.0f); aV6 = aPnt.xyz();
    aPnt = aMatInv * StGLVec4(aV7, 1.0f); aV7 = aPnt.xyz();
    thePrism.init(theCtx, aV0, aV1, aV2, aV3, aV4, aV5, aV6, aV7);
}
