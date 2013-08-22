/**
 * Copyright © 2009-2013 Kirill Gavrilov <kirill@sview.ru>
 *
 * StCore library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * StCore library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.
 * If not, see <http://www.gnu.org/licenses/>.
 */

#include <StCore/StSearchMonitors.h>

#include <StSettings/StSettings.h>
#include <StThreads/StMutex.h>
#include <StThreads/StTimer.h>

#ifdef _WIN32
    #include <wnt/nvapi.h> // NVIDIA API
#elif (defined(__linux__) || defined(__linux))
    #include "StWinHandles.h"
    #include <X11/extensions/Xrandr.h>
#endif

#include "StADLsdk.h"

StSearchMonitors::StSearchMonitors()
: StArrayList<StMonitor>(2) {
    //
}

StSearchMonitors::~StSearchMonitors() {
    //
}

StMonitor& StSearchMonitors::operator[](const StPointI_t& thePoint) {
    for(size_t id = 0; id < size(); ++id) {
        if(getValue(id).getVRect().isPointIn(thePoint)) {
            return changeValue(id);
        }
    }
    return changeValue(0); // return first anyway...
}

const StMonitor& StSearchMonitors::operator[](const StPointI_t& thePoint) const {
    for(size_t id = 0; id < size(); ++id) {
        if(getValue(id).getVRect().isPointIn(thePoint)) {
            return getValue(id);
        }
    }
    return getValue(0); // return first anyway...
}

/**
 * Detect classic multimonitor configurations.
 */
void StSearchMonitors::findMonitorsBlind(const int rootX, const int rootY) {
    StRectI_t rect0;
    StRectI_t rect1;
    bool hasTwo = true;
    if(rootX == 3360 && rootY == 1200) {
        rect0.setValues(0,  1050,    0, 1680);
        rect1.setValues(0,  1200, 1680, 3280);
    } else if(rootX == 3200) {
        rect0.setValues(0, rootY,    0, 1600);
        rect1.setValues(0, rootY, 1600, 3200);
    } else if(rootX == 3360) {
        rect0.setValues(0, rootY,    0, 1680);
        rect1.setValues(0, rootY, 1680, 3360);
    } else if(rootX == 2560 && rootY == 1024) {
        rect0.setValues(0, rootY,    0, 1280);
        rect1.setValues(0, rootY, 1280, 2560);
    } else if(rootX == 3840) {
        rect0.setValues(0, rootY,    0, 1920);
        rect1.setValues(0, rootY, 1920, 3840);
    } else if(rootX == 5120) {
        rect0.setValues(0, rootY,    0, 2560),
        rect1.setValues(0, rootY, 2560, 5120);
    } else {
        hasTwo = false;
    }
    if(hasTwo) {
        // classic dual monitor configuration detected
        StMonitor stMon1;
        StMonitor stMon2;
        stMon1.setVRect(rect0); stMon1.setId(0);
        stMon2.setVRect(rect1); stMon2.setId(1);
        add(stMon1);
        add(stMon2);
    } else {
        // setup just one display
        StMonitor stMon;
        rect0.setValues(0, rootY, 0, rootX);
        stMon.setVRect(rect0);
        stMon.setId(0);
        add(stMon);
    }
}

