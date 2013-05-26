/**
 * Copyright © 2009-2011 Kirill Gavrilov <kirill@sview.ru>
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

#ifndef __StDXManager_h_
#define __StDXManager_h_

#include <StStrings/StLogger.h>
#include <d3d9.h>

#include "StDXInfo.h"

class StMonitor;

typedef enum {
    ST_DX_ADAPTER_ANY = 0,
    ST_DX_ADAPTER_AMD,
    ST_DX_ADAPTER_NVIDIA,
} StDXAdapterClass;

/**
 * Simple Direct3D manager class.
 */
class StDXManager {

        public:

    /**
     * Launch test in other thread.
     */
    static void initInfoAsync();

    static bool getInfo(StDXInfo&  theInfo,
                        const bool theForced = false);

    enum {
        ST_DX_VENDOR_AMD    = 4098,
        ST_DX_VENDOR_NVIDIA = 4318,
    };

        public:

    /**
     * Empty constructor.
     */
    StDXManager();

    /**
     * Destroy D3D connection.
     */
    virtual ~StDXManager();

    /**
     * Access Direct3D device instance.
     */
    IDirect3DDevice9* getD3DDevice() {
        return myD3dDevice;
    }

    bool withAqbs() const {
        return myWithAqbs;
    }

    /**
     * Initialize Direct3D output device.
     */
    bool init(const HWND       theWinHandle,
              const int        theSizeX,
              const int        theSizeY,
              const bool       theFullscreen,
              const StMonitor& theMonitor,
              const StDXAdapterClass theAdapter = ST_DX_ADAPTER_ANY);

    /**
     * Reset Direct3D output settings. Could be used to switch windowed/fullscreen modes.
     * Use very carefully! Most objects should be released before and recareated after!
     */
    bool reset(const HWND theWinHandle,
               const int  theSizeX,
               const int  theSizeY,
               const bool theFullscreen);

    /**
     * Starts the scene render.
     * Clears the backbuffer and sets the device to start rendering to it.
     */
    void beginRender();

    /**
     * Ends the scene render.
     */
    void endRender();

    /**
     * Presents to the screen.
     */
    bool swap();

        protected:

    bool initDxLib();
    bool checkAqbsSupport(const HWND theWinHandle);

    IDirect3DDevice9* createAqbsDevice(const UINT                   theAdapterId,
                                       const HWND                   theWinHandle,
                                       const D3DPRESENT_PARAMETERS& theD3dParams);

    IDirect3DDevice9* createAqbsTmpDevice(const UINT                   theAdapterId,
                                          const HWND                   theWinHandle,
                                          const D3DPRESENT_PARAMETERS& theD3dParams);

        protected:

    IDirect3D9*           myD3dLib;      //!< Direct3D library instance
    IDirect3DDevice9*     myD3dDevice;   //!< Direct3D device object
    D3DPRESENT_PARAMETERS myD3dParams;   //!< parameters for created Direct3D device
    D3DDISPLAYMODE        myCurrMode;    //!< temporary variable
    UINT                  myRefreshRate;
    bool                  myWithAqbs;    //!< indicates that device was created with AQBS flags

};

#endif //__StDXManager_h_
