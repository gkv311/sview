/**
 * Copyright © 2007-2012 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#include <StCore/StRendererPlugin.h>
#include <StCore/StWindow.h>

StRendererPlugin::StRendererPlugin()
: stLib(),
  StRenderer_new(NULL),
  StRenderer_del(NULL),
  StRenderer_getStWindow(NULL),
  StRenderer_init(NULL),
  StRenderer_open(NULL),
  StRenderer_callback(NULL),
  StRenderer_stglDraw(NULL),
  StRenderer_getDevicesInfo(NULL),
  instance(NULL),
  stWindow(NULL) {
    //
}

bool StRendererPlugin::InitLibrary(const StString& thePluginPath) {
    stLib.close();
    if(!stLib.loadSimple(thePluginPath)) {
        StRenderer_new = NULL;
        StRenderer_del = NULL;
        StRenderer_getStWindow = NULL;
        StRenderer_init = NULL;
        StRenderer_open = NULL;
        StRenderer_callback = NULL;
        StRenderer_stglDraw = NULL;
        StRenderer_getDevicesInfo = NULL;
        return false;
    }
    stLib("StRenderer_new",         StRenderer_new);
    stLib("StRenderer_del",         StRenderer_del);
    stLib("StRenderer_getStWindow", StRenderer_getStWindow);
    stLib("StRenderer_init",        StRenderer_init);
    stLib("StRenderer_open",        StRenderer_open);
    stLib("StRenderer_callback",    StRenderer_callback);
    stLib("StRenderer_stglDraw",    StRenderer_stglDraw);
    stLib("getDevicesInfo",         StRenderer_getDevicesInfo);
    if(   StRenderer_new == NULL         || StRenderer_del == NULL
       || StRenderer_getStWindow == NULL || StRenderer_init == NULL
       || StRenderer_open == NULL        || StRenderer_callback == NULL || StRenderer_stglDraw == NULL
    ) {
        stLib.close();
        StRenderer_new = NULL;
        StRenderer_del = NULL;
        StRenderer_getStWindow = NULL;
        StRenderer_init = NULL;
        StRenderer_open = NULL;
        StRenderer_callback = NULL;
        StRenderer_stglDraw = NULL;
        StRenderer_getDevicesInfo = NULL;
        return false;
    }
    return true;
}

bool StRendererPlugin::init(const int&          theDeviceId,
                            const StNativeWin_t theNativeParent) {
    return init(stLib.getPath(), theDeviceId, theNativeParent);
}

bool StRendererPlugin::init(const StString&     theRendererPath,
                            const int&          theDeviceId,
                            const StNativeWin_t theNativeParent) {
    if(StRenderer_init(instance, theRendererPath.toCString(), theDeviceId, theNativeParent)) {
        stWindow = new StWindow(StRenderer_getStWindow(instance));
        return true;
    } else {
        return false;
    }
}

void StRendererPlugin::Destruct() {
    if(StRenderer_del != NULL && instance != NULL) {
        StRenderer_del(instance);
        instance = NULL;
    }
    delete stWindow;
    stWindow = NULL;
}
