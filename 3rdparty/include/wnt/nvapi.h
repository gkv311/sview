 /***************************************************************************\
|*                                                                           *|
|*      Copyright 2005-2010 NVIDIA Corporation.  All rights reserved.        *|
|*                                                                           *|
|*   NOTICE TO USER:                                                         *|
|*                                                                           *|
|*   This source code is subject to NVIDIA ownership rights under U.S.       *|
|*   and international Copyright laws.  Users and possessors of this         *|
|*   source code are hereby granted a nonexclusive, royalty-free             *|
|*   license to use this code in individual and commercial software.         *|
|*                                                                           *|
|*   NVIDIA MAKES NO REPRESENTATION ABOUT THE SUITABILITY OF THIS SOURCE     *|
|*   CODE FOR ANY PURPOSE. IT IS PROVIDED "AS IS" WITHOUT EXPRESS OR         *|
|*   IMPLIED WARRANTY OF ANY KIND. NVIDIA DISCLAIMS ALL WARRANTIES WITH      *|
|*   REGARD TO THIS SOURCE CODE, INCLUDING ALL IMPLIED WARRANTIES OF         *|
|*   MERCHANTABILITY, NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR          *|
|*   PURPOSE. IN NO EVENT SHALL NVIDIA BE LIABLE FOR ANY SPECIAL,            *|
|*   INDIRECT, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, OR ANY DAMAGES          *|
|*   WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN      *|
|*   AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING     *|
|*   OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOURCE      *|
|*   CODE.                                                                   *|
|*                                                                           *|
|*   U.S. Government End Users. This source code is a "commercial item"      *|
|*   as that term is defined at 48 C.F.R. 2.101 (OCT 1995), consisting       *|
|*   of "commercial computer  software" and "commercial computer software    *|
|*   documentation" as such terms are used in 48 C.F.R. 12.212 (SEPT 1995)   *|
|*   and is provided to the U.S. Government only as a commercial end item.   *|
|*   Consistent with 48 C.F.R.12.212 and 48 C.F.R. 227.7202-1 through        *|
|*   227.7202-4 (JUNE 1995), all U.S. Government End Users acquire the       *|
|*   source code with only those rights set forth herein.                    *|
|*                                                                           *|
|*   Any use of this source code in individual and commercial software must  *|
|*   include, in the user documentation and internal comments to the code,   *|
|*   the above Disclaimer and U.S. Government End Users Notice.              *|
|*                                                                           *|
|*                                                                           *|
 \***************************************************************************/
///////////////////////////////////////////////////////////////////////////////
//
// Date: May 14, 2010
// File: nvapi.h
//
// NvAPI provides an interface to NVIDIA devices. This file contains the
// interface constants, structure definitions and function prototypes.
//
//   Target Profile: developer
//  Target Platform: windows
//
///////////////////////////////////////////////////////////////////////////////
#ifndef _NVAPI_H
#define _NVAPI_H

#pragma pack(push,8) // Make sure we have consistent structure packings

#ifdef __cplusplus
extern "C" {
#endif

// ====================================================
// Universal NvAPI Definitions
// ====================================================
#ifndef _WIN32
#define __cdecl
#endif

#define NVAPI_INTERFACE extern NvAPI_Status __cdecl

/* 64-bit types for compilers that support them, plus some obsolete variants */
#if defined(__GNUC__) || defined(__arm) || defined(__IAR_SYSTEMS_ICC__) || defined(__ghs__) || defined(_WIN64)
typedef unsigned long long NvU64; /* 0 to 18446744073709551615          */
#else
typedef unsigned __int64   NvU64; /* 0 to 18446744073709551615              */
#endif

// mac os 32-bit still needs this
#if (defined(macintosh) || defined(__APPLE__)) && !defined(__LP64__)
typedef signed long        NvS32; /* -2147483648 to 2147483647               */
#else
typedef signed int         NvS32; /* -2147483648 to 2147483647               */
#endif

// mac os 32-bit still needs this
#if ( (defined(macintosh) && defined(__LP64__) && (__NVAPI_RESERVED0__)) || \
      (!defined(macintosh) && defined(__NVAPI_RESERVED0__)) )
typedef unsigned int       NvU32; /* 0 to 4294967295                         */
#else
typedef unsigned long      NvU32; /* 0 to 4294967295                         */
#endif

typedef unsigned short   NvU16;
typedef unsigned char    NvU8;

typedef struct _NV_RECT
{
    NvU32    left;
    NvU32    top;
    NvU32    right;
    NvU32    bottom;
} NV_RECT;

#define NV_DECLARE_HANDLE(name) struct name##__ { int unused; }; typedef struct name##__ *name

// NVAPI Handles - These handles are retrieved from various calls and passed in to others in NvAPI
//                 These are meant to be opaque types.  Do not assume they correspond to indices, HDCs,
//                 display indexes or anything else.
//
//                 Most handles remain valid until a display re-configuration (display mode set) or GPU
//                 reconfiguration (going into or out of SLI modes) occurs.  If NVAPI_HANDLE_INVALIDATED
//                 is received by an app, it should discard all handles, and re-enumerate them.
//
NV_DECLARE_HANDLE(NvDisplayHandle);                // Display Device driven by NVIDIA GPU(s) (an attached display)
NV_DECLARE_HANDLE(NvUnAttachedDisplayHandle);      // Unattached Display Device driven by NVIDIA GPU(s)
NV_DECLARE_HANDLE(NvLogicalGpuHandle);             // One or more physical GPUs acting in concert (SLI)
NV_DECLARE_HANDLE(NvPhysicalGpuHandle);            // A single physical GPU
NV_DECLARE_HANDLE(NvEventHandle);                  // A handle to an event registration instance
NV_DECLARE_HANDLE(NvVisualComputingDeviceHandle);  // A handle to Visual Computing Device
NV_DECLARE_HANDLE(NvHICHandle);                    // A handle to a Host Interface Card
NV_DECLARE_HANDLE(NvGSyncDeviceHandle);            // A handle to a GSync device
NV_DECLARE_HANDLE(NvVioHandle);                    // A handle to a SDI device
NV_DECLARE_HANDLE(NvTransitionHandle);             // A handle to address a single transition request
NV_DECLARE_HANDLE(NvAudioHandle);                  // Nvidia HD Audio Device
NV_DECLARE_HANDLE(NvIDMHandle);                    // A handle to a Win7 IDM-enabled display

typedef void* StereoHandle;

NV_DECLARE_HANDLE(NvSourceHandle);                 // Unique source handle on the system
NV_DECLARE_HANDLE(NvTargetHandle);                 // Unique target handle on the system


#define NVAPI_DEFAULT_HANDLE        0
#define NV_BIT(x)    (1 << (x))


#define NVAPI_GENERIC_STRING_MAX    4096
#define NVAPI_LONG_STRING_MAX       256
#define NVAPI_SHORT_STRING_MAX      64

typedef struct
{
    NvS32   sX;
    NvS32   sY;
    NvS32   sWidth;
    NvS32   sHeight;
} NvSBox;

typedef struct
{
    NvU32 data1;
    NvU16 data2;
    NvU16 data3;
    NvU8  data4[8];
} NvGUID, NvLUID;

#define NVAPI_MAX_PHYSICAL_GPUS             64
#define NVAPI_PHYSICAL_GPUS                 32
#define NVAPI_MAX_LOGICAL_GPUS              64
#define NVAPI_MAX_AVAILABLE_GPU_TOPOLOGIES  256
#define NVAPI_MAX_AVAILABLE_SLI_GROUPS      256
#define NVAPI_MAX_GPU_TOPOLOGIES            NVAPI_MAX_PHYSICAL_GPUS
#define NVAPI_MAX_GPU_PER_TOPOLOGY          8
#define NVAPI_MAX_DISPLAY_HEADS             2
#define NVAPI_ADVANCED_DISPLAY_HEADS        4
#define NVAPI_MAX_DISPLAYS                  NVAPI_PHYSICAL_GPUS * NVAPI_ADVANCED_DISPLAY_HEADS
#define NVAPI_MAX_ACPI_IDS                  16
#define NVAPI_MAX_VIEW_MODES                8
#define NV_MAX_HEADS                        4   // Maximum heads, each with NVAPI_DESKTOP_RES resolution
#define NVAPI_MAX_HEADS_PER_GPU             32

#define NV_MAX_HEADS        4   // Maximum heads, each with NVAPI_DESKTOP_RES resolution
#define NV_MAX_VID_STREAMS  4   // Maximum input video streams, each with a NVAPI_VIDEO_SRC_INFO
#define NV_MAX_VID_PROFILES 4   // Maximum output video profiles supported

#define NVAPI_SYSTEM_MAX_DISPLAYS           NVAPI_MAX_PHYSICAL_GPUS * NV_MAX_HEADS

#define NVAPI_SYSTEM_MAX_HWBCS              128
#define NVAPI_SYSTEM_HWBC_INVALID_ID        0xffffffff
#define NVAPI_MAX_AUDIO_DEVICES             16

typedef char NvAPI_String[NVAPI_GENERIC_STRING_MAX];
typedef char NvAPI_LongString[NVAPI_LONG_STRING_MAX];
typedef char NvAPI_ShortString[NVAPI_SHORT_STRING_MAX];

// =========================================================================================
// NvAPI Version Definition
// Maintain per structure specific version define using the MAKE_NVAPI_VERSION macro.
// Usage: #define NV_GENLOCK_STATUS_VER  MAKE_NVAPI_VERSION(NV_GENLOCK_STATUS, 1)
// =========================================================================================
#define MAKE_NVAPI_VERSION(typeName,ver) (NvU32)(sizeof(typeName) | ((ver)<<16))
#define GET_NVAPI_VERSION(ver) (NvU32)((ver)>>16)
#define GET_NVAPI_SIZE(ver) (NvU32)((ver) & 0xffff)

// ====================================================
// NvAPI Status Values
//    All NvAPI functions return one of these codes.
// ====================================================


typedef enum
{
    NVAPI_OK                                    =  0,      // Success
    NVAPI_ERROR                                 = -1,      // Generic error
    NVAPI_LIBRARY_NOT_FOUND                     = -2,      // nvapi.dll can not be loaded
    NVAPI_NO_IMPLEMENTATION                     = -3,      // not implemented in current driver installation
    NVAPI_API_NOT_INITIALIZED                   = -4,      // NvAPI_Initialize has not been called (successfully)
    NVAPI_INVALID_ARGUMENT                      = -5,      // invalid argument
    NVAPI_NVIDIA_DEVICE_NOT_FOUND               = -6,      // no NVIDIA display driver was found
    NVAPI_END_ENUMERATION                       = -7,      // no more to enum
    NVAPI_INVALID_HANDLE                        = -8,      // invalid handle
    NVAPI_INCOMPATIBLE_STRUCT_VERSION           = -9,      // an argument's structure version is not supported
    NVAPI_HANDLE_INVALIDATED                    = -10,     // handle is no longer valid (likely due to GPU or display re-configuration)
    NVAPI_OPENGL_CONTEXT_NOT_CURRENT            = -11,     // no NVIDIA OpenGL context is current (but needs to be)
    NVAPI_INVALID_POINTER                       = -14,     // An invalid pointer, usually NULL, was passed as a parameter
    NVAPI_NO_GL_EXPERT                          = -12,     // OpenGL Expert is not supported by the current drivers
    NVAPI_INSTRUMENTATION_DISABLED              = -13,     // OpenGL Expert is supported, but driver instrumentation is currently disabled
    NVAPI_EXPECTED_LOGICAL_GPU_HANDLE           = -100,    // expected a logical GPU handle for one or more parameters
    NVAPI_EXPECTED_PHYSICAL_GPU_HANDLE          = -101,    // expected a physical GPU handle for one or more parameters
    NVAPI_EXPECTED_DISPLAY_HANDLE               = -102,    // expected an NV display handle for one or more parameters
    NVAPI_INVALID_COMBINATION                   = -103,    // used in some commands to indicate that the combination of parameters is not valid
    NVAPI_NOT_SUPPORTED                         = -104,    // Requested feature not supported in the selected GPU
    NVAPI_PORTID_NOT_FOUND                      = -105,    // NO port ID found for I2C transaction
    NVAPI_EXPECTED_UNATTACHED_DISPLAY_HANDLE    = -106,    // expected an unattached display handle as one of the input param
    NVAPI_INVALID_PERF_LEVEL                    = -107,    // invalid perf level
    NVAPI_DEVICE_BUSY                           = -108,    // device is busy, request not fulfilled
    NVAPI_NV_PERSIST_FILE_NOT_FOUND             = -109,    // NV persist file is not found
    NVAPI_PERSIST_DATA_NOT_FOUND                = -110,    // NV persist data is not found
    NVAPI_EXPECTED_TV_DISPLAY                   = -111,    // expected TV output display
    NVAPI_EXPECTED_TV_DISPLAY_ON_DCONNECTOR     = -112,    // expected TV output on D Connector - HDTV_EIAJ4120.
    NVAPI_NO_ACTIVE_SLI_TOPOLOGY                = -113,    // SLI is not active on this device
    NVAPI_SLI_RENDERING_MODE_NOTALLOWED         = -114,    // setup of SLI rendering mode is not possible right now
    NVAPI_EXPECTED_DIGITAL_FLAT_PANEL           = -115,    // expected digital flat panel
    NVAPI_ARGUMENT_EXCEED_MAX_SIZE              = -116,    // argument exceeds expected size
    NVAPI_DEVICE_SWITCHING_NOT_ALLOWED          = -117,    // inhibit ON due to one of the flags in NV_GPU_DISPLAY_CHANGE_INHIBIT or SLI Active
    NVAPI_TESTING_CLOCKS_NOT_SUPPORTED          = -118,    // testing clocks not supported
    NVAPI_UNKNOWN_UNDERSCAN_CONFIG              = -119,    // the specified underscan config is from an unknown source (e.g. INF)
    NVAPI_TIMEOUT_RECONFIGURING_GPU_TOPO        = -120,    // timeout while reconfiguring GPUs
    NVAPI_DATA_NOT_FOUND                        = -121,    // Requested data was not found
    NVAPI_EXPECTED_ANALOG_DISPLAY               = -122,    // expected analog display
    NVAPI_NO_VIDLINK                            = -123,    // No SLI video bridge present
    NVAPI_REQUIRES_REBOOT                       = -124,    // NVAPI requires reboot for its settings to take effect
    NVAPI_INVALID_HYBRID_MODE                   = -125,    // the function is not supported with the current hybrid mode.
    NVAPI_MIXED_TARGET_TYPES                    = -126,    // The target types are not all the same
    NVAPI_SYSWOW64_NOT_SUPPORTED                = -127,    // the function is not supported from 32-bit on a 64-bit system
    NVAPI_IMPLICIT_SET_GPU_TOPOLOGY_CHANGE_NOT_ALLOWED = -128,    //there is any implicit GPU topo active. Use NVAPI_SetHybridMode to change topology.
    NVAPI_REQUEST_USER_TO_CLOSE_NON_MIGRATABLE_APPS = -129,      //Prompt the user to close all non-migratable apps.
    NVAPI_OUT_OF_MEMORY                         = -130,    // Could not allocate sufficient memory to complete the call
    NVAPI_WAS_STILL_DRAWING                     = -131,    // The previous operation that is transferring information to or from this surface is incomplete
    NVAPI_FILE_NOT_FOUND                        = -132,    // The file was not found
    NVAPI_TOO_MANY_UNIQUE_STATE_OBJECTS         = -133,    // There are too many unique instances of a particular type of state object
    NVAPI_INVALID_CALL                          = -134,    // The method call is invalid. For example, a method's parameter may not be a valid pointer
    NVAPI_D3D10_1_LIBRARY_NOT_FOUND             = -135,    // d3d10_1.dll can not be loaded
    NVAPI_FUNCTION_NOT_FOUND                    = -136,    // Couldn't find the function in loaded dll library
    NVAPI_INVALID_USER_PRIVILEGE                = -137,    // Current User is not Admin
    NVAPI_EXPECTED_NON_PRIMARY_DISPLAY_HANDLE   = -138,    // The handle corresponds to GDIPrimary
    NVAPI_EXPECTED_COMPUTE_GPU_HANDLE           = -139,    // Setting Physx GPU requires that the GPU is compute capable
    NVAPI_STEREO_NOT_INITIALIZED                = -140,    // Stereo part of NVAPI failed to initialize completely. Check if stereo driver is installed.
    NVAPI_STEREO_REGISTRY_ACCESS_FAILED         = -141,    // Access to stereo related registry keys or values failed.
    NVAPI_STEREO_REGISTRY_PROFILE_TYPE_NOT_SUPPORTED = -142, // Given registry profile type is not supported.
    NVAPI_STEREO_REGISTRY_VALUE_NOT_SUPPORTED   = -143,    // Given registry value is not supported.
    NVAPI_STEREO_NOT_ENABLED                    = -144,    // Stereo is not enabled and function needed it to execute completely.
    NVAPI_STEREO_NOT_TURNED_ON                  = -145,    // Stereo is not turned on and function needed it to execute completely.
    NVAPI_STEREO_INVALID_DEVICE_INTERFACE       = -146,    // Invalid device interface.
    NVAPI_STEREO_PARAMETER_OUT_OF_RANGE         = -147,    // Separation percentage or JPEG image capture quality out of [0-100] range.
    NVAPI_STEREO_FRUSTUM_ADJUST_MODE_NOT_SUPPORTED = -148, // Given frustum adjust mode is not supported.
    NVAPI_TOPO_NOT_POSSIBLE                     = -149,    // The mosaic topo is not possible given the current state of HW
    NVAPI_MODE_CHANGE_FAILED                    = -150,    // An attempt to do a display resolution mode change has failed
    NVAPI_D3D11_LIBRARY_NOT_FOUND               = -151,    // d3d11.dll/d3d11_beta.dll cannot be loaded.
    NVAPI_INVALID_ADDRESS                       = -152,    // Address outside of valid range.
    NVAPI_STRING_TOO_SMALL                      = -153,    // The pre-allocated string is too small to hold the result.
    NVAPI_MATCHING_DEVICE_NOT_FOUND             = -154,    // The input does not match any of the available devices.
    NVAPI_DRIVER_RUNNING                        = -155,    // Driver is running
    NVAPI_DRIVER_NOTRUNNING                     = -156,    // Driver is not running
    NVAPI_ERROR_DRIVER_RELOAD_REQUIRED          = -157,    // A driver reload is required to apply these settings
    NVAPI_SET_NOT_ALLOWED                       = -158,    // Intended Setting is not allowed
    NVAPI_ADVANCED_DISPLAY_TOPOLOGY_REQUIRED    = -159,    // Information can't be returned due to "advanced display topology"
    NVAPI_SETTING_NOT_FOUND                     = -160,    // Setting is not found
    NVAPI_SETTING_SIZE_TOO_LARGE                = -161,    // Setting size is too large
    NVAPI_TOO_MANY_SETTINGS_IN_PROFILE          = -162,    // There are too many settings for a profile
    NVAPI_PROFILE_NOT_FOUND                     = -163,    // Profile is not found
    NVAPI_PROFILE_NAME_IN_USE                   = -164,    // Profile name is duplicated
    NVAPI_PROFILE_NAME_EMPTY                    = -165,    // Profile name is empty
    NVAPI_EXECUTABLE_NOT_FOUND                  = -166,    // Application not found in Profile
    NVAPI_EXECUTABLE_ALREADY_IN_USE             = -167,    // Application already exists in the other profile
    NVAPI_DATATYPE_MISMATCH                     = -168,    // Data Type mismatch
    NVAPI_PROFILE_REMOVED                       = -169,    /// The profile passed as parameter has been removed and is no longer valid.
    NVAPI_UNREGISTERED_RESOURCE                 = -170,    // An unregistered resource was passed as a parameter
    NVAPI_ID_OUT_OF_RANGE                       = -171,    // The DisplayId corresponds to a display which is not within the normal outputId range
    NVAPI_DISPLAYCONFIG_VALIDATION_FAILED       = -172,    // Display topology is not valid so we can't do a mode set on this config
    NVAPI_MOSAIC_NOT_ACTIVE                     = -176,    // The requested action cannot be performed without Mosaic being enabled
} NvAPI_Status;

#define NVAPI_API_NOT_INTIALIZED        NVAPI_API_NOT_INITIALIZED       // Fix typo in error code
#define NVAPI_INVALID_USER_PRIVILEDGE   NVAPI_INVALID_USER_PRIVILEGE    // Fix typo in error code


///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_Initialize
//
//   DESCRIPTION: Initializes NVAPI library. This must be called before any
//                other NvAPI_ function.
//
//  SUPPORTED OS: Mac OS X, Windows XP and higher
//
// RETURN STATUS: NVAPI_ERROR            Something is wrong during the initialization process (generic error)
//                NVAPI_LIBRARYNOTFOUND  Can not load nvapi.dll
//                NVAPI_OK                  Initialized
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_Initialize();

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_Unload
//
//   DESCRIPTION: Unloads NVAPI library. This must be the last function called.
//
//  SUPPORTED OS: Mac OS X, Windows XP and higher
//
//  !! This is not thread safe. In a multithreaded environment, calling NvAPI_Unload       !!
//  !! while another thread is executing another NvAPI_XXX function, results in           !!
//  !! undefined behaviour and might even cause the application to crash. Developers       !!
//  !! must make sure that they are not in any other function before calling NvAPI_Unload. !!
//
//
//  Unloading NvAPI library is not supported when the library is in a resource locked state.
//  Some functions in the NvAPI library initiates an operation or allocates certain resources
//  and there are corresponding functions available, to complete the operation or free the
//  allocated resources. All such function pairs are designed to prevent unloading NvAPI library.
//
//  For example, if NvAPI_Unload is called after NvAPI_XXX which locks a resource, it fails with
//  NVAPI_ERROR. Developers need to call the corresponding NvAPI_YYY to unlock the resources,
//  before calling NvAPI_Unload again.
//
//
// RETURN STATUS: NVAPI_ERROR            One or more resources are locked and hence cannot unload NVAPI library
//                NVAPI_OK               NVAPI library unloaded
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_Unload();

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_GetErrorMessage
//
//   DESCRIPTION: converts an NvAPI error code into a null terminated string
//
//  SUPPORTED OS: Mac OS X, Windows XP and higher
//
// RETURN STATUS: null terminated string (always, never NULL)
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_GetErrorMessage(NvAPI_Status nr,NvAPI_ShortString szDesc);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_GetInterfaceVersionString
//
//   DESCRIPTION: Returns a string describing the version of the NvAPI library.
//                Contents of the string are human readable.  Do not assume a fixed
//                format.
//
//  SUPPORTED OS: Mac OS X, Windows XP and higher
//
// RETURN STATUS: User readable string giving info on NvAPI's version
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_GetInterfaceVersionString(NvAPI_ShortString szDesc);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_GetDisplayDriverVersion
//
//   DESCRIPTION: Returns a struct that describes aspects of the display driver
//                build.
//
//  SUPPORTED OS: Mac OS X, Windows XP and higher
//
// RETURN STATUS: NVAPI_ERROR or NVAPI_OK
//
///////////////////////////////////////////////////////////////////////////////
typedef struct
{
    NvU32              version;             // Structure version
    NvU32              drvVersion;
    NvU32              bldChangeListNum;
    NvAPI_ShortString  szBuildBranchString;
    NvAPI_ShortString  szAdapterString;
} NV_DISPLAY_DRIVER_VERSION;
#define NV_DISPLAY_DRIVER_VERSION_VER  MAKE_NVAPI_VERSION(NV_DISPLAY_DRIVER_VERSION,1)
NVAPI_INTERFACE NvAPI_GetDisplayDriverVersion(NvDisplayHandle hNvDisplay, NV_DISPLAY_DRIVER_VERSION *pVersion);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_EnumNvidiaDisplayHandle
//
//   DESCRIPTION: Returns the handle of the NVIDIA display specified by the enum
//                index (thisEnum). The client should keep enumerating until it
//                returns NVAPI_END_ENUMERATION.
//
//                Note: Display handles can get invalidated on a modeset, so the calling applications need to
//                renum the handles after every modeset.
//
//  SUPPORTED OS: Windows XP and higher
//
// RETURN STATUS: NVAPI_INVALID_ARGUMENT: either the handle pointer is NULL or enum index too big
//                NVAPI_OK: return a valid NvDisplayHandle based on the enum index
//                NVAPI_NVIDIA_DEVICE_NOT_FOUND: no NVIDIA device found in the system
//                NVAPI_END_ENUMERATION: no more display device to enumerate.
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_EnumNvidiaDisplayHandle(NvU32 thisEnum, NvDisplayHandle *pNvDispHandle);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_EnumNvidiaUnAttachedDisplayHandle
//
//   DESCRIPTION: Returns the handle of the NVIDIA UnAttached display specified by the enum
//                index (thisEnum). The client should keep enumerating till it
//                return error.
//
//                Note: Display handles can get invalidated on a modeset, so the calling applications need to
//                renum the handles after every modeset.
//
//  SUPPORTED OS: Windows XP and higher
//
// RETURN STATUS: NVAPI_INVALID_ARGUMENT: either the handle pointer is NULL or enum index too big
//                NVAPI_OK: return a valid NvDisplayHandle based on the enum index
//                NVAPI_NVIDIA_DEVICE_NOT_FOUND: no NVIDIA device found in the system
//                NVAPI_END_ENUMERATION: no more display device to enumerate.
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_EnumNvidiaUnAttachedDisplayHandle(NvU32 thisEnum, NvUnAttachedDisplayHandle *pNvUnAttachedDispHandle);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_EnumPhysicalGPUs
//
//   DESCRIPTION: Returns an array of physical GPU handles.
//
//                Each handle represents a physical GPU present in the system.
//                That GPU may be part of a SLI configuration, or not be visible to the OS directly.
//
//                At least 1 GPU must be present in the system and running an NV display driver.
//
//                The array nvGPUHandle will be filled with physical GPU handle values.  The returned
//                gpuCount determines how many entries in the array are valid.
//
//                Note: In drivers older than 105.00, all physical GPU handles get invalidated on a modeset. So the calling applications
//                      need to renum the handles after every modeset.
//                      With drivers 105.00 and up all physical GPU handles are constant.
//                      Physical GPU handles are constant as long as the GPUs are not physically moved and the SBIOS VGA order is unchanged.
//
//  SUPPORTED OS: Mac OS X, Windows XP and higher
//
// RETURN STATUS: NVAPI_INVALID_ARGUMENT: nvGPUHandle or pGpuCount is NULL
//                NVAPI_OK: one or more handles were returned
//                NVAPI_NVIDIA_DEVICE_NOT_FOUND: no NVIDIA GPU driving a display was found
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_EnumPhysicalGPUs(NvPhysicalGpuHandle nvGPUHandle[NVAPI_MAX_PHYSICAL_GPUS], NvU32 *pGpuCount);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_EnumLogicalGPUs
//
//   DESCRIPTION: Returns an array of logical GPU handles.
//
//                Each handle represents one or more GPUs acting in concert as a single graphics device.
//
//                At least 1 GPU must be present in the system and running an NV display driver.
//
//                The array nvGPUHandle will be filled with logical GPU handle values.  The returned
//                gpuCount determines how many entries in the array are valid.
//
//                Note: All logical GPUs handles get invalidated on a GPU topology change, so the calling application is required to
//                renum the logical GPU handles to get latest physical handle mapping after every GPU topology change activated
//                by a call to NvAPI_SetGpuTopologies.
//
//                To detect if SLI rendering is enabled please use NvAPI_D3D_GetCurrentSLIState
//
//  SUPPORTED OS: Mac OS X, Windows XP and higher
//
// RETURN STATUS: NVAPI_INVALID_ARGUMENT: nvGPUHandle or pGpuCount is NULL
//                NVAPI_OK: one or more handles were returned
//                NVAPI_NVIDIA_DEVICE_NOT_FOUND: no NVIDIA GPU driving a display was found
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_EnumLogicalGPUs(NvLogicalGpuHandle nvGPUHandle[NVAPI_MAX_LOGICAL_GPUS], NvU32 *pGpuCount);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_GetPhysicalGPUsFromDisplay
//
//   DESCRIPTION: Returns an array of physical GPU handles associated with the specified display.
//
//                At least 1 GPU must be present in the system and running an NV display driver.
//
//                The array nvGPUHandle will be filled with physical GPU handle values.  The returned
//                gpuCount determines how many entries in the array are valid.
//
//                If the display corresponds to more than one physical GPU, the first GPU returned
//                is the one with the attached active output.
//
//  SUPPORTED OS: Windows XP and higher
//
// RETURN STATUS: NVAPI_INVALID_ARGUMENT: hNvDisp is not valid; nvGPUHandle or pGpuCount is NULL
//                NVAPI_OK: one or more handles were returned
//                NVAPI_NVIDIA_DEVICE_NOT_FOUND: no NVIDIA GPU driving a display was found
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_GetPhysicalGPUsFromDisplay(NvDisplayHandle hNvDisp, NvPhysicalGpuHandle nvGPUHandle[NVAPI_MAX_PHYSICAL_GPUS], NvU32 *pGpuCount);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_GetPhysicalGPUFromUnAttachedDisplay
//
//   DESCRIPTION: Returns a physical source GPU handle associated with the specified unattached display.
//                The source GPU is a physical render GPU which renders the frame buffer but may or may not drive the scan out.
//
//                At least 1 GPU must be present in the system and running an NV display driver.
//
//  SUPPORTED OS: Windows XP and higher
//
// RETURN STATUS: NVAPI_INVALID_ARGUMENT: hNvUnAttachedDisp is not valid or pPhysicalGpu is NULL.
//                NVAPI_OK: one or more handles were returned
//                NVAPI_NVIDIA_DEVICE_NOT_FOUND: no NVIDIA GPU driving a display was found
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_GetPhysicalGPUFromUnAttachedDisplay(NvUnAttachedDisplayHandle hNvUnAttachedDisp, NvPhysicalGpuHandle *pPhysicalGpu);


///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_CreateDisplayFromUnAttachedDisplay
//
//   DESCRIPTION: The unattached display handle is converted to a active attached display handle.
//
//                At least 1 GPU must be present in the system and running an NV display driver.
//
//  SUPPORTED OS: Windows XP and higher
//
// RETURN STATUS: NVAPI_INVALID_ARGUMENT: hNvUnAttachedDisp is not valid or pNvDisplay is NULL.
//                NVAPI_OK: one or more handles were returned
//                NVAPI_NVIDIA_DEVICE_NOT_FOUND: no NVIDIA GPU driving a display was found
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_CreateDisplayFromUnAttachedDisplay(NvUnAttachedDisplayHandle hNvUnAttachedDisp, NvDisplayHandle *pNvDisplay);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_GetLogicalGPUFromDisplay
//
//   DESCRIPTION: Returns the logical GPU handle associated with the specified display.
//
//                At least 1 GPU must be present in the system and running an NV display driver.
//
//  SUPPORTED OS: Windows XP and higher
//
// RETURN STATUS: NVAPI_INVALID_ARGUMENT: hNvDisp is not valid; pLogicalGPU is NULL
//                NVAPI_OK: one or more handles were returned
//                NVAPI_NVIDIA_DEVICE_NOT_FOUND: no NVIDIA GPU driving a display was found
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_GetLogicalGPUFromDisplay(NvDisplayHandle hNvDisp, NvLogicalGpuHandle *pLogicalGPU);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_GetLogicalGPUFromPhysicalGPU
//
//   DESCRIPTION: Returns the logical GPU handle associated with specified physical GPU handle.
//
//                At least 1 GPU must be present in the system and running an NV display driver.
//
//  SUPPORTED OS: Mac OS X, Windows XP and higher
//
// RETURN STATUS: NVAPI_INVALID_ARGUMENT: hPhysicalGPU is not valid; pLogicalGPU is NULL
//                NVAPI_OK: one or more handles were returned
//                NVAPI_NVIDIA_DEVICE_NOT_FOUND: no NVIDIA GPU driving a display was found
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_GetLogicalGPUFromPhysicalGPU(NvPhysicalGpuHandle hPhysicalGPU, NvLogicalGpuHandle *pLogicalGPU);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_GetPhysicalGPUsFromLogicalGPU
//
//   DESCRIPTION: Returns the physical GPU handles associated with the specified logical GPU handle.
//
//                At least 1 GPU must be present in the system and running an NV display driver.
//
//                The array hPhysicalGPU will be filled with physical GPU handle values.  The returned
//                gpuCount determines how many entries in the array are valid.
//
//  SUPPORTED OS: Mac OS X, Windows XP and higher
//
// RETURN STATUS: NVAPI_INVALID_ARGUMENT: hLogicalGPU is not valid; hPhysicalGPU is NULL
//                NVAPI_OK: one or more handles were returned
//                NVAPI_NVIDIA_DEVICE_NOT_FOUND: no NVIDIA GPU driving a display was found
//                NVAPI_EXPECTED_LOGICAL_GPU_HANDLE: hLogicalGPU was not a logical GPU handle
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_GetPhysicalGPUsFromLogicalGPU(NvLogicalGpuHandle hLogicalGPU,NvPhysicalGpuHandle hPhysicalGPU[NVAPI_MAX_PHYSICAL_GPUS], NvU32 *pGpuCount);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_GetAssociatedNvidiaDisplayHandle
//
//   DESCRIPTION: Returns the handle of the NVIDIA display that is associated
//                with the display name given.  Eg: "\\DISPLAY1"
//
//  SUPPORTED OS: Windows XP and higher
//
// RETURN STATUS: NVAPI_INVALID_ARGUMENT: either argument is NULL
//                NVAPI_OK: *pNvDispHandle is now valid
//                NVAPI_NVIDIA_DEVICE_NOT_FOUND: no NVIDIA device maps to that display name
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_GetAssociatedNvidiaDisplayHandle(const char *szDisplayName, NvDisplayHandle *pNvDispHandle);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_DISP_GetAssociatedUnAttachedNvidiaDisplayHandle
//
//   DESCRIPTION: Returns the handle of an unattached NVIDIA display that is
//                associated with the display name given.  Eg: "\\DISPLAY1"
//
//  SUPPORTED OS: Windows XP and higher
//
// RETURN STATUS: NVAPI_INVALID_ARGUMENT: either argument is NULL
//                NVAPI_OK: *pNvUnAttachedDispHandle is now valid
//                NVAPI_NVIDIA_DEVICE_NOT_FOUND: no NVIDIA device maps to that display name
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_DISP_GetAssociatedUnAttachedNvidiaDisplayHandle(const char *szDisplayName, NvUnAttachedDisplayHandle *pNvUnAttachedDispHandle);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_GetAssociatedNvidiaDisplayName
//
//   DESCRIPTION: Returns the display name given.  Eg: "\\DISPLAY1" using the NVIDIA display handle
//
//  SUPPORTED OS: Windows XP and higher
//
// RETURN STATUS: NVAPI_INVALID_ARGUMENT: either argument is NULL
//                NVAPI_OK: *pNvDispHandle is now valid
//                NVAPI_NVIDIA_DEVICE_NOT_FOUND: no NVIDIA device maps to that display name
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_GetAssociatedNvidiaDisplayName(NvDisplayHandle NvDispHandle, NvAPI_ShortString szDisplayName);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_GetUnAttachedAssociatedDisplayName
//
//   DESCRIPTION: Returns the display name given.  Eg: "\\DISPLAY1" using the NVIDIA unattached display handle
//
//  SUPPORTED OS: Windows XP and higher
//
// RETURN STATUS: NVAPI_INVALID_ARGUMENT: either argument is NULL
//                NVAPI_OK: *pNvDispHandle is now valid
//                NVAPI_NVIDIA_DEVICE_NOT_FOUND: no NVIDIA device maps to that display name
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_GetUnAttachedAssociatedDisplayName(NvUnAttachedDisplayHandle hNvUnAttachedDisp, NvAPI_ShortString szDisplayName);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_EnableHWCursor
//
//   DESCRIPTION: Enable hardware cursor support
//
//  SUPPORTED OS: Windows XP
//
// RETURN STATUS: NVAPI_ERROR or NVAPI_OK
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_EnableHWCursor(NvDisplayHandle hNvDisplay);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_DisableHWCursor
//
//   DESCRIPTION: Disable hardware cursor support
//
//  SUPPORTED OS: Windows XP and higher
//
// RETURN STATUS: NVAPI_ERROR or NVAPI_OK
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_DisableHWCursor(NvDisplayHandle hNvDisplay);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_GetVBlankCounter
//
//   DESCRIPTION: get vblank counter
//
//  SUPPORTED OS: Windows XP and higher
//
// RETURN STATUS: NVAPI_ERROR or NVAPI_OK
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_GetVBlankCounter(NvDisplayHandle hNvDisplay, NvU32 *pCounter);

///////////////////////////////////////////////////////////////////////////////
// FUNCTION NAME: NvAPI_SetRefreshRateOverride
//   DESCRIPTION: Override the refresh rate on the given  display/outputsMask.
//                The new refresh rate can be applied right away in this API call or deferred to happen with the
//                next OS modeset. The override is only good for one modeset (doesn't matter it's deferred or immediate).
//
//  SUPPORTED OS: Windows XP
//
//
//         INPUT: hNvDisplay - the NVIDIA display handle. It can be NVAPI_DEFAULT_HANDLE or a handle
//                             enumerated from NvAPI_EnumNVidiaDisplayHandle().
//
//                outputsMask - a set of bits that identify all target outputs which are associated with the NVIDIA
//                              display handle to apply the refresh rate override. Note when SLI is enabled,  the
//                              outputsMask only applies to the GPU that is driving the display output.
//
//                refreshRate - the override value. "0.0" means cancel the override.
//
//
//                bSetDeferred - "0": apply the refresh rate override immediately in this API call.
//                               "1":  apply refresh rate at the next OS modeset.
//
// RETURN STATUS: NVAPI_INVALID_ARGUMENT: hNvDisplay or outputsMask is invalid
//                NVAPI_OK: the refresh rate override is correct set
//                NVAPI_ERROR: the operation failed
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_SetRefreshRateOverride(NvDisplayHandle hNvDisplay, NvU32 outputsMask, float refreshRate, NvU32 bSetDeferred);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_GetAssociatedDisplayOutputId
//
//   DESCRIPTION: Gets the active outputId associated with the display handle.
//
//  SUPPORTED OS: Windows XP and higher
//
//    PARAMETERS: hNvDisplay(IN) - NVIDIA Display selection. It can be NVAPI_DEFAULT_HANDLE or a handle enumerated from NvAPI_EnumNVidiaDisplayHandle().
//                outputId(OUT)  - The active display output id associated with the selected display handle hNvDisplay.
//                                 The outputid will have only one bit set. In case of clone or span this  will indicate the display
//                                 outputId of the primary display that the GPU is driving.
// RETURN STATUS: NVAPI_OK: call successful.
//                NVAPI_NVIDIA_DEVICE_NOT_FOUND: no NVIDIA GPU driving a display was found.
//                NVAPI_EXPECTED_DISPLAY_HANDLE: hNvDisplay is not a valid display handle.
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_GetAssociatedDisplayOutputId(NvDisplayHandle hNvDisplay, NvU32 *pOutputId);


///////////////////////////////////////////////////////////////////////////////
// FUNCTION NAME:   NvAPI_GetDisplayPortInfo
//
// DESCRIPTION:     This API returns the current DP related into on the specified device(monitor)
//
//  SUPPORTED OS: Windows XP and higher
//
// PARAMETERS:      hNvDisplay(IN) - NVIDIA Display selection. It can be NVAPI_DEFAULT_HANDLE or a handle enumerated from NvAPI_EnumNVidiaDisplayHandle().
//                  outputId(IN)   - The display output id. If it's "0" then the default outputId from NvAPI_GetAssociatedDisplayOutputId() will be used.
//                  pInfo(OUT)     - The display port info
//
// RETURN STATUS:
//                  NVAPI_OK - completed request
//                  NVAPI_ERROR - miscellaneous error occurred
//                  NVAPI_INVALID_ARGUMENT: Invalid input parameter.
//
///////////////////////////////////////////////////////////////////////////////
typedef enum
{
    NV_DP_1_62GBPS            = 6,
    NV_DP_2_70GBPS            = 0xA,
} NV_DP_LINK_RATE;

typedef enum
{
    NV_DP_1_LANE              = 1,
    NV_DP_2_LANE              = 2,
    NV_DP_4_LANE              = 4,
} NV_DP_LANE_COUNT;

typedef enum
{
    NV_DP_COLOR_FORMAT_RGB     = 0,
    NV_DP_COLOR_FORMAT_YCbCr422,
    NV_DP_COLOR_FORMAT_YCbCr444,
} NV_DP_COLOR_FORMAT;

typedef enum
{
    NV_DP_COLORIMETRY_RGB = 0,
    NV_DP_COLORIMETRY_YCbCr_ITU601,
    NV_DP_COLORIMETRY_YCbCr_ITU709,
} NV_DP_COLORIMETRY;

typedef enum
{
    NV_DP_DYNAMIC_RANGE_VESA  = 0,
    NV_DP_DYNAMIC_RANGE_CEA,
} NV_DP_DYNAMIC_RANGE;

typedef enum
{
    NV_DP_BPC_DEFAULT         = 0,
    NV_DP_BPC_6,
    NV_DP_BPC_8,
    NV_DP_BPC_10,
    NV_DP_BPC_12,
    NV_DP_BPC_16,
} NV_DP_BPC;

typedef struct
{
    NvU32               version;                     // structure version
    NvU32               dpcd_ver;                    // the DPCD version of the monitor
    NV_DP_LINK_RATE     maxLinkRate;                 // the max supported link rate
    NV_DP_LANE_COUNT    maxLaneCount;                // the max supported lane count
    NV_DP_LINK_RATE     curLinkRate;                 // the current link rate
    NV_DP_LANE_COUNT    curLaneCount;                // the current lane count
    NV_DP_COLOR_FORMAT  colorFormat;                 // the current color format
    NV_DP_DYNAMIC_RANGE dynamicRange;                // the dynamic range
    NV_DP_COLORIMETRY   colorimetry;                 // ignored in RGB space
    NV_DP_BPC           bpc;                         // the current bit-per-component;
    NvU32               isDp                   : 1;  // if the monitor is driven by display port
    NvU32               isInternalDp           : 1;  // if the monitor is driven by NV Dp transmitter
    NvU32               isColorCtrlSupported   : 1;  // if the color format change is supported
    NvU32               is6BPCSupported        : 1;  // if 6 bpc is supported
    NvU32               is8BPCSupported        : 1;  // if 8 bpc is supported
    NvU32               is10BPCSupported       : 1;  // if 10 bpc is supported
    NvU32               is12BPCSupported       : 1;  // if 12 bpc is supported
    NvU32               is16BPCSupported       : 1;  // if 16 bpc is supported
    NvU32               isYCrCb422Supported    : 1;  // if YCrCb422 is supported
    NvU32               isYCrCb444Supported    : 1;  // if YCrCb444 is supported
 } NV_DISPLAY_PORT_INFO;

#define NV_DISPLAY_PORT_INFO_VER   MAKE_NVAPI_VERSION(NV_DISPLAY_PORT_INFO,1)

NVAPI_INTERFACE NvAPI_GetDisplayPortInfo(NvDisplayHandle hNvDisplay, NvU32 outputId, NV_DISPLAY_PORT_INFO *pInfo);

///////////////////////////////////////////////////////////////////////////////
// FUNCTION NAME:   NvAPI_SetDisplayPort
//
// DESCRIPTION:     This API is used to setup DP related configurations.
//
//  SUPPORTED OS: Windows XP and higher
//
// PARAMETERS:      hNvDisplay(IN) - NVIDIA display handle. It can be NVAPI_DEFAULT_HANDLE or a handle enumerated from NvAPI_EnumNVidiaDisplayHandle().
//                  outputId(IN)   - This display output ID, when it's "0" it means the default outputId generated from the return of NvAPI_GetAssociatedDisplayOutputId().
//                  pCfg(IN)       - The display port config structure. If pCfg is NULL, it means to use the driver's default value to setup.
//
// RETURN STATUS:
//                  NVAPI_OK - completed request
//                  NVAPI_ERROR - miscellaneous error occurred
//                  NVAPI_INVALID_ARGUMENT: Invalid input parameter.
///////////////////////////////////////////////////////////////////////////////
typedef struct
{
    NvU32               version;                     // structure version - 2 is latest
    NV_DP_LINK_RATE     linkRate;                    // the link rate
    NV_DP_LANE_COUNT    laneCount;                   // the lane count
    NV_DP_COLOR_FORMAT  colorFormat;                 // the color format to set
    NV_DP_DYNAMIC_RANGE dynamicRange;                // the dynamic range
    NV_DP_COLORIMETRY   colorimetry;                 // ignored in RGB space
    NV_DP_BPC           bpc;                         // the current bit-per-component;
    NvU32               isHPD               : 1;     // if CPL is making this call due to HPD
    NvU32               isSetDeferred       : 1;     // requires an OS modeset to finalize the setup if set
    NvU32               isChromaLpfOff      : 1;     // force the chroma low_pass_filter to be off
    NvU32               isDitherOff         : 1;     // force to turn off dither
    NvU32               testLinkTrain       : 1;     // if testing mode, skip validation
    NvU32               testColorChange     : 1;     // if testing mode, skip validation
} NV_DISPLAY_PORT_CONFIG;

#define NV_DISPLAY_PORT_CONFIG_VER   MAKE_NVAPI_VERSION(NV_DISPLAY_PORT_CONFIG,2)
#define NV_DISPLAY_PORT_CONFIG_VER_1 MAKE_NVAPI_VERSION(NV_DISPLAY_PORT_CONFIG,1)
#define NV_DISPLAY_PORT_CONFIG_VER_2 MAKE_NVAPI_VERSION(NV_DISPLAY_PORT_CONFIG,2)

NVAPI_INTERFACE NvAPI_SetDisplayPort(NvDisplayHandle hNvDisplay, NvU32 outputId, NV_DISPLAY_PORT_CONFIG *pCfg);

///////////////////////////////////////////////////////////////////////////////
// FUNCTION NAME:   NvAPI_GetHDMISupportInfo
//
// DESCRIPTION:     This API returns the current infoframe data on the specified device(monitor)
//
//  SUPPORTED OS: Windows Vista and higher
//
// PARAMETERS:      hNvDisplay(IN) - NVIDIA Display selection. It can be NVAPI_DEFAULT_HANDLE or a handle enumerated from NvAPI_EnumNVidiaDisplayHandle().
//                  outputId(IN)   - The display output id. If it's "0" then the default outputId from NvAPI_GetAssociatedDisplayOutputId() will be used.
//                  pInfo(OUT)     - The monitor and GPU's HDMI support info
//
// RETURN STATUS:
//                  NVAPI_OK - completed request
//                  NVAPI_ERROR - miscellaneous error occurred
//                  NVAPI_INVALID_ARGUMENT: Invalid input parameter.
//
///////////////////////////////////////////////////////////////////////////////
typedef struct
{
    NvU32      version;                     // structure version
    NvU32      isGpuHDMICapable       : 1;  // if the GPU can handle HDMI
    NvU32      isMonUnderscanCapable  : 1;  // if the monitor supports underscan
    NvU32      isMonBasicAudioCapable : 1;  // if the monitor supports basic audio
    NvU32      isMonYCbCr444Capable   : 1;  // if YCbCr 4:4:4 is supported
    NvU32      isMonYCbCr422Capable   : 1;  // if YCbCr 4:2:2 is supported
    NvU32      isMonxvYCC601Capable   : 1;  // if xvYCC 601 is supported
    NvU32      isMonxvYCC709Capable   : 1;  // if xvYCC 709 is supported
    NvU32      isMonHDMI              : 1;  // if the monitor is HDMI (with IEEE's HDMI registry ID)
    NvU32      EDID861ExtRev;               // the revision number of the EDID 861 extension
 } NV_HDMI_SUPPORT_INFO;

#define NV_HDMI_SUPPORT_INFO_VER  MAKE_NVAPI_VERSION(NV_HDMI_SUPPORT_INFO,1)

NVAPI_INTERFACE NvAPI_GetHDMISupportInfo(NvDisplayHandle hNvDisplay, NvU32 outputId, NV_HDMI_SUPPORT_INFO *pInfo);


///////////////////////////////////////////////////////////////////////////////
// FUNCTION NAME:   NvAPI_DISP_GetMonitorCapabilities
//
// DESCRIPTION:     This API returns the Monitor capabilities
//
//  SUPPORTED OS: Windows Vista and higher
//
// PARAMETERS:      displayId(IN)                - Monitor Identifier
//                  pMonitorCapabilities(OUT)     - The monitor support info
//
// RETURN STATUS:
//                  NVAPI_OK - completed request
//                  NVAPI_ERROR - miscellaneous error occurred
//                  NVAPI_INVALID_ARGUMENT: Invalid input parameter.
//
///////////////////////////////////////////////////////////////////////////////

typedef enum
{
    // hdmi related caps
    NV_MONITOR_CAPS_TYPE_HDMI_VSDB = 0x1000,
    NV_MONITOR_CAPS_TYPE_HDMI_VCDB = 0x1001
} NV_MONITOR_CAPS_TYPE;

typedef enum
{
    NV_MONITOR_CONN_TYPE_UNINITIALIZED = 0,
    NV_MONITOR_CONN_TYPE_VGA,
    NV_MONITOR_CONN_TYPE_COMPONENT,
    NV_MONITOR_CONN_TYPE_SVIDEO,
    NV_MONITOR_CONN_TYPE_HDMI,
    NV_MONITOR_CONN_TYPE_DVI,
    NV_MONITOR_CONN_TYPE_LVDS,
    NV_MONITOR_CONN_TYPE_DP,
    NV_MONITOR_CONN_TYPE_COMPOSITE,
    NV_MONITOR_CONN_TYPE_UNKNOWN =  -1
} NV_MONITOR_CONN_TYPE;

typedef struct _NV_MONITOR_CAPS_VCDB
{
    NvU8    quantizationRangeYcc         : 1;
    NvU8    quantizationRangeRgb         : 1;
    NvU8    scanInfoPreferredVideoFormat : 2;
    NvU8    scanInfoITVideoFormats       : 2;
    NvU8    scanInfoCEVideoFormats       : 2;
} NV_MONITOR_CAPS_VCDB;

typedef struct _NV_MONITOR_CAPS_VSDB
{
    // byte 1
    NvU8    sourcePhysicalAddressB         : 4;
    NvU8    sourcePhysicalAddressA         : 4;
    // byte 2
    NvU8    sourcePhysicalAddressD         : 4;
    NvU8    sourcePhysicalAddressC         : 4;
    // byte 3
    NvU8    supportDualDviOperation        : 1;
    NvU8    reserved6                      : 2;
    NvU8    supportDeepColorYCbCr444       : 1;
    NvU8    supportDeepColor30bits         : 1;
    NvU8    supportDeepColor36bits         : 1;
    NvU8    supportDeepColor48bits         : 1;
    NvU8    supportAI                      : 1;
    // byte 4
    NvU8    maxTmdsClock;
    // byte 5
    NvU8    cnc0SupportGraphicsTextContent : 1;
    NvU8    cnc1SupportPhotoContent        : 1;
    NvU8    cnc2SupportCinemaContent       : 1;
    NvU8    cnc3SupportGameContent         : 1;
    NvU8    reserved8                      : 1;
    NvU8    hasVicEntries                  : 1;
    NvU8    hasInterlacedLatencyField      : 1;
    NvU8    hasLatencyField                : 1;
    // byte 6
    NvU8    videoLatency;
    // byte 7
    NvU8    audioLatency;
    // byte 8
    NvU8    interlacedVideoLatency;
    // byte 9
    NvU8    interlacedAudioLatency;
    // byte 10
    NvU8    reserved13                     : 7;
    NvU8    has3dEntries                   : 1;
    // byte 11
    NvU8    hdmi3dLength                   : 5;
    NvU8    hdmiVicLength                  : 3;
    // Remaining bytes
    NvU8    hdmi_vic[7];  // Keeping maximum length for 3 bits
    NvU8    hdmi_3d[31];  // Keeping maximum length for 5 bits
} NV_MONITOR_CAPS_VSDB;

typedef struct _NV_MONITOR_CAPABILITIES
{
    NvU32    version;
    NvU16    size;
    NvU32    infoType;
    NvU32    connectorType;        // out: vga, tv, dvi, hdmi, dp
    NvU8     bIsValidInfo : 1;     // boolean : Returns invalid if requested info is not present such as VCDB not present
    union {
        NV_MONITOR_CAPS_VSDB  vsdb;
        NV_MONITOR_CAPS_VCDB  vcdb;
    } data;
} NV_MONITOR_CAPABILITIES;

#define NV_MONITOR_CAPABILITIES_VER   MAKE_NVAPI_VERSION(NV_MONITOR_CAPABILITIES,1)

NVAPI_INTERFACE NvAPI_DISP_GetMonitorCapabilities(NvU32 displayId, NV_MONITOR_CAPABILITIES *pMonitorCapabilities);



///////////////////////////////////////////////////////////////////////////////
// FUNCTION NAME:   NvAPI_GetInfoFrame
//
// DESCRIPTION:     This API returns the current infoframe data on the specified device(monitor)
//
//  SUPPORTED OS: Windows XP and higher
//
// PARAMETERS:      hNvDisplay(IN) - NVIDIA Display selection. It can be NVAPI_DEFAULT_HANDLE or a handle enumerated from NvAPI_EnumNVidiaDisplayHandle().
//                  outputId(IN)   - The display output id. If it's "0" then the default outputId from NvAPI_GetAssociatedDisplayOutputId() will be used.
//                  type(IN)       - The type of infoframe to set
//                  pInfoFrame(IN) - The infoframe data
//
// RETURN STATUS:
//                  NVAPI_OK - completed request
//                  NVAPI_ERROR - miscellaneous error occurred
//                  NVAPI_INVALID_ARGUMENT: Invalid input parameter.
//
///////////////////////////////////////////////////////////////////////////////
typedef enum _NV_INFOFRAME_TYPE
{
    NV_INFOFRAME_TYPE_AVI   = 2,
    NV_INFOFRAME_TYPE_SPD   = 3,
    NV_INFOFRAME_TYPE_AUDIO = 4,
    NV_INFOFRAME_TYPE_MS    = 5,
} NV_INFOFRAME_TYPE;

typedef struct
{
    NvU8 type;
    NvU8 version;
    NvU8 length;
} NV_INFOFRAME_HEADER;

// since this is for Windows OS so far, we use this bit little endian defination
// to handle the translation
typedef struct
{
    // byte 1
    NvU8 channelCount         : 3;
    NvU8 rsvd_bits_byte1      : 1;
    NvU8 codingType           : 4;

    // byte 2
    NvU8 sampleSize           : 2;
    NvU8 sampleRate           : 3;
    NvU8 rsvd_bits_byte2      : 3;

    // byte 3
    NvU8 codingExtensionType  : 5;
    NvU8 rsvd_bits_byte3      : 3;

    // byte 4
    NvU8  speakerPlacement;

    // byte 5
    NvU8 lfePlaybackLevel     : 2;
    NvU8 rsvd_bits_byte5      : 1;
    NvU8 levelShift           : 4;
    NvU8 downmixInhibit       : 1;

    // byte 6~10
    NvU8 rsvd_byte6;
    NvU8 rsvd_byte7;
    NvU8 rsvd_byte8;
    NvU8 rsvd_byte9;
    NvU8 rsvd_byte10;

}NV_AUDIO_INFOFRAME;

typedef struct
{
    // byte 1
    NvU8 scanInfo                : 2;
    NvU8 barInfo                 : 2;
    NvU8 activeFormatInfoPresent : 1;
    NvU8 colorSpace              : 2;
    NvU8 rsvd_bits_byte1         : 1;

    // byte 2
    NvU8 activeFormatAspectRatio : 4;
    NvU8 picAspectRatio          : 2;
    NvU8 colorimetry             : 2;

    // byte 3
    NvU8 nonuniformScaling       : 2;
    NvU8 rgbQuantizationRange    : 2;
    NvU8 extendedColorimetry     : 3;
    NvU8 itContent               : 1;

    // byte 4
    NvU8 vic                     : 7;
    NvU8 rsvd_bits_byte4         : 1;

    // byte 5
    NvU8 pixelRepeat             : 4;
    NvU8 contentTypes            : 2;
    NvU8 yccQuantizationRange    : 2;

    // byte 6~13
    NvU8 topBarLow;
    NvU8 topBarHigh;
    NvU8 bottomBarLow;
    NvU8 bottomBarHigh;
    NvU8 leftBarLow;
    NvU8 leftBarHigh;
    NvU8 rightBarLow;
    NvU8 rightBarHigh;

} NV_VIDEO_INFOFRAME;

typedef struct
{
    NV_INFOFRAME_HEADER    header;
    union
    {
        NV_AUDIO_INFOFRAME audio;
        NV_VIDEO_INFOFRAME video;
    }u;
} NV_INFOFRAME;
NVAPI_INTERFACE NvAPI_GetInfoFrame(NvDisplayHandle hNvDisplay, NvU32 outputId, NV_INFOFRAME_TYPE type, NV_INFOFRAME *pInfoFrame);

///////////////////////////////////////////////////////////////////////////////
// FUNCTION NAME:   NvAPI_SetInfoFrame
//
// DESCRIPTION:     This API returns the current infoframe data on the specified device(monitor)
//
//  SUPPORTED OS: Windows XP and higher
//
// PARAMETERS:      hNvDisplay(IN) - NVIDIA Display selection. It can be NVAPI_DEFAULT_HANDLE or a handle enumerated from NvAPI_EnumNVidiaDisplayHandle().
//                  outputId(IN)   - The display output id. If it's "0" then the default outputId from NvAPI_GetAssociatedDisplayOutputId() will be used.
//                  type(IN)       - The type of infoframe to set
//                  pInfoFrame(IN) - The infoframe data, NULL mean reset to the default value.
//
// RETURN STATUS:
//                  NVAPI_OK - completed request
//                  NVAPI_ERROR - miscellaneous error occurred
//                  NVAPI_INVALID_ARGUMENT: Invalid input parameter.
//
///////////////////////////////////////////////////////////////////////////////

NVAPI_INTERFACE NvAPI_SetInfoFrame(NvDisplayHandle hNvDisplay, NvU32 outputId, NV_INFOFRAME_TYPE type, NV_INFOFRAME *pInfoFrame);


///////////////////////////////////////////////////////////////////////////////
// FUNCTION NAME:   NvAPI_Disp_InfoFrameControl
//
// DESCRIPTION:     Controls the InfoFrame values
//
//  SUPPORTED OS: Windows Vista and higher
//
// PARAMETERS:      displayId(IN)           - Monitor Identifier
//                  pInfoframeData(IN/OUT)  - Contains data corresponding to InfoFrame
//
// RETURN STATUS:
//                  NVAPI_OK - completed request
//                  NVAPI_ERROR - miscellaneous error occurred
//                  NVAPI_INVALID_ARGUMENT: Invalid input parameter.
//
///////////////////////////////////////////////////////////////////////////////

typedef enum
{
    NV_INFOFRAME_CMD_GET_DEFAULT = 0,     // returns the fields in the infoframe with values set by manufacturer:nvidia/oem
    NV_INFOFRAME_CMD_RESET,               // sets the fields in the infoframe to auto, and infoframe to default infoframe for use in a set
    NV_INFOFRAME_CMD_GET,                 // get the current infoframe state
    NV_INFOFRAME_CMD_SET,                 // set the current infoframe state (flushed to the monitor), the values are one time and does not persist
    NV_INFOFRAME_CMD_GET_OVERRIDE,        // get the override infoframe state, non-override fields will be set to value = AUTO, overridden fields will have the current override values
    NV_INFOFRAME_CMD_SET_OVERRIDE,        // set the override infoframe state, non-override fields will be set to value = AUTO, other values indicate override; persist across modeset/reboot
    NV_INFOFRAME_CMD_GET_PROPERTY,        // get properties associated with infoframe (each of the infoframe type will have properties)
    NV_INFOFRAME_CMD_SET_PROPERTY,        // set properties associated with infoframe
} NV_INFOFRAME_CMD;


typedef enum
{
    NV_INFOFRAME_PROPERTY_MODE_AUTO           = 0, // driver determines whether to send infoframes
    NV_INFOFRAME_PROPERTY_MODE_ENABLE,             // driver always sends infoframe
    NV_INFOFRAME_PROPERTY_MODE_DISABLE,            // driver always don't send infoframe
    NV_INFOFRAME_PROPERTY_MODE_ALLOW_OVERRIDE,     // driver only sends infoframe when client request thru infoframe escape call
} NV_INFOFRAME_PROPERTY_MODE;

// returns whether the current monitor is in blacklist or force this monitor to be in blacklist
typedef enum
{
    NV_INFOFRAME_PROPERTY_BLACKLIST_FALSE = 0,
    NV_INFOFRAME_PROPERTY_BLACKLIST_TRUE,
} NV_INFOFRAME_PROPERTY_BLACKLIST;

typedef struct
{
    NvU32 mode      :  4;
    NvU32 blackList :  2;
    NvU32 reserved  : 10;
    NvU32 version   :  8;
    NvU32 length    :  8;
} NV_INFOFRAME_PROPERTY;

// byte1 related
typedef enum
{
    NV_INFOFRAME_FIELD_VALUE_AVI_SCANINFO_NODATA    = 0,
    NV_INFOFRAME_FIELD_VALUE_AVI_SCANINFO_OVERSCAN,
    NV_INFOFRAME_FIELD_VALUE_AVI_SCANINFO_UNDERSCAN,
    NV_INFOFRAME_FIELD_VALUE_AVI_SCANINFO_FUTURE,
    NV_INFOFRAME_FIELD_VALUE_AVI_SCANINFO_AUTO      = 7
} NV_INFOFRAME_FIELD_VALUE_AVI_SCANINFO;


typedef enum
{
    NV_INFOFRAME_FIELD_VALUE_AVI_BARDATA_NOT_PRESENT         = 0,
    NV_INFOFRAME_FIELD_VALUE_AVI_BARDATA_VERTICAL_PRESENT,
    NV_INFOFRAME_FIELD_VALUE_AVI_BARDATA_HORIZONTAL_PRESENT,
    NV_INFOFRAME_FIELD_VALUE_AVI_BARDATA_BOTH_PRESENT,
    NV_INFOFRAME_FIELD_VALUE_AVI_BARDATA_AUTO                = 7
} NV_INFOFRAME_FIELD_VALUE_AVI_BARDATA;

typedef enum
{
    NV_INFOFRAME_FIELD_VALUE_AVI_AFI_ABSENT   = 0,
    NV_INFOFRAME_FIELD_VALUE_AVI_AFI_PRESENT,
    NV_INFOFRAME_FIELD_VALUE_AVI_AFI_AUTO     = 3
} NV_INFOFRAME_FIELD_VALUE_AVI_ACTIVEFORMATINFO;

typedef enum
{
    NV_INFOFRAME_FIELD_VALUE_AVI_COLORFORMAT_RGB      = 0,
    NV_INFOFRAME_FIELD_VALUE_AVI_COLORFORMAT_YCbCr422,
    NV_INFOFRAME_FIELD_VALUE_AVI_COLORFORMAT_YCbCr444,
    NV_INFOFRAME_FIELD_VALUE_AVI_COLORFORMAT_FUTURE,
    NV_INFOFRAME_FIELD_VALUE_AVI_COLORFORMAT_AUTO     = 7
} NV_INFOFRAME_FIELD_VALUE_AVI_COLORFORMAT;

typedef enum
{
    NV_INFOFRAME_FIELD_VALUE_AVI_F17_FALSE = 0,
    NV_INFOFRAME_FIELD_VALUE_AVI_F17_TRUE,
    NV_INFOFRAME_FIELD_VALUE_AVI_F17_AUTO = 3
} NV_INFOFRAME_FIELD_VALUE_AVI_F17;

// byte2 related
typedef enum
{
    NV_INFOFRAME_FIELD_VALUE_AVI_ASPECTRATIOACTIVEPORTION_NO_AFD           = 0,
    NV_INFOFRAME_FIELD_VALUE_AVI_ASPECTRATIOACTIVEPORTION_RESERVE01,
    NV_INFOFRAME_FIELD_VALUE_AVI_ASPECTRATIOACTIVEPORTION_RESERVE02,
    NV_INFOFRAME_FIELD_VALUE_AVI_ASPECTRATIOACTIVEPORTION_RESERVE03,
    NV_INFOFRAME_FIELD_VALUE_AVI_ASPECTRATIOACTIVEPORTION_LETTERBOX_GT16x9,
    NV_INFOFRAME_FIELD_VALUE_AVI_ASPECTRATIOACTIVEPORTION_RESERVE05,
    NV_INFOFRAME_FIELD_VALUE_AVI_ASPECTRATIOACTIVEPORTION_RESERVE06,
    NV_INFOFRAME_FIELD_VALUE_AVI_ASPECTRATIOACTIVEPORTION_RESERVE07,
    NV_INFOFRAME_FIELD_VALUE_AVI_ASPECTRATIOACTIVEPORTION_EQUAL_CODEDFRAME = 8,
    NV_INFOFRAME_FIELD_VALUE_AVI_ASPECTRATIOACTIVEPORTION_CENTER_4x3,
    NV_INFOFRAME_FIELD_VALUE_AVI_ASPECTRATIOACTIVEPORTION_CENTER_16x9,
    NV_INFOFRAME_FIELD_VALUE_AVI_ASPECTRATIOACTIVEPORTION_CENTER_14x9,
    NV_INFOFRAME_FIELD_VALUE_AVI_ASPECTRATIOACTIVEPORTION_RESERVE12,
    NV_INFOFRAME_FIELD_VALUE_AVI_ASPECTRATIOACTIVEPORTION_4x3_ON_14x9,
    NV_INFOFRAME_FIELD_VALUE_AVI_ASPECTRATIOACTIVEPORTION_16x9_ON_14x9,
    NV_INFOFRAME_FIELD_VALUE_AVI_ASPECTRATIOACTIVEPORTION_16x9_ON_4x3,
    NV_INFOFRAME_FIELD_VALUE_AVI_ASPECTRATIOACTIVEPORTION_AUTO             = 31,
} NV_INFOFRAME_FIELD_VALUE_AVI_ASPECTRATIOACTIVEPORTION;

typedef enum
{
    NV_INFOFRAME_FIELD_VALUE_AVI_ASPECTRATIOCODEDFRAME_NO_DATA = 0,
    NV_INFOFRAME_FIELD_VALUE_AVI_ASPECTRATIOCODEDFRAME_4x3,
    NV_INFOFRAME_FIELD_VALUE_AVI_ASPECTRATIOCODEDFRAME_16x9,
    NV_INFOFRAME_FIELD_VALUE_AVI_ASPECTRATIOCODEDFRAME_FUTURE,
    NV_INFOFRAME_FIELD_VALUE_AVI_ASPECTRATIOCODEDFRAME_AUTO    = 7
} NV_INFOFRAME_FIELD_VALUE_AVI_ASPECTRATIOCODEDFRAME;

typedef enum
{
    NV_INFOFRAME_FIELD_VALUE_AVI_COLORIMETRY_NO_DATA                   = 0,
    NV_INFOFRAME_FIELD_VALUE_AVI_COLORIMETRY_SMPTE_170M,
    NV_INFOFRAME_FIELD_VALUE_AVI_COLORIMETRY_ITUR_BT709,
    NV_INFOFRAME_FIELD_VALUE_AVI_COLORIMETRY_USE_EXTENDED_COLORIMETRY,
    NV_INFOFRAME_FIELD_VALUE_AVI_COLORIMETRY_AUTO                      = 7
} NV_INFOFRAME_FIELD_VALUE_AVI_COLORIMETRY;

// byte 3 related
typedef enum
{
    NV_INFOFRAME_FIELD_VALUE_AVI_NONUNIFORMPICTURESCALING_NO_DATA    = 0,
    NV_INFOFRAME_FIELD_VALUE_AVI_NONUNIFORMPICTURESCALING_HORIZONTAL,
    NV_INFOFRAME_FIELD_VALUE_AVI_NONUNIFORMPICTURESCALING_VERTICAL,
    NV_INFOFRAME_FIELD_VALUE_AVI_NONUNIFORMPICTURESCALING_BOTH,
    NV_INFOFRAME_FIELD_VALUE_AVI_NONUNIFORMPICTURESCALING_AUTO       = 7
} NV_INFOFRAME_FIELD_VALUE_AVI_NONUNIFORMPICTURESCALING;

typedef enum
{
    NV_INFOFRAME_FIELD_VALUE_AVI_RGBQUANTIZATION_DEFAULT       = 0,
    NV_INFOFRAME_FIELD_VALUE_AVI_RGBQUANTIZATION_LIMITED_RANGE,
    NV_INFOFRAME_FIELD_VALUE_AVI_RGBQUANTIZATION_FULL_RANGE,
    NV_INFOFRAME_FIELD_VALUE_AVI_RGBQUANTIZATION_RESERVED,
    NV_INFOFRAME_FIELD_VALUE_AVI_RGBQUANTIZATION_AUTO          = 7
} NV_INFOFRAME_FIELD_VALUE_AVI_RGBQUANTIZATION;

typedef enum
{
    NV_INFOFRAME_FIELD_VALUE_AVI_EXTENDEDCOLORIMETRY_XVYCC601     = 0,
    NV_INFOFRAME_FIELD_VALUE_AVI_EXTENDEDCOLORIMETRY_XVYCC709,
    NV_INFOFRAME_FIELD_VALUE_AVI_EXTENDEDCOLORIMETRY_SYCC601,
    NV_INFOFRAME_FIELD_VALUE_AVI_EXTENDEDCOLORIMETRY_ADOBEYCC601,
    NV_INFOFRAME_FIELD_VALUE_AVI_EXTENDEDCOLORIMETRY_ADOBERGB,
    NV_INFOFRAME_FIELD_VALUE_AVI_EXTENDEDCOLORIMETRY_RESERVED05,
    NV_INFOFRAME_FIELD_VALUE_AVI_EXTENDEDCOLORIMETRY_RESERVED06,
    NV_INFOFRAME_FIELD_VALUE_AVI_EXTENDEDCOLORIMETRY_RESERVED07,
    NV_INFOFRAME_FIELD_VALUE_AVI_EXTENDEDCOLORIMETRY_AUTO         = 15
} NV_INFOFRAME_FIELD_VALUE_AVI_EXTENDEDCOLORIMETRY;

typedef enum
{
    NV_INFOFRAME_FIELD_VALUE_AVI_ITC_VIDEO_CONTENT = 0,
    NV_INFOFRAME_FIELD_VALUE_AVI_ITC_ITCONTENT,
    NV_INFOFRAME_FIELD_VALUE_AVI_ITC_AUTO          = 3
} NV_INFOFRAME_FIELD_VALUE_AVI_ITC;

// byte 4 related
typedef enum
{
    NV_INFOFRAME_FIELD_VALUE_AVI_PIXELREPETITION_NONE = 0,
    NV_INFOFRAME_FIELD_VALUE_AVI_PIXELREPETITION_X02,
    NV_INFOFRAME_FIELD_VALUE_AVI_PIXELREPETITION_X03,
    NV_INFOFRAME_FIELD_VALUE_AVI_PIXELREPETITION_X04,
    NV_INFOFRAME_FIELD_VALUE_AVI_PIXELREPETITION_X05,
    NV_INFOFRAME_FIELD_VALUE_AVI_PIXELREPETITION_X06,
    NV_INFOFRAME_FIELD_VALUE_AVI_PIXELREPETITION_X07,
    NV_INFOFRAME_FIELD_VALUE_AVI_PIXELREPETITION_X08,
    NV_INFOFRAME_FIELD_VALUE_AVI_PIXELREPETITION_X09,
    NV_INFOFRAME_FIELD_VALUE_AVI_PIXELREPETITION_X10,
    NV_INFOFRAME_FIELD_VALUE_AVI_PIXELREPETITION_RESERVED10,
    NV_INFOFRAME_FIELD_VALUE_AVI_PIXELREPETITION_RESERVED11,
    NV_INFOFRAME_FIELD_VALUE_AVI_PIXELREPETITION_RESERVED12,
    NV_INFOFRAME_FIELD_VALUE_AVI_PIXELREPETITION_RESERVED13,
    NV_INFOFRAME_FIELD_VALUE_AVI_PIXELREPETITION_RESERVED14,
    NV_INFOFRAME_FIELD_VALUE_AVI_PIXELREPETITION_RESERVED15,
    NV_INFOFRAME_FIELD_VALUE_AVI_PIXELREPETITION_AUTO         = 31
} NV_INFOFRAME_FIELD_VALUE_AVI_PIXELREPETITION;

typedef enum
{
    NV_INFOFRAME_FIELD_VALUE_AVI_CONTENTTYPE_GRAPHICS = 0,
    NV_INFOFRAME_FIELD_VALUE_AVI_CONTENTTYPE_PHOTO,
    NV_INFOFRAME_FIELD_VALUE_AVI_CONTENTTYPE_CINEMA,
    NV_INFOFRAME_FIELD_VALUE_AVI_CONTENTTYPE_GAME,
    NV_INFOFRAME_FIELD_VALUE_AVI_CONTENTTYPE_AUTO     = 7
} NV_INFOFRAME_FIELD_VALUE_AVI_CONTENTTYPE;

typedef enum
{
    NV_INFOFRAME_FIELD_VALUE_AVI_YCCQUANTIZATION_LIMITED_RANGE = 0,
    NV_INFOFRAME_FIELD_VALUE_AVI_YCCQUANTIZATION_FULL_RANGE,
    NV_INFOFRAME_FIELD_VALUE_AVI_YCCQUANTIZATION_RESERVED02,
    NV_INFOFRAME_FIELD_VALUE_AVI_YCCQUANTIZATION_RESERVED03,
    NV_INFOFRAME_FIELD_VALUE_AVI_YCCQUANTIZATION_AUTO          = 7
} NV_INFOFRAME_FIELD_VALUE_AVI_YCCQUANTIZATION;


// Adding an Auto bit to each field
typedef struct
{
    NvU32 vic                     : 8;
    NvU32 pixelRepeat             : 5;
    NvU32 colorSpace              : 3;
    NvU32 colorimetry             : 3;
    NvU32 extendedColorimetry     : 4;
    NvU32 rgbQuantizationRange    : 3;
    NvU32 yccQuantizationRange    : 3;
    NvU32 itContent               : 2;
    NvU32 contentTypes            : 3;
    NvU32 scanInfo                : 3;
    NvU32 activeFormatInfoPresent : 2;
    NvU32 activeFormatAspectRatio : 5;
    NvU32 picAspectRatio          : 3;
    NvU32 nonuniformScaling       : 3;
    NvU32 barInfo                 : 3;
    NvU32 top_bar                 : 17;
    NvU32 bottom_bar              : 17;
    NvU32 left_bar                : 17;
    NvU32 right_bar               : 17;
    NvU32 Future17                : 2;
    NvU32 Future47                : 2;
} NV_INFOFRAME_VIDEO;

// byte 1 related
typedef enum
{
    NV_INFOFRAME_FIELD_VALUE_AUDIO_CHANNELCOUNT_IN_HEADER = 0,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_CHANNELCOUNT_2,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_CHANNELCOUNT_3,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_CHANNELCOUNT_4,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_CHANNELCOUNT_5,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_CHANNELCOUNT_6,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_CHANNELCOUNT_7,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_CHANNELCOUNT_8,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_CHANNELCOUNT_AUTO      = 15
} NV_INFOFRAME_FIELD_VALUE_AUDIO_CHANNELCOUNT;

typedef enum
{
    NV_INFOFRAME_FIELD_VALUE_AUDIO_CODINGTYPE_IN_HEADER                  = 0,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_CODINGTYPE_PCM,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_CODINGTYPE_AC3,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_CODINGTYPE_MPEG1,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_CODINGTYPE_MP3,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_CODINGTYPE_MPEG2,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_CODINGTYPE_AACLC,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_CODINGTYPE_DTS,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_CODINGTYPE_ATRAC,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_CODINGTYPE_DSD,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_CODINGTYPE_EAC3,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_CODINGTYPE_DTSHD,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_CODINGTYPE_MLP,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_CODINGTYPE_DST,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_CODINGTYPE_WMAPRO,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_CODINGTYPE_USE_CODING_EXTENSION_TYPE,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_CODINGTYPE_AUTO                      = 31
} NV_INFOFRAME_FIELD_VALUE_AUDIO_CODINGTYPE;

// byte 2 related
typedef enum
{
    NV_INFOFRAME_FIELD_VALUE_AUDIO_SAMPLESIZE_IN_HEADER = 0,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_SAMPLESIZE_16BITS,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_SAMPLESIZE_20BITS,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_SAMPLESIZE_24BITS,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_SAMPLESIZE_AUTO      = 7
} NV_INFOFRAME_FIELD_VALUE_AUDIO_SAMPLESIZE;

typedef enum
{
    NV_INFOFRAME_FIELD_VALUE_AUDIO_SAMPLEFREQUENCY_IN_HEADER = 0,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_SAMPLEFREQUENCY_32000HZ,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_SAMPLEFREQUENCY_44100HZ,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_SAMPLEFREQUENCY_48000HZ,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_SAMPLEFREQUENCY_88200KHZ,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_SAMPLEFREQUENCY_96000KHZ,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_SAMPLEFREQUENCY_176400KHZ,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_SAMPLEFREQUENCY_192000KHZ,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_SAMPLEFREQUENCY_AUTO      = 15
} NV_INFOFRAME_FIELD_VALUE_AUDIO_SAMPLEFREQUENCY;

// byte 3 related
typedef enum
{
    NV_INFOFRAME_FIELD_VALUE_AUDIO_CODINGEXTENSIONTYPE_USE_CODING_TYPE = 0,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_CODINGEXTENSIONTYPE_HEAAC,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_CODINGEXTENSIONTYPE_HEAACV2,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_CODINGEXTENSIONTYPE_MPEGSURROUND,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_CODINGEXTENSIONTYPE_RESERVE04,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_CODINGEXTENSIONTYPE_RESERVE05,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_CODINGEXTENSIONTYPE_RESERVE06,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_CODINGEXTENSIONTYPE_RESERVE07,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_CODINGEXTENSIONTYPE_RESERVE08,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_CODINGEXTENSIONTYPE_RESERVE09,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_CODINGEXTENSIONTYPE_RESERVE10,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_CODINGEXTENSIONTYPE_RESERVE11,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_CODINGEXTENSIONTYPE_RESERVE12,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_CODINGEXTENSIONTYPE_RESERVE13,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_CODINGEXTENSIONTYPE_RESERVE14,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_CODINGEXTENSIONTYPE_RESERVE15,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_CODINGEXTENSIONTYPE_RESERVE16,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_CODINGEXTENSIONTYPE_RESERVE17,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_CODINGEXTENSIONTYPE_RESERVE18,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_CODINGEXTENSIONTYPE_RESERVE19,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_CODINGEXTENSIONTYPE_RESERVE20,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_CODINGEXTENSIONTYPE_RESERVE21,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_CODINGEXTENSIONTYPE_RESERVE22,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_CODINGEXTENSIONTYPE_RESERVE23,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_CODINGEXTENSIONTYPE_RESERVE24,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_CODINGEXTENSIONTYPE_RESERVE25,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_CODINGEXTENSIONTYPE_RESERVE26,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_CODINGEXTENSIONTYPE_RESERVE27,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_CODINGEXTENSIONTYPE_RESERVE28,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_CODINGEXTENSIONTYPE_RESERVE29,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_CODINGEXTENSIONTYPE_RESERVE30,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_CODINGEXTENSIONTYPE_RESERVE31,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_CODINGEXTENSIONTYPE_AUTO           = 63
} NV_INFOFRAME_FIELD_VALUE_AUDIO_CODINGEXTENSIONTYPE;

// byte 4 related
typedef enum
{
    NV_INFOFRAME_FIELD_VALUE_AUDIO_CHANNELALLOCATION_X_X_X_X_X_X_FR_FL           =0,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_CHANNELALLOCATION_X_X_X_X_X_LFE_FR_FL,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_CHANNELALLOCATION_X_X_X_X_FC_X_FR_FL,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_CHANNELALLOCATION_X_X_X_X_FC_LFE_FR_FL,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_CHANNELALLOCATION_X_X_X_RC_X_X_FR_FL,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_CHANNELALLOCATION_X_X_X_RC_X_LFE_FR_FL,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_CHANNELALLOCATION_X_X_X_RC_FC_X_FR_FL,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_CHANNELALLOCATION_X_X_X_RC_FC_LFE_FR_FL,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_CHANNELALLOCATION_X_X_RR_RL_X_X_FR_FL,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_CHANNELALLOCATION_X_X_RR_RL_X_LFE_FR_FL,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_CHANNELALLOCATION_X_X_RR_RL_FC_X_FR_FL,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_CHANNELALLOCATION_X_X_RR_RL_FC_LFE_FR_FL,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_CHANNELALLOCATION_X_RC_RR_RL_X_X_FR_FL,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_CHANNELALLOCATION_X_RC_RR_RL_X_LFE_FR_FL,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_CHANNELALLOCATION_X_RC_RR_RL_FC_X_FR_FL,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_CHANNELALLOCATION_X_RC_RR_RL_FC_LFE_FR_FL,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_CHANNELALLOCATION_RRC_RLC_RR_RL_X_X_FR_FL,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_CHANNELALLOCATION_RRC_RLC_RR_RL_X_LFE_FR_FL,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_CHANNELALLOCATION_RRC_RLC_RR_RL_FC_X_FR_FL,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_CHANNELALLOCATION_RRC_RLC_RR_RL_FC_LFE_FR_FL,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_CHANNELALLOCATION_FRC_FLC_X_X_X_X_FR_FL,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_CHANNELALLOCATION_FRC_FLC_X_X_X_LFE_FR_FL,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_CHANNELALLOCATION_FRC_FLC_X_X_FC_X_FR_FL,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_CHANNELALLOCATION_FRC_FLC_X_X_FC_LFE_FR_FL,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_CHANNELALLOCATION_FRC_FLC_X_RC_X_X_FR_FL,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_CHANNELALLOCATION_FRC_FLC_X_RC_X_LFE_FR_FL,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_CHANNELALLOCATION_FRC_FLC_X_RC_FC_X_FR_FL,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_CHANNELALLOCATION_FRC_FLC_X_RC_FC_LFE_FR_FL,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_CHANNELALLOCATION_FRC_FLC_RR_RL_X_X_FR_FL,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_CHANNELALLOCATION_FRC_FLC_RR_RL_X_LFE_FR_FL,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_CHANNELALLOCATION_FRC_FLC_RR_RL_FC_X_FR_FL,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_CHANNELALLOCATION_FRC_FLC_RR_RL_FC_LFE_FR_FL,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_CHANNELALLOCATION_X_FCH_RR_RL_FC_X_FR_FL,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_CHANNELALLOCATION_X_FCH_RR_RL_FC_LFE_FR_FL,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_CHANNELALLOCATION_TC_X_RR_RL_FC_X_FR_FL,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_CHANNELALLOCATION_TC_X_RR_RL_FC_LFE_FR_FL,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_CHANNELALLOCATION_FRH_FLH_RR_RL_X_X_FR_FL,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_CHANNELALLOCATION_FRH_FLH_RR_RL_X_LFE_FR_FL,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_CHANNELALLOCATION_FRW_FLW_RR_RL_X_X_FR_FL,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_CHANNELALLOCATION_FRW_FLW_RR_RL_X_LFE_FR_FL,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_CHANNELALLOCATION_TC_RC_RR_RL_FC_X_FR_FL,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_CHANNELALLOCATION_TC_RC_RR_RL_FC_LFE_FR_FL,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_CHANNELALLOCATION_FCH_RC_RR_RL_FC_X_FR_FL,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_CHANNELALLOCATION_FCH_RC_RR_RL_FC_LFE_FR_FL,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_CHANNELALLOCATION_TC_FCH_RR_RL_FC_X_FR_FL,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_CHANNELALLOCATION_TC_FCH_RR_RL_FC_LFE_FR_FL,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_CHANNELALLOCATION_FRH_FLH_RR_RL_FC_X_FR_FL,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_CHANNELALLOCATION_FRH_FLH_RR_RL_FC_LFE_FR_FL,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_CHANNELALLOCATION_FRW_FLW_RR_RL_FC_X_FR_FL,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_CHANNELALLOCATION_FRW_FLW_RR_RL_FC_LFE_FR_FL  = 0X31,
    // all other values should default to auto
    NV_INFOFRAME_FIELD_VALUE_AUDIO_CHANNELALLOCATION_AUTO                        = 0x1FF
} NV_INFOFRAME_FIELD_VALUE_AUDIO_CHANNELALLOCATION;

// byte 5 related
typedef enum
{
    NV_INFOFRAME_FIELD_VALUE_AUDIO_LFEPLAYBACKLEVEL_NO_DATA    = 0,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_LFEPLAYBACKLEVEL_0DB,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_LFEPLAYBACKLEVEL_PLUS10DB,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_LFEPLAYBACKLEVEL_RESERVED03,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_LFEPLAYBACKLEVEL_AUTO       = 7
} NV_INFOFRAME_FIELD_VALUE_AUDIO_LFEPLAYBACKLEVEL;

typedef enum
{
    NV_INFOFRAME_FIELD_VALUE_AUDIO_LEVELSHIFTVALUES_0DB  = 0,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_LEVELSHIFTVALUES_1DB,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_LEVELSHIFTVALUES_2DB,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_LEVELSHIFTVALUES_3DB,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_LEVELSHIFTVALUES_4DB,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_LEVELSHIFTVALUES_5DB,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_LEVELSHIFTVALUES_6DB,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_LEVELSHIFTVALUES_7DB,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_LEVELSHIFTVALUES_8DB,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_LEVELSHIFTVALUES_9DB,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_LEVELSHIFTVALUES_10DB,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_LEVELSHIFTVALUES_11DB,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_LEVELSHIFTVALUES_12DB,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_LEVELSHIFTVALUES_13DB,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_LEVELSHIFTVALUES_14DB,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_LEVELSHIFTVALUES_15DB,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_LEVELSHIFTVALUES_AUTO = 31
} NV_INFOFRAME_FIELD_VALUE_AUDIO_LEVELSHIFTVALUES;

typedef enum
{
    NV_INFOFRAME_FIELD_VALUE_AUDIO_DOWNMIX_PERMITTED  = 0,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_DOWNMIX_PROHIBITED,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_DOWNMIX_AUTO       = 3
} NV_INFOFRAME_FIELD_VALUE_AUDIO_DOWNMIX;

typedef struct
{
    NvU32 codingType          : 5;
    NvU32 codingExtensionType : 6;
    NvU32 sampleSize          : 3;
    NvU32 sampleRate          : 4;
    NvU32 channelCount        : 4;
    NvU32 speakerPlacement    : 9;
    NvU32 downmixInhibit      : 2;
    NvU32 lfePlaybackLevel    : 3;
    NvU32 levelShift          : 5;
    NvU32 Future12            : 2;
    NvU32 Future2x            : 4;
    NvU32 Future3x            : 4;
    NvU32 Future52            : 2;
    NvU32 Future6             : 9;
    NvU32 Future7             : 9;
    NvU32 Future8             : 9;
    NvU32 Future9             : 9;
    NvU32 Future10            : 9;
} NV_INFOFRAME_AUDIO;

typedef struct
{
    NvU32 version; // version of this structure
    NvU16 size;    // size of this structure
    NvU8  cmd;     // The actions to perform from NV_INFOFRAME_CMD
    NvU8  type;    // type of infoframe

    union
    {
        NV_INFOFRAME_PROPERTY     property;  // This is nvidia specific and corresponds to the property cmds and associated infoframe
        NV_INFOFRAME_AUDIO        audio;
        NV_INFOFRAME_VIDEO        video;
    } infoframe;
} NV_INFOFRAME_DATA;

#define NV_INFOFRAME_DATA_VER   MAKE_NVAPI_VERSION(NV_INFOFRAME_DATA,1)

NVAPI_INTERFACE NvAPI_Disp_InfoFrameControl(NvU32 displayId, NV_INFOFRAME_DATA *pInfoframeData);


///////////////////////////////////////////////////////////////////////////////
// FUNCTION NAME:   NvAPI_Disp_ColorControl
//
// DESCRIPTION:     Controls the Color values
//
//  SUPPORTED OS: Windows Vista and higher
//
// PARAMETERS:      displayId(IN)           - Monitor Identifier
//                  pColorData(IN/OUT)      - Contains data corresponding to color information
//
// RETURN STATUS:
//                  NVAPI_OK - completed request
//                  NVAPI_ERROR - miscellaneous error occurred
//                  NVAPI_INVALID_ARGUMENT: Invalid input parameter.
//
///////////////////////////////////////////////////////////////////////////////

typedef enum
{
    NV_COLOR_CMD_GET                 = 1,
    NV_COLOR_CMD_SET,
    NV_COLOR_CMD_IS_SUPPORTED_COLOR,
    NV_COLOR_CMD_GET_DEFAULT
} NV_COLOR_CMD;

// See Table 14 of CEA-861E.  Not all this is supported by GPU.
typedef enum
{
    NV_COLOR_FORMAT_RGB             = 0,
    NV_COLOR_FORMAT_YUV422,
    NV_COLOR_FORMAT_YUV444,
    NV_COLOR_FORMAT_DEFAULT         = 0xFE,
    NV_COLOR_FORMAT_AUTO            = 0xFF
} NV_COLOR_FORMAT;

typedef enum
{
    NV_COLOR_COLORIMETRY_RGB             = 0,
    NV_COLOR_COLORIMETRY_YCC601,
    NV_COLOR_COLORIMETRY_YCC709,
    NV_COLOR_COLORIMETRY_XVYCC601,
    NV_COLOR_COLORIMETRY_XVYCC709,
    NV_COLOR_COLORIMETRY_SYCC601,
    NV_COLOR_COLORIMETRY_ADOBEYCC601,
    NV_COLOR_COLORIMETRY_ADOBERGB,
    NV_COLOR_COLORIMETRY_DEFAULT         = 0xFE,
    NV_COLOR_COLORIMETRY_AUTO            = 0xFF
} NV_COLOR_COLORIMETRY;

typedef struct
{
    NvU32 version; // version of this structure
    NvU16 size;    // size of this structure
    NvU8  cmd;
    struct
    {
        NvU8  colorFormat;
        NvU8  colorimetry;
    } data;
} NV_COLOR_DATA;

NVAPI_INTERFACE NvAPI_Disp_ColorControl(NvU32 displayId, NV_COLOR_DATA *pColorData);
#define NV_COLOR_DATA_VER   MAKE_NVAPI_VERSION(NV_COLOR_DATA,1)

typedef struct
{
    NvU32 version;                    // Structure version
    NvU32 maxNumAFRGroups;            // [OUT] The maximum possible value of numAFRGroups
    NvU32 numAFRGroups;               // [OUT] The number of AFR groups enabled in the system
    NvU32 currentAFRIndex;            // [OUT] The AFR group index for the frame currently being rendered
    NvU32 nextFrameAFRIndex;          // [OUT] What the AFR group index will be for the next frame (i.e. after calling Present)
    NvU32 previousFrameAFRIndex;      // [OUT] The AFR group index that was used for the previous frame (~0 if more than one frame has not been rendered yet)
    NvU32 bIsCurAFRGroupNew;          // [OUT] boolean: Is this frame the first time running on the current AFR group
} NV_GET_CURRENT_SLI_STATE;
#define NV_GET_CURRENT_SLI_STATE_VER  MAKE_NVAPI_VERSION(NV_GET_CURRENT_SLI_STATE,1)


#if defined(_D3D9_H_) || defined(__d3d10_h__) || defined(__d3d11_h__)
///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME:   NvAPI_D3D_GetCurrentSLIState
//
// DESCRIPTION:     Returns the current SLI state for this device.  The struct
//                  contains the number of AFR groups, the current AFR group index
//                  and what the AFR group index will be for the next frame.
//                  pDevice can be either a IDirect3DDevice9 or ID3D10Device ptr.
//
//  SUPPORTED OS: Windows XP and higher
//
// RETURN STATUS:
//                  NVAPI_OK - completed request
//                  NVAPI_ERROR - error occurred
//
///////////////////////////////////////////////////////////////////////////////

NVAPI_INTERFACE NvAPI_D3D_GetCurrentSLIState(IUnknown *pDevice, NV_GET_CURRENT_SLI_STATE *pSliState);

#endif //if defined(_D3D9_H_) || defined(__d3d10_h__) || defined(__d3d11_h__)

#if defined(_D3D9_H_)
///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME:   NvAPI_D3D9_RegisterResource
//
// DESCRIPTION:     To bind a resource (surface/texture) so that it can be retrieved
//                  internally by nvapi.
//
//  SUPPORTED OS: Windows XP and higher
// INPUT:           pResource   (IN)    surface/texture
//
// RETURN STATUS:
//                  NVAPI_OK - completed request
//                  NVAPI_ERROR - error occurred
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_D3D9_RegisterResource(IDirect3DResource9* pResource);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME:   NvAPI_D3D9_UnregisterResource
//
// DESCRIPTION:     To unbind a resource (surface/texture) after usage.
//
//  SUPPORTED OS: Windows XP and higher
// INPUT:           pResource   (IN)    surface/texture
//
// RETURN STATUS:
//                  NVAPI_OK - completed request
//                  NVAPI_ERROR - error occurred
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_D3D9_UnregisterResource(IDirect3DResource9* pResource);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_D3D9_AliasSurfaceAsTexture
//
//   DESCRIPTION: Create a texture that is an alias of a surface registered with NvAPI.  The
//                new texture can be bound with IDirect3DDevice9::SetTexture().  Note that the texture must
//                be unbound before drawing to the surface again.
//                Unless the USE_SUPER flag is passed, MSAA surfaces will be resolved before
//                being used as a texture.  MSAA depth buffers are resolved with a point filter,
//                and non-depth MSAA surfaces are resolved with a linear filter.
//
//  SUPPORTED OS: Windows XP and higher
//
//         INPUT:  pDev      (IN)   The D3D device that owns the objects
//                 pSurface  (IN)   Pointer to a surface that has been registered with NvAPI
//                                  to provide a texture alias to
//                 ppTexture (OUT)  Fill with the texture created
//                 dwFlag    (IN)   NVAPI_ALIAS_SURFACE_FLAG to describe how to handle the texture
//
// RETURN STATUS:
//                  NVAPI_OK - completed request
//                  NVAPI_INVALID_POINTER - A null pointer was passed as an argument
//                  NVAPI_INVALID_ARGUMENT - One of the arguments was invalid, probably dwFlag.
//                  NVAPI_UNREGISTERED_RESOURCE - pSurface has not been registered with NvAPI
//                  NVAPI_ERROR - error occurred
//
///////////////////////////////////////////////////////////////////////////////
typedef enum {
    NVAPI_ALIAS_SURFACE_FLAG_NONE                     = 0x00000000,
    NVAPI_ALIAS_SURFACE_FLAG_USE_SUPER                = 0x00000001,  // Use the surface's msaa buffer directly as a texture, rather than resolving. (This is much slower, but potentially has higher quality.)
    NVAPI_ALIAS_SURFACE_FLAG_MASK                     = 0x00000001
} NVAPI_ALIAS_SURFACE_FLAG;

NVAPI_INTERFACE NvAPI_D3D9_AliasSurfaceAsTexture(IDirect3DDevice9* pDev,
                                                 IDirect3DSurface9* pSurface,
                                                 IDirect3DTexture9 **ppTexture,
                                                 DWORD dwFlag);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME:   NvAPI_D3D9_StretchRectEx
//
// DESCRIPTION:     Copy the contents of the source resource to the destination
//                  resource.  This function can convert
//                  between a wider range of surfaces than
//                  IDirect3DDevice9::StretchRect.  For example, it can copy
//                  from a depth/stencil surface to a texture.
//                  Note that the source and destination resources *must* be registered
//                  with NvAPI before being used with NvAPI_D3D9_StretchRectEx.
//
//  SUPPORTED OS: Windows XP and higher
//
// INPUT:           pDevice        (IN)     The D3D device that owns the objects.
//                  pSourceResource(IN)     Pointer to the source resource.
//                  pSrcRect       (IN)     Defines the rectangle on the source to copy from.  If null, copy from the entire resource.
//                  pDestResource  (IN)     Pointer to the destination resource.
//                  pDstRect       (IN)     Defines the rectangle on the destination to copy to.  If null, copy to the entire resource.
//                  Filter         (IN)     Choose a filtering method: D3DTEXF_NONE, D3DTEXF_POINT, D3DTEXF_LINEAR.
//
// RETURN STATUS:
//                  NVAPI_OK - completed request
//                  NVAPI_INVALID_POINTER - An invalid pointer was passed as an argument (probably NULL)
//                  NVAPI_INVALID_ARGUMENT - One of the arguments was invalid
//                  NVAPI_UNREGISTERED_RESOURCE - a resource was passed in without being registered
//                  NVAPI_ERROR - error occurred
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_D3D9_StretchRectEx(IDirect3DDevice9 * pDevice,
                                         IDirect3DResource9 * pSourceResource,
                                         CONST RECT * pSourceRect,
                                         IDirect3DResource9 * pDestResource,
                                         CONST RECT * pDestRect,
                                         D3DTEXTUREFILTERTYPE Filter);


#endif //if defined(_D3D9_H_)

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_GPU_GetAllOutputs
//
//   DESCRIPTION: Returns set of all GPU-output identifiers as a bitmask.
//
//  SUPPORTED OS: Windows XP and higher
//
// RETURN STATUS: NVAPI_INVALID_ARGUMENT: hPhysicalGpu or pOutputsMask is NULL
//                NVAPI_OK: *pOutputsMask contains a set of GPU-output identifiers
//                NVAPI_NVIDIA_DEVICE_NOT_FOUND: no NVIDIA GPU driving a display was found
//                NVAPI_EXPECTED_PHYSICAL_GPU_HANDLE: hPhysicalGpu was not a physical GPU handle
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_GPU_GetAllOutputs(NvPhysicalGpuHandle hPhysicalGpu,NvU32 *pOutputsMask);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_GPU_GetConnectedOutputs
//
//   DESCRIPTION: Same as NvAPI_GPU_GetAllOutputs but returns only the set of GPU-output
//                identifiers that are connected to display devices.
//
//  SUPPORTED OS: Windows XP and higher
//
// RETURN STATUS: NVAPI_INVALID_ARGUMENT: hPhysicalGpu or pOutputsMask is NULL
//                NVAPI_OK: *pOutputsMask contains a set of GPU-output identifiers
//                NVAPI_NVIDIA_DEVICE_NOT_FOUND: no NVIDIA GPU driving a display was found
//                NVAPI_EXPECTED_PHYSICAL_GPU_HANDLE: hPhysicalGpu was not a physical GPU handle
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_GPU_GetConnectedOutputs(NvPhysicalGpuHandle hPhysicalGpu, NvU32 *pOutputsMask);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_GPU_GetConnectedSLIOutputs
//
//   DESCRIPTION: Same as NvAPI_GPU_GetConnectedOutputs but returns only the set of GPU-output
//                identifiers that can be selected in an SLI configuration.
//                NOTE: This function matches NvAPI_GPU_GetConnectedOutputs
//                 - On systems which are not SLI capable.
//                 - If the queried GPU is not part of a valid SLI group.
//
//  SUPPORTED OS: Windows XP and higher
//
// RETURN STATUS: NVAPI_INVALID_ARGUMENT: hPhysicalGpu or pOutputsMask is NULL
//                NVAPI_OK: *pOutputsMask contains a set of GPU-output identifiers
//                NVAPI_NVIDIA_DEVICE_NOT_FOUND: no NVIDIA GPU driving a display was found
//                NVAPI_EXPECTED_PHYSICAL_GPU_HANDLE: hPhysicalGpu was not a physical GPU handle
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_GPU_GetConnectedSLIOutputs(NvPhysicalGpuHandle hPhysicalGpu, NvU32 *pOutputsMask);


///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_GPU_GetConnectedOutputsWithLidState
//
//   DESCRIPTION: Similar to NvAPI_GPU_GetConnectedOutputs this API returns the connected display identifiers that are connected
//                as a output mask but unlike NvAPI_GPU_GetConnectedOutputs this API "always" reflects the Lid State in the output mask.
//                Thus if you expect the LID close state to be available in the connection mask use this API.
//                If LID is closed then this API will remove the LID panel from the connected display identifiers.
//                If LID is open then this API will reflect the LID panel in the connected display identifiers.
//                Note:This API should be used on laptop systems and on systems where LID state is required in the connection output mask.
//                     On desktop systems the returned identifiers will match NvAPI_GPU_GetConnectedOutputs.
//
//  SUPPORTED OS: Windows XP and higher
//
// RETURN STATUS: NVAPI_INVALID_ARGUMENT: hPhysicalGpu or pOutputsMask is NULL
//                NVAPI_OK: *pOutputsMask contains a set of GPU-output identifiers
//                NVAPI_NVIDIA_DEVICE_NOT_FOUND: no NVIDIA GPU driving a display was found
//                NVAPI_EXPECTED_PHYSICAL_GPU_HANDLE: hPhysicalGpu was not a physical GPU handle
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_GPU_GetConnectedOutputsWithLidState(NvPhysicalGpuHandle hPhysicalGpu, NvU32 *pOutputsMask);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_GPU_GetConnectedSLIOutputsWithLidState
//
//   DESCRIPTION: Same as NvAPI_GPU_GetConnectedOutputsWithLidState but returns only the set of GPU-output
//                identifiers that can be selected in an SLI configuration. With SLI disabled
//                this function matches NvAPI_GPU_GetConnectedOutputsWithLidState
//
//  SUPPORTED OS: Windows XP and higher
//
// RETURN STATUS: NVAPI_INVALID_ARGUMENT: hPhysicalGpu or pOutputsMask is NULL
//                NVAPI_OK: *pOutputsMask contains a set of GPU-output identifiers
//                NVAPI_NVIDIA_DEVICE_NOT_FOUND: no NVIDIA GPU driving a display was found
//                NVAPI_EXPECTED_PHYSICAL_GPU_HANDLE: hPhysicalGpu was not a physical GPU handle
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_GPU_GetConnectedSLIOutputsWithLidState(NvPhysicalGpuHandle hPhysicalGpu, NvU32 *pOutputsMask);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_GPU_GetSystemType
//
//   DESCRIPTION: Returns information to identify if the GPU type is for a laptop system or a desktop system.
//
//  SUPPORTED OS: Windows XP and higher
//
// RETURN STATUS: NVAPI_INVALID_ARGUMENT: hPhysicalGpu or pOutputsMask is NULL
//                NVAPI_OK: *pSystemType contains the GPU system type
//                NVAPI_NVIDIA_DEVICE_NOT_FOUND: no NVIDIA GPU driving a display was found
//                NVAPI_EXPECTED_PHYSICAL_GPU_HANDLE: hPhysicalGpu was not a physical GPU handle
//
///////////////////////////////////////////////////////////////////////////////
typedef enum
{
    NV_SYSTEM_TYPE_UNKNOWN = 0,
    NV_SYSTEM_TYPE_LAPTOP  = 1,
    NV_SYSTEM_TYPE_DESKTOP = 2,

} NV_SYSTEM_TYPE;

NVAPI_INTERFACE NvAPI_GPU_GetSystemType(NvPhysicalGpuHandle hPhysicalGpu, NV_SYSTEM_TYPE *pSystemType);


///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_GPU_GetActiveOutputs
//
//   DESCRIPTION: Same as NvAPI_GPU_GetAllOutputs but returns only the set of GPU-output
//                identifiers that are actively driving display devices.
//
//  SUPPORTED OS: Windows XP and higher
//
// RETURN STATUS: NVAPI_INVALID_ARGUMENT: hPhysicalGpu or pOutputsMask is NULL
//                NVAPI_OK: *pOutputsMask contains a set of GPU-output identifiers
//                NVAPI_NVIDIA_DEVICE_NOT_FOUND: no NVIDIA GPU driving a display was found
//                NVAPI_EXPECTED_PHYSICAL_GPU_HANDLE: hPhysicalGpu was not a physical GPU handle
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_GPU_GetActiveOutputs(NvPhysicalGpuHandle hPhysicalGpu, NvU32 *pOutputsMask);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_GPU_GetEDID
//
//   DESCRIPTION: Returns the EDID data for the specified GPU handle and connection bit mask.
//                displayOutputId should have exactly 1 bit set to indicate a single display.
//
//  SUPPORTED OS: Windows XP and higher
//
// RETURN STATUS: NVAPI_INVALID_ARGUMENT: pEDID is NULL; displayOutputId has 0 or > 1 bits set.
//                NVAPI_OK: *pEDID contains valid data.
//                NVAPI_NVIDIA_DEVICE_NOT_FOUND: no NVIDIA GPU driving a display was found.
//                NVAPI_EXPECTED_PHYSICAL_GPU_HANDLE: hPhysicalGpu was not a physical GPU handle.
//                NVAPI_DATA_NOT_FOUND: requested display does not contain an EDID
//
///////////////////////////////////////////////////////////////////////////////
#define NV_EDID_V1_DATA_SIZE   256
#define NV_EDID_DATA_SIZE      NV_EDID_V1_DATA_SIZE

typedef struct
{
    NvU32   version;        //structure version
    NvU8    EDID_Data[NV_EDID_DATA_SIZE];
} NV_EDID_V1;

typedef struct
{
    NvU32   version;        //structure version
    NvU8    EDID_Data[NV_EDID_DATA_SIZE];
    NvU32   sizeofEDID;
} NV_EDID_V2;

typedef struct
{
    NvU32   version;        //structure version
    NvU8    EDID_Data[NV_EDID_DATA_SIZE];
    NvU32   sizeofEDID;
    NvU32   edidId;     // edidId is an ID which always returned in a monotonically increasing counter.
                       // Across a split-edid read we need to verify that all calls returned the same edidId.
                       // This counter is incremented if we get the updated EDID.
    NvU32   offset;    // which 256byte page of the EDID we want to read. Start at 0.
                       // If the read succeeds with edidSize > NV_EDID_DATA_SIZE
                       // call back again with offset+256 until we have read the entire buffer
} NV_EDID_V3;

typedef NV_EDID_V3    NV_EDID;

#define NV_EDID_VER1    MAKE_NVAPI_VERSION(NV_EDID_V1,1)
#define NV_EDID_VER2    MAKE_NVAPI_VERSION(NV_EDID_V2,2)
#define NV_EDID_VER3    MAKE_NVAPI_VERSION(NV_EDID_V3,3)
#define NV_EDID_VER   NV_EDID_VER3

NVAPI_INTERFACE NvAPI_GPU_GetEDID(NvPhysicalGpuHandle hPhysicalGpu, NvU32 displayOutputId, NV_EDID *pEDID);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_GPU_SetEDID
//
//   DESCRIPTION: Sets the EDID data for the specified GPU handle and connection bit mask.
//                displayOutputId should have exactly 1 bit set to indicate a single display.
//                Note:The EDID will be cached across boot session and will be enumerated to the OS in this call.
//                     To remove the EDID set the sizeofEDID to zero.
//                     OS and NVAPI connection status APIs will reflect the newly set or removed EDID dynamically.
//
//  SUPPORTED OS: Windows XP and higher
//
// RETURN STATUS: NVAPI_INVALID_ARGUMENT: pEDID is NULL; displayOutputId has 0 or > 1 bits set
//                NVAPI_OK: *pEDID data was applied to the requested displayOutputId.
//                NVAPI_NVIDIA_DEVICE_NOT_FOUND: no NVIDIA GPU driving a display was found.
//                NVAPI_EXPECTED_PHYSICAL_GPU_HANDLE: hPhysicalGpu was not a physical GPU handle.
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_GPU_SetEDID(NvPhysicalGpuHandle hPhysicalGpu, NvU32 displayOutputId, NV_EDID *pEDID);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_GPU_GetOutputType
//
//   DESCRIPTION: Give a physical GPU handle and a single outputId (exactly 1 bit set), this API
//                returns the output type.
//
//  SUPPORTED OS: Windows XP and higher
//
// RETURN STATUS: NVAPI_INVALID_ARGUMENT: hPhysicalGpu, outputId or pOutputsMask is NULL; or outputId has > 1 bit set
//                NVAPI_OK: *pOutputType contains a NvGpuOutputType value
//                NVAPI_NVIDIA_DEVICE_NOT_FOUND: no NVIDIA GPU driving a display was found
//                NVAPI_EXPECTED_PHYSICAL_GPU_HANDLE: hPhysicalGpu was not a physical GPU handle
//
///////////////////////////////////////////////////////////////////////////////
typedef enum _NV_GPU_OUTPUT_TYPE
{
    NVAPI_GPU_OUTPUT_UNKNOWN  = 0,
    NVAPI_GPU_OUTPUT_CRT      = 1,     // CRT display device
    NVAPI_GPU_OUTPUT_DFP      = 2,     // Digital Flat Panel display device
    NVAPI_GPU_OUTPUT_TV       = 3,     // TV display device
} NV_GPU_OUTPUT_TYPE;

NVAPI_INTERFACE NvAPI_GPU_GetOutputType(NvPhysicalGpuHandle hPhysicalGpu, NvU32 outputId, NV_GPU_OUTPUT_TYPE *pOutputType);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_GPU_ValidateOutputCombination
//
//   DESCRIPTION: This call is used to determine if a set of GPU outputs can be active
//                simultaneously.  While a GPU may have <n> outputs, they can not typically
//                all be active at the same time due to internal resource sharing.
//
//                Given a physical GPU handle and a mask of candidate outputs, this call
//                will return NVAPI_OK if all of the specified outputs can be driven
//                simultaneously.  It will return NVAPI_INVALID_COMBINATION if they cannot.
//
//                Use NvAPI_GPU_GetAllOutputs() to determine which outputs are candidates.
//
//  SUPPORTED OS: Windows XP and higher
//
// RETURN STATUS: NVAPI_OK: combination of outputs in outputsMask are valid (can be active simultaneously)
//                NVAPI_INVALID_COMBINATION: combination of outputs in outputsMask are NOT valid
//                NVAPI_INVALID_ARGUMENT: hPhysicalGpu or outputsMask does not have at least 2 bits set
//                NVAPI_EXPECTED_PHYSICAL_GPU_HANDLE: hPhysicalGpu was not a physical GPU handle
//                NVAPI_NVIDIA_DEVICE_NOT_FOUND: no NVIDIA GPU driving a display was found
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_GPU_ValidateOutputCombination(NvPhysicalGpuHandle hPhysicalGpu, NvU32 outputsMask);

typedef enum _NV_GPU_CONNECTOR_TYPE
{
    NVAPI_GPU_CONNECTOR_VGA_15_PIN                      = 0x00000000,
    NVAPI_GPU_CONNECTOR_TV_COMPOSITE                    = 0x00000010,
    NVAPI_GPU_CONNECTOR_TV_SVIDEO                       = 0x00000011,
    NVAPI_GPU_CONNECTOR_TV_HDTV_COMPONENT               = 0x00000013,
    NVAPI_GPU_CONNECTOR_TV_SCART                        = 0x00000014,
    NVAPI_GPU_CONNECTOR_TV_COMPOSITE_SCART_ON_EIAJ4120  = 0x00000016,
    NVAPI_GPU_CONNECTOR_TV_HDTV_EIAJ4120                = 0x00000017,
    NVAPI_GPU_CONNECTOR_PC_POD_HDTV_YPRPB               = 0x00000018,
    NVAPI_GPU_CONNECTOR_PC_POD_SVIDEO                   = 0x00000019,
    NVAPI_GPU_CONNECTOR_PC_POD_COMPOSITE                = 0x0000001A,
    NVAPI_GPU_CONNECTOR_DVI_I_TV_SVIDEO                 = 0x00000020,
    NVAPI_GPU_CONNECTOR_DVI_I_TV_COMPOSITE              = 0x00000021,
    NVAPI_GPU_CONNECTOR_DVI_I                           = 0x00000030,
    NVAPI_GPU_CONNECTOR_DVI_D                           = 0x00000031,
    NVAPI_GPU_CONNECTOR_ADC                             = 0x00000032,
    NVAPI_GPU_CONNECTOR_LFH_DVI_I_1                     = 0x00000038,
    NVAPI_GPU_CONNECTOR_LFH_DVI_I_2                     = 0x00000039,
    NVAPI_GPU_CONNECTOR_SPWG                            = 0x00000040,
    NVAPI_GPU_CONNECTOR_OEM                             = 0x00000041,
    NVAPI_GPU_CONNECTOR_DISPLAYPORT_EXTERNAL            = 0x00000046,
    NVAPI_GPU_CONNECTOR_DISPLAYPORT_INTERNAL            = 0x00000047,
    NVAPI_GPU_CONNECTOR_HDMI_A                          = 0x00000061,
    NVAPI_GPU_CONNECTOR_UNKNOWN                         = 0xFFFFFFFF,
} NV_GPU_CONNECTOR_TYPE;

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_GPU_GetFullName
//
//   DESCRIPTION: Retrieves the full GPU name as an ascii string.  Eg: "Quadro FX 1400"
//
//  SUPPORTED OS: Mac OS X, Windows XP and higher
//
// RETURN STATUS: NVAPI_ERROR or NVAPI_OK
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_GPU_GetFullName(NvPhysicalGpuHandle hPhysicalGpu, NvAPI_ShortString szName);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_GPU_GetPCIIdentifiers
//
//   DESCRIPTION: Returns the PCI identifiers associated with this GPU.
//                  DeviceId - the internal PCI device identifier for the GPU.
//                  SubSystemId - the internal PCI subsystem identifier for the GPU.
//                  RevisionId - the internal PCI device-specific revision identifier for the GPU.
//                  ExtDeviceId - the external PCI device identifier for the GPU.
//
//  SUPPORTED OS: Mac OS X, Windows XP and higher
//
// RETURN STATUS: NVAPI_INVALID_ARGUMENT: hPhysicalGpu or an argument is NULL
//                NVAPI_OK: arguments are populated with PCI identifiers
//                NVAPI_NVIDIA_DEVICE_NOT_FOUND: no NVIDIA GPU driving a display was found
//                NVAPI_EXPECTED_PHYSICAL_GPU_HANDLE: hPhysicalGpu was not a physical GPU handle
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_GPU_GetPCIIdentifiers(NvPhysicalGpuHandle hPhysicalGpu,NvU32 *pDeviceId,NvU32 *pSubSystemId,NvU32 *pRevisionId,NvU32 *pExtDeviceId);

typedef enum _NV_GPU_TYPE
{
    NV_SYSTEM_TYPE_GPU_UNKNOWN     = 0,
    NV_SYSTEM_TYPE_IGPU            = 1, //integrated
    NV_SYSTEM_TYPE_DGPU            = 2, //discrete
} NV_GPU_TYPE;

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_GPU_GetGPUType
//
// DESCRIPTION: Returns information to identify the GPU type
//
//  SUPPORTED OS: Mac OS X, Windows XP and higher
//
// RETURN STATUS: NVAPI_INVALID_ARGUMENT: hPhysicalGpu
// NVAPI_OK: *pGpuType contains the GPU type
// NVAPI_NVIDIA_DEVICE_NOT_FOUND: no NVIDIA GPU driving a display was found
// NVAPI_EXPECTED_PHYSICAL_GPU_HANDLE: hPhysicalGpu was not a physical GPU handle
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_GPU_GetGPUType(NvPhysicalGpuHandle hPhysicalGpu, NV_GPU_TYPE *pGpuType);

typedef enum _NV_GPU_BUS_TYPE
{
    NVAPI_GPU_BUS_TYPE_UNDEFINED    = 0,
    NVAPI_GPU_BUS_TYPE_PCI          = 1,
    NVAPI_GPU_BUS_TYPE_AGP          = 2,
    NVAPI_GPU_BUS_TYPE_PCI_EXPRESS  = 3,
    NVAPI_GPU_BUS_TYPE_FPCI         = 4,
} NV_GPU_BUS_TYPE;

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_GPU_GetBusType
//
//   DESCRIPTION: Returns the type of bus associated with this GPU.
//
//  SUPPORTED OS: Mac OS X, Windows XP and higher
//
// RETURN STATUS: NVAPI_INVALID_ARGUMENT: hPhysicalGpu or pBusType is NULL
//                NVAPI_OK: *pBusType contains bus identifier
//                NVAPI_NVIDIA_DEVICE_NOT_FOUND: no NVIDIA GPU driving a display was found
//                NVAPI_EXPECTED_PHYSICAL_GPU_HANDLE: hPhysicalGpu was not a physical GPU handle
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_GPU_GetBusType(NvPhysicalGpuHandle hPhysicalGpu,NV_GPU_BUS_TYPE *pBusType);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_GPU_GetBusId
//
//   DESCRIPTION: Returns the ID of bus associated with this GPU.
//
//  SUPPORTED OS: Mac OS X, Windows XP and higher
//
// RETURN STATUS: NVAPI_INVALID_ARGUMENT: hPhysicalGpu or pBusId is NULL
//                NVAPI_OK: *pBusId contains bus id
//                NVAPI_NVIDIA_DEVICE_NOT_FOUND: no NVIDIA GPU driving a display was found
//                NVAPI_EXPECTED_PHYSICAL_GPU_HANDLE: hPhysicalGpu was not a physical GPU handle
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_GPU_GetBusId(NvPhysicalGpuHandle hPhysicalGpu, NvU32 *pBusId);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_GPU_GetBusSlotId
//
//   DESCRIPTION: Returns the ID of bus-slot associated with this GPU.
//
//  SUPPORTED OS: Mac OS X, Windows XP and higher
//
// RETURN STATUS: NVAPI_INVALID_ARGUMENT: hPhysicalGpu or pBusSlotId is NULL
//                NVAPI_OK: *pBusSlotId contains bus-slot id
//                NVAPI_NVIDIA_DEVICE_NOT_FOUND: no NVIDIA GPU driving a display was found
//                NVAPI_EXPECTED_PHYSICAL_GPU_HANDLE: hPhysicalGpu was not a physical GPU handle
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_GPU_GetBusSlotId(NvPhysicalGpuHandle hPhysicalGpu, NvU32 *pBusSlotId);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_GPU_GetIRQ
//
//   DESCRIPTION: Returns the interrupt number associated with this GPU.
//
//  SUPPORTED OS: Mac OS X, Windows XP and higher
//
// RETURN STATUS: NVAPI_INVALID_ARGUMENT: hPhysicalGpu or pIRQ is NULL
//                NVAPI_OK: *pIRQ contains interrupt number
//                NVAPI_NVIDIA_DEVICE_NOT_FOUND: no NVIDIA GPU driving a display was found
//                NVAPI_EXPECTED_PHYSICAL_GPU_HANDLE: hPhysicalGpu was not a physical GPU handle
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_GPU_GetIRQ(NvPhysicalGpuHandle hPhysicalGpu,NvU32 *pIRQ);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_GPU_GetVbiosRevision
//
//   DESCRIPTION: Returns the revision of the video bios associated with this GPU.
//
//  SUPPORTED OS: Mac OS X, Windows XP and higher
//
// RETURN STATUS: NVAPI_INVALID_ARGUMENT: hPhysicalGpu or pBiosRevision is NULL
//                NVAPI_OK: *pBiosRevision contains revision number
//                NVAPI_NVIDIA_DEVICE_NOT_FOUND: no NVIDIA GPU driving a display was found
//                NVAPI_EXPECTED_PHYSICAL_GPU_HANDLE: hPhysicalGpu was not a physical GPU handle
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_GPU_GetVbiosRevision(NvPhysicalGpuHandle hPhysicalGpu,NvU32 *pBiosRevision);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_GPU_GetVbiosOEMRevision
//
//   DESCRIPTION: Returns the OEM revision of the video bios associated with this GPU.
//
//  SUPPORTED OS: Mac OS X, Windows XP and higher
//
// RETURN STATUS: NVAPI_INVALID_ARGUMENT: hPhysicalGpu or pBiosRevision is NULL
//                NVAPI_OK: *pBiosRevision contains revision number
//                NVAPI_NVIDIA_DEVICE_NOT_FOUND: no NVIDIA GPU driving a display was found
//                NVAPI_EXPECTED_PHYSICAL_GPU_HANDLE: hPhysicalGpu was not a physical GPU handle
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_GPU_GetVbiosOEMRevision(NvPhysicalGpuHandle hPhysicalGpu,NvU32 *pBiosRevision);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_GPU_GetVbiosVersionString
//
//   DESCRIPTION: Returns the full bios version string in the form of xx.xx.xx.xx.yy where
//                the xx numbers come from NvAPI_GPU_GetVbiosRevision and yy comes from
//                NvAPI_GPU_GetVbiosOEMRevision.
//
//  SUPPORTED OS: Mac OS X, Windows XP and higher
//
// RETURN STATUS: NVAPI_INVALID_ARGUMENT: hPhysicalGpu is NULL
//                NVAPI_OK: szBiosRevision contains version string
//                NVAPI_NVIDIA_DEVICE_NOT_FOUND: no NVIDIA GPU driving a display was found
//                NVAPI_EXPECTED_PHYSICAL_GPU_HANDLE: hPhysicalGpu was not a physical GPU handle
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_GPU_GetVbiosVersionString(NvPhysicalGpuHandle hPhysicalGpu,NvAPI_ShortString szBiosRevision);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_GPU_GetAGPAperture
//
//   DESCRIPTION: Returns AGP aperture in megabytes
//
//  SUPPORTED OS: Mac OS X, Windows XP and higher
//
// RETURN STATUS: NVAPI_INVALID_ARGUMENT: pSize is NULL
//                NVAPI_OK: call successful
//                NVAPI_NVIDIA_DEVICE_NOT_FOUND: no NVIDIA GPU driving a display was found
//                NVAPI_EXPECTED_PHYSICAL_GPU_HANDLE: hPhysicalGpu was not a physical GPU handle
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_GPU_GetAGPAperture(NvPhysicalGpuHandle hPhysicalGpu,NvU32 *pSize);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_GPU_GetCurrentAGPRate
//
//   DESCRIPTION: Returns the current AGP Rate (1 = 1x, 2=2x etc, 0 = AGP not present)
//
//  SUPPORTED OS: Mac OS X, Windows XP and higher
//
// RETURN STATUS: NVAPI_INVALID_ARGUMENT: pRate is NULL
//                NVAPI_OK: call successful
//                NVAPI_NVIDIA_DEVICE_NOT_FOUND: no NVIDIA GPU driving a display was found
//                NVAPI_EXPECTED_PHYSICAL_GPU_HANDLE: hPhysicalGpu was not a physical GPU handle
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_GPU_GetCurrentAGPRate(NvPhysicalGpuHandle hPhysicalGpu,NvU32 *pRate);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_GPU_GetCurrentPCIEDownstreamWidth
//
//   DESCRIPTION: Returns the number of PCIE lanes being used for the PCIE interface
//                downstream from the GPU.
//
//                On systems that do not support PCIE, the maxspeed for the root link
//                will be zero.
//
//  SUPPORTED OS: Mac OS X, Windows XP and higher
//
// RETURN STATUS: NVAPI_INVALID_ARGUMENT: pWidth is NULL
//                NVAPI_OK: call successful
//                NVAPI_NVIDIA_DEVICE_NOT_FOUND: no NVIDIA GPU driving a display was found
//                NVAPI_EXPECTED_PHYSICAL_GPU_HANDLE: hPhysicalGpu was not a physical GPU handle
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_GPU_GetCurrentPCIEDownstreamWidth(NvPhysicalGpuHandle hPhysicalGpu,NvU32 *pWidth);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_GPU_GetPhysicalFrameBufferSize
//
//   DESCRIPTION: Returns the physical size of framebuffer in Kb.  This does NOT include any
//                system RAM that may be dedicated for use by the GPU.
//
//  SUPPORTED OS: Mac OS X, Windows XP and higher
//
// RETURN STATUS: NVAPI_INVALID_ARGUMENT: pSize is NULL
//                NVAPI_OK: call successful
//                NVAPI_NVIDIA_DEVICE_NOT_FOUND: no NVIDIA GPU driving a display was found
//                NVAPI_EXPECTED_PHYSICAL_GPU_HANDLE: hPhysicalGpu was not a physical GPU handle
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_GPU_GetPhysicalFrameBufferSize(NvPhysicalGpuHandle hPhysicalGpu,NvU32 *pSize);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_GPU_GetVirtualFrameBufferSize
//
//   DESCRIPTION: Returns the virtual size of framebuffer in Kb.  This includes the physical RAM plus any
//                system RAM that has been dedicated for use by the GPU.
//
//  SUPPORTED OS: Mac OS X, Windows XP and higher
//
// RETURN STATUS: NVAPI_INVALID_ARGUMENT: pSize is NULL
//                NVAPI_OK: call successful
//                NVAPI_NVIDIA_DEVICE_NOT_FOUND: no NVIDIA GPU driving a display was found
//                NVAPI_EXPECTED_PHYSICAL_GPU_HANDLE: hPhysicalGpu was not a physical GPU handle
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_GPU_GetVirtualFrameBufferSize(NvPhysicalGpuHandle hPhysicalGpu,NvU32 *pSize);

///////////////////////////////////////////////////////////////////////////////////
//  Thermal API
//  Provides ability to get temperature levels from the various thermal sensors associated with the GPU

#define NVAPI_MAX_THERMAL_SENSORS_PER_GPU 3

typedef enum
{
    NVAPI_THERMAL_TARGET_NONE          = 0,
    NVAPI_THERMAL_TARGET_GPU           = 1,     //GPU core temperature requires NvPhysicalGpuHandle
    NVAPI_THERMAL_TARGET_MEMORY        = 2,     //GPU memory temperature requires NvPhysicalGpuHandle
    NVAPI_THERMAL_TARGET_POWER_SUPPLY  = 4,     //GPU power supply temperature requires NvPhysicalGpuHandle
    NVAPI_THERMAL_TARGET_BOARD         = 8,     //GPU board ambient temperature requires NvPhysicalGpuHandle
    NVAPI_THERMAL_TARGET_VCD_BOARD     = 9,     //Visual Computing Device Board temperature requires NvVisualComputingDeviceHandle
    NVAPI_THERMAL_TARGET_VCD_INLET     = 10,    //Visual Computing Device Inlet temperature requires NvVisualComputingDeviceHandle
    NVAPI_THERMAL_TARGET_VCD_OUTLET    = 11,    //Visual Computing Device Outlet temperature requires NvVisualComputingDeviceHandle
    NVAPI_THERMAL_TARGET_ALL           = 15,
    NVAPI_THERMAL_TARGET_UNKNOWN       = -1,
} NV_THERMAL_TARGET;

typedef enum
{
    NVAPI_THERMAL_CONTROLLER_NONE = 0,
    NVAPI_THERMAL_CONTROLLER_GPU_INTERNAL,
    NVAPI_THERMAL_CONTROLLER_ADM1032,
    NVAPI_THERMAL_CONTROLLER_MAX6649,
    NVAPI_THERMAL_CONTROLLER_MAX1617,
    NVAPI_THERMAL_CONTROLLER_LM99,
    NVAPI_THERMAL_CONTROLLER_LM89,
    NVAPI_THERMAL_CONTROLLER_LM64,
    NVAPI_THERMAL_CONTROLLER_ADT7473,
    NVAPI_THERMAL_CONTROLLER_SBMAX6649,
    NVAPI_THERMAL_CONTROLLER_VBIOSEVT,
    NVAPI_THERMAL_CONTROLLER_OS,
    NVAPI_THERMAL_CONTROLLER_UNKNOWN = -1,
} NV_THERMAL_CONTROLLER;

typedef struct
{
    NvU32   version;                //structure version
    NvU32   count;                  //number of associated thermal sensors
    struct
    {
        NV_THERMAL_CONTROLLER       controller;         //internal, ADM1032, MAX6649...
        NvU32                       defaultMinTemp;     //the min default temperature value of the thermal sensor in degrees centigrade
        NvU32                       defaultMaxTemp;    //the max default temperature value of the thermal sensor in degrees centigrade
        NvU32                       currentTemp;       //the current temperature value of the thermal sensor in degrees centigrade
        NV_THERMAL_TARGET           target;             //thermal senor targeted @ GPU, memory, chipset, powersupply, Visual Computing Device...
    } sensor[NVAPI_MAX_THERMAL_SENSORS_PER_GPU];

} NV_GPU_THERMAL_SETTINGS_V1;

typedef struct
{
    NvU32   version;                //structure version
    NvU32   count;                  //number of associated thermal sensors
    struct
    {
        NV_THERMAL_CONTROLLER       controller;         //internal, ADM1032, MAX6649...
        NvS32                       defaultMinTemp;     //the min default temperature value of the thermal sensor in degrees centigrade
        NvS32                       defaultMaxTemp;     //the max default temperature value of the thermal sensor in degrees centigrade
        NvS32                       currentTemp;        //the current temperature value of the thermal sensor in degrees centigrade
        NV_THERMAL_TARGET           target;             //thermal senor targeted @ GPU, memory, chipset, powersupply, Visual Computing Device...
    } sensor[NVAPI_MAX_THERMAL_SENSORS_PER_GPU];

} NV_GPU_THERMAL_SETTINGS_V2;

typedef NV_GPU_THERMAL_SETTINGS_V2  NV_GPU_THERMAL_SETTINGS;

#define NV_GPU_THERMAL_SETTINGS_VER_1   MAKE_NVAPI_VERSION(NV_GPU_THERMAL_SETTINGS_V1,1)
#define NV_GPU_THERMAL_SETTINGS_VER_2   MAKE_NVAPI_VERSION(NV_GPU_THERMAL_SETTINGS_V2,2)
#define NV_GPU_THERMAL_SETTINGS_VER     NV_GPU_THERMAL_SETTINGS_VER_2

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME:   NvAPI_GPU_GetThermalSettings
//
// DESCRIPTION:     Retrieves the thermal information of all thermal sensors or specific thermal sensor associated with the selected GPU.
//                  Thermal sensors are indexed 0 to NVAPI_MAX_THERMAL_SENSORS_PER_GPU-1.
//                  To retrieve specific thermal sensor info set the sensorIndex to the required thermal sensor index.
//                  To retrieve info for all sensors set sensorIndex to NVAPI_THERMAL_TARGET_ALL.
//
//  SUPPORTED OS: Mac OS X, Windows XP and higher
//
// PARAMETERS :     hPhysicalGPU(IN) - GPU selection.
//                  sensorIndex(IN)  - Explicit thermal sensor index selection.
//                  pThermalSettings(OUT) - Array of thermal settings.
//
// RETURN STATUS:
//    NVAPI_OK - completed request
//    NVAPI_ERROR - miscellaneous error occurred
//    NVAPI_INVALID_ARGUMENT - pThermalInfo is NULL
//    NVAPI_HANDLE_INVALIDATED - handle passed has been invalidated (see user guide)
//    NVAPI_EXPECTED_PHYSICAL_GPU_HANDLE - handle passed is not a physical GPU handle
//    NVAPI_INCOMPATIBLE_STRUCT_VERSION - the version of the INFO struct is not supported
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_GPU_GetThermalSettings(NvPhysicalGpuHandle hPhysicalGpu, NvU32 sensorIndex, NV_GPU_THERMAL_SETTINGS *pThermalSettings);

////////////////////////////////////////////////////////////////////////////////
//NvAPI_TVOutput Information

typedef enum _NV_DISPLAY_TV_FORMAT
{
    NV_DISPLAY_TV_FORMAT_NONE         = 0,
    NV_DISPLAY_TV_FORMAT_SD_NTSCM     = 0x00000001,
    NV_DISPLAY_TV_FORMAT_SD_NTSCJ     = 0x00000002,
    NV_DISPLAY_TV_FORMAT_SD_PALM      = 0x00000004,
    NV_DISPLAY_TV_FORMAT_SD_PALBDGH   = 0x00000008,
    NV_DISPLAY_TV_FORMAT_SD_PALN      = 0x00000010,
    NV_DISPLAY_TV_FORMAT_SD_PALNC     = 0x00000020,
    NV_DISPLAY_TV_FORMAT_SD_576i      = 0x00000100,
    NV_DISPLAY_TV_FORMAT_SD_480i      = 0x00000200,
    NV_DISPLAY_TV_FORMAT_ED_480p      = 0x00000400,
    NV_DISPLAY_TV_FORMAT_ED_576p      = 0x00000800,
    NV_DISPLAY_TV_FORMAT_HD_720p      = 0x00001000,
    NV_DISPLAY_TV_FORMAT_HD_1080i     = 0x00002000,
    NV_DISPLAY_TV_FORMAT_HD_1080p     = 0x00004000,
    NV_DISPLAY_TV_FORMAT_HD_720p50    = 0x00008000,
    NV_DISPLAY_TV_FORMAT_HD_1080p24   = 0x00010000,
    NV_DISPLAY_TV_FORMAT_HD_1080i50   = 0x00020000,
    NV_DISPLAY_TV_FORMAT_HD_1080p50   = 0x00040000,

    NV_DISPLAY_TV_FORMAT_SD_OTHER     = 0x01000000,
    NV_DISPLAY_TV_FORMAT_ED_OTHER     = 0x02000000,
    NV_DISPLAY_TV_FORMAT_HD_OTHER     = 0x04000000,

    NV_DISPLAY_TV_FORMAT_ANY          = 0x80000000,

} NV_DISPLAY_TV_FORMAT;

///////////////////////////////////////////////////////////////////////////////////
//  I2C API
//  Provides ability to read or write data using I2C protocol.
//  These APIs allow I2C access only to DDC monitors

#define NVAPI_MAX_SIZEOF_I2C_DATA_BUFFER 4096
#define NVAPI_DISPLAY_DEVICE_MASK_MAX 24

typedef struct
{
    NvU32                   version;        //structure version
    NvU32                   displayMask;    //the Display Mask of the concerned display
    NvU8                    bIsDDCPort;     //Flag indicating DDC port or a communication port
    NvU8                    i2cDevAddress;  //the I2C target device address
    NvU8*                   pbI2cRegAddress;//the I2C target register address
    NvU32                   regAddrSize;    //the size in bytes of target register address
    NvU8*                   pbData;         //The buffer of data which is to be read/written
    NvU32                   cbSize;         //The size of Data buffer to be read.
    NvU32                   i2cSpeed;       //The target speed of the transaction (between 28kbps to 40kbps; not guaranteed)
} NV_I2C_INFO;

#define NV_I2C_INFO_VER  MAKE_NVAPI_VERSION(NV_I2C_INFO,1)
/***********************************************************************************/

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME:  NvAPI_I2CRead
//
// DESCRIPTION:    Read data buffer from I2C port
//
//  SUPPORTED OS: Mac OS X, Windows XP and higher
//
// PARAMETERS:     hPhysicalGPU(IN) - GPU selection.
//                 NV_I2C_INFO *pI2cInfo -The I2c data input structure
//
// RETURN STATUS:
//    NVAPI_OK - completed request
//    NVAPI_ERROR - miscellaneous error occurred
//    NVAPI_HANDLE_INVALIDATED - handle passed has been invalidated (see user guide)
//    NVAPI_EXPECTED_PHYSICAL_GPU_HANDLE - handle passed is not a physical GPU handle
//    NVAPI_INCOMPATIBLE_STRUCT_VERSION - structure version is not supported
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_I2CRead(NvPhysicalGpuHandle hPhysicalGpu, NV_I2C_INFO *pI2cInfo);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME:  NvAPI_I2CWrite
//
// DESCRIPTION:    Writes data buffer to I2C port
//
//  SUPPORTED OS: Mac OS X, Windows XP and higher
//
// PARAMETERS:     hPhysicalGPU(IN) - GPU selection.
//                 NV_I2C_INFO *pI2cInfo -The I2c data input structure
//
// RETURN STATUS:
//    NVAPI_OK - completed request
//    NVAPI_ERROR - miscellaneous error occurred
//    NVAPI_HANDLE_INVALIDATED - handle passed has been invalidated (see user guide)
//    NVAPI_EXPECTED_PHYSICAL_GPU_HANDLE - handle passed is not a physical GPU handle
//    NVAPI_INCOMPATIBLE_STRUCT_VERSION - structure version is not supported
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_I2CWrite(NvPhysicalGpuHandle hPhysicalGpu, NV_I2C_INFO *pI2cInfo);


typedef struct
{
    NvU32               version;            //structure version
    NvU32               vendorId;           //vendorId
    NvU32               deviceId;           //deviceId
    NvAPI_ShortString   szVendorName;       //vendor Name
    NvAPI_ShortString   szChipsetName;      //device Name
    NvU32               flags;              //Chipset info flags - obsolete
    NvU32               subSysVendorId;     //subsystem vendorId
    NvU32               subSysDeviceId;     //subsystem deviceId
    NvAPI_ShortString   szSubSysVendorName; //subsystem vendor Name
} NV_CHIPSET_INFO;

#define NV_CHIPSET_INFO_VER     MAKE_NVAPI_VERSION(NV_CHIPSET_INFO,3)

typedef enum
{
    NV_CHIPSET_INFO_HYBRID          = 0x00000001,
} NV_CHIPSET_INFO_FLAGS;

typedef struct
{
    NvU32               version;        //structure version
    NvU32               vendorId;       //vendorId
    NvU32               deviceId;       //deviceId
    NvAPI_ShortString   szVendorName;   //vendor Name
    NvAPI_ShortString   szChipsetName;  //device Name
    NvU32               flags;          //Chipset info flags
} NV_CHIPSET_INFO_v2;

#define NV_CHIPSET_INFO_VER_2   MAKE_NVAPI_VERSION(NV_CHIPSET_INFO_v2,2)

typedef struct
{
    NvU32               version;        //structure version
    NvU32               vendorId;       //vendorId
    NvU32               deviceId;       //deviceId
    NvAPI_ShortString   szVendorName;   //vendor Name
    NvAPI_ShortString   szChipsetName;  //device Name
} NV_CHIPSET_INFO_v1;

#define NV_CHIPSET_INFO_VER_1  MAKE_NVAPI_VERSION(NV_CHIPSET_INFO_v1,1)

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_SYS_GetChipSetInfo
//
//   DESCRIPTION: Returns information about the System's ChipSet
//
//  SUPPORTED OS: Mac OS X, Windows XP and higher
//
// RETURN STATUS: NVAPI_INVALID_ARGUMENT: pChipSetInfo is NULL
//                NVAPI_OK: *pChipSetInfo is now set
//                NVAPI_INCOMPATIBLE_STRUCT_VERSION - NV_CHIPSET_INFO version not compatible with driver
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_SYS_GetChipSetInfo(NV_CHIPSET_INFO *pChipSetInfo);

typedef struct
{
    NvU32 version;    // Structure version, constructed from macro below
    NvU32 currentLidState;
    NvU32 currentDockState;
    NvU32 currentLidPolicy;
    NvU32 currentDockPolicy;
    NvU32 forcedLidMechanismPresent;
    NvU32 forcedDockMechanismPresent;
}NV_LID_DOCK_PARAMS;

#define NV_LID_DOCK_PARAMS_VER  MAKE_NVAPI_VERSION(NV_LID_DOCK_PARAMS,1)

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_GetLidDockInfo
//
// DESCRIPTION: Returns current lid and dock information.
//
//  SUPPORTED OS: Mac OS X, Windows XP and higher
//
// RETURN STATUS: NVAPI_OK: now *pLidAndDock contains the returned lid and dock information.
//                NVAPI_ERROR:If any way call is not success.
//                NVAPI_NOT_SUPPORTED:If any way call is not success.
//                NVAPI_HANDLE_INVALIDATED:If nvapi escape result handle is invalid.
//                NVAPI_API_NOT_INTIALIZED:If NVAPI is not initialized.
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_SYS_GetLidAndDockInfo(NV_LID_DOCK_PARAMS *pLidAndDock);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_OGL_ExpertModeSet[Get]
//
//   DESCRIPTION: Configure OpenGL Expert Mode, an API usage feedback and
//                advice reporting mechanism. The effects of this call are
//                applied only to the current context, and are reset to the
//                defaults when the context is destroyed.
//
//                Note: This feature is valid at runtime only when GLExpert
//                      functionality has been built into the OpenGL driver
//                      installed on the system. All Windows Vista OpenGL
//                      drivers provided by NVIDIA have this instrumentation
//                      included by default. Windows XP, however, requires a
//                      special display driver available with the NVIDIA
//                      PerfSDK found at developer.nvidia.com.
//
//                Note: These functions are valid only for the current OpenGL
//                      context. Calling these functions prior to creating a
//                      context and calling MakeCurrent with it will result
//                      in errors and undefined behavior.
//
//    PARAMETERS: expertDetailMask  Mask made up of NVAPI_OGLEXPERT_DETAIL bits,
//                                  this parameter specifies the detail level in
//                                  the feedback stream.
//
//                expertReportMask  Mask made up of NVAPI_OGLEXPERT_REPORT bits,
//                                  this parameter specifies the areas of
//                                  functional interest.
//
//                expertOutputMask  Mask made up of NVAPI_OGLEXPERT_OUTPUT bits,
//                                  this parameter specifies the feedback output
//                                  location.
//
//                expertCallback    Used in conjunction with OUTPUT_TO_CALLBACK,
//                                  this is a simple callback function the user
//                                  may use to obtain the feedback stream. The
//                                  function will be called once per fully
//                                  qualified feedback stream entry.
//
// RETURN STATUS: NVAPI_API_NOT_INTIALIZED         : NVAPI not initialized
//                NVAPI_NVIDIA_DEVICE_NOT_FOUND    : no NVIDIA GPU found
//                NVAPI_OPENGL_CONTEXT_NOT_CURRENT : no NVIDIA OpenGL context
//                                                   which supports GLExpert
//                                                   has been made current
//                NVAPI_ERROR : OpenGL driver failed to load properly
//                NVAPI_OK    : success
//
///////////////////////////////////////////////////////////////////////////////
#define NVAPI_OGLEXPERT_DETAIL_NONE                 0x00000000
#define NVAPI_OGLEXPERT_DETAIL_ERROR                0x00000001
#define NVAPI_OGLEXPERT_DETAIL_SWFALLBACK           0x00000002
#define NVAPI_OGLEXPERT_DETAIL_BASIC_INFO           0x00000004
#define NVAPI_OGLEXPERT_DETAIL_DETAILED_INFO        0x00000008
#define NVAPI_OGLEXPERT_DETAIL_PERFORMANCE_WARNING  0x00000010
#define NVAPI_OGLEXPERT_DETAIL_QUALITY_WARNING      0x00000020
#define NVAPI_OGLEXPERT_DETAIL_USAGE_WARNING        0x00000040
#define NVAPI_OGLEXPERT_DETAIL_ALL                  0xFFFFFFFF

#define NVAPI_OGLEXPERT_REPORT_NONE                 0x00000000
#define NVAPI_OGLEXPERT_REPORT_ERROR                0x00000001
#define NVAPI_OGLEXPERT_REPORT_SWFALLBACK           0x00000002
#define NVAPI_OGLEXPERT_REPORT_PIPELINE_VERTEX      0x00000004
#define NVAPI_OGLEXPERT_REPORT_PIPELINE_GEOMETRY    0x00000008
#define NVAPI_OGLEXPERT_REPORT_PIPELINE_XFB         0x00000010
#define NVAPI_OGLEXPERT_REPORT_PIPELINE_RASTER      0x00000020
#define NVAPI_OGLEXPERT_REPORT_PIPELINE_FRAGMENT    0x00000040
#define NVAPI_OGLEXPERT_REPORT_PIPELINE_ROP         0x00000080
#define NVAPI_OGLEXPERT_REPORT_PIPELINE_FRAMEBUFFER 0x00000100
#define NVAPI_OGLEXPERT_REPORT_PIPELINE_PIXEL       0x00000200
#define NVAPI_OGLEXPERT_REPORT_PIPELINE_TEXTURE     0x00000400
#define NVAPI_OGLEXPERT_REPORT_OBJECT_BUFFEROBJECT  0x00000800
#define NVAPI_OGLEXPERT_REPORT_OBJECT_TEXTURE       0x00001000
#define NVAPI_OGLEXPERT_REPORT_OBJECT_PROGRAM       0x00002000
#define NVAPI_OGLEXPERT_REPORT_OBJECT_FBO           0x00004000
#define NVAPI_OGLEXPERT_REPORT_FEATURE_SLI          0x00008000
#define NVAPI_OGLEXPERT_REPORT_ALL                  0xFFFFFFFF

#define NVAPI_OGLEXPERT_OUTPUT_TO_NONE              0x00000000
#define NVAPI_OGLEXPERT_OUTPUT_TO_CONSOLE           0x00000001
#define NVAPI_OGLEXPERT_OUTPUT_TO_DEBUGGER          0x00000004
#define NVAPI_OGLEXPERT_OUTPUT_TO_CALLBACK          0x00000008
#define NVAPI_OGLEXPERT_OUTPUT_TO_ALL               0xFFFFFFFF

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION TYPE: NVAPI_OGLEXPERT_CALLBACK
//
//   DESCRIPTION: Used in conjunction with OUTPUT_TO_CALLBACK, this is a simple
//                callback function the user may use to obtain the feedback
//                stream. The function will be called once per fully qualified
//                feedback stream entry.
//
//    PARAMETERS: categoryId   Contains the bit from the NVAPI_OGLEXPERT_REPORT
//                             mask that corresponds to the current message
//                messageId    Unique Id for the current message
//                detailLevel  Contains the bit from the NVAPI_OGLEXPERT_DETAIL
//                             mask that corresponds to the current message
//                objectId     Unique Id of the object that corresponds to the
//                             current message
//                messageStr   Text string from the current message
//
///////////////////////////////////////////////////////////////////////////////
typedef void (* NVAPI_OGLEXPERT_CALLBACK) (unsigned int categoryId, unsigned int messageId, unsigned int detailLevel, int objectId, const char *messageStr);

//  SUPPORTED OS: Windows XP and higher
NVAPI_INTERFACE NvAPI_OGL_ExpertModeSet(NvU32 expertDetailLevel,
                                        NvU32 expertReportMask,
                                        NvU32 expertOutputMask,
                     NVAPI_OGLEXPERT_CALLBACK expertCallback);


//  SUPPORTED OS: Windows XP and higher
NVAPI_INTERFACE NvAPI_OGL_ExpertModeGet(NvU32 *pExpertDetailLevel,
                                        NvU32 *pExpertReportMask,
                                        NvU32 *pExpertOutputMask,
                     NVAPI_OGLEXPERT_CALLBACK *pExpertCallback);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_OGL_ExpertModeDefaultsSet[Get]
//
//   DESCRIPTION: Configure OpenGL Expert Mode global defaults. These settings
//                apply to any OpenGL application which starts up after these
//                values are applied (i.e. these settings *do not* apply to
//                currently running applications).
//
//    PARAMETERS: expertDetailLevel Value which specifies the detail level in
//                                  the feedback stream. This is a mask made up
//                                  of NVAPI_OGLEXPERT_LEVEL bits.
//
//                expertReportMask  Mask made up of NVAPI_OGLEXPERT_REPORT bits,
//                                  this parameter specifies the areas of
//                                  functional interest.
//
//                expertOutputMask  Mask made up of NVAPI_OGLEXPERT_OUTPUT bits,
//                                  this parameter specifies the feedback output
//                                  location. Note that using OUTPUT_TO_CALLBACK
//                                  here is meaningless and has no effect, but
//                                  using it will not cause an error.
//
// RETURN STATUS: NVAPI_ERROR or NVAPI_OK
//
///////////////////////////////////////////////////////////////////////////////

//  SUPPORTED OS: Windows XP and higher
NVAPI_INTERFACE NvAPI_OGL_ExpertModeDefaultsSet(NvU32 expertDetailLevel,
                                                NvU32 expertReportMask,
                                                NvU32 expertOutputMask);

//  SUPPORTED OS: Windows XP and higher
NVAPI_INTERFACE NvAPI_OGL_ExpertModeDefaultsGet(NvU32 *pExpertDetailLevel,
                                                NvU32 *pExpertReportMask,
                                                NvU32 *pExpertOutputMask);

#define NVAPI_MAX_VIEW_TARGET  2
#define NVAPI_ADVANCED_MAX_VIEW_TARGET 4

typedef enum _NV_TARGET_VIEW_MODE
{
    NV_VIEW_MODE_STANDARD  = 0,
    NV_VIEW_MODE_CLONE     = 1,
    NV_VIEW_MODE_HSPAN     = 2,
    NV_VIEW_MODE_VSPAN     = 3,
    NV_VIEW_MODE_DUALVIEW  = 4,
    NV_VIEW_MODE_MULTIVIEW = 5,
} NV_TARGET_VIEW_MODE;


// Following definitions are used in NvAPI_SetViewEx.
// Scaling modes
typedef enum _NV_SCALING
{
    NV_SCALING_DEFAULT          = 0,        // No change
    NV_SCALING_MONITOR_SCALING  = 1,
    NV_SCALING_ADAPTER_SCALING  = 2,
    NV_SCALING_CENTERED         = 3,
    NV_SCALING_ASPECT_SCALING   = 5,
    NV_SCALING_CUSTOMIZED       = 255       // For future use
} NV_SCALING;

// Rotate modes
typedef enum _NV_ROTATE
{
    NV_ROTATE_0           = 0,
    NV_ROTATE_90          = 1,
    NV_ROTATE_180         = 2,
    NV_ROTATE_270         = 3,
    NV_ROTATE_IGNORED     = 4,
} NV_ROTATE;

// Color formats
#define NVFORMAT_MAKEFOURCC(ch0, ch1, ch2, ch3)                              \
                         ((NvU32)(NvU8)(ch0) | ((NvU32)(NvU8)(ch1) << 8) |   \
                     ((NvU32)(NvU8)(ch2) << 16) | ((NvU32)(NvU8)(ch3) << 24 ))

typedef enum _NV_FORMAT
{
    NV_FORMAT_UNKNOWN           =  0,       // unknown. Driver will choose one as following value.
    NV_FORMAT_P8                = 41,       // for 8bpp mode
    NV_FORMAT_R5G6B5            = 23,       // for 16bpp mode
    NV_FORMAT_A8R8G8B8          = 21,       // for 32bpp mode
    NV_FORMAT_A16B16G16R16F     = 113,      // for 64bpp(floating point) mode.

} NV_FORMAT;

// TV standard


///////////////////////////////////////////////////////////////////////////////
// FUNCTION NAME:   NvAPI_SetView
//
// DESCRIPTION:     This API lets caller to modify target display arrangement for selected source display handle in any of the nview modes.
//                  It also allows to modify or extend the source display in dualview mode.
//                  Note: Maps the selected source to the associated target Ids.
//                  Note: Display PATH with this API is limited to single GPU. DUALVIEW across GPUs cannot be enabled with this API.
//
//  SUPPORTED OS: Windows Vista and higher
//
// PARAMETERS:      hNvDisplay(IN) - NVIDIA Display selection. NVAPI_DEFAULT_HANDLE not allowed, it has to be a handle enumerated with NvAPI_EnumNVidiaDisplayHandle().
//                  pTargetInfo(IN) - Pointer to array of NV_VIEW_TARGET_INFO, specifying device properties in this view.
//                                    The first device entry in the array is the physical primary.
//                                    The device entry with the lowest source id is the desktop primary.
//                  targetCount(IN) - Count of target devices specified in pTargetInfo.
//                  targetView(IN) - Target view selected from NV_TARGET_VIEW_MODE.
//
// RETURN STATUS:
//                  NVAPI_OK - completed request
//                  NVAPI_ERROR - miscellaneous error occurred
//                  NVAPI_INVALID_ARGUMENT: Invalid input parameter.
//
///////////////////////////////////////////////////////////////////////////////

typedef struct
{
    NvU32 version;     // (IN) structure version
    NvU32 count;       // (IN) target count
    struct
    {
        NvU32 deviceMask;   // (IN/OUT) device mask
        NvU32 sourceId;     // (IN/OUT) Values will be based on the number of heads exposed per GPU(0, 1?).
        NvU32 bPrimary:1;   // (OUT) Indicates if this is the GPU's primary view target. This is not the desktop GDI primary.
                            // NvAPI_SetView automatically selects the first target in NV_VIEW_TARGET_INFO index 0 as the GPU's primary view.
        NvU32 bInterlaced:1;// (IN/OUT) Indicates if the timing being used on this monitor is interlaced
        NvU32 bGDIPrimary:1;// (IN/OUT) Indicates if this is the desktop GDI primary.
        NvU32 bForceModeSet:1;// (IN) Used only on Win7 and higher during a call to NvAPI_SetView. Turns off optimization & forces OS to set supplied mode.
    } target[NVAPI_MAX_VIEW_TARGET];
} NV_VIEW_TARGET_INFO;

#define NV_VIEW_TARGET_INFO_VER  MAKE_NVAPI_VERSION(NV_VIEW_TARGET_INFO,2)

NVAPI_INTERFACE NvAPI_SetView(NvDisplayHandle hNvDisplay, NV_VIEW_TARGET_INFO *pTargetInfo, NV_TARGET_VIEW_MODE targetView);

///////////////////////////////////////////////////////////////////////////////
// FUNCTION NAME:   NvAPI_GetView
//
// DESCRIPTION:     This API lets caller retrieve the target display arrangement for selected source display handle.
//                  Note: Display PATH with this API is limited to single GPU. DUALVIEW across GPUs will be returned as STANDARD VIEW.
//                        Please use NvAPI_SYS_GetDisplayTopologies to query views across GPUs.
//  SUPPORTED OS: Windows Vista and higher
//
// PARAMETERS:      hNvDisplay(IN) - NVIDIA Display selection. NVAPI_DEFAULT_HANDLE not allowed, it has to be a handle enumerated with NvAPI_EnumNVidiaDisplayHandle().
//                  pTargetInfo(OUT) - User allocated storage to retrieve an array of  NV_VIEW_TARGET_INFO. Can be NULL to retrieve the targetCount.
//                  targetMaskCount(IN/OUT) - Count of target device mask specified in pTargetMask.
//                  targetView(OUT) - Target view selected from NV_TARGET_VIEW_MODE.
//
// RETURN STATUS:
//                  NVAPI_OK - completed request
//                  NVAPI_ERROR - miscellaneous error occurred
//                  NVAPI_INVALID_ARGUMENT: Invalid input parameter.
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_GetView(NvDisplayHandle hNvDisplay, NV_VIEW_TARGET_INFO *pTargets, NvU32 *pTargetMaskCount, NV_TARGET_VIEW_MODE *pTargetView);

///////////////////////////////////////////////////////////////////////////////
// FUNCTION NAME:   NvAPI_SetViewEx
//
// DESCRIPTION:     This API lets caller to modify the display arrangement for selected source display handle in any of the nview modes.
//                  It also allows to modify or extend the source display in dualview mode.
//                  Note: Maps the selected source to the associated target Ids.
//                  Note: Display PATH with this API is limited to single GPU. DUALVIEW across GPUs cannot be enabled with this API.
//
//  SUPPORTED OS: Windows Vista and higher
//
// PARAMETERS:      hNvDisplay(IN) - NVIDIA Display selection. NVAPI_DEFAULT_HANDLE not allowed, it has to be a handle enumerated with NvAPI_EnumNVidiaDisplayHandle().
//                  pPathInfo(IN)  - Pointer to array of NV_VIEW_PATH_INFO, specifying device properties in this view.
//                                    The first device entry in the array is the physical primary.
//                                    The device entry with the lowest source id is the desktop primary.
//                  pathCount(IN)  - Count of paths specified in pPathInfo.
//                  displayView(IN)- Display view selected from NV_TARGET_VIEW_MODE.
//
// RETURN STATUS:
//                  NVAPI_OK - completed request
//                  NVAPI_ERROR - miscellaneous error occurred
//                  NVAPI_INVALID_ARGUMENT: Invalid input parameter.
//
///////////////////////////////////////////////////////////////////////////////

#define NVAPI_MAX_DISPLAY_PATH  NVAPI_MAX_VIEW_TARGET

#define NVAPI_ADVANCED_MAX_DISPLAY_PATH  NVAPI_ADVANCED_MAX_VIEW_TARGET

typedef struct
{
    NvU32                   deviceMask;     // (IN) device mask
    NvU32                   sourceId;       // (IN) Values will be based on the number of heads exposed per GPU(0, 1?)
    NvU32                   bPrimary:1;     // (IN/OUT) Indicates if this is the GPU's primary view target. This is not the desktop GDI primary.
                                            // NvAPI_SetViewEx automatically selects the first target in NV_DISPLAY_PATH_INFO index 0 as the GPU's primary view.
    NV_GPU_CONNECTOR_TYPE   connector;      // (IN) Specify connector type. For TV only.

    // source mode information
    NvU32                   width;          // (IN) width of the mode
    NvU32                   height;         // (IN) height of the mode
    NvU32                   depth;          // (IN) depth of the mode
    NV_FORMAT               colorFormat;    //      color format if needs to specify. Not used now.

    //rotation setting of the mode
    NV_ROTATE               rotation;       // (IN) rotation setting.

    // the scaling mode
    NV_SCALING              scaling;        // (IN) scaling setting

    // Timing info
    NvU32                   refreshRate;    // (IN) refresh rate of the mode
    NvU32                   interlaced:1;   // (IN) interlaced mode flag

    NV_DISPLAY_TV_FORMAT    tvFormat;       // (IN) to choose the last TV format set this value to NV_DISPLAY_TV_FORMAT_NONE

    // Windows desktop position
    NvU32                   posx;           // (IN/OUT) x offset of this display on the Windows desktop
    NvU32                   posy;           // (IN/OUT) y offset of this display on the Windows desktop
    NvU32                   bGDIPrimary:1;  // (IN/OUT) Indicates if this is the desktop GDI primary.

    NvU32                   bForceModeSet:1;// (IN) Used only on Win7 and higher during a call to NvAPI_SetViewEx. Turns off optimization & forces OS to set supplied mode.
    NvU32                   bFocusDisplay:1;// (IN) If set, this display path should have the focus after the GPU topology change
    NvU32                   gpuId:24;       // (IN) the physical display/target Gpu id which is the owner of the scan out (for SLI multimon, display from the slave Gpu)

} NV_DISPLAY_PATH;

typedef struct
{
    NvU32 version;     // (IN) structure version
    NvU32 count;       // (IN) path count
    NV_DISPLAY_PATH path[NVAPI_MAX_DISPLAY_PATH];
} NV_DISPLAY_PATH_INFO;

#define NV_DISPLAY_PATH_INFO_VER  NV_DISPLAY_PATH_INFO_VER3
#define NV_DISPLAY_PATH_INFO_VER3 MAKE_NVAPI_VERSION(NV_DISPLAY_PATH_INFO,3)
#define NV_DISPLAY_PATH_INFO_VER2 MAKE_NVAPI_VERSION(NV_DISPLAY_PATH_INFO,2)
#define NV_DISPLAY_PATH_INFO_VER1 MAKE_NVAPI_VERSION(NV_DISPLAY_PATH_INFO,1)

NVAPI_INTERFACE NvAPI_SetViewEx(NvDisplayHandle hNvDisplay, NV_DISPLAY_PATH_INFO *pPathInfo, NV_TARGET_VIEW_MODE displayView);

///////////////////////////////////////////////////////////////////////////////
// FUNCTION NAME:   NvAPI_GetViewEx
//
// DESCRIPTION:     This API lets caller retrieve the target display arrangement for selected source display handle.
//                  Note: Display PATH with this API is limited to single GPU. DUALVIEW across GPUs will be returned as STANDARD VIEW.
//                        Please use NvAPI_SYS_GetDisplayTopologies to query views across GPUs.
//
//  SUPPORTED OS: Windows Vista and higher
//
// PARAMETERS:      hNvDisplay(IN) - NVIDIA Display selection. NVAPI_DEFAULT_HANDLE not allowed, it has to be a handle enumerated with NvAPI_EnumNVidiaDisplayHandle().
//                  pPathInfo(IN/OUT) - count field should be set to NVAPI_MAX_DISPLAY_PATH. Can be NULL to retrieve just the pathCount.
//                  pPathCount(IN/OUT) - Number of elements in array pPathInfo->path.
//                  pTargetViewMode(OUT)- Display view selected from NV_TARGET_VIEW_MODE.
//
// RETURN STATUS:
//                  NVAPI_OK - completed request
//                  NVAPI_API_NOT_INTIALIZED - NVAPI not initialized
//                  NVAPI_ERROR - miscellaneous error occurred
//                  NVAPI_INVALID_ARGUMENT - Invalid input parameter.
//                  NVAPI_EXPECTED_DISPLAY_HANDLE - hNvDisplay is not a valid display handle.
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_GetViewEx(NvDisplayHandle hNvDisplay, NV_DISPLAY_PATH_INFO *pPathInfo, NvU32 *pPathCount, NV_TARGET_VIEW_MODE *pTargetViewMode);

///////////////////////////////////////////////////////////////////////////////
// FUNCTION NAME:   NvAPI_GetSupportedViews
//
// DESCRIPTION:     This API lets caller enumerate all the supported NVIDIA display views - nview and dualview modes.
//
//  SUPPORTED OS: Windows XP and higher
//
// PARAMETERS:      hNvDisplay(IN) - NVIDIA Display selection. It can be NVAPI_DEFAULT_HANDLE or a handle enumerated from NvAPI_EnumNVidiaDisplayHandle().
//                  pTargetViews(OUT) - Array of supported views. Can be NULL to retrieve the pViewCount first.
//                  pViewCount(IN/OUT) - Count of supported views.
//
// RETURN STATUS:
//                  NVAPI_OK - completed request
//                  NVAPI_ERROR - miscellaneous error occurred
//                  NVAPI_INVALID_ARGUMENT: Invalid input parameter.
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_GetSupportedViews(NvDisplayHandle hNvDisplay, NV_TARGET_VIEW_MODE *pTargetViews, NvU32 *pViewCount);



////////////////////////////////////////////////////////////////////////////////////////
//
// MOSAIC allows a multi display target output scanout on a single source.
//
// SAMPLE of MOSAIC 1x4 topo with 8 pixel horizontal overlap
//
//+-------------------------++-------------------------++-------------------------++-------------------------+
//|                         ||                         ||                         ||                         |
//|                         ||                         ||                         ||                         |
//|                         ||                         ||                         ||                         |
//|        DVI1             ||           DVI2          ||         DVI3            ||          DVI4           |
//|                         ||                         ||                         ||                         |
//|                         ||                         ||                         ||                         |
//|                         ||                         ||                         ||                         |
//|                         ||                         ||                         ||                         |
//+-------------------------++-------------------------++-------------------------++-------------------------+

#define NVAPI_MAX_MOSAIC_DISPLAY_ROWS       8
#define NVAPI_MAX_MOSAIC_DISPLAY_COLUMNS    8

#define NV_MOSAIC_MAX_DISPLAYS      (64)

//
// These bits are used to describe the validity of a topo.
//
#define NV_MOSAIC_TOPO_VALIDITY_VALID               0x00000000  // The topo is valid
#define NV_MOSAIC_TOPO_VALIDITY_MISSING_GPU         0x00000001  // Not enough SLI GPUs were found to fill the entire
                                                                // topo.  hPhysicalGPU will be 0 for these.
#define NV_MOSAIC_TOPO_VALIDITY_MISSING_DISPLAY     0x00000002  // Not enough displays were found to fill the entire
                                                                // topo.  displayOutputId will be 0 for these.
#define NV_MOSAIC_TOPO_VALIDITY_MIXED_DISPLAY_TYPES 0x00000004  // Topo is only possible with displays of the same
                                                                // NV_GPU_OUTPUT_TYPE.  Check displayOutputIds to make
                                                                // sure they are all CRT, or all DFP.


//
// This structure defines the details of a topo.
//
typedef struct
{
    NvU32                version;              // version of this structure
    NvLogicalGpuHandle   hLogicalGPU;          // logical gpu this topo is for
    NvU32                validityMask;         // 0 means topo is valid with current hardware.
                                               // If not 0, inspect bits against NV_MOSAIC_TOPO_VALIDITY_*.
    NvU32                rowCount;             // number of displays in a row
    NvU32                colCount;             // number of displays in a column

    struct
    {
        NvPhysicalGpuHandle hPhysicalGPU;      // physical gpu to be used in the topo (0 if GPU missing)
        NvU32               displayOutputId;   // connected display target (0 if no display connected)
        NvS32               overlapX;          // pixels of overlap on left of target: (+overlap, -gap)
        NvS32               overlapY;          // pixels of overlap on top of target: (+overlap, -gap)

    } gpuLayout[NVAPI_MAX_MOSAIC_DISPLAY_ROWS][NVAPI_MAX_MOSAIC_DISPLAY_COLUMNS];

} NV_MOSAIC_TOPO_DETAILS;

#define NVAPI_MOSAIC_TOPO_DETAILS_VER         MAKE_NVAPI_VERSION(NV_MOSAIC_TOPO_DETAILS,1)


//
// These values refer to the different types of Mosaic topos that are possible.  When
// getting the supported Mosaic topos, you can specify one of these types to narrow down
// the returned list to only those that match the given type.
//
typedef enum
{
    NV_MOSAIC_TOPO_TYPE_ALL,                          // All mosaic topos
    NV_MOSAIC_TOPO_TYPE_BASIC,                        // Basic Mosaic topos
    NV_MOSAIC_TOPO_TYPE_PASSIVE_STEREO,               // Passive Stereo topos
    NV_MOSAIC_TOPO_TYPE_SCALED_CLONE,                 // Not supported at this time
    NV_MOSAIC_TOPO_TYPE_PASSIVE_STEREO_SCALED_CLONE,  // Not supported at this time
    NV_MOSAIC_TOPO_TYPE_MAX,                          // Always leave this at end of enum
} NV_MOSAIC_TOPO_TYPE;


//
// The complete list of supported Mosaic topos.
//
// NOTE: common\inc\nvEscDef.h shadows a couple PASSIVE_STEREO enums.  If this
//       enum list changes and effects the value of NV_MOSAIC_TOPO_BEGIN_PASSIVE_STEREO
//       please update the corresponding value in nvEscDef.h
//
typedef enum
{
    NV_MOSAIC_TOPO_NONE,

    // 'BASIC' topos start here
    //
    // The result of using one of these Mosaic topos is that multiple monitors
    // will combine to create a single desktop.
    //
    NV_MOSAIC_TOPO_BEGIN_BASIC,
    NV_MOSAIC_TOPO_1x2_BASIC = NV_MOSAIC_TOPO_BEGIN_BASIC,
    NV_MOSAIC_TOPO_2x1_BASIC,
    NV_MOSAIC_TOPO_1x3_BASIC,
    NV_MOSAIC_TOPO_3x1_BASIC,
    NV_MOSAIC_TOPO_1x4_BASIC,
    NV_MOSAIC_TOPO_4x1_BASIC,
    NV_MOSAIC_TOPO_2x2_BASIC,
    NV_MOSAIC_TOPO_2x3_BASIC,
    NV_MOSAIC_TOPO_2x4_BASIC,
    NV_MOSAIC_TOPO_3x2_BASIC,
    NV_MOSAIC_TOPO_4x2_BASIC,
    NV_MOSAIC_TOPO_1x5_BASIC,
    NV_MOSAIC_TOPO_1x6_BASIC,
    NV_MOSAIC_TOPO_7x1_BASIC,

    // Add padding for 10 more entries. 6 will be enough room to specify every
    // possible topology with 8 or fewer displays, so this gives us a little
    // extra should we need it.
    NV_MOSAIC_TOPO_END_BASIC = NV_MOSAIC_TOPO_7x1_BASIC + 9,

    // 'PASSIVE_STEREO' topos start here
    //
    // The result of using one of these Mosaic topos is that multiple monitors
    // will combine to create a single PASSIVE STEREO desktop.  What this means is
    // that there will be two topos that combine to create the overall desktop.
    // One topo will be used for the left eye, and the other topo (of the
    // same rows x cols), will be used for the right eye.  The difference between
    // the two topos is that different GPUs and displays will be used.
    //
    NV_MOSAIC_TOPO_BEGIN_PASSIVE_STEREO,    // value shadowed in nvEscDef.h
    NV_MOSAIC_TOPO_1x2_PASSIVE_STEREO = NV_MOSAIC_TOPO_BEGIN_PASSIVE_STEREO,
    NV_MOSAIC_TOPO_2x1_PASSIVE_STEREO,
    NV_MOSAIC_TOPO_1x3_PASSIVE_STEREO,
    NV_MOSAIC_TOPO_3x1_PASSIVE_STEREO,
    NV_MOSAIC_TOPO_1x4_PASSIVE_STEREO,
    NV_MOSAIC_TOPO_4x1_PASSIVE_STEREO,
    NV_MOSAIC_TOPO_2x2_PASSIVE_STEREO,
    NV_MOSAIC_TOPO_END_PASSIVE_STEREO = NV_MOSAIC_TOPO_2x2_PASSIVE_STEREO + 4,

    //
    // Total number of topos.  Always leave this at the end of the enumeration.
    //
    NV_MOSAIC_TOPO_MAX

} NV_MOSAIC_TOPO;


//
// This is a topo brief structure.  It tells you what you need to know about
// a topo at a high level.  A list of these is returned when you query for the
// supported Mosaic information.
//
// If you need more detailed information about the topo, call
// NvAPI_Mosaic_GetTopoGroup() with the topo value from this structure.
//
typedef struct
{
    NvU32                        version;            // version of this structure
    NV_MOSAIC_TOPO               topo;               // the topo
    NvU32                        enabled;            // 1 if topo is enabled, else 0
    NvU32                        isPossible;         // 1 if topo *can* be enabled, else 0

} NV_MOSAIC_TOPO_BRIEF;

#define NVAPI_MOSAIC_TOPO_BRIEF_VER         MAKE_NVAPI_VERSION(NV_MOSAIC_TOPO_BRIEF,1)


//
// Basic per display settings that are used in setting/getting the Mosaic mode
//
typedef struct
{
    NvU32                        version;            // version of this structure
    NvU32                        width;              // per display width
    NvU32                        height;             // per display height
    NvU32                        bpp;                // bits per pixel
    NvU32                        freq;               // display frequency
} NV_MOSAIC_DISPLAY_SETTING;

#define NVAPI_MOSAIC_DISPLAY_SETTING_VER         MAKE_NVAPI_VERSION(NV_MOSAIC_DISPLAY_SETTING,1)


//
// Set a reasonable max number of display settings to support
// so arrays are bound.
//
#define NV_MOSAIC_DISPLAY_SETTINGS_MAX 40

//
// This structure is used to contain a list of supported Mosaic topos
// along with the display settings that can be used.
//
typedef struct
{
    NvU32                       version;                                         // version of this structure
    NvU32                       topoBriefsCount;                                 // number of topos in below array
    NV_MOSAIC_TOPO_BRIEF        topoBriefs[NV_MOSAIC_TOPO_MAX];                  // list of supported topos with only brief details
    NvU32                       displaySettingsCount;                            // number of display settings in below array
    NV_MOSAIC_DISPLAY_SETTING   displaySettings[NV_MOSAIC_DISPLAY_SETTINGS_MAX]; // list of per display settings possible

} NV_MOSAIC_SUPPORTED_TOPO_INFO;

#define NVAPI_MOSAIC_SUPPORTED_TOPO_INFO_VER         MAKE_NVAPI_VERSION(NV_MOSAIC_SUPPORTED_TOPO_INFO,1)


//
// Indexes to use to access the topos array within the mosaic topo
//
#define NV_MOSAIC_TOPO_IDX_DEFAULT       0

#define NV_MOSAIC_TOPO_IDX_LEFT_EYE      0
#define NV_MOSAIC_TOPO_IDX_RIGHT_EYE     1
#define NV_MOSAIC_TOPO_NUM_EYES          2


//
// This defines the maximum number of topos that can be in a topo group.
// At this time, it is set to 2 because our largest topo group (passive
// stereo) only needs 2 topos (left eye and right eye).
//
// If a new topo group with more than 2 topos is added above, then this
// number will also have to be incremented.
//
#define NV_MOSAIC_MAX_TOPO_PER_TOPO_GROUP 2


//
// This structure defines a group of topos that work together to create one
// overall layout.  All of the supported topos are represented with this
// structure.
//
// For example, a 'Passive Stereo' topo would be represented with this
// structure, and would have separate topo details for the left and right eyes.
// The count would be 2.  A 'Basic' topo is also represented by this structure,
// with a count of 1.
//
// The structure is primarily used internally, but is exposed to applications in a
// read only fashion because there are some details in it that might be useful
// (like the number of rows/cols, or connected display information).  A user can
// get the filled in structure by calling NvAPI_Mosaic_GetTopoGroup().
//
// You can then look at the detailed values within the structure.  There are no
// entrypoints which take this structure as input (effectively making it read only).
//
typedef struct
{
    NvU32                      version;              // version of this structure
    NV_MOSAIC_TOPO_BRIEF       brief;                // the brief details of this topo
    NvU32                      count;                // number of topos in array below
    NV_MOSAIC_TOPO_DETAILS     topos[NV_MOSAIC_MAX_TOPO_PER_TOPO_GROUP];

} NV_MOSAIC_TOPO_GROUP;

#define NVAPI_MOSAIC_TOPO_GROUP_VER         MAKE_NVAPI_VERSION(NV_MOSAIC_TOPO_GROUP,1)

//  SUPPORTED OS: Windows XP and higher
///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME:   NvAPI_Mosaic_GetSupportedTopoInfo
//
// DESCRIPTION:     This API returns information on the topos and display resolutions
//                  supported by Mosaic.
//
//                  NOTE: Not all topos returned can be immediately set.
//                        See 'OUT' Notes below.
//
//                  Once you get the list of supported topos, you can call
//                  NvAPI_Mosaic_GetTopoGroup() with a Mosaic topo if you need
//                  more information about that topo.
//
// PARAMETERS:      pSupportedTopoInfo(IN/OUT):  Information about what topos and display resolutions
//                                               are supported for Mosaic.
//                  type(IN):                    The type of topos the caller is interested in
//                                               getting.  See NV_MOSAIC_TOPO_TYPE for possible
//                                               values.
//
//     'IN' Notes:  pSupportedTopoInfo->version must be set before calling this function.
//                  If the specified version is not supported by this implementation,
//                  an error will be returned (NVAPI_INCOMPATIBLE_STRUCT_VERSION).
//
//     'OUT' Notes: Some of the topos returned might not be valid for one reason or
//                  another.  It could be due to mismatched or missing displays.  It
//                  could also be because the required number of GPUs is not found.
//                  At a high level, you can see if the topo is valid and can be enabled
//                  by looking at the pSupportedTopoInfo->topoBriefs[xxx].isPossible flag.
//                  If this is true, the topo can be enabled.  Otherwise, if it
//                  is false, you can find out why it cannot be enabled by getting the
//                  details of the topo via NvAPI_Mosaic_GetTopoGroup().  From there,
//                  look at the validityMask of the individual topos.  The bits can
//                  be tested against the NV_MOSAIC_TOPO_VALIDITY_* bits.
//
//                  It is possible for this function to return NVAPI_OK with no topos
//                  listed in the return structure.  If this is the case, it means that
//                  the current hardware DOES support Mosaic, but with the given configuration
//                  no valid topos were found.  This most likely means that SLI was not
//                  enabled for the hardware.  Once enabled, you should see valid topos
//                  returned from this function.
//
// RETURN STATUS    NVAPI_OK:                          No errors in returning supported topos
//                  NVAPI_NOT_SUPPORTED:               Mosaic is not supported with the existing hardware
//                  NVAPI_INVALID_ARGUMENT:            One or more args passed in are invalid
//                  NVAPI_API_NOT_INTIALIZED:          The NvAPI API needs to be initialized first
//                  NVAPI_NO_IMPLEMENTATION:           This entrypoint not available
//                  NVAPI_INCOMPATIBLE_STRUCT_VERSION: The version of the structure passed in is not
//                                                     compatible with this entrypoint
//                  NVAPI_ERROR:                       Miscellaneous error occurred
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_Mosaic_GetSupportedTopoInfo(NV_MOSAIC_SUPPORTED_TOPO_INFO *pSupportedTopoInfo, NV_MOSAIC_TOPO_TYPE type);

//  SUPPORTED OS: Windows XP and higher
///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME:   NvAPI_Mosaic_GetTopoGroup
//
// DESCRIPTION:     This API returns a structure filled with the topo details
//                  for the given Mosaic topo.
//
//                  If the pTopoBrief passed in matches the topo which is
//                  current, then information in the brief and group structures
//                  will reflect what is current.  Thus the brief would have
//                  the current 'enable' status, and the group would have the
//                  current overlap values.  If there is no match, then the
//                  returned brief has an 'enable' status of FALSE (since it
//                  is obviously not enabled), and the overlap values will be 0.
//
// PARAMETERS:      pTopoBrief(IN):         The topo to get details for.
//                                          This must be one of the topo briefs
//                                          returned from NvAPI_Mosaic_GetSupportedTopoInfo().
//                  pTopoGroup(IN/OUT):     The topo details matching the brief.
//
//     'IN' Notes:  pTopoGroup->version must be set before calling this function.
//                  If the specified version is not supported by this implementation,
//                  an error will be returned (NVAPI_INCOMPATIBLE_STRUCT_VERSION).
//
// RETURN STATUS    NVAPI_OK:                          Details were retrieved successfully
//                  NVAPI_NOT_SUPPORTED:               Mosaic is not supported with the existing hardware
//                  NVAPI_INVALID_ARGUMENT:            One or more args passed in are invalid.
//                  NVAPI_API_NOT_INTIALIZED:          The NvAPI API needs to be initialized first
//                  NVAPI_NO_IMPLEMENTATION:           This entrypoint not available
//                  NVAPI_INCOMPATIBLE_STRUCT_VERSION: The version of the structure passed in is not
//                                                     compatible with this entrypoint
//                  NVAPI_ERROR:                       Miscellaneous error occurred
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_Mosaic_GetTopoGroup(NV_MOSAIC_TOPO_BRIEF *pTopoBrief, NV_MOSAIC_TOPO_GROUP *pTopoGroup);

//  SUPPORTED OS: Windows XP and higher
///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME:   NvAPI_Mosaic_GetOverlapLimits
//
// DESCRIPTION:     This API returns the X and Y overlap limits required if
//                  the given Mosaic topo and display settings are to be used.
//
// PARAMETERS:      pTopoBrief(IN):         The topo to get limits for.
//                                          This must be one of the topo briefs
//                                          returned from NvAPI_Mosaic_GetSupportedTopoInfo().
//                  pDisplaySetting(IN):    The display settings to get limits for.
//                                          This must be one of the settings
//                                          returned from NvAPI_Mosaic_GetSupportedTopoInfo().
//                  pMinOverlapX(OUT):      X overlap minimum
//                  pMaxOverlapX(OUT):      X overlap maximum
//                  pMinOverlapY(OUT):      Y overlap minimum
//                  pMaxOverlapY(OUT):      Y overlap maximum
//
//
//
// RETURN STATUS    NVAPI_OK:                          Details were retrieved successfully
//                  NVAPI_NOT_SUPPORTED:               Mosaic is not supported with the existing hardware
//                  NVAPI_INVALID_ARGUMENT:            One or more args passed in are invalid.
//                  NVAPI_API_NOT_INTIALIZED:          The NvAPI API needs to be initialized first
//                  NVAPI_NO_IMPLEMENTATION:           This entrypoint not available
//                  NVAPI_INCOMPATIBLE_STRUCT_VERSION: The version of the structure passed in is not
//                                                     compatible with this entrypoint
//                  NVAPI_ERROR:                       Miscellaneous error occurred
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_Mosaic_GetOverlapLimits(NV_MOSAIC_TOPO_BRIEF *pTopoBrief, NV_MOSAIC_DISPLAY_SETTING *pDisplaySetting, NvS32 *pMinOverlapX, NvS32 *pMaxOverlapX, NvS32 *pMinOverlapY, NvS32 *pMaxOverlapY);

//  SUPPORTED OS: Windows XP and higher
///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME:   NvAPI_Mosaic_SetCurrentTopo
//
// DESCRIPTION:     This API sets the Mosaic topo and does a mode change
//                  using the given display settings.
//
//                  If NVAPI_OK is returned, the current Mosaic topo was set
//                  correctly.  Any other status returned means the
//                  topo was not set, and remains what it was before this
//                  function was called.
//
//
//
// PARAMETERS:      pTopoBrief(IN):       The topo to set.
//                                        This must be one of the topos
//                                        returned from NvAPI_Mosaic_GetSupportedTopoInfo(),
//                                        and it must have an isPossible value of 1.
//                  pDisplaySetting(IN):  The per display settings to be used in the
//                                        setting of Mosaic mode.
//                                        This must be one of the settings
//                                        returned from NvAPI_Mosaic_GetSupportedTopoInfo().
//                  overlapX(IN):         The pixel overlap to use between horizontal
//                                        displays (use positive a number for overlap,
//                                        or a negative number to create a gap.)
//                                        If the overlap is out of bounds for what is
//                                        possible given the topo and display setting,
//                                        the overlap will be clamped.
//                  overlapY(IN):         The pixel overlap to use between vertical
//                                        displays (use positive a number for overlap,
//                                        or a negative number to create a gap.)
//                                        If the overlap is out of bounds for what is
//                                        possible given the topo and display setting,
//                                        the overlap will be clamped.
//                  enable(IN):           If 1, the topo being set will also be enabled,
//                                        meaning that the mode set will occur.
//                                        Passing a 0 means you don't want to be in
//                                        Mosaic mode right now, but want to set the current
//                                        Mosaic topo so you can enable it later with
//                                        NvAPI_Mosaic_EnableCurrentTopo().
//
// RETURN STATUS    NVAPI_OK:                          Mosaic topo was set
//                  NVAPI_NOT_SUPPORTED:               Mosaic is not supported with the existing hardware
//                  NVAPI_INVALID_ARGUMENT:            One or more args passed in are invalid.
//                  NVAPI_TOPO_NOT_POSSIBLE:           The topo passed in is not currently possible
//                  NVAPI_API_NOT_INTIALIZED:          The NvAPI API needs to be initialized first
//                  NVAPI_NO_IMPLEMENTATION:           This entrypoint not available
//                  NVAPI_INCOMPATIBLE_STRUCT_VERSION: The version of the structure passed in is not
//                                                     compatible with this entrypoint
//                  NVAPI_MODE_CHANGE_FAILED:          There was an error changing the display mode
//                  NVAPI_ERROR:                       Miscellaneous error occurred
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_Mosaic_SetCurrentTopo(NV_MOSAIC_TOPO_BRIEF *pTopoBrief, NV_MOSAIC_DISPLAY_SETTING *pDisplaySetting, NvS32 overlapX, NvS32 overlapY, NvU32 enable);

//  SUPPORTED OS: Windows XP and higher
///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME:   NvAPI_Mosaic_GetCurrentTopo
//
// DESCRIPTION:     This API returns information for the current Mosaic topo.
//                  This includes topo, display settings, and overlap values.
//
//                  You can call NvAPI_Mosaic_GetTopoGroup() with the topo
//                  if you require more information on the topo.
//
//                  If there isn't a current topo, then pTopoBrief->topo will
//                  be NV_MOSAIC_TOPO_NONE.
//
// PARAMETERS:      pTopoBrief(OUT):      The current Mosaic topo
//                  pDisplaySetting(OUT): The current per display settings
//                  pOverlapX(OUT):       The pixel overlap between horizontal displays
//                  pOverlapY(OUT):       The pixel overlap between vertical displays
//
//
// RETURN STATUS    NVAPI_OK:                          Success getting current info
//                  NVAPI_NOT_SUPPORTED:               Mosaic is not supported with the existing hardware
//                  NVAPI_INVALID_ARGUMENT:            One or more args passed in are invalid
//                  NVAPI_API_NOT_INTIALIZED:          The NvAPI API needs to be initialized first
//                  NVAPI_NO_IMPLEMENTATION:           This entrypoint not available
//                  NVAPI_ERROR:                       Miscellaneous error occurred
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_Mosaic_GetCurrentTopo(NV_MOSAIC_TOPO_BRIEF *pTopoBrief, NV_MOSAIC_DISPLAY_SETTING *pDisplaySetting, NvS32 *pOverlapX, NvS32 *pOverlapY);

//  SUPPORTED OS: Windows XP and higher
///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME:   NvAPI_Mosaic_EnableCurrentTopo
//
// DESCRIPTION:     This API enables or disables the current Mosaic topo
//                  based on the setting of the incoming 'enable' parameter.
//
//                  When enabling, this will enable the current Mosaic topo
//                  that was previously set.  Note that when the current Mosaic
//                  topo is retrieved, it must have an isPossible value of 1 or
//                  an error will occur.
//
//                  When disabling, the current Mosaic topo is disabled.
//                  The topo information will persist, even across reboots.
//                  To re-enable the Mosaic topo, simply call this function
//                  again with the enable parameter set to 1.
//
// PARAMETERS:      enable(IN):               1 to enable the current Mosaic topo, 0 to disable it.
//
//
// RETURN STATUS    NVAPI_OK:                 The Mosaic topo was enabled/disabled
//                  NVAPI_NOT_SUPPORTED:      Mosaic is not supported with the existing hardware
//                  NVAPI_INVALID_ARGUMENT:   One or more args passed in are invalid
//                  NVAPI_TOPO_NOT_POSSIBLE:  The current topo is not currently possible
//                  NVAPI_MODE_CHANGE_FAILED: There was an error changing the display mode
//                  NVAPI_ERROR:              Miscellaneous error occurred
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_Mosaic_EnableCurrentTopo(NvU32 enable);


//  SUPPORTED OS: Windows Vista and higher
///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME:   NvAPI_Mosaic_GetDisplayViewportsByResolution
//
// DESCRIPTION:     This API returns the viewports which would be applied on
//                  the requested display.
//
// PARAMETERS:      displayId(IN):    Display ID of a single display in the active
//                                    mosaic topology to query.
//                  srcWidth(IN):     Width of full display topology. If both
//                                    width and height are 0, the current
//                                    resolution is used.
//                  srcHeight(IN):    Height of full display topology. If both
//                                    width and height are 0, the current
//                                    resolution is used.
//                  viewports (OUT):  Array of NV_RECT viewports which represent
//                                    the displays as identified in
//                                    NvAPI_Mosaic_EnumGridTopologies. If the
//                                    requested resolution is a single-wide
//                                    resolution, only viewports[0] will
//                                    contain the viewport details, regardless
//                                    of which display is driving the display.
//                  bezelCorrected(OUT):  Returns 1 if the requested resolution is
//                                    bezel corrected. May be NULL.
//
// RETURN STATUS    NVAPI_OK:                          Capabilties have been returned.
//                  NVAPI_INVALID_ARGUMENT:            One or more args passed in are invalid.
//                  NVAPI_API_NOT_INTIALIZED:          The NvAPI API needs to be initialized first
//                  NVAPI_MOSAIC_NOT_ACTIVE:           The display does not belong to an active Mosaic Topology
//                  NVAPI_NO_IMPLEMENTATION:           This entrypoint not available
//                  NVAPI_ERROR:                       Miscellaneous error occurred
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_Mosaic_GetDisplayViewportsByResolution(NvU32 displayId, NvU32 srcWidth, NvU32 srcHeight, NV_RECT viewports[NV_MOSAIC_MAX_DISPLAYS], NvU8* bezelCorrected);

////////////////////////////////////////////////////////////////////////////////////////
//
// ###########################################################################
// DELME_RUSS - DELME_RUSS - DELME_RUSS - DELME_RUSS - DELME_RUSS - DELME_RUSS
//
//   Below is the Phase 1 Mosaic stuff, the Phase 2 stuff above is what will remain
//   once Phase 2 is complete.  For a small amount of time, the two will co-exist.  As
//   soon as apps (nvapichk, NvAPITestMosaic, and CPL) are updated to use the Phase 2
//   entrypoints, the code below will be deleted.
//
// DELME_RUSS - DELME_RUSS - DELME_RUSS - DELME_RUSS - DELME_RUSS - DELME_RUSS
// ###########################################################################
//
//
// Supported topos 1x4, 4x1 and 2x2 to start with.
//
// Selected scanout targets can be one per GPU or more than one on the same GPU.
//
// SAMPLE of MOSAIC 1x4 SCAN OUT TOPO with 8 pixel horizontal overlap
//
//+-------------------------++-------------------------++-------------------------++-------------------------+
//|                         ||                         ||                         ||                         |
//|                         ||                         ||                         ||                         |
//|                         ||                         ||                         ||                         |
//|        DVI1             ||           DVI2          ||         DVI3            ||          DVI4           |
//|                         ||                         ||                         ||                         |
//|                         ||                         ||                         ||                         |
//|                         ||                         ||                         ||                         |
//|                         ||                         ||                         ||                         |
//+-------------------------++-------------------------++-------------------------++-------------------------+

#define NVAPI_MAX_MOSAIC_DISPLAY_ROWS       8
#define NVAPI_MAX_MOSAIC_DISPLAY_COLUMNS    8
#define NVAPI_MAX_MOSAIC_TOPOS              16

typedef struct
{
    NvU32 version;                             // version number of mosaic topology
    NvU32 rowCount;                            // horizontal display count
    NvU32 colCount;                            // vertical display count

    struct
    {
        NvPhysicalGpuHandle hPhysicalGPU;      // physical gpu to be used in the topology
        NvU32               displayOutputId;   // connected display target
        NvS32               overlapX;          // pixels of overlap on left of target: (+overlap, -gap)
        NvS32               overlapY;          // pixels of overlap on top of target: (+overlap, -gap)

    } gpuLayout[NVAPI_MAX_MOSAIC_DISPLAY_ROWS][NVAPI_MAX_MOSAIC_DISPLAY_COLUMNS];

} NV_MOSAIC_TOPOLOGY;

#define NVAPI_MOSAIC_TOPOLOGY_VER         MAKE_NVAPI_VERSION(NV_MOSAIC_TOPOLOGY,1)

typedef struct
{
    NvU32                   version;
    NvU32                   totalCount;                     //count of valid topologies
    NV_MOSAIC_TOPOLOGY      topos[NVAPI_MAX_MOSAIC_TOPOS];  //max topologies

} NV_MOSAIC_SUPPORTED_TOPOLOGIES;

#define NVAPI_MOSAIC_SUPPORTED_TOPOLOGIES_VER         MAKE_NVAPI_VERSION(NV_MOSAIC_SUPPORTED_TOPOLOGIES,1)


///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME:   NvAPI_GetSupportedMosaicTopologies
//
// DESCRIPTION:     This API returns all valid Mosaic topologies
//
//  SUPPORTED OS: Windows XP
//
// PARAMETERS:      pMosaicTopos(OUT):  An array of valid Mosaic topologies.
//
// RETURN STATUS    NVAPI_OK:                      Call succeeded; 1 or more topologies were returned
//                  NVAPI_INVALID_ARGUMENT:        one or more args are invalid
//                  NVAPI_MIXED_TARGET_TYPES:      Mosaic topology is only possible with all targets of the same NV_GPU_OUTPUT_TYPE.
//                  NVAPI_NVIDIA_DEVICE_NOT_FOUND: no NVIDIA GPU driving a display was found
//                  NVAPI_NOT_SUPPORTED:           Mosaic is not supported with GPUs on this system.
//                  NVAPI_NO_ACTIVE_SLI_TOPOLOGY:  SLI is not enabled, yet needs to be, in order for this function to succeed.
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_GetSupportedMosaicTopologies(NV_MOSAIC_SUPPORTED_TOPOLOGIES *pMosaicTopos);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME:   NvAPI_GetCurrentMosaicTopology
//
// DESCRIPTION:     This API gets the current Mosaic topology
//
//  SUPPORTED OS: Windows XP
//
// PARAMETERS:      pMosaicTopo(OUT):  The current Mosaic topology
//                  pEnabled(OUT):     TRUE if returned topology is currently enabled, else FALSE
//
// RETURN STATUS    NVAPI_OK:                       Call succeeded.
//                  NVAPI_INVALID_ARGUMENT:         one or more args are invalid
//                  NVAPI_NVIDIA_DEVICE_NOT_FOUND:  no NVIDIA GPU driving a display was found
//                  NVAPI_NOT_SUPPORTED:            Mosaic is not supported with GPUs on this system.
//                  NVAPI_NO_ACTIVE_SLI_TOPOLOGY:   SLI is not enabled, yet needs to be, in order for this function to succeed.
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_GetCurrentMosaicTopology(NV_MOSAIC_TOPOLOGY *pMosaicTopo, NvU32 *pEnabled);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME:   NvAPI_SetCurrentMosaicTopology
//
// DESCRIPTION:     This API sets the Mosaic topology, and will enable it so the
//                  Mosaic display settings will be enumerated upon request.
//
//  SUPPORTED OS: Windows XP
//
// PARAMETERS:      pMosaicTopo(IN):  A valid Mosaic topology
//
// RETURN STATUS    NVAPI_OK:                      Call succeeded
//                  NVAPI_INVALID_ARGUMENT:        One or more args are invalid
//                  NVAPI_NVIDIA_DEVICE_NOT_FOUND: No NVIDIA GPU driving a display was found
//                  NVAPI_NOT_SUPPORTED:           Mosaic mode could not be set
//                  NVAPI_NO_ACTIVE_SLI_TOPOLOGY:  SLI is not enabled, yet needs to be, in order for this function to succeed.
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_SetCurrentMosaicTopology(NV_MOSAIC_TOPOLOGY *pMosaicTopo);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME:   NvAPI_EnableCurrentMosaicTopology
//
// DESCRIPTION:     This API enables or disables the current Mosaic topology.
//                  When enabling, this will use the last Mosaic topology that was set.
//                  If enabled, enumeration of display settings will include valid
//                  Mosaic resolutions.  If disabled, enumeration of display settings
//                  will not include Mosaic resolutions.
//
//  SUPPORTED OS: Windows XP
//
// PARAMETERS:      enable(IN):  TRUE to enable the Mosaic Topology, FALSE to disable it.
//
// RETURN STATUS    NVAPI_OK:                      Call succeeded
//                  NVAPI_INVALID_ARGUMENT:        One or more args are invalid
//                  NVAPI_NVIDIA_DEVICE_NOT_FOUND: No NVIDIA GPU driving a display was found
//                  NVAPI_NOT_SUPPORTED:           Mosaic mode could not be enabled/disabled
//                  NVAPI_NO_ACTIVE_SLI_TOPOLOGY:  SLI is not enabled, yet needs to be, in order for this function to succeed.
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_EnableCurrentMosaicTopology(NvU32 enable);


///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_GPU_GetHDCPSupportStatus
//
// DESCRIPTION: Returns information on a GPU's HDCP support status
//
//  SUPPORTED OS: Mac OS X, Windows XP and higher
//
// RETURN STATUS:
//    NVAPI_OK - completed request
//    NVAPI_ERROR - miscellaneous error occurred
//    NVAPI_INVALID_ARGUMENT - pGetGpuHdcpSupportStatus is NULL
//    NVAPI_HANDLE_INVALIDATED - handle passed has been invalidated (see user guide)
//    NVAPI_EXPECTED_PHYSICAL_GPU_HANDLE - handle passed is not a physical GPU handle
//    NVAPI_INCOMPATIBLE_STRUCT_VERSION - the version of the INFO struct is not supported
//
///////////////////////////////////////////////////////////////////////////////

typedef enum _NV_GPU_HDCP_FUSE_STATE
{
    NV_GPU_HDCP_FUSE_STATE_UNKNOWN  = 0,
    NV_GPU_HDCP_FUSE_STATE_DISABLED = 1,
    NV_GPU_HDCP_FUSE_STATE_ENABLED  = 2,
} NV_GPU_HDCP_FUSE_STATE;

typedef enum _NV_GPU_HDCP_KEY_SOURCE
{
    NV_GPU_HDCP_KEY_SOURCE_UNKNOWN    = 0,
    NV_GPU_HDCP_KEY_SOURCE_NONE       = 1,
    NV_GPU_HDCP_KEY_SOURCE_CRYPTO_ROM = 2,
    NV_GPU_HDCP_KEY_SOURCE_SBIOS      = 3,
    NV_GPU_HDCP_KEY_SOURCE_I2C_ROM    = 4,
    NV_GPU_HDCP_KEY_SOURCE_FUSES      = 5,
} NV_GPU_HDCP_KEY_SOURCE;

typedef enum _NV_GPU_HDCP_KEY_SOURCE_STATE
{
    NV_GPU_HDCP_KEY_SOURCE_STATE_UNKNOWN = 0,
    NV_GPU_HDCP_KEY_SOURCE_STATE_ABSENT  = 1,
    NV_GPU_HDCP_KEY_SOURCE_STATE_PRESENT = 2,
} NV_GPU_HDCP_KEY_SOURCE_STATE;

typedef struct
{
    NvU32                        version;               // Structure version
    NV_GPU_HDCP_FUSE_STATE       hdcpFuseState;         // GPU's HDCP fuse state
    NV_GPU_HDCP_KEY_SOURCE       hdcpKeySource;         // GPU's HDCP key source
    NV_GPU_HDCP_KEY_SOURCE_STATE hdcpKeySourceState;    // GPU's HDCP key source state
} NV_GPU_GET_HDCP_SUPPORT_STATUS;

#define NV_GPU_GET_HDCP_SUPPORT_STATUS_VER MAKE_NVAPI_VERSION(NV_GPU_GET_HDCP_SUPPORT_STATUS,1)

NVAPI_INTERFACE NvAPI_GPU_GetHDCPSupportStatus(NvPhysicalGpuHandle hPhysicalGpu, NV_GPU_GET_HDCP_SUPPORT_STATUS *pGetHDCPSupportStatus);


///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_Stereo_CreateConfigurationProfileRegistryKey
//
//  SUPPORTED OS: Windows Vista and higher
//
// DESCRIPTION:   Creates new configuration registry key for current application.
//
//                If there was no configuration profile prior to the function call,
//                tries to create brand new configuration profile registry key
//                for a given application and fill it with default values.
//                If an application already had a configuration profile registry key, does nothing.
//                Name of the key is automatically determined as the name of the executable that calls this function.
//                Because of this, application executable should have distinct and unique name.
//                If the application is using only one version of DirectX, than the default profile type will be appropriate.
//                If the application is using more than one version of DirectX from same executable,
//                it should use appropriate profile type for each configuration profile.
//
// PARAMETERS:    registryProfileType(IN) - Type of profile that application wants to create.
//                                          Should be one of the symbolic constants defined in NV_STEREO_REGISTRY_PROFILE_TYPE.
//                                          Any other value will cause function to do nothing and return NV_STEREO_REGISTRY_PROFILE_TYPE_NOT_SUPPORTED.
//
// HOW TO USE:    When there is a need for an application to have default stereo parameter values,
//                use this function to create a key where they will be stored.
//
// RETURN STATUS:
//                NVAPI_OK - Key exists in the registry.
//                NVAPI_STEREO_REGISTRY_PROFILE_TYPE_NOT_SUPPORTED - This profile type is not supported.
//                NVAPI_STEREO_REGISTRY_ACCESS_FAILED - Access to registry failed.
//                NVAPI_API_NOT_INTIALIZED - NVAPI not initialized.
//                NVAPI_STEREO_NOT_INITIALIZED - Stereo part of NVAPI not initialized.
//                NVAPI_ERROR - Something is wrong (generic error).
//
///////////////////////////////////////////////////////////////////////////////

typedef enum _NV_StereoRegistryProfileType
{
    NVAPI_STEREO_DEFAULT_REGISTRY_PROFILE, // Default registry configuration profile.
    NVAPI_STEREO_DX9_REGISTRY_PROFILE,     // Separate registry configuration profile for DX9 executable.
    NVAPI_STEREO_DX10_REGISTRY_PROFILE     // Separate registry configuration profile for DX10 executable.
} NV_STEREO_REGISTRY_PROFILE_TYPE;

NVAPI_INTERFACE NvAPI_Stereo_CreateConfigurationProfileRegistryKey(NV_STEREO_REGISTRY_PROFILE_TYPE registryProfileType);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_Stereo_DeleteConfigurationProfileRegistryKey
//
//  SUPPORTED OS: Windows Vista and higher
//
// DESCRIPTION:   Removes configuration registry key for current application.
//
//                If an application already had a configuration profile prior to the function call,
//                this function will try to remove application's configuration profile registry key from the registry.
//                If there was no configuration profile registry key prior to the function call,
//                the function will do nothing and will not report an error.
//
// PARAMETERS:    registryProfileType(IN) - Type of profile that application wants to delete.
//                                          Should be one of the symbolic constants defined in NV_STEREO_REGISTRY_PROFILE_TYPE.
//                                          Any other value will cause function to do nothing and return NV_STEREO_REGISTRY_PROFILE_TYPE_NOT_SUPPORTED.
//
// RETURN STATUS:
//                NVAPI_OK - Key does not exist in the registry any more.
//                NVAPI_STEREO_REGISTRY_PROFILE_TYPE_NOT_SUPPORTED - This profile type is not supported.
//                NVAPI_STEREO_REGISTRY_ACCESS_FAILED - Access to registry failed.
//                NVAPI_API_NOT_INTIALIZED - NVAPI not initialized.
//                NVAPI_STEREO_NOT_INITIALIZED - Stereo part of NVAPI not initialized.
//                NVAPI_ERROR - Something is wrong (generic error).
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_Stereo_DeleteConfigurationProfileRegistryKey(NV_STEREO_REGISTRY_PROFILE_TYPE registryProfileType);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_Stereo_SetConfigurationProfileValue
//
// PARAMETERS:    registryProfileType(IN) - Type of profile that application wants to access.
//                                          Should be one of the symbolic constants defined in NV_STEREO_REGISTRY_PROFILE_TYPE.
//                                          Any other value will cause function to do nothing and return NV_STEREO_REGISTRY_PROFILE_TYPE_NOT_SUPPORTED.
//                valueRegistryID(IN)     - ID of the value that is being set.
//                                          Should be one of the symbolic constants defined in NV_STEREO_REGISTRY_PROFILE_TYPE.
//                                          Any other value will cause function to do nothing and return NVAPI_STEREO_REGISTRY_VALUE_NOT_SUPPORTED.
//                pValue(IN)              - Address of the value that is being set.
//                                          Should be either address of a DWORD or of a float,
//                                          dependent on the type of the stereo parameter whose value is being set.
//                                          The API will then cast that address to DWORD*
//                                          and write whatever is in those 4 bytes as a DWORD to the registry.
//
//  SUPPORTED OS: Windows Vista and higher
//
// DESCRIPTION:   Sets given parameter value under the application's registry key.
//
//                If the value does not exist under the application's registry key,
//                the value will be created under the key.
//
// RETURN STATUS:
//                NVAPI_OK - Value is written to registry.
//                NVAPI_STEREO_REGISTRY_PROFILE_TYPE_NOT_SUPPORTED - This profile type is not supported.
//                NVAPI_STEREO_REGISTRY_VALUE_NOT_SUPPORTED - This value is not supported.
//                NVAPI_STEREO_REGISTRY_ACCESS_FAILED - Access to registry failed.
//                NVAPI_API_NOT_INTIALIZED - NVAPI not initialized.
//                NVAPI_STEREO_NOT_INITIALIZED - Stereo part of NVAPI not initialized.
//                NVAPI_ERROR - Something is wrong (generic error).
//
///////////////////////////////////////////////////////////////////////////////

typedef enum _NV_StereoRegistryID
{
    NVAPI_CONVERGENCE_ID,         // Symbolic constant for convergence registry ID.
    NVAPI_FRUSTUM_ADJUST_MODE_ID, // Symbolic constant for frustum adjust mode registry ID.
} NV_STEREO_REGISTRY_ID;

NVAPI_INTERFACE NvAPI_Stereo_SetConfigurationProfileValue(NV_STEREO_REGISTRY_PROFILE_TYPE registryProfileType, NV_STEREO_REGISTRY_ID valueRegistryID, void *pValue);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_Stereo_DeleteConfigurationProfileValue
//
// PARAMETERS:    registryProfileType(IN) - Type of profile that application wants to access.
//                                          Should be one of the symbolic constants defined in NV_STEREO_REGISTRY_PROFILE_TYPE.
//                                          Any other value will cause function to do nothing and return NV_STEREO_REGISTRY_PROFILE_TYPE_NOT_SUPPORTED.
//                valueRegistryID(IN)     - ID of the value that is being deleted.
//                                          Should be one of the symbolic constants defined in NV_STEREO_REGISTRY_PROFILE_TYPE.
//                                          Any other value will cause function to do nothing and return NVAPI_STEREO_REGISTRY_VALUE_NOT_SUPPORTED.
//
//  SUPPORTED OS: Windows Vista and higher
//
// DESCRIPTION:   Removes given value from application's configuration profile registry key.
//
//                If there is no such value, the function will do nothing and will not report an error.
//
// RETURN STATUS:
//                NVAPI_OK - Value does not exist in registry any more.
//                NVAPI_STEREO_REGISTRY_PROFILE_TYPE_NOT_SUPPORTED - This profile type is not supported.
//                NVAPI_STEREO_REGISTRY_VALUE_NOT_SUPPORTED - This value is not supported.
//                NVAPI_STEREO_REGISTRY_ACCESS_FAILED - Access to registry failed.
//                NVAPI_API_NOT_INTIALIZED - NVAPI not initialized.
//                NVAPI_STEREO_NOT_INITIALIZED - Stereo part of NVAPI not initialized.
//                NVAPI_ERROR - Something is wrong (generic error).
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_Stereo_DeleteConfigurationProfileValue(NV_STEREO_REGISTRY_PROFILE_TYPE registryProfileType, NV_STEREO_REGISTRY_ID valueRegistryID);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_Stereo_Enable
//
//  SUPPORTED OS: Windows Vista and higher
//
// DESCRIPTION:   Enables stereo mode in the registry.
//                Call to this function affects entire system.
//                Calls to functions that require stereo enabled with stereo disabled will have no effect,
//                and will return apropriate error code.
//
// RETURN STATUS:
//                NVAPI_OK - Stereo is now enabled.
//                NVAPI_API_NOT_INTIALIZED - NVAPI not initialized.
//                NVAPI_STEREO_NOT_INITIALIZED - Stereo part of NVAPI not initialized.
//                NVAPI_ERROR - Something is wrong (generic error).
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_Stereo_Enable(void);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_Stereo_Disable
//
//  SUPPORTED OS: Windows Vista and higher
//
// DESCRIPTION:   Disables stereo mode in the registry.
//                Call to this function affects entire system.
//                Calls to functions that require stereo enabled with stereo disabled will have no effect,
//                and will return apropriate error code.
//
// RETURN STATUS:
//                NVAPI_OK - Stereo is now disabled.
//                NVAPI_API_NOT_INTIALIZED - NVAPI not initialized.
//                NVAPI_STEREO_NOT_INITIALIZED - Stereo part of NVAPI not initialized.
//                NVAPI_ERROR - Something is wrong (generic error).
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_Stereo_Disable(void);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_Stereo_IsEnabled
//
// PARAMETERS:    pIsStereoEnabled(OUT)  - Address where result of the inquiry will be placed.
//
//  SUPPORTED OS: Windows Vista and higher
//
// DESCRIPTION:   Checks if stereo mode is enabled in the registry.
//
// RETURN STATUS:
//                NVAPI_OK - Check was sucessfully completed and result reflects current state of stereo availability.
//                NVAPI_API_NOT_INTIALIZED - NVAPI not initialized.
//                NVAPI_STEREO_NOT_INITIALIZED - Stereo part of NVAPI not initialized.
//                NVAPI_ERROR - Something is wrong (generic error).
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_Stereo_IsEnabled(NvU8 *pIsStereoEnabled);



#if defined(_D3D9_H_) || defined(__d3d10_h__) || defined(__d3d11_h__)
///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_Stereo_CreateHandleFromIUnknown
//
// PARAMETERS:    pDevice(IN) - Pointer to IUnknown interface that is IDirect3DDevice9*, ID3D10Device* or ID3D11Device.
//                pStereoHandle(OUT) - Pointer to newly created stereo handle.
//
//  SUPPORTED OS: Windows Vista and higher
//
// DESCRIPTION:   Creates stereo handle, that is used in subsequent calls related to given device interface.
//                This must be called before any other NvAPI_Stereo_ function for that handle.
//                Multiple devices can be used at one time using multiple calls to this function (one per each device).
//
// HOW TO USE:    After the Direct3D device is created, create stereo handle.
//                On call success:
//                Use all other NvAPI_Stereo_ functions that have stereo handle as first parameter.
//                After the device interface correspondent to the stereo handle is destroyed,
//                application should call NvAPI_DestroyStereoHandle for that stereo handle.
//
// RETURN STATUS:
//                NVAPI_OK - Stereo handle is created for given device interface.
//                NVAPI_INVALID_ARGUMENT - Provided device interface is invalid.
//                NVAPI_API_NOT_INTIALIZED - NVAPI not initialized.
//                NVAPI_STEREO_NOT_INITIALIZED - Stereo part of NVAPI not initialized.
//                NVAPI_ERROR - Something is wrong (generic error).
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_Stereo_CreateHandleFromIUnknown(IUnknown *pDevice, StereoHandle *pStereoHandle);
#endif // defined(_D3D9_H_) || defined(__d3d10_h__)

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_Stereo_DestroyHandle
//
// PARAMETERS:    stereoHandle(IN) - Stereo handle that is to be destroyed.
//
//  SUPPORTED OS: Windows Vista and higher
//
// DESCRIPTION:   Destroys stereo handle created with one of NvAPI_Stereo_CreateHandleFrom functions.
//                This should be called after device corresponding to the handle has been destroyed.
//
// RETURN STATUS:
//                NVAPI_OK - Stereo handle is destroyed.
//                NVAPI_API_NOT_INTIALIZED - NVAPI not initialized.
//                NVAPI_STEREO_NOT_INITIALIZED - Stereo part of NVAPI not initialized.
//                NVAPI_ERROR - Something is wrong (generic error).
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_Stereo_DestroyHandle(StereoHandle stereoHandle);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_Stereo_Activate
//
// PARAMETERS:    stereoHandle(IN) - Stereo handle correspondent to device interface.
//
//  SUPPORTED OS: Windows Vista and higher
//
// DESCRIPTION:   Activates stereo for device interface correspondent to given stereo handle.
//                Activating stereo will be possible only if stereo was enabled previously in the registry.
//                Calls to all functions that require stereo activated
//                with stereo deactivated will have no effect and will return appropriate error code.
//
// HOW TO USE:    After the stereo handle for device interface is created via successfull call to appropriate NvAPI_Stereo_CreateHandleFrom function.
//
// RETURN STATUS:
//                NVAPI_OK - Stereo is turned on.
//                NVAPI_STEREO_INVALID_DEVICE_INTERFACE - Device interface is not valid. Create again, then attach again.
//                NVAPI_API_NOT_INTIALIZED - NVAPI not initialized.
//                NVAPI_STEREO_NOT_INITIALIZED - Stereo part of NVAPI not initialized.
//                NVAPI_ERROR - Something is wrong (generic error).
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_Stereo_Activate(StereoHandle stereoHandle);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_Stereo_Deactivate
//
// PARAMETERS:    stereoHandle(IN) - Stereo handle correspondent to device interface.
//
//  SUPPORTED OS: Windows Vista and higher
//
// DESCRIPTION:   Deactivates stereo for given device interface.
//                Calls to all functions that require stereo activated
//                with stereo deactivated will have no effect and will return appropriate error code.
//
// HOW TO USE:    After the stereo handle for device interface is created via successfull call to appropriate NvAPI_Stereo_CreateHandleFrom function.
//
// RETURN STATUS:
//                NVAPI_OK - Stereo is turned off.
//                NVAPI_STEREO_INVALID_DEVICE_INTERFACE - Device interface is not valid. Create again, then attach again.
//                NVAPI_API_NOT_INTIALIZED - NVAPI not initialized.
//                NVAPI_STEREO_NOT_INITIALIZED - Stereo part of NVAPI not initialized.
//                NVAPI_ERROR - Something is wrong (generic error).
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_Stereo_Deactivate(StereoHandle stereoHandle);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_Stereo_IsActivated
//
// PARAMETERS:    stereoHandle(IN) - Stereo handle correspondent to device interface.
//                pIsStereoOn(IN)  - Address where result of the inquiry will be placed.
//
//  SUPPORTED OS: Windows Vista and higher
//
// DESCRIPTION:   Checks if stereo is activated for given device interface.
//
// HOW TO USE:    After the stereo handle for device interface is created via successfull call to appropriate NvAPI_Stereo_CreateHandleFrom function.
//
// RETURN STATUS:
//                NVAPI_OK - Check was sucessfully completed and result reflects current state of stereo (on/off).
//                NVAPI_STEREO_INVALID_DEVICE_INTERFACE - Device interface is not valid. Create again, then attach again.
//                NVAPI_API_NOT_INTIALIZED - NVAPI not initialized.
//                NVAPI_STEREO_NOT_INITIALIZED - Stereo part of NVAPI not initialized.
//                NVAPI_ERROR - Something is wrong (generic error).
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_Stereo_IsActivated(StereoHandle stereoHandle, NvU8 *pIsStereoOn);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_Stereo_GetSeparation
//
// PARAMETERS:    stereoHandle(IN)           - Stereo handle correspondent to device interface.
//                pSeparationPercentage(OUT) - Address of @c float type variable to store current separation percentage in.
//
//  SUPPORTED OS: Windows Vista and higher
//
// DESCRIPTION:   Gets current separation value (in percents).
//
// HOW TO USE:    After the stereo handle for device interface is created via successfull call to appropriate NvAPI_Stereo_CreateHandleFrom function.
//
// RETURN STATUS:
//                NVAPI_OK - Retrieval of separation percentage was successfull.
//                NVAPI_STEREO_INVALID_DEVICE_INTERFACE - Device interface is not valid. Create again, then attach again.
//                NVAPI_API_NOT_INTIALIZED - NVAPI not initialized.
//                NVAPI_STEREO_NOT_INITIALIZED - Stereo part of NVAPI not initialized.
//                NVAPI_ERROR - Something is wrong (generic error).
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_Stereo_GetSeparation(StereoHandle stereoHandle, float *pSeparationPercentage);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_Stereo_SetSeparation
//
// PARAMETERS:    stereoHandle(IN)            - Stereo handle correspondent to device interface.
//                newSeparationPercentage(IN) - New value for separation percentage.
//
//  SUPPORTED OS: Windows Vista and higher
//
// DESCRIPTION:   Sets separation to given percentage.
//
// HOW TO USE:    After the stereo handle for device interface is created via successfull call to appropriate NvAPI_Stereo_CreateHandleFrom function.
//
// RETURN STATUS:
//                NVAPI_OK - Setting of separation percentage was successfull.
//                NVAPI_STEREO_INVALID_DEVICE_INTERFACE - Device interface is not valid. Create again, then attach again.
//                NVAPI_API_NOT_INTIALIZED - NVAPI not initialized.
//                NVAPI_STEREO_NOT_INITIALIZED - Stereo part of NVAPI not initialized.
//                NVAPI_STEREO_PARAMETER_OUT_OF_RANGE - Given separation percentage is out of [0..100] range.
//                NVAPI_ERROR - Something is wrong (generic error).
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_Stereo_SetSeparation(StereoHandle stereoHandle, float newSeparationPercentage);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_Stereo_DecreaseSeparation
//
// PARAMETERS:    stereoHandle(IN) - Stereo handle correspondent to device interface.
//
//  SUPPORTED OS: Windows Vista and higher
//
// DESCRIPTION:   Decreases separation for given device interface (same like Ctrl+F3 hotkey).
//
// HOW TO USE:    After the stereo handle for device interface is created via successfull call to appropriate NvAPI_Stereo_CreateHandleFrom function.
//
// RETURN STATUS:
//                NVAPI_OK - Decrease of separation percentage was successfull.
//                NVAPI_STEREO_INVALID_DEVICE_INTERFACE - Device interface is not valid. Create again, then attach again.
//                NVAPI_API_NOT_INTIALIZED - NVAPI not initialized.
//                NVAPI_STEREO_NOT_INITIALIZED - Stereo part of NVAPI not initialized.
//                NVAPI_ERROR - Something is wrong (generic error).
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_Stereo_DecreaseSeparation(StereoHandle stereoHandle);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_Stereo_IncreaseSeparation
//
// PARAMETERS:    stereoHandle(IN) - Stereo handle correspondent to device interface.
//
//  SUPPORTED OS: Windows Vista and higher
//
// DESCRIPTION:   Increases separation for given device interface (same like Ctrl+F4 hotkey).
//
// HOW TO USE:    After the stereo handle for device interface is created via successfull call to appropriate NvAPI_Stereo_CreateHandleFrom function.
//
// RETURN STATUS:
//                NVAPI_OK - Increase of separation percentage was successfull.
//                NVAPI_STEREO_INVALID_DEVICE_INTERFACE - Device interface is not valid. Create again, then attach again.
//                NVAPI_API_NOT_INTIALIZED - NVAPI not initialized.
//                NVAPI_STEREO_NOT_INITIALIZED - Stereo part of NVAPI not initialized.
//                NVAPI_ERROR - Something is wrong (generic error).
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_Stereo_IncreaseSeparation(StereoHandle stereoHandle);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_Stereo_GetConvergence
//
// PARAMETERS:    stereoHandle(IN)  - Stereo handle correspondent to device interface.
//                pConvergence(OUT) - Address of @c float type variable to store current convergence value in.
//
//  SUPPORTED OS: Windows Vista and higher
//
// DESCRIPTION:   Gets current convergence value.
//
// HOW TO USE:    After the stereo handle for device interface is created via successfull call to appropriate NvAPI_Stereo_CreateHandleFrom function.
//
// RETURN STATUS:
//                NVAPI_OK - Retrieval of convergence value was successfull.
//                NVAPI_STEREO_INVALID_DEVICE_INTERFACE - Device interface is not valid. Create again, then attach again.
//                NVAPI_API_NOT_INTIALIZED - NVAPI not initialized.
//                NVAPI_STEREO_NOT_INITIALIZED - Stereo part of NVAPI not initialized.
//                NVAPI_ERROR - Something is wrong (generic error).
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_Stereo_GetConvergence(StereoHandle stereoHandle, float *pConvergence);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_Stereo_SetConvergence
//
// PARAMETERS:    stereoHandle(IN)             - Stereo handle correspondent to device interface.
//                newConvergencePercentage(IN) - New value for convergence.
//
//  SUPPORTED OS: Windows Vista and higher
//
// DESCRIPTION:   Sets convergence to given value.
//
// HOW TO USE:    After the stereo handle for device interface is created via successfull call to appropriate NvAPI_Stereo_CreateHandleFrom function.
//
// RETURN STATUS:
//                NVAPI_OK - Setting of convergence value was successfull.
//                NVAPI_STEREO_INVALID_DEVICE_INTERFACE - Device interface is not valid. Create again, then attach again.
//                NVAPI_API_NOT_INTIALIZED - NVAPI not initialized.
//                NVAPI_STEREO_NOT_INITIALIZED - Stereo part of NVAPI not initialized.
//                NVAPI_ERROR - Something is wrong (generic error).
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_Stereo_SetConvergence(StereoHandle stereoHandle, float newConvergencePercentage);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_Stereo_DecreaseConvergence
//
// PARAMETERS:    stereoHandle(IN) - Stereo handle correspondent to device interface.
//
//  SUPPORTED OS: Windows Vista and higher
//
// DESCRIPTION:   Decreases convergence for given device interface (same like Ctrl+F5 hotkey).
//
// HOW TO USE:    After the stereo handle for device interface is created via successfull call to appropriate NvAPI_Stereo_CreateHandleFrom function.
//
// RETURN STATUS:
//                NVAPI_OK - Decrease of convergence was successfull.
//                NVAPI_STEREO_INVALID_DEVICE_INTERFACE - Device interface is not valid. Create again, then attach again.
//                NVAPI_API_NOT_INTIALIZED - NVAPI not initialized.
//                NVAPI_STEREO_NOT_INITIALIZED - Stereo part of NVAPI not initialized.
//                NVAPI_ERROR - Something is wrong (generic error).
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_Stereo_DecreaseConvergence(StereoHandle stereoHandle);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_Stereo_IncreaseConvergence
//
// PARAMETERS:    stereoHandle(IN) - Stereo handle correspondent to device interface.
//
//  SUPPORTED OS: Windows Vista and higher
//
// DESCRIPTION:   Increases convergence for given device interface (same like Ctrl+F5 hotkey).
//
// HOW TO USE:    After the stereo handle for device interface is created via successfull call to appropriate NvAPI_Stereo_CreateHandleFrom function.
//
// RETURN STATUS:
//                NVAPI_OK - Increase of convergence was successfull.
//                NVAPI_STEREO_INVALID_DEVICE_INTERFACE - Device interface is not valid. Create again, then attach again.
//                NVAPI_API_NOT_INTIALIZED - NVAPI not initialized.
//                NVAPI_STEREO_NOT_INITIALIZED - Stereo part of NVAPI not initialized.
//                NVAPI_ERROR - Something is wrong (generic error).
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_Stereo_IncreaseConvergence(StereoHandle stereoHandle);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_Stereo_GetFrustumAdjustMode
//
// PARAMETERS:    stereoHandle(IN)        - Stereo handle correspondent to device interface.
//                pFrustumAdjustMode(OUT) - Address of the NV_FRUSTUM_ADJUST_MODE type variable to store current frustum value in.
//
//  SUPPORTED OS: Windows Vista and higher
//
// DESCRIPTION:   Gets current frustum adjust mode value.
//
// HOW TO USE:    After the stereo handle for device interface is created via successfull call to appropriate NvAPI_Stereo_CreateHandleFrom function.
//
// RETURN STATUS:
//                NVAPI_OK - Retrieval of frustum adjust mode was successfull.
//                NVAPI_STEREO_INVALID_DEVICE_INTERFACE - Device interface is not valid. Create again, then attach again.
//                NVAPI_API_NOT_INTIALIZED - NVAPI not initialized.
//                NVAPI_STEREO_NOT_INITIALIZED - Stereo part of NVAPI not initialized.
//                NVAPI_ERROR - Something is wrong (generic error).
//
///////////////////////////////////////////////////////////////////////////////

typedef enum _NV_FrustumAdjustMode
{
    NVAPI_NO_FRUSTUM_ADJUST,    // Do not adjust frustum.
    NVAPI_FRUSTUM_STRETCH,      // Stretch images in X.
    NVAPI_FRUSTUM_CLEAR_EDGES   // Clear corresponding edges for each eye.
} NV_FRUSTUM_ADJUST_MODE;

NVAPI_INTERFACE NvAPI_Stereo_GetFrustumAdjustMode(StereoHandle stereoHandle, NV_FRUSTUM_ADJUST_MODE *pFrustumAdjustMode);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_Stereo_SetFrustumAdjustMode
//
// PARAMETERS:    stereoHandle(IN)               - Stereo handle correspondent to device interface.
//                newFrustumAdjustModeValue (IN) - New value for frustum adjust mode.
//                                                 Should be one of the symbolic constants defined in NV_FRUSTUM_ADJUST_MODE.
//                                                 Any other value will cause function to do nothing and return NVAPI_STEREO_FRUSTUM_ADJUST_MODE_NOT_SUPPORTED.
//
//  SUPPORTED OS: Windows Vista and higher
//
// DESCRIPTION:   Sets current frustum adjust mode value.
//
// HOW TO USE:    After the stereo handle for device interface is created via successfull call to appropriate NvAPI_Stereo_CreateHandleFrom function.
//
// RETURN STATUS:
//                NVAPI_OK - Retrieval of frustum adjust mode was successfull.
//                NVAPI_STEREO_INVALID_DEVICE_INTERFACE - Device interface is not valid. Create again, then attach again.
//                NVAPI_API_NOT_INTIALIZED - NVAPI not initialized.
//                NVAPI_STEREO_NOT_INITIALIZED - Stereo part of NVAPI not initialized.
//                NVAPI_STEREO_FRUSTUM_ADJUST_MODE_NOT_SUPPORTED - Given frustum adjust mode is not supported.
//                NVAPI_ERROR - Something is wrong (generic error).
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_Stereo_SetFrustumAdjustMode(StereoHandle stereoHandle, NV_FRUSTUM_ADJUST_MODE newFrustumAdjustModeValue);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_Stereo_CaptureJpegImage
//
// PARAMETERS:    stereoHandle(IN) - Stereo handle correspondent to device interface.
//                quality(IN)      - Quality of the JPEG image to be captured. Integer value betweeen 0 and 100.
//
//  SUPPORTED OS: Windows Vista and higher
//
// DESCRIPTION:   Captures current stereo image in JPEG stereo format with given quality.
//                Only the last capture call per flip will be effective.
//
// HOW TO USE:    After the stereo handle for device interface is created via successfull call to appropriate NvAPI_Stereo_CreateHandleFrom function.
//
// RETURN STATUS:
//                NVAPI_OK - Image captured.
//                NVAPI_STEREO_INVALID_DEVICE_INTERFACE - Device interface is not valid. Create again, then attach again.
//                NVAPI_API_NOT_INTIALIZED - NVAPI not initialized.
//                NVAPI_STEREO_NOT_INITIALIZED - Stereo part of NVAPI not initialized.
//                NVAPI_STEREO_PARAMETER_OUT_OF_RANGE - Given quality is out of [0..100] range.
//                NVAPI_ERROR - Something is wrong (generic error).
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_Stereo_CaptureJpegImage(StereoHandle stereoHandle, NvU32 quality);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_Stereo_CapturePngImage
//
// PARAMETERS:    stereoHandle(IN) - Stereo handle correspondent to device interface.
//
//  SUPPORTED OS: Windows Vista and higher
//
// DESCRIPTION:   Captures current stereo image in PNG stereo format.
//                Only the last capture call per flip will be effective.
//
// HOW TO USE:    After the stereo handle for device interface is created via successfull call to appropriate NvAPI_Stereo_CreateHandleFrom function.
//
// RETURN STATUS:
//                NVAPI_OK - Image captured.
//                NVAPI_STEREO_INVALID_DEVICE_INTERFACE - Device interface is not valid. Create again, then attach again.
//                NVAPI_API_NOT_INTIALIZED - NVAPI not initialized.
//                NVAPI_STEREO_NOT_INITIALIZED - Stereo part of NVAPI not initialized.
//                NVAPI_ERROR - Something is wrong (generic error).
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_Stereo_CapturePngImage(StereoHandle stereoHandle);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_Stereo_ReverseStereoBlitControl
//
// PARAMETERS:    stereoHandle(IN) - Stereo handle correspondent to device interface.
//                TurnOn(IN)       != 0  - turns on,
//                                 == 0  - turns off
//
//  SUPPORTED OS: Windows Vista and higher
//
// DESCRIPTION:   Turns on/off reverse stereo blit
//
// HOW TO USE:    After the stereo handle for device interface is created via successfull call to appropriate
//                NvAPI_Stereo_CreateHandleFrom function.
//                After reversed stereo blit control turned on blit from stereo surface will
//                produce right eye image in the left side of the destination surface and left
//                eye image in the right side of the destination surface
//                In DX9 Dst surface has to be created as render target and StretchRect has to be used.
//                Conditions:
//                1. DstWidth == 2*SrcWidth
//                2. DstHeight == SrcHeight
//                3. Src surface is actually stereo surface.
//                4. SrcRect must be {0,0,SrcWidth,SrcHeight}
//                5. DstRect must be {0,0,DstWidth,DstHeight}
//
//                In DX10 ResourceCopyRegion has to be used
//                Conditions:
//                1. DstWidth == 2*SrcWidth
//                2. DstHeight == SrcHeight
//                3. dstX == 0,
//                4. dstY == 0,
//                5. dstZ == 0,
//                6  SrcBox: left=top=front==0;
//                           right==SrcWidth; bottom==SrcHeight; back==1;
//
// RETURN STATUS:
//                NVAPI_OK - Retrieval of frustum adjust mode was successfull.
//                NVAPI_STEREO_INVALID_DEVICE_INTERFACE - Device interface is not valid. Create again, then attach again.
//                NVAPI_API_NOT_INTIALIZED - NVAPI not initialized.
//                NVAPI_STEREO_NOT_INITIALIZED - Stereo part of NVAPI not initialized.
//                NVAPI_ERROR - Something is wrong (generic error).
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_Stereo_ReverseStereoBlitControl(StereoHandle hStereoHandle, NvU8 TurnOn);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_Stereo_SetNotificationMessage
//
// PARAMETERS:    stereoHandle(IN) - Stereo handle correspondent to device interface.
//                hWnd(IN)         - Window HWND that will be notified when user changed stereo driver state.
//                                   Actual HWND must be cast to an NvU64.
//                messageID(IN)    - MessageID of the message that will be posted to hWnd
//
//  SUPPORTED OS: Windows Vista and higher
//
// DESCRIPTION:   Setup notification message that stereo driver will use to notify application
//                when user changes stereo driver state.
//                Call this API with NULL hWnd to prohibit notification.
//
//
// HOW TO USE:    After the stereo handle for device interface is created via successfull call to appropriate
//                NvAPI_Stereo_CreateHandleFrom function.
//
//                When user changes stereo state Activated or Deactivated, separation or conversion
//                stereo driver will post defined message with the folloing parameters
//
//                wParam == MAKEWPARAM(l, h) where l == 0 if stereo is deactivated
//                                                      1 if stereo is deactivated
//                                                 h  - is current separation.
//                                                      Actual separation is float(h*100.f/0xFFFF);
//                lParam                           is current conversion.
//                                                      Actual conversion is *(float*)&lParam
//
// RETURN STATUS:
//                NVAPI_OK - Notification set.
//                NVAPI_STEREO_INVALID_DEVICE_INTERFACE - Device interface is not valid. Create again, then attach again.
//                NVAPI_API_NOT_INTIALIZED - NVAPI not initialized.
//                NVAPI_STEREO_NOT_INITIALIZED - Stereo part of NVAPI not initialized.
//                NVAPI_ERROR - Something is wrong (generic error).
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_Stereo_SetNotificationMessage(StereoHandle hStereoHandle, NvU64 hWnd,NvU64 messageID);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_Stereo_GetEyeSeparation
//
// PARAMETERS:    stereoHandle(IN) - Stereo handle correspondent to device interface.
//                pSeparation(OUT) - Eye separation.
//
//  SUPPORTED OS: Windows Vista and higher
//
// DESCRIPTION:   Return eye separation as <between eye distance>/<phisical screen width> ratio
//
// HOW TO USE:    After the stereo handle for device interface is created via successfull call to appropriate
//                Applyed only for DX9 and up.
//
// RETURN STATUS:
//                NVAPI_OK - Active eye is set.
//                NVAPI_STEREO_INVALID_DEVICE_INTERFACE - Device interface is not valid. Create again, then attach again.
//                NVAPI_API_NOT_INTIALIZED - NVAPI not initialized.
//                NVAPI_STEREO_NOT_INITIALIZED - Stereo part of NVAPI not initialized.
//                NVAPI_ERROR - Something is wrong (generic error).
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_Stereo_GetEyeSeparation(StereoHandle hStereoHandle,  float *pSeparation );


typedef NvU32   NVVIOOWNERID;                               // Unique identifier for VIO owner (process identifier or NVVIOOWNERID_NONE)
#define NVVIOOWNERID_NONE                   0               // Unregistered ownerId

typedef enum _NVVIOOWNERTYPE                                // Owner type for device
{
    NVVIOOWNERTYPE_NONE                             ,       //  No owner for device
    NVVIOOWNERTYPE_APPLICATION                      ,       //  Application owns device
    NVVIOOWNERTYPE_DESKTOP                          ,       //  Desktop transparent mode owns device (not applicable for video input)
}NVVIOOWNERTYPE;

// Access rights for NvAPI_VIO_Open()
#define NVVIO_O_READ                        0x00000000      // Read access             (not applicable for video output)
#define NVVIO_O_WRITE_EXCLUSIVE             0x00010001      // Write exclusive access  (not applicable for video input)

#define NVVIO_VALID_ACCESSRIGHTS            (NVVIO_O_READ              | \
                                             NVVIO_O_WRITE_EXCLUSIVE   )


// VIO_DATA.ulOwnerID high-bit is set only if device has been initialized by VIOAPI
// examined at NvAPI_GetCapabilities|NvAPI_VIO_Open to determine if settings need to be applied from registry or POR state read
#define NVVIO_OWNERID_INITIALIZED  0x80000000

// VIO_DATA.ulOwnerID next-bit is set only if device is currently in exclusive write access mode from NvAPI_VIO_Open()
#define NVVIO_OWNERID_EXCLUSIVE    0x40000000

// VIO_DATA.ulOwnerID lower bits are:
//  NVGVOOWNERTYPE_xxx enumerations indicating use context
#define NVVIO_OWNERID_TYPEMASK     0x0FFFFFFF // mask for NVVIOOWNERTYPE_xxx
//---------------------------------------------------------------------
// Enumerations
//---------------------------------------------------------------------

// Video signal format and resolution
typedef enum _NVVIOSIGNALFORMAT
{
    NVVIOSIGNALFORMAT_NONE,                         // Invalid signal format
    NVVIOSIGNALFORMAT_487I_59_94_SMPTE259_NTSC,     // 01  487i    59.94Hz  (SMPTE259) NTSC
    NVVIOSIGNALFORMAT_576I_50_00_SMPTE259_PAL,      // 02  576i    50.00Hz  (SMPTE259) PAL
    NVVIOSIGNALFORMAT_1035I_60_00_SMPTE260,         // 03  1035i   60.00Hz  (SMPTE260)
    NVVIOSIGNALFORMAT_1035I_59_94_SMPTE260,         // 04  1035i   59.94Hz  (SMPTE260)
    NVVIOSIGNALFORMAT_1080I_50_00_SMPTE295,         // 05  1080i   50.00Hz  (SMPTE295)
    NVVIOSIGNALFORMAT_1080I_60_00_SMPTE274,         // 06  1080i   60.00Hz  (SMPTE274)
    NVVIOSIGNALFORMAT_1080I_59_94_SMPTE274,         // 07  1080i   59.94Hz  (SMPTE274)
    NVVIOSIGNALFORMAT_1080I_50_00_SMPTE274,         // 08  1080i   50.00Hz  (SMPTE274)
    NVVIOSIGNALFORMAT_1080P_30_00_SMPTE274,         // 09  1080p   30.00Hz  (SMPTE274)
    NVVIOSIGNALFORMAT_1080P_29_97_SMPTE274,         // 10  1080p   29.97Hz  (SMPTE274)
    NVVIOSIGNALFORMAT_1080P_25_00_SMPTE274,         // 11  1080p   25.00Hz  (SMPTE274)
    NVVIOSIGNALFORMAT_1080P_24_00_SMPTE274,         // 12  1080p   24.00Hz  (SMPTE274)
    NVVIOSIGNALFORMAT_1080P_23_976_SMPTE274,        // 13  1080p   23.976Hz (SMPTE274)
    NVVIOSIGNALFORMAT_720P_60_00_SMPTE296,          // 14  720p    60.00Hz  (SMPTE296)
    NVVIOSIGNALFORMAT_720P_59_94_SMPTE296,          // 15  720p    59.94Hz  (SMPTE296)
    NVVIOSIGNALFORMAT_720P_50_00_SMPTE296,          // 16  720p    50.00Hz  (SMPTE296)
    NVVIOSIGNALFORMAT_1080I_48_00_SMPTE274,         // 17  1080I   48.00Hz  (SMPTE274)
    NVVIOSIGNALFORMAT_1080I_47_96_SMPTE274,         // 18  1080I   47.96Hz  (SMPTE274)
    NVVIOSIGNALFORMAT_720P_30_00_SMPTE296,          // 19  720p    30.00Hz  (SMPTE296)
    NVVIOSIGNALFORMAT_720P_29_97_SMPTE296,          // 20  720p    29.97Hz  (SMPTE296)
    NVVIOSIGNALFORMAT_720P_25_00_SMPTE296,          // 21  720p    25.00Hz  (SMPTE296)
    NVVIOSIGNALFORMAT_720P_24_00_SMPTE296,          // 22  720p    24.00Hz  (SMPTE296)
    NVVIOSIGNALFORMAT_720P_23_98_SMPTE296,          // 23  720p    23.98Hz  (SMPTE296)
    NVVIOSIGNALFORMAT_2048P_30_00_SMPTE372,         // 24  2048p   30.00Hz  (SMPTE372)
    NVVIOSIGNALFORMAT_2048P_29_97_SMPTE372,         // 25  2048p   29.97Hz  (SMPTE372)
    NVVIOSIGNALFORMAT_2048I_60_00_SMPTE372,         // 26  2048i   60.00Hz  (SMPTE372)
    NVVIOSIGNALFORMAT_2048I_59_94_SMPTE372,         // 27  2048i   59.94Hz  (SMPTE372)
    NVVIOSIGNALFORMAT_2048P_25_00_SMPTE372,         // 28  2048p   25.00Hz  (SMPTE372)
    NVVIOSIGNALFORMAT_2048I_50_00_SMPTE372,         // 29  2048i   50.00Hz  (SMPTE372)
    NVVIOSIGNALFORMAT_2048P_24_00_SMPTE372,         // 30  2048p   24.00Hz  (SMPTE372)
    NVVIOSIGNALFORMAT_2048P_23_98_SMPTE372,         // 31  2048p   23.98Hz  (SMPTE372)
    NVVIOSIGNALFORMAT_2048I_48_00_SMPTE372,         // 32  2048i   48.00Hz  (SMPTE372)
    NVVIOSIGNALFORMAT_2048I_47_96_SMPTE372,         // 33  2048i   47.96Hz  (SMPTE372)

    NVVIOSIGNALFORMAT_1080PSF_25_00_SMPTE274,       // 34  1080PsF 25.00Hz  (SMPTE274)
    NVVIOSIGNALFORMAT_1080PSF_29_97_SMPTE274,       // 35  1080PsF 29.97Hz  (SMPTE274)
    NVVIOSIGNALFORMAT_1080PSF_30_00_SMPTE274,       // 36  1080PsF 30.00Hz  (SMPTE274)
    NVVIOSIGNALFORMAT_1080PSF_24_00_SMPTE274,       // 37  1080PsF 24.00Hz  (SMPTE274)
    NVVIOSIGNALFORMAT_1080PSF_23_98_SMPTE274,       // 38  1080PsF 23.98Hz  (SMPTE274)

    NVVIOSIGNALFORMAT_1080P_50_00_SMPTE274_3G_LEVEL_A, // 39  1080P   50.00Hz  (SMPTE274) 3G Level A
    NVVIOSIGNALFORMAT_1080P_59_94_SMPTE274_3G_LEVEL_A, // 40  1080P   59.94Hz  (SMPTE274) 3G Level A
    NVVIOSIGNALFORMAT_1080P_60_00_SMPTE274_3G_LEVEL_A, // 41  1080P   60.00Hz  (SMPTE274) 3G Level A

    NVVIOSIGNALFORMAT_1080P_60_00_SMPTE274_3G_LEVEL_B, // 42  1080p   60.00Hz  (SMPTE274) 3G Level B
    NVVIOSIGNALFORMAT_1080I_60_00_SMPTE274_3G_LEVEL_B, // 43  1080i   60.00Hz  (SMPTE274) 3G Level B
    NVVIOSIGNALFORMAT_2048I_60_00_SMPTE372_3G_LEVEL_B, // 44  2048i   60.00Hz  (SMPTE372) 3G Level B
    NVVIOSIGNALFORMAT_1080P_50_00_SMPTE274_3G_LEVEL_B, // 45  1080p   50.00Hz  (SMPTE274) 3G Level B
    NVVIOSIGNALFORMAT_1080I_50_00_SMPTE274_3G_LEVEL_B, // 46  1080i   50.00Hz  (SMPTE274) 3G Level B
    NVVIOSIGNALFORMAT_2048I_50_00_SMPTE372_3G_LEVEL_B, // 47  2048i   50.00Hz  (SMPTE372) 3G Level B
    NVVIOSIGNALFORMAT_1080P_30_00_SMPTE274_3G_LEVEL_B, // 48  1080p   30.00Hz  (SMPTE274) 3G Level B
    NVVIOSIGNALFORMAT_2048P_30_00_SMPTE372_3G_LEVEL_B, // 49  2048p   30.00Hz  (SMPTE372) 3G Level B
    NVVIOSIGNALFORMAT_1080P_25_00_SMPTE274_3G_LEVEL_B, // 50  1080p   25.00Hz  (SMPTE274) 3G Level B
    NVVIOSIGNALFORMAT_2048P_25_00_SMPTE372_3G_LEVEL_B, // 51  2048p   25.00Hz  (SMPTE372) 3G Level B
    NVVIOSIGNALFORMAT_1080P_24_00_SMPTE274_3G_LEVEL_B, // 52  1080p   24.00Hz  (SMPTE274) 3G Level B
    NVVIOSIGNALFORMAT_2048P_24_00_SMPTE372_3G_LEVEL_B, // 53  2048p   24.00Hz  (SMPTE372) 3G Level B
    NVVIOSIGNALFORMAT_1080I_48_00_SMPTE274_3G_LEVEL_B, // 54  1080i   48.00Hz  (SMPTE274) 3G Level B
    NVVIOSIGNALFORMAT_2048I_48_00_SMPTE372_3G_LEVEL_B, // 55  2048i   48.00Hz  (SMPTE372) 3G Level B
    NVVIOSIGNALFORMAT_1080P_59_94_SMPTE274_3G_LEVEL_B, // 56  1080p   59.94Hz  (SMPTE274) 3G Level B
    NVVIOSIGNALFORMAT_1080I_59_94_SMPTE274_3G_LEVEL_B, // 57  1080i   59.94Hz  (SMPTE274) 3G Level B
    NVVIOSIGNALFORMAT_2048I_59_94_SMPTE372_3G_LEVEL_B, // 58  2048i   59.94Hz  (SMPTE372) 3G Level B
    NVVIOSIGNALFORMAT_1080P_29_97_SMPTE274_3G_LEVEL_B, // 59  1080p   29.97Hz  (SMPTE274) 3G Level B
    NVVIOSIGNALFORMAT_2048P_29_97_SMPTE372_3G_LEVEL_B, // 60  2048p   29.97Hz  (SMPTE372) 3G Level B
    NVVIOSIGNALFORMAT_1080P_23_98_SMPTE274_3G_LEVEL_B, // 61  1080p   29.98Hz  (SMPTE274) 3G Level B
    NVVIOSIGNALFORMAT_2048P_23_98_SMPTE372_3G_LEVEL_B, // 62  2048p   29.98Hz  (SMPTE372) 3G Level B
    NVVIOSIGNALFORMAT_1080I_47_96_SMPTE274_3G_LEVEL_B, // 63  1080i   47.96Hz  (SMPTE274) 3G Level B
    NVVIOSIGNALFORMAT_2048I_47_96_SMPTE372_3G_LEVEL_B, // 64  2048i   47.96Hz  (SMPTE372) 3G Level B

    NVVIOSIGNALFORMAT_END                              // 65  To indicate end of signal format list
}NVVIOSIGNALFORMAT;

// SMPTE standards format
typedef enum _NVVIOVIDEOSTANDARD
{
    NVVIOVIDEOSTANDARD_SMPTE259                        ,       // SMPTE259
    NVVIOVIDEOSTANDARD_SMPTE260                        ,       // SMPTE260
    NVVIOVIDEOSTANDARD_SMPTE274                        ,       // SMPTE274
    NVVIOVIDEOSTANDARD_SMPTE295                        ,       // SMPTE295
    NVVIOVIDEOSTANDARD_SMPTE296                        ,       // SMPTE296
    NVVIOVIDEOSTANDARD_SMPTE372                        ,       // SMPTE372
}NVVIOVIDEOSTANDARD;

// HD or SD video type
typedef enum _NVVIOVIDEOTYPE
{
    NVVIOVIDEOTYPE_SD                                  ,       // Standard-definition (SD)
    NVVIOVIDEOTYPE_HD                                  ,       // High-definition     (HD)
}NVVIOVIDEOTYPE;

// Interlace mode
typedef enum _NVVIOINTERLACEMODE
{
    NVVIOINTERLACEMODE_PROGRESSIVE                     ,       // Progressive               (p)
    NVVIOINTERLACEMODE_INTERLACE                       ,       // Interlace                 (i)
    NVVIOINTERLACEMODE_PSF                             ,       // Progressive Segment Frame (psf)
}NVVIOINTERLACEMODE;

// Video data format
typedef enum _NVVIODATAFORMAT
{
    NVVIODATAFORMAT_UNKNOWN   = -1                     ,       // Invalid DataFormat
    NVVIODATAFORMAT_R8G8B8_TO_YCRCB444                 ,       // R8:G8:B8                => YCrCb  (4:4:4)
    NVVIODATAFORMAT_R8G8B8A8_TO_YCRCBA4444             ,       // R8:G8:B8:A8             => YCrCbA (4:4:4:4)
    NVVIODATAFORMAT_R8G8B8Z10_TO_YCRCBZ4444            ,       // R8:G8:B8:Z10            => YCrCbZ (4:4:4:4)
    NVVIODATAFORMAT_R8G8B8_TO_YCRCB422                 ,       // R8:G8:B8                => YCrCb  (4:2:2)
    NVVIODATAFORMAT_R8G8B8A8_TO_YCRCBA4224             ,       // R8:G8:B8:A8             => YCrCbA (4:2:2:4)
    NVVIODATAFORMAT_R8G8B8Z10_TO_YCRCBZ4224            ,       // R8:G8:B8:Z10            => YCrCbZ (4:2:2:4)
    NVVIODATAFORMAT_X8X8X8_444_PASSTHRU                ,       // R8:G8:B8                => RGB    (4:4:4)
    NVVIODATAFORMAT_X8X8X8A8_4444_PASSTHRU             ,       // R8:G8:B8:A8             => RGBA   (4:4:4:4)
    NVVIODATAFORMAT_X8X8X8Z10_4444_PASSTHRU            ,       // R8:G8:B8:Z10            => RGBZ   (4:4:4:4)
    NVVIODATAFORMAT_X10X10X10_444_PASSTHRU             ,       // Y10:CR10:CB10           => YCrCb  (4:4:4)
    NVVIODATAFORMAT_X10X8X8_444_PASSTHRU               ,       // Y10:CR8:CB8             => YCrCb  (4:4:4)
    NVVIODATAFORMAT_X10X8X8A10_4444_PASSTHRU           ,       // Y10:CR8:CB8:A10         => YCrCbA (4:4:4:4)
    NVVIODATAFORMAT_X10X8X8Z10_4444_PASSTHRU           ,       // Y10:CR8:CB8:Z10         => YCrCbZ (4:4:4:4)
    NVVIODATAFORMAT_DUAL_R8G8B8_TO_DUAL_YCRCB422       ,       // R8:G8:B8 + R8:G8:B8     => YCrCb  (4:2:2 + 4:2:2)
    NVVIODATAFORMAT_DUAL_X8X8X8_TO_DUAL_422_PASSTHRU   ,       // Y8:CR8:CB8 + Y8:CR8:CB8 => YCrCb  (4:2:2 + 4:2:2)
    NVVIODATAFORMAT_R10G10B10_TO_YCRCB422              ,       // R10:G10:B10             => YCrCb  (4:2:2)
    NVVIODATAFORMAT_R10G10B10_TO_YCRCB444              ,       // R10:G10:B10             => YCrCb  (4:4:4)
    NVVIODATAFORMAT_X12X12X12_444_PASSTHRU             ,       // X12:X12:X12             => XXX    (4:4:4)
    NVVIODATAFORMAT_X12X12X12_422_PASSTHRU             ,       // X12:X12:X12             => XXX    (4:2:2)
    NVVIODATAFORMAT_Y10CR10CB10_TO_YCRCB422            ,       // Y10:CR10:CB10           => YCrCb  (4:2:2)
    NVVIODATAFORMAT_Y8CR8CB8_TO_YCRCB422               ,       // Y8:CR8:CB8              => YCrCb  (4:2:2)
    NVVIODATAFORMAT_Y10CR8CB8A10_TO_YCRCBA4224         ,       // Y10:CR8:CB8:A10         => YCrCbA (4:2:2:4)
    NVVIODATAFORMAT_R10G10B10_TO_RGB444                ,       // R10:G10:B10             => RGB    (4:4:4)
    NVVIODATAFORMAT_R12G12B12_TO_YCRCB444              ,       // R12:G12:B12             => YCrCb  (4:4:4)
    NVVIODATAFORMAT_R12G12B12_TO_YCRCB422              ,       // R12:G12:B12             => YCrCb  (4:2:2)
}NVVIODATAFORMAT;

// Video output area
typedef enum _NVVIOOUTPUTAREA
{
    NVVIOOUTPUTAREA_FULLSIZE                           ,       // Output to entire video resolution (full size)
    NVVIOOUTPUTAREA_SAFEACTION                         ,       // Output to centered 90% of video resolution (safe action)
    NVVIOOUTPUTAREA_SAFETITLE                          ,       // Output to centered 80% of video resolution (safe title)
}NVVIOOUTPUTAREA;

// Synchronization source
typedef enum _NVVIOSYNCSOURCE
{
    NVVIOSYNCSOURCE_SDISYNC                            ,       // SDI Sync  (Digital input)
    NVVIOSYNCSOURCE_COMPSYNC                           ,       // COMP Sync (Composite input)
}NVVIOSYNCSOURCE;

// Composite synchronization type
typedef enum _NVVIOCOMPSYNCTYPE
{
    NVVIOCOMPSYNCTYPE_AUTO                             ,       // Auto-detect
    NVVIOCOMPSYNCTYPE_BILEVEL                          ,       // Bi-level signal
    NVVIOCOMPSYNCTYPE_TRILEVEL                         ,       // Tri-level signal
}NVVIOCOMPSYNCTYPE;

// Video input output status
typedef enum _NVVIOINPUTOUTPUTSTATUS
{
    NVINPUTOUTPUTSTATUS_OFF                            ,       // Not in use
    NVINPUTOUTPUTSTATUS_ERROR                          ,       // Error detected
    NVINPUTOUTPUTSTATUS_SDI_SD                         ,       // SDI (standard-definition)
    NVINPUTOUTPUTSTATUS_SDI_HD                         ,       // SDI (high-definition)
}NVVIOINPUTOUTPUTSTATUS;

// Synchronization input status
typedef enum _NVVIOSYNCSTATUS
{
    NVVIOSYNCSTATUS_OFF                                ,       // Sync not detected
    NVVIOSYNCSTATUS_ERROR                              ,       // Error detected
    NVVIOSYNCSTATUS_SYNCLOSS                           ,       // Genlock in use, format mismatch with output
    NVVIOSYNCSTATUS_COMPOSITE                          ,       // Composite sync
    NVVIOSYNCSTATUS_SDI_SD                             ,       // SDI sync (standard-definition)
    NVVIOSYNCSTATUS_SDI_HD                             ,       // SDI sync (high-definition)
}NVVIOSYNCSTATUS;

//Video Capture Status
typedef enum _NVVIOCAPTURESTATUS
{
    NVVIOSTATUS_STOPPED                                ,       // Sync not detected
    NVVIOSTATUS_RUNNING                                ,       // Error detected
    NVVIOSTATUS_ERROR                                  ,       // Genlock in use, format mismatch with output
}NVVIOCAPTURESTATUS;

//Video Capture Status
typedef enum _NVVIOSTATUSTYPE
{
    NVVIOSTATUSTYPE_IN                                 ,       // Input Status
    NVVIOSTATUSTYPE_OUT                                ,       // Output Status
}NVVIOSTATUSTYPE;

#define NVAPI_MAX_VIO_DEVICES                 8   // Assumption, maximum 4 SDI input and 4 SDI output cards supported on a system
#define NVAPI_MAX_VIO_JACKS                   4   // 4 physical jacks supported on each SDI input card.
#define NVAPI_MAX_VIO_CHANNELS_PER_JACK       2   // Each physical jack an on SDI input card can have
                                                  // two "channels" in the case of "3G" VideoFormats, as specified
                                                  // by SMPTE 425; for non-3G VideoFormats, only the first channel within
                                                  // a physical jack is valid
#define NVAPI_MAX_VIO_STREAMS                 4   // 4 Streams, 1 per physical jack
#define NVAPI_MIN_VIO_STREAMS                 1
#define NVAPI_MAX_VIO_LINKS_PER_STREAM        2   // SDI input supports a max of 2 links per stream
#define NVAPI_MAX_FRAMELOCK_MAPPING_MODES     20
#define NVAPI_GVI_MIN_RAW_CAPTURE_IMAGES      1   // Min number of capture images
#define NVAPI_GVI_MAX_RAW_CAPTURE_IMAGES      32  // Max number of capture images
#define NVAPI_GVI_DEFAULT_RAW_CAPTURE_IMAGES  5   // Default number of capture images

// Data Signal notification events. These need a event handler in RM.
// Register/Unregister and PopEvent NVAPI's are already available.

// Device configuration
typedef enum _NVVIOCONFIGTYPE
{
    NVVIOCONFIGTYPE_IN                                 ,       // Input Status
    NVVIOCONFIGTYPE_OUT                                ,       // Output Status
}NVVIOCONFIGTYPE;

typedef enum _NVVIOCOLORSPACE
{
    NVVIOCOLORSPACE_UNKNOWN,
    NVVIOCOLORSPACE_YCBCR,
    NVVIOCOLORSPACE_YCBCRA,
    NVVIOCOLORSPACE_YCBCRD,
    NVVIOCOLORSPACE_GBR,
    NVVIOCOLORSPACE_GBRA,
    NVVIOCOLORSPACE_GBRD,
} NVVIOCOLORSPACE;

// Component sampling
typedef enum _NVVIOCOMPONENTSAMPLING
{
    NVVIOCOMPONENTSAMPLING_UNKNOWN,
    NVVIOCOMPONENTSAMPLING_4444,
    NVVIOCOMPONENTSAMPLING_4224,
    NVVIOCOMPONENTSAMPLING_444,
    NVVIOCOMPONENTSAMPLING_422
} NVVIOCOMPONENTSAMPLING;

typedef enum _NVVIOBITSPERCOMPONENT
{
    NVVIOBITSPERCOMPONENT_UNKNOWN,
    NVVIOBITSPERCOMPONENT_8,
    NVVIOBITSPERCOMPONENT_10,
    NVVIOBITSPERCOMPONENT_12,
} NVVIOBITSPERCOMPONENT;

typedef enum _NVVIOLINKID
{
    NVVIOLINKID_UNKNOWN,
    NVVIOLINKID_A,
    NVVIOLINKID_B,
    NVVIOLINKID_C,
    NVVIOLINKID_D
} NVVIOLINKID;

//---------------------------------------------------------------------
// Structures
//---------------------------------------------------------------------

#define NVVIOCAPS_VIDOUT_SDI                0x00000001      // Supports Serial Digital Interface (SDI) output
#define NVVIOCAPS_SYNC_INTERNAL             0x00000100      // Supports Internal timing source
#define NVVIOCAPS_SYNC_GENLOCK              0x00000200      // Supports Genlock timing source
#define NVVIOCAPS_SYNCSRC_SDI               0x00001000      // Supports Serial Digital Interface (SDI) synchronization input
#define NVVIOCAPS_SYNCSRC_COMP              0x00002000      // Supports Composite synchronization input
#define NVVIOCAPS_OUTPUTMODE_DESKTOP        0x00010000      // Supports Desktop transparent mode
#define NVVIOCAPS_OUTPUTMODE_OPENGL         0x00020000      // Supports OpenGL application mode
#define NVVIOCAPS_VIDIN_SDI                 0x00100000      // Supports Serial Digital Interface (SDI) input

#define NVVIOCLASS_SDI                      0x00000001      // SDI-class interface: SDI output with two genlock inputs

// Device capabilities
typedef struct _NVVIOCAPS
{
    NvU32             version;                              // Structure version
    NvAPI_String      adapterName;                          // Graphics adapter name
    NvU32             adapterClass;                         // Graphics adapter classes (NVVIOCLASS_SDI mask)
    NvU32             adapterCaps;                          // Graphics adapter capabilities (NVVIOCAPS_* mask)
    NvU32             dipSwitch;                            // On-board DIP switch settings bits
    NvU32             dipSwitchReserved;                    // On-board DIP switch settings reserved bits
    NvU32             boardID;                              // Board ID
    struct                                                  //
    {                                                       // Driver version
        NvU32          majorVersion;                        // Major version
        NvU32          minorVersion;                        // Minor version
    } driver;                                               //
    struct                                                  //
    {                                                       // Firmware version
        NvU32          majorVersion;                        // Major version
        NvU32          minorVersion;                        // Minor version
    } firmWare;                                             //
    NVVIOOWNERID      ownerId;                              // Unique identifier for owner of video output (NVVIOOWNERID_INVALID if free running)
    NVVIOOWNERTYPE    ownerType;                            // Owner type (OpenGL application or Desktop mode)
} NVVIOCAPS;

#define NVVIOCAPS_VER   MAKE_NVAPI_VERSION(NVVIOCAPS,1)

// Input channel status
typedef struct _NVVIOCHANNELSTATUS
{
    NvU32                  smpte352;                         // 4-byte SMPTE 352 video payload identifier
    NVVIOSIGNALFORMAT      signalFormat;                     // Signal format
    NVVIOBITSPERCOMPONENT  bitsPerComponent;                 // Bits per component
    NVVIOCOMPONENTSAMPLING samplingFormat;                   // Sampling format
    NVVIOCOLORSPACE        colorSpace;                       // Color space
    NVVIOLINKID            linkID;                           // Link ID
} NVVIOCHANNELSTATUS;

// Input device status
typedef struct _NVVIOINPUTSTATUS
{
    NVVIOCHANNELSTATUS     vidIn[NVAPI_MAX_VIO_JACKS][NVAPI_MAX_VIO_CHANNELS_PER_JACK];     // Video input status per channel within a jack
    NVVIOCAPTURESTATUS     captureStatus;                  // status of video capture
} NVVIOINPUTSTATUS;

// Output device status
typedef struct _NVVIOOUTPUTSTATUS
{
    NVVIOINPUTOUTPUTSTATUS  vid1Out;                        // Video 1 output status
    NVVIOINPUTOUTPUTSTATUS  vid2Out;                        // Video 2 output status
    NVVIOSYNCSTATUS         sdiSyncIn;                      // SDI sync input status
    NVVIOSYNCSTATUS         compSyncIn;                     // Composite sync input status
    NvU32                   syncEnable;                     // Sync enable (TRUE if using syncSource)
    NVVIOSYNCSOURCE         syncSource;                     // Sync source
    NVVIOSIGNALFORMAT       syncFormat;                     // Sync format
    NvU32                   frameLockEnable;                // Framelock enable flag
    NvU32                   outputVideoLocked;              // Output locked status
    NvU32                   dataIntegrityCheckErrorCount;   // Data integrity check error count
    NvU32                   dataIntegrityCheckEnabled;      // Data integrity check status enabled
    NvU32                   dataIntegrityCheckFailed;       // Data integrity check status failed
    NvU32                   uSyncSourceLocked;              // genlocked to framelocked to ref signal
    NvU32                   uPowerOn;                       // TRUE: indicates there is sufficient power
} NVVIOOUTPUTSTATUS;

// Video device status.
typedef struct _NVVIOSTATUS
{
    NvU32                 version;                        // Structure version
    NVVIOSTATUSTYPE       nvvioStatusType;                // Input or Output status
    union
    {
        NVVIOINPUTSTATUS  inStatus;                       //  Input device status
        NVVIOOUTPUTSTATUS outStatus;                      //  Output device status
    }vioStatus;
} NVVIOSTATUS;

#define NVVIOSTATUS_VER   MAKE_NVAPI_VERSION(NVVIOSTATUS,1)

// Output region
typedef struct _NVVIOOUTPUTREGION
{
    NvU32              x;                                    // Horizontal origin in pixels
    NvU32              y;                                    // Vertical origin in pixels
    NvU32              width;                                // Width of region in pixels
    NvU32              height;                               // Height of region in pixels
} NVVIOOUTPUTREGION;

// Gamma ramp (8-bit index)
typedef struct _NVVIOGAMMARAMP8
{
    NvU16              uRed[256];                            // Red channel gamma ramp (8-bit index, 16-bit values)
    NvU16              uGreen[256];                          // Green channel gamma ramp (8-bit index, 16-bit values)
    NvU16              uBlue[256];                           // Blue channel gamma ramp (8-bit index, 16-bit values)
} NVVIOGAMMARAMP8;

// Gamma ramp (10-bit index)
typedef struct _NVVIOGAMMARAMP10
{
    NvU16              uRed[1024];                           // Red channel gamma ramp (10-bit index, 16-bit values)
    NvU16              uGreen[1024];                         // Green channel gamma ramp (10-bit index, 16-bit values)
    NvU16              uBlue[1024];                          // Blue channel gamma ramp (10-bit index, 16-bit values)
} NVVIOGAMMARAMP10;

// Sync delay
typedef struct _NVVIOSYNCDELAY
{
    NvU32              version;                              // Structure version
    NvU32              horizontalDelay;                      // Horizontal delay in pixels
    NvU32              verticalDelay;                        // Vertical delay in lines
} NVVIOSYNCDELAY;

#define NVVIOSYNCDELAY_VER   MAKE_NVAPI_VERSION(NVVIOSYNCDELAY,1)


// Video mode information
typedef struct _NVVIOVIDEOMODE
{
    NvU32                horizontalPixels;                   // Horizontal resolution (in pixels)
    NvU32                verticalLines;                      // Vertical resolution for frame (in lines)
    float                fFrameRate;                         // Frame rate
    NVVIOINTERLACEMODE   interlaceMode;                      // Interlace mode
    NVVIOVIDEOSTANDARD   videoStandard;                      // SMPTE standards format
    NVVIOVIDEOTYPE       videoType;                          // HD or SD signal classification
} NVVIOVIDEOMODE;

// Signal format details
typedef struct _NVVIOSIGNALFORMATDETAIL
{
    NVVIOSIGNALFORMAT    signalFormat;                       // Signal format enumerated value
    NVVIOVIDEOMODE       videoMode;                          // Video mode for signal format
}NVVIOSIGNALFORMATDETAIL;

// Buffer formats
#define NVVIOBUFFERFORMAT_R8G8B8                  0x00000001   // R8:G8:B8
#define NVVIOBUFFERFORMAT_R8G8B8Z24               0x00000002   // R8:G8:B8:Z24
#define NVVIOBUFFERFORMAT_R8G8B8A8                0x00000004   // R8:G8:B8:A8
#define NVVIOBUFFERFORMAT_R8G8B8A8Z24             0x00000008   // R8:G8:B8:A8:Z24
#define NVVIOBUFFERFORMAT_R16FPG16FPB16FP         0x00000010   // R16FP:G16FP:B16FP
#define NVVIOBUFFERFORMAT_R16FPG16FPB16FPZ24      0x00000020   // R16FP:G16FP:B16FP:Z24
#define NVVIOBUFFERFORMAT_R16FPG16FPB16FPA16FP    0x00000040   // R16FP:G16FP:B16FP:A16FP
#define NVVIOBUFFERFORMAT_R16FPG16FPB16FPA16FPZ24 0x00000080   // R16FP:G16FP:B16FP:A16FP:Z24

// Data format details
typedef struct _NVVIODATAFORMATDETAIL
{
    NVVIODATAFORMAT   dataFormat;                              // Data format enumerated value
    NvU32             vioCaps;                                 // Data format capabilities (NVVIOCAPS_* mask)
}NVVIODATAFORMATDETAIL;

// Colorspace conversion
typedef struct _NVVIOCOLORCONVERSION
{
    NvU32       version;                                    //  Structure version
    float       colorMatrix[3][3];                          //  Output[n] =
    float       colorOffset[3];                             //  Input[0] * colorMatrix[n][0] +
    float       colorScale[3];                              //  Input[1] * colorMatrix[n][1] +
                                                            //  Input[2] * colorMatrix[n][2] +
                                                            //  OutputRange * colorOffset[n]
                                                            //  where OutputRange is the standard magnitude of
                                                            //  Output[n][n] and colorMatrix and colorOffset
                                                            //  values are within the range -1.0 to +1.0
    NvU32      compositeSafe;                               //  compositeSafe constrains luminance range when using composite output
} NVVIOCOLORCONVERSION;

#define NVVIOCOLORCONVERSION_VER   MAKE_NVAPI_VERSION(NVVIOCOLORCONVERSION,1)

// Gamma correction
typedef struct _NVVIOGAMMACORRECTION
{
    NvU32            version;                               // Structure version
    NvU32            vioGammaCorrectionType;                // Gamma correction type (8-bit or 10-bit)
    union                                                   // Gamma correction:
    {
        NVVIOGAMMARAMP8  gammaRamp8;                        // Gamma ramp (8-bit index, 16-bit values)
        NVVIOGAMMARAMP10 gammaRamp10;                       // Gamma ramp (10-bit index, 16-bit values)
    }gammaRamp;
    float            fGammaValueR;                          // Red Gamma value within gamma ranges. 0.5 - 6.0
    float            fGammaValueG;                          // Green Gamma value within gamma ranges. 0.5 - 6.0
    float            fGammaValueB;                          // Blue Gamma value within gamma ranges. 0.5 - 6.0
} NVVIOGAMMACORRECTION;

#define NVVIOGAMMACORRECTION_VER   MAKE_NVAPI_VERSION(NVVIOGAMMACORRECTION,1)

#define MAX_NUM_COMPOSITE_RANGE      2                      // maximum number of ranges per channel

typedef struct _NVVIOCOMPOSITERANGE
{
    NvU32   uRange;
    NvU32   uEnabled;
    NvU32   uMin;
    NvU32   uMax;
} NVVIOCOMPOSITERANGE;


// Device configuration (fields masks indicating NVVIOCONFIG fields to use for NvAPI_VIO_GetConfig/NvAPI_VIO_SetConfig() )
#define NVVIOCONFIG_SIGNALFORMAT            0x00000001      // fields: signalFormat
#define NVVIOCONFIG_DATAFORMAT              0x00000002      // fields: dataFormat
#define NVVIOCONFIG_OUTPUTREGION            0x00000004      // fields: outputRegion
#define NVVIOCONFIG_OUTPUTAREA              0x00000008      // fields: outputArea
#define NVVIOCONFIG_COLORCONVERSION         0x00000010      // fields: colorConversion
#define NVVIOCONFIG_GAMMACORRECTION         0x00000020      // fields: gammaCorrection
#define NVVIOCONFIG_SYNCSOURCEENABLE        0x00000040      // fields: syncSource and syncEnable
#define NVVIOCONFIG_SYNCDELAY               0x00000080      // fields: syncDelay
#define NVVIOCONFIG_COMPOSITESYNCTYPE       0x00000100      // fields: compositeSyncType
#define NVVIOCONFIG_FRAMELOCKENABLE         0x00000200      // fields: EnableFramelock
#define NVVIOCONFIG_422FILTER               0x00000400      // fields: bEnable422Filter
#define NVVIOCONFIG_COMPOSITETERMINATE      0x00000800      // fields: bCompositeTerminate        //Not Supported on Quadro FX 4000 SDI
#define NVVIOCONFIG_DATAINTEGRITYCHECK      0x00001000      // fields: bEnableDataIntegrityCheck  //Not Supported on Quadro FX 4000 SDI
#define NVVIOCONFIG_CSCOVERRIDE             0x00002000      // fields: colorConversion override
#define NVVIOCONFIG_FLIPQUEUELENGTH         0x00004000      // fields: flipqueuelength control
#define NVVIOCONFIG_ANCTIMECODEGENERATION   0x00008000      // fields: bEnableANCTimeCodeGeneration
#define NVVIOCONFIG_COMPOSITE               0x00010000      // fields: bEnableComposite
#define NVVIOCONFIG_ALPHAKEYCOMPOSITE       0x00020000      // fields: bEnableAlphaKeyComposite
#define NVVIOCONFIG_COMPOSITE_Y             0x00040000      // fields: compRange
#define NVVIOCONFIG_COMPOSITE_CR            0x00080000      // fields: compRange
#define NVVIOCONFIG_COMPOSITE_CB            0x00100000      // fields: compRange
#define NVVIOCONFIG_FULL_COLOR_RANGE        0x00200000      // fields: bEnableFullColorRange
#define NVVIOCONFIG_RGB_DATA                0x00400000      // fields: bEnableRGBData
#define NVVIOCONFIG_RESERVED_SDIOUTPUTENABLE         0x00800000      // fields: bEnableSDIOutput
#define NVVIOCONFIG_STREAMS                 0x01000000      // fields: streams

// Don't forget to update NVVIOCONFIG_VALIDFIELDS in nvapi.spec when NVVIOCONFIG_ALLFIELDS changes.
#define NVVIOCONFIG_ALLFIELDS   ( NVVIOCONFIG_SIGNALFORMAT          | \
                                  NVVIOCONFIG_DATAFORMAT            | \
                                  NVVIOCONFIG_OUTPUTREGION          | \
                                  NVVIOCONFIG_OUTPUTAREA            | \
                                  NVVIOCONFIG_COLORCONVERSION       | \
                                  NVVIOCONFIG_GAMMACORRECTION       | \
                                  NVVIOCONFIG_SYNCSOURCEENABLE      | \
                                  NVVIOCONFIG_SYNCDELAY             | \
                                  NVVIOCONFIG_COMPOSITESYNCTYPE     | \
                                  NVVIOCONFIG_FRAMELOCKENABLE       | \
                                  NVVIOCONFIG_422FILTER             | \
                                  NVVIOCONFIG_COMPOSITETERMINATE    | \
                                  NVVIOCONFIG_DATAINTEGRITYCHECK    | \
                                  NVVIOCONFIG_CSCOVERRIDE           | \
                                  NVVIOCONFIG_FLIPQUEUELENGTH       | \
                                  NVVIOCONFIG_ANCTIMECODEGENERATION | \
                                  NVVIOCONFIG_COMPOSITE             | \
                                  NVVIOCONFIG_ALPHAKEYCOMPOSITE     | \
                                  NVVIOCONFIG_COMPOSITE_Y           | \
                                  NVVIOCONFIG_COMPOSITE_CR          | \
                                  NVVIOCONFIG_COMPOSITE_CB          | \
                                  NVVIOCONFIG_FULL_COLOR_RANGE      | \
                                  NVVIOCONFIG_RGB_DATA              | \
                                  NVVIOCONFIG_RESERVED_SDIOUTPUTENABLE | \
                                  NVVIOCONFIG_STREAMS)

#define NVVIOCONFIG_VALIDFIELDS  ( NVVIOCONFIG_SIGNALFORMAT          | \
                                   NVVIOCONFIG_DATAFORMAT            | \
                                   NVVIOCONFIG_OUTPUTREGION          | \
                                   NVVIOCONFIG_OUTPUTAREA            | \
                                   NVVIOCONFIG_COLORCONVERSION       | \
                                   NVVIOCONFIG_GAMMACORRECTION       | \
                                   NVVIOCONFIG_SYNCSOURCEENABLE      | \
                                   NVVIOCONFIG_SYNCDELAY             | \
                                   NVVIOCONFIG_COMPOSITESYNCTYPE     | \
                                   NVVIOCONFIG_FRAMELOCKENABLE       | \
                                   NVVIOCONFIG_RESERVED_SDIOUTPUTENABLE | \
                                   NVVIOCONFIG_422FILTER             | \
                                   NVVIOCONFIG_COMPOSITETERMINATE    | \
                                   NVVIOCONFIG_DATAINTEGRITYCHECK    | \
                                   NVVIOCONFIG_CSCOVERRIDE           | \
                                   NVVIOCONFIG_FLIPQUEUELENGTH       | \
                                   NVVIOCONFIG_ANCTIMECODEGENERATION | \
                                   NVVIOCONFIG_COMPOSITE             | \
                                   NVVIOCONFIG_ALPHAKEYCOMPOSITE     | \
                                   NVVIOCONFIG_COMPOSITE_Y           | \
                                   NVVIOCONFIG_COMPOSITE_CR          | \
                                   NVVIOCONFIG_COMPOSITE_CB          | \
                                   NVVIOCONFIG_FULL_COLOR_RANGE      | \
                                   NVVIOCONFIG_RGB_DATA              | \
                                   NVVIOCONFIG_RESERVED_SDIOUTPUTENABLE | \
                                   NVVIOCONFIG_STREAMS)

#define NVVIOCONFIG_DRIVERFIELDS ( NVVIOCONFIG_OUTPUTREGION          | \
                                   NVVIOCONFIG_OUTPUTAREA            | \
                                   NVVIOCONFIG_COLORCONVERSION       | \
                                   NVVIOCONFIG_FLIPQUEUELENGTH)

#define NVVIOCONFIG_GAMMAFIELDS  ( NVVIOCONFIG_GAMMACORRECTION       )

#define NVVIOCONFIG_RMCTRLFIELDS ( NVVIOCONFIG_SIGNALFORMAT          | \
                                   NVVIOCONFIG_DATAFORMAT            | \
                                   NVVIOCONFIG_SYNCSOURCEENABLE      | \
                                   NVVIOCONFIG_COMPOSITESYNCTYPE     | \
                                   NVVIOCONFIG_FRAMELOCKENABLE       | \
                                   NVVIOCONFIG_422FILTER             | \
                                   NVVIOCONFIG_COMPOSITETERMINATE    | \
                                   NVVIOCONFIG_DATAINTEGRITYCHECK    | \
                                   NVVIOCONFIG_COMPOSITE             | \
                                   NVVIOCONFIG_ALPHAKEYCOMPOSITE     | \
                                   NVVIOCONFIG_COMPOSITE_Y           | \
                                   NVVIOCONFIG_COMPOSITE_CR          | \
                                   NVVIOCONFIG_COMPOSITE_CB)

#define NVVIOCONFIG_RMSKEWFIELDS ( NVVIOCONFIG_SYNCDELAY             )

#define NVVIOCONFIG_ALLOWSDIRUNNING_FIELDS ( NVVIOCONFIG_DATAINTEGRITYCHECK     | \
                                             NVVIOCONFIG_SYNCDELAY              | \
                                             NVVIOCONFIG_CSCOVERRIDE            | \
                                             NVVIOCONFIG_ANCTIMECODEGENERATION  | \
                                             NVVIOCONFIG_COMPOSITE              | \
                                             NVVIOCONFIG_ALPHAKEYCOMPOSITE      | \
                                             NVVIOCONFIG_COMPOSITE_Y            | \
                                             NVVIOCONFIG_COMPOSITE_CR           | \
                                             NVVIOCONFIG_COMPOSITE_CB)

 #define NVVIOCONFIG_RMMODESET_FIELDS ( NVVIOCONFIG_SIGNALFORMAT         | \
                                        NVVIOCONFIG_DATAFORMAT           | \
                                        NVVIOCONFIG_SYNCSOURCEENABLE     | \
                                        NVVIOCONFIG_FRAMELOCKENABLE      | \
                                        NVVIOCONFIG_COMPOSITESYNCTYPE )


// Output device configuration
// No members can be deleted from below structure. Only add new members at the
// end of the structure
typedef struct _NVVIOOUTPUTCONFIG
{
    NVVIOSIGNALFORMAT    signalFormat;                         // Signal format for video output
    NVVIODATAFORMAT      dataFormat;                           // Data format for video output
    NVVIOOUTPUTREGION    outputRegion;                         // Region for video output (Desktop mode)
    NVVIOOUTPUTAREA      outputArea;                           // Usable resolution for video output (safe area)
    NVVIOCOLORCONVERSION colorConversion;                      // Color conversion.
    NVVIOGAMMACORRECTION gammaCorrection;
    NvU32                syncEnable;                           // Sync enable (TRUE to use syncSource)
    NVVIOSYNCSOURCE      syncSource;                           // Sync source
    NVVIOSYNCDELAY       syncDelay;                            // Sync delay
    NVVIOCOMPSYNCTYPE    compositeSyncType;                    // Composite sync type
    NvU32                frameLockEnable;                      // Flag indicating whether framelock was on/off
    NvU32                psfSignalFormat;                      // Inidcates whether contained format is PSF Signal format
    NvU32                enable422Filter;                      // Enables/Disables 4:2:2 filter
    NvU32                compositeTerminate;                   // Composite termination
    NvU32                enableDataIntegrityCheck;             // Enable data integrity check: true - enable, false - disable
    NvU32                cscOverride;                          // Use provided CSC color matrix to overwrite
    NvU32                flipQueueLength;                      // Number of buffers used for the internal flipqueue
    NvU32                enableANCTimeCodeGeneration;          // Enable SDI ANC time code generation
    NvU32                enableComposite;                      // Enable composite
    NvU32                enableAlphaKeyComposite;              // Enable Alpha key composite
    NVVIOCOMPOSITERANGE  compRange;                            // Composite ranges
    NvU8                 reservedData[256];                    // Inicates last stored SDI output state TRUE-ON / FALSE-OFF
    NvU32                enableFullColorRange;                 // Flag indicating Full Color Range
    NvU32                enableRGBData;                        // Indicates data is in RGB format
} NVVIOOUTPUTCONFIG;

// Stream configuration
typedef struct _NVVIOSTREAM
{
    NvU32                   bitsPerComponent;                     // Bits per component
    NVVIOCOMPONENTSAMPLING  sampling;                             // Sampling
    NvU32                   expansionEnable;                      // Enable/disable 4:2:2->4:4:4 expansion
    NvU32                   numLinks;                             // Number of active links
    struct
    {
        NvU32               jack;                                 // This stream's link[i] will use the specified (0-based) channel within the
        NvU32               channel;                              // specified (0-based) jack
    } links[NVAPI_MAX_VIO_LINKS_PER_STREAM];
} NVVIOSTREAM;

// Input device configuration
typedef struct _NVVIOINPUTCONFIG
{
    NvU32                numRawCaptureImages;                  // numRawCaptureImages is the number of frames to keep in the capture queue.
                                                               // must be between NVAPI_GVI_MIN_RAW_CAPTURE_IMAGES and NVAPI_GVI_MAX_RAW_CAPTURE_IMAGES,
    NVVIOSIGNALFORMAT    signalFormat;                         // Signal format.
                                                               // Please note that both numRawCaptureImages and signalFormat should be set together.
    NvU32                numStreams;                           // Number of active streams.
    NVVIOSTREAM          streams[NVAPI_MAX_VIO_STREAMS];       // Stream configurations
    NvU32                bTestMode;                            // This attribute controls the GVI test mode. Possible values 0/1.
                                                               // When testmode enabled, the GVI device will generate fake data as quickly as possible.
} NVVIOINPUTCONFIG;

typedef struct _NVVIOCONFIG
{
    NvU32                version;                              // Structure version
    NvU32                fields;                               // Caller sets to NVVIOCONFIG_* mask for fields to use
    NVVIOCONFIGTYPE      nvvioConfigType;                      // Input or Output configuration
    union
    {
        NVVIOINPUTCONFIG  inConfig;                            //  Input device configuration
        NVVIOOUTPUTCONFIG outConfig;                           //  Output device configuration
    }vioConfig;
} NVVIOCONFIG;

#define NVVIOCONFIG_VER   MAKE_NVAPI_VERSION(NVVIOCONFIG,1)

typedef struct
{
    NvPhysicalGpuHandle                 hPhysicalGpu;                   //handle to Physical GPU (This could be NULL for GVI device if its not binded)
    NvVioHandle                         hVioHandle;                     //handle to SDI Input/Output device
    NvU32                               vioId;                          //device Id of SDI Input/Output device
    NvU32                               outputId;                       //deviceMask of the SDI display connected to GVO device.
                                                                        //outputId will be 0 for GVI device.
} NVVIOTOPOLOGYTARGET;

typedef struct _NV_VIO_TOPOLOGY
{
    NvU32                       version;
    NvU32                       vioTotalDeviceCount;                    //How many vio targets are valid
    NVVIOTOPOLOGYTARGET         vioTarget[NVAPI_MAX_VIO_DEVICES];       //Array of vio targets
}NV_VIO_TOPOLOGY, NVVIOTOPOLOGY;

#define NV_VIO_TOPOLOGY_VER  MAKE_NVAPI_VERSION(NV_VIO_TOPOLOGY,1)
#define NVVIOTOPOLOGY_VER    MAKE_NVAPI_VERSION(NVVIOTOPOLOGY,1)

//---------------------------------------------------------------------
// Function:    NvAPI_VIO_GetCapabilities
//
// Description: Determine graphics adapter video I/O capabilities.
//
//  SUPPORTED OS: Windows XP and higher
//
// Parameters:  NvVioHandle[IN] - The caller provides the SDI device handle as input.
//              pAdapterCaps[OUT] - Pointer to receive capabilities
//
// Returns:     NVAPI_OK                          - Success
//              NVAPI_API_NOT_INTIALIZED          - NVAPI Not Initialized
//              NVAPI_INVALID_ARGUMENT            - Arguments passed to API are not valid
//              NVAPI_INCOMPATIBLE_STRUCT_VERSION - NVVIOCAPS struct version used by the app is not compatible
//              NVAPI_NOT_SUPPORTED                - Video I/O not supported
//              NVAPI_ERROR                       - NVAPI Random errors
//---------------------------------------------------------------------
NVAPI_INTERFACE NvAPI_VIO_GetCapabilities(NvVioHandle     hVioHandle,
                                          NVVIOCAPS       *pAdapterCaps);

//---------------------------------------------------------------------
// Function:    NvAPI_VIO_Open
//
// Description: Open graphics adapter for video I/O operations
//              using the OpenGL application interface.  Read operations
//              are permitted in this mode by multiple clients, but Write
//              operations are application exclusive.
//
//  SUPPORTED OS: Windows XP and higher
//
// Parameters:  NvVioHandle[IN]     - The caller provides the SDI output device handle as input.
//              vioClass[IN]        - Class interface (NVVIOCLASS_* value)
//              ownerType[IN]       - user should specify the ownerType ( NVVIOOWNERTYPE_APPLICATION or NVVIOOWNERTYPE_DESKTOP)
//
// Returns:     NVAPI_OK                          - Success
//              NVAPI_API_NOT_INTIALIZED          - NVAPI Not Initialized
//              NVAPI_INVALID_ARGUMENT            - Arguments passed to API are not valid
//              NVAPI_NOT_SUPPORTED                - Video I/O not supported
//              NVAPI_ERROR                       - NVAPI Random errors
//              NVAPI_DEVICE_BUSY                 - Access denied for requested access
//---------------------------------------------------------------------
NVAPI_INTERFACE NvAPI_VIO_Open(NvVioHandle       hVioHandle,
                               NvU32             vioClass,
                               NVVIOOWNERTYPE    ownerType);

//---------------------------------------------------------------------
// Function:    NvAPI_VIO_Close
//
// Description: Closes graphics adapter for Graphics to Video operations
//              using the OpenGL application interface.  Closing an
//              OpenGL handle releases the device.
//
//  SUPPORTED OS: Windows XP and higher
//
// Parameters:  NvVioHandle[IN]  - The caller provides the SDI output device handle as input.
//              bRelease         - boolean value to decide on keeping or releasing ownership
//
// Returns:     NVAPI_OK                          - Success
//              NVAPI_API_NOT_INTIALIZED          - NVAPI Not Initialized
//              NVAPI_INVALID_ARGUMENT            - Arguments passed to API are not valid
//              NVAPI_NOT_SUPPORTED                - Video I/O not supported
//              NVAPI_ERROR                       - NVAPI Random errors
//              NVAPI_DEVICE_BUSY                 - Access denied for requested access
//---------------------------------------------------------------------
NVAPI_INTERFACE NvAPI_VIO_Close(NvVioHandle       hVioHandle,
                                NvU32             bRelease);

//---------------------------------------------------------------------
// Function:    NvAPI_VIO_Status
//
// Description: Get Video I/O LED status.
//
//  SUPPORTED OS: Windows XP and higher
//
// Parameters:  NvVioHandle[IN] - The caller provides the SDI device handle as input.
//              pStatus(OUT)   - returns pointer to the NVVIOSTATUS
//
// Returns:     NVAPI_OK                          - Success
//              NVAPI_API_NOT_INTIALIZED          - NVAPI Not Initialized
//              NVAPI_INVALID_ARGUMENT            - Arguments passed to API are not valid
//              NVAPI_INCOMPATIBLE_STRUCT_VERSION - Invalid structure version
//              NVAPI_NOT_SUPPORTED               - Video I/O not supported
//              NVAPI_ERROR                       - NVAPI Random errors
//---------------------------------------------------------------------
NVAPI_INTERFACE NvAPI_VIO_Status(NvVioHandle     hVioHandle,
                                 NVVIOSTATUS     *pStatus);

//---------------------------------------------------------------------
// Function:    NvAPI_VIO_SyncFormatDetect
//
// Description: Detects Video I/O incoming sync video format.
//
//  SUPPORTED OS: Windows XP and higher
//
// Parameters:  NvVioHandle[IN] - The caller provides the SDI device handle as input.
//              pWait(OUT)     - Pointer to receive milliseconds to wait
//                               before VIOStatus will return detected
//                               syncFormat.
//
// Returns:     NVAPI_OK                          - Success
//              NVAPI_API_NOT_INTIALIZED          - NVAPI Not Initialized
//              NVAPI_INVALID_ARGUMENT            - Arguments passed to API are not valid
//              NVAPI_NOT_SUPPORTED               - Video I/O not supported
//              NVAPI_ERROR                       - NVAPI Random errors
//---------------------------------------------------------------------
NVAPI_INTERFACE NvAPI_VIO_SyncFormatDetect(NvVioHandle hVioHandle,
                                           NvU32       *pWait);

//---------------------------------------------------------------------
// Function:    NvAPI_VIO_GetConfig
//
// Description: Get Graphics to Video configuration.
//
//  SUPPORTED OS: Windows XP and higher
//
// Parameters:  NvVioHandle[IN] - The caller provides the SDI device handle as input.
//              pConfig(OUT)    - Pointer to Graphics to Video configuration
//
// Returns:     NVAPI_OK                          - Success
//              NVAPI_API_NOT_INTIALIZED          - NVAPI Not Initialized
//              NVAPI_INVALID_ARGUMENT            - Arguments passed to API are not valid
//              NVAPI_INCOMPATIBLE_STRUCT_VERSION - Invalid structure version
//              NVAPI_NOT_SUPPORTED               - Video I/O not supported
//              NVAPI_ERROR                       - NVAPI Random errors
//---------------------------------------------------------------------
NVAPI_INTERFACE NvAPI_VIO_GetConfig(NvVioHandle        hVioHandle,
                                    NVVIOCONFIG        *pConfig);

//---------------------------------------------------------------------
// Function:    NvAPI_VIO_SetConfig
//
// Description: Set Graphics to Video configuration.
//
//  SUPPORTED OS: Windows XP and higher
//
// Parameters:  NvVioHandle[IN]     - The caller provides the SDI device handle as input.
//              pConfig(IN)         - Pointer to Graphics to Video config
//
// Returns:     NVAPI_OK                          - Success
//              NVAPI_API_NOT_INTIALIZED          - NVAPI Not Initialized
//              NVAPI_INVALID_ARGUMENT            - Arguments passed to API are not valid
//              NVAPI_INCOMPATIBLE_STRUCT_VERSION - Stucture version invalid
//              NVAPI_NOT_SUPPORTED               - Video I/O not supported
//              NVAPI_ERROR                       - NVAPI Random errors
//              NVAPI_DEVICE_BUSY                 - Access denied for requested access
//---------------------------------------------------------------------
NVAPI_INTERFACE NvAPI_VIO_SetConfig(NvVioHandle            hVioHandle,
                                    const NVVIOCONFIG      *pConfig);

//---------------------------------------------------------------------
// Function:    NvAPI_VIO_SetCSC
//
// Description: Set colorspace conversion parameters.
//
//  SUPPORTED OS: Windows XP and higher
//
// Parameters:  NvVioHandle[IN]     - The caller provides the SDI device handle as input.
//              pCSC(IN)            - Pointer to CSC parameters
//
// Returns:     NVAPI_OK                          - Success
//              NVAPI_API_NOT_INTIALIZED          - NVAPI Not Initialized
//              NVAPI_INVALID_ARGUMENT            - Arguments passed to API are not valid
//              NVAPI_INCOMPATIBLE_STRUCT_VERSION - Stucture version invalid
//              NVAPI_NOT_SUPPORTED               - Video I/O not supported
//              NVAPI_ERROR                       - NVAPI Random errors
//              NVAPI_DEVICE_BUSY                 - Access denied for requested access
//---------------------------------------------------------------------
NVAPI_INTERFACE NvAPI_VIO_SetCSC(NvVioHandle           hVioHandle,
                                 NVVIOCOLORCONVERSION  *pCSC);

//---------------------------------------------------------------------
// Function:    NvAPI_VIO_GetCSC
//
// Description: Get colorspace conversion parameters.
//
//  SUPPORTED OS: Windows XP and higher
//
// Parameters:  NvVioHandle[IN]     - The caller provides the SDI device handle as input.
//              pCSC(OUT)           - Pointer to CSC parameters
//
// Returns:     NVAPI_OK                          - Success
//              NVAPI_API_NOT_INTIALIZED          - NVAPI Not Initialized
//              NVAPI_INVALID_ARGUMENT            - Arguments passed to API are not valid
//              NVAPI_INCOMPATIBLE_STRUCT_VERSION - Stucture version invalid
//              NVAPI_NOT_SUPPORTED               - Video I/O not supported
//              NVAPI_ERROR                       - NVAPI Random errors
//---------------------------------------------------------------------
NVAPI_INTERFACE NvAPI_VIO_GetCSC(NvVioHandle           hVioHandle,
                                 NVVIOCOLORCONVERSION  *pCSC);

//---------------------------------------------------------------------
// Function:    NvAPI_VIO_SetGamma
//
// Description: Set gamma conversion parameters.
//
//  SUPPORTED OS: Windows XP and higher
//
// Parameters:  NvVioHandle[IN]     - The caller provides the SDI device handle as input.
//              pGamma(IN)          - Pointer to gamma parameters
//
// Returns:     NVAPI_OK                          - Success
//              NVAPI_API_NOT_INTIALIZED          - NVAPI Not Initialized
//              NVAPI_INVALID_ARGUMENT            - Arguments passed to API are not valid
//              NVAPI_INCOMPATIBLE_STRUCT_VERSION - Stucture version invalid
//              NVAPI_NOT_SUPPORTED               - Video I/O not supported
//              NVAPI_ERROR                       - NVAPI Random errors
//              NVAPI_DEVICE_BUSY                 - Access denied for requested access
//---------------------------------------------------------------------
NVAPI_INTERFACE NvAPI_VIO_SetGamma(NvVioHandle           hVioHandle,
                                   NVVIOGAMMACORRECTION  *pGamma);

//---------------------------------------------------------------------
// Function:    NvAPI_VIO_GetGamma
//
// Description: Get gamma conversion parameters.
//
//  SUPPORTED OS: Windows XP and higher
//
// Parameters:  NvVioHandle[IN]     - The caller provides the SDI device handle as input.
//              pGamma(OUT)         - Pointer to gamma parameters
//
// Returns:     NVAPI_OK                          - Success
//              NVAPI_API_NOT_INTIALIZED          - NVAPI Not Initialized
//              NVAPI_INVALID_ARGUMENT            - Arguments passed to API are not valid
//              NVAPI_INCOMPATIBLE_STRUCT_VERSION - Stucture version invalid
//              NVAPI_NOT_SUPPORTED               - Video I/O not supported
//              NVAPI_ERROR                       - NVAPI Random errors
//---------------------------------------------------------------------
NVAPI_INTERFACE NvAPI_VIO_GetGamma(NvVioHandle           hVioHandle,
                                   NVVIOGAMMACORRECTION* pGamma);

//---------------------------------------------------------------------
// Function:    NvAPI_VIO_SetSyncDelay
//
// Description: Set sync delay parameters.
//
//  SUPPORTED OS: Windows XP and higher
//
// Parameters:  NvVioHandle[IN] - The caller provides the SDI device handle as input.
//              pSyncDelay(IN)  - const Pointer to sync delay parameters
//
// Returns:     NVAPI_OK                          - Success
//              NVAPI_API_NOT_INTIALIZED          - NVAPI Not Initialized
//              NVAPI_INVALID_ARGUMENT            - Arguments passed to API are not valid
//              NVAPI_INCOMPATIBLE_STRUCT_VERSION - Stucture version invalid
//              NVAPI_ERROR                       - NVAPI Random errors
//              NVAPI_DEVICE_BUSY                 - Access denied for requested access
//---------------------------------------------------------------------
NVAPI_INTERFACE NvAPI_VIO_SetSyncDelay(NvVioHandle            hVioHandle,
                                       const NVVIOSYNCDELAY   *pSyncDelay);

//---------------------------------------------------------------------
// Function:    NvAPI_VIO_GetSyncDelay
//
// Description: Get sync delay parameters.
//
//  SUPPORTED OS: Windows XP and higher
//
// Parameters:  NvVioHandle[IN]     - The caller provides the SDI device handle as input.
//              pSyncDelay(OUT)     - Pointer to sync delay parameters
//
// Returns:     NVAPI_OK                          - Success
//              NVAPI_API_NOT_INTIALIZED          - NVAPI Not Initialized
//              NVAPI_INVALID_ARGUMENT            - Arguments passed to API are not valid
//              NVAPI_INCOMPATIBLE_STRUCT_VERSION - Stucture version invalid
//              NVAPI_ERROR                       - NVAPI Random errors
//---------------------------------------------------------------------
NVAPI_INTERFACE NvAPI_VIO_GetSyncDelay(NvVioHandle      hVioHandle,
                                       NVVIOSYNCDELAY   *pSyncDelay);

//---------------------------------------------------------------------
// Function:    NvAPI_VIO_IsRunning
//
// Description: Determine if Video I/O is running.
//
//  SUPPORTED OS: Windows XP and higher
//
// Parameters:  NvVioHandle[IN]            - The caller provides the SDI device handle as input.
//
// Returns:     NVAPI_DRIVER_RUNNING       - Video I/O running
//              NVAPI_DRIVER_NOTRUNNING    - Video I/O not running
//---------------------------------------------------------------------
NVAPI_INTERFACE NvAPI_VIO_IsRunning(NvVioHandle   hVioHandle);

//---------------------------------------------------------------------
// Function:    NvAPI_VIO_Start
//
// Description: Start Video I/O.
//              This API should be called for NVVIOOWNERTYPE_DESKTOP only and will not work for OGL applications.
//
//  SUPPORTED OS: Windows XP and higher
//
// Parameters:  NvVioHandle[IN]    - The caller provides the SDI device handle as input.
//
// Returns:     NVAPI_OK                          - Success
//              NVAPI_API_NOT_INTIALIZED          - NVAPI Not Initialized
//              NVAPI_INVALID_ARGUMENT            - Arguments passed to API are not valid
//              NVAPI_NOT_SUPPORTED               - Video I/O not supported
//              NVAPI_ERROR                       - NVAPI Random errors
//              NVAPI_DEVICE_BUSY                 - Access denied for requested access
//---------------------------------------------------------------------
NVAPI_INTERFACE NvAPI_VIO_Start(NvVioHandle     hVioHandle);

//---------------------------------------------------------------------
// Function:    NvAPI_VIO_Stop
//
// Description: Stop Video I/O.
//              This API should be called for NVVIOOWNERTYPE_DESKTOP only and will not work for OGL applications.
//
//  SUPPORTED OS: Windows XP and higher
//
// Parameters:  NvVioHandle[IN]    - The caller provides the SDI device handle as input.
//
// Returns:     NVAPI_OK                          - Success
//              NVAPI_API_NOT_INTIALIZED          - NVAPI Not Initialized
//              NVAPI_INVALID_ARGUMENT            - Arguments passed to API are not valid
//              NVAPI_NOT_SUPPORTED               - Video I/O not supported
//              NVAPI_ERROR                       - NVAPI Random errors
//              NVAPI_DEVICE_BUSY                 - Access denied for requested access
//---------------------------------------------------------------------
NVAPI_INTERFACE NvAPI_VIO_Stop(NvVioHandle     hVioHandle);


//---------------------------------------------------------------------
// Function:    NvAPI_VIO_IsFrameLockModeCompatible
//
// Description: Checkes whether modes are compatible in framelock mode
//
//  SUPPORTED OS: Windows XP and higher
//
// Parameters:  NvVioHandle[IN]         - The caller provides the SDI device handle as input.
//              srcEnumIndex(IN)        - Source Enumeration index
//              destEnumIndex(IN)       - Destination Enumeration index
//              pbCompatible(OUT)       - Pointer to receive compatability
//
// Returns:     NVAPI_OK                          - Success
//              NVAPI_API_NOT_INTIALIZED          - NVAPI Not Initialized
//              NVAPI_INVALID_ARGUMENT            - Arguments passed to API are not valid
//              NVAPI_NOT_SUPPORTED               - Video I/O not supported
//              NVAPI_ERROR                       - NVAPI Random errors
//---------------------------------------------------------------------
NVAPI_INTERFACE NvAPI_VIO_IsFrameLockModeCompatible(NvVioHandle              hVioHandle,
                                                    NvU32                    srcEnumIndex,
                                                    NvU32                    destEnumIndex,
                                                    NvU32*                   pbCompatible);

//---------------------------------------------------------------------
// Function:    NvAPI_VIO_EnumDevices
//
// Description: Enumerate all VIO devices connected to the system.
//
//  SUPPORTED OS: Windows XP and higher
//
// Parameters:  NvVioHandle[OUT]                  - User passes the pointer of NvVioHandle[] array to get handles to all the connected vio devices.
//              vioDeviceCount[OUT]               - User gets total number of VIO devices connected to the system.
//
// Returns:     NVAPI_OK                          - Success
//              NVAPI_API_NOT_INTIALIZED          - NVAPI Not Initialized
//              NVAPI_INVALID_ARGUMENT            - Arguments passed to API are not valid
//              NVAPI_ERROR                       - NVAPI Random errors
//              NVAPI_NVIDIA_DEVICE_NOT_FOUND     - No SDI Device found
//---------------------------------------------------------------------
NVAPI_INTERFACE NvAPI_VIO_EnumDevices(NvVioHandle       hVioHandle[NVAPI_MAX_VIO_DEVICES],
                                      NvU32             *vioDeviceCount);


//---------------------------------------------------------------------
// Function:    NvAPI_VIO_QueryTopology
//
// Description: Enumerate all valid SDI topologies
//
//  SUPPORTED OS: Windows XP and higher
//
// Parameters:  pNvVIOTopology[OUT]       - User passes the pointer to NVVIOTOPOLOGY to fetch all valid SDI Topologies.
//
// Returns:     NVAPI_OK                          - Success
//              NVAPI_API_NOT_INTIALIZED          - NVAPI Not Initialized
//              NVAPI_INVALID_ARGUMENT            - Arguments passed to API are not valid
//              NVAPI_INCOMPATIBLE_STRUCT_VERSION - Invalid structure version
//              NVAPI_ERROR                       - NVAPI Random errors
//---------------------------------------------------------------------
NVAPI_INTERFACE NvAPI_VIO_QueryTopology(NV_VIO_TOPOLOGY   *pNvVIOTopology);


//---------------------------------------------------------------------
// Function:    NvAPI_VIO_EnumSignalFormats
//
// Description: Enumerate signal formats supported by Video I/O.
//
//  SUPPORTED OS: Windows XP and higher
//
// Parameters:  NvVioHandle[IN]          - The caller provides the SDI device handle as input.
//              enumIndex(IN)            - Enumeration index
//              pSignalFormatDetail(OUT) - Pointer to receive detail or NULL
//
// Returns:     NVAPI_OK                 - Success
//              NVAPI_API_NOT_INTIALIZED - NVAPI not initialized
//              NVAPI_INVALID_ARGUMENT   - Invalid argument passed
//              NVAPI_END_ENUMERATION    - No more signal formats to enumerate
//---------------------------------------------------------------------
NVAPI_INTERFACE NvAPI_VIO_EnumSignalFormats(NvVioHandle              hVioHandle,
                                            NvU32                    enumIndex,
                                            NVVIOSIGNALFORMATDETAIL  *pSignalFormatDetail);

//---------------------------------------------------------------------
// Function:    NvAPI_VIO_EnumDataFormats
//
// Description: Enumerate data formats supported by Video I/O.
//
//  SUPPORTED OS: Windows XP and higher
//
// Parameters:  NvVioHandle[IN]        - The caller provides the SDI device handle as input.
//              enumIndex(IN)          - Enumeration index
//              pDataFormatDetail(OUT) - Pointer to receive detail or NULL
//
// Returns:     NVAPI_OK               - Success
//              NVAPI_END_ENUMERATION  - No more data formats to enumerate
//              NVAPI_NOT_SUPPORTED    - Unsupported NVVIODATAFORMAT_ enumeration
//---------------------------------------------------------------------
NVAPI_INTERFACE NvAPI_VIO_EnumDataFormats(NvVioHandle            hVioHandle,
                                          NvU32                  enumIndex,
                                          NVVIODATAFORMATDETAIL  *pDataFormatDetail);


//  SUPPORTED OS: Windows XP and higher
///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_GPU_GetTachReading
//
//   DESCRIPTION: This retrieves the tachometer reading for fan speed for the specified physical GPU.
//
//   PARAMETERS:   hPhysicalGpu(IN) - GPU selection.
//                 pValue(OUT)      - Pointer to a variable to get the tachometer reading
//   HOW TO USE:   NvU32 Value = 0;
//                 ret = NvAPI_GPU_GetTachReading(hPhysicalGpu, &Value);
//                 On call success:
//                 Value would contain the tachometer reading
//
// RETURN STATUS:
//    NVAPI_OK - completed request
//    NVAPI_ERROR - miscellaneous error occurred
//    NVAPI_NOT_SUPPORTED - functionality not supported
//    NVAPI_API_NOT_INTIALIZED - nvapi not initialized
//    NVAPI_INVALID_ARGUMENT - invalid argument passed
//    NVAPI_HANDLE_INVALIDATED - handle passed has been invalidated (see user guide)
//    NVAPI_EXPECTED_PHYSICAL_GPU_HANDLE - handle passed is not a physical GPU handle
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_GPU_GetTachReading(NvPhysicalGpuHandle hPhysicalGPU, NvU32 *pValue);


//
// NV_GET_SCALING_CAPS
//
// Interface structure used in NvAPI_GetScalingCaps call.
//
// This NvAPI_GetScalingCaps returns scaling capability info for the specified display device
//


///////////////////////////////////////////////////////////////////////////////
// FUNCTION NAME:   NvAPI_SYS_GetDisplayIdFromGpuAndOutputId
//
// DESCRIPTION:     Converts a Physical GPU handle and an output Id to a
//                  displayId
//
//  SUPPORTED OS: Windows XP and higher
//
// PARAMETERS:      hPhysicalGpu(IN)  - Handle to the physical GPU
//                  outputId(IN)      - Connected display output Id on the
//                                      target GPU, must only have one bit
//                                      set.
//                  displayId(OUT)    - Pointer to an NvU32 which will contain
//                                      the display ID
//
// RETURN STATUS:
//                  NVAPI_OK - completed request
//                  NVAPI_API_NOT_INTIALIZED - NVAPI not initialized
//                  NVAPI_ERROR - miscellaneous error occurred
//                  NVAPI_INVALID_ARGUMENT - Invalid input parameter.
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_SYS_GetDisplayIdFromGpuAndOutputId(NvPhysicalGpuHandle hPhysicalGpu, NvU32 outputId, NvU32* displayId);

///////////////////////////////////////////////////////////////////////////////
// FUNCTION NAME:   NvAPI_SYS_GetGpuAndOutputIdFromDisplayId
//
// DESCRIPTION:     Converts a displayId to a Physical GPU handle and an outputId
//
//  SUPPORTED OS: Windows XP and higher
//
// PARAMETERS:      displayId(IN)     - Display ID of display to retrieve
//                                      GPU and outputId for
//                  hPhysicalGpu(OUT) - Handle to the physical GPU
//                  outputId(OUT)     - Connected display output Id on the
//                                      target GPU, will only have one bit
//                                      set.
//
// RETURN STATUS:
//                  NVAPI_OK - completed request
//                  NVAPI_API_NOT_INTIALIZED - NVAPI not initialized
//                  NVAPI_ID_OUT_OF_RANGE - The DisplayId corresponds to a
//                                          display which is not within the
//                                          normal outputId range.
//                  NVAPI_ERROR - miscellaneous error occurred
//                  NVAPI_INVALID_ARGUMENT - Invalid input parameter.
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_SYS_GetGpuAndOutputIdFromDisplayId(NvU32 displayId, NvPhysicalGpuHandle *hPhysicalGpu, NvU32 *outputId);

//  SUPPORTED OS: Windows XP and higher
///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME:   NvAPI_DISP_GetDisplayIdByDisplayName
//
// DESCRIPTION:     This API retrieves the Display Id of a given display by
//                  display name. The display must be active to retrieve the
//                  displayId. In the case of clone mode or Surround gaming,
//                  the primary or top-left display will be returned.
//
// PARAMETERS:      displayName(IN):  Name of display (Eg: "\\DISPLAY1" to
//                                    retrieve the displayId for.
//                  displayId(OUT):   Display ID of the requested display.
//
// RETURN STATUS    NVAPI_OK:                          Capabilties have been returned.
//                  NVAPI_INVALID_ARGUMENT:            One or more args passed in are invalid.
//                  NVAPI_API_NOT_INTIALIZED:          The NvAPI API needs to be initialized first
//                  NVAPI_NO_IMPLEMENTATION:           This entrypoint not available
//                  NVAPI_ERROR:                       Miscellaneous error occurred
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_DISP_GetDisplayIdByDisplayName(const char *displayName, NvU32* displayId);

//  SUPPORTED OS: Windows XP and higher
///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME:   NvAPI_DISP_GetGDIPrimaryDisplayId
//
// DESCRIPTION:     This API returns the Display ID of the GDI Primary.
//
// PARAMETERS:      displayId(OUT):   Display ID of the GDI Primary display.
//
// RETURN STATUS    NVAPI_OK:                          Capabilties have been returned.
//                  NVAPI_NVIDIA_DEVICE_NOT_FOUND:     GDI Primary not on an NVIDIA GPU.
//                  NVAPI_INVALID_ARGUMENT:            One or more args passed in are invalid.
//                  NVAPI_API_NOT_INTIALIZED:          The NvAPI API needs to be initialized first
//                  NVAPI_NO_IMPLEMENTATION:           This entrypoint not available
//                  NVAPI_ERROR:                       Miscellaneous error occurred
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_DISP_GetGDIPrimaryDisplayId(NvU32* displayId);


///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME:   NvAPI_GPU_GetECCStatusInfo
//
// PARAMETERS:      hPhysicalGpu (IN) - A handle identifying the physical GPU for
//                                      which ECC status information is to be
//                                      retrieved.
//                  pECCStatusInfo (OUT) - A pointer to an ECC status structure.
//
//  SUPPORTED OS: Windows XP and higher
//
// DESCRIPTION:     This function returns ECC memory status information.
//
// RETURN STATUS:
//    NVAPI_OK - The request was completed successfully.
//    NVAPI_ERROR - An unknown error occurred.
//    NVAPI_EXPECTED_PHYSICAL_GPU_HANDLE - The provided GPU handle is not a physical GPU handle.
//    NVAPI_INVALID_HANDLE - The provided GPU handle is invalid.
//    NVAPI_HANDLE_INVALIDATED - The provided GPU handle is no longer valid.
//    NVAPI_INVALID_POINTER - An invalid argument pointer was provided.
//    NVAPI_NOT_SUPPORTED - The request is not supported.
//    NVAPI_API_NOT_INTIALIZED - NvAPI was not yet initialized.
//
///////////////////////////////////////////////////////////////////////////////

typedef enum _NV_ECC_CONFIGURATION
{
    NV_ECC_CONFIGURATION_NOT_SUPPORTED = 0,
    NV_ECC_CONFIGURATION_DEFERRED,           // Changes require a POST to take effect
    NV_ECC_CONFIGURATION_IMMEDIATE,          // Changes can optionally be made to take effect immediately
} NV_ECC_CONFIGURATION;

typedef struct
{
    NvU32                 version;               // Structure version
    NvU32                 isSupported : 1;       // ECC memory feature support
    NV_ECC_CONFIGURATION  configurationOptions;  // Supported ECC memory feature configuration options
    NvU32                 isEnabled : 1;         // Active ECC memory setting
} NV_GPU_ECC_STATUS_INFO;

#define NV_GPU_ECC_STATUS_INFO_VER MAKE_NVAPI_VERSION(NV_GPU_ECC_STATUS_INFO,1)

NVAPI_INTERFACE NvAPI_GPU_GetECCStatusInfo(NvPhysicalGpuHandle hPhysicalGpu,
                                           NV_GPU_ECC_STATUS_INFO *pECCStatusInfo);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME:   NvAPI_GPU_GetECCErrorInfo
//
// PARAMETERS:      hPhysicalGpu (IN) - A handle identifying the physical GPU for
//                                      which ECC error information is to be
//                                      retrieved.
//                  pECCErrorInfo (OUT) - A pointer to an ECC error structure.
//
//  SUPPORTED OS: Windows XP and higher
//
// DESCRIPTION:     This function returns ECC memory error information.
//
// RETURN STATUS:
//    NVAPI_OK - The request was completed successfully.
//    NVAPI_ERROR - An unknown error occurred.
//    NVAPI_EXPECTED_PHYSICAL_GPU_HANDLE - The provided GPU handle is not a physical GPU handle.
//    NVAPI_INVALID_POINTER - An invalid argument pointer was provided.
//    NVAPI_INVALID_HANDLE - The provided GPU handle is invalid.
//    NVAPI_HANDLE_INVALIDATED - The provided GPU handle is no longer valid.
//    NVAPI_NOT_SUPPORTED - The request is not supported.
//    NVAPI_API_NOT_INTIALIZED - NvAPI was not yet initialized.
//
///////////////////////////////////////////////////////////////////////////////

typedef struct
{
    NvU32   version;             // Structure version
    struct {
        NvU64  singleBitErrors;  // Number of single-bit ECC errors detected since last boot
        NvU64  doubleBitErrors;  // Number of double-bit ECC errors detected since last boot
    } current;
    struct {
        NvU64  singleBitErrors;  // Number of single-bit ECC errors detected since last counter reset
        NvU64  doubleBitErrors;  // Number of double-bit ECC errors detected since last counter reset
    } aggregate;
} NV_GPU_ECC_ERROR_INFO;

#define NV_GPU_ECC_ERROR_INFO_VER MAKE_NVAPI_VERSION(NV_GPU_ECC_ERROR_INFO,1)

NVAPI_INTERFACE NvAPI_GPU_GetECCErrorInfo(NvPhysicalGpuHandle hPhysicalGpu,
                                          NV_GPU_ECC_ERROR_INFO *pECCErrorInfo);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME:   NvAPI_GPU_ResetECCErrorInfo
//
// PARAMETERS:      hPhysicalGpu (IN) - A handle identifying the physical GPU for
//                                      which ECC error information is to be
//                                      cleared.
//                  bResetCurrent (IN) - Reset the current ECC error counters.
//                  bResetAggregate (IN) - Reset the aggregate ECC error counters.
//
//  SUPPORTED OS: Windows XP and higher
//
// DESCRIPTION:     This function resets ECC memory error counters.
//
// RETURN STATUS:
//    NVAPI_OK - The request was completed successfully.
//    NVAPI_ERROR - An unknown error occurred.
//    NVAPI_EXPECTED_PHYSICAL_GPU_HANDLE - The provided GPU handle is not a physical GPU handle.
//    NVAPI_INVALID_HANDLE - The provided GPU handle is invalid.
//    NVAPI_HANDLE_INVALIDATED - The provided GPU handle is no longer valid.
//    NVAPI_NOT_SUPPORTED - The request is not supported.
//    NVAPI_API_NOT_INTIALIZED - NvAPI was not yet initialized.
//
///////////////////////////////////////////////////////////////////////////////

NVAPI_INTERFACE NvAPI_GPU_ResetECCErrorInfo(NvPhysicalGpuHandle hPhysicalGpu, NvU8 bResetCurrent,
                                            NvU8 bResetAggregate);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME:   NvAPI_GPU_GetECCConfigurationInfo
//
// PARAMETERS:      hPhysicalGpu (IN) - A handle identifying the physical GPU for
//                                      which ECC configuration information
//                                      is to be retrieved.
//                  pECCConfigurationInfo (OUT) - A pointer to an ECC
//                                                configuration structure.
//
//  SUPPORTED OS: Windows XP and higher
//
// DESCRIPTION:     This function returns ECC memory configuration information.
//
// RETURN STATUS:
//    NVAPI_OK - The request was completed successfully.
//    NVAPI_ERROR - An unknown error occurred.
//    NVAPI_EXPECTED_PHYSICAL_GPU_HANDLE - The provided GPU handle is not a physical GPU handle.
//    NVAPI_INVALID_HANDLE - The provided GPU handle is invalid.
//    NVAPI_HANDLE_INVALIDATED - The provided GPU handle is no longer valid.
//    NVAPI_INVALID_POINTER - An invalid argument pointer was provided.
//    NVAPI_NOT_SUPPORTED - The request is not supported.
//    NVAPI_API_NOT_INTIALIZED - NvAPI was not yet initialized.
//
///////////////////////////////////////////////////////////////////////////////

typedef struct
{
    NvU32  version;                 // Structure version
    NvU32  isEnabled : 1;           // Current ECC configuration stored in non-volatile memory
    NvU32  isEnabledByDefault : 1;  // Factory default ECC configuration (static)
} NV_GPU_ECC_CONFIGURATION_INFO;

#define NV_GPU_ECC_CONFIGURATION_INFO_VER MAKE_NVAPI_VERSION(NV_GPU_ECC_CONFIGURATION_INFO,1)

NVAPI_INTERFACE NvAPI_GPU_GetECCConfigurationInfo(NvPhysicalGpuHandle hPhysicalGpu,
                                                  NV_GPU_ECC_CONFIGURATION_INFO *pECCConfigurationInfo);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME:   NvAPI_GPU_SetECCConfiguration
//
// PARAMETERS:      hPhysicalGpu (IN) - A handle identifying the physical GPU for
//                                      which to update the ECC configuration
//                                      setting.
//                  bEnable (IN) - The new ECC configuration setting.
//                  bEnableImmediately (IN) - Request that the new setting take effect immediately.
//
//  SUPPORTED OS: Windows XP and higher
//
// DESCRIPTION:     This function updates the ECC memory configuration setting.
//
// RETURN STATUS:
//    NVAPI_OK - The request was completed successfully.
//    NVAPI_ERROR - An unknown error occurred.
//    NVAPI_EXPECTED_PHYSICAL_GPU_HANDLE - The provided GPU handle is not a physical GPU handle.
//    NVAPI_INVALID_HANDLE - The provided GPU handle is invalid.
//    NVAPI_HANDLE_INVALIDATED - The provided GPU handle is no longer valid.
//    NVAPI_NOT_SUPPORTED - The request is not supported.
//    NVAPI_API_NOT_INTIALIZED - NvAPI was not yet initialized.
//
///////////////////////////////////////////////////////////////////////////////

NVAPI_INTERFACE NvAPI_GPU_SetECCConfiguration(NvPhysicalGpuHandle hPhysicalGpu, NvU8 bEnable,
                                              NvU8 bEnableImmediately);

#if defined(_D3D9_H_) || defined(__d3d10_h__) || defined(__d3d10_1_h__) || defined(__d3d11_h__)
///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_D3D_SetFPSIndicatorState
//
//   DESCRIPTION: Display an overlay that tracks the number of times the app presents per second, or,
//      the number of frames-per-second (FPS)
//
//  SUPPORTED OS: Windows XP and higher
//
//   PARAMETERS: bool(IN)  - Whether or not to enable the fps indicator.
//
//   RETURN STATUS: NVAPI_OK: SUCCESS if the profile is found
//                  NVAPI_ERROR: For miscellaneous errors.
//
///////////////////////////////////////////////////////////////////////////////

NVAPI_INTERFACE NvAPI_D3D_SetFPSIndicatorState(IUnknown *pDev, NvU8 doEnable);

#endif //if defined(_D3D9_H_) || defined(__d3d10_h__) || defined(__d3d10_1_h__) || defined(__d3d11_h__)


// GPU Profile APIs

#define NVAPI_UNICODE_STRING_MAX                             2048
#define NVAPI_BINARY_DATA_MAX                                4096
#define NVAPI_SETTING_MAX_VALUES                             100

NV_DECLARE_HANDLE(NvDRSSessionHandle);
NV_DECLARE_HANDLE(NvDRSProfileHandle);

#define NVAPI_DRS_GLOBAL_PROFILE                             ((NvDRSProfileHandle) -1)

typedef NvU16 NvAPI_UnicodeString[NVAPI_UNICODE_STRING_MAX];

typedef enum _NVDRS_SETTING_TYPE
{
     NVDRS_DWORD_TYPE,
     NVDRS_BINARY_TYPE,
     NVDRS_STRING_TYPE,
     NVDRS_WSTRING_TYPE
} NVDRS_SETTING_TYPE;

typedef enum _NVDRS_SETTING_LOCATION
{
     NVDRS_CURRENT_PROFILE_LOCATION,
     NVDRS_GLOBAL_PROFILE_LOCATION,
     NVDRS_BASE_PROFILE_LOCATION
} NVDRS_SETTING_LOCATION;

typedef struct _NVDRS_GPU_SUPPORT
{
    NvU32 geforce    :  1;
    NvU32 quadro     :  1;
    NvU32 nvs        :  1;
    NvU32 reserved4  :  1;
    NvU32 reserved5  :  1;
    NvU32 reserved6  :  1;
    NvU32 reserved7  :  1;
    NvU32 reserved8  :  1;
    NvU32 reserved9  :  1;
    NvU32 reserved10 :  1;
    NvU32 reserved11 :  1;
    NvU32 reserved12 :  1;
    NvU32 reserved13 :  1;
    NvU32 reserved14 :  1;
    NvU32 reserved15 :  1;
    NvU32 reserved16 :  1;
    NvU32 reserved17 :  1;
    NvU32 reserved18 :  1;
    NvU32 reserved19 :  1;
    NvU32 reserved20 :  1;
    NvU32 reserved21 :  1;
    NvU32 reserved22 :  1;
    NvU32 reserved23 :  1;
    NvU32 reserved24 :  1;
    NvU32 reserved25 :  1;
    NvU32 reserved26 :  1;
    NvU32 reserved27 :  1;
    NvU32 reserved28 :  1;
    NvU32 reserved29 :  1;
    NvU32 reserved30 :  1;
    NvU32 reserved31 :  1;
    NvU32 reserved32 :  1;
} NVDRS_GPU_SUPPORT;

typedef struct _NVDRS_BINARY_SETTING // Enum to decide on the datatype of setting value.
{
     NvU32                valueLength;               // valueLength should always be in number of bytes.
     NvU8                 valueData[NVAPI_BINARY_DATA_MAX];
} NVDRS_BINARY_SETTING;

typedef struct _NVDRS_SETTING_VALUES
{
     NvU32                      version;                // Structure Version
     NvU32                      numSettingValues;       // Total number of values available in a setting.
     NVDRS_SETTING_TYPE         settingType;            // Type of setting value.
     union                                              // Setting can hold either DWORD or Binary value or string. Not mixed types.
     {
         NvU32                      u32DefaultValue;    // Accessing default DWORD value of this setting.
         NVDRS_BINARY_SETTING       binaryDefaultValue; // Accessing default Binary value of this setting.
                                                        // Must be allocated by caller with valueLength specifying buffer size, or only valueLength will be filled in.
         NvAPI_UnicodeString        wszDefaultValue;    // Accessing default unicode string value of this setting.
     };
     union                                              // Setting values can be of either DWORD, Binary values or String type,
     {                                                  // NOT mixed types.
         NvU32                      u32Value;           // All possible DWORD values for a setting
         NVDRS_BINARY_SETTING       binaryValue;        // All possible Binary values for a setting
         NvAPI_UnicodeString        wszValue;           // Accessing current unicode string value of this setting.
     }settingValues[NVAPI_SETTING_MAX_VALUES];
} NVDRS_SETTING_VALUES;

#define NVDRS_SETTING_VALUES_VER    MAKE_NVAPI_VERSION(NVDRS_SETTING_VALUES,1)

typedef struct _NVDRS_SETTING
{
     NvU32                      version;                // Structure Version
     NvAPI_UnicodeString        settingName;            // String name of setting
     NvU32                      settingId;              // 32 bit setting Id
     NVDRS_SETTING_TYPE         settingType;            // Type of setting value.
     NVDRS_SETTING_LOCATION     settingLocation;        // Describes where the value in CurrentValue comes from.
     NvU32                      isCurrentPredefined;    // It is different than 0 if the currentValue is a predefined Value,
                                                        // 0 if the currentValue is a user value.
     NvU32                      isPredefinedValid;      // It is different than 0 if the PredefinedValue union contains a valid value.
     union                                              // Setting can hold either DWORD or Binary value or string. Not mixed types.
     {
         NvU32                      u32PredefinedValue;    // Accessing default DWORD value of this setting.
         NVDRS_BINARY_SETTING       binaryPredefinedValue; // Accessing default Binary value of this setting.
                                                           // Must be allocated by caller with valueLength specifying buffer size,
                                                           // or only valueLength will be filled in.
         NvAPI_UnicodeString        wszPredefinedValue;    // Accessing default unicode string value of this setting.
     };
     union                                              // Setting can hold either DWORD or Binary value or string. Not mixed types.
     {
         NvU32                      u32CurrentValue;    // Accessing current DWORD value of this setting.
         NVDRS_BINARY_SETTING       binaryCurrentValue; // Accessing current Binary value of this setting.
                                                        // Must be allocated by caller with valueLength specifying buffer size,
                                                        // or only valueLength will be filled in.
         NvAPI_UnicodeString        wszCurrentValue;    // Accessing current unicode string value of this setting.
     };
} NVDRS_SETTING;

#define NVDRS_SETTING_VER        MAKE_NVAPI_VERSION(NVDRS_SETTING,1)

typedef struct _NVDRS_APPLICATION
{
     NvU32                      version;            // Structure Version
     NvU32                      isPredefined;       // Is the application userdefined/predefined
     NvAPI_UnicodeString        appName;            // String name of the Application
     NvAPI_UnicodeString        userFriendlyName;   // UserFriendly name of the Application
     NvAPI_UnicodeString        launcher;
} NVDRS_APPLICATION;

#define NVDRS_APPLICATION_VER        MAKE_NVAPI_VERSION(NVDRS_APPLICATION,1)

typedef struct _NVDRS_PROFILE
{
     NvU32                      version;            // Structure Version
     NvAPI_UnicodeString        profileName;        // String name of the Profile
     NVDRS_GPU_SUPPORT          gpuSupport;         // This flag indicates the profile support on either,
                                                    // Quadro or Geforce or Both. Read Only.
     NvU32                      isPredefined;       // Is the Profile userdefined/predefined
     NvU32                      numOfApps;          // Total number of apps that belong to this profile. Read-only
     NvU32                      numOfSettings;      // Total number of settings applied for this Profile. Read-only
} NVDRS_PROFILE;

#define NVDRS_PROFILE_VER        MAKE_NVAPI_VERSION(NVDRS_PROFILE,1)

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_DRS_CreateSession
//
//   DESCRIPTION: Allocate memory and initialization
//
//  SUPPORTED OS: Windows XP and higher
//
//   PARAMETERS: NvDRSSessionHandle(OUT) - Returns pointer to session handle.
//
//   RETURN STATUS: NVAPI_OK: SUCCESS
//                  NVAPI_ERROR: For miscellaneous errors.
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_DRS_CreateSession(NvDRSSessionHandle *phSession);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_DRS_DestroySession
//
//   DESCRIPTION: Free allocation and cleanup of NvDrsSession
//
//  SUPPORTED OS: Windows XP and higher
//
//   PARAMETERS: NvDRSSessionHandle(IN) - Input to the session handle.
//
//   RETURN STATUS: NVAPI_OK: SUCCESS
//                  NVAPI_ERROR: For miscellaneous errors.
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_DRS_DestroySession(NvDRSSessionHandle hSession);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_DRS_LoadSettings
//
//   DESCRIPTION: Load and parse settings data
//
//  SUPPORTED OS: Windows XP and higher
//
//   PARAMETERS: NvDRSSessionHandle(IN) - Input to the session handle.
//
//   RETURN STATUS: NVAPI_OK: SUCCESS
//                  NVAPI_ERROR: For miscellaneous errors.
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_DRS_LoadSettings(NvDRSSessionHandle hSession);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_DRS_SaveSettings
//
//   DESCRIPTION: Save settings data to the system
//
//  SUPPORTED OS: Windows XP and higher
//
//   PARAMETERS: NvDRSSessionHandle(IN) - Input to the session handle.
//
//   RETURN STATUS: NVAPI_OK: SUCCESS
//                  NVAPI_ERROR: For miscellaneous errors.
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_DRS_SaveSettings(NvDRSSessionHandle hSession);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_DRS_LoadSettingsFromFile
//
//   DESCRIPTION: Load settings from the given file path
//
//  SUPPORTED OS: Windows XP and higher
//
//   PARAMETERS: NvDRSSessionHandle(IN) - Input to the session handle.
//               NvAPI_UnicodeString(IN) - Binary File Name/Path
//
//   RETURN STATUS: NVAPI_OK: SUCCESS
//                  NVAPI_ERROR: For miscellaneous errors.
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_DRS_LoadSettingsFromFile(NvDRSSessionHandle hSession, NvAPI_UnicodeString fileName);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_DRS_SaveSettingsToFile
//
//   DESCRIPTION: Save settings to the given file path
//
//  SUPPORTED OS: Windows XP and higher
//
//   PARAMETERS: NvDRSSessionHandle(IN) - Input to the session handle.
//               NvAPI_UnicodeString(IN) - Binary File Name/Path
//
//   RETURN STATUS: NVAPI_OK: SUCCESS
//                  NVAPI_ERROR: For miscellaneous errors.
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_DRS_SaveSettingsToFile(NvDRSSessionHandle hSession, NvAPI_UnicodeString fileName);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_DRS_CreateProfile
//
//   DESCRIPTION: Create an empty profile.
//
//  SUPPORTED OS: Windows XP and higher
//
//   PARAMETERS: NvDRSSessionHandle(IN)   - Input to the session handle.
//               NVDRS_PROFILE(IN)        - Input pointer to NVDRS_PROFILE.
//               NvDRSProfileHandle(OUT)  - Returns pointer to profile handle.
//
//   RETURN STATUS: NVAPI_OK: SUCCESS
//                  NVAPI_ERROR: For miscellaneous errors.
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_DRS_CreateProfile(NvDRSSessionHandle hSession, NVDRS_PROFILE *pProfileInfo, NvDRSProfileHandle *phProfile);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_DRS_DeleteProfile
//
//   DESCRIPTION: Delete a profile or sets it back to predefined value.
//
//  SUPPORTED OS: Windows XP and higher
//
//   PARAMETERS: NvDRSSessionHandle(IN)  - Input to the session handle.
//               NvDRSProfileHandle (IN) - Input profile handle.
//
//   RETURN STATUS: NVAPI_OK: SUCCESS if the profile is found
//                  NVAPI_ERROR: For miscellaneous errors.
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_DRS_DeleteProfile(NvDRSSessionHandle hSession, NvDRSProfileHandle hProfile);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_DRS_SetCurrentGlobalProfile
//
//   DESCRIPTION: Sets the current global profile in the driver.
//
//  SUPPORTED OS: Windows XP and higher
//
//   PARAMETERS: NvDRSSessionHandle(IN)  - Input to the session handle.
//               NvAPI_UnicodeString(IN) - Input current Global profile name.
//
//   RETURN STATUS: NVAPI_OK: SUCCESS
//                  NVAPI_ERROR: For miscellaneous errors.
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_DRS_SetCurrentGlobalProfile(NvDRSSessionHandle hSession, NvAPI_UnicodeString wszGlobalProfileName);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_DRS_GetCurrentGlobalProfile
//
//   DESCRIPTION: Returns the handle to the current global profile.
//
//  SUPPORTED OS: Windows XP and higher
//
//   PARAMETERS: NvDRSSessionHandle(IN)  - Input to the session handle.
//               NvDRSProfileHandle(OUT) - Returns current Global profile handle.
//
//   RETURN STATUS: NVAPI_OK: SUCCESS
//                  NVAPI_ERROR: For miscellaneous errors.
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_DRS_GetCurrentGlobalProfile(NvDRSSessionHandle hSession, NvDRSProfileHandle *phProfile);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_DRS_GetProfileInfo
//
//   DESCRIPTION: Get the information of the given profile. User needs to specify the name of the Profile.
//
//  SUPPORTED OS: Windows XP and higher
//
//   PARAMETERS: NvDRSSessionHandle(IN)  - Input to the session handle.
//               NvDRSProfileHandle(IN)  - Input profile handle.
//               NVDRS_PROFILE(OUT)      - Return the profile info.
//
//   RETURN STATUS: NVAPI_OK: SUCCESS
//                  NVAPI_ERROR: For miscellaneous errors.
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_DRS_GetProfileInfo(NvDRSSessionHandle hSession, NvDRSProfileHandle hProfile, NVDRS_PROFILE *pProfileInfo);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_DRS_SetProfileInfo
//
//   DESCRIPTION: Specifies flags for a given profile. Currently only the NVDRS_GPU_SUPPORT is
//                used to update the profile. Neither the name, number of settings or applications
//                or other profile information can be changed with this function.
//
//  SUPPORTED OS: Windows XP and higher
//
//   PARAMETERS: NvDRSSessionHandle(IN)  - Input to the session handle.
//               NvDRSProfileHandle(IN)  - Input profile handle.
//               NVDRS_PROFILE(IN)       - Input the new profile info.
//
//   RETURN STATUS: NVAPI_OK: SUCCESS
//                  NVAPI_ERROR: For miscellaneous errors.
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_DRS_SetProfileInfo(NvDRSSessionHandle hSession, NvDRSProfileHandle hProfile, NVDRS_PROFILE *pProfileInfo);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_DRS_FindProfileByName
//
//   DESCRIPTION: Find a profile in the current session.
//
//  SUPPORTED OS: Windows XP and higher
//
//   PARAMETERS: NvDRSSessionHandle(IN)  - Input to the session handle.
//               NvAPI_UnicodeString(IN) - Input profileName.
//               NvDRSProfileHandle(OUT) - Input profile handle.
//
//   RETURN STATUS: NVAPI_OK: SUCCESS if the profile is found
//                  NVAPI_PROFILE_NOT_FOUND: if profile is not found
//                  NVAPI_ERROR: For miscellaneous errors.
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_DRS_FindProfileByName(NvDRSSessionHandle hSession, NvAPI_UnicodeString profileName, NvDRSProfileHandle* phProfile);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_DRS_EnumProfiles
//
//   DESCRIPTION: Enumerate through all the profiles in the session.
//
//  SUPPORTED OS: Windows XP and higher
//
//   PARAMETERS: NvDRSSessionHandle(IN)   - Input to the session handle.
//               index(IN)                - Input the index for enumeration.
//               NvDRSProfileHandle(OUT)  - Returns profile handle.
//
//   RETURN STATUS: NVAPI_OK: SUCCESS if the profile is found
//                  NVAPI_ERROR: For miscellaneous errors.
//                  NVAPI_END_ENUMERATION: index exceeds the total number of available Profiles in DB.
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_DRS_EnumProfiles(NvDRSSessionHandle hSession, NvU32 index, NvDRSProfileHandle *phProfile);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_DRS_GetNumProfiles
//
//   DESCRIPTION: Obtain number of profiles in the current session object
//
//  SUPPORTED OS: Windows XP and higher
//
//   PARAMETERS: NvDRSSessionHandle(IN)  - Input to the session handle.
//               numProfiles(OUT) -  returns count of profiles in the current hSession.
//
//   RETURN STATUS: NVAPI_OK: SUCCESS
//                  NVAPI_API_NOT_INTIALIZED: Failed to initialize.
//                  NVAPI_INVALID_ARGUMENT: Invalid Arguments.
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_DRS_GetNumProfiles(NvDRSSessionHandle hSession, NvU32 *numProfiles);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_DRS_CreateApplication
//
//   DESCRIPTION: Add an executable name to a profile.
//
//  SUPPORTED OS: Windows XP and higher
//
//   PARAMETERS: NvDRSSessionHandle(IN)  - Input to the session handle.
//               NvDRSProfileHandle(IN)  - Input profile handle.
//               NVDRS_APPLICATION(IN)   - Input NVDRS_APPLICATION struct with the executable name to be added.
//
//   RETURN STATUS: NVAPI_OK: SUCCESS
//                  NVAPI_ERROR: For miscellaneous errors.
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_DRS_CreateApplication(NvDRSSessionHandle hSession, NvDRSProfileHandle  hProfile, NVDRS_APPLICATION *pApplication);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_DRS_DeleteApplication
//
//   DESCRIPTION: Removes an executable from a profile.
//
//  SUPPORTED OS: Windows XP and higher
//
//   PARAMETERS: NvDRSSessionHandle(IN)  - Input to the session handle.
//               NvDRSProfileHandle(IN)  - Input profile handle.
//               NvAPI_UnicodeString(IN) - Input the executable name to be removed.
//
//   RETURN STATUS: NVAPI_OK: SUCCESS
//                  NVAPI_ERROR: For miscellaneous errors.
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_DRS_DeleteApplication(NvDRSSessionHandle hSession, NvDRSProfileHandle hProfile, NvAPI_UnicodeString appName);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_DRS_GetApplicationInfo
//
//   DESCRIPTION: Get the information of the given application.
//
//  SUPPORTED OS: Windows XP and higher
//
//   PARAMETERS: NvDRSSessionHandle(IN)  - Input to the session handle.
//               NvDRSProfileHandle(IN)  - Input profile handle.
//               NvAPI_UnicodeString(IN) - Input application name.
//               NVDRS_APPLICATION(OUT)  - Returns NVDRS_APPLICATION struct with all the attributes.
//
//   RETURN STATUS: NVAPI_OK: SUCCESS
//                  NVAPI_ERROR: For miscellaneous errors.
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_DRS_GetApplicationInfo(NvDRSSessionHandle hSession, NvDRSProfileHandle hProfile, NvAPI_UnicodeString appName, NVDRS_APPLICATION *pApplication);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_DRS_EnumApplications
//
//   DESCRIPTION: Enumerate all the applications in a given profile from the starting index to the max length.
//
//  SUPPORTED OS: Windows XP and higher
//
//   PARAMETERS: NvDRSSessionHandle(IN)  - Input to the session handle.
//               NvDRSProfileHandle(IN)  - Input profile handle.
//               startIndex(IN)          - Indicates starting index for enumeration.
//               appCount(IN OUT)        - Input max length of the passed in arrays, Returns the actual length.
//               NVDRS_APPLICATION(OUT)  - Returns NVDRS_APPLICATION struct with all the attributes.
//
//   RETURN STATUS: NVAPI_OK: SUCCESS
//                  NVAPI_ERROR: For miscellaneous errors.
//                  NVAPI_END_ENUMERATION: startIndex exceeds the total appCount.
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_DRS_EnumApplications(NvDRSSessionHandle hSession, NvDRSProfileHandle hProfile, NvU32 startIndex, NvU32 *appCount, NVDRS_APPLICATION *pApplication);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_DRS_FindApplicationByName
//
//   DESCRIPTION: Search the Application and the associated Profile for given appName.
//
//  SUPPORTED OS: Windows XP and higher
//
//   PARAMETERS: NvDRSSessionHandle(IN)  - Input to the hSession handle.
//               NvAPI_UnicodeString(IN) - Input appName.
//               NvDRSProfileHandle(OUT) - Returns profile handle.
//               NVDRS_APPLICATION(OUT)  - Returns NVDRS_APPLICATION struct pointer.
//
//   RETURN STATUS: NVAPI_OK: SUCCESS if the profile is found
//                  NVAPI_APPLICATION_NOT_FOUND: if App not found
//                  NVAPI_ERROR: For miscellaneous errors.
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_DRS_FindApplicationByName(NvDRSSessionHandle hSession, NvAPI_UnicodeString appName, NvDRSProfileHandle *phProfile, NVDRS_APPLICATION *pApplication);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_DRS_SetSetting
//
//   DESCRIPTION: Add/Modify a setting to a Profile.
//
//  SUPPORTED OS: Windows XP and higher
//
//   PARAMETERS: NvDRSSessionHandle(IN)  - Input to the session handle.
//               NvDRSProfileHandle(IN)  - Input profile handle.
//               NVDRS_SETTING(IN)       - Input NVDRS_SETTING struct pointer.
//
//   RETURN STATUS: NVAPI_OK: SUCCESS
//                  NVAPI_ERROR: For miscellaneous errors.
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_DRS_SetSetting(NvDRSSessionHandle hSession, NvDRSProfileHandle hProfile, NVDRS_SETTING *pSetting);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_DRS_GetSetting
//
//   DESCRIPTION: Get the information of the given setting.
//
//  SUPPORTED OS: Windows XP and higher
//
//   PARAMETERS: NvDRSSessionHandle(IN)  - Input to the session handle.
//               NvDRSProfileHandle(IN)  - Input profile handle.
//               NvU32(IN)               - Input settingId.
//               NVDRS_SETTING(OUT)      - Returns all the setting info
//
//   RETURN STATUS: NVAPI_OK: SUCCESS
//                  NVAPI_ERROR: For miscellaneous errors.
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_DRS_GetSetting(NvDRSSessionHandle hSession, NvDRSProfileHandle hProfile, NvU32 settingId, NVDRS_SETTING *pSetting);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_DRS_EnumSettings
//
//   DESCRIPTION: Enumerate all the settings of a given profile from startIndex to the max length.
//
//  SUPPORTED OS: Windows XP and higher
//
//   PARAMETERS: NvDRSSessionHandle(IN)  - Input to the session handle.
//               NvDRSProfileHandle(IN)  - Input profile handle.
//               startIndex(IN)          - Indicates starting index for enumeration.
//               settingsCount(IN OUT)   - Input max length of the passed in arrays, Returns the actual length.
//               NVDRS_SETTING(OUT)      - Returns all the settings info.
//
//   RETURN STATUS: NVAPI_OK: SUCCESS
//                  NVAPI_ERROR: For miscellaneous errors.
//                  NVAPI_END_ENUMERATION: startIndex exceeds the total appCount.
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_DRS_EnumSettings(NvDRSSessionHandle hSession, NvDRSProfileHandle hProfile, NvU32 startIndex, NvU32 *settingsCount, NVDRS_SETTING *pSetting);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_DRS_EnumAvaliableSettingIds
//
//   DESCRIPTION: Enumerate all the Ids of all the settings recognized by NVAPI.
//
//  SUPPORTED OS: Windows XP and higher
//
//   PARAMETERS: pSettingIds(OUT)    - User-provided array of length *pMaxCount that NVAPI will fill with IDs.
//               pMaxCount(IN OUT)   - Input max length of the passed in array, Returns the actual length.
//
//   RETURN STATUS: NVAPI_OK: SUCCESS
//                  NVAPI_ERROR: For miscellaneous errors.
//                  NVAPI_END_ENUMERATION: the provided pMaxCount is not enough to hold all settingIds.
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_DRS_EnumAvailableSettingIds(NvU32 *pSettingIds, NvU32 *pMaxCount);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_DRS_EnumAvailableSettingValues
//
//   DESCRIPTION: Enumerate all available setting values for a given setting.
//
//  SUPPORTED OS: Windows XP and higher
//
//   PARAMETERS: settingId(IN)               - Input settingId.
//               maxNumCount(IN/OUT)         - Input max length of the passed in arrays, Returns the actual length.
//               NVDRS_SETTING_VALUES(OUT)   - Returns all available setting values and its count.
//
//   RETURN STATUS: NVAPI_OK: SUCCESS
//                  NVAPI_ERROR: For miscellaneous errors.
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_DRS_EnumAvailableSettingValues(NvU32 settingId, NvU32 *pMaxNumValues, NVDRS_SETTING_VALUES *pSettingValues);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_DRS_GetSettingIdFromName
//
//   DESCRIPTION: Get the binary ID of a setting given the setting name.
//
//  SUPPORTED OS: Windows XP and higher
//
//   PARAMETERS: NvAPI_UnicodeString(IN)  - Input Unicode settingName.
//               NvU32(OUT)               - Returns corresponding settingId.
//
//   RETURN STATUS: NVAPI_OK: SUCCESS if the profile is found
//                  NVAPI_PROFILE_NOT_FOUND: if profile is not found
//                  NVAPI_SETTING_NOT_FOUND: if setting is not found
//                  NVAPI_ERROR: For miscellaneous errors.
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_DRS_GetSettingIdFromName(NvAPI_UnicodeString settingName, NvU32 *pSettingId);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_DRS_GetSettingNameFromId
//
//   DESCRIPTION: Get the setting name given the binary ID.
//
//  SUPPORTED OS: Windows XP and higher
//
//   PARAMETERS: NvU32(IN)                - Input settingId.
//               NvAPI_UnicodeString(OUT) - Returns corresponding Unicode settingName.
//
//   RETURN STATUS: NVAPI_OK: SUCCESS if the profile is found
//                  NVAPI_PROFILE_NOT_FOUND: if profile is not found
//                  NVAPI_SETTING_NOT_FOUND: if setting is not found
//                  NVAPI_ERROR: For miscellaneous errors.
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_DRS_GetSettingNameFromId(NvU32 settingId, NvAPI_UnicodeString *pSettingName);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_DRS_DeleteProfileSetting
//
//   DESCRIPTION: Delete a setting or set it back to predefined value.
//
//  SUPPORTED OS: Windows XP and higher
//
//   PARAMETERS: NvDRSSessionHandle(IN)  - Input to the session handle.
//               NvDRSProfileHandle(IN)  - Input profile handle.
//               settingId(IN)           - Input settingId to be deleted.
//
//   RETURN STATUS: NVAPI_OK: SUCCESS if the profile is found
//                  NVAPI_ERROR: For miscellaneous errors.
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_DRS_DeleteProfileSetting(NvDRSSessionHandle hSession, NvDRSProfileHandle hProfile, NvU32 settingId);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_DRS_RestoreAllDefaults
//
//   DESCRIPTION: Restore the whole system to predefined(default) values.
//
//  SUPPORTED OS: Windows XP and higher
//
//   PARAMETERS: NvDRSSessionHandle(IN)  - Input to the session handle.
//
//   RETURN STATUS: NVAPI_OK: SUCCESS if the profile is found
//                  NVAPI_ERROR: For miscellaneous errors.
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_DRS_RestoreAllDefaults(NvDRSSessionHandle hSession);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_DRS_RestoreProfileDefaults
//
//   DESCRIPTION: Restore the given profile to predefined(default) values.
//                Any and all user specified modifications will be removed.
//                If the whole profile was set by the user, the profile will be removed.
//
//  SUPPORTED OS: Windows XP and higher
//
//   PARAMETERS: NvDRSSessionHandle(IN)  - Input to the session handle.
//               NvDRSProfileHandle(IN)  - Input profile handle.
//
//   RETURN STATUS: NVAPI_OK: SUCCESS if the profile is found
//                  NVAPI_PROFILE_REMOVED: SUCCESS, and the hProfile is no longer valid.
//                  NVAPI_ERROR: For miscellaneous errors.
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_DRS_RestoreProfileDefault(NvDRSSessionHandle hSession, NvDRSProfileHandle hProfile);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_DRS_RestoreProfileDefaultSetting
//
//   DESCRIPTION: Restore the given profile setting to predefined(default) values.
//
//  SUPPORTED OS: Windows XP and higher
//
//   PARAMETERS: NvDRSSessionHandle(IN)  - Input to the session handle.
//               NvDRSProfileHandle(IN)  - Input profile handle.
//               NvU32(IN)               - Input settingId.
//
//   RETURN STATUS: NVAPI_OK: SUCCESS if the profile is found
//                  NVAPI_ERROR: For miscellaneous errors.
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_DRS_RestoreProfileDefaultSetting(NvDRSSessionHandle hSession, NvDRSProfileHandle hProfile, NvU32 settingId);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_DRS_GetBaseProfile
//
//   DESCRIPTION: Returns the handle to the current global profile.
//
//  SUPPORTED OS: Windows XP and higher
//
//   PARAMETERS: NvDRSSessionHandle(IN)  - Input to the session handle.
//               NvDRSProfileHandle(OUT) - Returns Base profile handle.
//
//   RETURN STATUS: NVAPI_OK: SUCCESS
//                  NVAPI_ERROR: For miscellaneous errors.
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_DRS_GetBaseProfile(NvDRSSessionHandle hSession, NvDRSProfileHandle *phProfile);


#ifdef __cplusplus
}; //extern "C" {

#endif

#pragma pack(pop)

#endif // _NVAPI_H

