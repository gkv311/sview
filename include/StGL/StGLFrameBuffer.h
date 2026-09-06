/**
 * Copyright © 2009-2013 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#ifndef __StGLFrameBuffer_h_
#define __StGLFrameBuffer_h_

#include <StGL/StGLTexture.h>

#include <memory>

/**
 * Simple class represents Virtual (texture) stereo Frame buffer object.
 * This allow render to texture.
 */
class StGLFrameBuffer : public StGLResource {

        public:

    static constexpr GLuint NO_FRAMEBUFFER  = 0;
    static constexpr GLuint NO_RENDERBUFFER = 0;

    /**
     * Empty constructor.
     */
    ST_CPPEXPORT StGLFrameBuffer();

    /**
     * Destructor - should be called after release()!
     */
    ST_CPPEXPORT virtual ~StGLFrameBuffer();

    /**
     * Release OpenGL objects related to this FBO.
     */
    ST_CPPEXPORT virtual void release(StGLContext& theCtx) ST_ATTR_OVERRIDE;

    /**
     * Returns true if FBO was initialized.
     */
    ST_LOCAL bool isValid() const {
        return isValidFrameBuffer()
            && myTextureColor.get() != nullptr
            && myTextureColor->isValid();
    }

    /**
     * Initialize the FBO with specified dimensions.
     */
    ST_CPPEXPORT bool init(StGLContext&  theCtx,
                           const GLint   theTextureFormat,
                           const GLsizei theSizeX,
                           const GLsizei theSizeY,
                           const bool    theNeedDepthBuffer);

    /**
     * Initialize the FBO with specified color texture.
     */
    ST_CPPEXPORT bool init(StGLContext& theCtx,
                           const std::shared_ptr<StGLTexture>& theColorTexture,
                           const bool theNeedDepthBuffer);

    /**
     * (Re)initialize the FBO with specified dimensions.
     * If FBO already initialized it will be reused when possible.
     * @param theSizeX           required width
     * @param theSizeY           required height
     * @param theNeedDepthBuffer request depth buffer
     * @param theToCompress      if set to true then FBO will be re-initialized with lesser dimensions
     */
    ST_CPPEXPORT bool initLazy(StGLContext&  theCtx,
                               const GLint   theTextureFormat,
                               const GLsizei theSizeX,
                               const GLsizei theSizeY,
                               const bool    theNeedDepthBuffer,
                               const bool    theToCompress = true);

    /**
     * @return texture width.
     */
    ST_LOCAL GLsizei getSizeX() const {
        return myTextureColor->getSizeX();
    }

    /**
     * @return texture height.
     */
    ST_LOCAL GLsizei getSizeY() const {
        return myTextureColor->getSizeY();
    }

    /**
     * FBO viewport width.
     */
    ST_LOCAL GLsizei getVPSizeX() const {
        return myViewPortX;
    }

    /**
     * FBO viewport height.
     */
    ST_LOCAL GLsizei getVPSizeY() const {
        return myViewPortY;
    }

    /**
     * Set new FBO viewport width. Should be <= texture width.
     */
    ST_LOCAL void setVPSizeX(const GLsizei theSizeX) {
        myViewPortX = theSizeX;
    }

    /**
     * Set new FBO viewport height. Should be <= texture height.
     */
    ST_LOCAL void setVPSizeY(const GLsizei theSizeY) {
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
    ST_LOCAL void unbindBuffer(StGLContext& theCtx) {
        unbindBufferGlobal(theCtx);
    }

    /**
     * Bind zero frame buffer.
     */
    ST_CPPEXPORT static void unbindBufferGlobal(StGLContext& theCtx);

    /**
     * Return color texture.
     */
    ST_LOCAL const std::shared_ptr<StGLTexture>& getTextureColor() const {
        return myTextureColor;
    }

    /**
     * Bind color texture (to render the texture).
     */
    ST_LOCAL void bindTexture(StGLContext& theCtx,
                              const GLenum theTextureUnit = GL_TEXTURE0) {
        myTextureColor->bind(theCtx, theTextureUnit);
    }

    /**
     * Unbind color texture.
     */
    ST_LOCAL void unbindTexture(StGLContext& theCtx) {
        myTextureColor->unbind(theCtx);
    }

    /**
     * Upscale input parameters to power of two values.
     */
    ST_CPPEXPORT static void convertToPowerOfTwo(StGLContext& theCtx,
                                                 GLsizei&     theFrSizeX,
                                                 GLsizei&     theFrSizeY);

    /**
     * Detach texture from this FBO without destruction.
     */
    ST_CPPEXPORT void detachColorTexture(StGLContext& theCtx,
                                         const std::shared_ptr<StGLTexture>& theTextureColor);

    /**
     * Clear texture using glClear call.
     */
    ST_CPPEXPORT void clearTexture(StGLContext& theCtx);

    /**
     * Create temporary FBO and clear specified texture using glClear call.
     */
    ST_CPPEXPORT static void clearTexture(StGLContext& theCtx,
                                          const std::shared_ptr<StGLTexture>& theTexture);

        private:

    /**
     * Validate FrameBuffer id.
     */
    ST_LOCAL bool isValidFrameBuffer() const {
        return myGLFBufferId != NO_FRAMEBUFFER;
    }

    /**
     * Validate RenderBuffer id.
     */
    ST_LOCAL bool isValidDepthBuffer() const {
        return myGLDepthRBId != NO_RENDERBUFFER;
    }

        private:

    std::shared_ptr<StGLTexture> myTextureColor;

    GLuint  myGLFBufferId = NO_FRAMEBUFFER;  //!< FrameBuffer  object ID
    GLuint  myGLDepthRBId = NO_RENDERBUFFER; //!< RenderBuffer object for depth ID

    GLsizei myViewPortX = 0; //!< FBO viewport width  <= texture width
    GLsizei myViewPortY = 0; //!< FBO viewport height <= texture height

};

#endif // __StGLFrameBuffer_h_
