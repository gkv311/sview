/**
 * Copyright © 2007-2013 Kirill Gavrilov <kirill@sview.ru>
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

#if(defined(__linux__) || defined(__linux))

#include "StWindowImpl.h"
#include "stvkeysxarray.h" // X keys to VKEYs lookup array

#include <StStrings/StLogger.h>

#include <X11/extensions/Xrandr.h>
#include <X11/xpm.h>
#include <gdk/gdkx.h>
#include <cmath>

#include "../share/sView/icons/menu.xpm"

/**
 * Our own XError handle function.
 * Default handlers just show the error and exit application immediately.
 * This is very horrible for application, because many errors not critical and could be ignored.
 * The main target for this function creation - prevent sView to quit after
 * multiple BadMatch errors happens on fullscreen->windowed switching
 * (looking like a vendors OpenGL driver bug).
 * Thus, now we just show up the error description and just ignore it - hopes we will well.
 * Function behaviour could be extended in future.
 */
int stXErrorHandler(Display*     theDisplay,
                    XErrorEvent* theErrorEvent) {
    char aBuffer[4096];
    XGetErrorText(theDisplay, theErrorEvent->error_code, aBuffer, 4096);
    ST_DEBUG_LOG("XError happend: " + aBuffer + "; ignored");
    // have no idea WHAT we should return here....
    return 0;
}

Bool StWindowImpl::stXWaitMapped(Display* theDisplay,
                                 XEvent*  theEvent,
                                 char*    theArg) {
    return (theEvent->type        == MapNotify)
        && (theEvent->xmap.window == (Window )theArg);
}

namespace {

    static XSetWindowAttributes createDefaultAttribs(StXDisplayH theStDisplay) {
        XSetWindowAttributes aWinAttribsX;
        stMemSet(&aWinAttribsX, 0, sizeof(XSetWindowAttributes));

        // create an X colormap since probably not using default visual
        aWinAttribsX.colormap = XCreateColormap(theStDisplay->hDisplay,
                                                RootWindow(theStDisplay->hDisplay, theStDisplay->getScreen()),
                                                theStDisplay->getVisual(), AllocNone);
        aWinAttribsX.border_pixel = 0;

        // what events we want to recive:
        aWinAttribsX.event_mask =  KeyPressMask   | KeyReleaseMask    // receive keyboard events
                                | ButtonPressMask | ButtonReleaseMask // receive mouse events
                                | StructureNotifyMask;                // receive ConfigureNotify event on resize and move
                              //| ResizeRedirectMask                  // receive ResizeRequest event on resize (instead of common ConfigureNotify)
                              //| ExposureMask
                              //| EnterWindowMask|LeaveWindowMask
                              //| PointerMotionMask|PointerMotionHintMask|Button1MotionMask|Button2MotionMask|Button3MotionMask|Button4MotionMask|Button5MotionMask|ButtonMotionMask
                              //| KeymapStateMask|ExposureMask|VisibilityChangeMask
                              //| SubstructureNotifyMask|SubstructureRedirectMask
                              //| FocusChangeMask|PropertyChangeMask|ColormapChangeMask|OwnerGrabButtonMask
        aWinAttribsX.override_redirect = False;
        return aWinAttribsX;
    }

};

