/**
 * Copyright © 2011-2013 Kirill Gavrilov <kirill@sview.ru>
 *
 * StCore library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * StCore library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.
 * If not, see <http://www.gnu.org/licenses/>.
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

int StWinHandles::glCreateContext(StWinHandles* theSlave,
                                  const int     theDepthSize,
                                  const bool    theIsQuadStereo,
                                  const bool    theDebugCtx) {
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
