/**
 * StCore, window system independent C++ toolkit for writing OpenGL applications.
 * Copyright © 2011-2017 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#if (defined(__APPLE__))

#include "StWindowImpl.h"
#include "stvkeysxarray.h" // X keys to VKEYs lookup array

#include <StStrings/StLogger.h>
#include <StGL/StGLContext.h>

#include "StCocoaView.h"
#include "StCocoaWin.h"

#include <StCocoa/StCocoaLocalPool.h>
#include <StCocoa/StCocoaString.h>

#include <OpenGL/CGLRenderers.h>

#if !defined(MAC_OS_X_VERSION_10_7) || (MAC_OS_X_VERSION_MAX_ALLOWED < MAC_OS_X_VERSION_10_7)
@interface NSView (LionAPI)
    - (NSSize )convertSizeToBacking: (NSSize )theSize;
@end
#endif

void StWindowImpl::setTitle(const StString& theTitle) {
    myWindowTitle = theTitle;
    if(myMaster.hViewGl == NULL) {
        return;
    }

    if([NSThread isMainThread]) {
        [myMaster.hViewGl updateTitle: NULL];
    } else {
        [myMaster.hViewGl performSelectorOnMainThread: @selector(updateTitle:)
                                           withObject: NULL
                                        waitUntilDone: YES];
    }
}

void StWindowImpl::showCursor(bool toShow) {
    if(attribs.ToHideCursor != toShow) {
        return; // nothing to update
    }

    myMaster.hViewGl->myToHideCursor = !toShow;

    if([NSThread isMainThread]) {
        [myMaster.hWindow doResetCursors: NULL];
    } else {
        [myMaster.hWindow performSelectorOnMainThread: @selector(doResetCursors:)
                                           withObject: NULL
                                        waitUntilDone: YES];
    }

    attribs.ToHideCursor = !toShow;
}

void StWindowImpl::convertRectToBacking(StGLBoxPx& theRect,
                                        const int  theWinId) const {
    NSView* aView = (theWinId == ST_WIN_SLAVE) ? mySlave.hViewGl : myMaster.hViewGl;
    if(aView == NULL
    || ![aView respondsToSelector: @selector(convertSizeToBacking:)]) {
        return;
    }

    //[aView convertRectToBacking:(NSRect)aRect];
    NSSize aTopLeft     = { (float )theRect.x(),     (float )theRect.y() };
    NSSize aWidthHeight = { (float )theRect.width(), (float )theRect.height() };
    NSSize aRes = [aView convertSizeToBacking: aTopLeft];
    theRect.x() = (int )aRes.width;
    theRect.y() = (int )aRes.height;
    aRes = [aView convertSizeToBacking: aWidthHeight];
    theRect.width()  = (int )aRes.width;
    theRect.height() = (int )aRes.height;
}

void StWindowImpl::setPlacement(const StRectI_t& theRect,
                                const bool       theMoveToScreen) {
    if(theMoveToScreen) {
        const StPointI_t aCenter = theRect.center();
        const StMonitor& aMon = myMonitors[aCenter];
        if(!aMon.getVRect().isPointIn(aCenter)) {
            ST_DEBUG_LOG("Warning, window position is out of the monitor(" + aMon.getId() + ")!" + theRect.toString());
            const int aWidth  = theRect.width();
            const int aHeight = theRect.height();
            StRectI_t aRect;
            aRect.left()   = aMon.getVRect().left() + 256;
            aRect.right()  = aRect.left() + aWidth;
            aRect.top()    = aMon.getVRect().top() + 256;
            aRect.bottom() = aRect.top() + aHeight;
            myRectNorm = aRect;
        } else {
            myRectNorm = theRect;
        }
    } else {
        myRectNorm = theRect;
    }

    myIsUpdated = true;

    if(myMaster.hWindow != NULL
    && !attribs.IsFullScreen) {
        StCocoaLocalPool aLocalPool;
        NSRect aFrameRect = [myMaster.hWindow frameRectForContentRect: myCocoaCoords.normalToCocoa(myRectNorm)];
        [myMaster.hWindow setFrame: aFrameRect display: YES];
    }
}

void StWindowImpl::show(const int theWinNum) {
    if((theWinNum == ST_WIN_MASTER || theWinNum == ST_WIN_ALL)
    && attribs.IsHidden) {
        if(myMaster.hWindow != NULL) {
            [myMaster.hWindow orderFront: NULL];
        }
        attribs.IsHidden = false;
        updateWindowPos();
    }
    if((theWinNum == ST_WIN_SLAVE || theWinNum == ST_WIN_ALL)
    && attribs.IsSlaveHidden) {
        if(mySlave.hWindow != NULL) {
            [mySlave.hWindow orderFront: NULL];
        }
        attribs.IsSlaveHidden = false;
        updateWindowPos();
    }
}

void StWindowImpl::hide(const int theWinNum) {
    StCocoaLocalPool aLocalPool;
    if((theWinNum == ST_WIN_MASTER || theWinNum == ST_WIN_ALL)
    && !attribs.IsHidden) {
        if(myMaster.hWindow != NULL) {
            [myMaster.hWindow orderOut: NULL];
        }
        attribs.IsHidden = true;
    }
    if((theWinNum == ST_WIN_SLAVE || theWinNum == ST_WIN_ALL)
    && !attribs.IsSlaveHidden) {
        if(mySlave.hWindow != NULL) {
            [mySlave.hWindow orderOut: NULL];
        }
        attribs.IsSlaveHidden = true;
    }
}

@interface StWinInitInfo : NSObject
    {
        StWindowImpl*    myWinSt;
        NSOpenGLContext* myGLContextMaster;
        NSOpenGLContext* myGLContextSlave;
    }

    - (id ) init: (StWindowImpl* )    theWinSt
          master: (NSOpenGLContext* ) theGLContextMaster
           slave: (NSOpenGLContext* ) theGLContextSlave;

    - (void ) doCreateWindows: (id ) theSender;
@end

@implementation StWinInitInfo

    - (id ) init: (StWindowImpl* )    theWinSt
          master: (NSOpenGLContext* ) theGLContextMaster
           slave: (NSOpenGLContext* ) theGLContextSlave {
        self = [super init];
        if(self == NULL) {
            return NULL;
        }
        myWinSt = theWinSt;
        myGLContextMaster = theGLContextMaster;
        myGLContextSlave  = theGLContextSlave;
        return self;
    }

    - (void ) doCreateWindows: (id ) theSender {
        myWinSt->doCreateWindows(myGLContextMaster, myGLContextSlave);
    }

@end

void StWindowImpl::doCreateWindows(NSOpenGLContext* theGLContextMaster,
                                   NSOpenGLContext* theGLContextSlave) {
    StCocoaLocalPool aLocalPool;

    // create the Master window
    NSUInteger aWinStyle = attribs.IsNoDecor
                         ? NSBorderlessWindowMask
                         : NSTitledWindowMask | NSClosableWindowMask | NSResizableWindowMask;
    myMaster.hWindow = [[StCocoaWin alloc] initWithStWin: this
                                                    rect: myRectNorm
                                               styleMask: aWinStyle];
    if(myMaster.hWindow == NULL) {
        stError("Cocoa, Master Window Creation Error");
        myInitState = STWIN_ERROR_COCOA_CREATEWIN;
        return;
    }
    StCocoaString aTitle(myWindowTitle);
    [myMaster.hWindow setTitle: aTitle.toStringNs()];

    // create the view in Master window
    myMaster.hViewGl = [[StCocoaView alloc] initWithStWin: this
                                                    nsWin: myMaster.hWindow];
    if(myMaster.hViewGl == NULL) {
        myMaster.close();
        stError("Cocoa, Master GL View Creation Error");
        myInitState = STWIN_ERROR_COCOA_CREATEWIN;
        return;
    }
    // setup GL context (NSOpenGLView create context on first call
    // so nothing is released here)
    [myMaster.hViewGl setOpenGLContext: theGLContextMaster];
    [theGLContextMaster setView: myMaster.hViewGl];

    if(attribs.Slave != StWinSlave_slaveOff) {
        // create the Slave window
        StRectI_t aRectSlave(getSlaveTop(),  getSlaveTop() + getSlaveHeight(),
                             getSlaveLeft(), getSlaveLeft() + getSlaveWidth());
        mySlave.hWindow = [[StCocoaWin alloc] initWithStWin: this
                                                       rect: aRectSlave
                                                  styleMask: NSBorderlessWindowMask];
        if(mySlave.hWindow == NULL) {
            myMaster.close();
            mySlave.close();
            stError("Cocoa, Slave Window Creation Error");
            myInitState = STWIN_ERROR_COCOA_CREATEWIN;
            return;
        }
        [mySlave.hWindow setTitle: @"Slave window"];

        // create the view in Slave window
        mySlave.hViewGl = [[StCocoaView alloc] initWithStWin: this
                                                       nsWin: mySlave.hWindow];
        if(mySlave.hViewGl == NULL) {
            stError("Cocoa, Slave GL View Creation Error");
            myMaster.close();
            mySlave.close();
            myInitState = STWIN_ERROR_COCOA_CREATEWIN;
            return;
        }
        // setup GL context (NSOpenGLView create context on first call
        // so nothing is released here)
        [mySlave.hViewGl setOpenGLContext: theGLContextSlave];
        [theGLContextSlave setView: mySlave.hViewGl];

        // make slave window topmost
        [mySlave.hWindow setLevel: kCGMaximumWindowLevel];
        if(!attribs.IsSlaveHidden && (!isSlaveIndependent() || myMonitors.size() > 1)) {
            [mySlave.hWindow orderFront: NULL];
        }
    }

    [myMaster.hWindow makeKeyAndOrderFront: NSApp];
}

bool StWindowImpl::create() {
    myEventsThreaded = ![NSThread isMainThread];
    myKeysState.reset();
    myInitState = STWIN_INITNOTSTART;
    updateChildRect();

    if(NSApp == NULL) {
        stError("Cocoa, main application doesn't exists");
        myInitState = STWIN_ERROR_COCOA_NO_APP;
        return false;
    }

    StCocoaLocalPool aLocalPool;

    const bool isNoAccel = false;
ST_DISABLE_DEPRECATION_WARNINGS
    const NSOpenGLPixelFormatAttribute aDummyAttrib = NSOpenGLPFACompliant;
ST_ENABLE_DEPRECATION_WARNINGS
    NSOpenGLPixelFormatAttribute anAttribs[] = {
        attribs.IsGlStereo ? NSOpenGLPFAStereo : aDummyAttrib,
        //NSOpenGLPFAColorSize,   32,
        //NSOpenGLPFADepthSize,   0,
        //NSOpenGLPFAStencilSize, 0,
        NSOpenGLPFADoubleBuffer,
        isNoAccel ? NSOpenGLPFARendererID : NSOpenGLPFAAccelerated,
        isNoAccel ? kCGLRendererGenericFloatID : 0,
        0
    };

    // create the Master GL context
    ///NSOpenGLPixelFormat* aGlFormatMaster = [[[NSOpenGLView class] defaultPixelFormat] retain];
    NSOpenGLPixelFormat* aGLFormat    = NULL;
    NSOpenGLContext* aGLContextMaster = NULL;
    NSOpenGLContext* aGLContextSlave  = NULL;

    aGLFormat = [[[NSOpenGLPixelFormat alloc] initWithAttributes: anAttribs] autorelease];
    aGLContextMaster = [[[NSOpenGLContext alloc] initWithFormat: aGLFormat
                                                   shareContext: NULL] autorelease];
    if(aGLContextMaster == NULL
    && attribs.IsGlStereo) {
        ST_ERROR_LOG("Cocoa, fail to create Quad Buffered OpenGL context");
        anAttribs[0] = aDummyAttrib;
        aGLFormat = [[[NSOpenGLPixelFormat alloc] initWithAttributes: anAttribs] autorelease];
        aGLContextMaster = [[[NSOpenGLContext alloc] initWithFormat: aGLFormat
                                                       shareContext: NULL] autorelease];
    }
    if(aGLContextMaster == NULL) {
        stError("Cocoa, fail to create Double Buffered OpenGL context");
        myInitState = STWIN_ERROR_COCOA_NO_GL;
        return false;
    }

    // create the Slave GL context
    if(attribs.Slave != StWinSlave_slaveOff) {
        aGLContextSlave = [[[NSOpenGLContext alloc] initWithFormat: aGLFormat
                                                      shareContext: aGLContextMaster] autorelease];
        if(aGLContextSlave == NULL) {
            stError("Cocoa, fail to create Slave OpenGL context");
            myInitState = STWIN_ERROR_COCOA_NO_GL;
            return false;
        }
    }

    // create special object to invoke in main thread
    StWinInitInfo* aWinInit = [[StWinInitInfo alloc] init: this
                                                   master: aGLContextMaster
                                                    slave: aGLContextSlave];
    if(aWinInit == NULL) {
        myInitState = STWIN_ERROR_COCOA_CREATEWIN;
        return false;
    }

    if([NSThread isMainThread]) {
        [aWinInit doCreateWindows: NULL];
    } else {
        [aWinInit performSelectorOnMainThread: @selector(doCreateWindows:)
                                   withObject: NULL
                                waitUntilDone: YES];
    }
    [aWinInit release];

    if(myInitState != STWIN_INITNOTSTART) {
        return false;
    }

    myMaster.glMakeCurrent();
    myGlContext = new StGLContext(myResMgr);
    if(!myGlContext->stglInit()) {
        stError("Critical error - broken GL context!\nInvalid OpenGL driver?");
        myInitState = STWIN_ERROR_COCOA_NO_GL;
        return false;
    }

    myIsUpdated = true;
    myInitState = STWIN_INIT_SUCCESS;
    return true;
}

/**
 * Update StWindow position according to native parent position.
 */