// function create GUI window
bool StWindowImpl::create() {
    myMessageList.reset();

    // initialize helper GDK
    static bool isGdkInitialized = false;
    if(!isGdkInitialized) {
        if(!gdk_init_check(NULL, NULL)) {
            stError("GDK, init failed");
            //myInitState =
            return false;
        }
        gdk_rgb_init(); // only guess sets up the true colour colour map
        isGdkInitialized = true;
    }

    // replace default XError handler to ignore some errors
    /// TODO (Kirill Gavrilov#1) - GTK+ (re)initialization seems to be override our error handler!
    XSetErrorHandler(stXErrorHandler);

    myInitState = STWIN_INITNOTSTART;
    // X-server implementation
    // create window on unix systems throw X-server
    int dummy;

    // open a connection to the X server
    StXDisplayH stXDisplay = new StXDisplay();
    if(!stXDisplay->isOpened()) {
        stXDisplay.nullify();
        stError("X, could not open display");
        myInitState = STWIN_ERROR_X_OPENDISPLAY;
        return false;
    }
    myMaster.stXDisplay = stXDisplay;
    Display* hDisplay = stXDisplay->hDisplay;

    // make sure OpenGL's GLX extension supported
    if(!glXQueryExtension(hDisplay, &dummy, &dummy)) {
        myMaster.close();
        stError("X, server has no OpenGL GLX extension");
        myInitState = STWIN_ERROR_X_NOGLX;
        return false;
    }

    int aDblBuff[] = {
        GLX_RGBA,
        GLX_DEPTH_SIZE, attribs.GlDepthSize,
        GLX_DOUBLEBUFFER,
        None
    };

    if(attribs.IsGlStereo) {
        // find an appropriate visual
        int aQuadBuff[] = {
            GLX_RGBA,
            GLX_DEPTH_SIZE, attribs.GlDepthSize,
            GLX_DOUBLEBUFFER,
            GLX_STEREO,
            None
        };

        stXDisplay->hVisInfo = glXChooseVisual(hDisplay, DefaultScreen(hDisplay), aQuadBuff);
        if(stXDisplay->hVisInfo == NULL) {
            stError("X, no Quad Buffered visual");



            stXDisplay->hVisInfo = glXChooseVisual(hDisplay, DefaultScreen(hDisplay), aDblBuff);
            if(stXDisplay->hVisInfo == NULL) {
                myMaster.close();
                stError("X, no RGB visual with depth buffer");
                myInitState = STWIN_ERROR_X_NORGB;
                return false;
            }
        }
    } else {
        // find an appropriate visual
        // find an OpenGL-capable RGB visual with depth buffer
        stXDisplay->hVisInfo = glXChooseVisual(hDisplay, DefaultScreen(hDisplay), aDblBuff);
        if(stXDisplay->hVisInfo == NULL) {
            myMaster.close();
            stError("X, no RGB visual with depth buffer");
            myInitState = STWIN_ERROR_X_NORGB;
            return false;
        }
    }
    if(attribs.Slave != StWinSlave_slaveOff) {
        // just copy handle
        mySlave.stXDisplay = stXDisplay;
    }

    // create an X window with the selected visual
    XSetWindowAttributes aWinAttribsX = createDefaultAttribs(stXDisplay);
    updateChildRect();

    Window aParentWin = (Window )myParentWin;
    if(aParentWin == 0 && !attribs.IsNoDecor) {
        aWinAttribsX.override_redirect = False;
        myMaster.hWindow = XCreateWindow(hDisplay, stXDisplay->getRootWindow(),
                                         myRectNorm.left(),  myRectNorm.top(),
                                         myRectNorm.width(), myRectNorm.height(),
                                         0, stXDisplay->getDepth(),
                                         InputOutput,
                                         stXDisplay->getVisual(),
                                         CWBorderPixel | CWColormap | CWEventMask | CWOverrideRedirect, &aWinAttribsX);

        if(myMaster.hWindow == 0) {
            myMaster.close();
            stError("X, XCreateWindow failed for Master");
            myInitState = STWIN_ERROR_X_CREATEWIN;
            return false;
        }
        aParentWin = myMaster.hWindow;

        XSetStandardProperties(hDisplay, myMaster.hWindow,
                               myWindowTitle.toCString(),
                               myWindowTitle.toCString(),
                               None, NULL, 0, NULL);
    }

    aWinAttribsX.override_redirect = True; // GL window always undecorated
    myMaster.hWindowGl = XCreateWindow(hDisplay, (aParentWin != 0) ? aParentWin : stXDisplay->getRootWindow(),
                                       0, 0, myRectNorm.width(), myRectNorm.height(),
                                       0, stXDisplay->getDepth(),
                                       InputOutput,
                                       stXDisplay->getVisual(),
                                       CWBorderPixel | CWColormap | CWEventMask | CWOverrideRedirect, &aWinAttribsX);
    if(myMaster.hWindowGl == 0) {
        myMaster.close();
        stError("X, XCreateWindow failed for Master");
        myInitState = STWIN_ERROR_X_CREATEWIN;
        return false;
    }

    XSetStandardProperties(hDisplay, myMaster.hWindowGl,
                           "master window", "master window",
                           None, NULL, 0, NULL);

    if(attribs.Slave != StWinSlave_slaveOff) {
        aWinAttribsX.event_mask = NoEventMask; // we do not parse any events to slave window!
        aWinAttribsX.override_redirect = True; // slave window always undecorated
        mySlave.hWindowGl = XCreateWindow(hDisplay, stXDisplay->getRootWindow(),
                                          getSlaveLeft(),  getSlaveTop(),
                                          getSlaveWidth(), getSlaveHeight(),
                                          0, stXDisplay->getDepth(),
                                          InputOutput,
                                          stXDisplay->getVisual(),
                                          CWBorderPixel | CWColormap | CWEventMask | CWOverrideRedirect, &aWinAttribsX);

        if(mySlave.hWindowGl == 0) {
            myMaster.close();
            mySlave.close();
            stError("X, XCreateWindow failed for Slave");
            myInitState = STWIN_ERROR_X_CREATEWIN;
            return false;
        }

        XSetStandardProperties(hDisplay, mySlave.hWindowGl,
                               "slave window", "slave window",
                               None, NULL, 0, NULL);
    }

    int isGlCtx = myMaster.glCreateContext(attribs.Slave != StWinSlave_slaveOff ? &mySlave : NULL,
                                           attribs.GlDepthSize,
                                           attribs.IsGlStereo);
    if(isGlCtx != STWIN_INIT_SUCCESS) {
        myMaster.close();
        mySlave.close();
        myInitState = isGlCtx;
        return false;
    }

    // handle close window event
    if(myMaster.hWindow != 0) {
        XSetWMProtocols(hDisplay, myMaster.hWindow, &(stXDisplay->wndDestroyAtom), 1);
    }

    // Announce XDND support
    myMaster.setupXDND();

    // Initialize XRandr events reception
    if(XRRQueryExtension(hDisplay, &myMaster.xrandrEventBase, &dummy)) {
        XRRSelectInput(hDisplay,
                       stXDisplay->getRootWindow(),
                       RRScreenChangeNotifyMask |
                       RRCrtcChangeNotifyMask   |
                       RROutputPropertyNotifyMask);
        myMaster.isRecXRandrEvents = true;
    } else {
        myMaster.isRecXRandrEvents = false;
    }

    // request the X window to be displayed on the screen
    if(attribs.Slave != StWinSlave_slaveOff) {

        // request the X window to be displayed on the screen
        if(!attribs.IsSlaveHidden && (!isSlaveIndependent() || myMonitors.size() > 1)) {
            XMapWindow(hDisplay, mySlave.hWindowGl);
            //XIfEvent(hDisplay, &myXEvent, stXWaitMapped, (char* )mySlave.hWindowGl);
        }
        // always hise mouse cursor on slave window
        mySlave.setupNoCursor();
    }
    if(!attribs.IsHidden) {
        if(myMaster.hWindow != 0) {
            XMapWindow(hDisplay, myMaster.hWindow);
            //XIfEvent(hDisplay, &myXEvent, stXWaitMapped, (char* )myMaster.hWindow);
        }
        XMapWindow(hDisplay, myMaster.hWindowGl);
        //XIfEvent(hDisplay, &myXEvent, stXWaitMapped, (char* )myMaster.hWindowGl);
    }

    // setup default icon
    if((Window )myParentWin == 0) {
        XpmCreatePixmapFromData(hDisplay, myMaster.hWindow, (char** )sview_xpm, &myMaster.iconImage, &myMaster.iconShape, NULL);
        XWMHints anIconHints;
        anIconHints.flags       = IconPixmapHint | IconMaskHint;
        anIconHints.icon_pixmap = myMaster.iconImage;
        anIconHints.icon_mask   = myMaster.iconShape;
        XSetWMHints(hDisplay, myMaster.hWindow, &anIconHints);
    }

    // we need this call to go around bugs
    if(!attribs.IsFullScreen && myMaster.hWindow != 0) {
        XMoveResizeWindow(hDisplay, myMaster.hWindow,
                          myRectNorm.left(),  myRectNorm.top(),
                          myRectNorm.width(), myRectNorm.height());
    }
    // flushes the output buffer, most client apps needn't use this cause buffer is automatically flushed as needed by calls to XNextEvent()...
    XFlush(hDisplay);
    myIsUpdated = true;
    myInitState = STWIN_INIT_SUCCESS;
    return true;
}

