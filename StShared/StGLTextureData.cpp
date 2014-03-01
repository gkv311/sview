/**
 * Copyright © 2009-2013 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#include <StGLStereo/StGLTextureData.h>
#include <StStrings/StLogger.h>

#include <StGLCore/StGLCore11.h>

StGLTextureData::StGLTextureData()
: myPrev(NULL),
  myNext(NULL),
  myDataPtr(NULL),
  myDataSizeBytes(0),
  myStParams(),
  myPts(0.0),
  mySrcFormat(ST_V_SRC_AUTODETECT),
  myFillFromRow(0),
  myFillRows(0) {
    //
}

StGLTextureData::~StGLTextureData() {
    reset();
}

void StGLTextureData::reset() {
    myDataL.nullify();
    myDataR.nullify();
    if(myDataPtr != NULL) {
        stMemFreeAligned(myDataPtr);
        myDataPtr = NULL;
    }
    myDataSizeBytes = 0;
    myFillRows = myFillFromRow = 0;
}

bool StGLTextureData::reAllocate(const size_t theSizeBytes) {
    // reallocate only if summary data is not same
    // this allows to smoothly switch to different stereo source formats
    // over/under -> sideBySide -> mono
    // because the summary buffer needed for both views will be same
    if(myDataSizeBytes != theSizeBytes) {
        reset();
        myDataSizeBytes = theSizeBytes;
        myDataPtr       = stMemAllocAligned<GLubyte*>(myDataSizeBytes);

        // reset the buffer (make black)
        /// this is probably useless and wrong in case of non RGB image data
        stMemZero(myDataPtr, myDataSizeBytes);
        ST_DEBUG_LOG("StGLTextureData (re)allocated to " + myDataSizeBytes + " bytes");
        return true;
    }
    return false;
}

static GLubyte* readFromParallel(const StImagePlane& theSrc,
                                 GLubyte*            theDataPtr,
                                 StImagePlane&       theDataL,
                                 StImagePlane&       theDataR) {
    if(theSrc.isNull()) {
        return theDataPtr;
    }

    const size_t srcDataSizeXHalf = theSrc.getSizeX() / 2;
    const size_t anOutRowBytes    = getEvenNumber(srcDataSizeXHalf * theSrc.getSizePixelBytes());
    theDataL.initWrapper(theSrc.getFormat(), theDataPtr,
                         srcDataSizeXHalf, theSrc.getSizeY(),
                         anOutRowBytes);
    theDataR.initWrapper(theSrc.getFormat(), &theDataPtr[theDataL.getSizeBytes()],
                         theDataL.getSizeX(), theDataL.getSizeY(),
                         anOutRowBytes);

    const size_t aCopyRows      = stMin(theDataL.getSizeY(), theSrc.getSizeY());
    const size_t aCopyRowBytes  = stMin(theDataL.getSizeX(), srcDataSizeXHalf) * theDataL.getSizePixelBytes();

    // copy row by row
    size_t aRowInc = theSrc.isTopDown() ? 1 : size_t(-1);
    size_t aRowTo  = theSrc.isTopDown() ? 0 : (aCopyRows - 1);
    for(size_t aRowFrom = 0; aRowFrom < aCopyRows; ++aRowFrom, aRowTo += aRowInc) {
        stMemCpy(theDataL.changeData(aRowTo, 0),
                 theSrc.getData(aRowFrom, 0),
                 aCopyRowBytes);
        stMemCpy(theDataR.changeData(aRowTo, 0),
                 theSrc.getData(aRowFrom, srcDataSizeXHalf),
                 aCopyRowBytes);
    }
    return &theDataPtr[2 * theDataL.getSizeBytes()];
}

static GLubyte* readFromOverUnderLR(const StImagePlane& theSrc,
                                    GLubyte*            theDataPtr,
                                    StImagePlane&       theDataL,
                                    StImagePlane&       theDataR) {
    if(theSrc.isNull()) {
        return theDataPtr;
    }

    const size_t srcDataSizeYHalf = theSrc.getSizeY() / 2;
    const size_t anOutRowBytes    = getEvenNumber(theSrc.getSizeX() * theSrc.getSizePixelBytes());
    theDataL.initWrapper(theSrc.getFormat(), theDataPtr,
                         theSrc.getSizeX(), srcDataSizeYHalf,
                         anOutRowBytes);
    theDataR.initWrapper(theSrc.getFormat(), &theDataPtr[theDataL.getSizeBytes()],
                         theDataL.getSizeX(), theDataL.getSizeY(),
                         anOutRowBytes);

    const size_t aCopyRows      = stMin(theDataL.getSizeY(), srcDataSizeYHalf);
    const size_t aCopyRowBytes  = stMin(theDataL.getSizeX(), theSrc.getSizeX()) * theDataL.getSizePixelBytes();

    if(theDataL.getSizeRowBytes() == theSrc.getSizeRowBytes() && theSrc.isTopDown()) {
        // perform fat copy
        stMemCpy(theDataL.changeData(),
                 theSrc.getData(0, 0),
                 aCopyRows * aCopyRowBytes);
        stMemCpy(theDataR.changeData(),
                 theSrc.getData(srcDataSizeYHalf, 0),
                 aCopyRows * aCopyRowBytes);
    } else {
        // check if data is upside-down
        const size_t aRowTop    = theSrc.isTopDown() ? 0 : srcDataSizeYHalf;
        const size_t aRowBottom = theSrc.isTopDown() ? srcDataSizeYHalf : 0;

        // copy row by row
        const size_t aRowInc = theSrc.isTopDown() ? 1 : size_t(-1);
        size_t       aRowTo  = theSrc.isTopDown() ? 0 : (aCopyRows - 1);
        for(size_t aRow = 0; aRow < aCopyRows; ++aRow, aRowTo += aRowInc) {
            stMemCpy(theDataL.changeData(aRowTo, 0),
                     theSrc.getData(aRowTop + aRow, 0),
                     aCopyRowBytes);
        }
        aRowTo = theSrc.isTopDown() ? 0 : (aCopyRows - 1);
        for(size_t aRow = 0; aRow < aCopyRows; ++aRow, aRowTo += aRowInc) {
            stMemCpy(theDataR.changeData(aRowTo, 0),
                     theSrc.getData(aRowBottom + aRow, 0),
                     aCopyRowBytes);
        }
    }
    return &theDataPtr[2 * theDataL.getSizeBytes()];
}


static GLubyte* readFromRowInterlace(const StImagePlane& theSrc,
                                     GLubyte*            theDataPtr,
                                     StImagePlane&       theDataL,
                                     StImagePlane&       theDataR) {
    if(theSrc.isNull()) {
        return theDataPtr;
    }

    const size_t srcDataSizeYHalf = theSrc.getSizeY() / 2;
    const size_t anOutRowBytes = getEvenNumber(theSrc.getSizeX() * theSrc.getSizePixelBytes());
    theDataL.initWrapper(theSrc.getFormat(), theDataPtr,
                         theSrc.getSizeX(), srcDataSizeYHalf,
                         anOutRowBytes);
    theDataR.initWrapper(theSrc.getFormat(), &theDataPtr[theDataL.getSizeBytes()],
                         theDataL.getSizeX(), theDataL.getSizeY(),
                         anOutRowBytes);

    const size_t aCopyRows     = stMin(theDataL.getSizeY(), srcDataSizeYHalf);
    const size_t aCopyRowBytes = stMin(theDataL.getSizeX(), theSrc.getSizeX()) * theDataL.getSizePixelBytes();

    const size_t aSrcRowLeft   = theSrc.isTopDown() ? 0 : 1;
    const size_t aSrcRowRight  = theSrc.isTopDown() ? 1 : 0;

    // prepare iterator for bottom-up source data
    const size_t aRowInc = theSrc.isTopDown() ? 1 : size_t(-1);
    size_t       aRowTo  = theSrc.isTopDown() ? 0 : (aCopyRows - 1);

    // copy row by row
    for(size_t aRowFrom = 0; aRowFrom < aCopyRows; ++aRowFrom, aRowTo += aRowInc) {
        stMemCpy(theDataR.changeData(aRowTo, 0),
                 theSrc.getData(2 * aRowFrom + aSrcRowRight, 0),
                 aCopyRowBytes);
        stMemCpy(theDataL.changeData(aRowTo, 0),
                 theSrc.getData(2 * aRowFrom + aSrcRowLeft, 0),
                 aCopyRowBytes);
    }
    return &theDataPtr[2 * theDataL.getSizeBytes()];
}

static GLubyte* readFromTiled4X(const StImagePlane& theDataSrc,
                                GLubyte*            theDataOutPtr,
                                StImagePlane&       theDataOutL,
                                StImagePlane&       theDataOutR) {
    if(theDataSrc.isNull()) {
        return theDataOutPtr;
    }

    const size_t aDataSizeX = (theDataSrc.getSizeX() / 3) * 2;
    const size_t aDataSizeY = (theDataSrc.getSizeY() / 3) * 2;
    const size_t aDataSizeXHalf = aDataSizeX / 2;

    const size_t anOutRowBytes = getEvenNumber(aDataSizeX * theDataSrc.getSizePixelBytes());
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
    const size_t aRowInc = theDataSrc.isTopDown() ? 1 : size_t(-1);
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

static GLubyte* readFromMono(const StImagePlane& theSrc,
                             GLubyte*            theDataPtr,
                             StImagePlane&       theData) {
    if(theSrc.isNull()) {
        return theDataPtr;
    }

    const size_t anOutRowBytes = getEvenNumber(theSrc.getSizeX() * theSrc.getSizePixelBytes());
    theData.initWrapper(theSrc.getFormat(), theDataPtr,
                        theSrc.getSizeX(), theSrc.getSizeY(),
                        anOutRowBytes);

    if(theData.getSizeRowBytes() == theSrc.getSizeRowBytes() && theSrc.isTopDown()) {
        // perform fat copy
        stMemCpy(theData.changeData(), theSrc.getData(),
                 stMin(theData.getSizeBytes(), theSrc.getSizeBytes()));
    } else {
        const size_t aCopyRows     = stMin(theData.getSizeY(), theSrc.getSizeY());
        const size_t aCopyRowBytes = stMin(theData.getSizeX(), theSrc.getSizeX()) * theData.getSizePixelBytes();
        const size_t aRowInc       = theSrc.isTopDown() ? 1 : size_t(-1);
        size_t       aRowTo        = theSrc.isTopDown() ? 0 : (aCopyRows - 1);
        for(size_t aRowFrom = 0; aRowFrom < aCopyRows; ++aRowFrom, aRowTo += aRowInc) {
            stMemCpy(theData.changeData(aRowTo, 0), theSrc.getData(aRowFrom, 0), aCopyRowBytes);
        }
    }
    return &theDataPtr[theData.getSizeBytes()];
}

/**
 * Compute buffer size to fit image copy
 * with reserve for different source formats.
 */
