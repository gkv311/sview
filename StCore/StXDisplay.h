/**
 * Copyright © 2007-2011 Kirill Gavrilov <kirill@sview.ru>
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

#if(defined(__linux__) || defined(__linux))

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>

#include <StStrings/StString.h>
#include <StTemplates/StHandle.h>

struct ST_LOCAL Property {
    unsigned char* data;
    int format, nitems;
    Atom type;
};

class ST_LOCAL StXDisplay {

        public:

    Display*     hDisplay; // connection to the X-server
    XVisualInfo* hVisInfo; // visual info

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
    Atom       XA_TARGETS;

        private:

    /**
     * Open connection to the server.
     * @return true on success.
     */
    bool open();

    /**
     * Close the connection to the server.
     */
    void close();

    /**
     * Initialize used atoms.
     */
    void initAtoms();

        public:

    /**
     * Open connection to the X-server using default parameters (DISPLAY variable)
     * and initialize the common stuff.
     */
    StXDisplay();

    /**
     * X-server will be disconnected.
     */
    ~StXDisplay();

    /**
     * @return true if X-server is connected.
     */
    bool isOpened() const {
        return hDisplay != NULL;
    }

    Window getRootWindow() const {
        return (hDisplay != NULL) ? RootWindow(hDisplay, getScreen()) : 0;
    }

    int getScreen() const {
        return (hVisInfo != NULL) ? hVisInfo->screen : 0;
    }

    unsigned int getDepth() const {
        return (hVisInfo != NULL) ? hVisInfo->depth : 0;
    }

    Visual* getVisual() const {
        return (hVisInfo != NULL) ? hVisInfo->visual : NULL;
    }

    /**
     * Convert an atom name into StString
     */
    StString getAtomName(Atom theAtom) const {
        return (theAtom == None) ? StString("None") : StString(XGetAtomName(hDisplay, theAtom));
    }

    /**
     * This fetches all the data from a property
     */
    Property readProperty(Window hWindow, Atom property) const;

};

// just short typedef for handle
typedef StHandle<StXDisplay> StXDisplayH;

#endif //__linux__
#endif //__StXDisplay_h_
