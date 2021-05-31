/**
 * Copyright © 2010-2020 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#include <StImage/StImageFile.h>

#include <StImage/StDevILImage.h>
#include <StImage/StFreeImage.h>
#include <StImage/StWebPImage.h>
#include <StImage/StStbImage.h>
#include <StAV/StAVImage.h>
#include <StFile/StFileNode.h>
#include <StFile/StMIME.h>
#include <StFile/StRawFile.h>

#include <StStrings/StLogger.h>

/**
 * DDS Pixel Format structure.
 */
struct StDDSPixelFormat {
    uint32_t Size;
    uint32_t Flags;
    uint32_t FourCC;
    uint32_t RGBBitCount;
    uint32_t RBitMask;
    uint32_t GBitMask;
    uint32_t BBitMask;
    uint32_t ABitMask;
};

/**
 * DDS File header structure.
 */
struct StDDSFileHeader {
    /**
     * Caps2 flag indicating complete (6 faces) cubemap.
     */
    enum { DDSCompleteCubemap = 0xFE00 };

    /**
     * Return TRUE if cubmap flag is set.
     */
    bool isCompleteCubemap() const { return (Caps2 & DDSCompleteCubemap) != 0; }

    uint32_t Size;
    uint32_t Flags;
    uint32_t Height;
    uint32_t Width;
    uint32_t PitchOrLinearSize;
    uint32_t Depth;
    uint32_t MipMapCount;
    uint32_t Reserved1[11];
    StDDSPixelFormat PixelFormatDef;
    uint32_t Caps;
    uint32_t Caps2;
    uint32_t Caps3;
    uint32_t Caps4;
    uint32_t Reserved2;
};

StImageFile::StImageFile()
: mySrcFormat(StFormat_AUTO),
  mySrcPanorama(StPanorama_OFF) {
    //
}

StImageFile::~StImageFile() {
    //
}

StImageFile::ImageClass StImageFile::imgLibFromString(const StString& thePreferred) {
    StImageFile::ImageClass aPreferred = ST_LIBAV;
    if(thePreferred.isEqualsIgnoreCase(stCString("LibAV")) ||
       thePreferred.isEqualsIgnoreCase(stCString("FFmpeg")) ||
       thePreferred.isEqualsIgnoreCase(stCString("StAVImage"))) {
        aPreferred = ST_LIBAV;
    } else if(thePreferred.isEqualsIgnoreCase(stCString("FreeImage")) ||
              thePreferred.isEqualsIgnoreCase(stCString("StFreeImage"))) {
        aPreferred = ST_FREEIMAGE;
    } else if(thePreferred.isEqualsIgnoreCase(stCString("DevIL")) ||
              thePreferred.isEqualsIgnoreCase(stCString("StDevILImage"))) {
        aPreferred = ST_DEVIL;
    } else if(thePreferred.isEqualsIgnoreCase(stCString("WebP")) ||
              thePreferred.isEqualsIgnoreCase(stCString("StWebPImage"))) {
        aPreferred = ST_WEBP;
    } else if(thePreferred.isEqualsIgnoreCase(stCString("stb"))) {
        aPreferred = ST_STB;
    }
    return aPreferred;
}

StString StImageFile::imgLibToString(const ImageClass thePreferred) {
    switch(thePreferred) {
        case ST_FREEIMAGE: return "FreeImage";
        case ST_DEVIL:     return "DevIL";
        case ST_WEBP:      return "WebP";
        case ST_STB:       return "stb";
        default:
        case ST_LIBAV:     return "FFmpeg";
    }
}

