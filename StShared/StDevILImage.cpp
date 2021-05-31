/**
 * Copyright © 2011-2015 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#include <StImage/StDevILImage.h>

#include <StLibrary.h>
#include <StStrings/StLogger.h>
#include <StThreads/StMutex.h>
#include <StFile/StFileNode.h>

#if defined(_WIN32)
    #define ILAPIENTRY __stdcall
    typedef stUtfWide_t ilUtfChar_t;
#else
    #define ILAPIENTRY
    typedef stUtf8_t ilUtfChar_t;
#endif

typedef unsigned int   ILenum;
typedef unsigned char  ILboolean;
typedef int     	   ILint;
typedef size_t         ILsizei;
typedef unsigned char  ILubyte;
typedef unsigned int   ILuint;

// Origin Definitions
#define IL_ORIGIN_SET        0x0600
#define IL_ORIGIN_LOWER_LEFT 0x0601
#define IL_ORIGIN_UPPER_LEFT 0x0602
#define IL_ORIGIN_MODE       0x0603

#define IL_NO_ERROR          0x0000

// Values
#define IL_VERSION_NUM       0x0DE2
#define IL_IMAGE_WIDTH       0x0DE4
#define IL_IMAGE_HEIGHT      0x0DE5
#define IL_IMAGE_DEPTH       0x0DE6
#define IL_IMAGE_FORMAT      0x0DEA
#define IL_IMAGE_TYPE        0x0DEB

// Image types
#define IL_TYPE_UNKNOWN 0x0000
#define IL_BMP          0x0420
#define IL_JPG          0x0425
#define IL_PNG          0x042A
#define IL_PSD          0x0439
#define IL_HDR          0x043F
#define IL_DDS          0x0437

// Matches OpenGL's right now.
#define IL_RGB              0x1907
#define IL_RGBA             0x1908
#define IL_BGR              0x80E0
#define IL_BGRA             0x80E1
#define IL_LUMINANCE        0x1909
#define IL_LUMINANCE_ALPHA  0x190A
#define IL_ALPHA			0x190B

#define IL_UNSIGNED_BYTE  0x1401
#define IL_FLOAT          0x1406

#define ILU_FILTER         0x2600
#define ILU_BILINEAR       0x2603
#define ILU_SCALE_LANCZOS3 0x2608

// IL library
typedef void      (ILAPIENTRY *ilInit_t)(void);
typedef ILubyte*  (ILAPIENTRY *ilGetData_t)(void);
typedef void      (ILAPIENTRY *ilGenImages_t)(ILsizei Num, ILuint* Images);
typedef void      (ILAPIENTRY *ilDeleteImages_t)(ILsizei Num, const ILuint* Images);
typedef void      (ILAPIENTRY *ilBindImage_t)(ILuint Image);
typedef ILenum    (ILAPIENTRY *ilGetError_t)(void);
typedef ILint     (ILAPIENTRY *ilGetInteger_t)(ILenum Mode);
typedef ILboolean (ILAPIENTRY *ilEnable_t)(ILenum Mode);
typedef ILboolean (ILAPIENTRY *ilOriginFunc_t)(ILenum Mode);
typedef ILboolean (ILAPIENTRY *ilLoad_t)(ILenum Type, ilUtfChar_t const* FileName);
typedef ILboolean (ILAPIENTRY *ilLoadImage_t)(ilUtfChar_t const* FileName);
typedef ILboolean (ILAPIENTRY *ilLoadL_t)(ILenum Type, const void* Lump, ILuint Size);
typedef ILboolean (ILAPIENTRY *ilSave_t)(ILenum Type, const ilUtfChar_t* FileName);
typedef ILboolean (ILAPIENTRY *ilConvertImage_t)(ILenum DestFormat, ILenum DestType);
typedef ILboolean (ILAPIENTRY *ilTexImage_t)(ILuint Width, ILuint Height, ILuint Depth, ILubyte numChannels, ILenum Format, ILenum Type, void *Data);
// ILU library
typedef void      (ILAPIENTRY *iluInit_t)(void);
typedef ILint     (ILAPIENTRY *iluGetInteger_t)(ILenum Mode);
typedef ilUtfChar_t const* (ILAPIENTRY *iluErrorString_t)(ILenum Error);
typedef ILboolean (ILAPIENTRY *iluScale_t)(ILuint Width, ILuint Height, ILuint Depth);
typedef void      (ILAPIENTRY *iluImageParameter_t)(ILenum PName, ILenum Param);
typedef ILboolean (ILAPIENTRY *iluFlipImage_t)(void);

namespace {

    static StMutex stDevILMutex;
    static StLibrary stLibIL;
    static StLibrary stLibILU;
    static ilInit_t             ilInit = NULL;
    static ilGetData_t          ilGetData = NULL;
    static ilGenImages_t        ilGenImages = NULL;
    static ilDeleteImages_t     ilDeleteImages = NULL;
    static ilBindImage_t        ilBindImage = NULL;
    static ilGetError_t         ilGetError = NULL;
    static ilGetInteger_t       ilGetInteger = NULL;
    static ilEnable_t           ilEnable = NULL;
    static ilOriginFunc_t       ilOriginFunc = NULL;
    static ilLoad_t             ilLoad = NULL;
    static ilLoadImage_t        ilLoadImage = NULL;
    static ilLoadL_t            ilLoadL = NULL;
    static ilSave_t             ilSave = NULL;
    static ilConvertImage_t     ilConvertImage = NULL;
    static ilTexImage_t         ilTexImage = NULL;
    static iluInit_t            iluInit = NULL;
    static iluGetInteger_t      iluGetInteger = NULL;
    static iluErrorString_t     iluErrorString = NULL;
    static iluScale_t           iluScale = NULL;
    static iluImageParameter_t  iluImageParameter = NULL;
    static iluFlipImage_t       iluFlipImage = NULL;

#ifdef ST_DEBUG
    static ILint getVerMajor(ILint theFullVersion) {
        return theFullVersion / 100;
    }

    static ILint getVerMinor(ILint theFullVersion) {
        return (theFullVersion - getVerMajor(theFullVersion) * 100) / 10;
    }

    static ILint getVerMicro(ILint theFullVersion) {
        return theFullVersion - getVerMajor(theFullVersion) * 100 - getVerMinor(theFullVersion) * 10;
    }
#endif

    static bool initOnce() {
        StMutexAuto stTempLock(stDevILMutex);
        // load the libraries
    #if defined(_WIN32)
        if(!stLibIL.loadSimple("DevIL.dll")) {
            return false;
        }
        if(!stLibILU.loadSimple("ILU.dll")) {
            stLibIL.close();
            return false;
        }
    #else
        if(!stLibIL.loadSimple("libIL.so.1")
        && !stLibIL.loadSimple("libIL.so")) {
            return false;
        }
        if(!stLibILU.loadSimple("libILU.so.1")
        && !stLibILU.loadSimple("libILU.so")) {
            stLibIL.close();
            return false;
        }
    #endif

        // find functions
        if(!stLibIL("ilInit",         ilInit)
        || !stLibIL("ilGetData",      ilGetData)
        || !stLibIL("ilGenImages",    ilGenImages)
        || !stLibIL("ilDeleteImages", ilDeleteImages)
        || !stLibIL("ilBindImage",    ilBindImage)
        || !stLibIL("ilGetError",     ilGetError)
        || !stLibIL("ilGetInteger",   ilGetInteger)
        || !stLibIL("ilEnable",       ilEnable)
        || !stLibIL("ilOriginFunc",   ilOriginFunc)
        || !stLibIL("ilLoad",         ilLoad)
        || !stLibIL("ilLoadImage",    ilLoadImage)
        || !stLibIL("ilLoadL",        ilLoadL)
        || !stLibIL("ilSave",         ilSave)
        || !stLibIL("ilConvertImage", ilConvertImage)
        || !stLibIL("ilTexImage",     ilTexImage)) {
            // wrong library...
            stLibIL.close();
            stLibILU.close();
            return false;
        }

        if(!stLibILU("iluInit",           iluInit)
        || !stLibILU("iluGetInteger",     iluGetInteger)
        || !stLibILU("iluErrorString",    iluErrorString)
        || !stLibILU("iluScale",          iluScale)
        || !stLibILU("iluImageParameter", iluImageParameter)
        || !stLibILU("iluFlipImage",      iluFlipImage)) {
            // wrong library...
            stLibIL.close();
            stLibILU.close();
            return false;
        }

        // call initializers
        ilInit();   // initialization main DevIL library
        iluInit();  // initialization DevIL-utilities library
        ilOriginFunc(IL_ORIGIN_UPPER_LEFT); // be sure origin always right
        ilEnable(IL_ORIGIN_SET);            // if source image data saved with another origin - DevIL flip image automatically
        ST_DEBUG_LOG("DevIL library initialized");

        // show up information about dynamically linked libraries
    #ifdef ST_DEBUG
        ILint aVersion = ilGetInteger(IL_VERSION_NUM);
        ST_DEBUG_LOG("  IL\t"  + getVerMajor(aVersion) + '.' + getVerMinor(aVersion) + '.' + getVerMicro(aVersion));
        aVersion = iluGetInteger(IL_VERSION_NUM);
        ST_DEBUG_LOG("  ILU\t" + getVerMajor(aVersion) + '.' + getVerMinor(aVersion) + '.' + getVerMicro(aVersion));
    #endif
        return true;
    }

    /**
     * Auxiliary function to retrieve error description from DevIL.
     */
    static bool isNoError(StString& theDescription) {
        int errDevIL = ilGetError(); // get DevIL-error code from stack
        if(errDevIL != IL_NO_ERROR) {
            bool isFirstError = true;
            theDescription = "DevIL library, ";
            while(errDevIL != IL_NO_ERROR) {
                theDescription += StString(isFirstError ? "" : ", ");
                theDescription += StString(iluErrorString(errDevIL));
                errDevIL = ilGetError();
                isFirstError = false;
            }
            return false;
        }
        return true;
    }

    static ILenum convertStImage2DevIL(int theImageType) {
        switch(theImageType) {
            case StImageFile::ST_TYPE_PNG:
            case StImageFile::ST_TYPE_PNS:  return IL_PNG;
            case StImageFile::ST_TYPE_JPEG:
            case StImageFile::ST_TYPE_MPO:
            case StImageFile::ST_TYPE_JPS:  return IL_JPG;
            case StImageFile::ST_TYPE_PSD:  return IL_PSD;
            case StImageFile::ST_TYPE_HDR:  return IL_HDR;
            case StImageFile::ST_TYPE_DDS:  return IL_DDS;
            case StImageFile::ST_TYPE_NONE:
            default:
                return IL_TYPE_UNKNOWN;
        }
    }

    static StImagePlane::ImgFormat convertFromDevILFormat() {
        ILenum theFormat = ilGetInteger(IL_IMAGE_FORMAT);
        ILenum theType   = ilGetInteger(IL_IMAGE_TYPE);
        if(theFormat == IL_RGB) {
            if(theType == IL_UNSIGNED_BYTE) {
                return StImagePlane::ImgRGB;
            } else if(theType == IL_FLOAT) {
                return StImagePlane::ImgRGBF;
            }
        } else if(theFormat == IL_BGR) {
            if(theType == IL_UNSIGNED_BYTE) {
                return StImagePlane::ImgBGR;
            } else if(theType == IL_FLOAT) {
                return StImagePlane::ImgBGRF;
            }
        } else if(theFormat == IL_RGBA) {
            if(theType == IL_UNSIGNED_BYTE) {
                return StImagePlane::ImgRGBA;
            } else if(theType == IL_FLOAT) {
                return StImagePlane::ImgRGBAF;
            }
        } else if(theFormat == IL_BGRA) {
            if(theType == IL_UNSIGNED_BYTE) {
                return StImagePlane::ImgBGRA;
            } else if(theType == IL_FLOAT) {
                return StImagePlane::ImgBGRAF;
            }
        } else if(theFormat == IL_LUMINANCE || theFormat == IL_LUMINANCE_ALPHA || theFormat == IL_ALPHA) {
            if(theType == IL_UNSIGNED_BYTE) {
                return StImagePlane::ImgGray;
            } else if(theType == IL_FLOAT) {
                return StImagePlane::ImgGrayF;
            }
        }
        return StImagePlane::ImgUNKNOWN;
    }

};

