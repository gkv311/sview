/**
 * Copyright © 2009-2012 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#include <StGL/StGLTexture.h>
#include <StStrings/StLogger.h>
#include <StImage/StImagePlane.h>

#include <StGLCore/StGLCore20.h>
#include <StGL/StGLContext.h>

#include <StStrings/StLogger.h>
#include <stAssert.h>

bool StGLTexture::getInternalFormat(const StImagePlane& theData,
                                    GLint& theInternalFormat) {
    switch(theData.getFormat()) {
        case StImagePlane::ImgRGBAF:
        case StImagePlane::ImgBGRAF:
            theInternalFormat = GL_RGBA32F;
            return true;
        case StImagePlane::ImgRGBF:
        case StImagePlane::ImgBGRF:
            theInternalFormat = GL_RGB32F;
            return true;
        case StImagePlane::ImgRGBA:
        case StImagePlane::ImgBGRA:
            theInternalFormat = GL_RGBA8;
            return true;
        case StImagePlane::ImgRGB:
        case StImagePlane::ImgBGR:
        case StImagePlane::ImgRGB32:
        case StImagePlane::ImgBGR32:
            theInternalFormat = GL_RGB8;
            return true;
        case StImagePlane::ImgGrayF:
            //theInternalFormat = GL_R32F;    // OpenGL3+ hardware
            //theInternalFormat = GL_ALPHA32F_ARB;
            theInternalFormat = GL_ALPHA16; // backward compatibility
            return true;
        case StImagePlane::ImgGray:
            // this texture format is deprecated with OpenGL3+, use GL_R8 (GL_RED) instead
            //theInternalFormat = GL_R8;   // OpenGL3+ hardware
            theInternalFormat = GL_ALPHA8; // backward compatibility
            return true;
        default:
            return false;
    }
}

bool StGLTexture::getDataFormat(const StImagePlane& theData,
                                GLenum& thePixelFormat,
                                GLenum& theDataType) {
    thePixelFormat = GL_RGB;
    theDataType = GL_UNSIGNED_BYTE;
    switch(theData.getFormat()) {
        case StImagePlane::ImgGray: {
            // we fill ALPHA channel in the texture!
            //thePixelFormat = GL_RED;
            thePixelFormat = GL_ALPHA;
            theDataType = GL_UNSIGNED_BYTE;
            return true;
        }
        case StImagePlane::ImgRGB: {
            thePixelFormat = GL_RGB;
            theDataType = GL_UNSIGNED_BYTE;
            return true;
        }
        case StImagePlane::ImgBGR: {
            thePixelFormat = GL_BGR;
            theDataType = GL_UNSIGNED_BYTE;
            return true;
        }
        case StImagePlane::ImgRGBA:
        case StImagePlane::ImgRGB32: {
            thePixelFormat = GL_RGBA;
            theDataType = GL_UNSIGNED_BYTE;
            return true;
        }
        case StImagePlane::ImgBGRA:
        case StImagePlane::ImgBGR32: {
            thePixelFormat = GL_BGRA;
            theDataType = GL_UNSIGNED_BYTE;
            return true;
        }
        case StImagePlane::ImgGrayF: {
            //thePixelFormat = GL_RED;
            thePixelFormat = GL_ALPHA;
            theDataType = GL_FLOAT;
            return true;
        }
        case StImagePlane::ImgRGBF: {
            thePixelFormat = GL_RGB;
            theDataType = GL_FLOAT;
            return true;
        }
        case StImagePlane::ImgBGRF: {
            thePixelFormat = GL_BGR;
            theDataType = GL_FLOAT;
            return true;
        }
        case StImagePlane::ImgRGBAF: {
            thePixelFormat = GL_RGBA;
            theDataType = GL_FLOAT;
            return true;
        }
        case StImagePlane::ImgBGRAF: {
            thePixelFormat = GL_BGRA;
            theDataType = GL_FLOAT;
            return true;
        }
        default: return false;
    }
}

#ifdef __ST_DEBUG__
/**
 * Dummy function to display texture internal format.
 */