StImageFile::ImageType StImageFile::guessImageType(const StString& theFileName,
                                                   const StMIME&   theMIMEType) {
    StString anExt = !theMIMEType.isEmpty() ? theMIMEType.getExtension() : StFileNode::getExtension(theFileName);
    if(anExt.isEqualsIgnoreCase(stCString("mpo"))
    || theMIMEType.getMIMEType().isEquals(stCString("image/mpo"))
    || theMIMEType.getMIMEType().isEquals(stCString("image/x-mpo"))) {
        return StImageFile::ST_TYPE_MPO;
    } else if(anExt.isEqualsIgnoreCase(stCString("jps"))
           || theMIMEType.getMIMEType().isEquals(stCString("image/jps"))
           || theMIMEType.getMIMEType().isEquals(stCString("image/x-jps"))) {
        return StImageFile::ST_TYPE_JPS;
    } else if(anExt.isEqualsIgnoreCase(stCString("pns"))
           || theMIMEType.getMIMEType().isEquals(stCString("image/pns"))
           || theMIMEType.getMIMEType().isEquals(stCString("image/x-pns"))) {
        return StImageFile::ST_TYPE_PNS;
    } else if(anExt.isEqualsIgnoreCase(stCString("jpg"))
           || anExt.isEqualsIgnoreCase(stCString("jpeg"))
           || anExt.isEqualsIgnoreCase(stCString("jpe"))
           || theMIMEType.getMIMEType().isEquals(stCString("image/jpg"))
           || theMIMEType.getMIMEType().isEquals(stCString("image/jpeg"))) {
        return StImageFile::ST_TYPE_JPEG;
    } else if(anExt.isEqualsIgnoreCase(stCString("png"))
           || theMIMEType.getMIMEType().isEquals(stCString("image/png"))) {
        return StImageFile::ST_TYPE_PNG;
    } else if(anExt.isEqualsIgnoreCase(stCString("exr"))) {
        return StImageFile::ST_TYPE_EXR;
    } else if(anExt.isEqualsIgnoreCase(stCString("psd"))) {
        return StImageFile::ST_TYPE_PSD;
    } else if(anExt.isEqualsIgnoreCase(stCString("ico"))) {
        return StImageFile::ST_TYPE_ICO;
    } else if(anExt.isEqualsIgnoreCase(stCString("hdr"))) {
        return StImageFile::ST_TYPE_HDR;
    } else if(anExt.isEqualsIgnoreCase(stCString("webp"))
           || theMIMEType.getMIMEType().isEquals(stCString("image/webp"))) {
        return StImageFile::ST_TYPE_WEBP;
    } else if(anExt.isEqualsIgnoreCase(stCString("webpll"))
           || theMIMEType.getMIMEType().isEquals(stCString("image/webpll"))) {
        return StImageFile::ST_TYPE_WEBPLL;
    } else if(anExt.isEqualsIgnoreCase(stCString("dds"))
           || theMIMEType.getMIMEType().isEquals(stCString("image/vnd-ms.dds"))) {
       return StImageFile::ST_TYPE_DDS;
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
        case ST_STB: {
            if(StStbImage::init()) {
                return new StStbImage();
            }
            break;
        }
        default:
        case ST_LIBAV: {
            if(StAVImage::init()) {
                return new StAVImage();
            }
            break;
        }
    }

    // use default library anyway (that currently always linked)
    if(StAVImage::init()) {
        return new StAVImage();
    }
    return StHandle<StImageFile>();
}

