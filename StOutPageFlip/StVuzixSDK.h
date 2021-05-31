/**
 * StOutPageFlip, class providing stereoscopic output for Shutter Glasses displays using StCore toolkit.
 * Copyright © 2009 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#ifndef __StVuzixSDK_h_
#define __StVuzixSDK_h_

#include <StCore/StSearchMonitors.h>
#include <StLibrary.h>

namespace {
    static const char LIB_TRACKER_NAME[]   = "IWEARDRV";
    static const char LIB_STEREO_NAME[]    = "IWRSTDRV";
    static const char PNP_ID_VUZIX_VR920[] = "IWR0002";
};

class StVuzixSDK {

        private:

#ifndef _WIN32
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

    ST_LOCAL StVuzixSDK()
    : myVrStereoHandle(NULL) {
        stMemSet(&myFunctions, 0, sizeof(myFunctions));
    }

    ST_LOCAL ~StVuzixSDK() {
        setMonoOut();
        close();
        if(myVTrackerLib.isOpened() && myFunctions.IWRCloseTracker != NULL) {
            myFunctions.IWRCloseTracker();
        }
        myVTrackerLib.close();
        myVStereoLib.close();
    }

    ST_LOCAL void open() {
        if(myFunctions.IWRSTEREO_Open != NULL && myVrStereoHandle == NULL) {
            myVrStereoHandle = myFunctions.IWRSTEREO_Open();
        }
    }

    ST_LOCAL void close() {
        if(myFunctions.IWRSTEREO_Close != NULL && myVrStereoHandle != NULL) {
            myFunctions.IWRSTEREO_Close(myVrStereoHandle);
            myVrStereoHandle = NULL;
        }
    }

    ST_LOCAL void setStereoOut() {
        if(myVrStereoHandle != NULL && myFunctions.IWRSTEREO_SetStereo != NULL) {
            myFunctions.IWRSTEREO_SetStereo(myVrStereoHandle, IWR_STEREO_MODE);
        }
    }

    ST_LOCAL void setMonoOut() {
        if(myVrStereoHandle != NULL && myFunctions.IWRSTEREO_SetStereo != NULL) {
            myFunctions.IWRSTEREO_SetStereo(myVrStereoHandle, IWR_MONO_MODE);
        }
    }

    ST_LOCAL void waitAckLeft() {
        if(myVrStereoHandle != NULL && myFunctions.IWRSTEREO_WaitForAck != NULL) {
            myFunctions.IWRSTEREO_WaitForAck(myVrStereoHandle, LEFT_EYE);
        }
    }

    ST_LOCAL void waitAckRight() {
        if(myVrStereoHandle != NULL && myFunctions.IWRSTEREO_WaitForAck != NULL) {
            myFunctions.IWRSTEREO_WaitForAck(myVrStereoHandle, RIGHT_EYE);
        }
    }

    ST_LOCAL void setLeft() {
        if(myVrStereoHandle != NULL && myFunctions.IWRSTEREO_SetLR != NULL) {
            myFunctions.IWRSTEREO_SetLR(myVrStereoHandle, LEFT_EYE);
        }
    }

    ST_LOCAL void setRight() {
        if(myVrStereoHandle != NULL && myFunctions.IWRSTEREO_SetLR != NULL) {
            myFunctions.IWRSTEREO_SetLR(myVrStereoHandle, RIGHT_EYE);
        }
    }

    // INIT library function - called by host application to use library classes
    ST_LOCAL int init() {
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

    ST_LOCAL static bool isConnected(const StSearchMonitors& theMonitors) {
        const StString aPnP920 = PNP_ID_VUZIX_VR920;
        for(size_t aMonIter = 0; aMonIter < theMonitors.size(); ++aMonIter) {
            if(theMonitors[aMonIter].getPnPId() == aPnP920) {
                return true;
            }
        }
        return false;
    }

};

#endif //__StVuzixSDK_h_
