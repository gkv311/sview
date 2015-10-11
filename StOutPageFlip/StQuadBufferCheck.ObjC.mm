/**
 * StOutPageFlip, class providing stereoscopic output for Shutter Glasses displays using StCore toolkit.
 * Copyright © 2011 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
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
