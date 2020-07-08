/**
 * Copyright © 2010-2017 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#ifndef __StGLProjCamera_h_
#define __StGLProjCamera_h_

#include <StTemplates/StRect.h>
#include <StGL/StGLEnums.h>
#include <StGL/StGLMatrix.h>

class StGLContext;

/**
 * This class represents the PROJECTION camera settings and defines projection matrix as result.
 * That is - it defines:
 *  - projection type (orthographic or perspective);
 *  - projection options (near / far clipping planes, field of view);
 *  - stereoscopic distortions for perspective projection
 *    (Z-screen position of zero parallax and intra-ocular distance);
 * To setup the camera position, direction and rotation you should use another matrix (model view)!
 */
class StGLProjCamera {

        public:

    /**
     * Default projection camera:
     *  FOVy    = 45
     *  ZNear   =  3
     *  ZFar    = 30
     *  ZScreen = 10
     */
    ST_CPPEXPORT StGLProjCamera();

    /**
     * Copy constructor.
     */
    ST_CPPEXPORT StGLProjCamera(const StGLProjCamera& theOther);

    /**
     * Custom projection camera.
     */
    ST_CPPEXPORT StGLProjCamera(const GLfloat theFOVy,
                                const GLfloat theZNear,
                                const GLfloat theZFar,
                                const GLfloat theZScreen);

    /**
     * Copy camera settings.
     */
    ST_CPPEXPORT void copyFrom(const StGLProjCamera& theOther);

    /**
     * Copy camera settings.
     */
    ST_LOCAL StGLProjCamera& operator=(const StGLProjCamera& theOther) {
        copyFrom(theOther);
        return *this;
    }

    /**
     * Get projection type.
     */
    inline bool isPerspective() const {
        return myIsPersp;
    }

    /**
     * Changes the projection type.
     */
    ST_CPPEXPORT void setPerspective(const bool theIsPerspective);

    /**
     * Returns the linear zoom factor.
     * @return zoom factor
     */
    inline GLfloat getZoom() const {
        return myZoom;
    }

    /**
     * Changes the linear zoom factor.
     * @param theZoom zoom factor
     */
    inline void setZoom(const GLfloat theZoom) {
        myZoom = theZoom;
    }

    /**
     * Returns the intraocular distance (stereo separation).
     * @return the intraocular distance
     */
    inline GLfloat getIOD() const {
        return myIOD;
    }

    /**
     * Sets the intraocular distance (stereo separation).
     * @param theIOD the intraocular distance
     */
    inline void setIOD(const GLfloat theIOD) {
        myIOD = theIOD;
    }

    /**
     * Return the current Field Of View in y-axis.
     * @return Field Of View in y-axis
     */
    inline GLfloat getFOVy() const {
        return myFOVy;
    }

    /**
     * Setup the Field Of View in y-axis (degrees). Should be in 0 < FOVy < 180.
     * @param theFOVy new Field Of View value
     */
    inline void setFOVy(const GLfloat theFOVy) {
        myFOVy = theFOVy;
        updateFrustum();
    }

    /**
     * Return the Field Of View in y-axis with linear zoom factor applied.
     * @return Field Of View in y-axis (zoomed)
     */
    ST_CPPEXPORT GLfloat getFOVyZoomed() const;

    /**
     * Returns the Near Z-clipping plane position.
     * @return Z-position
     */
    inline GLfloat getZNear() const {
        return myFrustM.zNear;
    }

    /**
     * Get current Screen Z plane position. This is a stereoscopic focus (zero-parallax plane).
     * @return Z-position
     */
    inline GLfloat getZScreen() const {
        return myZScreen;
    }

    /**
     * Setup the Screen Z plane position. This is a stereoscopic focus (zero-parallax plane).
     * Z-near and IOD will be proportionally changed!
     * @param theZScreen Z-position
     */
    ST_CPPEXPORT void setZScreen(const GLfloat theZScreen);

    /**
     * Returns the Far Z-clipping plane position.
     * @return Z-position
     */
    inline GLfloat getZFar() const {
        return myFrustM.zFar;
    }

