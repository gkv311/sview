/**
 * This include file contains all the DX9 Stereo Display extension definitions
 * (structures, enums, constants) shared between the driver and the application.
 *
 * Copyright © 2011 ADVANCED MICRO DEVICES, INC.
 */

#if(defined(_WIN32) || defined(__WIN32__))
#ifndef _ATIDX9STEREO_H_
#define _ATIDX9STEREO_H_

#include <d3d9.h>

#define ATI_STEREO_VERSION_MAJOR 0
#define ATI_STEREO_VERSION_MINOR 3

#pragma pack(push, 4)

#define FOURCC_AQBS MAKEFOURCC('A','Q','B','S')

// SetSrcEye/SetDstEye Parameters
#define ATI_STEREO_LEFTEYE  0
#define ATI_STEREO_RIGHTEYE 1

/**
 * This is the enum for all the commands that can be sent to the driver in the surface communication packet.
 */
typedef enum _ATIDX9STEREOCOMMAND {
    ATI_STEREO_GETVERSIONDATA      = 0,    //!< return version data structure
    ATI_STEREO_ENABLESTEREO        = 1,    //!< enable stereo
    ATI_STEREO_ENABLELEFTONLY      = 2,    //!< enable stereo but only display left eye
    ATI_STEREO_ENABLERIGHTONLY     = 3,    //!< enable stereo but only display right eye
    ATI_STEREO_ENABLESTEREOSWAPPED = 4,    //!< enable stereo but swap left and right eyes
    ATI_STEREO_GETLINEOFFSET       = 5,    //!< return the line offset from end of left eye to beginning of right eye
    ATI_STEREO_GETDISPLAYMODES     = 6,    //!< return an array of all the supported stereo display modes in a TBD format
    ATI_STEREO_SETSRCEYE           = 7,    //!< sets the source eye for blts and surface copies (left/right eye specified in dwParams)
    ATI_STEREO_SETDSTEYE           = 8,    //!< sets the destination eye for blts and surface copies (left/right eye specified in dwParams)
    ATI_STEREO_ENABLEPERSURFAA     = 9,    //!< create independent AA buffers for all multi-sample render targets (excluding the flip chain)
    ATI_STEREO_ENABLEPRIMARYAA     = 10,   //!< enable AA for primaries when multi-sample fields in present params are set and stereo is enabled
    ATI_STEREO_COMMANDMAX          = 11,   //!< largest command enum
    ATI_STEREO_FORCEDWORD          = 0xffffffff
} ATIDX9STEREOCOMMAND;

/**
 * Structure used to send commands and get data from the driver through the
 * FOURCC_AQBS surface.  When a FOURCC_AQBS surface is created and locked,
 * a pointer to this structure is returned.  If properly filled in, it will
 * process the appropriate command when the surface is unlocked
 */
typedef struct _ATIDX9STEREOCOMMPACKET {
    DWORD               dwSignature;     //!< signature to indicate to the driver that the app is sending a command
    DWORD               dwSize;          //!< size of this structure.  Passed to the app on lock
    ATIDX9STEREOCOMMAND stereoCommand;   //!< command given to the driver
    HRESULT*            pResult;         //!< pointer to buffer where error code will be written to. D3D_OK if successful
    DWORD               dwOutBufferSize; //!< size of optional buffer to place outgoing data into (in bytes).  Must be set if data is returned
    BYTE*               pOutBuffer;      //!< pointer to buffer for outgoing data. (lineoffset, displaymodes, etc)
    DWORD               dwInBufferSize;  //!< size of optional buffer to place incoming parameters
    BYTE*               pInBuffer;       //!< pointer to buffer for incoming data (SetSrcEye, SetDstEye, etc)
} ATIDX9STEREOCOMMPACKET;

typedef struct _ATIDX9STEREOVERSION {
    DWORD dwSize;         //!< size of this structure
    DWORD dwVersionMajor; //!< major Version
    DWORD dwVersionMinor; //!< minor Version
    DWORD dwMaxCommand;   //!< max command enum
    DWORD dwCaps;         //!< stereo Caps (not implemented yet)
    DWORD dwReserved[11];
} ATIDX9STEREOVERSION;

typedef struct _ATIDX9GETDISPLAYMODES {
    UINT            dwNumModes;   //!< number of stereo modes available.
    D3DDISPLAYMODE* pStereoModes; //!< list containing stereo mode details for all the modes.
} ATIDX9GETDISPLAYMODES;

#pragma pack(pop)

#endif //_ATIDX9STEREO_H_
#endif //_WIN32