static StString formatInternalFormat(const GLint theInternalFormat) {
    switch(theInternalFormat) {
        // RED variations (GL_RED, OpenGL 3.0+)
        case GL_R8:       return "GL_R8";
        case GL_R16:      return "GL_R16";
        case GL_R16F:     return "GL_R16F"; // half-float
        case GL_R32F:     return "GL_R32F"; // float
        // RGB variations
        case GL_RGB4:     return "GL_RGB4";
        case GL_RGB5:     return "GL_RGB5";
        case GL_RGB8:     return "GL_RGB8";
        case GL_RGB10:    return "GL_RGB10";
        case GL_RGB12:    return "GL_RGB12";
        case GL_RGB16:    return "GL_RGB16";
        case GL_RGB16F:   return "GL_RGB16F"; // half-float
        case GL_RGB32F:   return "GL_RGB32F"; // float
        // RGBA variations
        case GL_RGBA8:    return "GL_RGBA8";
        case GL_RGB10_A2: return "GL_RGB10_A2";
        case GL_RGBA12:   return "GL_RGBA12";
        case GL_RGBA16:   return "GL_RGBA16";
        case GL_RGBA16F:  return "GL_RGBA16F"; // half-float
        case GL_RGBA32F:  return "GL_RGBA32F"; // float
        // ALPHA variations (deprecated)
        case GL_ALPHA8:   return "GL_ALPHA8";
        case GL_ALPHA16:  return "GL_ALPHA16";
        // unknown...
        default:          return StString("GL_? (") + theInternalFormat + ')';
    }
}
#endif

StGLTexture::StGLTexture()
: mySizeX(0),
  mySizeY(0),
  myTextFormat(GL_RGBA8),
  myTextureId(NO_TEXTURE),
  myTextureUnit(GL_TEXTURE0),
  myTextureFilt(GL_LINEAR) {
    //
}

StGLTexture::StGLTexture(const GLint theTextureFormat)
: mySizeX(0),
  mySizeY(0),
  myTextFormat(theTextureFormat),
  myTextureId(NO_TEXTURE),
  myTextureUnit(GL_TEXTURE0),
  myTextureFilt(GL_LINEAR) {
    //
}

StGLTexture::~StGLTexture() {
    ST_ASSERT(!isValid(), "~StGLTexture with unreleased GL resources");
}

void StGLTexture::release(StGLContext& theCtx) {
    if(isValid()) {
        theCtx.core20fwd->glDeleteTextures(1, &myTextureId);
        myTextureId = NO_TEXTURE;
    }
    mySizeX = mySizeY = 0;
}

void StGLTexture::bind(StGLContext& theCtx,
                       const GLenum theTextureUnit) {
    myTextureUnit = theTextureUnit;
    theCtx.core20fwd->glActiveTexture(theTextureUnit);
    theCtx.core20fwd->glBindTexture(GL_TEXTURE_2D, myTextureId);
}

void StGLTexture::unbindGlobal(StGLContext& theCtx,
                               const GLenum theTextureUnit) {
    theCtx.core20fwd->glActiveTexture(theTextureUnit);
    theCtx.core20fwd->glBindTexture(GL_TEXTURE_2D, NO_TEXTURE);
}