inline size_t computeBufferSize(const StImage& theData) {
    if(theData.isNull()) {
        return 0;
    }
    size_t aBufferSize = 0;
    for(size_t aPlaneId = 0; aPlaneId < 4; ++aPlaneId) {
        const StImagePlane& aPlane = theData.getPlane(aPlaneId);
        size_t sizeX_4 = getAligned(aPlane.getSizeX(), 4);
        size_t sizeY_4 = getAligned(aPlane.getSizeY(), 4);
        aBufferSize += sizeX_4 * sizeY_4 * aPlane.getSizePixelBytes();
    }
    return aBufferSize;
}

void StGLTextureData::updateData(const StImage&                  theDataL,
                                 const StImage&                  theDataR,
                                 const StHandle<StStereoParams>& theStParams,
                                 const StFormatEnum              theFormat,
                                 const double                    thePts) {
    // setup new stereo source
    myStParams  = theStParams;
    myPts       = thePts;
    mySrcFormat = theFormat != ST_V_SRC_AUTODETECT ? theFormat : ST_V_SRC_MONO;

    // reset fill texture state
    myFillRows = myFillFromRow = 0;

    // reallocate buffer if needed
    const size_t aNewSizeBytes = computeBufferSize(theDataL) + computeBufferSize(theDataR);
    if(aNewSizeBytes == 0) {
        // invalid data
        myDataL.nullify();
        myDataR.nullify();
        return;
    }

    reAllocate(aNewSizeBytes);
    myDataL.setColorModel(theDataL.getColorModel());
    myDataL.setColorScale(theDataL.getColorScale());
    myDataR.setColorModel(theDataR.isNull() ? theDataL.getColorModel() : theDataR.getColorModel());
    myDataR.setColorScale(theDataR.isNull() ? theDataL.getColorScale() : theDataR.getColorScale());
    myDataL.setPixelRatio(theDataL.getPixelRatio());
    myDataR.setPixelRatio(theDataL.getPixelRatio());

    switch(mySrcFormat) {
        case ST_V_SRC_SIDE_BY_SIDE:
        case ST_V_SRC_PARALLEL_PAIR: {
            GLubyte* aDataDispl = myDataPtr;
            for(size_t aPlaneId = 0; aPlaneId < 4; ++aPlaneId) {
                aDataDispl = readFromParallel(theDataL.getPlane(aPlaneId), aDataDispl,
                                              (mySrcFormat == ST_V_SRC_PARALLEL_PAIR) ? myDataL.changePlane(aPlaneId) : myDataR.changePlane(aPlaneId),
                                              (mySrcFormat == ST_V_SRC_PARALLEL_PAIR) ? myDataR.changePlane(aPlaneId) : myDataL.changePlane(aPlaneId));
            }
            break;
        }
        case ST_V_SRC_OVER_UNDER_RL:
        case ST_V_SRC_OVER_UNDER_LR: {
            GLubyte* aDataDispl = myDataPtr;
            for(size_t aPlaneId = 0; aPlaneId < 4; ++aPlaneId) {
                aDataDispl = readFromOverUnderLR(theDataL.getPlane(aPlaneId), aDataDispl,
                                                 (mySrcFormat == ST_V_SRC_OVER_UNDER_LR) ? myDataL.changePlane(aPlaneId) : myDataR.changePlane(aPlaneId),
                                                 (mySrcFormat == ST_V_SRC_OVER_UNDER_LR) ? myDataR.changePlane(aPlaneId) : myDataL.changePlane(aPlaneId));
            }
            break;
        }
        case ST_V_SRC_ROW_INTERLACE: {
            myDataL.setPixelRatio(theDataL.getPixelRatio() * 0.5f);
            myDataR.setPixelRatio(theDataL.getPixelRatio() * 0.5f);
            GLubyte* aDataDispl = myDataPtr;
            // TODO (Kirill Gavrilov#9) wrong for yuv420p?
            for(size_t aPlaneId = 0; aPlaneId < 4; ++aPlaneId) {
                aDataDispl = readFromRowInterlace(theDataL.getPlane(aPlaneId), aDataDispl,
                                                  myDataL.changePlane(aPlaneId), myDataR.changePlane(aPlaneId));

            }
            break;
        }
        case ST_V_SRC_PAGE_FLIP:
        case ST_V_SRC_SEPARATE_FRAMES: {
            myDataR.setColorModel(theDataR.getColorModel());
            myDataR.setPixelRatio(theDataR.getPixelRatio());
            GLubyte* aDataDispl = myDataPtr;
            for(size_t aPlaneId = 0; aPlaneId < 4; ++aPlaneId) {
                aDataDispl = readFromMono(theDataL.getPlane(aPlaneId), aDataDispl, myDataL.changePlane(aPlaneId));
            }
            for(size_t aPlaneId = 0; aPlaneId < 4; ++aPlaneId) {
                aDataDispl = readFromMono(theDataR.getPlane(aPlaneId), aDataDispl, myDataR.changePlane(aPlaneId));
            }
            break;
        }
        case ST_V_SRC_TILED_4X: {
            GLubyte* aDataDispl = myDataPtr;
            for(size_t aPlaneId = 0; aPlaneId < 4; ++aPlaneId) {
                aDataDispl = readFromTiled4X(theDataL.getPlane(aPlaneId), aDataDispl,
                                             myDataL.changePlane(aPlaneId), myDataR.changePlane(aPlaneId));
            }
            break;
        }
        case ST_V_SRC_ANAGLYPH_RED_CYAN:
        case ST_V_SRC_ANAGLYPH_G_RB:
        case ST_V_SRC_ANAGLYPH_YELLOW_BLUE:
        case ST_V_SRC_VERTICAL_INTERLACE: // not supported
        case ST_V_SRC_MONO:
        default: {
            GLubyte* aDataDispl = myDataPtr;
            for(size_t aPlaneId = 0; aPlaneId < 4; ++aPlaneId) {
                aDataDispl = readFromMono(theDataL.getPlane(aPlaneId), aDataDispl, myDataL.changePlane(aPlaneId));
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
    theFrameTexture.fill(theCtx, theData, myFillFromRow, myFillFromRow + myFillRows);
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
    theTextureFrame.setColorModel(theImage.getColorModel(),
                                  theImage.getColorScale());
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
    if(myFillRows == 0 || myFillFromRow == 0) {
        // prepare textures for new data
        prepareTextures(theCtx, myDataL, theQTexture.getBack(StGLQuadTexture::LEFT_TEXTURE));
        prepareTextures(theCtx, myDataR, theQTexture.getBack(StGLQuadTexture::RIGHT_TEXTURE));

        // remove links to old stereo parameters
        theQTexture.getBack(StGLQuadTexture::LEFT_TEXTURE).setSource(StHandle<StStereoParams>());
        theQTexture.getBack(StGLQuadTexture::RIGHT_TEXTURE).setSource(StHandle<StStereoParams>());

        // TODO (Kirill Gavrilov#9) this value is meanfull only for PageFlip,
        //                          also rows number may be replaced with bytes count
        static const GLsizei UPDATED_ROWS_MAX = 1088; // we use optimal value to update 1080p video frame at-once
        GLsizei maxRows    = stMin(GLsizei(myDataL.getSizeY()), theQTexture.getBack(StGLQuadTexture::LEFT_TEXTURE).getSizeY());
        GLsizei iterations = (maxRows / (UPDATED_ROWS_MAX * 2)) + 1;
        if(!myDataR.isNull()) {
            maxRows    = stMax(maxRows, stMin(GLsizei(myDataR.getSizeY()), theQTexture.getBack(StGLQuadTexture::RIGHT_TEXTURE).getSizeY()));
            iterations = maxRows / UPDATED_ROWS_MAX + 1;
        }
        myFillRows = maxRows / iterations;
        myFillFromRow = 0;
    }

    if(myFillRows == 0) {
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

    myFillFromRow += myFillRows;
    if(myFillFromRow >= GLsizei(myDataL.getSizeY())
    && (myDataR.isNull() || myFillFromRow >= GLsizei(myDataR.getSizeY()))) {
        if(!myDataL.isNull() && theQTexture.getBack(StGLQuadTexture::LEFT_TEXTURE).isValid()) {
            setupAttributes(theQTexture.getBack(StGLQuadTexture::LEFT_TEXTURE), myDataL);
        }
        if(!myDataR.isNull() && theQTexture.getBack(StGLQuadTexture::RIGHT_TEXTURE).isValid()) {
            setupAttributes(theQTexture.getBack(StGLQuadTexture::RIGHT_TEXTURE), myDataR);
        }

        if(!myStParams.isNull()) {
            myStParams->StereoFormat = mySrcFormat;
        }
        return true;
    } else {
        return false;
    }
}

void StGLTextureData::getCopy(StImage* theDataL,
                              StImage* theDataR) const {
    if(theDataL != NULL) {
        theDataL->initCopy(myDataL);
    }
    if(theDataR != NULL) {
        theDataR->initCopy(myDataR);
    }
}
