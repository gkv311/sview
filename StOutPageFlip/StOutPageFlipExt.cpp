/**
 * Copyright © 2007-2013 Kirill Gavrilov <kirill@sview.ru>
 *
 * StOutPageFlip library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * StOutPageFlip library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.
 * If not, see <http://www.gnu.org/licenses/>.
 */

#include "StOutPageFlipExt.h"

#include <StSys/StSys.h>
#include <StGL/StGLEnums.h>
#include <StGL/StGLContext.h>
#include <StGLCore/StGLCore20.h>
#include <StGLStereo/StGLStereoFrameBuffer.h>
#include <StCore/StWindow.h>
#include <StSettings/StSettings.h>
#include <StSettings/StTranslations.h>

namespace {
    static const StString ST_SETTING_DEV_CONTROL = "deviceControl";
    static const StString ST_SETTING_ADVANCED    = "advanced";
    static const StString ST_OUT_PLUGIN_NAME_EXT = "StOutPageFlip";
};

void StOutPageFlipExt::optionsStructAlloc() {
    StOutPageFlip::optionsStructAlloc();
    StTranslations aLangMap(ST_OUT_PLUGIN_NAME_EXT);

    ((StSDOnOff_t* )myOptions->options[DEVICE_OPTION_EXTRA])->value = true;

    // Quad Buffer type option
    StSDSwitch_t* aSwitchQB = ((StSDSwitch_t* )myOptions->options[DEVICE_OPTION_QUADBUFFER]);
    aSwitchQB->valuesTitles[QUADBUFFER_SOFT] = StWindow::memAllocNCopy(aLangMap.changeValueId(STTR_PARAMETER_QB_EMULATED, "OpenGL Emulated"));

    // shader switch option
    myOptions->options[DEVICE_OPTION_CONTROL] = (StSDOption_t* )StWindow::memAlloc(sizeof(StSDSwitch_t));
    myOptions->options[DEVICE_OPTION_CONTROL]->title = StWindow::memAllocNCopy(aLangMap.changeValueId(STTR_PARAMETER_CONTROL_CODE, "Glasses control codes"));
    myOptions->options[DEVICE_OPTION_CONTROL]->optionType = ST_DEVICE_OPTION_SWITCH;
    StSDSwitch_t* aSwitchCtrl = ((StSDSwitch_t* )myOptions->options[DEVICE_OPTION_CONTROL]);
    aSwitchCtrl->value = myDeviceCtrl;
    aSwitchCtrl->valuesCount = 4;
    aSwitchCtrl->valuesTitles = (stUtf8_t** )StWindow::memAlloc(aSwitchCtrl->valuesCount * sizeof(stUtf8_t*));
    aSwitchCtrl->valuesTitles[DEVICE_CONTROL_NONE]      = StWindow::memAllocNCopy(aLangMap.changeValueId(STTR_PARAMETER_CONTROL_NO,        "No codes"));
    aSwitchCtrl->valuesTitles[DEVICE_CONTROL_BLUELINE]  = StWindow::memAllocNCopy(aLangMap.changeValueId(STTR_PARAMETER_CONTROL_BLUELINE,  "Blue line sync"));
    aSwitchCtrl->valuesTitles[DEVICE_CONTROL_WHITELINE] = StWindow::memAllocNCopy(aLangMap.changeValueId(STTR_PARAMETER_CONTROL_WHITELINE, "White line sync"));
    aSwitchCtrl->valuesTitles[DEVICE_CONTROL_ED_ON_OFF] = StWindow::memAllocNCopy(aLangMap.changeValueId(STTR_PARAMETER_CONTROL_ED,        "eDimensional auto on/off"));
}

void StOutPageFlipExt::setDeviceControl(DeviceControlEnum newDeviceControl) {
    switch(newDeviceControl) {
        case DEVICE_CONTROL_BLUELINE:
            myCodesLine.setBlueColor();
            myDeviceCtrl = newDeviceControl;
            getDeviceControl()->setMode(StGLDeviceControl::OUT_UNDEFINED);
            break;
        case DEVICE_CONTROL_WHITELINE:
            myCodesLine.setWhiteColor();
            myDeviceCtrl = newDeviceControl;
            getDeviceControl()->setMode(StGLDeviceControl::OUT_UNDEFINED);
            break;
        case DEVICE_CONTROL_ED_ON_OFF:
            myDeviceCtrl = newDeviceControl;
            getDeviceControl()->setMode(StGLDeviceControl::OUT_UNDEFINED);
            break;
        default: myDeviceCtrl = DEVICE_CONTROL_NONE;
    }
}

