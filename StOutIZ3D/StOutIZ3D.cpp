/**
 * Copyright © 2009-2012 Kirill Gavrilov <kirill@sview.ru>
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

#include <StCore/StWindow.h>
#include <StSettings/StSettings.h>
#include <StSettings/StTranslations.h>

namespace {
    static bool HAS_LOGGER_ID = StLogger::IdentifyModule("StOutIZ3D");

    static const char ST_OUT_PLUGIN_NAME[] = "StOutIZ3D";

    // settings
    static const char ST_SETTING_WINDOWPOS[]      = "windowPos";
    static const char ST_SETTING_VSYNC[]          = "vsync";
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
        STTR_PARAMETER_VSYNC    = 1100,
        STTR_PARAMETER_SHOW_FPS = 1101,
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

void StOutIZ3D::optionsStructAlloc() {
    StTranslations stLangMap(ST_OUT_PLUGIN_NAME);

    // create device options structure
    myOptions = (StSDOptionsList_t* )StWindow::memAlloc(sizeof(StSDOptionsList_t)); stMemSet(myOptions, 0, sizeof(StSDOptionsList_t));
    myOptions->curRendererPath = StWindow::memAllocNCopy(myPluginPath);
    myOptions->curDeviceId = 0;

    myOptions->optionsCount = 3;
    myOptions->options = (StSDOption_t** )StWindow::memAlloc(sizeof(StSDOption_t*) * myOptions->optionsCount);

    // VSync option
    myOptions->options[DEVICE_OPTION_VSYNC] = (StSDOption_t* )StWindow::memAlloc(sizeof(StSDOnOff_t));
    myOptions->options[DEVICE_OPTION_VSYNC]->title = StWindow::memAllocNCopy(stLangMap.changeValueId(STTR_PARAMETER_VSYNC, "VSync"));
    myOptions->options[DEVICE_OPTION_VSYNC]->optionType = ST_DEVICE_OPTION_ON_OFF;
    ((StSDOnOff_t* )myOptions->options[DEVICE_OPTION_VSYNC])->value = myIsVSyncOn;

    // Show FPS option
    myOptions->options[DEVICE_OPTION_SHOWFPS] = (StSDOption_t* )StWindow::memAlloc(sizeof(StSDOnOff_t));
    myOptions->options[DEVICE_OPTION_SHOWFPS]->title = StWindow::memAllocNCopy(stLangMap.changeValueId(STTR_PARAMETER_SHOW_FPS, "Show FPS"));
    myOptions->options[DEVICE_OPTION_SHOWFPS]->optionType = ST_DEVICE_OPTION_ON_OFF;
    ((StSDOnOff_t* )myOptions->options[DEVICE_OPTION_SHOWFPS])->value = myToShowFPS;

    // shader switch option
    myOptions->options[DEVICE_OPTION_SHADER] = (StSDOption_t* )StWindow::memAlloc(sizeof(StSDSwitch_t));
    myOptions->options[DEVICE_OPTION_SHADER]->title = StWindow::memAllocNCopy(stLangMap.changeValueId(STTR_PARAMETER_GLASSES, "iZ3D glasses"));
    myOptions->options[DEVICE_OPTION_SHADER]->optionType = ST_DEVICE_OPTION_SWITCH;
    StSDSwitch_t* switchOption = ((StSDSwitch_t* )myOptions->options[DEVICE_OPTION_SHADER]);
    switchOption->value = myShaders.getMode();
    switchOption->valuesCount = 3;
    switchOption->valuesTitles = (stUtf8_t** )StWindow::memAlloc(switchOption->valuesCount * sizeof(stUtf8_t*));
    switchOption->valuesTitles[StOutIZ3DShaders::IZ3D_TABLE_OLD] = StWindow::memAllocNCopy(stLangMap.changeValueId(STTR_PARAMETER_GLASSES_CLASSIC,      "Classic"));
    switchOption->valuesTitles[StOutIZ3DShaders::IZ3D_TABLE_NEW] = StWindow::memAllocNCopy(stLangMap.changeValueId(STTR_PARAMETER_GLASSES_MODERN,       "Modern"));
    switchOption->valuesTitles[StOutIZ3DShaders::IZ3D_CLASSIC  ] = StWindow::memAllocNCopy(stLangMap.changeValueId(STTR_PARAMETER_GLASSES_CLASSIC_FAST, "Classic (fast)"));
}

StAtomic<int32_t> StOutIZ3D::myInstancesNb(0);

StOutIZ3D::StOutIZ3D()
: myOptions(NULL),
  myToSavePlacement(true),
  myIsVSyncOn(true),
  myToShowFPS(false),
  myToCompressMem(myInstancesNb.increment() > 1),
  myIsBroken(false) {
    myFrBuffer = new StGLStereoFrameBuffer();
}

StOutIZ3D::~StOutIZ3D() {
    myInstancesNb.decrement();
    if(!myContext.isNull()) {
        myShaders.release(*myContext);
        myTexTableOld.release(*myContext);
        myTexTableNew.release(*myContext);
        myFrBuffer->release(*myContext);
    }
    if(!myStCore.isNull() && !mySettings.isNull()) {
        stMemFree(myOptions, StWindow::memFree);

        // read windowed placement
        getStWindow()->hide(ST_WIN_MASTER);
        getStWindow()->hide(ST_WIN_SLAVE);
        if(myToSavePlacement) {
            getStWindow()->setFullScreen(false);
            mySettings->saveInt32Rect(ST_SETTING_WINDOWPOS, getStWindow()->getPlacement());
        }
        mySettings->saveBool (ST_SETTING_VSYNC,          myIsVSyncOn);
        mySettings->saveInt32(ST_SETTING_TABLE,          myShaders.getMode());
    }
    mySettings.nullify();
    myStCore.nullify();
    StCore::FREE();
}

bool StOutIZ3D::init(const StString&     theRendererPath,
                     const int& ,
                     const StNativeWin_t theNativeParent) {
    myToSavePlacement = (theNativeParent == (StNativeWin_t )NULL);
    myPluginPath = theRendererPath;
    if(!StVersionInfo::checkTimeBomb("sView - IZ3D Output plugin")) {
        return false;
    }
    ST_DEBUG_LOG_AT("INIT IZ3D output plugin");
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
        loadedRect.left()   = stMonitor.getVRect().left() + 256;
        loadedRect.right()  = loadedRect.left() + w;
        loadedRect.top()    = stMonitor.getVRect().top() + 256;
        loadedRect.bottom() = loadedRect.top() + h;
    }
    getStWindow()->setPlacement(loadedRect);

    mySettings->loadBool(ST_SETTING_VSYNC, myIsVSyncOn);

    int iZ3DMode = myShaders.getMode();
    mySettings->loadInt32(ST_SETTING_TABLE, iZ3DMode);
    myShaders.setMode(iZ3DMode);

    // allocate and setup the structure pointer
    optionsStructAlloc();
    getStWindow()->setValue(ST_WIN_DATAKEYS_RENDERER, (size_t )myOptions);

    // create our window!
    StWinAttributes_t attribs = stDefaultWinAttributes();
    attribs.isSlave = true;
    getStWindow()->stglCreate(&attribs, theNativeParent);

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
    if(!myContext->stglSetVSync(myIsVSyncOn)) {
        // enable/disable VSync by config
        ST_DEBUG_LOG(ST_OUT_PLUGIN_NAME + " Plugin, VSync extension not available!");
    }
    // TODO (Kirill Gavrilov#5) vsync
    ///getStWindow()->stglMakeCurrent(ST_WIN_SLAVE);
    ///stglSetVSync(myIsVSyncOn);

    // INIT iZ3D tables textures
    const StString texturesFolder = StProcess::getStCoreFolder() + "textures" + SYS_FS_SPLITTER;
    const StString tableTextureOldPath = texturesFolder + "iz3dTableOld.std";
    const StString tableTextureNewPath = texturesFolder + "iz3dTableNew.std";

    StHandle<StImageFile> stImage = StImageFile::create();
    if(stImage.isNull()) {
        stError("IZ3D plugin should be linked with at least one image library!");
        return false;
    }
    if(!stImage->load(tableTextureOldPath, StImageFile::ST_TYPE_PNG)) {
        stError(stImage->getState());
        return false;
    }
    myTexTableOld.setMinMagFilter(*myContext, GL_NEAREST); // we need not linear filtrating for lookup-table!
    if(!myTexTableOld.init(*myContext, stImage->getPlane())) {
        stError("Fail to create lookup-table texture!");
        return false;
    }
    if(!stImage->load(tableTextureNewPath, StImageFile::ST_TYPE_PNG)) {
        stError(stImage->getState());
        return false;
    }
    myTexTableNew.setMinMagFilter(*myContext, GL_NEAREST); // we need not linear filtrating for lookup-table!
    if(!myTexTableNew.init(*myContext, stImage->getPlane())) {
        stError("Fail to create lookup-table texture!");
        return false;
    }
    stImage.nullify();

    // INIT shaders
    return myShaders.init(*myContext);
}

void StOutIZ3D::callback(StMessage_t* stMessages) {
    myStCore->callback(stMessages);
    for(size_t i = 0; stMessages[i].uin != StMessageList::MSG_NULL; ++i) {
        switch(stMessages[i].uin) {
            case StMessageList::MSG_RESIZE: {
                StRectI_t aRect = getStWindow()->getPlacement();
                getStWindow()->stglMakeCurrent(ST_WIN_MASTER);
                myContext->stglResize(aRect);
                getStWindow()->stglMakeCurrent(ST_WIN_SLAVE);
                myContext->stglResize(aRect);
                break;
            }
            case StMessageList::MSG_KEYS: {
                bool* keysMap = ((bool* )stMessages[i].data);
                if(keysMap[ST_VK_F1]) {
                    myShaders.setMode(StOutIZ3DShaders::IZ3D_TABLE_OLD); keysMap[ST_VK_F1] = false;

                    // send 'update' message to StDrawer
                    StMessage_t msg; msg.uin = StMessageList::MSG_DEVICE_OPTION;
                    StSDSwitch_t* option = ((StSDSwitch_t* )myOptions->options[DEVICE_OPTION_SHADER]);
                    option->value = StOutIZ3DShaders::IZ3D_TABLE_OLD;
                    msg.data = (void* )option->valuesTitles[option->value];
                    getStWindow()->appendMessage(msg);
                } else if(keysMap[ST_VK_F2]) {
                    myShaders.setMode(StOutIZ3DShaders::IZ3D_TABLE_NEW); keysMap[ST_VK_F2] = false;

                    // send 'update' message to StDrawer
                    StMessage_t msg; msg.uin = StMessageList::MSG_DEVICE_OPTION;
                    StSDSwitch_t* option = ((StSDSwitch_t* )myOptions->options[DEVICE_OPTION_SHADER]);
                    option->value = StOutIZ3DShaders::IZ3D_TABLE_NEW;
                    msg.data = (void* )option->valuesTitles[option->value];
                    getStWindow()->appendMessage(msg);
                } else if(keysMap[ST_VK_F3]) {
                    myShaders.setMode(StOutIZ3DShaders::IZ3D_CLASSIC); keysMap[ST_VK_F3] = false;

                    // send 'update' message to StDrawer
                    StMessage_t msg; msg.uin = StMessageList::MSG_DEVICE_OPTION;
                    StSDSwitch_t* option = ((StSDSwitch_t* )myOptions->options[DEVICE_OPTION_SHADER]);
                    option->value = StOutIZ3DShaders::IZ3D_CLASSIC;
                    msg.data = (void* )option->valuesTitles[option->value];
                    getStWindow()->appendMessage(msg);
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
            case StMessageList::MSG_DEVICE_OPTION: {
                bool newVSync = ((StSDOnOff_t* )myOptions->options[DEVICE_OPTION_VSYNC])->value;
                if(newVSync != myIsVSyncOn) {
                    myIsVSyncOn = newVSync;
                    getStWindow()->stglMakeCurrent(ST_WIN_MASTER);
                    myContext->stglSetVSync(myIsVSyncOn);
                    // TODO (Kirill Gavrilov#5) vsync
                    ///getStWindow()->stglMakeCurrent(ST_WIN_SLAVE);
                    ///stglSetVSync(myIsVSyncOn);
                }

                myToShowFPS = ((StSDOnOff_t* )myOptions->options[DEVICE_OPTION_SHOWFPS])->value;
                myShaders.setMode((int )((StSDSwitch_t* )myOptions->options[DEVICE_OPTION_SHADER])->value);
                break;
            }
        }
    }
}

void StOutIZ3D::stglDraw(unsigned int ) {
    myFPSControl.setTargetFPS(getStWindow()->stglGetTargetFps());
    if(myToShowFPS && myFPSControl.isUpdated()) {
        getStWindow()->setTitle(StString("iZ3D Rendering FPS= ") + myFPSControl.getAverage());
    }

    if(!getStWindow()->isStereoOutput() || myIsBroken) {
        getStWindow()->stglMakeCurrent(ST_WIN_MASTER);
        if(myToCompressMem) {
            myFrBuffer->release(*myContext);
        }

        myStCore->stglDraw(ST_DRAW_LEFT);

        getStWindow()->stglMakeCurrent(ST_WIN_SLAVE);
        myContext->core20fwd->glClearColor(0.729740052840723f, 0.729740052840723f, 0.729740052840723f, 0.0f);
        // clear the screen and the depth buffer
        myContext->core20fwd->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        myContext->core20fwd->glClearColor(0, 0, 0, 0);

        myFPSControl.sleepToTarget(); // decrease FPS to target by thread sleeps
        getStWindow()->stglSwap(ST_WIN_MASTER);
        getStWindow()->stglSwap(ST_WIN_SLAVE);
        ++myFPSControl;
        return;
    }
    getStWindow()->stglMakeCurrent(ST_WIN_MASTER);

    // resize FBO
    StRectI_t aWinRect = getStWindow()->getPlacement();
    if(!myFrBuffer->initLazy(*myContext, aWinRect.width(), aWinRect.height())) {
        stError(StString(ST_OUT_PLUGIN_NAME) + " Plugin, Failed to init Frame Buffer");
        myIsBroken = true;
        return;
    }

    // draw into virtual frame buffers (textures)
    GLint aVPort[4]; // real window viewport
    myContext->core20fwd->glGetIntegerv(GL_VIEWPORT, aVPort);
    myFrBuffer->setupViewPort(*myContext);    // we set TEXTURE sizes here
    myFrBuffer->bindBufferLeft(*myContext);
        myStCore->stglDraw(ST_DRAW_LEFT);
    myFrBuffer->bindBufferRight(*myContext);
        myStCore->stglDraw(ST_DRAW_RIGHT);
    myFrBuffer->unbindBufferRight(*myContext);
    myContext->core20fwd->glViewport(aVPort[0], aVPort[1], aVPort[2], aVPort[3]);

    // now draw to real screen buffer
    // clear the screen and the depth buffer
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

    getStWindow()->stglMakeCurrent(ST_WIN_SLAVE);
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

    myFPSControl.sleepToTarget(); // decrease FPS to target by thread sleeps
    getStWindow()->stglSwap(ST_WIN_MASTER);
    getStWindow()->stglSwap(ST_WIN_SLAVE);
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
    int aSupportLevel = ST_DEVICE_SUPPORT_NONE;
    if(theToDetectPriority) {
        if(StCore::INIT() != STERROR_LIBNOERROR) {
            ST_DEBUG_LOG(ST_OUT_PLUGIN_NAME + " Plugin, Core library not available!");
        } else {
            StArrayList<StMonitor> stMonitors = StCore::getStMonitors();
            for(size_t m = 0; m < stMonitors.size(); ++m) {
                const StMonitor& mon = stMonitors[m];
                if(isFrontDisplay(mon.getPnPId()) || isBackDisplay(mon.getPnPId())) {
                    aSupportLevel = ST_DEVICE_SUPPORT_PREFER; // we sure that iZ3D connected
                    break;
                } else if(mon.getPnPId() == IZ3D_MODEL_MATROXTH2GO0 || mon.getPnPId() == IZ3D_MODEL_MATROXTH2GO1) {
                    // TODO
                    aSupportLevel = ST_DEVICE_SUPPORT_FULL; // is it possible
                }
            }
            StCore::FREE();
        }
    }

    // devices list
    static StString aName = aLangMap.changeValueId(STTR_IZ3D_NAME, "IZ3D Display");
    static StString aDesc = aLangMap.changeValueId(STTR_IZ3D_DESC, "IZ3D Display");
    static StStereoDeviceInfo_t aDevicesArray[1] = {
        { "IZ3D", aName.toCString(), aDesc.toCString(), aSupportLevel }
    };
    ST_SELF_INFO.devices = &aDevicesArray[0];
    ST_SELF_INFO.count   = 1;

    // about string
    StString& aTitle     = aLangMap.changeValueId(STTR_PLUGIN_TITLE, "sView - iZ3D Output module");
    StString& aVerString = aLangMap.changeValueId(STTR_VERSION_STRING, "version");
    StString& aDescr     = aLangMap.changeValueId(STTR_PLUGIN_DESCRIPTION,
        "(C) 2009-2012 Kirill Gavrilov <kirill@sview.ru>\nOfficial site: www.sview.ru\n\nThis library distributed under LGPL3.0");
    static StString anAboutString = aTitle + '\n' + aVerString + ": " + StVersionInfo::getSDKVersionString() + "\n \n" + aDescr;
    ST_SELF_INFO.aboutString = (stUtf8_t* )anAboutString.toCString();

    return &ST_SELF_INFO;
}

ST_EXPORT StRendererInterface* StRenderer_new() {
    return new StOutIZ3D(); }
ST_EXPORT void StRenderer_del(StRendererInterface* inst) {
    delete (StOutIZ3D* )inst; }
ST_EXPORT StWindowInterface* StRenderer_getStWindow(StRendererInterface* inst) {
    // This is VERY important return libImpl pointer here!
    return ((StOutIZ3D* )inst)->getStWindow()->getLibImpl(); }
ST_EXPORT stBool_t StRenderer_init(StRendererInterface* inst,
                                   const stUtf8_t*      theRendererPath,
                                   const int&           theDeviceId,
                                   const StNativeWin_t  theNativeParent) {
    return ((StOutIZ3D* )inst)->init(StString(theRendererPath), theDeviceId, theNativeParent); }
ST_EXPORT stBool_t StRenderer_open(StRendererInterface* inst, const StOpenInfo_t* stOpenInfo) {
    return ((StOutIZ3D* )inst)->open(StOpenInfo(stOpenInfo)); }
ST_EXPORT void StRenderer_callback(StRendererInterface* inst, StMessage_t* stMessages) {
    ((StOutIZ3D* )inst)->callback(stMessages); }
ST_EXPORT void StRenderer_stglDraw(StRendererInterface* inst, unsigned int views) {
    ((StOutIZ3D* )inst)->stglDraw(views); }