#ifdef _WIN32
void StSearchMonitors::findMonitorsWinAPI() {
    int monCount = 0;

    // retrieve global scale factor (deprecated since Win 8.1)
    float aScale = 1.0f;
    HDC aDeskCtx = GetDC(NULL);
    if(aDeskCtx != NULL) {
        const int aDpiX = GetDeviceCaps(aDeskCtx, LOGPIXELSX);
        ReleaseDC(NULL, aDeskCtx);
        aScale = float(aDpiX) / 96.0f; // 96 is 100% in Windows
    }

    // load HiDPI API for Win8.1+
    typedef HRESULT (WINAPI *GetScaleFactorForMonitor_t)(HMONITOR   theMon,
                                                         int*       theScale);
    typedef HRESULT (WINAPI *RegisterScaleChangeEvent_t)(HANDLE     theEvent,
                                                         DWORD_PTR* theCookie);
    typedef HRESULT (WINAPI *UnregisterScaleChangeEvent_t)(DWORD_PTR theCookie);
    HMODULE aShcoreLib = GetModuleHandleW(L"Shcore");
    GetScaleFactorForMonitor_t aMonScaleFunc = NULL;
    if(aShcoreLib != NULL) {
        aMonScaleFunc = (GetScaleFactorForMonitor_t )GetProcAddress(aShcoreLib, "GetScaleFactorForMonitor");
    }

    // collect system and monitor information, and display it using a message box
    DISPLAY_DEVICEW dispDevice; dispDevice.cb = sizeof(dispDevice);
    DWORD dev = 0;  // device index
    int monId = 1;  // monitor number, as used by Display Properties > Settings
    for(; EnumDisplayDevicesW(NULL, dev, &dispDevice, 0); ++dev, ++monId) {
        if((dispDevice.StateFlags & DISPLAY_DEVICE_MIRRORING_DRIVER)) {
            // ignore virtual mirror displays
            continue;
        }
        // get information about the monitor attached to this display adapter;
        DISPLAY_DEVICEW ddMon; ZeroMemory(&ddMon, sizeof(ddMon)); ddMon.cb = sizeof(ddMon);
        DWORD devMon = 0;

        // please note that this enumeration may not return the correct monitor if multiple monitors
        // are attached; this is because not all display drivers return the ACTIVE flag for the monitor
        // that is actually active
        while(EnumDisplayDevicesW(dispDevice.DeviceName, devMon, &ddMon, 0)) {
            //$DISPLAY_DEVICE_ACTIVE = 0x00000001;
            if(ddMon.StateFlags & 0x00000001) {
                break;
            }
            ++devMon;
        }

        if(*ddMon.DeviceString == L'\0') {
            EnumDisplayDevicesW(dispDevice.DeviceName, 0, &ddMon, 0);
            if(*ddMon.DeviceString == L'\0') {
                lstrcpy(ddMon.DeviceString, L"Default Monitor");
            }
        }

        // get information about the display's position and the current display mode
        DEVMODE dm; ZeroMemory(&dm, sizeof(dm)); dm.dmSize = sizeof(dm);
        if(EnumDisplaySettingsExW(dispDevice.DeviceName, ENUM_CURRENT_SETTINGS, &dm, 0) == FALSE) {
            EnumDisplaySettingsExW(dispDevice.DeviceName, ENUM_REGISTRY_SETTINGS, &dm, 0);
        }

        // get the monitor handle and workspace
        HMONITOR hMonitor = NULL;
        MONITORINFO monInfo; ZeroMemory(&monInfo, sizeof(monInfo)); monInfo.cbSize = sizeof(monInfo);
        if(dispDevice.StateFlags & DISPLAY_DEVICE_ATTACHED_TO_DESKTOP) {
            // display is enabled, only enabled displays have a monitor handle
            POINT pt = {dm.dmPosition.x, dm.dmPosition.y};
            hMonitor = MonitorFromPoint(pt, MONITOR_DEFAULTTONULL);
            if(hMonitor != NULL) {
                GetMonitorInfo(hMonitor, &monInfo);
            }
        }

        if(hMonitor == NULL) {
            continue;
        }
        StMonitor aMon;

        if(aMonScaleFunc != NULL) {
            int aScalePercents = 100;
            aMonScaleFunc(hMonitor, &aScalePercents);
            aScale = float(aScalePercents) * 0.01f;
        }
        aMon.setScale(aScale);

        // status flags: primary, disabled, removable
        bool isPrimary = false;
        if(!(dispDevice.StateFlags & DISPLAY_DEVICE_ATTACHED_TO_DESKTOP)) {
            ///aMon.setDisabled(true);
        } else if(dispDevice.StateFlags & DISPLAY_DEVICE_PRIMARY_DEVICE) {
            isPrimary = true;
        }
        if(dispDevice.StateFlags & DISPLAY_DEVICE_REMOVABLE) {
            ///removable
        }

        StRectI_t stRect(monInfo.rcMonitor.top, monInfo.rcMonitor.bottom, monInfo.rcMonitor.left, monInfo.rcMonitor.right);
        aMon.setVRect(stRect);
        aMon.setId(monId);
        aMon.setFreq((int )dm.dmDisplayFrequency);
        // TODO
        aMon.setFreqMax((int )dm.dmDisplayFrequency);

        // ddMon.DeviceString = "Plug and Play monitor"
        // ddMon.DeviceName = "\\.\DISPLAY2\Monitor0"
        aMon.setName(StString() + (*ddMon.DeviceName ? ddMon.DeviceName : dispDevice.DeviceName));

        // NOTE: we get wrong id here for second monitor on WinXP x64
        StString aPnpId;
        const stUtfWide_t* aStart = NULL;
        for(StUtfWideIter anIter = StUtfWideIter(ddMon.DeviceID); *anIter != 0; ++anIter) {
            if(*anIter == stUtf32_t('\\')) {
                if(aStart == NULL) {
                    aStart = anIter.getBufferNext();
                } else {
                    // PnPId is always ASCII chars
                    aPnpId = StString(aStart, size_t(anIter.getBufferHere() - aStart));
                    break;
                }
            }
        }
        if(aPnpId.isEmpty()) {
          aPnpId = StString(ddMon.DeviceID);
        }
        aMon.setPnPId(aPnpId);
        aMon.setGpuName(StString(dispDevice.DeviceString));
        add(aMon);

        // make primary display first in our list
        if(isPrimary && monCount != 0) {
            StMonitor copy = getFirst();
            changeFirst() = getLast();
            changeLast() = copy;
        }
        ++monCount;
    }

    // group monitors by GPU
    for(size_t m = 1; m < size(); ++m) {
        if(getValue(m).getGpuName() != getFirst().getGpuName()) {
            for(size_t mm = m + 1; mm < size(); ++mm) {
                if(getValue(mm).getGpuName() == getFirst().getGpuName()) {
                    StMonitor copy = getValue(m);
                    changeValue(m) = getValue(mm);
                    changeValue(mm) = copy;
                }
            }
        }
    }
}
#endif // WinAPI

