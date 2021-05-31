/**
 * Copyright © 2019-2020 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#include <StGLMesh/StGLUVCylinder.h>

#include <StGLCore/StGLCore20.h>
#include <StGL/StGLContext.h>

StGLUVCylinder::StGLUVCylinder(const StGLVec3& theCenter,
                               const float theHeight,
                               const float theRadius,
                               const int theRings)
: StGLMesh(GL_TRIANGLE_STRIP),
  myCenter(theCenter),
  myRadius(theRadius),
  myHeight(theHeight),
  myAngleFrom(0.0f),
  myAngleTo(float(M_PI * 2.0)),
  myNbRings(theRings) {
    //
}

StGLUVCylinder::StGLUVCylinder(const StGLVec3& theCenter,
                               const float theHeight,
                               const float theRadius,
                               const float theAngleFrom,
                               const float theAngleTo,
                               const int theRings)
: StGLMesh(GL_TRIANGLE_STRIP),
  myCenter(theCenter),
  myRadius(theRadius),
  myHeight(theHeight),
  myAngleFrom(theAngleFrom),
  myAngleTo(theAngleTo),
  myNbRings(theRings) {
    //
}


StGLUVCylinder::~StGLUVCylinder() {
    //
}

bool StGLUVCylinder::computeMesh() {
    clearRAM();
    if(myNbRings == 0) {
        return false;
    }

    const int aNbVerts = (myNbRings + 1) * 2;
    myVertices.initArray(aNbVerts);
    myNormals .initArray(aNbVerts);
    myTCoords .initArray(aNbVerts);

    const float anAngle = myAngleTo - myAngleFrom;
    for(int aRingIter = 0; aRingIter <= myNbRings; ++aRingIter) {
        const float aPhi    = myAngleFrom + float(aRingIter) * anAngle / float(myNbRings);
        const float aTexCrd = float(aRingIter) / float(myNbRings);
        myTCoords.changeValue(aRingIter * 2 + 0) = StGLVec2(aTexCrd, 0.0f);
        myTCoords.changeValue(aRingIter * 2 + 1) = StGLVec2(aTexCrd, 1.0f);

        const StGLVec3 aNorm(cosf(aPhi), 0.0f, sinf(aPhi));
        myNormals.changeValue(aRingIter * 2 + 0) = aNorm;
        myNormals.changeValue(aRingIter * 2 + 1) = aNorm;

        myVertices.changeValue(aRingIter * 2 + 0) = myCenter + aNorm * myRadius - StGLVec3(0.0f, myHeight * 0.5f, 0.0f);
        myVertices.changeValue(aRingIter * 2 + 1) = myCenter + aNorm * myRadius + StGLVec3(0.0f, myHeight * 0.5f, 0.0f);
    }
    return true;
}
