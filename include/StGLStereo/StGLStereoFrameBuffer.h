/**
 * Copyright © 2009-2012 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#ifndef __StGLStereoFrameBuffer_h_
#define __StGLStereoFrameBuffer_h_

#include <StGL/StGLVertexBuffer.h>
#include <StGL/StGLProgram.h>
#include <StGL/StGLFrameBuffer.h>
#include <StGLStereo/StGLStereoTexture.h>

/**
 * Simple class represents Virtual (texture) stereo Frame buffer object.
 * This allow render to texture.
 */
class ST_LOCAL StGLStereoFrameBuffer : public StGLStereoTexture {

        public:

    /**
     * Class represents simple stereo program.
     * Each program should contains:
     * - "attribute vec4 vVertex" in Vertex Shader (vertex coordinates);
     * - "attribute vec2 vTexCoord" in Vertex Shader (texture coordinates per vertex);
     * - "uniform sampler2D texR" in Fragment Shader (texture sample for Right view - texture should be bound to GL_TEXTURE0);
     * - "uniform sampler2D texL" in Fragment Shader (texture sample for Left view - texture should be bound to GL_TEXTURE1).
     */
    class StGLStereoProgram : public StGLProgram {

            private:

        StGLVarLocation atrVVertexLoc;
        StGLVarLocation atrVTexCoordLoc;

            public:

        StGLStereoProgram(const StString& title)
        : StGLProgram(title),
          atrVVertexLoc(),
          atrVTexCoordLoc() {
            //
        }

        StGLVarLocation getVVertexLoc() const {
            return atrVVertexLoc;
        }

        StGLVarLocation getVTexCoordLoc() const {
            return atrVTexCoordLoc;
        }

        virtual bool link(StGLContext& theCtx);

    };

        public:

    /**
     * Empty constructor with GL_RGBA8 texture.
     */
    StGLStereoFrameBuffer();

    /**
     * Empty constructor.
     */
    StGLStereoFrameBuffer(const GLint theTextureFormat);

    /**
     * OpenGL objects will be automatically released.
     * Make sure it is called from thread with bound GL context
     * or release the object in advance.
     */
    virtual ~StGLStereoFrameBuffer();

    /**
     * Release OpenGL objects related to this FBO.
     */
    virtual void release(StGLContext& theCtx);

    const StGLVertexBuffer& getQuadVertices() const {
        return myVerticesBuf;
    }

    const StGLVertexBuffer& getQuadTexCoords() const {
        return myTexCoordBuf;
    }

    /**
     * Returns true if FBO was initialized.
     */
    bool isValid() const {
        return isValidFrameBuffer() && StGLStereoTexture::isValid();// && isValidDepthBuffer();
    }

    /**
     * Initialize the FBO with specified dimensions.
     */
    bool init(StGLContext&  theCtx,
              const GLsizei theTextureSizeX,
              const GLsizei theTextureSizeY);

    /**
     * (Re)initialize the FBO with specified dimensions.
     * If FBO already initialized it will be reused when possible.
     * @param theSizeX      - required width
     * @param theSizeY      - required height
     * @param theToCompress - if set to true then FBO will be re-initialized with lesser dimensions
     */
    bool initLazy(StGLContext&  theCtx,
                  const GLsizei theSizeX,
                  const GLsizei theSizeY,
                  const bool    theToCompress = true);

    /**
     * FBO viewport width.
     */
    GLsizei getVPSizeX() const {
        return myViewPortX;
    }

    /**
     * FBO viewport height.
     */
    GLsizei getVPSizeY() const {
        return myViewPortY;
    }

    /**
     * Set new FBO viewport width x height. Should be <= texture dimensions.
     */
    void setVPDimensions(StGLContext&  theCtx,
                         const GLsizei theSizeX,
                         const GLsizei theSizeY);

    /**
     * Setup OpenGL viewport equal to FBO dimensions
     */
    void setupViewPort(StGLContext& theCtx);

    void bindTextureLeft(StGLContext& theCtx,
                         const GLenum theTextureUnit = GL_TEXTURE0) {
        StGLStereoTexture::bindLeft(theCtx, theTextureUnit);
    }

    void unbindTextureLeft(StGLContext& theCtx) {
        StGLStereoTexture::unbindLeft(theCtx);
    }

    void bindTextureRight(StGLContext& theCtx,
                          const GLenum theTextureUnit = GL_TEXTURE0) {
        StGLStereoTexture::bindRight(theCtx, theTextureUnit);
    }

    void unbindTextureRight(StGLContext& theCtx) {
        StGLStereoTexture::unbindRight(theCtx);
    }

    /**
     * Bind left frame buffer (to render into the left texture).
     */
    void bindBufferLeft(StGLContext& theCtx);

    void unbindBufferLeft(StGLContext& theCtx) {
        StGLFrameBuffer::unbindBufferGlobal(theCtx);
    }

    /**
     * Bind right frame buffer (to render into the right texture).
     */
    void bindBufferRight(StGLContext& theCtx);

    void unbindBufferRight(StGLContext& theCtx) {
        StGLFrameBuffer::unbindBufferGlobal(theCtx);
    }

    void bindMultiTexture(StGLContext& theCtx,
                          const GLenum theTextureUnit0 = GL_TEXTURE0,
                          const GLenum theTextureUnit1 = GL_TEXTURE1) {
        bindTextureLeft (theCtx, theTextureUnit0);
        bindTextureRight(theCtx, theTextureUnit1);
    }

    void unbindMultiTexture(StGLContext& theCtx) {
        unbindTextureLeft(theCtx);
        unbindTextureRight(theCtx);
    }

    void drawQuad(StGLContext& theCtx,
                  const StGLStereoFrameBuffer::StGLStereoProgram* theProgram) const;

        private:

    /**
     * Validate FrameBuffer ids.
     */
    bool isValidFrameBuffer() const {
        return myGLFBufferIds[StGLStereoTexture::LEFT_TEXTURE ] != StGLFrameBuffer::NO_FRAMEBUFFER
            && myGLFBufferIds[StGLStereoTexture::RIGHT_TEXTURE] != StGLFrameBuffer::NO_FRAMEBUFFER;
    }

    /**
     * Validate RenderBuffer ids.
     */
    bool isValidDepthBuffer() const {
        return myGLDepthRBIds[StGLStereoTexture::LEFT_TEXTURE ] != StGLFrameBuffer::NO_RENDERBUFFER
            && myGLDepthRBIds[StGLStereoTexture::RIGHT_TEXTURE] != StGLFrameBuffer::NO_RENDERBUFFER;
    }

        private:

    StGLVertexBuffer myVerticesBuf;     //!< buffers to draw simple fullsreen quad
    StGLVertexBuffer myTexCoordBuf;
    GLuint           myGLFBufferIds[2]; //!< FrameBuffer objects
    GLuint           myGLDepthRBIds[2]; //!< RenderBuffer objects for depth ID
    GLsizei          myViewPortX;       //!< FBO viewport width  <= texture width
    GLsizei          myViewPortY;       //!< FBO viewport height <= texture height

};

#endif //__StGLStereoFrameBuffer_h_
