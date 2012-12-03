/**
 * Copyright © 2009-2010 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#include <StCore/StDrawerInfo.h>
#include <StCore/StDrawer.h>
#include <StLibrary.h>

StDrawerInfo::StDrawerInfo()
: drawerPath(),
  mimeList(),
  bValid(false) {
    //
}

StDrawerInfo::StDrawerInfo(const StDrawerInfo& toCopy)
: drawerPath(toCopy.drawerPath),
  mimeList(toCopy.mimeList),
  bValid(toCopy.bValid) {
    //
}

StDrawerInfo::StDrawerInfo(const StString& drawerPath)
: drawerPath(drawerPath),
  mimeList(),
  bValid(false) {
    StDrawer stDrawer;
    if(stDrawer.InitLibrary(drawerPath)) {
        const stUtf8_t* aMIMEList = stDrawer.GetMIMEList();
        if(aMIMEList != NULL) {
            mimeList = StMIMEList(StString(aMIMEList));
            bValid = true;
        }
    }
}

StDrawerInfo::~StDrawerInfo() {
    //
}

const StMIME& StDrawerInfo::DRAWER_MIME() {
    static const StMIME DRAWER_MIME_VARIABLE(StString("application/x-sview-drawer"),
                                             StString(ST_DLIB_EXTENSION),
                                             StString("sView Drawer plugin"));
    return DRAWER_MIME_VARIABLE;
}

const StMIME& StDrawerInfo::CLOSE_MIME() {
    static const StMIME CLOSE_MIME_VARIABLE(StString("application/x-sview-close"),
                                            StString(ST_DLIB_EXTENSION),
                                            StString("sView Close Drawer action"));
    return CLOSE_MIME_VARIABLE;
}
