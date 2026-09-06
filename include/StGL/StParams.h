/**
 * Copyright © 2009-2020 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#ifndef __StStereoParams_h_
#define __StStereoParams_h_

#include <StGL/StGLVec.h>

#include <StGLStereo/StFormatEnum.h>
#include <StTemplates/StHandle.h>

/**
 * Surface to map image onto.
 */
enum StViewSurface {
    StViewSurface_Plain,      //!< normal 2D image
    StViewSurface_Theater,    //!< theater panorama
    StViewSurface_Cubemap,    //!< cubemap texture
    StViewSurface_Sphere,     //!< spherical panorama, 360 degrees
    StViewSurface_Hemisphere, //!< spherical panorama, 180 degrees
    StViewSurface_Cylinder,   //!< cylindrical panorama
    StViewSurface_CubemapEAC, //!< equi-angular cubemap
};

/**
 * Common parameters for stereo image representation.
 */
class StStereoParams {

        public:

    ST_LOCAL static StString GET_VIEW_MODE_NAME(StViewSurface theViewMode) {
        switch(theViewMode) {
            case StViewSurface_Cubemap:    return "cubemap";
            case StViewSurface_CubemapEAC: return "eac";
            case StViewSurface_Sphere:     return "sphere";
            case StViewSurface_Hemisphere: return "hemisphere";
            case StViewSurface_Cylinder:   return "cylinder";
            case StViewSurface_Theater:    return "theater";
            case StViewSurface_Plain:      return "flat";
        }
        return "flat";
    }

    ST_LOCAL static StViewSurface GET_VIEW_MODE_FROM_STRING(const StString& theViewModeStr) {
        if(theViewModeStr.isStartsWithIgnoreCase(stCString("cubemap"))) {
            return StViewSurface_Cubemap;
        } else if(theViewModeStr.isStartsWithIgnoreCase(stCString("eac"))) {
            return StViewSurface_CubemapEAC;
        } else if(theViewModeStr.isStartsWithIgnoreCase(stCString("sphere"))) {
            return StViewSurface_Sphere;
        } else if(theViewModeStr.isStartsWithIgnoreCase(stCString("hemisphere"))) {
            return StViewSurface_Hemisphere;
        } else if(theViewModeStr.isStartsWithIgnoreCase(stCString("cylinder"))) {
            return StViewSurface_Cylinder;
        } else if(theViewModeStr.isStartsWithIgnoreCase(stCString("theater"))) {
            return StViewSurface_Theater;
        } else {
            return StViewSurface_Plain;
        }
    }

    /**
     * Convert panorama mode into surface view.
     */
    ST_LOCAL static StViewSurface getViewSurfaceForPanoramaSource(StPanorama thePano, bool theToFallbackSphere) {
        switch(thePano) {
            case StPanorama_Cubemap6_1:
            case StPanorama_Cubemap1_6:
            case StPanorama_Cubemap3_2:
                return StViewSurface_Cubemap;
            case StPanorama_Cubemap3_2ytb:
            case StPanorama_Cubemap2_3ytb:
              return StViewSurface_CubemapEAC;
            case StPanorama_Sphere:
                return StViewSurface_Sphere;
            case StPanorama_Hemisphere:
                return StViewSurface_Hemisphere;
            case StPanorama_OFF:
                break;
        }
        return theToFallbackSphere ? StViewSurface_Sphere : StViewSurface_Plain;
    }

        public:

    /**
     * Main constructor - default parameters.
     */
    ST_LOCAL StStereoParams()
    : PanCenter(0.0f, 0.0f) {
        //
    }

    /**
     * @return true if source format is MONO image.
     */
    ST_LOCAL bool isMono() const {
        return StereoFormat == StFormat_Mono;
    }

    /**
     * @return horizontal separation in pixels.
     * This number means delta between views.
     */
    ST_LOCAL int getSeparationDx() const {
        return mySepDxZeroPx + mySepDxPx;
    }

    /**
     * Set horizontal separation in pixels.
     */
    ST_LOCAL void setSeparationDx(int theValue) {
        mySepDxPx = theValue - mySepDxZeroPx;
    }

    /** Return neutral point. */
    ST_LOCAL int getSeparationNeutral() const { return mySepDxZeroPx; }

    /**
     * Setup neutral point.
     */
    ST_LOCAL void setSeparationNeutral(int theSepDx = 0) {
        mySepDxZeroPx = theSepDx;
    }

    /**
     * @return vertical separation in pixels.
     * This number means delta between views.
     */
    ST_LOCAL GLint getSeparationDy() const {
        return mySepDyPx;
    }

    /**
     * Set vertical separation in pixels.
     */
    ST_LOCAL void setSeparationDy(int theValue) {
        mySepDyPx = theValue;
    }

    /**
     * @return angular separation between views in degrees.
     */
    ST_LOCAL float getSepRotation() const {
        return mySepRotDegrees;
    }