/**
 * Update StWindow position according to native parent position.
 */
void StWindowImpl::updateChildRect() {
    if(!attribs.IsFullScreen && (Window )myParentWin != 0 && !myMaster.stXDisplay.isNull()) {
        Display* hDisplay = myMaster.stXDisplay->hDisplay;
        Window dummyWin;
        int xReturn, yReturn;
        unsigned int widthReturn, heightReturn, uDummy;
        XGetGeometry(hDisplay, (Window )myParentWin, &dummyWin,
                     &xReturn, &yReturn, &widthReturn, &heightReturn,
                     &uDummy, &uDummy);
        XTranslateCoordinates(hDisplay, (Window )myParentWin,
                              myMaster.stXDisplay->getRootWindow(),
                              0, 0, &myRectNorm.left(), &myRectNorm.top(), &dummyWin);
        myRectNorm.right()  = myRectNorm.left() + widthReturn;
        myRectNorm.bottom() = myRectNorm.top() + heightReturn;

        // update only when changes
        if(myRectNorm != myRectNormPrev) {
            myRectNormPrev = myRectNorm;
            myIsUpdated    = true;
            myMessageList.append(StMessageList::MSG_RESIZE);

            const StRectI_t& aRect = attribs.IsFullScreen ? myRectFull : myRectNorm;
            myStEvent.Type       = stEvent_Size;
            myStEvent.Size.Time  = getEventTime();
            myStEvent.Size.SizeX = aRect.width();
            myStEvent.Size.SizeY = aRect.height();
            signals.onResize->emit(myStEvent.Size);
        }
    }
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
        // TODO (Kirill Gavrilov#9) parse correctly
        // do nothing, just set the flag
        return;
    } else if(myMaster.stXDisplay.isNull()) {
        return;
    }

    Display* hDisplay = myMaster.stXDisplay->hDisplay;
    if(attribs.IsFullScreen) {
        const StMonitor& stMon = (myMonMasterFull == -1) ? myMonitors[myRectNorm.center()] : myMonitors[myMonMasterFull];
        myRectFull = stMon.getVRect();
        XUnmapWindow(hDisplay, myMaster.hWindowGl); // workaround for strange bugs
        StRectI_t aRect = myRectFull;

        // use tiled Master+Slave layout within single window if possible
        if(attribs.Slave != StWinSlave_slaveOff && isSlaveIndependent() && myMonitors.size() > 1) {
            StRectI_t aRectSlave;
            aRectSlave.left()   = getSlaveLeft();
            aRectSlave.top()    = getSlaveTop();
            aRectSlave.right()  = aRectSlave.left() + myRectFull.width();
            aRectSlave.bottom() = aRectSlave.top()  + myRectFull.height();
            myTiledCfg = TiledCfg_Separate;
            if(myRectFull.top()   == aRectSlave.top()) {
                if(myRectFull.right() == aRectSlave.left()) {
                    myTiledCfg = TiledCfg_MasterSlaveX;
                } else if(myRectFull.left() == aRectSlave.right()) {
                    myTiledCfg = TiledCfg_SlaveMasterX;
                }
            } else if(myRectFull.left() == aRectSlave.left()) {
                if(myRectFull.bottom() == aRectSlave.top()) {
                    myTiledCfg = TiledCfg_MasterSlaveY;
                } else if(myRectFull.top() == aRectSlave.bottom()) {
                    myTiledCfg = TiledCfg_SlaveMasterY;
                }
            }
        }

        if(myTiledCfg != TiledCfg_Separate) {
            XUnmapWindow(hDisplay, mySlave.hWindowGl);
            getTiledWinRect(aRect);
        }

        if((Window )myParentWin != 0 || myMaster.hWindow != 0) {
            XReparentWindow(hDisplay, myMaster.hWindowGl, myMaster.stXDisplay->getRootWindow(), 0, 0);
            XMoveResizeWindow(hDisplay, myMaster.hWindowGl,
                              aRect.left(),  aRect.top(),
                              aRect.width(), aRect.height());
            XFlush(hDisplay);
            XMapWindow(hDisplay, myMaster.hWindowGl);
            //XIfEvent(hDisplay, &myXEvent, stXWaitMapped, (char* )myMaster.hWindowGl);
        } else {
            XMoveResizeWindow(hDisplay, myMaster.hWindowGl,
                              aRect.left(),  aRect.top(),
                              aRect.width(), aRect.height());
            XFlush(hDisplay);
            XMapWindow(hDisplay, myMaster.hWindowGl);
            //XIfEvent(hDisplay, &myXEvent, stXWaitMapped, (char* )myMaster.hWindowGl);
        }

        if(attribs.Slave != StWinSlave_slaveOff
        && myTiledCfg == TiledCfg_Separate
        && (!isSlaveIndependent() || myMonitors.size() > 1)) {
            XMoveResizeWindow(hDisplay, mySlave.hWindowGl,
                              getSlaveLeft(),  getSlaveTop(),
                              getSlaveWidth(), getSlaveHeight());
        }
    } else {
        Window aParent = ((Window )myParentWin != 0) ? (Window )myParentWin : myMaster.hWindow;
        if(aParent != 0) {
            // workaround bugs in some OpenGL drivers (Catalyst etc.) - entirely re-create window but not GL context
            XSetWindowAttributes aWinAttribsX = createDefaultAttribs(myMaster.stXDisplay);
            aWinAttribsX.override_redirect = True; // GL window always undecorated
            Window aWin = XCreateWindow(hDisplay, aParent,
                                        0, 0, myRectNorm.width(), myRectNorm.height(),
                                        0, myMaster.stXDisplay->getDepth(),
                                        InputOutput,
                                        myMaster.stXDisplay->getVisual(),
                                        CWBorderPixel | CWColormap | CWEventMask | CWOverrideRedirect, &aWinAttribsX);
            if(!myMaster.hRC->makeCurrent(aWin)) {
                ST_ERROR_LOG("X, FAILED to bind rendering context to NEW master window");
                XDestroyWindow(hDisplay, aWin);
                XReparentWindow(hDisplay, myMaster.hWindowGl, aParent, 0, 0);
            } else {
                XUnmapWindow  (hDisplay, myMaster.hWindowGl);
                XDestroyWindow(hDisplay, myMaster.hWindowGl);
                myMaster.hWindowGl = aWin;
                XSetStandardProperties(hDisplay, myMaster.hWindowGl,
                                       "master window", "master window",
                                       None, NULL, 0, NULL);
                myMaster.setupXDND();
                if(attribs.ToHideCursor) {
                    myMaster.setupNoCursor();
                }
                XMapWindow(hDisplay, myMaster.hWindowGl);
            }
            myIsUpdated = true;
        } else {
            XUnmapWindow(hDisplay, myMaster.hWindowGl); // workaround for strange bugs
            XResizeWindow(hDisplay, myMaster.hWindowGl, 256, 256);
            if(attribs.Slave != StWinSlave_slaveOff && (!isSlaveIndependent() || myMonitors.size() > 1)) {
                XUnmapWindow (hDisplay, mySlave.hWindowGl);
                XResizeWindow(hDisplay, mySlave.hWindowGl, 256, 256);
            }
            XFlush(hDisplay);
            XMapWindow(hDisplay, myMaster.hWindowGl);
            //XIfEvent(hDisplay, &myXEvent, stXWaitMapped, (char* )myMaster.hWindowGl);
            if(attribs.Slave != StWinSlave_slaveOff && (!isSlaveIndependent() || myMonitors.size() > 1)) {
                XMapWindow(hDisplay, mySlave.hWindowGl);
                //XIfEvent(hDisplay, &myXEvent, stXWaitMapped, (char* )mySlave.hWindowGl);
            }
            XFlush(hDisplay);
            XMoveResizeWindow(hDisplay, myMaster.hWindowGl,
                              myRectNorm.left(),  myRectNorm.top(),
                              myRectNorm.width(), myRectNorm.height());
        }
    }
    XSetInputFocus(hDisplay, myMaster.hWindowGl, RevertToParent, CurrentTime);

    const StRectI_t& aRect = attribs.IsFullScreen ? myRectFull : myRectNorm;
    myStEvent.Type       = stEvent_Size;
    myStEvent.Size.Time  = getEventTime();
    myStEvent.Size.SizeX = aRect.width();
    myStEvent.Size.SizeY = aRect.height();
    signals.onResize->emit(myStEvent.Size);

    myMessageList.append(StMessageList::MSG_RESIZE); // add event to update GL rendering scape
    myMessageList.append(StMessageList::MSG_FULLSCREEN_SWITCH);
    // flushes the output buffer, most client apps needn't use this cause buffer is automatically flushed as needed by calls to XNextEvent()...
    XFlush(hDisplay);
}

