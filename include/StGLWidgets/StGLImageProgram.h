/**
 * Copyright © 2010-2012 Kirill Gavrilov <kirill@sview.ru>
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

class ST_LOCAL StGLImageProgram : public StGLMeshProgram {

        public:

    typedef enum tagTextureFilter {
        FILTER_NEAREST, //!< ugly filter
        FILTER_LINEAR,  //!< linear filter
        FILTER_BLEND,   //!< blend deinterlace filter
    } TextureFilter;

        public:

    StGLImageProgram(const StString& theTitle);

    virtual ~StGLImageProgram();

    virtual void release(StGLContext& theCtx);

    /**
     * Setup color model conversion
     */
    void setupSrcColorShader(StGLContext&           theCtx,
                             StImage::ImgColorModel theColorModel);

    void setTextureSizePx(StGLContext&    theCtx,
                          const StGLVec2& theVec2);

    void setTextureMainDataSize(StGLContext&    theCtx,
                                const StGLVec4& theTexDataVec4);

    void setTextureUVDataSize(StGLContext&    theCtx,
                              const StGLVec4& theTexDataVec4);

    void setColorScale(StGLContext&    theCtx,
                       const StGLVec3& theScale);
    void resetColorScale(StGLContext& theCtx);

    void setContext(const StHandle<StGLContext>& theCtx);

    /**
     * Initialize default shaders, nothing more.
     */
    virtual bool init(StGLContext& theCtx);

    /**
     * Perform (re)link and search variables locations.
     */
    virtual bool link(StGLContext& theCtx);

        public: //!< Properties

    struct {

        StHandle<StFloat32Param> gamma;      //!< gamma correction coefficient
        StHandle<StFloat32Param> brightness; //!< brightness level
        StHandle<StFloat32Param> saturation; //!< saturation value

    } params;

        private: //!< callback Slots

    void doGammaChanged(const float );
    void doSetupCorrectionShader(const float );

        protected:

    void setCorrectionUniform(StGLContext& theCtx);
    void setGammaUniform(StGLContext& theCtx);

    bool isNoColorScale() const;

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
    StGLFragmentShader  fYUV2RGB;
    StGLFragmentShader  fYUV2RGBjpeg;
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