    /**
     * @return angular separation between views in degrees.
     */
    ST_LOCAL void setSepRotation(float theValue) {
        mySepRotDegrees = theValue;
    }

    /**
     * @return angle for panorama view
     */
    ST_LOCAL float getPanYaw() const {
        return myPanYawZero + myPanYaw;
    }

    /**
     * @return angle for panorama view
     */
    ST_LOCAL float getPanPitch() const {
        return myPanPitchZero + myPanPitch;
    }

    /**
     * @return rotation angle in degrees.
     */
    ST_LOCAL float getXRotate() const {
        return myXRotateDegrees;
    }

    /**
     * Change rotation angle in degrees.
     */
    ST_LOCAL void setXRotate(float theValue) {
        myXRotateDegrees = theValue;
    }

    /**
     * @return rotation angle in degrees.
     */
    ST_LOCAL float getYRotate() const {
        return myYRotateDegrees;
    }

    /**
     * Change rotation angle in degrees.
     */
    ST_LOCAL void setYRotate(float theValue) {
        myYRotateDegrees = theValue;
    }

    /**
     * @return rotation angle in degrees.
     */
    ST_LOCAL float getZRotate() const {
        return myZRotateZero + myZRotateDegrees;
    }

    /**
     * @param theAngleDegrees - rotation angle in degrees.
     */
    ST_LOCAL void setZRotateZero(float theAngleDegrees) {
        myZRotateZero = theAngleDegrees;
    }

    /**
     * Setup default rotation.
     */
    ST_LOCAL void setRotateZero(float theYaw, float thePitch, float theRoll) {
        myPanYawZero   = theYaw;
        myPanPitchZero = thePitch;
        myZRotateZero  = theRoll;
    }

    /**
     * @return true if rotation is not modified
     */
    ST_LOCAL bool isZeroRotate() const {
        return myPanYaw == 0.0f
            && myPanPitch == 0.0f
            && myZRotateDegrees == 0.0f;
    }

    /**
     * @param toSwapLR set swap Left/Right flag.
     */
    ST_LOCAL void setSwapLR(const bool theToSwapLR) {
        ToSwapLR = theToSwapLR;
    }

    /**
     * Reverse swap Left/Right flag.
     */
    ST_LOCAL void doSwapLR() {
        ToSwapLR = !ToSwapLR;
    }

    /**
     * Zoom in.
     */
    ST_LOCAL void scaleIn(const GLfloat theDuration = 0.02f) {
        ScaleFactor *= (1.0f + theDuration);
    }

    /**
     * Zoom out.
     */
    ST_LOCAL void scaleOut(const GLfloat theDuration = 0.02f) {
        ScaleFactor = stMax(ScaleFactor / (1.0f + theDuration), 0.05f);
    }

    /**
     * Help method used in StGLImageRegion to fit image into rectangle.
     */
    ST_LOCAL static inline StGLVec2 getRatioScale(const GLfloat rectRatio, const GLfloat imageRatio) {
        if(rectRatio < imageRatio) {
            return StGLVec2(rectRatio, rectRatio / imageRatio);
        } else {
            return StGLVec2(imageRatio, 1.0f);
        }
    }

    ST_LOCAL StGLVec2 moveFlatDelta(const StGLVec2& theMoveVec,
                                    const GLfloat   theRectRatio) const {
        return StGLVec2((theMoveVec.x() * theRectRatio) / ScaleFactor,
                         theMoveVec.y() / ScaleFactor);
    }

    ST_LOCAL void moveFlat(const StGLVec2& theMoveVec,
                  const GLfloat   theRectRatio) {
        PanCenter += moveFlatDelta(theMoveVec, theRectRatio);
    }

    ST_LOCAL void moveSphere(const StGLVec2& theMoveVec) {
        myPanYaw += theMoveVec.x();
        myPanPitch = clipPitch(myPanPitch + theMoveVec.y());
    }

    ST_LOCAL void moveToRight(const float theDuration = 0.02f) {
        switch(ViewingMode) {
            case StViewSurface_Sphere:
            case StViewSurface_Hemisphere:
            case StViewSurface_Cylinder:
            case StViewSurface_Theater:
            case StViewSurface_Cubemap:
            case StViewSurface_CubemapEAC:
                myPanYaw += 100.0f * theDuration;
                break;
            case StViewSurface_Plain:
                PanCenter.x() += 0.5f * theDuration / ScaleFactor;
                break;
        }
    }

    ST_LOCAL void moveToLeft(const float theDuration = 0.02f) {
        switch(ViewingMode) {
            case StViewSurface_Sphere:
            case StViewSurface_Hemisphere:
            case StViewSurface_Cylinder:
            case StViewSurface_Theater:
            case StViewSurface_Cubemap:
            case StViewSurface_CubemapEAC:
                myPanYaw -= 100.0f * theDuration;
                break;
            case StViewSurface_Plain:
                PanCenter.x() -= 0.5f * theDuration / ScaleFactor;
                break;
        }
    }

