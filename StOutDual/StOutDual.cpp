/**
 * StOutDual, class providing stereoscopic output for Dual Input hardware using StCore toolkit.
 * Copyright © 2007-2016 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#include "StOutDual.h"

#include <StGL/StGLContext.h>
#include <StGL/StGLEnums.h>
#include <StGL/StGLProgram.h>
#include <StGL/StGLFrameBuffer.h>
#include <StGLCore/StGLCore20.h>
#include <StSettings/StSettings.h>
#include <StSettings/StTranslations.h>
#include <StSettings/StEnumParam.h>
#include <StCore/StSearchMonitors.h>
#include <StVersion.h>

namespace {

    static const char ST_OUT_PLUGIN_NAME[]   = "StOutDual";

    static const char ST_SETTING_DEVICE_ID[] = "deviceId";
    static const char ST_SETTING_WINDOWPOS[] = "windowPos";

    // translation resources
    enum {
        STTR_DUAL_NAME   = 1000,
        STTR_DUAL_DESC   = 1001,
        STTR_MIRROR_NAME = 1002,
        STTR_MIRROR_DESC = 1003,

        // parameters
        STTR_PARAMETER_SLAVE_ID  = 1102,
        STTR_PARAMETER_MONOCLONE = 1103,

        // about info
        STTR_PLUGIN_TITLE       = 2000,
        STTR_VERSION_STRING     = 2001,
        STTR_PLUGIN_DESCRIPTION = 2002,
    };

}

/**
 * Just dummy GLSL program.
 */
class StProgramMM : public StGLProgram {

        private:

    StGLVarLocation atrVVertexLoc;
    StGLVarLocation atrVTexCoordLoc;

        public:

    StProgramMM()
    : StGLProgram("StProgramMM"),
      atrVVertexLoc(),
      atrVTexCoordLoc() {
        //
    }

    StGLVarLocation getVVertexLoc() const {
        return atrVVertexLoc;
    }

    StGLVarLocation getVTexCoordLoc() const {
        return atrVTexCoordLoc;
    }

    virtual bool init(StGLContext& theCtx) ST_ATTR_OVERRIDE {
        const char VERTEX_SHADER[] =
           "attribute vec4 vVertex; \
            attribute vec2 vTexCoord; \
            varying vec2 fTexCoord; \
            void main(void) { \
                fTexCoord = vTexCoord; \
                gl_Position = vVertex; \
            }";

        const char FRAGMENT_SHADER[] =
           "uniform sampler2D texR, texL; \
            varying vec2 fTexCoord; \
            void main(void) { \
                gl_FragColor = texture2D(texR, fTexCoord); \
            }";

        StGLVertexShader aVertexShader(StGLProgram::getTitle());
        StGLAutoRelease aTmp1(theCtx, aVertexShader);
        aVertexShader.init(theCtx, VERTEX_SHADER);

        StGLFragmentShader aFragmentShader(StGLProgram::getTitle());
        StGLAutoRelease aTmp2(theCtx, aFragmentShader);
        aFragmentShader.init(theCtx, FRAGMENT_SHADER);
        if(!StGLProgram::create(theCtx)
           .attachShader(theCtx, aVertexShader)
           .attachShader(theCtx, aFragmentShader)
           .link(theCtx)) {
            return false;
        }

        atrVVertexLoc   = StGLProgram::getAttribLocation(theCtx, "vVertex");
        atrVTexCoordLoc = StGLProgram::getAttribLocation(theCtx, "vTexCoord");
        return atrVVertexLoc.isValid() && atrVTexCoordLoc.isValid();
    }

};

void StOutDual::replaceDualAttribute(const DeviceEnum theValue) {
    StWinSlave aCfg = StWinSlave_slaveSync;
    switch(theValue) {
        case DUALMODE_XMIRROW:
            aCfg = StWinSlave_slaveFlipX;
            break;
        case DUALMODE_YMIRROW:
            aCfg = StWinSlave_slaveFlipY;
            break;
        default: break;
    }

    StWindow::setAttribute(StWinAttr_SlaveCfg, aCfg);
    myDevice = theValue;
}

StAtomic<int32_t> StOutDual::myInstancesNb(0);

StString StOutDual::getRendererAbout() const {
    return myAbout;
}

const char* StOutDual::getRendererId() const {
    return ST_OUT_PLUGIN_NAME;
}

const char* StOutDual::getDeviceId() const {
    switch(myDevice) {
        case DUALMODE_XMIRROW:
        case DUALMODE_YMIRROW: return "Mirror";
        case DUALMODE_SIMPLE:
        default:               return "Dual";
    }
}

