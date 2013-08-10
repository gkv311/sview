/**
 * Copyright © 2013 Kirill Gavrilov <kirill@sview.ru>
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

#include "StMoviePlayerStrings.h"

#include <StStrings/StLangMap.h>

namespace StMoviePlayerStrings {

void loadDefaults(StLangMap& theStrings) {
    theStrings(BUTTON_CLOSE,
               "Close");
    theStrings(BUTTON_RESET,
               "Reset");
    theStrings(MENU_MEDIA,
               "Media");
    theStrings(MENU_VIEW,
               "View");
    theStrings(MENU_AUDIO,
               "Audio");
    theStrings(MENU_AUDIO_NONE,
               "None");
    theStrings(MENU_AUDIO_DELAY,
               "Audio/Video delay");
    theStrings(MENU_AUDIO_ATTACH,
               "Attach from file");
    theStrings(DIALOG_AUDIO_DELAY_TITLE,
               "Audio/Video syncronization");
    theStrings(DIALOG_AUDIO_DELAY_DESC,
               "Enter positive value if audio appears earlier than video and negative otherwise.");
    theStrings(DIALOG_AUDIO_DELAY_LABEL,
               "Audio delay:");
    theStrings(DIALOG_AUDIO_DELAY_UNITS,
               "seconds");
    theStrings(MENU_SUBTITLES,
               "Subtitles");
    theStrings(MENU_SUBTITLES_NONE,
               "None");
    theStrings(MENU_SUBTITLES_ATTACH,
               "Attach from file");
    theStrings(MENU_HELP,
               "Help");
    theStrings(MENU_MEDIA_OPEN_MOVIE,
               "Open Movie...");
    theStrings(MENU_MEDIA_SAVE_SNAPSHOT_AS,
               "Save Snapshot As...");
    theStrings(MENU_MEDIA_SRC_FORMAT,
               "Source stereo format");
    theStrings(MENU_MEDIA_AL_DEVICE,
               "Audio Device"),
    theStrings(MENU_MEDIA_GPU_DECODING,
               "Video decoding on GPU");
    theStrings(MENU_MEDIA_SHUFFLE,
               "Shuffle");
    theStrings(MENU_MEDIA_RECENT,
               "Recent files");
    theStrings(MENU_MEDIA_WEBUI,
               "Web UI");
    theStrings(MENU_MEDIA_WEBUI_OFF,
               "Turn Off");
    theStrings(MENU_MEDIA_WEBUI_ONCE,
               "Launch once");
    theStrings(MENU_MEDIA_WEBUI_ON,
               "Launch every time");
    theStrings(MENU_MEDIA_WEBUI_SHOW_ERRORS,
               "Show errors");
    theStrings(WEBUI_ERROR_PORT_BUSY,
               "Web UI can not be started on {0} port!");
    theStrings(MENU_MEDIA_QUIT,
               "Quit");
    theStrings(MENU_MEDIA_OPEN_MOVIE_1,
               "From One file");
    theStrings(MENU_MEDIA_OPEN_MOVIE_2,
               "Left+Right files");
    theStrings(MENU_SRC_FORMAT_AUTO,
               "Autodetection");
    theStrings(MENU_SRC_FORMAT_MONO,
               "Mono");
    theStrings(MENU_SRC_FORMAT_CROSS_EYED,
               "Cross-eyed");
    theStrings(MENU_SRC_FORMAT_PARALLEL,
               "Parallel Pair");
    theStrings(MENU_SRC_FORMAT_OVERUNDER_RL,
               "Over/Under (R/L)");
    theStrings(MENU_SRC_FORMAT_OVERUNDER_LR,
               "Over/Under (L/R)");
    theStrings(MENU_SRC_FORMAT_INTERLACED,
               "Interlaced");
    theStrings(MENU_SRC_FORMAT_ANA_RC,
               "Anaglyph Red/Cyan");
    theStrings(MENU_SRC_FORMAT_ANA_RB,
               "Anaglyph Green/Red+Blue");
    theStrings(MENU_SRC_FORMAT_ANA_YB,
               "Anaglyph Yellow/Blue");
    theStrings(MENU_SRC_FORMAT_PAGEFLIP,
               "Frame-sequential");
    theStrings(MENU_SRC_FORMAT_TILED_4X,
               "Tiled 4X");
    theStrings(MENU_SRC_FORMAT_SEPARATE,
               "2 streams");
    theStrings(MENU_MEDIA_RECENT_CLEAR,
               "Clear history");
    theStrings(MENU_VIEW_DISPLAY_MODE,
               "Stereo Output");
    theStrings(MENU_VIEW_FULLSCREEN,
               "Fullscreen");
    theStrings(MENU_VIEW_RESET,
               "Reset");
    theStrings(MENU_VIEW_SWAP_LR,
               "Swap Left/Right");
    theStrings(MENU_VIEW_DISPLAY_RATIO,
               "Display Ratio");
    theStrings(MENU_VIEW_TEXFILTER,
               "Smooth Filter");
    theStrings(MENU_VIEW_IMAGE_ADJUST,
               "Image Adjust");
    theStrings(MENU_VIEW_ADJUST_RESET,
               "Reset to defaults");
    theStrings(MENU_VIEW_ADJUST_BRIGHTNESS,
               "Brightness");
    theStrings(MENU_VIEW_ADJUST_SATURATION,
               "Saturation");
    theStrings(MENU_VIEW_ADJUST_GAMMA,
               "Gamma");
    theStrings(MENU_VIEW_SURFACE,
               "Surface");
    theStrings(MENU_VIEW_SURFACE_PLANE,
               "Plane");
    theStrings(MENU_VIEW_SURFACE_SPHERE,
               "Sphere");
    theStrings(MENU_VIEW_SURFACE_CYLINDER,
               "Cylinder");
    theStrings(MENU_VIEW_DISPLAY_MODE_STEREO,
               "Stereo");
    theStrings(MENU_VIEW_DISPLAY_MODE_LEFT,
               "Left view");
    theStrings(MENU_VIEW_DISPLAY_MODE_RIGHT,
               "Right view");
    theStrings(MENU_VIEW_DISPLAY_MODE_PARALLEL,
               "Parallel pair");
    theStrings(MENU_VIEW_DISPLAY_MODE_CROSSYED,
               "Cross-eyed pair");
    theStrings(MENU_VIEW_TEXFILTER_NEAREST,
               "Nearest");
    theStrings(MENU_VIEW_TEXFILTER_LINEAR,
               "Linear");
    theStrings(MENU_VIEW_TEXFILTER_BLEND,
               "Blend Deinterlace");
    theStrings(MENU_CHANGE_DEVICE,
               "Change Device");
    theStrings(MENU_ABOUT_RENDERER,
               "About Plugin...");
    theStrings(MENU_FPS,
               "FPS Control");
    theStrings(MENU_FPS_VSYNC,
               "VSync");
    theStrings(MENU_FPS_METER,
               "Show FPS");
    theStrings(MENU_FPS_BOUND,
               "Reduce CPU usage");

    theStrings(ABOUT_DPLUGIN_NAME,
               "sView - Movie Player");
    theStrings(ABOUT_VERSION,
               "version");
    theStrings(ABOUT_DESCRIPTION,
               "Movie player allows you to play stereoscopic video.\n"
               "(C) 2007-2013 Kirill Gavrilov <kirill@sview.ru>\n"
               "Official site: www.sview.ru\n"
               "\n"
               "This program distributed under GPL3.0");
    theStrings(MENU_HELP_ABOUT,
               "About...");
    theStrings(MENU_HELP_USERTIPS,
               "User Tips");
    theStrings(MENU_HELP_LICENSE,
               "License text");
    theStrings(MENU_HELP_SYSINFO,
               "System Info");
    theStrings(MENU_HELP_EXPERIMENTAL,
               "Experimental features");
    theStrings(MENU_HELP_BLOCKSLP,
               "Block sleeping");
    theStrings(MENU_HELP_UPDATES,
               "Check for updates");
    theStrings(MENU_HELP_LANGS,
               "Language");
    theStrings(MENU_HELP_BLOCKSLP_NEVER,
               "Never");
    theStrings(MENU_HELP_BLOCKSLP_PLAYBACK,
               "During Playback");
    theStrings(MENU_HELP_BLOCKSLP_FULLSCR,
               "When in fullscreen");
    theStrings(MENU_HELP_BLOCKSLP_ALWAYS,
               "Always");
    theStrings(MENU_HELP_UPDATES_NOW,
               "Now");
    theStrings(MENU_HELP_UPDATES_DAY,
               "Each day");
    theStrings(MENU_HELP_UPDATES_WEEK,
               "Each week");
    theStrings(MENU_HELP_UPDATES_YEAR,
               "Each year");
    theStrings(MENU_HELP_UPDATES_NEVER,
               "Never");
    theStrings(FILE_VIDEO_OPEN,
               "Open another movie");
    theStrings(BTN_SRC_FORMAT,
               "Source format:\n");
    theStrings(VIDEO_PLAYPAUSE,
               "Play/Pause");
    theStrings(VIDEO_LIST_PREV,
               "Play Previous File");
    theStrings(VIDEO_LIST_NEXT,
               "Play Next File");
    theStrings(VIDEO_LIST,
               "Show/Hide playlist");
    theStrings(FULLSCREEN,
               "Switch\nfullscreen/windowed");
    theStrings(UPDATES_NOTIFY,
               "A new version of sView is available on the official site www.sview.ru.\n"
               "Please update your program.");
    theStrings(DIALOG_OPEN_LEFT,
               "Choose LEFT video file to open");
    theStrings(DIALOG_OPEN_FILE,
               "Choose the video file to open");
    theStrings(DIALOG_OPEN_RIGHT,
               "Choose RIGHT video file to open");
    theStrings(DIALOG_NOTHING_TO_SAVE,
               "Nothing to save!");
    theStrings(DIALOG_NO_SNAPSHOT,
               "Snapshot not available!");
    theStrings(DIALOG_SAVE_SNAPSHOT,
               "Choose location to save snapshot");
}

};
