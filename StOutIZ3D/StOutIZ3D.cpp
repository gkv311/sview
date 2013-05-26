/**
 * Copyright © 2009-2013 Kirill Gavrilov <kirill@sview.ru>
 *
 * StOutIZ3D library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * StOutIZ3D library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.
 * If not, see <http://www.gnu.org/licenses/>.
 */

#include "StOutIZ3D.h"

#include <StGL/StGLEnums.h>
#include <StGL/StGLContext.h>
#include <StGLCore/StGLCore20.h>

#include <StImage/StImageFile.h>

#include <StSettings/StSettings.h>
#include <StSettings/StTranslations.h>
#include <StSettings/StEnumParam.h>
#include <StCore/StSearchMonitors.h>
#include <StVersion.h>

namespace {

    static const char ST_OUT_PLUGIN_NAME[] = "StOutIZ3D";

    // settings
    static const char ST_SETTING_WINDOWPOS[]      = "windowPos";
    static const char ST_SETTING_FRBUFF_X[]       = "frbufferX";
    static const char ST_SETTING_FRBUFF_Y[]       = "frbufferY";
    static const char ST_SETTING_USER_FRMB_SIZE[] = "useUserFrmbSize";
    static const char ST_SETTING_TABLE[]          = "tableId";

    // iZ3D monitor's models for autodetect
    // old model have not strong back/front separation
    static const StString IZ3D_MODEL_OLD0           = "CMO3228";
    static const StString IZ3D_MODEL_OLD1           = "CMO3229";

    // Matrox TripleHead2Go Digital Edition
    static const StString IZ3D_MODEL_MATROXTH2GO0   = "MTX0510";
    static const StString IZ3D_MODEL_MATROXTH2GO1   = "MTX0511";

    // new models
    static const StString IZ3D_MODEL_FRONT_NEW      = "CMO3239";
    static const StString IZ3D_MODEL_BACK_NEW0      = "CMO3238";
    static const StString IZ3D_MODEL_BACK_NEW1      = "CMO3237";
    static const StString IZ3D_MODEL_FRONT_SAPPHIRE = "CMO4932";
    static const StString IZ3D_MODEL_BACK_SAPPHIRE0 = "CMO4832";
    static const StString IZ3D_MODEL_BACK_SAPPHIRE1 = "CMO4732";

    static bool isFrontDisplay(const StString& model) {
        return (model == IZ3D_MODEL_OLD0      || model == IZ3D_MODEL_OLD1
             || model == IZ3D_MODEL_FRONT_NEW || model == IZ3D_MODEL_FRONT_SAPPHIRE);
    }

    static bool isBackDisplay(const StString& model) {
        return (model == IZ3D_MODEL_OLD0           || model == IZ3D_MODEL_OLD1
             || model == IZ3D_MODEL_BACK_NEW0      || model == IZ3D_MODEL_BACK_NEW1
             || model == IZ3D_MODEL_BACK_SAPPHIRE0 || model == IZ3D_MODEL_BACK_SAPPHIRE0);
    }

    // translation resources
    enum {
        STTR_IZ3D_NAME = 1000,
        STTR_IZ3D_DESC = 1001,

        // parameters
        STTR_PARAMETER_GLASSES  = 1102,

        STTR_PARAMETER_GLASSES_CLASSIC      = 1120,
        STTR_PARAMETER_GLASSES_MODERN       = 1121,
        STTR_PARAMETER_GLASSES_CLASSIC_FAST = 1122,

        // about info
        STTR_PLUGIN_TITLE       = 2000,
        STTR_VERSION_STRING     = 2001,
        STTR_PLUGIN_DESCRIPTION = 2002,
    };

};

StAtomic<int32_t> StOutIZ3D::myInstancesNb(0);

StString StOutIZ3D::getRendererAbout() const {
    return myAbout;
}

const char* StOutIZ3D::getRendererId() const {
    return ST_OUT_PLUGIN_NAME;
}

const char* StOutIZ3D::getDeviceId() const {
    return "iZ3D";
}

void StOutIZ3D::getDevices(StOutDevicesList& theList) const {
    for(size_t anIter = 0; anIter < myDevices.size(); ++anIter) {
        theList.add(myDevices[anIter]);
    }
}

void StOutIZ3D::getOptions(StParamsList& theList) const {
    theList.add(params.Glasses);
}

