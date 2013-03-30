/**
 * Copyright © 2007-2012 Kirill Gavrilov <kirill@sview.ru>
 *
 * StOutAnaglyph library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * StOutAnaglyph library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.
 * If not, see <http://www.gnu.org/licenses/>.
 */

#include "StOutAnaglyph.h"

#include <StGL/StGLEnums.h>
#include <StGL/StGLContext.h>
#include <StGLCore/StGLCore20.h>
#include <StThreads/StThreads.h> // threads header (mutexes, threads,...)
#include <StCore/StWindow.h>
#include <StSettings/StSettings.h>
#include <StSettings/StTranslations.h>

namespace {
    static bool HAS_LOGGER_ID = StLogger::IdentifyModule("StOutAnaglyph");

    // shaders data
    static const char VSHADER[]         = "vAnaglyph.shv";
    static const char FSHADER_SIMPLE[]  = "fAnaglyphSimple.shf";
    static const char FSHADER_GRAY[]    = "fAnaglyphGray.shf";
    static const char FSHADER_TRUE[]    = "fAnaglyphTrue.shf";
    static const char FSHADER_OPTIM[]   = "fAnaglyphOptim.shf";
    static const char FSHADER_YELLOW[]  = "fAnaglyphYellow.shf";
    static const char FSHADER_YELLOWD[] = "fAnaglyphYellowDubois.shf";
    static const char FSHADER_GREEN[]   = "fAnaglyphGreen.shf";

    static const char ST_OUT_PLUGIN_NAME[] = "StOutAnaglyph";

    static const char ST_SETTING_GLASSES[]        = "glasses";
    static const char ST_SETTING_REDCYAN[]        = "optionRedCyan";
    static const char ST_SETTING_AMBERBLUE[]      = "optionAmberBlue";
    static const char ST_SETTING_WINDOWPOS[]      = "windowPos";
    static const char ST_SETTING_VSYNC[]          = "vsync";

    // translation resources
    enum {
        // devices' names
        STTR_ANAGLYPH_NAME     = 1000,
        STTR_ANAGLYPH_DESC     = 1001,

        // glasses
        STTR_ANAGLYPH_GLASSES  = 1010,
        STTR_ANAGLYPH_REDCYAN  = 1011,
        STTR_ANAGLYPH_YELLOW   = 1012,
        STTR_ANAGLYPH_GREEN    = 1013,

        // parameters
        STTR_ANAGLYPH_VSYNC            = 1100,
        STTR_ANAGLYPH_REDCYAN_MENU     = 1102,
        STTR_ANAGLYPH_REDCYAN_SIMPLE   = 1120,
        STTR_ANAGLYPH_REDCYAN_OPTIM    = 1121,
        STTR_ANAGLYPH_REDCYAN_GRAY     = 1122,
        STTR_ANAGLYPH_REDCYAN_DARK     = 1123,
        STTR_ANAGLYPH_AMBERBLUE_MENU   = 1103,
        STTR_ANAGLYPH_AMBERBLUE_SIMPLE = 1130,
        STTR_ANAGLYPH_AMBERBLUE_DUBIOS = 1131,

        // about info
        STTR_PLUGIN_TITLE       = 2000,
        STTR_VERSION_STRING     = 2001,
        STTR_PLUGIN_DESCRIPTION = 2002,
    };

};

StAtomic<int32_t> StOutAnaglyph::myInstancesNb(0);

