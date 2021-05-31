/**
 * StCore, window system independent C++ toolkit for writing OpenGL applications.
 * Copyright © 2009-2015 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#include <StCore/StSearchMonitors.h>

#include <StSettings/StSettings.h>
#include <StStrings/StStringStream.h>
#include <StThreads/StMutex.h>
#include <StThreads/StTimer.h>

#ifdef _WIN32
    #include <wnt/nvapi.h>
    #ifdef _MSC_VER
        #ifdef _WIN64
            #pragma comment(lib, "nvapi64.lib")
        #else
            #pragma comment(lib, "nvapi.lib")
        #endif
    #endif
    #pragma comment(linker, "/NODEFAULTLIB:libcmt.lib")
#elif defined(__ANDROID__)
    //
#elif defined(__linux__)
    #include "StWinHandles.h"
    #include <X11/extensions/Xrandr.h>
#endif

#if !defined(__ANDROID__)
    #include "StADLsdk.h"
#endif

StSearchMonitors::StSearchMonitors()
: StArrayList<StMonitor>(2),
  myIsUpdater(false) {
    //
}

StSearchMonitors::~StSearchMonitors() {
    if(myIsUpdater) {
        registerUpdater(false);
    }
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
    typedef HRESULT (WINAPI *GetDpiForMonitor_t)(HMONITOR theMon,
                                                 int      theDpiType, // MONITOR_DPI_TYPE
                                                 UINT*    theDpiX,
                                                 UINT*    theDpiY);
    typedef HRESULT (WINAPI *GetScaleFactorForMonitor_t)(HMONITOR   theMon,
                                                         int*       theScale);
    typedef HRESULT (WINAPI *RegisterScaleChangeEvent_t)(HANDLE     theEvent,
                                                         DWORD_PTR* theCookie);
    typedef HRESULT (WINAPI *UnregisterScaleChangeEvent_t)(DWORD_PTR theCookie);
    HMODULE aShcoreLib = GetModuleHandleW(L"Shcore");
    GetDpiForMonitor_t         aMonDpiFunc   = NULL;
    //GetScaleFactorForMonitor_t aMonScaleFunc = NULL;
    if(aShcoreLib != NULL) {
        aMonDpiFunc   = (GetDpiForMonitor_t         )GetProcAddress(aShcoreLib, "GetDpiForMonitor");
        //aMonScaleFunc = (GetScaleFactorForMonitor_t )GetProcAddress(aShcoreLib, "GetScaleFactorForMonitor");
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

        if(aMonDpiFunc != NULL) {
            // theDpiType = MDT_Default = MDT_Effective_DPI = 0
            UINT aDpiX = 96;
            UINT aDpiY = 96;
            aMonDpiFunc(hMonitor, 0, &aDpiX, &aDpiY);
            aScale = float(aDpiX) / 96.0f; // 96 is 100% in Windows
            //int aScalePercents = 100;
            //aMonScaleFunc(hMonitor, &aScalePercents);
            //aScale = float(aScalePercents) * 0.01f;
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
        aMon.setId(monCount);
        aMon.setFreq((float )dm.dmDisplayFrequency);
        // TODO
        aMon.setFreqMax((float )dm.dmDisplayFrequency);

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
            StMonitor aCopy = getFirst();
            changeFirst() = getLast();
            changeFirst().setId(0);
            changeLast()  = aCopy;
            changeLast().setId(monCount);
        }
        ++monCount;
    }

    // group monitors by GPU
    for(size_t m = 1; m < size(); ++m) {
        if(getValue(m).getGpuName() != getFirst().getGpuName()) {
            for(size_t mm = m + 1; mm < size(); ++mm) {
                if(getValue(mm).getGpuName() == getFirst().getGpuName()) {
                    StMonitor aCopy = getValue(m);
                    changeValue(m) = getValue(mm);
                    changeValue(m) .setId((int )m);
                    changeValue(mm) = aCopy;
                    changeValue(mm).setId((int )mm);
                }
            }
        }
    }
}
#endif // WinAPI

#if defined(__linux__) && !defined(__ANDROID__)
bool StSearchMonitors::getXRootSize(int& theSizeX, int& theSizeY) {
    Display* aDisplay = XOpenDisplay(NULL); // get first display on server from DISPLAY in env
    if(aDisplay == NULL) {
        ST_ERROR_LOG("StSearchMonitors, X: could not open display");
        return false;
    }

    XWindowAttributes aWinAttr;
    XGetWindowAttributes(aDisplay, RootWindow(aDisplay, 0), &aWinAttr);
    theSizeX = aWinAttr.width;
    theSizeY = aWinAttr.height;
    XCloseDisplay(aDisplay);
    return (theSizeX > 0 && theSizeY > 0);
}

namespace {
    /**
     * Auxiliary function to retrieve EDID propetry.
     */
    static StEDIDParser readXPropertyEDID(Display* theDisplay,
                                          RROutput theOutput,
                                          Atom     theAtom) {
        unsigned char* aPropData      = NULL;
        int            anActualFormat = 0;
        unsigned long  anItemsNb      = 0;
        unsigned long  aBytesAfter    = 0;
        Atom           anActualType   = (Atom )0;
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

    static const int THE_XLIB_DEF_DPI = 96;
    static const StCString THE_XFTDPI = stCString("Xft.dpi:");

    /**
     * Auxiliary function to retrieve Xft.dpi propetry.
     */
    static int readXftDpi(Display* theDisplay) {

        char* aXResMgr = XResourceManagerString(theDisplay);
        if(aXResMgr == NULL) {
            return THE_XLIB_DEF_DPI;
        }

        const StString aFullStr(aXResMgr);
        const StHandle <StArrayList<StString> > aList = aFullStr.split(stUtf32_t('\n'));
        for(size_t anIter = 0; anIter < aList->size(); ++anIter) {
            const StString& anEntry = aList->getValue(anIter);
            if(!anEntry.isStartsWith(THE_XFTDPI)) {
                continue;
            }

            const StString aValueStr = anEntry.subString(THE_XFTDPI.Length, anEntry.Length);
            StCLocale aCLocale;
            const int aValue = (int )stStringToLong(aValueStr.toCString(), 10, aCLocale);
            return stMax(aValue, 72);
        }
        return THE_XLIB_DEF_DPI;
    }

};

