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

#ifndef __APPLE__

#include "StSearchMonitors.h"

#if(defined(__linux__) || defined(__linux))
    #ifndef LINUX
        #define LINUX       // needed for ADLsdk
    #endif
#endif
#include <StLibrary.h>      // header for dynamic library loading
#include <adlsdk/adl_sdk.h> // AMD display library SDK

struct ST_LOCAL ADLsdkFunctions {
    typedef int (*ADL_Main_Control_Create_t)(ADL_MAIN_MALLOC_CALLBACK , int );
        ADL_Main_Control_Create_t ADL_Main_Control_Create;
    typedef int (*ADL_Main_Control_Destroy_t)();
        ADL_Main_Control_Destroy_t ADL_Main_Control_Destroy;
    typedef int (*ADL_Adapter_NumberOfAdapters_Get_t)(int* );
        ADL_Adapter_NumberOfAdapters_Get_t ADL_Adapter_NumberOfAdapters_Get;
    typedef int (*ADL_Adapter_AdapterInfo_Get_t)(LPAdapterInfo , int );
        ADL_Adapter_AdapterInfo_Get_t ADL_Adapter_AdapterInfo_Get;
    typedef int (*ADL_Adapter_Active_Get_t)(int , int* );
        ADL_Adapter_Active_Get_t ADL_Adapter_Active_Get;
    typedef int (*ADL_Display_ColorCaps_Get_t)(int , int , int* , int* );
        ADL_Display_ColorCaps_Get_t ADL_Display_ColorCaps_Get;
    typedef int (*ADL_Display_Color_Get_t)(int , int , int , int* , int* , int* , int* , int* );
        ADL_Display_Color_Get_t ADL_Display_Color_Get;
    typedef int (*ADL_Display_Color_Set_t)(int , int , int , int );
        ADL_Display_Color_Set_t ADL_Display_Color_Set;
    typedef int (*ADL_Display_DisplayInfo_Get_t)(int , int* , ADLDisplayInfo** , int );
        ADL_Display_DisplayInfo_Get_t ADL_Display_DisplayInfo_Get;
    typedef int (*ADL_Display_Position_Get_t)(int , int , int* , int* , int* , int* , int* , int* , int* , int*, int* , int* );
        ADL_Display_Position_Get_t ADL_Display_Position_Get;
    typedef int (*ADL_Display_Position_Set_t)(int , int , int , int);
        ADL_Display_Position_Set_t ADL_Display_Position_Set;
    typedef int (*ADL_Display_ModeTimingOverride_Get_t)(int , int , ADLDisplayMode* , ADLDisplayModeInfo* );
        ADL_Display_ModeTimingOverride_Get_t ADL_Display_ModeTimingOverride_Get;
    typedef int (*ADL_Display_EdidData_Get_t)(int , int , ADLDisplayEDIDData* );
        ADL_Display_EdidData_Get_t ADL_Display_EdidData_Get;
    typedef int (*ADL_Display_WriteAndReadI2C_t )(int , ADLI2C* );
        ADL_Display_WriteAndReadI2C_t ADL_Display_WriteAndReadI2C;
    typedef int (*ADL_Display_DDCInfo_Get_t )(int , int , ADLDDCInfo* );
        ADL_Display_DDCInfo_Get_t ADL_Display_DDCInfo_Get;
    typedef int (*ADL_DesktopConfig_Get_t )(int , int* ); // Linux only
        ADL_DesktopConfig_Get_t ADL_DesktopConfig_Get;
    typedef int (*ADL_DesktopConfig_Set_t )(int , int );  // Linux only
        ADL_DesktopConfig_Set_t ADL_DesktopConfig_Set;
};

/**
 * Wrapper over ADL SDK.
 */
class ST_LOCAL StADLsdk {

        private:

    StLibrary myLib;
    ADLsdkFunctions myFunctions;
    AdapterInfo* myAdaptersInfoList;
    int myNumAdapters;

        private:

    bool countAdapters();
    void close();

        public:

    static void* __stdcall ADL_Main_Memory_Alloc(int theSizeBytes);
    static void __stdcall ADL_Main_Memory_Free(void* thePtr);

        public:

    StADLsdk();
    ~StADLsdk();

    ADLsdkFunctions* getFunctions() {
        return &myFunctions;
    }

    int getAdaptersNum() const {
        return myNumAdapters;
    }

    const AdapterInfo* getAdapters() const {
        return myAdaptersInfoList;
    }

    bool init();

};

#endif // !__APPLE__
