/**
 * StGLWidgets, small C++ toolkit for writing GUI using OpenGL.
 * Copyright © 2010-2015 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#include <StGLWidgets/StGLImageProgram.h>
#include <StGL/StGLResources.h>
#include <StGL/StGLContext.h>
#include <StGLCore/StGLCore20.h>
#include <StFile/StRawFile.h>

namespace {
    const char F_DEF_2D[] =
        "#define stSampler sampler2D\n"
        "#define stTexture(theSampler, theCoords) texture2D(theSampler, theCoords.xy)\n";
    const char F_DEF_CUBEMAP[] =
        "#define stSampler samplerCube\n"
        "#define stTexture(theSampler, theCoords) textureCube(theSampler, theCoords)\n";
}

void StGLImageProgram::regToRgb(const int       thePartIndex,
                                const StString& theText) {
  registerFragmentShaderPart(FragSection_ToRgb, thePartIndex,
                             StString(F_DEF_2D)      + theText);
  registerFragmentShaderPart(FragSection_ToRgb, FragToRgb_CUBEMAP + thePartIndex,
                             StString(F_DEF_CUBEMAP) + theText);
}

StGLImageProgram::StGLImageProgram()
: myColorScale(1.0f, 1.0f, 1.0f) {
    myTitle = "StGLImageProgram";

    const char F_SHADER_GET_COLOR_BLEND[] =
       "uniform sampler2D uTexture;\n"
       "uniform vec4 uTexData;\n"
       "uniform vec2 uTexelSize;\n"

       "vec4 getColor(in vec3 texCoord) {\n"
       "    if(texCoord.y < (uTexData.y + uTexelSize.y)) {\n"
       "        return texture2D(uTexture, texCoord.xy);\n"
       "    }\n"
       "    return mix(texture2D(uTexture, texCoord.xy - vec2(0.0, uTexelSize.y)),\n"
       "               texture2D(uTexture, texCoord.xy), 0.5);\n"
       "}\n\n";

    registerFragmentShaderPart(FragSection_GetColor, FragGetColor_Normal,
        "uniform sampler2D uTexture;\n"
        "vec4 getColor(in vec3 texCoord) {\n"
        "    return texture2D(uTexture, texCoord.xy);\n"
        "}\n\n"
    );

    registerFragmentShaderPart(FragSection_GetColor, FragGetColor_Blend, F_SHADER_GET_COLOR_BLEND);

    registerFragmentShaderPart(FragSection_GetColor, FragGetColor_Cubemap,
        "uniform samplerCube uTexture;\n"
        "vec4 getColor(in vec3 texCoord) {\n"
        "    return textureCube(uTexture, texCoord);\n"
        "}\n\n"
    );

    registerFragmentShaderPart(FragSection_Correct, FragCorrect_Off,
        "void applyCorrection(inout vec4 color) {}\n\n");
    registerFragmentShaderPart(FragSection_Correct, FragCorrect_On,
        "uniform mat4 uColorProcessing;\n"
        "void applyCorrection(inout vec4 color) {\n"
        "    color = uColorProcessing * color;\n"
        "}\n\n");

    registerFragmentShaderPart(FragSection_Gamma, FragGamma_Off,
        "void applyGamma(inout vec4 color) {}\n\n");
    registerFragmentShaderPart(FragSection_Gamma, FragGamma_On,
        "uniform vec4 uGamma;\n"
        "void applyGamma(inout vec4 color) {\n"
        "    color = pow(color, uGamma);\n"
        "}\n\n");

    registerFragmentShaderPart(FragSection_ToRgb, FragToRgb_FromRgb,
        "void convertToRGB(inout vec4 color, in vec3 texCoord) {}\n\n");

    registerFragmentShaderPart(FragSection_ToRgb, FragToRgb_FromRgba,
        "void convertToRGB(inout vec4 color, in vec3 texCoord) {\n"
        "    vec4 backColor;\n"
        "    bool evenX = int(mod(floor(gl_FragCoord.x + 1.5), 16.0)) >= 8;\n" // just simple 8 pixels check-board
        "    bool evenY = int(mod(floor(gl_FragCoord.y + 1.5), 16.0)) >= 8;\n"
        "    if((evenX && evenY) || (!evenX && !evenY)) {\n"
        "        backColor = vec4(0.4, 0.4, 0.4, 1.0);\n"
        "    } else {\n"
        "        backColor = vec4(0.6, 0.6, 0.6, 1.0);\n"
        "    }\n"
        "    color = mix(backColor, color, color.a);\n"
        "}\n\n");
    registerFragmentShaderPart(FragSection_ToRgb, FragToRgb_FromGray,
        "void convertToRGB(inout vec4 color, in vec3 texCoord) {\n"
        "    color.r = color.a;\n" // gray scale stored in alpha
        "    color.g = color.a;\n"
        "    color.b = color.a;\n"
        "}\n\n");

    // color conversion shaders
    const char F_SHADER_YUV2RGB_MPEG[] =
       "uniform stSampler uTextureU;\n"
       "uniform stSampler uTextureV;\n"
       "void convertToRGB(inout vec4 color, in vec3 texCoordUV) {\n"
       "    vec3 colorYUV = vec3(color.a, stTexture(uTextureU, texCoordUV).a, stTexture(uTextureV, texCoordUV).a);\n"
       "    colorYUV   *= TheRangeBits;\n"
       "    colorYUV.x  = 1.1643 * (colorYUV.x - 0.0625);\n"
       "    colorYUV.y -= 0.5;\n"
       "    colorYUV.z -= 0.5;\n"
       "    color.r = colorYUV.x +  1.5958 * colorYUV.z;\n"
       "    color.g = colorYUV.x - 0.39173 * colorYUV.y - 0.81290 * colorYUV.z;\n"
       "    color.b = colorYUV.x +   2.017 * colorYUV.y;\n"
       "}\n\n";

    const char F_SHADER_YUV2RGB_FULL[] =
       "uniform stSampler uTextureU;\n"
       "uniform stSampler uTextureV;\n"
       "void convertToRGB(inout vec4 color, in vec3 texCoordUV) {\n"
       "    vec3 colorYUV = vec3(color.a, stTexture(uTextureU, texCoordUV).a, stTexture(uTextureV, texCoordUV).a);\n"
       "    colorYUV   *= TheRangeBits;\n"
       "    colorYUV.x  = colorYUV.x;\n"
       "    colorYUV.y -= 0.5;\n"
       "    colorYUV.z -= 0.5;\n"
       "    color.r = colorYUV.x + 1.402 * colorYUV.z;\n"
       "    color.g = colorYUV.x - 0.344 * colorYUV.y - 0.714 * colorYUV.z;\n"
       "    color.b = colorYUV.x + 1.772 * colorYUV.y;\n"
       "}\n\n";

    regToRgb(FragToRgb_FromYuvFull, StString()
        + "const float TheRangeBits = 1.0;\n"
        + F_SHADER_YUV2RGB_FULL);

    regToRgb(FragToRgb_FromYuvMpeg, StString()
        + "const float TheRangeBits = 1.0;\n"
        + F_SHADER_YUV2RGB_MPEG);

    regToRgb(FragToRgb_FromYuv9Full, StString()
        + "const float TheRangeBits = 65535.0 / 511.0;\n"
        + F_SHADER_YUV2RGB_FULL);

    regToRgb(FragToRgb_FromYuv9Mpeg, StString()
        + "const float TheRangeBits = 65535.0 / 511.0;\n"
        + F_SHADER_YUV2RGB_MPEG);

    regToRgb(FragToRgb_FromYuv10Full, StString()
        + "const float TheRangeBits = 65535.0 / 1023.0;\n"
        + F_SHADER_YUV2RGB_FULL);

    regToRgb(FragToRgb_FromYuv10Mpeg, StString()
        + "const float TheRangeBits = 65535.0 / 1023.0;\n"
        + F_SHADER_YUV2RGB_MPEG);

    params.gamma = new StFloat32Param(   1.0f,         // initial value
                                        0.05f, 99.0f, // min, max values
                                         1.0f,         // default value
                                        0.05f,         // incremental step
                                      0.0001f);        // equality tolerance
    params.brightness = new StFloat32Param(   1.0f,         // initial value
                                              0.0f, 99.0f, // min, max values
                                              1.0f,         // default value
                                             0.05f,         // incremental step
                                           0.0001f);        // equality tolerance
    params.saturation = new StFloat32Param(   1.0f,         // initial value
                                            -10.0f, 99.0f, // min, max values
                                              1.0f,         // default value
                                             0.05f,         // incremental step
                                           0.0001f);        // equality tolerance

    // main shader parts
    const char V_SHADER_FLAT[] =
       "uniform mat4 uProjMat;\n"
       "uniform mat4 uModelMat;\n"
       "uniform vec4 uTexData;\n"
       "uniform vec4 uTexUVData;\n"

       "attribute vec4 vVertex;\n"
       "attribute vec2 vTexCoord;\n"

       "varying vec3 fTexCoord;\n"
       "varying vec3 fTexUVCoord;\n"

       "void main(void) {\n"
       "    fTexCoord   = vec3(uTexData.xy   + vTexCoord * uTexData.zw,   0.0);\n"
       "    fTexUVCoord = vec3(uTexUVData.xy + vTexCoord * uTexUVData.zw, 0.0);\n"
       "    gl_Position = uProjMat * uModelMat * vVertex;\n"
       "}\n";

    const char V_SHADER_CUBEMAP[] =
       "uniform mat4 uProjMat;\n"
       "uniform mat4 uModelMat;\n"
       "uniform vec4 uTexData;\n"
       "uniform vec4 uTexUVData;\n"

       "attribute vec4 vVertex;\n"
       "attribute vec2 vTexCoord;\n"

       "varying vec3 fTexCoord;\n"
       "varying vec3 fTexUVCoord;\n"

       "void main(void) {\n"
       "    gl_Position = vec4(vVertex.x, vVertex.y, 0.0, 1.0);\n"
       "    fTexCoord   = (uProjMat * gl_Position).xyz;"
       "    fTexUVCoord = (uProjMat * gl_Position).xyz;"
       "}\n";

    const char F_SHADER_FLAT[] =
       "varying vec3 fTexCoord;\n"
       "varying vec3 fTexUVCoord;\n"
        // we split these functions for two reasons:
        // - to change function code (like color conversion);
        // - to optimize rendering on old hardware not supported conditions (GeForce FX for example).
       "vec4 getColor(in vec3 texCoord);\n"
       "void convertToRGB(inout vec4 theColor, in vec3 theTexUVCoord);\n"
       "void applyCorrection(inout vec4 theColor);\n"
       "void applyGamma(inout vec4 theColor);\n"

       "void main(void) {\n"
            // extract color from main texture
       "    vec4 aColor = getColor(fTexCoord);\n"
            // convert from alien color model (like YUV) to RGB
       "    convertToRGB(aColor, fTexUVCoord);\n"
            // color processing (saturation, brightness, etc)
       "    applyCorrection(aColor);\n"
            // gamma correction
       "    applyGamma(aColor);\n"
       "    gl_FragColor = aColor;\n"
       "}";

    registerVertexShaderPart  (0, VertMain_Normal,  V_SHADER_FLAT);
    registerVertexShaderPart  (0, VertMain_Cubemap, V_SHADER_CUBEMAP);
    registerFragmentShaderPart(FragSection_Main, 0, F_SHADER_FLAT);
}

StGLImageProgram::~StGLImageProgram() {
    //
}

void StGLImageProgram::setTextureSizePx(StGLContext&    theCtx,
                                        const StGLVec2& theVec2) {
    theCtx.core20fwd->glUniform2fv(uniTexSizePxLoc, 1, theVec2);
    theCtx.core20fwd->glUniform2f(uniTexelSizePxLoc, 1.0f / theVec2.x(), 1.0f / theVec2.y());
}

void StGLImageProgram::setTextureMainDataSize(StGLContext&    theCtx,
                                              const StGLVec4& theTexDataVec4) {
    theCtx.core20fwd->glUniform4fv(uniTexMainDataLoc, 1, theTexDataVec4);
}

void StGLImageProgram::setTextureUVDataSize(StGLContext&    theCtx,
                                            const StGLVec4& theTexDataVec4) {
    theCtx.core20fwd->glUniform4fv(uniTexUVDataLoc, 1, theTexDataVec4);
}

void StGLImageProgram::setupCorrection(StGLContext& theCtx) {
    if(getFragmentShaderPart(FragSection_Correct) == FragCorrect_Off) {
        return;
    }

    StGLBrightnessMatrix aBrightness;
    StGLSaturationMatrix aSaturation;
    aBrightness.setBrightness(params.brightness->getValue());
    aSaturation.setSaturation(params.saturation->getValue());
    StGLMatrix aColorMat = StGLMatrix::multiply(aSaturation, aBrightness);
    aColorMat.scale(myColorScale.r(), myColorScale.g(), myColorScale.b());
    theCtx.core20fwd->glUniformMatrix4fv(uniColorProcessingLoc, 1, GL_FALSE, aColorMat);
}

/**
 * Return color conversion shader part from StImage definition.
 */
