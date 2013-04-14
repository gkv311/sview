/**
 * Copyright © 2009-2013 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#ifndef __StStereoParams_h_
#define __StStereoParams_h_

#include <StGL/StGLVec.h>

#include <StGLStereo/StFormatEnum.h>
#include <StTemplates/StHandle.h>

/**
 * Common parameters for stereo image representation.
 */
class StStereoParams {

        public:

    typedef enum tagViewMode {
        FLAT_IMAGE,        // flat image
        PANORAMA_SPHERE,   // spherical panorama
        PANORAMA_CYLINDER, // cylindrical panorama
    } ViewMode;

    static StString GET_VIEW_MODE_NAME(ViewMode theViewMode) {
        switch(theViewMode) {
            case PANORAMA_SPHERE: return "sphere";
            case FLAT_IMAGE:
            default:              return "flat";
        }
    }

    static ViewMode GET_VIEW_MODE_FROM_STRING(const StString& theViewModeStr) {
        if(theViewModeStr.isStartsWithIgnoreCase("sphere")) {
            return PANORAMA_SPHERE;
        } else {
            return FLAT_IMAGE;
        }
    }

        public:

    /**
     * Main constructor - default parameters.
     */
    StStereoParams(ViewMode theViewMode = FLAT_IMAGE)
    : mySrcFormat(ST_V_SRC_MONO),
      mySepDxPx(0),
      mySepDxZeroPx(0),
      mySepDyPx(0),
      mySepStepPx(2),
      mySepRotDegrees(0.0f),
      myZRotateDegrees(0.0f),
      myZRotateZero(0.0f),
      myCenter(0.0f, 0.0f),
      myMovStep(0.01f),
      myScaleFactor(1.0f),
      myScaleStep(0.02f),
      myPanoTheta(0.0f),
      myPanoPhi(0.0f),
      myViewMode(theViewMode),
      myToSwapLR(false) {
        //
    }

    /**
     * @return true if source format is MONO image.
     */
    bool isMono() const {
        return mySrcFormat == ST_V_SRC_MONO;
    }

    /**
     * @return image source format.
     */
    StFormatEnum getSrcFormat() const {
        return mySrcFormat;
    }

    /**
     * @param theSrcFormat - image source format.
     */
    void setSrcFormat(StFormatEnum theSrcFormat) {
        mySrcFormat = theSrcFormat;
    }

    /**
     * @return viewing mode (panorama or flat image).
     */
    ViewMode getViewMode() const {
        return myViewMode;
    }

    /**
     * @param theViewMode - viewing mode (panorama or flat image).
     */
    void setViewMode(ViewMode theViewMode) {
        myViewMode = theViewMode;
    }

    /**
     * Switch to the next viewing mode.
     */
    void nextViewMode() {
        switch(myViewMode) {
            case FLAT_IMAGE:        myViewMode = PANORAMA_SPHERE; break;
            case PANORAMA_SPHERE:
            case PANORAMA_CYLINDER:
            default:                myViewMode = FLAT_IMAGE;
        }
    }

    /**
     * @return scaling factor ('zoom').
     */
    GLfloat getScale() const {
        return myScaleFactor;
    }

    /**
     * @return scaling factor inc/dec step.
     */
    GLfloat getScaleStep() const {
        return myScaleStep;
    }

    /**
     * @return horizontal separation in pixels.
     * This number means delta between views.
     */
    GLint getSeparationDx() const {
        return mySepDxZeroPx + mySepDxPx;
    }

    /**
     * Setup neutral point.
     */
    void setSeparationNeutral(const GLint theSepDx = 0) {
        mySepDxZeroPx = theSepDx;
    }

    /**
     * @return vertical separation in pixels.
     * This number means delta between views.
     */
    GLint getSeparationDy() const {
        return mySepDyPx;
    }

    /**
     * @return angular separation between views in degrees.
     */
    GLfloat getSepRotation() const {
        return mySepRotDegrees;
    }

    /**
     * @return flat position vector.
     */
    const StGLVec2& getCenter() const {
        return myCenter;
    }

    /**
     * @return rotation angle #1 for panorama view.
     */
    GLfloat getTheta() const {
        return myPanoTheta;
    }

    /**
     * @return rotation angle #2 for panorama view.
     */
    GLfloat getPhi() const {
        return myPanoPhi;
    }

    /**
     * @return rotation anlge in degrees.
     */
    GLfloat getZRotate() const {
        return myZRotateZero + myZRotateDegrees;
    }

    /**
     * @param theAngleDegrees - rotation anlge in degrees.
     */
    void setZRotateZero(const GLfloat theAngleDegrees) {
        myZRotateZero = theAngleDegrees;
    }

    /**
     * @return true if swap Left/Right flag setted.
     */
    bool isSwapLR() const {
        return myToSwapLR;
    }

    /**
     * @param toSwapLR (bool ) - set swap Left/Right flag.
     */
    void setSwapLR(bool toSwapLR) {
        myToSwapLR = toSwapLR;
    }

    /**
     * Reverse swap Left/Right flag.
     */
    void doSwapLR() {
        myToSwapLR = !myToSwapLR;
    }

