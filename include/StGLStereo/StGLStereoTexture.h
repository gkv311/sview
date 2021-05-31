/**
 * Copyright © 2009-2013 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#ifndef __StGLStereoTexture_h_
#define __StGLStereoTexture_h_

#include <StGL/StGLTexture.h>
#include <StGL/StGLVec.h>

/**
 * This class represent OpenGL stereo multitexture (Left + Right).
 */
class StGLStereoTexture : public StGLResource {

        protected:

    enum {
        LEFT_TEXTURE  = 0,
        RIGHT_TEXTURE = 1,
    };

    inline void setTextureFormat(const GLint theTextureFormat) {
        myTextures[LEFT_TEXTURE ].setTextureFormat(theTextureFormat);
        myTextures[RIGHT_TEXTURE].setTextureFormat(theTextureFormat);
    }

        public:

    /**
     * Empty constructor.
     */
    StGLStereoTexture(const GLint theTextureFormat) {
        setTextureFormat(theTextureFormat);
    }

    /**
     * Destructor - should be called after release()!
     */
    virtual ~StGLStereoTexture() {
        //ST_ASSERT(!myTextures[LEFT_TEXTURE ].isValid()
        //       && !myTextures[RIGHT_TEXTURE].isValid(), "~StGLStereoTexture() with unreleased GL resources");
    }

    /**
     * Release GL resource.
     */
    virtual void release(StGLContext& theCtx) {
        myTextures[LEFT_TEXTURE ].release(theCtx);
        myTextures[RIGHT_TEXTURE].release(theCtx);
    }

    bool isValid() const {
        return myTextures[LEFT_TEXTURE].isValid() && myTextures[RIGHT_TEXTURE].isValid();
    }

    /**
     * @param theCtx          - active context
     * @param theTextureSizeX - each texture width
     * @param theTextureSizeY - each texture height
     * @param theDataFormat
     * @param theDataR
     * @param theDataL
     * @return true on success.
     */
    bool init(StGLContext&   theCtx,
              const GLsizei  theTextureSizeX,
              const GLsizei  theTextureSizeY,
              const GLenum   theDataFormat,
              const GLubyte* theDataR,
              const GLubyte* theDataL) {
        return myTextures[LEFT_TEXTURE ].init(theCtx, theTextureSizeX, theTextureSizeY, theDataFormat, theDataL)
            && myTextures[RIGHT_TEXTURE].init(theCtx, theTextureSizeX, theTextureSizeY, theDataFormat, theDataR);
    }

    bool initBlack(StGLContext&  theCtx,
                   const GLsizei theTextureSizeX,
                   const GLsizei theTextureSizeY) {
        return myTextures[LEFT_TEXTURE ].initBlack(theCtx, theTextureSizeX, theTextureSizeY)
            && myTextures[RIGHT_TEXTURE].initBlack(theCtx, theTextureSizeX, theTextureSizeY);
    }

    /**
     * In GL version 1.1 or greater, pData may be a NULL pointer.
     * However data of the texture will be undefined!
     */
    bool initTrash(StGLContext&  theCtx,
                   const GLsizei theTextureSizeX,
                   const GLsizei theTextureSizeY) {
        return myTextures[LEFT_TEXTURE ].initTrash(theCtx, theTextureSizeX, theTextureSizeY)
            && myTextures[RIGHT_TEXTURE].initTrash(theCtx, theTextureSizeX, theTextureSizeY);
    }

    /**
     * Bind left texture.
     */
    void bindLeft(StGLContext& theCtx,
                  const GLenum theTextureUnit = GL_TEXTURE0) {
        myTextures[LEFT_TEXTURE].bind(theCtx, theTextureUnit);
    }

    /**
     * Bind right texture.
     */
    void bindRight(StGLContext& theCtx,
                   const GLenum theTextureUnit = GL_TEXTURE0) {
        myTextures[RIGHT_TEXTURE].bind(theCtx, theTextureUnit);
    }

    void unbindLeft(StGLContext& theCtx) {
        myTextures[LEFT_TEXTURE].unbind(theCtx);
    }

    void unbindRight(StGLContext& theCtx) {
        myTextures[RIGHT_TEXTURE].unbind(theCtx);
    }

    /**
     * @return sizeX (GLsizei ) - texture width.
     */
    GLsizei getSizeX() const {
        return myTextures[LEFT_TEXTURE].getSizeX();
    }

    /**
     * @return sizeY (GLsizei ) - texture height.
     */
    GLsizei getSizeY() const {
        return myTextures[LEFT_TEXTURE].getSizeY();
    }

    /**
     * Change Min and Mag filter.
     * After this call current texture will be undefined.
     */
    void setMinMagFilter(StGLContext& theCtx,
                         const GLenum theMinMagFilter) {
        myTextures[LEFT_TEXTURE ].setMinMagFilter(theCtx, theMinMagFilter);
        myTextures[RIGHT_TEXTURE].setMinMagFilter(theCtx, theMinMagFilter);
    }

    /**
     * Return the left texture object.
     */
    const StGLTexture& getTextureLeft() const {
        return myTextures[LEFT_TEXTURE];
    }

    /**
     * Return the right texture object.
     */
    const StGLTexture& getTextureRight() const {
        return myTextures[RIGHT_TEXTURE];
    }

        protected:

    StGLTexture myTextures[2];

};

#endif //__StGLStereoTexture_h_
