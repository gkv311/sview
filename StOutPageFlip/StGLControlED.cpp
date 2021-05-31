/**
 * StOutPageFlip, class providing stereoscopic output for Shutter Glasses displays using StCore toolkit.
 * Copyright © 2010-2012 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#include "StGLControlED.h"

#include <StGL/StGLProgram.h>
#include <StGL/StGLContext.h>
#include <StGLCore/StGLCore20.h>

#include <stAssert.h>

class ST_LOCAL StGLControlED::StEDProgram : public StGLProgram {

        private:

    StGLVarLocation atrVVertexLoc;
    StGLVarLocation uniVPSizeYLoc;

        public:

    StEDProgram(const StString& title)
    : StGLProgram(title),
      atrVVertexLoc(),
      uniVPSizeYLoc() {
        //
    }

    virtual ~StEDProgram() {
        //
    }

    StGLVarLocation getVVertexLoc() const {
        return atrVVertexLoc;
    }

    virtual bool link(StGLContext& theCtx) ST_ATTR_OVERRIDE {
        if(!StGLProgram::link(theCtx)) {
            return false;
        }

        atrVVertexLoc = StGLProgram::getAttribLocation(theCtx, "vVertex");
        uniVPSizeYLoc = StGLProgram::getUniformLocation(theCtx, "uVPSizeY");

        return atrVVertexLoc.isValid() && uniVPSizeYLoc.isValid();
    }

    using StGLProgram::use;
    void use(StGLContext& theCtx, const GLint vpSizeY) {
        StGLProgram::use(theCtx);
        theCtx.core20fwd->glUniform1i(uniVPSizeYLoc, vpSizeY);
    }

};

const double StGLControlED::DELAY_MS = 500.0;

StGLControlED::StGLControlED()
: StGLDeviceControl(),
  myProgramOn(NULL),
  myProgramOff(NULL),
  myProgramBlack(NULL),
  myTimerCode(false),
  myTimerBlack(false) {
    //
}

StGLControlED::~StGLControlED() {
    ST_ASSERT(!myVertexBuf.isValid()
            && myProgramOn    == NULL
            && myProgramOff   == NULL
            && myProgramBlack == NULL,
              "~StGLControlED() with unreleased GL resources");
}

bool StGLControlED::stglInit(StGLContext& theCtx) {
    const GLfloat QUAD_VERTICES[4 * 4] = {
         1.0f,  1.0f, 0.0f, 1.0f, // top-right
         1.0f, -1.0f, 0.0f, 1.0f, // bottom-right
        -1.0f,  1.0f, 0.0f, 1.0f, // top-left
        -1.0f, -1.0f, 0.0f, 1.0f  // bottom-left
    };
    myVertexBuf.init(theCtx, 4, 4, QUAD_VERTICES);

    myProgramOn    = new StEDProgram("StGLControlED ON");
    myProgramOff   = new StEDProgram("StGLControlED OFF");
    myProgramBlack = new StEDProgram("StGLControlED Black");

    const char VERTEX_SHADER[] =
       "attribute vec4 vVertex; \
        void main(void) { \
            gl_Position = vVertex; \
        }";

    const char FRAGMENT_SHADER_ON[] =
       "uniform int uVPSizeY; \
        void main(void) { \
            int yFromTop = uVPSizeY - int(floor(gl_FragCoord.y - 0.5)) - 1; \
            if(yFromTop >= 0 && yFromTop <= 1) { \
                gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0); \
            } else if(yFromTop >= 2 && yFromTop <= 3) { \
                gl_FragColor = vec4(0.0, 1.0, 0.0, 1.0); \
            } else if(yFromTop >= 4 && yFromTop <= 7) { \
                gl_FragColor = vec4(1.0, 1.0, 0.0, 1.0); \
            } else if(yFromTop >= 8 && yFromTop <= 9) { \
                gl_FragColor = vec4(0.0, 1.0, 0.0, 1.0); \
            } else { \
                gl_FragColor = vec4(0.0, 0.0, 0.0, 0.0); \
            } \
        }";

    const char FRAGMENT_SHADER_OFF[] =
       "uniform int uVPSizeY; \
        void main(void) { \
            int yFromTop = uVPSizeY - int(floor(gl_FragCoord.y - 0.5)) - 1; \
            if(yFromTop >= 0 && yFromTop <= 1) { \
                gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0); \
            } else if(yFromTop >= 2 && yFromTop <= 3) { \
                gl_FragColor = vec4(0.0, 1.0, 0.0, 1.0); \
            } else if(yFromTop >= 4 && yFromTop <= 7) { \
                gl_FragColor = vec4(1.0, 1.0, 0.0, 1.0); \
            } else if(yFromTop >= 8 && yFromTop <= 9) { \
                gl_FragColor = vec4(0.0, 0.0, 0.0, 1.0); \
            } else { \
                gl_FragColor = vec4(0.0, 0.0, 0.0, 0.0); \
            } \
        }";

    const char FRAGMENT_SHADER_BLACK[] =
       "uniform int uVPSizeY; \
        void main(void) { \
            int yFromTop = uVPSizeY - int(floor(gl_FragCoord.y - 0.5)) - 1; \
            if(yFromTop >= 0 && yFromTop <= 9) { \
                gl_FragColor = vec4(0.0, 0.0, 0.0, 1.0); \
            } else { \
                gl_FragColor = vec4(0.0, 0.0, 0.0, 0.0); \
            } \
        }";

    StGLVertexShader aVertexShader("StGLControlED");
    StGLAutoRelease aTmp1(theCtx, aVertexShader);
    aVertexShader.init(theCtx, VERTEX_SHADER);

    StGLFragmentShader stFPageFlipOn(myProgramOn->getTitle());
    StGLAutoRelease aTmp2(theCtx, stFPageFlipOn);
    stFPageFlipOn.init(theCtx, FRAGMENT_SHADER_ON);
    myProgramOn->create(theCtx)
                .attachShader(theCtx, aVertexShader)
                .attachShader(theCtx, stFPageFlipOn)
                .link(theCtx);

    StGLFragmentShader stFPageFlipOff(myProgramOff->getTitle());
    StGLAutoRelease aTmp3(theCtx, stFPageFlipOff);
    stFPageFlipOff.init(theCtx, FRAGMENT_SHADER_OFF);
    myProgramOff->create(theCtx)
                 .attachShader(theCtx, aVertexShader)
                 .attachShader(theCtx, stFPageFlipOff)
                 .link(theCtx);

    StGLFragmentShader stFBlack(myProgramBlack->getTitle());
    StGLAutoRelease aTmp4(theCtx, stFBlack);
    stFBlack.init(theCtx, FRAGMENT_SHADER_BLACK);
    myProgramBlack->create(theCtx)
                   .attachShader(theCtx, aVertexShader)
                   .attachShader(theCtx, stFBlack)
                   .link(theCtx);

    return myProgramOn->isValid() && myProgramOff->isValid() && myProgramBlack->isValid();
}

void StGLControlED::release(StGLContext& theCtx) {
    myVertexBuf.release(theCtx);
    if(myProgramOn != NULL) {
        myProgramOn->release(theCtx);
        myProgramOff->release(theCtx);
        myProgramBlack->release(theCtx);
    }
    delete myProgramOn;    myProgramOn = NULL;
    delete myProgramOff;   myProgramOff = NULL;
    delete myProgramBlack; myProgramBlack = NULL;
}

void StGLControlED::stglDraw(StGLContext& theCtx,
                             unsigned int ,
                             const int    theWinWidth,
                             const int    theWinHeight) {
    if(!isActive() || myProgramOn  == NULL   || !myProgramOn->isValid()
                   || myProgramOff == NULL   || !myProgramOff->isValid()
                   || myProgramBlack == NULL || !myProgramBlack->isValid()) {
        return;
    }

    StEDProgram* aProgram = myTimerBlack.isOn() ? myProgramBlack : (isStereo() ? myProgramOn : myProgramOff);

    theCtx.core20fwd->glEnable(GL_SCISSOR_TEST);
    theCtx.core20fwd->glScissor(0, theWinHeight - 10, theWinWidth, 10);

    theCtx.core20fwd->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    theCtx.core20fwd->glEnable(GL_BLEND);
    aProgram->use(theCtx, theWinHeight);
    myVertexBuf.bindVertexAttrib(theCtx, aProgram->getVVertexLoc());
    theCtx.core20fwd->glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    myVertexBuf.unBindVertexAttrib(theCtx, aProgram->getVVertexLoc());
    aProgram->unuse(theCtx);
    theCtx.core20fwd->glDisable(GL_BLEND);

    theCtx.core20fwd->glDisable(GL_SCISSOR_TEST);
}