void StOutAnaglyph::setShader(const int theGlasses,
                              const int theOptionRedCyan,
                              const int theOptionAmberBlue) {
    myGlasses         = theGlasses;
    myOptionRedCyan   = theOptionRedCyan;
    myOptionAmberBlue = theOptionAmberBlue;
    switch(theGlasses) {
        case GLASSES_TYPE_REDCYAN: {
            switch(myOptionRedCyan) {
                case REDCYAN_MODE_OPTIM:  myStereoProgram = &myOptimAnaglyph; break;
                case REDCYAN_MODE_GRAY:   myStereoProgram = &myGrayAnaglyph;  break;
                case REDCYAN_MODE_DARK:   myStereoProgram = &myTrueAnaglyph;  break;
                case REDCYAN_MODE_SIMPLE:
                default: myStereoProgram = &mySimpleAnaglyph; break;
            }
            break;
        }
        case GLASSES_TYPE_YELLOW: {
            switch(myOptionAmberBlue) {
                case AMBERBLUE_MODE_DUBOIS: myStereoProgram = &myYellowDubiosAnaglyph; break;
                case AMBERBLUE_MODE_SIMPLE:
                default: myStereoProgram = &myYellowAnaglyph; break;
            }
            break;
        }
        case GLASSES_TYPE_GREEN: myStereoProgram = &myGreenAnaglyph; break;
    }
    if(myOptions != NULL) {
        StSDSwitch_t* optionGlasses  = ((StSDSwitch_t* )myOptions->options[DEVICE_OPTION_GLASSES]);
        optionGlasses->value = myGlasses;
        StSDSwitch_t* optionRCFilter = ((StSDSwitch_t* )myOptions->options[DEVICE_OPTION_REDCYAN]);
        optionRCFilter->value = myOptionRedCyan;
        StSDSwitch_t* optionABFilter = ((StSDSwitch_t* )myOptions->options[DEVICE_OPTION_YELLOW]);
        optionABFilter->value = myOptionAmberBlue;
    }
}