StOutIZ3D::StOutIZ3D(const StNativeWin_t theParentWindow)
: StWindow(theParentWindow),
  mySettings(new StSettings(ST_OUT_PLUGIN_NAME)),
  myFrBuffer(new StGLStereoFrameBuffer()),
  myToSavePlacement(theParentWindow == (StNativeWin_t )NULL),
  myToCompressMem(myInstancesNb.increment() > 1),
  myIsBroken(false) {
    const StSearchMonitors& aMonitors = StWindow::getMonitors();
    StTranslations aLangMap(ST_OUT_PLUGIN_NAME);

    // about string
    StString& aTitle     = aLangMap.changeValueId(STTR_PLUGIN_TITLE,   "sView - iZ3D Output module");
    StString& aVerString = aLangMap.changeValueId(STTR_VERSION_STRING, "version");
    StString& aDescr     = aLangMap.changeValueId(STTR_PLUGIN_DESCRIPTION,
        "(C) 2009-2013 Kirill Gavrilov <kirill@sview.ru>\nOfficial site: www.sview.ru\n\nThis library distributed under LGPL3.0");
    myAbout = aTitle + '\n' + aVerString + ": " + StVersionInfo::getSDKVersionString() + "\n \n" + aDescr;

    // detect connected displays
    int aSupportLevel = ST_DEVICE_SUPPORT_NONE;
    for(size_t aMonIter = 0; aMonIter < aMonitors.size(); ++aMonIter) {
        const StMonitor& aMon = aMonitors[aMonIter];
        if(isFrontDisplay(aMon.getPnPId())
        || isBackDisplay (aMon.getPnPId())) {
            aSupportLevel = ST_DEVICE_SUPPORT_PREFER; // we sure that iZ3D connected
            break;
        }/* else if(aMon.getPnPId() == IZ3D_MODEL_MATROXTH2GO0
               || aMon.getPnPId() == IZ3D_MODEL_MATROXTH2GO1) {
            aSupportLevel = ST_DEVICE_SUPPORT_FULL; // is it possible
        }*/
    }

    // devices list
    StHandle<StOutDevice> aDevice = new StOutDevice();
    aDevice->PluginId = ST_OUT_PLUGIN_NAME;
    aDevice->DeviceId = "iZ3D";
    aDevice->Priority = aSupportLevel;
    aDevice->Name     = aLangMap.changeValueId(STTR_IZ3D_NAME, "IZ3D Display");
    aDevice->Desc     = aLangMap.changeValueId(STTR_IZ3D_DESC, "IZ3D Display");
    myDevices.add(aDevice);

    // shader switch option
    StHandle<StEnumParam> aGlasses = new StEnumParam(myShaders.getMode(),
                                                     aLangMap.changeValueId(STTR_PARAMETER_GLASSES, "iZ3D glasses"));
    aGlasses->changeValues().add(aLangMap.changeValueId(STTR_PARAMETER_GLASSES_CLASSIC,      "Classic"));
    aGlasses->changeValues().add(aLangMap.changeValueId(STTR_PARAMETER_GLASSES_MODERN,       "Modern"));
    aGlasses->changeValues().add(aLangMap.changeValueId(STTR_PARAMETER_GLASSES_CLASSIC_FAST, "Classic (fast)"));
    aGlasses->signals.onChanged.connect(&myShaders, &StOutIZ3DShaders::doSetMode);
    params.Glasses = aGlasses;

    // load window position
    StRect<int32_t> aRect(256, 768, 256, 1024);
    mySettings->loadInt32Rect(ST_SETTING_WINDOWPOS, aRect);
    StWindow::setPlacement(aRect, true);
    StWindow::setTitle("sView - iZ3D Renderer");

    // load parameters
    mySettings->loadParam(ST_SETTING_TABLE, params.Glasses);

    // request slave window
    StWindow::setAttribute(StWinAttr_SlaveCfg, StWinSlave_slaveSync);
}

void StOutIZ3D::releaseResources() {
    if(!myContext.isNull()) {
        myShaders.release(*myContext);
        myTexTableOld.release(*myContext);
        myTexTableNew.release(*myContext);
        myFrBuffer->release(*myContext);
    }
    myContext.nullify();

    // read windowed placement
    StWindow::hide();
    if(myToSavePlacement) {
        StWindow::setFullScreen(false);
        mySettings->saveInt32Rect(ST_SETTING_WINDOWPOS, StWindow::getPlacement());
    }
    mySettings->saveParam(ST_SETTING_TABLE, params.Glasses);
}

