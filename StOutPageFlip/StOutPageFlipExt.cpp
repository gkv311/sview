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
#include <StCore/StSearchMonitors.h>
#include <StSettings/StSettings.h>
#include <StSettings/StTranslations.h>

namespace {
    static const StString ST_SETTING_DEV_CONTROL = "deviceControl";
    static const StString ST_SETTING_ADVANCED    = "advanced";
    static const StString ST_OUT_PLUGIN_NAME_EXT = "StOutPageFlip";
};

void StOutPageFlipExt::doSetDeviceControl(const int32_t theValue) {
    switch(theValue) {
        case DEVICE_CONTROL_BLUELINE:
            myCodesLine.setBlueColor();
            getDeviceControl()->setMode(StGLDeviceControl::OUT_UNDEFINED);
            break;
        case DEVICE_CONTROL_WHITELINE:
            myCodesLine.setWhiteColor();
            getDeviceControl()->setMode(StGLDeviceControl::OUT_UNDEFINED);
            break;
        case DEVICE_CONTROL_ED_ON_OFF:
            getDeviceControl()->setMode(StGLDeviceControl::OUT_UNDEFINED);
            break;
        default:
            break;
    }
    myToResetDevice = true;
}

void StOutPageFlipExt::getOptions(StParamsList& theList) const {
    StOutPageFlip::getOptions(theList);
    if(StOutPageFlip::params.ToShowExtra->getValue()) {
        theList.add(params.ControlCode);
    }
}

StOutPageFlipExt::StOutPageFlipExt(const StNativeWin_t theParentWindow)
: StOutPageFlip(theParentWindow),
  myVpSizeY(0),
  myVpSizeX(0),
  myIsQuiting(false) {

    // Control Code option
    params.ControlCode = new StEnumParam(DEVICE_CONTROL_NONE, myLangMap.changeValueId(STTR_PARAMETER_CONTROL_CODE, "Glasses control codes"));
    params.ControlCode->changeValues().add(myLangMap.changeValueId(STTR_PARAMETER_CONTROL_NO,        "No codes"));
    params.ControlCode->changeValues().add(myLangMap.changeValueId(STTR_PARAMETER_CONTROL_BLUELINE,  "Blue line sync"));
    params.ControlCode->changeValues().add(myLangMap.changeValueId(STTR_PARAMETER_CONTROL_WHITELINE, "White line sync"));
    params.ControlCode->changeValues().add(myLangMap.changeValueId(STTR_PARAMETER_CONTROL_ED,        "eDimensional auto on/off"));
    params.ControlCode->signals.onChanged.connect(this, &StOutPageFlipExt::doSetDeviceControl);

    // load shutter glasses controller
    mySettings->loadParam(ST_SETTING_DEV_CONTROL, params.ControlCode);
    myToResetDevice = false;
}

void StOutPageFlipExt::releaseResources() {
    if(!myContext.isNull()) {
        myCodesLine.release(*myContext);
        myCodesEDOnOff.release(*myContext);
    }
    mySettings->saveParam(ST_SETTING_DEV_CONTROL, params.ControlCode);
    StOutPageFlip::releaseResources();
}

StOutPageFlipExt::~StOutPageFlipExt() {
    releaseResources();
}

bool StOutPageFlipExt::create() {
    // request slave
    StWinAttributes_t anAttribs = stDefaultWinAttributes();
    StWindow::getAttributes(anAttribs);
    if(params.ControlCode->getValue() != DEVICE_CONTROL_NONE) {
#ifdef _WIN32
    if(myIsVistaPlus) {
#endif
        anAttribs.isSlave         = true;
        anAttribs.isSlaveHLineTop = true;
        anAttribs.isSlaveHide     = true;
#ifdef _WIN32
    }
#endif
    } else {
        anAttribs.isSlave         = false;
        anAttribs.isSlaveHLineTop = false;
        anAttribs.isSlaveHide     = false;
    }
    StWindow::setAttributes(anAttribs);
    if(!StOutPageFlip::create()) {
        return false;
    }

    // initialize device controls
    myCodesLine.stglInit(*myContext);
    myCodesEDOnOff.stglInit(*myContext);

    doSetDeviceControl(params.ControlCode->getValue());
    myToResetDevice = false;

    return true;
}

