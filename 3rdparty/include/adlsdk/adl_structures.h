///
///  Copyright (c) 2008 - 2009 Advanced Micro Devices, Inc.
 
///  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND,
///  EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
///  WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR PURPOSE.

/// \file adl_structures.h
///\brief This file contains the structure declarations that are used by the public ADL interfaces for \ALL platforms.\n <b>Included in ADL SDK</b>
///
/// All data structures used in AMD Display Library (ADL) public interfaces should be defined in this header file.
///

#ifndef ADL_STRUCTURES_H_
#define ADL_STRUCTURES_H_

#include "adl_defines.h"

/////////////////////////////////////////////////////////////////////////////////////////////
///\brief Structure containing information about the graphics adapter.
///
/// This structure is used to store various information about the graphics adapter.  This 
/// information can be returned to the user. Alternatively, it can be used to access various driver calls to set
/// or fetch various settings upon the user's request.
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////
typedef struct AdapterInfo
{
/// \ALL_STRUCT_MEM

/// Size of the structure.
    int iSize;
/// The ADL index handle. One GPU may be associated with one or two index handles
    int iAdapterIndex;
/// The unique device ID associated with this adapter.
    char strUDID[ADL_MAX_PATH];	
/// The BUS number associated with this adapter.
    int iBusNumber;
/// The driver number associated with this adapter.
    int iDeviceNumber;
/// The function number.
    int iFunctionNumber;
/// The vendor ID associated with this adapter.
    int iVendorID;
/// Adapter name.
    char strAdapterName[ADL_MAX_PATH];
/// Display name. For example, "\\Display0" for Windows or ":0:0" for Linux.
    char strDisplayName[ADL_MAX_PATH];
/// Present or not; 1 is present and 0 is not present.
	int iPresent;				
// @}

#if defined (_WIN32) || defined (_WIN64)
/// \WIN_STRUCT_MEM

/// Exist or not; 1 is present and 0 is not present.
    int iExist;
/// Driver registry path.
    char strDriverPath[ADL_MAX_PATH];
/// Driver registry path Ext for.
    char strDriverPathExt[ADL_MAX_PATH];
/// PNP string from Windows.
    char strPNPString[ADL_MAX_PATH];
/// It is generated from EnumDisplayDevices.
    int iOSDisplayIndex;	
// @}
#endif /* (_WIN32) || (_WIN64) */

#if defined (LINUX)
/// \LNX_STRUCT_MEM

/// Internal X screen number from GPUMapInfo (DEPRICATED use XScreenInfo)
    int iXScreenNum;
/// Internal driver index from GPUMapInfo
    int iDrvIndex;
/// \deprecated Internal x config file screen identifier name. Use XScreenInfo instead.
    char strXScreenConfigName[ADL_MAX_PATH];
   
// @}
#endif /* (LINUX) */
} AdapterInfo, *LPAdapterInfo;

/////////////////////////////////////////////////////////////////////////////////////////////
///\brief Structure containing information about the Linux X screen information.
///
/// This structure is used to store the current screen number and xorg.conf ID name assoicated with an adapter index.  
/// This structure is updated during ADL_Main_Control_Refresh or ADL_ScreenInfo_Update.  
/// Note:  This structure should be used in place of iXScreenNum and strXScreenConfigName in AdapterInfo as they will be 
/// depricated.
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////
#if defined (LINUX)
typedef struct XScreenInfo
{
/// Internal X screen number from GPUMapInfo.
	int iXScreenNum;
/// Internal x config file screen identifier name.
    char strXScreenConfigName[ADL_MAX_PATH];
} XScreenInfo, *LPXScreenInfo;
#endif /* (LINUX) */


/////////////////////////////////////////////////////////////////////////////////////////////
///\brief Structure containing information about the ASIC memory.
///
/// This structure is used to store various information about the ASIC memory.  This 
/// information can be returned to the user.
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////
typedef struct ADLMemoryInfo
{
/// Memory size in bytes.
    long long iMemorySize;	
/// Memory type in string.
    char strMemoryType[ADL_MAX_PATH];
/// Memory bandwidth in Mbytes/s.
    long long iMemoryBandwidth;
} ADLMemoryInfo, *LPADLMemoryInfo;