#if (defined(__linux__) || defined(__linux))
bool StSearchMonitors::getXRootSize(int& sizeX, int& sizeY) {
    Display* hDisplay = XOpenDisplay(NULL); // get first display on server from DISPLAY in env
    if(hDisplay == NULL) {
        ST_DEBUG_LOG("StSearchMonitors, X: could not open display");
        return false;
    }

    XWindowAttributes xwa;
    XGetWindowAttributes(hDisplay, RootWindow(hDisplay, 0), &xwa);
    sizeX = xwa.width;
    sizeY = xwa.height;
    XCloseDisplay(hDisplay);
    return (sizeX > 0 && sizeY > 0);
}

namespace {
    static StEDIDParser readXPropertyEDID(Display* theDisplay,
                                          RROutput theOutput,
                                          Atom     theAtom) {
        unsigned char* aPropData      = NULL;
        int            anActualFormat = 0;
        unsigned long  anItemsNb      = 0;
        unsigned long  aBytesAfter    = 0;
        Atom           anActualType;
        XRRGetOutputProperty(theDisplay, theOutput, theAtom,
                             0, 100, False, False,
                             AnyPropertyType,
                             &anActualType, &anActualFormat,
                             &anItemsNb, &aBytesAfter, &aPropData);

        StEDIDParser anEDID;
        if(anActualType == XA_INTEGER
        && anActualFormat == 8
        && (anItemsNb != 0 && (anItemsNb % 128 == 0))) {
            anEDID.init(aPropData, anItemsNb);
        }
        XFree(aPropData);
        return anEDID;
    }
};

