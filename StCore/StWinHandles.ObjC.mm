/**
 * StCore, window system independent C++ toolkit for writing OpenGL applications.
 * Copyright © 2011-2013 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#include "StWinHandles.h"

#include <StStrings/StLogger.h>
#include <OpenGL/gl.h>

StWinHandles::StWinHandles()
: hWindow(NULL),
  hViewGl(NULL) {
    //
}

StWinHandles::~StWinHandles() {
    close();
}

void StWinHandles::glSwap() {
    if(hViewGl != NULL) {
        ///glSwapAPPLE();
        glFinish();
        [[hViewGl openGLContext] flushBuffer];
    }
}

bool StWinHandles::glMakeCurrent() {
    if(hViewGl != NULL) {
        [[hViewGl openGLContext] makeCurrentContext];
        return true;
    }
    return false;
}

int StWinHandles::glCreateContext(StWinHandles*    theSlave,
                                  const StRectI_t& theRect,
                                  const int        theDepthSize,
                                  const int        theStencilSize,
                                  const bool       theIsQuadStereo,
                                  const bool       theDebugCtx) {
    return STWIN_INIT_SUCCESS;
}

bool StWinHandles::close() {
    if(hWindow != NULL) {
        [hWindow setReleasedWhenClosed: YES];
        [hWindow forceClose];
    }
    hWindow = NULL;
    hViewGl = NULL;
    return true;
}
