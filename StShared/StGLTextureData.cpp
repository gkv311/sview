/**
 * Copyright © 2009-2012 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#include <StGLStereo/StGLTextureData.h>
#include <StStrings/StLogger.h>

#include <StGLCore/StGLCore11.h>

StGLTextureData::StGLTextureData()
: prev(NULL),
  next(NULL),
  dataPtr(NULL),
  dataSizeBytes(0),
  myDataL(),
  myDataR(),
  myStParams(),
  srcPTS(0.0),
  srcFormat(ST_V_SRC_AUTODETECT),
  fillFromRow(0),
  fillRows(0) {
    //
}

StGLTextureData::~StGLTextureData() {
    reset();
}

void StGLTextureData::reset() {
    myDataL.nullify();
    myDataR.nullify();
    if(dataPtr != NULL) {
        stMemFreeAligned(dataPtr);
        dataPtr = NULL;
    }
    dataSizeBytes = 0;
    fillRows = fillFromRow = 0;
}

bool StGLTextureData::reAllocate(size_t newSizeBytes) {
    // reallocate only if summary data is not same
    // this allows to smoothly switch to different stereo source formats
    // over/under -> sideBySide -> mono
    // because the summary buffer needed for both views will be same
    if(dataSizeBytes != newSizeBytes) {
        reset();
        dataSizeBytes = newSizeBytes;
        dataPtr = stMemAllocAligned<GLubyte*>(dataSizeBytes);

        // reset the buffer (make black)
        /// this is probably useless and wrong in case of non RGB image data
        stMemSet(dataPtr, 0, dataSizeBytes);
        ST_DEBUG_LOG("StGLTextureData (re)allocated to " + dataSizeBytes + " bytes");
        return true;
    }
    return false;
}

/**
 * Function tries to detect stereo format;
 * this detection possible only for many side by side files.
 * Note: most original DVDs video couldn't be detected as stereo at all.
 * @param ratio (const GLfloat ) - image ratio;
 * @return StFormatEnum - autodetected mono/stereo format.
 */
static StFormatEnum formatFromRatio(const GLfloat ratio) {
    static const GLfloat LAMBDA = 0.18f;
    if(   stAreEqual(ratio, videoRatio::TV_SIDEBYSIDE, LAMBDA)
       || stAreEqual(ratio, videoRatio::WIDE_SIDEBYSIDE, LAMBDA)
       || stAreEqual(ratio, videoRatio::USERDEF_SIDEBYSIDE, LAMBDA)) {
        return ST_V_SRC_SIDE_BY_SIDE;
    }
    return ST_V_SRC_MONO;
}

static GLubyte* readFromParallel(const StImagePlane& srcData,
                                 GLubyte* outDataPtr,
                                 StImagePlane& outDataL,
                                 StImagePlane& outDataR) {
    if(srcData.isNull()) {
        return outDataPtr;
    }

    size_t srcDataSizeXHalf = srcData.getSizeX() / 2;
    size_t anOutRowBytes = getEvenNumber(srcDataSizeXHalf * srcData.getSizePixelBytes());
    outDataL.initWrapper(srcData.getFormat(), outDataPtr,
                         srcDataSizeXHalf, srcData.getSizeY(),
                         anOutRowBytes);
    outDataR.initWrapper(srcData.getFormat(), &outDataPtr[outDataL.getSizeBytes()],
                         outDataL.getSizeX(), outDataL.getSizeY(),
                         anOutRowBytes);

    size_t copyRows      = stMin(outDataL.getSizeY(), srcData.getSizeY());
    size_t copyRowBytes  = stMin(outDataL.getSizeX(), srcDataSizeXHalf) * outDataL.getSizePixelBytes();

    // copy row by row
    size_t aRowInc = srcData.isTopDown() ? 1 : size_t(-1);
    size_t aRowTo  = srcData.isTopDown() ? 0 : (copyRows - 1);
    for(size_t aRowFrom = 0; aRowFrom < copyRows; ++aRowFrom, aRowTo += aRowInc) {
        stMemCpy(outDataL.changeData(aRowTo, 0),
                 srcData.getData(aRowFrom, 0),
                 copyRowBytes);
        stMemCpy(outDataR.changeData(aRowTo, 0),
                 srcData.getData(aRowFrom, srcDataSizeXHalf),
                 copyRowBytes);
    }
    return &outDataPtr[2 * outDataL.getSizeBytes()];
}

