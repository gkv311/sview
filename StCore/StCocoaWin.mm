/**
 * StCore, window system independent C++ toolkit for writing OpenGL applications.
 * Copyright © 2011-2013 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#if (defined(__APPLE__))

#include "StCocoaWin.h"
#include "StWindowImpl.h"

@implementation StCocoaWin

    - (id ) initWithStWin: (StWindowImpl* ) theStWin
                     rect: (StRectI_t )     theRect
                   screen: (NSScreen* )     theScreen
                styleMask: (NSUInteger )    theWinStyle {

        NSRect aRectNs = theStWin->myCocoaCoords.normalToCocoa(theRect);
        if (theScreen != nullptr) {
            NSRect aScreenRect = [theScreen frame];
            aRectNs.origin.x  -= aScreenRect.origin.x;
            aRectNs.origin.y  -= aScreenRect.origin.y;
        }

        self = [super initWithContentRect: aRectNs
                                styleMask: theWinStyle
                                  backing: NSBackingStoreBuffered
                                    defer: NO
                                   screen: theScreen];
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

    - (void ) doResetCursors: (id ) theSender {
        [self resetCursorRects];

        [[self contentView] setNeedsDisplay: YES];
        [self disableCursorRects];
        [self enableCursorRects];
    }

@end

#endif // __APPLE__
