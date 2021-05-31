/**
 * StCore, window system independent C++ toolkit for writing OpenGL applications.
 * Copyright © 2007-2009 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#ifndef __StWinErrorCodes_h_
#define __StWinErrorCodes_h_

enum StWinErrorCodes {
    STWIN_INITNOTSTART = -1,
    STWIN_INIT_SUCCESS = 0,

    // windows error codes
    STWIN_ERROR_WIN32_REGCLASS = 100,
    STWIN_ERROR_WIN32_CREATE = 101,
    STWIN_ERROR_WIN32_GLDC = 102,
    STWIN_ERROR_WIN32_PIXELFORMATF = 103,
    STWIN_ERROR_WIN32_PIXELFORMATS = 104,
    STWIN_ERROR_WIN32_GLRC_CREATE = 105,
    STWIN_ERROR_WIN32_GLRC_SHARE = 106,
    STWIN_ERROR_WIN32_GLRC_ACTIVATE = 107,

    STWIN_DEINIT_SUCCESS = 0,
    STWIN_ERROR_WIN32_UNREGCLASS = 150,
    STWIN_ERROR_WIN32_DESTROY = 151,
    STWIN_ERROR_WIN32_GLDC_RELEASE = 152,
    STWIN_ERROR_WIN32_GLRC_DELETE = 155,
    STWIN_ERROR_WIN32_GLRC_RELEASE = 156,

    // x-window error codes
    STWIN_ERROR_X_OPENDISPLAY = 200,
    STWIN_ERROR_X_NOGLX = 201,
    STWIN_ERROR_X_NORGB = 202,
    STWIN_ERROR_X_GLRC_CREATE = 203,
    STWIN_ERROR_X_CREATEWIN = 204,

    // x-window error codes
    STWIN_ERROR_COCOA_NO_APP    = 300,
    STWIN_ERROR_COCOA_NO_GL     = 301,
    STWIN_ERROR_COCOA_CREATEWIN = 302,

};

#endif //__StWinErrorCodes_h_
