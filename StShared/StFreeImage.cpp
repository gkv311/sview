/**
 * Copyright © 2011-2013 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#include <StImage/StFreeImage.h>

#include <StLibrary.h>
#include <StStrings/StLogger.h>
#include <StThreads/StMutex.h>
#include <StFile/StFileNode.h>

#if defined(__APPLE__)
    #include <StThreads/StProcess.h>
#endif

#if defined(_WIN32)
    #define FIAPIENTRY __stdcall
    typedef stUtfWide_t fiUtfChar_t;
#else
    #define FIAPIENTRY
    typedef stUtf8_t fiUtfChar_t;
#endif

// I/O image format identifiers.
enum FREE_IMAGE_FORMAT {
    FIF_UNKNOWN    = -1,
    FIF_BMP        = 0,
    FIF_ICO        = 1,
    FIF_JPEG       = 2,
    FIF_JNG        = 3,
    FIF_KOALA      = 4,
    FIF_LBM        = 5,
    FIF_IFF = FIF_LBM,
    FIF_MNG        = 6,
    FIF_PBM        = 7,
    FIF_PBMRAW     = 8,
    FIF_PCD        = 9,
    FIF_PCX        = 10,
    FIF_PGM        = 11,
    FIF_PGMRAW     = 12,
    FIF_PNG        = 13,
    FIF_PPM        = 14,
    FIF_PPMRAW     = 15,
    FIF_RAS        = 16,
    FIF_TARGA      = 17,
    FIF_TIFF       = 18,
    FIF_WBMP       = 19,
    FIF_PSD        = 20,
    FIF_CUT        = 21,
    FIF_XBM        = 22,
    FIF_XPM        = 23,
    FIF_DDS        = 24,
    FIF_GIF        = 25,
    FIF_HDR        = 26,
    FIF_FAXG3      = 27,
    FIF_SGI        = 28,
    FIF_EXR        = 29,
    FIF_J2K        = 30,
    FIF_JP2        = 31
};

// Image type used in FreeImage.
enum FREE_IMAGE_TYPE {
    FIT_UNKNOWN    = 0,  // unknown type
    FIT_BITMAP     = 1,  // standard image            : 1-, 4-, 8-, 16-, 24-, 32-bit
    FIT_UINT16     = 2,  // array of unsigned short    : unsigned 16-bit
    FIT_INT16      = 3,  // array of short            : signed 16-bit
    FIT_UINT32     = 4,  // array of unsigned long    : unsigned 32-bit
    FIT_INT32      = 5,  // array of long            : signed 32-bit
    FIT_FLOAT      = 6,  // array of float            : 32-bit IEEE floating point
    FIT_DOUBLE     = 7,  // array of double            : 64-bit IEEE floating point
    FIT_COMPLEX    = 8,  // array of FICOMPLEX        : 2 x 64-bit IEEE floating point
    FIT_RGB16      = 9,  // 48-bit RGB image            : 3 x 16-bit
    FIT_RGBA16     = 10, // 64-bit RGBA image        : 4 x 16-bit
    FIT_RGBF       = 11, // 96-bit RGB float image    : 3 x 32-bit IEEE floating point
    FIT_RGBAF      = 12  // 128-bit RGBA float image    : 4 x 32-bit IEEE floating point
};

// Image color type used in FreeImage.
enum FREE_IMAGE_COLOR_TYPE {
    FIC_MINISWHITE = 0,  // min value is white
    FIC_MINISBLACK = 1,  // min value is black
    FIC_RGB        = 2,  // RGB color model
    FIC_PALETTE    = 3,  // color map indexed
    FIC_RGBALPHA   = 4,  // RGB color model with alpha channel
    FIC_CMYK       = 5   // CMYK color model
};

typedef int32_t FIBOOL;
typedef struct FIMEMORY FIMEMORY;

// FreeImage library
typedef const char* (FIAPIENTRY *FreeImage_GetVersion_t)(void);
typedef FREE_IMAGE_FORMAT (FIAPIENTRY *FreeImage_GetFileType_t)(const fiUtfChar_t* filename, int size);
typedef FREE_IMAGE_FORMAT (FIAPIENTRY *FreeImage_GetFIFFromFilename_t)(const fiUtfChar_t* filename);
typedef FIBITMAP*   (FIAPIENTRY *FreeImage_Load_t)(FREE_IMAGE_FORMAT fif, const fiUtfChar_t* filename, int flags);
typedef FIBOOL      (FIAPIENTRY *FreeImage_Save_t)(FREE_IMAGE_FORMAT fif, FIBITMAP* dib, const fiUtfChar_t* filename, int flags);
typedef void        (FIAPIENTRY *FreeImage_Unload_t)(FIBITMAP* dib);
typedef FIBOOL      (FIAPIENTRY *FreeImage_FIFSupportsReading_t)(FREE_IMAGE_FORMAT fif);
typedef FIBITMAP*   (FIAPIENTRY *FreeImage_AllocateT_t)(FREE_IMAGE_TYPE type, int width, int height, int bpp,
                                                        unsigned red_mask, unsigned green_mask, unsigned blue_mask);
// DIB info routines
typedef uint8_t* (FIAPIENTRY *FreeImage_GetBits_t)  (FIBITMAP* dib);
typedef unsigned (FIAPIENTRY *FreeImage_GetBPP_t)   (FIBITMAP* dib);
typedef unsigned (FIAPIENTRY *FreeImage_GetWidth_t) (FIBITMAP* dib);
typedef unsigned (FIAPIENTRY *FreeImage_GetHeight_t)(FIBITMAP* dib);
typedef unsigned (FIAPIENTRY *FreeImage_GetPitch_t) (FIBITMAP* dib);
typedef FREE_IMAGE_TYPE       (FIAPIENTRY *FreeImage_GetImageType_t)(FIBITMAP* dib);
typedef FREE_IMAGE_COLOR_TYPE (FIAPIENTRY *FreeImage_GetColorType_t)(FIBITMAP* dib);
// memory I/O
typedef FIMEMORY* (FIAPIENTRY *FreeImage_OpenMemory_t)(uint8_t* data, int32_t size_in_bytes);
typedef void      (FIAPIENTRY *FreeImage_CloseMemory_t)(FIMEMORY* stream);
typedef FIBITMAP* (FIAPIENTRY *FreeImage_LoadFromMemory_t)(FREE_IMAGE_FORMAT fif, FIMEMORY* stream, int flags);

namespace {

    static StLibrary stLibFI;
    static FreeImage_GetVersion_t     FreeImage_GetVersion = NULL;
    static FreeImage_GetFileType_t    FreeImage_GetFileType = NULL;
    static FreeImage_GetFIFFromFilename_t FreeImage_GetFIFFromFilename = NULL;
    static FreeImage_Load_t           FreeImage_Load = NULL;
    static FreeImage_Save_t           FreeImage_Save = NULL;
    static FreeImage_Unload_t         FreeImage_Unload = NULL;
    static FreeImage_FIFSupportsReading_t FreeImage_FIFSupportsReading = NULL;
    static FreeImage_AllocateT_t      FreeImage_AllocateT = NULL;
    // DIB info routines
    static FreeImage_GetBits_t        FreeImage_GetBits = NULL;
    static FreeImage_GetBPP_t         FreeImage_GetBPP = NULL;
    static FreeImage_GetWidth_t       FreeImage_GetWidth = NULL;
    static FreeImage_GetHeight_t      FreeImage_GetHeight = NULL;
    static FreeImage_GetPitch_t       FreeImage_GetPitch = NULL;
    static FreeImage_GetImageType_t   FreeImage_GetImageType = NULL;
    static FreeImage_GetColorType_t   FreeImage_GetColorType = NULL;
    // memory I/O
    static FreeImage_OpenMemory_t     FreeImage_OpenMemory = NULL;
    static FreeImage_CloseMemory_t    FreeImage_CloseMemory = NULL;
    static FreeImage_LoadFromMemory_t FreeImage_LoadFromMemory = NULL;


    static bool initOnce() {
        // load the libraries
    #if defined(_WIN32)
        if(!stLibFI.loadSimple("FreeImage.dll")) {
            return false;
        }
    #elif defined(__APPLE__)
        StString aPath = StProcess::getStCoreFolder() + "../Frameworks/libfreeimage.dylib";
        if(!stLibFI.loadSimple(aPath)
        && !stLibFI.loadSimple("libfreeimage.dylib")) {
            return false;
        }
    #else
        if(!stLibFI.loadSimple("libfreeimage.so.3")
        && !stLibFI.loadSimple("libfreeimage.so")) {
            return false;
        }
    #endif

        // find functions
        if(!stLibFI("FreeImage_GetVersion",          FreeImage_GetVersion)
        #if defined(_WIN32)                          // retrieve Unicode versions
        || !stLibFI("FreeImage_GetFileTypeU",        FreeImage_GetFileType)
        || !stLibFI("FreeImage_GetFIFFromFilenameU", FreeImage_GetFIFFromFilename)
        || !stLibFI("FreeImage_LoadU",               FreeImage_Load)
        || !stLibFI("FreeImage_SaveU",               FreeImage_Save)
        #else
        || !stLibFI("FreeImage_GetFileType",         FreeImage_GetFileType)
        || !stLibFI("FreeImage_GetFIFFromFilename",  FreeImage_GetFIFFromFilename)
        || !stLibFI("FreeImage_Load",                FreeImage_Load)
        || !stLibFI("FreeImage_Save",                FreeImage_Save)
        #endif
        || !stLibFI("FreeImage_Unload",              FreeImage_Unload)
        || !stLibFI("FreeImage_FIFSupportsReading",  FreeImage_FIFSupportsReading)
        || !stLibFI("FreeImage_AllocateT",           FreeImage_AllocateT)
        // DIB info routines
        || !stLibFI("FreeImage_GetBits",             FreeImage_GetBits)
        || !stLibFI("FreeImage_GetBPP",              FreeImage_GetBPP)
        || !stLibFI("FreeImage_GetWidth",            FreeImage_GetWidth)
        || !stLibFI("FreeImage_GetHeight",           FreeImage_GetHeight)
        || !stLibFI("FreeImage_GetPitch",            FreeImage_GetPitch)
        || !stLibFI("FreeImage_GetImageType",        FreeImage_GetImageType)
        || !stLibFI("FreeImage_GetColorType",        FreeImage_GetColorType)
        // memory I/O
        || !stLibFI("FreeImage_OpenMemory",          FreeImage_OpenMemory)
        || !stLibFI("FreeImage_CloseMemory",         FreeImage_CloseMemory)
        || !stLibFI("FreeImage_LoadFromMemory",      FreeImage_LoadFromMemory)
        ) {
            // wrong library...
            stLibFI.close();
            return false;
        }

        // show up information about dynamically linked libraries
        ST_DEBUG_LOG("FreeImage " + FreeImage_GetVersion() + " initialized");
        return true;
    }

    static StImagePlane::ImgFormat convertFromFreeFormat(FREE_IMAGE_TYPE theFormatFI,
                                                         FREE_IMAGE_COLOR_TYPE theColorTypeFI,
                                                         unsigned theBitsPerPixel) {
        switch(theFormatFI) {
            case FIT_RGBF:   return StImagePlane::ImgRGBF;
            case FIT_RGBAF:  return StImagePlane::ImgRGBAF;
            case FIT_FLOAT:  return StImagePlane::ImgGrayF;
            case FIT_BITMAP: {
                switch(theColorTypeFI) {
                    case FIC_MINISBLACK: {
                        return StImagePlane::ImgGray;
                    }
                    case FIC_RGB: {
                    #ifdef FREEIMAGE_BIGENDIAN
                        return (theBitsPerPixel == 32) ? StImagePlane::ImgRGB32 : StImagePlane::ImgRGB;
                    #else
                        return (theBitsPerPixel == 32) ? StImagePlane::ImgBGR32 : StImagePlane::ImgBGR;
                    #endif
                    }
                    case FIC_RGBALPHA: {
                    #ifdef FREEIMAGE_BIGENDIAN
                        return StImagePlane::ImgRGBA;
                    #else
                        return StImagePlane::ImgBGRA;
                    #endif
                    }
                    default: return StImagePlane::ImgUNKNOWN;
                }
            }
            default:
                return StImagePlane::ImgUNKNOWN;
        }
    }

    static bool convertToFreeFormat(StImagePlane::ImgFormat theStFormat,
                                    FREE_IMAGE_TYPE& theFormatFI) {
        switch(theStFormat) {
            case StImagePlane::ImgGrayF:
                theFormatFI = FIT_FLOAT;
                return true;
            //case StImagePlane::ImgBGRAF:
            case StImagePlane::ImgRGBAF:
                theFormatFI = FIT_RGBAF;
                return true;
            //case StImagePlane::ImgBGRF:
            case StImagePlane::ImgRGBF:
                theFormatFI = FIT_RGBF;
                return true;
            case StImagePlane::ImgRGBA:
            case StImagePlane::ImgBGRA:
            case StImagePlane::ImgRGB32:
            case StImagePlane::ImgBGR32:
                theFormatFI = FIT_BITMAP;
                return true;
            case StImagePlane::ImgRGB:
            case StImagePlane::ImgBGR:
                theFormatFI = FIT_BITMAP;
                return true;
            case StImagePlane::ImgGray:
                theFormatFI = FIT_BITMAP;
                return true;
            default:
                return false;
        }
    }

};

bool StFreeImage::init() {
    static const bool isFreeImageInitiailed = initOnce();
    return isFreeImageInitiailed;
}

StFreeImage::StFreeImage()
: StImageFile(),
  myDIB(NULL) {
    StFreeImage::init();
}

StFreeImage::~StFreeImage() {
    close();
}

void StFreeImage::close() {
    if(!StFreeImage::init()) {
        return;
    }

    if(myDIB != NULL) {
        FreeImage_Unload(myDIB);
        myDIB = NULL;
    }
}

static FREE_IMAGE_FORMAT convertToFIF(StImageFile::ImageType theImageType) {
    switch(theImageType) {
        case StImageFile::ST_TYPE_PNG:
        case StImageFile::ST_TYPE_PNS: return FIF_PNG;
        case StImageFile::ST_TYPE_JPEG:
        case StImageFile::ST_TYPE_MPO:
        case StImageFile::ST_TYPE_JPS: return FIF_JPEG;
        case StImageFile::ST_TYPE_EXR: return FIF_EXR;
        case StImageFile::ST_TYPE_HDR: return FIF_HDR;
        case StImageFile::ST_TYPE_DDS: return FIF_DDS;
        default: return FIF_UNKNOWN;
    }
}

bool StFreeImage::loadExtra(const StString& theFilePath,
                            ImageType       theImageType,
                            uint8_t*        theDataPtr,
                            int             theDataSize,
                            bool            theIsOnlyRGB) {
    (void )theIsOnlyRGB;
    if(!StFreeImage::init()) {
        setState("FreeImage library is not initialized");
        return false;
    }

    // reset current data
    StImage::nullify();
    setState();
    close();

    FREE_IMAGE_FORMAT aFIF = convertToFIF(theImageType);
    if(theDataPtr != NULL && theDataSize != 0 && aFIF != FIF_UNKNOWN) {
        FIMEMORY* aFIMemory = FreeImage_OpenMemory(theDataPtr, theDataSize);
        if(aFIMemory == NULL) {
            setState("FreeImage library, internal error");
            return false;
        }
        myDIB = FreeImage_LoadFromMemory(aFIF, aFIMemory, 0);
        FreeImage_CloseMemory(aFIMemory);
    } else {
        // check the file signature and deduce its format
    #if defined(_WIN32)
        StStringUtfWide aFilePathWide = theFilePath.toUtfWide();
        aFIF = FreeImage_GetFileType(aFilePathWide.toCString(), 0);
    #else
        aFIF = FreeImage_GetFileType(theFilePath.toCString(), 0);
    #endif
        if(aFIF == FIF_UNKNOWN) {
            // no signature? try to guess the file format from the file extension
        #if defined(_WIN32)
            aFIF = FreeImage_GetFIFFromFilename(aFilePathWide.toCString());
        #else
            aFIF = FreeImage_GetFIFFromFilename(theFilePath.toCString());
        #endif
        }
        if((aFIF == FIF_UNKNOWN) || !FreeImage_FIFSupportsReading(aFIF)) {
            setState("FreeImage library does not support image format");
            return false;
        }

        int loadFlags = 0;
        if(aFIF == FIF_GIF) {
            // GIF_PLAYBACK - 'Play' the GIF to generate each frame (as 32bpp) instead of returning raw frame data when loading
            loadFlags = 2;
        } else if(aFIF == FIF_ICO) {
            // ICO_MAKEALPHA - convert to 32bpp and create an alpha channel from the AND-mask when loading
            loadFlags = 1;
        }
    #if defined(_WIN32)
        myDIB = FreeImage_Load(aFIF, aFilePathWide.toCString(), loadFlags);
    #else
        myDIB = FreeImage_Load(aFIF, theFilePath.toCString(),   loadFlags);
    #endif
    }
    if(myDIB == NULL) {
        setState("FreeImage library, loading file failed");
        return false;
    }

    StImagePlane::ImgFormat stImgFormat = convertFromFreeFormat(FreeImage_GetImageType(myDIB),
                                                                FreeImage_GetColorType(myDIB),
                                                                FreeImage_GetBPP(myDIB));

    if(stImgFormat == StImagePlane::ImgUNKNOWN) {
        setState(StString("StFreeImage, image format ")
                 + FreeImage_GetImageType(myDIB) + ", " + FreeImage_GetColorType(myDIB)
                 + " doesn't supported by application");
        close();
        return false;
    }

    setColorModelPacked(stImgFormat);
    changePlane(0).initWrapper(stImgFormat, FreeImage_GetBits(myDIB),
                               FreeImage_GetWidth(myDIB),
                               FreeImage_GetHeight(myDIB),
                               FreeImage_GetPitch(myDIB));
    // FreeImage data always bottom-up...
    changePlane(0).setTopDown(false);

    // set debug information
    StString aDummy, aFileName;
    StFileNode::getFolderAndFile(theFilePath, aDummy, aFileName);
    setState(StString("FreeImage library, loaded image '") + aFileName + "' " + getDescription());

    // we should not close the file because we create a wrapper over FreeImage native object
    return true;
}

bool StFreeImage::save(const StString& theFilePath,
                       ImageType       theImageType,
                       StFormat ) {
    if(!StFreeImage::init()) {
        setState("FreeImage library is not initialized");
        return false;
    }

    FREE_IMAGE_FORMAT aFIF = convertToFIF(theImageType);
    if(aFIF == FIF_UNKNOWN) {
        setState("FreeImage library, not supported image file format");
        return false;
    }

    StImage stSaveImage;
    if(getColorModel() != ImgColor_RGB && getColorModel() != ImgColor_RGBA && getColorModel() != ImgColor_GRAY) {
        // convert from YUV and so on
        if(!stSaveImage.initRGB(*this)) {
            setState("StFreeImage, only RGB image could be saved");
            return false;
        }
    } else {
        stSaveImage.initWrapper(*this);
    }
    const StImagePlane& stImgPlane = stSaveImage.getPlane();

    FREE_IMAGE_TYPE aSaveFormatFI = FIT_UNKNOWN;
    if(!convertToFreeFormat(stImgPlane.getFormat(), aSaveFormatFI)) {
        setState("StFreeImage, image format currently not supported");
        return false;
    }

    // allocate FreeImage native structure
    FIBITMAP* aSaveDIB = FreeImage_AllocateT(aSaveFormatFI, (int )stImgPlane.getSizeX(), (int )stImgPlane.getSizeY(),
                                             (unsigned )stImgPlane.getSizePixelBytes() * 8, 0, 0, 0);
    if(aSaveDIB == NULL) {
        setState("FreeImage library, internal error");
        FreeImage_Unload(aSaveDIB);
        return false;
    }
    // wrapper the created data
    StImagePlane stImgPlaneSave;
    StImagePlane::ImgFormat stImgFormatSave = convertFromFreeFormat(FreeImage_GetImageType(aSaveDIB),
                                                                    FreeImage_GetColorType(aSaveDIB),
                                                                    FreeImage_GetBPP(aSaveDIB));
    stImgPlaneSave.initWrapper(stImgFormatSave, FreeImage_GetBits(aSaveDIB),
                               FreeImage_GetWidth(aSaveDIB), FreeImage_GetHeight(aSaveDIB),
                               FreeImage_GetPitch(aSaveDIB));
    // FreeImage data should be bottom-up...
    stImgPlaneSave.setTopDown(false);

    // copy from local structure to the FreeImage structure
    size_t aRowInc = (( stImgPlaneSave.isTopDown() &&  stImgPlane.isTopDown()) ||
                      (!stImgPlaneSave.isTopDown() && !stImgPlane.isTopDown())) ? 1 : size_t(-1);
    size_t aRowTo = (aRowInc == 1) ? 0 : (stImgPlane.getSizeY() - 1);
    for(size_t aRowFrom = 0; aRowFrom < stImgPlane.getSizeY(); ++aRowFrom, aRowTo += aRowInc) {
        for(size_t aCol = 0; aCol < stImgPlane.getSizeX(); ++aCol) {
            stMemCpy(stImgPlaneSave.changeData(aRowTo, aCol), stImgPlane.getData(aRowFrom, aCol), stImgPlane.getSizePixelBytes());
        }
    }

    // now save the image file!
#if defined(_WIN32)
    if(!FreeImage_Save(aFIF, aSaveDIB, theFilePath.toUtfWide().toCString(), 0)) {
#else
    if(!FreeImage_Save(aFIF, aSaveDIB, theFilePath.toCString(), 0)) {
#endif
        setState("FreeImage library, image save failed");
        FreeImage_Unload(aSaveDIB);
        return false;
    }

    // free resources
    FreeImage_Unload(aSaveDIB);

    // set debug information
    StString aDummy, aFileName;
    StFileNode::getFolderAndFile(theFilePath, aDummy, aFileName);
    setState(StString("FreeImage library, saved image '") + aFileName + "' " + getDescription());

    return true;
}
