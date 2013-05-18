/**
 * Copyright © 2010-2013 Kirill Gavrilov <kirill@sview.ru>
 *
 * StDiagnostics program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * StDiagnostics program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program.
 * If not, see <http://www.gnu.org/licenses/>.
 */

#include "StDiagnostics.h"
#include "StDiagnosticsGUI.h"

#include <StSettings/StSettings.h>

#include <StGL/StGLContext.h>
#include <StGLCore/StGLCore20.h>

#include "../StOutAnaglyph/StOutAnaglyph.h"
#include "../StOutDual/StOutDual.h"
#include "../StOutIZ3D/StOutIZ3D.h"
#include "../StOutInterlace/StOutInterlace.h"
#include "../StOutPageFlip/StOutPageFlipExt.h"
#include "../StOutDistorted/StOutDistorted.h"

const StString StDiagnostics::ST_DRAWER_PLUGIN_NAME("StDiagnostics");

StDiagnostics::StDiagnostics(const StNativeWin_t         theParentWin,
                             const StHandle<StOpenInfo>& theOpenInfo)
: StApplication(theParentWin, theOpenInfo),
  myToQuit(false) {
    myTitle = "sView - Stereoscopic Device Diagnostics";
    myGUI = new StDiagnosticsGUI(this);

    addRenderer(new StOutAnaglyph(theParentWin));
    addRenderer(new StOutDual(theParentWin));
    addRenderer(new StOutIZ3D(theParentWin));
    addRenderer(new StOutInterlace(theParentWin));
    addRenderer(new StOutPageFlipExt(theParentWin));
    addRenderer(new StOutDistorted(theParentWin));
}

StDiagnostics::~StDiagnostics() {
    myGUI.nullify();
}

bool StDiagnostics::open() {
    if(!StApplication::open()) {
        return false;
    }

    // initialize GL context
    myContext = new StGLContext();
    if(!myContext->stglInit()) {
        stError("VideoPlugin, OpenGL context is broken!\n(OpenGL library internal error?)");
        return false;
    } else if(!myContext->isGlGreaterEqual(2, 0)) {
        stError("VideoPlugin, OpenGL2.0+ not available!");
        return false;
    }
    myGUI->setContext(myContext);

    myWindow->setTargetFps(50.0);
    myWindow->setStereoOutput(true);

    if(!myGUI->stglInit()) {
        return false;
    }
    myGUI->stglResize(myWindow->getPlacement());
    return true;
}

void StDiagnostics::doResize(const StSizeEvent& ) {
    if(myGUI.isNull()) {
        return;
    }

    myGUI->stglResize(myWindow->getPlacement());
}

void StDiagnostics::doMouseDown(const StClickEvent& theEvent) {
    if(myGUI.isNull()) {
        return;
    }

    myGUI->tryClick(StPointD_t(theEvent.PointX, theEvent.PointY), theEvent.Button);
}

void StDiagnostics::doMouseUp(const StClickEvent& theEvent) {
    if(myGUI.isNull()) {
        return;
    }

    if(theEvent.Button == ST_MOUSE_MIDDLE) {
        doSwitchFullscreen();
    }
    myGUI->tryUnClick(StPointD_t(theEvent.PointX, theEvent.PointY), theEvent.Button);
}

void StDiagnostics::doKeyDown(const StKeyEvent& theEvent) {
    if(myGUI.isNull()) {
        return;
    }

    if(myGUI->getFocus() != NULL) {
        myGUI->doKeyDown(theEvent);
        return;
    }

    switch(theEvent.VKey) {
        case ST_VK_ESCAPE:
            StApplication::exit(0);
            return;
        case ST_VK_F:
        case ST_VK_RETURN:
            doSwitchFullscreen();
            return;
        case ST_VK_M:
            myWindow->setStereoOutput(false);
            return;
        case ST_VK_S:
            myWindow->setStereoOutput(true);
            return;
        default:
            break;
    }
}

void StDiagnostics::doKeyHold(const StKeyEvent& theEvent) {
    if(!myGUI.isNull()
    && myGUI->getFocus() != NULL) {
        myGUI->doKeyHold(theEvent);
    }
}

void StDiagnostics::doKeyUp(const StKeyEvent& theEvent) {
    if(!myGUI.isNull()
    && myGUI->getFocus() != NULL) {
        myGUI->doKeyUp(theEvent);
    }
}

void StDiagnostics::beforeDraw() {
    myGUI->setVisibility(myWindow->getMousePos(), true);
}

void StDiagnostics::stglDraw(unsigned int theView) {
    if(!myContext.isNull()) {
        // clear the screen and the depth buffer
        myContext->core20fwd->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

    if(myGUI.isNull()) {
        return;
    }

    myGUI->getCamera()->setView(theView);
    if(theView == ST_DRAW_LEFT) {
        myGUI->stglUpdate(myWindow->getMousePos());
    }

    // draw GUI
    myGUI->stglDraw(theView);
}

void StDiagnostics::doSwitchFullscreen(const size_t ) {
    myWindow->setFullScreen(!myWindow->isFullScreen());
}

void StDiagnostics::doFpsClick(const size_t ) {
    myWindow->setTargetFps((myWindow->getTargetFps() > 0.0) ? 0.0 : 50.0);
}