bool StGLTexture::init(StGLContext&   theCtx,
                       const GLsizei  theTextureSizeX,
                       const GLsizei  theTextureSizeY,
                       const GLenum   theDataFormat,
                       const GLubyte* theData) {
    // check texture size is fit dimension maximum
    const GLint aMaxTexDim = theCtx.getMaxTextureSize();
    if(theTextureSizeX < 32 || theTextureSizeY < 32) {
        ST_DEBUG_LOG("Texture size X or Y ("  + theTextureSizeX + " x " + theTextureSizeY + ") lesser than minimum dimension (32)!");
        return false;
    } else if(theTextureSizeX > aMaxTexDim && theTextureSizeY > aMaxTexDim) {
        ST_DEBUG_LOG("Texture size X and Y (" + theTextureSizeX + " x " + theTextureSizeY + ") greater than maximum dimension (" + aMaxTexDim + ")!");
        return false;
    } else if(theTextureSizeX > aMaxTexDim) {
        ST_DEBUG_LOG("Texture size X (" + theTextureSizeX + ") greater than maximal dimension (" + aMaxTexDim + ")!");
        return false;
    } else if(theTextureSizeY > aMaxTexDim) {
        ST_DEBUG_LOG("Texture size Y (" + theTextureSizeY + ") greater than maximal dimension (" + aMaxTexDim + ")!");
        return false;
    }

    mySizeX = theTextureSizeX;
    mySizeY = theTextureSizeY;

    return create(theCtx, theDataFormat, theData);
}

bool StGLTexture::initBlack(StGLContext&  theCtx,
                            const GLsizei theTextureSizeX,
                            const GLsizei theTextureSizeY) {
    size_t aDataSize = 4 * theTextureSizeX * theTextureSizeY;
    GLubyte* aBlackData = new GLubyte[aDataSize];
    stMemSet(aBlackData, 0, aDataSize);
    bool aResult = init(theCtx, theTextureSizeX, theTextureSizeY, GL_RGBA, aBlackData);
    delete[] aBlackData;
    return aResult;
}

bool StGLTexture::initTrash(StGLContext&  theCtx,
                            const GLsizei theTextureSizeX,
                            const GLsizei theTextureSizeY) {
    return init(theCtx, theTextureSizeX, theTextureSizeY, GL_RGBA, NULL);
}

bool StGLTexture::isProxySuccess(StGLContext& theCtx) {
    // use proxy to check texture could be created or not
    theCtx.core20fwd->glTexImage2D(GL_PROXY_TEXTURE_2D, 0, myTextFormat,
                                   mySizeX, mySizeY, 0,
                                   GL_RGBA, GL_UNSIGNED_BYTE, NULL); // no mention (we check graphical RAM here)
    GLint aTestParamX = 0;
    GLint aTestParamY = 0;
    theCtx.core20fwd->glGetTexLevelParameteriv(GL_PROXY_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &aTestParamX);
    theCtx.core20fwd->glGetTexLevelParameteriv(GL_PROXY_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &aTestParamY);
    if(aTestParamX == 0 || aTestParamY == 0) {
        ST_DEBUG_LOG("Creation texture with size (" + mySizeX + " x " + mySizeY + ") FAILED!");
        return false;
    }
    return true;
}

bool StGLTexture::create(StGLContext&   theCtx,
                         const GLenum   theDataFormat,
                         const GLubyte* theData) {
    if(!isValid()) {
        theCtx.core20fwd->glGenTextures(1, &myTextureId); // Create The Texture
    }
    bind(theCtx);

    // texture interpolation parameters - could be overridden later
    theCtx.core20fwd->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, myTextureFilt);
    theCtx.core20fwd->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, myTextureFilt);

    if(!isProxySuccess(theCtx)) {
        release(theCtx);
        return false;
    }

    theCtx.core20fwd->glTexImage2D(GL_TEXTURE_2D, 0, myTextFormat,
                                   mySizeX, mySizeY, 0,
                                   theDataFormat, GL_UNSIGNED_BYTE, theData);

    // detect which texture was actually created
    GLint aResFormat = 0;
    GLint aResSizeX  = 0;
    GLint aResSizeY  = 0;
    theCtx.core20fwd->glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH,           &aResSizeX);
    theCtx.core20fwd->glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT,          &aResSizeY);
    theCtx.core20fwd->glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_INTERNAL_FORMAT, &aResFormat);
