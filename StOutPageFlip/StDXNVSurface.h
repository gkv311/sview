/**
 * Copyright © 2009-2013 Kirill Gavrilov <kirill@sview.ru>
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

#ifndef __StDXNVSurface_h_
#define __StDXNVSurface_h_

#include <d3d9.h>
#include <d3dx9tex.h>

class StDXAqbsControl;

/**
 * Simple NVIDIA stereo-coded Direct3D9 2D surface.
 * Stereo data should be filled in (left, right) order (sideBySide).
 */
class StDXNVSurface {

        public:

    StDXNVSurface(const size_t theSizeX,
                  const size_t theSizeY);

    virtual ~StDXNVSurface();

    /**
     * Create surface with constructor initialized size.
     */
    bool create(IDirect3DDevice9* theD3dDevice);

    /**
     * Fill surface with data. Left and right data filled in separate calls.
     * @param theSrcSizeX (const size_t ) - source data width;
     * @param theSrcSizeY (const size_t ) - source data height;
     * @param theSrcData (const unsigned char* ) - data to copy;
     * @param bool (isLeft ) - indicate left or right data.
     */
    void update(const size_t theSrcSizeX,
                const size_t theSrcSizeY,
                const unsigned char* theSrcData,
                bool isLeft);

    /**
     * Sign surface with NV stereo code and render to back-buffer,
     */
    void render(IDirect3DDevice9* theD3dDevice);

        private:

    IDirect3DSurface9* mySurface;     //!< surface
    StDXAqbsControl*   myAqbsControl; //!< AQBS control surface if available

    RECT    mySrcRect;  //!< rectangle in surface to render in mono output
    RECT    myDstRect;  //!< rectangle in backbuffer
    RECT    mySrcRectL; //!< rectangle in surface corresponding to LEFT  view
    RECT    mySrcRectR; //!< rectangle in surface corresponding to RIGHT view
    size_t  mySizeX;    //!< surface width (double-sized)
    size_t  mySizeY;    //!< surface height

};

#endif // __StDXNVSurface_h_
