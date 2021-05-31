/**
 * StOutPageFlip, class providing stereoscopic output for Shutter Glasses displays using StCore toolkit.
 * Copyright © 2010-2012 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#include "StGLColoredLine.h"

#include <StGL/StGLProgram.h>
#include <StGL/StGLContext.h>
#include <StGLCore/StGLCore20.h>

#include <stAssert.h>

class ST_LOCAL StGLColoredLine::StColoredLineProgram : public StGLProgram {

        private:

    StGLVarLocation atrVVertexLoc;
    StGLVarLocation uniColorLoc;
    StGLVarLocation uniLineLenPxLoc;

        public:

    StColoredLineProgram()
    : StGLProgram("StColoredLine"),
      atrVVertexLoc(),
      uniColorLoc(),
      uniLineLenPxLoc() {
        //
    }

    virtual ~StColoredLineProgram() {
        //
    }

    StGLVarLocation getVVertexLoc() const {
        return atrVVertexLoc;
    }

    virtual bool init(StGLContext& theCtx) {
        const char VERTEX_SHADER[] =
           "attribute vec4 vVertex; \
            void main(void) { \
                gl_Position = vVertex; \
            }";

        const char FRAGMENT_SHADER[] =
           "uniform vec4 uColor; \
            uniform int uLineLenPx; \
            void main(void) { \
                int yFromBottom = int(floor(gl_FragCoord.y - 0.5)); \
                int xFromLeft = int(floor(gl_FragCoord.x - 0.5)); \
                if(yFromBottom == 0 && xFromLeft < uLineLenPx) { \
                    gl_FragColor = uColor; \
                } else { \
                    gl_FragColor = vec4(0.0, 0.0, 0.0, 0.0); \
                } \
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

        atrVVertexLoc   = StGLProgram::getAttribLocation(theCtx, "vVertex");
        uniColorLoc     = StGLProgram::getUniformLocation(theCtx, "uColor");
        uniLineLenPxLoc = StGLProgram::getUniformLocation(theCtx, "uLineLenPx");

        return atrVVertexLoc.isValid() && uniColorLoc.isValid() && uniLineLenPxLoc.isValid();
    }

    using StGLProgram::use;
    void use(StGLContext&    theCtx,
             const StGLVec4& theColorVec4,
             const GLint     theLineLen) {
        StGLProgram::use(theCtx);
        theCtx.core20fwd->glUniform4fv(uniColorLoc, 1, theColorVec4);
        theCtx.core20fwd->glUniform1i(uniLineLenPxLoc, theLineLen);
    }

};

StGLColoredLine::StGLColoredLine()
: StGLDeviceControl(),
  myProgram(NULL) {
    setWhiteColor();
}

StGLColoredLine::~StGLColoredLine() {
    ST_ASSERT(!myVertexBuf.isValid()
            && myProgram == NULL,
              "~StGLColoredLine() with unreleased GL resources");
}

bool StGLColoredLine::stglInit(StGLContext& theCtx) {
    const GLfloat QUAD_VERTICES[4 * 4] = {
         1.0f,  1.0f, 0.0f, 1.0f, // top-right
         1.0f, -1.0f, 0.0f, 1.0f, // bottom-right
        -1.0f,  1.0f, 0.0f, 1.0f, // top-left
        -1.0f, -1.0f, 0.0f, 1.0f  // bottom-left
    };
    myVertexBuf.init(theCtx, 4, 4, QUAD_VERTICES);

    myProgram = new StColoredLineProgram();
    return myProgram->init(theCtx);
}

void StGLColoredLine::release(StGLContext& theCtx) {
    myVertexBuf.release(theCtx);
    if(myProgram != NULL) {
        myProgram->release(theCtx);
    }
    delete myProgram;
    myProgram = NULL;
}

void StGLColoredLine::stglDraw(StGLContext& theCtx,
                               unsigned int theView,
                               const int    theWinWidth,
                               const int ) {
    if(!isActive() || myProgram == NULL || !myProgram->isValid()) {
        return;
    }

    GLint aLineLen = (theView == ST_DRAW_LEFT) ? (theWinWidth / 4) : (3 * theWinWidth / 4);

    theCtx.core20fwd->glEnable(GL_SCISSOR_TEST);
    theCtx.core20fwd->glScissor(0, 0, aLineLen, 1);

    theCtx.core20fwd->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    theCtx.core20fwd->glEnable(GL_BLEND);
    myProgram->use(theCtx, myLineColor, aLineLen);
        myVertexBuf.bindVertexAttrib(theCtx, myProgram->getVVertexLoc());
        theCtx.core20fwd->glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        myVertexBuf.unBindVertexAttrib(theCtx, myProgram->getVVertexLoc());
    myProgram->unuse(theCtx);
    theCtx.core20fwd->glDisable(GL_BLEND);

    theCtx.core20fwd->glDisable(GL_SCISSOR_TEST);
}
