/**
 * Copyright © 2010-2013 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#ifndef __StGLImageRegion_h_
#define __StGLImageRegion_h_

#include <StGLMesh/StGLUVSphere.h>
#include <StGLMesh/StGLQuads.h>

#include "StGLWidget.h"
#include "StGLImageFlatProgram.h"
#include "StGLImageSphereProgram.h"
#include <StGLStereo/StGLTextureQueue.h>
#include <StGL/StParams.h>

#include <StSettings/StParam.h>

class StAction;

class StGLImageRegion : public StGLWidget {

        public:

    typedef enum tagDisplayMode {
        MODE_STEREO,        //!< normal draw
        MODE_ONLY_LEFT,     //!< draw only Left  view
        MODE_ONLY_RIGHT,    //!< draw only Right view
        MODE_PARALLEL,      //!< draw parallel   pair
        MODE_CROSSYED,      //!< draw cross-eyed pair
        MODE_OVER_UNDER_LR, //!< draw Over/Under
        MODE_OVER_UNDER_RL, //!< draw Over/Under
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

        public: //!< public interface

    /**
     * Default constructor.
     */
    ST_CPPEXPORT StGLImageRegion(StGLWidget* theParent,
                                 const StHandle<StGLTextureQueue>& theTextureQueue);

    ST_LOCAL inline StHandle<StGLTextureQueue>& getTextureQueue() {
        return myTextureQueue;
    }

    ST_CPPEXPORT StHandle<StStereoParams> getSource();

    const StArrayList< StHandle<StAction> >& getActions() const {
        return myActions;
    }

    ST_CPPEXPORT virtual ~StGLImageRegion();
    ST_CPPEXPORT virtual const StString& getClassName();
    ST_CPPEXPORT virtual void stglUpdate(const StPointD_t& pointZo);
    ST_CPPEXPORT virtual bool stglInit();
    ST_CPPEXPORT virtual void stglDraw(unsigned int theView);
    ST_CPPEXPORT virtual bool tryClick  (const StPointD_t& theCursorZo, const int& theMouseBtn, bool& isItemClicked);
    ST_CPPEXPORT virtual bool tryUnClick(const StPointD_t& theCursorZo, const int& theMouseBtn, bool& isItemUnclicked);

    ST_CPPEXPORT virtual bool doKeyHold(const StKeyEvent& theEvent);

        public: //! @name Properties

    struct {

        StHandle<StInt32Param>   displayMode;   //!< StGLImageRegion::DisplayMode    - display mode
        StHandle<StInt32Param>   displayRatio;  //!< StGLImageRegion::DisplayRatio   - display ratio
        StHandle<StInt32Param>   textureFilter; //!< StGLImageProgram::TextureFilter - texture filter;
        StHandle<StFloat32Param> gamma;         //!< gamma correction coefficient
        StHandle<StFloat32Param> brightness;    //!< brightness level
        StHandle<StFloat32Param> saturation;    //!< saturation value

        // per file parameters
        StHandle<StStereoParams> stereoFile;
        StHandle<StBoolParam>    swapLR;        //!< reversion flag

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

    ST_LOCAL void doParamsRotZ(const size_t theDir) {
        if(!params.stereoFile.isNull()) {
            theDir == 1 ? params.stereoFile->incZRotate() : params.stereoFile->decZRotate();
        }
    }

    ST_LOCAL void doParamsModeNext(const size_t ) {
        if(!params.stereoFile.isNull()) { params.stereoFile->nextViewMode(); }
    }

        private: //! @name private callback Slots

    ST_LOCAL void doChangeTexFilter(const int32_t theTextureFilter);

        private: //! @name private methods

    ST_LOCAL StGLVec2 getMouseMoveFlat(const StPointD_t& theCursorZoFrom,
                                       const StPointD_t& theCursorZoTo);
    ST_LOCAL StGLVec2 getMouseMoveFlat();
    ST_LOCAL StGLVec2 getMouseMoveSphere(const StPointD_t& theCursorZoFrom,
                                         const StPointD_t& theCursorZoTo);
    ST_LOCAL StGLVec2 getMouseMoveSphere();

    ST_LOCAL void stglDrawView(unsigned int theView);

        private: //! @name private fields

    StArrayList< StHandle<StAction> >
                               myActions;        //!< actions list

    StGLQuads                  myQuad;           //!< flat quad
    StGLUVSphere               myUVSphere;       //!< sphere output helper class
    StGLImageFlatProgram       myProgramFlat;    //!< GL program to draw flat image
    StGLImageSphereProgram     myProgramSphere;  //!< GL program to draw spheric panorama
    StHandle<StGLTextureQueue> myTextureQueue;   //!< shared texture queue
    StPointD_t                 myClickPntZo;     //!< remembered mouse click position
    bool                       myIsInitialized;  //!< initialization state
    bool                       myHasVideoStream; //!< should be initialized for each new stream

};

#endif //__StGLImageRegion_h_
