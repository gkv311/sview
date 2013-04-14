/**
 * Copyright © 2009-2013 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#ifndef __StGLTexture_h_
#define __StGLTexture_h_

#include <stTypes.h>
#include <StGL/StGLResource.h>

class StImagePlane;
class StGLContext;

#define GL_TEXTURE0 0x84C0
#define GL_TEXTURE1 0x84C1

/**
 * This class represent OpenGL texture.
 */
class StGLTexture : public StGLResource {

        public:

    static const GLuint NO_TEXTURE = 0;

    /**
     * Function setup the best internal texture format to fit the image data.
     * Currently GL_ALPHA is returned for grayscale image data.
     * @return true if internal format was found.
     */
    ST_CPPEXPORT static bool getInternalFormat(const StImagePlane& theData,
                                               GLint& theInternalFormat);

    /**
     * Function convert StImagePlane format into OpenGL data format.
     * @return true if format supported.
     */
    ST_CPPEXPORT static bool getDataFormat(const StImagePlane& theData,
                                           GLenum& thePixelFormat,
                                           GLenum& theDataType);

    /**
     * Empty constructor for GL_RGBA8 format.
     */
    ST_CPPEXPORT StGLTexture();

    /**
     * Empty constructor.
     */
    ST_CPPEXPORT StGLTexture(const GLint theTextureFormat);

    /**
     * Destructor - should be called after release()!
     */
    ST_CPPEXPORT virtual ~StGLTexture();

    /**
     * Release GL resource.
     */
    ST_CPPEXPORT virtual void release(StGLContext& theCtx);

    bool operator==(const StGLTexture& theOther) const {
       return myTextureId == theOther.myTextureId;
    }

    /**
     * @return true if texture has valid ID (was created but not necessary initialized with valid data!).
     */
    inline bool isValid() const {
        return myTextureId != NO_TEXTURE;
    }

    /**
     * Get texture format.
     */
    inline GLint getTextureFormat() {
        return myTextFormat;
    }

    /**
     * Set texture format. Texture should be re-initialized after this change.
     */
    inline void setTextureFormat(GLint theTextureFormat) {
        myTextFormat = theTextureFormat;
    }

    /**
     * @param theCtx          current context
     * @param theTextureSizeX texture width
     * @param theTextureSizeY texture height
     * @param theDataFormat
     * @param theData
     * @return true on success
     */
    ST_CPPEXPORT bool init(StGLContext&   theCtx,
                           const GLsizei  theTextureSizeX,
                           const GLsizei  theTextureSizeY,
                           const GLenum   theDataFormat,
                           const GLubyte* theData);

    ST_CPPEXPORT bool init(StGLContext&        theCtx,
                           const StImagePlane& theData);

    ST_CPPEXPORT bool initBlack(StGLContext&  theCtx,
                                const GLsizei theTextureSizeX,
                                const GLsizei theTextureSizeY);

    /**
     * In GL version 1.1 or greater, theData may be a NULL pointer.
     * However data of the texture will be undefined!
     */
    ST_CPPEXPORT bool initTrash(StGLContext&  theCtx,
                                const GLsizei theTextureSizeX,
                                const GLsizei theTextureSizeY);

    /**
     * Bind the texture to specified unit.
     */
    ST_CPPEXPORT void bind(StGLContext& theCtx,
                           const GLenum theTextureUnit = GL_TEXTURE0);

    inline void unbind(StGLContext& theCtx) const {
        unbindGlobal(theCtx, myTextureUnit);
    }

    ST_CPPEXPORT static void unbindGlobal(StGLContext& theCtx,
                                          const GLenum theTextureUnit = GL_TEXTURE0);

    /**
     * @return texture width.
     */
    inline GLsizei getSizeX() const {
        return mySizeX;
    }

    /**
     * @return texture height.
     */
    inline GLsizei getSizeY() const {
        return mySizeY;
    }

    /**
     * Change Min and Mag filter.
     * After this call current texture will be undefined.
     */
    ST_CPPEXPORT void setMinMagFilter(StGLContext& theCtx,
                                      const GLenum theMinMagFilter);

    /**
     * Fill the texture with the image plane.
     * @param theCtx       current context
     * @param theData      the image plane to copy data from
     * @param theRowFrom   fill data from row (for both - input image plane and the texture!)
     * @param theRowTo     fill data up to the row (if zero - all rows)
     * @param theBatchRows maximal step for GL function call (greater - more effective)
     * @return true on success
     */
    ST_CPPEXPORT bool fill(StGLContext&        theCtx,
                           const StImagePlane& theData,
                           const GLsizei       theRowFrom   = 0,
                           const GLsizei       theRowTo     = 0,
                           const GLsizei       theBatchRows = 128);

    /**
     * @return GL texture ID.
     */
    inline GLuint getTextureId() const {
        return myTextureId;
    }

        protected:

    /**
     * Help function for GL texture creation.
     * @param theCtx        current context
     * @param theDataFormat data format
     * @param theData       initialization data (INPUT)
     * @return true on success
     */
    ST_CPPEXPORT bool create(StGLContext&   theCtx,
                             const GLenum   theDataFormat,
                             const GLubyte* theData);

    ST_CPPEXPORT bool isProxySuccess(StGLContext& theCtx);

        protected:

    GLsizei mySizeX;       //!< texture width
    GLsizei mySizeY;       //!< texture height
    GLint   myTextFormat;  //!< texture format - GL_RGB, GL_RGBA,...
    GLuint  myTextureId;   //!< GL texture ID
    GLenum  myTextureUnit; //!< texture unit
    GLenum  myTextureFilt; //!< current texture filter

        private:

    friend class StGLStereoTexture;
    friend class StGLStereoFrameBuffer;

};

#endif //__StGLTexture_h_