/////////////////////////////////////////////////////////////////////////////////////////////
///\brief Structure containing DDC information.
///
/// This structure is used to store various DDC information that can be returned to the user.
/// Note that all fields of type int are actually defined as unsigned int types within the driver.
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////
typedef struct ADLDDCInfo
{
/// Size of the structure
    int  ulSize;
/// Indicates whether the attached display supports DDC. If this field is zero on return, no other DDC information fields will be used.
    int  ulSupportsDDC;
/// Returns the manufacturer ID of the display device. Should be zeroed if this information is not available.
    int  ulManufacturerID;
/// Returns the product ID of the display device. Should be zeroed if this information is not available.
    int  ulProductID;
/// Returns the name of the display device. Should be zeroed if this information is not available.
    char cDisplayName[ADL_MAX_DISPLAY_NAME];
/// Returns the maximum Horizontal supported resolution. Should be zeroed if this information is not available.
    int  ulMaxHResolution;
/// Returns the maximum Vertical supported resolution. Should be zeroed if this information is not available.
    int  ulMaxVResolution;
/// Returns the maximum supported refresh rate. Should be zeroed if this information is not available.
    int  ulMaxRefresh;
/// Returns the display device preferred timing mode's horizontal resolution.
    int  ulPTMCx;
/// Returns the display device preferred timing mode's vertical resolution.
    int  ulPTMCy;
/// Returns the display device preferred timing mode's refresh rate.
    int  ulPTMRefreshRate;
/// Return EDID flags.
    int  ulDDCInfoFlag;
} ADLDDCInfo, *LPADLDDCInfo;

/////////////////////////////////////////////////////////////////////////////////////////////
///\brief Structure containing information controller Gamma settings.
///
/// This structure is used to store the red, green and blue color channel information for the.
/// controller gamma setting. This information is returned by ADL, and it can also be used to  
/// set the controller gamma setting.
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////
typedef struct ADLGamma
{
/// Red color channel gamma value.
	float fRed;
/// Green color channel gamma value.
	float fGreen;
/// Blue color channel gamma value.
	float fBlue;
} ADLGamma, *LPADLGamma;

/////////////////////////////////////////////////////////////////////////////////////////////
///\brief Structure containing information about component video custom modes.
///
/// This structure is used to store the component video custom mode.
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////
typedef struct ADLCustomMode
{
/// Custom mode flags.  They are returned by the ADL driver.
	int iFlags;
/// Custom mode width.
	int iModeWidth;
/// Custom mode height.
	int iModeHeight;
/// Custom mode base width.
	int iBaseModeWidth;
/// Custom mode base height.
	int iBaseModeHeight;
/// Custom mode refresh rate.
	int iRefreshRate;
} ADLCustomMode, *LPADLCustomMode;

/////////////////////////////////////////////////////////////////////////////////////////////
///\brief Structure containing Clock information for OD5 calls.
///
/// This structure is used to retrieve clock information for OD5 calls.
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////
typedef struct ADLGetClocksOUT
{
    long ulHighCoreClock;
    long ulHighMemoryClock;
    long ulHighVddc;
    long ulCoreMin;
    long ulCoreMax;
    long ulMemoryMin;
    long ulMemoryMax;
    long ulActivityPercent;
    long ulCurrentCoreClock;
    long ulCurrentMemoryClock;
    long ulReserved;
} ADLGetClocksOUT;

/////////////////////////////////////////////////////////////////////////////////////////////
///\brief Structure containing HDTV information for display calls.
///
/// This structure is used to retrieve HDTV information information for display calls.
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////
typedef struct ADLDisplayConfig
{
/// Size of the structure
  long ulSize;
/// HDTV connector type.
  long ulConnectorType;
/// HDTV capabilities.
  long ulDeviceData;
/// Overridden HDTV capabilities.
  long ulOverridedDeviceData;
/// Reserved field
  long ulReserved;	
} ADLDisplayConfig;