#ifdef __ST_DEBUG_TEXTURES__
    ST_DEBUG_LOG("Created StGLTexture " + aResSizeX + " x "+ aResSizeY
               + " (format " + formatInternalFormat(aResFormat) + ')');
#endif

    unbind(theCtx);
    return true;
}

void StGLTexture::setMinMagFilter(StGLContext& theCtx,
                                  const GLenum theMinMagFilter) {
    if(!isValid()) {
        myTextureFilt = theMinMagFilter;
        return;
    } else if(myTextureFilt == theMinMagFilter) {
        return;
    }
    myTextureFilt = theMinMagFilter;
    bind(theCtx);
        theCtx.core20fwd->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, myTextureFilt);
        theCtx.core20fwd->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, myTextureFilt);
    unbind(theCtx);
}

bool StGLTexture::init(StGLContext&        theCtx,
                       const StImagePlane& theData) {
    if(theData.isNull()) {
        return false;
    }
    if(!initTrash(theCtx, (GLsizei )theData.getSizeX(), (GLsizei )theData.getSizeY())) {
        return false;
    }
    return fill(theCtx, theData);
}

bool StGLTexture::fill(StGLContext&        theCtx,
                       const StImagePlane& theData,
                       const GLsizei       theRowFrom,
                       const GLsizei       theRowTo,
                       const GLsizei       theBatchRows) {
    if(theData.isNull() || !isValid()) {
        return false;
    }
    GLenum aPixelFormat, aDataType;
    if(!getDataFormat(theData, aPixelFormat, aDataType)) {
        return false;
    }

    GLsizei aRowTo = GLsizei(stMin(theData.getSizeY(), size_t(getSizeY())));
    if(theRowTo > 0) {
        aRowTo = stMin(theRowTo, aRowTo);
    }

    if(theRowFrom >= aRowTo) {
        // out of range
        return false;
    }

    bind(theCtx);

    // setup the alignment
    size_t anExtraBytes = theData.getRowExtraBytes();
    size_t anAligment = stMin(theData.getMaxRowAligment(), size_t(8)); // limit to 8 bytes for OpenGL
    theCtx.core20fwd->glPixelStorei(GL_UNPACK_ALIGNMENT, GLint(anAligment));

    if(theData.getSizeX() <= size_t(getSizeX()) && anExtraBytes < anAligment && theBatchRows > 1) {
        // do batch copy (more effective)
        GLsizei aPatchWidth = GLsizei(theData.getSizeX());
        GLsizei aBatchRows = theBatchRows;
        for(GLsizei aRow(theRowFrom), aRowsRemain(aRowTo); aRow < aRowTo; aRow += theBatchRows) {
            aRowsRemain = aRowTo - aRow;
            if(aRowsRemain < aBatchRows) {
                aBatchRows = aRowsRemain;
            }

            theCtx.core20fwd->glTexSubImage2D(GL_TEXTURE_2D, 0, // 0 = LOD number
                                              0, aRow,          // a texel offset in the (x, y) direction
                                              aPatchWidth, aBatchRows,
                                              aPixelFormat,     // format of the pixel data
                                              aDataType,        // data type of the pixel data
                                              theData.getData(aRow, 0));
        }
    } else {
        // copy row by row copy (the image plane greater than texture or batch copy is impossible)
        GLsizei aPatchWidth = stMin(GLsizei(theData.getSizeX()), getSizeX());
        for(GLsizei aRow = theRowFrom; aRow < aRowTo; ++aRow) {
            theCtx.core20fwd->glTexSubImage2D(GL_TEXTURE_2D, 0, // 0 = LOD number
                                              0, aRow,          // a texel offset in the (x, y) direction
                                              aPatchWidth, 1,   // the (width, height) of the texture sub-image
                                              aPixelFormat,     // format of the pixel data
                                              aDataType,        // data type of the pixel data
                                              theData.getData(aRow, 0));
        }
    }

    // turn back safe alignment...
    theCtx.core20fwd->glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    unbind(theCtx);
    return true;
}
