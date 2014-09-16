/**
 * Copyright © 2009-2014 Kirill Gavrilov <kirill@sview.ru>
 *
 * StMoviePlayer program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * StMoviePlayer program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program.
 * If not, see <http://www.gnu.org/licenses/>.
 */

#include "StALContext.h"
#include <StStrings/StLogger.h>

#ifndef ALC_CONNECTED
    #define ALC_CONNECTED 0x313 // undefined on mac os
#endif

StALContext::StALContext()
: hasExtEAX2(false),
  hasExtFloat32(false),
  hasExtFloat64(false),
  hasExtMultiChannel(false),
  hasExtDisconnect(false),
  hDevice(NULL),
  hContext(NULL) {
    //
}

StALContext::~StALContext() {
    destroy();
}

StString StALContext::toStringExtensions() const {
    StString extList = "OpenAL extensions:\n";
    if(hasExtEAX2) {
        extList += " - EAX2.0;\n";
    }
    if(hasExtFloat32) {
        extList += " - float32 mono/stereo formats;\n";
    }
    if(hasExtFloat64) {
        extList += " - float64 mono/stereo formats;\n";
    }
    if(hasExtMultiChannel) {
        extList += " - multi-channel formats;\n";
    }
    if(hasExtDisconnect) {
        extList += " - ALC_EXT_disconnect;\n";
    }
    return extList;
}

bool StALContext::create(const StString& theDeviceName) {
    if(theDeviceName.isEmpty()) {
        // open default device
        hDevice = alcOpenDevice(NULL);
    } else {
    #if defined(_WIN32)
        char aBuffer[ST_MAX_PATH];
        theDeviceName.toLocale(aBuffer, ST_MAX_PATH);
        hDevice = alcOpenDevice(aBuffer);
    #else
        hDevice = alcOpenDevice(theDeviceName.toCString());
    #endif
    }
    if(hDevice == NULL) {
        return false;
    }
    hContext = alcCreateContext(hDevice, NULL);
    makeCurrent();

    // check extensions
    hasExtEAX2         = alIsExtensionPresent("EAX2.0") == AL_TRUE;
#if !defined(__ANDROID__) // disable extensions broken on Android port
    hasExtFloat32      = alIsExtensionPresent("AL_EXT_float32")   == AL_TRUE;
    hasExtFloat64      = alIsExtensionPresent("AL_EXT_double")    == AL_TRUE;
#endif
    hasExtMultiChannel = alIsExtensionPresent("AL_EXT_MCFORMATS") == AL_TRUE;
    hasExtDisconnect   = alcIsExtensionPresent(hDevice, "ALC_EXT_disconnect") == AL_TRUE;

    // debug info
    ST_DEBUG_LOG(toStringExtensions());

    return true;
}

void StALContext::destroy() {
    alcMakeContextCurrent(NULL);
    if(hContext != NULL && hDevice != NULL) {
        alcDestroyContext(hContext);
        alcCloseDevice(hDevice);
    }
    hContext = NULL;
    hDevice = NULL;

    // remove extensions
    hasExtEAX2 = false;
    hasExtFloat32 = false;
    hasExtFloat64 = false;
    hasExtMultiChannel = false;
    hasExtDisconnect = false;
}

bool StALContext::makeCurrent() {
    return alcMakeContextCurrent(hContext) == AL_TRUE;
}

bool StALContext::isConnected() const {
    if(hDevice == NULL) {
        return false;
    } else if(!hasExtDisconnect) {
        return true;
    }

    ALint aConnected = AL_FALSE;
    alcGetIntegerv(hDevice, ALC_CONNECTED, 1, &aConnected);
    return aConnected == AL_TRUE;
}
