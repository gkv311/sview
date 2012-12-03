/**
 * Copyright © 2010-2011 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#include <StImage/StImageFile.h>
#include <StImage/StDevILImage.h>
#include <StImage/StLibAVImage.h>
#include <StImage/StFreeImage.h>
#include <StImage/StWebPImage.h>

#include <StFile/StFileNode.h>
#include <StFile/StMIME.h>

StImageFile::ImageClass StImageFile::imgLibFromString(const StString& thePreferred) {
    StImageFile::ImageClass aPreferred = ST_LIBAV;
    if(thePreferred.isEqualsIgnoreCase("LibAV") ||
       thePreferred.isEqualsIgnoreCase("FFmpeg") ||
       thePreferred.isEqualsIgnoreCase("StLibAVImage")) {
        aPreferred = ST_LIBAV;
    } else if(thePreferred.isEqualsIgnoreCase("FreeImage") ||
              thePreferred.isEqualsIgnoreCase("StFreeImage")) {
        aPreferred = ST_FREEIMAGE;
    } else if(thePreferred.isEqualsIgnoreCase("DevIL") ||
              thePreferred.isEqualsIgnoreCase("StDevILImage")) {
        aPreferred = ST_DEVIL;
    } else if(thePreferred.isEqualsIgnoreCase("WebP") ||
              thePreferred.isEqualsIgnoreCase("StWebPImage")) {
        aPreferred = ST_WEBP;
    }
    return aPreferred;
}

StString StImageFile::imgLibToString(const ImageClass thePreferred) {
    switch(thePreferred) {
        case ST_FREEIMAGE: return "FreeImage";
        case ST_DEVIL:     return "DevIL";
        case ST_WEBP:      return "WebP";
        default:
        case ST_LIBAV:     return "FFmpeg";
    }
}

StImageFile::ImageType StImageFile::guessImageType(const StString& theFileName,
                                                   const StMIME&   theMIMEType) {
    StString anExt = !theMIMEType.isEmpty() ? theMIMEType.getExtension() : StFileNode::getExtension(theFileName);
    if(anExt.isEqualsIgnoreCase("mpo")
    || theMIMEType.getMIMEType() == StString("image/mpo")
    || theMIMEType.getMIMEType() == StString("image/x-mpo")) {
        return StImageFile::ST_TYPE_MPO;
    } else if(anExt.isEqualsIgnoreCase("jps")
           || theMIMEType.getMIMEType() == StString("image/jps")
           || theMIMEType.getMIMEType() == StString("image/x-jps")) {
        return StImageFile::ST_TYPE_JPS;
    } else if(anExt.isEqualsIgnoreCase("pns")
           || theMIMEType.getMIMEType() == StString("image/pns")
           || theMIMEType.getMIMEType() == StString("image/x-pns")) {
        return StImageFile::ST_TYPE_PNS;
    } else if(anExt.isEqualsIgnoreCase("jpg")
           || anExt.isEqualsIgnoreCase("jpeg")
           || anExt.isEqualsIgnoreCase("jpe")
           || theMIMEType.getMIMEType() == StString("image/jpg")
           || theMIMEType.getMIMEType() == StString("image/jpeg")) {
        return StImageFile::ST_TYPE_JPEG;
    } else if(anExt.isEqualsIgnoreCase("exr")) {
        return StImageFile::ST_TYPE_EXR;
    } else if(anExt.isEqualsIgnoreCase("psd")) {
        return StImageFile::ST_TYPE_PSD;
    } else if(anExt.isEqualsIgnoreCase("ico")) {
        return StImageFile::ST_TYPE_ICO;
    } else if(anExt.isEqualsIgnoreCase("hdr")) {
        return StImageFile::ST_TYPE_HDR;
    } else if(anExt.isEqualsIgnoreCase("webp")
           || theMIMEType.getMIMEType() == StString("image/webp")) {
        return StImageFile::ST_TYPE_WEBP;
    } else if(anExt.isEqualsIgnoreCase("webpll")
           || theMIMEType.getMIMEType() == StString("image/webpll")) {
        return StImageFile::ST_TYPE_WEBPLL;
    }
    return StImageFile::ST_TYPE_NONE;
}

StHandle<StImageFile> StImageFile::create(const StString& thePreferred,
                                          ImageType       theImgType) {
    return StImageFile::create(imgLibFromString(thePreferred), theImgType);
}

StHandle<StImageFile> StImageFile::create(StImageFile::ImageClass thePreferred,
                                          ImageType               theImgType) {
    // firstly parse image type - exceptions
    // that supported not by all libraries
    switch(theImgType) {
        case ST_TYPE_EXR: {
            // only FreeImage currently supports OpenEXR images
            if(StFreeImage::init()) {
                return new StFreeImage();
            }
            break;
        }
        case ST_TYPE_PSD: {
            // only DevIL currently supports PSD images
            if(StDevILImage::init()) {
                return new StDevILImage();
            }
            break;
        }
        case ST_TYPE_WEBP:
        case ST_TYPE_WEBPLL: {
            // only WebP currently supports WebP images
            if(StWebPImage::init()) {
                return new StWebPImage();
            }
            break;
        }
        case ST_TYPE_ICO:
        case ST_TYPE_HDR: {
            // FFmpeg doesn't supports ICO and HDR
            // DevIL supports them best (FreeImage has problems)
            if(StDevILImage::init()) {
                return new StDevILImage();
            } else if(StFreeImage::init()) {
                return new StFreeImage();
            }
            break;
        }
        default:
            break;
    }

    // open requested library if it exists
    switch(thePreferred) {
        case ST_FREEIMAGE: {
            if(StFreeImage::init()) {
                return new StFreeImage();
            }
            break;
        }
        case ST_DEVIL: {
            if(StDevILImage::init()) {
                return new StDevILImage();
            }
            break;
        }
        case ST_WEBP: {
            if(StWebPImage::init()) {
                return new StWebPImage();
            }
            break;
        }
        default:
        case ST_LIBAV: {
            if(StLibAVImage::init()) {
                return new StLibAVImage();
            }
            break;
        }
    }

    // use default library anyway (that currently always linked)
    if(StLibAVImage::init()) {
        return new StLibAVImage();
    }
    return StHandle<StImageFile>();
}