void StWindowImpl::parseXDNDClientMsg() {
    const StXDisplayH& aDisplay = myMaster.stXDisplay;
    // myMaster.hWindow or myMaster.hWindowGl
    Window aWinReciever = ((XClientMessageEvent* )&myXEvent)->window;
    if(myXEvent.xclient.message_type == aDisplay->xDNDEnter) {
        myMaster.xDNDVersion = (myXEvent.xclient.data.l[1] >> 24);
        bool isMoreThan3 = myXEvent.xclient.data.l[1] & 1;
        Window aSrcWin = myXEvent.xclient.data.l[0];
        /*ST_DEBUG_LOG(
            "Xdnd: Source window = 0x" + (int )myXEvent.xclient.data.l[0] + "\n"
            + "Supports > 3 types = " + (more_than_3) + "\n"
            + "Protocol version = " + myMaster.xDNDVersion + "\n"
            + "Type 1 = " + myMaster.getAtomName(myXEvent.xclient.data.l[2]) + "\n"
            + "Type 2 = " + myMaster.getAtomName(myXEvent.xclient.data.l[3]) + "\n"
            + "Type 3 = " + myMaster.getAtomName(myXEvent.xclient.data.l[4]) + "\n"
        );*/
        if(isMoreThan3) {
            Property aProperty = aDisplay->readProperty(aSrcWin, aDisplay->xDNDTypeList);
            Atom* anAtomList = (Atom* )aProperty.data;
            for(int anIter = 0; anIter < aProperty.nitems; ++anIter) {
                if(anAtomList[anIter] == aDisplay->xDNDPlainText) {
                    myMaster.xDNDRequestType = aDisplay->xDNDPlainText;
                    break;
                }
            }
            XFree(aProperty.data);
        } else if((Atom )myXEvent.xclient.data.l[2] == aDisplay->xDNDPlainText
               || (Atom )myXEvent.xclient.data.l[3] == aDisplay->xDNDPlainText
               || (Atom )myXEvent.xclient.data.l[4] == aDisplay->xDNDPlainText) {
            myMaster.xDNDRequestType = aDisplay->xDNDPlainText;
        } else {
            myMaster.xDNDRequestType = XA_STRING;
        }
    } else if(myXEvent.xclient.message_type == aDisplay->xDNDPosition) {
        /*ST_DEBUG_LOG(
            "Source window = 0x" + (int )myXEvent.xclient.data.l[0] + "\n"
            + "Position: x=" + (int )(myXEvent.xclient.data.l[2] >> 16) + " y=" + (int )(myXEvent.xclient.data.l[2] & 0xffff)  + "\n"
            + "Timestamp = " + (int )myXEvent.xclient.data.l[3] + " (Version >= 1 only)\n"
        );*/

        // Xdnd: reply with an XDND status message
        XClientMessageEvent aMsg;
        stMemSet(&aMsg, 0, sizeof(aMsg));
        aMsg.type    = ClientMessage;
        aMsg.display = myXEvent.xclient.display;
        aMsg.window  = myXEvent.xclient.data.l[0];
        aMsg.message_type = aDisplay->xDNDStatus;
        aMsg.format  = 32;
        aMsg.data.l[0] = aWinReciever;
        aMsg.data.l[1] = True; //
        aMsg.data.l[2] = 0;    // specify an empty rectangle
        aMsg.data.l[3] = 0;
        aMsg.data.l[4] = aDisplay->xDNDActionCopy; // we only accept copying anyway.

        XSendEvent(aDisplay->hDisplay, myXEvent.xclient.data.l[0], False, NoEventMask, (XEvent* )&aMsg);
        XFlush(aDisplay->hDisplay);
    } else if(myXEvent.xclient.message_type == aDisplay->xDNDLeave) {
        //
    } else if(myXEvent.xclient.message_type == aDisplay->xDNDDrop) {
        myMaster.xDNDSrcWindow = myXEvent.xclient.data.l[0];
        Atom aSelection = aDisplay->xDNDPrimary;
        if(myMaster.xDNDVersion >= 1) {
            XConvertSelection(aDisplay->hDisplay, aDisplay->xDNDSelection, myMaster.xDNDRequestType,
                              aSelection, aWinReciever, myXEvent.xclient.data.l[2]);
        } else {
            XConvertSelection(aDisplay->hDisplay, aDisplay->xDNDSelection, myMaster.xDNDRequestType,
                              aSelection, aWinReciever, CurrentTime);
        }
    }
}

