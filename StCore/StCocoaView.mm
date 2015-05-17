/**
 * Copyright © 2011-2015 Kirill Gavrilov <kirill@sview.ru>
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

#include "StCocoaView.h"
#include "StWindowImpl.h"
#include "stvkeyscarbon.h"

#include <StStrings/StLogger.h>
#include <StTemplates/StRect.h>

#if !defined(MAC_OS_X_VERSION_10_7) || (MAC_OS_X_VERSION_MAX_ALLOWED < MAC_OS_X_VERSION_10_7)
@interface NSOpenGLView (LionAPI)
    - (void )setWantsBestResolutionOpenGLSurface: (BOOL )theFlag;
@end
#endif

@implementation StCocoaView

    - (id ) initWithStWin: (StWindowImpl* ) theStWin
                    nsWin: (NSWindow* )     theNsWin {
        NSRect aBounds = [[theNsWin contentView] bounds];
        self = [super initWithFrame: aBounds
                        pixelFormat: [[NSOpenGLView class] defaultPixelFormat]];
        if(self == NULL) {
            return NULL;
        }
        myStWin        = theStWin;
        myWinStyle     = 0;
        myIsFullscreen = false;

        // enable HiDPI mode
        if([self respondsToSelector: @selector(setWantsBestResolutionOpenGLSurface:)]) {
            [self setWantsBestResolutionOpenGLSurface: YES];
        }

        // create blank cursor
        NSImage* aBlackImg = [[NSImage alloc] initWithSize: NSMakeSize(16, 16)];
        myBlankCursor = [[NSCursor alloc] initWithImage: aBlackImg hotSpot: NSZeroPoint];
        [aBlackImg release];
        myToHideCursor = false;

        // setup fullscreen options
        NSString* aKeys[] = {
            NSFullScreenModeSetting,
            NSFullScreenModeWindowLevel // we override window level here, needed for slave window
        };
        NSNumber* aValues[] = {
            [NSNumber numberWithBool:    YES],
            [NSNumber numberWithInteger: kCGMaximumWindowLevel]
        };
        myFullScrOpts = [[NSDictionary alloc] initWithObjects: aValues
                                                      forKeys: aKeys
                                                        count: 2];

        // register Drag & Drop supports
        NSArray* aDndTypes = [NSArray arrayWithObjects: NSFilenamesPboardType, NULL];
        [self registerForDraggedTypes: aDndTypes];

        // replace content view in the window
        [theNsWin setContentView: self];

        // make view as first responder in winow to capture all useful events
        [theNsWin makeFirstResponder: self];
        return self;
    }

    - (void ) dealloc {
        [myBlankCursor release];
        [myFullScrOpts release];
        [super dealloc];
    }

    /**
     * Left mouse button - down.
     */
    - (void ) mouseDown: (NSEvent* ) theEvent {
        const StPointD_t aPnt    = myStWin->getMousePos();
        myStEvent.Type           = stEvent_MouseDown;
        myStEvent.Button.Time    = [theEvent timestamp];
        myStEvent.Button.Button  = ST_MOUSE_LEFT;
        myStEvent.Button.Buttons = 0;
        myStEvent.Button.PointX  = aPnt.x();
        myStEvent.Button.PointY  = aPnt.y();
        if(myStWin->myEventsThreaded) {
            myStWin->myEventsBuffer.append(myStEvent);
        } else {
            myStWin->signals.onMouseDown->emit(myStEvent.Button);
        }
    }

    /**
     * Left mouse button - up.
     */
    - (void ) mouseUp: (NSEvent* ) theEvent {
        const StPointD_t aPnt    = myStWin->getMousePos();
        myStEvent.Type           = stEvent_MouseUp;
        myStEvent.Button.Time    = [theEvent timestamp];
        myStEvent.Button.Button  = ST_MOUSE_LEFT;
        myStEvent.Button.Buttons = 0;
        myStEvent.Button.PointX  = aPnt.x();
        myStEvent.Button.PointY  = aPnt.y();
        if(myStWin->myEventsThreaded) {
            myStWin->myEventsBuffer.append(myStEvent);
        } else {
            myStWin->signals.onMouseUp->emit(myStEvent.Button);
        }
    }

    /**
     * Right mouse button - down.
     */
    - (void ) rightMouseDown: (NSEvent* ) theEvent {
        const StPointD_t aPnt    = myStWin->getMousePos();
        myStEvent.Type           = stEvent_MouseDown;
        myStEvent.Button.Time    = [theEvent timestamp];
        myStEvent.Button.Button  = ST_MOUSE_RIGHT;
        myStEvent.Button.Buttons = 0;
        myStEvent.Button.PointX  = aPnt.x();
        myStEvent.Button.PointY  = aPnt.y();
        if(myStWin->myEventsThreaded) {
            myStWin->myEventsBuffer.append(myStEvent);
        } else {
            myStWin->signals.onMouseDown->emit(myStEvent.Button);
        }
    }

    /**
     * Right mouse button - up.
     */
    - (void ) rightMouseUp: (NSEvent* ) theEvent {
        const StPointD_t aPnt    = myStWin->getMousePos();
        myStEvent.Type           = stEvent_MouseUp;
        myStEvent.Button.Time    = [theEvent timestamp];
        myStEvent.Button.Button  = ST_MOUSE_RIGHT;
        myStEvent.Button.Buttons = 0;
        myStEvent.Button.PointX  = aPnt.x();
        myStEvent.Button.PointY  = aPnt.y();
        if(myStWin->myEventsThreaded) {
            myStWin->myEventsBuffer.append(myStEvent);
        } else {
            myStWin->signals.onMouseUp->emit(myStEvent.Button);
        }
    }

    /**
     * Another (nor left nor right) mouse button - down.
     */
    - (void ) otherMouseDown: (NSEvent* ) theEvent {
        StVirtButton aBtnId = ST_NOMOUSE;
        if([theEvent buttonNumber] == 2) {
            aBtnId = ST_MOUSE_MIDDLE;
        }
        if(aBtnId != ST_NOMOUSE) {
            const StPointD_t aPnt    = myStWin->getMousePos();
            myStEvent.Type           = stEvent_MouseDown;
            myStEvent.Button.Time    = [theEvent timestamp];
            myStEvent.Button.Button  = aBtnId;
            myStEvent.Button.Buttons = 0;
            myStEvent.Button.PointX  = aPnt.x();
            myStEvent.Button.PointY  = aPnt.y();
            if(myStWin->myEventsThreaded) {
                myStWin->myEventsBuffer.append(myStEvent);
            } else {
                myStWin->signals.onMouseDown->emit(myStEvent.Button);
            }
        }
    }

    /**
     * Another (nor left nor right) mouse button - up.
     */
    - (void ) otherMouseUp: (NSEvent* ) theEvent {
        StVirtButton aBtnId = ST_NOMOUSE;
        if([theEvent buttonNumber] == 2) {
            aBtnId = ST_MOUSE_MIDDLE;
        }
        if(aBtnId != ST_NOMOUSE) {
            const StPointD_t aPnt    = myStWin->getMousePos();
            myStEvent.Type           = stEvent_MouseUp;
            myStEvent.Button.Time    = [theEvent timestamp];
            myStEvent.Button.Button  = aBtnId;
            myStEvent.Button.Buttons = 0;
            myStEvent.Button.PointX  = aPnt.x();
            myStEvent.Button.PointY  = aPnt.y();
            if(myStWin->myEventsThreaded) {
                myStWin->myEventsBuffer.append(myStEvent);
            } else {
                myStWin->signals.onMouseUp->emit(myStEvent.Button);
            }
        }
    }

    /**
     * Mouse scroll.
     */
    - (void ) scrollWheel: (NSEvent* ) theEvent {
        const CGFloat    aDeltaY = [theEvent deltaY];
        const CGFloat    aDeltaX = [theEvent deltaX];
        const StPointD_t aPnt    = myStWin->getMousePos();
        StVirtButton aBtnId = ST_NOMOUSE;
        if(stAreEqual(aDeltaX, 0.0, 0.01)) {
            if(stAreEqual(aDeltaY, 0.0, 0.01)) {
                // a lot of values near zero can be generated by touchpad
                return;
            }
            aBtnId = (aDeltaY > 0.0) ? ST_MOUSE_SCROLL_V_UP : ST_MOUSE_SCROLL_V_DOWN;
        } else {
            aBtnId = (aDeltaX > 0.0) ? ST_MOUSE_SCROLL_LEFT : ST_MOUSE_SCROLL_RIGHT;
        }

        //if([theEvent subtype] == NSMouseEventSubtype) {
        myStEvent.Type           = stEvent_MouseDown;
        myStEvent.Button.Time    = [theEvent timestamp];
        myStEvent.Button.Button  = aBtnId;
        myStEvent.Button.Buttons = 0;
        myStEvent.Button.PointX  = aPnt.x();
        myStEvent.Button.PointY  = aPnt.y();
        if(myStWin->myEventsThreaded) {
            myStWin->myEventsBuffer.append(myStEvent);
            myStEvent.Type = stEvent_MouseUp;
            myStWin->myEventsBuffer.append(myStEvent);
        } else {
            myStWin->signals.onMouseDown->emit(myStEvent.Button);
            myStEvent.Type = stEvent_MouseUp;
            myStWin->signals.onMouseUp  ->emit(myStEvent.Button);
        }
        //}
    }

    /**
     * 3-fingers swipe.
     */
    - (void ) swipeWithEvent: (NSEvent* ) theEvent {
        const CGFloat aDeltaX = [theEvent deltaX];
        const CGFloat aDeltaY = [theEvent deltaY];
        myStEvent.Type = stEvent_Navigate;
        myStEvent.Navigate.Time = [theEvent timestamp];
        if(stAreEqual(aDeltaX, 0.0, 0.001)) {
            if(!stAreEqual(aDeltaY, 0.0, 0.001)) {
                myStEvent.Navigate.Target = (aDeltaY > 0.0)
                                          ? stNavigate_Top
                                          : stNavigate_Bottom;
                if(myStWin->myEventsThreaded) {
                    myStWin->myEventsBuffer.append(myStEvent);
                } else {
                    myStWin->signals.onNavigate->emit(myStEvent.Navigate);
                }
            }
        } else {
            myStEvent.Navigate.Target = (aDeltaX > 0.0)
                                      ? stNavigate_Backward
                                      : stNavigate_Forward;
            if(myStWin->myEventsThreaded) {
                myStWin->myEventsBuffer.append(myStEvent);
            } else {
                myStWin->signals.onNavigate->emit(myStEvent.Navigate);
            }
        }
    }

    /**
     * Keyboard shortcuts event.
     */
    /**- (BOOL ) performKeyEquivalent: (NSEvent* ) theEvent {
        unsigned short aKeyCode = [theEvent keyCode];
        ST_DEBUG_LOG("performKeyEquivalent " + aKeyCode);
        if(aKeyCode >= ST_CARBON2ST_VK_SIZE) {
            ST_DEBUG_LOG("performKeyEquivalent, keycode= " + aKeyCode + " ignored!\n");
            return NO;
        }
        return NO;
    }*/

    /**
     * Modifier key pressed.
     */
    - (void ) flagsChanged: (NSEvent* ) theEvent {
        NSUInteger aFlags = [theEvent modifierFlags];
        if(aFlags & NSControlKeyMask) {
            if(!myStWin->myKeysState.isKeyDown(ST_VK_CONTROL)) {
                myStWin->myKeysState.keyDown(ST_VK_CONTROL, [theEvent timestamp]);
            }
        } else {
            if(myStWin->myKeysState.isKeyDown(ST_VK_CONTROL)) {
                myStWin->myKeysState.keyUp(ST_VK_CONTROL, [theEvent timestamp]);
            }
        }
        if(aFlags & NSShiftKeyMask) {
            if(!myStWin->myKeysState.isKeyDown(ST_VK_SHIFT)) {
                myStWin->myKeysState.keyDown(ST_VK_SHIFT, [theEvent timestamp]);
            }
        } else {
            if(myStWin->myKeysState.isKeyDown(ST_VK_SHIFT)) {
                myStWin->myKeysState.keyUp(ST_VK_SHIFT, [theEvent timestamp]);
            }
        }
        if(aFlags & NSCommandKeyMask) {
            if(!myStWin->myKeysState.isKeyDown(ST_VK_COMMAND)) {
                myStWin->myKeysState.keyDown(ST_VK_COMMAND, [theEvent timestamp]);
            }
        } else {
            if(myStWin->myKeysState.isKeyDown(ST_VK_COMMAND)) {
                myStWin->myKeysState.keyUp(ST_VK_COMMAND, [theEvent timestamp]);
            }
        }
        if(aFlags & NSFunctionKeyMask) {
            if(!myStWin->myKeysState.isKeyDown(ST_VK_FUNCTION)) {
                myStWin->myKeysState.keyDown(ST_VK_FUNCTION, [theEvent timestamp]);
            }
        } else {
            if(myStWin->myKeysState.isKeyDown(ST_VK_FUNCTION)) {
                myStWin->myKeysState.keyUp(ST_VK_FUNCTION, [theEvent timestamp]);
            }
        }
        if(aFlags & NSAlternateKeyMask) {
            if(!myStWin->myKeysState.isKeyDown(ST_VK_MENU)) {
                myStWin->myKeysState.keyDown(ST_VK_MENU, [theEvent timestamp]);
            }
        } else {
            if(myStWin->myKeysState.isKeyDown(ST_VK_MENU)) {
                myStWin->myKeysState.keyUp(ST_VK_MENU, [theEvent timestamp]);
            }
        }
    }

    /**
     * Key down event.
     */
    - (void ) keyDown: (NSEvent* ) theEvent {
        unsigned short aKeyCode = [theEvent keyCode];
        if(aKeyCode >= ST_CARBON2ST_VK_SIZE) {
            ST_DEBUG_LOG("keyDown, keycode= " + aKeyCode + " ignored!\n");
            return;
        }

        NSUInteger aFlags = [theEvent modifierFlags];
        if(aFlags & NSCommandKeyMask) {
            // should be fixed by StNSApp
            //return; // ignore Command + key combinations - key up event doesn't called!
        }

        StUtf8Iter aUIter([[theEvent characters] UTF8String]);
        myStEvent.Key.Char  = *aUIter;
        myStEvent.Key.VKey  = (StVirtKey )ST_CARBON2ST_VK[aKeyCode];
        myStEvent.Key.Time  = [theEvent timestamp];
        /*myStEvent.Key.Flags = ST_VF_NONE;
        if(aFlags & NSShiftKeyMask) {
            myStEvent.Key.Flags = StVirtFlags(myStEvent.Key.Flags | ST_VF_SHIFT);
        }
        if(aFlags & NSControlKeyMask) {
            myStEvent.Key.Flags = StVirtFlags(myStEvent.Key.Flags | ST_VF_CONTROL);
        }*/

        myStWin->postKeyDown(myStEvent);
    }

    /**
     * Key up event.
     */
    - (void ) keyUp: (NSEvent* ) theEvent {
        unsigned short aKeyCode = [theEvent keyCode];
        if(aKeyCode >= ST_CARBON2ST_VK_SIZE) {
            ST_DEBUG_LOG("keyUp,   keycode= " + aKeyCode + " ignored!\n");
            return;
        }

        myStEvent.Key.VKey  = (StVirtKey )ST_CARBON2ST_VK[aKeyCode];
        myStEvent.Key.Time  = [theEvent timestamp];
        /*NSUInteger aFlags = [theEvent modifierFlags];
        myStEvent.Key.Flags = ST_VF_NONE;
        if(aFlags & NSShiftKeyMask) {
            myStEvent.Key.Flags = StVirtFlags(myStEvent.Key.Flags | ST_VF_SHIFT);
        }
        if(aFlags & NSControlKeyMask) {
            myStEvent.Key.Flags = StVirtFlags(myStEvent.Key.Flags | ST_VF_CONTROL);
        }*/

        myStWin->postKeyUp(myStEvent);
    }

    - (void ) goToFullscreen: (id ) theSender {
        if(myStWin->attribs.IsExclusiveFullScr) {
            if(![self isInFullScreenMode]) {
                [self enterFullScreenMode: [[self window] screen] withOptions: myFullScrOpts];
            }
            return;
        } else if(myIsFullscreen) {
            return;
        }

        const bool isSlave = (self != myStWin->myMaster.hViewGl);
        const StMonitor& aMon = (myStWin->myMonMasterFull == -1)
                              ? myStWin->myMonitors[myStWin->myRectNorm.center()]
                              : myStWin->myMonitors[myStWin->myMonMasterFull];
        StRectI_t aRect = aMon.getVRect();
        if(isSlave) {
            if(myStWin->attribs.Slave == StWinSlave_slaveOff
            || !myStWin->isSlaveIndependent()
            || myStWin->myMonitors.size() == 1) {
                return;
            }

            aRect.left()   = myStWin->getSlaveLeft();
            aRect.top()    = myStWin->getSlaveTop();
            aRect.right()  = aRect.left() + myStWin->myRectFull.width();
            aRect.bottom() = aRect.top()  + myStWin->myRectFull.height();
        }
        NSRect aFullRect = myStWin->myCocoaCoords.normalToCocoa(aRect);

        NSWindow* aWin = [self window];
        myRectWindowed = [aWin frame];
        myIsFullscreen = true;
        myWinStyle     = [aWin styleMask];
        [aWin setStyleMask: NSBorderlessWindowMask];
        [aWin setLevel: NSPopUpMenuWindowLevel];
        [aWin setFrame: aFullRect display: YES];
        [aWin makeFirstResponder: self];
    }

    - (void ) goToWindowed: (id ) theSender {
        if([self isInFullScreenMode]) {
            [self exitFullScreenModeWithOptions: myFullScrOpts];
            [[self window] makeFirstResponder: self];
            return;
        } else if(!myIsFullscreen) {
            return;
        }

        NSWindow* aWin = [self window];
        myIsFullscreen = false;
        [aWin setStyleMask: myWinStyle];
        [aWin setLevel: NSNormalWindowLevel];
        [aWin setFrame: myRectWindowed display: YES];
        [aWin makeFirstResponder: self];
    }

    - (NSDragOperation ) draggingEntered: (id <NSDraggingInfo> ) theSender {
        if((NSDragOperationGeneric & [theSender draggingSourceOperationMask]) == NSDragOperationGeneric) {
            return NSDragOperationGeneric;
        }
        // not a drag we can use
        return NSDragOperationNone;
    }

    - (void ) resetCursorRects {
        if(myToHideCursor) {
            [self addCursorRect: [self visibleRect] cursor: myBlankCursor];
        }
    }

    - (void ) draggingExited: (id <NSDraggingInfo> ) theSender {
        //
    }

    - (BOOL ) prepareForDragOperation: (id <NSDraggingInfo> ) theSender {
        return YES;
    }

    - (BOOL ) performDragOperation: (id <NSDraggingInfo> ) theSender {
        NSPasteboard* aPasteBoard = [theSender draggingPasteboard];
        if([[aPasteBoard types] containsObject: NSFilenamesPboardType]) {
            NSArray* aFiles = [aPasteBoard propertyListForType: NSFilenamesPboardType];
            int aFilesCount = [aFiles count];
            if(aFilesCount < 1) {
                return NO;
            }

            for(NSUInteger aFileId = 0; aFileId < [aFiles count]; ++aFileId) {
                NSString* aFilePathNs = (NSString* )[aFiles objectAtIndex: aFileId];
                if(aFilePathNs == NULL
                || [aFilePathNs isKindOfClass: [NSString class]] == NO) {
                    continue;
                }

                // automatically convert filenames from decomposed form used by Mac OS X file systems
                const StString aFile = [[aFilePathNs precomposedStringWithCanonicalMapping] UTF8String];
                myStEvent.Type = stEvent_FileDrop;
                myStEvent.DNDrop.Time = myStWin->getEventTime();
                myStEvent.DNDrop.File = aFile.toCString();
                if(myStWin->myEventsThreaded) {
                    myStWin->myEventsBuffer.append(myStEvent);
                } else {
                    myStWin->signals.onFileDrop->emit(myStEvent.DNDrop);
                }
            }
        }
        return YES;
    }

@end

#endif // __APPLE__
