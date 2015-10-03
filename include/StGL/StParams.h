/**
 * Copyright © 2009-2015 Kirill Gavrilov <kirill@sview.ru>
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

    enum ViewMode {
        FLAT_IMAGE,        //!< normal 2D image
        PANORAMA_CUBEMAP,  //!< cubemap texture
        PANORAMA_SPHERE,   //!< spherical panorama
        //PANORAMA_CYLINDER, //!< cylindrical panorama
    };

    static const int THE_SEP_STEP_PX = 2; //!< separation inc/dec step

    static StString GET_VIEW_MODE_NAME(ViewMode theViewMode) {
        switch(theViewMode) {
            case PANORAMA_CUBEMAP: return "cubemap";
            case PANORAMA_SPHERE:  return "sphere";
            case FLAT_IMAGE:
            default:               return "flat";
        }
    }

    static ViewMode GET_VIEW_MODE_FROM_STRING(const StString& theViewModeStr) {
        if(theViewModeStr.isStartsWithIgnoreCase(stCString("cubemap"))) {
            return PANORAMA_CUBEMAP;
        } else if(theViewModeStr.isStartsWithIgnoreCase(stCString("sphere"))) {
            return PANORAMA_SPHERE;
        } else {
            return FLAT_IMAGE;
        }
    }

        public:

    /**
     * Main constructor - default parameters.
     */
    StStereoParams()
    : Src1SizeX(0),
      Src1SizeY(0),
      Src2SizeX(0),
      Src2SizeY(0),
      ViewingMode(FLAT_IMAGE),
      Timestamp(0.0f),
      StereoFormat(StFormat_Mono),
      ToSwapLR(false),
      PanCenter(0.0f, 0.0f),
      PanTheta(0.0f),
      PanPhi(0.0f),
      ScaleFactor(1.0f),
      mySepDxPx(0),
      mySepDxZeroPx(0),
      mySepDyPx(0),
      mySepRotDegrees(0.0f),
      myXRotateDegrees(0.0f),
      myYRotateDegrees(0.0f),
      myZRotateDegrees(0.0f),
      myZRotateZero(0.0f) {
        //
    }

    /**
     * @return true if source format is MONO image.
     */
    bool isMono() const {
        return StereoFormat == StFormat_Mono;
    }

    /**
     * Switch to the next viewing mode.
     */
    void nextViewMode() {
        switch(ViewingMode) {
            case FLAT_IMAGE:        ViewingMode = PANORAMA_SPHERE; break;
            case PANORAMA_CUBEMAP:
            case PANORAMA_SPHERE:
            default:                ViewingMode = FLAT_IMAGE;
        }
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
     * @return rotation angle in degrees.
     */
    GLfloat getXRotate() const {
        return myXRotateDegrees;
    }

    /**
     * Change rotation angle in degrees.
     */
    void setXRotate(const GLfloat theValue) {
        myXRotateDegrees = theValue;
    }

    /**
     * @return rotation angle in degrees.
     */
    GLfloat getYRotate() const {
        return myYRotateDegrees;
    }

    /**
     * Change rotation angle in degrees.
     */
    void setYRotate(const GLfloat theValue) {
        myYRotateDegrees = theValue;
    }

    /**
     * @return rotation angle in degrees.
     */
    GLfloat getZRotate() const {
        return myZRotateZero + myZRotateDegrees;
    }

    /**
     * @param theAngleDegrees - rotation angle in degrees.
     */
    void setZRotateZero(const GLfloat theAngleDegrees) {
        myZRotateZero = theAngleDegrees;
    }

    /**
     * @param toSwapLR set swap Left/Right flag.
     */
    void setSwapLR(const bool theToSwapLR) {
        ToSwapLR = theToSwapLR;
    }

    /**
     * Reverse swap Left/Right flag.
     */
    void doSwapLR() {
        ToSwapLR = !ToSwapLR;
    }

    /**
     * Zoom in.
     */
    void scaleIn(const GLfloat theDuration = 0.02f) {
        ScaleFactor *= (1.0f + theDuration);
    }

    /**
     * Zoom out.
     */
    void scaleOut(const GLfloat theDuration = 0.02f) {
        ScaleFactor = stMax(ScaleFactor / (1.0f + theDuration), 0.05f);
    }

    void incSeparationDx() {
        mySepDxPx += THE_SEP_STEP_PX;
    }

    void decSeparationDx() {
        mySepDxPx -= THE_SEP_STEP_PX;
    }

    void incSeparationDy() {
        mySepDyPx += THE_SEP_STEP_PX;
    }

    void decSeparationDy() {
        mySepDyPx -= THE_SEP_STEP_PX;
    }

    void incSepRotation(const GLfloat theDuration = 0.02f) {
        mySepRotDegrees += 5.0f * theDuration;
    }

    void decSepRotation(const GLfloat theDuration = 0.02f) {
        mySepRotDegrees -= 5.0f * theDuration;
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
        return StGLVec2((theMoveVec.x() * theRectRatio) / ScaleFactor,
                         theMoveVec.y() / ScaleFactor);
    }

    void moveFlat(const StGLVec2& theMoveVec,
                  const GLfloat   theRectRatio) {
        PanCenter += moveFlatDelta(theMoveVec, theRectRatio);
    }

    void moveSphere(const StGLVec2& theMoveVec) {
        PanPhi   += theMoveVec.x();
        PanTheta = clipPitch(PanTheta + theMoveVec.y());
    }

    void moveToRight(const GLfloat theDuration = 0.02f) {
        switch(ViewingMode) {
            case PANORAMA_SPHERE:
            case PANORAMA_CUBEMAP: PanPhi += 100.0f * theDuration; break;
            case FLAT_IMAGE:
            default: PanCenter.x() += 0.5f * theDuration / ScaleFactor;
        }
    }

    void moveToLeft(const GLfloat theDuration = 0.02f) {
        switch(ViewingMode) {
            case PANORAMA_SPHERE:
            case PANORAMA_CUBEMAP: PanPhi -= 100.0f * theDuration; break;
            case FLAT_IMAGE:
            default: PanCenter.x() -= 0.5f * theDuration / ScaleFactor;
        }
    }

    /**
     * Clip pitch angle to the range [-90, 90] degrees.
     */
    static float clipPitch(const float thePitchDeg) {
        if(thePitchDeg <= -90.0f) {
            return -90.0f;
        } else if(thePitchDeg >= 90.0f) {
            return  90.0f;
        }
        return thePitchDeg;
    }

    void moveToDown(const GLfloat theDuration = 0.02f) {
        switch(ViewingMode) {
            case PANORAMA_SPHERE:
            case PANORAMA_CUBEMAP:
                PanTheta = clipPitch(PanTheta - 100.0f * theDuration);
                break;
            case FLAT_IMAGE:
            default:
                PanCenter.y() -= 0.5f * theDuration / ScaleFactor;
                break;
        }
    }

    void moveToUp(const GLfloat theDuration = 0.02f) {
        switch(ViewingMode) {
            case PANORAMA_SPHERE:
            case PANORAMA_CUBEMAP:
                PanTheta = clipPitch(PanTheta + 100.0f * theDuration);
                break;
            case FLAT_IMAGE:
            default:
                PanCenter.y() += 0.5f * theDuration / ScaleFactor;
                break;
        }
    }

    void incZRotate() {
        myZRotateDegrees -= 90.0f;
    }

    void decZRotate() {
        myZRotateDegrees += 90.0f;
    }

    void incZRotateL(const GLfloat theDuration = 0.02f) {
        myZRotateDegrees -= 5.0f * theDuration;
    }

    void decZRotateL(const GLfloat theDuration = 0.02f) {
        myZRotateDegrees += 5.0f * theDuration;
    }

    /**
     * Reset parameters.
     */
    void reset() {
        mySepDxPx = mySepDyPx = 0;
        mySepRotDegrees  = 0.0f;
        myXRotateDegrees = 0.0f;
        myYRotateDegrees = 0.0f;
        myZRotateDegrees = 0.0f;
        PanCenter.x() = 0.0f;
        PanCenter.y() = 0.0f;
        PanTheta      = 0.0f;
        PanPhi        = 0.0f;
        ScaleFactor   = 1.0f;
        ToSwapLR      = false;
    }

        public:

    size_t       Src1SizeX;        //!< width  of the 1st original image
    size_t       Src1SizeY;        //!< height of the 1st original image
    size_t       Src2SizeX;        //!< width  of the 2nd original image
    size_t       Src2SizeY;        //!< height of the 2nd original image

    ViewMode     ViewingMode;      //!< viewing mode - panorama or flat image
    GLfloat      Timestamp;        //!< playback timestamp

    StFormat     StereoFormat;     //!< stereoscopic format
    bool         ToSwapLR;         //!< reverse left/right views

    StGLVec2     PanCenter;        //!< relative position
    GLfloat      PanTheta;         //!< angle for panorama view
    GLfloat      PanPhi;           //!< angle for panorama view

    GLfloat      ScaleFactor;      //!< scaling factor

        private:

    GLint        mySepDxPx;        //!< horizontal    separation in pixels
    GLint        mySepDxZeroPx;    //!< zero-parallax separation in pixels
    GLint        mySepDyPx;        //!< vertical      separation in pixels
    GLfloat      mySepRotDegrees;  //!< angular separation in degrees
    GLfloat      myXRotateDegrees; //!< rotation angle in degrees
    GLfloat      myYRotateDegrees; //!< rotation angle in degrees
    GLfloat      myZRotateDegrees; //!< rotation angle in degrees
    GLfloat      myZRotateZero;    //!< zero-rotation angle in degrees

};

#endif // __StStereoParams_h_
