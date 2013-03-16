/**
 * Copyright © 2007-2012 Kirill Gavrilov <kirill@sview.ru>
 *
 * StOutDual library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * StOutDual library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.
 * If not, see <http://www.gnu.org/licenses/>.
 */

#include "StOutDual.h"

#include <StGL/StGLContext.h>
#include <StGL/StGLEnums.h>
#include <StGL/StGLProgram.h>
#include <StGL/StGLFrameBuffer.h>
#include <StGLCore/StGLCore20.h>
#include <StCore/StWindow.h>
#include <StSettings/StSettings.h>
#include <StSettings/StTranslations.h>

namespace {
    static bool HAS_LOGGER_ID = StLogger::IdentifyModule("StOutDual");

    static const char ST_OUT_PLUGIN_NAME[]   = "StOutDual";

    static const char ST_SETTING_DEVICE_ID[] = "deviceId";
    static const char ST_SETTING_WINDOWPOS[] = "windowPos";
    static const char ST_SETTING_SLAVE_ID[]  = "slaveId";
    static const char ST_SETTING_VSYNC[]     = "vsync";

    // translation resources
    enum {
        STTR_DUAL_NAME   = 1000,
        STTR_DUAL_DESC   = 1001,
        STTR_MIRROR_NAME = 1002,
        STTR_MIRROR_DESC = 1003,

        // parameters
        STTR_PARAMETER_VSYNC    = 1100,
        STTR_PARAMETER_SHOW_FPS = 1101,
        STTR_PARAMETER_SLAVE_ID = 1102,

        // about info
        STTR_PLUGIN_TITLE       = 2000,
        STTR_VERSION_STRING     = 2001,
        STTR_PLUGIN_DESCRIPTION = 2002,
    };

};

/**
 * Just dummy GLSL program.
 */
class ST_LOCAL StProgramMM : public StGLProgram {

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