static GLubyte* readFromOverUnderLR(const StImagePlane& srcData,
                                    GLubyte* outDataPtr,
                                    StImagePlane& outDataL,
                                    StImagePlane& outDataR) {
    if(srcData.isNull()) {
        return outDataPtr;
    }

    size_t srcDataSizeYHalf = srcData.getSizeY() / 2;
    size_t anOutRowBytes = getEvenNumber(srcData.getSizeX() * srcData.getSizePixelBytes());
    outDataL.initWrapper(srcData.getFormat(), outDataPtr,
                         srcData.getSizeX(), srcDataSizeYHalf,
                         anOutRowBytes);
    outDataR.initWrapper(srcData.getFormat(), &outDataPtr[outDataL.getSizeBytes()],
                         outDataL.getSizeX(), outDataL.getSizeY(),
                         anOutRowBytes);

    size_t copyRows      = stMin(outDataL.getSizeY(), srcDataSizeYHalf);
    size_t copyRowBytes  = stMin(outDataL.getSizeX(), srcData.getSizeX()) * outDataL.getSizePixelBytes();

    if(outDataL.getSizeRowBytes() == srcData.getSizeRowBytes() && srcData.isTopDown()) {
        // perform fat copy
        stMemCpy(outDataL.changeData(),
                 srcData.getData(0, 0),
                 copyRows * copyRowBytes);
        stMemCpy(outDataR.changeData(),
                 srcData.getData(srcDataSizeYHalf, 0),
                 copyRows * copyRowBytes);
    } else {
        // check if data is upside-down
        size_t aRowTop    = srcData.isTopDown() ? 0 : srcDataSizeYHalf;
        size_t aRowBottom = srcData.isTopDown() ? srcDataSizeYHalf : 0;

        // copy row by row
        size_t aRowInc = srcData.isTopDown() ? 1 : size_t(-1);
        size_t aRowTo  = srcData.isTopDown() ? 0 : (copyRows - 1);
        for(size_t aRow = 0; aRow < copyRows; ++aRow, aRowTo += aRowInc) {
            stMemCpy(outDataL.changeData(aRowTo, 0),
                     srcData.getData(aRowTop + aRow, 0),
                     copyRowBytes);
        }
        aRowTo = srcData.isTopDown() ? 0 : (copyRows - 1);
        for(size_t aRow = 0; aRow < copyRows; ++aRow, aRowTo += aRowInc) {
            stMemCpy(outDataR.changeData(aRowTo, 0),
                     srcData.getData(aRowBottom + aRow, 0),
                     copyRowBytes);
        }
    }
    return &outDataPtr[2 * outDataL.getSizeBytes()];
}


static GLubyte* readFromRowInterlace(const StImagePlane& srcData,
                                     GLubyte* outDataPtr,
                                     StImagePlane& outDataL,
                                     StImagePlane& outDataR) {
    if(srcData.isNull()) {
        return outDataPtr;
    }

    size_t srcDataSizeYHalf = srcData.getSizeY() / 2;
    size_t anOutRowBytes = getEvenNumber(srcData.getSizeX() * srcData.getSizePixelBytes());
    outDataL.initWrapper(srcData.getFormat(), outDataPtr,
                         srcData.getSizeX(), srcDataSizeYHalf,
                         anOutRowBytes);
    outDataR.initWrapper(srcData.getFormat(), &outDataPtr[outDataL.getSizeBytes()],
                         outDataL.getSizeX(), outDataL.getSizeY(),
                         anOutRowBytes);

    size_t copyRows      = stMin(outDataL.getSizeY(), srcDataSizeYHalf);
    size_t copyRowBytes  = stMin(outDataL.getSizeX(), srcData.getSizeX()) * outDataL.getSizePixelBytes();

    size_t srcRowLeft  = srcData.isTopDown() ? 0 : 1;
    size_t srcRowRight = srcData.isTopDown() ? 1 : 0;

    // prepare iterator for bottom-up source data
    size_t aRowInc = srcData.isTopDown() ? 1 : size_t(-1);
    size_t aRowTo  = srcData.isTopDown() ? 0 : (copyRows - 1);

    // copy row by row
    for(size_t aRowFrom = 0; aRowFrom < copyRows; ++aRowFrom, aRowTo += aRowInc) {
        stMemCpy(outDataR.changeData(aRowTo, 0),
                 srcData.getData(2 * aRowFrom + srcRowRight, 0),
                 copyRowBytes);
        stMemCpy(outDataL.changeData(aRowTo, 0),
                 srcData.getData(2 * aRowFrom + srcRowLeft, 0),
                 copyRowBytes);
    }
    return &outDataPtr[2 * outDataL.getSizeBytes()];
}