bool StOutDual::setDevice(const StString& theDevice) {
    if(theDevice == "Dual") {
        replaceDualAttribute(DUALMODE_SIMPLE);
    } else if(theDevice == "Mirror"
           && myDevice == DUALMODE_SIMPLE) {
        replaceDualAttribute(DUALMODE_XMIRROW);
    }
    return false;
}

void StOutDual::getDevices(StOutDevicesList& theList) const {
    for(size_t anIter = 0; anIter < myDevices.size(); ++anIter) {
        theList.add(myDevices[anIter]);
    }
}

void StOutDual::getOptions(StParamsList& theList) const {
    theList.add(params.SlaveMonId);
    theList.add(params.MonoClone);
}

void StOutDual::updateStrings() {
    StTranslations aLangMap(getResourceManager(), ST_OUT_PLUGIN_NAME);

    myDevices[DUALMODE_SIMPLE] ->Name = aLangMap.changeValueId(STTR_DUAL_NAME,   "Dual Output");
    myDevices[DUALMODE_SIMPLE] ->Desc = aLangMap.changeValueId(STTR_DUAL_DESC,   "Stereo-device with dual input: some HMD, Mirrored Stereo monitors, Dual-Projectors");
    myDevices[DUALMODE_XMIRROW]->Name = aLangMap.changeValueId(STTR_MIRROR_NAME, "Mirror Output");
    myDevices[DUALMODE_XMIRROW]->Desc = aLangMap.changeValueId(STTR_MIRROR_DESC, "Hand-make Mirrored Stereo monitors (mirror in X-direction)");

    params.SlaveMonId->setName(aLangMap.changeValueId(STTR_PARAMETER_SLAVE_ID,  "Slave Monitor"));
    params.MonoClone ->setName(aLangMap.changeValueId(STTR_PARAMETER_MONOCLONE, "Show Mono in Stereo"));

    // about string
    StString& aTitle     = aLangMap.changeValueId(STTR_PLUGIN_TITLE,   "sView - Dual Output module");
    StString& aVerString = aLangMap.changeValueId(STTR_VERSION_STRING, "version");
    StString& aDescr     = aLangMap.changeValueId(STTR_PLUGIN_DESCRIPTION,
        "(C) {0} Kirill Gavrilov <{1}>\nOfficial site: {2}\n\nThis library is distributed under LGPL3.0");
    myAbout = aTitle + '\n' + aVerString + " " + StVersionInfo::getSDKVersionString() + "\n \n"
            + aDescr.format("2007-2020", "kirill@sview.ru", "www.sview.ru");
}

StOutDual::StOutDual(const StHandle<StResourceManager>& theResMgr,
                     const StNativeWin_t                theParentWindow)
: StWindow(theResMgr, theParentWindow),
  mySettings(new StSettings(theResMgr, ST_OUT_PLUGIN_NAME)),
  myFrBuffer(new StGLFrameBuffer()),
  myProgram(new StProgramMM()),
  myDevice(DEVICE_AUTO),
  myToCompressMem(myInstancesNb.increment() > 1),
  myIsBroken(false) {
    const StSearchMonitors& aMonitors = StWindow::getMonitors();

    // detect connected displays
    int aSupportLevel = ST_DEVICE_SUPPORT_NONE;
    if(aMonitors.size() >= 2) {
        const StMonitor& aMon0 = aMonitors[0];
        const StMonitor& aMon1 = aMonitors[1];
        if(aMon0.getVRect().width()  == aMon1.getVRect().width()
        && aMon0.getVRect().height() == aMon1.getVRect().height()
        && aMon0.getFreq()           == aMon1.getFreq()) {
            aSupportLevel = ST_DEVICE_SUPPORT_HIGHT;
        }
    }

    // devices list
    StHandle<StOutDevice> aDevDual = new StOutDevice();
    aDevDual->PluginId = ST_OUT_PLUGIN_NAME;
    aDevDual->DeviceId = stCString("Dual");
    aDevDual->Priority = aSupportLevel;
    aDevDual->Name     = stCString("Dual Output");
    myDevices.add(aDevDual);

    StHandle<StOutDevice> aDevMirr = new StOutDevice();
    aDevMirr->PluginId = ST_OUT_PLUGIN_NAME;
    aDevMirr->DeviceId = stCString("Mirror");
    aDevMirr->Priority = aSupportLevel;
    aDevMirr->Name     = stCString("Mirror Output");
    myDevices.add(aDevMirr);

    // Slave Monitor option
    params.SlaveMonId = new StEnumParam(1, stCString("slaveId"), stCString("slaveId"));
    mySettings->loadParam(params.SlaveMonId);
    size_t aMonCount = stMax(aMonitors.size(), size_t(2), size_t(params.SlaveMonId->getValue() + 1));
    for(size_t aMonId = 0; aMonId < aMonCount; ++aMonId) {
        StString aName = StString("Monitor #") + aMonId;
        if(aMonId < aMonitors.size()) {
            aName += " (" + aMonitors[aMonId].getName() + ")";
        } else {
            aName += " (disconnected)";
        }
        params.SlaveMonId->changeValues().add(aName);
    }
    params.SlaveMonId->signals.onChanged.connect(this, &StOutDual::doSlaveMon);

    params.MonoClone = new StBoolParamNamed(false, stCString("monoClone"), stCString("monoClone"));
    updateStrings();

    mySettings->loadParam(params.MonoClone);

    // load window position
    if(isMovable()) {
        StRect<int32_t> aRect;
        if(!mySettings->loadInt32Rect(ST_SETTING_WINDOWPOS, aRect)) {
            aRect = defaultRect();
        }
        StWindow::setPlacement(aRect, true);
    }
    StWindow::setTitle("sView - Dual Renderer");

    // load device settings
    int32_t aDevice = myDevice;
    mySettings->loadInt32(ST_SETTING_DEVICE_ID, aDevice);
    myDevice = (DeviceEnum )aDevice;
    if(myDevice == DEVICE_AUTO) {
        myDevice = DUALMODE_SIMPLE;
    }

    // request slave window
    StWinAttr anAttribs[] = {
        StWinAttr_SlaveMon, (StWinAttr )params.SlaveMonId->getValue(),
        StWinAttr_NULL
    };
    StWindow::setAttributes(anAttribs);
    replaceDualAttribute(myDevice);
}

