/**
 * Copyright © 2013-2014 Kirill Gavrilov <kirill@sview.ru>
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

#include "StImageViewerStrings.h"

#include <StStrings/StLangMap.h>

namespace StImageViewerStrings {

void loadDefaults(StLangMap& theStrings) {
    theStrings(BUTTON_CLOSE,
               "Close");
    theStrings(MENU_MEDIA,
               "Media");
    theStrings(MENU_VIEW,
               "View");
    theStrings(MENU_HELP,
               "Help");
    theStrings(MENU_MEDIA_OPEN_IMAGE,
               "Open Image...");
    theStrings(MENU_MEDIA_SAVE_IMAGE_AS,
               "Save Image As...");
    theStrings(MENU_MEDIA_SRC_FORMAT,
               "Source stereo format");
    theStrings(MENU_MEDIA_FILE_INFO,
               "File info");
    theStrings(MENU_MEDIA_QUIT,
               "Quit");
    theStrings(MENU_MEDIA_OPEN_IMAGE_1,
               "From One file");
    theStrings(MENU_MEDIA_OPEN_IMAGE_2,
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
    theStrings(MENU_SRC_FORMAT_SEPARATE,
               "2 streams");
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
    theStrings(MENU_VIEW_DISPLAY_RATIO_SRC,
               "Source");
    theStrings(MENU_VIEW_KEEP_ON_RESTART,
               "Keep on restart");
    theStrings(MENU_VIEW_TEXFILTER_NEAREST,
               "Nearest");
    theStrings(MENU_VIEW_TEXFILTER_LINEAR,
               "Linear"),
    theStrings(MENU_CHANGE_DEVICE,
               "Change Device");
    theStrings(MENU_ABOUT_RENDERER,
               "About Plugin...");
    theStrings(MENU_SHOW_FPS,
               "Show FPS");
    theStrings(MENU_VSYNC,
               "VSync");
    theStrings(ABOUT_DPLUGIN_NAME,
               "sView - Image Viewer");
    theStrings(ABOUT_VERSION,
               "version");
    theStrings(ABOUT_DESCRIPTION,
               "Image viewer allows you to open stereoscopic images in formats JPEG, PNG, MPO and others.\n"
               "(C) 2007-2014 Kirill Gavrilov <kirill@sview.ru>\n"
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
    theStrings(MENU_HELP_SCALE,
               "Scale Interface");
    theStrings(MENU_HELP_SCALE_SMALL,
               "Small");
    theStrings(MENU_HELP_SCALE_NORMAL,
               "Normal");
    theStrings(MENU_HELP_SCALE_BIG,
               "Big");
    theStrings(MENU_HELP_SCALE_HIDPI2X,
               "Force HiDPI 2X");
    theStrings(MENU_HELP_UPDATES,
               "Check for updates");
    theStrings(MENU_HELP_LANGS,
               "Language");
    theStrings(MENU_HELP_UPDATES_NOW,
               "Now");
    theStrings(MENU_HELP_UPDATES_DAY,
               "Each day");
    theStrings(MENU_HELP_UPDATES_WEEK,
               "Each week");
    theStrings(MENU_HELP_UPDATES_YEAR,
               "Each year");
    theStrings(MENU_HELP_UPDATES_NEVER,
               "Never"),
    theStrings(IMAGE_OPEN,
               "Open another image");
    theStrings(IMAGE_PREVIOUS,
               "Previous image");
    theStrings(IMAGE_NEXT,
               "Next image");
    theStrings(PLAYLIST,
               "PlayList");
    theStrings(FULLSCREEN,
               "Switch\n"
               "fullscreen/windowed");
    theStrings(BTN_SRC_FORMAT,
               "Source format:\n");
    theStrings(UPDATES_NOTIFY,
               "A new version of sView is available on the official site www.sview.ru.\n"
               "Please update your program.");
    theStrings(DIALOG_OPEN_FILE,
               "Choose the image file to open");
    theStrings(DIALOG_OPEN_LEFT,
               "Choose LEFT image file to open");
    theStrings(DIALOG_OPEN_RIGHT,
               "Choose RIGHT image file to open");
    theStrings(DIALOG_NOTHING_TO_SAVE,
               "Nothing to save!");
    theStrings(DIALOG_NO_SNAPSHOT,
               "Snapshot not available!");
    theStrings(DIALOG_SAVE_SNAPSHOT,
               "Choose location to save snapshot");
}

};
