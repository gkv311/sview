/**
 * Copyright © 2026 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#include <StImage/StNsImage.h>

#include <StStrings/StLogger.h>
#include <StThreads/StMutex.h>
#include <StFile/StFileNode.h>

#ifdef __APPLE__
#define ST_HAVE_NSIMAGE
#include <StCocoa/StCocoaLocalPool.h>
#include <StCocoa/StCocoaString.h>
#endif

#include <sstream>

#ifdef ST_HAVE_NSIMAGE
    #import <Cocoa/Cocoa.h>

namespace {

    /** Convert NSBitmapImageRep format to sView pixel format. */
    static StImagePlane::ImgFormat convertFromNSFormat(NSBitmapImageRep* theNsImgRep) {
        if ([theNsImgRep isPlanar]) {
            // planar formats are excluded
            return StImagePlane::ImgUNKNOWN;
        }

        const bool isFloat = ([theNsImgRep bitmapFormat] & NSBitmapFormatFloatingPointSamples) != 0;

        if ([theNsImgRep samplesPerPixel] == 1) {
            if ([theNsImgRep bitsPerPixel] == 8)
                return StImagePlane::ImgGray;
            else if (!isFloat && [theNsImgRep bitsPerPixel] == 16)
                return StImagePlane::ImgGray16;
            else if (isFloat && [theNsImgRep bitsPerPixel] == 16) // from EXR
                return StImagePlane::ImgUNKNOWN; // unsupported half-float format
            else if (isFloat && [theNsImgRep bitsPerPixel] == 32)
                return StImagePlane::ImgGrayF;
        } else if ([theNsImgRep samplesPerPixel] == 3) {
            if ([theNsImgRep bitsPerPixel] == 24)
                return StImagePlane::ImgRGB;
            else if ([theNsImgRep bitsPerPixel] == 32)
                return StImagePlane::ImgRGB32;
            else if ([theNsImgRep bitsPerPixel] == 48)
                return StImagePlane::ImgRGB48;
            else if (isFloat && [theNsImgRep bitsPerPixel] == 96)
                return StImagePlane::ImgRGBF;
            else if (isFloat && [theNsImgRep bitsPerPixel] == 128)
                return StImagePlane::ImgRGBAF; // nit: we don't have RGBF format with unused A
        } else if ([theNsImgRep samplesPerPixel] == 4) {
            if ([theNsImgRep bitsPerPixel] == 32)
                return StImagePlane::ImgRGBA;
            else if (!isFloat && [theNsImgRep bitsPerPixel] == 64)
                return StImagePlane::ImgRGBA64;
            else if (isFloat && [theNsImgRep bitsPerPixel] == 64) // from EXR
                return StImagePlane::ImgUNKNOWN; // unsupported half-float format
            else if (isFloat && [theNsImgRep bitsPerPixel] == 128)
                return StImagePlane::ImgRGBAF;
        }

        return StImagePlane::ImgUNKNOWN;
    }

}

void StNsImage::close() {
    if (myNSBitmap != nullptr) {
        [myNSBitmap release];
        myNSBitmap = nullptr;
    }
}

bool StNsImage::loadExtra(const StString& theFilePath,
                          ImageType       theImageType,
                          uint8_t*        theDataPtr,
                          int             theDataSize,
                          bool            theIsOnlyRGB) {
    (void )theIsOnlyRGB;
    if (!StNsImage::init()) {
        setState("StNsImage library is not initialized");
        return false;
    }

    // reset current data
    StImage::nullify();
    setState();
    close();

    StCocoaLocalPool aLocalPool;

    NSImage* anNsImage = nullptr;
    NSData*  anNsData  = nullptr;
    if (theDataPtr != nullptr) {
        (void )theImageType;
        anNsData = [[[NSData alloc] initWithBytesNoCopy: theDataPtr
                                                 length: theDataSize
                                           freeWhenDone: NO]  autorelease];
        anNsImage = [[[NSImage alloc] initWithData: anNsData] autorelease];
    } else {
        StCocoaString aFilePath(theFilePath);
        anNsImage = [[[NSImage alloc] initWithContentsOfFile: aFilePath.toStringNs()] autorelease];
    }

    if (anNsImage == nullptr) {
        setState("Unable to read StNsImage");
        return false;
    }

    NSBitmapImageRep* anNsImgRep = (NSBitmapImageRep*)[[anNsImage representations] objectAtIndex: 0];
    if (![anNsImgRep isKindOfClass: [NSBitmapImageRep class]]) {
        setState("Unsupported StNsImage representation");
        return false;
    }

    const auto formatImgRep = [](NSBitmapImageRep* theRep) -> std::string
    {
        std::stringstream aStr;
        aStr << "NSBitmapImageRep"
             << " samplesPerPixel: " << [theRep samplesPerPixel]
             << "; bitsPerPixel: "   << [theRep bitsPerPixel]
             << "; size: "           << [theRep pixelsWide] << "x" << [theRep pixelsHigh]
             << "; bytesPerRow: "    << [theRep bytesPerRow]
             << "; isPlanar: "       << [theRep isPlanar]
             << "; bitmapFormat: "   << [theRep bitmapFormat]
             << "; colorSpace: "     << [[[theRep colorSpace] localizedName] UTF8String];
        return aStr.str();
    };

    const StImagePlane::ImgFormat anImgFormat = convertFromNSFormat(anNsImgRep);
    if (anImgFormat == StImagePlane::ImgUNKNOWN) {
        std::string aFormatStr = formatImgRep(anNsImgRep);
        setState(StString("Unsupported ") + aFormatStr.c_str());
        return false;
    }

    setColorModelPacked(anImgFormat);
    if (!changePlane(0).initWrapper(anImgFormat,
                                    [anNsImgRep bitmapData],
                                    [anNsImgRep pixelsWide], [anNsImgRep pixelsHigh],
                                    [anNsImgRep bytesPerRow])) {
        setState("Unable to wrap NSImage bitmap");
        return false;
    }
    changePlane(0).setTopDown(true);

    myNSBitmap = [anNsImgRep retain];

    // set debug information
    StString aDummy, aFileName;
    StFileNode::getFolderAndFile(theFilePath, aDummy, aFileName);
    setState(StString("StNsImage library, loaded image '") + aFileName + "' " + getDescription());
    return true;
}

#endif