/////////////////////////////////////////////////////////////////////////////////////////////
///\brief Structure containing information about the display device.
///
/// This structure is used to store display device information
/// such as display index, type, name, connection status, mapped adapter and controller indexes, 
/// whether or not multiple VPUs are supported, local display connections or not (through Lasso), etc.
/// This information can be returned to the user. Alternatively, it can be used to access various driver calls to set
/// or fetch various display device related settings upon the user's request.
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////
typedef struct ADLDisplayID
{
/// The logical display index belonging to this adapter.
	int iDisplayLogicalIndex;

///\brief The physical display index.
/// For example, display index 2 from adapter 2 can be used by current adapter 1.\n
/// So current adapter may enumerate this adapter as logical display 7 but the physical display 
/// index is still 2.
	int iDisplayPhysicalIndex;

/// The persistent logical adapter index for the display.
	int iDisplayLogicalAdapterIndex;

///\brief The persistent physical adapter index for the display.
/// It can be the current adapter or a non-local adapter. \n
/// If this adapter index is different than the current adapter, 
/// the Display Non Local flag is set inside DisplayInfoValue.
    int iDisplayPhysicalAdapterIndex;
} ADLDisplayID, *LPADLDisplayID;

/////////////////////////////////////////////////////////////////////////////////////////////
///\brief Structure containing information about the display device.
///
/// This structure is used to store various information about the display device.  This 
/// information can be returned to the user, or used to access various driver calls to set
/// or fetch various display-device-related settings upon the user's request
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////
typedef struct ADLDisplayInfo
{
/// The DisplayID structure
	ADLDisplayID displayID; 

///\deprecated The controller index to which the display is mapped.\n Will not be used in the future\n
	int  iDisplayControllerIndex;	
	
/// The display's EDID name.
	char strDisplayName[ADL_MAX_PATH];        
	
/// The display's manufacturer name.
	char strDisplayManufacturerName[ADL_MAX_PATH];	

/// The Display type. For example: CRT, TV, CV, DFP.
	int  iDisplayType; 
	
/// The display output type. For example: HDMI, SVIDEO, COMPONMNET VIDEO.
	int  iDisplayOutputType; 
	
/// The connector type for the device.  
	int  iDisplayConnector; 

///\brief The bit mask identifies the number of bits ADLDisplayInfo is currently using. \n
/// It will be the sum all the bit definitions in ADL_DISPLAY_DISPLAYINFO_xxx.	
	int  iDisplayInfoMask; 
	
/// The bit mask identifies the display status. \ref define_displayinfomask
	int  iDisplayInfoValue; 
} ADLDisplayInfo, *LPADLDisplayInfo;


/////////////////////////////////////////////////////////////////////////////////////////////
///\brief Structure containing information about display mapping.
///
/// This structure is used to store the display mapping data such as display manner.
/// For displays with horizontal or vertical stretch manner, 
/// this structure also stores the display order, display row, and column data.
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////
typedef struct ADLDisplayMap
{
///\brief The current display map's index. It is the OS desktop index.\n
/// For example, if the OS index 1 is showing clone mode, the display map will be 1. 
	int iDisplayMapIndex; 
	
///\brief If the manner is NStretch, iDisplayRow indicates the number of rows for the span screen. If the 
///	manner is not NStretch, the display order field is not relevant.
	int iDisplayRow;   

///\brief If the manner is NStretch, iDisplayColumn indicates the number of columns for the span screen. 
///	If the manner is not NStretch, the display order field is not relevant.
	int iDisplayColumn;      

///\brief The bit mask identifies the number of bits DisplayMap is currently using. 
/// It is the sum of all the bit definitions defined in ADL_DISPLAY_DISPLAYMAP_MANNER_xxx.
 	int  iDisplayMapMask; 
	
///The bit mask identifies the display status. The detailed definition is in ADL_DISPLAY_DISPLAYMAP_MANNER_xxx.
	int  iDisplayMapValue; 
} ADLDisplayMap, *LPADLDisplayMap;

/////////////////////////////////////////////////////////////////////////////////////////////
///\brief Structure containing the display mode definition used per controller.
///
/// This structure is used to store the display mode definition used per controller.
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////
typedef struct ADLDisplayMode
{
/// Vertical resolution (in pixels).
   int  iPelsHeight;
/// Horizontal resolution (in pixels).
   int  iPelsWidth;
/// Color depth.
   int  iBitsPerPel;
/// Refresh rate.
   int  iDisplayFrequency;
} ADLDisplayMode;

