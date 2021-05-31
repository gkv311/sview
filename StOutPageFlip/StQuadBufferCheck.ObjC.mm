/**
 * StOutPageFlip, class providing stereoscopic output for Shutter Glasses displays using StCore toolkit.
 * Copyright © 2011 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#if (defined(__APPLE__))

#include "StQuadBufferCheck.h"

#include <StCocoa/StCocoaLocalPool.h>

#import <Cocoa/Cocoa.h>

bool StQuadBufferCheck::testQuadBufferSupport() {
    StCocoaLocalPool aLocalPool;

    const NSOpenGLPixelFormatAttribute THE_QUAD_BUFF[] = {
        NSOpenGLPFAStereo, 1,
        NSOpenGLPFADoubleBuffer,
        NSOpenGLPFAAccelerated,
        0
    };

    // create the Master GL context
    NSOpenGLPixelFormat* aGLFormat = [[[NSOpenGLPixelFormat alloc] initWithAttributes: THE_QUAD_BUFF] autorelease];
    NSOpenGLContext* aGLContext = [[[NSOpenGLContext alloc] initWithFormat: aGLFormat
                                                              shareContext: NULL] autorelease];
    return aGLContext != NULL;
}

#endif // __APPLE__
