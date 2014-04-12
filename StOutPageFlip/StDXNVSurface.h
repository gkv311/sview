/**
 * Copyright © 2009-2014 Kirill Gavrilov <kirill@sview.ru>
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

#include <StTemplates/StHandle.h>

#include <d3d9.h>
#include <d3dx9tex.h>

class StDXAqbsControl;
class StDXManager;

/**
 * Simple NVIDIA stereo-coded Direct3D9 2D surface.
 * Stereo data should be filled in (left, right) order (sideBySide).
 */
class StDXNVSurface {

        public:

    /**
     * Empty constructor.
     * @param theSizeX width  of back buffer (mono)
     * @param theSizeY height of back buffer (mono)
     */
    StDXNVSurface(const size_t theSizeX,
                  const size_t theSizeY);

    virtual ~StDXNVSurface();

    /**
     * Release D3D resources.
     */
    virtual void release();

    /**
     * Create surface with constructor initialized size.
     */
    bool create(StDXManager& theD3d,
                const bool   theHasWglDx);

    /**
     * Texture for left buffer.
     */
    IDirect3DTexture9* TextureL() const {
        return myTextureL;
    }

    /**
     * Texture for right buffer.
     */
    IDirect3DTexture9* TextureR() const {
        return myTextureR;
    }

    /**
     * WDDM share handle for left buffer.
     */
    HANDLE TextureLShare() const {
        return myTextureLShare;
    }

    /**
     * WDDM share handle for right buffer.
     */
    HANDLE TextureRShare() const {
        return myTextureRShare;
    }

    /**
     * Fill surface with data. Left and right data filled in separate calls.
     * @param theSrcSizeX source data width
     * @param theSrcSizeY source data height
     * @param theSrcData  data to copy
     * @param theIsLeft   indicate left or right data
     */
    void update(const size_t     theSrcSizeX,
                const size_t     theSrcSizeY,
                const stUByte_t* theSrcData,
                bool             theIsLeft);

    /**
     * Sign surface with NV stereo code and render to back-buffer,
     */
    void render(StDXManager& theD3d);

        private:

    IDirect3DSurface9*        mySurfaceStereo; //!< surface for stereoscopic pair
    IDirect3DTexture9*        myTextureL;      //!< texture for left  buffer
    IDirect3DTexture9*        myTextureR;      //!< texture for right buffer
    HANDLE                    myTextureLShare; //!< WDDM share handle for left  buffer
    HANDLE                    myTextureRShare; //!< WDDM share handle for right buffer
    IDirect3DSurface9*        mySurfaceL;      //!< surface for left  texture
    IDirect3DSurface9*        mySurfaceR;      //!< surface for right texture
    StHandle<StDXAqbsControl> myAqbsControl;   //!< AQBS control surface if available

    RECT    mySrcRect;  //!< rectangle in surface to render in mono output
    RECT    myDstRect;  //!< rectangle in backbuffer
    RECT    mySrcRectL; //!< rectangle in surface corresponding to LEFT  view
    RECT    mySrcRectR; //!< rectangle in surface corresponding to RIGHT view
    size_t  mySizeX;    //!< surface width (double-sized)
    size_t  mySizeY;    //!< surface height

};

#endif // __StDXNVSurface_h_