/////////////////////////////////////////////////////////////////////////////////////////////
///\brief Structure containing detailed timing parameters.
///
/// This structure is used to store the detailed timing parameters.
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////
typedef struct ADLDetailedTiming
{
/// Size of the structure.
     int   iSize;
/// Timing flags. \ref define_detailed_timing_flags
     short sTimingFlags;
/// Total width (columns).
     short sHTotal;
/// Displayed width.
     short sHDisplay;
/// Horizontal sync signal offset.
     short sHSyncStart;
/// Horizontal sync signal width.
     short sHSyncWidth;
/// Total height (rows).
     short sVTotal;
/// Displayed height.
     short sVDisplay;
/// Vertical sync signal offset.
     short sVSyncStart;
/// Vertical sync signal width.
     short sVSyncWidth;
/// Pixel clock value.
     short sPixelClock;
/// Overscan right.
     short sHOverscanRight;
/// Overscan left.
     short sHOverscanLeft;
/// Overscan bottom.
     short sVOverscanBottom;
/// Overscan top.
     short sVOverscanTop;
     short sOverscan8B;
     short sOverscanGR;
} ADLDetailedTiming;

/////////////////////////////////////////////////////////////////////////////////////////////
///\brief Structure containing display mode information.
///
/// This structure is used to store the display mode information.
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////
typedef struct ADLDisplayModeInfo
{
/// Timing standard of the current mode. \ref define_modetiming_standard
  int  iTimingStandard;
/// Applicable timing standards for the current mode.
  int  iPossibleStandard;
/// Refresh rate factor.
  int  iRefreshRate;
/// Num of pixels in a row.
  int  iPelsWidth;
/// Num of pixels in a column.
  int  iPelsHeight;
/// Detailed timing parameters.
  ADLDetailedTiming  sDetailedTiming;
} ADLDisplayModeInfo;

/////////////////////////////////////////////////////////////////////////////////////////////
/// \brief Structure containing information about display property.
///
/// This structure is used to store the display property for the current adapter.
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////
typedef struct ADLDisplayProperty
{
/// Must be set to sizeof the structure
  int iSize;	
/// Must be set to \ref ADL_DL_DISPLAYPROPERTY_TYPE_EXPANSIONMODE or \ref ADL_DL_DISPLAYPROPERTY_TYPE_EXPANSIONMODE
  int iPropertyType;
/// Get or Set \ref ADL_DL_DISPLAYPROPERTY_EXPANSIONMODE_CENTER or \ref ADL_DL_DISPLAYPROPERTY_EXPANSIONMODE_FULLSCREEN or \ref ADL_DL_DISPLAYPROPERTY_EXPANSIONMODE_ASPECTRATIO
  int iExpansionMode;
/// Display Property supported? 1: Supported, 0: Not supported
  int iSupport;
/// Display Property current value 
  int iCurrent;
/// Display Property Default value
  int iDefault;			
} ADLDisplayProperty;

/////////////////////////////////////////////////////////////////////////////////////////////
///\brief Structure containing information about Clock.
///
/// This structure is used to store the clock information for the current adapter 
/// such as core clock and memory clock info.
///\nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////
typedef struct ADLClockInfo
{
/// Core clock in 10 KHz.
    int iCoreClock;
/// Memory clock in 10 KHz.
    int iMemoryClock;			
} ADLClockInfo, *LPADLClockInfo;

/////////////////////////////////////////////////////////////////////////////////////////////
///\brief Structure containing information about I2C.
///
/// This structure is used to store the I2C information for the current adapter.
/// This structure is used by the ADL_Display_WriteAndReadI2C function.
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////
typedef struct ADLI2C
{
/// Size of the structure
    int iSize;
/// Numerical value representing hardware I2C.
    int iLine;
/// The 7-bit I2C slave device address.
    int iAddress;
/// The offset of the data from the address.
    int iOffset;
/// Read from or write to slave device. \ref ADL_DL_I2C_ACTIONREAD or \ref ADL_DL_I2C_ACTIONWRITE
    int iAction;
/// I2C clock speed in KHz.
    int iSpeed;
/// A numerical value representing the number of bytes to be sent or received on the I2C bus.
    int iDataSize;
/// Address of the characters which are to be sent or received on the I2C bus.
    char *pcData;              
} ADLI2C;

/////////////////////////////////////////////////////////////////////////////////////////////
///\brief Structure containing information about EDID data.
///
/// This structure is used to store the information about EDID data for the adapter.
/// This structure is used by the ADL_Display_EdidData_Get function.
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////
typedef struct ADLDisplayEDIDData
{
/// Size of the structure
  int iSize;
/// Set to 0
  int iFlag;
  /// Size of cEDIDData. Set by ADL_Display_EdidData_Get() upon return
  int iEDIDSize;
/// 0, 1 or 2. If set to 3 or above an error ADL_ERR_INVALID_PARAM is generated
  int iBlockIndex;
/// EDID data
  char cEDIDData[ADL_MAX_EDIDDATA_SIZE];
/// Reserved
  int iReserved[4];
}ADLDisplayEDIDData;

