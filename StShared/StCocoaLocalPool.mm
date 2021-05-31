/**
 * Copyright © 2011 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#if (defined(__APPLE__))

#include <StCocoa/StCocoaLocalPool.h>

#import <Cocoa/Cocoa.h>

StCocoaLocalPool::StCocoaLocalPool()
: myPoolObj([[NSAutoreleasePool alloc] init]) {
    //
}

StCocoaLocalPool::~StCocoaLocalPool() {
    NSAutoreleasePool* aPool = (NSAutoreleasePool* )myPoolObj;
    //[aPool drain];
    [aPool release];
}

#endif // __APPLE__