void StOutAnaglyph::optionsStructAlloc() {
    StTranslations stLangMap(ST_OUT_PLUGIN_NAME);

    // create device options structure
    myOptions = (StSDOptionsList_t* )StWindow::memAlloc(sizeof(StSDOptionsList_t)); stMemSet(myOptions, 0, sizeof(StSDOptionsList_t));
    myOptions->curRendererPath = StWindow::memAllocNCopy(myPluginPath);
    myOptions->curDeviceId = 0;

    myOptions->optionsCount = 4;
    myOptions->options = (StSDOption_t** )StWindow::memAlloc(sizeof(StSDOption_t*) * myOptions->optionsCount);

    // VSync option
    myOptions->options[DEVICE_OPTION_VSYNC] = (StSDOption_t* )StWindow::memAlloc(sizeof(StSDOnOff_t));
    myOptions->options[DEVICE_OPTION_VSYNC]->optionType = ST_DEVICE_OPTION_ON_OFF;
    ((StSDOnOff_t* )myOptions->options[DEVICE_OPTION_VSYNC])->value = myIsVSyncOn;
    myOptions->options[DEVICE_OPTION_VSYNC]->title = StWindow::memAllocNCopy(stLangMap.changeValueId(STTR_ANAGLYPH_VSYNC, "VSync"));

    // Glasses switch option
    myOptions->options[DEVICE_OPTION_GLASSES] = (StSDOption_t* )StWindow::memAlloc(sizeof(StSDSwitch_t));
    myOptions->options[DEVICE_OPTION_GLASSES]->title = StWindow::memAllocNCopy(stLangMap.changeValueId(STTR_ANAGLYPH_GLASSES, "Glasses type"));
    myOptions->options[DEVICE_OPTION_GLASSES]->optionType = ST_DEVICE_OPTION_SWITCH;
    StSDSwitch_t* switchOptionGlasses = ((StSDSwitch_t* )myOptions->options[DEVICE_OPTION_GLASSES]);
    switchOptionGlasses->value = myGlasses;
    switchOptionGlasses->valuesCount = 3;
    switchOptionGlasses->valuesTitles = (stUtf8_t** )StWindow::memAlloc(switchOptionGlasses->valuesCount * sizeof(stUtf8_t*));
    switchOptionGlasses->valuesTitles[GLASSES_TYPE_REDCYAN] = StWindow::memAllocNCopy(stLangMap.changeValueId(STTR_ANAGLYPH_REDCYAN, "Red-cyan"));
    switchOptionGlasses->valuesTitles[GLASSES_TYPE_YELLOW]  = StWindow::memAllocNCopy(stLangMap.changeValueId(STTR_ANAGLYPH_YELLOW,  "Yellow-Blue"));
    switchOptionGlasses->valuesTitles[GLASSES_TYPE_GREEN]   = StWindow::memAllocNCopy(stLangMap.changeValueId(STTR_ANAGLYPH_GREEN,   "Green-Magenta"));

    // Red-cyan filter switch option
    myOptions->options[DEVICE_OPTION_REDCYAN] = (StSDOption_t* )StWindow::memAlloc(sizeof(StSDSwitch_t));
    myOptions->options[DEVICE_OPTION_REDCYAN]->title = StWindow::memAllocNCopy(stLangMap.changeValueId(STTR_ANAGLYPH_REDCYAN_MENU, "Red-Cyan filter"));
    myOptions->options[DEVICE_OPTION_REDCYAN]->optionType = ST_DEVICE_OPTION_SWITCH;
    StSDSwitch_t* switchOptionRCFilter = ((StSDSwitch_t* )myOptions->options[DEVICE_OPTION_REDCYAN]);
    switchOptionRCFilter->value = myOptionRedCyan;
    switchOptionRCFilter->valuesCount = 4;
    switchOptionRCFilter->valuesTitles = (stUtf8_t** )StWindow::memAlloc(switchOptionRCFilter->valuesCount * sizeof(stUtf8_t*));
    switchOptionRCFilter->valuesTitles[REDCYAN_MODE_SIMPLE] = StWindow::memAllocNCopy(stLangMap.changeValueId(STTR_ANAGLYPH_REDCYAN_SIMPLE, "Simple"));
    switchOptionRCFilter->valuesTitles[REDCYAN_MODE_OPTIM]  = StWindow::memAllocNCopy(stLangMap.changeValueId(STTR_ANAGLYPH_REDCYAN_OPTIM,  "Optimized"));
    switchOptionRCFilter->valuesTitles[REDCYAN_MODE_GRAY]   = StWindow::memAllocNCopy(stLangMap.changeValueId(STTR_ANAGLYPH_REDCYAN_GRAY,   "Grayed"));
    switchOptionRCFilter->valuesTitles[REDCYAN_MODE_DARK]   = StWindow::memAllocNCopy(stLangMap.changeValueId(STTR_ANAGLYPH_REDCYAN_DARK,   "Dark"));

    // Amber-Blue filter switch option
    myOptions->options[DEVICE_OPTION_YELLOW] = (StSDOption_t* )StWindow::memAlloc(sizeof(StSDSwitch_t));
    myOptions->options[DEVICE_OPTION_YELLOW]->title = StWindow::memAllocNCopy(stLangMap.changeValueId(STTR_ANAGLYPH_AMBERBLUE_MENU, "Yellow filter"));
    myOptions->options[DEVICE_OPTION_YELLOW]->optionType = ST_DEVICE_OPTION_SWITCH;
    StSDSwitch_t* switchOptionABFilter = ((StSDSwitch_t* )myOptions->options[DEVICE_OPTION_YELLOW]);
    switchOptionABFilter->value = myOptionAmberBlue;
    switchOptionABFilter->valuesCount = 2;
    switchOptionABFilter->valuesTitles = (stUtf8_t** )StWindow::memAlloc(switchOptionABFilter->valuesCount * sizeof(stUtf8_t*));
    switchOptionABFilter->valuesTitles[AMBERBLUE_MODE_SIMPLE] = StWindow::memAllocNCopy(stLangMap.changeValueId(STTR_ANAGLYPH_AMBERBLUE_SIMPLE, "Simple"));
    switchOptionABFilter->valuesTitles[AMBERBLUE_MODE_DUBOIS] = StWindow::memAllocNCopy(stLangMap.changeValueId(STTR_ANAGLYPH_AMBERBLUE_DUBIOS, "Dubios"));
}

