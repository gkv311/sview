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

#if(defined(_WIN32) || defined(__WIN32__))
#include "StDXNVSurface.h"
#include "StDXAqbsControl.h"

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
: mySurface(NULL),
  myAqbsControl(NULL),
  mySizeX(theSizeX),
  mySizeY(theSizeY) {
    // back buffer dimensions should be equal to surface half
    myDstRect.left   = 0;
    myDstRect.right  = (LONG )mySizeX / 2;
    myDstRect.top    = 0;
    myDstRect.bottom = (LONG )mySizeY;

    // cross-eyed, right view
    mySrcRectR.left   = 0;
    mySrcRectR.right  = (LONG )mySizeX / 2;
    mySrcRectR.top    = 0;
    mySrcRectR.bottom = (LONG )mySizeY;

    // cross-eyed, left view
    mySrcRectL.left   = (LONG )mySizeX / 2;
    mySrcRectL.right  = (LONG )mySizeX;
    mySrcRectL.top    = 0;
    mySrcRectL.bottom = (LONG )mySizeY;

    // those sizes only used for mono (stereo driver off / windowed;
    // otherwise NVIDIA driver will ignore them and will use header in surface itself)
    // we should used downsized values to prevent render fail
    if(StSys::isVistaPlus()) {
        // only one view showed if we set width/2 here as for WinXP way
        mySrcRect.left = 0; mySrcRect.right = (LONG )mySizeX;
    } else {
        // picture showed with wrong ratio (width/4) in other (like Vista+) way
        mySrcRect.left = 0; mySrcRect.right = (LONG )mySizeX / 2;
    }
    mySrcRect.top = 0; mySrcRect.bottom = (LONG )mySizeY;
}

StDXNVSurface::~StDXNVSurface() {
    if(mySurface != NULL) {
        mySurface->Release();
        mySurface = NULL;
    }
    if(myAqbsControl != NULL) {
        delete myAqbsControl;
        myAqbsControl = NULL;
    }
}

bool StDXNVSurface::create(IDirect3DDevice9* theD3dDevice) {
    // connect to AQBS if available
    myAqbsControl = new StDXAqbsControl(theD3dDevice);

    // create the Off Screen Surface
    HRESULT hResult = theD3dDevice->CreateOffscreenPlainSurface(
        (UINT )mySizeX,     // stereo width is twice the source width
        (UINT )mySizeY + 1, // stereo height add one raw to encode signature
        D3DFMT_A8R8G8B8,    // surface format, D3DFMT_A8R8G8B8 is a 32 bit format with 8 alpha bits
        D3DPOOL_DEFAULT,    // create it in the default memory pool
        &mySurface,
        NULL
    );
    if(hResult != D3D_OK) {
        return false;
    }
    ST_DEBUG_LOG("Direct3D9, Created StDXNVSurface (WxH= " + mySizeX + "x"+ mySizeY + ")");

    D3DLOCKED_RECT aLockedRect;
    mySurface->LockRect(&aLockedRect, NULL, 0);
    unsigned char* aData = (unsigned char* )aLockedRect.pBits;

    // set black color
    stMemSet(&aData[0], 0, 4 * mySizeY * mySizeX);

    // write stereo signature in the last raw of the stereo image
    NVSTEREOIMAGEHEADER* aNvHeader
        = (NVSTEREOIMAGEHEADER* )(((unsigned char* )aLockedRect.pBits) + (aLockedRect.Pitch * mySizeY));
    // update the signature header values
    aNvHeader->dwSignature = NVSTEREO_IMAGE_SIGNATURE;
    aNvHeader->dwBPP       = 32;
    aNvHeader->dwFlags     = SIH_SWAP_EYES;          // Src image has left on left and right on right
    aNvHeader->dwWidth     = (unsigned int )mySizeX; // full side by side width here
    aNvHeader->dwHeight    = (unsigned int )mySizeY; // image height without NV header

    mySurface->UnlockRect(); // unlock surface

    return true;
}

void StDXNVSurface::update(const size_t theSrcSizeX, const size_t theSrcSizeY,
                           const unsigned char* theSrcData, bool isLeft) {
    if(mySurface == NULL) {
        return;
    }

    size_t aBytesOutLine = mySizeX * 4;
    size_t aBytesOutDispl = isLeft ? (aBytesOutLine / 2) : 0; // cross-eyed for SIH_SWAP_EYES flag
    if((mySizeX > (2 * theSrcSizeX))) {
        aBytesOutDispl += 4 * ((mySizeX / 2 - theSrcSizeX) / 2);
    }
    size_t aBytesSrcLine = theSrcSizeX * 4;
    size_t aBytesSrcDispl = (mySizeX < (2 * theSrcSizeX)) ? (4 * ((theSrcSizeX - mySizeX / 2) / 2)) : 0;
    size_t aCopyHeight = mySizeY <= theSrcSizeY ? mySizeY : theSrcSizeY;
    size_t aCopyBytes = mySizeX <= theSrcSizeX ? (aBytesOutLine / 2) : aBytesSrcLine;
    D3DLOCKED_RECT aLockedRect;
    mySurface->LockRect(&aLockedRect, NULL, 0);
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
    mySurface->UnlockRect(); // unlock surface
}

void StDXNVSurface::render(IDirect3DDevice9* theD3dDevice) {
    if(mySurface == NULL) {
        return;
    }

    // gets the backbuffer and copys our surface onto it
    IDirect3DSurface9* aBackbuffer = NULL;
    theD3dDevice->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &aBackbuffer);

    if(myAqbsControl->isValid()) {
        if(!myAqbsControl->setDstLeft()) {
            ST_DEBUG_LOG("AQBS set LEFT error");
        }
        theD3dDevice->StretchRect(mySurface, &mySrcRectL, aBackbuffer, &myDstRect, D3DTEXF_LINEAR);

        if(!myAqbsControl->setDstRight()) {
            ST_DEBUG_LOG("AQBS set RIGHT error");
        }
        theD3dDevice->StretchRect(mySurface, &mySrcRectR, aBackbuffer, &myDstRect, D3DTEXF_LINEAR);
    } else {
        theD3dDevice->StretchRect(mySurface, &mySrcRect,  aBackbuffer, &myDstRect, D3DTEXF_LINEAR);
    }

    aBackbuffer->Release();
}

#endif // _WIN32