/////////////////////////////////////////////////////////////////////////////////////////////
///\brief Structure containing information about input of controller overlay adjustment.
///
/// This structure is used to store the information about input of controller overlay adjustment for the adapter. 
/// This structure is used by the ADL_Display_ControllerOverlayAdjustmentCaps_Get, ADL_Display_ControllerOverlayAdjustmentData_Get, and
/// ADL_Display_ControllerOverlayAdjustmentData_Set functions.
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////
typedef struct ADLControllerOverlayInput
{
/// Should be set to the sizeof the structure
  int  iSize;
///\ref ADL_DL_CONTROLLER_OVERLAY_ALPHA or \ref ADL_DL_CONTROLLER_OVERLAY_ALPHAPERPIX
  int  iOverlayAdjust;	
/// Data.
  int  iValue;
/// Should be 0.
  int  iReserved;			
} ADLControllerOverlayInput;

/////////////////////////////////////////////////////////////////////////////////////////////
///\brief Structure containing information about overlay adjustment.
///
/// This structure is used to store the information about overlay adjustment for the adapter. 
/// This structure is used by the ADLControllerOverlayInfo function.
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////
typedef struct ADLAdjustmentinfo
{
/// Default value
  int iDefault;
/// Minimum value
  int iMin;
/// Maximum Value
  int iMax;
/// Step value
  int iStep;
} ADLAdjustmentinfo;

/////////////////////////////////////////////////////////////////////////////////////////////
///\brief Structure containing information about controller overlay information.
///
/// This structure is used to store information about controller overlay info for the adapter.
/// This structure is used by the ADL_Display_ControllerOverlayAdjustmentCaps_Get function.
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////
typedef struct ADLControllerOverlayInfo
{
/// Should be set to the sizeof the structure
  int					iSize;
/// Data.
  ADLAdjustmentinfo	    sOverlayInfo;
/// Should be 0.
  int					iReserved[3];
} ADLControllerOverlayInfo;

/////////////////////////////////////////////////////////////////////////////////////////////
///\brief Structure containing GL-Sync module information.
///
/// This structure is used to retrieve GL-Sync module information for
/// Workstation Framelock/Genlock.
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////
typedef struct ADLGLSyncModuleID
{
/// Unique GL-Sync module ID.
	int		iModuleID;
/// GL-Sync GPU port index (to be passed into ADLGLSyncGenlockConfig.lSignalSource and ADLGlSyncPortControl.lSignalSource).
	int		iGlSyncGPUPort;
/// GL-Sync module firmware version of Boot Sector.
	int		iFWBootSectorVersion;
/// GL-Sync module firmware version of User Sector.
	int		iFWUserSectorVersion;
} ADLGLSyncModuleID , *LPADLGLSyncModuleID;

/////////////////////////////////////////////////////////////////////////////////////////////
///\brief Structure containing GL-Sync ports capabilities.
///
/// This structure is used to retrieve hardware capabilities for the ports of the GL-Sync module
/// for Workstation Framelock/Genlock (such as port type and number of associated LEDs).
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////
typedef struct ADLGLSyncPortCaps
{
/// Port type. Bitfield of ADL_GLSYNC_PORTTYPE_*  \ref define_glsync
	int		iPortType;
/// Number of LEDs associated for this port.
	int		iNumOfLEDs;
}ADLGLSyncPortCaps, *LPADLGLSyncPortCaps;