StOutAnaglyph::StOutAnaglyph()
: myStereoProgram(NULL),
  mySimpleAnaglyph("Anaglyph Simple"),
  myGrayAnaglyph("Anaglyph Gray"),
  myTrueAnaglyph("Anaglyph True"),
  myOptimAnaglyph("Anaglyph Optimized"),
  myYellowAnaglyph("Anaglyph Yellow"),
  myYellowDubiosAnaglyph("Anaglyph Yellow Dubios"),
  myGreenAnaglyph("Anaglyph Green"),
  myGlasses(GLASSES_TYPE_REDCYAN),
  myOptionRedCyan(REDCYAN_MODE_SIMPLE),
  myOptionAmberBlue(AMBERBLUE_MODE_SIMPLE),
  myOptions(NULL),
  myToSavePlacement(true),
  myIsVSyncOn(true),
  myToCompressMem(myInstancesNb.increment() > 1),
  myIsBroken(false) {
    myFrBuffer      = new StGLStereoFrameBuffer();
    myStereoProgram = &mySimpleAnaglyph;
}

StOutAnaglyph::~StOutAnaglyph() {
    myInstancesNb.decrement();
    if(!myStCore.isNull() && !mySettings.isNull()) {
        if(!myContext.isNull()) {
            StGLContext& aCtx = *myContext;
            mySimpleAnaglyph.release(aCtx);
            myGrayAnaglyph.release(aCtx);
            myTrueAnaglyph.release(aCtx);
            myOptimAnaglyph.release(aCtx);
            myYellowAnaglyph.release(aCtx);
            myYellowDubiosAnaglyph.release(aCtx);
            myGreenAnaglyph.release(aCtx);
            myFrBuffer->release(*myContext);
        }
        stMemFree(myOptions, StWindow::memFree);

        // read windowed placement
        getStWindow()->hide(ST_WIN_MASTER);
        if(myToSavePlacement) {
            getStWindow()->setFullScreen(false);
            StRect<int32_t> savedRect = getStWindow()->getPlacement();
            mySettings->saveInt32Rect(ST_SETTING_WINDOWPOS, savedRect);
        }
        mySettings->saveBool (ST_SETTING_VSYNC,          myIsVSyncOn);
        mySettings->saveInt32(ST_SETTING_GLASSES,        myGlasses);
        mySettings->saveInt32(ST_SETTING_REDCYAN,        myOptionRedCyan);
        mySettings->saveInt32(ST_SETTING_AMBERBLUE,      myOptionAmberBlue);
    }
    mySettings.nullify();
    myStCore.nullify();
    StCore::FREE();
}

namespace {
    static bool initProgram(StGLContext&      theCtx,
                            StGLStereoFrameBuffer::StGLStereoProgram& theProgram,
                            StGLVertexShader& theVertShader,
                            const StString&   theFragPath) {
        StGLFragmentShader aFragShader(theProgram.getTitle());
        if(!aFragShader.initFile(theCtx, theFragPath)) {
            aFragShader.release(theCtx);
            return false;
        }

        theProgram.create(theCtx)
                  .attachShader(theCtx, theVertShader)
                  .attachShader(theCtx, aFragShader)
                  .link(theCtx);

        aFragShader.release(theCtx);
        if(!theProgram.isValid()) {
            theProgram.release(theCtx);
            return false;
        }

        return true;
    }
};