void StWindowImpl::parseXDNDSelectionMsg() {
    Atom aTarget = myXEvent.xselection.target;
    const StXDisplayH& aDisplay = myMaster.stXDisplay;
    // myMaster.hWindow or myMaster.hWindowGl
    Window aWinReciever = ((XClientMessageEvent* )&myXEvent)->window;
    /*ST_DEBUG_LOG(
        "A selection notify has arrived!\n"
        + "Requestor = 0x" + (int )myXEvent.xselectionrequest.requestor + "\n"
        + "Selection atom = " + myMaster.getAtomName(myXEvent.xselection.selection) + "\n"
        + "Target atom    = " + myMaster.getAtomName(aTarget) + "\n"
        + "Property atom  = " + myMaster.getAtomName(myXEvent.xselection.property) + "\n"
    );*/
    if(myXEvent.xselection.property == None) {
        return;
    } else {
        Atom aSelection = aDisplay->xDNDPrimary;
        Property aProperty = aDisplay->readProperty(aWinReciever, aSelection);
        //If we're being given a list of targets (possible conversions)
        if(aTarget == aDisplay->XA_TARGETS) {
            XConvertSelection(aDisplay->hDisplay, aSelection, XA_STRING, aSelection, aWinReciever, CurrentTime);
        } else if(aTarget == myMaster.xDNDRequestType) {
            StString aData = StString((char* )aProperty.data);

            size_t anEndChar = aData.getLength();
            for(StUtf8Iter anIter = aData.iterator(); *anIter != 0; ++anIter) {
                // cut only first filename, separated with CR/LF
                if(*anIter == stUtf32_t('\n') || *anIter == stUtf32_t(13)) {
                    anEndChar = anIter.getIndex();
                    break;
                }
            }

            const StString ST_FILE_PROTOCOL("file://");
            size_t aCutFrom = aData.isStartsWith(ST_FILE_PROTOCOL) ? ST_FILE_PROTOCOL.getLength() : 0;
            aData = aData.subString(aCutFrom, anEndChar);
            StString aFile;
            if(myMaster.xDNDRequestType != XA_STRING) {
                aFile.fromUrl(aData);
            } else {
                aFile = aData;
            }
            myStEvent.Type = stEvent_FileDrop;
            myStEvent.DNDrop.Time = getEventTime(myXEvent.xselection.time);
            myStEvent.DNDrop.File = aFile.toCString();
            signals.onFileDrop->emit(myStEvent.DNDrop);

            //ST_DEBUG_LOG(data);

            // Reply OK
            XClientMessageEvent aMsg;
            stMemSet(&aMsg, 0, sizeof(aMsg));
            aMsg.type      = ClientMessage;
            aMsg.display   = aDisplay->hDisplay;
            aMsg.window    = myMaster.xDNDSrcWindow;
            aMsg.message_type = aDisplay->xDNDFinished;
            aMsg.format    = 32;
            aMsg.data.l[0] = aWinReciever;
            aMsg.data.l[1] = 1;
            aMsg.data.l[2] = aDisplay->xDNDActionCopy;

            // Reply that all is well
            XSendEvent(aDisplay->hDisplay, myMaster.xDNDSrcWindow, False, NoEventMask, (XEvent* )&aMsg);
            XSync(aDisplay->hDisplay, False);
        }
        XFree(aProperty.data);
    }
}