/////////////////////////////////////////////////////////////////////////////////////////////
///\brief Structure containing GL-Sync Genlock settings.
///
/// This structure is used to get and set genlock settings for the GPU ports of the GL-Sync module
/// for Workstation Framelock/Genlock.\n
/// \see define_glsync 
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////
typedef struct ADLGLSyncGenlockConfig
{
/// Specifies what fields in this structure are valid \ref define_glsync
	int		iValidMask;
/// Delay (ms) generating a sync signal.
	int		iSyncDelay;
/// Vector of framelock control bits. Bitfield of ADL_GLSYNC_FRAMELOCKCNTL_* \ref define_glsync
	int		iFramelockCntlVector;
/// Source of the sync signal. Either GL_Sync GPU Port index or ADL_GLSYNC_SIGNALSOURCE_* \ref define_glsync
	int		iSignalSource;	
/// Use sampled sync signal. A value of 0 specifies no sampling.
	int		iSampleRate;
///< For interlaced sync signals, the value can be ADL_GLSYNC_SYNCFIELD_1 or *_BOTH \ref define_glsync
	int		iSyncField;
///< The signal edge that should trigger synchronization. ADL_GLSYNC_TRIGGEREDGE_* \ref define_glsync
	int		iTriggerEdge;
///< Scan rate multiplier applied to the sync signal. ADL_GLSYNC_SCANRATECOEFF_* \ref define_glsync
	int		iScanRateCoeff;
}ADLGLSyncGenlockConfig, *LPADLGLSyncGenlockConfig;

/////////////////////////////////////////////////////////////////////////////////////////////
///\brief Structure containing GL-Sync port information.
///
/// This structure is used to get status of the GL-Sync ports (BNC or RJ45s)
/// for Workstation Framelock/Genlock.
/// \see define_glsync 
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////
typedef struct ADLGlSyncPortInfo
{
/// Type of GL-Sync port (ADL_GLSYNC_PORT_*).
	int		iPortType;
/// The number of LEDs for this port. It's also filled within ADLGLSyncPortCaps.
	int		iNumOfLEDs;
/// Port state ADL_GLSYNC_PORTSTATE_*  \ref define_glsync
	int		iPortState;
/// Scanned frequency for this port (vertical refresh rate in milliHz; 60000 means 60 Hz).
	int		iFrequency;
/// Used for ADL_GLSYNC_PORT_BNC. It is ADL_GLSYNC_SIGNALTYPE_*   \ref define_glsync
	int		iSignalType;
/// Used for ADL_GLSYNC_PORT_RJ45PORT*. It is GL_Sync GPU Port index or ADL_GLSYNC_SIGNALSOURCE_*.  \ref define_glsync
	int		iSignalSource;	

} ADLGlSyncPortInfo, *LPADLGlSyncPortInfo;

/////////////////////////////////////////////////////////////////////////////////////////////
///\brief Structure containing GL-Sync port control settings.
///
/// This structure is used to configure the GL-Sync ports (RJ45s only)
/// for Workstation Framelock/Genlock.
/// \see define_glsync 
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////
typedef struct ADLGlSyncPortControl
{
/// Port to control ADL_GLSYNC_PORT_RJ45PORT1 or ADL_GLSYNC_PORT_RJ45PORT2   \ref define_glsync
	int		iPortType;
/// Port control data ADL_GLSYNC_PORTCNTL_*   \ref define_glsync
	int		iControlVector;	
/// Source of the sync signal. Either GL_Sync GPU Port index or ADL_GLSYNC_SIGNALSOURCE_*   \ref define_glsync
	int		iSignalSource;
} ADLGlSyncPortControl;

/////////////////////////////////////////////////////////////////////////////////////////////
///\brief Structure containing GL-Sync mode of a display.
///
/// This structure is used to get and set GL-Sync mode settings for a display connected to
/// an adapter attached to a GL-Sync module for Workstation Framelock/Genlock.
/// \see define_glsync 
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////
typedef struct ADLGlSyncMode
{
/// Mode control vector. Bitfield of ADL_GLSYNC_MODECNTL_*   \ref define_glsync
	int		iControlVector;			
/// Mode status vector. Bitfield of ADL_GLSYNC_MODECNTL_STATUS_*   \ref define_glsync
	int		iStatusVector;
/// Index of GL-Sync connector used to genlock the display/controller.
	int		iGLSyncConnectorIndex;
} ADLGlSyncMode, *LPADLGlSyncMode;

/////////////////////////////////////////////////////////////////////////////////////////////
///\brief Structure containing the packet info of a display.
///
/// This structure is used to get and set the packet information of a display. 
/// This structure is used by ADLDisplayDataPacket.
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////
typedef struct  ADLInfoPacket
{
	char hb0;
	char hb1;
	char hb2;
/// sb0~sb27
	char sb[28]; 
}ADLInfoPacket;