static inline StGLImageProgram::FragToRgb getColorShader(const StImage::ImgColorModel theColorModel,
                                                         const StImage::ImgColorScale theColorScale) {
    switch(theColorModel) {
        case StImage::ImgColor_RGB:  return StGLImageProgram::FragToRgb_FromRgb;
        case StImage::ImgColor_RGBA: return StGLImageProgram::FragToRgb_FromRgba;
        case StImage::ImgColor_GRAY: return StGLImageProgram::FragToRgb_FromGray;
        case StImage::ImgColor_YUV: {
            switch(theColorScale) {
                case StImage::ImgScale_Mpeg9:  return StGLImageProgram::FragToRgb_FromYuv9Mpeg;
                case StImage::ImgScale_Mpeg10: return StGLImageProgram::FragToRgb_FromYuv10Mpeg;
                case StImage::ImgScale_Jpeg9:  return StGLImageProgram::FragToRgb_FromYuv9Full;
                case StImage::ImgScale_Jpeg10: return StGLImageProgram::FragToRgb_FromYuv10Full;
                case StImage::ImgScale_Mpeg:   return StGLImageProgram::FragToRgb_FromYuvMpeg;
                case StImage::ImgScale_Full:   return StGLImageProgram::FragToRgb_FromYuvFull;
            }
            return StGLImageProgram::FragToRgb_FromYuvFull;
        }
        default: {
            ST_DEBUG_LOG("No GLSL shader for this color model = " + theColorModel);
            ST_DEBUG_ASSERT(false);
        }
    }
    return StGLImageProgram::FragToRgb_FromRgb;
}

