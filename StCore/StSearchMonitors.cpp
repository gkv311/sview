/**
 * Copyright © 2009-2011 Kirill Gavrilov <kirill@sview.ru>
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

#include "StSearchMonitors.h"

#include <StSettings/StSettings.h>

#if (defined(_WIN32) || defined(__WIN32__))
    #include <wnt/nvapi.h> // NVIDIA API
#elif (defined(__linux__) || defined(__linux))
    #include "StWinHandles.h"
    #include <X11/extensions/Xrandr.h>
#endif

#include "StADLsdk.h"

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

#if (defined(_WIN32) || defined(__WIN32__))
void StSearchMonitors::findMonitorsWinAPI() {
    int monCount = 0;
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
        StMonitor stMon;

        // status flags: primary, disabled, removable
        bool isPrimary = false;
        if(!(dispDevice.StateFlags & DISPLAY_DEVICE_ATTACHED_TO_DESKTOP)) {
            ///stMon.setDisabled(true);
        } else if(dispDevice.StateFlags & DISPLAY_DEVICE_PRIMARY_DEVICE) {
            isPrimary = true;
        }
        if(dispDevice.StateFlags & DISPLAY_DEVICE_REMOVABLE) {
            ///removable
        }

        StRectI_t stRect(monInfo.rcMonitor.top, monInfo.rcMonitor.bottom, monInfo.rcMonitor.left, monInfo.rcMonitor.right);
        stMon.setVRect(stRect);
        stMon.setId(monId);
        stMon.setFreq((int )dm.dmDisplayFrequency);
        // TODO
        stMon.setFreqMax((int )dm.dmDisplayFrequency);

        // ddMon.DeviceString = "Plug and Play monitor"
        // ddMon.DeviceName = "\\.\DISPLAY2\Monitor0"
        stMon.setName(StString() + (*ddMon.DeviceName ? ddMon.DeviceName : dispDevice.DeviceName));

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
        stMon.setPnPId(aPnpId);
        stMon.setGpuName(StString(dispDevice.DeviceString));
        add(stMon);

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
    static StEDIDParser readXPropertyEDID(Display* hDisplay, RROutput theOutput, Atom atom) {
        StEDIDParser anEDID;
        unsigned char* propData = NULL;
        int actualFormat;
        unsigned long nItems = 0;
        unsigned long bytesAfter = 0;
        Atom actualType;

        XRRGetOutputProperty(hDisplay, theOutput, atom,
                             0, 100, False, False,
                             AnyPropertyType,
                             &actualType, &actualFormat,
                             &nItems, &bytesAfter, &propData);

        if(actualType == XA_INTEGER && actualFormat == 8 && nItems == 128) {
            anEDID.init(propData);
        }
        XFree(propData);
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

    Atom anEDIDAtom = XInternAtom(hDisplay, "EDID_DATA", False);
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
        aMonitor.changeEdid() = readXPropertyEDID(hDisplay, anOutput, anEDIDAtom);
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
                StEDIDParser aParser((unsigned char* )anEdidData.cEDIDData);

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

#if (defined(_WIN32) || defined(__WIN32__))
    NvAPI_Status errState = NvAPI_Initialize();
    if(errState == NVAPI_OK) {
        ///ST_DEBUG_LOG_AT("NvAPI_Initialized");
        NvPhysicalGpuHandle nvGPUHandles[NVAPI_MAX_PHYSICAL_GPUS];
        stMemSet(nvGPUHandles, 0, sizeof(NvPhysicalGpuHandle));
        NvU32 nvGpuCount = 0;
        errState = NvAPI_EnumPhysicalGPUs(nvGPUHandles, &nvGpuCount);
        if(errState == NVAPI_OK && nvGpuCount > 0) {
            // we got some NVIDIA GPUs...
            NV_EDID nv_edid;
            stMemSet(&nv_edid, 0 ,sizeof(NV_EDID));
            nv_edid.version = NV_EDID_VER;
            ///ST_DEBUG_LOG_AT("nvGpuCount= " + (size_t )nvGpuCount);
            for(size_t h = 0; h < nvGpuCount; h++) {
                NvU32 outputsMask = 0;
                errState = NvAPI_GPU_GetAllOutputs(nvGPUHandles[h], &outputsMask);
                if(errState != NVAPI_OK) {
                    ///ST_DEBUG_LOG_AT("NvAPI_GPU_GetAllOutputs FAILED");
                    continue;
                }
                int maxOutputsNum = sizeof(NvU32) * 8;
                for(int i = 0; i < maxOutputsNum; ++i) {
                    NvU32 dispOutId = 1 << i;
                    if(dispOutId & outputsMask) {
                        ///ST_DEBUG_LOG_AT("got the " + (size_t )dispOutId + " vs outputsMask= " + (size_t )outputsMask);
                        // got some desplay id...
                        errState = NvAPI_GPU_GetEDID(nvGPUHandles[h], dispOutId, &nv_edid);
                        if(errState == NVAPI_OK) {
                            StEDIDParser aParser((unsigned char* )nv_edid.EDID_Data);
                            theEdids.add(aParser);
                        } else {
                            NvAPI_ShortString shortString;
                            NvAPI_GetErrorMessage(errState, shortString);
                            ///ST_DEBUG_LOG_AT("Fail to get EDID data, err= " + (char* )shortString);
                        }
                    }
                }
            }
        }
    }
#endif // _WIN32
}

void StSearchMonitors::init() {
    clear();
    initFromConfig();
    if(isEmpty()) {
        initFromSystem();
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
    if(!aGlobalSettings.isValid()) {
        ST_DEBUG_LOG("StSearchMonitors::initFromConfig(), Settings library not available!");
        return;
    }
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
#if (defined(_WIN32) || defined(__WIN32__))
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

namespace {
    static StSearchMonitors ST_MONITORS_CACHED;
    static StMutex          ST_MONITORS_LOCK;
};

ST_EXPORT int StCore_getStMonitors(StMonitor_t* monList, const int& inSize, stBool_t toForceUpdate) {
    StMutexAuto aLockAuto(ST_MONITORS_LOCK);
    if(toForceUpdate || ST_MONITORS_CACHED.isEmpty()) {
        ST_MONITORS_CACHED.init();
    }
    size_t minSize = (size_t(inSize) < ST_MONITORS_CACHED.size()) ? size_t(inSize) : ST_MONITORS_CACHED.size();
    for(size_t m = 0; m < minSize; ++m) {
        StMonitor_t mst = ST_MONITORS_CACHED[m].getStruct();
        stMemCpy(&monList[m], &mst, sizeof(StMonitor_t));
    }
    return int(ST_MONITORS_CACHED.size());
}
