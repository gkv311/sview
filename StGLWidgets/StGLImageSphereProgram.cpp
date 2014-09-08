/**
 * Copyright © 2010-2014 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#include <StGLWidgets/StGLImageSphereProgram.h>
#include <StGL/StGLResources.h>
#include <StFile/StRawFile.h>

StGLImageSphereProgram::StGLImageSphereProgram()
: StGLImageProgram("StGLImageSphereProgram") {
    const char V_SHADER[] =
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

    const char F_SHADER[] =
       "varying vec2 fTexCoord;\n"
       "varying vec2 fTexUVCoord;\n"

       "vec4 getColor(in vec2 texCoord);\n"
       "void convertToRGB(inout vec4 color, in vec2 texUVCoord);\n"
       "void applyGamma(inout vec4 color);\n"

       "void main(void) {\n"
       "    vec4 color = getColor(fTexCoord);\n"
       "    convertToRGB(color, fTexUVCoord);\n"
       "    applyGamma(color);\n"
       "    gl_FragColor = color;\n"
       "}\n\n";

    const char F_SHADER_COLOR[] =
       "uniform sampler2D uTexture;\n"
       "uniform vec4 uTexData;\n"
       "uniform vec2 uTexSizePx;\n"
       "uniform vec2 uTexelSize;\n"

       "vec4 getColor(in vec2 texCoord) {\n"
       "    vec2 txCoord_CC = floor(uTexSizePx * texCoord) / uTexSizePx;\n"
       "    vec2 aDiff = (texCoord - txCoord_CC) * uTexSizePx;\n"
       "    if(txCoord_CC.y >= (uTexData.w - uTexelSize.y)) {\n"
       "        aDiff.y = 0.0;\n"
       "    }\n"
       "    vec2 diffSign = sign(aDiff);\n"
       "    vec2 diffAbs  = abs(aDiff);\n"
       "    vec2 txCoord_R = txCoord_CC + diffSign * vec2(uTexelSize.x, 0.0);\n"
       "    if(txCoord_R.x > (uTexData.z - uTexelSize.x)) {\n"
       "        txCoord_R.x = uTexelSize.x;\n"
       "    } else if(txCoord_R.x < uTexelSize.x) {\n"
       "        txCoord_R.x = uTexData.z - uTexelSize.x;\n"
       "    }\n"
       "    vec4 color_CC = texture2D(uTexture, txCoord_CC);\n"
       "    vec4 colorXX1 = mix(color_CC,\n"
       "                        texture2D(uTexture,\n"
       "                                  txCoord_R),\n"
       "                        diffAbs.x);\n"
       "    vec4 colorXX2 = mix(texture2D(uTexture,\n"
       "                                  txCoord_CC + diffSign * vec2(0.0, uTexelSize.y)),\n"
       "                        texture2D(uTexture,\n"
       "                                  txCoord_R + diffSign * vec2(0.0, uTexelSize.y)),\n"
       "                        diffAbs.x);\n"
       "    return mix(colorXX1, colorXX2, diffAbs.y);\n"
       "}\n\n";

    registerVertexShaderPart  (0,                    0, V_SHADER);
    registerFragmentShaderPart(FragSection_Main,     0, F_SHADER);
    registerFragmentShaderPart(FragSection_GetColor, FragGetColor_Blend, F_SHADER_COLOR);
}