void StSearchMonitors::findMonitorsXRandr() {
    Display* aDisplay = XOpenDisplay(NULL); // get first display on server from DISPLAY in env
    if(aDisplay == NULL) {
        ST_ERROR_LOG("StSearchMonitors, X: could not open display");
        return;
    }

    // read global DPI value
    const float aScale = float(readXftDpi(aDisplay)) / float(THE_XLIB_DEF_DPI);

    int anXRandrEvent(0), anXRandrError(0);
    int anXRandrMajor(0), anXRandrMinor(0);
    int isOK = XRRQueryExtension(aDisplay, &anXRandrEvent, &anXRandrError);
    if(!isOK) {
        XCloseDisplay(aDisplay);
        return;
    }
    ///X11DRV_expect_error(aDisplay, XRandRErrorHandler, NULL); // TODO
    isOK = XRRQueryVersion(aDisplay, &anXRandrMajor, &anXRandrMinor);
    ///if (X11DRV_check_error()) { isOK = false; }
    if(!isOK || anXRandrMajor < 1 || (anXRandrMajor == 1 && anXRandrMinor < 2)) {
        XCloseDisplay(aDisplay);
        return;
    }

    Window aRootWin = RootWindow(aDisplay, 0);
    XRRScreenResources* aScrResources = NULL;
    if(anXRandrMajor > 1 || (anXRandrMajor == 1 && anXRandrMinor >= 3)) {
        // for XRandr 1.3+; works much faster
        aScrResources = XRRGetScreenResourcesCurrent(aDisplay, aRootWin);
    } else {
        // for XRandr 1.2; works VERY slow (~0.8sec)!
        aScrResources = XRRGetScreenResources(aDisplay, aRootWin);
    }

#ifndef RR_PROPERTY_RANDR_EDID
    #define RR_PROPERTY_RANDR_EDID "EDID"
#endif
    Atom anAtomsEdid[] = {
        XInternAtom(aDisplay, RR_PROPERTY_RANDR_EDID,      False), // should be returned by all new drivers
        XInternAtom(aDisplay, "EDID_DATA",                 False), // returned by old and proprietary drivers
        XInternAtom(aDisplay, "XFree86_DDC_EDID1_RAWDATA", False), // outdated atom?
        0
    };

    for(int aCrtcId = 0; aCrtcId < aScrResources->ncrtc; ++aCrtcId) {
        StMonitor aMonitor;
        // CRTC is a CRT Controller (this is X terminology)
        XRRCrtcInfo* aCrtcInfo = XRRGetCrtcInfo(aDisplay, aScrResources, aScrResources->crtcs[aCrtcId]);
        if(aCrtcInfo->noutput == 0) {
            XRRFreeCrtcInfo(aCrtcInfo);
            continue;
        }

        // read virtual coordinates
        StRectI_t aRect(aCrtcInfo->y, aCrtcInfo->y + aCrtcInfo->height,
                        aCrtcInfo->x, aCrtcInfo->x + aCrtcInfo->width);
        aMonitor.setVRect(aRect);
        aMonitor.setId(aCrtcId);
        aMonitor.setScale(aScale);

        // detect active refresh rate
        for(int aModeIter = 0; aModeIter < aScrResources->nmode; ++aModeIter) {
            const XRRModeInfo& aMode = aScrResources->modes[aModeIter];
            if(aMode.id != aCrtcInfo->mode) {
                continue;
            } else if(aMode.hTotal == 0 || aMode.vTotal == 0) {
                break;
            }
            double aRate = std::floor(double(aMode.dotClock) / (double(aMode.hTotal) * double(aMode.vTotal)) + 0.5);
            aMonitor.setFreq(float(aRate));
            break;
        }

        RROutput anOutput = aCrtcInfo->outputs[0];
        XRROutputInfo* anOutputInfo = XRRGetOutputInfo(aDisplay, aScrResources, anOutput);

        // read EDID
        for(size_t anIter = 0; anAtomsEdid[anIter] != 0; ++anIter) {
            aMonitor.changeEdid() = readXPropertyEDID(aDisplay, anOutput, anAtomsEdid[anIter]);
            if(aMonitor.getEdid().isValid()) {
                break;
            }
        }

        if(aMonitor.getEdid().isValid()) {
            aMonitor.setPnPId(aMonitor.getEdid().getPnPId());
            aMonitor.setName(aMonitor.getEdid().getName());
        } else {
            aMonitor.setName(anOutputInfo->name);
            ST_ERROR_LOG("EDID from XRandr for Output #" + uint64_t(anOutput) + " ("+ aMonitor.getName() + ") is invalid!");
        }

        // detect max refresh rate
        double aMaxRate = 0.0;
        double aRate = 0.0;
        for(int aModeInOutputIter = 0; aModeInOutputIter < anOutputInfo->nmode; ++aModeInOutputIter) {
            RRMode aModeId = anOutputInfo->modes[aModeInOutputIter];
            for(int aModeIter = 0; aModeIter < aScrResources->nmode; ++aModeIter) {
                const XRRModeInfo& aMode = aScrResources->modes[aModeIter];
                if(aMode.id != aModeId) {
                    continue;
                }
                aRate = std::floor(double(aMode.dotClock) / (double(aMode.hTotal) * double(aMode.vTotal)) + 0.5);
                aMaxRate = stMax(aMaxRate, aRate);
            }
        }
        aMonitor.setFreqMax(float(aMaxRate));
        XRRFreeOutputInfo(anOutputInfo);

        XRRFreeCrtcInfo(aCrtcInfo);
        add(aMonitor);
    }

    XRRFreeScreenResources(aScrResources);
    XCloseDisplay(aDisplay);
}
#endif // __linux__

