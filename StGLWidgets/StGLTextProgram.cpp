/**
 * Copyright © 2009-2015 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#include <StGLWidgets/StGLTextProgram.h>

#include <StGL/StGLContext.h>
#include <StGL/StGLMatrix.h>
#include <StGLCore/StGLCore20.h>

StGLTextProgram::StGLTextProgram()
: StGLProgram("StGLTextProgram") {
    //
}

StGLTextProgram::~StGLTextProgram() {
    //
}

void StGLTextProgram::setProjMat(StGLContext&      theCtx,
                                 const StGLMatrix& theProjMat) {
    theCtx.core20fwd->glUniformMatrix4fv(myUniformProjMat, 1, GL_FALSE, theProjMat);
}

void StGLTextProgram::setModelMat(StGLContext&      theCtx,
                                  const StGLMatrix& theModelMat) {
    theCtx.core20fwd->glUniformMatrix4fv(myUniformModelMat, 1, GL_FALSE, theModelMat);
}

void StGLTextProgram::setColor(StGLContext&    theCtx,
                               const StGLVec4& theColor) {
    theCtx.core20fwd->glUniform4fv(myUniformColor, 1, theColor);
}

bool StGLTextProgram::init(StGLContext& theCtx) {
    const char VERTEX_SHADER[] =
       "uniform mat4 uProjMat; \
        uniform mat4 uModelMat; \
        attribute vec4 vVertex; \
        attribute vec2 vTexCoord; \
        varying vec2 fTexCoord; \
        void main(void) { \
            fTexCoord = vTexCoord; \
            gl_Position = uProjMat * uModelMat * vVertex; \
        }";

    const char FRAGMENT_GET_RED[] =
       "float getAlpha(void) { return texture2D(uTexture, fTexCoord).r; }";

    const char FRAGMENT_GET_ALPHA[] =
       "float getAlpha(void) { return texture2D(uTexture, fTexCoord).a; }";

    const char FRAGMENT_SHADER[] =
       "uniform sampler2D uTexture;"
       "uniform vec4 uTextColor;"
       "varying vec2 fTexCoord;"
       "float getAlpha(void);"
       "void main(void) {"
       "     vec4 color = uTextColor;"
       "     color.a *= getAlpha();"
       "     gl_FragColor = color;"
       "}";

    StGLVertexShader aVertexShader(StGLProgram::getTitle());
    aVertexShader.init(theCtx, VERTEX_SHADER);
    StGLAutoRelease aTmp1(theCtx, aVertexShader);

    StGLFragmentShader aFragmentShader(StGLProgram::getTitle());
    aFragmentShader.init(theCtx, FRAGMENT_SHADER,
                         theCtx.arbTexRG ? FRAGMENT_GET_RED : FRAGMENT_GET_ALPHA);
    StGLAutoRelease aTmp2(theCtx, aFragmentShader);
    if(!StGLProgram::create(theCtx)
       .attachShader(theCtx, aVertexShader)
       .attachShader(theCtx, aFragmentShader)
       .bindAttribLocation(theCtx, "vVertex",   getVVertexLoc())
       .bindAttribLocation(theCtx, "vTexCoord", getVTexCoordLoc())
       .link(theCtx)) {
        return false;
    }

    myUniformProjMat  = StGLProgram::getUniformLocation(theCtx, "uProjMat");
    myUniformModelMat = StGLProgram::getUniformLocation(theCtx, "uModelMat");
    myUniformColor    = StGLProgram::getUniformLocation(theCtx, "uTextColor");

    StGLVarLocation aUniformTexture = StGLProgram::getUniformLocation(theCtx, "uTexture");
    if(aUniformTexture.isValid()) {
        StGLProgram::use(theCtx);
        theCtx.core20fwd->glUniform1i(aUniformTexture, StGLProgram::TEXTURE_SAMPLE_0);
        StGLProgram::unuse(theCtx);
    }

    return myUniformProjMat.isValid()
        && myUniformModelMat.isValid()
        && myUniformColor.isValid()
        && aUniformTexture.isValid();
}
