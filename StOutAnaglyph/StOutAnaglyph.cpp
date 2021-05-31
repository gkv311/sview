/**
 * StOutAnaglyph, class providing stereoscopic output in Anaglyph format using StCore toolkit.
 * Copyright © 2007-2016 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#include "StOutAnaglyph.h"

#include <StGL/StGLEnums.h>
#include <StGL/StGLContext.h>
#include <StGLCore/StGLCore20.h>
#include <StSettings/StSettings.h>
#include <StSettings/StTranslations.h>
#include <StSettings/StEnumParam.h>
#include <StVersion.h>

namespace {
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

    static const char ST_SETTING_WINDOWPOS[] = "windowPos";

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

}

StAtomic<int32_t> StOutAnaglyph::myInstancesNb(0);

StString StOutAnaglyph::getRendererAbout() const {
    return myAbout;
}

const char* StOutAnaglyph::getRendererId() const {
    return ST_OUT_PLUGIN_NAME;
}

const char* StOutAnaglyph::getDeviceId() const {
    return "Anaglyph";
}

void StOutAnaglyph::getDevices(StOutDevicesList& theList) const {
    for(size_t anIter = 0; anIter < myDevices.size(); ++anIter) {
        theList.add(myDevices[anIter]);
    }
}

void StOutAnaglyph::getOptions(StParamsList& theList) const {
    theList.add(params.Glasses);
    theList.add(params.RedCyan);
    theList.add(params.AmberBlue);
}

void StOutAnaglyph::updateStrings() {
    StTranslations aLangMap(getResourceManager(), ST_OUT_PLUGIN_NAME);

    myDevices[DEVICE_ANAGLYPH]->Name = aLangMap.changeValueId(STTR_ANAGLYPH_NAME, "Anaglyph glasses");
    myDevices[DEVICE_ANAGLYPH]->Desc = aLangMap.changeValueId(STTR_ANAGLYPH_DESC, "Simple glasses with color-filters");

    params.Glasses->setName(aLangMap.changeValueId(STTR_ANAGLYPH_GLASSES, "Glasses type"));
    params.Glasses->defineOption(GLASSES_TYPE_REDCYAN, aLangMap.changeValueId(STTR_ANAGLYPH_REDCYAN, "Red-cyan"));
    params.Glasses->defineOption(GLASSES_TYPE_YELLOW,  aLangMap.changeValueId(STTR_ANAGLYPH_YELLOW,  "Yellow-Blue"));
    params.Glasses->defineOption(GLASSES_TYPE_GREEN,   aLangMap.changeValueId(STTR_ANAGLYPH_GREEN,   "Green-Magenta"));

    params.RedCyan->setName(aLangMap.changeValueId(STTR_ANAGLYPH_REDCYAN_MENU, "Red-Cyan filter"));
    params.RedCyan->defineOption(REDCYAN_MODE_SIMPLE, aLangMap.changeValueId(STTR_ANAGLYPH_REDCYAN_SIMPLE, "Simple"));
    params.RedCyan->defineOption(REDCYAN_MODE_OPTIM,  aLangMap.changeValueId(STTR_ANAGLYPH_REDCYAN_OPTIM,  "Optimized"));
    params.RedCyan->defineOption(REDCYAN_MODE_GRAY,   aLangMap.changeValueId(STTR_ANAGLYPH_REDCYAN_GRAY,   "Grayed"));
    params.RedCyan->defineOption(REDCYAN_MODE_DARK,   aLangMap.changeValueId(STTR_ANAGLYPH_REDCYAN_DARK,   "Dark"));

    params.AmberBlue->setName(aLangMap.changeValueId(STTR_ANAGLYPH_AMBERBLUE_MENU, "Yellow filter"));
    params.AmberBlue->defineOption(AMBERBLUE_MODE_SIMPLE, aLangMap.changeValueId(STTR_ANAGLYPH_AMBERBLUE_SIMPLE, "Simple"));
    params.AmberBlue->defineOption(AMBERBLUE_MODE_DUBOIS, aLangMap.changeValueId(STTR_ANAGLYPH_AMBERBLUE_DUBIOS, "Dubios"));

    // about string
    StString& aTitle     = aLangMap.changeValueId(STTR_PLUGIN_TITLE,   "sView - Anaglyph Output module");
    StString& aVerString = aLangMap.changeValueId(STTR_VERSION_STRING, "version");
    StString& aDescr     = aLangMap.changeValueId(STTR_PLUGIN_DESCRIPTION,
        "(C) {0} Kirill Gavrilov <{1}>\nOfficial site: {2}\n\nThis library is distributed under LGPL3.0");
    myAbout = aTitle + '\n' + aVerString + " " + StVersionInfo::getSDKVersionString() + "\n \n"
            + aDescr.format("2007-2020", "kirill@sview.ru", "www.sview.ru");
}

StOutAnaglyph::StOutAnaglyph(const StHandle<StResourceManager>& theResMgr,
                             const StNativeWin_t                theParentWindow)
: StWindow(theResMgr, theParentWindow),
  mySettings(new StSettings(theResMgr, ST_OUT_PLUGIN_NAME)),
  myFrBuffer(new StGLStereoFrameBuffer()),
  myStereoProgram(NULL),
  mySimpleAnaglyph("Anaglyph Simple"),
  myGrayAnaglyph("Anaglyph Gray"),
  myTrueAnaglyph("Anaglyph True"),
  myOptimAnaglyph("Anaglyph Optimized"),
  myYellowAnaglyph("Anaglyph Yellow"),
  myYellowDubiosAnaglyph("Anaglyph Yellow Dubios"),
  myGreenAnaglyph("Anaglyph Green"),
  myToCompressMem(myInstancesNb.increment() > 1),
  myIsBroken(false) {
    myStereoProgram = &mySimpleAnaglyph;

    // devices list
    StHandle<StOutDevice> aDevice = new StOutDevice();
    aDevice->PluginId = ST_OUT_PLUGIN_NAME;
    aDevice->DeviceId = stCString("Anaglyph");
    aDevice->Priority = ST_DEVICE_SUPPORT_LOW; // anaglyph could be run on every display...
    aDevice->Name     = stCString("Anaglyph glasses");
    myDevices.add(aDevice);

    // Glasses switch option
    params.Glasses = new StEnumParam(GLASSES_TYPE_REDCYAN, stCString("glasses"), stCString("glasses"));
    params.Glasses->signals.onChanged.connect(this, &StOutAnaglyph::doSetShader);

    // Red-cyan filter switch option
    params.RedCyan = new StEnumParam(REDCYAN_MODE_SIMPLE, stCString("optionRedCyan"), stCString("optionRedCyan"));
    params.RedCyan->signals.onChanged.connect(this, &StOutAnaglyph::doSetShader);

    // Amber-Blue filter switch option
    params.AmberBlue = new StEnumParam(AMBERBLUE_MODE_SIMPLE, stCString("optionAmberBlue"), stCString("optionAmberBlue"));
    params.AmberBlue->signals.onChanged.connect(this, &StOutAnaglyph::doSetShader);

    // load window position
    if(isMovable()) {
        StRect<int32_t> aRect;
        if(!mySettings->loadInt32Rect(ST_SETTING_WINDOWPOS, aRect)) {
            aRect = defaultRect();
        }
        StWindow::setPlacement(aRect, true);
    }
    updateStrings();
    StWindow::setTitle("sView - Anaglyph Renderer");

    // load glasses settings
    mySettings->loadParam(params.Glasses);
    mySettings->loadParam(params.RedCyan);
    mySettings->loadParam(params.AmberBlue);
}

void StOutAnaglyph::releaseResources() {
    if(!myContext.isNull()) {
        StGLContext& aCtx = *myContext;
        mySimpleAnaglyph.release(aCtx);
        myGrayAnaglyph.release(aCtx);
        myTrueAnaglyph.release(aCtx);
        myOptimAnaglyph.release(aCtx);
        myYellowAnaglyph.release(aCtx);
        myYellowDubiosAnaglyph.release(aCtx);
        myGreenAnaglyph.release(aCtx);
        myFrBuffer->release(aCtx);
    }
    myContext.nullify();

    // read windowed placement
    StWindow::hide(ST_WIN_MASTER);
    if(isMovable()) {
        StWindow::setFullScreen(false);
    }
}

void StOutAnaglyph::beforeClose() {
    if(isMovable() && myWasUsed) {
        mySettings->saveInt32Rect(ST_SETTING_WINDOWPOS, StWindow::getWindowedPlacement());
    }
    mySettings->saveParam(params.Glasses);
    mySettings->saveParam(params.RedCyan);
    mySettings->saveParam(params.AmberBlue);
    mySettings->flush();
}

StOutAnaglyph::~StOutAnaglyph() {
    myInstancesNb.decrement();
    releaseResources();
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

void StOutAnaglyph::close() {
    StWindow::params.VSyncMode->signals.onChanged -= stSlot(this, &StOutAnaglyph::doSwitchVSync);
    beforeClose();
    releaseResources();
    StWindow::close();
}

bool StOutAnaglyph::create() {
    StWindow::show();
    if(!StWindow::create()) {
        return false;
    }

    // initialize GL context
    myContext = StWindow::getContext();
    myContext->setMessagesQueue(myMsgQueue);
    if(!myContext->isGlGreaterEqual(2, 0)) {
        myMsgQueue->pushError(stCString("OpenGL 2.0 is required by Anaglyph Output"));
        myIsBroken = true;
        return true;
    }

    myContext->stglSetVSync((StGLContext::VSync_Mode )StWindow::params.VSyncMode->getValue());
    StWindow::params.VSyncMode->signals.onChanged += stSlot(this, &StOutAnaglyph::doSwitchVSync);

    // INIT shaders
    const StString aShadersRoot = StString("shaders" ST_FILE_SPLITTER) + ST_OUT_PLUGIN_NAME + SYS_FS_SPLITTER;
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
        myMsgQueue->pushError(stCString("Anaglyph output - critical error:\nShaders initialization failed!"));
        myIsBroken = true;
        return true;
    }

    aVertShader.release(*myContext);
    myIsBroken = false;
    return true;
}

void StOutAnaglyph::processEvents() {
    StWindow::processEvents();

    // don't care about holded key - StParam calls it's callbacks only on value change
    const StKeysState& aKeys = StWindow::getKeysState();
    if(aKeys.isKeyDown(ST_VK_F1)) {
        params.Glasses->setValue(GLASSES_TYPE_REDCYAN);
        params.RedCyan->setValue(REDCYAN_MODE_SIMPLE);
    } else if(aKeys.isKeyDown(ST_VK_F2)) {
        params.Glasses->setValue(GLASSES_TYPE_REDCYAN);
        params.RedCyan->setValue(REDCYAN_MODE_OPTIM);
    } else if(aKeys.isKeyDown(ST_VK_F3)) {
        params.Glasses->setValue(GLASSES_TYPE_REDCYAN);
        params.RedCyan->setValue(REDCYAN_MODE_GRAY);
    } else if(aKeys.isKeyDown(ST_VK_F4)) {
        params.Glasses->setValue(GLASSES_TYPE_REDCYAN);
        params.RedCyan->setValue(REDCYAN_MODE_DARK);
    } else if(aKeys.isKeyDown(ST_VK_F5)) {
        params.Glasses->setValue(GLASSES_TYPE_YELLOW);
    } else if(aKeys.isKeyDown(ST_VK_F6)) {
        params.Glasses->setValue(GLASSES_TYPE_GREEN);
    }
}

void StOutAnaglyph::stglDraw() {
    if(!StWindow::stglMakeCurrent(ST_WIN_MASTER)) {
        StWindow::signals.onRedraw(ST_DRAW_MONO);
        StThread::sleep(10);
        return;
    }

    myFPSControl.setTargetFPS(StWindow::getTargetFps());
    const StGLBoxPx aVPort = StWindow::stglViewport(ST_WIN_MASTER);
    if(!StWindow::isStereoOutput() || myIsBroken) {
        myContext->stglResizeViewport(aVPort);
        if(myToCompressMem) {
            myFrBuffer->release(*myContext);
        }

        StWindow::signals.onRedraw(ST_DRAW_LEFT);

        myFPSControl.sleepToTarget(); // decrease FPS to target by thread sleeps
        StWindow::stglSwap(ST_WIN_MASTER);
        ++myFPSControl;
        return;
    }

    // resize FBO
    if(!myFrBuffer->initLazy(*myContext, aVPort.width(), aVPort.height(), StWindow::hasDepthBuffer())) {
        myMsgQueue->pushError(stCString("Anaglyph output - critical error:\nFrame Buffer Object resize failed!"));
        myIsBroken = true;
        return;
    }

    // draw into virtual frame buffers (textures)
    myFrBuffer->setupViewPort(*myContext);       // we set TEXTURE sizes here
    myFrBuffer->bindBufferLeft(*myContext);
        StWindow::signals.onRedraw(ST_DRAW_LEFT);
    myFrBuffer->bindBufferRight(*myContext);
        StWindow::signals.onRedraw(ST_DRAW_RIGHT);
    myFrBuffer->unbindBufferRight(*myContext);

    // now draw to real screen buffer
    // clear the screen and the depth buffer
    myContext->stglResizeViewport(aVPort);
    myContext->core20fwd->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    myContext->core20fwd->glDisable(GL_DEPTH_TEST);
    myContext->core20fwd->glDisable(GL_BLEND);
    myFrBuffer->bindMultiTexture(*myContext);
    myFrBuffer->drawQuad(*myContext, myStereoProgram);
    myFrBuffer->unbindMultiTexture(*myContext);

    myFPSControl.sleepToTarget(); // decrease FPS to target by thread sleeps
    StWindow::stglSwap(ST_WIN_MASTER);
    ++myFPSControl;
}

void StOutAnaglyph::doSetShader(const int32_t ) {
    switch(params.Glasses->getValue()) {
        case GLASSES_TYPE_REDCYAN: {
            switch(params.RedCyan->getValue()) {
                case REDCYAN_MODE_OPTIM:  myStereoProgram = &myOptimAnaglyph; break;
                case REDCYAN_MODE_GRAY:   myStereoProgram = &myGrayAnaglyph;  break;
                case REDCYAN_MODE_DARK:   myStereoProgram = &myTrueAnaglyph;  break;
                case REDCYAN_MODE_SIMPLE:
                default: myStereoProgram = &mySimpleAnaglyph; break;
            }
            break;
        }
        case GLASSES_TYPE_YELLOW: {
            switch(params.AmberBlue->getValue()) {
                case AMBERBLUE_MODE_DUBOIS: myStereoProgram = &myYellowDubiosAnaglyph; break;
                case AMBERBLUE_MODE_SIMPLE:
                default: myStereoProgram = &myYellowAnaglyph; break;
            }
            break;
        }
        case GLASSES_TYPE_GREEN: myStereoProgram = &myGreenAnaglyph; break;
    }
}

void StOutAnaglyph::doSwitchVSync(const int32_t theValue) {
    if(myContext.isNull()) {
        return;
    }

    StWindow::stglMakeCurrent(ST_WIN_MASTER);
    myContext->stglSetVSync((StGLContext::VSync_Mode )theValue);
}
