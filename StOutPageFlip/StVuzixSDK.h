/**
 * Copyright © 2009 Kirill Gavrilov <kirill@sview.ru>
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

#ifndef __StVuzixSDK_h_
#define __StVuzixSDK_h_

#include <StLibrary.h>  // header for dynamic library loading

namespace {
    static const char LIB_TRACKER_NAME[]   = "IWEARDRV";
    static const char LIB_STEREO_NAME[]    = "IWRSTDRV";
    static const char PNP_ID_VUZIX_VR920[] = "IWR0002";
};

class ST_LOCAL StVuzixSDK {

        private:

#if (!defined(_WIN32) && !defined(__WIN32__))
    typedef unsigned long DWORD;
    typedef long          LONG;
    typedef void*         HANDLE;
    typedef int           BOOL;
    typedef unsigned char BYTE;
#endif

    typedef struct tag_IWRVERSION {
        unsigned short DLLMajor;
        unsigned short DLLMinor;
        unsigned short DLLSubMinor;
        unsigned short DLLBuildNumber;
        char USBFirmwareMajor;
        char USBFirmwareMinor;
        char TrackerFirmwareMajor;
        char TrackerFirmwareMinor;
        char VideoFirmware;
    } IWRVERSION, *PIWRVERSION;

    struct IWRsdkFunctions {
        // tracker API
        typedef DWORD (*IWROpenTracker_t)(void);
            IWROpenTracker_t IWROpenTracker;
        typedef void (*IWRCloseTracker_t)(void);
            IWRCloseTracker_t IWRCloseTracker;
        typedef void (*IWRZeroSet_t)(void);
            IWRZeroSet_t IWRZeroSet;
        typedef void (*IWRZeroClear_t)(void);
            IWRZeroClear_t IWRZeroClear;
        typedef DWORD (*IWRGetTracking_t)(LONG* , LONG* , LONG* );
            IWRGetTracking_t IWRGetTracking;
        typedef DWORD (*IWRGetVersion_t)(PIWRVERSION );
            IWRGetVersion_t IWRGetVersion;
        typedef void (*IWRSetFilterState_t)(BOOL );
            IWRSetFilterState_t IWRSetFilterState;
        typedef BOOL (*IWRGetFilterState_t)(void);
            IWRGetFilterState_t IWRGetFilterState;
        // stereo API
        typedef HANDLE (*IWRSTEREO_Open_t)(void);
            IWRSTEREO_Open_t IWRSTEREO_Open;
        typedef void (*IWRSTEREO_Close_t)(HANDLE hDev);
            IWRSTEREO_Close_t IWRSTEREO_Close;
        typedef BOOL (*IWRSTEREO_SetStereo_t)(HANDLE hDev, BOOL on);
            IWRSTEREO_SetStereo_t IWRSTEREO_SetStereo;
        typedef BYTE (*IWRSTEREO_WaitForAck_t)(HANDLE hDev, BOOL eye);
            IWRSTEREO_WaitForAck_t IWRSTEREO_WaitForAck;
        typedef BOOL (*IWRSTEREO_SetLR_t)(HANDLE hDev, BOOL eye);
            IWRSTEREO_SetLR_t IWRSTEREO_SetLR;
        typedef void (*IWRSTEREO_SetLREx_t)(HANDLE hDev, BOOL eye, BOOL wait);
            IWRSTEREO_SetLREx_t IWRSTEREO_SetLREx;
        typedef BOOL (*IWRSTEREO_GetVersion_t)(PIWRVERSION);
            IWRSTEREO_GetVersion_t IWRSTEREO_GetVersion;
    } myFunctions;

    enum {
        IWR_MONO_MODE = 0,
        IWR_STEREO_MODE
    };

    enum {
        LEFT_EYE = 0,
        RIGHT_EYE,
        MONO_EYES
    };

    StLibrary myVTrackerLib;
    StLibrary myVStereoLib;
    HANDLE    myVrStereoHandle;

        public:

    StVuzixSDK()
    : myVrStereoHandle(NULL) {
        stMemSet(&myFunctions, 0, sizeof(myFunctions));
    }

    ~StVuzixSDK() {
        setMonoOut();
        close();
        if(myVTrackerLib.isOpened() && myFunctions.IWRCloseTracker != NULL) {
            myFunctions.IWRCloseTracker();
        }
        myVTrackerLib.close();
        myVStereoLib.close();
    }

    void open() {
        if(myFunctions.IWRSTEREO_Open != NULL && myVrStereoHandle == NULL) {
            myVrStereoHandle = myFunctions.IWRSTEREO_Open();
        }
    }

    void close() {
        if(myFunctions.IWRSTEREO_Close != NULL && myVrStereoHandle != NULL) {
            myFunctions.IWRSTEREO_Close(myVrStereoHandle);
            myVrStereoHandle = NULL;
        }
    }

    void setStereoOut() {
        if(myVrStereoHandle != NULL && myFunctions.IWRSTEREO_SetStereo != NULL) {
            myFunctions.IWRSTEREO_SetStereo(myVrStereoHandle, IWR_STEREO_MODE);
        }
    }

    void setMonoOut() {
        if(myVrStereoHandle != NULL && myFunctions.IWRSTEREO_SetStereo != NULL) {
            myFunctions.IWRSTEREO_SetStereo(myVrStereoHandle, IWR_MONO_MODE);
        }
    }

    void waitAckLeft() {
        if(myVrStereoHandle != NULL && myFunctions.IWRSTEREO_WaitForAck != NULL) {
            myFunctions.IWRSTEREO_WaitForAck(myVrStereoHandle, LEFT_EYE);
        }
    }

    void waitAckRight() {
        if(myVrStereoHandle != NULL && myFunctions.IWRSTEREO_WaitForAck != NULL) {
            myFunctions.IWRSTEREO_WaitForAck(myVrStereoHandle, RIGHT_EYE);
        }
    }

    void setLeft() {
        if(myVrStereoHandle != NULL && myFunctions.IWRSTEREO_SetLR != NULL) {
            myFunctions.IWRSTEREO_SetLR(myVrStereoHandle, LEFT_EYE);
        }
    }

    void setRight() {
        if(myVrStereoHandle != NULL && myFunctions.IWRSTEREO_SetLR != NULL) {
            myFunctions.IWRSTEREO_SetLR(myVrStereoHandle, RIGHT_EYE);
        }
    }

    // INIT library function - called by host application to use library classes
    int init() {
        if(!myVTrackerLib.load(LIB_TRACKER_NAME)) {
            return STERROR_LIBLOADFAILED;
        }

        // get functions addresses in library
        myVTrackerLib("IWROpenTracker",    myFunctions.IWROpenTracker);
        myVTrackerLib("IWRCloseTracker",   myFunctions.IWRCloseTracker);
        myVTrackerLib("IWRZeroSet",        myFunctions.IWRZeroSet);
        myVTrackerLib("IWRGetTracking",    myFunctions.IWRGetTracking);
        myVTrackerLib("IWRSetFilterState", myFunctions.IWRSetFilterState);
        if(myFunctions.IWROpenTracker    == NULL || myFunctions.IWRCloseTracker == NULL
        || myFunctions.IWRZeroSet        == NULL || myFunctions.IWRGetTracking  == NULL
        || myFunctions.IWRSetFilterState == NULL) {
            myVTrackerLib.close();
            return STERROR_LIBFUNCTIONNOTFOUND;
        }

        if(!myVStereoLib.load(LIB_STEREO_NAME)) {
            return STERROR_LIBLOADFAILED;
        }

        // get functions addresses in library
        myVStereoLib("IWRSTEREO_Open",       myFunctions.IWRSTEREO_Open);
        myVStereoLib("IWRSTEREO_Close",      myFunctions.IWRSTEREO_Close);
        myVStereoLib("IWRSTEREO_SetLR",      myFunctions.IWRSTEREO_SetLR);
        myVStereoLib("IWRSTEREO_SetStereo",  myFunctions.IWRSTEREO_SetStereo);
        myVStereoLib("IWRSTEREO_WaitForAck", myFunctions.IWRSTEREO_WaitForAck);
        if(myFunctions.IWRSTEREO_Open       == NULL || myFunctions.IWRSTEREO_Close     == NULL
        || myFunctions.IWRSTEREO_SetLR      == NULL || myFunctions.IWRSTEREO_SetStereo == NULL
        || myFunctions.IWRSTEREO_WaitForAck == NULL) {
            myVStereoLib.close();
            return STERROR_LIBFUNCTIONNOTFOUND;
        }
        myFunctions.IWROpenTracker();
        // initialization success
        return STERROR_LIBNOERROR;
    }

    static bool isConnected() {
        // be sure Core library loaded before!
        const StString aPnP920 = PNP_ID_VUZIX_VR920;
        StArrayList<StMonitor> aMonitors = StCore::getStMonitors();
        for(size_t aMonIter = 0; aMonIter < aMonitors.size(); ++aMonIter) {
            if(aMonitors[aMonIter].getPnPId() == aPnP920) {
                return true;
            }
        }
        return false;
    }

};

#endif //__StVuzixSDK_h_
