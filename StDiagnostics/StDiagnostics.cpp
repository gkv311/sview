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
: StApplication(theParentWin, theOpenInfo) {
    myTitle = "sView - Stereoscopic Device Diagnostics";
    params.IsFullscreen = new StBoolParam(false);
    params.IsFullscreen->signals.onChanged.connect(this, &StDiagnostics::doFullscreen);

    myGUI = new StDiagnosticsGUI(this);

    addRenderer(new StOutAnaglyph(theParentWin));
    addRenderer(new StOutDual(theParentWin));
    addRenderer(new StOutIZ3D(theParentWin));
    addRenderer(new StOutInterlace(theParentWin));
    addRenderer(new StOutDistorted(theParentWin));
    addRenderer(new StOutPageFlipExt(theParentWin));

    // create actions
    StHandle<StAction> anAction;
    anAction = new StActionBool(stCString("DoFullscreen"), params.IsFullscreen);
    addAction(Action_Fullscreen, anAction, ST_VK_F, ST_VK_RETURN);

    anAction = new StActionIntSlot(stCString("DoStereoModeOn"),  stSlot(this, &StDiagnostics::doStereoMode), 1);
    addAction(Action_StereoModeOn,  anAction, ST_VK_S);

    anAction = new StActionIntSlot(stCString("DoStereoModeOff"), stSlot(this, &StDiagnostics::doStereoMode), 0);
    addAction(Action_StereoModeOff, anAction, ST_VK_M);
}

StDiagnostics::~StDiagnostics() {
    myGUI.nullify();
}

bool StDiagnostics::open() {
    if(!StApplication::open()) {
        myMsgQueue->popAll();
        return false;
    }

    // initialize GL context
    myContext = myWindow->getContext();
    myContext->setMessagesQueue(myMsgQueue);
    if(!myContext->isGlGreaterEqual(2, 0)) {
        myMsgQueue->pushError(stCString("OpenGL 2.0 is required by StDiagnostics!"));
        myMsgQueue->popAll();
        return false;
    }
    myGUI->setContext(myContext);

    myWindow->setTargetFps(50.0);
    myWindow->setStereoOutput(true);

    if(!myGUI->stglInit()) {
        myMsgQueue->pushError(stCString("StDiagnostics - critical error:\nGUI initialization failed!"));
        myMsgQueue->popAll();
        myGUI.nullify();
        return false;
    }
    myGUI->stglResize(myWindow->stglViewport(ST_WIN_MASTER));
    registerHotKeys();
    return true;
}

void StDiagnostics::doResize(const StSizeEvent& ) {
    if(myGUI.isNull()) {
        return;
    }

    myGUI->stglResize(myWindow->stglViewport(ST_WIN_MASTER));
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
        params.IsFullscreen->reverse();
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

    StApplication::doKeyDown(theEvent);
    switch(theEvent.VKey) {
        case ST_VK_ESCAPE:
            StApplication::exit(0);
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
    if(myGUI.isNull()) {
        return;
    }

    myGUI->setVisibility(myWindow->getMousePos(), true);
}

void StDiagnostics::stglDraw(unsigned int theView) {
    if(!myContext.isNull()
    && myContext->core20fwd != NULL) {
        // clear the screen and the depth buffer
        myContext->core20fwd->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

    if(myGUI.isNull()) {
        return;
    }

    myGUI->getCamera()->setView(theView);
    if(theView == ST_DRAW_LEFT
    || theView == ST_DRAW_MONO) {
        myGUI->stglUpdate(myWindow->getMousePos());
    }

    // draw GUI
    myGUI->stglDraw(theView);
}

void StDiagnostics::doFullscreen(const bool theIsFullscreen) {
    if(!myWindow.isNull()) {
        myWindow->setFullScreen(theIsFullscreen);
    }
}

void StDiagnostics::doStereoMode(const size_t theMode) {
    if(!myWindow.isNull()) {
        myWindow->setStereoOutput(theMode != 0);
    }
}

void StDiagnostics::doFpsClick(const size_t ) {
    myWindow->setTargetFps((myWindow->getTargetFps() > 0.0) ? 0.0 : 50.0);
}