    /**
     * Zoom in.
     */
    void scaleIn(const GLfloat theSteps = 1.0f) {
        myScaleFactor *= (1.0f + theSteps * myScaleStep);
    }

    /**
     * Zoom out.
     */
    void scaleOut(const GLfloat theSteps = 1.0f) {
        myScaleFactor /= (1.0f + theSteps * myScaleStep);
    }

    void incSeparationDx() {
        mySepDxPx += mySepStepPx;
    }

    void decSeparationDx() {
        mySepDxPx -= mySepStepPx;
    }

    void incSeparationDy() {
        mySepDyPx += mySepStepPx;
    }

    void decSeparationDy() {
        mySepDyPx -= mySepStepPx;
    }

    void incSepRotation() {
        mySepRotDegrees += 0.1f;
    }

    void decSepRotation() {
        mySepRotDegrees -= 0.1f;
    }

    /**
     * Help method used in StGLImageRegion to fit image into rectangle.
     */
    static inline StGLVec2 getRatioScale(const GLfloat rectRatio, const GLfloat imageRatio) {
        if(rectRatio < imageRatio) {
            return StGLVec2(rectRatio, rectRatio / imageRatio);
        } else {
            return StGLVec2(imageRatio, 1.0f);
        }
    }

    StGLVec2 moveFlatDelta(const StGLVec2& theMoveVec,
                           const GLfloat   theRectRatio) const {
        return StGLVec2((theMoveVec.x() * theRectRatio) / myScaleFactor,
                         theMoveVec.y() / myScaleFactor);
    }

    void moveFlat(const StGLVec2& theMoveVec,
                  const GLfloat   theRectRatio) {
        myCenter += moveFlatDelta(theMoveVec, theRectRatio);
    }

    void moveSphere(const StGLVec2& theMoveVec) {
        myPanoPhi   += theMoveVec.x();
        myPanoTheta += theMoveVec.y();
    }

    void moveToRight() {
        switch(myViewMode) {
            case PANORAMA_SPHERE:
            case PANORAMA_CYLINDER: myPanoPhi += 2.0f; break;
            case FLAT_IMAGE:
            default: myCenter.x() += myMovStep / myScaleFactor;
        }
    }

    void moveToLeft() {
        switch(myViewMode) {
            case PANORAMA_SPHERE:
            case PANORAMA_CYLINDER: myPanoPhi -= 2.0f; break;
            case FLAT_IMAGE:
            default: myCenter.x() -= myMovStep / myScaleFactor;
        }
    }

    void moveToDown() {
        switch(myViewMode) {
            case PANORAMA_SPHERE:
            case PANORAMA_CYLINDER: myPanoTheta -= 2.0f; break;
            case FLAT_IMAGE:
            default: myCenter.y() -= myMovStep / myScaleFactor;
        }
    }

    void moveToUp() {
        switch(myViewMode) {
            case PANORAMA_SPHERE:
            case PANORAMA_CYLINDER: myPanoTheta += 2.0f; break;
            case FLAT_IMAGE:
            default: myCenter.y() += myMovStep / myScaleFactor;
        }
    }

    void incZRotate() {
        myZRotateDegrees -= 90.0f;
    }

    void decZRotate() {
        myZRotateDegrees += 90.0f;
    }

    void incZRotateL() {
        myZRotateDegrees -= 0.1f;
    }

    void decZRotateL() {
        myZRotateDegrees += 0.1f;
    }

    /**
     * Reset parameters.
     */
    void reset() {
        mySepDxPx = mySepDyPx = 0;
        mySepRotDegrees = myZRotateDegrees = 0.0f;
        myCenter.x()  = 0.0f;
        myCenter.y()  = 0.0f;
        myPanoTheta   = 0.0f;
        myPanoPhi     = 0.0f;
        myScaleFactor = 1.0f;
        myToSwapLR    = false;
    }

        private:

    // TODO (Kirill Gavrilov#2#) implement velocity + timer behavior for smoothness
    StFormatEnum mySrcFormat;      //!< source format
    GLint        mySepDxPx;        //!< horizontal    separation in pixels
    GLint        mySepDxZeroPx;    //!< zero-parallax separation in pixels
    GLint        mySepDyPx;        //!< vertical      separation in pixels
    GLint        mySepStepPx;      //!< separation inc/dec step
    GLfloat      mySepRotDegrees;  //!< angular separation in degrees
    GLfloat      myZRotateDegrees; //!< rotation angle in degrees
    GLfloat      myZRotateZero;    //!< zero-rotation angle in degrees
    StGLVec2     myCenter;         //!< relative position
    GLfloat      myMovStep;        //!< flat movement step
    GLfloat      myScaleFactor;    //!< scaling factor
    GLfloat      myScaleStep;      //!< scaling factor inc/dec step
    GLfloat      myPanoTheta;      //!< angle for panorama view
    GLfloat      myPanoPhi;        //!< angle for panorama view
    ViewMode     myViewMode;       //!< viewing mode - panorama or flat image
    bool         myToSwapLR;       //!< reverse left/right views

};

#endif //__StStereoParams_h_
