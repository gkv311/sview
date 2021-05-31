/**
 * StOutPageFlip, class providing stereoscopic output for Shutter Glasses displays using StCore toolkit.
 * Copyright © 2009-2014 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#if defined(_WIN32)
#include "StDXNVSurface.h"
#include "StDXAqbsControl.h"
#include "StDXManager.h"

#include <StStrings/StLogger.h>
#include <StSys/StSys.h>

// NVIDIA Stereo Blit defines
#define NVSTEREO_IMAGE_SIGNATURE 0x4433564e // NV3D
typedef struct _Nv_Stereo_Image_Header {
    unsigned int dwSignature;
    unsigned int dwWidth;
    unsigned int dwHeight;
    unsigned int dwBPP;
    unsigned int dwFlags;
} NVSTEREOIMAGEHEADER;

// ORed flags in the dwFlags fiels of the _Nv_Stereo_Image_Header structure above
#define SIH_SWAP_EYES    0x00000001
#define SIH_SCALE_TO_FIT 0x00000002

StDXNVSurface::StDXNVSurface(const size_t theSizeX,
                             const size_t theSizeY)
: mySurfaceStereo(NULL),
  myTextureL(NULL),
  myTextureR(NULL),
  myTextureLShare(NULL),
  myTextureRShare(NULL),
  mySurfaceL(NULL),
  mySurfaceR(NULL),
  mySizeX(theSizeX),
  mySizeY(theSizeY) {
    // back buffer dimensions should be equal to surface half
    myDstRect.left   = 0;
    myDstRect.right  = (LONG )theSizeX;
    myDstRect.top    = 0;
    myDstRect.bottom = (LONG )theSizeY;

    // left view
    mySrcRectL.left   = 0;
    mySrcRectL.right  = (LONG )theSizeX;
    mySrcRectL.top    = 0;
    mySrcRectL.bottom = (LONG )theSizeY;

    // right view
    mySrcRectR.left   = (LONG )theSizeX;
    mySrcRectR.right  = (LONG )theSizeX * 2;
    mySrcRectR.top    = 0;
    mySrcRectR.bottom = (LONG )theSizeY;

    // those sizes only used for mono (stereo driver off / windowed;
    // otherwise NVIDIA driver will ignore them and will use header in surface itself)
    // we should used downsized values to prevent render fail

    // only one view shown if we set width/2 here as for WinXP way
    //mySrcRect.left = 0; mySrcRect.right = (LONG )theSizeX; // WinXP
    mySrcRect.left = 0; mySrcRect.right  = (LONG )theSizeX * 2;
    mySrcRect.top  = 0; mySrcRect.bottom = (LONG )theSizeY;
}

StDXNVSurface::~StDXNVSurface() {
    release();
}

void StDXNVSurface::release() {
    if(mySurfaceL != NULL) {
        mySurfaceL->Release();
        mySurfaceL = NULL;
    }
    if(myTextureL != NULL) {
        myTextureL->Release();
        myTextureL      = NULL;
        myTextureLShare = NULL;
    }
    if(mySurfaceR != NULL) {
        mySurfaceR->Release();
        mySurfaceR = NULL;
    }
    if(myTextureR != NULL) {
        myTextureR->Release();
        myTextureR      = NULL;
        myTextureRShare = NULL;
    }
    if(mySurfaceStereo != NULL) {
        mySurfaceStereo->Release();
        mySurfaceStereo = NULL;
    }
    myAqbsControl.nullify();
}

bool StDXNVSurface::create(StDXManager& theD3d,
                           const bool   theHasWglDx) {
    // connect to AQBS if available
    myAqbsControl = new StDXAqbsControl(theD3d.getDevice());

    // create independent sharable textures for left/right views
    D3DFORMAT aFormat = D3DFMT_A8R8G8B8;
    if(theHasWglDx) {
        if(theD3d.getDevice()->CreateTexture((UINT )mySizeX, (UINT )mySizeY, 0, 0,
                                             aFormat, D3DPOOL_DEFAULT,
                                             &myTextureL, &myTextureLShare) != D3D_OK
        || myTextureL->GetSurfaceLevel(0, &mySurfaceL) != D3D_OK) {
            release();
            return false;
        }
        if(theD3d.getDevice()->CreateTexture((UINT )mySizeX, (UINT )mySizeY, 0, 0,
                                             aFormat, D3DPOOL_DEFAULT,
                                             &myTextureR, &myTextureRShare) != D3D_OK
        || myTextureR->GetSurfaceLevel(0, &mySurfaceR) != D3D_OK) {
            release();
            return false;
        }
    }

    // create special lockable surface for 3D-Vision hack
    if(theD3d.getDevice()->CreateRenderTarget((UINT )mySizeX * 2, (UINT )mySizeY + 1, // the extra row is for the stereo header
                                              aFormat, D3DMULTISAMPLE_NONE, 0, TRUE,  // must be lockable to write header
                                              &mySurfaceStereo, NULL) != D3D_OK) {
        release();
        return false;
    }

    ST_DEBUG_LOG("Direct3D9, Created StDXNVSurface (WxH= " + mySizeX + "x"+ mySizeY + ")");

    // write stereo signature in the last raw of the stereo image
    D3DLOCKED_RECT aLockedRect;
    mySurfaceStereo->LockRect(&aLockedRect, NULL, 0);
    stUByte_t* aData = (stUByte_t* )aLockedRect.pBits;
    stMemZero(&aData[0], 4 * mySizeY * mySizeX);
    NVSTEREOIMAGEHEADER* aNvHeader = (NVSTEREOIMAGEHEADER* )(aData + aLockedRect.Pitch * mySizeY);
    aNvHeader->dwSignature = NVSTEREO_IMAGE_SIGNATURE;
    aNvHeader->dwBPP       = 32;
    aNvHeader->dwFlags     = 0;
    aNvHeader->dwWidth     = (unsigned int )mySizeX * 2;
    aNvHeader->dwHeight    = (unsigned int )mySizeY;
    mySurfaceStereo->UnlockRect();
    return true;
}

void StDXNVSurface::update(const size_t     theSrcSizeX,
                           const size_t     theSrcSizeY,
                           const stUByte_t* theSrcData,
                           bool             theIsLeft) {
    if(mySurfaceStereo == NULL) {
        return;
    }

    size_t aBytesOutLine  = mySizeX * 2 * 4;
    size_t aBytesOutDispl = theIsLeft ? (aBytesOutLine / 2) : 0; // cross-eyed for SIH_SWAP_EYES flag
    if(mySizeX * 2 > theSrcSizeX * 2) {
        aBytesOutDispl += 4 * ((mySizeX - theSrcSizeX) / 2);
    }
    size_t aBytesSrcLine  = theSrcSizeX * 4;
    size_t aBytesSrcDispl = (mySizeX < (2 * theSrcSizeX)) ? (4 * ((theSrcSizeX - mySizeX) / 2)) : 0;
    size_t aCopyHeight    = mySizeY     <= theSrcSizeY ? mySizeY : theSrcSizeY;
    size_t aCopyBytes     = mySizeX * 2 <= theSrcSizeX ? (aBytesOutLine / 2) : aBytesSrcLine;
    D3DLOCKED_RECT aLockedRect;
    mySurfaceStereo->LockRect(&aLockedRect, NULL, 0);
        unsigned char* aDstData = (unsigned char* )aLockedRect.pBits;
        if(mySizeY == theSrcSizeY) {
            for(size_t aRow = 0; aRow < aCopyHeight; ++aRow) {
                stMemCpy(&aDstData  [               aRow      * aBytesOutLine + aBytesOutDispl],
                         &theSrcData[(aCopyHeight - aRow - 1) * aBytesSrcLine + aBytesSrcDispl], aCopyBytes);
            }
        } else {
            double yy = double(mySizeY) / double(theSrcSizeY);
            for(size_t aRow = 0; aRow < theSrcSizeY; ++aRow) {
                stMemCpy(&aDstData  [size_t(double(aRow) * yy)  * aBytesOutLine + aBytesOutDispl],
                         &theSrcData[(theSrcSizeY   - aRow - 1) * aBytesSrcLine + aBytesSrcDispl], aCopyBytes);
            }
        }
    mySurfaceStereo->UnlockRect(); // unlock surface
}

void StDXNVSurface::render(StDXManager& theD3d) {
    if(mySurfaceStereo == NULL) {
        return;
    }

    // get the backbuffer and copy surface into it
    IDirect3DSurface9* aBackbuffer = NULL;
    theD3d.getDevice()->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &aBackbuffer);

    if(myAqbsControl->isValid()) {
        if(!myAqbsControl->setDstLeft()) {
            ST_DEBUG_LOG("AQBS set LEFT error");
        }

        if(mySurfaceL != NULL) {
            theD3d.getDevice()->StretchRect(mySurfaceL, NULL, aBackbuffer, NULL, D3DTEXF_LINEAR);
        } else {
            theD3d.getDevice()->StretchRect(mySurfaceStereo, &mySrcRectL, aBackbuffer, &myDstRect, D3DTEXF_LINEAR);
        }

        if(!myAqbsControl->setDstRight()) {
            ST_DEBUG_LOG("AQBS set RIGHT error");
        }

        if(mySurfaceR != NULL) {
            theD3d.getDevice()->StretchRect(mySurfaceR, NULL, aBackbuffer, NULL, D3DTEXF_LINEAR);
        } else {
            theD3d.getDevice()->StretchRect(mySurfaceStereo, &mySrcRectR, aBackbuffer, &myDstRect, D3DTEXF_LINEAR);
        }
    } else {
        if(mySurfaceL != NULL) {
            theD3d.getDevice()->StretchRect(mySurfaceL, NULL, mySurfaceStereo, &mySrcRectL, D3DTEXF_NONE);
            theD3d.getDevice()->StretchRect(mySurfaceR, NULL, mySurfaceStereo, &mySrcRectR, D3DTEXF_NONE);
        }
        theD3d.getDevice()->StretchRect(mySurfaceStereo, &mySrcRect, aBackbuffer, &myDstRect, D3DTEXF_LINEAR);
    }

    aBackbuffer->Release();
}

#endif // _WIN32
