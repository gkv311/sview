/**
 * Copyright © 2012 Kirill Gavrilov <kirill@sview.ru>
 *
 * StTests program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * StTests program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program.
 * If not, see <http://www.gnu.org/licenses/>.
 */

#if(defined(__APPLE__))

#include "StTestEmbed.h"

#include <StCocoa/StCocoaLocalPool.h>
#include <StCore/StApplication.h>
#include <StCore/StCore.h>
#include <StCore/StWindow.h>

#include <StStrings/stConsole.h>
#include <StTemplates/StHandle.h>

#import <Cocoa/Cocoa.h>

bool StTestEmbed::createNative() {
    stMemSet(&myParent, 0, sizeof(StNativeWin_t));
    ///NSAutoreleasePool* aPool = [[NSAutoreleasePool alloc] init];
    NSRect aRect = NSMakeRect(128, 128, 400, 400);
    NSWindow* aWin = [NSWindow alloc];
    [aWin initWithContentRect: aRect
                    styleMask: NSTitledWindowMask | NSClosableWindowMask | NSResizableWindowMask
                      backing: NSBackingStoreBuffered
                        defer: NO];
    [aWin setTitle: @"DummyWindow"];
    [aWin makeKeyAndOrderFront: NSApp];
    ///[aWin release];
    myParent = aWin;
    return true;
}

void StTestEmbed::nativeLoop() {
    [NSApp run];
}

#endif // __APPLE__
