/**
 * Copyright © 2011-2012 Kirill Gavrilov <kirill@sview.ru>
 *
 * StCore library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * StCore library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.
 * If not, see <http://www.gnu.org/licenses/>.
 */

#if (defined(__APPLE__))

#include "StCocoaWin.h"
#include "StWindowImpl.h"

/**
 * Delegate for window.
 */
@interface StCocoaWinDelegate : NSObject <NSWindowDelegate>

    - (id ) init;

    + (StCocoaWinDelegate* ) sharedInstance;

@end

namespace {
    static StCocoaWinDelegate* TheWinDelegate = NULL;
};

@implementation StCocoaWinDelegate

    - (id ) init {
        if(TheWinDelegate != NULL) {
            if(self != TheWinDelegate) {
                // should never happens
                [self release];
            }
            return TheWinDelegate;
        }

        self = [super init];
        if(self == NULL) {
            return NULL;
        }

        return self;
    }

        /// @name singletone implementation

    + (StCocoaWinDelegate* ) sharedInstance {
        if(TheWinDelegate == NULL) {
            TheWinDelegate = [[super allocWithZone: NULL] init];
        }
        return TheWinDelegate;
    }

    + (id ) allocWithZone: (NSZone* ) theZone {
        return [[self sharedInstance] retain];

    }

    - (id ) copyWithZone: (NSZone* ) theZone {
        return self;
    }

    - (id ) retain {
        return self;
    }

    - (NSUInteger ) retainCount {
        return NSUIntegerMax;  //denotes an object that cannot be released
    }

    - (void ) release {
        //do nothing
    }

        /// @name NSWindowDelegate implementation

    //- (void ) windowDidBecomeKey: (NSNotification* ) theNotification {}

    - (void ) windowDidResignKey: (NSNotification* ) theNotification {
        StCocoaWin* aWin = [theNotification object];
        [aWin windowDidResignKey];
    }

@end

@implementation StCocoaWin

    - (id ) initWithStWin: (StWindowImpl* ) theStWin
                     rect: (StRectI_t )     theRect
                styleMask: (NSUInteger )    theWinStyle {
        NSRect aRectNs = theStWin->myCocoaCoords.normalToCocoa(theRect);
        self = [super initWithContentRect: aRectNs
                                styleMask: theWinStyle
                                  backing: NSBackingStoreBuffered
                                    defer: NO];
        if(self == NULL) {
            return NULL;
        }
        myStWin = theStWin;

        StCocoaWinDelegate* aWinDelegate = [StCocoaWinDelegate sharedInstance];
        [self setDelegate: aWinDelegate];

        return self;
    }

    - (void ) close {
        myStWin->myMessageList.append(StMessageList::MSG_CLOSE);
    }

    - (void ) forceClose {
        [super close];
    }

    - (void ) windowDidResignKey {
        // reset any pressed keys
        myStWin->myMessageList.resetKeysMap();
    }

@end

#endif // __APPLE__
