/**
 * Copyright © 2010-2012 Kirill Gavrilov <kirill@sview.ru>
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

#include <StCore/StCore.h>
#include <StCore/StWindow.h>
#include <StSettings/StSettings.h>

#include <StGL/StGLContext.h>
#include <StGLCore/StGLCore20.h>

namespace {
    static bool HAS_LOGGER_ID = StLogger::IdentifyModule("StDiagnostics");
};

const StString StDiagnostics::ST_DRAWER_PLUGIN_NAME("StDiagnostics");

StDiagnostics::StDiagnostics()
: stWin(NULL),
  stSettings(NULL),
  stGUI(NULL),
  bQuit(false) {
    //
    stGUI = new StDiagnosticsGUI(this);
}

StDiagnostics::~StDiagnostics() {
    /**if(stSettings != NULL) {
        stSettings->saveInt32(ST_SETTING_FPSBOUND, fpsBound);
    }*/

    delete stGUI;
    delete stSettings;
    delete stWin;
    StCore::FREE();
}

bool StDiagnostics::init(StWindowInterface* inStWin) {
    if(!StVersionInfo::checkTimeBomb("sView - Stereoscopic Device Diagnostics plugin")) {
        return false;
    }
    // Firstly INIT core library!
    if(StCore::INIT() != STERROR_LIBNOERROR) {
        stError("StDiagnostics, Core library not available!");
        return false;
    }
    stWin = new StWindow(inStWin);
    stWin->setTitle("sView - Stereoscopic Device Diagnostics");

    // initialize GL context
    myContext = new StGLContext();
    if(!myContext->stglInit()) {
        stError("VideoPlugin, OpenGL context is broken!\n(OpenGL library internal error?)");
        return false;
    } else if(!myContext->isGlGreaterEqual(2, 0)) {
        stError("VideoPlugin, OpenGL2.0+ not available!");
        return false;
    }
    stGUI->setContext(myContext);

    // INIT settings library
    stSettings = new StSettings(ST_DRAWER_PLUGIN_NAME);
    stWin->stglSetTargetFps(50.0);
    stWin->setStereoOutput(true);

    if(!stGUI->stglInit()) {
        return false;
    }

    /*if(!mySettings->isValid()) {
        myGUI->myMsgStack->doPushMessage("Settings plugin is not available!\nAll changes will be lost after restart.");
    }*/
    return true;
}

bool StDiagnostics::open(const StOpenInfo& ) {
    return true;
}

void StDiagnostics::parseCallback(StMessage_t* stMessages) {
    if(bQuit) {
        stMessages[0].uin = StMessageList::MSG_EXIT;
        stMessages[1].uin = StMessageList::MSG_NULL;
    }
    size_t evId(0);
    for(; stMessages[evId].uin != StMessageList::MSG_NULL; ++evId) {
        switch(stMessages[evId].uin) {
            case StMessageList::MSG_RESIZE: {
                stGUI->stglResize(stWin->getPlacement());
                break;
            }
            case StMessageList::MSG_CLOSE:
            case StMessageList::MSG_EXIT: {
                stMessages[0].uin = StMessageList::MSG_EXIT;
                stMessages[1].uin = StMessageList::MSG_NULL;
                break;
            }
            case StMessageList::MSG_KEYS: {
                bool* keysMap = (bool* )stMessages[evId].data;
                if(keysMap[ST_VK_ESCAPE]) {
                    // we could parse Escape key in other way
                    stMessages[0].uin = StMessageList::MSG_EXIT;
                    stMessages[1].uin = StMessageList::MSG_NULL;
                    return;
                }
                if(keysMap[ST_VK_F]) {
                    doSwitchFullscreen();
                    keysMap[ST_VK_F] = false;
                }
                if(keysMap[ST_VK_RETURN]) {
                    doSwitchFullscreen();
                    keysMap[ST_VK_RETURN] = false;
                }
                if(keysMap[ST_VK_M]) {
                    stWin->setStereoOutput(false);
                    keysMap[ST_VK_M] = false;
                }
                if(keysMap[ST_VK_S]) {
                    stWin->setStereoOutput(true);
                    keysMap[ST_VK_S] = false;
                }
                break;
            }
            case StMessageList::MSG_MOUSE_DOWN: {
                StPointD_t pt;
                int mouseBtn = stWin->getMouseDown(&pt);
                stGUI->tryClick(pt, mouseBtn);
                break;
            }
            case StMessageList::MSG_MOUSE_UP: {
                StPointD_t pt;
                int mouseBtn = stWin->getMouseUp(&pt);
                if(mouseBtn == ST_MOUSE_MIDDLE) {
                    doSwitchFullscreen();
                }
                stGUI->tryUnClick(pt, mouseBtn);
                break;
            }
        }
    }

    stGUI->setVisibility(stWin->getMousePos(), true);
}

void StDiagnostics::stglDraw(unsigned int view) {
    stGUI->getCamera()->setView(view);
    if(view == ST_DRAW_LEFT) {
        stGUI->stglUpdate(stWin->getMousePos());
    }

    // clear the screen and the depth buffer
    myContext->core20fwd->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // draw GUI
    stGUI->stglDraw(view);
}

void StDiagnostics::doSwitchFullscreen(const size_t ) {
    stWin->setFullScreen(!stWin->isFullScreen());
}

ST_EXPORT StDrawerInterface* StDrawer_new() {
    return new StDiagnostics(); }
ST_EXPORT void StDrawer_del(StDrawerInterface* inst) {
    delete (StDiagnostics* )inst; }
ST_EXPORT stBool_t StDrawer_init(StDrawerInterface* inst, StWindowInterface* stWin) {
    return ((StDiagnostics* )inst)->init(stWin); }
ST_EXPORT stBool_t StDrawer_open(StDrawerInterface* inst, const StOpenInfo_t* stOpenInfo) {
    return ((StDiagnostics* )inst)->open(StOpenInfo(stOpenInfo)); }
ST_EXPORT void StDrawer_parseCallback(StDrawerInterface* inst, StMessage_t* stMessages) {
    ((StDiagnostics* )inst)->parseCallback(stMessages); }
ST_EXPORT void StDrawer_stglDraw(StDrawerInterface* inst, unsigned int view) {
    ((StDiagnostics* )inst)->stglDraw(view); }

// SDK version was used
ST_EXPORT void getSDKVersion(StVersion* ver) {
    *ver = StVersionInfo::getSDKVersion();
}

// plugin version
ST_EXPORT void getPluginVersion(StVersion* ver) {
    *ver = StVersionInfo::getSDKVersion();
}

ST_EXPORT const stUtf8_t* getMIMEDescription() {
    return NULL;
}
