/**
 * Copyright © 2010-2020 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#include <StImage/StImagePlane.h>

StString StImagePlane::formatImgFormat(ImgFormat theImgFormat) {
    switch(theImgFormat) {
        case ImgGray:    return "ImgGray";
        case ImgGray16:  return "ImgGray16";
        case ImgRGB:     return "ImgRGB";
        case ImgBGR:     return "ImgBGR";
        case ImgRGB32:   return "ImgRGB32";
        case ImgBGR32:   return "ImgBGR32";
        case ImgRGB48:   return "ImgRGB48";
        case ImgRGBA:    return "ImgRGBA";
        case ImgBGRA:    return "ImgBGRA";
        case ImgRGBA64:  return "ImgRGBA64";
        case ImgGrayF:   return "ImgGrayF";
        case ImgRGBF:    return "ImgRGBF";
        case ImgBGRF:    return "ImgBGRF";
        case ImgRGBAF:   return "ImgRGBAF";
        case ImgBGRAF:   return "ImgBGRAF";
        case ImgUV:      return "ImgUV";
        case ImgUNKNOWN: return "ImgUNKNOWN";
    }
    return "unknown";
}

StImagePlane::StImagePlane()
: myDataPtr(NULL),
  mySizeBPP(1),
  mySizeX(0),
  mySizeY(0),
  mySizeRowBytes(0),
  myImgFormat(StImagePlane::ImgGray),
  myIsOwnPointer(true),
  myIsTopBottom(true) {
    //
    setFormat(myImgFormat);
}

StImagePlane::~StImagePlane() {
    nullify();
}

void StImagePlane::setFormat(StImagePlane::ImgFormat thePixelFormat) {
    myImgFormat = thePixelFormat;
    switch(myImgFormat) {
        case ImgGrayF:
            mySizeBPP = sizeof(GLfloat);
            break;
        case ImgRGBAF:
        case ImgBGRAF:
            mySizeBPP = sizeof(GLfloat) * 4;
            break;
        case ImgRGBF:
        case ImgBGRF:
            mySizeBPP = sizeof(GLfloat) * 3;
            break;
        case ImgRGBA:
        case ImgBGRA:
            mySizeBPP = 4;
            break;
        case ImgRGBA64:
            mySizeBPP = 8;
            break;
        case ImgRGB32:
        case ImgBGR32:
            mySizeBPP = 4;
            break;
        case ImgRGB48:
            mySizeBPP = 6;
            break;
        case ImgRGB:
        case ImgBGR:
            mySizeBPP = 3;
            break;
        case ImgGray16:
            mySizeBPP = 2;
            break;
        case ImgUV:
            mySizeBPP = 2;
            break;
        case ImgGray:
        default:
            mySizeBPP = 1;
    }
}

bool StImagePlane::initWrapper(StImagePlane::ImgFormat thePixelFormat,
                               GLubyte*      theDataPtr,
                               const size_t  theSizeX,
                               const size_t  theSizeY,
                               const size_t  theSizeRowBytes) {
    nullify(thePixelFormat);
    if((theSizeX == 0) || (theSizeY == 0) || (theDataPtr == NULL)) {
        return false;
    }
    mySizeX = theSizeX;
    mySizeY = theSizeY;
    mySizeRowBytes = (theSizeRowBytes != 0) ? theSizeRowBytes : (theSizeX * mySizeBPP);
    myDataPtr = theDataPtr;
    myIsOwnPointer = false;
    return true;
}

bool StImagePlane::initTrash(StImagePlane::ImgFormat thePixelFormat,
                             const size_t theSizeX,
                             const size_t theSizeY,
                             const size_t theSizeRowBytes) {
    nullify(thePixelFormat);
    if((theSizeX == 0) || (theSizeY == 0)) {
        return false;
    }
    mySizeX = theSizeX;
    mySizeY = theSizeY;
    mySizeRowBytes = mySizeX * mySizeBPP;
    if(theSizeRowBytes > mySizeRowBytes) {
        // use argument only if it greater
        mySizeRowBytes = theSizeRowBytes;
    }
    myDataPtr = stMemAllocAligned<GLubyte*>(getSizeBytes());
    myIsOwnPointer = true;
    return myDataPtr != NULL;
}

bool StImagePlane::initZero(StImagePlane::ImgFormat thePixelFormat,
                            const size_t theSizeX,
                            const size_t theSizeY,
                            const size_t theSizeRowBytes,
                            const int theValue) {
    if(!initTrash(thePixelFormat, theSizeX, theSizeY, theSizeRowBytes)) {
        return false;
    }
    stMemSet(myDataPtr, theValue, getSizeBytes());
    return true;
}

bool StImagePlane::initCopy(const StImagePlane& theCopy,
                            const bool          theIsCompact) {
    if(!initTrash(theCopy.myImgFormat, theCopy.mySizeX, theCopy.mySizeY, theIsCompact ? 0 : theCopy.mySizeRowBytes)) {
        return false;
    }

    if(mySizeRowBytes == theCopy.mySizeRowBytes) {
        stMemCpy(changeData(), theCopy.getData(), theCopy.getSizeBytes());
        return true;
    }

    const size_t aCopyRowBytes = stMin(mySizeRowBytes, theCopy.mySizeRowBytes);
    for(size_t aRow = 0; aRow < mySizeY; ++aRow) {
        stMemCpy(changeData(aRow, 0), theCopy.getData(aRow, 0), aCopyRowBytes);
    }
    return true;
}

bool StImagePlane::initTransposedCopy(const StImagePlane& theCopy,
                                      const bool theIsClockwise) {
    if(myImgFormat != theCopy.myImgFormat
    || mySizeX != theCopy.mySizeX
    || mySizeY != theCopy.mySizeY) {
        if(!initTrash(theCopy.myImgFormat, theCopy.mySizeY, theCopy.mySizeX)) {
            return false;
        }
    }

    const size_t aPixelSize = getSizePixelBytes();
    const size_t aSrcColFrom =  theIsClockwise ? theCopy.mySizeX - 1 : 0;
    const size_t aSrcRowFrom = !theIsClockwise ? theCopy.mySizeY - 1 : 0;
    const size_t aSrcColIncr = aSrcColFrom == 0 ? 1 : size_t(-1);
    const size_t aSrcRowIncr = aSrcRowFrom == 0 ? 1 : size_t(-1);
    for(size_t aDstRow = 0, aSrcCol = aSrcColFrom; aDstRow < mySizeY; ++aDstRow, aSrcCol += aSrcColIncr) {
        for(size_t aDstCol = 0, aSrcRow = aSrcRowFrom; aDstCol < mySizeX; ++aDstCol, aSrcRow += aSrcRowIncr) {
            stMemCpy(changeData(aDstRow, aDstCol), theCopy.getData(aSrcRow, aSrcCol), aPixelSize);
        }
    }
    return true;
}

bool StImagePlane::initWrapper(const StImagePlane& theCopy) {
    if(!initWrapper(theCopy.myImgFormat, theCopy.myDataPtr,
                    theCopy.mySizeX, theCopy.mySizeY, theCopy.mySizeRowBytes)) {
        return false;
    }
    myIsTopBottom = theCopy.myIsTopBottom;
    return true;
}

bool StImagePlane::initSideBySide(const StImagePlane& theImageL,
                                  const StImagePlane& theImageR,
                                  const int theSeparationDx,
                                  const int theSeparationDy,
                                  const int theValue) {
    if(theImageL.isNull() || theImageR.isNull()) {
        // just ignore
        return true;
    }
    if(theImageL.getSizeX() != theImageR.getSizeX() ||
       theImageL.getSizeY() != theImageR.getSizeY()) {
        // currently unsupported operation
        return false;
    }
    size_t dxAbsPx = size_t(abs(theSeparationDx));
    size_t dxLeftRPx  = (theSeparationDx > 0) ?     dxAbsPx : 0;
    size_t dxLeftLPx  = (theSeparationDx < 0) ? 2 * dxAbsPx : 0;

    size_t dyAbsPx = size_t(abs(theSeparationDy));
    size_t dyTopLPx  = (theSeparationDy > 0) ? dyAbsPx : 0;
    size_t dyTopRPx  = (theSeparationDy < 0) ? dyAbsPx : 0;

    size_t outSizeX = (theImageL.getSizeX() + dxAbsPx) * 2;
    size_t outSizeY =  theImageL.getSizeY() + dyAbsPx  * 2;

    setFormat(theImageL.getFormat());
    if(!initZero(theImageL.getFormat(), outSizeX, outSizeY, outSizeX * theImageL.getSizePixelBytes(), theValue)) {
        return false;
    }

    // save cross-eyed
    for(size_t row = 0; row < theImageR.getSizeY(); ++row) {
        stMemCpy(changeData(dyTopRPx + row, dxLeftRPx),
                 theImageR.getData(row, 0),
                 theImageR.getSizeRowBytes());
    }
    for(size_t row = 0; row < theImageR.getSizeY(); ++row) {
        stMemCpy(changeData(dyTopLPx + row, theImageR.getSizeX() + dxLeftLPx + dxLeftRPx),
                 theImageL.getData(row, 0),
                 theImageL.getSizeRowBytes());
    }
    return true;
}

bool StImagePlane::fill(const StImagePlane& theCopy,
                        const bool          theIsCompact) {
    if(getSizeY()        != theCopy.getSizeY()
    || getSizeRowBytes() != theCopy.getSizeRowBytes()
    || getFormat()       != theCopy.getFormat()) {
        return initCopy(theCopy, theIsCompact);
    }

    const size_t aCopyRowBytes = stMin(mySizeRowBytes, theCopy.mySizeRowBytes);
    for(size_t row = 0; row < theCopy.getSizeY(); ++row) {
        stMemCpy(changeData(row, 0),
                 theCopy.getData(row, 0),
                 aCopyRowBytes);
    }
    return true;
}

void StImagePlane::nullify(StImagePlane::ImgFormat thePixelFormat) {
    if(myIsOwnPointer && (myDataPtr != NULL)) {
        stMemFreeAligned(myDataPtr);
    }
    myDataPtr = NULL;
    myIsOwnPointer = true;
    mySizeX = mySizeY = mySizeRowBytes = 0;
    setFormat(thePixelFormat);
    myIsTopBottom = true;
}
