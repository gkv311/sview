/**
 * Copyright © 2009-2019 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#include <StGL/StGLTexture.h>
#include <StStrings/StLogger.h>
#include <StImage/StImagePlane.h>

#include <StGLCore/StGLCore20.h>
#include <StGL/StGLArbFbo.h>
#include <StGL/StGLContext.h>

#include <StStrings/StLogger.h>
#include <stAssert.h>

#ifndef GL_R16
    #define GL_R16      0x822A
    #define GL_R16F     0x822D
    #define GL_R32F     0x822E
    #define GL_RGB16F   0x881B
    #define GL_RGBA32F  0x8814
    #define GL_RGB32F   0x8815
    #define GL_RGBA16F  0x881A
    #define GL_RGB16F   0x881B
    #define GL_RGB4     0x804F
    #define GL_RGB5     0x8050
    #define GL_RGB8     0x8051
    #define GL_RGB10    0x8052
    #define GL_RGB12    0x8053
    #define GL_RGB16    0x8054
    #define GL_RGBA8    0x8058
    #define GL_RGB10_A2 0x8059
    #define GL_RGBA12   0x805A
    #define GL_RGBA16   0x805B
    //#define GL_ALPHA4   0x803B
    #define GL_ALPHA8   0x803C
    #define GL_ALPHA16  0x803E

    #define GL_HALF_FLOAT                     0x140B
    #define GL_UNSIGNED_INT_2_10_10_10_REV    0x8368
#endif

bool StGLTexture::getInternalFormat(const StGLContext& theCtx,
                                    const StImagePlane::ImgFormat theFormat,
                                    GLint& theInternalFormat) {
    switch(theFormat) {
        case StImagePlane::ImgRGBAF:
        case StImagePlane::ImgBGRAF: // BGRA requires texture swizzling on OpenGL ES
            theInternalFormat = GL_RGBA32F;
            if(!theCtx.arbTexFloat) {
            #if defined(GL_ES_VERSION_2_0)
                return false;
            #else
                theInternalFormat = GL_RGBA8;
            #endif
            }
            return true;
        case StImagePlane::ImgRGBF:
        case StImagePlane::ImgBGRF: // BGR requires texture swizzling on OpenGL ES
            theInternalFormat = GL_RGB32F;
            if(!theCtx.arbTexFloat) {
            #if defined(GL_ES_VERSION_2_0)
                return false;
            #else
                theInternalFormat = GL_RGB8;
            #endif
            }
            return true;
        case StImagePlane::ImgRGBA:
        case StImagePlane::ImgBGRA:
        #if defined(GL_ES_VERSION_2_0)
            theInternalFormat = GL_RGBA; // non-sized for compatibility with OpenGL ES 2.0
        #else
            theInternalFormat = GL_RGBA8;
        #endif
            return true;
        case StImagePlane::ImgRGBA64:
            theInternalFormat = theCtx.arbTexRG ? GL_RGBA16 : GL_RGBA8;
            return theCtx.extTexR16;
        case StImagePlane::ImgRGB: {
            theInternalFormat = GL_RGB8;
            return true;
        }
        case StImagePlane::ImgRGB32: {
        #if defined(GL_ES_VERSION_2_0)
            theInternalFormat = GL_RGBA8;
        #else
            theInternalFormat = GL_RGB8;
        #endif
            return true;
        }
        case StImagePlane::ImgBGR: {
            theInternalFormat = GL_RGB8;
        #if defined(GL_ES_VERSION_2_0)
            return false; // still can be done via swizzling...
        #else
            return true;
        #endif
        }
        case StImagePlane::ImgBGR32:
            theInternalFormat = GL_RGB8;
        #if defined(GL_ES_VERSION_2_0)
            if(theCtx.extTexBGRA8) {
                // GL_EXT_texture_format_BGRA8888 has been introduced for OpenGL ES 1.0
                // and does not defined sized format supported by OpenGL ES 3.0+
                theInternalFormat = GL_BGRA_EXT;
            } else {
                return false; // still can be done via swizzling...
            }
        #endif
            return true;
        case StImagePlane::ImgRGB48:
            theInternalFormat = GL_RGB16;
            return theCtx.extTexR16;
        case StImagePlane::ImgGrayF:
            theInternalFormat = GL_R32F;
            if(!theCtx.arbTexFloat || !theCtx.arbTexRG) {
            #if defined(GL_ES_VERSION_2_0)
                return false;
            #else
                // there is also GL_ALPHA32F_ARB
                theInternalFormat = theCtx.arbTexRG ? GL_R16 : GL_ALPHA16;
            #endif
            }
            return true;
        case StImagePlane::ImgGray16:
            theInternalFormat = GL_R16; // equals to GL_R16_EXT on OpenGL ES extension
        #if !defined(GL_ES_VERSION_2_0)
            theInternalFormat = theCtx.arbTexRG ? GL_R16 : GL_ALPHA16;
        #endif
            return theCtx.extTexR16;
        case StImagePlane::ImgGray:
        #if defined(GL_ES_VERSION_2_0)
            theInternalFormat = theCtx.arbTexRG ? GL_R8 : GL_ALPHA;
        #else
            theInternalFormat = theCtx.arbTexRG ? GL_R8 : GL_ALPHA8;
        #endif
            return true;
        case StImagePlane::ImgUV:
            //theInternalFormat = theCtx.arbTexRG ? GL_RG8 : GL_LUMINANCE_ALPHA; // OpenGL3+
            theInternalFormat = GL_LUMINANCE_ALPHA;
            return true;
        default:
            return false;
    }
}

bool StGLTexture::getDataFormat(const StGLContext&  theCtx,
                                const StImagePlane& theData,
                                GLenum& thePixelFormat,
                                GLenum& theDataType) {
    thePixelFormat = GL_RGB;
    theDataType    = GL_UNSIGNED_BYTE;
    switch(theData.getFormat()) {
        case StImagePlane::ImgGray: {
            thePixelFormat = theCtx.arbTexRG ? GL_RED : GL_ALPHA;
            theDataType = GL_UNSIGNED_BYTE;
            return true;
        }
        case StImagePlane::ImgGray16: {
            thePixelFormat = theCtx.arbTexRG ? GL_RED : GL_ALPHA;
            theDataType = GL_UNSIGNED_SHORT;
            return true;
        }
        case StImagePlane::ImgUV: {
            //thePixelFormat = theCtx.arbTexRG ? GL_RG : GL_LUMINANCE_ALPHA;
            thePixelFormat = GL_LUMINANCE_ALPHA;
            theDataType = GL_UNSIGNED_BYTE;
            return true;
        }
        case StImagePlane::ImgRGB: {
            thePixelFormat = GL_RGB;
            theDataType = GL_UNSIGNED_BYTE;
            return true;
        }
        case StImagePlane::ImgBGR: {
        #if defined(GL_ES_VERSION_2_0)
            return false; // no extension available
        #else
            thePixelFormat = GL_BGR;
            theDataType = GL_UNSIGNED_BYTE;
            return true;
        #endif
        }
        case StImagePlane::ImgRGB48: {
            thePixelFormat = GL_RGB;
            theDataType    = GL_UNSIGNED_SHORT;
            return true;
        }
        case StImagePlane::ImgRGBA:
        case StImagePlane::ImgRGB32: {
            thePixelFormat = GL_RGBA;
            theDataType = GL_UNSIGNED_BYTE;
            return true;
        }
        case StImagePlane::ImgRGBA64: {
            thePixelFormat = GL_RGBA;
            theDataType    = GL_UNSIGNED_SHORT;
            return true;
        }
        case StImagePlane::ImgBGRA:
        case StImagePlane::ImgBGR32: {
        #if defined(GL_ES_VERSION_2_0)
            if(!theCtx.extTexBGRA8) {
                return false;
            }
            thePixelFormat = GL_BGRA_EXT;
        #else
            thePixelFormat = GL_BGRA;
        #endif
            theDataType = GL_UNSIGNED_BYTE;
            return true;
        }
        case StImagePlane::ImgGrayF: {
            thePixelFormat = theCtx.arbTexRG ? GL_RED : GL_ALPHA;
            theDataType = GL_FLOAT;
            return true;
        }
        case StImagePlane::ImgRGBF: {
            thePixelFormat = GL_RGB;
            theDataType = GL_FLOAT;
            return true;
        }
        case StImagePlane::ImgBGRF: {
        #if defined(GL_ES_VERSION_2_0)
            return false;
        #else
            thePixelFormat = GL_BGR;
            theDataType = GL_FLOAT;
            return true;
        #endif
        }
        case StImagePlane::ImgRGBAF: {
            thePixelFormat = GL_RGBA;
            theDataType = GL_FLOAT;
            return true;
        }
        case StImagePlane::ImgBGRAF: {
        #if defined(GL_ES_VERSION_2_0)
            return false;
        #else
            thePixelFormat = GL_BGRA;
            theDataType = GL_FLOAT;
            return true;
        #endif
        }
        default: return false;
    }
}

bool StGLTexture::isAlphaFormat(const GLint theInternalFormat) {
    switch(theInternalFormat) {
        // RED variations (GL_RED, OpenGL 3.0+)
        case GL_RED:
        case GL_R8:
        case GL_R16:
        case GL_R16F:
        case GL_R32F:
        // ALPHA variations (deprecated)
        case GL_ALPHA:
        case GL_ALPHA8:
        case GL_ALPHA16:
            return true;
        default:
            return false;
    }
}

/**
 * Dummy function to display texture internal format.
 */
