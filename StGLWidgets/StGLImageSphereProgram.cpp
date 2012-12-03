/**
 * Copyright © 2010-2012 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#include <StGLWidgets/StGLImageSphereProgram.h>
#include <StGL/StGLResources.h>

StGLImageSphereProgram::StGLImageSphereProgram()
: StGLImageProgram("StGLImageSphereProgram"),
  fGetColorPtr(NULL),
  fGetColorLinear("StGLImageSphereProgram::fGetColorLinear") {
    fGetColorPtr = &fGetColor;
}

void StGLImageSphereProgram::release(StGLContext& theCtx) {
    fGetColorLinear.release(theCtx);
    StGLImageProgram::release(theCtx);
}

void StGLImageSphereProgram::setSmoothFilter(StGLContext&        theCtx,
                                             const TextureFilter theTextureFilter) {
    switch(theTextureFilter) {
        case FILTER_BLEND:
        case FILTER_LINEAR: {
            if(isValid()) {
                StGLProgram::detachShader(theCtx, *fGetColorPtr).attachShader(theCtx, fGetColorLinear).link(theCtx);
            }
            fGetColorPtr = &fGetColorLinear;
            break;
        }
        case FILTER_NEAREST:
        default: {
            if(isValid()) {
                StGLProgram::detachShader(theCtx, *fGetColorPtr).attachShader(theCtx, fGetColor).link(theCtx);
            }
            fGetColorPtr = &fGetColor;
            break;
        }
    }
}

bool StGLImageSphereProgram::init(StGLContext& theCtx) {
    if(!StGLImageProgram::init(theCtx)) {
        return false;
    }

    const StGLResources aShaders("StGLWidgets");
    StGLVertexShader vShaderMain(StGLProgram::getTitle());
    vShaderMain.initFile(theCtx, aShaders.getShaderFile("sphereImage.shv"));
    StGLAutoRelease aTmp1(theCtx, vShaderMain);

    StGLFragmentShader fShaderMain(StGLProgram::getTitle());
    fShaderMain.initFile(theCtx, aShaders.getShaderFile("sphereImage.shf"));
    StGLAutoRelease aTmp2(theCtx, fShaderMain);

    fGetColorLinear.initFile(theCtx, aShaders.getShaderFile("sphereGetColorLinear.shf"));

    return StGLProgram::create(theCtx)
        .attachShader(theCtx, vShaderMain)
        .attachShader(theCtx, fShaderMain)
        .attachShader(theCtx, *fGetColorPtr)
        .attachShader(theCtx, *f2RGBPtr)
        .attachShader(theCtx, *fCorrectPtr)
        .attachShader(theCtx, *fGammaPtr)
        .link(theCtx);
}
