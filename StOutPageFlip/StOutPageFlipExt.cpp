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
#include "StOutPageFlipStrings.h"

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
    myWinRect.left()   = 0;
    myWinRect.right()  = 0;
    myWinRect.top()    = 0;
    myWinRect.bottom() = 0;

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

void StOutPageFlipExt::close() {
    beforeClose();
    StOutPageFlip::close();
}

void StOutPageFlipExt::beforeClose() {
    if(!StOutPageFlip::params.ToShowExtra->getValue()) {
        return;
    }

    if(isControlOn() && StWindow::isStereoOutput()) {
        myIsQuiting = true;
        const double aTime = getDeviceControl()->quitMS();
        StTimer aQuitTimer(true);
        while(aQuitTimer.getElapsedTimeInMilliSec() <= aTime) {
            stglDraw();
            StThread::sleep(10);
        }
        dxRelease();
        StWindow::setStereoOutput(false);
    }
}

bool StOutPageFlipExt::create() {
    // request slave
    if(params.ControlCode->getValue() != DEVICE_CONTROL_NONE) {
#ifdef _WIN32
    if(myIsVistaPlus) {
#endif
        StWindow::setAttribute(StWinAttr_SlaveCfg, StWinSlave_slaveHLineTop);
        StWindow::hide(ST_WIN_SLAVE);
#ifdef _WIN32
    }
#endif
    }
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

void StOutPageFlipExt::processEvents() {
    StOutPageFlip::processEvents();
    if(!StOutPageFlip::params.ToShowExtra->getValue()) {
        return;
    }

    // resize extra stuff
    const StRectI_t aRect = StWindow::getPlacement();
    if(aRect != myWinRect) {
        myWinRect = aRect;
        myVpSizeY = aRect.height();
        myVpSizeX = aRect.width();
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
                myMonitor = new StMonitor(aMonitors[aRect.center()]);
            } else if(!myMonitor->getVRect().isPointIn(aRect.center())) {
                *myMonitor = aMonitors[aRect.center()];
            }
            myVpSizeX = myMonitor->getVRect().width();
            if(getDeviceControl() != NULL) {
                myVpSizeY = getDeviceControl()->getSizeY();
            }
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
        #if !defined(GL_ES_VERSION_2_0)
            if(!StWindow::isStereoOutput()) {
                myContext->core20fwd->glDrawBuffer(GL_BACK);
            } else {
                myContext->core20fwd->glDrawBuffer(theView == ST_DRAW_LEFT ? GL_BACK_LEFT : GL_BACK_RIGHT);
            }
        #endif
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
    StWinSlave aCfg = StWinSlave_slaveOff;
    if(thePositionId == StGLDeviceControl::SLAVE_HLINE_TOP) {
        aCfg = StWinSlave_slaveHLineTop;
    } else if(thePositionId == StGLDeviceControl::SLAVE_HTOP2PX) {
        aCfg = StWinSlave_slaveHTop2Px;
    } else if(thePositionId == StGLDeviceControl::SLAVE_HLINE_BOTTOM) {
        aCfg = StWinSlave_slaveHLineBottom;
    }
    StWindow::setAttribute(StWinAttr_SlaveCfg, aCfg);
}
