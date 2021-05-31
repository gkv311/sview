/**
 * Copyright © 2009-2013 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#include <StGL/StGLFrameBuffer.h>

#include <StGLCore/StGLCore11Fwd.h>
#include <StGL/StGLArbFbo.h>
#include <StGL/StGLContext.h>

#include <StStrings/StLogger.h>
#include <stAssert.h>

StGLFrameBuffer::StGLFrameBuffer()
: myGLFBufferId(NO_FRAMEBUFFER),
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
    if(!myTextureColor.isNull()) {
        myTextureColor->release(theCtx);
    }
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
    theCtx.stglResizeViewport(myViewPortX, myViewPortY);
}

void StGLFrameBuffer::bindBuffer(StGLContext& theCtx) {
    theCtx.stglBindFramebuffer(myGLFBufferId);
}

void StGLFrameBuffer::unbindBufferGlobal(StGLContext& theCtx) {
    theCtx.stglBindFramebuffer(NO_FRAMEBUFFER);
}

bool StGLFrameBuffer::initLazy(StGLContext&  theCtx,
                               const GLint   theTextureFormat,
                               const GLsizei theSizeX,
                               const GLsizei theSizeY,
                               const bool    theNeedDepthBuffer,
                               const bool    theToCompress) {
    if(isValid()
    && (theSizeX <= myTextureColor->getSizeX() && myTextureColor->getSizeX() < theCtx.getMaxTextureSize())
    && (theSizeY <= myTextureColor->getSizeY() && myTextureColor->getSizeY() < theCtx.getMaxTextureSize())) {
        if(!theToCompress
        || (((myTextureColor->getSizeX() - theSizeX) < 256)
        &&  ((myTextureColor->getSizeY() - theSizeY) < 256))) {
            setVPSizeX(theSizeX);
            setVPSizeY(theSizeY);
            return true;
        }
    }

    GLsizei aSizeX = stMax(32, (GLsizei )getAligned(theSizeX, 256));
    GLsizei aSizeY = stMax(32, (GLsizei )getAligned(theSizeY, 256));
    if(!theCtx.arbNPTW) {
        StGLFrameBuffer::convertToPowerOfTwo(theCtx, aSizeX, aSizeY);
    }

    if(!init(theCtx, theTextureFormat, aSizeX, aSizeY, theNeedDepthBuffer)) {
        return false;
    }

    theCtx.stglFillBitsFBO(myGLFBufferId, aSizeX, aSizeY);
    ST_DEBUG_LOG("FBO resized to " + aSizeX + " x " + aSizeY + " (for " + theSizeX + " x " + theSizeY + ")");

    setVPSizeX(stMin(aSizeX, theSizeX));
    setVPSizeY(stMin(aSizeY, theSizeY));
    return true;
}

bool StGLFrameBuffer::init(StGLContext&  theCtx,
                           const GLint   theTextureFormat,
                           const GLsizei theSizeX,
                           const GLsizei theSizeY,
                           const bool    theNeedDepthBuffer) {
    // create the texture
    if(myTextureColor.isNull()) {
        myTextureColor = new StGLTexture(theTextureFormat);
    }
    if(!myTextureColor->initTrash(theCtx, theSizeX, theSizeY)) {
        release(theCtx);
        return false;
    }
    return init(theCtx, myTextureColor, theNeedDepthBuffer);
}

bool StGLFrameBuffer::init(StGLContext&  theCtx,
                           const StHandle<StGLTexture>& theColorTexture,
                           const bool    theNeedDepthBuffer) {
    if(theColorTexture.isNull()
    || theCtx.arbFbo == NULL) {
        release(theCtx);
        return false;
    } else if(myTextureColor != theColorTexture) {
        if(!myTextureColor.isNull()) {
            myTextureColor->release(theCtx);
        }
    }

    const GLuint aFboBakDraw = theCtx.stglFramebufferDraw();
    const GLuint aFboBakRead = theCtx.stglFramebufferRead();
    theCtx.stglBindFramebuffer(NO_FRAMEBUFFER);

    if(theNeedDepthBuffer) {
        // create RenderBuffer (will be used as depth buffer)
        if(myGLDepthRBId == NO_RENDERBUFFER) {
            theCtx.arbFbo->glGenRenderbuffers(1, &myGLDepthRBId);
        }

        const GLint aDepthFormat = theCtx.isGlGreaterEqual(3, 0) ? GL_DEPTH24_STENCIL8 : GL_DEPTH_COMPONENT16;
        theCtx.arbFbo->glBindRenderbuffer(GL_RENDERBUFFER, myGLDepthRBId);
        theCtx.arbFbo->glRenderbufferStorage(GL_RENDERBUFFER, aDepthFormat,
                                             theColorTexture->getSizeX(), theColorTexture->getSizeY());
    } else if(myGLDepthRBId != NO_RENDERBUFFER) {
        theCtx.arbFbo->glDeleteRenderbuffers(1, &myGLDepthRBId);
        myGLDepthRBId = NO_RENDERBUFFER;
    }

    // build FBO and setup it as texture
    if(myGLFBufferId == NO_FRAMEBUFFER) {
        theCtx.arbFbo->glGenFramebuffers(1, &myGLFBufferId);
    }
    bindBuffer(theCtx);
    // attach texture to the FBO as color buffer
    theCtx.arbFbo->glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                                          theColorTexture->getTextureId(), 0);
    if(theNeedDepthBuffer) {
        // bind render buffer to the FBO as depth buffer
        theCtx.arbFbo->glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER,
                                                 myGLDepthRBId);
    }
    const bool isOk = theCtx.arbFbo->glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;
    if(myGLDepthRBId != NO_RENDERBUFFER) {
        theCtx.arbFbo->glBindRenderbuffer(GL_RENDERBUFFER, NO_RENDERBUFFER);
    }
    theCtx.stglBindFramebufferDraw(aFboBakDraw);
    theCtx.stglBindFramebufferRead(aFboBakRead);

    if(!isOk) {
        release(theCtx);
        return false;
    }

    myViewPortX    = theColorTexture->getSizeX();
    myViewPortY    = theColorTexture->getSizeY();
    myTextureColor = theColorTexture;
    return true;
}

void StGLFrameBuffer::detachColorTexture(StGLContext&                 theCtx,
                                         const StHandle<StGLTexture>& theTextureColor) {
    if(myGLFBufferId == NO_FRAMEBUFFER
    || theTextureColor.isNull()
    || myTextureColor != theTextureColor
    || !myTextureColor->isValid()) {
        return;
    }

    bindBuffer(theCtx);
    theCtx.arbFbo->glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                                          0, 0);
    unbindBuffer(theCtx);
    myTextureColor.nullify();
}

void StGLFrameBuffer::clearTexture(StGLContext& theCtx) {
    if(!isValid()) {
        return;
    }

    const GLuint aFboBakDraw = theCtx.stglFramebufferDraw();
    const GLuint aFboBakRead = theCtx.stglFramebufferRead();

    const StGLBoxPx aVPortBack = theCtx.stglViewport();
    setupViewPort(theCtx);
    if(theCtx.stglHasScissorRect()) {
        theCtx.core11fwd->glDisable(GL_SCISSOR_TEST);
    }

    bindBuffer(theCtx);
    //theCtx.core11fwd->glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    theCtx.core11fwd->glClear(GL_COLOR_BUFFER_BIT);
    theCtx.stglBindFramebufferDraw(aFboBakDraw);
    theCtx.stglBindFramebufferRead(aFboBakRead);

    theCtx.stglResizeViewport(aVPortBack);
    if(theCtx.stglHasScissorRect()) {
        theCtx.core11fwd->glEnable(GL_SCISSOR_TEST);
    }
}

void StGLFrameBuffer::clearTexture(StGLContext&                 theCtx,
                                   const StHandle<StGLTexture>& theTexture) {
    StGLFrameBuffer aFbo;
    if(!aFbo.init(theCtx, theTexture, false)) {
        return;
    }

    aFbo.clearTexture(theCtx);
    aFbo.detachColorTexture(theCtx, theTexture);
    aFbo.release(theCtx);
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
