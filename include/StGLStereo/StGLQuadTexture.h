/**
 * Copyright © 2009-2020 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
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
class StGLFrameTexture : public StGLTexture {

        public:

    ST_CPPEXPORT StGLFrameTexture();

    /**
     * @return data size in the texture.
     */
    ST_LOCAL const StGLVec2& getDataSize() { return myDataSize; }

    /**
     * @param theDataSize - set data size in the texture.
     */
    ST_LOCAL void setDataSize(const StGLVec2& theDataSize) { myDataSize = theDataSize; }

    /**
     * @return display aspect ratio.
     */
    ST_LOCAL GLfloat getDisplayRatio() const { return myDisplayRatio; }

    /**
     * @param theValue - display aspect ratio.
     */
    ST_LOCAL void setDisplayRatio(const GLfloat theValue) { myDisplayRatio = theValue; }

    /**
     * Returns Pixel Aspect Ratio.
     */
    ST_LOCAL GLfloat getPixelRatio() const { return myPAR; }

    /**
     * Sets Pixel Aspect Ratio.
     */
    ST_LOCAL void setPixelRatio(const GLfloat thePAR) { myPAR = thePAR; }

    /**
     * Returns packed panorama format.
     */
    ST_LOCAL StPanorama getPackedPanorama() const { return myPanorama; }

    /**
     * Sets packed panorama format.
     */
    ST_LOCAL void setPackedPanorama(StPanorama thePano) { myPanorama = thePano; }

        private:

    StGLVec2   myDataSize;                  //!< data size in the texture (x()=right and y()=bottom)
    float      myDisplayRatio = 1.0f;       //!< display aspect ratio
    float      myPAR = 1.0f;                //!< pixel aspect ratio
    StPanorama myPanorama = StPanorama_OFF; //!< packed panorama format

};

class StGLFrameTextures : public StGLResource {

        public:

    ST_CPPEXPORT StGLFrameTextures();

    ST_CPPEXPORT virtual ~StGLFrameTextures();

    /**
     * Release OpenGL texture objects.
     */
    ST_CPPEXPORT virtual void release(StGLContext& theCtx) ST_ATTR_OVERRIDE;

    ST_LOCAL std::shared_ptr<StStereoParams> getSource() const {
        return myParams;
    }

    ST_LOCAL void setSource(const std::shared_ptr<StStereoParams>& theParams) {
        myParams = theParams;
    }

    ST_LOCAL StImage::ImgColorScale getColorScale() const {
        return myImgScale;
    }

    ST_LOCAL StImage::ImgColorModel getColorModel() const {
        return myImgCM;
    }

    ST_LOCAL void setColorModel(const StImage::ImgColorModel theColorModel,
                                const StImage::ImgColorScale theColorScale) {
        myImgCM    = theColorModel;
        myImgScale = theColorScale;
    }

    /**
     * @return true if main data is valid.
     */
    ST_LOCAL bool isValid() const {
        return myTextures[0].isValid();
    }

    /**
     * @return sizeX (GLsizei ) - main texture width.
     */
    ST_LOCAL GLsizei getSizeX() const {
        return myTextures[0].getSizeX();
    }

    /**
     * @return sizeY (GLsizei ) - main texture height.
     */
    ST_LOCAL GLsizei getSizeY() const {
        return myTextures[0].getSizeY();
    }

    /**
     * Access to texture plane using id.
     */
    ST_LOCAL StGLFrameTexture& getPlane(size_t thePlaneId = 0) {
        ST_ASSERT(thePlaneId < 4, "StGLFrameTextures::getPlane() - out of range access");
        return myTextures[thePlaneId];
    }

    /**
     * Bind as multitexture.
     */
    ST_LOCAL void bind(StGLContext& theCtx,
                       const GLenum theTextureUnit = GL_TEXTURE0) {
        if(myTextures[0].isValid()) { myTextures[0].bind(theCtx, theTextureUnit);     }
        if(myTextures[1].isValid()) { myTextures[1].bind(theCtx, theTextureUnit + 1); }
        if(myTextures[2].isValid()) { myTextures[2].bind(theCtx, theTextureUnit + 2); }
        if(myTextures[3].isValid()) { myTextures[3].bind(theCtx, theTextureUnit + 3); }
    }