void StSearchMonitors::findMonitorsXRandr() {
    Display* hDisplay = XOpenDisplay(NULL); // get first display on server from DISPLAY in env
    if(hDisplay == NULL) {
        ST_DEBUG_LOG("StSearchMonitors, X: could not open display");
        return;
    }

    int xrandrEvent(0), xrandrError(0);
    int xrandrMajor(0), xrandrMinor(0);
    int isOK = XRRQueryExtension(hDisplay, &xrandrEvent, &xrandrError);
    if(!isOK) {
        XCloseDisplay(hDisplay);
        return;
    }
    ///X11DRV_expect_error(hDisplay, XRandRErrorHandler, NULL); // TODO
    isOK = XRRQueryVersion(hDisplay, &xrandrMajor, &xrandrMinor);
    ///if (X11DRV_check_error()) { isOK = false; }
    if(!isOK || xrandrMajor < 1 || (xrandrMajor == 1 && xrandrMinor < 2)) {
        XCloseDisplay(hDisplay);
        return;
    }

    Window rootWindow = RootWindow(hDisplay, 0);
    XRRScreenResources* pScreenResources = NULL;
    if(xrandrMajor > 1 || (xrandrMajor == 1 && xrandrMinor >= 3)) {
        // for XRandr 1.3+; works much faster
        pScreenResources = XRRGetScreenResourcesCurrent(hDisplay, rootWindow);
    } else {
        // for XRandr 1.2; works VERY slow (~0.8sec)!
        pScreenResources = XRRGetScreenResources(hDisplay, rootWindow);
    }

#ifndef RR_PROPERTY_RANDR_EDID
    #define RR_PROPERTY_RANDR_EDID "EDID"
#endif
    Atom anAtomsEdid[] = {
        XInternAtom(hDisplay, RR_PROPERTY_RANDR_EDID,      False), // should be returned by all new drivers
        XInternAtom(hDisplay, "EDID_DATA",                 False), // returned by old and proprietary drivers
        XInternAtom(hDisplay, "XFree86_DDC_EDID1_RAWDATA", False), // outdated atom?
        0
    };

    for(int crtcId = 0; crtcId < pScreenResources->ncrtc; ++crtcId) {
        StMonitor aMonitor;
        // CRTC is a CRT Controller (this is X terminology)
        XRRCrtcInfo* pCrtcInfo = XRRGetCrtcInfo(hDisplay, pScreenResources, pScreenResources->crtcs[crtcId]);
        if(pCrtcInfo->noutput == 0) {
            XRRFreeCrtcInfo(pCrtcInfo);
            continue;
        }

        // read virtual coordinates
        StRectI_t stRect(pCrtcInfo->y, pCrtcInfo->y + pCrtcInfo->height,
                         pCrtcInfo->x, pCrtcInfo->x + pCrtcInfo->width);
        aMonitor.setVRect(stRect);
        aMonitor.setId(crtcId);

        // detect active refresh rate
        for(int aModeIter = 0; aModeIter < pScreenResources->nmode; ++aModeIter) {
            const XRRModeInfo& aMode = pScreenResources->modes[aModeIter];
            if(aMode.id != pCrtcInfo->mode) {
                continue;
            } else if(aMode.hTotal == 0 || aMode.vTotal == 0) {
                break;
            }
            double aRate = std::floor(double(aMode.dotClock) / (double(aMode.hTotal) * double(aMode.vTotal)) + 0.5);
            aMonitor.setFreq(int(aRate));
            break;
        }

        RROutput anOutput = pCrtcInfo->outputs[0];
        XRROutputInfo* anOutputInfo = XRRGetOutputInfo(hDisplay, pScreenResources, anOutput);

        // read EDID
        for(size_t anIter = 0; anAtomsEdid[anIter] != 0; ++anIter) {
            aMonitor.changeEdid() = readXPropertyEDID(hDisplay, anOutput, anAtomsEdid[anIter]);
            if(aMonitor.getEdid().isValid()) {
                break;
            }
        }

        if(aMonitor.getEdid().isValid()) {
            aMonitor.setPnPId(aMonitor.getEdid().getPnPId());
            aMonitor.setName(aMonitor.getEdid().getName());
        } else {
            aMonitor.setName(anOutputInfo->name);
            ST_DEBUG_LOG("EDID from XRandr for Output #" + anOutput + " ("+ aMonitor.getName() + ") is invalid!");
        }

        // detect max refresh rate
        double aMaxRate = 0.0;
        double aRate = 0.0;
        for(int aModeInOutputIter = 0; aModeInOutputIter < anOutputInfo->nmode; ++aModeInOutputIter) {
            RRMode aModeId = anOutputInfo->modes[aModeInOutputIter];
            for(int aModeIter = 0; aModeIter < pScreenResources->nmode; ++aModeIter) {
                const XRRModeInfo& aMode = pScreenResources->modes[aModeIter];
                if(aMode.id != aModeId) {
                    continue;
                }
                aRate = std::floor(double(aMode.dotClock) / (double(aMode.hTotal) * double(aMode.vTotal)) + 0.5);
                aMaxRate = stMax(aMaxRate, aRate);
            }
        }
        aMonitor.setFreqMax(int(aMaxRate));
        XRRFreeOutputInfo(anOutputInfo);

        XRRFreeCrtcInfo(pCrtcInfo);
        add(aMonitor);
    }

    XRRFreeScreenResources(pScreenResources);
    XCloseDisplay(hDisplay);
}

