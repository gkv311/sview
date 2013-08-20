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

        return self;
    }

    - (void ) close {
        StEvent anEvent;
        anEvent.Type = stEvent_Close;
        anEvent.Close.Time = myStWin->getEventTime();
        if(myStWin->myEventsThreaded) {
            myStWin->myEventsBuffer.append(anEvent);
        } else {
            myStWin->signals.onClose->emit(anEvent.Close);
        }
    }

    - (void ) forceClose {
        [super close];
    }

    /**
     * Lost focus event.
     */
    - (void ) resignKeyWindow {
        // reset any pressed keys
        myStWin->myKeysState.reset();
        [super resignKeyWindow];

        // release topmost state
        [self setLevel: NSNormalWindowLevel];
    }

    - (void ) becomeKeyWindow {
        [super becomeKeyWindow];
        if(myStWin->isFullScreen()) {
            // restore topmost state
            [self setLevel: NSPopUpMenuWindowLevel];
        }
    }

@end

#endif // __APPLE__
