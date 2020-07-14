/**
 * StGLWidgets, small C++ toolkit for writing GUI using OpenGL.
 * Copyright © 2010-2020 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#ifndef __StGLImageRegion_h_
#define __StGLImageRegion_h_

#include <StGLMesh/StGLUVCylinder.h>
#include <StGLMesh/StGLUVSphere.h>
#include <StGLMesh/StGLQuads.h>

#include <StGLWidgets/StGLWidget.h>
#include <StGLWidgets/StGLImageProgram.h>
#include <StGLStereo/StGLTextureQueue.h>

#include <StGL/StParams.h>
#include <StGL/StPlayList.h>

#include <StSlots/StAction.h>
#include <StSettings/StEnumParam.h>

class StGLIcon;

class StGLImageRegion : public StGLWidget {

        public:

    typedef enum tagDisplayMode {
        MODE_STEREO,        //!< normal draw
        MODE_ONLY_LEFT,     //!< draw only Left  view
        MODE_ONLY_RIGHT,    //!< draw only Right view
        MODE_PARALLEL,      //!< draw parallel   pair
        MODE_CROSSYED,      //!< draw cross-eyed pair
    } DisplayMode;

    typedef enum tagDisplayRatio {
        RATIO_AUTO,  //!< use PAR
        RATIO_1_1,   //!< 1:1
        RATIO_4_3,   //!< 4:3
        RATIO_16_9,  //!< 16:9
        RATIO_16_10, //!< 16:10
        RATIO_221_1, //!< 2.21:1
        RATIO_5_4,   //!< 5:4
    } DisplayRatio;

    /**
     * Actions identifiers.
     */
    enum ActionId {
        Action_Reset = 0,
        Action_SwapLR,
        Action_GammaDec,
        Action_GammaInc,
        Action_SepXDec,
        Action_SepXInc,
        Action_SepYDec,
        Action_SepYInc,
        Action_SepRotDec,
        Action_SepRotInc,
        Action_Rot90Counter,
        Action_Rot90Clockwise,
        Action_RotCounter,
        Action_RotClockwise,
        Action_ModeNext,
        Action_PanLeft,
        Action_PanRight,
        Action_PanUp,
        Action_PanDown,
        Action_ScaleIn,
        Action_ScaleOut,
        Action_RotYLeft,
        Action_RotYRight,
        Action_RotXUp,
        Action_RotXDown,
        ActionsNb,
    };

        public: //!< public interface

    /**
     * Default constructor.
     */
    ST_CPPEXPORT StGLImageRegion(StGLWidget* theParent,
                                 const StHandle<StGLTextureQueue>& theTextureQueue,
                                 const bool  theUsePanningKeys);

    /**
     * Return playlist.
     */
    ST_LOCAL const StHandle<StPlayList>& getPlayList() const {
        return myList;
    }

    /**
     * Set playlist.
     */
    ST_LOCAL void setPlayList(const StHandle<StPlayList>& theList) {
        myList = theList;
    }

    /**
     * Return icon widget displayed on swipe gesture.
     */
    StGLIcon* changeIconPrev() { return myIconPrev; }

    /**
     * Return icon widget displayed on swipe gesture.
     */
    StGLIcon* changeIconNext() { return myIconNext; }

    /**
     * Setup device orientation.
     */
    ST_LOCAL void setDeviceOrientation(const StGLQuaternion& theQ) { myDeviceQuat = theQ; }

    /**
     * Dragging delay in milliseconds, 0.0 by default.
     */
    ST_LOCAL inline double getDragDelayMs() const {
        return myDragDelayMs;
    }

    /**
     * Setup dragging delay in milliseconds.
     */
    ST_LOCAL inline void setDragDelayMs(const double theValue) {
        myDragDelayMs = theValue;
    }

    ST_LOCAL inline StHandle<StGLTextureQueue>& getTextureQueue() {
        return myTextureQueue;
    }

    ST_CPPEXPORT StHandle<StStereoParams> getSource();

    /**
     * Compute the head orientation.
     * @param theView view identifier (to apply stereo separation)
     * @param theToApplyDefShift if TRUE the default transformation will be applied (stored in the file + 90 degrees Yaw shift)
     */
    ST_LOCAL StGLQuaternion getHeadOrientation(unsigned int theView,
                                               const bool theToApplyDefShift) const {
        StGLQuaternion anOri;
        getHeadOrientation(anOri, theView, theToApplyDefShift);
        return anOri;
    }

    /**
     * Compute the head orientation.
     * @param theView view identifier (to apply stereo separation)
     * @param theToApplyDefShift if TRUE the default transformation will be applied (stored in the file + 90 degrees Yaw shift)
     */
    ST_CPPEXPORT bool getHeadOrientation(StGLQuaternion& theOrient,
                                         unsigned int theView,
                                         const bool theToApplyDefShift) const;

    /**
     * Return true if there is any video stream.
     */
    ST_LOCAL bool hasVideoStream() { return myHasVideoStream; }

    const StArrayList< StHandle<StAction> >& getActions() const {
        return myActions;
    }

    StArrayList< StHandle<StAction> >& changeActions() {
        return myActions;
    }

    ST_CPPEXPORT virtual ~StGLImageRegion();
    ST_CPPEXPORT virtual void stglUpdate(const StPointD_t& thePointZo,
                                         bool theIsPreciseInput) ST_ATTR_OVERRIDE;
    ST_CPPEXPORT virtual bool stglInit() ST_ATTR_OVERRIDE;
    ST_CPPEXPORT virtual void stglDraw(unsigned int theView) ST_ATTR_OVERRIDE;
    ST_CPPEXPORT virtual bool tryClick  (const StClickEvent& theEvent, bool& theIsItemClicked)   ST_ATTR_OVERRIDE;
    ST_CPPEXPORT virtual bool tryUnClick(const StClickEvent& theEvent, bool& theIsItemUnclicked) ST_ATTR_OVERRIDE;
    ST_CPPEXPORT virtual bool doKeyDown (const StKeyEvent& theEvent) ST_ATTR_OVERRIDE;
    ST_CPPEXPORT virtual bool doKeyUp   (const StKeyEvent& theEvent) ST_ATTR_OVERRIDE;
    ST_CPPEXPORT virtual bool doScroll  (const StScrollEvent& theEvent) ST_ATTR_OVERRIDE;

    ST_CPPEXPORT bool doGesture(const StGestureEvent& theEvent);

    /**
     * Auxiliary method to discard frames in the textures queue without bound OpenGL context.
     */
    ST_CPPEXPORT void stglSkipFrames();

    /**
     * Return sample aspect ratio.
     */
    ST_LOCAL float getSampleRatio() const { return mySampleRatio; }

    /**
    * Return frame dimensions.
    */
    ST_LOCAL const StVec2<int>& getFrameSize() const { return myFrameSize; }

        public: //! @name Properties

    struct {

        StHandle<StEnumParam>         DisplayMode;           //!< StGLImageRegion::DisplayMode    - display mode
        StHandle<StEnumParam>         DisplayRatio;          //!< StGLImageRegion::DisplayRatio   - display ratio
        StHandle<StBoolParamNamed>    ToHealAnamorphicRatio; //!< correct aspect ratio for 1080p/720p anamorphic pairs
        StHandle<StEnumParam>         TextureFilter;         //!< StGLImageProgram::TextureFilter - texture filter;
        StHandle<StFloat32Param>      Gamma;                 //!< gamma correction coefficient
        StHandle<StFloat32Param>      Brightness;            //!< brightness level
        StHandle<StFloat32Param>      Saturation;            //!< saturation value

        // per file parameters
        StHandle<StStereoParams>      stereoFile;
        StHandle<StBoolParamNamed>    SwapLR;                //!< reversion flag
        StHandle<StInt32ParamNamed>   ViewMode;              //!< StStereoParams::ViewMode
        StHandle<StFloat32Param>      SeparationDX;          //!< DX separation
        StHandle<StFloat32Param>      SeparationDY;          //!< DY separation
        StHandle<StFloat32Param>      SeparationRot;         //!< angle separation

    } params;

        public:  //! @name Signals

    struct {
        /**
         * Emit new item signal.
         */
        StSignal<void (void )> onOpenItem;
    } signals;

        private:

    /**
     * Call bound callbacks bound to parameters.
     */
    ST_LOCAL void onParamsChanged();

    ST_LOCAL void doParamsReset(const size_t );

    ST_LOCAL void doParamsGamma(const size_t theDir) {
        if(!params.stereoFile.isNull()) {
            theDir == 1 ? params.Gamma->increment() : params.Gamma->decrement();
        }
    }

    ST_LOCAL void doParamsSepX(const size_t theDir) {
        if(!params.stereoFile.isNull()) {
            theDir == 1 ? params.SeparationDX->increment() : params.SeparationDX->decrement();
        }
    }

    ST_LOCAL void doParamsSepY(const size_t theDir) {
        if(!params.stereoFile.isNull()) {
            theDir == 1 ? params.SeparationDY->increment() : params.SeparationDY->decrement();
        }
    }

    ST_LOCAL void doParamsRotZ90(const size_t theDir) {
        if(!params.stereoFile.isNull()) {
            theDir == 1 ? params.stereoFile->incZRotate() : params.stereoFile->decZRotate();
        }
    }

    ST_LOCAL void doParamsRotZLeft(const double theValue) {
        if(!params.stereoFile.isNull()) {
            params.stereoFile->decZRotateL((GLfloat )theValue);
        }
    }

    ST_LOCAL void doParamsRotZRight(const double theValue) {
        if(!params.stereoFile.isNull()) {
            params.stereoFile->incZRotateL((GLfloat )theValue);
        }
    }

    ST_LOCAL void doParamsSepZDec(const double theValue) {
        float aValue = params.SeparationRot->getValue() - 5.0f * float(theValue);
        params.SeparationRot->setValue(aValue);
    }

    ST_LOCAL void doParamsSepZInc(const double theValue) {
        float aValue = params.SeparationRot->getValue() + 5.0f * float(theValue);
        params.SeparationRot->setValue(aValue);
    }

    ST_LOCAL void doParamsModeNext(const size_t ) {
        const int aMode = params.ViewMode->getValue();
        switch(aMode) {
            case StViewSurface_Plain: {
                params.ViewMode->setValue(StViewSurface_Sphere);
                return;
            }
            case StViewSurface_Cubemap: {
                params.ViewMode->setValue(StViewSurface_Plain);
                return;
            }
            case StViewSurface_Sphere: {
                params.ViewMode->setValue(StViewSurface_Cubemap);
                return;
            }
        }
    }

    ST_LOCAL void doParamsPanLeft(const double theValue) {
        if(!params.stereoFile.isNull()) { params.stereoFile->moveToRight((GLfloat )theValue); }
    }

    ST_LOCAL void doParamsPanRight(const double theValue) {
        if(!params.stereoFile.isNull()) { params.stereoFile->moveToLeft((GLfloat )theValue); }
    }

    ST_LOCAL void doParamsPanUp(const double theValue) {
        if(!params.stereoFile.isNull()) { params.stereoFile->moveToDown((GLfloat )theValue); }
    }

    ST_LOCAL void doParamsPanDown(const double theValue) {
        if(!params.stereoFile.isNull()) { params.stereoFile->moveToUp((GLfloat )theValue); }
    }

    ST_LOCAL void doParamsScaleIn(const double theValue) {
        if(!params.stereoFile.isNull()) { params.stereoFile->scaleIn((GLfloat )theValue); }
    }

    ST_LOCAL void doParamsScaleOut(const double theValue) {
        if(!params.stereoFile.isNull()) { params.stereoFile->scaleOut((GLfloat )theValue); }
    }

    ST_LOCAL void doParamsRotYLeft (const double theValue);
    ST_LOCAL void doParamsRotYRight(const double theValue);
    ST_LOCAL void doParamsRotXUp   (const double theValue);
    ST_LOCAL void doParamsRotXDown (const double theValue);

        private: //! @name private methods


    ST_LOCAL StGLVec2 getMouseMoveFlat(const StPointD_t& theCursorZoFrom,
                                       const StPointD_t& theCursorZoTo) const;
    ST_LOCAL StGLVec2 getMouseMoveSphere(const StPointD_t& theCursorZoFrom,
                                         const StPointD_t& theCursorZoTo) const;
    ST_LOCAL StGLVec2 getMouseMoveSphere() const;

    ST_LOCAL void scaleAt(const StPointD_t& thePoint,
                          const float       theStep);

    ST_LOCAL void doRightUnclick(const StPointD_t& theCursorZo);

    ST_LOCAL bool stglInitCube(const StGLVec4& theClampUV = StGLVec4(0.0f, 0.0f, 1.0f, 1.0f),
                               const StPanorama thePano = StPanorama_OFF);
    ST_LOCAL void stglDrawView(unsigned int theView);

        private: //! @name private fields

    StArrayList< StHandle<StAction> >
                               myActions;        //!< actions list
    StHandle<StPlayList>       myList;           //!< handle to playlist

    StGLIcon*                  myIconPrev;       //!< icon displayed on swipe gesture
    StGLIcon*                  myIconNext;       //!< icon displayed on swipe gesture

    StGLQuads                  myQuad;           //!< flat quad
    StGLMesh                   myCube;           //!< cube for drawing cubemap
    StGLVec4                   myCubeClamp;      //!< cubemap clamping vector
    StPanorama                 myCubePano;       //!< cubemap panorama format
    StGLUVSphere               myUVSphere;       //!< sphere mesh object
    StGLUVSphere               myHemisphere;     //!< hemisphere mesh object
    StGLUVCylinder             myCylinder;       //!< cylinder mesh object
    StGLUVCylinder             myTheater;        //!< theater cylinder mesh object
    StGLProjCamera             myProjCam;        //!< copy of projection camera
    StGLImageProgram           myProgram;        //!< GL program to draw flat image
    StHandle<StGLTextureQueue> myTextureQueue;   //!< shared texture queue
    StPointD_t                 myClickPntZo;     //!< remembered mouse click position
    StTimer                    myClickTimer;     //!< timer to delay dragging action
    StTimer                    myFadeTimer;      //!< timer for transition to the next file
    StPointD_t                 myFadeFrom;       //!< fade starting delta
    StGLQuaternion             myDeviceQuat;     //!< device orientation
    StVirtFlags                myKeyFlags;       //!< active key flags
    double                     myDragDelayMs;    //!< dragging delay in milliseconds
    double                     myDragDelayTmpMs; //!< temporary dragging delay
    StVec2<int>                myFrameSize;      //!< frame dimensions
    float                      mySampleRatio;    //!< sample aspect ratio
    float                      myRotAngle;       //!< rotation angle gesture progress
    bool                       myIsClickAborted;
    bool                       myToRightRotate;
    bool                       myIsInitialized;  //!< initialization state
    bool                       myHasVideoStream; //!< should be initialized for each new stream

};

#endif //__StGLImageRegion_h_