void StOutDual::releaseResources() {
    if(!myContext.isNull()) {
        myProgram->release(*myContext);
        myVertFlatBuf.release(*myContext);
        myVertXMirBuf.release(*myContext);
        myVertYMirBuf.release(*myContext);
        myTexCoordBuf.release(*myContext);
        myFrBuffer->release(*myContext);
    }
    myContext.nullify();

    // read windowed placement
    StWindow::hide();
    if(isMovable()) {
        StWindow::setFullScreen(false);
    }
}

void StOutDual::beforeClose() {
    if(isMovable() && myWasUsed) {
        mySettings->saveInt32Rect(ST_SETTING_WINDOWPOS, StWindow::getWindowedPlacement());
    }
    mySettings->saveParam(params.SlaveMonId);
    mySettings->saveParam(params.MonoClone);
    mySettings->saveInt32(ST_SETTING_DEVICE_ID, myDevice);
    mySettings->flush();
}

StOutDual::~StOutDual() {
    myInstancesNb.decrement();
    releaseResources();
}

void StOutDual::close() {
    StWindow::params.VSyncMode->signals.onChanged -= stSlot(this, &StOutDual::doSwitchVSync);
    beforeClose();
    releaseResources();
    StWindow::close();
}

bool StOutDual::create() {
    StWindow::show();
    if(!StWindow::create()) {
        return false;
    }

    // initialize GL context
    myContext = StWindow::getContext();
    myContext->setMessagesQueue(myMsgQueue);
    if(!myContext->isGlGreaterEqual(2, 0)) {
        myMsgQueue->pushError(stCString("OpenGL 2.0 is required by Dual Output"));
        myIsBroken = true;
        return true;
    }

    StWindow::stglMakeCurrent(ST_WIN_MASTER);
    myContext->stglSetVSync((StGLContext::VSync_Mode )StWindow::params.VSyncMode->getValue());
    StWindow::params.VSyncMode->signals.onChanged += stSlot(this, &StOutDual::doSwitchVSync);

    if(!myProgram->init(*myContext)) {
        myMsgQueue->pushError(stCString("Dual output - critical error:\nShader initialization failed!"));
        myIsBroken = true;
        return true;
    }
    // create vertices buffers to draw simple textured quad
    const GLfloat QUAD_VERTICES[4 * 4] = {
         1.0f, -1.0f, 0.0f, 1.0f, // top-right
         1.0f,  1.0f, 0.0f, 1.0f, // bottom-right
        -1.0f, -1.0f, 0.0f, 1.0f, // top-left
        -1.0f,  1.0f, 0.0f, 1.0f  // bottom-left
    };
    const GLfloat QUAD_VERTICES_XMIR[4 * 4] = {
        -1.0f, -1.0f, 0.0f, 1.0f, // top-right
        -1.0f,  1.0f, 0.0f, 1.0f, // bottom-right
         1.0f, -1.0f, 0.0f, 1.0f, // top-left
         1.0f,  1.0f, 0.0f, 1.0f  // bottom-left
    };
    const GLfloat QUAD_VERTICES_YMIR[4 * 4] = {
         1.0f,  1.0f, 0.0f, 1.0f, // top-right
         1.0f, -1.0f, 0.0f, 1.0f, // bottom-right
        -1.0f,  1.0f, 0.0f, 1.0f, // top-left
        -1.0f, -1.0f, 0.0f, 1.0f  // bottom-left
    };

    const GLfloat QUAD_TEXCOORD[2 * 4] = {
        1.0f, 0.0f,
        1.0f, 1.0f,
        0.0f, 0.0f,
        0.0f, 1.0f
    };

    myVertFlatBuf.init(*myContext, 4, 4, QUAD_VERTICES);
    myVertXMirBuf.init(*myContext, 4, 4, QUAD_VERTICES_XMIR);
    myVertYMirBuf.init(*myContext, 4, 4, QUAD_VERTICES_YMIR);
    myTexCoordBuf.init(*myContext, 2, 4, QUAD_TEXCOORD);
    myIsBroken = false;

    return true;
}

