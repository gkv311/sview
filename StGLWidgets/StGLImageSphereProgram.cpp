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
    const StGLResources aShaders("StGLWidgets");
    StRawFile aVShaderFile(aShaders.getShaderFile("sphereImage.shv"));
    StRawFile aFShaderFile(aShaders.getShaderFile("sphereImage.shf"));
    StRawFile aGetColorLinFile(aShaders.getShaderFile("sphereGetColorLinear.shf"));
    if(!aVShaderFile.readFile()) {
        //theCtx.pushError(StString("Shader file '") + aVShaderFile.getPath() + "' is not found!");
        ST_ERROR_LOG(StString("Shader file '") + aVShaderFile.getPath() + "' is not found!");
    }
    if(!aFShaderFile.readFile()) {
        //theCtx.pushError(StString("Shader file '") + aFShaderFile.getPath() + "' is not found!");
        ST_ERROR_LOG(StString("Shader file '") + aFShaderFile.getPath() + "' is not found!");
    }
    if(!aGetColorLinFile.readFile()) {
        //theCtx.pushError(StString("Shader file '") + aGetColorLinFile.getPath() + "' is not found!");
        ST_ERROR_LOG(StString("Shader file '") + aGetColorLinFile.getPath() + "' is not found!");
    }

    registerVertexShaderPart  (0,                    0, (const char* )aVShaderFile.getBuffer());
    registerFragmentShaderPart(FragSection_Main,     0, (const char* )aFShaderFile.getBuffer());
    registerFragmentShaderPart(FragSection_GetColor, FragGetColor_Blend,
                               (const char* )aGetColorLinFile.getBuffer());
}
