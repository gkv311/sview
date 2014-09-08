/**
 * Copyright © 2010-2014 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#include <StGLWidgets/StGLImageFlatProgram.h>
#include <StGL/StGLResources.h>
#include <StFile/StRawFile.h>

StGLImageFlatProgram::StGLImageFlatProgram()
: StGLImageProgram("StGLImageFlatProgram") {
    const char V_SHADER_FLAT[] =
       "uniform mat4 uProjMat;\n"
       "uniform mat4 uModelMat;\n"
       "uniform vec4 uTexData;\n"
       "uniform vec4 uTexUVData;\n"

       "attribute vec4 vVertex;\n"
       "attribute vec2 vTexCoord;\n"

       "varying vec2 fTexCoord;\n"
       "varying vec2 fTexUVCoord;\n"

       "void main(void) {\n"
       "    fTexCoord   = uTexData.xy   + vTexCoord * uTexData.zw;\n"
       "    fTexUVCoord = uTexUVData.xy + vTexCoord * uTexUVData.zw;\n"
       "    gl_Position = uProjMat * uModelMat * vVertex;\n"
       "}\n";

    const char F_SHADER_FLAT[] =
       "varying vec2 fTexCoord;\n"
       "varying vec2 fTexUVCoord;\n"
        // we split these functions for two reasons:
        // - to change function code (like color conversion);
        // - to optimize rendering on old hardware not supported conditions (GeForce FX for example).
       "vec4 getColor(in vec2 texCoord);\n"
       "void convertToRGB(inout vec4 theColor, in vec2 theTexUVCoord);\n"
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

    registerVertexShaderPart  (0,                0, V_SHADER_FLAT);
    registerFragmentShaderPart(FragSection_Main, 0, F_SHADER_FLAT);
}
