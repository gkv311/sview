/**
 * Copyright © 2007-2013 Kirill Gavrilov <kirill@sview.ru>
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
