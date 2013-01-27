/**
 * Copyright © 2010-2012 Kirill Gavrilov <kirill@sview.ru>
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

class ST_LOCAL StGLImageRegion : public StGLWidget {

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

    StGLImageRegion(StGLWidget* theParent,
                    size_t      theTextureQueueSizeMax);

    StHandle<StGLTextureQueue>& getTextureQueue() {
        return myTextureQueue;
    }

    StHandle<StStereoParams> getSource();

    virtual ~StGLImageRegion();
    virtual const StString& getClassName();
    virtual void stglUpdate(const StPointD_t& pointZo);
    virtual bool stglInit();
    virtual void stglDraw(unsigned int theView);
    virtual bool tryClick  (const StPointD_t& theCursorZo, const int& theMouseBtn, bool& isItemClicked);
    virtual bool tryUnClick(const StPointD_t& theCursorZo, const int& theMouseBtn, bool& isItemUnclicked);

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

        private: //! @name private callback Slots

    void doChangeTexFilter(const int32_t theTextureFilter);

        private: //! @name private methods

    StGLVec2 getMouseMoveFlat(const StPointD_t& theCursorZoFrom,
                              const StPointD_t& theCursorZoTo);
    StGLVec2 getMouseMoveFlat();
    StGLVec2 getMouseMoveSphere(const StPointD_t& theCursorZoFrom,
                                const StPointD_t& theCursorZoTo);
    StGLVec2 getMouseMoveSphere();

    void stglDrawView(unsigned int theView);

        private: //! @name private fields

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
