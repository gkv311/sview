/**
 * Copyright © 2012-2015 Kirill Gavrilov <kirill@sview.ru>
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

#if (defined(__APPLE__))

#include "StTestResponder.h"

#include <StCore/StApplication.h>
#include <StCocoa/StCocoaLocalPool.h>
#include <StStrings/stConsole.h>
#include <StThreads/StProcess.h>

#include "StTestMutex.h"
#include "StTestGlBand.h"
#include "StTestEmbed.h"
#include "StTestImageLib.h"

namespace {

    static StTestResponder* TheAppResponder = NULL;

};

@implementation StTestResponder

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

    /**
     * Singletone implementation.
     */
    + (id ) allocWithZone: (NSZone* ) theZone {
        return [[self sharedInstance] retain];

    }

    /**
     * Singletone implementation.
     */
    - (id ) copyWithZone: (NSZone* ) theZone {
        return self;
    }

    /**
     * Singletone implementation.
     */
    - (id ) retain {
        return self;
    }

    /**
     * Singletone implementation.
     */
    - (NSUInteger ) retainCount {
        return NSUIntegerMax;  //denotes an object that cannot be released
    }

    /**
     * Singletone implementation.
     */
    - (oneway void ) release {
        //do nothing
    }

    /**
     * Singletone implementation.
     */
    - (id ) autorelease {
        return self;
    }

    /**
     * Cocoa file opening.
     */
    - (BOOL ) application: (NSApplication* ) theApplication
                 openFile: (NSString* ) theFilename {
        //StString aFilePath([theFilename UTF8String]);
        return YES;
    }

    - (void ) applicationDidFinishLaunching: (NSNotification* ) theNotification {
        st::cout << stostream_text("This application performs some synthetic tests\n");

        StArrayList<StString> anArgs = StProcess::getArguments();
        const StString ST_TEST_MUTICES = "mutex";
        const StString ST_TEST_GLBAND  = "glband";
        const StString ST_TEST_EMBED   = "embed";
        const StString ST_TEST_IMAGE   = "image";
        const StString ST_TEST_ALL     = "all";
        size_t aFound = 0;
        for(size_t anArgId = 0; anArgId < anArgs.size(); ++anArgId) {
            const StString& aParam = anArgs[anArgId];
            if(aParam == ST_TEST_MUTICES) {
                // mutex speed test
                StTestMutex aMutices;
                aMutices.perform();
                ++aFound;
            } else if(aParam == ST_TEST_GLBAND) {
                // gl <-> cpu trasfer speed test
                StTestGlBand aGlBand;
                aGlBand.perform();
                ++aFound;
            } else if(aParam == ST_TEST_EMBED) {
                // StWindow embed to native window
                StTestEmbed anEmbed;
                anEmbed.perform();
                ++aFound;
            } else if(aParam == ST_TEST_IMAGE) {
                // image libraries performance tests
                if(++anArgId >= anArgs.size()) {
                    st::cout << stostream_text("Broken syntax - image file awaited!\n");
                    break;
                }

                StTestImageLib anImage(anArgs[anArgId]);
                anImage.perform();
                ++aFound;
            } else if(aParam == ST_TEST_ALL) {
                // mutex speed test
                StTestMutex aMutices;
                aMutices.perform();

                // gl <-> cpu trasfer speed test
                StTestGlBand aGlBand;
                aGlBand.perform();

                // StWindow embed to native window
                StTestEmbed anEmbed;
                anEmbed.perform();

                ++aFound;
                break;
            }
        }

        // show help
        if(aFound == 0) {
            st::cout << stostream_text("No test selected. Options:\n")
                     << stostream_text("  all    - execute all available tests\n")
                     << stostream_text("  mutex  - mutex speed test\n")
                     << stostream_text("  glband - gl <-> cpu trasfer speed test\n")
                     << stostream_text("  embed  - test window embedding\n")
                     << stostream_text("  image fileName - test image libraries\n");
        }
    }

    - (void ) applicationWillTerminate: (NSNotification* ) theNotification {
        //StCore::FREE();
    }

    + (void ) doDummyThread: (id ) theParam {}

@end

int main(int , char** ) {
#ifdef ST_DEBUG
    // TODO (Kirill Gavrilov#9) debug environment
    #if (defined(_LP64) || defined(__LP64__))
        const StString ST_ENV_NAME_STCORE_PATH = "StCore64";
    #else
        const StString ST_ENV_NAME_STCORE_PATH = "StCore32";
    #endif
    StProcess::setEnv(ST_ENV_NAME_STCORE_PATH, StProcess::getProcessFolder());
#endif

    // autorelease pool is required for Cocoa
    StCocoaLocalPool aPool;

    // create dummy NSThread to ensure Cocoa thread-safety
    [NSThread detachNewThreadSelector: @selector(doDummyThread: )
                             toTarget: [StTestResponder class]
                           withObject: NULL];

    // initialize Cocoa application
    NSApplication* anAppNs = [NSApplication sharedApplication];
    StTestResponder* anAppResp = [StTestResponder sharedInstance];
    [anAppNs setDelegate: anAppResp];
    //[NSBundle loadNibNamed: @"MainMenu"
    //                 owner: anAppResp];

    // create dummy hidden window - workaround against strange bug
    // when right half of GL window title doesn't allow to be moved
    NSWindow* aWin = [NSWindow alloc];
    [aWin initWithContentRect: NSMakeRect(16, 16, 16, 16)
                    styleMask: NSBorderlessWindowMask
                      backing: NSBackingStoreBuffered
                        defer: NO];
    [aWin release];

    // allow our application to steal input focus (when needed)
    [anAppNs activateIgnoringOtherApps: YES];

    //StCore::INIT();

    // Cocoa event loop can be started ONLY in main thread
    [anAppNs run];
    // will never come here - we should use applicationWillTerminate if necessary
    return 0;
}

#endif // __APPLE__
