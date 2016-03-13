/**
 * This source is a part of sView program.
 *
 * Copyright © Kirill Gavrilov, 2016
 */

#include "StCADFrameBuffer.h"

#include <OpenGl_ArbFBO.hxx>

StCADFrameBuffer::StCADFrameBuffer()
: myVPortX(0),
  myVPortY(0),
  myIsFakeFboId(false) {}

StCADFrameBuffer::~StCADFrameBuffer() {}

void StCADFrameBuffer::BindBuffer(const Handle(OpenGl_Context)& theGlCtx) {
    GLuint anFboId = myIsFakeFboId ? NO_FRAMEBUFFER : myGlFBufferId;
    theGlCtx->arbFBO->glBindFramebuffer(GL_FRAMEBUFFER, anFboId);
    glViewport(myVPortX, myVPortY, myVPSizeX, myVPSizeY);
}

void StCADFrameBuffer::BindDrawBuffer(const Handle(OpenGl_Context)& theGlCtx) {
    GLuint anFboId = myIsFakeFboId ? NO_FRAMEBUFFER : myGlFBufferId;
    theGlCtx->arbFBO->glBindFramebuffer(GL_DRAW_FRAMEBUFFER, anFboId);
    glViewport(myVPortX, myVPortY, myVPSizeX, myVPSizeY);
}

void StCADFrameBuffer::BindReadBuffer(const Handle(OpenGl_Context)& theGlCtx) {
    GLuint anFboId = myIsFakeFboId ? NO_FRAMEBUFFER : myGlFBufferId;
    theGlCtx->arbFBO->glBindFramebuffer(GL_READ_FRAMEBUFFER, anFboId);
}

/*StCADFrameBuffer::SetupViewport(const Handle(OpenGl_Context)& theGlCtx) {
    glViewport(myVPortX, myVPortY, myVPSizeX, myVPSizeY);
}*/

void StCADFrameBuffer::wrapFbo(const StGLContext& theStCtx) {
    myNbSamples   = 0;
    myVPortX      = theStCtx.stglViewport().x();
    myVPortY      = theStCtx.stglViewport().y();
    myVPSizeX     = theStCtx.stglViewport().width();
    myVPSizeY     = theStCtx.stglViewport().height();
    myGlFBufferId = theStCtx.stglFramebufferDraw();
    if(myGlFBufferId == NO_FRAMEBUFFER) {
        myIsFakeFboId = true;
        myGlFBufferId = GLuint(-1);
    } else {
        myIsFakeFboId = false;
    }
    myIsOwnBuffer = false;
}