ST_LOCAL inline StString formatInternalFormat(const GLint theInternalFormat) {
    switch(theInternalFormat) {
        // RED variations (GL_RED, OpenGL 3.0+)
        case GL_RED:      return "GL_RED";
        case GL_R8:       return "GL_R8";
        case GL_R16:      return "GL_R16";
        case GL_R16F:     return "GL_R16F"; // half-float
        case GL_R32F:     return "GL_R32F"; // float
        // RGB variations
        case GL_RGB:      return "GL_RGB";
        case GL_RGB4:     return "GL_RGB4";
        case GL_RGB5:     return "GL_RGB5";
        case GL_RGB8:     return "GL_RGB8";
        case GL_RGB10:    return "GL_RGB10";
        case GL_RGB12:    return "GL_RGB12";
        case GL_RGB16:    return "GL_RGB16";
        case GL_RGB16F:   return "GL_RGB16F"; // half-float
        case GL_RGB32F:   return "GL_RGB32F"; // float
        // RGBA variations
        case GL_RGBA:     return "GL_RGBA";
        case GL_RGBA8:    return "GL_RGBA8";
        case GL_RGB10_A2: return "GL_RGB10_A2";
        case GL_RGBA12:   return "GL_RGBA12";
        case GL_RGBA16:   return "GL_RGBA16";
        case GL_RGBA16F:  return "GL_RGBA16F"; // half-float
        case GL_RGBA32F:  return "GL_RGBA32F"; // float
        //
        case GL_BGRA_EXT:  return "GL_BGRA";
        // ALPHA variations (deprecated)
        case GL_ALPHA:     return "GL_ALPHA";
        case GL_ALPHA8:    return "GL_ALPHA8";
        case GL_ALPHA16:   return "GL_ALPHA16";
        case GL_LUMINANCE: return "GL_LUMINANCE";
        case GL_LUMINANCE_ALPHA: return "GL_LUMINANCE_ALPHA";
        // unknown...
        default:          return StString("GL_? (") + theInternalFormat + ')';
    }
}

