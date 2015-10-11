/**
 * StOutPageFlip, class providing stereoscopic output for Shutter Glasses displays using StCore toolkit.
 * Copyright © 2011 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#ifndef __StDXAqbsControl_h_
#define __StDXAqbsControl_h_

#include <wnt/AtiDx9Stereo.h>

/**
 * Class provides an interface to communicate with AMD driver and AQBS functionality control.
 * Communication realized within D3D surface with special flag.
 */
class StDXAqbsControl {

        private:

    IDirect3DSurface9* myComSurface; //!< communication surface

        public:

    /**
     * Create communication surface on specified D3d device.
     */
    StDXAqbsControl(IDirect3DDevice9* theD3dDevice);

    /**
     * Release the surface.
     */
    ~StDXAqbsControl();

    /**
     * Returns true if connection is established.
     */
    bool isValid() {
        return myComSurface != NULL;
    }

    /**
     * Send the command using AQBS interface.
     */
    bool sendCommand(ATIDX9STEREOCOMMAND theCommand,
                     BYTE* theOutBuffer = NULL, DWORD theOutBufferSize = 0,
                     BYTE* theInBuffer  = NULL, DWORD theInBufferSize  = 0);

    bool enableStereo();
    bool setDstLeft();
    bool setDstRight();

};

#endif //__StDXAqbsControl_h_
