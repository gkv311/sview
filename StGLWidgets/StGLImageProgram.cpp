/**
 * Copyright © 2010-2012 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#include <StGLWidgets/StGLImageProgram.h>
#include <StGL/StGLResources.h>
#include <StGL/StGLContext.h>
#include <StGLCore/StGLCore20.h>

StGLImageProgram::StGLImageProgram(const StString& theTitle)
: StGLMeshProgram(StString("StGLImageProgram, ") + theTitle),
  uniTexMainDataLoc(),
  uniTexUVDataLoc(),
  uniTexSizePxLoc(),
  uniTexelSizePxLoc(),
  uniColorProcessingLoc(),
  uniGammaLoc(),
  fGetColor     (StString("StGLImageProgram::fGetColor, ")      + theTitle),
  fGetColorBlend(StString("StGLImageProgram::fGetColorBlend, ") + theTitle),
  // color conversion
  f2RGBPtr(NULL),
  fRGB2RGB    (StString("StGLImageProgram::fRGB2RGB, ")  + theTitle),
  fRGBA2RGB   (StString("StGLImageProgram::fRGBA2RGB, ") + theTitle),
  fGray2RGB   (StString("StGLImageProgram::fGray2RGB, ") + theTitle),
  fYUV2RGB    (StString("StGLImageProgram::fYUV2RGB, ")  + theTitle),
  fYUV2RGBjpeg(StString("StGLImageProgram::fYUV2RGBjpeg, ") + theTitle),
  // color correction
  fCorrectPtr(NULL),
  fCorrectNO (StString("StGLImageProgram::fCorrectNO, ")  + theTitle),
  fCorrectON (StString("StGLImageProgram::fCorrectON, ")  + theTitle),
  myBrightness(),
  mySaturation(),
  myColorScale(1.0f, 1.0f, 1.0f),
  // gamma correction
  fGammaPtr(NULL),
  fGammaNO (StString("StGLImageProgram::fGammaNO, ")  + theTitle),
  fGammaON (StString("StGLImageProgram::fGammaON, ")  + theTitle) {
    //
    f2RGBPtr    = &fRGB2RGB;
    fCorrectPtr = &fCorrectNO;
    fGammaPtr   = &fGammaNO;

    params.gamma = new StFloat32Param(   1.0f,         // initial value
                                        0.05f, 100.0f, // min, max values
                                         1.0f,         // default value
                                        0.05f,         // incremental step
                                      0.0001f);        // equality tolerance
    params.gamma->signals.onChanged.connect(this, &StGLImageProgram::doGammaChanged);
    params.brightness = new StFloat32Param(   1.0f,         // initial value
                                              0.0f, 100.0f, // min, max values
                                              1.0f,         // default value
                                             0.05f,         // incremental step
                                           0.0001f);        // equality tolerance
    params.brightness->signals.onChanged.connect(this, &StGLImageProgram::doSetupCorrectionShader);
    params.saturation = new StFloat32Param(   1.0f,         // initial value
                                            -10.0f, 100.0f, // min, max values
                                              1.0f,         // default value
                                             0.05f,         // incremental step
                                           0.0001f);        // equality tolerance
    params.saturation->signals.onChanged.connect(this, &StGLImageProgram::doSetupCorrectionShader);
}

StGLImageProgram::~StGLImageProgram() {
    //
}

void StGLImageProgram::release(StGLContext& theCtx) {
    fGetColor.release(theCtx);
    fGetColorBlend.release(theCtx);
    fRGB2RGB.release(theCtx);
    fRGBA2RGB.release(theCtx);
    fGray2RGB.release(theCtx);
    fYUV2RGB.release(theCtx);
    fYUV2RGBjpeg.release(theCtx);
    fCorrectNO.release(theCtx);
    fCorrectON.release(theCtx);
    fGammaNO.release(theCtx);
    fGammaON.release(theCtx);
    StGLMeshProgram::release(theCtx);
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

bool StGLImageProgram::isNoColorScale() const {
    return myColorScale.r() > 0.9f
        && myColorScale.g() > 0.9f
        && myColorScale.b() > 0.9f;
}

void StGLImageProgram::setColorScale(StGLContext&  /*theCtx*/,
                                     const StGLVec3& theScale) {
    myColorScale = theScale;
    doSetupCorrectionShader(0.0f);
}

void StGLImageProgram::resetColorScale(StGLContext& /*theCtx*/) {
    if(isNoColorScale()) {
        return;
    }
    myColorScale = StGLVec3(1.0f, 1.0f, 1.0f);
    doSetupCorrectionShader(0.0f);
}

void StGLImageProgram::setCorrectionUniform(StGLContext& theCtx) {
    if(fCorrectPtr == &fCorrectNO) {
        return;
    }
    StGLMatrix aColorMat = StGLMatrix::multiply(mySaturation, myBrightness);
    aColorMat.scale(myColorScale.r(), myColorScale.g(), myColorScale.b());
    theCtx.core20fwd->glUniformMatrix4fv(uniColorProcessingLoc, 1, GL_FALSE, aColorMat);
}

