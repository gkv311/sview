/**
 * StOutPageFlip, class providing stereoscopic output for Shutter Glasses displays using StCore toolkit.
 * Copyright © 2009-2014 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
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

    static bool getInfo(StDXInfo&  theInfo,
                        const bool theForced = false);

    static StString printErrorDesc(HRESULT theErrCode);

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
    IDirect3DDevice9* getDevice() const {
        return myD3dDevice;
    }

    /**
     * @return adapters count
     */
    inline UINT getAdapterCount() const {
        return myD3dLib->GetAdapterCount();
    }

    /**
     * Fill adapter info structure.
     */
    inline HRESULT getAdapterIdentifier(UINT                    theAdapter,
                                        DWORD                   theFlags,
                                        D3DADAPTER_IDENTIFIER9* theIdentifier) const {
        return myD3dLib->GetAdapterIdentifier(theAdapter, theFlags, theIdentifier);
    }

    /**
     * Retrieves the current display mode of the adapter.
     */
    inline HRESULT getAdapterDisplayMode(UINT            theAdapter,
                                         D3DDISPLAYMODE* theMode) const {
        return myD3dLib->GetAdapterDisplayMode(theAdapter, theMode);
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
     * Use very carefully! Most objects should be released before and recreated after!
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

    IDirect3D9Ex*         myD3dLib;      //!< Direct3D library instance
    IDirect3DDevice9*     myD3dDevice;   //!< Direct3D device object
    D3DPRESENT_PARAMETERS myD3dParams;   //!< parameters for created Direct3D device
    D3DDISPLAYMODE        myCurrMode;    //!< temporary variable
    UINT                  myRefreshRate;
    bool                  myWithAqbs;    //!< indicates that device was created with AQBS flags

};

#endif // __StDXManager_h_
