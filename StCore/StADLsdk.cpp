/**
 * StCore, window system independent C++ toolkit for writing OpenGL applications.
 * Copyright © 2009-2011 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#ifndef __APPLE__

#include "StADLsdk.h"

namespace {
#if(defined(_WIN32) || defined(__WIN32__))
    static const char LIB_NAME[]    = "atiadlxx";
    static const char LIB_NAME_Y[]  = "atiadlxy";
#elif(defined(__linux__) || defined(__linux))
    static const char LIB_NAME[]    = "libatiadlxx";
#endif
}

bool StADLsdk::countAdapters() {
    if(myFunctions.ADL_Adapter_AdapterInfo_Get      == NULL
    || myFunctions.ADL_Adapter_NumberOfAdapters_Get == NULL) {
        return false;
    }

    // obtain the number of adapters for the system
    myFunctions.ADL_Adapter_NumberOfAdapters_Get(&myNumAdapters);
    if(myNumAdapters <= 0) {
        return false;
    }
    myAdaptersInfoList = (AdapterInfo* )ADL_Main_Memory_Alloc(sizeof(AdapterInfo) * myNumAdapters);

    // get the AdapterInfo structure for all adapters in the system
    myFunctions.ADL_Adapter_AdapterInfo_Get(myAdaptersInfoList, sizeof(AdapterInfo) * myNumAdapters);
    return true;
}

void* __stdcall StADLsdk::ADL_Main_Memory_Alloc(int theSizeBytes) {
    void* aPtr = stMemAllocAligned(theSizeBytes);
    stMemSet(aPtr, 0, theSizeBytes);
    return aPtr;
}

void __stdcall StADLsdk::ADL_Main_Memory_Free(void* thePtr) {
    stMemFreeAligned(thePtr);
}

StADLsdk::StADLsdk()
: myLib(),
  myAdaptersInfoList(NULL),
  myNumAdapters(0) {
    //
    stMemSet(&myFunctions, 0, sizeof(ADLsdkFunctions));
}

void StADLsdk::close() {
    if(myFunctions.ADL_Main_Control_Destroy != NULL) {
        myFunctions.ADL_Main_Control_Destroy();
    }
    ADL_Main_Memory_Free(myAdaptersInfoList);
    myAdaptersInfoList = NULL;
    myLib.close();
    myNumAdapters = 0;
    stMemSet(&myFunctions, 0, sizeof(ADLsdkFunctions));
}

StADLsdk::~StADLsdk() {
    close();
}

bool StADLsdk::init() {
    // reset state
    close();

    if(!myLib.load(LIB_NAME)) {
    #if(defined(_WIN32) || defined(__WIN32__)) \
     && !defined(WIN64) && !defined(_WIN64) && !defined(__WIN64__)
        // library in WOW64, only for 32-bit application under 64-bit Windows
        if(!myLib.load(LIB_NAME_Y)) {
            return false;
        }
    #elif(defined(__linux__) || defined(__linux))
        return false;
    #endif
    }

    myLib("ADL_Main_Control_Create",  myFunctions.ADL_Main_Control_Create);
    myLib("ADL_Main_Control_Destroy", myFunctions.ADL_Main_Control_Destroy);

    // The second parameter is 0, which means:
    // retrieve adapter information only for adapters that are physically present and enabled in the system
    if(myFunctions.ADL_Main_Control_Create == NULL
    || myFunctions.ADL_Main_Control_Create(ADL_Main_Memory_Alloc, 0) != ADL_OK) {
        ST_DEBUG_LOG("StADLsdk, library Initialization error!");
        close();
        return false;
    }

    myLib("ADL_Adapter_NumberOfAdapters_Get",   myFunctions.ADL_Adapter_NumberOfAdapters_Get);
    myLib("ADL_Display_DisplayInfo_Get",        myFunctions.ADL_Display_DisplayInfo_Get);
    myLib("ADL_Adapter_AdapterInfo_Get",        myFunctions.ADL_Adapter_AdapterInfo_Get);
    myLib("ADL_Adapter_Active_Get",             myFunctions.ADL_Adapter_Active_Get);
    myLib("ADL_Display_ColorCaps_Get",          myFunctions.ADL_Display_ColorCaps_Get);
    myLib("ADL_Display_Color_Get",              myFunctions.ADL_Display_Color_Get);
    myLib("ADL_Display_Color_Set",              myFunctions.ADL_Display_Color_Set);
    myLib("ADL_Display_Position_Get",           myFunctions.ADL_Display_Position_Get);
    myLib("ADL_Display_Position_Set",           myFunctions.ADL_Display_Position_Set);
    myLib("ADL_Display_ModeTimingOverride_Get", myFunctions.ADL_Display_ModeTimingOverride_Get);
    myLib("ADL_Display_EdidData_Get",           myFunctions.ADL_Display_EdidData_Get);
    myLib("ADL_Display_WriteAndReadI2C",        myFunctions.ADL_Display_WriteAndReadI2C);
    myLib("ADL_Display_DDCInfo_Get",            myFunctions.ADL_Display_DDCInfo_Get);
#if(defined(__linux__) || defined(__linux))
    myLib("ADL_DesktopConfig_Get",              myFunctions.ADL_DesktopConfig_Get);
    myLib("ADL_DesktopConfig_Set",              myFunctions.ADL_DesktopConfig_Set);
#endif
    if(!countAdapters()
    || myFunctions.ADL_Adapter_NumberOfAdapters_Get == NULL
    || myFunctions.ADL_Adapter_AdapterInfo_Get == NULL
    || myFunctions.ADL_Display_DisplayInfo_Get == NULL
    || myFunctions.ADL_Adapter_Active_Get      == NULL) {
        // no AMD adapters...
        close();
        return false;
    }
    return true;
}

#endif // !__APPLE__
