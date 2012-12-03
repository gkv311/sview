/**
 * Copyright © 2009-2012 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#ifndef __StGLFrameBuffer_h_
#define __StGLFrameBuffer_h_

#include <StGL/StGLTexture.h>

/**
 * Simple class represents Virtual (texture) stereo Frame buffer object.
 * This allow render to texture.
 */
class ST_LOCAL StGLFrameBuffer : public StGLTexture {

        public:

    static const GLuint NO_FRAMEBUFFER  = 0;
    static const GLuint NO_RENDERBUFFER = 0;

    /**
     * Empty constructor.
     */
    StGLFrameBuffer();

    /**
     * Empty constructor.
     */
    StGLFrameBuffer(const GLint theTextureFormat);

    /**
     * Destructor - should be called after release()!
     */
    virtual ~StGLFrameBuffer();

    /**
     * Release OpenGL objects related to this FBO.
     */
    virtual void release(StGLContext& theCtx);

    /**
     * Returns true if FBO was initialized.
     */
    bool isValid() const {
        return isValidFrameBuffer() && StGLTexture::isValid();// && isValidDepthBuffer();
    }

    /**
     * Initialize the FBO with specified dimensions.
     */
    bool init(StGLContext&  theCtx,
              const GLsizei theSizeX,
              const GLsizei theSizeY);

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
     * Set new FBO viewport width. Should be <= texture width.
     */
    void setVPSizeX(const GLsizei theSizeX) {
        myViewPortX = theSizeX;
    }

    /**
     * Set new FBO viewport height. Should be <= texture height.
     */
    void setVPSizeY(const GLsizei theSizeY) {
        myViewPortY = theSizeY;
    }

    /**
     * Setup OpenGL viewport equal to FBO dimensions
     */
    void setupViewPort(StGLContext& theCtx);

    /**
     * Bind frame buffer (to render into the texture).
     */
    void bindBuffer(StGLContext& theCtx);

    /**
     * Unbind frame buffer.
     */
    void unbindBuffer(StGLContext& theCtx) {
        unbindBufferGlobal(theCtx);
    }

    /**
     * Bind zero frame buffer.
     */
    static void unbindBufferGlobal(StGLContext& theCtx);

    /**
     * Bind color texture (to render the texture).
     */
    void bindTexture(StGLContext& theCtx,
                     const GLenum theTextureUnit = GL_TEXTURE0) {
        StGLTexture::bind(theCtx, theTextureUnit);
    }

    /**
     * Unbind color texture.
     */
    void unbindTexture(StGLContext& theCtx) {
        StGLTexture::unbind(theCtx);
    }

    /**
     * Upscale input parameters to power of two values.
     */
    static void convertToPowerOfTwo(StGLContext& theCtx,
                                    GLsizei&     theFrSizeX,
                                    GLsizei&     theFrSizeY);

        private:

    /**
     * Validate FrameBuffer id.
     */
    bool isValidFrameBuffer() const {
        return myGLFBufferId != NO_FRAMEBUFFER;
    }

    /**
     * Validate RenderBuffer id.
     */
    bool isValidDepthBuffer() const {
        return myGLDepthRBId != NO_RENDERBUFFER;
    }

        private:

    GLuint  myGLFBufferId; //!< FrameBuffer  object ID
    GLuint  myGLDepthRBId; //!< RenderBuffer object for depth ID
    GLsizei myViewPortX;   //!< FBO viewport width  <= texture width
    GLsizei myViewPortY;   //!< FBO viewport height <= texture height

};

#endif //__StGLFrameBuffer_h_
