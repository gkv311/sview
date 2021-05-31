/**
 * StOutPageFlip, class providing stereoscopic output for Shutter Glasses displays using StCore toolkit.
 * Copyright © 2011 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#if(defined(_WIN32) || defined(__WIN32__))

#include "StDXAqbsControl.h"

#include <StStrings/StLogger.h>

StDXAqbsControl::StDXAqbsControl(IDirect3DDevice9* theD3dDevice)
: myComSurface(NULL) {
    if(theD3dDevice->CreateOffscreenPlainSurface(10, 10, (D3DFORMAT )FOURCC_AQBS, D3DPOOL_DEFAULT, &myComSurface, NULL) < 0) {
        myComSurface = NULL;
    }
}

StDXAqbsControl::~StDXAqbsControl() {
    if(myComSurface != NULL) {
        ULONG aRefCount = 1;
        while(aRefCount != 0) {
            aRefCount = myComSurface->Release();
        }
        myComSurface = 0;
    }
}

bool StDXAqbsControl::sendCommand(ATIDX9STEREOCOMMAND theCommand,
                                  BYTE* theOutBuffer, DWORD theOutBufferSize,
                                  BYTE* theInBuffer,  DWORD theInBufferSize) {
    if(myComSurface == NULL
    || (theOutBuffer != NULL && theOutBufferSize == 0)
    || (theInBuffer  != NULL && theInBufferSize  == 0)) {
        ST_DEBUG_LOG("Invalid call to StDXAqbsControl::sendCommand()!");
        return false;
    }

    D3DLOCKED_RECT aLockedRect;
    if(myComSurface->LockRect(&aLockedRect, 0, 0)) {
        return false;
    }
    HRESULT isOK = D3D_OK;
    ATIDX9STEREOCOMMPACKET* aCommPacket = (ATIDX9STEREOCOMMPACKET* )aLockedRect.pBits;
    aCommPacket->dwSignature     = 0x53544552; // STER
    aCommPacket->pResult         = &isOK;
    aCommPacket->stereoCommand   = theCommand;
    aCommPacket->pOutBuffer      = theOutBuffer;
    aCommPacket->dwOutBufferSize = theOutBufferSize;
    aCommPacket->pInBuffer       = theInBuffer;
    aCommPacket->dwInBufferSize  = theInBufferSize;
    myComSurface->UnlockRect();
    return isOK == D3D_OK;
}

bool StDXAqbsControl::enableStereo() {
    return sendCommand(ATI_STEREO_ENABLESTEREO);
}

bool StDXAqbsControl::setDstLeft() {
    if(myComSurface == NULL) {
        return false;
    }
    DWORD anEye = ATI_STEREO_LEFTEYE;
    return sendCommand(ATI_STEREO_SETDSTEYE, NULL, 0, (BYTE* )&anEye, sizeof(anEye));
}

bool StDXAqbsControl::setDstRight() {
    if(myComSurface == NULL) {
        return false;
    }
    DWORD anEye = ATI_STEREO_RIGHTEYE;
    return sendCommand(ATI_STEREO_SETDSTEYE, NULL, 0, (BYTE* )&anEye, sizeof(anEye));
}

#endif //_WIN32
