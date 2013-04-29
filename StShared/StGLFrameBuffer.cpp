/**
 * Copyright © 2009-2012 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#include <StGL/StGLFrameBuffer.h>

#include <StGLCore/StGLCore11Fwd.h>
#include <StGL/StGLArbFbo.h>
#include <StGL/StGLContext.h>

#include <StStrings/StLogger.h>
#include <stAssert.h>

StGLFrameBuffer::StGLFrameBuffer()
: StGLTexture(GL_RGBA8),
  myGLFBufferId(NO_FRAMEBUFFER),
  myGLDepthRBId(NO_RENDERBUFFER),
  myViewPortX(0),
  myViewPortY(0) {
    //
}

StGLFrameBuffer::StGLFrameBuffer(const GLint theTextureFormat)
: StGLTexture(theTextureFormat),
  myGLFBufferId(NO_FRAMEBUFFER),
  myGLDepthRBId(NO_RENDERBUFFER),
  myViewPortX(0),
  myViewPortY(0) {
    //
}

StGLFrameBuffer::~StGLFrameBuffer() {
    ST_ASSERT(!isValidDepthBuffer() && !isValidFrameBuffer(),
              "~StGLFrameBuffer() with unreleased GL resources");
}

void StGLFrameBuffer::release(StGLContext& theCtx) {
    StGLTexture::release(theCtx);
    if(isValidDepthBuffer()) {
        theCtx.arbFbo->glDeleteRenderbuffers(1, &myGLDepthRBId);
        myGLDepthRBId = NO_RENDERBUFFER;
    }
    if(isValidFrameBuffer()) {
        theCtx.arbFbo->glDeleteFramebuffers(1, &myGLFBufferId);
        myGLFBufferId = NO_FRAMEBUFFER;
    }
}

void StGLFrameBuffer::setupViewPort(StGLContext& theCtx) {
    theCtx.core11fwd->glViewport(0, 0, myViewPortX, myViewPortY);
}

void StGLFrameBuffer::bindBuffer(StGLContext& theCtx) {
    theCtx.arbFbo->glBindFramebuffer(GL_FRAMEBUFFER, myGLFBufferId);
}

void StGLFrameBuffer::unbindBufferGlobal(StGLContext& theCtx) {
    theCtx.arbFbo->glBindFramebuffer(GL_FRAMEBUFFER, NO_FRAMEBUFFER);
}

bool StGLFrameBuffer::initLazy(StGLContext&  theCtx,
                               const GLsizei theSizeX,
                               const GLsizei theSizeY,
                               const bool    theNeedDepthBuffer,
                               const bool    theToCompress) {
    if(isValid()
    && (theSizeX <= getSizeX() && getSizeX() < theCtx.getMaxTextureSize())
    && (theSizeY <= getSizeY() && getSizeY() < theCtx.getMaxTextureSize())) {
        if(!theToCompress
        || (((getSizeX() - theSizeX) < 256)
        &&  ((getSizeY() - theSizeY) < 256))) {
            setVPSizeX(theSizeX);
            setVPSizeY(theSizeY);
            return true;
        }
    }
    release(theCtx);

    GLsizei aSizeX = stMax(32, (GLsizei )getAligned(theSizeX, 256));
    GLsizei aSizeY = stMax(32, (GLsizei )getAligned(theSizeY, 256));
    if(!theCtx.stglIsRectangularFboSupported()) {
        StGLFrameBuffer::convertToPowerOfTwo(theCtx, aSizeX, aSizeY);
        ST_DEBUG_LOG("Ancient videocard detected (GLSL 1.1)!");
    }

    if(!init(theCtx, aSizeX, aSizeY, theNeedDepthBuffer)) {
        return false;
    }

    ST_DEBUG_LOG("FBO resized to " + aSizeX + " x " + aSizeY + " (for " + theSizeX + " x " + theSizeY + ")");

    setVPSizeX(stMin(aSizeX, theSizeX));
    setVPSizeY(stMin(aSizeY, theSizeY));
    return true;
}

bool StGLFrameBuffer::init(StGLContext&  theCtx,
                           const GLsizei theSizeX,
                           const GLsizei theSizeY,
                           const bool    theNeedDepthBuffer) {
    // release current object
    release(theCtx);

    // create the texture
    if(!StGLTexture::initTrash(theCtx, theSizeX, theSizeY)) {
        release(theCtx);
        return false;
    }

    if(theNeedDepthBuffer) {
        // create RenderBuffer (will be used as depth buffer)
        theCtx.arbFbo->glGenRenderbuffers(1, &myGLDepthRBId);
        theCtx.arbFbo->glBindRenderbuffer(GL_RENDERBUFFER, myGLDepthRBId);
        theCtx.arbFbo->glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, theSizeX, theSizeY);
    }

    // build FBO and setup it as texture
    theCtx.arbFbo->glGenFramebuffers(1, &myGLFBufferId);
    bindBuffer(theCtx);
    // bind texture to the FBO as color buffer
    StGLTexture::bind(theCtx);
    theCtx.arbFbo->glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                                          StGLTexture::getTextureId(), 0);
    if(theNeedDepthBuffer) {
        // bind render buffer to the FBO as depth buffer
        theCtx.arbFbo->glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER,
                                                 myGLFBufferId);
    }
    if(theCtx.arbFbo->glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        release(theCtx);
        theCtx.arbFbo->glBindRenderbuffer(GL_RENDERBUFFER, NO_RENDERBUFFER);
        return false;
    }
    StGLTexture::unbind(theCtx);
    unbindBuffer(theCtx);
    theCtx.arbFbo->glBindRenderbuffer(GL_RENDERBUFFER, NO_RENDERBUFFER);

    myViewPortX = theSizeX;
    myViewPortY = theSizeY;
    return true;
}

void StGLFrameBuffer::convertToPowerOfTwo(StGLContext& theCtx,
                                          GLsizei&     theFrSizeX,
                                          GLsizei&     theFrSizeY) {
    // compute optimal values...
    GLint aMaxTexDim = theCtx.getMaxTextureSize();
    const GLsizei aSizeXGreater = getPowerOfTwo(theFrSizeX, GLsizei(aMaxTexDim));
    const GLsizei aSizeYGreater = getPowerOfTwo(theFrSizeY, GLsizei(aMaxTexDim));
    const GLsizei aSizeXLess = aSizeXGreater / 2;
    const GLsizei aSizeYLess = aSizeYGreater / 2;
    theFrSizeX = (aSizeXGreater + aSizeXLess - 2 * theFrSizeX) >= 0 ? aSizeXLess : aSizeXGreater;
    theFrSizeY = (aSizeYGreater + aSizeYLess - 2 * theFrSizeY) >= 0 ? aSizeYLess : aSizeYGreater;
}
