/**
 * Copyright © 2011 Kirill Gavrilov <kirill@sview.ru>
 *
 * StOutPageFlip library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * StOutPageFlip library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.
 * If not, see <http://www.gnu.org/licenses/>.
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
