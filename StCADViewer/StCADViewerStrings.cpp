/**
 * This source is a part of sView program.
 *
 * Copyright © Kirill Gavrilov, 2011-2016
 */

#include "StCADViewerStrings.h"

#include "StCADViewer.h"
#include <StStrings/StLangMap.h>

namespace StCADViewerStrings {

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
    theStrings(BUTTON_DELETE,
               "Delete");
    theStrings(BUTTON_DEFAULT,
               "Default");
    theStrings(BUTTON_DEFAULTS,
               "Defaults");
    theStrings(BUTTON_ASSIGN,
               "Assign");

    theStrings(MENU_VIEW,
               "View");
    theStrings(MENU_HELP,
               "Help");
    
    theStrings(MENU_VIEW_FULLSCREEN,
               "Fullscreen");
    theStrings(MENU_VIEW_TRIHEDRON,
               "Show trihedron");
    theStrings(MENU_VIEW_PROJECTION,
               "Projection");
    theStrings(MENU_VIEW_FITALL,
               "Fit ALL");

    theStrings(MENU_VIEW_PROJ_ORTHO,
               "Orthogonal");
    theStrings(MENU_VIEW_PROJ_PERSP,
               "Perspective");
    theStrings(MENU_VIEW_PROJ_STEREO,
               "Stereo");

    theStrings(MENU_SHOW_FPS,
               "Show FPS");

    theStrings(MENU_HELP_ABOUT,
               "About...");
    theStrings(MENU_HELP_LICENSE,
               "License text");
    theStrings(MENU_HELP_LANGS,
               "Language");
    theStrings(MENU_HELP_HOTKEYS,
               "Hotkeys");
    theStrings(MENU_HELP_SETTINGS,
               "Settings");

    theStrings(ABOUT_DPLUGIN_NAME,
               "sView - Tiny CAD Viewer");
    theStrings(ABOUT_VERSION,
               "version");
    theStrings(ABOUT_DESCRIPTION,
               "Tiny CAD viewer allows you to view CAD files in formats IGES, STEP, BREP using OpenCASCADE Technology.\n"
               "(C) 2011-2016 Kirill Gavrilov <kirill@sview.ru>\n"
               "Official site: www.sview.ru\n"
               "\n"
               "This program is distributed under GPL3.0");
    theStrings(ABOUT_SYSTEM,
               "System Info");

    // define actions
    addAction(theStrings, StCADViewer::Action_Fullscreen,
              "DoFullscreen",
              "Switch fullscreen/windowed");
    addAction(theStrings, StCADViewer::Action_ShowFps,
              "DoShowFPS",
              "Show/hide FPS meter");
    addAction(theStrings, StCADViewer::Action_FileInfo,
              "DoFileInfo",
              "Show file info");
    addAction(theStrings, StCADViewer::Action_ListFirst,
              "DoListFirst",
              "Playlist - Go to the first item");
    addAction(theStrings, StCADViewer::Action_ListLast,
              "DoListLast",
              "Playlist - Go to the last item");
    addAction(theStrings, StCADViewer::Action_ListPrev,
              "DoListPrev",
              "Playlist - Go to the previous item");
    addAction(theStrings, StCADViewer::Action_ListNext,
              "DoListNext",
              "Playlist - Go to the next item");

}

};
