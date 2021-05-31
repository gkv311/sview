/**
 * Copyright © 2010-2012 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#include <StGLMesh/StGLCircle.h>

#include <StGLCore/StGLCore11.h>

/// TODO (Kirill Gavrilov#9) move to the common header
namespace {
    static const GLfloat ST_TWOPI = 6.2831853071795864769252867665590f;
};

StGLCircle::StGLCircle()
: StGLMesh(GL_LINE_LOOP),
  myCenter(),
  myRadiusX(0.0f),
  myRadiusY(0.0f),
  myPointsCount(0) {
    //
}

StGLCircle::StGLCircle(const StGLVec3& theCenter,
                       const GLfloat   theRadius,
                       const GLsizei   thePointsCount)
: StGLMesh(GL_LINE_LOOP),
  myCenter(theCenter),
  myRadiusX(theRadius),
  myRadiusY(theRadius),
  myPointsCount(thePointsCount) {
    //
}

void StGLCircle::create(const StGLVec3& theCenter,
                        GLfloat theRadius,
                        GLsizei thePointsCount) {
    create(theCenter, theRadius, theRadius, thePointsCount);
}

void StGLCircle::create(const StGLVec3& theCenter,
                        const GLfloat   theRadiusX,
                        const GLfloat   theRadiusY,
                        const GLsizei   thePointsCount) {
    myCenter      = theCenter;
    myRadiusX     = theRadiusX;
    myRadiusY     = theRadiusY;
    myPointsCount = (thePointsCount < 8) ? 8 : thePointsCount;

    // reset current mesh
    clearRAM();
}

bool StGLCircle::computeMesh() {
    // reset current mesh
    clearRAM();

    if(myPointsCount <= 0) {
        return false;
    }

    myVertices.initList(myPointsCount);
    GLfloat theta = 0.0f;
    for(GLsizei pointId = 0; pointId < myPointsCount; ++pointId) {
        theta = GLfloat(pointId) * ST_TWOPI / GLfloat(myPointsCount);
        myVertices.add(StGLVec3(myCenter.x() + myRadiusX * std::cos(theta),
                                myCenter.y() + myRadiusY * std::sin(theta),
                                myCenter.z()));
    }

    // define the bounding sphere
    myBndSphere.define(myCenter, stMax(myRadiusX, myRadiusY));
    return true;
}

bool StGLCircle::initColorsArray(const StGLVec4& theColor) {
    if(myVertices.isEmpty()) {
        return false;
    }
    myColors.initList(myVertices.size());
    for(size_t aPntId = 0; aPntId < myVertices.size(); ++aPntId) {
        myColors.add(theColor);
    }
    return true;
}