static inline GLenum getDataFormat(const GLint theInternalFormat) {
    switch(theInternalFormat) {
        // RED variations (GL_RED, OpenGL 3.0+)
        case GL_RED:
        case GL_R8:
        case GL_R16:
        case GL_R16F:
        case GL_R32F:
            return GL_RED;
        // RGB variations
        case GL_RGB:
        case GL_RGB4:
        case GL_RGB5:
        case GL_RGB8:
        case GL_RGB10:
        case GL_RGB12:
        case GL_RGB16:
        case GL_RGB16F:
        case GL_RGB32F:
            return GL_RGB;
        // RGBA variations
        case GL_RGBA:
        case GL_RGBA8:
        case GL_RGB10_A2:
        case GL_RGBA12:
        case GL_RGBA16:
        case GL_RGBA16F:
        case GL_RGBA32F:
            return GL_RGBA;
        case GL_BGRA_EXT:
            return GL_BGRA_EXT;
        // ALPHA variations (deprecated)
        case GL_ALPHA:
        case GL_ALPHA8:
        case GL_ALPHA16:
            return GL_ALPHA;
        case GL_LUMINANCE:
            return GL_LUMINANCE;
        case GL_LUMINANCE_ALPHA:
            return GL_LUMINANCE_ALPHA;
        // unknown...
        default:
            return GL_RGBA;
    }
}

