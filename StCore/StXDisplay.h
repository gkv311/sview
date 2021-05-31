/**
 * StCore, window system independent C++ toolkit for writing OpenGL applications.
 * Copyright © 2007-2015 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#ifndef __StXDisplay_h_
#define __StXDisplay_h_

#if defined(__linux__) && !defined(__ANDROID__)

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>

// exclude modern definitions and system-provided glext.h, should be defined before gl.h inclusion
#ifndef GL_GLEXT_LEGACY
    #define GL_GLEXT_LEGACY
#endif
#ifndef GLX_GLXEXT_LEGACY
    #define GLX_GLXEXT_LEGACY
#endif
#include <GL/glx.h>

#include <StStrings/StString.h>
#include <StTemplates/StHandle.h>

struct Property {
    unsigned char* data;
    int format, nitems;
    Atom type;
};

class StXDisplay {

        public:

    Display*     hDisplay; // connection to the X-server
    XVisualInfo* hVisInfo; // visual info
    GLXFBConfig  FBCfg;

    XIM          hInputMethod;
    XIC          hInputCtx;

    Atom         wndProtocols;
    Atom         wndDestroyAtom; // Atom for close message

    Atom         xDNDEnter; // Atoms for X drag&drop protocol
    Atom         xDNDPosition;
    Atom         xDNDStatus;
    Atom         xDNDTypeList;
    Atom         xDNDActionCopy;
    Atom         xDNDDrop;
    Atom         xDNDLeave;
    Atom         xDNDFinished;
    Atom         xDNDSelection;
    Atom         xDNDProxy;
    Atom         xDNDAware;
    Atom         xDNDUriList;
    Atom         xDNDPlainText;
    Atom         xDNDPrimary;

    Atom         XA_TARGETS;
    Atom         XA_COMPOUND_TEXT;
    Atom         XA_UTF8_STRING;
    Atom         XA_CLIPBOARD;

        private:

    /**
     * Open connection to the server.
     * @return true on success.
     */
    ST_LOCAL bool open();

    /**
     * Close the connection to the server.
     */
    ST_LOCAL void close();

    /**
     * Initialize used atoms.
     */
    ST_LOCAL void initAtoms();

        public:

    /**
     * Open connection to the X-server using default parameters (DISPLAY variable)
     * and initialize the common stuff.
     */
    ST_LOCAL StXDisplay();

    /**
     * X-server will be disconnected.
     */
    ST_LOCAL ~StXDisplay();

    /**
     * @return true if X-server is connected.
     */
    ST_LOCAL bool isOpened() const {
        return hDisplay != NULL;
    }

    ST_LOCAL Window getRootWindow() const {
        return (hDisplay != NULL) ? RootWindow(hDisplay, getScreen()) : 0;
    }

    ST_LOCAL int getScreen() const {
        return (hVisInfo != NULL) ? hVisInfo->screen : 0;
    }

    ST_LOCAL unsigned int getDepth() const {
        return (hVisInfo != NULL) ? hVisInfo->depth : 0;
    }

    ST_LOCAL Visual* getVisual() const {
        return (hVisInfo != NULL) ? hVisInfo->visual : NULL;
    }

    /**
     * Convert an atom name into StString
     */
    ST_LOCAL StString getAtomName(Atom theAtom) const {
        return (theAtom == None) ? StString("None") : StString(XGetAtomName(hDisplay, theAtom));
    }

    /**
     * This fetches all the data from a property
     */
    ST_LOCAL Property readProperty(Window hWindow, Atom property) const;

};

// just short typedef for handle
typedef StHandle<StXDisplay> StXDisplayH;

#endif //__linux__
#endif //__StXDisplay_h_
