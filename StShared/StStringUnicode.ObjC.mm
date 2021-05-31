/**
 * Copyright © 2012 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#if (defined(__APPLE__))

#include <StStrings/StString.h>
#include <StCocoa/StCocoaLocalPool.h>

#import <Cocoa/Cocoa.h>

StString stFromUtf8Mac(const char* theString) {
    StCocoaLocalPool aLocalPool;
    NSString* aStrNs     = [NSString stringWithUTF8String: theString];
    NSString* aStrCompNs = [aStrNs precomposedStringWithCanonicalMapping];
    return StString([aStrCompNs UTF8String]);
}

StString stToUtf8Mac(const char* theString) {
    StCocoaLocalPool aLocalPool;
    NSString* aStrNs     = [NSString stringWithUTF8String: theString];
    NSString* aStrCompNs = [aStrNs decomposedStringWithCanonicalMapping];
    return StString([aStrCompNs UTF8String]);
}

#endif // __APPLE__
