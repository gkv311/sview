/**
 * Copyright © 2010-2015 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#include <StImage/StImage.h>

StString StImage::formatImgColorModel(ImgColorModel theColorModel) {
#ifdef ST_DEBUG
    switch(theColorModel) {
        case ImgColor_RGB:     return "ImgColor_RGB";
        case ImgColor_RGBA:    return "ImgColor_RGBA";
        case ImgColor_GRAY:    return "ImgColor_GRAY";
        case ImgColor_YUV:     return "ImgColor_YUV";
        case ImgColor_CMYK:    return "ImgColor_CMYK";
        case ImgColor_HSV:     return "ImgColor_HSV";
        case ImgColor_HSL:     return "ImgColor_HSL";
        default:               return "ImgColor_UNKNOWN";
    }
#else
    switch(theColorModel) {
        case ImgColor_RGB:     return "RGB";
        case ImgColor_RGBA:    return "RGBA";
        case ImgColor_GRAY:    return "Grayscale";
        case ImgColor_YUV:     return "YUV";
        case ImgColor_CMYK:    return "CMYK";
        case ImgColor_HSV:     return "HSV";
        case ImgColor_HSL:     return "HSL";
        default:               return StString("UNKNOWN[") + theColorModel + "]";
    }
#endif
}

StImage::StImage()
: myPAR(1.0f),
  myColorModel(ImgColor_RGB),
  myColorScale(ImgScale_Full) {
    //
}

StImage::~StImage() {
    //
}

bool StImage::initCopy(const StImage& theCopy) {
    nullify();
    setColorModel(theCopy.getColorModel());
    setColorScale(theCopy.getColorScale());
    for(size_t aPlaneId = 0; aPlaneId < 4; ++aPlaneId) {
        const StImagePlane& aFromPlane = theCopy.getPlane(aPlaneId);
        if(aFromPlane.isNull()) {
            continue;
        }
        if(!changePlane(aPlaneId).initCopy(aFromPlane)) {
            return false;
        }
    }
    return true;
}

bool StImage::initWrapper(const StImage& theCopy) {
    nullify();
    setColorModel(theCopy.getColorModel());
    setColorScale(theCopy.getColorScale());
    for(size_t aPlaneId = 0; aPlaneId < 4; ++aPlaneId) {
        const StImagePlane& aFromPlane = theCopy.getPlane(aPlaneId);
        if(aFromPlane.isNull()) {
            continue;
        }
        if(!changePlane(aPlaneId).initWrapper(aFromPlane)) {
            return false;
        }
    }
    return true;
}

bool StImage::initReference(const StImage& theCopy) {
    nullify();
    if( theCopy.myBufCounter.isNull()
    || !initWrapper(theCopy)) {
        return false;
    }

    theCopy.myBufCounter->createReference(myBufCounter);
    return true;
}

bool StImage::initReference(const StImage&                   theCopy,
                            const StHandle<StBufferCounter>& theRef) {
    nullify();
    if( theRef.isNull()
    || !initWrapper(theCopy)) {
        return false;
    }

    theRef->createReference(myBufCounter);
    return true;
}

bool StImage::initTrashLimited(const StImage& theRef,
                               const size_t   theSizeX,
                               const size_t   theSizeY) {
    nullify();
    setColorModel(theRef.getColorModel());
    setColorScale(theRef.getColorScale());
    if(theRef.isNull()
    || theRef.getSizeX() < 1
    || theRef.getSizeY() < 1
    || theSizeX < 1
    || theSizeY < 1) {
        return false;
    }

    double aRatioX = double(theSizeX) / double(theRef.getSizeX());
    double aRatioY = double(theSizeY) / double(theRef.getSizeY());
    myPAR = float(double(theRef.getPixelRatio()) * aRatioY / aRatioX);

    for(size_t aPlaneId = 0; aPlaneId < 4; ++aPlaneId) {
        const StImagePlane& aFromPlane = theRef.getPlane(aPlaneId);
        if(aFromPlane.isNull()) {
            continue;
        }

        const size_t aScaleX = theRef.getSizeX() / aFromPlane.getSizeX();
        const size_t aScaleY = theRef.getSizeY() / aFromPlane.getSizeY();
        const size_t aSizeX  = theSizeX / aScaleX;
        const size_t aSizeY  = theSizeY / aScaleY;
        if(!changePlane(aPlaneId).initTrash(aFromPlane.getFormat(), aSizeX, aSizeY)) {
            return false;
        }
    }
    return true;
}

StPixelRGB StImage::getRGBFromYUV(const size_t theRow, const size_t theCol) const {
    // this code is not acceptable for full-range YUV!
    int OY = 298 * (getPlane(0).getFirstByte(theRow, theCol) - 16);
    int U = getPlane(1).getFirstByte(getScaledRow(1, theRow),
                                     getScaledCol(1, theCol)) - 128;
    int V = getPlane(2).getFirstByte(getScaledRow(2, theRow),
                                     getScaledCol(2, theCol)) - 128;
    return StPixelRGB(clamp<int>((OY + 409 * V + 128) >> 8),
                      clamp<int>((OY - 100 * U - 208 * V + 128) >> 8),
                      clamp<int>((OY + 516 * U + 128) >> 8));
}

bool StImage::initRGB(const StImage& theCopy) {
    if(this == &theCopy) {
        // not supported operation
        return false;
    }
    nullify();
    if(theCopy.isNull()) {
        return false;
    }
    switch(theCopy.getColorModel()) {
        case StImage::ImgColor_RGB:
        case StImage::ImgColor_RGBA: {
            return initWrapper(theCopy);
        }
        case StImage::ImgColor_YUV: {
            if(isPacked()
            || theCopy.getColorScale() != StImage::ImgScale_Mpeg
            || theCopy.getPlane(0).getFormat() != StImagePlane::ImgGray16) {
                // not supported
                return false;
            }
            // this is SLOW conversion!
            StImagePlane& anRGBPlane = changePlane(0);
            anRGBPlane.initTrash(StImagePlane::ImgRGB,
                                 theCopy.getSizeX(), theCopy.getSizeY());
            for(size_t aRow = 0; aRow < anRGBPlane.getSizeY(); ++aRow) {
                for(size_t aCol = 0; aCol < anRGBPlane.getSizeX(); ++aCol) {
                    anRGBPlane.changePixelRGB(aRow, aCol) = theCopy.getRGBFromYUV(aRow, aCol);
                }
            }
            return true;
        }
        case ImgColor_GRAY:
        case ImgColor_CMYK:
        case ImgColor_HSV:
        case ImgColor_HSL:
        default: return false;
    }
}

bool StImage::initSideBySide(const StImage& theImageL,
                             const StImage& theImageR,
                             const int theSeparationDx,
                             const int theSeparationDy) {
    const bool isYUV = theImageL.getColorModel() == StImage::ImgColor_YUV;
    for(size_t aPlaneId = 0; aPlaneId < 4; ++aPlaneId) {
        float aScaleX = (theImageL.getPlane(aPlaneId).getSizeX() > 0) ? theImageL.getScaleFactorX(aPlaneId) : 1.0f;
        float aScaleY = (theImageL.getPlane(aPlaneId).getSizeY() > 0) ? theImageL.getScaleFactorY(aPlaneId) : 1.0f;

        // setup black color per plane
        int aValue = (isYUV && aPlaneId != 0) ? 128 : 0;
        if(!changePlane(aPlaneId).initSideBySide(theImageL.getPlane(aPlaneId),
                                                 theImageR.getPlane(aPlaneId),
                                                 int(aScaleX * theSeparationDx),
                                                 int(aScaleY * theSeparationDy),
                                                 aValue)) {
            nullify();
            return false;
        }
    }
    setColorModel(theImageL.getColorModel());
    setColorScale(theImageL.getColorScale());
    return true;
}

bool StImage::fill(const StImage& theCopy) {
    if(getColorModel() != theCopy.getColorModel()) {
        return initCopy(theCopy);
    }
    for(size_t aPlaneId = 0; aPlaneId < 4; ++aPlaneId) {
        if(!changePlane(aPlaneId).fill(theCopy.getPlane(aPlaneId))) {
            nullify();
            return false;
        }
    }
    return true;
}

bool StImage::isNull() const {
    // first plane should not be NULL
    return myPlanes[0].isNull();
}

void StImage::nullify() {
    myPlanes[0].nullify();
    myPlanes[1].nullify();
    myPlanes[2].nullify();
    myPlanes[3].nullify();
    if(!myBufCounter.isNull()) {
        myBufCounter->releaseReference();
    }
    myPAR = 1.0f;
}