StOutPageFlipExt::StOutPageFlipExt(const StHandle<StSettings>& theSettings)
: StOutPageFlip(theSettings),
  myVpSizeY(0),
  myVpSizeX(0),
  myDeviceCtrl(DEVICE_CONTROL_NONE),
  myIsQuiting(false) {
    //
    myDeviceOptionsNb = DEVICE_OPTION_CONTROL + 1;
    myQuadBufferMax = QUADBUFFER_SOFT;
#if (defined(_WIN32) || defined(__WIN32__))
    if(myIsVistaPlus) {
#endif
        myWinAttribs.isSlave = true;
        myWinAttribs.isSlaveHLineTop = true;
        myWinAttribs.isSlaveHide = true;
#if (defined(_WIN32) || defined(__WIN32__))
    }
#endif
    setDeviceControl(DEVICE_CONTROL_NONE);
}

StOutPageFlipExt::~StOutPageFlipExt() {
    if(!myContext.isNull()) {
        myCodesLine.release(*myContext);
        myCodesEDOnOff.release(*myContext);
    }
    if(!myStCore.isNull() && !mySettings.isNull()) {
        mySettings->saveInt32(ST_SETTING_DEV_CONTROL, myDeviceCtrl);
    }
}

bool StOutPageFlipExt::init(const StString&     theRendererPath,
                            const int&          theDeviceId,
                            const StNativeWin_t theNativeParent) {
    // load shutter glasses controller
    int32_t aDevCtrlInt = myDeviceCtrl;
    mySettings->loadInt32(ST_SETTING_DEV_CONTROL, aDevCtrlInt);
    myDeviceCtrl = DeviceControlEnum(aDevCtrlInt);

    if(!StOutPageFlip::init(theRendererPath, theDeviceId, theNativeParent)) {
        return false;
    }

    // initialize device controls
    myCodesLine.stglInit(*myContext);
    myCodesEDOnOff.stglInit(*myContext);

    setDeviceControl(myDeviceCtrl);

    return true;
}

void StOutPageFlipExt::stglResize(const StRectI_t& theWinRect) {
    myVpSizeY = theWinRect.height();
    myVpSizeX = theWinRect.width();
    if(!getStWindow()->isFullScreen()
#if (defined(_WIN32) || defined(__WIN32__))
    && myIsVistaPlus
#endif
    ) {
        if(myMonitor.isNull()) {
            myMonitor = new StMonitor(StCore::getMonitorFromPoint(theWinRect.center()));
        } else if(!myMonitor->getVRect().isPointIn(theWinRect.center())) {
            *myMonitor = StCore::getMonitorFromPoint(theWinRect.center());
        }
        myVpSizeX = myMonitor->getVRect().width();
        if(getDeviceControl() != NULL) {
            myVpSizeY = getDeviceControl()->getSizeY();
        }
    }
}

void StOutPageFlipExt::parseKeys(bool* theKeysMap) {
    if(theKeysMap[ST_VK_F1]) {
        setDeviceControl(DEVICE_CONTROL_NONE);
        theKeysMap[ST_VK_F1] = false;

        // send 'update' message to StDrawer
        StMessage_t msg; msg.uin = StMessageList::MSG_DEVICE_OPTION;
        StSDSwitch_t* option = ((StSDSwitch_t* )myOptions->options[DEVICE_OPTION_CONTROL]);
        option->value = myDeviceCtrl;
        msg.data = (void* )option->valuesTitles[option->value];
        getStWindow()->appendMessage(msg);
    }
    if(theKeysMap[ST_VK_F2]) {
        setDeviceControl(DEVICE_CONTROL_BLUELINE);
        theKeysMap[ST_VK_F2] = false;

        // send 'update' message to StDrawer
        StMessage_t msg; msg.uin = StMessageList::MSG_DEVICE_OPTION;
        StSDSwitch_t* option = ((StSDSwitch_t* )myOptions->options[DEVICE_OPTION_CONTROL]);
        option->value = myDeviceCtrl;
        msg.data = (void* )option->valuesTitles[option->value];
        getStWindow()->appendMessage(msg);
    }
    if(theKeysMap[ST_VK_F3]) {
        setDeviceControl(DEVICE_CONTROL_WHITELINE);
        theKeysMap[ST_VK_F3] = false;

        // send 'update' message to StDrawer
        StMessage_t msg; msg.uin = StMessageList::MSG_DEVICE_OPTION;
        StSDSwitch_t* option = ((StSDSwitch_t* )myOptions->options[DEVICE_OPTION_CONTROL]);
        option->value = myDeviceCtrl;
        msg.data = (void* )option->valuesTitles[option->value];
        getStWindow()->appendMessage(msg);
    }
    if(theKeysMap[ST_VK_F5]) {
        setDeviceControl(DEVICE_CONTROL_ED_ON_OFF);
        theKeysMap[ST_VK_F5] = false;

        // send 'update' message to StDrawer
        StMessage_t msg; msg.uin = StMessageList::MSG_DEVICE_OPTION;
        StSDSwitch_t* option = ((StSDSwitch_t* )myOptions->options[DEVICE_OPTION_CONTROL]);
        option->value = myDeviceCtrl;
        msg.data = (void* )option->valuesTitles[option->value];
        getStWindow()->appendMessage(msg);
    }
    StOutPageFlip::parseKeys(theKeysMap);
}

