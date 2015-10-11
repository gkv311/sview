/**
 * StGLWidgets, small C++ toolkit for writing GUI using OpenGL.
 * Copyright © 2010-2015 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#ifndef __StGLImageRegion_h_
#define __StGLImageRegion_h_

#include <StGLMesh/StGLUVSphere.h>
#include <StGLMesh/StGLQuads.h>

#include <StGLWidgets/StGLWidget.h>
#include <StGLWidgets/StGLImageProgram.h>
#include <StGLStereo/StGLTextureQueue.h>

#include <StGL/StParams.h>

#include <StSettings/StEnumParam.h>

class StAction;

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
    ST_CPPEXPORT virtual void stglUpdate(const StPointD_t& pointZo);
    ST_CPPEXPORT virtual bool stglInit();
    ST_CPPEXPORT virtual void stglDraw(unsigned int theView);
    ST_CPPEXPORT virtual bool tryClick  (const StPointD_t& theCursorZo, const int& theMouseBtn, bool& isItemClicked);
    ST_CPPEXPORT virtual bool tryUnClick(const StPointD_t& theCursorZo, const int& theMouseBtn, bool& isItemUnclicked);
    ST_CPPEXPORT virtual bool doKeyDown (const StKeyEvent& theEvent);
    ST_CPPEXPORT virtual bool doKeyUp   (const StKeyEvent& theEvent);

    /**
     * Auxiliary method to discard frames in the textures queue without bound OpenGL context.
     */
    ST_CPPEXPORT void stglSkipFrames();

        public: //! @name Properties

    struct {

        StHandle<StEnumParam>    displayMode;   //!< StGLImageRegion::DisplayMode    - display mode
        StHandle<StInt32Param>   displayRatio;  //!< StGLImageRegion::DisplayRatio   - display ratio
        StHandle<StBoolParam>    ToHealAnamorphicRatio; //!< correct aspect ratio for 1080p/720p anamorphic pairs
        StHandle<StInt32Param>   textureFilter; //!< StGLImageProgram::TextureFilter - texture filter;
        StHandle<StFloat32Param> gamma;         //!< gamma correction coefficient
        StHandle<StFloat32Param> brightness;    //!< brightness level
        StHandle<StFloat32Param> saturation;    //!< saturation value

        // per file parameters
        StHandle<StStereoParams> stereoFile;
        StHandle<StBoolParam>    swapLR;        //!< reversion flag
        StHandle<StInt32Param>   ViewMode;      //!< StStereoParams::ViewMode

    } params;

        private:

    ST_LOCAL void doParamsReset(const size_t ) {
        if(!params.stereoFile.isNull()) { params.stereoFile->reset(); }
    }

    ST_LOCAL void doParamsGamma(const size_t theDir) {
        if(!params.stereoFile.isNull()) {
            theDir == 1 ? params.gamma->increment() : params.gamma->decrement();
        }
    }

    ST_LOCAL void doParamsSepX(const size_t theDir) {
        if(!params.stereoFile.isNull()) {
            theDir == 1 ? params.stereoFile->incSeparationDx() : params.stereoFile->decSeparationDx();
        }
    }

    ST_LOCAL void doParamsSepY(const size_t theDir) {
        if(!params.stereoFile.isNull()) {
            theDir == 1 ? params.stereoFile->incSeparationDy() : params.stereoFile->decSeparationDy();
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
        if(!params.stereoFile.isNull()) {
            params.stereoFile->decSepRotation((GLfloat )theValue);
        }
    }

    ST_LOCAL void doParamsSepZInc(const double theValue) {
        if(!params.stereoFile.isNull()) {
            params.stereoFile->incSepRotation((GLfloat )theValue);
        }
    }

    ST_LOCAL void doParamsModeNext(const size_t ) {
        const int aMode = params.ViewMode->getValue();
        switch(aMode) {
            case StStereoParams::FLAT_IMAGE: {
                params.ViewMode->setValue(StStereoParams::PANORAMA_SPHERE);
                return;
            }
            case StStereoParams::PANORAMA_CUBEMAP: {
                params.ViewMode->setValue(StStereoParams::FLAT_IMAGE);
                return;
            }
            case StStereoParams::PANORAMA_SPHERE: {
                params.ViewMode->setValue(StStereoParams::PANORAMA_CUBEMAP);
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
                                       const StPointD_t& theCursorZoTo);
    ST_LOCAL StGLVec2 getMouseMoveSphere(const StPointD_t& theCursorZoFrom,
                                         const StPointD_t& theCursorZoTo);
    ST_LOCAL StGLVec2 getMouseMoveSphere();

    ST_LOCAL void doRightUnclick(const StPointD_t& theCursorZo);

    ST_LOCAL void stglDrawView(unsigned int theView);

        private: //! @name private fields

    StArrayList< StHandle<StAction> >
                               myActions;        //!< actions list

    StGLQuads                  myQuad;           //!< flat quad
    StGLUVSphere               myUVSphere;       //!< sphere output helper class
    StGLImageProgram           myProgram;        //!< GL program to draw flat image
    StHandle<StGLTextureQueue> myTextureQueue;   //!< shared texture queue
    StPointD_t                 myClickPntZo;     //!< remembered mouse click position
    StTimer                    myClickTimer;     //!< timer to delay dragging action
    StGLQuaternion             myDeviceQuat;     //!< device orientation
    StVirtFlags                myKeyFlags;       //!< active key flags
    double                     myDragDelayMs;    //!< dragging delay in milliseconds
    bool                       myIsClickAborted;
    bool                       myToRightRotate;
    bool                       myIsInitialized;  //!< initialization state
    bool                       myHasVideoStream; //!< should be initialized for each new stream

};

#endif //__StGLImageRegion_h_
