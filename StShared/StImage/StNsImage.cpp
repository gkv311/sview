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
#endif

bool StNsImage::init() {
#ifdef ST_HAVE_NSIMAGE
    return true;
#else
    return false;
#endif
}

StNsImage::StNsImage() {
    StNsImage::init();
#ifndef ST_HAVE_NSIMAGE
    (void)myNSBitmap;
#endif
}

StNsImage::~StNsImage() {
    close();
}

#ifndef ST_HAVE_NSIMAGE
void StNsImage::close() {
    //
}

bool StNsImage::loadExtra(const StString& , ImageType , uint8_t* , int , bool ) {
    return false;
}
#endif

bool StNsImage::save(const StString&, const SaveImageParams& ) {
    setState("StNsImage library, save operation is NOT implemented");
    return false;
}
