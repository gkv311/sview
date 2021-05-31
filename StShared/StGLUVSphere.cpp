/**
 * Copyright © 2010-2019 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#include <StGLMesh/StGLUVSphere.h>
#include <StGLMesh/StBndSphere.h>

#include <StGLCore/StGLCore20.h>
#include <StGL/StGLContext.h>

namespace {
    static const GLfloat ST_PI     = 3.1415926535897932384626433832795f;
    static const GLfloat ST_TWOPI  = 6.2831853071795864769252867665590f;
    static const GLfloat ST_PIDIV2 = 1.5707963267948966192313216916397f;
}

StGLUVSphere::StGLUVSphere(const StGLVec3& theCenter,
                           const GLfloat   theRadius,
                           const size_t    theRings,
                           const bool      theIsHemisphere)
: StGLMesh(GL_TRIANGLE_STRIP),
  myPrimCounts(1),
  myIndPointers(1),
  myCenter(theCenter),
  myRadius(theRadius),
  myRings(theRings),
  myIsHemisphere(theIsHemisphere) {
    //
}

StGLUVSphere::StGLUVSphere(const StBndSphere& theBndSphere,
                           const size_t       theRings)
: StGLMesh(GL_TRIANGLE_STRIP),
  myPrimCounts(1),
  myIndPointers(1),
  myCenter(theBndSphere.getCenter()),
  myRadius(theBndSphere.getRadius()),
  myRings(theRings),
  myIsHemisphere(false) {
    //
}

StGLUVSphere::~StGLUVSphere() {
    //
}

bool StGLUVSphere::computeMesh() {
    // reset current mesh
    clearRAM();
    if(myRings == 0) {
        return false;
    }

    size_t aRingsCount  = myRings;
    size_t pointPerRing = myRings;

    const size_t aVerticesCount = (aRingsCount + 1) * (pointPerRing + 1);
    myVertices.initArray(aVerticesCount);
    myNormals.initArray(aVerticesCount);
    myTCoords.initArray(aVerticesCount);

    GLfloat theta = 0.0f;
    GLfloat phi = 0.0f;

    StGLVec3* aNorm = NULL;
    StGLVec3* aVert = NULL;
    StGLVec2 tcrd(0.0f, 0.0f);

    for(size_t ringId = 0; ringId <= aRingsCount; ++ringId) {
        theta = GLfloat(ringId) * ST_PI / GLfloat(aRingsCount) - ST_PIDIV2;
        tcrd.y() = GLfloat(ringId) / GLfloat(aRingsCount);

        for(size_t pointId = 0; pointId <= pointPerRing; ++pointId) {
            if(myIsHemisphere) {
                phi = ST_PIDIV2 + GLfloat(pointId) * ST_PI / GLfloat(pointPerRing);
            } else {
                phi = GLfloat(pointId) * ST_TWOPI / GLfloat(pointPerRing);
            }

            tcrd.x() = GLfloat(pointId) / GLfloat(pointPerRing);

            aNorm = &myNormals [(pointPerRing + 1) * ringId + pointId];
            aVert = &myVertices[(pointPerRing + 1) * ringId + pointId];

            aNorm->x() = cosf(theta) * cosf(phi);
            aNorm->y() = sinf(theta);
            aNorm->z() = cosf(theta) * sinf(phi);
            aVert->x() = myCenter.x() + myRadius * aNorm->x();
            aVert->y() = myCenter.y() + myRadius * aNorm->y();
            aVert->z() = myCenter.z() + myRadius * aNorm->z();

            myTCoords[(pointPerRing + 1) * ringId + pointId] = tcrd;
        }
    }

    // create the indices arrays
    size_t inicesCount = 2 * (pointPerRing + 1) * aRingsCount;
    myIndices.initList(inicesCount);

    myPrimCounts.initList(aRingsCount);
    myIndPointers.initList(aRingsCount);

    GLsizei primCount = 2 * GLsizei(pointPerRing + 1);
    for(size_t ringId = 0; ringId < aRingsCount; ++ringId) {
        myIndPointers.add((GLvoid* )(ringId * primCount * sizeof(GLuint))); // pointer diff
        myPrimCounts.add(primCount);
        for(size_t pointId = 0; pointId <= pointPerRing; ++pointId) {
            myIndices.add(GLuint((pointPerRing + 1) * (ringId + 1) + pointId));
            myIndices.add(GLuint((pointPerRing + 1) * ringId + pointId));
        }
    }

    // define the bounding sphere
    myBndSphere.define(myCenter, myRadius);
    return true;
}

void StGLUVSphere::drawKernel(StGLContext& theCtx) const {
    myIndexBuf.bind(theCtx);
    theCtx.core20fwd->glMultiDrawElements(GL_TRIANGLE_STRIP,
                                           (GLsizei* )&myPrimCounts.getFirst(), // pointer will be used for iteration - data will not be changed
                                           myIndexBuf.getDataType(),
                                           (const GLvoid** )&myIndPointers.getFirst(),
                                           GLsizei(myIndPointers.size()));
    myIndexBuf.unbind(theCtx);
}