void StOutPageFlipExt::updateOptions(const StSDOptionsList_t* theOptions,
                                     StMessage_t&             theMsg) {
    setDeviceControl(DeviceControlEnum(((StSDSwitch_t* )theOptions->options[DEVICE_OPTION_CONTROL])->value));
    StOutPageFlip::updateOptions(theOptions, theMsg);
}

void StOutPageFlipExt::callback(StMessage_t* theMessages) {
    StOutPageFlip::callback(theMessages);
    for(size_t i = 0; theMessages[i].uin != StMessageList::MSG_NULL; ++i) {
        if(theMessages[i].uin == StMessageList::MSG_EXIT) {
            if(isControlOn() && getStWindow()->isStereoOutput()) {
                myIsQuiting = true;
                double timeMS = getDeviceControl()->quitMS();
                StTimer stQuitTimer(true);
                while(stQuitTimer.getElapsedTimeInMilliSec() <= timeMS) {
                    stglDraw(0);
                    StThread::sleep(10);
                }
                dxRelease();
            }
            break;
        }
    }
}

void StOutPageFlipExt::stglDrawExtra(unsigned int theView,
                                     int          theMode) {
    if(!isControlOn()) {
        getStWindow()->hide(ST_WIN_SLAVE);
        return;
    }
    getDeviceControl()->setMode(myIsQuiting ? StGLDeviceControl::OUT_MONO : theMode);
    if(!getDeviceControl()->isActive()) {
        getStWindow()->hide(ST_WIN_SLAVE);
        return;
    }

    const bool toDrawWindowed = !getStWindow()->isFullScreen()
#if (defined(_WIN32) || defined(__WIN32__))
        && myIsVistaPlus
#endif
    ;
    if(!toDrawWindowed) {
        getStWindow()->hide(ST_WIN_SLAVE);
    }
    if(toDrawWindowed) {
        myVpSizeY = getDeviceControl()->getSizeY();
        setSlavePosition(getDeviceControl()->getSlaveId());
        getStWindow()->show(ST_WIN_SLAVE);
        getStWindow()->stglMakeCurrent(ST_WIN_SLAVE);
        myContext->core20fwd->glViewport(0, 0, myVpSizeX, myVpSizeY); // always update slave window Viewport

        if(myQuadBuffer == QUADBUFFER_HARD_OPENGL) {
            if(!getStWindow()->isStereoOutput()) {
                myContext->core20fwd->glDrawBuffer(GL_BACK);
            } else {
                myContext->core20fwd->glDrawBuffer(theView == ST_DRAW_LEFT ? GL_BACK_LEFT : GL_BACK_RIGHT);
            }
        }
        myContext->core20fwd->glClear(GL_COLOR_BUFFER_BIT);  // clear the screen
    }
    getDeviceControl()->stglDraw(*myContext, theView, myVpSizeX, myVpSizeY);
    if(toDrawWindowed) {
        if(myQuadBuffer == QUADBUFFER_HARD_OPENGL) {
            if(theView != ST_DRAW_LEFT) {
                getStWindow()->stglSwap(ST_WIN_SLAVE);
            }
        } else {
            getStWindow()->stglSwap(ST_WIN_SLAVE);
        }
    }
}

void StOutPageFlipExt::setSlavePosition(int thePositionId) {
    StWinAttributes_t aWinAttribs = stDefaultWinAttributes();
    getStWindow()->getAttributes(&aWinAttribs);
    StWinAttributes_t anOrigAttribs = aWinAttribs;

    // unset concurrent values
    aWinAttribs.isSlaveHLineTop    = false;
    aWinAttribs.isSlaveHTop2Px     = false;
    aWinAttribs.isSlaveHLineBottom = false;

    if(thePositionId == StGLDeviceControl::SLAVE_HLINE_TOP) {
        aWinAttribs.isSlaveHLineTop = true;
    } else if(thePositionId == StGLDeviceControl::SLAVE_HTOP2PX) {
        aWinAttribs.isSlaveHTop2Px = true;
    } else if(thePositionId == StGLDeviceControl::SLAVE_HLINE_BOTTOM) {
        aWinAttribs.isSlaveHLineBottom = true;
    }

    // update only if changed
    if(!areSame(&anOrigAttribs, &aWinAttribs)) {
        getStWindow()->setAttributes(&aWinAttribs);
    }
}