void StWindowImpl::updateChildRect() {
    ///
}

void StWindowImpl::setFullScreen(bool theFullscreen) {
    if(attribs.IsFullScreen != theFullscreen) {
        attribs.IsFullScreen = theFullscreen;
        if(attribs.IsFullScreen) {
            myFullScreenWinNb.increment();
        } else {
            myFullScreenWinNb.decrement();
        }
    }

    if(attribs.IsHidden) {
        // do nothing, just set the flag
        // we should not post actions to the main thread within applicationWillTerminate() waiter
        return;
    }

    if(attribs.IsFullScreen) {
        const StMonitor& aMon = (myMonMasterFull == -1) ? myMonitors[myRectNorm.center()] : myMonitors[myMonMasterFull];
        myRectFull = aMon.getVRect();

        if(attribs.Slave != StWinSlave_slaveOff && mySlave.hViewGl != NULL) {
            if([NSThread isMainThread]) {
                [mySlave.hViewGl goToFullscreen: NULL];
            } else {
                [mySlave.hViewGl performSelectorOnMainThread: @selector(goToFullscreen:)
                                                  withObject: NULL
                                               waitUntilDone: YES];
            }
        } else if(attribs.Split == StWinSlave_splitHorizontal) {
            myTiledCfg = TiledCfg_MasterSlaveX;
            myRectFull.right() -= myRectFull.width() / 2;
        } else if(attribs.Split == StWinSlave_splitVertical) {
            myTiledCfg = TiledCfg_MasterSlaveY;
            myRectFull.bottom() -= myRectFull.height() / 2;
        }

        if(myMaster.hViewGl != NULL) {
            if([NSThread isMainThread]) {
                [myMaster.hViewGl goToFullscreen: NULL];
            } else {
                [myMaster.hViewGl performSelectorOnMainThread: @selector(goToFullscreen:)
                                                   withObject: NULL
                                                waitUntilDone: YES];
            }
        }
    } else {
        myTiledCfg = TiledCfg_Separate;
        if(attribs.Slave != StWinSlave_slaveOff && mySlave.hViewGl != NULL) {
            if([NSThread isMainThread]) {
                [mySlave.hViewGl goToWindowed: NULL];
            } else {
                [mySlave.hViewGl performSelectorOnMainThread: @selector(goToWindowed:)
                                                  withObject: NULL
                                               waitUntilDone: YES];
            }
        }
        if(myMaster.hViewGl != NULL) {
            if([NSThread isMainThread]) {
                [myMaster.hViewGl goToWindowed: NULL];
            } else {
                [myMaster.hViewGl performSelectorOnMainThread: @selector(goToWindowed:)
                                                   withObject: NULL
                                                waitUntilDone: YES];
            }
        }
    }
}

