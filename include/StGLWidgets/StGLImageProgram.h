/**
 * StGLWidgets, small C++ toolkit for writing GUI using OpenGL.
 * Copyright © 2010-2019 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#ifndef __StGLImageProgram_h_
#define __StGLImageProgram_h_

#include <StGL/StGLSaturationMatrix.h>
#include <StGL/StGLBrightnessMatrix.h>
#include <StGL/StGLProgramMatrix.h>
#include <StGLMesh/StGLMesh.h>
#include <StImage/StImage.h>
#include <StSettings/StFloat32Param.h>

/**
 * GLSL program for Image Region widget.
 */
class StGLImageProgram : public StGLProgramMatrix<1, 6, StGLMeshProgram> {

        public:

    typedef enum tagTextureFilter {
        FILTER_NEAREST,   //!< ugly filter
        FILTER_LINEAR,    //!< linear filter
        FILTER_BLEND,     //!< blend deinterlace filter
        FILTER_TRILINEAR, //!< trilinear filter (generate mip-maps)
    } TextureFilter;

    /**
     * Options for GLSL Vertex Shader.
     */
    enum VertMain {
        VertMain_Normal = 0,
        VertMain_Cubemap,
        VertMain_NB
    };

    /**
     * Color conversion options in GLSL Fragment Shader.
     */
    enum FragSection {
        FragSection_Main = 0,     //!< section with main() function
        FragSection_GetColor,     //!< read color values from textures
        FragSection_GetTexCoords, //!< EAC texture coordinates correction
        FragSection_ToRgb,        //!< color conversion
        FragSection_Correct,      //!< color correction
        FragSection_Gamma,        //!< gamma correction
        FragSection_NB
    };

    /**
     * Color getter options in GLSL Fragment Shader.
     */
    enum FragGetColor {
        FragGetColor_Normal = 0,
        FragGetColor_Blend,
        FragGetColor_Cubemap,
        FragGetColor_NB
    };

    /**
     * Color conversion options in GLSL Fragment Shader.
     */
    enum FragToRgb {
        FragToRgb_FromRgb = 0,
        FragToRgb_FromRgba,
        FragToRgb_FromGray,
        FragToRgb_FromXyz,
        FragToRgb_FromYuvFull,
        FragToRgb_FromYuvaFull,
        FragToRgb_FromYuvMpeg,
        FragToRgb_FromYuvaMpeg,
        FragToRgb_FromYuv9Full,
        FragToRgb_FromYuva9Full,
        FragToRgb_FromYuv9Mpeg,
        FragToRgb_FromYuva9Mpeg,
        FragToRgb_FromYuv10Full,
        FragToRgb_FromYuva10Full,
        FragToRgb_FromYuv10Mpeg,
        FragToRgb_FromYuva10Mpeg,
        FragToRgb_FromYuvNvFull,
        FragToRgb_FromYuvNvMpeg,
        FragToRgb_CUBEMAP,
        //FragToRgb_NB = FragToRgb_CUBEMAP * 2
    };

    /**
     * Color correction options in GLSL Fragment Shader.
     */
    enum FragCorrect {
        FragCorrect_Off = 0,
        FragCorrect_On,
        FragCorrect_NB
    };

    /**
     * Gamma correction options in GLSL Fragment Shader.
     */
    enum FragGamma {
        FragGamma_Off = 0,
        FragGamma_On,
        FragGamma_NB
    };

    /**
     * Texture coordinates EAC correction options in GLSL Fragment Shader.
     */
    enum FragTexEAC {
        FragTexEAC_Off = 0,
        FragTexEAC_On,
        FragTexEAC_NB
    };

        public:

    ST_CPPEXPORT StGLImageProgram();

    ST_CPPEXPORT virtual ~StGLImageProgram();

    ST_CPPEXPORT void setTextureSizePx(StGLContext&    theCtx,
                                       const StGLVec2& theVec2);

    ST_CPPEXPORT void setTextureMainDataSize(StGLContext&    theCtx,
                                             const StGLVec4& theTexDataVec4);

    ST_CPPEXPORT void setTextureUVDataSize(StGLContext&    theCtx,
                                           const StGLVec4& theTexDataVec4);

    ST_CPPEXPORT void setTextureADataSize(StGLContext&    theCtx,
                                          const StGLVec4& theTexDataVec4);

    ST_CPPEXPORT void setCubeTextureFlipZ(StGLContext&    theCtx,
                                          bool theToFlip);

    ST_LOCAL void setColorScale(const StGLVec3& theScale) {
        myColorScale = theScale;
    }

    ST_LOCAL void resetColorScale() {
        myColorScale = StGLVec3(1.0f, 1.0f, 1.0f);
    }

    /**
     * Initialize default shaders, nothing more.
     */
    ST_CPPEXPORT bool init(StGLContext&                 theCtx,
                           const StImage::ImgColorModel theColorModel,
                           const StImage::ImgColorScale theColorScale,
                           const FragGetColor           theFilter,
                           const FragTexEAC theTexCoord = FragTexEAC_Off);

        public: //!< Properties

    struct {

        StHandle<StFloat32Param> gamma;      //!< gamma correction coefficient
        StHandle<StFloat32Param> brightness; //!< brightness level
        StHandle<StFloat32Param> saturation; //!< saturation value

    } params;

        private: //!< callback Slots

    ST_LOCAL void setupCorrection(StGLContext& theCtx);
    ST_LOCAL void regToRgb(const StGLContext& theCtx,
                           const int       thePartIndex,
                           const StString& theText);
    ST_LOCAL void registerFragments(const StGLContext& theCtx);

        protected:

    ST_LOCAL bool hasNoColorScale() const {
        return myColorScale.r() > 0.9f
            && myColorScale.g() > 0.9f
            && myColorScale.b() > 0.9f;
    }

        protected:

    StGLVarLocation uniTexMainDataLoc;
    StGLVarLocation uniTexUVDataLoc;
    StGLVarLocation uniTexADataLoc;
    StGLVarLocation uniTexSizePxLoc;
    StGLVarLocation uniTexelSizePxLoc;
    StGLVarLocation uniTexCubeFlipZLoc;
    StGLVarLocation uniColorProcessingLoc;
    StGLVarLocation uniGammaLoc;

    StGLVec3        myColorScale; //!< scale filter for de-anaglyph processing
    bool            myIsRegistered;

};

#endif //__StGLImageProgram_h_
