/**
 * Copyright © 2009-2025 Kirill Gavrilov <kirill@sview.ru>
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
#include <StThreads/StProcess.h>

#ifndef ALC_CONNECTED
    #define ALC_CONNECTED 0x313 // undefined on mac os
#endif

namespace {
    /**
     * Return string for ALC_OUTPUT_MODE_SOFT mode.
     */
    const char* outputModeToString(ALCint theMode) {
        switch(theMode) {
            case ALC_ANY_SOFT:          return "Auto";
            case ALC_MONO_SOFT:         return "Mono";
            case ALC_STEREO_SOFT:       return "Stereo (unspecified)";
            case ALC_STEREO_BASIC_SOFT: return "Stereo (basic)";
            case ALC_STEREO_UHJ_SOFT:   return "Stereo (UHJ)";
            case ALC_STEREO_HRTF_SOFT:  return "Stereo (HRTF)";
            case ALC_QUAD_SOFT:         return "Quadraphonic";
            case ALC_SURROUND_5_1_SOFT: return "5.1 Surround";
            case ALC_SURROUND_6_1_SOFT: return "6.1 Surround";
            case ALC_SURROUND_7_1_SOFT: return "7.1 Surround";
        }
        return "UNKNOWN";
    }

    /**
     * Return string for the status.
     */
    const char* hrtfStatusToString(ALCint theStatus) {
        switch(theStatus) {
            case ALC_HRTF_DISABLED_SOFT:            return "Disabled";
            case ALC_HRTF_ENABLED_SOFT:             return "Enabled";
            case ALC_HRTF_DENIED_SOFT:              return "Denied";
            case ALC_HRTF_REQUIRED_SOFT:            return "Required";
            case ALC_HRTF_HEADPHONES_DETECTED_SOFT: return "Headphones detected";
            case ALC_HRTF_UNSUPPORTED_FORMAT_SOFT:  return "Unsupported format";
        }
        return "UNKNOWN";
    }

    /**
     * Initialize OpenAL resources.
     */
    bool stalGlobalInit() {
    #ifdef __APPLE__
        static bool isFirstCall = true;
        if(!isFirstCall) {
            return true;
        }
        isFirstCall = false;

        // append sView resources folder to OpenAL search path
        // so that we can put default hrtf tables into OS X application bundle
        const StString anStResFolder = StProcess::getStShareFolder();
        StString aDataDirs = StProcess::getEnv("XDG_DATA_DIRS");
        if(aDataDirs.isEmpty()) {
            aDataDirs = "/usr/local/share/:/usr/share/";
        }
        aDataDirs = aDataDirs + ":" + anStResFolder;
        StProcess::setEnv("XDG_DATA_DIRS", aDataDirs);
    #endif
        return true;
    }
}

StALContext::StALContext()
: hasExtEAX2(false),
  hasExtFloat32(false),
  hasExtFloat64(false),
  hasExtMultiChannel(false),
  hasExtBFormat(false),
  hasExtDisconnect(false),
  hasExtSoftOutMode(false),
  hasExtSoftHrtf(false),
  alcGetStringiSOFT(NULL),
  alcResetDeviceSOFT(NULL),
  myAlDevice(NULL),
  myAlContext(NULL) {
    stalGlobalInit();
}

StALContext::~StALContext() {
    destroy();
}

