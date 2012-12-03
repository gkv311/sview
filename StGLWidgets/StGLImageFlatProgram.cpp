/**
 * Copyright © 2010-2012 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#include <StGLWidgets/StGLImageFlatProgram.h>
#include <StGL/StGLResources.h>

StGLImageFlatProgram::StGLImageFlatProgram()
: StGLImageProgram("StGLImageFlatProgram"),
  fGetColorPtr(NULL) {
    fGetColorPtr = &fGetColor;
}

void StGLImageFlatProgram::setSmoothFilter(StGLContext&        theCtx,
                                           const TextureFilter theTextureFilter) {
    if(!isValid()) {
        return;
    }
    switch(theTextureFilter) {
        case FILTER_BLEND: {
            StGLProgram::detachShader(theCtx, *fGetColorPtr).attachShader(theCtx, fGetColorBlend).link(theCtx);
            fGetColorPtr = &fGetColorBlend;
            break;
        }
        case FILTER_NEAREST:
        case FILTER_LINEAR:
        default: {
            StGLProgram::detachShader(theCtx, *fGetColorPtr).attachShader(theCtx, fGetColor).link(theCtx);
            fGetColorPtr = &fGetColor;
            break;
        }
    }
}

bool StGLImageFlatProgram::init(StGLContext& theCtx) {
    if(!StGLImageProgram::init(theCtx)) {
        return false;
    }
    const StGLResources aShaders("StGLWidgets");
    StGLVertexShader vShaderMain(StGLProgram::getTitle());
    vShaderMain.initFile(theCtx, aShaders.getShaderFile("flatImage.shv"));
    StGLAutoRelease aTmp1(theCtx, vShaderMain);

    StGLFragmentShader fShaderMain(StGLProgram::getTitle());
    fShaderMain.initFile(theCtx, aShaders.getShaderFile("flatImage.shf"));
    StGLAutoRelease aTmp2(theCtx, fShaderMain);

    return StGLProgram::create(theCtx)
        .attachShader(theCtx, vShaderMain)
        .attachShader(theCtx, fShaderMain)
        .attachShader(theCtx, *fGetColorPtr)
        .attachShader(theCtx, *f2RGBPtr)
        .attachShader(theCtx, *fCorrectPtr)
        .attachShader(theCtx, *fGammaPtr)
        .link(theCtx);
}