void StWindowImpl::updateWindowPos() {
    const StXDisplayH& aDisplay = myMaster.stXDisplay;
    if(aDisplay.isNull() || myMaster.hWindowGl == 0) {
        return;
    }

    if(!attribs.IsFullScreen) {
        if(myRectNorm.left() == 0 && myRectNorm.top() == 0 && myMaster.hWindow != 0) {
            int width  = myRectNorm.width();
            int height = myRectNorm.height();
            Window hChildWin;
            XTranslateCoordinates(aDisplay->hDisplay, myMaster.hWindow,
                                  aDisplay->getRootWindow(),
                                  0, 0, &myRectNorm.left(), &myRectNorm.top(), &hChildWin);
            myRectNorm.right()  = myRectNorm.left() + width;
            myRectNorm.bottom() = myRectNorm.top() + height;
        }
        if(myMaster.hWindow != 0) {
            XMoveResizeWindow(aDisplay->hDisplay, myMaster.hWindowGl,
                              0, 0,
                              myRectNorm.width(), myRectNorm.height());
        }
        if(attribs.Slave != StWinSlave_slaveOff && (!isSlaveIndependent() || myMonitors.size() > 1)) {
            XMoveResizeWindow(aDisplay->hDisplay, mySlave.hWindowGl,
                              getSlaveLeft(),  getSlaveTop(),
                              getSlaveWidth(), getSlaveHeight());
        }

        if(myTiledCfg != TiledCfg_Separate) {
            myTiledCfg = TiledCfg_Separate;
            if(!attribs.IsHidden && myMonitors.size() > 1) {
                XMapWindow(aDisplay->hDisplay, mySlave.hWindowGl);
            }
        }
    }
    myMessageList.append(StMessageList::MSG_RESIZE); // add event to update GL rendering scape

    const StRectI_t& aRect = attribs.IsFullScreen ? myRectFull : myRectNorm;
    myStEvent.Type       = stEvent_Size;
    myStEvent.Size.Time  = getEventTime();
    myStEvent.Size.SizeX = aRect.width();
    myStEvent.Size.SizeY = aRect.height();
    signals.onResize->emit(myStEvent.Size);

    // force input focus to Master
    XSetInputFocus(aDisplay->hDisplay, myMaster.hWindowGl, RevertToParent, CurrentTime);

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
    const StXDisplayH& aDisplay = myMaster.stXDisplay;
    if(aDisplay.isNull() || myMaster.hWindowGl == 0) {
        // window is closed!
        myMessageList.popList(theMessages);
        return;
    }

    // detect embedded window was moved
    if(!attribs.IsFullScreen && (Window )myParentWin != 0 && myMaster.hWindowGl != 0
    //&&  (attribs.Slave == StWinSlave_slaveSync || attribs.Slave == StWinSlave_slaveFlipX || attribs.Slave == StWinSlave_slaveFlipY)
    && ++mySyncCounter > 4) {
        // sadly, this Xlib call takes a lot of time
        // perform it rarely
        Window aDummyWin;
        int aCurrPosX, aCurrPosY;
        if(XTranslateCoordinates(aDisplay->hDisplay, myMaster.hWindowGl,
                                 aDisplay->getRootWindow(),
                                 0, 0, &aCurrPosX, &aCurrPosY, &aDummyWin)
        && (aCurrPosX != myRectNorm.left() || aCurrPosY != myRectNorm.top())) {
            //ST_DEBUG_LOG("need updateChildRect= " + aCurrPosX + " x " + aCurrPosY);
            updateChildRect();
        }
        mySyncCounter = 0;
    }

    int anEventsNb = XPending(aDisplay->hDisplay);
    for(int anIter = 0; anIter < anEventsNb && XPending(aDisplay->hDisplay) > 0; ++anIter) {
        XNextEvent(aDisplay->hDisplay, &myXEvent);
        switch(myXEvent.type) {
            case ClientMessage: {
                /*ST_DEBUG_LOG("A ClientMessage has arrived:\n"
                    + "Type = " + aDisplay->getAtomName(myXEvent.xclient.message_type)
                    + " (" + myXEvent.xclient.format + ")\n"
                    + " message_type " + myXEvent.xclient.message_type + "\n"
                );*/
                parseXDNDClientMsg();
                if(myXEvent.xclient.data.l[0] == (int )aDisplay->wndDestroyAtom) {
                    myMessageList.append(StMessageList::MSG_CLOSE);
                }
                break;
            }
            case SelectionNotify: {
                parseXDNDSelectionMsg();
                break;
            }
            case DestroyNotify: {
                // something else...
                break;
            }
            case KeyPress: {
                XKeyEvent*   aKeyEvent = (XKeyEvent* )&myXEvent;
                const KeySym aKeySym   = XLookupKeysym(aKeyEvent, 0);
                myStEvent.Key.Char = 0;

                Status aStatus;
                KeySym aKeySymLoc;
                int aByteNb = Xutf8LookupString(aDisplay->hInputCtx, &myXEvent.xkey, myXInputBuff, sizeof(myXInputBuff), &aKeySymLoc, &aStatus);
                switch(aStatus) {
                    case XLookupChars:
                    case XLookupBoth: {
                        if(aByteNb > 0) {
                            StUtf8Iter aCharIter(myXInputBuff);
                            myStEvent.Key.Char = *aCharIter;
                        }
                        break;
                    }
                    case XBufferOverflow: ST_DEBUG_LOG("XBufferOverflow"); break;
                    //case XLookupNone:     ST_DEBUG_LOG("XLookupNone");     break;
                    //case XLookupKeySym:   ST_DEBUG_LOG("XLookupKeySym");   break;
                    default: break;
                }

                StVirtKey aVKeySt = ST_VK_NULL;
                if(aKeySym < ST_XK2ST_VK_SIZE) {
                    aVKeySt = (StVirtKey )ST_XK2ST_VK[aKeySym];
                } else if(aKeySym >= ST_XKMEDIA_FIRST && aKeySym <= ST_XKMEDIA_LAST) {
                    aVKeySt = (StVirtKey )ST_XKMEDIA2ST_VK[aKeySym - ST_XKMEDIA_FIRST];
                }
                if(aVKeySt != ST_VK_NULL) {
                    myMessageList.getKeysMap()[aVKeySt] = true;

                    myStEvent.Type = stEvent_KeyDown;
                    myStEvent.Key.Time  = getEventTime(aKeyEvent->time);
                    myStEvent.Key.VKey  = aVKeySt;
                    myStEvent.Key.Flags = ST_VF_NONE;
                    if(myMessageList.getKeysMap()[ST_VK_SHIFT]) {
                        myStEvent.Key.Flags = StVirtFlags(myStEvent.Key.Flags | ST_VF_SHIFT);
                    }
                    if(myMessageList.getKeysMap()[ST_VK_CONTROL]) {
                        myStEvent.Key.Flags = StVirtFlags(myStEvent.Key.Flags | ST_VF_CONTROL);
                    }

                    signals.onKeyDown->emit(myStEvent.Key);
                }
                break;
            }
            case KeyRelease: {
                XKeyEvent*   aKeyEvent = (XKeyEvent* )&myXEvent;
                const KeySym aKeySym   = XLookupKeysym(aKeyEvent, 0);
                //ST_DEBUG_LOG("KeyRelease, keycode= " + aKeyEvent->keycode
                //         + "; KeySym = " + (unsigned int )aKeySym + "\n");
                StVirtKey aVKeySt = ST_VK_NULL;
                if(aKeySym < ST_XK2ST_VK_SIZE) {
                    aVKeySt = (StVirtKey )ST_XK2ST_VK[aKeySym];
                } else if(aKeySym >= ST_XKMEDIA_FIRST && aKeySym <= ST_XKMEDIA_LAST) {
                    aVKeySt = (StVirtKey )ST_XKMEDIA2ST_VK[aKeySym - ST_XKMEDIA_FIRST];
                }
                if(aVKeySt != ST_VK_NULL) {
                    myMessageList.getKeysMap()[aVKeySt] = false;

                    myStEvent.Type = stEvent_KeyUp;
                    myStEvent.Key.Time  = getEventTime(aKeyEvent->time);
                    myStEvent.Key.VKey  = aVKeySt;
                    myStEvent.Key.Flags = ST_VF_NONE;
                    myStEvent.Key.Char  = 0;
                    if(myMessageList.getKeysMap()[ST_VK_SHIFT]) {
                        myStEvent.Key.Flags = StVirtFlags(myStEvent.Key.Flags | ST_VF_SHIFT);
                    }
                    if(myMessageList.getKeysMap()[ST_VK_CONTROL]) {
                        myStEvent.Key.Flags = StVirtFlags(myStEvent.Key.Flags | ST_VF_CONTROL);
                    }
                    signals.onKeyUp->emit(myStEvent.Key);
                }
                break;
            }
            case ButtonPress:
            case ButtonRelease: {
                const XButtonEvent* aBtnEvent = &myXEvent.xbutton;
                int aPosX = aBtnEvent->x;
                int aPosY = aBtnEvent->y;
                correctTiledCursor(aPosX, aPosY);
                const StRectI_t& aRect = attribs.IsFullScreen ? myRectFull : myRectNorm;
                StVirtButton aMouseBtn = ST_NOMOUSE;
                switch(aBtnEvent->button) {
                    case 1:  aMouseBtn = ST_MOUSE_LEFT;     break;
                    case 3:  aMouseBtn = ST_MOUSE_RIGHT;    break;
                    case 2:  aMouseBtn = ST_MOUSE_MIDDLE;   break;
                    default: aMouseBtn = (StVirtButton )aBtnEvent->button; break;
                }
                // force input focus to the master window
                XSetInputFocus(aDisplay->hDisplay, myMaster.hWindowGl, RevertToParent, CurrentTime);

                myStEvent.Button.Time    = getEventTime(aBtnEvent->time);
                myStEvent.Button.Button  = aMouseBtn;
                myStEvent.Button.Buttons = 0;
                myStEvent.Button.PointX  = double(aPosX) / double(aRect.width());
                myStEvent.Button.PointY  = double(aPosY) / double(aRect.height());
                if(myXEvent.type == ButtonPress) {
                    myStEvent.Type = stEvent_MouseDown;
                    signals.onMouseDown->emit(myStEvent.Button);
                } else {
                    myStEvent.Type = stEvent_MouseUp;
                    signals.onMouseUp->emit(myStEvent.Button);
                }
                break;
            }
            //case ResizeRequest:
            //case ConfigureRequest:
            case ConfigureNotify: {
                //ST_DEBUG_LOG("ConfigureNotify");
                const XConfigureEvent* aCfgEvent = &myXEvent.xconfigure;
                if(!attribs.IsFullScreen && aCfgEvent->window == myMaster.hWindow) {
                    StRectI_t aNewRect;
                    if(aCfgEvent->x > 64 && aCfgEvent->y > 64) {
                        // we got real GLorigin position
                        // worked when we in Compiz mode and on MoveWindow
                        aNewRect.left()   = aCfgEvent->x;
                        aNewRect.right()  = aCfgEvent->x + aCfgEvent->width;
                        aNewRect.top()    = aCfgEvent->y;
                        aNewRect.bottom() = aCfgEvent->y + aCfgEvent->height;
                    } else {
                        // we got psevdo x and y (~window decorations?)
                        // on ResizeWindow without Compiz
                        Window aChild;
                        aNewRect.left() = 0;
                        aNewRect.top()  = 0;
                        XTranslateCoordinates(aDisplay->hDisplay, myMaster.hWindow,
                                              aDisplay->getRootWindow(),
                                              0, 0, &aNewRect.left(), &aNewRect.top(), &aChild);
                        aNewRect.right()  = aNewRect.left() + aCfgEvent->width;
                        aNewRect.bottom() = aNewRect.top()  + aCfgEvent->height;
                    }
                    if(myRectNorm != aNewRect) {
                        // new compiz send messages when placement not really changed
                        myRectNorm  = aNewRect;
                        myIsUpdated = true; // call updateWindowPos() to update position
                    }
                }
                break;
            }
            default: {
                // process XRandr events
                if(myMaster.isRecXRandrEvents) {
                    int xrandrEventType = myXEvent.type - myMaster.xrandrEventBase;
                    if(xrandrEventType == RRNotify || xrandrEventType == RRScreenChangeNotify) {
                        ST_DEBUG_LOG("XRandr update event");
                        updateMonitors();
                    }
                }
            }
        }
    }

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

    // StWindow XLib implementation process events in the same thread
    // thus this double buffer is not in use
    // however user events may be posted to it
    swapEventsBuffers();
}

#endif