StString StALContext::toStringExtensions() const {
    StString anExtList = "OpenAL extensions: ";
    if(hasExtEAX2)        { anExtList += "EAX2.0 "; }
    if(hasExtFloat32)     { anExtList += "float32 "; }
    if(hasExtFloat64)     { anExtList += "float64 "; }
    if(hasExtMultiChannel){ anExtList += "multi-channel "; }
    if(hasExtBFormat)     { anExtList += "B-Format "; }
    if(hasExtDisconnect)  { anExtList += "ALC_EXT_disconnect "; }
    if(hasExtSoftOutMode) {
        ALCint anOutMode = ALC_ANY_SOFT;
        alcGetIntegerv(myAlDevice, ALC_OUTPUT_MODE_SOFT, 1, &anOutMode);
        anExtList += StString("ALC_SOFT_output_mode [") + outputModeToString(anOutMode) + "] ";
    }
    if(hasExtSoftHrtf) {
        ALCint aHrtfStatus = ALC_HRTF_DISABLED_SOFT;
        alcGetIntegerv(myAlDevice, ALC_HRTF_STATUS_SOFT, 1, &aHrtfStatus);

        anExtList += StString("ALC_SOFT_HRTF [");
        anExtList += hrtfStatusToString(aHrtfStatus);

        const StString aSpec = (alcGetString(myAlDevice, ALC_HRTF_SPECIFIER_SOFT));
        if (!aSpec.isEmpty()) {
            anExtList += StString("; specifier: '") + aSpec + "'";
        }

        ALCint aNbHrtfSpecs = 0;
        alcGetIntegerv(myAlDevice, ALC_NUM_HRTF_SPECIFIERS_SOFT, 1, &aNbHrtfSpecs);\
        if (aNbHrtfSpecs != 0) {
            anExtList += StString("; specifiers: ");
            for (ALCint aSpecIter = 0; aSpecIter < aNbHrtfSpecs; ++aSpecIter) {
                if (aSpecIter != 0) { anExtList += ","; }
                anExtList += StString("'") + alcGetStringiSOFT(myAlDevice, ALC_HRTF_SPECIFIER_SOFT, aSpecIter) + "'";
            }
        }
        anExtList += "] ";
    }
    return anExtList;
}

bool StALContext::create(const std::string& theDeviceName) {
    if(theDeviceName.empty()) {
        // open default device
        myAlDevice = alcOpenDevice(NULL);
    } else {
        myAlDevice = alcOpenDevice(theDeviceName.c_str());
    }
    if(myAlDevice == NULL) {
        return false;
    }
    myAlContext = alcCreateContext(myAlDevice, NULL);
    makeCurrent();

    // check extensions
    hasExtEAX2         = alIsExtensionPresent("EAX2.0") == AL_TRUE;
    hasExtFloat32      = alIsExtensionPresent("AL_EXT_float32")   == AL_TRUE;
    hasExtFloat64      = alIsExtensionPresent("AL_EXT_double")    == AL_TRUE;
    hasExtMultiChannel = alIsExtensionPresent("AL_EXT_MCFORMATS") == AL_TRUE;
    hasExtBFormat      = alIsExtensionPresent("AL_EXT_BFORMAT")   == AL_TRUE;

    hasExtDisconnect   = alcIsExtensionPresent(myAlDevice, "ALC_EXT_disconnect") == AL_TRUE;
    hasExtSoftOutMode  = alcIsExtensionPresent(myAlDevice, "ALC_SOFT_output_mode") == AL_TRUE;
    if(alcIsExtensionPresent(myAlDevice, "ALC_SOFT_HRTF") == AL_TRUE) {
        alcGetStringiSOFT  = (alcGetStringiSOFT_t  )alcGetProcAddress(myAlDevice, "alcGetStringiSOFT");
        alcResetDeviceSOFT = (alcResetDeviceSOFT_t )alcGetProcAddress(myAlDevice, "alcResetDeviceSOFT");
        hasExtSoftHrtf = alcGetStringiSOFT  != NULL
                      && alcResetDeviceSOFT != NULL;
    }

    // debug info
    ST_DEBUG_LOG(toStringExtensions());
    return true;
}

