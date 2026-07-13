/**
 * Copyright © 2010-2012 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#include <StGLMesh/StGLPrism.h>
#include <StGLMesh/StBndBox.h>

#include <StGLCore/StGLCore11.h>

StGLPrism::StGLPrism()
: StGLMesh(GL_LINES),
  myNbIsosX(0),
  myNbIsosY(0),
  myNbIsosZ(0),
  myToShowZ0(true),
  myToShowZ1(true),
  myToShowX0(true),
  myToShowX1(true),
  myToShowY0(true),
  myToShowY1(true) {
    //
}

void StGLPrism::setWireframe(size_t theNbIsosX,
                             size_t theNbIsosY,
                             size_t theNbIsosZ) {
    myNbIsosX = theNbIsosX;
    myNbIsosY = theNbIsosY;
    myNbIsosZ = theNbIsosZ;
}

void StGLPrism::setVisibilityX(bool theToShowX0, bool theToShowX1) {
    myToShowX0 = theToShowX0;
    myToShowX1 = theToShowX1;
}

void StGLPrism::setVisibilityY(bool theToShowY0, bool theToShowY1) {
    myToShowY0 = theToShowY0;
    myToShowY1 = theToShowY1;
}

void StGLPrism::setVisibilityZ(bool theToShowZ0, bool theToShowZ1) {
    myToShowZ0 = theToShowZ0;
    myToShowZ1 = theToShowZ1;
}

bool StGLPrism::init(StGLContext&    theCtx,
                     const StBndBox& theBndBox) {
    const StGLVec3& aMin = theBndBox.getMin();
    const StGLVec3& aMax = theBndBox.getMax();
    return init(theCtx,
                StGLVec3::getLERP(aMin, aMax, 0.5f),
                aMax.x() - aMin.x(),
                aMax.y() - aMin.y(),
                aMax.z() - aMin.z());
}

bool StGLPrism::init(StGLContext&    theCtx,
                     const StGLVec3& theCenter,
                     const GLfloat   theDX,
                     const GLfloat   theDY,
                     const GLfloat   theDZ) {
    return init(theCtx,
                StGLVec3(theCenter.x() - theDX * 0.5f, theCenter.y() - theDY * 0.5f, theCenter.z() - theDZ * 0.5f),
                StGLVec3(theCenter.x() + theDX * 0.5f, theCenter.y() - theDY * 0.5f, theCenter.z() - theDZ * 0.5f),
                StGLVec3(theCenter.x() + theDX * 0.5f, theCenter.y() + theDY * 0.5f, theCenter.z() - theDZ * 0.5f),
                StGLVec3(theCenter.x() - theDX * 0.5f, theCenter.y() + theDY * 0.5f, theCenter.z() - theDZ * 0.5f),
                StGLVec3(theCenter.x() - theDX * 0.5f, theCenter.y() - theDY * 0.5f, theCenter.z() + theDZ * 0.5f),
                StGLVec3(theCenter.x() + theDX * 0.5f, theCenter.y() - theDY * 0.5f, theCenter.z() + theDZ * 0.5f),
                StGLVec3(theCenter.x() + theDX * 0.5f, theCenter.y() + theDY * 0.5f, theCenter.z() + theDZ * 0.5f),
                StGLVec3(theCenter.x() - theDX * 0.5f, theCenter.y() + theDY * 0.5f, theCenter.z() + theDZ * 0.5f));
}

bool StGLPrism::init(StGLContext&    theCtx,
                     const StGLVec3& theX0Y0Z0, // rectangle in near Z plane
                     const StGLVec3& theX1Y0Z0,
                     const StGLVec3& theX1Y1Z0,
                     const StGLVec3& theX0Y1Z0,
                     const StGLVec3& theX0Y0Z1, // rectangle in far Z plane
                     const StGLVec3& theX1Y0Z1,
                     const StGLVec3& theX1Y1Z1,
                     const StGLVec3& theX0Y1Z1) {
    myVertices.initList(12 * 2
        + myNbIsosX * 2 * (size_t(myToShowZ0) + size_t(myToShowZ1) + size_t(myToShowY0) + size_t(myToShowY1))
        + myNbIsosY * 2 * (size_t(myToShowZ0) + size_t(myToShowZ1) + size_t(myToShowX0) + size_t(myToShowX1))
        + myNbIsosZ * 2 * (size_t(myToShowX0) + size_t(myToShowX1) + size_t(myToShowY0) + size_t(myToShowY1)));
    // top-left DZ
    myVertices.add(theX0Y0Z0).add(theX0Y0Z1);
    // top-right DZ
    myVertices.add(theX1Y0Z0).add(theX1Y0Z1);
    // bottom-right DZ
    myVertices.add(theX1Y1Z0).add(theX1Y1Z1);
    // bottom-left DZ
    myVertices.add(theX0Y1Z0).add(theX0Y1Z1);
    // far top DX
    myVertices.add(theX0Y0Z1).add(theX1Y0Z1);
    // far bottom DX
    myVertices.add(theX0Y1Z1).add(theX1Y1Z1);
    // far left DY
    myVertices.add(theX0Y0Z1).add(theX0Y1Z1);
    // far right DY
    myVertices.add(theX1Y0Z1).add(theX1Y1Z1);
    // near top DX
    myVertices.add(theX0Y0Z0).add(theX1Y0Z0);
    // near bottom DX
    myVertices.add(theX0Y1Z0).add(theX1Y1Z0);
    // near left DY
    myVertices.add(theX0Y0Z0).add(theX0Y1Z0);
    // near right DY
    myVertices.add(theX1Y0Z0).add(theX1Y1Z0);

    // X-isolines
    for(size_t aDX = 1; aDX < (myNbIsosX + 1); ++aDX) {
        // DY lines near
        if(myToShowZ0) {
            myVertices.add(StGLVec3::getLERP(theX0Y0Z0, theX1Y0Z0, GLfloat(aDX) / GLfloat(myNbIsosX + 1)));
            myVertices.add(StGLVec3::getLERP(theX0Y1Z0, theX1Y1Z0, GLfloat(aDX) / GLfloat(myNbIsosX + 1)));
        }
        // DY lines far
        if(myToShowZ1) {
            myVertices.add(StGLVec3::getLERP(theX0Y0Z1, theX1Y0Z1, GLfloat(aDX) / GLfloat(myNbIsosX + 1)));
            myVertices.add(StGLVec3::getLERP(theX0Y1Z1, theX1Y1Z1, GLfloat(aDX) / GLfloat(myNbIsosX + 1)));
        }
        // DZ lines left
        if(myToShowY0) {
            myVertices.add(StGLVec3::getLERP(theX0Y0Z0, theX1Y0Z0, GLfloat(aDX) / GLfloat(myNbIsosX + 1)));
            myVertices.add(StGLVec3::getLERP(theX0Y0Z1, theX1Y0Z1, GLfloat(aDX) / GLfloat(myNbIsosX + 1)));
        }
        // DZ lines right
        if(myToShowY1) {
            myVertices.add(StGLVec3::getLERP(theX0Y1Z0, theX1Y1Z0, GLfloat(aDX) / GLfloat(myNbIsosX + 1)));
            myVertices.add(StGLVec3::getLERP(theX0Y1Z1, theX1Y1Z1, GLfloat(aDX) / GLfloat(myNbIsosX + 1)));
        }
    }

    // Y-isolines
    for(size_t aDY = 1; aDY < (myNbIsosY + 1); ++aDY) {
        // DX lines near
        if(myToShowZ0) {
            myVertices.add(StGLVec3::getLERP(theX0Y0Z0, theX0Y1Z0, GLfloat(aDY) / GLfloat(myNbIsosY + 1)));
            myVertices.add(StGLVec3::getLERP(theX1Y0Z0, theX1Y1Z0, GLfloat(aDY) / GLfloat(myNbIsosY + 1)));
        }
        // DX lines far
        if(myToShowZ1) {
            myVertices.add(StGLVec3::getLERP(theX0Y0Z1, theX0Y1Z1, GLfloat(aDY) / GLfloat(myNbIsosY + 1)));
            myVertices.add(StGLVec3::getLERP(theX1Y0Z1, theX1Y1Z1, GLfloat(aDY) / GLfloat(myNbIsosY + 1)));
        }
        // DZ lines left
        if(myToShowX0) {
            myVertices.add(StGLVec3::getLERP(theX0Y0Z0, theX0Y1Z0, GLfloat(aDY) / GLfloat(myNbIsosY + 1)));
            myVertices.add(StGLVec3::getLERP(theX0Y0Z1, theX0Y1Z1, GLfloat(aDY) / GLfloat(myNbIsosY + 1)));
        }
        // DZ lines right
        if(myToShowX1) {
            myVertices.add(StGLVec3::getLERP(theX1Y0Z0, theX1Y1Z0, GLfloat(aDY) / GLfloat(myNbIsosY + 1)));
            myVertices.add(StGLVec3::getLERP(theX1Y0Z1, theX1Y1Z1, GLfloat(aDY) / GLfloat(myNbIsosY + 1)));
        }
    }

    // Z-isolines
    for(size_t aDZ = 1; aDZ < (myNbIsosZ + 1); ++aDZ) {
        // DX lines from near-top to far-top
        if(myToShowY0) {
            myVertices.add(StGLVec3::getLERP(theX0Y0Z0, theX0Y0Z1, GLfloat(aDZ) / GLfloat(myNbIsosZ + 1)));
            myVertices.add(StGLVec3::getLERP(theX1Y0Z0, theX1Y0Z1, GLfloat(aDZ) / GLfloat(myNbIsosZ + 1)));
        }
        // DX lines from near-bottom to far-bottom
        if(myToShowY1) {
            myVertices.add(StGLVec3::getLERP(theX0Y1Z0, theX0Y1Z1, GLfloat(aDZ) / GLfloat(myNbIsosZ + 1)));
            myVertices.add(StGLVec3::getLERP(theX1Y1Z0, theX1Y1Z1, GLfloat(aDZ) / GLfloat(myNbIsosZ + 1)));
        }
        // DY lines from near-left to far-left
        if(myToShowX0) {
            myVertices.add(StGLVec3::getLERP(theX0Y0Z0, theX0Y0Z1, GLfloat(aDZ) / GLfloat(myNbIsosZ + 1)));
            myVertices.add(StGLVec3::getLERP(theX0Y1Z0, theX0Y1Z1, GLfloat(aDZ) / GLfloat(myNbIsosZ + 1)));
        }
        // DY lines from near-right to far-right
        if(myToShowX1) {
            myVertices.add(StGLVec3::getLERP(theX1Y0Z0, theX1Y0Z1, GLfloat(aDZ) / GLfloat(myNbIsosZ + 1)));
            myVertices.add(StGLVec3::getLERP(theX1Y1Z0, theX1Y1Z1, GLfloat(aDZ) / GLfloat(myNbIsosZ + 1)));
        }
    }
    return myVertexBuf.init(theCtx, myVertices);
}