bool StDevILImage::init() {
    static const bool isDevILInitiailed = initOnce();
    return isDevILInitiailed;
}

StDevILImage::StDevILImage()
: StImageFile(),
  myImageId(0) {
    StDevILImage::init();
}

StDevILImage::~StDevILImage() {
    close();
}

void StDevILImage::close() {
    // DevIL is not thread-safe! Lock for whole function
    StMutexAuto stTempLock(stDevILMutex);
    if(!StDevILImage::init()) {
        return;
    }

    if(isValid()) {
        ilBindImage(0);
        ilDeleteImages(1, &myImageId);
        myImageId = 0;
    }
}

bool StDevILImage::loadExtra(const StString& theFilePath,
                             ImageType       theImageType,
                             uint8_t*        theDataPtr,
                             int             theDataSize,
                             bool            theIsOnlyRGB) {
    (void )theIsOnlyRGB;

    // DevIL is not thread-safe! Lock for whole function
    StMutexAuto stTempLock(stDevILMutex);
    if(!StDevILImage::init()) {
        setState("DevIL library is not initialized");
        return false;
    }

    // reset current data
    StImage::nullify();
    setState();
    close();

    // generate new id
    ilGenImages(1, &myImageId);
    ilBindImage(myImageId);

    ILenum anImageTypeIl = convertStImage2DevIL(theImageType);

    if(theDataPtr != NULL && theDataSize != 0 && anImageTypeIl != IL_TYPE_UNKNOWN) {
        ilLoadL(anImageTypeIl, theDataPtr, (ILuint )theDataSize);
    } else if(anImageTypeIl != IL_TYPE_UNKNOWN) {
    #if defined(_WIN32)
        ilLoad(anImageTypeIl, theFilePath.toUtfWide().toCString());
    #else
        ilLoad(anImageTypeIl, theFilePath.toCString());
    #endif
    } else {
        // loading image with format auto-detection
    #if defined(_WIN32)
        ilLoadImage(theFilePath.toUtfWide().toCString());
    #else
        ilLoadImage(theFilePath.toCString());
    #endif
    }

    if(!isNoError(changeState())) {
        close();
        return false;
    } else if(ilGetInteger(IL_IMAGE_WIDTH) <= 0 || ilGetInteger(IL_IMAGE_HEIGHT) <= 0) {
        setState("DevIL library, wrong frame size");
        close();
        return false;
    }

    StImagePlane::ImgFormat anImgFormat = convertFromDevILFormat();
    if(anImgFormat == StImagePlane::ImgUNKNOWN) {
        ilConvertImage(IL_RGB, IL_UNSIGNED_BYTE);
        anImgFormat = StImagePlane::ImgRGB;
    }

    setColorModelPacked(anImgFormat);
    changePlane(0).initWrapper(anImgFormat, ilGetData(),
                               size_t(ilGetInteger(IL_IMAGE_WIDTH)), size_t(ilGetInteger(IL_IMAGE_HEIGHT)));

    // set debug information
    StString dummy, fileName;
    StFileNode::getFolderAndFile(theFilePath, dummy, fileName);
    setState(StString("DevIL library, loaded image '") + fileName + "' " + getDescription());

    // we should not close the file because we create a wrapper over DevIL native object
    ilBindImage(0);
    return true;
}