static GLubyte* readFromTiled4X(const StImagePlane& theDataSrc,
                                GLubyte* theDataOutPtr,
                                StImagePlane& theDataOutL,
                                StImagePlane& theDataOutR) {
    if(theDataSrc.isNull()) {
        return theDataOutPtr;
    }

    size_t aDataSizeX = (theDataSrc.getSizeX() / 3) * 2;
    size_t aDataSizeY = (theDataSrc.getSizeY() / 3) * 2;
    size_t aDataSizeXHalf = aDataSizeX / 2;

    size_t anOutRowBytes = getEvenNumber(aDataSizeX * theDataSrc.getSizePixelBytes());
    theDataOutL.initWrapper(theDataSrc.getFormat(), theDataOutPtr,
                            aDataSizeX, aDataSizeY,
                            anOutRowBytes);
    theDataOutR.initWrapper(theDataSrc.getFormat(), &theDataOutPtr[theDataOutL.getSizeBytes()],
                            theDataOutL.getSizeX(), theDataOutL.getSizeY(),
                            anOutRowBytes);

    size_t aCopyRows     = stMin(theDataOutL.getSizeY(), aDataSizeY);
    size_t aCopyRowBytes = stMin(theDataOutL.getSizeX(), aDataSizeX) * theDataOutL.getSizePixelBytes();

    // check if data is upside-down
    size_t aRowSrcTop = theDataSrc.isTopDown() ? 0 : (theDataSrc.getSizeY() - 1);

    // copy Left view (1 big tile at top-left corner)
    size_t aRowInc = theDataSrc.isTopDown() ? 1 : size_t(-1);
    size_t aRowTo  = 0;
    size_t aRowSrc = aRowSrcTop;
    for(; aRowTo < aCopyRows; ++aRowTo, aRowSrc += aRowInc) {
        stMemCpy(theDataOutL.changeData(aRowTo, 0),
                 theDataSrc.getData(aRowSrc, 0),
                 aCopyRowBytes);
    }

    // copy Right view (first half-width tile at top-right
    aCopyRowBytes = (aDataSizeX / 2) * theDataOutL.getSizePixelBytes();
    aRowTo  = 0;
    aRowSrc = aRowSrcTop;
    for(; aRowTo < aCopyRows; ++aRowTo, aRowSrc += aRowInc) {
        stMemCpy(theDataOutR.changeData(aRowTo, 0),
                 theDataSrc.getData(aRowSrc, aDataSizeX),
                 aCopyRowBytes);
    }

    // copy Right view (first 0.25 tile at bottom-left)
    aCopyRows = aDataSizeY / 2;
    aRowSrcTop = theDataSrc.isTopDown() ? aDataSizeY : (theDataSrc.getSizeY() - aDataSizeY);
    aRowTo  = 0;
    aRowSrc = aRowSrcTop;
    for(; aRowTo < aCopyRows; ++aRowTo, aRowSrc += aRowInc) {
        stMemCpy(theDataOutR.changeData(aRowTo, aDataSizeXHalf),
                 theDataSrc.getData(aRowSrc, 0),
                 aCopyRowBytes);
    }

    // copy Right view (second 0.25 tile at bottom)
    aRowTo  = aCopyRows;
    aRowSrc = aRowSrcTop;
    aCopyRows += aCopyRows;
    for(; aRowTo < aCopyRows; ++aRowTo, aRowSrc += aRowInc) {
        stMemCpy(theDataOutR.changeData(aRowTo, aDataSizeXHalf),
                 theDataSrc.getData(aRowSrc, aDataSizeXHalf),
                 aCopyRowBytes);
    }

    return &theDataOutPtr[2 * theDataOutL.getSizeBytes()];
}

