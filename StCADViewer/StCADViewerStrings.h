/**
 * This source is a part of sView program.
 *
 * Copyright © Kirill Gavrilov, 2011-2016
 */

#ifndef __StCADViewerStrings_h_
#define __StCADViewerStrings_h_

#include <stTypes.h>

class StLangMap;

namespace StCADViewerStrings {

    // strings' ids in the language dictionary
    enum {
        // Root -> View menu
        MENU_VIEW = 1200,
        MENU_VIEW_FULLSCREEN   = 1202,
        MENU_VIEW_TRIHEDRON    = 1204,
        MENU_VIEW_PROJECTION   = 1206,
        MENU_VIEW_FITALL       = 1208,

        // Root -> View menu -> Projection
        MENU_VIEW_PROJ_ORTHO   = 1240,
        MENU_VIEW_PROJ_PERSP   = 1241,
        MENU_VIEW_PROJ_STEREO  = 1242,

        // Root -> Help menu
        MENU_HELP         = 1500,
        MENU_HELP_ABOUT   = 1501,
        MENU_HELP_LICENSE = 1503,
        MENU_HELP_LANGS   = 1504,
        MENU_HELP_HOTKEYS = 1510,
        MENU_HELP_SETTINGS= 1511,

        // About dialog
        ABOUT_DPLUGIN_NAME     = 3000,
        ABOUT_VERSION          = 3001,
        ABOUT_DESCRIPTION      = 3002,
        ABOUT_SYSTEM           = 3004,

        BUTTON_CLOSE           = 4000,
        BUTTON_CANCEL          = 4001,
        BUTTON_DELETE          = 4007,
        BUTTON_DEFAULT         = 4008,
        BUTTON_DEFAULTS        = 4009,
        BUTTON_ASSIGN          = 4010,

        // keys reserved for actions (see StCADViewer::ActionId)
        ACTIONS_FROM           = 6000,

    };

    /**
     * Load default strings for entries not found in language file.
     */
    ST_LOCAL void loadDefaults(StLangMap& theStrings);

};

#endif //__StCADViewerStrings_h_