StOutIZ3D::~StOutIZ3D() {
    myInstancesNb.decrement();
    releaseResources();
}

void StOutIZ3D::close() {
    StWindow::params.VSyncMode->signals.onChanged -= stSlot(this, &StOutIZ3D::doSwitchVSync);
    releaseResources();
    StWindow::close();
}

bool StOutIZ3D::create() {
    StWindow::show();
    if(!StWindow::create()) {
        return false;
    }

    // initialize GL context
    myContext = new StGLContext();
    myContext->setMessagesQueue(myMsgQueue);
    if(!myContext->stglInit()) {
        myMsgQueue->pushError(stCString("iZ3D output - critical error:\nOpenGL context is broken!\n(OpenGL library internal error?)"));
        return false;
    } else if(!myContext->isGlGreaterEqual(2, 0)) {
        myMsgQueue->pushError(stCString("OpenGL 2.0 is required by iZ3D Output"));
        myIsBroken = true;
        return true;
    }

    StWindow::stglMakeCurrent(ST_WIN_MASTER);
    myContext->stglSetVSync((StGLContext::VSync_Mode )StWindow::params.VSyncMode->getValue());
    StWindow::params.VSyncMode->signals.onChanged += stSlot(this, &StOutIZ3D::doSwitchVSync);

    // INIT iZ3D tables textures
    const StString aTexturesFolder = StProcess::getStCoreFolder() + "textures" + SYS_FS_SPLITTER;
    const StString aTableOldPath   = aTexturesFolder + "iz3dTableOld.std";
    const StString aTableNewPath   = aTexturesFolder + "iz3dTableNew.std";

    StHandle<StImageFile> aTableImg = StImageFile::create();
    if(aTableImg.isNull()) {
        myMsgQueue->pushError(stCString("iZ3D output - internal error!"));
        myIsBroken = true;
        return true;
    }
    if(!aTableImg->load(aTableOldPath, StImageFile::ST_TYPE_PNG)) {
        myMsgQueue->pushError(StString("iZ3D output - critical error:\n") + aTableImg->getState());
        myIsBroken = true;
        return true;
    }
    myTexTableOld.setMinMagFilter(*myContext, GL_NEAREST); // we need not linear filtrating for lookup-table!
    if(!myTexTableOld.init(*myContext, aTableImg->getPlane())) {
        myMsgQueue->pushError(stCString("iZ3D output - critical error:\nLookup-table initalization failed!"));
        myIsBroken = true;
        return true;
    }
    if(!aTableImg->load(aTableNewPath, StImageFile::ST_TYPE_PNG)) {
        myMsgQueue->pushError(StString("iZ3D output - critical error:\n") + aTableImg->getState());
        myIsBroken = true;
        return true;
    }
    myTexTableNew.setMinMagFilter(*myContext, GL_NEAREST); // we need not linear filtrating for lookup-table!
    if(!myTexTableNew.init(*myContext, aTableImg->getPlane())) {
        myMsgQueue->pushError(stCString("iZ3D output - critical error:\nLookup-table initalization failed!"));
        myIsBroken = true;
        return true;
    }
    aTableImg.nullify();

    // INIT shaders
    if(!myShaders.init(*myContext)) {
        myMsgQueue->pushError(stCString("iZ3D output - critical error:\nShaders initialization failed!"));
        myIsBroken = true;
        return true;
    }

    myIsBroken = false;
    return true;
}

void StOutIZ3D::processEvents() {
    StWindow::processEvents();

    // don't care about holded key - StParam calls it's callbacks only on value change
    const StKeysState& aKeys = StWindow::getKeysState();
    if(aKeys.isKeyDown(ST_VK_F1)) {
        params.Glasses->setValue(StOutIZ3DShaders::IZ3D_TABLE_OLD);
    } else if(aKeys.isKeyDown(ST_VK_F2)) {
        params.Glasses->setValue(StOutIZ3DShaders::IZ3D_TABLE_NEW);
    } else if(aKeys.isKeyDown(ST_VK_F3)) {
        params.Glasses->setValue(StOutIZ3DShaders::IZ3D_CLASSIC);
    }
}