void StWindowImpl::updateWindowPos() {
    if(myMaster.hViewGl == NULL) {
        return;
    }

    if(attribs.Slave != StWinSlave_slaveOff && !attribs.IsSlaveHidden
    && (!isSlaveIndependent() || myMonitors.size() > 1)
    && !attribs.IsFullScreen) {
        StRectI_t aRectSlave(getSlaveTop(),  getSlaveTop() + getSlaveHeight(),
                             getSlaveLeft(), getSlaveLeft() + getSlaveWidth());
        NSRect aFrameRect = [mySlave.hWindow frameRectForContentRect: myCocoaCoords.normalToCocoa(aRectSlave)];
        [mySlave.hWindow setFrame: aFrameRect display: YES];
    }

    // detect when window moved to another monitor
    if(!attribs.IsFullScreen && myMonitors.size() > 1) {
        int aNewMonId = myMonitors[myRectNorm.center()].getId();
        if(myWinOnMonitorId != aNewMonId) {
            myStEventAux.Size.init(getEventTime(), myRectNorm.width(), myRectNorm.height(), myForcedAspect);
            myStEventAux.Type = stEvent_NewMonitor;
            myWinOnMonitorId = aNewMonId;
            signals.onAnotherMonitor->emit(myStEventAux.Size);
        }
    }
}