static GLubyte* readFromMono(const StImagePlane& srcData,
                             GLubyte* outDataPtr,
                             StImagePlane& outData) {
    if(srcData.isNull()) {
        return outDataPtr;
    }

    size_t anOutRowBytes = getEvenNumber(srcData.getSizeX() * srcData.getSizePixelBytes());
    outData.initWrapper(srcData.getFormat(), outDataPtr,
                        srcData.getSizeX(), srcData.getSizeY(),
                        anOutRowBytes);

    if(outData.getSizeRowBytes() == srcData.getSizeRowBytes() && srcData.isTopDown()) {
        // perform fat copy
        stMemCpy(outData.changeData(), srcData.getData(),
                 stMin(outData.getSizeBytes(), srcData.getSizeBytes()));
    } else {
        size_t     copyRows = stMin(outData.getSizeY(), srcData.getSizeY());
        size_t copyRowBytes = stMin(outData.getSizeX(), srcData.getSizeX()) * outData.getSizePixelBytes();
        size_t      aRowInc = srcData.isTopDown() ? 1 : size_t(-1);
        size_t       aRowTo = srcData.isTopDown() ? 0 : (copyRows - 1);
        for(size_t aRowFrom = 0; aRowFrom < copyRows; ++aRowFrom, aRowTo += aRowInc) {
            stMemCpy(outData.changeData(aRowTo, 0), srcData.getData(aRowFrom, 0), copyRowBytes);
        }
    }
    return &outDataPtr[outData.getSizeBytes()];
}

/**
 * Compute buffer size to fit image copy
 * with reserve for different source formats.
 */
inline size_t computeBufferSize(const StImage& srcData) {
    if(srcData.isNull()) {
        return 0;
    }
    size_t aBufferSize = 0;
    for(size_t aPlaneId = 0; aPlaneId < 4; ++aPlaneId) {
        const StImagePlane& aPlane = srcData.getPlane(aPlaneId);
        size_t sizeX_4 = getAligned(aPlane.getSizeX(), 4);
        size_t sizeY_4 = getAligned(aPlane.getSizeY(), 4);
        aBufferSize += sizeX_4 * sizeY_4 * aPlane.getSizePixelBytes();
    }
    return aBufferSize;
}

void StGLTextureData::updateData(const StImage& srcDataL,
                                 const StImage& srcDataR,
                                 const StHandle<StStereoParams>& theStParams,
                                 StFormatEnum srcFormat,
                                 double srcPTS) {
    // setup new stereo source
    myStParams = theStParams;
    this->srcPTS = srcPTS;

    // detect format from ratio if not defined
    if(srcFormat == ST_V_SRC_AUTODETECT) {
        srcFormat = formatFromRatio(srcDataL.getRatio());
    }
    this->srcFormat = srcFormat;

    // reset fill texture state
    fillRows = fillFromRow = 0;

    // reallocate buffer if needed
    size_t newSizeBytes = computeBufferSize(srcDataL) + computeBufferSize(srcDataR);
    if(newSizeBytes == 0) {
        // invalid data
        myDataL.nullify();
        myDataR.nullify();
        return;
    }

    reAllocate(newSizeBytes);
    myDataL.setColorModel(srcDataL.getColorModel());
    myDataR.setColorModel(srcDataL.getColorModel());
    myDataL.setPixelRatio(srcDataL.getPixelRatio());
    myDataR.setPixelRatio(srcDataL.getPixelRatio());

    switch(srcFormat) {
        case ST_V_SRC_SIDE_BY_SIDE:
        case ST_V_SRC_PARALLEL_PAIR: {
            GLubyte* dataDispl = dataPtr;
            for(size_t aPlaneId = 0; aPlaneId < 4; ++aPlaneId) {
                dataDispl = readFromParallel(srcDataL.getPlane(aPlaneId), dataDispl,
                                             (srcFormat == ST_V_SRC_PARALLEL_PAIR) ? myDataL.changePlane(aPlaneId) : myDataR.changePlane(aPlaneId),
                                             (srcFormat == ST_V_SRC_PARALLEL_PAIR) ? myDataR.changePlane(aPlaneId) : myDataL.changePlane(aPlaneId));
            }
            break;
        }
        case ST_V_SRC_OVER_UNDER_RL:
        case ST_V_SRC_OVER_UNDER_LR: {
            GLubyte* dataDispl = dataPtr;
            for(size_t aPlaneId = 0; aPlaneId < 4; ++aPlaneId) {
                dataDispl = readFromOverUnderLR(srcDataL.getPlane(aPlaneId), dataDispl,
                                                (srcFormat == ST_V_SRC_OVER_UNDER_LR) ? myDataL.changePlane(aPlaneId) : myDataR.changePlane(aPlaneId),
                                                (srcFormat == ST_V_SRC_OVER_UNDER_LR) ? myDataR.changePlane(aPlaneId) : myDataL.changePlane(aPlaneId));
            }
            break;
        }
        case ST_V_SRC_ROW_INTERLACE: {
            myDataL.setPixelRatio(srcDataL.getPixelRatio() * 0.5f);
            myDataR.setPixelRatio(srcDataL.getPixelRatio() * 0.5f);
            GLubyte* dataDispl = dataPtr;
            // TODO (Kirill Gavrilov#9) wrong for yuv420p?
            for(size_t aPlaneId = 0; aPlaneId < 4; ++aPlaneId) {
                dataDispl = readFromRowInterlace(srcDataL.getPlane(aPlaneId), dataDispl,
                                                 myDataL.changePlane(aPlaneId), myDataR.changePlane(aPlaneId));

            }
            break;
        }
        case ST_V_SRC_SEPARATE_FRAMES: {
            myDataR.setColorModel(srcDataR.getColorModel());
            myDataR.setPixelRatio(srcDataR.getPixelRatio());
            GLubyte* dataDispl = dataPtr;
            for(size_t aPlaneId = 0; aPlaneId < 4; ++aPlaneId) {
                dataDispl = readFromMono(srcDataL.getPlane(aPlaneId), dataDispl, myDataL.changePlane(aPlaneId));
            }
            for(size_t aPlaneId = 0; aPlaneId < 4; ++aPlaneId) {
                dataDispl = readFromMono(srcDataR.getPlane(aPlaneId), dataDispl, myDataR.changePlane(aPlaneId));
            }
            break;
        }
        case ST_V_SRC_TILED_4X: {
            GLubyte* dataDispl = dataPtr;
            for(size_t aPlaneId = 0; aPlaneId < 4; ++aPlaneId) {
                dataDispl = readFromTiled4X(srcDataL.getPlane(aPlaneId), dataDispl,
                                            myDataL.changePlane(aPlaneId), myDataR.changePlane(aPlaneId));
            }
            break;
        }
        case ST_V_SRC_ANAGLYPH_RED_CYAN:
        case ST_V_SRC_ANAGLYPH_G_RB:
        case ST_V_SRC_ANAGLYPH_YELLOW_BLUE:
        case ST_V_SRC_VERTICAL_INTERLACE: // not supported
        case ST_V_SRC_PAGE_FLIP:          // not supported
        case ST_V_SRC_MONO:
        default: {
            GLubyte* dataDispl = dataPtr;
            for(size_t aPlaneId = 0; aPlaneId < 4; ++aPlaneId) {
                dataDispl = readFromMono(srcDataL.getPlane(aPlaneId), dataDispl, myDataL.changePlane(aPlaneId));
            }
            break;
        }
    }
}

