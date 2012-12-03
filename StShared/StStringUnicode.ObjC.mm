/**
 * Copyright © 2012 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
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
