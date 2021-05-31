/**
 * Copyright © 2009-2013 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
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
class StGLStereoFrameBuffer : public StGLStereoTexture {

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

        ST_CPPEXPORT StGLStereoProgram(const StString& theTitle);

        ST_CPPEXPORT ~StGLStereoProgram();

        StGLVarLocation getVVertexLoc() const {
            return atrVVertexLoc;
        }

        StGLVarLocation getVTexCoordLoc() const {
            return atrVTexCoordLoc;
        }

        ST_CPPEXPORT virtual bool link(StGLContext& theCtx) ST_ATTR_OVERRIDE;

    };

        public:

    /**
     * Empty constructor with GL_RGBA8 texture.
     */
    ST_CPPEXPORT StGLStereoFrameBuffer();

    /**
     * Empty constructor.
     */
    ST_CPPEXPORT StGLStereoFrameBuffer(const GLint theTextureFormat);

    /**
     * OpenGL objects will be automatically released.
     * Make sure it is called from thread with bound GL context
     * or release the object in advance.
     */
    ST_CPPEXPORT virtual ~StGLStereoFrameBuffer();

    /**
     * Release OpenGL objects related to this FBO.
     */
    ST_CPPEXPORT virtual void release(StGLContext& theCtx) ST_ATTR_OVERRIDE;

    inline const StGLVertexBuffer& getQuadVertices() const {
        return myVerticesBuf;
    }

    inline const StGLVertexBuffer& getQuadTexCoords() const {
        return myTexCoordBuf;
    }

    /**
     * Returns true if FBO was initialized.
     */
    inline bool isValid() const {
        return isValidFrameBuffer() && StGLStereoTexture::isValid();// && isValidDepthBuffer();
    }

    /**
     * Initialize the FBO with specified dimensions.
     */
    ST_CPPEXPORT bool init(StGLContext&  theCtx,
                           const GLsizei theTextureSizeX,
                           const GLsizei theTextureSizeY,
                           const bool    theNeedDepthBuffer);

    /**
     * (Re)initialize the FBO with specified dimensions.
     * If FBO already initialized it will be reused when possible.
     * @param theSizeX           required width
     * @param theSizeY           required height
     * @param theNeedDepthBuffer request depth buffer
     * @param theToCompress      if set to true then FBO will be re-initialized with lesser dimensions
     */
    ST_CPPEXPORT bool initLazy(StGLContext&  theCtx,
                               const GLsizei theSizeX,
                               const GLsizei theSizeY,
                               const bool    theNeedDepthBuffer,
                               const bool    theToCompress = true);

    /**
     * FBO viewport width.
     */
    inline GLsizei getVPSizeX() const {
        return myViewPortX;
    }

    /**
     * FBO viewport height.
     */
    inline GLsizei getVPSizeY() const {
        return myViewPortY;
    }

    /**
     * Set new FBO viewport width x height. Should be <= texture dimensions.
     */
    ST_CPPEXPORT void setVPDimensions(StGLContext&  theCtx,
                                      const GLsizei theSizeX,
                                      const GLsizei theSizeY);

    /**
     * Setup OpenGL viewport equal to FBO dimensions
     */
    ST_CPPEXPORT void setupViewPort(StGLContext& theCtx);

    inline void bindTextureLeft(StGLContext& theCtx,
                                const GLenum theTextureUnit = GL_TEXTURE0) {
        StGLStereoTexture::bindLeft(theCtx, theTextureUnit);
    }

    inline void unbindTextureLeft(StGLContext& theCtx) {
        StGLStereoTexture::unbindLeft(theCtx);
    }

    inline void bindTextureRight(StGLContext& theCtx,
                                 const GLenum theTextureUnit = GL_TEXTURE0) {
        StGLStereoTexture::bindRight(theCtx, theTextureUnit);
    }

    inline void unbindTextureRight(StGLContext& theCtx) {
        StGLStereoTexture::unbindRight(theCtx);
    }

    /**
     * Bind left frame buffer (to render into the left texture).
     */
    ST_CPPEXPORT void bindBufferLeft(StGLContext& theCtx);

    inline void unbindBufferLeft(StGLContext& theCtx) {
        StGLFrameBuffer::unbindBufferGlobal(theCtx);
    }

    /**
     * Bind right frame buffer (to render into the right texture).
     */
    ST_CPPEXPORT void bindBufferRight(StGLContext& theCtx);

    inline void unbindBufferRight(StGLContext& theCtx) {
        StGLFrameBuffer::unbindBufferGlobal(theCtx);
    }

    inline void bindMultiTexture(StGLContext& theCtx,
                                 const GLenum theTextureUnit0 = GL_TEXTURE0,
                                 const GLenum theTextureUnit1 = GL_TEXTURE1) {
        bindTextureLeft (theCtx, theTextureUnit0);
        bindTextureRight(theCtx, theTextureUnit1);
    }

    inline void unbindMultiTexture(StGLContext& theCtx) {
        unbindTextureLeft(theCtx);
        unbindTextureRight(theCtx);
    }

    ST_CPPEXPORT void drawQuad(StGLContext& theCtx,
                               const StGLStereoFrameBuffer::StGLStereoProgram* theProgram) const;

        private:

    /**
     * Validate FrameBuffer ids.
     */
    inline bool isValidFrameBuffer() const {
        return myGLFBufferIds[StGLStereoTexture::LEFT_TEXTURE ] != StGLFrameBuffer::NO_FRAMEBUFFER
            && myGLFBufferIds[StGLStereoTexture::RIGHT_TEXTURE] != StGLFrameBuffer::NO_FRAMEBUFFER;
    }

    /**
     * Validate RenderBuffer ids.
     */
    inline bool isValidDepthBuffer() const {
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
