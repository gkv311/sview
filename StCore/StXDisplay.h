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

#ifndef __StXDisplay_h_
#define __StXDisplay_h_

#if defined(__linux__)

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>
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
    GLXFBConfig     FBCfg;

    XIM      hInputMethod;
    XIC         hInputCtx;

    Atom     wndProtocols;
    Atom   wndDestroyAtom; // Atom for close message

    Atom        xDNDEnter; // Atoms for X drag&drop protocol
    Atom     xDNDPosition;
    Atom       xDNDStatus;
    Atom     xDNDTypeList;
    Atom   xDNDActionCopy;
    Atom         xDNDDrop;
    Atom        xDNDLeave;
    Atom     xDNDFinished;
    Atom    xDNDSelection;
    Atom        xDNDProxy;
    Atom        xDNDAware;
    Atom    xDNDPlainText;
    Atom      xDNDPrimary;
    Atom       XA_TARGETS;

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
