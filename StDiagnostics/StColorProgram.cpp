/**
 * Copyright © 2010-2012 Kirill Gavrilov <kirill@sview.ru>
 *
 * StDiagnostics program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * StDiagnostics program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program.
 * If not, see <http://www.gnu.org/licenses/>.
 */

#include "StColorProgram.h"

#include <StGL/StGLContext.h>
#include <StGLCore/StGLCore20.h>

StColorProgram::StColorProgram()
: StGLMeshProgram("StColorProgram"),
  uniScaleLoc(),
  uniTranslateLoc() {
    //
}

bool StColorProgram::init(StGLContext& theCtx) {
    const char VERTEX_SHADER[] =
       "uniform vec4 uScale; \
        uniform vec4 uTranslate; \
        attribute vec4 vVertex; \
        attribute vec4 vColor; \
        varying vec4 fColor; \
        void main(void) { \
            fColor = vColor; \
            gl_Position = vVertex * uScale + uTranslate; \
        }";

    const char FRAGMENT_SHADER[] =
       "varying vec4 fColor; \
        void main(void) { \
            gl_FragColor = fColor; \
        }";

    StGLVertexShader aVertexShader(StGLProgram::getTitle());
    StGLAutoRelease aTmp1(theCtx, aVertexShader);
    aVertexShader.init(theCtx, VERTEX_SHADER);

    StGLFragmentShader aFragmentShader(StGLProgram::getTitle());
    StGLAutoRelease aTmp2(theCtx, aFragmentShader);
    aFragmentShader.init(theCtx, FRAGMENT_SHADER);
    if(!StGLProgram::create(theCtx)
       .attachShader(theCtx, aVertexShader)
       .attachShader(theCtx, aFragmentShader)
       .link(theCtx)) {
        return false;
    }

    uniScaleLoc     = StGLProgram::getUniformLocation(theCtx, "uScale");
    uniTranslateLoc = StGLProgram::getUniformLocation(theCtx, "uTranslate");
    atrVVertexLoc   = StGLProgram::getAttribLocation(theCtx, "vVertex");
    atrVColorsLoc   = StGLProgram::getAttribLocation(theCtx, "vColor");
    return atrVVertexLoc.isValid() && atrVColorsLoc.isValid();
}

void StColorProgram::setScaleTranslate(StGLContext&    theCtx,
                                       const StGLVec4& theScaleVec4,
                                       const StGLVec4& theTranslateVec4) {
    theCtx.core20fwd->glUniform4fv(uniScaleLoc,     1, theScaleVec4);
    theCtx.core20fwd->glUniform4fv(uniTranslateLoc, 1, theTranslateVec4);
}
