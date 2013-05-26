/**
 * Copyright © 2011 Kirill Gavrilov <kirill@sview.ru>
 *
 * StOutPageFlip library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * StOutPageFlip library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.
 * If not, see <http://www.gnu.org/licenses/>.
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