    virtual bool init(StGLContext& theCtx) {
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

void StOutDual::replaceDualAttribute(const DeviceEnum theFrom,
                                     const DeviceEnum theTo) {
    StWinAttributes_t anAttribs = stDefaultWinAttributes();
    getStWindow()->getAttributes(&anAttribs);
    StWinAttributes_t anAttribsBefore = anAttribs;
    switch(theFrom) {
        case DUALMODE_XMIRROW:
            anAttribs.isSlaveXMirrow = false;
            break;
        case DUALMODE_YMIRROW:
            anAttribs.isSlaveYMirrow = false;
        default: break;
    }
    switch(theTo) {
        case DUALMODE_XMIRROW:
            anAttribs.isSlaveXMirrow = true;
            break;
        case DUALMODE_YMIRROW:
            anAttribs.isSlaveYMirrow = true;
            break;
        default: break;
    }
    if(!areSame(&anAttribsBefore, &anAttribs)) {
        getStWindow()->setAttributes(&anAttribs);
    }
    myDevice = theTo;
    if(myOptions != NULL) {
        myOptions->curDeviceId = myDevice;
    }
}

void StOutDual::optionsStructAlloc() {
    StTranslations stLangMap(ST_OUT_PLUGIN_NAME);

    // create device options structure
    myOptions = (StSDOptionsList_t* )StWindow::memAlloc(sizeof(StSDOptionsList_t)); stMemSet(myOptions, 0, sizeof(StSDOptionsList_t));
    myOptions->curRendererPath = StWindow::memAllocNCopy(myPluginPath);
    myOptions->curDeviceId = myDevice;

    myOptions->optionsCount = 3;
    myOptions->options = (StSDOption_t** )StWindow::memAlloc(sizeof(StSDOption_t*) * myOptions->optionsCount);

    // VSync option
    myOptions->options[DEVICE_OPTION_VSYNC] = (StSDOption_t* )StWindow::memAlloc(sizeof(StSDOnOff_t));
    myOptions->options[DEVICE_OPTION_VSYNC]->optionType = ST_DEVICE_OPTION_ON_OFF;
    ((StSDOnOff_t* )myOptions->options[DEVICE_OPTION_VSYNC])->value = myIsVSyncOn;
    myOptions->options[DEVICE_OPTION_VSYNC]->title = StWindow::memAllocNCopy(stLangMap.changeValueId(STTR_PARAMETER_VSYNC, "VSync"));

    // Show FPS option
    myOptions->options[DEVICE_OPTION_SHOWFPS] = (StSDOption_t* )StWindow::memAlloc(sizeof(StSDOnOff_t));
    myOptions->options[DEVICE_OPTION_SHOWFPS]->optionType = ST_DEVICE_OPTION_ON_OFF;
    ((StSDOnOff_t* )myOptions->options[DEVICE_OPTION_SHOWFPS])->value = myToShowFPS;
    myOptions->options[DEVICE_OPTION_SHOWFPS]->title = StWindow::memAllocNCopy(stLangMap.changeValueId(STTR_PARAMETER_SHOW_FPS, "Show FPS"));

    // Slave Monitor option
    StArrayList<StMonitor> aMonitors = StCore::getStMonitors();
    myOptions->options[DEVICE_OPTION_SLAVEID] = (StSDOption_t* )StWindow::memAlloc(sizeof(StSDSwitch_t));
    myOptions->options[DEVICE_OPTION_SLAVEID]->title = StWindow::memAllocNCopy(stLangMap.changeValueId(STTR_PARAMETER_SLAVE_ID, "Slave Monitor"));
    myOptions->options[DEVICE_OPTION_SLAVEID]->optionType = ST_DEVICE_OPTION_SWITCH;
    StSDSwitch_t* aSwitchSlaveID = ((StSDSwitch_t* )myOptions->options[DEVICE_OPTION_SLAVEID]);
    aSwitchSlaveID->value        = mySlaveMonId;
    aSwitchSlaveID->valuesCount  = stMax(aMonitors.size(), size_t(2), size_t(mySlaveMonId + 1));
    aSwitchSlaveID->valuesTitles = (stUtf8_t** )StWindow::memAlloc(aSwitchSlaveID->valuesCount * sizeof(stUtf8_t*));
    for(size_t aMonId = 0; aMonId < aSwitchSlaveID->valuesCount; ++aMonId) {
        StString aName = StString("Monitor #") + aMonId;
        if(aMonId < aMonitors.size()) {
            aName += " (" + aMonitors[aMonId].getName() + ")";
        } else {
            aName += " (disconnected)";
        }
        aSwitchSlaveID->valuesTitles[aMonId] = StWindow::memAllocNCopy(aName);
    }
}

StAtomic<int32_t> StOutDual::myInstancesNb(0);

StOutDual::StOutDual()
: myProgram(new StProgramMM()),
  myOptions(NULL),
  myDevice(DEVICE_AUTO),
  mySlaveMonId(1),
  myToSavePlacement(true),
  myIsVSyncOn(true),
  myToShowFPS(false),
  myToCompressMem(myInstancesNb.increment() > 1),
  myIsBroken(false) {
    myFrBuffer = new StGLFrameBuffer();
}

StOutDual::~StOutDual() {
    myInstancesNb.decrement();
    if(!myStCore.isNull() && !mySettings.isNull()) {
        if(!myContext.isNull()) {
            myProgram->release(*myContext);
            myVertFlatBuf.release(*myContext);
            myVertXMirBuf.release(*myContext);
            myVertYMirBuf.release(*myContext);
            myTexCoordBuf.release(*myContext);
            myFrBuffer->release(*myContext);
        }
        stMemFree(myOptions, StWindow::memFree);

        // read windowed placement
        getStWindow()->hide(ST_WIN_MASTER);
        getStWindow()->hide(ST_WIN_SLAVE);
        if(myToSavePlacement) {
            getStWindow()->setFullScreen(false);
            mySettings->saveInt32Rect(ST_SETTING_WINDOWPOS, getStWindow()->getPlacement());
        }
        mySettings->saveInt32(ST_SETTING_SLAVE_ID,       mySlaveMonId);
        mySettings->saveBool (ST_SETTING_VSYNC,          myIsVSyncOn);
        mySettings->saveInt32(ST_SETTING_DEVICE_ID,      myDevice);
    }
    mySettings.nullify();
    myStCore.nullify();
    StCore::FREE();
}

bool StOutDual::init(const StString&     theRendererPath,
                     const int&          theDeviceId,
                     const StNativeWin_t theNativeParent) {
    myToSavePlacement = (theNativeParent == (StNativeWin_t )NULL);
    myDevice = (DeviceEnum )theDeviceId;
    myPluginPath = theRendererPath;
    if(!StVersionInfo::checkTimeBomb("sView - Dual Output plugin")) {
        return false;
    }
    ST_DEBUG_LOG_AT("INIT Dual output plugin");
    // Firstly INIT core library!
    if(StCore::INIT() != STERROR_LIBNOERROR) {
        stError(StString(ST_OUT_PLUGIN_NAME) + " Plugin, Core library not available!");
        return false;
    }

    // INIT settings library
    mySettings = new StSettings(ST_OUT_PLUGIN_NAME);
    myStCore   = new StCore();

    // load window position
    StRect<int32_t> loadedRect(256, 768, 256, 1024);
    mySettings->loadInt32Rect(ST_SETTING_WINDOWPOS, loadedRect);
    StMonitor stMonitor = StCore::getMonitorFromPoint(loadedRect.center());
    if(!stMonitor.getVRect().isPointIn(loadedRect.center())) {
        ST_DEBUG_LOG("Warning, stored window position is out of the monitor(" + stMonitor.getId() + ")!" + loadedRect.toString());
        int w = loadedRect.width();
        int h = loadedRect.height();
        loadedRect.left() = stMonitor.getVRect().left() + 256;
        loadedRect.right() = loadedRect.left() + w;
        loadedRect.top() = stMonitor.getVRect().top() + 256;
        loadedRect.bottom() = loadedRect.top() + h;
    }
    getStWindow()->setPlacement(loadedRect);

    mySettings->loadBool(ST_SETTING_VSYNC, myIsVSyncOn);

    // load device settings
    if(myDevice == DEVICE_AUTO) {
        int32_t aDevice = myDevice;
        mySettings->loadInt32(ST_SETTING_DEVICE_ID, aDevice);
        myDevice = (DeviceEnum )aDevice;
        if(myDevice == DEVICE_AUTO) {
            myDevice = DUALMODE_SIMPLE;
        }
    }
    mySettings->loadInt32(ST_SETTING_SLAVE_ID, mySlaveMonId);

    // allocate and setup the structure pointer
    optionsStructAlloc();
    getStWindow()->setValue(ST_WIN_DATAKEYS_RENDERER, (size_t )myOptions);

    // create our window!
    getStWindow()->setTitle("sView - Dual Renderer plugin");
    StWinAttributes_t attribs = stDefaultWinAttributes();
    attribs.isSlave = true;
    attribs.slaveMonId = int8_t(mySlaveMonId);
    getStWindow()->stglCreate(&attribs, theNativeParent);
    replaceDualAttribute(DUALMODE_SIMPLE, myDevice);

    // initialize GL context
    myContext = new StGLContext();
    if(!myContext->stglInit()) {
        stError(StString(ST_OUT_PLUGIN_NAME) + " Plugin, OpenGL context is broken!\n(OpenGL library internal error?)");
        return false;
    } else if(!myContext->isGlGreaterEqual(2, 0)) {
        stError(StString(ST_OUT_PLUGIN_NAME) + " Plugin, OpenGL2.0+ not available!");
        return false;
    }

    getStWindow()->stglMakeCurrent(ST_WIN_MASTER);
    if(!myContext->stglSetVSync(myIsVSyncOn ? StGLContext::VSync_ON : StGLContext::VSync_OFF)) {
        // enable/disable VSync by config
        ST_DEBUG_LOG(ST_OUT_PLUGIN_NAME + " Plugin, VSync extension not available!");
    }

    if(!myProgram->init(*myContext)) {
        stError(StString(ST_OUT_PLUGIN_NAME) + " Plugin, Failed to init Shader");
        return false;
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

    return true;
}

void StOutDual::callback(StMessage_t* stMessages) {
    myStCore->callback(stMessages);
    for(size_t i = 0; stMessages[i].uin != StMessageList::MSG_NULL; ++i) {
        switch(stMessages[i].uin) {
            case StMessageList::MSG_KEYS: {
                bool* keysMap = ((bool* )stMessages[i].data);
                if(keysMap[ST_VK_F1]) {
                    replaceDualAttribute(myDevice, DUALMODE_SIMPLE);
                    keysMap[ST_VK_F1] = false;
                } else if(keysMap[ST_VK_F2]) {
                    replaceDualAttribute(myDevice, DUALMODE_XMIRROW);
                    keysMap[ST_VK_F2] = false;
                } else if(keysMap[ST_VK_F3]) {
                    replaceDualAttribute(myDevice, DUALMODE_YMIRROW);
                    keysMap[ST_VK_F3] = false;
                }
                if(keysMap[ST_VK_F12]) {
                    myToShowFPS = !myToShowFPS;
                    keysMap[ST_VK_F12] = false;

                    // send 'update' message to StDrawer
                    StMessage_t msg; msg.uin = StMessageList::MSG_DEVICE_OPTION;
                    StSDSwitch_t* option = ((StSDSwitch_t* )myOptions->options[DEVICE_OPTION_SHOWFPS]);
                    option->value = myToShowFPS; msg.data = (void* )option;
                    getStWindow()->appendMessage(msg);
                }
                break;
            }
            case StMessageList::MSG_DEVICE_INFO: {
                if(myOptions->curDeviceId != myDevice) {
                    StString newPluginPath(myOptions->curRendererPath);
                    if(newPluginPath == myPluginPath) {
                        replaceDualAttribute(myDevice, (DeviceEnum )myOptions->curDeviceId);
                        stMessages[i].uin = StMessageList::MSG_NONE;
                    } // else - another plugin
                }
                break;
            }
            case StMessageList::MSG_DEVICE_OPTION: {
                bool newVSync = ((StSDOnOff_t* )myOptions->options[DEVICE_OPTION_VSYNC])->value;
                if(newVSync != myIsVSyncOn) {
                    myIsVSyncOn = newVSync;
                    getStWindow()->stglMakeCurrent(ST_WIN_MASTER);
                    myContext->stglSetVSync(myIsVSyncOn ? StGLContext::VSync_ON : StGLContext::VSync_OFF);
                }

                myToShowFPS  = ((StSDOnOff_t*  )myOptions->options[DEVICE_OPTION_SHOWFPS])->value;
                mySlaveMonId = (int32_t )(((StSDSwitch_t* )myOptions->options[DEVICE_OPTION_SLAVEID])->value);

                StWinAttributes_t anAttribs = stDefaultWinAttributes();
                getStWindow()->getAttributes(&anAttribs);
                StWinAttributes_t anOrigAttribs = anAttribs;
                anAttribs.slaveMonId = int8_t(mySlaveMonId);
                if(!areSame(&anOrigAttribs, &anAttribs)) {
                    getStWindow()->setAttributes(&anAttribs);
                }

                break;
            }
        }
    }
}

void StOutDual::stglDraw(unsigned int ) {
    myFPSControl.setTargetFPS(getStWindow()->stglGetTargetFps());
    if(myToShowFPS && myFPSControl.isUpdated()) {
        getStWindow()->setTitle(StString("Dual Rendering FPS= ") + myFPSControl.getAverage());
    }

    const StRectI_t aRect = getStWindow()->getPlacement();
    if(!getStWindow()->isStereoOutput() || myIsBroken) {
        getStWindow()->stglMakeCurrent(ST_WIN_MASTER);
        myContext->stglResize(aRect);

        if(myToCompressMem) {
            myFrBuffer->release(*myContext);
        }

        myStCore->stglDraw(ST_DRAW_LEFT);

        // TODO (Kirill Gavrilov#4#) we could do this once
        getStWindow()->stglMakeCurrent(ST_WIN_SLAVE);
        myContext->stglResize(aRect);
        myContext->core20fwd->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        myFPSControl.sleepToTarget(); // decrease FPS to target by thread sleeps
        getStWindow()->stglSwap(ST_WIN_ALL);
        ++myFPSControl;
        return;
    }
    getStWindow()->stglMakeCurrent(ST_WIN_MASTER);
    myContext->stglResize(aRect);
    if(myDevice == DUALMODE_SIMPLE) {
        myStCore->stglDraw(ST_DRAW_LEFT);

        getStWindow()->stglMakeCurrent(ST_WIN_SLAVE);
        myContext->stglResize(aRect);
        myStCore->stglDraw(ST_DRAW_RIGHT);
    } else {
        // resize FBO
        StRectI_t aWinRect = getStWindow()->getPlacement();
        if(!myFrBuffer->initLazy(*myContext, aWinRect.width(), aWinRect.height())) {
            stError(StString(ST_OUT_PLUGIN_NAME) + " Plugin, Failed to init Frame Buffer");
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
        GLint aVPort[4]; // real window viewport
        myContext->core20fwd->glGetIntegerv(GL_VIEWPORT, aVPort);
        myFrBuffer->setupViewPort(*myContext); // we set TEXTURE sizes here
        myFrBuffer->bindBuffer(*myContext);
            myStCore->stglDraw(ST_DRAW_LEFT);
        myFrBuffer->unbindBuffer(*myContext);
        myContext->core20fwd->glViewport(aVPort[0], aVPort[1], aVPort[2], aVPort[3]);

        // now draw to real screen buffer
        // clear the screen and the depth buffer
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

        // draw Right View into virtual frame buffer
        myFrBuffer->setupViewPort(*myContext); // we set TEXTURE sizes here
        myFrBuffer->bindBuffer(*myContext);
            myStCore->stglDraw(ST_DRAW_RIGHT);
        myFrBuffer->unbindBuffer(*myContext);
        myContext->core20fwd->glViewport(aVPort[0], aVPort[1], aVPort[2], aVPort[3]);

        getStWindow()->stglMakeCurrent(ST_WIN_SLAVE);
        myContext->stglResize(aRect);
        // clear the screen and the depth buffer
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
    }
    myFPSControl.sleepToTarget(); // decrease FPS to target by thread sleeps
    getStWindow()->stglSwap(ST_WIN_ALL);
    ++myFPSControl;
    // make sure all GL changes in callback (in StDrawer) will fine
    getStWindow()->stglMakeCurrent(ST_WIN_MASTER);
}

// SDK version was used
ST_EXPORT void getSDKVersion(StVersion* ver) {
    *ver = StVersionInfo::getSDKVersion();
}

// plugin version
ST_EXPORT void getPluginVersion(StVersion* ver) {
    *ver = StVersionInfo::getSDKVersion();
}

ST_EXPORT const StRendererInfo_t* getDevicesInfo(const stBool_t theToDetectPriority) {
    static StRendererInfo_t ST_SELF_INFO = { NULL, NULL, NULL, 0 };
    if(ST_SELF_INFO.devices != NULL) {
        return &ST_SELF_INFO;
    }

    StTranslations aLangMap(ST_OUT_PLUGIN_NAME);

    // detect connected displays
    int supportLevel = ST_DEVICE_SUPPORT_NONE;
    if(theToDetectPriority) {
        if(StCore::INIT() != STERROR_LIBNOERROR) {
            ST_DEBUG_LOG(ST_OUT_PLUGIN_NAME + " Plugin, Core library not available!");
        } else {
            StArrayList<StMonitor> stMonitors = StCore::getStMonitors();
            if(stMonitors.size() >= 2) {
                const StMonitor& mon0 = stMonitors[0];
                const StMonitor& mon1 = stMonitors[1];
                if(   mon0.getVRect().width()  == mon1.getVRect().width()
                   && mon0.getVRect().height() == mon1.getVRect().height()
                   && mon0.getFreq() == mon1.getFreq()) {
                    supportLevel = ST_DEVICE_SUPPORT_HIGHT;
                }
            }
            StCore::FREE();
        }
    }

    // devices list
    static StString aDualName   = aLangMap.changeValueId(STTR_DUAL_NAME,   "Dual Output");
    static StString aDualDesc   = aLangMap.changeValueId(STTR_DUAL_DESC,   "Stereo-device with dual input: some HMD, Mirrored Stereo monitors, Dual-Projectors");
    static StString aMirrorName = aLangMap.changeValueId(STTR_MIRROR_NAME, "Mirror Output");
    static StString aMirrorDesc = aLangMap.changeValueId(STTR_MIRROR_DESC, "Hand-make Mirrored Stereo monitors (mirror in X-direction)");
    static StStereoDeviceInfo_t aDevicesArray[2] = {
        { "StOutDual",    aDualName.toCString(),   aDualDesc.toCString(),   supportLevel },
        { "StOutMirrorX", aMirrorName.toCString(), aMirrorDesc.toCString(), supportLevel }
    };
    ST_SELF_INFO.devices = &aDevicesArray[0];
    ST_SELF_INFO.count   = 2;

    // about string
    StString& aTitle     = aLangMap.changeValueId(STTR_PLUGIN_TITLE,   "sView - Dual Output module");
    StString& aVerString = aLangMap.changeValueId(STTR_VERSION_STRING, "version");
    StString& aDescr     = aLangMap.changeValueId(STTR_PLUGIN_DESCRIPTION,
        "(C) 2007-2012 Kirill Gavrilov <kirill@sview.ru>\nOfficial site: www.sview.ru\n\nThis library distributed under LGPL3.0");
    static StString anAboutString = aTitle + '\n' + aVerString + ": " + StVersionInfo::getSDKVersionString() + "\n \n" + aDescr;
    ST_SELF_INFO.aboutString = (stUtf8_t* )anAboutString.toCString();

    return &ST_SELF_INFO;
}

ST_EXPORT StRendererInterface* StRenderer_new() {
    return new StOutDual(); }
ST_EXPORT void StRenderer_del(StRendererInterface* inst) {
    delete (StOutDual* )inst; }
ST_EXPORT StWindowInterface* StRenderer_getStWindow(StRendererInterface* inst) {
    // This is VERY important return libImpl pointer here!
    return ((StOutDual* )inst)->getStWindow()->getLibImpl(); }
ST_EXPORT stBool_t StRenderer_init(StRendererInterface* inst,
                                   const stUtf8_t*      theRendererPath,
                                   const int&           theDeviceId,
                                   const StNativeWin_t  theNativeParent) {
    return ((StOutDual* )inst)->init(StString(theRendererPath), theDeviceId, theNativeParent); }
ST_EXPORT stBool_t StRenderer_open(StRendererInterface* inst, const StOpenInfo_t* stOpenInfo) {
    return ((StOutDual* )inst)->open(StOpenInfo(stOpenInfo)); }
ST_EXPORT void StRenderer_callback(StRendererInterface* inst, StMessage_t* stMessages) {
    ((StOutDual* )inst)->callback(stMessages); }
ST_EXPORT void StRenderer_stglDraw(StRendererInterface* inst, unsigned int views) {
    ((StOutDual* )inst)->stglDraw(views); }
