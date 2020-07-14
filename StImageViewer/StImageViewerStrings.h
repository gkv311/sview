/**
 * Copyright © 2009-2020 Kirill Gavrilov <kirill@sview.ru>
 *
 * StImageViewer program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * StImageViewer program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program.
 * If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __StImageViewerStrings_h_
#define __StImageViewerStrings_h_

#include <StStrings/StString.h>

class StLangMap;

namespace StImageViewerStrings {

    // strings' ids in the language dictionary
    enum {
        IMAGE_PREVIOUS = 1000,
        IMAGE_NEXT = 1001,
        SWAP_LR_OFF = 1002,
        SWAP_LR_ON = 1003,
        BTN_SRC_FORMAT = 1004,
        IMAGE_OPEN = 1015,
        PLAYLIST   = 1028,
        FULLSCREEN = 1029,

        // Root -> Media menu
        MENU_MEDIA = 1100,
        MENU_MEDIA_OPEN_IMAGE = 1101,
        MENU_MEDIA_SAVE_IMAGE_AS = 1102,
        MENU_MEDIA_SRC_FORMAT = 1103,
        MENU_MEDIA_FILE_INFO  = 1104,
        MENU_MEDIA_QUIT = 1109,

        // Root -> Media menu -> Open File menu
        MENU_MEDIA_OPEN_IMAGE_1 = 1110,
        MENU_MEDIA_OPEN_IMAGE_2 = 1111,

        // Root -> Media menu -> Source Format menu
        MENU_SRC_FORMAT_AUTO = 1130,
        MENU_SRC_FORMAT_MONO = 1131,
        MENU_SRC_FORMAT_CROSS_EYED = 1132,
        MENU_SRC_FORMAT_PARALLEL = 1133,
        MENU_SRC_FORMAT_OVERUNDER_RL = 1134,
        MENU_SRC_FORMAT_OVERUNDER_LR = 1135,
        MENU_SRC_FORMAT_INTERLACED = 1136,
        MENU_SRC_FORMAT_ANA_RC = 1137,
        MENU_SRC_FORMAT_ANA_RB = 1138,
        MENU_SRC_FORMAT_ANA_YB = 1139,
        MENU_SRC_FORMAT_SEPARATE = 1142,

        // Root -> View menu
        MENU_VIEW = 1200,
        MENU_VIEW_DISPLAY_MODE  = 1201,
        MENU_VIEW_FULLSCREEN    = 1202,
        MENU_VIEW_RESET         = 1203,
        MENU_VIEW_SWAP_LR       = 1204,
        MENU_VIEW_DISPLAY_RATIO = 1205,
        MENU_VIEW_TEXFILTER     = 1206,
        MENU_VIEW_IMAGE_ADJUST  = 1207,
        MENU_VIEW_PANORAMA      = 1208,

        MENU_VIEW_DISPLAY_MODE_STEREO       = 1210,
        MENU_VIEW_DISPLAY_MODE_LEFT         = 1211,
        MENU_VIEW_DISPLAY_MODE_RIGHT        = 1212,
        MENU_VIEW_DISPLAY_MODE_PARALLEL     = 1213,
        MENU_VIEW_DISPLAY_MODE_CROSSYED     = 1214,

        MENU_VIEW_DISPLAY_RATIO_SRC     = 1250,
        MENU_VIEW_RATIO_KEEP_ON_RESTART = 1251,
        MENU_VIEW_RATIO_HEAL_ANAMORPHIC = 1252,

        MENU_VIEW_TEXFILTER_NEAREST = 1260,
        MENU_VIEW_TEXFILTER_LINEAR  = 1261,
        MENU_VIEW_TEXFILTER_TRILINEAR = 1263,

        MENU_VIEW_ADJUST_RESET      = 1270,
        MENU_VIEW_ADJUST_BRIGHTNESS = 1271,
        MENU_VIEW_ADJUST_SATURATION = 1272,
        MENU_VIEW_ADJUST_GAMMA      = 1273,

        MENU_VIEW_SURFACE_PLANE     = 1280,
        MENU_VIEW_SURFACE_SPHERE    = 1281,
        MENU_VIEW_SURFACE_CYLINDER  = 1282,
        MENU_VIEW_SURFACE_CUBEMAP   = 1283,
        MENU_VIEW_SURFACE_HEMISPHERE= 1284,
        MENU_VIEW_TRACK_HEAD        = 1285,
        MENU_VIEW_TRACK_HEAD_POOR   = 1286,
        MENU_VIEW_STICK_PANORAMA360 = 1288,
        MENU_VIEW_FLIPZ_CUBE6x1     = 1291,
        MENU_VIEW_FLIPZ_CUBE3x2     = 1292,
        MENU_VIEW_SURFACE_CUBEMAP_EAC = 1293,
        MENU_VIEW_SURFACE_THEATER   = 1294,

        // Root -> Output -> Change Device menu
        MENU_CHANGE_DEVICE  = 1400,
        MENU_ABOUT_RENDERER = 1401,
        MENU_SHOW_FPS       = 1402,
        MENU_VSYNC          = 1403,
        MENU_EXCLUSIVE_FULLSCREEN = 1404,

        // Root -> Help menu
        MENU_HELP         = 1500,
        MENU_HELP_ABOUT   = 1501,
        MENU_HELP_UPDATES = 1502,
        MENU_HELP_LICENSE = 1503,
        MENU_HELP_LANGS   = 1504,
        MENU_HELP_USERTIPS= 1506,
        MENU_HELP_SYSINFO = 1508,
        MENU_HELP_SCALE   = 1509,
        MENU_HELP_HOTKEYS = 1510,
        MENU_HELP_SETTINGS= 1511,

        // Root -> Help -> Check for updates menu
        MENU_HELP_UPDATES_NOW   = 1520,
        MENU_HELP_UPDATES_DAY   = 1521,
        MENU_HELP_UPDATES_WEEK  = 1522,
        MENU_HELP_UPDATES_YEAR  = 1523,
        MENU_HELP_UPDATES_NEVER = 1524,

        MENU_HELP_SCALE_SMALL   = 1590,
        MENU_HELP_SCALE_NORMAL  = 1591,
        MENU_HELP_SCALE_BIG     = 1592,
        MENU_HELP_SCALE_HIDPI2X = 1593,

        // Settings -> Options
        OPTION_EXIT_ON_ESCAPE              = 1701,
        OPTION_EXIT_ON_ESCAPE_NEVER        = 1702,
        OPTION_EXIT_ON_ESCAPE_ONE_CLICK    = 1703,
        OPTION_EXIT_ON_ESCAPE_DOUBLE_CLICK = 1704,
        OPTION_EXIT_ON_ESCAPE_WINDOWED     = 1705,
        OPTION_HIDE_NAVIGATION_BAR         = 1710,
        OPTION_OPEN_LAST_ON_STARTUP        = 1711,
        OPTION_SWAP_JPS                    = 1712,

        // Open/Save dialogs
        DIALOG_OPEN_FILE       = 2000,
        DIALOG_OPEN_LEFT       = 2001,
        DIALOG_OPEN_RIGHT      = 2002,
        DIALOG_FILE_INFO       = 2003,
        DIALOG_FILE_NOINFO     = 2004,
        DIALOG_DELETE_FILE_TITLE     = 2005,
        DIALOG_DELETE_FILE_QUESTION  = 2006,
        DIALOG_SAVE_INFO_TITLE       = 2007,
        DIALOG_SAVE_INFO_QUESTION    = 2008,
        DIALOG_SAVE_INFO_UNSUPPORTED = 2009,

        DIALOG_SAVE_SNAPSHOT   = 2010,
        DIALOG_NOTHING_TO_SAVE = 2011,
        DIALOG_NO_SNAPSHOT     = 2012,

        DIALOG_ASSIGN_HOT_KEY  = 2013,
        DIALOG_CONFLICTS_WITH  = 2014,

        // About dialog
        ABOUT_DPLUGIN_NAME     = 3000,
        ABOUT_VERSION          = 3001,
        ABOUT_DESCRIPTION      = 3002,
        UPDATES_NOTIFY         = 3003,
        ABOUT_SYSTEM           = 3004,

        BUTTON_CLOSE           = 4000,
        BUTTON_CANCEL          = 4001,
        BUTTON_SAVE_METADATA   = 4006,
        BUTTON_DELETE          = 4007,
        BUTTON_DEFAULT         = 4008,
        BUTTON_DEFAULTS        = 4009,
        BUTTON_ASSIGN          = 4010,

        // metadata keys
        INFO_LEFT              = 5000,
        INFO_RIGHT             = 5001,
        INFO_FILE_NAME         = 5002,
        INFO_DIMENSIONS        = 5003,
        INFO_LOAD_TIME         = 5004,
        INFO_TIME_MSEC         = 5005,
        INFO_PIXEL_RATIO       = 5006,
        INFO_PIXEL_FORMAT      = 5007,
        INFO_NO_SRCFORMAT      = 5008,
        INFO_WRONG_SRCFORMAT   = 5009,
        INFO_NO_SRCFORMAT_EX   = 5011,
        INFO_COLOR_MODEL       = 5012,

        // metadata keys
        METADATA_JPEG_COMMENT     = 5100,
        METADATA_JPEG_JPSCOMMENT  = 5101,
        METADATA_EXIF_MAKER       = 5200,
        METADATA_EXIF_MODEL       = 5201,
        METADATA_EXIF_USERCOMMENT = 5202,
        METADATA_EXIF_DATETIME    = 5203,

        // keys reserved for actions (see StImageViewer::ActionId)
        ACTIONS_FROM              = 6000,

    };

    /**
     * Load default strings for entries not found in language file.
     */
    ST_LOCAL void loadDefaults(StLangMap& theStrings);

};

#endif // __StImageViewerStrings_h_
