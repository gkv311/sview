/**
 * Copyright © 2011 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
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