// Function set to argument-buffer given events
void StWindowImpl::processEvents() {
    if(myIsDispChanged) {
        updateMonitors();
    }

    if(myMaster.hViewGl == NULL) {
        // window is closed!
        return;
    }

    // detect master window movements
    if(attribs.IsFullScreen) {
        if(myRectNormPrev != myRectFull) {
            myRectNormPrev = myRectFull;
            myIsUpdated    = true;

            myStEvent.Size.init(getEventTime(), myRectFull.width(), myRectFull.height(), myForcedAspect);
            signals.onResize->emit(myStEvent.Size);
        }
    } else {
        StRectI_t aWinRectNew = myCocoaCoords.cocoaToNormal([myMaster.hWindow contentRectForFrameRect: [myMaster.hWindow frame]]);
        if(myRectNormPrev != aWinRectNew) {
            myRectNorm     = aWinRectNew;
            myRectNormPrev = aWinRectNew;
            myIsUpdated    = true;

            myStEvent.Size.init(getEventTime(), myRectNorm.width(), myRectNorm.height(), myForcedAspect);
            signals.onResize->emit(myStEvent.Size);
        }
    }

    // detect mouse movements
    StPointD_t aNewMousePt = getMousePos();
    myIsMouseMoved = false;
    if(aNewMousePt.x() >= 0.0 && aNewMousePt.x() <= 1.0 && aNewMousePt.y() >= 0.0 && aNewMousePt.y() <= 1.0) {
        StPointD_t aDspl = aNewMousePt - myMousePt;
        if(std::abs(aDspl.x()) >= 0.0008 || std::abs(aDspl.y()) >= 0.0008) {
            myIsMouseMoved = true;
        }
    }
    myMousePt = aNewMousePt;

    if(myIsUpdated) {
        // update position only when all messages are parsed
        updateWindowPos();
        myIsUpdated = false;
    }
    updateActiveState();

    swapEventsBuffers();
}