bool StOutAnaglyph::init(const StString&     theRendererPath,
                         const int& ,
                         const StNativeWin_t theNativeParent) {
    myToSavePlacement = (theNativeParent == (StNativeWin_t )NULL);
    myPluginPath = theRendererPath;
    if(!StVersionInfo::checkTimeBomb("sView - Anaglyph Output plugin")) {
        return false;
    }
    ST_DEBUG_LOG_AT("INIT Anaglyph output plugin");
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

    // load glasses settings
    mySettings->loadInt32(ST_SETTING_GLASSES,   myGlasses);
    mySettings->loadInt32(ST_SETTING_REDCYAN,   myOptionRedCyan);
    mySettings->loadInt32(ST_SETTING_AMBERBLUE, myOptionAmberBlue);
    setShader(myGlasses, myOptionRedCyan, myOptionAmberBlue);

    // allocate and setup the structure pointer
    optionsStructAlloc();
    getStWindow()->setValue(ST_WIN_DATAKEYS_RENDERER, (size_t )myOptions);

    // create our window!
    getStWindow()->setTitle("sView - Anaglyph Renderer plugin");
    StWinAttributes_t attribs = stDefaultWinAttributes();
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
    if(!myContext->stglSetVSync(myIsVSyncOn ? StGLContext::VSync_ON : StGLContext::VSync_OFF)) {
        // enable/disable VSync by config
        ST_DEBUG_LOG(ST_OUT_PLUGIN_NAME + " Plugin, VSync extension not available!");
    }

    // INIT shaders
    StString aShadersError = StString(ST_OUT_PLUGIN_NAME) + " Plugin, Failed to init Shaders";
    const StString aShadersRoot = StProcess::getStCoreFolder() + "shaders" + SYS_FS_SPLITTER
                                + ST_OUT_PLUGIN_NAME + SYS_FS_SPLITTER;
    StGLVertexShader aVertShader("Anaglyph"); // common vertex shader
    if(!aVertShader.initFile(*myContext, aShadersRoot + VSHADER)
    || !initProgram(*myContext, mySimpleAnaglyph, aVertShader,
                    aShadersRoot + FSHADER_SIMPLE)
    || !initProgram(*myContext, myGrayAnaglyph, aVertShader,
                    aShadersRoot + FSHADER_GRAY)
    || !initProgram(*myContext, myTrueAnaglyph, aVertShader,
                    aShadersRoot + FSHADER_TRUE)
    || !initProgram(*myContext, myOptimAnaglyph, aVertShader,
                    aShadersRoot + FSHADER_OPTIM)
    || !initProgram(*myContext, myYellowAnaglyph, aVertShader,
                    aShadersRoot + FSHADER_YELLOW)
    || !initProgram(*myContext, myYellowDubiosAnaglyph, aVertShader,
                    aShadersRoot + FSHADER_YELLOWD)
    || !initProgram(*myContext, myGreenAnaglyph, aVertShader,
                    aShadersRoot + FSHADER_GREEN)) {
        aVertShader.release(*myContext);
        stError(aShadersError);
        return false;
    }

    aVertShader.release(*myContext);
    return true;
}

