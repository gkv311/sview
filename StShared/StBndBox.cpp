/**
 * Copyright © 2011 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#include <StGLMesh/StBndBox.h>
#include <StTemplates/StArrayList.h>

namespace {
    static StGLVec3 getMinValues(const StGLVec3& theVec1, const StGLVec3& theVec2) {
        return StGLVec3(stMin(theVec1.x(), theVec2.x()),
                        stMin(theVec1.y(), theVec2.y()),
                        stMin(theVec1.z(), theVec2.z()));
    }

    static StGLVec3 getMaxValues(const StGLVec3& theVec1, const StGLVec3& theVec2) {
        return StGLVec3(stMax(theVec1.x(), theVec2.x()),
                        stMax(theVec1.y(), theVec2.y()),
                        stMax(theVec1.z(), theVec2.z()));
    }
};

StBndBox::StBndBox()
: StBndContainer() {
    //
}

StBndBox::StBndBox(const StGLVec3& theMin,
                   const StGLVec3& theMax)
: myMin(theMin),
  myMax(theMax) {
    setDefined();
}

StBndBox::~StBndBox() {
    //
}

void StBndBox::reset() {
    StBndContainer::reset();
    myMin = StGLVec3();
    myMax = StGLVec3();
}

bool StBndBox::isIn(const StGLVec3& thePnt) const {
    if(isVoid()) {
        return false;
    }
    return thePnt.x() <= myMax.x() && thePnt.x() >= myMin.x()
        && thePnt.y() <= myMax.y() && thePnt.y() >= myMin.y()
        && thePnt.z() <= myMax.z() && thePnt.z() >= myMin.z();
}

bool StBndBox::areDisjoint(const StBndBox& theBndBox) const {
    return isVoid() || theBndBox.isVoid()
        || this->myMax.x() < theBndBox.myMin.x() || theBndBox.myMax.x() < this->myMin.x()
        || this->myMax.y() < theBndBox.myMin.y() || theBndBox.myMax.y() < this->myMin.y()
        || this->myMax.z() < theBndBox.myMin.z() || theBndBox.myMax.z() < this->myMin.z();
}

void StBndBox::enlarge(const GLfloat theTolerance) {
    if(!isVoid()) {
        StGLVec3 aTolerVec(theTolerance, theTolerance, theTolerance);
        myMin -= aTolerVec;
        myMax += aTolerVec;
    }
}

void StBndBox::enlarge(const StGLVec3& theNewPnt) {
    if(isVoid()) {
        // setup the first point
        myMin = theNewPnt;
        myMax = theNewPnt;
        StBndContainer::setDefined();
    } else {
        myMin = getMinValues(myMin, theNewPnt);
        myMax = getMaxValues(myMax, theNewPnt);
    }
}

void StBndBox::enlarge(const StArray<StGLVec3>& thePoints) {
    if(thePoints.size() == 0) {
        return;
    }
    if(isVoid()) {
        // setup the first point
        myMin = thePoints.getFirst();
        myMax = thePoints.getFirst();
        StBndContainer::setDefined();
    }
    for(size_t aPntId = 1; aPntId < thePoints.size(); ++aPntId) {
        const StGLVec3& aPnt = thePoints[aPntId];
        myMin = getMinValues(myMin, aPnt);
        myMax = getMaxValues(myMax, aPnt);
    }
}