    /**
     * Clip pitch angle to the range [-90, 90] degrees.
     */
    ST_LOCAL static float clipPitch(const float thePitchDeg) {
        if(thePitchDeg <= -90.0f) {
            return -90.0f;
        } else if(thePitchDeg >= 90.0f) {
            return  90.0f;
        }
        return thePitchDeg;
    }

    ST_LOCAL void moveToDown(const float theDuration = 0.02f) {
        switch(ViewingMode) {
            case StViewSurface_Sphere:
            case StViewSurface_Hemisphere:
            case StViewSurface_Cylinder:
            case StViewSurface_Theater:
            case StViewSurface_Cubemap:
            case StViewSurface_CubemapEAC:
                myPanPitch = clipPitch(myPanPitch - 100.0f * theDuration);
                break;
            case StViewSurface_Plain:
                PanCenter.y() -= 0.5f * theDuration / ScaleFactor;
                break;
        }
    }

    ST_LOCAL void moveToUp(const float theDuration = 0.02f) {
        switch(ViewingMode) {
            case StViewSurface_Sphere:
            case StViewSurface_Hemisphere:
            case StViewSurface_Cylinder:
            case StViewSurface_Theater:
            case StViewSurface_Cubemap:
            case StViewSurface_CubemapEAC:
                myPanPitch = clipPitch(myPanPitch + 100.0f * theDuration);
                break;
            case StViewSurface_Plain:
                PanCenter.y() += 0.5f * theDuration / ScaleFactor;
                break;
        }
    }

    ST_LOCAL void incZRotate() {
        myZRotateDegrees -= 90.0f;
    }

    ST_LOCAL void decZRotate() {
        myZRotateDegrees += 90.0f;
    }

    ST_LOCAL void incZRotateL(const GLfloat theDuration = 0.02f) {
        myZRotateDegrees -= 5.0f * theDuration;
    }

    ST_LOCAL void decZRotateL(const GLfloat theDuration = 0.02f) {
        myZRotateDegrees += 5.0f * theDuration;
    }

    /**
     * Reset parameters.
     */
    ST_LOCAL bool reset() {
        if (mySepDxPx == 0
            && mySepDyPx == 0
            && mySepRotDegrees == 0.0f
            && myXRotateDegrees == 0.0f
            && myYRotateDegrees == 0.0f
            && myZRotateDegrees == 0.0f
            && PanCenter.x() == 0.0f
            && PanCenter.y() == 0.0f
            && myPanPitch == 0.0f
            && myPanYaw == 0.0f
            && ScaleFactor == 1.0f
            && !ToSwapLR) {
            return false;
        }

        mySepDxPx = mySepDyPx = 0;
        mySepRotDegrees  = 0.0f;
        myXRotateDegrees = 0.0f;
        myYRotateDegrees = 0.0f;
        myZRotateDegrees = 0.0f;
        PanCenter.x() = 0.0f;
        PanCenter.y() = 0.0f;
        myPanPitch    = 0.0f;
        myPanYaw      = 0.0f;
        ScaleFactor   = 1.0f;
        ToSwapLR      = false;
        return true;
    }

        public:

    size_t       Src1SizeX = 0;        //!< width  of the 1st original image
    size_t       Src1SizeY = 0;        //!< height of the 1st original image
    size_t       Src2SizeX = 0;        //!< width  of the 2nd original image
    size_t       Src2SizeY = 0;        //!< height of the 2nd original image

    StViewSurface ViewingMode = StViewSurface_Plain; //!< viewing mode - panorama or flat image

    GLfloat       Timestamp = 0.0f; //!< playback timestamp

    StFormat     StereoFormat = StFormat_Mono; //!< stereoscopic format
    bool         ToSwapLR = false;             //!< reverse left/right views
    bool         ToFlipCubeZ = false;          //!< reverse Z-coordinate in cubemap

    StGLVec2     PanCenter;          //!< relative position
    GLfloat      ScaleFactor = 1.0f; //!< scaling factor

        private:

    int   mySepDxPx = 0;           //!< horizontal    separation in pixels
    int   mySepDxZeroPx = 0;       //!< zero-parallax separation in pixels
    int   mySepDyPx = 0;           //!< vertical      separation in pixels
    float mySepRotDegrees = 0.0f;  //!< angular separation in degrees
    float myXRotateDegrees = 0.0f; //!< rotation angle in degrees
    float myYRotateDegrees = 0.0f; //!< rotation angle in degrees
    float myZRotateDegrees = 0.0f; //!< rotation angle in degrees
    float myPanYaw = 0.0f;         //!< angle for panorama view
    float myPanPitch = 0.0f;       //!< angle for panorama view
    float myPanYawZero = 0.0f;     //!< zero-rotation angle in degrees
    float myPanPitchZero = 0.0f;   //!< zero-rotation angle in degrees
    float myZRotateZero = 0.0f;    //!< zero-rotation angle in degrees

};

#endif // __StStereoParams_h_