bool StImageFile::load(const StString& theFilePath,
                       ImageType theImageType,
                       uint8_t* theDataPtr, int theDataSize) {
    if(theImageType == ST_TYPE_DDS) {
        // Most image libraries ignore arrays/cubemaps in DDS file.
        // As DDS format is pretty simple - parse it here and load cubemap as vertically stacked image.
        StRawFile aRawFile(theFilePath);
        if(theDataPtr == NULL) {
            if(!aRawFile.readFile()) {
                return loadExtra(theFilePath, theImageType, theDataPtr, theDataSize, false);
            }
            theDataPtr = (uint8_t* )aRawFile.getBuffer();
            theDataSize = (int )aRawFile.getSize();
        }

        if (theDataSize < 128
         || memcmp (&theDataPtr[0], "DDS ", 4) != 0) {
            return loadExtra(theFilePath, theImageType, theDataPtr, theDataSize, false);
        }

        const StDDSFileHeader* aSrcHeader = (const StDDSFileHeader* )&theDataPtr[4];
        if (aSrcHeader->Size != 124
         || aSrcHeader->Width  == 0
         || aSrcHeader->Height == 0
         || aSrcHeader->Width != aSrcHeader->Height
         || aSrcHeader->PixelFormatDef.Size != 32
         || (aSrcHeader->Caps2 & StDDSFileHeader::DDSCompleteCubemap) != StDDSFileHeader::DDSCompleteCubemap) {
            return loadExtra(theFilePath, theImageType, theDataPtr, theDataSize, false);
        }

        const int aHeaderSize  = aSrcHeader->Size + 4;
        const int aPayLoadSize = theDataSize - aHeaderSize;
        if (aPayLoadSize % 6 != 0
         || aPayLoadSize <= 0) {
            return loadExtra(theFilePath, theImageType, theDataPtr, theDataSize, false);
        }

        const int aTileDataSize = aPayLoadSize / 6;
        StArray<uint8_t> aTileBuffer (aTileDataSize + aHeaderSize);
        memcpy(&aTileBuffer.changeFirst(),  "DDS ", 4);
        memcpy(&aTileBuffer.changeValue(4), aSrcHeader, aSrcHeader->Size);
        StDDSFileHeader* aHeaderTile = (StDDSFileHeader* )&aTileBuffer.changeValue(4);
        aHeaderTile->Caps2 &= ~(StDDSFileHeader::DDSCompleteCubemap);

        StHandle<StImageFile> aTileImage = createEmpty();

        memcpy(&aTileBuffer.changeValue(aHeaderSize), &theDataPtr[aHeaderSize], aTileDataSize);
        if(!aTileImage->loadExtra(theFilePath, theImageType, &aTileBuffer.changeFirst(), (int )aTileBuffer.size(), false)
         || aTileImage->isNull()
         || aTileImage->getSizeX() < 1
         || aTileImage->getSizeY() < 1
         || aTileImage->getSizeX() != aTileImage->getSizeY()) {
            return loadExtra(theFilePath, theImageType, theDataPtr, theDataSize, false);
        }

        close();
        nullify();
        myMetadata   = aTileImage->myMetadata;
        myStateDescr = aTileImage->myStateDescr;
        mySrcFormat  = aTileImage->mySrcFormat;
        setColorModel(aTileImage->getColorModel());
        setColorScale(aTileImage->getColorScale());
        setPixelRatio(aTileImage->getPixelRatio());
        const bool isTopDownLayout = aTileImage->isTopDown();
        for(size_t aPlaneId = 0; aPlaneId < 4; ++aPlaneId) {
            const StImagePlane& aFromPlane = aTileImage->getPlane(aPlaneId);
            const size_t aTileIndex = isTopDownLayout ? 0 : 5;
            if(!aFromPlane.isNull()) {
                if(!changePlane(aPlaneId).initTrash(aFromPlane.getFormat(), aFromPlane.getSizeX(), aFromPlane.getSizeX() * 6)) {
                    return false;
                }
                changePlane(aPlaneId).setTopDown(aFromPlane.isTopDown());
                memcpy(changePlane(aPlaneId).changeData() + aFromPlane.getSizeBytes() * aTileIndex,
                       aFromPlane.getData(), aFromPlane.getSizeBytes());
            }
        }

        for(size_t aTileIter = 1; aTileIter < 6; ++aTileIter) {
            memcpy(&aTileBuffer.changeValue(aHeaderSize), &theDataPtr[aTileIter * aTileDataSize + aHeaderSize], aTileDataSize);
            aTileImage = createEmpty();
            if(!aTileImage->loadExtra(theFilePath, theImageType, &aTileBuffer.changeFirst(), (int )aTileBuffer.size(), false)
             || aTileImage->isNull()
             || aTileImage->getSizeX() != getSizeX()
             || aTileImage->getSizeY() != getSizeX() // square
             || aTileImage->getColorModel() != getColorModel()
             || aTileImage->getPlane().getFormat() != getPlane().getFormat()) {
                ST_ERROR_LOG("Internal error: DDS file is decoded into inconsistent tiles");
                return loadExtra(theFilePath, theImageType, theDataPtr, theDataSize, false);
            }

            const size_t aTileIndex = isTopDownLayout ? aTileIter : (5 - aTileIter);
            for(size_t aPlaneId = 0; aPlaneId < 4; ++aPlaneId) {
                const StImagePlane& aFromPlane = aTileImage->getPlane(aPlaneId);
                if(!aFromPlane.isNull()) {
                    memcpy(changePlane(aPlaneId).changeData() + aFromPlane.getSizeBytes() * aTileIndex,
                           aFromPlane.getData(), aFromPlane.getSizeBytes());
                }
            }
        }
        mySrcPanorama = StPanorama_Cubemap1_6;
        return true;
    }
    return loadExtra(theFilePath, theImageType, theDataPtr, theDataSize, false);
}

StImageFileCounter::~StImageFileCounter() {}

void StImageFileCounter::createReference(StHandle<StBufferCounter>& theOther) const {
    StHandle<StImageFileCounter> anImgFileRef = StHandle<StImageFileCounter>::downcast(theOther);
    if(anImgFileRef.isNull()) {
        anImgFileRef = new StImageFileCounter();
        theOther = anImgFileRef;
    }
    anImgFileRef->myImageFile = myImageFile;
}

void StImageFileCounter::releaseReference() {
    myImageFile.nullify();
}
