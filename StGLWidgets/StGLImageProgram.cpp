/**
 * StGLWidgets, small C++ toolkit for writing GUI using OpenGL.
 * Copyright © 2010-2019 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#include <StGLWidgets/StGLImageProgram.h>
#include <StGL/StGLResources.h>
#include <StGL/StGLContext.h>
#include <StGLCore/StGLCore20.h>
#include <StFile/StRawFile.h>

namespace {
    const char F_DEF_2D_ALPHA[] =
        "#define stSampler sampler2D\n"
        "#define stTexture(theSampler, theCoords) texture2D(theSampler, theCoords.xy)\n"
        "#define stAlpha a\n";
    const char F_DEF_2D_RED[] =
        "#define stSampler sampler2D\n"
        "#define stTexture(theSampler, theCoords) texture2D(theSampler, theCoords.xy)\n"
        "#define stAlpha r\n";
    const char F_DEF_CUBEMAP_ALPHA[] =
        "#define stSampler samplerCube\n"
        "#define stTexture(theSampler, theCoords) textureCube(theSampler, theCoords)\n"
        "#define stAlpha a\n";
    const char F_DEF_CUBEMAP_RED[] =
        "#define stSampler samplerCube\n"
        "#define stTexture(theSampler, theCoords) textureCube(theSampler, theCoords)\n"
        "#define stAlpha r\n";
}

void StGLImageProgram::regToRgb(const StGLContext& theCtx,
                                const int       thePartIndex,
                                const StString& theText) {
  registerFragmentShaderPart(FragSection_ToRgb, thePartIndex,
                             StString(theCtx.arbTexRG ? F_DEF_2D_RED : F_DEF_2D_ALPHA) + theText);
  registerFragmentShaderPart(FragSection_ToRgb, FragToRgb_CUBEMAP + thePartIndex,
                             StString(theCtx.arbTexRG ? F_DEF_CUBEMAP_RED : F_DEF_CUBEMAP_ALPHA) + theText);
}

StGLImageProgram::StGLImageProgram()
: myColorScale(1.0f, 1.0f, 1.0f),
  myIsRegistered(false) {
    myTitle = "StGLImageProgram";

    params.gamma = new StFloat32Param(1.0f);
    params.gamma->setMinMaxValues(0.05f, 99.0f);
    params.gamma->setEffectiveMinMaxValues(0.05f, 2.0f);
    params.gamma->setDefValue(1.0f);
    params.gamma->setStep(0.05f);
    params.gamma->setTolerance(0.0001f);

    params.brightness = new StFloat32Param(1.0f);
    params.brightness->setMinMaxValues(0.0f, 99.0f);
    params.brightness->setDefValue(1.0f);
    params.brightness->setEffectiveMinMaxValues(0.0f, 5.0f);
    params.brightness->setStep(0.05f);
    params.brightness->setTolerance(0.0001f);

    params.saturation = new StFloat32Param(1.0f);
    params.saturation->setMinMaxValues(-10.0f, 99.0f);
    params.saturation->setEffectiveMinMaxValues(0.0f, 2.0f);
    params.saturation->setDefValue(1.0f);
    params.saturation->setStep(0.05f);
    params.saturation->setTolerance(0.0001f);
}

void StGLImageProgram::registerFragments(const StGLContext& theCtx) {
    if(myIsRegistered) {
        return;
    }
    myIsRegistered = true;

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

    // equiangular cubemap texture coordinates correction
    registerFragmentShaderPart(FragSection_GetTexCoords, FragTexEAC_Off,
                               "vec3 getTexCoords(in vec3 theCoords, in vec4 theClamp) { return theCoords; }\n\n");
    registerFragmentShaderPart(FragSection_GetTexCoords, FragTexEAC_On,
                               "#define ST_PI 3.1415926535897932384626433832795\n"
                               "vec3 getTexCoords(in vec3 theCoords, in vec4 theClamp) {\n"
                               "    vec3 aCoords = theCoords;\n"
                               "    aCoords.x = 4.0 / ST_PI * atan(theCoords.x);\n"
                               "    aCoords.y = 4.0 / ST_PI * atan(theCoords.y);\n"
                               "    aCoords.z = 4.0 / ST_PI * atan(theCoords.z);\n"
                               "\n"
                               "    float aClampX = abs(theClamp.x) < 1.0 ? theClamp.w * 2.0 : 0.0;\n"
                               "    if(theClamp.x >= 0.0) {\n"
                               "        aCoords.x = -1.0 + theClamp.x * (aCoords.x + 1.0);\n"
                               "        aCoords.x = clamp(aCoords.x, -1.0 + aClampX, 1.0 * theClamp.x);\n"
                               "    } else {\n"
                               "        aCoords.x =  1.0 + theClamp.x * (1.0 - aCoords.x);\n"
                               "        aCoords.x = clamp(aCoords.x, 1.0 * theClamp.x, 1.0 - aClampX);\n"
                               "    }\n"
                               "\n"
                               "    float aClampY = abs(theClamp.y) < 1.0 ? theClamp.w * 2.0 : 0.0;\n"
                               "    if(theClamp.y >= 0.0) {\n"
                               "        aCoords.y = -1.0 + theClamp.y * (aCoords.y + 1.0);\n"
                               "        aCoords.y = clamp(aCoords.y, -1.0 + aClampY, 1.0 * theClamp.y);\n"
                               "    } else {\n"
                               "        aCoords.y =  1.0 + theClamp.y * (1.0 - aCoords.y);\n"
                               "        aCoords.y = clamp(aCoords.y, 1.0 * theClamp.y, 1.0 - aClampY);\n"
                               "    }\n"
                               "\n"
                               "    float aClampZ = abs(theClamp.z) < 1.0 ? theClamp.w * 2.0 : 0.0;\n"
                               "    if(theClamp.z >= 0.0) {\n"
                               "        aCoords.z = -1.0 + theClamp.z * (aCoords.z + 1.0);\n"
                               "        aCoords.z = clamp(aCoords.z, -1.0 + aClampZ, 1.0 * theClamp.z);\n"
                               "    } else {\n"
                               "        aCoords.z =  1.0 + theClamp.z * (1.0 - aCoords.z);\n"
                               "        aCoords.z = clamp(aCoords.z, 1.0 * theClamp.z, 1.0 - aClampZ);\n"
                               "    }\n"
                               "    return aCoords;\n"
                               "}\n\n");

    registerFragmentShaderPart(FragSection_ToRgb, FragToRgb_FromRgb,
        "void convertToRGB(inout vec4 color, in vec3 texCoord, in vec3 texCoordA) {}\n\n");

    const char F_ALPHA_BACKGROUND[] =
        "void drawAlphaBackground(inout vec4 theColor) {\n"
        "    bool anEvenX = int(mod(floor(gl_FragCoord.x + 1.5), 16.0)) >= 8;\n" // just simple 8 pixels check-board
        "    bool anEvenY = int(mod(floor(gl_FragCoord.y + 1.5), 16.0)) >= 8;\n"
        "    vec4 aBackColor = vec4(0.6, 0.6, 0.6, 1.0);\n"
        "    if((anEvenX && anEvenY) || (!anEvenX && !anEvenY)) {\n"
        "        aBackColor = vec4(0.4, 0.4, 0.4, 1.0);\n"
        "    };\n"
        "    theColor = mix(aBackColor, theColor, theColor.a);\n"
        "}\n\n";

    registerFragmentShaderPart(FragSection_ToRgb, FragToRgb_FromRgba, StString()
      + F_ALPHA_BACKGROUND
      + "void convertToRGB(inout vec4 color, in vec3 texCoord, in vec3 texCoordA) {\n"
        "    drawAlphaBackground(color);\n"
        "}\n\n");

    regToRgb(theCtx, FragToRgb_FromGray,
        "void convertToRGB(inout vec4 color, in vec3 texCoord, in vec3 texCoordA) {\n"
        "    color.r = color.stAlpha;\n" // gray scale stored in alpha
        "    color.g = color.stAlpha;\n"
        "    color.b = color.stAlpha;\n"
        "}\n\n");

    // color conversion shaders
    registerFragmentShaderPart(FragSection_ToRgb, FragToRgb_FromXyz,
       // XYZ to sRGB matrix
       "const mat4 THE_XYZ2RGB_MAT = mat4(3.2404542, -0.9692660,  0.0556434, 0.0,"
       "                                 -1.5371385,  1.8760108, -0.2040259, 0.0,"
       "                                 -0.4985314,  0.0415560,  1.0572252, 0.0,"
       "                                        0.0,        0.0,        0.0, 1.0);"
       "const vec4 THE_GAMMA_XYZ =       vec4(2.6, 2.6, 2.6, 1.0);"
       "const vec4 THE_GAMMA_RGB = 1.0 / vec4(2.2, 2.2, 2.2, 1.0);"
       "void convertToRGB(inout vec4 theColor, in vec3 texCoord, in vec3 texCoordA) {\n"
       "    vec4 aColor = pow(theColor, THE_GAMMA_XYZ);"
       "    aColor = THE_XYZ2RGB_MAT * aColor;\n"
       "    aColor = pow(aColor, THE_GAMMA_RGB);"
       "    theColor = aColor;"
       "}\n\n");

    const char F_SHADER_YUV2RGB_MPEG[] =
       "uniform stSampler uTextureU;\n"
       "uniform stSampler uTextureV;\n"
       "void convertToRGB(inout vec4 color, in vec3 texCoordUV, in vec3 texCoordA) {\n"
       "    vec3 colorYUV = vec3(color.stAlpha, stTexture(uTextureU, texCoordUV).stAlpha, stTexture(uTextureV, texCoordUV).stAlpha);\n"
       "    colorYUV   *= TheRangeBits;\n"
       "    colorYUV.x  = 1.1643 * (colorYUV.x - 0.0625);\n"
       "    colorYUV.y -= 0.5;\n"
       "    colorYUV.z -= 0.5;\n"
       "    color.r = colorYUV.x +  1.5958 * colorYUV.z;\n"
       "    color.g = colorYUV.x - 0.39173 * colorYUV.y - 0.81290 * colorYUV.z;\n"
       "    color.b = colorYUV.x +   2.017 * colorYUV.y;\n"
       "}\n\n";

    const char F_SHADER_YUVA2RGB_MPEG[] =
       "uniform stSampler uTextureU;\n"
       "uniform stSampler uTextureV;\n"
       "uniform stSampler uTextureA;\n"
       "void convertToRGB(inout vec4 color, in vec3 texCoordUV, in vec3 texCoordA) {\n"
       "    vec3 colorYUV = vec3(color.stAlpha, stTexture(uTextureU, texCoordUV).stAlpha, stTexture(uTextureV, texCoordUV).stAlpha);\n"
       "    colorYUV   *= TheRangeBits;\n"
       "    colorYUV.x  = 1.1643 * (colorYUV.x - 0.0625);\n"
       "    colorYUV.y -= 0.5;\n"
       "    colorYUV.z -= 0.5;\n"
       "    color.r = colorYUV.x +  1.5958 * colorYUV.z;\n"
       "    color.g = colorYUV.x - 0.39173 * colorYUV.y - 0.81290 * colorYUV.z;\n"
       "    color.b = colorYUV.x +   2.017 * colorYUV.y;\n"
       "    color.a = stTexture(uTextureA, texCoordA).stAlpha;\n" // TODO how to handle TheRangeBits?
       "    drawAlphaBackground(color);\n"
       "}\n\n";

    const char F_SHADER_YUV2RGB_FULL[] =
       "uniform stSampler uTextureU;\n"
       "uniform stSampler uTextureV;\n"
       "void convertToRGB(inout vec4 color, in vec3 texCoordUV, in vec3 texCoordA) {\n"
       "    vec3 colorYUV = vec3(color.stAlpha, stTexture(uTextureU, texCoordUV).stAlpha, stTexture(uTextureV, texCoordUV).stAlpha);\n"
       "    colorYUV   *= TheRangeBits;\n"
       "    colorYUV.x  = colorYUV.x;\n"
       "    colorYUV.y -= 0.5;\n"
       "    colorYUV.z -= 0.5;\n"
       "    color.r = colorYUV.x + 1.402 * colorYUV.z;\n"
       "    color.g = colorYUV.x - 0.344 * colorYUV.y - 0.714 * colorYUV.z;\n"
       "    color.b = colorYUV.x + 1.772 * colorYUV.y;\n"
       "}\n\n";

    const char F_SHADER_YUVA2RGB_FULL[] =
       "uniform stSampler uTextureU;\n"
       "uniform stSampler uTextureV;\n"
       "uniform stSampler uTextureA;\n"
       "void convertToRGB(inout vec4 color, in vec3 texCoordUV, in vec3 texCoordA) {\n"
       "    vec3 colorYUV = vec3(color.stAlpha, stTexture(uTextureU, texCoordUV).stAlpha, stTexture(uTextureV, texCoordUV).stAlpha);\n"
       "    colorYUV   *= TheRangeBits;\n"
       "    colorYUV.x  = colorYUV.x;\n"
       "    colorYUV.y -= 0.5;\n"
       "    colorYUV.z -= 0.5;\n"
       "    color.r = colorYUV.x + 1.402 * colorYUV.z;\n"
       "    color.g = colorYUV.x - 0.344 * colorYUV.y - 0.714 * colorYUV.z;\n"
       "    color.b = colorYUV.x + 1.772 * colorYUV.y;\n"
       "    color.a = stTexture(uTextureA, texCoordA).stAlpha;\n" // TODO how to handle TheRangeBits?
       "    drawAlphaBackground(color);\n"
       "}\n\n";

    const char F_SHADER_YUVNV2RGB_MPEG[] =
       "uniform stSampler uTextureU;\n"
       "void convertToRGB(inout vec4 color, in vec3 texCoordUV, in vec3 texCoordA) {\n"
       "    vec3 colorYUV = vec3(color.stAlpha, stTexture(uTextureU, texCoordUV).r, stTexture(uTextureU, texCoordUV).a);\n"
       "    colorYUV   *= TheRangeBits;\n"
       "    colorYUV.x  = 1.1643 * (colorYUV.x - 0.0625);\n"
       "    colorYUV.y -= 0.5;\n"
       "    colorYUV.z -= 0.5;\n"
       "    color.r = colorYUV.x +  1.5958 * colorYUV.z;\n"
       "    color.g = colorYUV.x - 0.39173 * colorYUV.y - 0.81290 * colorYUV.z;\n"
       "    color.b = colorYUV.x +   2.017 * colorYUV.y;\n"
       "}\n\n";

    const char F_SHADER_YUVNV2RGB_FULL[] =
       "uniform stSampler uTextureU;\n"
       "void convertToRGB(inout vec4 color, in vec3 texCoordUV, in vec3 texCoordA) {\n"
       "    vec3 colorYUV = vec3(color.stAlpha, stTexture(uTextureU, texCoordUV).r, stTexture(uTextureU, texCoordUV).a);\n"
       "    colorYUV   *= TheRangeBits;\n"
       "    colorYUV.x  = colorYUV.x;\n"
       "    colorYUV.y -= 0.5;\n"
       "    colorYUV.z -= 0.5;\n"
       "    color.r = colorYUV.x + 1.402 * colorYUV.z;\n"
       "    color.g = colorYUV.x - 0.344 * colorYUV.y - 0.714 * colorYUV.z;\n"
       "    color.b = colorYUV.x + 1.772 * colorYUV.y;\n"
       "}\n\n";

    regToRgb(theCtx, FragToRgb_FromYuvFull, StString()
        + "const float TheRangeBits = 1.0;\n"
        + F_SHADER_YUV2RGB_FULL);

    regToRgb(theCtx, FragToRgb_FromYuvaFull, StString()
        + "const float TheRangeBits = 1.0;\n"
        + F_ALPHA_BACKGROUND
        + F_SHADER_YUVA2RGB_FULL);

    regToRgb(theCtx, FragToRgb_FromYuvMpeg, StString()
        + "const float TheRangeBits = 1.0;\n"
        + F_SHADER_YUV2RGB_MPEG);

    regToRgb(theCtx, FragToRgb_FromYuvaMpeg, StString()
        + "const float TheRangeBits = 1.0;\n"
        + F_ALPHA_BACKGROUND
        + F_SHADER_YUVA2RGB_MPEG);

    regToRgb(theCtx, FragToRgb_FromYuv9Full, StString()
        + "const float TheRangeBits = 65535.0 / 511.0;\n"
        + F_SHADER_YUV2RGB_FULL);

    regToRgb(theCtx, FragToRgb_FromYuva9Full, StString()
        + "const float TheRangeBits = 65535.0 / 511.0;\n"
        + F_ALPHA_BACKGROUND
        + F_SHADER_YUVA2RGB_FULL);

    regToRgb(theCtx, FragToRgb_FromYuv9Mpeg, StString()
        + "const float TheRangeBits = 65535.0 / 511.0;\n"
        + F_SHADER_YUV2RGB_MPEG);

    regToRgb(theCtx, FragToRgb_FromYuva9Mpeg, StString()
        + "const float TheRangeBits = 65535.0 / 511.0;\n"
        + F_ALPHA_BACKGROUND
        + F_SHADER_YUVA2RGB_MPEG);

    regToRgb(theCtx, FragToRgb_FromYuv10Full, StString()
        + "const float TheRangeBits = 65535.0 / 1023.0;\n"
        + F_SHADER_YUV2RGB_FULL);

    regToRgb(theCtx, FragToRgb_FromYuva10Full, StString()
        + "const float TheRangeBits = 65535.0 / 1023.0;\n"
        + F_ALPHA_BACKGROUND
        + F_SHADER_YUVA2RGB_FULL);

    regToRgb(theCtx, FragToRgb_FromYuv10Mpeg, StString()
        + "const float TheRangeBits = 65535.0 / 1023.0;\n"
        + F_SHADER_YUV2RGB_MPEG);

    regToRgb(theCtx, FragToRgb_FromYuva10Mpeg, StString()
        + "const float TheRangeBits = 65535.0 / 1023.0;\n"
        + F_ALPHA_BACKGROUND
        + F_SHADER_YUVA2RGB_MPEG);

    regToRgb(theCtx, FragToRgb_FromYuvNvFull, StString()
        + "const float TheRangeBits = 1.0;\n"
        + F_SHADER_YUVNV2RGB_FULL);

    regToRgb(theCtx, FragToRgb_FromYuvNvMpeg, StString()
        + "const float TheRangeBits = 1.0;\n"
        + F_SHADER_YUVNV2RGB_MPEG);

    // main shader parts
    const char V_SHADER_FLAT[] =
       "uniform mat4 uProjMat;\n"
       "uniform mat4 uModelMat;\n"
       "uniform vec4 uTexData;\n"
       "uniform vec4 uTexUVData;\n"
       "uniform vec4 uTexAData;\n"

       "attribute vec4 vVertex;\n"
       "attribute vec2 vTexCoord;\n"

       "varying vec3 fTexCoord;\n"
       "varying vec3 fTexUVCoord;\n"
       "varying vec3 fTexACoord;\n"
       "varying vec3 fTexClamp;\n"
       "varying float fTexClampW;\n"

       "void main(void) {\n"
       "    fTexCoord   = vec3(uTexData.xy   + vTexCoord * uTexData.zw,   0.0);\n"
       "    fTexUVCoord = vec3(uTexUVData.xy + vTexCoord * uTexUVData.zw, 0.0);\n"
       "    fTexACoord  = vec3(uTexAData.xy  + vTexCoord * uTexAData.zw,  0.0);\n"
       "    fTexClamp   = vec3(0.0, 0.0, 0.0);\n"
       "    fTexClampW  = 0.0;\n"
       "    gl_Position = uProjMat * uModelMat * vVertex;\n"
       "}\n";

    const char V_SHADER_CUBEMAP[] =
       "uniform mat4 uProjMat;\n"
       "uniform mat4 uModelMat;\n"
       "uniform vec4 uTexData;\n"
       "uniform vec4 uTexUVData;\n"
       "uniform vec4 uTexAData;\n"
       "uniform float uTexCubeFlipZ;\n"

       "attribute vec4 vVertex;\n"
       "attribute vec3 vNormal;\n"
       "attribute vec4 vColor;\n"

       "varying vec3 fTexCoord;\n"
       "varying vec3 fTexUVCoord;\n"
       "varying vec3 fTexACoord;\n"
       "varying vec3 fTexClamp;\n"
       "varying float fTexClampW;\n"

       "void main(void) {\n"
       "    vec4 aPos = uProjMat * uModelMat * vec4(vVertex.xyz, 1.0);\n"
       "    gl_Position = aPos.xyww;\n"
       "    vec3 aTCoord = vNormal;\n" // fake normal attribute
       "    aTCoord.z  *= uTexCubeFlipZ;\n"
       "    fTexCoord   = aTCoord;\n"
       "    fTexUVCoord = aTCoord;\n"
       "    fTexACoord  = aTCoord;\n"
       "    fTexClamp   = vColor.xyz;\n" // fake color attribute
       "    fTexClampW  = vColor.w;\n"
       "}\n";

    const char F_SHADER_FLAT[] =
       "varying vec3 fTexCoord;\n"
       "varying vec3 fTexUVCoord;\n"
       "varying vec3 fTexACoord;\n"
       "varying vec3 fTexClamp;\n"
       "varying float fTexClampW;\n"
        // we split these functions for two reasons:
        // - to change function code (like color conversion);
        // - to optimize rendering on old hardware not supported conditions (GeForce FX for example).
       "vec3 getTexCoords(in vec3 theCoords, in vec4 theClamp);\n"
       "vec4 getColor(in vec3 texCoord);\n"
       "void convertToRGB(inout vec4 theColor, in vec3 theTexUVCoord, in vec3 texCoordA);\n"
       "void applyCorrection(inout vec4 theColor);\n"
       "void applyGamma(inout vec4 theColor);\n"

       "void main(void) {\n"
       "    vec4 aTexClamp   = vec4(fTexClamp, fTexClampW);\n"
       "    vec3 aTexCoord   = getTexCoords(fTexCoord,   aTexClamp);\n"
       "    vec3 aTexCoordUV = getTexCoords(fTexUVCoord, aTexClamp);\n"
       "    vec3 aTexCoordA  = getTexCoords(fTexACoord,  aTexClamp);\n"
            // extract color from main texture
       "    vec4 aColor = getColor(aTexCoord);\n"
            // convert from alien color model (like YUV) to RGB
       "    convertToRGB(aColor, aTexCoordUV, aTexCoordA);\n"
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

void StGLImageProgram::setTextureADataSize(StGLContext&    theCtx,
                                           const StGLVec4& theTexDataVec4) {
  theCtx.core20fwd->glUniform4fv(uniTexADataLoc, 1, theTexDataVec4);
}

void StGLImageProgram::setCubeTextureFlipZ(StGLContext& theCtx,
                                           bool theToFlip) {
    theCtx.core20fwd->glUniform1f(uniTexCubeFlipZLoc, theToFlip ? 1.0f : -1.0f);
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
        case StImage::ImgColor_XYZ:  return StGLImageProgram::FragToRgb_FromXyz;
        case StImage::ImgColor_YUV:
        case StImage::ImgColor_YUVA: {
            const bool hasAlpha = theColorModel == StImage::ImgColor_YUVA;
            switch(theColorScale) {
                case StImage::ImgScale_Mpeg9:  return hasAlpha ? StGLImageProgram::FragToRgb_FromYuva9Mpeg  : StGLImageProgram::FragToRgb_FromYuv9Mpeg;
                case StImage::ImgScale_Mpeg10: return hasAlpha ? StGLImageProgram::FragToRgb_FromYuva10Mpeg : StGLImageProgram::FragToRgb_FromYuv10Mpeg;
                case StImage::ImgScale_Jpeg9:  return hasAlpha ? StGLImageProgram::FragToRgb_FromYuva9Full  : StGLImageProgram::FragToRgb_FromYuv9Full;
                case StImage::ImgScale_Jpeg10: return hasAlpha ? StGLImageProgram::FragToRgb_FromYuva10Full : StGLImageProgram::FragToRgb_FromYuv10Full;
                case StImage::ImgScale_Mpeg:   return hasAlpha ? StGLImageProgram::FragToRgb_FromYuvaMpeg   : StGLImageProgram::FragToRgb_FromYuvMpeg;
                case StImage::ImgScale_Full:   return hasAlpha ? StGLImageProgram::FragToRgb_FromYuvaFull   : StGLImageProgram::FragToRgb_FromYuvFull;
                case StImage::ImgScale_NvMpeg: return StGLImageProgram::FragToRgb_FromYuvNvMpeg;
                case StImage::ImgScale_NvFull: return StGLImageProgram::FragToRgb_FromYuvNvFull;
            }
            return hasAlpha ? StGLImageProgram::FragToRgb_FromYuvaFull : StGLImageProgram::FragToRgb_FromYuvFull;
        }
        default: {
            ST_DEBUG_LOG("No GLSL shader for this color model = " + theColorModel);
            ST_ASSERT(false, "StGLImageProgram::getColorShader() - unsupported color model!");
        }
    }
    return StGLImageProgram::FragToRgb_FromRgb;
}

bool StGLImageProgram::init(StGLContext&                 theCtx,
                            const StImage::ImgColorModel theColorModel,
                            const StImage::ImgColorScale theColorScale,
                            const FragGetColor           theFilter,
                            const FragTexEAC theTexCoord) {
    registerFragments(theCtx);

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
    isChanged = setFragmentShaderPart(theCtx, FragSection_GetTexCoords, theTexCoord) || isChanged;
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
        uniTexADataLoc        = myActiveProgram->getUniformLocation(theCtx, "uTexAData");
        uniTexSizePxLoc       = myActiveProgram->getUniformLocation(theCtx, "uTexSizePx");
        uniTexelSizePxLoc     = myActiveProgram->getUniformLocation(theCtx, "uTexelSize");
        uniTexCubeFlipZLoc    = myActiveProgram->getUniformLocation(theCtx, "uTexCubeFlipZ");
        uniColorProcessingLoc = myActiveProgram->getUniformLocation(theCtx, "uColorProcessing");
        uniGammaLoc           = myActiveProgram->getUniformLocation(theCtx, "uGamma");
        myActiveProgram->atrVVertexLoc  = myActiveProgram->getAttribLocation(theCtx, "vVertex");
        myActiveProgram->atrVTCoordLoc  = myActiveProgram->getAttribLocation(theCtx, "vTexCoord");
        myActiveProgram->atrVNormalLoc  = myActiveProgram->getAttribLocation(theCtx, "vNormal");
        myActiveProgram->atrVColorsLoc  = myActiveProgram->getAttribLocation(theCtx, "vColor");

        StGLVarLocation uniTextureLoc  = myActiveProgram->getUniformLocation(theCtx, "uTexture");
        StGLVarLocation uniTextureULoc = myActiveProgram->getUniformLocation(theCtx, "uTextureU");
        StGLVarLocation uniTextureVLoc = myActiveProgram->getUniformLocation(theCtx, "uTextureV");
        StGLVarLocation uniTextureALoc = myActiveProgram->getUniformLocation(theCtx, "uTextureA");
        myActiveProgram->use(theCtx);
        theCtx.core20fwd->glUniform1i(uniTextureLoc,  StGLProgram::TEXTURE_SAMPLE_0);
        theCtx.core20fwd->glUniform1i(uniTextureULoc, StGLProgram::TEXTURE_SAMPLE_1);
        theCtx.core20fwd->glUniform1i(uniTextureVLoc, StGLProgram::TEXTURE_SAMPLE_2);
        theCtx.core20fwd->glUniform1i(uniTextureALoc, StGLProgram::TEXTURE_SAMPLE_3);
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
