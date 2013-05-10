/**
 * Copyright © 2011-2013 Kirill Gavrilov <kirill@sview.ru>
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

#include "StWindowImpl.h"
#include "stvkeysxarray.h" // X keys to VKEYs lookup array

#include <StStrings/StLogger.h>

#include "StCocoaView.h"
#include "StCocoaWin.h"

#include <StCocoa/StCocoaLocalPool.h>
#include <StCocoa/StCocoaString.h>

static const NSOpenGLPixelFormatAttribute THE_DOUBLE_BUFF[] = {
    //NSOpenGLPFAColorSize, (NSOpenGLPixelFormatAttribute) 32,
    //NSOpenGLPFADepthSize, (NSOpenGLPixelFormatAttribute) 0,
    //NSOpenGLPFAStencilSize, 0,
    NSOpenGLPFADoubleBuffer,
    NSOpenGLPFAAccelerated,
    0
};

static const NSOpenGLPixelFormatAttribute THE_QUAD_BUFF[] = {
    NSOpenGLPFAStereo, 1,
    NSOpenGLPFADoubleBuffer,
    NSOpenGLPFAAccelerated,
    0
};

void StWindowImpl::setTitle(const StString& theTitle) {
    myWindowTitle = theTitle;
    if(myMaster.hWindow != NULL) {
        StCocoaLocalPool aLocalPool;
        StCocoaString aTitle(myWindowTitle);
        [myMaster.hWindow setTitle: aTitle.toStringNs()];
    }
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
    myMessageList.reset();
    myKeysState.reset();
    myInitState = STWIN_INITNOTSTART;
    updateChildRect();

    if(NSApp == NULL) {
        stError("Cocoa, main application doesn't exists");
        myInitState = STWIN_ERROR_COCOA_NO_APP;
        return false;
    }

    StCocoaLocalPool aLocalPool;

    // create the Master GL context
    ///NSOpenGLPixelFormat* aGlFormatMaster = [[[NSOpenGLView class] defaultPixelFormat] retain];
    NSOpenGLPixelFormat* aGLFormat    = NULL;
    NSOpenGLContext* aGLContextMaster = NULL;
    NSOpenGLContext* aGLContextSlave  = NULL;
    if(attribs.IsGlStereo) {
        aGLFormat = [[[NSOpenGLPixelFormat alloc] initWithAttributes: THE_QUAD_BUFF] autorelease];
        aGLContextMaster = [[[NSOpenGLContext alloc] initWithFormat: aGLFormat
                                                       shareContext: NULL] autorelease];
        if(aGLContextMaster == NULL) {
            aGLFormat = [[[NSOpenGLPixelFormat alloc] initWithAttributes: THE_DOUBLE_BUFF] autorelease];
            aGLContextMaster = [[[NSOpenGLContext alloc] initWithFormat: aGLFormat
                                                           shareContext: NULL] autorelease];
            if(aGLContextMaster == NULL) {
                stError("Cocoa, fail to create Double Buffered OpenGL context");
                myInitState = STWIN_ERROR_COCOA_NO_GL;
                return false;
            } else {
                stError("Cocoa, fail to create Quad Buffered OpenGL context");
            }
        }
    } else {
        aGLFormat = [[[NSOpenGLPixelFormat alloc] initWithAttributes: THE_DOUBLE_BUFF] autorelease];
        aGLContextMaster = [[[NSOpenGLContext alloc] initWithFormat: aGLFormat
                                                       shareContext: NULL] autorelease];
        if(aGLContextMaster == NULL) {
            stError("Cocoa, fail to create Double Buffered OpenGL context");
            myInitState = STWIN_ERROR_COCOA_NO_GL;
            return false;
        }
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

    if(attribs.IsFullScreen) {
        const StMonitor& aMon = (myMonMasterFull == -1) ? myMonitors[myRectNorm.center()] : myMonitors[myMonMasterFull];
        myRectFull = aMon.getVRect();

        if(attribs.Slave != StWinSlave_slaveOff && mySlave.hViewGl != NULL) {
            [mySlave.hViewGl goToFullscreen];
        }
        if(myMaster.hViewGl != NULL) {
            [myMaster.hViewGl goToFullscreen];
        }
    } else {
        if(attribs.Slave != StWinSlave_slaveOff && mySlave.hViewGl != NULL) {
            [mySlave.hViewGl goToWindowed];
        }
        if(myMaster.hViewGl != NULL) {
            [myMaster.hViewGl goToWindowed];
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
            myMessageList.append(StMessageList::MSG_WIN_ON_NEW_MONITOR);
            myWinOnMonitorId = aNewMonId;
        }
    }
}

// Function set to argument-buffer given events
void StWindowImpl::processEvents(StMessage_t* theMessages) {
    if(myIsDispChanged) {
        updateMonitors();
    }

    if(myMaster.hViewGl == NULL) {
        // window is closed!
        myMessageList.popList(theMessages);
        return;
    }

    // detect master window movements
    if(attribs.IsFullScreen) {
        if(myRectNormPrev != myRectFull) {
            myRectNormPrev = myRectFull;
            myIsUpdated    = true;

            myStEvent.Type       = stEvent_Size;
            myStEvent.Size.Time  = getEventTime();
            myStEvent.Size.SizeX = myRectFull.width();
            myStEvent.Size.SizeY = myRectFull.height();
            signals.onResize->emit(myStEvent.Size);
        }
    } else {
        StRectI_t aWinRectNew = myCocoaCoords.cocoaToNormal([myMaster.hWindow contentRectForFrameRect: [myMaster.hWindow frame]]);
        if(myRectNormPrev != aWinRectNew) {
            myRectNorm     = aWinRectNew;
            myRectNormPrev = aWinRectNew;
            myIsUpdated    = true;

            myStEvent.Type       = stEvent_Size;
            myStEvent.Size.Time  = getEventTime();
            myStEvent.Size.SizeX = myRectNorm.width();
            myStEvent.Size.SizeY = myRectNorm.height();
            signals.onResize->emit(myStEvent.Size);
        }
    }

    // detect mouse movements
    StPointD_t aNewMousePt = getMousePos();
    if(aNewMousePt.x() >= 0.0 && aNewMousePt.x() <= 1.0 && aNewMousePt.y() >= 0.0 && aNewMousePt.y() <= 1.0) {
        StPointD_t aDspl = aNewMousePt - myMousePt;
        if(std::abs(aDspl.x()) >= 0.0008 || std::abs(aDspl.y()) >= 0.0008) {
            myMessageList.append(StMessageList::MSG_MOUSE_MOVE);
        }
    }
    myMousePt = aNewMousePt;

    if(myIsUpdated) {
        // update position only when all messages are parsed
        updateWindowPos();
        myIsUpdated = false;
    }
    updateActiveState();

    myMessageList.popList(theMessages);
    swapEventsBuffers();
}

#endif // __APPLE__
