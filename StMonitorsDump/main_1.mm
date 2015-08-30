/**
 * Copyright © 2015 Kirill Gavrilov <kirill@sview.ru>
 *
 * StMonitorsDump program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * StMonitorsDump program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program.
 * If not, see <http://www.gnu.org/licenses/>.
 */

#if defined(__APPLE__)

#include <StThreads/StThread.h>

#import <Cocoa/Cocoa.h>

#include <StCore/StApplication.h>
#include <StCocoa/StCocoaLocalPool.h>
#include <StStrings/stConsole.h>
#include <StThreads/StProcess.h>

extern int stMonitorsDump_main();

/**
 * Main Cocoa application responder.
 */
@interface StTestResponder : NSObject <NSApplicationDelegate>
    {
    }

    + (StTestResponder* ) sharedInstance;

    /**
     * Default constructor.
     */
    - (id ) init;

    /**
     * Dummy method for thread-safety Cocoa initialization.
     */
    + (void ) doDummyThread: (id ) theParam;

@end

namespace {

    static StTestResponder* TheAppResponder = NULL;

}

@implementation StTestResponder

        /// Singletone implementation.

    - (id ) init {
        if(TheAppResponder != NULL) {
            if(self != TheAppResponder) {
                // should never happens
                [self release];
            }
            return TheAppResponder;
        }

        self = [super init];
        if(self == NULL) {
            return NULL;
        }
        return self;
    }

    + (StTestResponder* ) sharedInstance {
        if(TheAppResponder == NULL) {
            TheAppResponder = [[super allocWithZone: NULL] init];
        }
        return TheAppResponder;
    }

    + (id ) allocWithZone: (NSZone* ) theZone { return [[self sharedInstance] retain]; }
    - (id ) copyWithZone: (NSZone* ) theZone { return self; }
    - (id ) retain { return self; }
    - (NSUInteger ) retainCount { return NSUIntegerMax; } //denotes an object that cannot be released
    - (oneway void ) release {}
    - (id ) autorelease { return self; }

        /// application logic

    - (BOOL ) application: (NSApplication* ) theApplication
                 openFile: (NSString* ) theFilename {
        return YES;
    }

    - (void ) applicationDidFinishLaunching: (NSNotification* ) theNotification {
        int aResult = stMonitorsDump_main();
        exit(aResult);
    }

    - (void ) applicationWillTerminate: (NSNotification* ) theNotification {}
    + (void ) doDummyThread: (id ) theParam {}

@end

int main(int , char** ) {
    StCocoaLocalPool aPool;

    // dummy NSThread to ensure Cocoa thread-safety
    [NSThread detachNewThreadSelector: @selector(doDummyThread: )
                             toTarget: [StTestResponder class]
                           withObject: NULL];

    NSApplication* anAppNs = [NSApplication sharedApplication];
    StTestResponder* anAppResp = [StTestResponder sharedInstance];
    [anAppNs setDelegate: anAppResp];

    // dummy hidden window for as workaround
    NSWindow* aWin = [NSWindow alloc];
    [aWin initWithContentRect: NSMakeRect(16, 16, 16, 16)
                    styleMask: NSBorderlessWindowMask
                      backing: NSBackingStoreBuffered
                        defer: NO];
    [aWin release];

    [anAppNs activateIgnoringOtherApps: YES];
    [anAppNs run];
    return 0;
}

#endif // __APPLE__