bool StDevILImage::resize(size_t theSizeX, size_t theSizeY) {
    if(!isValid()) {
        setState("DevIL library, can not resize an alien image");
        return false;
    }

    // DevIL is not thread-safe! Lock for whole function
    StMutexAuto stTempLock(stDevILMutex);
    if(!StDevILImage::init()) {
        setState("DevIL library is not initialized");
        return false;
    }

    ilBindImage(myImageId);
        iluImageParameter(ILU_FILTER, ILU_BILINEAR); // ILU_SCALE_LANCZOS3
        iluScale((ILint )theSizeX, (ILint )theSizeY, 3);
    ilBindImage(0);
    return true;
}

bool StDevILImage::save(const StString& theFilePath,
                        ImageType       theImageType,
                        StFormat ) {
    setState();

    ILenum anImageTypeIl = convertStImage2DevIL(theImageType);
    if(anImageTypeIl == IL_TYPE_UNKNOWN) {
        setState("DevIL library, doesn't requested image type");
        return false;
    }

    StImage anRGBImage;
    if(!anRGBImage.initRGB(*this)) {
        setState("StDevILImage, only RGB image could be saved");
        return false;
    }

    // DevIL is not thread-safe! Lock for whole function
    StMutexAuto stTempLock(stDevILMutex);
    if(!StDevILImage::init()) {
        setState("DevIL library is not initialized");
        return false;
    }

    if(!isValid()) {
        ilGenImages(1, &myImageId);
    }
    if(!isValid()) {
        setState("DevIL library, fail to generate image object");
        return false;
    }

    ilBindImage(myImageId);
        ilTexImage(ILuint(anRGBImage.getSizeX()), ILuint(anRGBImage.getSizeY()), 1, 3, IL_RGB, IL_UNSIGNED_BYTE, NULL);
        size_t aRowSizeBytes = anRGBImage.getSizeX() * 3;
        for(size_t aRow = 0; aRow < anRGBImage.getSizeY(); ++aRow) {
            stMemCpy(ilGetData() + aRowSizeBytes * aRow,
                     anRGBImage.getPlane(0).getData(aRow),
                     aRowSizeBytes);
        }
        iluFlipImage(); // what a hell are we doing???

    #if defined(_WIN32)
        ilSave(anImageTypeIl, theFilePath.toUtfWide().toCString());
    #else
        ilSave(anImageTypeIl, theFilePath.toCString());
    #endif
    ilBindImage(0);

    if(!isNoError(changeState())) {
       return false;
    }

    // set debug information
    StString dummy, fileName;
    StFileNode::getFolderAndFile(theFilePath, dummy, fileName);
    setState(StString("DevIL library, saved image '") + fileName + "' " + getDescription());
    return true;
}