/////////////////////////////////////////////////////////////////////////////////////////////
///\brief Structure containing the AVI packet info of a display.
///
/// This structure is used to get and set AVI the packet info of a display.
/// This structure is used by ADLDisplayDataPacket.
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////
typedef struct ADLAVIInfoPacket  //Valid user defined data/
{
/// byte 3, bit 7
   char bPB3_ITC;
/// byte 5, bit [7:4].
   char bPB5;             
}ADLAVIInfoPacket;

// Overdrive clock setting structure definition.

/////////////////////////////////////////////////////////////////////////////////////////////
///\brief Structure containing the Overdrive clock setting.
///
/// This structure is used to get the Overdrive clock setting.
/// This structure is used by ADLAdapterODClockInfo.
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////
typedef struct ADLODClockSetting
{
/// Deafult clock
	int iDefaultClock;
/// Current clock
	int iCurrentClock;
/// Maximum clcok
	int iMaxClock;
/// Minimum clock
	int iMinClock;
/// Requested clcock
	int iRequestedClock;
/// Step
	int iStepClock;
} ADLODClockSetting;

/////////////////////////////////////////////////////////////////////////////////////////////
///\brief Structure containing the Overdrive clock information.
///
/// This structure is used to get the Overdrive clock information.
/// This structure is used by the ADL_Display_ODClockInfo_Get function.
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////
typedef struct ADLAdapterODClockInfo
{
/// Size of the structure
	int iSize;
/// Flag \ref define_clockinfo_flags
	int iFlags;
/// Memory Clock
	ADLODClockSetting sMemoryClock;
/// Engine Clock
	ADLODClockSetting sEngineClock;
} ADLAdapterODClockInfo;

/////////////////////////////////////////////////////////////////////////////////////////////
///\brief Structure containing the Overdrive clock configuration.
///
/// This structure is used to set the Overdrive clock configuration.
/// This structure is used by the ADL_Display_ODClockConfig_Set function.
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////
typedef struct ADLAdapterODClockConfig
{
/// Size of the structure
  int iSize;
/// Flag \ref define_clockinfo_flags
  int iFlags;
/// Memory Clock
  int iMemoryClock;
/// Engine Clock
  int iEngineClock;
} ADLAdapterODClockConfig;

/////////////////////////////////////////////////////////////////////////////////////////////
///\brief Structure containing information about current power management related activity.
///
/// This structure is used to store information about current power management related activity.
/// This structure (Overdrive 5 interfaces) is used by the ADL_PM_CurrentActivity_Get function.
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////
typedef struct ADLPMActivity
{
/// Must be set to the size of the structure
	int iSize;
/// Current engine clock.
	int iEngineClock;
/// Current memory clock.
	int iMemoryClock;
/// Current core voltage.
	int iVddc;
/// GPU utilization.
	int iActivityPercent;
/// Performance level index.
	int iCurrentPerformanceLevel;
/// Current PCIE bus speed.
	int iCurrentBusSpeed;
/// Number of PCIE bus lanes.
	int iCurrentBusLanes;
/// Maximum number of PCIE bus lanes.
	int iMaximumBusLanes;
/// Reserved for future purposes.
	int iReserved;								
} ADLPMActivity;

/////////////////////////////////////////////////////////////////////////////////////////////
///\brief Structure containing information about thermal controller.
///
/// This structure is used to store information about thermal controller.
/// This structure is used by ADL_PM_ThermalDevices_Enum.
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////
typedef struct ADLThermalControllerInfo
{
/// Must be set to the size of the structure
  int iSize;	
/// Possible valies: \ref ADL_DL_THERMAL_DOMAIN_OTHER or \ref ADL_DL_THERMAL_DOMAIN_GPU.
  int iThermalDomain;
///	GPU 0, 1, etc.
  int iDomainIndex;
/// Possible valies: \ref ADL_DL_THERMAL_FLAG_INTERRUPT or \ref ADL_DL_THERMAL_FLAG_FANCONTROL
  int iFlags;						
} ADLThermalControllerInfo;

/////////////////////////////////////////////////////////////////////////////////////////////
///\brief Structure containing information about thermal controller temperature.
///
/// This structure is used to store information about thermal controller temperature. 
/// This structure is used by the ADL_PM_Temperature_Get function.
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////
typedef struct ADLTemperature
{
/// Must be set to the size of the structure
  int iSize;	
/// Temperature in millidegrees Celsius.
  int iTemperature;  
} ADLTemperature;

