/**
 * Copyright © 2013-2020 Kirill Gavrilov <kirill@sview.ru>
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

#include "StImageViewer.h"
#include <StStrings/StLangMap.h>

// Degree sign in UTF-8 encoding.
#define THE_DEGREE_SIGN "\xC2\xB0"

namespace StImageViewerStrings {

inline void addAction(StLangMap&              theStrings,
                      const int               theAction,
                      const StString&         theAlias,
                      const char*             theDefValue) {
    theStrings(ACTIONS_FROM + theAction, theDefValue);
    theStrings.addAlias(theAlias, ACTIONS_FROM + theAction);
}

void loadDefaults(StLangMap& theStrings) {
    theStrings(BUTTON_CLOSE,
               "Close");
    theStrings(BUTTON_CANCEL,
               "Cancel");
    theStrings(BUTTON_SAVE_METADATA,
               "Save");
    theStrings(BUTTON_DELETE,
               "Delete");
    theStrings(BUTTON_DEFAULT,
               "Default");
    theStrings(BUTTON_DEFAULTS,
               "Defaults");
    theStrings(BUTTON_ASSIGN,
               "Assign");
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
               "Source");
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
    theStrings(MENU_VIEW_PANORAMA,
               "Panorama");
    theStrings(MENU_VIEW_SURFACE_PLANE,
               "Plane");
    theStrings(MENU_VIEW_SURFACE_SPHERE,
               "Sphere");
    theStrings(MENU_VIEW_SURFACE_HEMISPHERE,
               "Hemisphere");
    theStrings(MENU_VIEW_SURFACE_CYLINDER,
               "Cylinder");
    theStrings(MENU_VIEW_SURFACE_CUBEMAP,
               "Cubemap");
    theStrings(MENU_VIEW_SURFACE_THEATER,
               "Theater");
    theStrings(MENU_VIEW_SURFACE_CUBEMAP_EAC,
               "Equiangular cubemap");
    theStrings(MENU_VIEW_TRACK_HEAD,
               "Track orientation");
    theStrings(MENU_VIEW_TRACK_HEAD_POOR,
               "Track orientation (poor)");
    theStrings(MENU_VIEW_STICK_PANORAMA360,
               "Stick at panorama 360" THE_DEGREE_SIGN);
    theStrings(MENU_VIEW_FLIPZ_CUBE6x1,
               "Cubemap 6x1 - flip Z");
    theStrings(MENU_VIEW_FLIPZ_CUBE3x2,
               "Cubemap 3x2 - flip Z");
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
    theStrings(MENU_VIEW_RATIO_KEEP_ON_RESTART,
               "Keep on restart");
    theStrings(MENU_VIEW_RATIO_HEAL_ANAMORPHIC,
               "Heal anamorphic 1080p/720p");
    theStrings(MENU_VIEW_TEXFILTER_NEAREST,
               "Nearest");
    theStrings(MENU_VIEW_TEXFILTER_LINEAR,
               "Linear"),
    theStrings(MENU_VIEW_TEXFILTER_TRILINEAR,
               "Trilinear"),
    theStrings(MENU_CHANGE_DEVICE,
               "Change Device");
    theStrings(MENU_ABOUT_RENDERER,
               "About Plugin...");
    theStrings(MENU_SHOW_FPS,
               "Show FPS");
    theStrings(MENU_VSYNC,
               "VSync");
    theStrings(MENU_EXCLUSIVE_FULLSCREEN,
               "Exclusive Fullscreen mode");
    theStrings(ABOUT_DPLUGIN_NAME,
               "sView - Image Viewer");
    theStrings(ABOUT_VERSION,
               "version");
    theStrings(ABOUT_DESCRIPTION,
               "Image viewer allows you to open stereoscopic images in formats JPEG, PNG, MPO and others.\n"
               "(C) 2007-2016 Kirill Gavrilov <kirill@sview.ru>\n"
               "Official site: www.sview.ru\n"
               "\n"
               "This program is distributed under GPL3.0");
    theStrings(ABOUT_SYSTEM,
               "System Info");
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
    theStrings(MENU_HELP_HOTKEYS,
               "Hotkeys");
    theStrings(MENU_HELP_SETTINGS,
               "Settings");
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
               "Source format:");

    theStrings(OPTION_EXIT_ON_ESCAPE,
               "Exit sView on Escape");
    theStrings(OPTION_EXIT_ON_ESCAPE_NEVER,
               "Never");
    theStrings(OPTION_EXIT_ON_ESCAPE_ONE_CLICK,
               "One click");
    theStrings(OPTION_EXIT_ON_ESCAPE_DOUBLE_CLICK,
               "Double click");
    theStrings(OPTION_EXIT_ON_ESCAPE_WINDOWED,
               "When windowed");
    theStrings(OPTION_HIDE_NAVIGATION_BAR,
               "Hide system navigation bar");
    theStrings(OPTION_OPEN_LAST_ON_STARTUP,
               "Open last viewed file on startup");
    theStrings(OPTION_SWAP_JPS,
               "Swap JPS/PNS views order");

    theStrings(UPDATES_NOTIFY,
               "A new version of sView is available on the official site www.sview.ru.\n"
               "Please update your program.");
    theStrings(DIALOG_OPEN_FILE,
               "Choose the image file to open");
    theStrings(DIALOG_OPEN_LEFT,
               "Choose LEFT image file to open");
    theStrings(DIALOG_OPEN_RIGHT,
               "Choose RIGHT image file to open");
    theStrings(DIALOG_FILE_INFO,
               "Image Info");
    theStrings(DIALOG_FILE_NOINFO,
               "Information is unavailable");
    theStrings(DIALOG_DELETE_FILE_TITLE,
               "File deletion");
    theStrings(DIALOG_DELETE_FILE_QUESTION,
               "Do you really want to completely remove this file?");
    theStrings(DIALOG_SAVE_INFO_TITLE,
               "File metadata saving");
    theStrings(DIALOG_SAVE_INFO_QUESTION,
               "Do you really want to save metadata to the file?");
    theStrings(DIALOG_SAVE_INFO_UNSUPPORTED,
               "Metadata can be saved only into JPEG files.");
    theStrings(DIALOG_NOTHING_TO_SAVE,
               "Nothing to save!");
    theStrings(DIALOG_NO_SNAPSHOT,
               "Snapshot not available!");
    theStrings(DIALOG_SAVE_SNAPSHOT,
               "Choose location to save snapshot");
    theStrings(DIALOG_ASSIGN_HOT_KEY,
               "Assign new Hot Key for action\n<i>{0}</i>");
    theStrings(DIALOG_CONFLICTS_WITH,
               "Conflicts with: <i>{0}</i>");

    theStrings(INFO_LEFT,
               "[left]");
    theStrings(INFO_RIGHT,
               "[right]");
    theStrings(INFO_FILE_NAME,
               "File name");
    theStrings(INFO_DIMENSIONS,
               "Dimensions");
    theStrings(INFO_LOAD_TIME,
               "Load time");
    theStrings(INFO_TIME_MSEC,
               "msec");
    theStrings(INFO_PIXEL_RATIO,
               "Pixel ratio");
    theStrings(INFO_PIXEL_FORMAT,
               "Pixel format");
    theStrings(INFO_COLOR_MODEL,
               "Color model");
    theStrings(INFO_NO_SRCFORMAT,
               "(does not stored in metadata)");
    theStrings(INFO_WRONG_SRCFORMAT,
               "(does not match metadata)");
    theStrings(INFO_NO_SRCFORMAT_EX,
               "(does not stored in metadata\nbut detected from file name)");

    theStrings(METADATA_JPEG_COMMENT,
               "JPEG comment");
    theStrings(METADATA_JPEG_JPSCOMMENT,
               "JPS comment");

    theStrings(METADATA_EXIF_MAKER,
               "Camera maker");
    theStrings(METADATA_EXIF_MODEL,
               "Camera model");
    theStrings(METADATA_EXIF_USERCOMMENT,
               "User comment");
    theStrings(METADATA_EXIF_DATETIME,
               "Image timestamp");

    // define metadata keys
    theStrings.addAlias("Jpeg.Comment",        METADATA_JPEG_COMMENT);
    theStrings.addAlias("Jpeg.JpsComment",     METADATA_JPEG_JPSCOMMENT);
    theStrings.addAlias("Exif.Image.Make",     METADATA_EXIF_MAKER);
    theStrings.addAlias("Exif.Image.Model",    METADATA_EXIF_MODEL);
    theStrings.addAlias("Exif.UserComment",    METADATA_EXIF_USERCOMMENT);
    theStrings.addAlias("Exif.Image.DateTime", METADATA_EXIF_DATETIME);

    // define actions
    addAction(theStrings, StImageViewer::Action_Fullscreen,
              "DoFullscreen",
              "Switch fullscreen/windowed");
    addAction(theStrings, StImageViewer::Action_ShowFps,
              "DoShowFPS",
              "Show/hide FPS meter");
    addAction(theStrings, StImageViewer::Action_SrcAuto,
              "DoSrcAuto",
              "Stereo format - Auto");
    addAction(theStrings, StImageViewer::Action_SrcMono,
              "DoSrcMono",
              "Stereo format - Mono");
    addAction(theStrings, StImageViewer::Action_SrcOverUnderLR,
              "DoSrcOverUnder",
              "Stereo format - Over/Under");
    addAction(theStrings, StImageViewer::Action_SrcSideBySideRL,
              "DoSrcSideBySide",
              "Stereo format - Side by side");
    addAction(theStrings, StImageViewer::Action_FileInfo,
              "DoFileInfo",
              "Show file info");
    addAction(theStrings, StImageViewer::Action_ListFirst,
              "DoListFirst",
              "Playlist - Go to the first item");
    addAction(theStrings, StImageViewer::Action_ListLast,
              "DoListLast",
              "Playlist - Go to the last item");
    addAction(theStrings, StImageViewer::Action_ListPrev,
              "DoListPrev",
              "Playlist - Go to the previous item");
    addAction(theStrings, StImageViewer::Action_ListNext,
              "DoListNext",
              "Playlist - Go to the next item");
    addAction(theStrings, StImageViewer::Action_SlideShow,
              "DoSlideShow",
              "Playlist - Start/stop slideshow");
    addAction(theStrings, StImageViewer::Action_SavePng,
              "DoSaveImageAsPng",
              "Save in PNG format");
    addAction(theStrings, StImageViewer::Action_SaveJpeg,
              "DoSaveImageAsJpeg",
              "Save in JPEG format");
    addAction(theStrings, StImageViewer::Action_SaveFileInfo,
              "DoSaveFileInfo",
              "Save file metadata");
    addAction(theStrings, StImageViewer::Action_DeleteFile,
              "DoDeleteFile",
              "Delete the file from file system");
    addAction(theStrings, StImageViewer::Action_ImageAdjustReset,
              "DoImageAdjustReset",
              "Reset image adjustment");

    // image region actions
    addAction(theStrings, StImageViewer::Action_StereoParamsBegin + StGLImageRegion::Action_Reset,
              "DoParamsReset",
              "Reset image position");
    addAction(theStrings, StImageViewer::Action_StereoParamsBegin + StGLImageRegion::Action_SwapLR,
              "DoParamsSwapLR",
              "Swap Left/Right");
    addAction(theStrings, StImageViewer::Action_StereoParamsBegin + StGLImageRegion::Action_GammaDec,
              "DoParamsGammaDec",
              "Gamma correction - decrease");
    addAction(theStrings, StImageViewer::Action_StereoParamsBegin + StGLImageRegion::Action_GammaInc,
              "DoParamsGammaInc",
              "Gamma correction - increase");
    addAction(theStrings, StImageViewer::Action_StereoParamsBegin + StGLImageRegion::Action_SepXDec,
              "DoParamsSepXDec",
              "DX separation - decrease");
    addAction(theStrings, StImageViewer::Action_StereoParamsBegin + StGLImageRegion::Action_SepXInc,
              "DoParamsSepXInc",
              "DX separation - increase");
    addAction(theStrings, StImageViewer::Action_StereoParamsBegin + StGLImageRegion::Action_SepYDec,
              "DoParamsSepYDec",
              "DY separation - decrease");
    addAction(theStrings, StImageViewer::Action_StereoParamsBegin + StGLImageRegion::Action_SepYInc,
              "DoParamsSepYInc",
              "DY separation - increase");
    addAction(theStrings, StImageViewer::Action_StereoParamsBegin + StGLImageRegion::Action_SepRotDec,
              "DoParamsSepRotDec",
              "Angular separation - decrease");
    addAction(theStrings, StImageViewer::Action_StereoParamsBegin + StGLImageRegion::Action_SepRotInc,
              "DoParamsSepRotInc",
              "Angular separation - increase");
    addAction(theStrings, StImageViewer::Action_StereoParamsBegin + StGLImageRegion::Action_Rot90Counter,
              "DoParamsRotZ90Dec",
              "Rotate 90" THE_DEGREE_SIGN " counterclockwise");
    addAction(theStrings, StImageViewer::Action_StereoParamsBegin + StGLImageRegion::Action_Rot90Clockwise,
              "DoParamsRotZ90Inc",
              "Rotate 90" THE_DEGREE_SIGN " clockwise");
    addAction(theStrings, StImageViewer::Action_StereoParamsBegin + StGLImageRegion::Action_RotCounter,
              "DoParamsRotZDec",
              "Rotate counterclockwise");
    addAction(theStrings, StImageViewer::Action_StereoParamsBegin + StGLImageRegion::Action_RotClockwise,
              "DoParamsRotZInc",
              "Rotate clockwise");
    addAction(theStrings, StImageViewer::Action_StereoParamsBegin + StGLImageRegion::Action_ModeNext,
              "DoParamsModeNext",
              "Select next view mode");
    addAction(theStrings, StImageViewer::Action_StereoParamsBegin + StGLImageRegion::Action_PanLeft,
              "DoParamsPanLeft",
              "Panning - navigate to the left");
    addAction(theStrings, StImageViewer::Action_StereoParamsBegin + StGLImageRegion::Action_PanRight,
              "DoParamsPanRight",
              "Panning - navigate to the right");
    addAction(theStrings, StImageViewer::Action_StereoParamsBegin + StGLImageRegion::Action_PanUp,
              "DoParamsPanUp",
              "Panning - navigate to the top");
    addAction(theStrings, StImageViewer::Action_StereoParamsBegin + StGLImageRegion::Action_PanDown,
              "DoParamsPanDown",
              "Panning - navigate to the bottom");
    addAction(theStrings, StImageViewer::Action_StereoParamsBegin + StGLImageRegion::Action_ScaleIn,
              "DoParamsScaleIn",
              "Scale - increment");
    addAction(theStrings, StImageViewer::Action_StereoParamsBegin + StGLImageRegion::Action_ScaleOut,
              "DoParamsScaleOut",
              "Scale - decrement");
    addAction(theStrings, StImageViewer::Action_StereoParamsBegin + StGLImageRegion::Action_RotYLeft,
              "DoParamsRotYLeft",
              "Y Rotation - left");
    addAction(theStrings, StImageViewer::Action_StereoParamsBegin + StGLImageRegion::Action_RotYRight,
              "DoParamsRotYRight",
              "Y Rotation - right");
    addAction(theStrings, StImageViewer::Action_StereoParamsBegin + StGLImageRegion::Action_RotXUp,
              "DoParamsRotXUp",
              "X Rotation - up");
    addAction(theStrings, StImageViewer::Action_StereoParamsBegin + StGLImageRegion::Action_RotXDown,
              "DoParamsRotXDown",
              "X Rotation - down");

    // new actions
    addAction(theStrings, StImageViewer::Action_PanoramaOnOff,
              "DoPanoramaOnOff",
              "Enable/disable panorama mode");
    addAction(theStrings, StImageViewer::Action_ShowGUI,
              "DoShowGUI",
              "Show/hide GUI");

    theStrings.addAlias("DoOutStereoNormal",       MENU_VIEW_DISPLAY_MODE_STEREO);
    theStrings.addAlias("DoOutStereoLeftView",     MENU_VIEW_DISPLAY_MODE_LEFT);
    theStrings.addAlias("DoOutStereoRightView",    MENU_VIEW_DISPLAY_MODE_RIGHT);
    theStrings.addAlias("DoOutStereoParallelPair", MENU_VIEW_DISPLAY_MODE_PARALLEL);
    theStrings.addAlias("DoOutStereoCrossEyed",    MENU_VIEW_DISPLAY_MODE_CROSSYED);
}

};
