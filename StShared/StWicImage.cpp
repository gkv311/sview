/**
 * Copyright © 2026 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#include <StImage/StWicImage.h>

#include <StStrings/StLogger.h>
#include <StThreads/StMutex.h>
#include <StFile/StFileNode.h>

#ifdef _WIN32
#define ST_HAVE_WINCODEC
#endif

#ifdef ST_HAVE_WINCODEC
    #include <wincodec.h>

namespace {

    /** Return a zero GUID */
    static GUID getNullGuid() {
        GUID aGuid = {0, 0, 0, {0, 0, 0, 0, 0, 0, 0, 0}};
        return aGuid;
    }

    /** Sentry over IUnknown pointer. */
    template<class T> class Image_ComPtr {

            public:

        /** Empty constructor. */
        Image_ComPtr() : myPtr(nullptr) {}

        /** Destructor. */
        ~Image_ComPtr() { nullify(); }

        /** Return TRUE if pointer is NULL. */
        bool isNull() const { return myPtr == nullptr; }

        /** Release the pointer. */
        void nullify() {
            if (myPtr != nullptr) {
                myPtr->Release();
                myPtr = nullptr;
            }
        }

        /** Return pointer for initialization. */
        T*& changePtr() {
            if (myPtr != nullptr) {
                ST_ERROR_LOG("StWicImage, Pointer cannot be initialized twice!")
            }
            return myPtr;
        }

        /** Return pointer. */
        T* get() { return myPtr; }

        /** Return pointer. */
        T* operator->() { return get(); }

        /** Cast handle to contained type */
        T& operator*() { return *get(); }

            private:

        T* myPtr = nullptr;
    };

    /** Convert WIC GUID to sView pixel format. */
    static StImagePlane::ImgFormat convertFromWicFormat(const WICPixelFormatGUID& theFormat) {
        if (theFormat == GUID_WICPixelFormat32bppBGRA) {
            return StImagePlane::ImgBGRA;
        } else if (theFormat == GUID_WICPixelFormat32bppBGR) {
            return StImagePlane::ImgBGR32;
        } else if (theFormat == GUID_WICPixelFormat24bppRGB) {
            return StImagePlane::ImgRGB;
        } else if (theFormat == GUID_WICPixelFormat24bppBGR) {
            return StImagePlane::ImgBGR;
        } else if (theFormat == GUID_WICPixelFormat8bppGray) {
            return StImagePlane::ImgGray;
        } else if (theFormat == GUID_WICPixelFormat16bppGray) {
            return StImagePlane::ImgGray16;
        } else if (theFormat == GUID_WICPixelFormat32bppGrayFloat) {
            return StImagePlane::ImgGrayF;
        } else if (theFormat == GUID_WICPixelFormat32bppRGB) {
            return StImagePlane::ImgRGB32;
        } else if (theFormat == GUID_WICPixelFormat32bppRGBA) {
            return StImagePlane::ImgRGBA;
        } else if (theFormat == GUID_WICPixelFormat48bppRGB) {
            return StImagePlane::ImgRGB48;
        } else if (theFormat == GUID_WICPixelFormat64bppRGBA) {
            return StImagePlane::ImgRGBA64;
        } else if (theFormat == GUID_WICPixelFormat96bppRGBFloat) {
            return StImagePlane::ImgRGBF;
        } else if (theFormat == GUID_WICPixelFormat128bppRGBAFloat) {
            return StImagePlane::ImgRGBAF;
        }
        // planar formats are excluded
        return StImagePlane::ImgUNKNOWN;
    }

    /** Convert StImagePlane::ImgFormat to WIC GUID. */
    static WICPixelFormatGUID convertToWicFormat(StImagePlane::ImgFormat theFormat) {
        switch (theFormat) {
            case StImagePlane::ImgUNKNOWN: return getNullGuid();
            case StImagePlane::ImgGray:    return GUID_WICPixelFormat8bppGray;
            case StImagePlane::ImgGray16:  return GUID_WICPixelFormat16bppGray;
            case StImagePlane::ImgRGB:     return GUID_WICPixelFormat24bppRGB;
            case StImagePlane::ImgBGR:     return GUID_WICPixelFormat24bppBGR;
            case StImagePlane::ImgRGB32:   return GUID_WICPixelFormat32bppRGB;
            case StImagePlane::ImgBGR32:   return GUID_WICPixelFormat32bppBGR;
            case StImagePlane::ImgRGB48:   return GUID_WICPixelFormat48bppRGB;
            case StImagePlane::ImgRGBA:    return GUID_WICPixelFormat32bppRGBA;
            case StImagePlane::ImgBGRA:    return GUID_WICPixelFormat32bppBGRA;
            case StImagePlane::ImgRGBA64:  return GUID_WICPixelFormat64bppRGBA;
            case StImagePlane::ImgGrayF:   return GUID_WICPixelFormat32bppGrayFloat;
            case StImagePlane::ImgRGBF:    return GUID_WICPixelFormat96bppRGBFloat;
            case StImagePlane::ImgBGRF:    return getNullGuid();
            case StImagePlane::ImgRGBAF:   return GUID_WICPixelFormat128bppRGBAFloat;
            case StImagePlane::ImgBGRAF:   return getNullGuid();
            case StImagePlane::ImgUV:      return getNullGuid();
        }
        return getNullGuid();
    }

}

