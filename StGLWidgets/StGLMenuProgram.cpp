/**
 * Copyright © 2009-2012 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#include <StGLWidgets/StGLMenuProgram.h>

#include <StGLCore/StGLCore20.h>
#include <StGL/StGLContext.h>
#include <StGL/StGLMatrix.h>

StGLMenuProgram::StGLMenuProgram()
: StGLProgram("StGLMenuProgram"),
  uniProjMatLoc(),
  uniColorLoc(),
  atrVVertexLoc() {
    //
}

void StGLMenuProgram::setProjMat(StGLContext&      theCtx,
                                 const StGLMatrix& theProjMat) {
    theCtx.core20fwd->glUniformMatrix4fv(uniProjMatLoc, 1, GL_FALSE, theProjMat);
}

void StGLMenuProgram::setColor(StGLContext&    theCtx,
                               const StGLVec4& theColor,
                               const GLfloat   theOpacityValue) {
    theCtx.core20fwd->glUniform4fv(uniColorLoc, 1, StGLVec4(theColor.rgb(), theColor.a() * theOpacityValue));
}

bool StGLMenuProgram::init(StGLContext& theCtx) {
    const char VERTEX_SHADER[] =
       "uniform mat4 uProjMat; \
        attribute vec4 vVertex; \
        void main(void) { \
            gl_Position = uProjMat * vVertex; \
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
       .link(theCtx)) {
        return false;
    }

    uniProjMatLoc = StGLProgram::getUniformLocation(theCtx, "uProjMat");
    uniColorLoc   = StGLProgram::getUniformLocation(theCtx, "uColor");
    atrVVertexLoc = StGLProgram::getAttribLocation(theCtx, "vVertex");

    return uniProjMatLoc.isValid()
        && uniColorLoc.isValid()
        && atrVVertexLoc.isValid();
}