void StOutDual::processEvents() {
    StWindow::processEvents();

    // don't care about holded key - StWindow::setAttributes do not emit slave update if no change
    const StKeysState& aKeys = StWindow::getKeysState();
    if(aKeys.isKeyDown(ST_VK_F1)) {
        replaceDualAttribute(DUALMODE_SIMPLE);
    } else if(aKeys.isKeyDown(ST_VK_F2)) {
        replaceDualAttribute(DUALMODE_XMIRROW);
    } else if(aKeys.isKeyDown(ST_VK_F3)) {
        replaceDualAttribute(DUALMODE_YMIRROW);
    }
}

void StOutDual::stglDraw() {
    if(!StWindow::stglMakeCurrent(ST_WIN_MASTER)) {
        StWindow::signals.onRedraw(ST_DRAW_MONO);
        StThread::sleep(10);
        return;
    }

    myFPSControl.setTargetFPS(StWindow::getTargetFps());

    const StGLBoxPx aVPMaster = StWindow::stglViewport(ST_WIN_MASTER);
    const StGLBoxPx aVPSlave  = StWindow::stglViewport(ST_WIN_SLAVE);
    const bool toShowStereo = (StWindow::isStereoSource() || params.MonoClone->getValue()) && !myIsBroken;
    myIsForcedStereo = toShowStereo && params.MonoClone->getValue();
    if(!toShowStereo) {
        if(myToCompressMem) {
            myFrBuffer->release(*myContext);
        }

        myContext->stglResizeViewport(aVPMaster);
        myContext->stglSetScissorRect(aVPMaster, false);
        StWindow::signals.onRedraw(ST_DRAW_LEFT);
        myContext->stglResetScissorRect();

        // TODO (Kirill Gavrilov#4#) we could do this once
        StWindow::stglMakeCurrent(ST_WIN_SLAVE);
        myContext->stglResizeViewport(aVPSlave);
        myContext->stglSetScissorRect(aVPSlave, false);
        myContext->core20fwd->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        myContext->stglResetScissorRect();

        myFPSControl.sleepToTarget(); // decrease FPS to target by thread sleeps
        StWindow::stglSwap(ST_WIN_ALL);
        ++myFPSControl;
        return;
    }

    if(myDevice == DUALMODE_SIMPLE) {
        myContext->stglResizeViewport(aVPMaster);
        myContext->stglSetScissorRect(aVPMaster, false);
        StWindow::signals.onRedraw(ST_DRAW_LEFT);
        myContext->stglResetScissorRect();

        StWindow::stglMakeCurrent(ST_WIN_SLAVE);
        myContext->stglResizeViewport(aVPSlave);
        myContext->stglSetScissorRect(aVPSlave, false);
        StWindow::signals.onRedraw(ST_DRAW_RIGHT);
        myContext->stglResetScissorRect();
    } else {
        // resize FBO
        if(!myFrBuffer->initLazy(*myContext, GL_RGBA8, aVPMaster.width(), aVPMaster.height(), StWindow::hasDepthBuffer())) {
            myMsgQueue->pushError(stCString("Dual output - critical error:\nFrame Buffer Object resize failed!"));
            myIsBroken = true;
            return;
        }

        // reduce viewport to avoid additional aliasing of narrow lines
        GLfloat aDX = GLfloat(myFrBuffer->getVPSizeX()) / GLfloat(myFrBuffer->getSizeX());
        GLfloat aDY = GLfloat(myFrBuffer->getVPSizeY()) / GLfloat(myFrBuffer->getSizeY());
        StArray<StGLVec2> aTCoords(4);
        aTCoords[0] = StGLVec2(aDX,  0.0f);
        aTCoords[1] = StGLVec2(aDX,  aDY);
        aTCoords[2] = StGLVec2(0.0f, 0.0f);
        aTCoords[3] = StGLVec2(0.0f, aDY);
        myTexCoordBuf.init(*myContext, aTCoords);

        // draw Left View into virtual frame buffer
        myFrBuffer->setupViewPort(*myContext); // we set TEXTURE sizes here
        myFrBuffer->bindBuffer(*myContext);
            StWindow::signals.onRedraw(ST_DRAW_LEFT);
        myFrBuffer->unbindBuffer(*myContext);

        // now draw to real screen buffer
        // clear the screen and the depth buffer
        myContext->stglResizeViewport(aVPMaster);
        myContext->stglSetScissorRect(aVPMaster, false);
        myContext->core20fwd->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        myFrBuffer->bindTexture(*myContext);
        myProgram->use(*myContext);
            myVertFlatBuf.bindVertexAttrib(*myContext, myProgram->getVVertexLoc());
            myTexCoordBuf.bindVertexAttrib(*myContext, myProgram->getVTexCoordLoc());

            myContext->core20fwd->glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

            myTexCoordBuf.unBindVertexAttrib(*myContext, myProgram->getVTexCoordLoc());
            myVertFlatBuf.unBindVertexAttrib(*myContext, myProgram->getVVertexLoc());
        myProgram->unuse(*myContext);
        myFrBuffer->unbindTexture(*myContext);
        myContext->stglResetScissorRect();

        // draw Right View into virtual frame buffer
        myFrBuffer->setupViewPort(*myContext); // we set TEXTURE sizes here
        myFrBuffer->bindBuffer(*myContext);
            StWindow::signals.onRedraw(ST_DRAW_RIGHT);
        myFrBuffer->unbindBuffer(*myContext);

        // clear the screen and the depth buffer
        StWindow::stglMakeCurrent(ST_WIN_SLAVE);
        myContext->stglResizeViewport(aVPSlave);
        myContext->stglSetScissorRect(aVPSlave, false);
        myContext->core20fwd->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        myFrBuffer->bindTexture(*myContext);
        myProgram->use(*myContext);
        if(myDevice == DUALMODE_XMIRROW) {
            myVertXMirBuf.bindVertexAttrib(*myContext, myProgram->getVVertexLoc());
        } else if(myDevice == DUALMODE_YMIRROW) {
            myVertYMirBuf.bindVertexAttrib(*myContext, myProgram->getVVertexLoc());
        } else {
            myVertFlatBuf.bindVertexAttrib(*myContext, myProgram->getVVertexLoc());
        }
        myTexCoordBuf.bindVertexAttrib(*myContext, myProgram->getVTexCoordLoc());

        myContext->core20fwd->glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        myTexCoordBuf.unBindVertexAttrib(*myContext, myProgram->getVTexCoordLoc());
        if(myDevice == DUALMODE_XMIRROW) {
            myVertXMirBuf.unBindVertexAttrib(*myContext, myProgram->getVVertexLoc());
        } else if(myDevice == DUALMODE_YMIRROW) {
            myVertYMirBuf.unBindVertexAttrib(*myContext, myProgram->getVVertexLoc());
        } else {
            myVertFlatBuf.unBindVertexAttrib(*myContext, myProgram->getVVertexLoc());
        }
        myProgram->unuse(*myContext);
        myFrBuffer->unbindTexture(*myContext);
        myContext->stglResetScissorRect();
    }
    myFPSControl.sleepToTarget(); // decrease FPS to target by thread sleeps
    StWindow::stglSwap(ST_WIN_ALL);
    ++myFPSControl;
    // make sure all GL changes in callback (in StDrawer) will fine
    StWindow::stglMakeCurrent(ST_WIN_MASTER);
}

void StOutDual::doSwitchVSync(const int32_t theValue) {
    if(myContext.isNull()) {
        return;
    }

    StWindow::stglMakeCurrent(ST_WIN_MASTER);
    myContext->stglSetVSync((StGLContext::VSync_Mode )theValue);
}

void StOutDual::doSlaveMon(const int32_t theValue) {
    StWindow::setAttribute(StWinAttr_SlaveMon, theValue);
}