void StGLTextureData::fillTexture(StGLContext&        theCtx,
                                  StGLFrameTexture&   theFrameTexture,
                                  const StImagePlane& theData) {
    if(!theFrameTexture.isValid() || theData.isNull()) {
        return;
    }
    theFrameTexture.fill(theCtx, theData, fillFromRow, fillFromRow + fillRows);
}

static void setupDataRectangle(const StImagePlane& theImagePlane,
                               const GLfloat       thePixelRatio,
                               StGLFrameTexture&   theTextureFrame) {
    if(theImagePlane.isNull() || !theTextureFrame.isValid()) {
        return;
    }

    const GLfloat aSizeXFloat = stMin(GLfloat(theImagePlane.getSizeX()), GLfloat(theTextureFrame.getSizeX()));
    const GLfloat aSizeYFloat = stMin(GLfloat(theImagePlane.getSizeY()), GLfloat(theTextureFrame.getSizeY()));
    StGLVec2 aDataSize (aSizeXFloat / GLfloat(theTextureFrame.getSizeX()),
                        aSizeYFloat / GLfloat(theTextureFrame.getSizeY()));
    if(aDataSize.x() > 1.0f) {
        aDataSize.x() = 1.0f;
    }
    if(aDataSize.y() > 1.0f) {
        aDataSize.y() = 1.0f;
    }
    theTextureFrame.setDataSize(aDataSize);
    theTextureFrame.setDisplayRatio((thePixelRatio * aSizeXFloat) / aSizeYFloat);
}

void StGLTextureData::setupAttributes(StGLFrameTextures& stFrameTextures, const StImage& theImage) {
    for(size_t aPlaneId = 0; aPlaneId < 4; ++aPlaneId) {
        setupDataRectangle(theImage.getPlane(aPlaneId), theImage.getPixelRatio(), stFrameTextures.getPlane(aPlaneId));
    }
    stFrameTextures.setSource(myStParams);
}

