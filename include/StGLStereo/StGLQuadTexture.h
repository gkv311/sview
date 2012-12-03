/**
 * Copyright © 2009-2012 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#ifndef __StGLQuadTexture_h_
#define __StGLQuadTexture_h_

#include "StGLStereoTexture.h"
#include <StImage/StImage.h>
#include <StGL/StParams.h>

/**
 * Frame texture extend GL texture with several attributes:
 *  - data size (represents data position and size in the texture);
 *  - data DAR (Display Aspect Ratio).
 */
class ST_LOCAL StGLFrameTexture : public StGLTexture {

        public:

    StGLFrameTexture();

    /**
     * @return data size in the texture.
     */
    const StGLVec2& getDataSize() {
        return myDataSize;
    }

    /**
     * @param theDataSize - setted data size in the texture.
     */
    void setDataSize(const StGLVec2& theDataSize) {
        myDataSize = theDataSize;
    }

    /**
     * @return display aspect ratio.
     */
    GLfloat getDisplayRatio() const {
        return myDisplayRatio;
    }

    /**
     * @param theValue - display aspect ratio.
     */
    void setDisplayRatio(const GLfloat theValue) {
        myDisplayRatio = theValue;
    }

        private:

    StGLVec2 myDataSize;     //!< data size in the texture (x()=right and y()=bottom)
    GLfloat  myDisplayRatio; //!< display aspect ratio

};

class ST_LOCAL StGLFrameTextures : public StGLResource {

        public:

    StGLFrameTextures();

    virtual ~StGLFrameTextures();

    /**
     * Release OpenGL texture objects.
     */
    virtual void release(StGLContext& theCtx);

    StHandle<StStereoParams> getSource() {
        return myParams;
    }

    void setSource(const StHandle<StStereoParams>& theParams) {
        myParams = theParams;
    }

    StImage::ImgColorModel getColorModel() const {
        return myImgCM;
    }

    void setColorModel(const StImage::ImgColorModel theColorModel) {
        myImgCM = theColorModel;
    }

    /**
     * @return true if main data is valid.
     */
    bool isValid() const {
        return myTextures[0].isValid();
    }

    /**
     * @return sizeX (GLsizei ) - main texture width.
     */
    GLsizei getSizeX() const {
        return myTextures[0].getSizeX();
    }

    /**
     * @return sizeY (GLsizei ) - main texture height.
     */
    GLsizei getSizeY() const {
        return myTextures[0].getSizeY();
    }

    /**
     * Access to texture plane using id.
     */
    StGLFrameTexture& getPlane(size_t thePlaneId = 0) {
        ST_DEBUG_ASSERT(thePlaneId < 4);
        return myTextures[thePlaneId];
    }

    /**
     * Bind as multitexture.
     */
    void bind(StGLContext& theCtx,
              const GLenum theTextureUnit = GL_TEXTURE0) {
        if(myTextures[0].isValid()) { myTextures[0].bind(theCtx, theTextureUnit);     }
        if(myTextures[1].isValid()) { myTextures[1].bind(theCtx, theTextureUnit + 1); }
        if(myTextures[2].isValid()) { myTextures[2].bind(theCtx, theTextureUnit + 2); }
        if(myTextures[3].isValid()) { myTextures[3].bind(theCtx, theTextureUnit + 3); }
    }

    /**
     * Unbind textures.
     */
    void unbind(StGLContext& theCtx) {
        if(myTextures[3].isValid()) { myTextures[3].unbind(theCtx); }
        if(myTextures[2].isValid()) { myTextures[2].unbind(theCtx); }
        if(myTextures[1].isValid()) { myTextures[1].unbind(theCtx); }
        if(myTextures[0].isValid()) { myTextures[0].unbind(theCtx); }
    }

    void increaseSize(StGLContext&      theCtx,
                      StGLFrameTexture& theTexture,
                      const GLsizei     theTextureSizeX,
                      const GLsizei     theTextureSizeY);

    void preparePlane(StGLContext&  theCtx,
                      const size_t  thePlaneId,
                      const GLsizei theSizeX,
                      const GLsizei theizeY,
                      const GLint   theInternalFormat);

    /**
     * Change Min and Mag filter.
     * After this call current bound texture will be undefined.
     */
    void setMinMagFilter(StGLContext& theCtx,
                         const GLenum theMinMagFilter);

        private:

    StHandle<StStereoParams> myParams;      //!< source pointer
    StGLFrameTexture         myTextures[4]; //!< texture planes
    StImage::ImgColorModel   myImgCM;       //!< color model

};

/**
 * This class represent float quad texture buffer. Consists of 4 lists of textures:
 *  - Front Left and Front Right (currently displayed);
 *  - Back Left and Back Right (prepared);
 * Float means each list is independent.
 */
class ST_LOCAL StGLQuadTexture : public StGLResource {

        public:

    typedef enum tagFrontOrBack {
        FRONT_TEXTURE = 0,
        BACK_TEXTURE  = 2,
    } FrontOrBack;

    typedef enum tagLeftOrRight {
        LEFT_TEXTURE  = 0,
        RIGHT_TEXTURE = 1,
    } LeftOrRight;

        public:

    StGLQuadTexture();

    virtual ~StGLQuadTexture();

    /**
     * Release OpenGL texture objects.
     */
    virtual void release(StGLContext& theCtx);

    /**
     * @return texture (StGLFrameTextures& ) - texture from front pair (for rendering).
     */
    StGLFrameTextures& getFront(const LeftOrRight theLeftOrRight) {
        return myTextures[getFrontId() + theLeftOrRight];
    }

    /**
     * @return texture (StGLFrameTextures& ) - texture from back pair (for filling).
     */
    StGLFrameTextures& getBack(const LeftOrRight theLeftOrRight) {
        return myTextures[getBackId() + theLeftOrRight];
    }

    /**
     * Swap FRONT / BACK.
     */
    void swapFB() {
        myActive = !myActive; // process swap itself
    }

    /**
     * Change Min and Mag filter.
     * After this call current bound texture will be undefined.
     */
    void setMinMagFilter(StGLContext& theCtx,
                         const GLenum theMinMagFilter);

        private:

    inline size_t getFrontId() const {
        return myActive ? FRONT_TEXTURE : BACK_TEXTURE;
    }

    inline size_t getBackId() const {
        return myActive ? BACK_TEXTURE : FRONT_TEXTURE;
    }

        private:

    StGLFrameTextures myTextures[4]; //!< Front and Back stereo textures
    bool              myActive;      //!< Front/Back flag

};

#endif //__StGLQuadTexture_h_
