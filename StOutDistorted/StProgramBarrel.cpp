/**
 * StOutDistorted, class providing stereoscopic output in anamorph side by side format using StCore toolkit.
 * Copyright © 2016 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#include "StProgramBarrel.h"

#include <StGLCore/StGLCore20.h>
#include <StGL/StGLContext.h>

StProgramBarrel::StProgramBarrel()
: StGLProgram("StProgramBarrel") {}

bool StProgramBarrel::init(StGLContext& theCtx) {
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
       "\n"
       "uniform vec4 uChromAb;\n"
       "uniform vec4 uWarpCoef;\n"
       "uniform vec2 uLensCenter;\n"
       "uniform vec2 uScale;\n"
       "uniform vec2 uScaleIn;\n"
       "\n"
       "void main(void) {\n"
       "  vec2 aTheta = (fTexCoord - uLensCenter) * uScaleIn;\n" // scales to [-1, 1]
       "  float rSq = aTheta.x * aTheta.x + aTheta.y * aTheta.y;\n"
       "  vec2 aTheta1 = aTheta * (uWarpCoef.x + uWarpCoef.y * rSq +\n"
       "                           uWarpCoef.z * rSq * rSq +\n"
       "                           uWarpCoef.w * rSq * rSq * rSq);\n"
       "  vec2 aThetaBlue = aTheta1 * (uChromAb.z + uChromAb.w * rSq);\n"
       "  vec2 aTCrdsBlue = uLensCenter + uScale * aThetaBlue;\n"
       "  if(any(bvec2(clamp(aTCrdsBlue, vec2(0.0, 0.0), vec2(1.0, 1.0)) - aTCrdsBlue))) {\n"
       "    gl_FragColor = vec4(0.0, 0.0, 0.0, 1.0);\n"
       "    return;\n"
       "  }\n"
       "\n"
       "  vec2 aTCrdsGreen = uLensCenter + uScale * aTheta1;\n"
       "  vec2 aThetaRed = aTheta1 * (uChromAb.x + uChromAb.y * rSq);\n"
       "  vec2 aTCrdsRed = uLensCenter + uScale * aThetaRed;\n"
       "  gl_FragColor = vec4(texture2D(texR, aTCrdsRed  ).r,\n"
       "                      texture2D(texR, aTCrdsGreen).g,\n"
       "                      texture2D(texR, aTCrdsBlue ).b, 1.0);\n"
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

    uniChromAbLoc    = StGLProgram::getUniformLocation(theCtx, "uChromAb");
    uniWarpCoeffLoc  = StGLProgram::getUniformLocation(theCtx, "uWarpCoef");
    uniLensCenterLoc = StGLProgram::getUniformLocation(theCtx, "uLensCenter");
    uniScaleLoc      = StGLProgram::getUniformLocation(theCtx, "uScale");
    uniScaleInLoc    = StGLProgram::getUniformLocation(theCtx, "uScaleIn");
    return true;
}

void StProgramBarrel::setupCoeff(StGLContext&    theCtx,
                                 const StGLVec4& theVec) {
    use(theCtx);
    theCtx.core20fwd->glUniform4fv(uniWarpCoeffLoc, 1, theVec);
    unuse(theCtx);
}

void StProgramBarrel::setupChrome(StGLContext&    theCtx,
                                  const StGLVec4& theVec) {
    use(theCtx);
    theCtx.core20fwd->glUniform4fv(uniChromAbLoc, 1, theVec);
    unuse(theCtx);
}

void StProgramBarrel::setLensCenter(StGLContext&    theCtx,
                                    const StGLVec2& theVec) {
    use(theCtx);
    theCtx.core20fwd->glUniform2fv(uniLensCenterLoc, 1, theVec);
    unuse(theCtx);
}

void StProgramBarrel::setScale(StGLContext&    theCtx,
                               const StGLVec2& theVec) {
    use(theCtx);
    theCtx.core20fwd->glUniform2fv(uniScaleLoc, 1, theVec);
    unuse(theCtx);
}

void StProgramBarrel::setScaleIn(StGLContext&    theCtx,
                                 const StGLVec2& theVec) {
    use(theCtx);
    theCtx.core20fwd->glUniform2fv(uniScaleInLoc, 1, theVec);
    unuse(theCtx);
}
