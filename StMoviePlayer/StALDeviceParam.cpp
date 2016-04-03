/**
 * Copyright © 2016 Kirill Gavrilov <kirill@sview.ru>
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

#include "StALDeviceParam.h"

#include "StVideo/StALContext.h"

namespace {

#ifdef _WIN32
    inline void stFromLocaleOrUtf8(StString&   theStrResult,
                                   const char* theStrInput) {
        int aWideSize = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, theStrInput, -1, NULL, 0);
        if(aWideSize == 0) {
            theStrResult.fromLocale(theStrInput);
            return;
        }

        wchar_t* aWideBuffer = new wchar_t[aWideSize + 1];
        MultiByteToWideChar(CP_UTF8, 0, theStrInput, -1, aWideBuffer, aWideSize);
        aWideBuffer[aWideSize] = L'\0';
        theStrResult.fromUnicode(aWideBuffer);
        delete[] aWideBuffer;
    }
#endif

}

StALDeviceParam::StALDeviceParam()
: StInt32ParamNamed(0, stCString("alDevice"), stCString("OpenAL Device")) {
    initList();
}

void StALDeviceParam::initList() {
    myValue = 0;
    myDevicesLoc.clear();
    myDevicesUtf.clear();
    StString aName;
    if(alcIsExtensionPresent(NULL, "ALC_ENUMERATE_ALL_EXT") != AL_TRUE) {
        // ancient OpenAL implementations (like from apples) support only single device
        const ALchar* aDefDevice = alcGetString(NULL, ALC_DEFAULT_DEVICE_SPECIFIER);
    #ifdef _WIN32
        stFromLocaleOrUtf8(aName, aDefDevice);
    #else
        aName.fromUnicode(aDefDevice);
    #endif
        myDevicesUtf.add(aName);
        myDevicesLoc.push_back(std::string(aDefDevice));
        return;
    }

    const StString THE_ALSOFT_SUFFIX(" on OpenAL Soft");
    const ALchar* aDevicesNames = alcGetString(NULL, ALC_ALL_DEVICES_SPECIFIER);
    while(aDevicesNames != NULL
      && *aDevicesNames != '\0') {
        std::string aCName(aDevicesNames);
    #ifdef _WIN32
        stFromLocaleOrUtf8(aName, aDevicesNames);
    #else
        aName.fromUnicode(aDevicesNames);
    #endif
        // cut-off redundant suffixes - the names are too long
        if(aName.isEndsWithIgnoreCase(THE_ALSOFT_SUFFIX)) {
            size_t anEnd = aName.getLength() - THE_ALSOFT_SUFFIX.getLength();
            aName = aName.subString(0, anEnd);
        }

        myDevicesUtf.add(aName);
        myDevicesLoc.push_back(aCName);
        aDevicesNames += aCName.length() + 1;
    }
    if(myDevicesUtf.isEmpty()) {
        // append dummy device
        myDevicesUtf.add("None");
        myDevicesLoc.push_back("");
    }
}

StALDeviceParam::~StALDeviceParam() {}

int32_t StALDeviceParam::getValueFromName(const StString& theName) {
    for(size_t anId = 0; anId < myDevicesUtf.size(); ++anId) {
        if(myDevicesUtf[anId] == theName) {
            return int32_t(anId);
        }
    }
    return -1;
}

bool StALDeviceParam::init(const StString& theActive) {
    myValue = getValueFromName(theActive);
    return myValue >= 0;
}

StString StALDeviceParam::getUtfTitle() const {
    if(myDevicesUtf.isEmpty()) {
        return StString();
    }

    int32_t anActive = getValue();
    return myDevicesUtf[(anActive >= 0 && size_t(anActive) < myDevicesUtf.size()) ? size_t(anActive) : 0];
}

std::string StALDeviceParam::getCTitle() const {
    if(myDevicesUtf.isEmpty()) {
        return std::string();
    }

    int32_t anActive = getValue();
    return myDevicesLoc[(anActive >= 0 && size_t(anActive) < myDevicesUtf.size()) ? size_t(anActive) : 0];
}
