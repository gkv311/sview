/**
 * Copyright © 2009-2020 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#include <StGLStereo/StGLTextureData.h>
#include <StStrings/StLogger.h>

#include <StGLCore/StGLCore11.h>
#include <StGL/StGLContext.h>

#include <StAV/StAVImage.h>

StGLTextureData::StGLTextureData(const StHandle<StGLTextureUploadParams>& theUploadParams)
: myPrev(NULL),
  myNext(NULL),
  myDataPtr(NULL),
  myDataSizeBytes(0),
  myStParams(),
  myPts(0.0),
  mySrcFormat(StFormat_AUTO),
  myCubemapFormat(StCubemap_OFF),
  myUploadParams(theUploadParams),
  myFillFromRow(0),
  myFillRows(0) {
    //
}

StGLTextureData::~StGLTextureData() {
    reset();
}

void StGLTextureData::reset() {
    myDataPair.nullify();
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

/**
 * Determine packed cubemap format (6:1 or 3:2).
 */
inline bool checkCubeMap(const StImagePlane& thePlane,
                         size_t&             theCoeffX,
                         size_t&             theCoeffY) {
    if(thePlane.isNull()) {
        return true;
    }

    if(theCoeffX == 0) {
        if(thePlane.getSizeX() / 6 == thePlane.getSizeY()) {
            theCoeffX = 6;
            theCoeffY = 1;
            return true;
        } else if(thePlane.getSizeY() / 6 == thePlane.getSizeX()) {
            theCoeffX = 1;
            theCoeffY = 6;
            return true;
        } else if(thePlane.getSizeX() / 3 == thePlane.getSizeY() / 2) {
            theCoeffX = 3;
            theCoeffY = 2;
            return true;
        } else if(thePlane.getSizeX() / 2 == thePlane.getSizeY() / 3) {
            theCoeffX = 2;
            theCoeffY = 3;
            return true;
        }
        return false;
    }
    return thePlane.getSizeX() / theCoeffX == thePlane.getSizeY() / theCoeffY;
}

void StGLTextureData::copyProps(const StImage& theDataL,
                                const StImage& theDataR) {
    myDataPair.setColorModel(theDataL.getColorModel());
    myDataPair.setColorScale(theDataL.getColorScale());
    myDataPair.setPixelRatio(theDataL.getPixelRatio());
    myDataL.setColorModel(theDataL.getColorModel());
    myDataL.setColorScale(theDataL.getColorScale());
    myDataL.setPixelRatio(theDataL.getPixelRatio());
    myDataR.setColorModel(theDataR.isNull() ? theDataL.getColorModel() : theDataR.getColorModel());
    myDataR.setColorScale(theDataR.isNull() ? theDataL.getColorScale() : theDataR.getColorScale());
    myDataR.setPixelRatio(theDataR.isNull() ? theDataL.getPixelRatio() : theDataR.getPixelRatio());
}

inline bool canCopyReference(const StImage& theData) {
    if(theData.isNull()) {
        return true;
    }

    return !theData.getBufferCounter().isNull()
        &&  theData.isTopDown();
}

