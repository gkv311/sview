/**
 * Copyright © 2012 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#include <StImage/StWebPImage.h>

#include <StLibrary.h>
#include <StStrings/StLogger.h>
#include <StThreads/StMutex.h>
#include <StFile/StRawFile.h>

extern "C" {

    #define WEBP_DECODER_ABI_VERSION 0x0200 // MAJOR(8b) + MINOR(8b)

    typedef enum {
        VP8_STATUS_OK = 0,
        VP8_STATUS_OUT_OF_MEMORY,
        VP8_STATUS_INVALID_PARAM,
        VP8_STATUS_BITSTREAM_ERROR,
        VP8_STATUS_UNSUPPORTED_FEATURE,
        VP8_STATUS_SUSPENDED,
        VP8_STATUS_USER_ABORT,
        VP8_STATUS_NOT_ENOUGH_DATA
    } VP8StatusCode;

#ifdef ST_HAVE_WEBP
    static const char* const WebPStatusMessages[] = {
        "OK",                  // VP8_STATUS_OK
        "OUT_OF_MEMORY",       // VP8_STATUS_OUT_OF_MEMORY
        "INVALID_PARAM",       // VP8_STATUS_INVALID_PARAM
        "BITSTREAM_ERROR",     // VP8_STATUS_BITSTREAM_ERROR
        "UNSUPPORTED_FEATURE", // VP8_STATUS_UNSUPPORTED_FEATURE
        "SUSPENDED",           // VP8_STATUS_SUSPENDED
        "USER_ABORT",          // VP8_STATUS_USER_ABORT
        "NOT_ENOUGH_DATA"      // VP8_STATUS_NOT_ENOUGH_DATA
    };
#endif

    extern int WebPInitDecoderConfigInternal(StWebPImage::WebPDecoderConfig* theConfig, int theVersion);

    inline int WebPInitDecoderConfig(StWebPImage::WebPDecoderConfig* theConfig) {
        return WebPInitDecoderConfigInternal(theConfig, WEBP_DECODER_ABI_VERSION);
    }

    extern VP8StatusCode WebPGetFeaturesInternal(const uint8_t* theData, size_t theDataSize,
                                                 StWebPImage::WebPBitstreamFeatures* theFeatures, int theVersion);

    inline VP8StatusCode WebPGetFeatures(const uint8_t* theData, size_t theDataSize,
                                         StWebPImage::WebPBitstreamFeatures* theFeatures) {
        return WebPGetFeaturesInternal(theData, theDataSize, theFeatures, WEBP_DECODER_ABI_VERSION);
    }

    extern VP8StatusCode WebPDecode(const uint8_t* theData, size_t theDataSize,
                                    StWebPImage::WebPDecoderConfig* theConfig);

    extern void WebPFreeDecBuffer(StWebPImage::WebPDecBuffer* theBuffer);

};

bool StWebPImage::init() {
#ifdef ST_HAVE_WEBP
    WebPDecoderConfig aDummy;
    return WebPInitDecoderConfig(&aDummy) != 0;
#else
    return false;
#endif
}

StWebPImage::StWebPImage()
: myIsCompat(false) {
#ifdef ST_HAVE_WEBP
    myIsCompat = WebPInitDecoderConfig(&myConfig) != 0;
#endif
}

StWebPImage::~StWebPImage() {
    close();
}

void StWebPImage::close() {
#ifdef ST_HAVE_WEBP
    if(myIsCompat) {
        WebPFreeDecBuffer(&myConfig.output);
    }
#else
    (void )myConfig;
    (void )myIsCompat;
#endif
}

bool StWebPImage::loadInternal(const StString& theFilePath,
                               const uint8_t*  theDataPtr,
                               const int       theDataSize,
                               const bool      theIsRGB) {
#ifdef ST_HAVE_WEBP
    if(!WebPInitDecoderConfig(&myConfig)) {
        setState("WebP library, library version mismatch");
        close();
        return false;
    }

    // MT
    //myConfig.options.use_threads = 1;

    VP8StatusCode aRes = WebPGetFeatures(theDataPtr, (size_t )theDataSize, &myConfig.input);
    if(aRes != VP8_STATUS_OK) {
        StString aDesc = (aRes >= VP8_STATUS_OK && aRes <= VP8_STATUS_NOT_ENOUGH_DATA) ? WebPStatusMessages[aRes] : "UNKNOWN";
        setState(StString("WebP library, could not decode file header [") + aDesc + "]");
        close();
        return false;
    }

    if(myConfig.input.has_alpha) {
        // sView currently doesn't support YUVA - force RGBA
        myConfig.output.colorspace = MODE_RGBA;
    } else if(theIsRGB) {
        myConfig.output.colorspace = MODE_RGB;
    } else {
        myConfig.output.colorspace = MODE_YUV;
    }

    aRes = WebPDecode(theDataPtr, (size_t )theDataSize, &myConfig);
    if(aRes != VP8_STATUS_OK) {
        StString aDesc = (aRes >= VP8_STATUS_OK && aRes <= VP8_STATUS_NOT_ENOUGH_DATA) ? WebPStatusMessages[aRes] : "UNKNOWN";
        setState(StString("WebP library, could not decode the file [") + aDesc + "]");
        close();
        return false;
    }

    if(myConfig.input.has_alpha) {
        const WebPRGBABuffer& aRGBA = myConfig.output.u.RGBA;
        setColorModel(StImage::ImgColor_RGBA);
        changePlane(0).initWrapper(StImagePlane::ImgRGBA, (GLubyte* )aRGBA.rgba,
                                   myConfig.output.width, myConfig.output.height, aRGBA.stride);
    } else if(theIsRGB) {
        const WebPRGBABuffer& aRGBA = myConfig.output.u.RGBA;
        setColorModel(StImage::ImgColor_RGB);
        changePlane(0).initWrapper(StImagePlane::ImgRGB, (GLubyte* )aRGBA.rgba,
                                   myConfig.output.width, myConfig.output.height, aRGBA.stride);
    } else {
        const WebPYUVABuffer& aYUVA = myConfig.output.u.YUVA;

        setColorModel(StImage::ImgColor_YUV); // not full range!
        changePlane(0).initWrapper(StImagePlane::ImgGray, aYUVA.y,
                                   myConfig.output.width, myConfig.output.height, aYUVA.y_stride);
        changePlane(1).initWrapper(StImagePlane::ImgGray, aYUVA.u,
                                   myConfig.output.width / 2, myConfig.output.height / 2, aYUVA.u_stride);
        changePlane(2).initWrapper(StImagePlane::ImgGray, aYUVA.v,
                                   myConfig.output.width / 2, myConfig.output.height / 2, aYUVA.v_stride);
    }

    // set debug information
    StString aDummy, aFileName;
    StFileNode::getFolderAndFile(theFilePath, aDummy, aFileName);
    setState(StString("WebP library, loaded image '") + aFileName + "' " + getDescription());

    return true;
#else
    return false;
#endif
}

bool StWebPImage::loadExtra(const StString& theFilePath,
                            ImageType       theImageType,
                            uint8_t*        theDataPtr,
                            int             theDataSize,
                            bool            theIsOnlyRGB) {
#ifndef ST_HAVE_WEBP
    setState("WebP library is not initialized");
    return false;
#else
    // reset current data
    StImage::nullify();
    setState();
    close();

    // read file
    StRawFile aRawFile(theFilePath);
    if(theDataPtr == NULL || theDataSize == 0) {
        if(!aRawFile.readFile()) {
            setState("StWebPImage, could not read the file");
            close();
            return false;
        }
        theDataPtr  = (uint8_t* )aRawFile.getBuffer();
        theDataSize = (int      )aRawFile.getSize();
    }

    if(theImageType == ST_TYPE_NONE) {
        theImageType = guessImageType(theFilePath, StMIME());
    }

    switch(theImageType) {
        case ST_TYPE_NONE:
        case ST_TYPE_WEBP: {
            return loadInternal(theFilePath, theDataPtr, theDataSize, theIsOnlyRGB);
        }
        case ST_TYPE_WEBPLL: {
            return loadInternal(theFilePath, theDataPtr, theDataSize, true);
        }
        default: {
            setState(StString("StWebPImage, unsupported image type id #") + theImageType + '!');
            close();
            return false;
        }
    }
#endif
}

bool StWebPImage::save(const StString& /*theFilePath*/,
                       ImageType       /*theImageType*/,
                       StFormat        /*theSrcFormat*/) {
    return false;
}
