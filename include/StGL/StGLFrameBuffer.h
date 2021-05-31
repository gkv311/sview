/**
 * Copyright © 2009-2013 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#ifndef __StGLFrameBuffer_h_
#define __StGLFrameBuffer_h_

#include <StGL/StGLTexture.h>
#include <StTemplates/StHandle.h>

/**
 * Simple class represents Virtual (texture) stereo Frame buffer object.
 * This allow render to texture.
 */
class StGLFrameBuffer : public StGLResource {

        public:

    static const GLuint NO_FRAMEBUFFER  = 0;
    static const GLuint NO_RENDERBUFFER = 0;

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
    inline bool isValid() const {
        return isValidFrameBuffer()
            && !myTextureColor.isNull()
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
    ST_CPPEXPORT bool init(StGLContext&  theCtx,
                           const StHandle<StGLTexture>& theColorTexture,
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
                               const GLint   theTextureFormat,
                               const GLsizei theSizeX,
                               const GLsizei theSizeY,
                               const bool    theNeedDepthBuffer,
                               const bool    theToCompress = true);

    /**
     * @return texture width.
     */
    inline GLsizei getSizeX() const {
        return myTextureColor->getSizeX();
    }

    /**
     * @return texture height.
     */
    inline GLsizei getSizeY() const {
        return myTextureColor->getSizeY();
    }

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
     * Return color texture.
     */
    ST_LOCAL const StHandle<StGLTexture>& getTextureColor() const {
        return myTextureColor;
    }

    /**
     * Bind color texture (to render the texture).
     */
    inline void bindTexture(StGLContext& theCtx,
                            const GLenum theTextureUnit = GL_TEXTURE0) {
        myTextureColor->bind(theCtx, theTextureUnit);
    }

    /**
     * Unbind color texture.
     */
    inline void unbindTexture(StGLContext& theCtx) {
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
    ST_CPPEXPORT void detachColorTexture(StGLContext&                 theCtx,
                                         const StHandle<StGLTexture>& theTextureColor);

    /**
     * Clear texture using glClear call.
     */
    ST_CPPEXPORT void clearTexture(StGLContext& theCtx);

    /**
     * Create temporary FBO and clear specified texture using glClear call.
     */
    ST_CPPEXPORT static void clearTexture(StGLContext&                 theCtx,
                                          const StHandle<StGLTexture>& theTexture);

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

    StHandle<StGLTexture> myTextureColor;

    GLuint  myGLFBufferId; //!< FrameBuffer  object ID
    GLuint  myGLDepthRBId; //!< RenderBuffer object for depth ID
    GLsizei myViewPortX;   //!< FBO viewport width  <= texture width
    GLsizei myViewPortY;   //!< FBO viewport height <= texture height

};

#endif // __StGLFrameBuffer_h_