void StSearchMonitors::findMonitorsADLsdk() {
    StADLsdk anAdlSdk;
    if(!anAdlSdk.init()) {
        return;
    }

    ADLsdkFunctions* flist = anAdlSdk.getFunctions();

    size_t xMonCount = 0;
    size_t monCount = 0;
    int xRootSizeX = 0;
    int xRootSizeY = 0;
    bool isHaveXRootSizes = getXRootSize(xRootSizeX, xRootSizeY);
    int aDisplaysNb = 0;
    LPADLDisplayInfo anAdlDisplayInfo = NULL;
    if(flist->ADL_Display_DisplayInfo_Get(-1, &aDisplaysNb, &anAdlDisplayInfo, 0) != ADL_OK) {
        return;
    }

    StArrayList<int> aDipArr;
    ADLDisplayEDIDData anEdidData;
    ADLDDCInfo aDdcInfo;
    for(int aDispId = 0; aDispId < aDisplaysNb; ++aDispId) {
        const ADLDisplayInfo& aDispInfo = anAdlDisplayInfo[aDispId];
        if((ADL_DISPLAY_DISPLAYINFO_DISPLAYCONNECTED & aDispInfo.iDisplayInfoValue) == 0 ||
           (ADL_DISPLAY_DISPLAYINFO_DISPLAYMAPPED    & aDispInfo.iDisplayInfoValue) == 0) {
            continue; // skip the not connected or non-active displays
        }

        // filter duplicated entities
        bool isAlreadyShown = false;
        for(size_t aDId = 0; aDId < aDipArr.size(); ++aDId) {
            if(aDipArr[aDId] == aDispInfo.displayID.iDisplayPhysicalIndex) {
                isAlreadyShown = true;
                break;
            }
        }
        if(isAlreadyShown) {
            continue;
        }
        aDipArr.add(aDispInfo.displayID.iDisplayPhysicalIndex);

        // read desktop configuration
        int aDesktopConfig = ADL_DESKTOPCONFIG_UNKNOWN;
        if(flist->ADL_DesktopConfig_Get != NULL) {
            flist->ADL_DesktopConfig_Get(aDispInfo.displayID.iDisplayLogicalAdapterIndex, &aDesktopConfig);
        }
        /**StString aDeskConfDesc = "UNKNOWN";
        switch(aDesktopConfig) {
            case ADL_DESKTOPCONFIG_SINGLE:     aDeskConfDesc = "single"; break;
            case ADL_DESKTOPCONFIG_CLONE:      aDeskConfDesc = "clone"; break;
            case ADL_DESKTOPCONFIG_BIGDESK_H:  aDeskConfDesc = "big desktop H"; break;
            case ADL_DESKTOPCONFIG_BIGDESK_V:  aDeskConfDesc = "big desktop V"; break;
            case ADL_DESKTOPCONFIG_BIGDESK_HR: aDeskConfDesc = "big desktop HR"; break;
            case ADL_DESKTOPCONFIG_BIGDESK_VR: aDeskConfDesc = "big desktop VR"; break;
            case ADL_DESKTOPCONFIG_RANDR12:    aDeskConfDesc = "randr v.1.2"; break;
        }
        ST_DEBUG_LOG("StMonitors, X ADL desktop config is \"" + aDeskConfDesc + "\"");*/

        switch(aDesktopConfig) {
            case ADL_DESKTOPCONFIG_UNKNOWN:
            case ADL_DESKTOPCONFIG_SINGLE:
            case ADL_DESKTOPCONFIG_CLONE:
            case ADL_DESKTOPCONFIG_BIGDESK_H:
            case ADL_DESKTOPCONFIG_BIGDESK_V:
                if(xMonCount == 0 && isHaveXRootSizes) {
                    findMonitorsBlind(xRootSizeX, xRootSizeY);
                    xMonCount = size();
                }
                break;
            case ADL_DESKTOPCONFIG_BIGDESK_HR:
            case ADL_DESKTOPCONFIG_BIGDESK_VR:
                if(xMonCount == 0 && isHaveXRootSizes) {
                    findMonitorsBlind(xRootSizeX, xRootSizeY);
                    xMonCount = size();
                    if(xMonCount == 2) {
                        // reverse rectangles
                        StRectI_t aCopyRect = getValue(0).getVRect();
                        changeValue(0).setVRect(getValue(1).getVRect());
                        changeValue(1).setVRect(aCopyRect);
                    }
                }
                break;
            case ADL_DESKTOPCONFIG_RANDR12:
                if(xMonCount == 0) {
                    findMonitorsXRandr();
                    xMonCount = size();
                    if(xMonCount == 0 && isHaveXRootSizes) {
                        findMonitorsBlind(xRootSizeX, xRootSizeY);
                        xMonCount = size();
                    }
                }
                break;
        }

        // retrieve EDID data
        stMemSet(&anEdidData, 0, sizeof(ADLDisplayEDIDData));
        anEdidData.iSize = sizeof(ADLDisplayEDIDData);
        if(flist->ADL_Display_EdidData_Get != NULL) {
            flist->ADL_Display_EdidData_Get(aDispInfo.displayID.iDisplayLogicalAdapterIndex,
                                            aDispInfo.displayID.iDisplayLogicalIndex,
                                            &anEdidData);
        }

        // retrieve DDC info
        stMemSet(&aDdcInfo, 0, sizeof(ADLDDCInfo));
        aDdcInfo.ulSize = sizeof(ADLDDCInfo);
        if(flist->ADL_Display_DDCInfo_Get != NULL) {
            flist->ADL_Display_DDCInfo_Get(aDispInfo.displayID.iDisplayLogicalAdapterIndex,
                                           aDispInfo.displayID.iDisplayLogicalIndex,
                                           &aDdcInfo);
        }

        // fill StMonitor structure
        if(size() <= monCount) {
            add(StMonitor());
        }
        StMonitor& aMonitor = changeValue(monCount++);
        aMonitor.setFreq(aDdcInfo.ulPTMRefreshRate);
        aMonitor.setFreqMax(aDdcInfo.ulMaxRefresh);
        aMonitor.changeEdid().init((unsigned char* )anEdidData.cEDIDData);
        if(aMonitor.getEdid().isValid()) {
            aMonitor.setPnPId(aMonitor.getEdid().getPnPId());
        }
        aMonitor.setName(StString(aDispInfo.strDisplayName));

        for(int anAdaptId = 0; anAdaptId < anAdlSdk.getAdaptersNum(); ++anAdaptId) {
            const AdapterInfo& anAdapInfo = anAdlSdk.getAdapters()[anAdaptId];
            if(anAdapInfo.iAdapterIndex == aDispInfo.displayID.iDisplayLogicalAdapterIndex) {
                aMonitor.setGpuName(StString(anAdapInfo.strAdapterName));
                break;
            }
        }
    }
    StADLsdk::ADL_Main_Memory_Free(anAdlDisplayInfo);
}
#endif // ADLsdk for Linux