    /**
     * Unbind textures.
     */
    ST_LOCAL void unbind(StGLContext& theCtx) {
        if(myTextures[3].isValid()) { myTextures[3].unbind(theCtx); }
        if(myTextures[2].isValid()) { myTextures[2].unbind(theCtx); }
        if(myTextures[1].isValid()) { myTextures[1].unbind(theCtx); }
        if(myTextures[0].isValid()) { myTextures[0].unbind(theCtx); }
    }

    ST_CPPEXPORT void increaseSize(StGLContext&      theCtx,
                                   StGLFrameTexture& theTexture,
                                   const GLsizei     theTextureSizeX,
                                   const GLsizei     theTextureSizeY);

    ST_CPPEXPORT void preparePlane(StGLContext&  theCtx,
                                   const size_t  thePlaneId,
                                   const GLsizei theSizeX,
                                   const GLsizei theizeY,
                                   const GLint   theInternalFormat,
                                   const GLenum  theTarget);

    /**
     * Change Min and Mag filter.
     * After this call current bound texture will be undefined.
     */
    ST_CPPEXPORT void setMinMagFilter(StGLContext& theCtx,
                                      const GLenum theMinFilter,
                                      const GLenum theMagFilter);

    /**
     * Change Min and Mag filter.
     * After this call current bound texture will be undefined.
     */
    ST_LOCAL void setMinMagFilter(StGLContext& theCtx,
                                  const GLenum theMinMagFilter) {
        setMinMagFilter(theCtx, theMinMagFilter, theMinMagFilter);
    }

        private:

    std::shared_ptr<StStereoParams> myParams; //!< source pointer

    StGLFrameTexture       myTextures[4]; //!< texture planes
    StImage::ImgColorModel myImgCM    = StImage::ImgColor_RGB;  //!< color model
    StImage::ImgColorScale myImgScale = StImage::ImgScale_Full; //!< color scale

};

/**
 * This class represent float quad texture buffer. Consists of 4 lists of textures:
 *  - Front Left and Front Right (currently displayed);
 *  - Back Left and Back Right (prepared);
 * Float means each list is independent.
 */
class StGLQuadTexture : public StGLResource {

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

    ST_CPPEXPORT StGLQuadTexture();

    ST_CPPEXPORT virtual ~StGLQuadTexture();

    /**
     * Release OpenGL texture objects.
     */
    ST_CPPEXPORT virtual void release(StGLContext& theCtx) ST_ATTR_OVERRIDE;

    /**
     * @return texture (StGLFrameTextures& ) - texture from front pair (for rendering).
     */
    ST_LOCAL StGLFrameTextures& getFront(const LeftOrRight theLeftOrRight) {
        return myTextures[getFrontId() + theLeftOrRight];
    }

    /**
     * @return texture (StGLFrameTextures& ) - texture from back pair (for filling).
     */
    ST_LOCAL StGLFrameTextures& getBack(const LeftOrRight theLeftOrRight) {
        return myTextures[getBackId() + theLeftOrRight];
    }

    /**
     * Swap FRONT / BACK.
     */
    ST_LOCAL void swapFB() {
        myActive = !myActive; // process swap itself
    }

    /**
     * Change Min and Mag filter.
     * After this call current bound texture will be undefined.
     */
    ST_CPPEXPORT void setMinMagFilter(StGLContext& theCtx,
                                      const GLenum theMinFilter,
                                      const GLenum theMagFilter);

    /**
     * Change Min and Mag filter.
     * After this call current bound texture will be undefined.
     */
    ST_LOCAL void setMinMagFilter(StGLContext& theCtx,
                                  const GLenum theMinMagFilter) {
        setMinMagFilter(theCtx, theMinMagFilter, theMinMagFilter);
    }

        private:

    ST_LOCAL size_t getFrontId() const {
        return myActive ? FRONT_TEXTURE : BACK_TEXTURE;
    }

    ST_LOCAL size_t getBackId() const {
        return myActive ? BACK_TEXTURE : FRONT_TEXTURE;
    }

        private:

    StGLFrameTextures myTextures[4];   //!< Front and Back stereo textures
    bool              myActive = true; //!< Front/Back flag

};

#endif //__StGLQuadTexture_h_
