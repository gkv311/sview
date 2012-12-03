/**
 * This source is a part of sView program.
 *
 * Copyright © Kirill Gavrilov, 2011
 */

#ifndef __StCADViewerStrings_h_
#define __StCADViewerStrings_h_

namespace StCADViewerStrings {

    // strings' ids in the language dictionary
    enum {
        // Root -> View menu
        MENU_VIEW = 1200,
        MENU_VIEW_DISPLAY_MODE = 1201,
        MENU_VIEW_FULLSCREEN   = 1202,
        MENU_VIEW_NORMALS      = 1203,
        MENU_VIEW_TRIHEDRON    = 1204,
        MENU_VIEW_TWOSIDES     = 1205,
        MENU_VIEW_PROJECTION   = 1206,
        MENU_VIEW_FILLMODE     = 1207,
        MENU_VIEW_FITALL       = 1208,

        // Root -> View menu -> Projection
        MENU_VIEW_PROJ_ORTHO   = 1240,
        MENU_VIEW_PROJ_PERSP   = 1241,
        MENU_VIEW_PROJ_STEREO  = 1242,

        // Root -> View menu -> Fill Mode
        MENU_VIEW_FILL_MESH        = 1250,
        MENU_VIEW_FILL_SHADED      = 1251,
        MENU_VIEW_FILL_SHADED_MESH = 1252,

        // Root -> Help menu
        MENU_HELP         = 1500,
        MENU_HELP_ABOUT   = 1501,
        MENU_HELP_LICENSE = 1503,
        MENU_HELP_LANGS   = 1504,

        // About dialog
        ABOUT_DPLUGIN_NAME     = 3000,
        ABOUT_VERSION          = 3001,
        ABOUT_DESCRIPTION      = 3002,

    };

};

#endif //__StCADViewerStrings_h_