void StALContext::fullInfo(StDictionary& theMap) const {
    StString anExtensions, aSoftOutput;
    StString aHrtfInfo, aHrtfState, aHrtfSpec, aHrtfSpecList;
    if(hasExtFloat32)     { anExtensions += "AL_EXT_float32 "; }
    if(hasExtFloat64)     { anExtensions += "AL_EXT_double "; }
    if(hasExtMultiChannel){ anExtensions += "AL_EXT_MCFORMATS "; }
    if(hasExtBFormat)     { anExtensions += "AL_EXT_BFORMAT "; }
    if(hasExtDisconnect)  { anExtensions += "ALC_EXT_disconnect "; }
    if(hasExtSoftOutMode) {
        anExtensions += "ALC_SOFT_output_mode ";

        ALCint anOutMode = ALC_ANY_SOFT;
        alcGetIntegerv(myAlDevice, ALC_OUTPUT_MODE_SOFT, 1, &anOutMode);
        aSoftOutput = outputModeToString(anOutMode);
    }
    if(hasExtSoftHrtf) {
        anExtensions += "ALC_SOFT_HRTF ";

        ALCint aHrtfStatus = ALC_HRTF_DISABLED_SOFT;
        alcGetIntegerv(myAlDevice, ALC_HRTF_STATUS_SOFT, 1, &aHrtfStatus);
        aHrtfState = hrtfStatusToString(aHrtfStatus);

        aHrtfSpec = alcGetString(myAlDevice, ALC_HRTF_SPECIFIER_SOFT);

        ALCint aNbHrtfSpecs = 0;
        alcGetIntegerv(myAlDevice, ALC_NUM_HRTF_SPECIFIERS_SOFT, 1, &aNbHrtfSpecs);\
        if (aNbHrtfSpecs != 0) {
            for (ALCint aSpecIter = 0; aSpecIter < aNbHrtfSpecs; ++aSpecIter) {
                if (aSpecIter != 0) { aHrtfSpecList += ", "; }
                aHrtfSpecList += StString("'") + alcGetStringiSOFT(myAlDevice, ALC_HRTF_SPECIFIER_SOFT, aSpecIter) + "'";
            }
        }

        aHrtfInfo = StString("State: ") + aHrtfState;
        if (!aHrtfSpec.isEmpty()) {
            aHrtfInfo += StString("\nSpecifier: '") + aHrtfSpec + "'";
        }
        if (aNbHrtfSpecs != 0) {
            aHrtfInfo += StString("\nSpecifiers: ") + aHrtfSpecList;
        }
    }

    ALCint aFreq = 0;
    alcGetIntegerv(myAlDevice, ALC_FREQUENCY, 1, &aFreq);
    if(alcGetError(myAlDevice) == ALC_NO_ERROR) {
        //
    }

    theMap.add(StDictEntry("ALvendor",    (const char* )alGetString(AL_VENDOR)));
    theMap.add(StDictEntry("ALrenderer",  (const char* )alGetString(AL_RENDERER)));
    theMap.add(StDictEntry("ALversion",   (const char* )alGetString(AL_VERSION)));
    if(aFreq > 0) {
        theMap.add(StDictEntry("Audio device sample rate", StString(aFreq) + " Hz"));
    }
    theMap.add(StDictEntry("OpenAL extensions", anExtensions));
    if(!aSoftOutput.isEmpty()) {
        theMap.add(StDictEntry("OpenAL output mode", aSoftOutput));
    }
    theMap.add(StDictEntry("OpenAL HRTF mixing", !aHrtfInfo.isEmpty() ? aHrtfInfo : "Not implemented"));
}

void StALContext::destroy() {
    alcMakeContextCurrent(NULL);
    if(myAlContext != NULL && myAlDevice != NULL) {
        alcDestroyContext(myAlContext);
        alcCloseDevice(myAlDevice);
    }
    myAlContext = NULL;
    myAlDevice = NULL;

    // remove extensions
    hasExtEAX2 = false;
    hasExtFloat32 = false;
    hasExtFloat64 = false;
    hasExtMultiChannel = false;
    hasExtBFormat      = false;
    hasExtDisconnect   = false;
    hasExtSoftOutMode  = false;
    hasExtSoftHrtf     = false;
    alcGetStringiSOFT  = NULL;
    alcResetDeviceSOFT = NULL;
}

bool StALContext::makeCurrent() {
    return alcMakeContextCurrent(myAlContext) == AL_TRUE;
}

bool StALContext::isConnected() const {
    if(myAlDevice == NULL) {
        return false;
    } else if(!hasExtDisconnect) {
        return true;
    }

    ALint aConnected = AL_FALSE;
    alcGetIntegerv(myAlDevice, ALC_CONNECTED, 1, &aConnected);
    return aConnected == AL_TRUE;
}