void StOutIZ3D::stglDraw() {
    myFPSControl.setTargetFPS(StWindow::getTargetFps());

    const StGLBoxPx aVPMaster = StWindow::stglViewport(ST_WIN_MASTER);
    const StGLBoxPx aVPSlave  = StWindow::stglViewport(ST_WIN_SLAVE);
    if(!StWindow::isStereoOutput() || myIsBroken) {
        StWindow::stglMakeCurrent(ST_WIN_MASTER);
        if(myToCompressMem) {
            myFrBuffer->release(*myContext);
        }

        myContext->stglResizeViewport(aVPMaster);
        myContext->stglSetScissorRect(aVPMaster, false);
        StWindow::signals.onRedraw(ST_DRAW_LEFT);
        myContext->stglResetScissorRect();

        StWindow::stglMakeCurrent(ST_WIN_SLAVE);
        myContext->stglResizeViewport(aVPSlave);
        myContext->stglSetScissorRect(aVPSlave, false);
        myContext->core20fwd->glClearColor(0.729740052840723f, 0.729740052840723f, 0.729740052840723f, 0.0f);
        // clear the screen and the depth buffer
        myContext->core20fwd->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        myContext->core20fwd->glClearColor(0, 0, 0, 0);
        myContext->stglResetScissorRect();

        myFPSControl.sleepToTarget(); // decrease FPS to target by thread sleeps
        StWindow::stglSwap(ST_WIN_ALL);
        ++myFPSControl;
        return;
    }
    StWindow::stglMakeCurrent(ST_WIN_MASTER);

    // resize FBO
    if(!myFrBuffer->initLazy(*myContext, aVPMaster.width(), aVPMaster.height(), StWindow::hasDepthBuffer())) {
        myMsgQueue->pushError(stCString("iZ3D output - critical error:\nFrame Buffer Object resize failed!"));
        myIsBroken = true;
        return;
    }

    // draw into virtual frame buffers (textures)
    myFrBuffer->setupViewPort(*myContext);    // we set TEXTURE sizes here
    myFrBuffer->bindBufferLeft(*myContext);
        StWindow::signals.onRedraw(ST_DRAW_LEFT);
    myFrBuffer->bindBufferRight(*myContext);
        StWindow::signals.onRedraw(ST_DRAW_RIGHT);
    myFrBuffer->unbindBufferRight(*myContext);

    // now draw to real screen buffer
    // clear the screen and the depth buffer
    myContext->stglResizeViewport(aVPMaster);
    myContext->stglSetScissorRect(aVPMaster, false);
    myContext->core20fwd->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    myContext->core20fwd->glDisable(GL_DEPTH_TEST);
    myContext->core20fwd->glDisable(GL_BLEND);

    StGLTexture& stTexTable = (myShaders.getMode() == StOutIZ3DShaders::IZ3D_TABLE_NEW) ? myTexTableNew : myTexTableOld;

    myShaders.master()->use(*myContext);
    myFrBuffer->bindMultiTexture(*myContext);
    stTexTable.bind(*myContext, GL_TEXTURE2);

    myFrBuffer->drawQuad(*myContext, myShaders.master());

    stTexTable.unbind(*myContext);
    myFrBuffer->unbindMultiTexture(*myContext);
    myShaders.master()->unuse(*myContext);
    myContext->stglResetScissorRect();

    StWindow::stglMakeCurrent(ST_WIN_SLAVE);
    myContext->stglResizeViewport(aVPSlave);
    myContext->stglSetScissorRect(aVPSlave, false);
    myContext->core20fwd->glClearColor(0.729740052840723f, 0.729740052840723f, 0.729740052840723f, 0.0f);
    // clear the screen and the depth buffer
    myContext->core20fwd->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    myContext->core20fwd->glClearColor(0, 0, 0, 0);

    myShaders.slave()->use(*myContext);
    myFrBuffer->bindMultiTexture(*myContext);
    stTexTable.bind(*myContext, GL_TEXTURE2);

    myFrBuffer->drawQuad(*myContext, myShaders.slave());

    stTexTable.unbind(*myContext);
    myFrBuffer->unbindMultiTexture(*myContext);
    myShaders.slave()->unuse(*myContext);
    myContext->stglResetScissorRect();

    myFPSControl.sleepToTarget(); // decrease FPS to target by thread sleeps
    StWindow::stglSwap(ST_WIN_ALL);
    ++myFPSControl;
    // make sure all GL changes in callback (in StDrawer) will fine
    StWindow::stglMakeCurrent(ST_WIN_MASTER);
}

void StOutIZ3D::doSwitchVSync(const int32_t theValue) {
    if(myContext.isNull()) {
        return;
    }

    StWindow::stglMakeCurrent(ST_WIN_MASTER);
    myContext->stglSetVSync((StGLContext::VSync_Mode )theValue);
}
