/**
 * Copyright © 2010-2013 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#ifndef __StGLImageProgram_h_
#define __StGLImageProgram_h_

#include <StGL/StGLSaturationMatrix.h>
#include <StGL/StGLBrightnessMatrix.h>
#include <StGLMesh/StGLMesh.h>
#include <StImage/StImage.h>
#include <StSettings/StFloat32Param.h>

class StGLImageProgram : public StGLMeshProgram {

        public:

    typedef enum tagTextureFilter {
        FILTER_NEAREST, //!< ugly filter
        FILTER_LINEAR,  //!< linear filter
        FILTER_BLEND,   //!< blend deinterlace filter
    } TextureFilter;

        public:

    ST_CPPEXPORT StGLImageProgram(const StString& theTitle);

    ST_CPPEXPORT virtual ~StGLImageProgram();

    ST_CPPEXPORT virtual void release(StGLContext& theCtx);

    /**
     * Setup color model conversion
     */
    ST_CPPEXPORT void setupSrcColorShader(StGLContext&                 theCtx,
                                          const StImage::ImgColorModel theColorModel,
                                          const StImage::ImgColorScale theColorScale);

    ST_CPPEXPORT void setTextureSizePx(StGLContext&    theCtx,
                                       const StGLVec2& theVec2);

    ST_CPPEXPORT void setTextureMainDataSize(StGLContext&    theCtx,
                                             const StGLVec4& theTexDataVec4);

    ST_CPPEXPORT void setTextureUVDataSize(StGLContext&    theCtx,
                                           const StGLVec4& theTexDataVec4);

    ST_CPPEXPORT void setColorScale(StGLContext&    theCtx,
                                    const StGLVec3& theScale);
    ST_CPPEXPORT void resetColorScale(StGLContext& theCtx);

    ST_CPPEXPORT void setContext(const StHandle<StGLContext>& theCtx);

    /**
     * Initialize default shaders, nothing more.
     */
    ST_CPPEXPORT virtual bool init(StGLContext& theCtx);

    /**
     * Perform (re)link and search variables locations.
     */
    ST_CPPEXPORT virtual bool link(StGLContext& theCtx);

        public: //!< Properties

    struct {

        StHandle<StFloat32Param> gamma;      //!< gamma correction coefficient
        StHandle<StFloat32Param> brightness; //!< brightness level
        StHandle<StFloat32Param> saturation; //!< saturation value

    } params;

        private: //!< callback Slots

    ST_LOCAL void doGammaChanged(const float );
    ST_LOCAL void doSetupCorrectionShader(const float );

        protected:

    ST_CPPEXPORT void setCorrectionUniform(StGLContext& theCtx);
    ST_CPPEXPORT void setGammaUniform(StGLContext& theCtx);

    ST_CPPEXPORT bool isNoColorScale() const;

        protected:

    StHandle<StGLContext> myContext; //!< hold handle to GL context for callback

    StGLVarLocation uniTexMainDataLoc;
    StGLVarLocation uniTexUVDataLoc;
    StGLVarLocation uniTexSizePxLoc;
    StGLVarLocation uniTexelSizePxLoc;
    StGLVarLocation uniColorProcessingLoc;
    StGLVarLocation uniGammaLoc;

    StGLFragmentShader  fGetColor;
    StGLFragmentShader  fGetColorBlend;

    // color conversion
    StGLFragmentShader* f2RGBPtr;
    StGLFragmentShader  fRGB2RGB;
    StGLFragmentShader  fRGBA2RGB;
    StGLFragmentShader  fGray2RGB;
    StGLFragmentShader  fYUVtoRGB_full;
    StGLFragmentShader  fYUVtoRGB_mpeg;
    StGLFragmentShader  fYUV9toRGB_full;
    StGLFragmentShader  fYUV9toRGB_mpeg;
    StGLFragmentShader  fYUV10toRGB_full;
    StGLFragmentShader  fYUV10toRGB_mpeg;
    // color correction
    StGLFragmentShader* fCorrectPtr;
    StGLFragmentShader  fCorrectNO;
    StGLFragmentShader  fCorrectON;
    StGLBrightnessMatrix myBrightness;
    StGLSaturationMatrix mySaturation;
    StGLVec3             myColorScale; //!< scale filter for de-anaglyph processing
    // gamma correction
    StGLFragmentShader* fGammaPtr;
    StGLFragmentShader  fGammaNO;
    StGLFragmentShader  fGammaON;

};

#endif //__StGLImageProgram_h_
