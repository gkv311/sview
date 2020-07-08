/**
 * StGLWidgets, small C++ toolkit for writing GUI using OpenGL.
 * Copyright © 2009-2020 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#include <StGLWidgets/StGLTextBorderProgram.h>

#include <StGL/StGLContext.h>
#include <StGL/StGLMatrix.h>
#include <StGLCore/StGLCore20.h>

StGLTextBorderProgram::StGLTextBorderProgram()
: StGLProgram("StGLTextBorderProgram") {
    //
}

StGLTextBorderProgram::~StGLTextBorderProgram() {
    //
}

void StGLTextBorderProgram::setProjMat(StGLContext&      theCtx,
                                       const StGLMatrix& theProjMat) {
    theCtx.core20fwd->glUniformMatrix4fv(myUniformProjMat, 1, GL_FALSE, theProjMat);
}

void StGLTextBorderProgram::setDisplacement(StGLContext& theCtx,
                                            const StGLVec3& theDisp,
                                            const float theScale) {
    theCtx.core20fwd->glUniform4fv(myUniformDispl, 1, StGLVec4(theDisp, theScale));
}

void StGLTextBorderProgram::setColor(StGLContext&    theCtx,
                                     const StGLVec4& theColor) {
    theCtx.core20fwd->glUniform4fv(myUniformColor, 1, theColor);
}

bool StGLTextBorderProgram::init(StGLContext& theCtx) {
    const char VERTEX_SHADER[] =
       "uniform mat4 uProjMat; \
        uniform vec4 uDisp; \
        attribute vec4 vVertex; \
        void main(void) { \
            gl_Position = uProjMat * (vec4(vVertex.xy * uDisp.w, 0.0, 1.0) + vec4(uDisp.xyz, 0.0)); \
        }";

    const char FRAGMENT_SHADER[] =
       "uniform vec4 uColor; \
        void main(void) { \
            gl_FragColor = uColor; \
        }";

    StGLVertexShader aVertexShader(StGLProgram::getTitle());
    aVertexShader.init(theCtx, VERTEX_SHADER);
    StGLAutoRelease aTmp1(theCtx, aVertexShader);

    StGLFragmentShader aFragmentShader(StGLProgram::getTitle());
    aFragmentShader.init(theCtx, FRAGMENT_SHADER);
    StGLAutoRelease aTmp2(theCtx, aFragmentShader);
    if(!StGLProgram::create(theCtx)
       .attachShader(theCtx, aVertexShader)
       .attachShader(theCtx, aFragmentShader)
       .bindAttribLocation(theCtx, "vVertex", getVVertexLoc())
       .link(theCtx)) {
        return false;
    }

    myUniformProjMat  = StGLProgram::getUniformLocation(theCtx, "uProjMat");
    myUniformDispl    = StGLProgram::getUniformLocation(theCtx, "uDisp");
    myUniformColor    = StGLProgram::getUniformLocation(theCtx, "uColor");
    return myUniformProjMat.isValid()
        && myUniformDispl.isValid()
        && myUniformColor.isValid();
}
