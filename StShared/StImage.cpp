/**
 * Copyright © 2010-2019 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#include <StImage/StImage.h>

StString StImage::formatImgColorModel(ImgColorModel theColorModel) {
#ifdef ST_DEBUG
    switch(theColorModel) {
        case ImgColor_RGB:     return "ImgColor_RGB";
        case ImgColor_RGBA:    return "ImgColor_RGBA";
        case ImgColor_GRAY:    return "ImgColor_GRAY";
        case ImgColor_XYZ:     return "ImgColor_XYZ";
        case ImgColor_YUV:     return "ImgColor_YUV";
        case ImgColor_YUVA:    return "ImgColor_YUVA";
        case ImgColor_CMYK:    return "ImgColor_CMYK";
        case ImgColor_HSV:     return "ImgColor_HSV";
        case ImgColor_HSL:     return "ImgColor_HSL";
    }
    return "ImgColor_UNKNOWN";
#else
    switch(theColorModel) {
        case ImgColor_RGB:     return "RGB";
        case ImgColor_RGBA:    return "RGBA";
        case ImgColor_GRAY:    return "Grayscale";
        case ImgColor_XYZ:     return "XYZ";
        case ImgColor_YUV:     return "YUV";
        case ImgColor_YUVA:    return "YUVA";
        case ImgColor_CMYK:    return "CMYK";
        case ImgColor_HSV:     return "HSV";
        case ImgColor_HSL:     return "HSL";
    }
    return StString("UNKNOWN[") + theColorModel + "]";
#endif
}

const char* StImage::formatImgPixelFormat() const {
    switch(myColorModel) {
        case ImgColor_RGB:
        case ImgColor_RGBA:
        case ImgColor_GRAY: {
            switch(myPlanes[0].getFormat()) {
                case StImagePlane::ImgGray:    return "gray8";
                case StImagePlane::ImgGray16:  return "gray16";
                case StImagePlane::ImgRGB:     return "rgb24";
                case StImagePlane::ImgBGR:     return "bgr24";
                case StImagePlane::ImgRGB32:   return "rgb32";
                case StImagePlane::ImgBGR32:   return "bgr32";
                case StImagePlane::ImgRGB48:   return "rgb48";
                case StImagePlane::ImgRGBA:    return "rgba32";
                case StImagePlane::ImgBGRA:    return "bgra32";
                case StImagePlane::ImgRGBA64:  return "rgba64";
                case StImagePlane::ImgGrayF:   return "grayf";
                case StImagePlane::ImgRGBF:    return "rgbf";
                case StImagePlane::ImgBGRF:    return "bgrf";
                case StImagePlane::ImgRGBAF:   return "rgbaf";
                case StImagePlane::ImgBGRAF:   return "bgraf";
                case StImagePlane::ImgUV:      return "uv";
                case StImagePlane::ImgUNKNOWN: return "unknown";
            }
            return "invalid_rgb";
        }
        case ImgColor_XYZ: {
            switch(myPlanes[0].getFormat()) {
                case StImagePlane::ImgRGB:     return "xyz8";
                case StImagePlane::ImgRGB48:   return "xyz12";
                case StImagePlane::ImgRGBF:    return "xyzf";
                default: break;
            }
            return "invalid_xyz";
        }
        case ImgColor_YUV:
        case ImgColor_YUVA: {
            const bool  hasAlpha = myColorModel == ImgColor_YUVA;
            const size_t aDelimX = (myPlanes[1].getSizeX() > 0) ? (myPlanes[0].getSizeX() / myPlanes[1].getSizeX()) : 1;
            const size_t aDelimY = (myPlanes[1].getSizeY() > 0) ? (myPlanes[0].getSizeY() / myPlanes[1].getSizeY()) : 1;
            if(myPlanes[1].getFormat() == StImagePlane::ImgUV) {
                return "nv12";
            } else if(aDelimX == 1 && aDelimY == 1) {
                switch(myColorScale) {
                    case StImage::ImgScale_Mpeg:
                        return myPlanes[0].getFormat() == StImagePlane::ImgGray16
                             ? (hasAlpha ? "yuva444p16" : "yuv444p16")
                             : (hasAlpha ? "yuva444p"   : "yuv444p");
                    case StImage::ImgScale_Mpeg9:
                    case StImage::ImgScale_Jpeg9:
                        return (hasAlpha ? "yuva444p9" : "yuv444p9");
                    case StImage::ImgScale_Mpeg10:
                    case StImage::ImgScale_Jpeg10:
                        return (hasAlpha ? "yuva444p10" : "yuv444p10");
                    case StImage::ImgScale_Full:
                    default:
                        return myPlanes[0].getFormat() == StImagePlane::ImgGray16
                             ? (hasAlpha ? "yuvaj444p16" : "yuvj444p16")
                             : (hasAlpha ? "yuvaj444p"   : "yuvj444p");
                }
            } else if(aDelimX == 2 && aDelimY == 2) {
                switch(myColorScale) {
                    case StImage::ImgScale_Mpeg:
                        return myPlanes[0].getFormat() == StImagePlane::ImgGray16
                             ? (hasAlpha ? "yuva420p16" : "yuv420p16")
                             : (hasAlpha ? "yuva420p"   : "yuv420p");
                    case StImage::ImgScale_Mpeg9:
                    case StImage::ImgScale_Jpeg9:
                        return (hasAlpha ? "yuva420p9" : "yuv420p9");
                    case StImage::ImgScale_Mpeg10:
                    case StImage::ImgScale_Jpeg10:
                        return (hasAlpha ? "yuva420p10" : "yuv420p10");
                    case StImage::ImgScale_Full:
                    default:
                        return myPlanes[0].getFormat() == StImagePlane::ImgGray16
                             ? (hasAlpha ? "yuvaj420p16" : "yuvj420p16")
                             : (hasAlpha ? "yuvaj420p"   : "yuvj420p");
                }
            } else if(aDelimX == 2 && aDelimY == 1) {
                switch(myColorScale) {
                    case StImage::ImgScale_Mpeg:
                        return myPlanes[0].getFormat() == StImagePlane::ImgGray16
                             ? (hasAlpha ? "yuva422p16" : "yuv422p16")
                             : (hasAlpha ? "yuva422p"   : "yuv422p");
                    case StImage::ImgScale_Mpeg9:
                    case StImage::ImgScale_Jpeg9:
                        return (hasAlpha ? "yuva422p9" : "yuv422p9");
                    case StImage::ImgScale_Mpeg10:
                    case StImage::ImgScale_Jpeg10:
                        return (hasAlpha ? "yuva422p10" : "yuv422p10");
                    case StImage::ImgScale_Full:
                    default:
                        return myPlanes[0].getFormat() == StImagePlane::ImgGray16
                             ? (hasAlpha ? "yuvaj422p16" : "yuvj422p16")
                             : (hasAlpha ? "yuvaj422p"   : "yuvj422p");
                }
            } else if(aDelimX == 1 && aDelimY == 2) {
                return myColorScale == StImage::ImgScale_Mpeg
                     ? (hasAlpha ? "yuva440p"  : "yuv440p")
                     : (hasAlpha ? "yuvaj440p" : "yuvj440p");
            } else if(aDelimX == 4 && aDelimY == 1) {
                return myColorScale == StImage::ImgScale_Mpeg
                     ? (hasAlpha ? "yuva411p"  : "yuv411p")
                     : (hasAlpha ? "yuvaj411p" : "yuvj411p");
            } else if(aDelimX == 4 && aDelimY == 4) {
                return myColorScale == StImage::ImgScale_Mpeg
                     ? (hasAlpha ? "yuva410p"  : "yuv410p")
                     : (hasAlpha ? "yuvaj410p" : "yuvj410p");
            }
            return (hasAlpha ? "yuva_unknown" : "yuv_unknown");
        }
        case ImgColor_CMYK:
            return "CMYK";
        case ImgColor_HSV:
            return "HSV";
        case ImgColor_HSL:
            return "HSL";
    }
    return "unknown";
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

bool StImage::initCopy(const StImage& theCopy,
                       const bool     theIsCompact) {
    nullify();
    setColorModel(theCopy.getColorModel());
    setColorScale(theCopy.getColorScale());
    myPAR = theCopy.myPAR;
    for(size_t aPlaneId = 0; aPlaneId < 4; ++aPlaneId) {
        const StImagePlane& aFromPlane = theCopy.getPlane(aPlaneId);
        if(aFromPlane.isNull()) {
            continue;
        }
        if(!changePlane(aPlaneId).initCopy(aFromPlane, theIsCompact)) {
            return false;
        }
    }
    return true;
}

bool StImage::initWrapper(const StImage& theCopy) {
    nullify();
    setColorModel(theCopy.getColorModel());
    setColorScale(theCopy.getColorScale());
    myPAR = theCopy.myPAR;
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
    const bool isYUV = theImageL.getColorModel() == StImage::ImgColor_YUV
                    || theImageL.getColorModel() == StImage::ImgColor_YUVA;
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

bool StImage::fill(const StImage& theCopy,
                   const bool     theIsCompact) {
    if(getColorModel() != theCopy.getColorModel()) {
        return initCopy(theCopy, theIsCompact);
    }
    for(size_t aPlaneId = 0; aPlaneId < 4; ++aPlaneId) {
        if(!changePlane(aPlaneId).fill(theCopy.getPlane(aPlaneId), theIsCompact)) {
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
