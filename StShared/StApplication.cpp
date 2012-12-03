/**
 * Copyright © 2009-2012 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#include <StCore/StApplication.h>
#include <StLibrary.h>

StApplication::AppFunctions::AppFunctions()
: StApplication_new(NULL),
  StApplication_del(NULL),
  StApplication_isOpened(NULL),
  StApplication_isFullscreen(NULL),
  StApplication_create(NULL),
  StApplication_open(NULL),
  StApplication_callback(NULL) {
    //
}

StApplication::AppFunctions::~AppFunctions() {}

void StApplication::AppFunctions::load(StLibrary& theLib) {
    theLib("StApplication_new",          StApplication_new);
    theLib("StApplication_del",          StApplication_del);
    theLib("StApplication_isOpened",     StApplication_isOpened);
    theLib("StApplication_isFullscreen", StApplication_isFullscreen);
    theLib("StApplication_create",       StApplication_create);
    theLib("StApplication_open",         StApplication_open);
    theLib("StApplication_callback",     StApplication_callback);
}

bool StApplication::AppFunctions::isNull() const {
    return StApplication_new == NULL      || StApplication_del == NULL
        || StApplication_isOpened == NULL || StApplication_create == NULL
        || StApplication_isFullscreen == NULL
        || StApplication_open == NULL     || StApplication_callback == NULL;
}

void StApplication::AppFunctions::nullify() {
    StApplication_new = NULL;
    StApplication_del = NULL;
    StApplication_isOpened = NULL;
    StApplication_isFullscreen = NULL;
    StApplication_create = NULL;
    StApplication_open = NULL;
    StApplication_callback = NULL;
}

namespace {
    static StApplication::AppFunctions ST_APP_FUNCTIONS;
};

StApplication::AppFunctions& StApplication::GetFunctions() {
    return ST_APP_FUNCTIONS;
}