    /**
     * Setup the Far Z-clipping plane. Does NOT affects the perspective distortions.
     * Should be larger than Near Z-clipping plane position and enough to fit whole scene.
     * Infinity values are OK but cause lower Z-buffer accuracy.
     * @param theZFar Z-position
     */
    inline void setZFar(const GLfloat theZFar) {
        myFrustM.zFar = myFrustL.zFar = myFrustR.zFar = theZFar;
    }

    /**
     * Compute the frustum section (rectangle) at current Z-screen position.
     */
    inline void getZParams(StRectD_t& theSectRect) const {
        getZParams(myZScreen, theSectRect);
    }

    /**
     * Compute the frustum section (rectangle) at specified Z-position
     * (perspective distortion).
     * @param theZValue   Z position
     * @param theSectRect computed frustum section
     */
    ST_CPPEXPORT void getZParams(const GLdouble theZValue,
                                 StRectD_t&     theSectRect) const;

    /**
     * Returns the GL projection matrix. It generally should NOT be changed outside.
     * @return the projection matrix
     */
    inline const StGLMatrix& getProjMatrix() const {
        return myMatrix;
    }

    /**
     * Returns the mono GL projection matrix.
     * It could be used if some object should be always rendered in mono even in stereoscopic mode.
     * For example - the stereo-pairs which are prerendered stereoscopic scenes and should not be modified.
     * Use this method only when you sure!
     * @return the mono projection matrix
     */
    inline const StGLMatrix& getProjMatrixMono() const {
        return myMatrixMono;
    }

    /**
     * Returns the display aspect ratio.
     */
    inline GLfloat getAspect() const {
        return myAspect;
    }

    /**
     * Recompute projection matrix.
     */
    ST_CPPEXPORT void resize(const float theAspect);

    /**
     * Recompute projection matrix.
     */
    ST_LOCAL void resize(int theSizeX,
                         int theSizeY) {
        resize(float(theSizeX) / float(theSizeY > 0 ? theSizeY : 1));
    }

    /**
     * Setup projection frustum.
     */
    ST_CPPEXPORT void updateFrustum();

    /**
     * Setup camera projection matrices according to specified view.
     * @param theView stereo flag (left eye / right eye / mono)
     */
    ST_CPPEXPORT void setView(const unsigned int theView);

    /**
     * Setup projection matrix according to computed frustum.
     */
    ST_CPPEXPORT void setupMatrix();

    /**
     * Setup current global OpenGL projection matrix (fixed pipeline!).
     * @deprecated This method uses deprecated OpenGL API!
     */
    ST_CPPEXPORT void setupFixed(StGLContext& theCtx);

    inline const StGLVolume* getMonoFrustrum() const {
        return &myFrustM;
    }

    /**
     * Return TRUE if custom stereoscopic projection frustum has been set.
     */
    ST_LOCAL bool isCustomProjection() const { return myIsCustomFrust; }

    /**
     * Unset custom projection.
     */
    ST_CPPEXPORT void resetCustomProjection();

    /**
     * Setup custom projection.
     */
    ST_CPPEXPORT void setCustomProjection(const StRectF_t& theLeft, const StRectF_t& theRight);

    /**
     * Returns the string description for the camera.
     * For debug purposes...
     */
    ST_CPPEXPORT StString toString() const;

        private:

    StGLMatrix  myMatrix;     //!< current projection matrix
    StGLMatrix  myMatrixMono; //!< we store mono projection matrix to allow draw special object

    StRectF_t   myVrFrustumL; //!< custom VR projection frustum, left eye
    StRectF_t   myVrFrustumR; //!< custom VR projection frustum, right eye
    bool        myIsCustomFrust;

    GLfloat     myFOVy;       //!< field of view in y-axis (degrees)
    GLfloat     myZoom;       //!< linear zoom factor (changes the effective FOVy value!)
    GLfloat     myAspect;     //!< screen aspect ratio, recomputed on window size change
    GLfloat     myZScreen;    //!< screen projection plane
    GLfloat     myIOD;        //!< intraocular distance
    StGLVolume  myFrustL;     //!< frustum for left view
    StGLVolume  myFrustR;     //!< frustum for right view
    StGLVolume  myFrustM;     //!< frustum for mono view
    StGLVolume* myFrust;      //!< active frustum
    bool        myIsPersp;    //!< projection matrix type: perspective or orthogonal

};

#endif //__StGLProjCamera_h_
