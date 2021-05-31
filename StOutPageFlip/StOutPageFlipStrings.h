/**
 * StOutPageFlip, class providing stereoscopic output for Shutter Glasses displays using StCore toolkit.
 * Copyright © 2007-2013 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#ifndef __StOutPageFlipStrings_h_
#define __StOutPageFlipStrings_h_

/**
 * Translation resources
 */
enum {

    STTR_PAGEFLIP_NAME = 1000,
    STTR_PAGEFLIP_DESC = 1001,
    STTR_VUZIX_NAME    = 1002,
    STTR_VUZIX_DESC    = 1003,

    // parameters
    STTR_PARAMETER_QBUFFER_TYPE = 1102,
    STTR_PARAMETER_CONTROL_CODE = 1103,

    STTR_PARAMETER_QB_EMULATED      = 1120,
    STTR_PARAMETER_QB_HARDWARE      = 1122,
    STTR_PARAMETER_QB_D3D_ANY       = 1123,
    STTR_PARAMETER_QB_D3D_OFF       = 1124,
    STTR_PARAMETER_QB_D3D_AMD       = 1125,
    STTR_PARAMETER_QB_D3D_AMD_OFF   = 1126,
    STTR_PARAMETER_QB_D3D_NV        = 1127,
    STTR_PARAMETER_QB_D3D_NV_OFF    = 1128,

    STTR_PARAMETER_CONTROL_NO        = 1130,
    STTR_PARAMETER_CONTROL_BLUELINE  = 1131,
    STTR_PARAMETER_CONTROL_WHITELINE = 1132,
    STTR_PARAMETER_CONTROL_ED        = 1134,

    // about info
    STTR_PLUGIN_TITLE       = 2000,
    STTR_VERSION_STRING     = 2001,
    STTR_PLUGIN_DESCRIPTION = 2002,

    // errors
    STTR_NO_GL_QUADBUFFER   = 3001,

};

#endif // __StOutPageFlipStrings_h_
