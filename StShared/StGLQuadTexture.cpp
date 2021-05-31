/**
 * Copyright © 2009-2020 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#include <StGLStereo/StGLQuadTexture.h>

#include <StGLCore/StGLCore11.h>
#include <StGL/StGLContext.h>
#include <stAssert.h>

StGLFrameTexture::StGLFrameTexture()
#if defined(GL_ES_VERSION_2_0)
: StGLTexture(GL_RGB),
#else
: StGLTexture(GL_RGB8),
#endif
  myDataSize(1.0f, 1.0f),
  myDisplayRatio(1.0f),
  myPAR(1.0f),
  myPanorama(StPanorama_OFF) {
    //
}

StGLFrameTextures::StGLFrameTextures()
: myParams(),
  myImgCM(StImage::ImgColor_RGB) {
    //
}

StGLFrameTextures::~StGLFrameTextures() {
    ST_ASSERT(!myTextures[0].isValid()
           && !myTextures[1].isValid()
           && !myTextures[2].isValid()
           && !myTextures[3].isValid(),
              "~StGLFrameTextures() with unreleased GL resources");
}

void StGLFrameTextures::release(StGLContext& theCtx) {
    myTextures[0].release(theCtx);
    myTextures[1].release(theCtx);
    myTextures[2].release(theCtx);
    myTextures[3].release(theCtx);
}

void StGLFrameTextures::increaseSize(StGLContext&      theCtx,
                                     StGLFrameTexture& theTexture,
                                     const GLsizei     theTextureSizeX,
                                     const GLsizei     theTextureSizeY) {
    // test existing size / new size
    /// TODO (Kirill Gavrilov#8) we can automatically reduce texture size here
    if(theTexture.isValid()) {
        if(theTexture.getSizeX() == theTextureSizeX
        && theTexture.getSizeY() == theTextureSizeY) {
            return;
        }

        if(theTexture.getTarget() != GL_TEXTURE_CUBE_MAP
        && theTexture.getSizeX() >= theTextureSizeX
        && theTexture.getSizeY() >= theTextureSizeY) {
            return;
        }
    }

    ST_DEBUG_LOG("Requested texture size (" + theTextureSizeX + 'x' + theTextureSizeY
                + ") larger than current texture size(" + theTexture.getSizeX() + 'x' + theTexture.getSizeY() + ')');
    const GLsizei anOriginalSizeX = theTexture.getSizeX();
    const GLsizei anOriginalSizeY = theTexture.getSizeY();

    const GLint aMaxTexDim = theCtx.getMaxTextureSize();
    GLsizei aNewSizeX = stMin(theTextureSizeX, GLsizei(aMaxTexDim));
    GLsizei aNewSizeY = stMin(theTextureSizeY, GLsizei(aMaxTexDim));
    if(!theCtx.arbNPTW) {
        aNewSizeX = getPowerOfTwo(theTextureSizeX, GLsizei(aMaxTexDim));
        aNewSizeY = getPowerOfTwo(theTextureSizeY, GLsizei(aMaxTexDim));
    }
    if((aNewSizeY != anOriginalSizeY)
    || (aNewSizeX != anOriginalSizeX)) {
        if(!theTexture.initTrash(theCtx, aNewSizeX, aNewSizeY)) {
            theTexture.initTrash(theCtx,
                                  (anOriginalSizeX > 0) ? anOriginalSizeX : 512,
                                  (anOriginalSizeY > 0) ? anOriginalSizeY : 512);
            ST_DEBUG_LOG("FAILED to Increase the texture size to (" + aNewSizeX + 'x' +  aNewSizeY + ")!");
        } else {
            ST_DEBUG_LOG("Increase the texture size to (" + aNewSizeX + 'x' +  aNewSizeY + ") success!");
        }
    } else {
        ST_DEBUG_LOG("Not possible to Increase the texture size!");
    }
}

void StGLFrameTextures::preparePlane(StGLContext&  theCtx,
                                     const size_t  thePlaneId,
                                     const GLsizei theSizeX,
                                     const GLsizei theSizeY,
                                     const GLint   theInternalFormat,
                                     const GLenum  theTarget) {

    StGLFrameTexture& aPlane = myTextures[thePlaneId];
    if(aPlane.getTextureFormat() != theInternalFormat
    || aPlane.getTarget()        != theTarget) {
        // wrong texture format
        aPlane.release(theCtx);
        aPlane.setTextureFormat(theInternalFormat);
        aPlane.setTarget(theTarget);
    }
    increaseSize(theCtx, aPlane, theSizeX, theSizeY);
}

void StGLFrameTextures::setMinMagFilter(StGLContext& theCtx,
                                        const GLenum theMinFilter,
                                        const GLenum theMagFilter) {
    myTextures[0].setMinMagFilter(theCtx, theMinFilter, theMagFilter);
    /// TODO (Kirill Gavrilov#4) investigate
    const GLenum aMinFilter = theMinFilter != GL_NEAREST ? theMinFilter : GL_LINEAR;
    const GLenum aMagFilter = theMagFilter != GL_NEAREST ? theMagFilter : GL_LINEAR;
    myTextures[1].setMinMagFilter(theCtx, aMinFilter, aMagFilter);
    myTextures[2].setMinMagFilter(theCtx, aMinFilter, aMagFilter);
    myTextures[3].setMinMagFilter(theCtx, aMinFilter, aMagFilter);
}

void StGLQuadTexture::setMinMagFilter(StGLContext& theCtx,
                                        const GLenum theMinFilter,
                                        const GLenum theMagFilter) {
    myTextures[FRONT_TEXTURE +  LEFT_TEXTURE].setMinMagFilter(theCtx, theMinFilter, theMagFilter);
    myTextures[FRONT_TEXTURE + RIGHT_TEXTURE].setMinMagFilter(theCtx, theMinFilter, theMagFilter);
    myTextures[BACK_TEXTURE  +  LEFT_TEXTURE].setMinMagFilter(theCtx, theMinFilter, theMagFilter);
    myTextures[BACK_TEXTURE  + RIGHT_TEXTURE].setMinMagFilter(theCtx, theMinFilter, theMagFilter);
}

StGLQuadTexture::StGLQuadTexture()
: myActive(true) {
    //
}

StGLQuadTexture::~StGLQuadTexture() {
    ST_ASSERT(!myTextures[0].isValid()
           && !myTextures[1].isValid()
           && !myTextures[2].isValid()
           && !myTextures[3].isValid(),
              "~StGLQuadTexture() with unreleased GL resources");
}

void StGLQuadTexture::release(StGLContext& theCtx) {
    myTextures[0].release(theCtx);
    myTextures[1].release(theCtx);
    myTextures[2].release(theCtx);
    myTextures[3].release(theCtx);
}
