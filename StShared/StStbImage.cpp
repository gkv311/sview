/**
 * Copyright © 2019 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#include <StImage/StStbImage.h>

#include <StStrings/StLogger.h>
#include <StThreads/StMutex.h>
#include <StFile/StFileNode.h>

//#define ST_HAVE_STB_IMAGE

#ifdef ST_HAVE_STB_IMAGE
    #define STB_IMAGE_IMPLEMENTATION
    //#define STB_IMAGE_RESIZE_IMPLEMENTATION
    //#define STB_IMAGE_WRITE_IMPLEMENTATION
    #define STBI_WINDOWS_UTF8
    #include <stb_image.h>
    //#include <stb_image_resize.h>
    //#include <stb_image_write.h>

namespace {

    static StImagePlane::ImgFormat convertFromStbFormat(int theChannels, bool theIsHDR) {
        switch(theChannels) {
            case STBI_grey:       return theIsHDR ? StImagePlane::ImgGrayF : StImagePlane::ImgGray;
            case STBI_grey_alpha: return StImagePlane::ImgUNKNOWN;
            case STBI_rgb:        return theIsHDR ? StImagePlane::ImgRGBF  : StImagePlane::ImgRGB;
            case STBI_rgb_alpha:  return theIsHDR ? StImagePlane::ImgRGBAF : StImagePlane::ImgRGBA;
        }
        return StImagePlane::ImgUNKNOWN;
    }

}

#endif

bool StStbImage::init() {
#ifdef ST_HAVE_STB_IMAGE
    return true;
#else
    return false;
#endif
}

StStbImage::StStbImage()
: myStbImage(NULL) {
    StStbImage::init();
}

StStbImage::~StStbImage() {
    close();
}

void StStbImage::close() {
    if(myStbImage != NULL) {
    #ifdef ST_HAVE_STB_IMAGE
        stbi_image_free(myStbImage);
    #endif
        myStbImage = NULL;
    }
}

bool StStbImage::loadExtra(const StString& theFilePath,
                           ImageType       theImageType,
                           uint8_t*        theDataPtr,
                           int             theDataSize,
                           bool            theIsOnlyRGB) {
    (void )theIsOnlyRGB;
    if(!StStbImage::init()) {
        setState("STB library is not initialized");
        return false;
    }

    // reset current data
    StImage::nullify();
    setState();
    close();

#ifdef ST_HAVE_STB_IMAGE
    bool isHdr = false;
    int aWidth = 0, aHeight = 0, aChannels = 0;
    //stbi_set_flip_vertically_on_load(true);
    if(theDataPtr != NULL) {
        (void )theImageType;
        //stbi_info_from_memory();
        isHdr = stbi_is_hdr_from_memory((unsigned char*)theDataPtr, theDataSize) != 0;
        if(isHdr) {
            //myStbImage = stbi_load_16_from_memory((unsigned char*)theDataPtr, theDataSize, &aWidth, &aHeight, &aChannels, STBI_default);
            myStbImage = stbi_loadf_from_memory((unsigned char*)theDataPtr, theDataSize, &aWidth, &aHeight, &aChannels, STBI_default);
        } else {
            myStbImage = stbi_load_from_memory((unsigned char*)theDataPtr, theDataSize, &aWidth, &aHeight, &aChannels, STBI_default);
        }
    } else {
        //stbi_info();
        isHdr = stbi_is_hdr(theFilePath.toCString()) != 0;
        if(isHdr) {
            //myStbImage = stbi_load_16(theFilePath.toCString(), &aWidth, &aHeight, &aChannels, STBI_default);
            myStbImage = stbi_loadf(theFilePath.toCString(), &aWidth, &aHeight, &aChannels, STBI_default);
        } else {
            myStbImage = stbi_load(theFilePath.toCString(), &aWidth, &aHeight, &aChannels, STBI_default);
        }
    }

    if(myStbImage == NULL
    || aWidth  < 1
    || aHeight < 1) {
        //stbi_failure_reason()
        setState("STB library, unable to load image");
        close();
        return false;
    }

    StImagePlane::ImgFormat anImgFormat = convertFromStbFormat(aChannels, isHdr);
    if(anImgFormat == StImagePlane::ImgUNKNOWN) {
        setState("STB library, unknown pixel format");
        close();
        return false;
    }

    setColorModelPacked(anImgFormat);
    changePlane(0).initWrapper(anImgFormat, (unsigned char* )myStbImage, aWidth, aHeight);

    // set debug information
    StString aDummy, aFileName;
    StFileNode::getFolderAndFile(theFilePath, aDummy, aFileName);
    setState(StString("STB library, loaded image '") + aFileName + "' " + getDescription());
    return true;
#else
    (void )theFilePath;
    (void )theImageType;
    (void )theDataPtr;
    (void )theDataSize;
    return false;
#endif
}

bool StStbImage::save(const StString& ,
                      ImageType ,
                      StFormat ) {
    setState("STB library, save operation is NOT implemented");
    return false;
}