void StGLTextureData::updateData(const StGLDeviceCaps&           theDeviceCaps,
                                 const StImage&                  theDataL,
                                 const StImage&                  theDataR,
                                 const StHandle<StStereoParams>& theStParams,
                                 const StFormat                  theFormat,
                                 const StCubemap                 theCubemap,
                                 const double                    thePts) {
    // setup new stereo source
    myStParams  = theStParams;
    myPts       = thePts;
    mySrcFormat = theFormat != StFormat_AUTO ? theFormat : StFormat_Mono;

    // reset fill texture state
    myFillRows = myFillFromRow = 0;

    if(canCopyReference(theDataL)
    && canCopyReference(theDataR)) {
        bool toCopy = false;
        switch(mySrcFormat) {
            case StFormat_SideBySide_LR:
            case StFormat_SideBySide_RL: {
                if(!theDeviceCaps.hasUnpack) {
                    // slow copying to GPU memory
                    toCopy = true;
                    break;
                }

                reset();
                copyProps(theDataL, theDataR);
                StImage& aDataL = (mySrcFormat == StFormat_SideBySide_LR)
                                ? myDataL : myDataR;
                StImage& aDataR = (mySrcFormat == StFormat_SideBySide_LR)
                                ? myDataR : myDataL;
                myDataPair.initReference(theDataL);
                for(size_t aPlaneId = 0; aPlaneId < 4; ++aPlaneId) {
                    const StImagePlane& aFromPlane = myDataPair.getPlane(aPlaneId);
                    if(aFromPlane.isNull()) {
                        continue;
                    }
                    const size_t aSizeX = aFromPlane.getSizeX() / 2;
                    aDataL.changePlane(aPlaneId).initWrapper(aFromPlane.getFormat(),
                                                             aFromPlane.accessData(0, 0),
                                                             aSizeX, aFromPlane.getSizeY(),
                                                             aFromPlane.getSizeRowBytes());
                    aDataR.changePlane(aPlaneId).initWrapper(aFromPlane.getFormat(),
                                                             aFromPlane.accessData(0, aSizeX),
                                                             aSizeX, aFromPlane.getSizeY(),
                                                             aFromPlane.getSizeRowBytes());
                }
                break;
            }
            case StFormat_TopBottom_LR:
            case StFormat_TopBottom_RL: {
                if(!theDeviceCaps.hasUnpack
                &&  theCubemap != StCubemap_OFF) {
                    // slow copying to GPU memory
                    toCopy = true;
                    break;
                }

                reset();
                copyProps(theDataL, theDataR);
                StImage& aDataL = (mySrcFormat == StFormat_TopBottom_LR)
                                ? myDataL : myDataR;
                StImage& aDataR = (mySrcFormat == StFormat_TopBottom_LR)
                                ? myDataR : myDataL;
                myDataPair.initReference(theDataL);
                for(size_t aPlaneId = 0; aPlaneId < 4; ++aPlaneId) {
                    const StImagePlane& aFromPlane = myDataPair.getPlane(aPlaneId);
                    if(aFromPlane.isNull()) {
                        continue;
                    }
                    const size_t aSizeY = aFromPlane.getSizeY() / 2;
                    aDataL.changePlane(aPlaneId).initWrapper(aFromPlane.getFormat(),
                                                             aFromPlane.accessData(0, 0),
                                                             aFromPlane.getSizeX(), aSizeY,
                                                             aFromPlane.getSizeRowBytes());
                    aDataR.changePlane(aPlaneId).initWrapper(aFromPlane.getFormat(),
                                                             aFromPlane.accessData(aSizeY, 0),
                                                             aFromPlane.getSizeX(), aSizeY,
                                                             aFromPlane.getSizeRowBytes());
                }
                break;
            }
            case StFormat_Rows: {
                if(!theDeviceCaps.hasUnpack) {
                    // slow copying to GPU memory
                    toCopy = true;
                    break;
                }

                reset();
                copyProps(theDataL, theDataR);
                myDataL.setPixelRatio(theDataL.getPixelRatio() * 0.5f);
                myDataR.setPixelRatio(theDataL.getPixelRatio() * 0.5f);
                StImage& aDataL = myDataL;
                StImage& aDataR = myDataR;
                myDataPair.initReference(theDataL);
                for(size_t aPlaneId = 0; aPlaneId < 4; ++aPlaneId) {
                    const StImagePlane& aFromPlane = myDataPair.getPlane(aPlaneId);
                    if(aFromPlane.isNull()) {
                        continue;
                    }
                    const size_t aSizeY = aFromPlane.getSizeY() / 2;
                    aDataL.changePlane(aPlaneId).initWrapper(aFromPlane.getFormat(),
                                                             aFromPlane.accessData(0, 0),
                                                             aFromPlane.getSizeX(), aSizeY,
                                                             aFromPlane.getSizeRowBytes() * 2);
                    aDataR.changePlane(aPlaneId).initWrapper(aFromPlane.getFormat(),
                                                             aFromPlane.accessData(1, 0),
                                                             aFromPlane.getSizeX(), aSizeY,
                                                             aFromPlane.getSizeRowBytes() * 2);
                }
                break;
            }
            case StFormat_Columns:
            case StFormat_Tiled4x: {
                toCopy = true;
                break;
            }
            case StFormat_Mono:
            case StFormat_SeparateFrames:
            case StFormat_FrameSequence:
            case StFormat_AnaglyphRedCyan:
            case StFormat_AnaglyphGreenMagenta:
            case StFormat_AnaglyphYellowBlue:
            case StFormat_AUTO: {
                if(!theDeviceCaps.hasUnpack
                &&  theCubemap != StCubemap_OFF) {
                    // slow copying to GPU memory
                    toCopy = true;
                    break;
                }

                reset();
                copyProps(theDataL, theDataR);
                myDataL.initReference(theDataL);
                myDataR.initReference(theDataR);
                break;
            }
            case StFormat_NB: {
                reset();
                return;
            }
        }

        if(!toCopy) {
            validateCubemap(theCubemap);
            return;
        }
    }

    myDataPair.setBufferCounter(NULL);
    myDataL.setBufferCounter(NULL);
    myDataR.setBufferCounter(NULL);

    // reallocate buffer if needed
    const size_t aNewSizeBytes = computeBufferSize(theDataL) + computeBufferSize(theDataR);
    if(aNewSizeBytes == 0) {
        // invalid data
        myDataPair.nullify();
        myDataL.nullify();
        myDataR.nullify();
        myCubemapFormat = StCubemap_OFF;
        return;
    }

    reAllocate(aNewSizeBytes);
    copyProps(theDataL, theDataR);

    switch(mySrcFormat) {
        case StFormat_SideBySide_LR:
        case StFormat_SideBySide_RL: {
            GLubyte* aDataDispl = myDataPtr;
            for(size_t aPlaneId = 0; aPlaneId < 4; ++aPlaneId) {
                aDataDispl = readFromParallel(theDataL.getPlane(aPlaneId), aDataDispl,
                                              (mySrcFormat == StFormat_SideBySide_LR) ? myDataL.changePlane(aPlaneId) : myDataR.changePlane(aPlaneId),
                                              (mySrcFormat == StFormat_SideBySide_LR) ? myDataR.changePlane(aPlaneId) : myDataL.changePlane(aPlaneId));
            }
            break;
        }
        case StFormat_TopBottom_LR:
        case StFormat_TopBottom_RL: {
            GLubyte* aDataDispl = myDataPtr;
            for(size_t aPlaneId = 0; aPlaneId < 4; ++aPlaneId) {
                aDataDispl = readFromOverUnderLR(theDataL.getPlane(aPlaneId), aDataDispl,
                                                 (mySrcFormat == StFormat_TopBottom_LR) ? myDataL.changePlane(aPlaneId) : myDataR.changePlane(aPlaneId),
                                                 (mySrcFormat == StFormat_TopBottom_LR) ? myDataR.changePlane(aPlaneId) : myDataL.changePlane(aPlaneId));
            }
            break;
        }
        case StFormat_Rows: {
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
        case StFormat_FrameSequence:
        case StFormat_SeparateFrames: {
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
        case StFormat_Tiled4x: {
            GLubyte* aDataDispl = myDataPtr;
            for(size_t aPlaneId = 0; aPlaneId < 4; ++aPlaneId) {
                aDataDispl = readFromTiled4X(theDataL.getPlane(aPlaneId), aDataDispl,
                                             myDataL.changePlane(aPlaneId), myDataR.changePlane(aPlaneId));
            }
            break;
        }
        case StFormat_AnaglyphRedCyan:
        case StFormat_AnaglyphGreenMagenta:
        case StFormat_AnaglyphYellowBlue:
        case StFormat_Columns: // not supported
        case StFormat_Mono:
        default: {
            GLubyte* aDataDispl = myDataPtr;
            for(size_t aPlaneId = 0; aPlaneId < 4; ++aPlaneId) {
                aDataDispl = readFromMono(theDataL.getPlane(aPlaneId), aDataDispl, myDataL.changePlane(aPlaneId));
            }
            break;
        }
    }
    validateCubemap(theCubemap);
}

void StGLTextureData::validateCubemap(const StCubemap theCubemap) {
    if(theCubemap != StCubemap_Packed
    && theCubemap != StCubemap_PackedEAC) {
        myCubemapFormat = StCubemap_OFF;
        return;
    }

    myCubemapFormat = theCubemap;
    if(theCubemap == StCubemap_PackedEAC) {
        return;
    }

    size_t aCoeffs[2] = {0, 0};
    if(!myDataL.isNull()) {
        for(size_t aPlaneId = 0; aPlaneId < 4; ++aPlaneId) {
            const StImagePlane& aPlane = myDataL.getPlane(aPlaneId);
            if(!checkCubeMap(aPlane, aCoeffs[0], aCoeffs[1])) {
                myCubemapFormat = StCubemap_OFF;
                return;
            }
        }
    }
    if(!myDataR.isNull()) {
        for(size_t aPlaneId = 0; aPlaneId < 4; ++aPlaneId) {
            const StImagePlane& aPlane = myDataR.getPlane(aPlaneId);
            if(!checkCubeMap(aPlane, aCoeffs[0], aCoeffs[1])) {
                myCubemapFormat = StCubemap_OFF;
                return;
            }
        }
    }
}

void StGLTextureData::fillTexture(StGLContext&        theCtx,
                                  StGLFrameTexture&   theFrameTexture,
                                  const StImagePlane& theData) {
    if(!theFrameTexture.isValid() || theData.isNull()) {
        return;
    }

    if(myCubemapFormat != StCubemap_Packed
    && myCubemapFormat != StCubemap_PackedEAC) {
        theFrameTexture.fillPatch(theCtx, theData, GL_TEXTURE_2D, myFillFromRow, myFillFromRow + myFillRows);
        return;
    }

    size_t aCoeffs[2] = {0, 0};
    if(myCubemapFormat == StCubemap_PackedEAC) {
        if(theData.getSizeX() > theData.getSizeY()) {
            aCoeffs[0] = 3;
            aCoeffs[1] = 2;
        } else {
            aCoeffs[0] = 2;
            aCoeffs[1] = 3;
        }
    } else {
        if(!checkCubeMap(theData, aCoeffs[0], aCoeffs[1])) {
            return;
        }
    }

    const size_t aPatchX = theData.getSizeX() / aCoeffs[0];
    const size_t aPatchY = theData.getSizeY() / aCoeffs[1];
    if(aPatchX < 2 || aPatchY < 2) {
        return;
    }

    static const GLenum THE_SIDES_GL[6] = { GL_TEXTURE_CUBE_MAP_POSITIVE_X, GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
                                            GL_TEXTURE_CUBE_MAP_POSITIVE_Y, GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
                                            GL_TEXTURE_CUBE_MAP_POSITIVE_Z, GL_TEXTURE_CUBE_MAP_NEGATIVE_Z };
    static const GLenum THE_SIDES_EAC32[6] = { GL_TEXTURE_CUBE_MAP_POSITIVE_X, GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
                                               GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, GL_TEXTURE_CUBE_MAP_POSITIVE_Z, GL_TEXTURE_CUBE_MAP_POSITIVE_Y };
    static const GLenum THE_SIDES_EAC23[6] = { GL_TEXTURE_CUBE_MAP_NEGATIVE_X, GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
                                               GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
                                               GL_TEXTURE_CUBE_MAP_POSITIVE_X, GL_TEXTURE_CUBE_MAP_NEGATIVE_Y };

    const GLenum* aTargets = myCubemapFormat == StCubemap_PackedEAC
                           ? (aCoeffs[1] == 3 ? THE_SIDES_EAC23 : THE_SIDES_EAC32)
                           : THE_SIDES_GL;
    for(size_t aTargetIter = 0; aTargetIter < 6; ++aTargetIter) {
        StImagePlane aPlane;
        size_t aTop = 0, aLeft = 0;
        bool toTranspose = false;
        switch(aCoeffs[1]) {
            case 1: { // 6x1
                aLeft = aPatchX * aTargetIter;
                aTop  = 0;
                break;
            }
            case 2: { // 3x2
                if(aTargetIter >= 3) {
                    // second row
                    aLeft = aPatchX * (aTargetIter - 3);
                    aTop  = aPatchY;
                    if(myCubemapFormat == StCubemap_PackedEAC) {
                        toTranspose = true;
                    }
                } else {
                    // first row
                    aLeft = aPatchX * aTargetIter;
                    aTop  = 0;
                }
                break;
            }
            case 3: { // 2x3
                if(aTargetIter >= 4) {
                    // third row
                    aLeft = aPatchX * (aTargetIter - 4);
                    aTop  = aPatchY * 2;
                } else if(aTargetIter >= 2) {
                    // second row
                    aLeft = aPatchX * (aTargetIter - 2);
                    aTop  = aPatchY;
                } else {
                    // first row
                    aLeft = aPatchX * aTargetIter;
                    aTop  = 0;
                }

                if(myCubemapFormat == StCubemap_PackedEAC
                && (aTargetIter % 2) == 0) {
                    toTranspose = true;
                }
                break;
            }
            case 6: { // 1x6
                aLeft = 0;
                aTop  = aPatchY * aTargetIter;
                break;
            }
        }
        if(!aPlane.initWrapper(theData.getFormat(), const_cast<GLubyte* >(theData.getData(aTop, aLeft)),
                               aPatchX, aPatchY, theData.getSizeRowBytes())) {
            ST_DEBUG_LOG("StGLTextureData::fillTexture(). wrapping failure");
            continue;
        }

        StImagePlane* aResPlane = &aPlane;

        // this is too slow without multi-threading
        (void )toTranspose;
        /*if(toTranspose) {
            StImagePlane& aTmpPlane = theCtx.getTmpImagePlane1();
            aTmpPlane.initTransposedCopy(aPlane, aCoeffs[1] != 3);
            aResPlane = &aTmpPlane;
        }

        if(aPatchX != aPatchY) {
            size_t aPatch = stMax(aPatchX, aPatchY);
            StImagePlane& aTmpPlane2 = theCtx.getTmpImagePlane2();
            if(aTmpPlane2.getFormat() != aResPlane->getFormat()
            || aTmpPlane2.getSizeX() != aResPlane->getSizeX()
            || aTmpPlane2.getSizeY() != aResPlane->getSizeY()) {
                aTmpPlane2.initTrash(aResPlane->getFormat(), aPatch, aPatch);
            }
            if(!aTmpPlane2.isNull()) {
                StAVImage::resizePlane(*aResPlane, aTmpPlane2);
                aResPlane = &aTmpPlane2;
            }
        }*/

        theFrameTexture.fillPatch(theCtx, *aResPlane, aTargets[aTargetIter], myFillFromRow, myFillFromRow + myFillRows);
    }
}

void StGLTextureData::setupDataRectangle(const StImagePlane& theImagePlane,
                                         const GLfloat       thePixelRatio,
                                         StGLFrameTexture&   theTextureFrame) {
    if(theImagePlane.isNull() || !theTextureFrame.isValid()) {
        return;
    }

    size_t anImgSizeX = theImagePlane.getSizeX();
    size_t anImgSizeY = theImagePlane.getSizeY();

    StPanorama aPano = StPanorama_OFF;
    size_t aCoeffs[2] = {0, 0};
    if(myCubemapFormat == StCubemap_Packed
    && checkCubeMap(theImagePlane, aCoeffs[0], aCoeffs[1])) {
        anImgSizeX = anImgSizeX / aCoeffs[0];
        anImgSizeY = anImgSizeY / aCoeffs[1];
        switch(aCoeffs[0]) {
            case 1: aPano = StPanorama_Cubemap1_6; break;
            case 3: aPano = StPanorama_Cubemap3_2; break;
            case 6: aPano = StPanorama_Cubemap6_1; break;
        }
    } else if(myCubemapFormat == StCubemap_PackedEAC) {
        if(theImagePlane.getSizeX() > theImagePlane.getSizeY()) {
            aCoeffs[0] = 3;
            aCoeffs[1] = 2;
            aPano = StPanorama_Cubemap3_2ytb;
        } else {
            aCoeffs[0] = 2;
            aCoeffs[1] = 3;
            aPano = StPanorama_Cubemap2_3ytb;
        }
        anImgSizeX = anImgSizeX / aCoeffs[0];
        anImgSizeY = anImgSizeY / aCoeffs[1];
    }

    const GLfloat aSizeXFloat = stMin(GLfloat(anImgSizeX), GLfloat(theTextureFrame.getSizeX()));
    const GLfloat aSizeYFloat = stMin(GLfloat(anImgSizeY), GLfloat(theTextureFrame.getSizeY()));
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
    theTextureFrame.setPixelRatio(thePixelRatio);
    theTextureFrame.setPackedPanorama(aPano);
}

void StGLTextureData::setupAttributes(StGLFrameTextures& stFrameTextures, const StImage& theImage) {
    for(size_t aPlaneId = 0; aPlaneId < 4; ++aPlaneId) {
        setupDataRectangle(theImage.getPlane(aPlaneId), theImage.getPixelRatio(), stFrameTextures.getPlane(aPlaneId));
    }
    stFrameTextures.setSource(myStParams);
}

static void prepareTextures(StGLContext&       theCtx,
                            const StImage&     theImage,
                            const StCubemap    theCubemap,
                            StGLFrameTextures& theTextureFrame) {
    GLint anInternalFormat = GL_RGB8;
    theTextureFrame.setColorModel(theImage.getColorModel(),
                                  theImage.getColorScale());
    for(size_t aPlaneId = 0; aPlaneId < 4; ++aPlaneId) {
        const StImagePlane& anImgPlane = theImage.getPlane(aPlaneId);
        StGLFrameTexture&   aTexture   = theTextureFrame.getPlane(aPlaneId);
        if(anImgPlane.isNull()) {
            aTexture.release(theCtx);
            continue;
        }

        if(!StGLTexture::getInternalFormat(theCtx, anImgPlane.getFormat(), anInternalFormat)) {
            aTexture.release(theCtx);
            continue;
        }
        GLenum  aTarget = GL_TEXTURE_2D;
        GLsizei aSizeX  = (GLsizei )anImgPlane.getSizeX();
        GLsizei aSizeY  = (GLsizei )anImgPlane.getSizeY();
        if(theCubemap == StCubemap_Packed) {
            size_t aCoeffs[2] = {0, 0};
            if(!checkCubeMap(anImgPlane, aCoeffs[0], aCoeffs[1])) {
                aTexture.release(theCtx);
                continue;
            }
            aTarget = GL_TEXTURE_CUBE_MAP;
            aSizeX  = aSizeX / GLsizei(aCoeffs[0]);
            aSizeY  = aSizeY / GLsizei(aCoeffs[1]);
            if(aSizeX < 1) {
                aTexture.release(theCtx);
                continue;
            }
            aSizeX  = stMax(aSizeX, aSizeY); // cubemap requires squared images
            aSizeY  = stMax(aSizeX, aSizeY);
        } else if(theCubemap == StCubemap_PackedEAC) {
            if(aSizeX > aSizeY) {
                aSizeX /= 3;
                aSizeY /= 2;
            } else {
                aSizeX /= 2;
                aSizeY /= 3;
            }
            aTarget = GL_TEXTURE_CUBE_MAP;
            aSizeX  = stMax(aSizeX, aSizeY); // cubemap requires squared images
            aSizeY  = stMax(aSizeX, aSizeY);
        }
        if(aSizeX < 1) {
            aTexture.release(theCtx);
            continue;
        }
        theTextureFrame.preparePlane(theCtx,
                                     aPlaneId,
                                     aSizeX, aSizeY,
                                     anInternalFormat,
                                     aTarget);
    }
}

bool StGLTextureData::fillTexture(StGLContext&     theCtx,
                                  StGLQuadTexture& theQTexture) {

    // setup rows count to be filled per fillTexture()
    if(myFillRows == 0 || myFillFromRow == 0) {
        // prepare textures for new data
        prepareTextures(theCtx, myDataL, myCubemapFormat, theQTexture.getBack(StGLQuadTexture::LEFT_TEXTURE));
        prepareTextures(theCtx, myDataR, myCubemapFormat, theQTexture.getBack(StGLQuadTexture::RIGHT_TEXTURE));

        // remove links to old stereo parameters
        theQTexture.getBack(StGLQuadTexture::LEFT_TEXTURE).setSource(StHandle<StStereoParams>());
        theQTexture.getBack(StGLQuadTexture::RIGHT_TEXTURE).setSource(StHandle<StStereoParams>());

        const int aNbRowsL = stMin(int(myDataL.getSizeY()), theQTexture.getBack(StGLQuadTexture::LEFT_TEXTURE).getSizeY());
        const int aNbRowsR = !myDataR.isNull()
                           ? stMin(int(myDataR.getSizeY()), theQTexture.getBack(StGLQuadTexture::RIGHT_TEXTURE).getSizeY())
                           : 0;
        const int aNbMaxRows = stMax(aNbRowsL, aNbRowsR);

        const int aMaxUploadChunkMiB   = myUploadParams->MaxUploadChunkMiB;
        const int aMaxUploadIterations = myUploadParams->MaxUploadIterations;
        int aNbIters = 1;
        if(aMaxUploadChunkMiB > 0 && aMaxUploadIterations > 1) {
            size_t aStride = 0;
            for(int aPlaneIter = 0; aPlaneIter < 4; ++aPlaneIter) {
                const StImagePlane& aPlaneL = myDataL.getPlane(aPlaneIter);
                if(!aPlaneL.isNull()) {
                    // don't use aPlane.getSizeRowBytes() here since it may contain extra padding for side-by-side input
                    aStride += aPlaneL.getSizeX() * aPlaneL.getSizePixelBytes();
                }
                if(!myDataR.isNull()) {
                    const StImagePlane& aPlaneR = myDataR.getPlane(aPlaneIter);
                    if(!aPlaneR.isNull()) {
                        aStride += aPlaneR.getSizeX() * aPlaneR.getSizePixelBytes();
                    }
                }
            }

            const int aNbMaxFrameRows = int((size_t(aMaxUploadChunkMiB) * 1024 * 1024) / aStride);
            aNbIters = stMin(aMaxUploadIterations, aNbMaxRows / aNbMaxFrameRows);
        }
        myFillRows = (aNbIters > 0) ? (aNbMaxRows / aNbIters) : aNbMaxRows;
        if(myCubemapFormat == StCubemap_Packed || myCubemapFormat == StCubemap_PackedEAC) {
            myFillRows = INT_MAX; /// TODO handle cube maps incremental updates specificall
        }
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
        theDataL->initCopy(myDataL, true);
    }
    if(theDataR != NULL) {
        theDataR->initCopy(myDataR, true);
    }
}