void StOutPageFlipExt::stglResize(const StRectI_t& theWinRect) {
    myVpSizeY = theWinRect.height();
    myVpSizeX = theWinRect.width();
    if(!StOutPageFlip::params.ToShowExtra->getValue()) {
        return;
    }

    if(!StWindow::isFullScreen()
#ifdef _WIN32
    && myIsVistaPlus
#endif
    ) {
        const StSearchMonitors& aMonitors = StWindow::getMonitors();
        if(myMonitor.isNull()) {
            myMonitor = new StMonitor(aMonitors[theWinRect.center()]);
        } else if(!myMonitor->getVRect().isPointIn(theWinRect.center())) {
            *myMonitor = aMonitors[theWinRect.center()];
        }
        myVpSizeX = myMonitor->getVRect().width();
        if(getDeviceControl() != NULL) {
            myVpSizeY = getDeviceControl()->getSizeY();
        }
    }
}

void StOutPageFlipExt::processEvents(StMessage_t* theMessages) {
    StOutPageFlip::processEvents(theMessages);
    if(!StOutPageFlip::params.ToShowExtra->getValue()) {
        return;
    }

    for(size_t anIter = 0; theMessages[anIter].uin != StMessageList::MSG_NULL; ++anIter) {
        if(theMessages[anIter].uin == StMessageList::MSG_EXIT) {
            if(isControlOn() && StWindow::isStereoOutput()) {
                myIsQuiting = true;
                const double aTime = getDeviceControl()->quitMS();
                StTimer aQuitTimer(true);
                while(aQuitTimer.getElapsedTimeInMilliSec() <= aTime) {
                    stglDraw();
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
    if(!StOutPageFlip::params.ToShowExtra->getValue()) {
        return;
    }

    if(!isControlOn()) {
        StWindow::hide(ST_WIN_SLAVE);
        return;
    }
    getDeviceControl()->setMode(myIsQuiting ? StGLDeviceControl::OUT_MONO : theMode);
    if(!getDeviceControl()->isActive()) {
        StWindow::hide(ST_WIN_SLAVE);
        return;
    }

    const bool toDrawWindowed = !StWindow::isFullScreen()
#ifdef _WIN32
        && myIsVistaPlus
#endif
    ;
    if(!toDrawWindowed) {
        StWindow::hide(ST_WIN_SLAVE);
    }
    if(toDrawWindowed) {
        myVpSizeY = getDeviceControl()->getSizeY();
        setSlavePosition(getDeviceControl()->getSlaveId());
        StWindow::show(ST_WIN_SLAVE);
        StWindow::stglMakeCurrent(ST_WIN_SLAVE);
        myContext->core20fwd->glViewport(0, 0, myVpSizeX, myVpSizeY); // always update slave window Viewport

        if(StOutPageFlip::params.QuadBuffer->getValue() == QUADBUFFER_HARD_OPENGL) {
            if(!StWindow::isStereoOutput()) {
                myContext->core20fwd->glDrawBuffer(GL_BACK);
            } else {
                myContext->core20fwd->glDrawBuffer(theView == ST_DRAW_LEFT ? GL_BACK_LEFT : GL_BACK_RIGHT);
            }
        }
        myContext->core20fwd->glClear(GL_COLOR_BUFFER_BIT);  // clear the screen
    }
    getDeviceControl()->stglDraw(*myContext, theView, myVpSizeX, myVpSizeY);
    if(toDrawWindowed) {
        if(StOutPageFlip::params.QuadBuffer->getValue() == QUADBUFFER_HARD_OPENGL) {
            if(theView != ST_DRAW_LEFT) {
                StWindow::stglSwap(ST_WIN_SLAVE);
            }
        } else {
            StWindow::stglSwap(ST_WIN_SLAVE);
        }
    }
}

void StOutPageFlipExt::setSlavePosition(int thePositionId) {
    StWinAttributes_t aWinAttribs = stDefaultWinAttributes();
    StWindow::getAttributes(aWinAttribs);
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
        StWindow::setAttributes(aWinAttribs);
    }
}