void StGLImageProgram::setGammaUniform(StGLContext& theCtx) {
    if(fGammaPtr == &fGammaNO) {
        return;
    }
    GLfloat aReversed = 1.0f / params.gamma->getValue();
    StGLVec4 aVec(aReversed, aReversed, aReversed, 1.0f);
    theCtx.core20fwd->glUniform4fv(uniGammaLoc, 1, aVec);
}

void StGLImageProgram::doSetupCorrectionShader(const float ) {
    myBrightness.setBrightness(params.brightness->getValue());
    mySaturation.setSaturation(params.saturation->getValue());
    if(!isValid()) {
        fCorrectPtr = (params.brightness->isDefaultValue() && params.saturation->isDefaultValue() && isNoColorScale())
                    ? &fCorrectNO : &fCorrectON;
        return;
    }

    StGLContext& aCtx = *myContext;
    if(params.brightness->isDefaultValue()
    && params.saturation->isDefaultValue()
    && isNoColorScale()) {
        StGLProgram::swapShader(aCtx, *fCorrectPtr, fCorrectNO);
        fCorrectPtr = &fCorrectNO;
        return;
    } else if(fCorrectPtr != &fCorrectON) {
        StGLProgram::swapShader(aCtx, *fCorrectPtr, fCorrectON);
        fCorrectPtr = &fCorrectON;
    }
    StGLProgram::use(aCtx);
    setCorrectionUniform(aCtx);
    StGLProgram::unuse(aCtx);
}

void StGLImageProgram::doGammaChanged(const float theGamma) {
    if(!isValid()) {
        fGammaPtr = stAreEqual(theGamma, 1.0f, 0.0001f) ? &fGammaNO : &fGammaON;
        return;
    }

    StGLContext& aCtx = *myContext;
    if(stAreEqual(theGamma, 1.0f, 0.0001f)) {
        StGLProgram::swapShader(aCtx, *fGammaPtr, fGammaNO);
        fGammaPtr = &fGammaNO;
        return;
    } else if(fGammaPtr != &fGammaON) {
        StGLProgram::swapShader(aCtx, *fGammaPtr, fGammaON);
        fGammaPtr = &fGammaON;
    }
    StGLProgram::use(aCtx);
    setGammaUniform(aCtx);
    StGLProgram::unuse(aCtx);
}

void StGLImageProgram::setupSrcColorShader(StGLContext&           theCtx,
                                           StImage::ImgColorModel theColorModel) {
    if(!isValid()) {
        return;
    }
    switch(theColorModel) {
        case StImage::ImgColor_RGB: {
            StGLProgram::swapShader(theCtx, *f2RGBPtr, fRGB2RGB);
            f2RGBPtr = &fRGB2RGB;
            break;
        }
        case StImage::ImgColor_RGBA: {
            StGLProgram::swapShader(theCtx, *f2RGBPtr, fRGBA2RGB);
            f2RGBPtr = &fRGBA2RGB;
            break;
        }
        case StImage::ImgColor_GRAY: {
            StGLProgram::swapShader(theCtx, *f2RGBPtr, fGray2RGB);
            f2RGBPtr = &fGray2RGB;
            break;
        }
        case StImage::ImgColor_YUV: {
            StGLProgram::swapShader(theCtx, *f2RGBPtr, fYUV2RGB);
            f2RGBPtr = &fYUV2RGB;
            break;
        }
        case StImage::ImgColor_YUVjpeg: {
            StGLProgram::swapShader(theCtx, *f2RGBPtr, fYUV2RGBjpeg);
            f2RGBPtr = &fYUV2RGBjpeg;
            break;
        }
        default:
            ST_DEBUG_LOG("No GLSL shader for this color model = " + theColorModel);
            ST_DEBUG_ASSERT(false);
    }
}

void StGLImageProgram::setContext(const StHandle<StGLContext>& theCtx) {
    myContext = theCtx;
}

