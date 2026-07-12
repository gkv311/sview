/**
 * Copyright © 2011 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#if (defined(__APPLE__))

#include <StCocoa/StCocoaCoords.h>
#include <StCocoa/StCocoaLocalPool.h>

#import <Cocoa/Cocoa.h>

StCocoaCoords::StCocoaCoords() {
    init();
}

bool StCocoaCoords::init() {
    if (NSApp == nullptr) {
        return false;
    }

    StCocoaLocalPool aLocalPool;
    NSArray* aScreens = [NSScreen screens];
    if (aScreens == nullptr || [aScreens count] == 0) {
        return false;
    }

    NSScreen* aScreen = (NSScreen* )[aScreens objectAtIndex: 0];

    const NSRect aRect = [aScreen frame];
    myScreenBottom = aRect.origin.y + aRect.size.height;
    return true;
}

CGRect StCocoaCoords::normalToCocoa(const StRectI_t& theRectSt) const {
    return NSMakeRect(CGFloat(theRectSt.left()),
                      myScreenBottom - CGFloat(theRectSt.bottom()),
                      CGFloat(theRectSt.width()),
                      CGFloat(theRectSt.height()));
}

StRectI_t StCocoaCoords::cocoaToNormal(const CGRect& theRectNs) const {
    const CGFloat aBottom = myScreenBottom - theRectNs.origin.y;
    return StRectI_t(aBottom - theRectNs.size.height,
                     aBottom,
                     theRectNs.origin.x,
                     theRectNs.origin.x + theRectNs.size.width);
}

#endif // __APPLE__
