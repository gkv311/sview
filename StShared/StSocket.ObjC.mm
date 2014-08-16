/**
 * Copyright © 2011 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#if (defined(__APPLE__))

#include <StThreads/StProcess.h>
#include <StCocoa/StCocoaLocalPool.h>

#import <Cocoa/Cocoa.h>

void StProcess::openURL(const StString& theUrl) {
    StCocoaLocalPool aLocalPool;
    NSWorkspace* aWorkSpace = [NSWorkspace sharedWorkspace];
    if(aWorkSpace == NULL) {
        return;
    }
    NSString* aPath = [[NSString alloc] initWithUTF8String: theUrl.toCString()];
    if(![aWorkSpace openURL: [NSURL URLWithString: aPath]]) {
        [aWorkSpace openFile: aPath];
    }

    [aPath release];
}

#endif // __APPLE__