static void prepareTextures(StGLContext&       theCtx,
                            const StImage&     theImage,
                            StGLFrameTextures& theTextureFrame) {
    GLint anInternalFormat = GL_RGB8;
    theTextureFrame.setColorModel(theImage.getColorModel());
    for(size_t aPlaneId = 0; aPlaneId < 4; ++aPlaneId) {
        const StImagePlane& anImgPlane = theImage.getPlane(aPlaneId);
        if(anImgPlane.isNull()) {
            theTextureFrame.getPlane(aPlaneId).release(theCtx);
            continue;
        }

        if(!StGLTexture::getInternalFormat(anImgPlane, anInternalFormat)) {
            theTextureFrame.getPlane(aPlaneId).release(theCtx);
            continue;
        }
        theTextureFrame.preparePlane(theCtx,
                                     aPlaneId,
                                     (GLsizei )theImage.getPlane(aPlaneId).getSizeX(),
                                     (GLsizei )theImage.getPlane(aPlaneId).getSizeY(),
                                     anInternalFormat);
    }
}

bool StGLTextureData::fillTexture(StGLContext&     theCtx,
                                  StGLQuadTexture& theQTexture) {

    // setup rows count to be filled per fillTexture()
    if(fillRows == 0 || fillFromRow == 0) {
        // prepare textures for new data
        prepareTextures(theCtx, myDataL, theQTexture.getBack(StGLQuadTexture::LEFT_TEXTURE));
        prepareTextures(theCtx, myDataR, theQTexture.getBack(StGLQuadTexture::RIGHT_TEXTURE));

        // remove links to old stereo parameters
        theQTexture.getBack(StGLQuadTexture::LEFT_TEXTURE).setSource(StHandle<StStereoParams>());
        theQTexture.getBack(StGLQuadTexture::RIGHT_TEXTURE).setSource(StHandle<StStereoParams>());

        // TODO (Kirill Gavrilov#9) this value is meanfull only for PageFlip,
        //                          also rows number may be replaced with bytes count
        static const GLsizei UPDATED_ROWS_MAX = 1088; // we use optimal value to update 1080p video frame at-once
        GLsizei maxRows = stMin(GLsizei(myDataL.getSizeY()), theQTexture.getBack(StGLQuadTexture::LEFT_TEXTURE).getSizeY());
        GLsizei iterations = (maxRows / (myDataR.isNull() ? UPDATED_ROWS_MAX * 2 : UPDATED_ROWS_MAX)) + 1;
        fillRows = maxRows / iterations;
        fillFromRow = 0;
    }

    if(fillRows == 0) {
        // prevent dead loop
        return true;
    }

    if(theQTexture.getBack(StGLQuadTexture::LEFT_TEXTURE).isValid()) {
        for(size_t aPlaneId = 0; aPlaneId < 4; ++aPlaneId) {
            fillTexture(theCtx,
                        theQTexture.getBack(StGLQuadTexture::LEFT_TEXTURE).getPlane(aPlaneId),
                        myDataL.getPlane(aPlaneId));
        }
    }
    if(theQTexture.getBack(StGLQuadTexture::RIGHT_TEXTURE).isValid()) {
        for(size_t aPlaneId = 0; aPlaneId < 4; ++aPlaneId) {
            fillTexture(theCtx,
                        theQTexture.getBack(StGLQuadTexture::RIGHT_TEXTURE).getPlane(aPlaneId),
                        myDataR.getPlane(aPlaneId));
        }
    }
    theQTexture.getBack(StGLQuadTexture::LEFT_TEXTURE).unbind(theCtx);

    fillFromRow += fillRows;
    if(fillFromRow >= GLsizei(myDataL.getSizeY())) {
        if(!myDataL.isNull() && theQTexture.getBack(StGLQuadTexture::LEFT_TEXTURE).isValid()) {
            setupAttributes(theQTexture.getBack(StGLQuadTexture::LEFT_TEXTURE), myDataL);
        }
        if(!myDataR.isNull() && theQTexture.getBack(StGLQuadTexture::RIGHT_TEXTURE).isValid()) {
            setupAttributes(theQTexture.getBack(StGLQuadTexture::RIGHT_TEXTURE), myDataR);
        }

        if(!myStParams.isNull()) {
            myStParams->setSrcFormat(srcFormat);
        }
        return true;
    } else {
        return false;
    }
}

void StGLTextureData::getCopy(StImage* outDataL, StImage* outDataR) const {
    if(outDataL != NULL) {
        outDataL->initCopy(myDataL);
    }
    if(outDataR != NULL) {
        outDataR->initCopy(myDataR);
    }
}