#endif

bool StWicImage::init() {
#ifdef ST_HAVE_WINCODEC
    return true;
#else
    return false;
#endif
}

StWicImage::StWicImage() {
    StWicImage::init();
}

StWicImage::~StWicImage() {
    close();
}

void StWicImage::close() {
    //
}

bool StWicImage::loadExtra(const StString& theFilePath,
                           ImageType       theImageType,
                           uint8_t*        theDataPtr,
                           int             theDataSize,
                           bool            theIsOnlyRGB) {
    (void )theIsOnlyRGB;
    if (!StWicImage::init()) {
        setState("WIC library is not initialized");
        return false;
    }

    // reset current data
    StImage::nullify();
    setState();
    close();

#ifdef ST_HAVE_WINCODEC
    Image_ComPtr<IWICImagingFactory> aWicImgFactory;
    CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    if (CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER,
                         IID_PPV_ARGS(&aWicImgFactory.changePtr())) != S_OK) {
        setState("Cannot initialize WIC Imaging Factory");
        return false;
    }

    Image_ComPtr<IWICBitmapDecoder> aWicDecoder;
    Image_ComPtr<IWICStream> aWicStream;

    if (theDataPtr != nullptr) {
        (void )theImageType;
        if (aWicImgFactory->CreateStream(&aWicStream.changePtr()) != S_OK
         || aWicStream->InitializeFromMemory((BYTE* )theDataPtr, (DWORD )theDataSize) != S_OK) {
            setState("Error: cannot initialize WIC Stream");
            return false;
        }
        if (aWicImgFactory->CreateDecoderFromStream(aWicStream.get(), nullptr,
                                                    WICDecodeMetadataCacheOnDemand,
                                                    &aWicDecoder.changePtr()) != S_OK) {
            setState("Error: cannot create WIC Image Decoder");
            return false;
        }

    } else {
        StStringUtfWide aFilePathWide = theFilePath.toUtfWide();
        if (aWicImgFactory->CreateDecoderFromFilename(aFilePathWide.toCString(), nullptr, GENERIC_READ,
                                                      WICDecodeMetadataCacheOnDemand,
                                                      &aWicDecoder.changePtr()) != S_OK) {
            setState("Error: cannot create WIC Image Decoder");
            return false;
        }
    }

    UINT aFrameCount = 0, aFrameSizeX = 0, aFrameSizeY = 0;
    WICPixelFormatGUID aWicPixelFormat = getNullGuid();
    Image_ComPtr<IWICBitmapFrameDecode> aWicFrameDecode;
    if (aWicDecoder->GetFrameCount(&aFrameCount) != S_OK
     || aFrameCount < 1
     || aWicDecoder->GetFrame(0, &aWicFrameDecode.changePtr()) != S_OK
     || aWicFrameDecode->GetSize(&aFrameSizeX, &aFrameSizeY) != S_OK
     || aWicFrameDecode->GetPixelFormat(&aWicPixelFormat)) {
        setState("Cannot get WIC Image Frame");
        return false;
    }

    Image_ComPtr<IWICFormatConverter> aWicConvertedFrame;
    StImagePlane::ImgFormat anImgFormat = convertFromWicFormat(aWicPixelFormat);
    if (anImgFormat == StImagePlane::ImgUNKNOWN) {
        anImgFormat = StImagePlane::ImgRGB;
        if (aWicImgFactory->CreateFormatConverter(&aWicConvertedFrame.changePtr()) != S_OK
         || aWicConvertedFrame->Initialize(aWicFrameDecode.get(), convertToWicFormat(anImgFormat),
                                           WICBitmapDitherTypeNone, nullptr, 0.0f,
                                           WICBitmapPaletteTypeCustom) != S_OK) {
            setState("Cannot convert WIC Image Frame to RGB format");
            return false;
        }
        ST_DEBUG_LOG("WIC image has been converted to RGB format")
        aWicFrameDecode.nullify();
    }

    setColorModelPacked(anImgFormat);
    if (!changePlane(0).initTrash(anImgFormat, aFrameSizeX, aFrameSizeY)) {
        setState("Unable to allocate memory for image data");
        return false;
    }

    IWICBitmapSource* aWicSrc = aWicFrameDecode.get();
    if (!aWicConvertedFrame.isNull()) {
        aWicSrc = aWicConvertedFrame.get();
    }

    if (aWicSrc->CopyPixels(nullptr,
                            (UINT)changePlane(0).getSizeRowBytes(),
                            (UINT)changePlane(0).getSizeBytes(),
                            changePlane(0).changeData()) != S_OK) {
        setState("Cannot copy pixels from WIC Image");
        return false;
    }
    changePlane(0).setTopDown(true);

    // set debug information
    StString aDummy, aFileName;
    StFileNode::getFolderAndFile(theFilePath, aDummy, aFileName);
    setState(StString("WIC library, loaded image '") + aFileName + "' " + getDescription());
    return true;
#else
    (void )theFilePath;
    (void )theImageType;
    (void )theDataPtr;
    (void )theDataSize;
    return false;
#endif
}

bool StWicImage::save(const StString&, const SaveImageParams& ) {
    setState("WIC library, save operation is NOT implemented");
    return false;
}
