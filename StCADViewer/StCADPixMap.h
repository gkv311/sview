/**
 * This source is a part of sView program.
 *
 * Copyright © Kirill Gavrilov, 2016
 */

#ifndef __StCADPixMap_h_
#define __StCADPixMap_h_

#include <Image_PixMap.hxx>

#include <StImage/StImageFile.h>

/**
 * StImage wrapper.
 */
class StCADPixMap : public Image_PixMap {

        public:

    /**
     * Empty constructor.
     */
    ST_LOCAL StCADPixMap() {}

    /**
     * Destructor
     */
    ST_LOCAL virtual ~StCADPixMap() {}

    /**
     * Clear the image.
     */
    ST_LOCAL virtual void Clear() Standard_OVERRIDE {
        Image_PixMap::Clear();
        myStImage.nullify();
    }

    /**
     * Load image from file.
     */
    ST_LOCAL bool Load(const TCollection_AsciiString& theFileName) {
        Clear();
        StHandle<StAVImage> anImg = new StAVImage();
        if(!anImg->loadExtra(theFileName.ToCString(), StImageFile::ST_TYPE_NONE, NULL, 0, true)) {
            Clear();
            return false;
        }

        Image_PixMap::ImgFormat anImgFormat = Image_PixMap::ImgRGBA;
        switch(anImg->getPlane().getFormat()) {
            case StImagePlane::ImgUNKNOWN: return false;
            case StImagePlane::ImgGray:    anImgFormat = ImgGray; break;
            case StImagePlane::ImgGray16:  return false;
            case StImagePlane::ImgRGB:     anImgFormat = ImgRGB; break;
            case StImagePlane::ImgBGR:     anImgFormat = ImgBGR; break;
            case StImagePlane::ImgRGB32:   anImgFormat = ImgRGB32; break;
            case StImagePlane::ImgBGR32:   anImgFormat = ImgBGR32; break;
            case StImagePlane::ImgRGB48:   return false;
            case StImagePlane::ImgRGBA:    anImgFormat = ImgRGBA; break;
            case StImagePlane::ImgBGRA:    anImgFormat = ImgBGRA; break;
            case StImagePlane::ImgRGBA64:  return false;
            case StImagePlane::ImgGrayF:   anImgFormat = ImgGrayF; break;
            case StImagePlane::ImgRGBF:    anImgFormat = ImgRGBF; break;
            case StImagePlane::ImgBGRF:    anImgFormat = ImgBGRF; break;
            case StImagePlane::ImgRGBAF:   anImgFormat = ImgRGBAF; break;
            case StImagePlane::ImgBGRAF:   anImgFormat = ImgBGRAF; break;
        }

        ///SetTopDown (anImg->getPlane().isTopDown());
        Image_PixMap::InitWrapper (anImgFormat,
                                   anImg->changePlane().changeData(),
                                   anImg->changePlane().getSizeX(),
                                   anImg->changePlane().getSizeY(),
                                   anImg->changePlane().getSizeRowBytes());
        myStImage = anImg;
        return true;
    }

        private:

    StHandle<StImageFile> myStImage;

        public:

    DEFINE_STANDARD_RTTI_INLINE(StCADPixMap, Image_PixMap)

};

DEFINE_STANDARD_HANDLE(StCADPixMap, Image_PixMap)

#endif // __StCADPixMap_h_