void StSearchMonitors::listEDID(StArrayList<StEDIDParser>& theEdids) {
    theEdids.clear();
#if !defined( __APPLE__) && !defined(__ANDROID__)
    StADLsdk anAdlSdk;
    if(anAdlSdk.init()) {
        ADLsdkFunctions* aFuncs = anAdlSdk.getFunctions();
        StArrayList<int> aDipArr;
        ADLDisplayEDIDData anEdidData;
        for(int anAdIter = -1; anAdIter < anAdlSdk.getAdaptersNum(); ++anAdIter) {
            int aDisplaysNb = 0;
            LPADLDisplayInfo anAdlDisplayInfo = NULL;
            int anAdapterIndex = (anAdIter >= 0) ? anAdlSdk.getAdapters()[anAdIter].iAdapterIndex : -1;
            if(aFuncs->ADL_Display_DisplayInfo_Get(anAdapterIndex, &aDisplaysNb, &anAdlDisplayInfo, 0) != ADL_OK) {
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
                if(aFuncs->ADL_Display_EdidData_Get != NULL) {
                    aFuncs->ADL_Display_EdidData_Get(aDispInfo.displayID.iDisplayLogicalAdapterIndex,
                                                     iDisplayIndex, &anEdidData);
                }

                // notice that this API reads 256 bytes pages from EDID, not 128 blocks
                StEDIDParser aParser((unsigned char* )anEdidData.cEDIDData, anEdidData.iEDIDSize);
                if(aParser.getExtensionsNb() >= 2) {
                    anEdidData.iBlockIndex = 2; // read extra blocks
                    aFuncs->ADL_Display_EdidData_Get(aDispInfo.displayID.iDisplayLogicalAdapterIndex,
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
    static StSearchMonitors THE_MONS_CACHED;
    static int              THE_MON_NB_UPDATERS = 0;
    static int              THE_MON_WAIT_TO_UPDATE = 0;
#if !defined(__ANDROID__)
    static bool             THE_MON_IS_FIRST_CALL = true;
#endif

}

void StSearchMonitors::initGlobal() {
    clear();
    initFromSystem();
#if !defined(__ANDROID__)
    initFromConfig();
#endif
}

#if defined(__ANDROID__)
void StSearchMonitors::setupGlobalDisplay(const StMonitor& theDisplay) {
    StMutexAuto aLock(THE_MON_MUTEX);
    THE_MONS_CACHED.clear();
    THE_MONS_CACHED.add(theDisplay);
}
#endif

void StSearchMonitors::registerUpdater(const bool theIsUpdater) {
    if(myIsUpdater == theIsUpdater) {
        return;
    }
    myIsUpdater = theIsUpdater;

    StMutexAuto aLock(THE_MON_MUTEX);
    if(myIsUpdater) {
        ++THE_MON_NB_UPDATERS;
        THE_MON_WAIT_TO_UPDATE = 0;
    } else {
        --THE_MON_NB_UPDATERS;
        THE_MON_WAIT_TO_UPDATE = 0;
    }
}

void StSearchMonitors::init(const bool theForced) {
    clear();
    StMutexAuto aLock(THE_MON_MUTEX);
#if !defined(__ANDROID__)
    bool toUpdate = THE_MON_IS_FIRST_CALL;
    if(theForced && myIsUpdater) {
        ++THE_MON_WAIT_TO_UPDATE;
        if(THE_MON_WAIT_TO_UPDATE == 1) {
            // update within first listener in queue
            toUpdate = true;
        }
        if(THE_MON_WAIT_TO_UPDATE == THE_MON_NB_UPDATERS) {
            // all listeners received update event from system - reset counter
            THE_MON_WAIT_TO_UPDATE = 0;
        }
    }
    if(toUpdate) {
        THE_MONS_CACHED.initGlobal();
        THE_MON_IS_FIRST_CALL = false;
    }
#endif

    for(size_t aMonIter = 0; aMonIter < THE_MONS_CACHED.size(); ++aMonIter) {
        add(THE_MONS_CACHED.getValue(aMonIter));
    }
}

static void readMonitor(const StString& thePrefix,
                        StSettings&     theConfig,
                        StMonitor&      theMon) {
    // read/override monitor properties
    const StString aRectKey  = thePrefix + stCString(".rect");
    const StString aScaleKey = thePrefix + stCString(".scale");
    theConfig.loadInt32Rect(aRectKey, theMon.changeVRect());

    float aScale = 1.0f;
    if(theConfig.loadFloat(aScaleKey, aScale)) {
        theMon.setScale(aScale);
    }
}

void StSearchMonitors::initFromConfig() {
    const StString ST_GLOBAL_SETTINGS_GROUP("sview");
    const StString ST_GLOBAL_SETTINGS_MONITORS("monitors");

    StSettings aGlobalSettings(new StResourceManager(), ST_GLOBAL_SETTINGS_GROUP);
    StMonitor aMonDummy;
    for(size_t aParamIter = 0; aParamIter < 256; ++aParamIter) {
        const StString aPrefix     = ST_GLOBAL_SETTINGS_MONITORS + stCString(".") + aParamIter;
        const StString anActiveKey = aPrefix + stCString(".active");
        bool isActive = false;
        aGlobalSettings.loadBool(anActiveKey, isActive);
        if(!isActive) {
            break;
        }

        // read monitor(s) identifier
        const StString anIdKey   = aPrefix + stCString(".id");
        const StString aPnpIdKey = aPrefix + stCString(".pnpid");
        int32_t  aMonId = -1;
        StString aPnpId;
        aGlobalSettings.loadInt32 (anIdKey,   aMonId);
        aGlobalSettings.loadString(aPnpIdKey, aPnpId);
        if(aPnpId.getLength() == 7) {
            for(size_t aMonIter = 0; aMonIter < size(); ++aMonIter) {
                StMonitor& aMon = changeValue(aMonIter);
                if(aMon.getPnPId() == aPnpId) {
                    readMonitor(aPrefix, aGlobalSettings, aMon);
                }
            }
        } else if(aMonId >= 0) {
            for(int aMonAddIter = (int )size(); aMonAddIter <= aMonId; ++aMonAddIter) {
                aMonDummy.setId(aMonAddIter);
                add(aMonDummy);
            }
            readMonitor(aPrefix, aGlobalSettings, changeValue((size_t )aMonId));
        }
    }

    // save sample configuration to simplify manual edition
    const StString anActiveKey = ST_GLOBAL_SETTINGS_MONITORS + stCString(".999.active");
    bool isActive = false;
    if(aGlobalSettings.loadBool(anActiveKey, isActive)) {
        return;
    }

    aGlobalSettings.saveBool     (anActiveKey, false);
    aGlobalSettings.saveInt32    (ST_GLOBAL_SETTINGS_MONITORS + stCString(".999.id"),    999);
    aGlobalSettings.saveString   (ST_GLOBAL_SETTINGS_MONITORS + stCString(".999.pnpid"), "PNP0000");
    aGlobalSettings.saveInt32Rect(ST_GLOBAL_SETTINGS_MONITORS + stCString(".999.rect"),  StRect<int32_t>(0, 1080, 0, 1920));
    aGlobalSettings.saveFloat    (ST_GLOBAL_SETTINGS_MONITORS + stCString(".999.scale"), 1.2f);
}

void StSearchMonitors::initFromSystem() {
    clear();
#ifdef _WIN32
    findMonitorsWinAPI();
#elif defined(__APPLE__)
    findMonitorsCocoa();
#elif defined(__ANDROID__)
    findMonitorsBlind(1280, 720);
#elif defined(__linux__)
    findMonitorsXRandr();
    if(isEmpty()) {
        int aRootX(0), aRootY(0);
        if(!getXRootSize(aRootX, aRootY)) {
            ST_DEBUG_ASSERT(aRootX > 0 && aRootY > 0);
            aRootX = aRootY = 800;
        }
        findMonitorsBlind(aRootX, aRootY);
    }
#endif
}
