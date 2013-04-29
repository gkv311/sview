/**
 * Copyright © 2009-2013 Kirill Gavrilov <kirill@sview.ru>
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
class StGLFrameBuffer : public StGLTexture {

        public:

    static const GLuint NO_FRAMEBUFFER  = 0;
    static const GLuint NO_RENDERBUFFER = 0;

    /**
     * Empty constructor.
     */
    ST_CPPEXPORT StGLFrameBuffer();

    /**
     * Empty constructor.
     */
    ST_CPPEXPORT StGLFrameBuffer(const GLint theTextureFormat);

    /**
     * Destructor - should be called after release()!
     */
    ST_CPPEXPORT virtual ~StGLFrameBuffer();

    /**
     * Release OpenGL objects related to this FBO.
     */
    ST_CPPEXPORT virtual void release(StGLContext& theCtx);

    /**
     * Returns true if FBO was initialized.
     */
    inline bool isValid() const {
        return isValidFrameBuffer() && StGLTexture::isValid();// && isValidDepthBuffer();
    }

    /**
     * Initialize the FBO with specified dimensions.
     */
    ST_CPPEXPORT bool init(StGLContext&  theCtx,
                           const GLsizei theSizeX,
                           const GLsizei theSizeY,
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
     * Set new FBO viewport width. Should be <= texture width.
     */
    inline void setVPSizeX(const GLsizei theSizeX) {
        myViewPortX = theSizeX;
    }

    /**
     * Set new FBO viewport height. Should be <= texture height.
     */
    inline void setVPSizeY(const GLsizei theSizeY) {
        myViewPortY = theSizeY;
    }

    /**
     * Setup OpenGL viewport equal to FBO dimensions
     */
    ST_CPPEXPORT void setupViewPort(StGLContext& theCtx);

    /**
     * Bind frame buffer (to render into the texture).
     */
    ST_CPPEXPORT void bindBuffer(StGLContext& theCtx);

    /**
     * Unbind frame buffer.
     */
    inline void unbindBuffer(StGLContext& theCtx) {
        unbindBufferGlobal(theCtx);
    }

    /**
     * Bind zero frame buffer.
     */
    ST_CPPEXPORT static void unbindBufferGlobal(StGLContext& theCtx);

    /**
     * Bind color texture (to render the texture).
     */
    inline void bindTexture(StGLContext& theCtx,
                     const GLenum theTextureUnit = GL_TEXTURE0) {
        StGLTexture::bind(theCtx, theTextureUnit);
    }

    /**
     * Unbind color texture.
     */
    inline void unbindTexture(StGLContext& theCtx) {
        StGLTexture::unbind(theCtx);
    }

    /**
     * Upscale input parameters to power of two values.
     */
    ST_CPPEXPORT static void convertToPowerOfTwo(StGLContext& theCtx,
                                                 GLsizei&     theFrSizeX,
                                                 GLsizei&     theFrSizeY);

        private:

    /**
     * Validate FrameBuffer id.
     */
    inline bool isValidFrameBuffer() const {
        return myGLFBufferId != NO_FRAMEBUFFER;
    }

    /**
     * Validate RenderBuffer id.
     */
    inline bool isValidDepthBuffer() const {
        return myGLDepthRBId != NO_RENDERBUFFER;
    }

        private:

    GLuint  myGLFBufferId; //!< FrameBuffer  object ID
    GLuint  myGLDepthRBId; //!< RenderBuffer object for depth ID
    GLsizei myViewPortX;   //!< FBO viewport width  <= texture width
    GLsizei myViewPortY;   //!< FBO viewport height <= texture height

};

#endif //__StGLFrameBuffer_h_
