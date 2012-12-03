/**
 * Copyright © 2011 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
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
