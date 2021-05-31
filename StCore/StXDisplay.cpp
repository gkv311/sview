/**
 * StCore, window system independent C++ toolkit for writing OpenGL applications.
 * Copyright © 2007-2015 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#if defined(__linux__) && !defined(__ANDROID__)

#include "StXDisplay.h"
#include <StStrings/StLogger.h>

StXDisplay::StXDisplay()
: hDisplay(NULL),
  hVisInfo(NULL),
  hInputMethod(None),
  hInputCtx(None),
  wndProtocols(None),
  wndDestroyAtom(None),
  xDNDEnter(None),
  xDNDPosition(None),
  xDNDStatus(None),
  xDNDTypeList(None),
  xDNDActionCopy(None),
  xDNDDrop(None),
  xDNDLeave(None),
  xDNDFinished(None),
  xDNDSelection(None),
  xDNDProxy(None),
  xDNDAware(None),
  xDNDUriList(None),
  xDNDPlainText(None),
  xDNDPrimary(None),
  XA_TARGETS(None),
  XA_COMPOUND_TEXT(None),
  XA_UTF8_STRING(None),
  XA_CLIPBOARD(None) {
    stMemZero(&FBCfg, sizeof(GLXFBConfig)); // should be just a pointer
    open();
}

StXDisplay::~StXDisplay() {
    close();
}

bool StXDisplay::open() {
    hDisplay = XOpenDisplay(NULL); // get first display on server from DISPLAY in env
    //hDisplay = XOpenDisplay(":0.0");
    //hDisplay = XOpenDisplay("somehost:0.0");
    //hDisplay = XOpenDisplay("192.168.1.10:0.0");
    if(isOpened()) {
        initAtoms();

        hInputMethod = XOpenIM(hDisplay, NULL, NULL, NULL);
        if(hInputMethod == NULL) {
            return true;
        }

        XIMStyles* anIMStyles = NULL;
        char* anIMValues = XGetIMValues(hInputMethod, XNQueryInputStyle, &anIMStyles, NULL);
        if(anIMValues != NULL
        || anIMStyles == NULL
        || anIMStyles->count_styles <= 0) {
            // input method doesn't support any styles
            if(anIMStyles != NULL) {
                XFree(anIMStyles);
            }
            return true;
        }
        const XIMStyle anIMStyle = anIMStyles->supported_styles[0];
        XFree(anIMStyles);

        hInputCtx = XCreateIC(hInputMethod, XNInputStyle, anIMStyle, NULL);

        return true;
    }
    return false;
}

void StXDisplay::close() {
    if(hInputCtx != None) {
        XDestroyIC(hInputCtx);
        hInputCtx = None;
    }
    if(hInputMethod != None) {
        XCloseIM(hInputMethod);
        hInputMethod = None;
    }
    if(hDisplay != NULL) {
        XCloseDisplay(hDisplay);
        hDisplay = NULL;
    }
    if(hVisInfo != NULL) {
        XFree(hVisInfo);
        hVisInfo = NULL;
    }
}

void StXDisplay::initAtoms() {
    wndDestroyAtom = XInternAtom(hDisplay, "WM_DELETE_WINDOW", True);
    wndProtocols   = XInternAtom(hDisplay, "WM_PROTOCOLS",     True);

    // Atoms for Xdnd
    xDNDEnter      = XInternAtom(hDisplay, "XdndEnter",        False);
    xDNDPosition   = XInternAtom(hDisplay, "XdndPosition",     False);
    xDNDStatus     = XInternAtom(hDisplay, "XdndStatus",       False);
    xDNDTypeList   = XInternAtom(hDisplay, "XdndTypeList",     False);
    xDNDActionCopy = XInternAtom(hDisplay, "XdndActionCopy",   False);
    xDNDDrop       = XInternAtom(hDisplay, "XdndDrop",         False);
    xDNDLeave      = XInternAtom(hDisplay, "XdndLeave",        False);
    xDNDFinished   = XInternAtom(hDisplay, "XdndFinished",     False);
    xDNDSelection  = XInternAtom(hDisplay, "XdndSelection",    False);
    xDNDProxy      = XInternAtom(hDisplay, "XdndProxy",        False);
    xDNDAware      = XInternAtom(hDisplay, "XdndAware",        False);
    xDNDUriList    = XInternAtom(hDisplay, "text/uri-list",    False);
    xDNDPlainText  = XInternAtom(hDisplay, "text/plain",       False);
    xDNDPrimary    = XInternAtom(hDisplay, "PRIMARY",          False);
    // This is a meta-format for data to be "pasted" in to.
    // Requesting this format acquires a list of possible
    // formats from the application which copied the data.
    XA_TARGETS       = XInternAtom(hDisplay, "TARGETS",        True);
    XA_COMPOUND_TEXT = XInternAtom(hDisplay, "COMPOUND_TEXT",  True);
    XA_UTF8_STRING   = XInternAtom(hDisplay, "UTF8_STRING",    True);
    XA_CLIPBOARD     = XInternAtom(hDisplay, "CLIPBOARD",      True);

}

Property StXDisplay::readProperty(Window hWindow, Atom property) const {
    Atom actualType;
    int actualFormat = 0;
    unsigned long nItems = 0;
    unsigned long bytesAfter = 0;
    unsigned char* ret = NULL;
    int readBytes = 1024;

    // Keep trying to read the property until there are no
    // bytes unread
    do {
        if(ret != NULL) {
            XFree(ret);
        }
        XGetWindowProperty(hDisplay, hWindow, property, 0, readBytes, False, AnyPropertyType,
                           &actualType, &actualFormat, &nItems, &bytesAfter,
                           &ret);
        readBytes *= 2;
    } while(bytesAfter != 0);
    /*ST_DEBUG_LOG("Actual type: " + getAtomName(actualType) + "\n"
        + "Actual format: " + actualFormat + "\n"
        + "Number of items: " + nItems + "\n"
    );*/
    Property aProperty = {ret, actualFormat, int(nItems), actualType};
    /// TODO (Kirill Gavrilov#9) - possible memory leek (no XFree(ret)) - should be avoided using Handles?
    return aProperty;
}

#endif
