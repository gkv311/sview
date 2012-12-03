/**
 * Copyright © 2009-2010 Kirill Gavrilov <kirill@sview.ru>
 *
 * StBrowserPlugin NPAPI plugin is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * StBrowserPlugin NPAPI plugin is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this library.
 * If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __StMIMETypes_h__
#define __StMIMETypes_h__

// ActiveX resources
#define ST_OCX_RESTXT_NAME 1

#define ST_OCX_RESBMP_ICON 1

#define ST_DISPID_DISPOSE 311

/**
 * Plugin about info
 */
#define ST_BROWSER_PLUGIN_NAME "sView Browser plugin"
#define ST_BROWSER_PLUGIN_DESC "sView Browser plugin for stereoscoping viewing"

/**
 * MIME types, supported by plugin
 */
// JPS - jpeg stereo
#define ST_JPS_MIME   "image/jps"
#define ST_X_JPS_MIME "image/x-jps"
#define ST_JPS_EXT    "jps"
#define ST_JPS_DESC   "JPS - jpeg stereo image"

// PNS - PNG stereo
#define ST_PNS_MIME   "image/pns"
#define ST_X_PNS_MIME "image/x-pns"
#define ST_PNS_EXT    "pns"
#define ST_PNS_DESC   "PNS - png stereo image"

// MPO
#define ST_MPO_MIME   "image/mpo"
#define ST_X_MPO_MIME "image/x-mpo"
#define ST_MPO_EXT    "mpo"
#define ST_MPO_DESC   "MPO - multi picture object"

// define supported MIME lists (for Win32 resource file)
#define ST_NPAPI_MIME_LIST ST_JPS_MIME "|" ST_X_JPS_MIME "|" ST_PNS_MIME "|" ST_X_PNS_MIME "|" ST_MPO_MIME "|" ST_X_MPO_MIME "\000"
#define ST_NPAPI_MIME_EXT  ST_JPS_EXT  "|" ST_JPS_EXT    "|" ST_PNS_EXT  "|" ST_PNS_EXT    "|" ST_MPO_EXT  "|" ST_MPO_EXT "\000"
#define ST_NPAPI_MIME_DESC ST_JPS_DESC "|" ST_JPS_DESC   "|" ST_PNS_DESC "|" ST_PNS_DESC   "|" ST_MPO_DESC "|" ST_MPO_DESC "\000"

// define supported MIME list (for Linux)
#define ST_NPAPI_MIME ST_JPS_MIME ":" ST_JPS_EXT ":" ST_JPS_DESC ";" \
ST_X_JPS_MIME ":" ST_JPS_EXT ":" ST_JPS_DESC ";" \
ST_PNS_MIME   ":" ST_PNS_EXT ":" ST_PNS_DESC ";" \
ST_X_PNS_MIME ":" ST_PNS_EXT ":" ST_PNS_DESC ";" \
ST_MPO_MIME   ":" ST_MPO_EXT ":" ST_MPO_DESC ";" \
ST_X_MPO_MIME ":" ST_MPO_EXT ":" ST_MPO_DESC \
"\000"

#endif //__StMIMETypes_h__
