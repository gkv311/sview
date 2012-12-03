/**
 * Copyright © 2007-2010 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#include <StCore/StDrawer.h>

StDrawer::StDrawer()
: stLib(),
  StDrawer_new(NULL),
  StDrawer_del(NULL),
  StDrawer_init(NULL),
  StDrawer_open(NULL),
  StDrawer_parseCallback(NULL),
  StDrawer_stglDraw(NULL),
  getMIMEDescription(NULL),
  instance(NULL) {
    //
}

bool StDrawer::InitLibrary(const StString& thePluginPath) {
    if(!stLib.loadSimple(thePluginPath)) {
        StDrawer_new = NULL;
        StDrawer_del = NULL;
        StDrawer_init = NULL;
        StDrawer_open = NULL;
        StDrawer_parseCallback = NULL;
        StDrawer_stglDraw = NULL;
        getMIMEDescription = NULL;
        return false;
    }
    stLib("StDrawer_new",           StDrawer_new);
    stLib("StDrawer_del",           StDrawer_del);
    stLib("StDrawer_init",          StDrawer_init);
    stLib("StDrawer_open",          StDrawer_open);
    stLib("StDrawer_parseCallback", StDrawer_parseCallback);
    stLib("StDrawer_stglDraw",      StDrawer_stglDraw);
    stLib("getMIMEDescription",     getMIMEDescription);
    if(   StDrawer_new == NULL  || StDrawer_del == NULL
       || StDrawer_init == NULL || StDrawer_stglDraw == NULL
       || StDrawer_open == NULL || StDrawer_parseCallback == NULL
    ) {
        stLib.close();
        StDrawer_new = NULL;
        StDrawer_del = NULL;
        StDrawer_init = NULL;
        StDrawer_open = NULL;
        StDrawer_parseCallback = NULL;
        StDrawer_stglDraw = NULL;
        getMIMEDescription = NULL;
        return false;
    }
    return true;
}

void StDrawer::Destruct() {
    if(StDrawer_del != NULL) {
        StDrawer_del(instance);
        instance = NULL;
    }
}
