/**
 * StOutDistorted, class providing stereoscopic output in anamorph side by side format using StCore toolkit.
 * Copyright © 2016 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#include "StProgramFlat.h"

#include <StGLCore/StGLCore20.h>
#include <StGL/StGLContext.h>

StProgramFlat::StProgramFlat()
: StGLProgram("StProgramFlat") {}

bool StProgramFlat::init(StGLContext& theCtx) {
    const char VERTEX_SHADER[] =
       "attribute vec4 vVertex;\n"
       "attribute vec2 vTexCoord;\n"
       "varying vec2 fTexCoord;\n"
       "void main(void) {\n"
       "  fTexCoord = vTexCoord;\n"
       "  gl_Position = vVertex;\n"
       "}\n";

    const char FRAGMENT_SHADER[] =
       "uniform sampler2D texR, texL;\n"
       "varying vec2 fTexCoord;\n"
       "void main(void) {\n"
       "  gl_FragColor = texture2D(texR, fTexCoord);\n"
       "}\n";

    StGLVertexShader aVertexShader(StGLProgram::getTitle());
    StGLAutoRelease aTmp1(theCtx, aVertexShader);
    aVertexShader.init(theCtx, VERTEX_SHADER);

    StGLFragmentShader aFragmentShader(StGLProgram::getTitle());
    StGLAutoRelease aTmp2(theCtx, aFragmentShader);
    aFragmentShader.init(theCtx, FRAGMENT_SHADER);
    if(!StGLProgram::create(theCtx)
       .attachShader(theCtx, aVertexShader)
       .attachShader(theCtx, aFragmentShader)
       .bindAttribLocation(theCtx, "vVertex",   getVVertexLoc())
       .bindAttribLocation(theCtx, "vTexCoord", getVTexCoordLoc())
       .link(theCtx)) {
        return false;
    }
    return true;
}
