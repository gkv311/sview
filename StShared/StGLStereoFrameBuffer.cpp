/**
 * Copyright © 2009-2012 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#include <StGLStereo/StGLStereoFrameBuffer.h>

#include <StGLCore/StGLCore20.h>
#include <StGL/StGLArbFbo.h>
#include <StGL/StGLContext.h>

#include <StStrings/StLogger.h>
#include <stAssert.h>

bool StGLStereoFrameBuffer::StGLStereoProgram::link(StGLContext& theCtx) {
    if(!StGLProgram::link(theCtx)) {
        return false;
    }
    StGLVarLocation uniTexLLoc = StGLProgram::getUniformLocation(theCtx, "texL");
    StGLVarLocation uniTexRLoc = StGLProgram::getUniformLocation(theCtx, "texR");
    atrVVertexLoc   = StGLProgram::getAttribLocation(theCtx, "vVertex");
    atrVTexCoordLoc = StGLProgram::getAttribLocation(theCtx, "vTexCoord");

    if(uniTexLLoc.isValid() && uniTexRLoc.isValid()) {
        use(theCtx);
        theCtx.core20fwd->glUniform1i(uniTexLLoc, StGLProgram::TEXTURE_SAMPLE_0); // GL_TEXTURE0 in multitexture
        theCtx.core20fwd->glUniform1i(uniTexRLoc, StGLProgram::TEXTURE_SAMPLE_1); // GL_TEXTURE1 in multitexture
        unuse(theCtx);
    }

    return uniTexRLoc.isValid() && uniTexLLoc.isValid() && atrVVertexLoc.isValid() && atrVTexCoordLoc.isValid();
}

StGLStereoFrameBuffer::StGLStereoFrameBuffer()
: StGLStereoTexture(GL_RGBA8),
  myVerticesBuf(),
  myTexCoordBuf(),
  myViewPortX(0),
  myViewPortY(0) {
    myGLFBufferIds[StGLStereoTexture::LEFT_TEXTURE ] = StGLFrameBuffer::NO_FRAMEBUFFER;
    myGLFBufferIds[StGLStereoTexture::RIGHT_TEXTURE] = StGLFrameBuffer::NO_FRAMEBUFFER;
}

StGLStereoFrameBuffer::StGLStereoFrameBuffer(const GLint theTextureFormat)
: StGLStereoTexture(theTextureFormat),
  myVerticesBuf(),
  myTexCoordBuf(),
  myViewPortX(0),
  myViewPortY(0) {
    myGLFBufferIds[StGLStereoTexture::LEFT_TEXTURE ] = StGLFrameBuffer::NO_FRAMEBUFFER;
    myGLFBufferIds[StGLStereoTexture::RIGHT_TEXTURE] = StGLFrameBuffer::NO_FRAMEBUFFER;
}

StGLStereoFrameBuffer::~StGLStereoFrameBuffer() {
    ST_ASSERT(myGLFBufferIds[StGLStereoTexture::LEFT_TEXTURE]  == StGLFrameBuffer::NO_FRAMEBUFFER
           && myGLFBufferIds[StGLStereoTexture::RIGHT_TEXTURE] == StGLFrameBuffer::NO_FRAMEBUFFER
           && myGLDepthRBIds[StGLStereoTexture::LEFT_TEXTURE]  == StGLFrameBuffer::NO_RENDERBUFFER
           && myGLDepthRBIds[StGLStereoTexture::RIGHT_TEXTURE] == StGLFrameBuffer::NO_RENDERBUFFER
           && !myVerticesBuf.isValid()
           && !myTexCoordBuf.isValid(),
              "~StGLStereoFrameBuffer() with unreleased GL resources");
}

void StGLStereoFrameBuffer::release(StGLContext& theCtx) {
    // release texture
    StGLStereoTexture::release(theCtx);
    // release frame buffers
    if(myGLFBufferIds[StGLStereoTexture::LEFT_TEXTURE] != StGLFrameBuffer::NO_FRAMEBUFFER) {
        theCtx.arbFbo->glDeleteFramebuffers(1, &myGLFBufferIds[StGLStereoTexture::LEFT_TEXTURE]);
        myGLFBufferIds[StGLStereoTexture::LEFT_TEXTURE] = StGLFrameBuffer::NO_FRAMEBUFFER;
    }
    if(myGLFBufferIds[StGLStereoTexture::RIGHT_TEXTURE] != StGLFrameBuffer::NO_FRAMEBUFFER) {
        theCtx.arbFbo->glDeleteFramebuffers(1, &myGLFBufferIds[StGLStereoTexture::RIGHT_TEXTURE]);
        myGLFBufferIds[StGLStereoTexture::RIGHT_TEXTURE] = StGLFrameBuffer::NO_FRAMEBUFFER;
    }
    // release render buffers
    if(myGLDepthRBIds[StGLStereoTexture::LEFT_TEXTURE] != StGLFrameBuffer::NO_RENDERBUFFER) {
        theCtx.arbFbo->glDeleteRenderbuffers(1, &myGLDepthRBIds[StGLStereoTexture::LEFT_TEXTURE]);
        myGLDepthRBIds[StGLStereoTexture::LEFT_TEXTURE] = StGLFrameBuffer::NO_RENDERBUFFER;
    }
    if(myGLDepthRBIds[StGLStereoTexture::RIGHT_TEXTURE] != StGLFrameBuffer::NO_RENDERBUFFER) {
        theCtx.arbFbo->glDeleteRenderbuffers(1, &myGLDepthRBIds[StGLStereoTexture::RIGHT_TEXTURE]);
        myGLDepthRBIds[StGLStereoTexture::RIGHT_TEXTURE] = StGLFrameBuffer::NO_RENDERBUFFER;
    }
    // release VBOs
    myVerticesBuf.release(theCtx);
    myTexCoordBuf.release(theCtx);
}

bool StGLStereoFrameBuffer::init(StGLContext&  theCtx,
                                 const GLsizei theTextureSizeX,
                                 const GLsizei theTextureSizeY) {
    // release current objects
    release(theCtx);

    if(theCtx.arbFbo == NULL) {
        return false;
    }

    // create the textures
    if(!StGLStereoTexture::initTrash(theCtx, theTextureSizeX, theTextureSizeY)) {
        release(theCtx);
        return false;
    }

    // generate RenderBuffers (will be used as depth buffer)
    theCtx.arbFbo->glGenRenderbuffers(2, myGLDepthRBIds);

    // create left RenderBuffer (will be used as depth buffer)
    theCtx.arbFbo->glBindRenderbuffer(GL_RENDERBUFFER, myGLDepthRBIds[StGLStereoTexture::LEFT_TEXTURE]);
    theCtx.arbFbo->glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, theTextureSizeX, theTextureSizeY);

    // build FBOs
    theCtx.arbFbo->glGenFramebuffers(2, myGLFBufferIds);
    theCtx.arbFbo->glBindFramebuffer(GL_FRAMEBUFFER, myGLFBufferIds[StGLStereoTexture::LEFT_TEXTURE]);

    // bind left texture to left FBO as color buffer
    StGLStereoTexture::bindLeft(theCtx);
    theCtx.arbFbo->glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                                          StGLStereoTexture::myTextures[StGLStereoTexture::LEFT_TEXTURE].getTextureId(), 0);
    // bind left render buffer to left FBO as depth buffer
    theCtx.arbFbo->glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER,
                                             myGLFBufferIds[StGLStereoTexture::LEFT_TEXTURE]);
    if(theCtx.arbFbo->glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        release(theCtx);
        theCtx.arbFbo->glBindRenderbuffer(GL_RENDERBUFFER, StGLFrameBuffer::NO_RENDERBUFFER);
        return false;
    }
    unbindBufferLeft(theCtx);
    StGLStereoTexture::unbindLeft(theCtx);

    // create right RenderBuffer (will be used as depth buffer)
    theCtx.arbFbo->glBindRenderbuffer(GL_RENDERBUFFER, myGLDepthRBIds[StGLStereoTexture::RIGHT_TEXTURE]);
    theCtx.arbFbo->glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, theTextureSizeX, theTextureSizeY);

    theCtx.arbFbo->glBindFramebuffer(GL_FRAMEBUFFER, myGLFBufferIds[StGLStereoTexture::RIGHT_TEXTURE]);

    // bind right texture to rights FBO as color buffer
    StGLStereoTexture::bindRight(theCtx);
    theCtx.arbFbo->glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                                          StGLStereoTexture::myTextures[StGLStereoTexture::RIGHT_TEXTURE].getTextureId(), 0);

    // bind right render buffer to right FBO as depth buffer
    theCtx.arbFbo->glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER,
                                             myGLFBufferIds[StGLStereoTexture::RIGHT_TEXTURE]);
    if(theCtx.arbFbo->glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        release(theCtx);
        theCtx.arbFbo->glBindRenderbuffer(GL_RENDERBUFFER, StGLFrameBuffer::NO_RENDERBUFFER);
        return false;
    }
    unbindBufferRight(theCtx);
    StGLStereoTexture::unbindRight(theCtx);
    theCtx.arbFbo->glBindRenderbuffer(GL_RENDERBUFFER, StGLFrameBuffer::NO_RENDERBUFFER);

    ST_DEBUG_LOG("OpenGL, created StFrameBuffer(WxH= " + getSizeX() + 'x' + getSizeY() + ')');

    // create vertices buffers to draw simple textured quad
    StArray<StGLVec4> aQuad(4);
    aQuad[0] = StGLVec4( 1.0f, -1.0f, 0.0f, 1.0f); // top-right
    aQuad[1] = StGLVec4( 1.0f,  1.0f, 0.0f, 1.0f); // bottom-right
    aQuad[2] = StGLVec4(-1.0f, -1.0f, 0.0f, 1.0f); // top-left
    aQuad[3] = StGLVec4(-1.0f,  1.0f, 0.0f, 1.0f); // bottom-left
    myVerticesBuf.init(theCtx, aQuad);

    StArray<StGLVec2> aQuadTC(4);
    aQuadTC[0] = StGLVec2(1.0f, 0.0f);
    aQuadTC[1] = StGLVec2(1.0f, 1.0f);
    aQuadTC[2] = StGLVec2(0.0f, 0.0f);
    aQuadTC[3] = StGLVec2(0.0f, 1.0f);
    myTexCoordBuf.init(theCtx, aQuadTC);

    myViewPortX = theTextureSizeX;
    myViewPortY = theTextureSizeY;
    return true;
}

bool StGLStereoFrameBuffer::initLazy(StGLContext&  theCtx,
                                     const GLsizei theSizeX,
                                     const GLsizei theSizeY,
                                     const bool    theToCompress) {
    if(isValid()
    && (theSizeX <= getSizeX() && getSizeX() < theCtx.getMaxTextureSize())
    && (theSizeY <= getSizeY() && getSizeY() < theCtx.getMaxTextureSize())) {
        if(!theToCompress
        || (((getSizeX() - theSizeX) < 256)
        &&  ((getSizeY() - theSizeY) < 256))) {
            setVPDimensions(theCtx, theSizeX, theSizeY);
            return true;
        }
    }
    release(theCtx);

    GLsizei aSizeX = (GLsizei )getAligned(theSizeX, 256);
    GLsizei aSizeY = (GLsizei )getAligned(theSizeY, 256);
    if(!theCtx.stglIsRectangularFboSupported()) {
        StGLFrameBuffer::convertToPowerOfTwo(theCtx, aSizeX, aSizeY);
        ST_DEBUG_LOG("Ancient videocard detected (GLSL 1.1)!");
    }

    if(!init(theCtx, aSizeX, aSizeY)) {
        return false;
    }

    ST_DEBUG_LOG("FBO resized to " + aSizeX + " x " + aSizeY + " (for " + theSizeX + " x " + theSizeY + ")");

    setVPDimensions(theCtx, theSizeX, theSizeY);
    return true;
}

void StGLStereoFrameBuffer::setVPDimensions(StGLContext&  theCtx,
                                            const GLsizei theSizeX,
                                            const GLsizei theSizeY) {
    GLsizei aVPSizeX = stMin(theSizeX, getSizeX());
    GLsizei aVPSizeY = stMin(theSizeY, getSizeY());
    if(aVPSizeX != myViewPortX
    || aVPSizeY != myViewPortY) {
        GLfloat aDX = GLfloat(aVPSizeX) / GLfloat(getSizeX());
        GLfloat aDY = GLfloat(aVPSizeY) / GLfloat(getSizeY());
        StArray<StGLVec2> aTCoords(4);
        aTCoords[0] = StGLVec2(aDX,  0.0f);
        aTCoords[1] = StGLVec2(aDX,  aDY);
        aTCoords[2] = StGLVec2(0.0f, 0.0f);
        aTCoords[3] = StGLVec2(0.0f, aDY);
        myTexCoordBuf.init(theCtx, aTCoords);
        myViewPortX = aVPSizeX;
        myViewPortY = aVPSizeY;
    }
}

void StGLStereoFrameBuffer::setupViewPort(StGLContext& theCtx) {
    theCtx.core20fwd->glViewport(0, 0, myViewPortX, myViewPortY);
}

void StGLStereoFrameBuffer::drawQuad(StGLContext& theCtx,
                                     const StGLStereoFrameBuffer::StGLStereoProgram* theProgram) const {
    theProgram->use(theCtx);
    myVerticesBuf.bindVertexAttrib(theCtx, theProgram->getVVertexLoc());
    myTexCoordBuf.bindVertexAttrib(theCtx, theProgram->getVTexCoordLoc());
        theCtx.core20fwd->glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    myTexCoordBuf.unBindVertexAttrib(theCtx, theProgram->getVTexCoordLoc());
    myVerticesBuf.unBindVertexAttrib(theCtx, theProgram->getVVertexLoc());
    theProgram->unuse(theCtx);
}

void StGLStereoFrameBuffer::bindBufferLeft(StGLContext& theCtx) {
    theCtx.arbFbo->glBindFramebuffer(GL_FRAMEBUFFER, myGLFBufferIds[StGLStereoTexture::LEFT_TEXTURE]);
}

void StGLStereoFrameBuffer::bindBufferRight(StGLContext& theCtx) {
    theCtx.arbFbo->glBindFramebuffer(GL_FRAMEBUFFER, myGLFBufferIds[StGLStereoTexture::RIGHT_TEXTURE]);
}