/**
 * Return appropriate data type for specified internal format.
 */
static GLenum getDataType(GLint theInternalFormat) {
    switch(theInternalFormat) {
        case GL_RED:
        case GL_R8:
        case GL_RGB:
        case GL_RGB8:
        case GL_RGBA:
        case GL_RGBA8:
        case GL_BGRA_EXT:
        case GL_ALPHA:
        case GL_ALPHA8:
        case GL_LUMINANCE:
        case GL_LUMINANCE_ALPHA:
            return GL_UNSIGNED_BYTE;
        case GL_R16:
        case GL_RGB16:
        case GL_RGBA16:
        case GL_ALPHA16:
            return GL_UNSIGNED_SHORT;
        case GL_R16F:
        case GL_RGB16F:
        case GL_RGBA16F:
            return GL_HALF_FLOAT;
        case GL_R32F:
        case GL_RGB32F:
        case GL_RGBA32F:
            return GL_FLOAT;
        /*case GL_RGB4:
        case GL_RGB5:
        case GL_RGB10:
        case GL_RGB12:
        case GL_RGBA12:
            return GL_UNSIGNED_BYTE;*/
        case GL_RGB10_A2:
            return GL_UNSIGNED_INT_2_10_10_10_REV;
        default:
            return GL_UNSIGNED_BYTE;
    }
}

StGLTexture::StGLTexture()
: mySizeX(0),
  mySizeY(0),
  myTarget(GL_TEXTURE_2D),
  myTextFormat(GL_RGBA8),
  myTextureId(NO_TEXTURE),
  myTextureUnit(GL_TEXTURE0),
  myFilterMin(GL_LINEAR),
  myFilterMag(GL_LINEAR),
  myHasMipMaps(0) {
    //
}

StGLTexture::StGLTexture(const GLint theTextureFormat)
: mySizeX(0),
  mySizeY(0),
  myTarget(GL_TEXTURE_2D),
  myTextFormat(theTextureFormat),
  myTextureId(NO_TEXTURE),
  myTextureUnit(GL_TEXTURE0),
  myFilterMin(GL_LINEAR),
  myFilterMag(GL_LINEAR),
  myHasMipMaps(0) {
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
    theCtx.core20fwd->glBindTexture(myTarget, myTextureId);
}

void StGLTexture::unbind(StGLContext& theCtx) {
    theCtx.core20fwd->glActiveTexture(myTextureUnit);
    theCtx.core20fwd->glBindTexture(myTarget, NO_TEXTURE);
}

