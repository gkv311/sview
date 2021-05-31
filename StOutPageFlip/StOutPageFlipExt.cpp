/**
 * StOutPageFlip, class providing stereoscopic output for Shutter Glasses displays using StCore toolkit.
 * Copyright © 2007-2014 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
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
    static const StString ST_SETTING_ADVANCED    = "advanced";
    static const StString ST_OUT_PLUGIN_NAME_EXT = "StOutPageFlip";
}

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

void StOutPageFlipExt::updateStringsExt() {
    params.ControlCode->setName(myLangMap.changeValueId(STTR_PARAMETER_CONTROL_CODE, "Glasses control codes"));
    params.ControlCode->defineOption(DEVICE_CONTROL_NONE,      myLangMap.changeValueId(STTR_PARAMETER_CONTROL_NO,        "No codes"));
    params.ControlCode->defineOption(DEVICE_CONTROL_BLUELINE,  myLangMap.changeValueId(STTR_PARAMETER_CONTROL_BLUELINE,  "Blue line sync"));
    params.ControlCode->defineOption(DEVICE_CONTROL_WHITELINE, myLangMap.changeValueId(STTR_PARAMETER_CONTROL_WHITELINE, "White line sync"));
    params.ControlCode->defineOption(DEVICE_CONTROL_ED_ON_OFF, myLangMap.changeValueId(STTR_PARAMETER_CONTROL_ED,        "eDimensional auto on/off"));
}

StOutPageFlipExt::StOutPageFlipExt(const StHandle<StResourceManager>& theResMgr,
                                   const StNativeWin_t                theParentWindow)
: StOutPageFlip(theResMgr, theParentWindow),
  myVpSizeY(0),
  myVpSizeX(0),
  myIsQuiting(false) {
    myWinRect.left()   = 0;
    myWinRect.right()  = 0;
    myWinRect.top()    = 0;
    myWinRect.bottom() = 0;

    // Control Code option
    params.ControlCode = new StEnumParam(DEVICE_CONTROL_NONE, stCString("deviceControl"), stCString("deviceControl"));
    params.ControlCode->defineOption(DEVICE_CONTROL_NONE,      stCString("noCodes"));
    params.ControlCode->defineOption(DEVICE_CONTROL_BLUELINE,  stCString("blueLine"));
    params.ControlCode->defineOption(DEVICE_CONTROL_WHITELINE, stCString("whiteLine"));
    params.ControlCode->defineOption(DEVICE_CONTROL_ED_ON_OFF, stCString("eD"));
    params.ControlCode->signals.onChanged.connect(this, &StOutPageFlipExt::doSetDeviceControl);

    // load shutter glasses controller
    updateStringsExt();
    mySettings->loadParam(params.ControlCode);
    myToResetDevice = false;
}

void StOutPageFlipExt::releaseResources() {
    if(!myContext.isNull()) {
        myCodesLine.release(*myContext);
        myCodesEDOnOff.release(*myContext);
    }
    StOutPageFlip::releaseResources();
}

StOutPageFlipExt::~StOutPageFlipExt() {
    releaseResources();
}

void StOutPageFlipExt::beforeClose() {
    StOutPageFlip::beforeClose();
    mySettings->saveParam(params.ControlCode);
    mySettings->flush();
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
    myIsQuiting = false;

    // request slave
    if(params.ControlCode->getValue() != DEVICE_CONTROL_NONE) {
        StWindow::setAttribute(StWinAttr_SlaveCfg, StWinSlave_slaveHLineTop);
        StWindow::hide(ST_WIN_SLAVE);
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

void StOutPageFlipExt::setFullScreen(const bool theFullScreen) {
    if(StOutPageFlip::params.QuadBuffer->getValue() == QUADBUFFER_SOFT) {
        setAttribute(StWinAttr_ExclusiveFullScreen, true);
    }
    StOutPageFlip::setFullScreen(theFullScreen);
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

        if(!StWindow::isFullScreen()) {
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

    const bool toDrawWindowed = !StWindow::isFullScreen();
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
