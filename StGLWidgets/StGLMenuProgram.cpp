/**
 * StGLWidgets, small C++ toolkit for writing GUI using OpenGL.
 * Copyright © 2009-2013 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#include <StGLWidgets/StGLMenuProgram.h>

#include <StGLCore/StGLCore20.h>
#include <StGL/StGLContext.h>
#include <StGL/StGLMatrix.h>

StGLMenuProgram::StGLMenuProgram()
: StGLProgram("StGLMenuProgram") {
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

void StGLMenuProgram::use(StGLContext&  theCtx,
                          const GLfloat theDispX) {
    StGLProgram::use(theCtx);
    if(!stAreEqual(myDispX, theDispX, 0.0001f)) {
        myDispX = theDispX;
        theCtx.core20fwd->glUniform4fv(uniDispLoc,  1, StGLVec4(theDispX, 0.0f, 0.0f, 0.0f));
    }
}

void StGLMenuProgram::use(StGLContext&    theCtx,
                          const StGLVec4& theColor,
                          const GLfloat   theOpacityValue,
                          const GLfloat   theDispX) {
    StGLProgram::use(theCtx);
    theCtx.core20fwd->glUniform4fv(uniColorLoc, 1, StGLVec4(theColor.rgb(), theColor.a() * theOpacityValue));
    if(!stAreEqual(myDispX, theDispX, 0.0001f)) {
        myDispX = theDispX;
        theCtx.core20fwd->glUniform4fv(uniDispLoc,  1, StGLVec4(theDispX, 0.0f, 0.0f, 0.0f));
    }
}

bool StGLMenuProgram::init(StGLContext& theCtx) {
    const char VERTEX_SHADER[] =
       "uniform mat4 uProjMat;\n"
       "uniform vec4 uDisp;\n"
       "attribute vec4 vVertex;\n"
       "void main(void) {\n"
       "    gl_Position = uProjMat * (vVertex + uDisp);\n"
       "}\n";

    const char FRAGMENT_SHADER[] =
       "uniform vec4 uColor;\n"
       "void main(void) {\n"
       "    gl_FragColor = uColor;\n"
       "}\n";

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

    uniProjMatLoc = StGLProgram::getUniformLocation(theCtx, "uProjMat");
    uniDispLoc    = StGLProgram::getUniformLocation(theCtx, "uDisp");
    uniColorLoc   = StGLProgram::getUniformLocation(theCtx, "uColor");
    return uniProjMatLoc.isValid()
        && uniColorLoc.isValid();
}