bool StGLImageProgram::init(StGLContext&                 theCtx,
                            const StImage::ImgColorModel theColorModel,
                            const StImage::ImgColorScale theColorScale,
                            const FragGetColor           theFilter) {

    // re-configure shader parts when required
    bool isChanged = myActiveProgram.isNull();
    isChanged = setFragmentShaderPart(theCtx, FragSection_Main,    0) || isChanged;
    isChanged = setFragmentShaderPart(theCtx, FragSection_Gamma,
                                      stAreEqual(params.gamma->getValue(), 1.0f, 0.0001f) ? FragGamma_Off : FragGamma_On) || isChanged;
    isChanged = setFragmentShaderPart(theCtx, FragSection_Correct,
                                      params.brightness->isDefaultValue()
                                   && params.saturation->isDefaultValue()
                                   && hasNoColorScale() ? FragCorrect_Off : FragCorrect_On) || isChanged;
    int aToRgb = getColorShader(theColorModel, theColorScale);
    if(aToRgb >= FragToRgb_FromYuvFull
    && theFilter == FragGetColor_Cubemap) {
        aToRgb += FragToRgb_CUBEMAP;
    }

    isChanged = setFragmentShaderPart(theCtx, FragSection_ToRgb,    aToRgb) || isChanged;
    isChanged = setFragmentShaderPart(theCtx, FragSection_GetColor, theFilter) || isChanged;
    isChanged = setVertexShaderPart  (theCtx, 0, theFilter == FragGetColor_Cubemap ? VertMain_Cubemap : VertMain_Normal) || isChanged;
    if(isChanged) {
        if(!initProgram(theCtx)) {
            return false;
        }

        myActiveProgram->uniProjMatLoc  = myActiveProgram->getUniformLocation(theCtx, "uProjMat");
        myActiveProgram->uniModelMatLoc = myActiveProgram->getUniformLocation(theCtx, "uModelMat");
        uniTexMainDataLoc     = myActiveProgram->getUniformLocation(theCtx, "uTexData");
        uniTexUVDataLoc       = myActiveProgram->getUniformLocation(theCtx, "uTexUVData");
        uniTexSizePxLoc       = myActiveProgram->getUniformLocation(theCtx, "uTexSizePx");
        uniTexelSizePxLoc     = myActiveProgram->getUniformLocation(theCtx, "uTexelSize");
        uniColorProcessingLoc = myActiveProgram->getUniformLocation(theCtx, "uColorProcessing");
        uniGammaLoc           = myActiveProgram->getUniformLocation(theCtx, "uGamma");
        myActiveProgram->atrVVertexLoc  = myActiveProgram->getAttribLocation(theCtx, "vVertex");
        myActiveProgram->atrVTCoordLoc  = myActiveProgram->getAttribLocation(theCtx, "vTexCoord");

        StGLVarLocation uniTextureLoc  = myActiveProgram->getUniformLocation(theCtx, "uTexture");
        StGLVarLocation uniTextureULoc = myActiveProgram->getUniformLocation(theCtx, "uTextureU");
        StGLVarLocation uniTextureVLoc = myActiveProgram->getUniformLocation(theCtx, "uTextureV");
        myActiveProgram->use(theCtx);
        theCtx.core20fwd->glUniform1i(uniTextureLoc,  StGLProgram::TEXTURE_SAMPLE_0);
        theCtx.core20fwd->glUniform1i(uniTextureULoc, StGLProgram::TEXTURE_SAMPLE_1);
        theCtx.core20fwd->glUniform1i(uniTextureVLoc, StGLProgram::TEXTURE_SAMPLE_2);
        myActiveProgram->unuse(theCtx);

        /*if (!uniModelMatLoc.isValid()
         || !uniTexMainDataLoc.isValid()
        //|| !uniTexSizePxLoc.isValid()
        //|| !uniTexelSizePxLoc.isValid()
        //|| !uniSmoothFilterLoc.isValid()
        //|| !uniColorProcessingLoc
         || !atrVVertexLoc.isValid()
         || !atrVTCoordLoc.isValid()
         || !uniTextureLoc.isValid()) {
            return false;
        }*/
    }
    if(!isValid()) {
        return false;
    }

    myActiveProgram->use(theCtx);
    if(getFragmentShaderPart(FragSection_Gamma) != FragGamma_Off) {
        GLfloat aReversed = 1.0f / params.gamma->getValue();
        StGLVec4 aVec(aReversed, aReversed, aReversed, 1.0f);
        theCtx.core20fwd->glUniform4fv(uniGammaLoc, 1, aVec);
    }
    setupCorrection(theCtx);
    myActiveProgram->unuse(theCtx);

    const StGLResources aShaders("StGLWidgets");
    return true;
}