bool StWindowImpl::toClipboard(const StString& theText) {
    StCocoaLocalPool aLocalPool;
    NSPasteboard* aPasteBoard = [NSPasteboard generalPasteboard];
    [aPasteBoard clearContents];
    StCocoaString aStringMy(theText);
    NSArray* anObjects = [NSArray arrayWithObject: aStringMy.toStringNs()];
    return [aPasteBoard writeObjects: anObjects] == YES;
}

bool StWindowImpl::fromClipboard(StString& theText) {
    StCocoaLocalPool aLocalPool;
    NSPasteboard* aPasteBoard  = [NSPasteboard generalPasteboard];
    NSArray*      aClasses     = [[NSArray alloc] initWithObjects: [NSString class], NULL];
    NSDictionary* anOptions    = [NSDictionary dictionary];
    NSArray*      aCopiedItems = [aPasteBoard readObjectsForClasses: aClasses options: anOptions];
    if( aCopiedItems == NULL
    || [aCopiedItems count] < 1) {
        return false;
    }

    NSString* aTextNs = (NSString* )[aCopiedItems objectAtIndex: 0];
    if( aTextNs == NULL
    || [aTextNs isKindOfClass: [NSString class]] == NO) {
        return false;
    }

    theText = [[aTextNs precomposedStringWithCanonicalMapping] UTF8String];
    return true;
}

#endif // __APPLE__