bool StGLTexture::init(StGLContext&   theCtx,
                       const GLsizei  theTextureSizeX,
                       const GLsizei  theTextureSizeY,
                       const GLenum   theDataFormat,
                       const GLubyte* theData) {
    // check texture size is fit dimension maximum
    const GLint aMaxTexDim = theCtx.getMaxTextureSize();
    if(theTextureSizeX < 16 || theTextureSizeY < 4) {
        ST_ERROR_LOG("Texture size X or Y ("  + theTextureSizeX + " x " + theTextureSizeY + ") lesser than minimum dimension (16)!");
        return false;
    } else if(theTextureSizeX > aMaxTexDim && theTextureSizeY > aMaxTexDim) {
        ST_ERROR_LOG("Texture size X and Y (" + theTextureSizeX + " x " + theTextureSizeY + ") greater than maximum dimension (" + aMaxTexDim + ")!");
        return false;
    } else if(theTextureSizeX > aMaxTexDim) {
        ST_ERROR_LOG("Texture size X (" + theTextureSizeX + ") greater than maximal dimension (" + aMaxTexDim + ")!");
        return false;
    } else if(theTextureSizeY > aMaxTexDim) {
        ST_ERROR_LOG("Texture size Y (" + theTextureSizeY + ") greater than maximal dimension (" + aMaxTexDim + ")!");
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
    return init(theCtx, theTextureSizeX, theTextureSizeY, ::getDataFormat(myTextFormat), NULL);
}

bool StGLTexture::isProxySuccess(StGLContext& theCtx) {
#if defined(GL_ES_VERSION_2_0)
    (void )theCtx;
    return true; // unavailable on OpenGL ES
#else
    // use proxy to check texture could be created or not
    const GLenum aTarget = myTarget == GL_TEXTURE_CUBE_MAP ? GL_PROXY_TEXTURE_CUBE_MAP : GL_PROXY_TEXTURE_2D;
    theCtx.core20fwd->glTexImage2D(aTarget, 0, myTextFormat,
                                   mySizeX, mySizeY, 0,
                                   GL_RGBA, GL_UNSIGNED_BYTE, NULL); // no mention (we check graphical RAM here)
    GLint aTestParamX = 0;
    GLint aTestParamY = 0;
    theCtx.core20fwd->glGetTexLevelParameteriv(aTarget, 0, GL_TEXTURE_WIDTH,  &aTestParamX);
    theCtx.core20fwd->glGetTexLevelParameteriv(aTarget, 0, GL_TEXTURE_HEIGHT, &aTestParamY);
    if(aTestParamX == 0 || aTestParamY == 0) {
        ST_DEBUG_LOG("Creation texture with size (" + mySizeX + " x " + mySizeY + ") FAILED!");
        return false;
    }
    return true;
#endif
}

bool StGLTexture::create(StGLContext&   theCtx,
                         const GLenum   theDataFormat,
                         const GLubyte* theData) {
#if defined(GL_ES_VERSION_2_0)
    theCtx.stglResetErrors();
#endif

    myHasMipMaps = 0;
    if(!isValid()) {
        theCtx.core20fwd->glGenTextures(1, &myTextureId); // Create The Texture
    }
    bind(theCtx);

    // texture interpolation parameters - could be overridden later
    theCtx.core20fwd->glTexParameteri(myTarget, GL_TEXTURE_MIN_FILTER, myFilterMin);
    theCtx.core20fwd->glTexParameteri(myTarget, GL_TEXTURE_MAG_FILTER, myFilterMag);
    theCtx.core20fwd->glTexParameteri(myTarget, GL_TEXTURE_WRAP_S,     GL_CLAMP_TO_EDGE);
    theCtx.core20fwd->glTexParameteri(myTarget, GL_TEXTURE_WRAP_T,     GL_CLAMP_TO_EDGE);

    if(!isProxySuccess(theCtx)) {
        release(theCtx);
        return false;
    }

    // NULL is passed to glTexImage2D(), so that there is no actual data to be interpreted.
    // However, OpenGL ES raises GL_INVALID_OPERATION if invalid type is passed.
    const GLenum aDataType = getDataType(myTextFormat);
    GLint anInternalFormat = myTextFormat;
#if defined(GL_ES_VERSION_2_0)
    if(!theCtx.isGlGreaterEqual(3, 0) && aDataType == GL_UNSIGNED_BYTE) {
        // sized formats are not supported here
        anInternalFormat = theDataFormat;
    }
#endif

    if(myTarget == GL_TEXTURE_CUBE_MAP) {
        const GLenum aTargets[6] = { GL_TEXTURE_CUBE_MAP_POSITIVE_X,
                                     GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
                                     GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
                                     GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
                                     GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
                                     GL_TEXTURE_CUBE_MAP_NEGATIVE_Z };
        for(int aTargetIter = 0; aTargetIter < 6; ++aTargetIter) {
            theCtx.core20fwd->glTexImage2D(aTargets[aTargetIter], 0, anInternalFormat,
                                       mySizeX, mySizeY, 0,
                                       theDataFormat, aDataType, theData);
        }
    } else {
        theCtx.core20fwd->glTexImage2D(myTarget, 0, anInternalFormat,
                                       mySizeX, mySizeY, 0,
                                       theDataFormat, aDataType, theData);
    }
#if defined(GL_ES_VERSION_2_0)
    // proxy texture is unavailable - check for errors
    GLenum anErr = theCtx.core20fwd->glGetError();
    if(anErr != GL_NO_ERROR) {
        ST_ERROR_LOG("Creation texture with size (" + mySizeX + " x "+ mySizeY
                   + " @" + formatInternalFormat(myTextFormat) + " with data " + formatInternalFormat(theDataFormat) + ") FAILED: " + theCtx.stglErrorToString(anErr) + "!");
        release(theCtx);
        return false;
    }
#ifdef ST_DEBUG_TEXTURES
    ST_DEBUG_LOG("Created StGLTexture " + mySizeX + " x "+ mySizeY
               + " (format " + formatInternalFormat(myTextFormat) + ')');
#endif

#else
    // detect which texture was actually created
    GLint aResFormat = 0;
    GLint aResSizeX  = 0;
    GLint aResSizeY  = 0;
    GLenum aTarget = myTarget == GL_TEXTURE_CUBE_MAP ? GL_TEXTURE_CUBE_MAP_NEGATIVE_Z : myTarget;
    theCtx.core20fwd->glGetTexLevelParameteriv(aTarget, 0, GL_TEXTURE_WIDTH,           &aResSizeX);
    theCtx.core20fwd->glGetTexLevelParameteriv(aTarget, 0, GL_TEXTURE_HEIGHT,          &aResSizeY);
    theCtx.core20fwd->glGetTexLevelParameteriv(aTarget, 0, GL_TEXTURE_INTERNAL_FORMAT, &aResFormat);
#ifdef ST_DEBUG_TEXTURES
    ST_DEBUG_LOG("Created StGLTexture "
               + aResSizeX + " x "+ aResSizeY
               + " (format " + formatInternalFormat(aResFormat)
               + (myTarget == GL_TEXTURE_CUBE_MAP ? " [CUBEMAP]" : "")
               + ")");
#endif
#endif

    unbind(theCtx);
    return true;
}

void StGLTexture::setMinMagFilter(StGLContext& theCtx,
                                  const GLenum theMinFilter,
                                  const GLenum theMagFilter) {
    const bool needsMipMaps = theMinFilter != GL_NEAREST
                           && theMinFilter != GL_LINEAR;
    if(myFilterMin == theMinFilter
    && myFilterMag == theMagFilter
    && (!needsMipMaps || myHasMipMaps != 0)) {
        return;
    }

    myFilterMin = theMinFilter;
    myFilterMag = theMagFilter;
    if(!isValid()) {
        return;
    }

    bind(theCtx);
        if(needsMipMaps && myHasMipMaps == 0) {
            theCtx.arbFbo->glGenerateMipmap(myTarget);
            const GLenum anErr = glGetError();
            if(anErr != GL_NO_ERROR) {
                // note that OpenGL ES 2.0 does not support mip-map generation of non-power-of-two textures
                ST_ERROR_LOG("Failed to generate mipmap levels with error " + theCtx.stglErrorToString(anErr));
                myHasMipMaps = -1;
            } else {
                myHasMipMaps = 1;
            }
        }
        GLenum aMinFilter = !needsMipMaps || myHasMipMaps == 1
                          ? myFilterMin
                          : GL_LINEAR;
        theCtx.core20fwd->glTexParameteri(myTarget, GL_TEXTURE_MIN_FILTER, aMinFilter);
        theCtx.core20fwd->glTexParameteri(myTarget, GL_TEXTURE_MAG_FILTER, myFilterMag);

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

bool StGLTexture::fillPatch(StGLContext&        theCtx,
                            const StImagePlane& theData,
                            const GLenum        theTarget,
                            const GLsizei       theRowFrom,
                            const GLsizei       theRowTo) {
#ifdef __ANDROID__
    GLsizei aBatchRows = 0;
#else
    GLsizei aBatchRows = 128; // TODO does it makes sense nowadays?
#endif
    return fillPatch(theCtx, theData, theTarget, theRowFrom, theRowTo, aBatchRows);
}

bool StGLTexture::fillPatch(StGLContext&        theCtx,
                            const StImagePlane& theData,
                            GLenum              theTarget,
                            const GLsizei       theRowFrom,
                            const GLsizei       theRowTo,
                            const GLsizei       theBatchRows) {
    if(theTarget == 0) {
        theTarget = myTarget;
    }
    if(theData.isNull() || !isValid()) {
        return false;
    }

    GLenum aPixelFormat, aDataType;
    if(!getDataFormat(theCtx, theData, aPixelFormat, aDataType)) {
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

    myHasMipMaps = 0;
    bind(theCtx);

    // setup the alignment
    size_t anAligment = stMin(theData.getMaxRowAligment(), size_t(8)); // limit to 8 bytes for OpenGL
    theCtx.core20fwd->glPixelStorei(GL_UNPACK_ALIGNMENT, GLint(anAligment));

    bool toBatchCopy = theData.getSizeX() <= size_t(getSizeX())
                    && theBatchRows != 1;

    size_t anExtraBytes       = theData.getRowExtraBytes();
    size_t aPixelsWidth       = theData.getSizeRowBytes() / theData.getSizePixelBytes();
    size_t aSizeRowBytesEstim = getAligned(theData.getSizePixelBytes() * aPixelsWidth, anAligment);
    if(anExtraBytes < anAligment) {
        aPixelsWidth = 0;
    } else if(aSizeRowBytesEstim != theData.getSizeRowBytes()) {
        aPixelsWidth = 0;
        toBatchCopy  = false;
    }

    if(!theCtx.getDeviceCaps().hasUnpack
    && anExtraBytes >= anAligment) {
        toBatchCopy = false;
    }

    if(toBatchCopy) {
        if(theCtx.getDeviceCaps().hasUnpack) {
            theCtx.core20fwd->glPixelStorei(GL_UNPACK_ROW_LENGTH, GLint(aPixelsWidth));
        }

        // do batch copy (more effective)
        GLsizei aPatchWidth = GLsizei(theData.getSizeX());
        GLsizei aBatchRows  = theBatchRows >= 1 ? theBatchRows : (aRowTo - theRowFrom);
        GLsizei aNbRows     = aBatchRows;
        for(GLsizei aRow(theRowFrom), aRowsRemain(aRowTo); aRow < aRowTo; aRow += aBatchRows) {
            aRowsRemain = aRowTo - aRow;
            if(aNbRows > aRowsRemain) {
                aNbRows = aRowsRemain;
            }

            theCtx.core20fwd->glTexSubImage2D(theTarget, 0,     // 0 = LOD number
                                              0, aRow,          // a texel offset in the (x, y) direction
                                              aPatchWidth, aNbRows,
                                              aPixelFormat,     // format of the pixel data
                                              aDataType,        // data type of the pixel data
                                              theData.getData(aRow, 0));
        }

        if(theCtx.getDeviceCaps().hasUnpack) {
            theCtx.core20fwd->glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
        }
    } else {
        // copy row by row copy (the image plane greater than texture or batch copy is impossible)
        if(theCtx.getDeviceCaps().hasUnpack) {
            theCtx.core20fwd->glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
        }

        GLsizei aPatchWidth = stMin(GLsizei(theData.getSizeX()), getSizeX());
        for(GLsizei aRow = theRowFrom; aRow < aRowTo; ++aRow) {
            theCtx.core20fwd->glTexSubImage2D(theTarget, 0,     // 0 = LOD number
                                              0, aRow,          // a texel offset in the (x, y) direction
                                              aPatchWidth, 1,   // the (width, height) of the texture sub-image
                                              aPixelFormat,     // format of the pixel data
                                              aDataType,        // data type of the pixel data
                                              theData.getData(aRow, 0));
        }
    }

    // turn back safe alignment...
    theCtx.core20fwd->glPixelStorei(GL_UNPACK_ALIGNMENT,  1);

    unbind(theCtx);
    return true;
}

StGLNamedTexture::StGLNamedTexture() {
    //
}

StGLNamedTexture::~StGLNamedTexture() {
    //
}
