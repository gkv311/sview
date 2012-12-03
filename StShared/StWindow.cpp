/**
 * Copyright © 2007-2012 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#include <StCore/StWindow.h>
#include <StLibrary.h>

StWindow::WindowFunctions::WindowFunctions() {
    nullify();
}

void StWindow::WindowFunctions::load(StLibrary& theLib) {
    #define ST_WIN_READ_FUNC(theFunc) theLib(#theFunc, theFunc)

    ST_WIN_READ_FUNC(StWindow_new);
    ST_WIN_READ_FUNC(StWindow_del);
    ST_WIN_READ_FUNC(StWindow_close);
    ST_WIN_READ_FUNC(StWindow_setTitle);
    ST_WIN_READ_FUNC(StWindow_getAttributes);
    ST_WIN_READ_FUNC(StWindow_setAttributes);
    ST_WIN_READ_FUNC(StWindow_isActive);
    ST_WIN_READ_FUNC(StWindow_isStereoOutput);
    ST_WIN_READ_FUNC(StWindow_setStereoOutput);
    ST_WIN_READ_FUNC(StWindow_show);
    ST_WIN_READ_FUNC(StWindow_showCursor);
    ST_WIN_READ_FUNC(StWindow_isFullScreen);
    ST_WIN_READ_FUNC(StWindow_setFullScreen);
    ST_WIN_READ_FUNC(StWindow_getPlacement);
    ST_WIN_READ_FUNC(StWindow_setPlacement);
    ST_WIN_READ_FUNC(StWindow_getMousePos);
    ST_WIN_READ_FUNC(StWindow_getMouseDown);
    ST_WIN_READ_FUNC(StWindow_getMouseUp);
    ST_WIN_READ_FUNC(StWindow_getDragNDropFile);
    ST_WIN_READ_FUNC(StWindow_stglCreate);
    ST_WIN_READ_FUNC(StWindow_stglSwap);
    ST_WIN_READ_FUNC(StWindow_stglMakeCurrent);
    ST_WIN_READ_FUNC(StWindow_stglGetTargetFps);
    ST_WIN_READ_FUNC(StWindow_stglSetTargetFps);
    ST_WIN_READ_FUNC(StWindow_callback);
    ST_WIN_READ_FUNC(StWindow_appendMessage);
    ST_WIN_READ_FUNC(StWindow_getValue);
    ST_WIN_READ_FUNC(StWindow_setValue);
    ST_WIN_READ_FUNC(StWindow_memAlloc);
    ST_WIN_READ_FUNC(StWindow_memFree);
}

bool StWindow::WindowFunctions::isNull() const {
    return StWindow_new  == NULL || StWindow_del == NULL || StWindow_close == NULL
        || StWindow_callback == NULL
        || StWindow_appendMessage == NULL
        || StWindow_stglSwap == NULL         || StWindow_stglCreate == NULL || StWindow_stglMakeCurrent == NULL
        || StWindow_stglGetTargetFps == NULL || StWindow_stglSetTargetFps == NULL
        || StWindow_setTitle == NULL
        || StWindow_getAttributes == NULL    || StWindow_setAttributes == NULL
        || StWindow_isActive == NULL
        || StWindow_isStereoOutput == NULL   || StWindow_setStereoOutput == NULL
        || StWindow_show == NULL             || StWindow_showCursor == NULL
        || StWindow_isFullScreen == NULL     || StWindow_setFullScreen == NULL
        || StWindow_getPlacement == NULL     || StWindow_setPlacement == NULL
        || StWindow_getMousePos == NULL      || StWindow_getMouseDown == NULL || StWindow_getMouseUp == NULL
        || StWindow_getDragNDropFile == NULL
        || StWindow_getValue == NULL         || StWindow_setValue == NULL
        || StWindow_memAlloc == NULL         || StWindow_memFree == NULL;
}

void StWindow::WindowFunctions::nullify() {
    stMemSet(this, 0, sizeof(StWindow::WindowFunctions));
}

namespace {
    static StWindow::WindowFunctions ST_WINDOW_FUNCTIONS;
};

StWindow::WindowFunctions& StWindow::GetFunctions() {
    return ST_WINDOW_FUNCTIONS;
}