void StSearchMonitors::listEDID(StArrayList<StEDIDParser>& theEdids) {
    theEdids.clear();
#ifndef __APPLE__
    StADLsdk anAdlSdk;
    if(anAdlSdk.init()) {
        ADLsdkFunctions* flist = anAdlSdk.getFunctions();
        StArrayList<int> aDipArr;
        ADLDisplayEDIDData anEdidData;
        for(int anAdIter = -1; anAdIter < anAdlSdk.getAdaptersNum(); ++anAdIter) {
            int aDisplaysNb = 0;
            LPADLDisplayInfo anAdlDisplayInfo = NULL;
            int anAdapterIndex = (anAdIter >= 0) ? anAdlSdk.getAdapters()[anAdIter].iAdapterIndex : -1;
            if(flist->ADL_Display_DisplayInfo_Get(anAdapterIndex, &aDisplaysNb, &anAdlDisplayInfo, 0) != ADL_OK) {
                aDisplaysNb = 0;
            }

            for(int aDispId = 0; aDispId < aDisplaysNb; ++aDispId) {
                const ADLDisplayInfo& aDispInfo = anAdlDisplayInfo[aDispId];
                if((ADL_DISPLAY_DISPLAYINFO_DISPLAYCONNECTED & aDispInfo.iDisplayInfoValue) == 0 ||
                   (ADL_DISPLAY_DISPLAYINFO_DISPLAYMAPPED    & aDispInfo.iDisplayInfoValue) == 0) {
                    continue; // skip the not connected or non-active displays
                }

                int iDisplayIndex = aDispInfo.displayID.iDisplayLogicalIndex;
                stMemSet(&anEdidData, 0, sizeof(ADLDisplayEDIDData));
                anEdidData.iSize = sizeof(ADLDisplayEDIDData);
                if(flist->ADL_Display_EdidData_Get != NULL) {
                    flist->ADL_Display_EdidData_Get(aDispInfo.displayID.iDisplayLogicalAdapterIndex,
                                                    iDisplayIndex, &anEdidData);
                }

                // notice that this API reads 256 bytes pages from EDID, not 128 blocks
                StEDIDParser aParser((unsigned char* )anEdidData.cEDIDData, anEdidData.iEDIDSize);
                if(aParser.getExtensionsNb() >= 2) {
                    anEdidData.iBlockIndex = 2; // read extra blocks
                    flist->ADL_Display_EdidData_Get(aDispInfo.displayID.iDisplayLogicalAdapterIndex,
                                                    iDisplayIndex, &anEdidData);
                    aParser.add((unsigned char* )anEdidData.cEDIDData, anEdidData.iEDIDSize);
                }

                // filter duplicated entities
                bool isAlreadyShown = false;
                for(size_t aDId = 0; aDId < aDipArr.size(); ++aDId) {
                    if(aDipArr[aDId] == aDispInfo.displayID.iDisplayPhysicalIndex) {
                        isAlreadyShown = true;
                        break;
                    }
                }
                if(isAlreadyShown) {
                    continue;
                }
                aDipArr.add(aDispInfo.displayID.iDisplayPhysicalIndex);

                theEdids.add(aParser);

            }
            StADLsdk::ADL_Main_Memory_Free(anAdlDisplayInfo);
            if(anAdapterIndex == -1 && aDisplaysNb > 0) {
                // info for all adapters already count
                break;
            }
        }
    }
#endif // !__APPLE__

#ifdef _WIN32
    NvAPI_Status anErrStateNv = NvAPI_Initialize();
    if(anErrStateNv == NVAPI_OK) {
        NvPhysicalGpuHandle nvGPUHandles[NVAPI_MAX_PHYSICAL_GPUS];
        stMemSet(nvGPUHandles, 0, sizeof(NvPhysicalGpuHandle));
        NvU32 nvGpuCount = 0;
        anErrStateNv = NvAPI_EnumPhysicalGPUs(nvGPUHandles, &nvGpuCount);
        if(anErrStateNv == NVAPI_OK && nvGpuCount > 0) {
            // we got some NVIDIA GPUs...
            NV_EDID anEdidNv;
            stMemSet(&anEdidNv, 0 ,sizeof(NV_EDID));
            anEdidNv.version = NV_EDID_VER;
            for(size_t h = 0; h < nvGpuCount; h++) {
                NvU32 outputsMask = 0;
                anErrStateNv = NvAPI_GPU_GetAllOutputs(nvGPUHandles[h], &outputsMask);
                if(anErrStateNv != NVAPI_OK) {
                    continue;
                }
                int maxOutputsNum = sizeof(NvU32) * 8;
                for(int i = 0; i < maxOutputsNum; ++i) {
                    NvU32 dispOutId = 1 << i;
                    if(dispOutId & outputsMask) {
                        // got some desplay id...
                        anErrStateNv = NvAPI_GPU_GetEDID(nvGPUHandles[h], dispOutId, &anEdidNv);
                        if(anErrStateNv != NVAPI_OK) {
                            NvAPI_ShortString aShortStr;
                            NvAPI_GetErrorMessage(anErrStateNv, aShortStr);
                            continue;
                        }

                        // notice that this API reads 256 bytes pages from EDID, not 128 blocks
                        NvU32 aTotalSize = anEdidNv.sizeofEDID;
                        StEDIDParser aParser((unsigned char* )anEdidNv.EDID_Data, stMin(aTotalSize, NvU32(256)));
                        for(NvU32 anOffset = 256; (aTotalSize - anOffset) < aTotalSize; anOffset += 256) {
                            anEdidNv.offset = anOffset;
                            NvAPI_GPU_GetEDID(nvGPUHandles[h], dispOutId, &anEdidNv);
                            if(anErrStateNv == NVAPI_OK) {
                                aParser.add((unsigned char* )anEdidNv.EDID_Data, stMin(aTotalSize - anOffset, NvU32(256)));
                            }
                        }
                        theEdids.add(aParser);
                    }
                }
            }
        }
    }
#endif // _WIN32
}