void StOutAnaglyph::callback(StMessage_t* stMessages) {
    myStCore->callback(stMessages);
    for(size_t i = 0; stMessages[i].uin != StMessageList::MSG_NULL; ++i) {
        switch(stMessages[i].uin) {
            case StMessageList::MSG_KEYS: {
                bool* keysMap = ((bool* )stMessages[i].data);
                if(keysMap[ST_VK_F1]) {
                    setShader(GLASSES_TYPE_REDCYAN, REDCYAN_MODE_SIMPLE, myOptionAmberBlue); keysMap[ST_VK_F1] = false;

                    // send 'update' message to StDrawer
                    StMessage_t msg; msg.uin = StMessageList::MSG_DEVICE_OPTION;
                    StSDSwitch_t* option = ((StSDSwitch_t* )myOptions->options[DEVICE_OPTION_REDCYAN]);
                    msg.data = (void* )option->valuesTitles[option->value];
                    getStWindow()->appendMessage(msg);
                } else if(keysMap[ST_VK_F2]) {
                    setShader(GLASSES_TYPE_REDCYAN, REDCYAN_MODE_OPTIM, myOptionAmberBlue);  keysMap[ST_VK_F2] = false;

                    // send 'update' message to StDrawer
                    StMessage_t msg; msg.uin = StMessageList::MSG_DEVICE_OPTION;
                    StSDSwitch_t* option = ((StSDSwitch_t* )myOptions->options[DEVICE_OPTION_REDCYAN]);
                    msg.data = (void* )option->valuesTitles[option->value];
                    getStWindow()->appendMessage(msg);
                } else if(keysMap[ST_VK_F3]) {
                    setShader(GLASSES_TYPE_REDCYAN, REDCYAN_MODE_GRAY, myOptionAmberBlue);   keysMap[ST_VK_F3] = false;

                    // send 'update' message to StDrawer
                    StMessage_t msg; msg.uin = StMessageList::MSG_DEVICE_OPTION;
                    StSDSwitch_t* option = ((StSDSwitch_t* )myOptions->options[DEVICE_OPTION_REDCYAN]);
                    msg.data = (void* )option->valuesTitles[option->value];
                    getStWindow()->appendMessage(msg);
                } else if(keysMap[ST_VK_F4]) {
                    setShader(GLASSES_TYPE_REDCYAN, REDCYAN_MODE_DARK, myOptionAmberBlue);   keysMap[ST_VK_F4] = false;

                    // send 'update' message to StDrawer
                    StMessage_t msg; msg.uin = StMessageList::MSG_DEVICE_OPTION;
                    StSDSwitch_t* option = ((StSDSwitch_t* )myOptions->options[DEVICE_OPTION_REDCYAN]);
                    msg.data = (void* )option->valuesTitles[option->value];
                    getStWindow()->appendMessage(msg);
                } else if(keysMap[ST_VK_F5]) {
                    setShader(GLASSES_TYPE_YELLOW, myOptionRedCyan, myOptionAmberBlue);      keysMap[ST_VK_F5] = false;

                    // send 'update' message to StDrawer
                    StMessage_t msg; msg.uin = StMessageList::MSG_DEVICE_OPTION;
                    StSDSwitch_t* option = ((StSDSwitch_t* )myOptions->options[DEVICE_OPTION_REDCYAN]);
                    msg.data = (void* )option->valuesTitles[option->value];
                    getStWindow()->appendMessage(msg);
                } else if(keysMap[ST_VK_F6]) {
                    setShader(GLASSES_TYPE_GREEN, myOptionRedCyan, myOptionAmberBlue);       keysMap[ST_VK_F6] = false;

                    // send 'update' message to StDrawer
                    StMessage_t msg; msg.uin = StMessageList::MSG_DEVICE_OPTION;
                    StSDSwitch_t* option = ((StSDSwitch_t* )myOptions->options[DEVICE_OPTION_REDCYAN]);
                    msg.data = (void* )option->valuesTitles[option->value];
                    getStWindow()->appendMessage(msg);
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

                setShader ((int )((StSDSwitch_t* )myOptions->options[DEVICE_OPTION_GLASSES])->value,
                           (int )((StSDSwitch_t* )myOptions->options[DEVICE_OPTION_REDCYAN])->value,
                           (int )((StSDSwitch_t* )myOptions->options[DEVICE_OPTION_YELLOW ])->value);
                break;
            }
        }
    }
}

void StOutAnaglyph::stglDraw(unsigned int ) {
    myFPSControl.setTargetFPS(getStWindow()->stglGetTargetFps());
    if(!getStWindow()->isStereoOutput() || myIsBroken) {
        getStWindow()->stglMakeCurrent(ST_WIN_MASTER);
        myContext->stglResize(getStWindow()->getPlacement());
        if(myToCompressMem) {
            myFrBuffer->release(*myContext);
        }

        myStCore->stglDraw(ST_DRAW_LEFT);

        myFPSControl.sleepToTarget(); // decrease FPS to target by thread sleeps
        getStWindow()->stglSwap(ST_WIN_MASTER);
        ++myFPSControl;
        return;
    }
    getStWindow()->stglMakeCurrent(ST_WIN_MASTER);
    myContext->stglResize(getStWindow()->getPlacement());
    const StRectI_t aWinRect = getStWindow()->getPlacement();

    // resize FBO
    if(!myFrBuffer->initLazy(*myContext, aWinRect.width(), aWinRect.height())) {
        stError(StString(ST_OUT_PLUGIN_NAME) + " Plugin, Failed to init Frame Buffer");
        myIsBroken = true;
        return;
    }

    // draw into virtual frame buffers (textures)
    GLint aVPort[4]; // real window viewport
    myContext->core20fwd->glGetIntegerv(GL_VIEWPORT, aVPort);
    myFrBuffer->setupViewPort(*myContext);       // we set TEXTURE sizes here
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
    myFrBuffer->bindMultiTexture(*myContext);
    myFrBuffer->drawQuad(*myContext, myStereoProgram);
    myFrBuffer->unbindMultiTexture(*myContext);

    myFPSControl.sleepToTarget(); // decrease FPS to target by thread sleeps
    getStWindow()->stglSwap(ST_WIN_MASTER);
    ++myFPSControl;
}

// SDK version was used
ST_EXPORT void getSDKVersion(StVersion* ver) {
    *ver = StVersionInfo::getSDKVersion();
}

// plugin version
ST_EXPORT void getPluginVersion(StVersion* ver) {
    *ver = StVersionInfo::getSDKVersion();
}

ST_EXPORT const StRendererInfo_t* getDevicesInfo(const stBool_t /*theToDetectPriority*/) {
    static StRendererInfo_t ST_SELF_INFO = { NULL, NULL, NULL, 0 };
    if(ST_SELF_INFO.devices != NULL) {
        return &ST_SELF_INFO;
    }

    StTranslations aLangMap(ST_OUT_PLUGIN_NAME);

    // devices list
    static StString aName = aLangMap.changeValueId(STTR_ANAGLYPH_NAME, "Anaglyph glasses");
    static StString aDesc = aLangMap.changeValueId(STTR_ANAGLYPH_DESC, "Simple glasses with color-filters");
    static StStereoDeviceInfo_t aDevicesArray[1] = {
        { "Anaglyph", aName.toCString(), aDesc.toCString(), ST_DEVICE_SUPPORT_LOW } // anaglyph could be run on every display...
    };
    ST_SELF_INFO.devices = &aDevicesArray[0];
    ST_SELF_INFO.count   = 1;

    // about string
    StString& aTitle     = aLangMap.changeValueId(STTR_PLUGIN_TITLE, "sView - Anaglyph Output module");
    StString& aVerString = aLangMap.changeValueId(STTR_VERSION_STRING, "version");
    StString& aDescr     = aLangMap.changeValueId(STTR_PLUGIN_DESCRIPTION,
        "(C) 2007-2013 Kirill Gavrilov <kirill@sview.ru>\nOfficial site: www.sview.ru\n\nThis library distributed under LGPL3.0");
    static StString anAboutString = aTitle + '\n' + aVerString + ": " + StVersionInfo::getSDKVersionString() + "\n \n" + aDescr;
    ST_SELF_INFO.aboutString = (stUtf8_t* )anAboutString.toCString();

    return &ST_SELF_INFO;
}

ST_EXPORT StRendererInterface* StRenderer_new() {
    return new StOutAnaglyph(); }
ST_EXPORT void StRenderer_del(StRendererInterface* inst) {
    delete (StOutAnaglyph* )inst; }
ST_EXPORT StWindowInterface* StRenderer_getStWindow(StRendererInterface* inst) {
    // This is VERY important return libImpl pointer here!
    return ((StOutAnaglyph* )inst)->getStWindow()->getLibImpl(); }
ST_EXPORT stBool_t StRenderer_init(StRendererInterface* inst,
                                   const stUtf8_t*      theRendererPath,
                                   const int&           deviceId,
                                   const StNativeWin_t  theNativeParent) {
    return ((StOutAnaglyph* )inst)->init(StString(theRendererPath), deviceId, theNativeParent); }
ST_EXPORT stBool_t StRenderer_open(StRendererInterface* inst, const StOpenInfo_t* stOpenInfo) {
    return ((StOutAnaglyph* )inst)->open(StOpenInfo(stOpenInfo)); }
ST_EXPORT void StRenderer_callback(StRendererInterface* inst, StMessage_t* stMessages) {
    ((StOutAnaglyph* )inst)->callback(stMessages); }
ST_EXPORT void StRenderer_stglDraw(StRendererInterface* inst, unsigned int views) {
    ((StOutAnaglyph* )inst)->stglDraw(views); }