bool StGLImageProgram::init(StGLContext& theCtx) {
    const StGLResources aShaders("StGLWidgets");

    if(myContext.isNull()) {
        myContext = new StGLContext();
        if(!myContext->stglInit()) {
            return false;
        }
    }

    const char F_SHADER_GET_COLOR[] =
       "uniform sampler2D uTexture;"
       "vec4 getColor(in vec2 texCoord) {"
       "    return texture2D(uTexture, texCoord);"
       "}";

    // color correction shaders
    const char F_SHADER_CORRECT_NO[] =
       "void applyCorrection(inout vec4 color) {}";

    const char F_SHADER_CORRECT_ON[] =
       "uniform mat4 uColorProcessing;"
       "void applyCorrection(inout vec4 color) {"
       "    color = uColorProcessing * color;"
       "}";

    // gamma correction shaders
    const char F_SHADER_GAMMA_NO[] =
       "void applyGamma(inout vec4 color) {}";

    const char F_SHADER_GAMMA_ON[] =
       "uniform vec4 uGamma;"
       "void applyGamma(inout vec4 color) {"
       "    color = pow(color, uGamma);"
       "}";

    // color conversion shaders
    const char F_SHADER_RGB2RGB[] =
       "void convertToRGB(inout vec4 color, in vec2 texCoord) {}";

    const char F_SHADER_RGBA2RGB[] =
       "void convertToRGB(inout vec4 color, in vec2 texCoord) {"
       "    vec4 backColor;"
       "    bool evenX = int(mod(floor(gl_FragCoord.x + 1.5), 16.0)) >= 8;" // just simple 8 pixels check-board
       "    bool evenY = int(mod(floor(gl_FragCoord.y + 1.5), 16.0)) >= 8;"
       "    if((evenX && evenY) || (!evenX && !evenY)) {"
       "        backColor = vec4(0.2, 0.2, 0.2, 1.0);"
       "    } else {"
       "        backColor = vec4(1.0, 1.0, 1.0, 1.0);"
       "    }"
       "    color = mix(backColor, color, color.a);"
       "}";

    const char F_SHADER_GRAY2RGB[] =
       "void convertToRGB(inout vec4 color, in vec2 texCoord) {"
       "color.r = color.a;" // gray scale stored in alpha
       "color.g = color.a;"
       "color.b = color.a;"
       //"color.a = 1.0;"
       "}";

    return fGetColor.init (theCtx, F_SHADER_GET_COLOR)
        && fCorrectNO.init(theCtx, F_SHADER_CORRECT_NO)
        && fCorrectON.init(theCtx, F_SHADER_CORRECT_ON)
        && fGammaNO.init  (theCtx, F_SHADER_GAMMA_NO)
        && fGammaON.init  (theCtx, F_SHADER_GAMMA_ON)
        && fRGB2RGB.init  (theCtx, F_SHADER_RGB2RGB)
        && fRGBA2RGB.init (theCtx, F_SHADER_RGBA2RGB)
        && fGray2RGB.init (theCtx, F_SHADER_GRAY2RGB)
        && fGetColorBlend.initFile(theCtx, aShaders.getShaderFile("flatGetColorBlend.shf"))
        && fYUV2RGB.initFile      (theCtx, aShaders.getShaderFile("convertYUV2RGB.shf"))
        && fYUV2RGBjpeg.initFile  (theCtx, aShaders.getShaderFile("convertYUV2RGBjpeg.shf"));
}

bool StGLImageProgram::link(StGLContext& theCtx) {
    if(!StGLProgram::link(theCtx)) {
        return false;
    }

    uniProjMatLoc      = StGLProgram::getUniformLocation(theCtx, "uProjMat");
    uniModelMatLoc     = StGLProgram::getUniformLocation(theCtx, "uModelMat");
    uniTexMainDataLoc  = StGLProgram::getUniformLocation(theCtx, "uTexData");
    uniTexUVDataLoc    = StGLProgram::getUniformLocation(theCtx, "uTexUVData");
    uniTexSizePxLoc    = StGLProgram::getUniformLocation(theCtx, "uTexSizePx");
    uniTexelSizePxLoc  = StGLProgram::getUniformLocation(theCtx, "uTexelSize");
    uniColorProcessingLoc = StGLProgram::getUniformLocation(theCtx, "uColorProcessing");
    uniGammaLoc        = StGLProgram::getUniformLocation(theCtx, "uGamma");
    atrVVertexLoc      = StGLProgram::getAttribLocation(theCtx, "vVertex");
    atrVTCoordLoc      = StGLProgram::getAttribLocation(theCtx, "vTexCoord");

    StGLVarLocation uniTextureLoc  = StGLProgram::getUniformLocation(theCtx, "uTexture");
    StGLVarLocation uniTextureULoc = StGLProgram::getUniformLocation(theCtx, "uTextureU");
    StGLVarLocation uniTextureVLoc = StGLProgram::getUniformLocation(theCtx, "uTextureV");
    StGLProgram::use(theCtx);
    theCtx.core20fwd->glUniform1i(uniTextureLoc,  StGLProgram::TEXTURE_SAMPLE_0);
    theCtx.core20fwd->glUniform1i(uniTextureULoc, StGLProgram::TEXTURE_SAMPLE_1);
    theCtx.core20fwd->glUniform1i(uniTextureVLoc, StGLProgram::TEXTURE_SAMPLE_2);
    setCorrectionUniform(theCtx);
    setGammaUniform(theCtx);
    StGLProgram::unuse(theCtx);

    return uniModelMatLoc.isValid()
        && uniTexMainDataLoc.isValid()
        //&& uniTexSizePxLoc.isValid()
        //&& uniTexelSizePxLoc.isValid()
        //&& uniSmoothFilterLoc.isValid()
        //&& uniColorProcessingLoc
        && atrVVertexLoc.isValid()
        && atrVTCoordLoc.isValid()
        && uniTextureLoc.isValid();
}
