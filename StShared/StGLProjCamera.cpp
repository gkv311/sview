/**
 * Copyright © 2010-2017 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#include <StGLStereo/StGLProjCamera.h>

#include <StGL/StGLContext.h>
#include <StGLCore/StGLCore11.h>

namespace {
    static const GLfloat ST_DTR_HALF = 0.5f * 0.0174532925f; // (degrees -> radians) * 0.5
};

StGLProjCamera::StGLProjCamera()
: myMatrix(),
  myMatrixMono(),
  myFOVy(45.0f),
  myZoom(1.0f),
  myAspect(1.0f),
  myZScreen(10.0f),
  myIOD(0.5f),
  myFrustL(),
  myFrustR(),
  myFrustM(),
  myFrust(NULL),
  myIsPersp(true) {
    //
    myFrust = &myFrustM;
    myFrustM.zNear = myFrustL.zNear = myFrustR.zNear = 3.0f;
    myFrustM.zFar  = myFrustL.zFar  = myFrustR.zFar  = 30.0f;
    updateFrustum();
}

StGLProjCamera::StGLProjCamera(const GLfloat theFOVy,
                               const GLfloat theZNear,
                               const GLfloat theZFar,
                               const GLfloat theZScreen)
: myMatrix(),
  myMatrixMono(),
  myFOVy(theFOVy),
  myZoom(1.0f),
  myAspect(1.0f),
  myZScreen(theZScreen),
  myIOD(0.5f),
  myFrustL(),
  myFrustR(),
  myFrustM(),
  myFrust(NULL),
  myIsPersp(true) {
    //
    myFrust = &myFrustM;
    myFrustM.zNear = myFrustL.zNear = myFrustR.zNear = theZNear;
    myFrustM.zFar  = myFrustL.zFar  = myFrustR.zFar  = theZFar;
    updateFrustum();
}

StGLProjCamera::StGLProjCamera(const StGLProjCamera& theOther)
: myMatrix(theOther.myMatrix),
  myMatrixMono(theOther.myMatrixMono),
  myFOVy(theOther.myFOVy),
  myZoom(theOther.myZoom),
  myAspect(theOther.myAspect),
  myZScreen(theOther.myZScreen),
  myIOD(theOther.myIOD),
  myFrustL(theOther.myFrustL),
  myFrustR(theOther.myFrustR),
  myFrustM(theOther.myFrustM),
  myFrust(NULL),
  myIsPersp(theOther.myIsPersp) {
    myFrust = &myFrustM;
    if(theOther.myFrust == &theOther.myFrustL) {
        myFrust = &myFrustL;
    } else if(theOther.myFrust == &theOther.myFrustR) {
        myFrust = &myFrustR;
    }
    updateFrustum();
}

void StGLProjCamera::copyFrom(const StGLProjCamera& theOther) {
    myMatrix = theOther.myMatrix;
    myMatrixMono = theOther.myMatrixMono;
    myFOVy = theOther.myFOVy;
    myZoom = theOther.myZoom;
    myAspect = theOther.myAspect;
    myZScreen = theOther.myZScreen;
    myIOD = theOther.myIOD;
    myFrustL = theOther.myFrustL;
    myFrustR = theOther.myFrustR;
    myFrustM = theOther.myFrustM;
    myIsPersp = theOther.myIsPersp;

    myFrust = &myFrustM;
    if(theOther.myFrust == &theOther.myFrustL) {
        myFrust = &myFrustL;
    } else if(theOther.myFrust == &theOther.myFrustR) {
        myFrust = &myFrustR;
    }
    updateFrustum();
}

void StGLProjCamera::getZParams(const GLdouble theZValue,
                                StRectD_t&     theSectRect) const {
    if(myIsPersp) {
        theSectRect.top() = GLdouble(myZoom) * theZValue * std::tan(ST_DTR_HALF * myFOVy);
    } else {
        // Z-value doesn't change the section
        theSectRect.top() = GLdouble(myZoom) * myFrustM.zNear;/// * std::tan(ST_DTR_HALF * myFOVy);
    }
    theSectRect.bottom() = -theSectRect.top();
    theSectRect.left()   = -myAspect * theSectRect.top();
    theSectRect.right()  = -theSectRect.left();
}

void StGLProjCamera::setZScreen(const GLfloat theZScreen) {
    // save the relationship
    myIOD = myIOD / myZScreen * theZScreen;
    myFrustM.zNear = myFrustL.zNear = myFrustR.zNear = myFrustM.zNear / myZScreen * theZScreen;
    // set new value
    myZScreen = theZScreen;
}

void StGLProjCamera::setPerspective(const bool theIsPerspective) {
    myIsPersp = theIsPerspective;
    updateFrustum();
}

void StGLProjCamera::resize(const float theAspect) {
    myAspect = theAspect;
    updateFrustum();
}

GLfloat StGLProjCamera::getFOVyZoomed() const {
    GLfloat aDYHalf = myFrustM.zNear * std::tan(ST_DTR_HALF * myFOVy); /// ??? myIsPersp
    return std::atan2(aDYHalf, myFrustM.zNear * myZoom);
}

void StGLProjCamera::updateFrustum() {
    // sets top of frustum based on FOVy and near clipping plane
    GLfloat aDYHalf = myIsPersp
                    ? (myZoom * myFrustM.zNear * std::tan(ST_DTR_HALF * myFOVy))
                    : (myZoom * myFrustM.zNear); ///
    // sets right of frustum based on aspect ratio
    GLfloat aDXHalf = myAspect * aDYHalf;
    GLfloat aDXStereoShift = (0.5f * myIOD) * myFrustM.zNear / myZScreen;

    // frustum for left view
    myFrustL.yTop    =  aDYHalf;
    myFrustL.yBottom = -aDYHalf;
    myFrustL.xLeft   = -aDXHalf + aDXStereoShift;
    myFrustL.xRight  =  aDXHalf + aDXStereoShift;
    myFrustL.xTranslation =  0.5f * myIOD; // X translation to cancel parallax
    // frustum for right view
    myFrustR.yTop    =  aDYHalf;
    myFrustR.yBottom = -aDYHalf;
    myFrustR.xLeft   = -aDXHalf - aDXStereoShift;
    myFrustR.xRight  =  aDXHalf - aDXStereoShift;
    myFrustR.xTranslation = -0.5f * myIOD; // X translation to cancel parallax
    // frustum for mono view
    myFrustM.yTop    =  aDYHalf;
    myFrustM.yBottom = -aDYHalf;
    myFrustM.xLeft   = -aDXHalf;
    myFrustM.xRight  =  aDXHalf;
    myFrustM.xTranslation = 0.0f;

    // update current matrix
    setupMatrix();
}

void StGLProjCamera::setView(const unsigned int theView) {
    switch(theView) {
        case ST_DRAW_LEFT:
            myFrust = &myFrustL;
            break;
        case ST_DRAW_RIGHT:
            myFrust = &myFrustR;
            break;
        case ST_DRAW_MONO:
        default:
            myFrust = &myFrustM;
    }
    // update current matrix
    setupMatrix();
}

void StGLProjCamera::setupMatrix() {
    if(myIsPersp) {
        myMatrix.initFrustum(*myFrust);
        myMatrixMono.initFrustum(myFrustM);
    } else {
        myMatrix.initOrtho(myFrustM);
        myMatrixMono.initOrtho(myFrustM);
    }
}

void StGLProjCamera::setupFixed(StGLContext& theCtx) {
#if defined(GL_ES_VERSION_2_0)
    (void )theCtx;
#else
    theCtx.core11->glMatrixMode(GL_PROJECTION);
    theCtx.core11->glLoadIdentity();

    if(myIsPersp) {
        // setup current frustum
        theCtx.core11->glFrustum(myFrust->xLeft, myFrust->xRight,
                                 myFrust->yBottom, myFrust->yTop,
                                 myFrust->zNear, myFrust->zFar);
        // translate to cancel parallax
        theCtx.core11->glTranslatef(myFrust->xTranslation, 0.0f, 0.0f);
    } else {
        theCtx.core11->glOrtho(myFrustM.xLeft, myFrustM.xRight,
                               myFrustM.yBottom, myFrustM.yTop,
                               myFrustM.zNear, myFrustM.zFar);
    }

    // turn back to model view matrix
    theCtx.core11->glMatrixMode(GL_MODELVIEW);
#endif
}

StString StGLProjCamera::toString() const {
    StRectD_t aSect; getZParams(aSect);
    return StString("ProjCamera, FOV= ") + myFOVy
       + ";\n Z-Near=   " + myFrustM.zNear
       + "; Z-Screen= "   + myZScreen
       + "; Z-Far=    "   + myFrustM.zFar
       + "; IOD= "    + myIOD
       + ";\nscr L= " + aSect.left()
       + "; R= "      + aSect.right()
       + "; B= "      + aSect.bottom()
       + "; T= "      + aSect.top();
}