namespace {

    static StMutex          THE_MON_MUTEX;
    static StTimer          THE_MON_INIT_TIMER(false);
    static StSearchMonitors THE_MONS_CACHED;

};

void StSearchMonitors::initGlobal() {
    clear();
    initFromConfig();
    if(isEmpty()) {
        initFromSystem();
    }
}

void StSearchMonitors::init(const bool theForced) {
    clear();
    StMutexAuto aLock(THE_MON_MUTEX);
    if(!THE_MON_INIT_TIMER.isOn()
    || (theForced && THE_MON_INIT_TIMER.getElapsedTime() > 30.0)) {
        THE_MONS_CACHED.initGlobal();
        THE_MON_INIT_TIMER.restart(0);
    }

    for(size_t aMonIter = 0; aMonIter < THE_MONS_CACHED.size(); ++aMonIter) {
        add(THE_MONS_CACHED.getValue(aMonIter));
    }
}

void StSearchMonitors::initFromConfig() {
    clear();
    const StString ST_GLOBAL_SETTINGS_GROUP("sview");
    const StString ST_GLOBAL_SETTINGS_MON_MASTER("monMaster");
    const StString ST_GLOBAL_SETTINGS_MON_SLAVE("monSlave");

    StSearchMonitors sysMons;
    sysMons.initFromSystem();
    if(sysMons.isEmpty()) {
        return;
    }

    StMonitor stMonMaster;
    StMonitor stMonSlave;

    StSettings aGlobalSettings(ST_GLOBAL_SETTINGS_GROUP);
    aGlobalSettings.loadInt32Rect(ST_GLOBAL_SETTINGS_MON_MASTER, stMonMaster.changeVRect());
    aGlobalSettings.loadInt32Rect(ST_GLOBAL_SETTINGS_MON_SLAVE,  stMonSlave.changeVRect());

    // save settings (to simple change them)
    aGlobalSettings.saveInt32Rect(ST_GLOBAL_SETTINGS_MON_MASTER, stMonMaster.changeVRect());
    aGlobalSettings.saveInt32Rect(ST_GLOBAL_SETTINGS_MON_SLAVE,  stMonSlave.changeVRect());

    if(stMonMaster.isValid()) {
        StRectI_t stRectCopy = stMonMaster.getVRect();
        // copy setting from some real display
        stMonMaster = sysMons[stRectCopy.center()];
        stMonMaster.setVRect(stRectCopy);
        stMonMaster.setId(0);
        stMonMaster.setName("StMasterDisplay");
        add(stMonMaster);
        if(stMonSlave.isValid()) {
            stRectCopy = stMonSlave.getVRect();
            stMonSlave = sysMons[stRectCopy.center()];
            stMonSlave.setVRect(stRectCopy);
            stMonSlave.setId(1);
            stMonSlave.setName("StSlaveDisplay");
            add(stMonSlave);
        }
    }
}

void StSearchMonitors::initFromSystem() {
    clear();
#ifdef _WIN32
    findMonitorsWinAPI();
#elif (defined(__APPLE__))
    findMonitorsCocoa();
#elif (defined(__linux__) || defined(__linux))
    // It seems this library is not thread-safe - random crashes and freezes
    // are detected when launched from browsers.
    ///findMonitorsADLsdk();

    if(isEmpty()) {
        findMonitorsXRandr();
        if(isEmpty()) {
            int rootX(0), rootY(0);
            if(!getXRootSize(rootX, rootY)) {
                ST_DEBUG_ASSERT(rootX > 0 && rootY > 0);
                rootX = rootY = 800;
            }
            findMonitorsBlind(rootX, rootY);
        }
    }
#endif
}
