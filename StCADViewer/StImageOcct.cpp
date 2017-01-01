/**
 * This source is a part of sView program.
 *
 * Copyright © Kirill Gavrilov, 2016
 */

#include "StImageOcct.h"

StImageOcct::StImageOcct() {
    //
}

StImageOcct::~StImageOcct() {
    //
}

void StImageOcct::Clear() {
    Image_PixMap::Clear();
    myStImage.nullify();
}

bool StImageOcct::Load(const StString& theFileName,
                       const StMIME& theMime,
                       uint8_t* theDataPtr, int theDataSize) {
    Clear();

    const StImageFile::ImageType anImgType = StImageFile::guessImageType(theFileName, theMime);
    StHandle<StImageFile> anStImage = StImageFile::create(StImageFile::ST_LIBAV, anImgType);
    if(anStImage.isNull()) {
        return false;
    }

    if(!anStImage->loadExtra(theFileName, anImgType, theDataPtr, theDataSize, true)) {
        return false;
    }

    SetTopDown(true);
    Image_PixMap::ImgFormat aFormat = Image_PixMap::ImgUNKNOWN;
    switch(anStImage->getPlane().getFormat()) {
        case StImagePlane::ImgBGR:   aFormat = Image_PixMap::ImgBGR;   break;
        case StImagePlane::ImgBGR32: aFormat = Image_PixMap::ImgBGR32; break;
        case StImagePlane::ImgRGB:   aFormat = Image_PixMap::ImgRGB;   break;
        case StImagePlane::ImgRGB32: aFormat = Image_PixMap::ImgRGB32; break;
        case StImagePlane::ImgBGRA:  aFormat = Image_PixMap::ImgBGRA;  break;
        case StImagePlane::ImgRGBA:  aFormat = Image_PixMap::ImgRGBA;  break;
        case StImagePlane::ImgBGRF:  aFormat = Image_PixMap::ImgBGRF;  break;
        case StImagePlane::ImgRGBF:  aFormat = Image_PixMap::ImgRGBF;  break;
        case StImagePlane::ImgBGRAF: aFormat = Image_PixMap::ImgBGRAF; break;
        case StImagePlane::ImgRGBAF: aFormat = Image_PixMap::ImgRGBAF; break;
        case StImagePlane::ImgGray:  aFormat = Image_PixMap::ImgGray;  break;
        case StImagePlane::ImgGrayF: aFormat = Image_PixMap::ImgGrayF; break;
        default: {
            Clear();
            return false;
        }
    }

    if(!Image_PixMap::InitWrapper(aFormat, anStImage->changePlane().changeData(), anStImage->getSizeX(), anStImage->getSizeY(), anStImage->getPlane().getSizeRowBytes())) {
        return false;
    }

    myStImage = anStImage;
    return true;
}
