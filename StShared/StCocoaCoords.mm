/**
 * Copyright © 2011 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#if (defined(__APPLE__))

#include <StCocoa/StCocoaCoords.h>
#include <StCocoa/StCocoaLocalPool.h>

#import <Cocoa/Cocoa.h>

StCocoaCoords::StCocoaCoords()
: myScreenBottom(0.0f),
  myScale(1.0f),
  myUnScale(1.0f) {
    init();
}

bool StCocoaCoords::init() {
    if(NSApp == nil) {
        return false;
    }
    StCocoaLocalPool aLocalPool;
    NSArray* aScreens = [NSScreen screens];
    if(aScreens == NULL
    || [aScreens count] == 0) {
        return false;
    }

    NSScreen* aScreen = (NSScreen* )[aScreens objectAtIndex: 0];
    NSDictionary* aDict = [aScreen deviceDescription];
    NSNumber* aNumber = [aDict objectForKey: @"NSScreenNumber"];
    if(aNumber == NULL
    || [aNumber isKindOfClass: [NSNumber class]] == NO) {
        return false;
    }

    CGDirectDisplayID aDispId = [aNumber unsignedIntValue];
    CGRect aRect = CGDisplayBounds(aDispId);
    myScreenBottom = aRect.origin.y + aRect.size.height;
ST_DISABLE_DEPRECATION_WARNINGS
    myScale        = [aScreen userSpaceScaleFactor];
ST_ENABLE_DEPRECATION_WARNINGS
    myUnScale      = 1.0 / myScale;
    return true;
}

CGRect StCocoaCoords::normalToCocoa(const StRectI_t& theRectSt) const {
    return NSMakeRect(CGFloat(theRectSt.left()),
                      myScreenBottom - CGFloat(theRectSt.bottom()),
                      CGFloat(theRectSt.width()) * myUnScale,
                      CGFloat(theRectSt.height()) * myUnScale);
}

StRectI_t StCocoaCoords::cocoaToNormal(const CGRect& theRectNs) const {
    return StRectI_t(myScreenBottom - theRectNs.origin.y - theRectNs.size.height * myScale,
                     myScreenBottom - theRectNs.origin.y,
                     theRectNs.origin.x,
                     theRectNs.origin.x + theRectNs.size.width * myScale);
}

#endif // __APPLE__
