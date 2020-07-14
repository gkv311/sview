/**
 * Copyright © 2009-2020 Kirill Gavrilov <kirill@sview.ru>
 *
 * StMoviePlayer program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * StMoviePlayer program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program.
 * If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __StMoviePlayerStrings_h_
#define __StMoviePlayerStrings_h_

#include <stTypes.h>

class StLangMap;

namespace StMoviePlayerStrings {

    // strings' ids in the language dictionary
    enum {
        SWAP_LR_OFF = 1002,
        SWAP_LR_ON = 1003,
        BTN_SRC_FORMAT = 1004,
        FILE_VIDEO_OPEN = 1015,
        VIDEO_PLAYPAUSE = 1020,
        VIDEO_LIST = 1021,
        VIDEO_LIST_PREV = 1022,
        VIDEO_LIST_NEXT = 1023,
        FULLSCREEN = 1029,

        // Root -> Media menu
        MENU_MEDIA = 1100,
        MENU_MEDIA_OPEN_MOVIE   = 1101,
        MENU_MEDIA_SAVE_SNAPSHOT_AS = 1102,
        MENU_MEDIA_SRC_FORMAT   = 1103,
        MENU_MEDIA_AL_DEVICE    = 1104,
        MENU_MEDIA_SHUFFLE      = 1105,
        MENU_MEDIA_RECENT       = 1106,
        MENU_MEDIA_GPU_DECODING = 1107,
        MENU_MEDIA_WEBUI        = 1108,
        MENU_MEDIA_QUIT = 1109,
        MENU_MEDIA_FILE_INFO    = 1170,

        // Root -> Media menu -> Open File menu
        MENU_MEDIA_OPEN_MOVIE_1 = 1110,
        MENU_MEDIA_OPEN_MOVIE_2 = 1111,

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
        MENU_SRC_FORMAT_PAGEFLIP = 1140,
        MENU_SRC_FORMAT_TILED_4X = 1141,
        MENU_SRC_FORMAT_SEPARATE = 1142,

        // Root -> Media menu -> Recent files menu
        MENU_MEDIA_RECENT_CLEAR = 1160,

        // Root -> Media menu -> Web UI menu
        MENU_MEDIA_WEBUI_OFF    = 1180,
        MENU_MEDIA_WEBUI_ONCE   = 1181,
        MENU_MEDIA_WEBUI_ON     = 1182,
        MENU_MEDIA_WEBUI_SHOW_ERRORS = 1185,
        WEBUI_ERROR_PORT_BUSY   = 1186,

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
        MENU_VIEW_TEXFILTER_BLEND   = 1262,
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
        MENU_VIEW_TRACK_HEAD_AUDIO  = 1287,
        MENU_VIEW_STICK_PANORAMA360 = 1288,
        MENU_VIEW_SURFACE_CUBEMAP_EAC = 1293,
        MENU_VIEW_SURFACE_THEATER   = 1294,

        // Root -> Audio menu
        MENU_AUDIO = 1300,
        MENU_AUDIO_NONE   = 1301,
        MENU_AUDIO_DELAY  = 1302,
        MENU_AUDIO_ATTACH = 1303,

        DIALOG_AUDIO_DELAY_TITLE  = 1320,
        DIALOG_AUDIO_DELAY_DESC   = 1321,
        DIALOG_AUDIO_DELAY_LABEL  = 1322,
        DIALOG_AUDIO_DELAY_UNITS  = 1323,
        //

        // Root -> Subtitles menu
        MENU_SUBTITLES = 1350,
        MENU_SUBTITLES_NONE     = 1351,
        MENU_SUBTITLES_ATTACH   = 1353,
        MENU_SUBTITLES_SIZE     = 1354,
        MENU_SUBTITLES_PARALLAX = 1355,
        MENU_SUBTITLES_PARSER   = 1356,
        MENU_SUBTITLES_PLACEMENT= 1357,
        MENU_SUBTITLES_TOP      = 1358,
        MENU_SUBTITLES_BOTTOM   = 1359,
        MENU_SUBTITLES_STEREO   = 1370,

        MENU_SUBTITLES_PLAIN_TEXT = 1360,
        MENU_SUBTITLES_LITE_HTML  = 1361,

        // Root -> Output menu
        MENU_CHANGE_DEVICE  = 1400, // Root -> Output -> Change Device menu
        MENU_ABOUT_RENDERER = 1401,
        MENU_FPS            = 1402,
        MENU_EXCLUSIVE_FULLSCREEN = 1404,

        // Root -> Output -> FPS Control menu
        MENU_FPS_VSYNC      = 1420,
        MENU_FPS_METER      = 1421,
        MENU_FPS_BOUND      = 1422,

        // Root -> Help menu
        MENU_HELP = 1500,
        MENU_HELP_ABOUT   = 1501,
        MENU_HELP_UPDATES = 1502,
        MENU_HELP_LICENSE = 1503,
        MENU_HELP_LANGS   = 1504,
        MENU_HELP_BLOCKSLP= 1505,
        MENU_HELP_USERTIPS= 1506,
        MENU_HELP_EXPERIMENTAL= 1507,
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

        // Root -> Help -> Block sleeping
        MENU_HELP_BLOCKSLP_NEVER    = 1550,
        MENU_HELP_BLOCKSLP_ALWAYS   = 1551,
        MENU_HELP_BLOCKSLP_PLAYBACK = 1552,
        MENU_HELP_BLOCKSLP_FULLSCR  = 1553,

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
        DIALOG_DELETE_FILE_TITLE    = 2005,
        DIALOG_DELETE_FILE_QUESTION = 2006,
        DIALOG_FILE_DECODERS   = 2007,
        DIALOG_OPEN_AUDIO      = 2008,
        DIALOG_OPEN_SUBTITLES  = 2009,

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
        BUTTON_RESET           = 4005,
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
        INFO_DURATION          = 5010,
        INFO_NO_SRCFORMAT_EX   = 5011,

        // metadata keys
        METADATA_TITLE         = 5300,
        METADATA_COMPOSER      = 5301,
        METADATA_ARTIST        = 5302,
        METADATA_ALBUM_ARTIST  = 5303,
        METADATA_ALBUM         = 5304,
        METADATA_DISC          = 5305,
        METADATA_DISC_TOTAL    = 5306,
        METADATA_GENRE         = 5307,
        METADATA_COMMENT       = 5308,
        METADATA_NOTES         = 5309,
        METADATA_DESCRIPTION   = 5310,
        METADATA_PUBLISHER     = 5311,
        METADATA_COPYRIGHT     = 5312,
        METADATA_ENCODER       = 5313,
        METADATA_ENGINEER      = 5314,
        METADATA_SOURCE        = 5315,
        METADATA_CREATION_TIME = 5316,
        METADATA_DATE          = 5317,
        METADATA_YEAR          = 5318,
        METADATA_LANGUAGE      = 5319,
        METADATA_TRACK         = 5320,
        METADATA_TRACK_TOTAL   = 5321,
        METADATA_TRACK_GAIN    = 5322,
        METADATA_TRACK_PEAK    = 5323,
        METADATA_ALBUM_GAIN    = 5324,
        METADATA_ALBUM_PEAK    = 5325,

        // keys reserved for actions (see StMoviePlayer::ActionId)
        ACTIONS_FROM           = 6000,

    };

    /**
     * Load default strings for entries not found in language file.
     */
    ST_LOCAL void loadDefaults(StLangMap& theStrings);

};

#endif // __StMoviePlayerStrings_h_