/////////////////////////////////////////////////////////////////////////////////////////////
///\brief Structure containing information about thermal controller fan speed.
///
/// This structure is used to store information about thermal controller fan speed.
/// This structure is used by the ADL_PM_FanSpeedInfo_Get function.
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////
typedef struct ADLFanSpeedInfo
{
/// Must be set to the size of the structure
  int iSize;	
/// \ref define_fanctrl
  int iFlags;
/// Minimum possible fan speed value in percents.
  int iMinPercent;
/// Maximum possible fan speed value in percents.
  int iMaxPercent;	
/// Minimum possible fan speed value in RPM.
  int iMinRPM;
/// Maximum possible fan speed value in RPM.
  int iMaxRPM;
} ADLFanSpeedInfo;

/////////////////////////////////////////////////////////////////////////////////////////////
///\brief Structure containing information about fan speed reported by thermal controller.
///
/// This structure is used to store information about fan speed reported by thermal controller.
/// This structure is used by the ADL_Overdrive5_FanSpeed_Get() and ADL_Overdrive5_FanSpeed_Set() functions.
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////
typedef struct ADLFanSpeedValue
{
/// Must be set to the size of the structure
  int iSize;
/// Possible valies: \ref ADL_DL_FANCTRL_SPEED_TYPE_PERCENT or \ref ADL_DL_FANCTRL_SPEED_TYPE_RPM
  int iSpeedType;
/// Fan speed value
  int iFanSpeed;
/// The only flag for now is: \ref ADL_DL_FANCTRL_FLAG_USER_DEFINED_SPEED
  int iFlags;				
} ADLFanSpeedValue;

////////////////////////////////////////////////////////////////////////////////////////////
///\brief Structure containing the range of Overdrive parameter.
///
/// This structure is used to store information about the range of Overdrive parameter.
/// This structure is used by ADLODParameters.
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////
typedef struct ADLODParameterRange
{
/// Minimum parameter value.
  int iMin;
/// Maximum parameter value.
  int iMax;	
/// Parameter step value.
  int iStep;	
} ADLODParameterRange;

/////////////////////////////////////////////////////////////////////////////////////////////
///\brief Structure containing information about Overdrive parameters.
///
/// This structure is used to store information about Overdrive parameters.
/// This structure is used by the ADL_Overdrive5_ODParameters_Get() function.
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////
typedef struct ADLODParameters
{
/// Must be set to the size of the structure
  int iSize;	
/// Number of standard performance states.
  int iNumberOfPerformanceLevels;
/// Indicates whether the GPU is capable to measure its activity.
  int iActivityReportingSupported;
/// Indicates whether the GPU supports discrete performance levels or performance range.
  int iDiscretePerformanceLevels;
/// Reserved for future use.
  int iReserved;	
/// Engine clock range.
  ADLODParameterRange sEngineClock;	
/// Memory clock range.
  ADLODParameterRange sMemoryClock;
/// Core voltage range.
  ADLODParameterRange sVddc;	
} ADLODParameters;

/////////////////////////////////////////////////////////////////////////////////////////////
///\brief Structure containing information about Overdrive level.
///
/// This structure is used to store information about Overdrive level.
/// This structure is used by ADLODPerformanceLevels.
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////
typedef struct ADLODPerformanceLevel
{
/// Engine clock.
  int iEngineClock;
/// Memory clock.
  int iMemoryClock;
/// Core voltage.
  int iVddc;
} ADLODPerformanceLevel;

/////////////////////////////////////////////////////////////////////////////////////////////
///\brief Structure containing information about Overdrive performance levels.
///
/// This structure is used to store information about Overdrive performance levels.
/// This structure is used by the ADL_Overdrive5_ODPerformanceLevels_Get() and ADL_Overdrive5_ODPerformanceLevels_Set() functions.
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////
typedef struct ADLODPerformanceLevels
{
/// Must be set to sizeof( \ref ADLODPerformanceLevels ) + sizeof( \ref ADLODPerformanceLevel ) * (ADLODParameters.iNumberOfPerformanceLevels - 1)
  int iSize;
  int iReserved;
/// Array of performance state descriptors. Must have ADLODParameters.iNumberOfPerformanceLevels elements.
  ADLODPerformanceLevel aLevels [1];	
} ADLODPerformanceLevels;

#endif /* ADL_STRUCTURES_H_ */

