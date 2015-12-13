/**
 * Copyright © 2007-2015 Kirill Gavrilov <kirill@sview.ru>
 *
 * StImageViewer program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * StImageViewer program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program.
 * If not, see <http://www.gnu.org/licenses/>.
 */

#include "StImageViewer.h"

#include "StImagePluginInfo.h"
#include "StImageViewerStrings.h"

#include <StGL/StGLContext.h>
#include <StGLCore/StGLCore20.h>
#include <StGLWidgets/StGLButton.h>
#include <StGLWidgets/StGLCheckbox.h>
#include <StGLWidgets/StGLTextureButton.h>
#include <StGLWidgets/StGLImageRegion.h>
#include <StGLWidgets/StGLMessageBox.h>
#include <StGLWidgets/StGLMsgStack.h>
#include <StGLWidgets/StGLPlayList.h>
#include <StSettings/StSettings.h>
#include <StSocket/StCheckUpdates.h>
#include <StThreads/StThread.h>
#include <StImage/StImageFile.h>
#include <StCore/StSearchMonitors.h>

#include "../StOutAnaglyph/StOutAnaglyph.h"
#include "../StOutDual/StOutDual.h"
#include "../StOutIZ3D/StOutIZ3D.h"
#include "../StOutInterlace/StOutInterlace.h"
#include "../StOutPageFlip/StOutPageFlipExt.h"
#include "../StOutDistorted/StOutDistorted.h"

#include <cstdlib> // std::abs(int)

const char* StImageViewer::ST_DRAWER_PLUGIN_NAME = "StImageViewer";

namespace {

    static const char ST_SETTING_SLIDESHOW_DELAY[] = "slideShowDelay";
    static const char ST_SETTING_FPSTARGET[]   = "fpsTarget";
    static const char ST_SETTING_SRCFORMAT[]   = "srcFormat";
    static const char ST_SETTING_LAST_FOLDER[] = "lastFolder";
    static const char ST_SETTING_RECENT_L[]    = "recentL";
    static const char ST_SETTING_RECENT_R[]    = "recentR";
    static const char ST_SETTING_SAVE_RECENT[] = "toSaveRecent";
    static const char ST_SETTING_SHOW_LIST[]   = "showPlaylist";
    static const char ST_SETTING_COMPRESS[]    = "toCompress";
    static const char ST_SETTING_ESCAPENOQUIT[]= "escNoQuit";
    static const char ST_SETTING_FULLSCREENUI[]= "fullScreenUI";

    static const char ST_SETTING_SCALE_ADJUST[]  = "scaleAdjust";
    static const char ST_SETTING_SCALE_FORCE2X[] = "scale2X";
    static const char ST_SETTING_FULLSCREEN[]  = "fullscreen";
    static const char ST_SETTING_SLIDESHOW[]   = "slideshow";
    static const char ST_SETTING_TRACK_HEAD[]  = "toTrackHead";
    static const char ST_SETTING_SHOW_FPS[]    = "toShowFps";
    static const char ST_SETTING_MOBILE_UI[]   = "isMobileUI";
    static const char ST_SETTING_VSYNC[]       = "vsync";
    static const char ST_SETTING_VIEWMODE[]    = "viewMode";
    static const char ST_SETTING_STEREO_MODE[] = "viewStereoMode";
    static const char ST_SETTING_TEXFILTER[]   = "viewTexFilter";
    static const char ST_SETTING_GAMMA[]       = "viewGamma";
    static const char ST_SETTING_RATIO[]       = "ratio";
    static const char ST_SETTING_HEAL_ANAMORPHIC[]    = "toHealAnamorphic";
    static const char ST_SETTING_UPDATES_LAST_CHECK[] = "updatesLastCheck";
    static const char ST_SETTING_UPDATES_INTERVAL[]   = "updatesInterval";
    static const char ST_SETTING_IMAGELIB[]    = "imageLib";

    static const char ST_ARGUMENT_FILE_LEFT[]  = "left";
    static const char ST_ARGUMENT_FILE_RIGHT[] = "right";
    static const char ST_ARGUMENT_FILE_LAST[]  = "last";

    static const char ST_ARGUMENT_SHOW_MENU[]  = "toShowMenu";
    static const char ST_ARGUMENT_SHOW_TOPBAR[]= "toShowTopbar";
    static const char ST_ARGUMENT_MONITOR[]    = "monitorId";
    static const char ST_ARGUMENT_WINLEFT[]    = "windowLeft";
    static const char ST_ARGUMENT_WINTOP[]     = "windowTop";
    static const char ST_ARGUMENT_WINWIDTH[]   = "windowWidth";
    static const char ST_ARGUMENT_WINHEIGHT[]  = "windowHeight";

}

/**
 * Auxiliary class to create standard non-blocking open file dialog in dedicated thread.
 */
class StImageViewer::StOpenImage {

        public:

    enum DialogState {
        Dialog_Inactive,     //!< dialog is not opened
        Dialog_ActiveSingle, //!< dialog is opened and waiting for user input (one file)
        Dialog_ActiveDouble, //!< dialog is opened and waiting for user input (two files)
        Dialog_HasFiles,     //!< dialog has been closed and waiting for processing results
    };

        public:

    /**
     * Main constructor.
     */
    ST_LOCAL StOpenImage(StImageViewer* thePlugin)
    : myPlugin(thePlugin),
      myState(StOpenImage::Dialog_Inactive) {}

    /**
     * Destructor.
     */
    ST_LOCAL ~StOpenImage() {
        if(!myThread.isNull()) {
            myThread->wait();
        }
    }

    /**
     * Create open file dialog.
     */
    bool openDialog(const size_t theNbFiles) {
        StMutexAuto aLock(myMutex);
        if(myState != StOpenImage::Dialog_Inactive) {
            return false;
        }

        if(myPlugin->params.lastFolder.isEmpty()) {
            StHandle<StFileNode> aCurrFile = myPlugin->myLoader->getPlayList().getCurrentFile();
            if(!aCurrFile.isNull()) {
                myPlugin->params.lastFolder = aCurrFile->isEmpty() ? aCurrFile->getFolderPath() : aCurrFile->getValue(0)->getFolderPath();
            }
        }

        myFolder = myPlugin->params.lastFolder;
        myState  = theNbFiles == 2 ? StOpenImage::Dialog_ActiveDouble : StOpenImage::Dialog_ActiveSingle;
        myThread = new StThread(openDialogThread, this);
        return true;
    }

    /**
     * Return true for Dialog_HasFiles state.
     */
    ST_LOCAL bool hasResults() {
        StMutexAuto aLock(myMutex);
        return myState == StOpenImage::Dialog_HasFiles;
    }

    /**
     * Reset results.
     */
    ST_LOCAL void resetResults() {
        StMutexAuto aLock(myMutex);
        if(myState != StOpenImage::Dialog_HasFiles) {
            return;
        }

        if(!myThread.isNull()) {
            myThread->wait();
            myThread.nullify();
        }

        myState = Dialog_Inactive;
        myPathLeft .clear();
        myPathRight.clear();
    }

    /**
     * Return path to the left file.
     * Should NOT be called within Active state.
     */
    ST_LOCAL const StString& getPathLeft()  const { return myPathLeft; }

    /**
     * Set paths to open.
     */
    ST_LOCAL void setPaths(const StString& thePathLeft,
                           const StString& thePathRight) {
        StMutexAuto aLock(myMutex);
        if(myState != StOpenImage::Dialog_Inactive) {
            return;
        }

        myPathLeft  = thePathLeft;
        myPathRight = thePathRight;
        if(!myPathLeft.isEmpty()) {
            myState = StOpenImage::Dialog_HasFiles;
        }
    }

    /**
     * Return path to the right file.
     * Should NOT be called within Active state.
     */
    ST_LOCAL const StString& getPathRight() const { return myPathRight; }

        private:

    /**
     * Thread function wrapper.
     */
    static SV_THREAD_FUNCTION openDialogThread(void* theArg) {
        StOpenImage* aHandler = (StOpenImage* )theArg;
        aHandler->dialogLoop();
        return SV_THREAD_RETURN 0;
    }

    /**
     * Thread function.
     */
    ST_LOCAL void dialogLoop() {
        myPathLeft .clear();
        myPathRight.clear();
        StString aTitle = myPlugin->myLangMap->getValue(myState == StOpenImage::Dialog_ActiveDouble
                                                      ? StImageViewerStrings::DIALOG_OPEN_LEFT
                                                      : StImageViewerStrings::DIALOG_OPEN_FILE);

        StString aDummy;
        if(!StFileNode::openFileDialog(myFolder, aTitle, myPlugin->myLoader->getMimeList(), myPathLeft, false)) {
            StMutexAuto aLock(myMutex);
            myState = StOpenImage::Dialog_Inactive;
            return;
        } else if(myState == StOpenImage::Dialog_ActiveDouble) {
            aTitle = myPlugin->myLangMap->getValue(StImageViewerStrings::DIALOG_OPEN_RIGHT);
            StFileNode::getFolderAndFile(myPathLeft, myFolder, aDummy);
            if(!StFileNode::openFileDialog(myFolder, aTitle, myPlugin->myLoader->getMimeList(), myPathRight, false)) {
                StMutexAuto aLock(myMutex);
                myState = StOpenImage::Dialog_Inactive;
                return;
            }
        }

        StMutexAuto aLock(myMutex);
        myState = StOpenImage::Dialog_HasFiles;
    }

        private:

    StImageViewer*     myPlugin;
    StHandle<StThread> myThread;
    StMutex            myMutex;
    StString           myFolder;
    StString           myPathLeft;
    StString           myPathRight;
    DialogState        myState;

};

StImageViewer::StImageViewer(const StHandle<StResourceManager>& theResMgr,
                             const StNativeWin_t                theParentWin,
                             const StHandle<StOpenInfo>&        theOpenInfo,
                             const StString&                    theAppName)
: StApplication(theResMgr, theParentWin, theOpenInfo),
  myPlayList(new StPlayList(1, false)),
  myAppName(!theAppName.isEmpty() ? theAppName : ST_DRAWER_PLUGIN_NAME),
  myEventLoaded(false),
  //
  mySlideShowTimer(false),
  mySlideShowDelay(4.0),
  //
  myLastUpdateDay(0),
  myToCheckUpdates(true),
  myToRecreateMenu(false),
  myToSaveSrcFormat(false),
  myEscNoQuit(false),
  myToHideUIFullScr(false),
  myToCheckPoorOrient(true) {
    mySettings = new StSettings(myResMgr, myAppName);
    myLangMap  = new StTranslations(myResMgr, StImageViewer::ST_DRAWER_PLUGIN_NAME);
    myOpenDialog = new StOpenImage(this);
    StImageViewerStrings::loadDefaults(*myLangMap);

    myTitle = "sView - Image Viewer";
    if(!theAppName.isEmpty()) {
        myTitle = theAppName;
    }
    //
    params.isFullscreen = new StBoolParam(false);
    params.isFullscreen->signals.onChanged.connect(this, &StImageViewer::doFullscreen);
    params.toRestoreRatio   = new StBoolParam(false);
    params.ScaleAdjust      = new StInt32Param(StGLRootWidget::ScaleAdjust_Normal);
    mySettings->loadParam (ST_SETTING_SCALE_ADJUST, params.ScaleAdjust);
    params.ScaleAdjust->signals.onChanged = stSlot(this, &StImageViewer::doScaleGui);
    params.ScaleHiDPI       = new StFloat32Param(1.0f,       // initial value
                                                 0.5f, 3.0f, // min, max values
                                                 1.0f,       // default value
                                                 1.0f,       // incremental step
                                                 0.001f);    // equality tolerance
    params.ScaleHiDPI2X     = new StBoolParam(false);
    mySettings->loadParam (ST_SETTING_SCALE_FORCE2X, params.ScaleHiDPI2X);
    params.ScaleHiDPI2X->signals.onChanged = stSlot(this, &StImageViewer::doScaleHiDPI);
    params.checkUpdatesDays = new StInt32Param(7);
    params.srcFormat        = new StInt32Param(StFormat_AUTO);
    params.srcFormat->signals.onChanged.connect(this, &StImageViewer::doSwitchSrcFormat);
    params.ToShowPlayList   = new StBoolParam(false);
    params.ToShowPlayList->signals.onChanged = stSlot(this, &StImageViewer::doShowPlayList);
    params.ToTrackHead   = new StBoolParamNamed(true,  tr(StImageViewerStrings::MENU_VIEW_TRACK_HEAD));
    params.ToShowFps     = new StBoolParamNamed(false, tr(StImageViewerStrings::MENU_SHOW_FPS));
    params.ToShowMenu    = new StBoolParamNamed(true, "Show main menu");
    params.ToShowTopbar  = new StBoolParamNamed(true, "Show top toolbar");
    params.IsMobileUI = new StBoolParamNamed(StWindow::isMobile(), "Mobile UI");
    params.IsMobileUI->signals.onChanged = stSlot(this, &StImageViewer::doScaleHiDPI);
    params.IsVSyncOn  = new StBoolParam(true);
    params.IsVSyncOn->signals.onChanged = stSlot(this, &StImageViewer::doSwitchVSync);
    StApplication::params.VSyncMode->setValue(StGLContext::VSync_ON);
    params.ToSaveRecent = new StBoolParam(false);

    params.imageLib = StImageFile::ST_LIBAV,
    params.TargetFps = 0;

    mySettings->loadInt32 (ST_SETTING_FPSTARGET,          params.TargetFps);
    mySettings->loadString(ST_SETTING_LAST_FOLDER,        params.lastFolder);
    mySettings->loadInt32 (ST_SETTING_UPDATES_LAST_CHECK, myLastUpdateDay);
    mySettings->loadParam (ST_SETTING_UPDATES_INTERVAL,   params.checkUpdatesDays);
    myToCheckPoorOrient = !mySettings->loadParam(ST_SETTING_TRACK_HEAD, params.ToTrackHead);
    mySettings->loadParam (ST_SETTING_SHOW_FPS,           params.ToShowFps);
    mySettings->loadParam (ST_SETTING_MOBILE_UI,          params.IsMobileUI);
    mySettings->loadParam (ST_SETTING_VSYNC,              params.IsVSyncOn);
    mySettings->loadParam (ST_SETTING_SHOW_LIST,          params.ToShowPlayList);

    int32_t aSlideShowDelayInt = int32_t(mySlideShowDelay);
    mySettings->loadInt32 (ST_SETTING_SLIDESHOW_DELAY,    aSlideShowDelayInt);
    mySlideShowDelay = double(aSlideShowDelayInt);

#if defined(__ANDROID__)
    addRenderer(new StOutInterlace  (myResMgr, theParentWin));
    addRenderer(new StOutAnaglyph   (myResMgr, theParentWin));
    addRenderer(new StOutDistorted  (myResMgr, theParentWin));
#else
    addRenderer(new StOutAnaglyph   (myResMgr, theParentWin));
    addRenderer(new StOutDual       (myResMgr, theParentWin));
    addRenderer(new StOutIZ3D       (myResMgr, theParentWin));
    addRenderer(new StOutInterlace  (myResMgr, theParentWin));
    addRenderer(new StOutDistorted  (myResMgr, theParentWin));
    addRenderer(new StOutPageFlipExt(myResMgr, theParentWin));
#endif

    // no need in Depth buffer
    const StWinAttr anAttribs[] = {
        StWinAttr_GlDepthSize, (StWinAttr )0,
        StWinAttr_NULL
    };
    for(size_t aRendIter = 0; aRendIter < myRenderers.size(); ++aRendIter) {
        StHandle<StWindow>& aRend = myRenderers[aRendIter];
        aRend->setAttributes(anAttribs);
    }

    // create actions
    StHandle<StAction> anAction;
    anAction = new StActionBool(stCString("DoFullscreen"), params.isFullscreen);
    addAction(Action_Fullscreen, anAction, ST_VK_F, ST_VK_RETURN);

    anAction = new StActionBool(stCString("DoShowFPS"), params.ToShowFps);
    addAction(Action_ShowFps, anAction, ST_VK_F12);

    anAction = new StActionIntValue(stCString("DoSrcAuto"), params.srcFormat, StFormat_AUTO);
    addAction(Action_SrcAuto, anAction, ST_VK_A);

    anAction = new StActionIntValue(stCString("DoSrcMono"), params.srcFormat, StFormat_Mono);
    addAction(Action_SrcMono, anAction, ST_VK_M);

    anAction = new StActionIntValue(stCString("DoSrcOverUnder"), params.srcFormat, StFormat_TopBottom_LR);
    addAction(Action_SrcOverUnderLR, anAction, ST_VK_O);

    anAction = new StActionIntValue(stCString("DoSrcSideBySide"), params.srcFormat, StFormat_SideBySide_RL);
    addAction(Action_SrcSideBySideRL, anAction, ST_VK_S);

    anAction = new StActionIntSlot(stCString("DoFileInfo"), stSlot(this, &StImageViewer::doAboutImage), 0);
    addAction(Action_FileInfo, anAction, ST_VK_I);

    anAction = new StActionIntSlot(stCString("DoSaveFileInfo"), stSlot(this, &StImageViewer::doSaveImageInfoBegin), 0);
    addAction(Action_SaveFileInfo, anAction, ST_VK_I | ST_VF_SHIFT);

    anAction = new StActionIntSlot(stCString("DoListFirst"), stSlot(this, &StImageViewer::doListFirst), 0);
    addAction(Action_ListFirst, anAction, ST_VK_HOME);

    anAction = new StActionIntSlot(stCString("DoListLast"), stSlot(this, &StImageViewer::doListLast), 0);
    addAction(Action_ListLast, anAction, ST_VK_END);

    anAction = new StActionIntSlot(stCString("DoListPrev"), stSlot(this, &StImageViewer::doListPrev), 0);
    addAction(Action_ListPrev, anAction, ST_VK_PRIOR);

    anAction = new StActionIntSlot(stCString("DoListNext"), stSlot(this, &StImageViewer::doListNext), 0);
    addAction(Action_ListNext, anAction, ST_VK_NEXT);

    anAction = new StActionIntSlot(stCString("DoSlideShow"), stSlot(this, &StImageViewer::doSlideShow), 0);
    addAction(Action_SlideShow, anAction, ST_VK_SPACE);

    anAction = new StActionIntSlot(stCString("DoSaveImageAsPng"), stSlot(this, &StImageViewer::doSaveImageAs), StImageFile::ST_TYPE_PNG);
#ifdef __APPLE__
    addAction(Action_SavePng, anAction, ST_VK_S | ST_VF_CONTROL, ST_VK_S | ST_VF_COMMAND);
#else
    addAction(Action_SavePng, anAction, ST_VK_S | ST_VF_CONTROL);
#endif

    anAction = new StActionIntSlot(stCString("DoSaveImageAsJpeg"), stSlot(this, &StImageViewer::doSaveImageAs), StImageFile::ST_TYPE_JPEG);
    addAction(Action_SaveJpeg, anAction);

    anAction = new StActionIntSlot(stCString("DoDeleteFile"), stSlot(this, &StImageViewer::doDeleteFileBegin), 0);
    addAction(Action_DeleteFile, anAction, ST_VK_DELETE | ST_VF_SHIFT);

    anAction = new StActionIntSlot(stCString("DoImageAdjustReset"), stSlot(this, &StImageViewer::doImageAdjustReset), 0);
    addAction(Action_ImageAdjustReset, anAction);

    anAction = new StActionIntSlot(stCString("DoPanoramaOnOff"), stSlot(this, &StImageViewer::doPanoramaOnOff), 0);
    addAction(Action_PanoramaOnOff, anAction, ST_VK_P);
}

bool StImageViewer::resetDevice() {
    if(myGUI.isNull()
    || myLoader.isNull()) {
        return init();
    }

    // be sure Render plugin process quit correctly
    myWindow->beforeClose();

    releaseDevice();
    myWindow->close();
    myWindow.nullify();
    return open();
}

void StImageViewer::saveGuiParams() {
    if(myGUI.isNull()) {
        return;
    }

    mySettings->saveParam(ST_SETTING_STEREO_MODE, myGUI->myImage->params.displayMode);
    mySettings->saveInt32(ST_SETTING_GAMMA, stRound(100.0f * myGUI->myImage->params.gamma->getValue()));
    mySettings->saveParam(ST_SETTING_HEAL_ANAMORPHIC, myGUI->myImage->params.ToHealAnamorphicRatio);
    if(params.toRestoreRatio->getValue()) {
        mySettings->saveParam(ST_SETTING_RATIO, myGUI->myImage->params.displayRatio);
    } else {
        mySettings->saveInt32(ST_SETTING_RATIO, StGLImageRegion::RATIO_AUTO);
    }
    mySettings->saveParam(ST_SETTING_TEXFILTER, myGUI->myImage->params.textureFilter);
}

void StImageViewer::saveAllParams() {
    saveGuiParams();
    if(!myGUI.isNull()) {
        mySettings->saveParam (ST_SETTING_SCALE_ADJUST,  params.ScaleAdjust);
        mySettings->saveParam (ST_SETTING_SCALE_FORCE2X, params.ScaleHiDPI2X);
        mySettings->saveInt32(ST_SETTING_FPSTARGET, params.TargetFps);
        mySettings->saveInt32(ST_SETTING_SLIDESHOW_DELAY, int(mySlideShowDelay));
        mySettings->saveInt32(ST_SETTING_UPDATES_LAST_CHECK, myLastUpdateDay);
        mySettings->saveParam(ST_SETTING_UPDATES_INTERVAL, params.checkUpdatesDays);
        mySettings->saveString(ST_SETTING_IMAGELIB,  StImageFile::imgLibToString(params.imageLib));
        mySettings->saveParam (ST_SETTING_TRACK_HEAD,params.ToTrackHead);
        mySettings->saveParam (ST_SETTING_SHOW_FPS,  params.ToShowFps);
        mySettings->saveParam (ST_SETTING_MOBILE_UI, params.IsMobileUI);
        mySettings->saveParam (ST_SETTING_VSYNC,     params.IsVSyncOn);
        mySettings->saveParam (ST_SETTING_SHOW_LIST, params.ToShowPlayList);
        if(myToSaveSrcFormat) {
            mySettings->saveParam(ST_SETTING_SRCFORMAT, params.srcFormat);
        }

        // store hot-keys
        for(std::map< int, StHandle<StAction> >::iterator anIter = myActions.begin();
            anIter != myActions.end(); ++anIter) {
            mySettings->saveHotKey(anIter->second);
        }
    }

    StString aLastL, aLastR;
    StHandle<StFileNode> aFile = myLoader->getPlayList().getCurrentFile();
    if(params.ToSaveRecent->getValue()
    && !aFile.isNull()) {
        if(aFile->isEmpty()) {
            aLastL = aFile->getPath();
        } else if(aFile->size() == 2) {
            aLastL = aFile->getValue(0)->getPath();
            aLastR = aFile->getValue(1)->getPath();
        }
    }
    mySettings->saveString(ST_SETTING_RECENT_L, aLastL);
    mySettings->saveString(ST_SETTING_RECENT_R, aLastR);
    mySettings->flush();
}

void StImageViewer::releaseDevice() {
    saveAllParams();

    // release GUI data and GL resources before closing the window
    myKeyActions.clear();
    myGUI.nullify();
    myContext.nullify();
}

StImageViewer::~StImageViewer() {
    myUpdates.nullify();
    releaseDevice();
    // wait image loading thread to quit and release resources
    myLoader.nullify();
}

bool StImageViewer::createGui() {
    if(!myGUI.isNull()) {
        saveGuiParams();
        myGUI.nullify();
        myKeyActions.clear();
    }

    // create the GUI with default values
    params.ScaleHiDPI->setValue(myWindow->getScaleFactor());
    myGUI = new StImageViewerGUI(this, myWindow.access(), myLangMap.access(), myPlayList,
                                 myLoader.isNull() ? NULL : myLoader->getTextureQueue());
    myGUI->setContext(myContext);

    // load settings
    myWindow->setTargetFps(double(params.TargetFps));
    mySettings->loadParam (ST_SETTING_STEREO_MODE,        myGUI->myImage->params.displayMode);
    mySettings->loadParam (ST_SETTING_TEXFILTER,          myGUI->myImage->params.textureFilter);
    mySettings->loadParam (ST_SETTING_RATIO,              myGUI->myImage->params.displayRatio);
    mySettings->loadParam (ST_SETTING_HEAL_ANAMORPHIC,    myGUI->myImage->params.ToHealAnamorphicRatio);
    params.toRestoreRatio->setValue(myGUI->myImage->params.displayRatio->getValue() != StGLImageRegion::RATIO_AUTO);
    int32_t loadedGamma = 100; // 1.0f
        mySettings->loadInt32(ST_SETTING_GAMMA, loadedGamma);
        myGUI->myImage->params.gamma->setValue(0.01f * loadedGamma);

    // initialize frame region early to show dedicated error description
    if(!myGUI->myImage->stglInit()) {
        myMsgQueue->pushError(stCString("Image Viewer - critical error:\nFrame region initialization failed!"));
        myMsgQueue->popAll();
        myGUI.nullify();
        return false;
    }
    myGUI->stglInit();
    myGUI->stglResize(myWindow->stglViewport(ST_WIN_MASTER));

    for(size_t anIter = 0; anIter < myGUI->myImage->getActions().size(); ++anIter) {
        StHandle<StAction>& anAction = myGUI->myImage->changeActions()[anIter];
        mySettings->loadHotKey(anAction);
        addAction(Action_StereoParamsBegin + int(anIter), anAction);
    }
    registerHotKeys();
    return true;
}

bool StImageViewer::init() {
    const bool isReset = !myLoader.isNull();
    if(!myContext.isNull()
    && !myGUI.isNull()) {
        return true;
    }

    // initialize GL context
    myContext = myWindow->getContext();
    myContext->setMessagesQueue(myMsgQueue);
    if(!myContext->isGlGreaterEqual(2, 0)) {
        myMsgQueue->pushError(stCString("OpenGL 2.0 is required by Image Viewer!"));
        myMsgQueue->popAll();
        return false;
    }

    // load hot-keys
    if(!isReset) {
        for(std::map< int, StHandle<StAction> >::iterator anIter = myActions.begin();
            anIter != myActions.end(); ++anIter) {
            mySettings->loadHotKey(anIter->second);
        }
    }

    // create the GUI with default values
    if(!createGui()) {
        myMsgQueue->pushError(stCString("Image Viewer - critical error:\nFrame region initialization failed!"));
        myMsgQueue->popAll();
        myGUI.nullify();
        return false;
    }

    // create the image loader thread
    if(isReset) {
        return true;
    }

    StString anImgLibStr;
    mySettings->loadString(ST_SETTING_IMAGELIB, anImgLibStr);
    params.imageLib = StImageFile::imgLibFromString(anImgLibStr);
    myLoader = new StImageLoader(params.imageLib, myMsgQueue, myLangMap, myPlayList,
                                 myGUI->myImage->getTextureQueue(), myContext->getMaxTextureSize());
    myLoader->signals.onLoaded.connect(this, &StImageViewer::doLoaded);
    myLoader->setCompressMemory(myWindow->isMobile());

    // load this parameter AFTER image thread creation
    mySettings->loadParam(ST_SETTING_SRCFORMAT, params.srcFormat);

#if !defined(ST_NO_UPDATES_CHECK)
    // read the current time
    time_t aRawtime;
    time(&aRawtime);
    struct tm* aTimeinfo = localtime(&aRawtime);
    int32_t aCurrentDayInYear = aTimeinfo->tm_yday;
    if(params.checkUpdatesDays->getValue() > 0
    && std::abs(aCurrentDayInYear - myLastUpdateDay) > params.checkUpdatesDays->getValue()) {
        myUpdates = new StCheckUpdates();
        myUpdates->init();
        myLastUpdateDay = aCurrentDayInYear;
        mySettings->saveInt32(ST_SETTING_UPDATES_LAST_CHECK, myLastUpdateDay);
    }
#endif
    return true;
}

void StImageViewer::parseArguments(const StArgumentsMap& theArguments) {
    StArgument anArgSlideshow  = theArguments[ST_SETTING_SLIDESHOW];
    StArgument anArgViewMode   = theArguments[ST_SETTING_VIEWMODE];
    StArgument anArgSrcFormat  = theArguments[ST_SETTING_SRCFORMAT];
    StArgument anArgImgLibrary = theArguments[ST_SETTING_IMAGELIB];
    StArgument anArgToCompress = theArguments[ST_SETTING_COMPRESS];
    StArgument anArgEscNoQuit  = theArguments[ST_SETTING_ESCAPENOQUIT];
    StArgument anArgFullScreenUI = theArguments[ST_SETTING_FULLSCREENUI];
    StArgument anArgShowMenu   = theArguments[ST_ARGUMENT_SHOW_MENU];
    StArgument anArgShowTopbar = theArguments[ST_ARGUMENT_SHOW_TOPBAR];
    StArgument anArgSaveRecent = theArguments[ST_SETTING_SAVE_RECENT];

    StArgument anArgFullscreen = theArguments[ST_SETTING_FULLSCREEN];
    StArgument anArgMonitor    = theArguments[ST_ARGUMENT_MONITOR];
    StArgument anArgWinLeft    = theArguments[ST_ARGUMENT_WINLEFT];
    StArgument anArgWinTop     = theArguments[ST_ARGUMENT_WINTOP];
    StArgument anArgWinWidth   = theArguments[ST_ARGUMENT_WINWIDTH];
    StArgument anArgWinHeight  = theArguments[ST_ARGUMENT_WINHEIGHT];
    StRect<int32_t> aRect = myWindow->getWindowedPlacement();
    bool toSetRect = false;
    if(anArgMonitor.isValid()) {
        const size_t     aMonId  = (size_t )::atol(anArgMonitor.getValue().toCString());
        const StMonitor& aMonOld = myWindow->getMonitors()[aRect.center()];
        const StMonitor& aMonNew = myWindow->getMonitors()[aMonId];
        if(aMonOld.getId() != aMonNew.getId()) {
            const int aLeft = aRect.left() - aMonOld.getVRect().left();
            const int aTop  = aRect.top()  - aMonOld.getVRect().top();
            aRect.moveLeftTo(aMonNew.getVRect().left() + aLeft);
            aRect.moveTopTo (aMonNew.getVRect().top()  + aTop);
            toSetRect = true;
        }
    }
    if(anArgWinLeft.isValid()) {
        aRect.moveLeftTo(::atol(anArgWinLeft.getValue().toCString()));
        toSetRect = true;
    }
    if(anArgWinTop.isValid()) {
        aRect.moveTopTo(::atol(anArgWinTop.getValue().toCString()));
        toSetRect = true;
    }
    if(anArgWinWidth.isValid()) {
        aRect.right() = aRect.left() + ::atol(anArgWinWidth.getValue().toCString());
        toSetRect = true;
    }
    if(anArgWinHeight.isValid()) {
        aRect.bottom() = aRect.top() + ::atol(anArgWinHeight.getValue().toCString());
        toSetRect = true;
    }
    if(toSetRect) {
        myWindow->setPlacement(aRect, true);
    }
    if(anArgFullscreen.isValid()) {
        params.isFullscreen->setValue(!anArgFullscreen.isValueOff());
    }

    if(anArgToCompress.isValid()) {
        myLoader->setCompressMemory(!anArgToCompress.isValueOff());
    }
    if(anArgEscNoQuit.isValid()) {
        myEscNoQuit = !anArgEscNoQuit.isValueOff();
    }
    if(anArgFullScreenUI.isValid()) {
        myToHideUIFullScr = anArgFullScreenUI.isValueOff();
    }
    if(anArgShowMenu.isValid()) {
        params.ToShowMenu->setValue(!anArgShowMenu.isValueOff());
    }
    if(anArgShowTopbar.isValid()) {
        params.ToShowTopbar->setValue(!anArgShowTopbar.isValueOff());
    }
    if( anArgSlideshow.isValid()
    && !anArgSlideshow.isValueOff()) {
        doSlideShow();
    }
    if(anArgViewMode.isValid()) {
        myLoader->getPlayList().changeDefParams().ViewingMode = StStereoParams::GET_VIEW_MODE_FROM_STRING(anArgViewMode.getValue());
    }
    if(anArgSrcFormat.isValid()) {
        params.srcFormat->setValue(st::formatFromString(anArgSrcFormat.getValue()));
        myToSaveSrcFormat = false; // this setting is temporary!
    }
    if(anArgImgLibrary.isValid()) {
        params.imageLib = StImageFile::imgLibFromString(anArgImgLibrary.getValue());
        myLoader->setImageLib(params.imageLib);
    }
    if(anArgSaveRecent.isValid()) {
        params.ToSaveRecent->setValue(!anArgSaveRecent.isValueOff());
    }
}

bool StImageViewer::open() {
    const bool isReset = !mySwitchTo.isNull();
    if(!StApplication::open()
    || !init()) {
        myMsgQueue->popAll();
        return false;
    }

    // disable head-tracking by default for poor sensors
    if(myToCheckPoorOrient
    && myWindow->isPoorOrientationSensor()) {
        myToCheckPoorOrient = false;
        params.ToTrackHead->setValue(false);
    }

    if(isReset) {
        myLoader->doLoadNext();
        return true;
    }

    parseArguments(myOpenFileInfo->getArgumentsMap());
    const StMIME anOpenMIME = myOpenFileInfo->getMIME();
    if(myOpenFileInfo->getPath().isEmpty()) {
        const StArgument anArgLast = myOpenFileInfo->getArgumentsMap()[ST_ARGUMENT_FILE_LAST];
        if(anArgLast.isValid() && !anArgLast.isValueOff()) {
            StString aLastL, aLastR;
            mySettings->loadString(ST_SETTING_RECENT_L, aLastL);
            mySettings->loadString(ST_SETTING_RECENT_R, aLastR);
            if(!aLastL.isEmpty()) {
                if(!aLastR.isEmpty()) {
                    myLoader->getPlayList().clear();
                    myLoader->getPlayList().addOneFile(aLastL, aLastR);
                } else {
                    myLoader->getPlayList().open(aLastL);
                }

                if(!myLoader->getPlayList().isEmpty()) {
                    doUpdateStateLoading();
                    myLoader->doLoadNext();
                }
                return true;
            }
        }

        // open drawer without files
        return true;
    }

    // clear playlist first
    myLoader->getPlayList().clear();

    //StArgument argFile1     = myOpenFileInfo->getArgumentsMap()[ST_ARGUMENT_FILE + 1]; // playlist?
    StArgument argFileLeft  = myOpenFileInfo->getArgumentsMap()[ST_ARGUMENT_FILE_LEFT];
    StArgument argFileRight = myOpenFileInfo->getArgumentsMap()[ST_ARGUMENT_FILE_RIGHT];
    if(argFileLeft.isValid() && argFileRight.isValid()) {
        // meta-file
        /// TODO (Kirill Gavrilov#4) we should use MIME type!
        myLoader->getPlayList().addOneFile(argFileLeft.getValue(), argFileRight.getValue());
    } else if(!anOpenMIME.isEmpty()) {
        // create just one-file playlist
        myLoader->getPlayList().addOneFile(myOpenFileInfo->getPath(), anOpenMIME);
    } else {
        // create playlist from file's folder
        myLoader->getPlayList().open(myOpenFileInfo->getPath());
    }

    if(!myLoader->getPlayList().isEmpty()) {
        doUpdateStateLoading();
        myLoader->doLoadNext();
    }
    return true;
}

void StImageViewer::doChangeDevice(const int32_t theValue) {
    StApplication::doChangeDevice(theValue);
    // update menu
}

void StImageViewer::doPause(const StPauseEvent& theEvent) {
    StApplication::doPause(theEvent);
    saveAllParams();
}

void StImageViewer::doResize(const StSizeEvent& ) {
    if(myGUI.isNull()) {
        return;
    }

    myGUI->stglResize(myWindow->stglViewport(ST_WIN_MASTER));
}

void StImageViewer::doImageAdjustReset(const size_t ) {
    if(myGUI.isNull()) {
        return;
    }

    myGUI->myImage->params.gamma     ->reset();
    myGUI->myImage->params.brightness->reset();
    myGUI->myImage->params.saturation->reset();
}

void StImageViewer::doSaveImageInfoBegin(const size_t ) {
    if(!myFileInfo.isNull()) {
        return; // already opened
    }

    StHandle<StFileNode>     aFileNode;
    StHandle<StStereoParams> aParams;
    if(!getCurrentFile(aFileNode, aParams, myFileInfo)
    ||  myFileInfo.isNull()) {
        myMsgQueue->pushInfo(myLangMap->getValue(StImageViewerStrings::DIALOG_FILE_NOINFO));
        myFileInfo.nullify();
        return;
    } else if(!myFileInfo->IsSavable) {
        myMsgQueue->pushInfo(myLangMap->getValue(StImageViewerStrings::DIALOG_SAVE_INFO_UNSUPPORTED));
        myFileInfo.nullify();
        return;
    }

    const StString aText = myLangMap->getValue(StImageViewerStrings::DIALOG_SAVE_INFO_QUESTION)
                         + "\n" + myFileInfo->Path;

    StInfoDialog* aDialog = new StInfoDialog(this, myGUI.access(), myLangMap->getValue(StImageViewerStrings::DIALOG_SAVE_INFO_TITLE),
                                             myGUI->scale(512), myGUI->scale(256));
    aDialog->setText(aText);

    StHandle<StBoolParam> toOmitQuestion = new StBoolParam(false);
    new StGLCheckbox(aDialog, toOmitQuestion, 32, -aDialog->getMarginBottom(), StGLCorner(ST_VCORNER_BOTTOM, ST_HCORNER_LEFT));

    StGLTextArea* aCheckText = new StGLTextArea(aDialog, 64, -aDialog->getMarginBottom(), StGLCorner(ST_VCORNER_BOTTOM, ST_HCORNER_LEFT));
    aCheckText->setText("Do not ask again during 1 minute");
    aCheckText->setTextColor(StGLVec3(1.0f, 1.0f, 1.0f));

    StGLButton* aSaveBtn = aDialog->addButton(myLangMap->getValue(StImageViewerStrings::BUTTON_SAVE_METADATA), true);
    aSaveBtn->setUserData(1);
    aSaveBtn->signals.onBtnClick += stSlot(this, &StImageViewer::doSaveImageInfo);

    aDialog->addButton(myLangMap->getValue(StImageViewerStrings::BUTTON_CANCEL));
    aDialog->stglInit();
}

void StImageViewer::doDeleteFileBegin(const size_t ) {
    //if(!myFileToDelete.isNull()) {
    //    return;
    //}

    myFileToDelete = myLoader->getPlayList().getCurrentFile();
    if(myFileToDelete.isNull()
    || myFileToDelete->size() != 0) {
        myFileToDelete.nullify();
        return;
    }

    const StString aText = myLangMap->getValue(StImageViewerStrings::DIALOG_DELETE_FILE_QUESTION)
                         + "\n" + myFileToDelete->getPath();

    StGLMessageBox* aDialog = new StGLMessageBox(myGUI.access(), myLangMap->getValue(StImageViewerStrings::DIALOG_DELETE_FILE_TITLE),
                                                 aText, myGUI->scale(512), myGUI->scale(256));
    aDialog->addButton(myLangMap->getValue(StImageViewerStrings::BUTTON_DELETE), true)->signals.onBtnClick += stSlot(this, &StImageViewer::doDeleteFileEnd);
    aDialog->addButton(myLangMap->getValue(StImageViewerStrings::BUTTON_CANCEL), false);
    aDialog->stglInit();
}

void StImageViewer::doDeleteFileEnd(const size_t ) {
    if(myFileToDelete.isNull()
    || myLoader.isNull()) {
        return;
    }

    myLoader->getPlayList().removePhysically(myFileToDelete);
    if(!myLoader->getPlayList().isEmpty()) {
        doUpdateStateLoading();
        myLoader->doLoadNext();
    }
    myFileToDelete.nullify();
}

void StImageViewer::doKeyDown(const StKeyEvent& theEvent) {
    if(myGUI.isNull()) {
        return;
    }

    myGUI->myImage->doKeyDown(theEvent);
    if(myGUI->getFocus() != NULL) {
        myGUI->doKeyDown(theEvent);
        return;
    }

    StApplication::doKeyDown(theEvent);
    switch(theEvent.VKey) {
        case ST_VK_ESCAPE: {
            if(!myEscNoQuit) {
                StApplication::exit(0);
            } else if(myWindow->isFullScreen()) {
                params.isFullscreen->setValue(false);
            }
            return;
        }

        // file walk
        case ST_VK_MEDIA_PREV_TRACK:
        case ST_VK_BROWSER_BACK:
            doListPrev();
            return;
        case ST_VK_MEDIA_NEXT_TRACK:
        case ST_VK_BROWSER_FORWARD:
            doListNext();
            return;
        case ST_VK_O: {
            if(theEvent.Flags == ST_VF_CONTROL
            || theEvent.Flags == ST_VF_COMMAND) {
                myOpenDialog->openDialog(1);
            }
            return;
        }

        // post process keys
        case ST_VK_B: {
            if(theEvent.Flags == ST_VF_SHIFT) {
                myGUI->myImage->params.brightness->increment();
            } else if(theEvent.Flags == ST_VF_CONTROL) {
                myGUI->myImage->params.brightness->decrement();
            }
            return;
        }
        case ST_VK_T: {
            /// TODO (Kirill Gavrilov#9) remove this hot key
            if(theEvent.Flags == ST_VF_SHIFT) {
                myGUI->myImage->params.saturation->increment();
            } else if(theEvent.Flags == ST_VF_CONTROL) {
                myGUI->myImage->params.saturation->decrement();
            }
            return;
        }
        default: break;
    }
}

void StImageViewer::doKeyHold(const StKeyEvent& theEvent) {
    if(myGUI.isNull()) {
        return;
    }

    if(myGUI->getFocus() != NULL) {
        myGUI->doKeyHold(theEvent);
    } else {
        StApplication::doKeyHold(theEvent);
    }
}

void StImageViewer::doKeyUp(const StKeyEvent& theEvent) {
    if(myGUI.isNull()) {
        return;
    }

    myGUI->myImage->doKeyUp(theEvent);
    if(myGUI->getFocus() != NULL) {
        myGUI->doKeyUp(theEvent);
    }
}

void StImageViewer::doMouseDown(const StClickEvent& theEvent) {
    if(myGUI.isNull()) {
        return;
    }

    myGUI->tryClick(StPointD_t(theEvent.PointX, theEvent.PointY), theEvent.Button);
}

void StImageViewer::doMouseUp(const StClickEvent& theEvent) {
    if(myGUI.isNull()) {
        return;
    }

    if(theEvent.Button == ST_MOUSE_MIDDLE) {
        params.isFullscreen->reverse();
    }
    myGUI->tryUnClick(StPointD_t(theEvent.PointX, theEvent.PointY), theEvent.Button);
}

void StImageViewer::doScroll(const StScrollEvent& theEvent) {
    if(myGUI.isNull()) {
        return;
    }

    if(myEscNoQuit
    && !myWindow->isFullScreen()) {
        return; // ignore scrolling
    }

    myGUI->doScroll(theEvent);
}

void StImageViewer::doFileDrop(const StDNDropEvent& theEvent) {
    if(theEvent.NbFiles == 0) {
        return;
    }

    const StString aFile1 = theEvent.Files[0];
    if(!myLoader->getPlayList().checkExtension(aFile1)) {
        return;
    } else if(theEvent.NbFiles == 1) {
        myLoader->getPlayList().open(aFile1);
        doUpdateStateLoading();
        myLoader->doLoadNext();
        return;
    } else if(theEvent.NbFiles == 2
          && !StFolder::isFolder(aFile1)
          && !StFolder::isFolder(StString(theEvent.Files[1]))) {
        myLoader->getPlayList().clear();
        myLoader->getPlayList().addOneFile(aFile1, StString(theEvent.Files[1]));
        doUpdateStateLoading();
        myLoader->doLoadNext();
        return;
    }

    myLoader->getPlayList().clear();
    for(uint32_t aFileIter = 0; aFileIter < theEvent.NbFiles; ++aFileIter) {
        StString aPath(theEvent.Files[aFileIter]);
        if(!StFolder::isFolder(aPath)) {
            myLoader->getPlayList().addOneFile(aPath, StMIME());
        }
    }

    doUpdateStateLoading();
    myLoader->doLoadNext();
}

void StImageViewer::doNavigate(const StNavigEvent& theEvent) {
    switch(theEvent.Target) {
        case stNavigate_Backward: doListPrev(); break;
        case stNavigate_Forward:  doListNext(); break;
        default: break;
    }
}

void StImageViewer::beforeDraw() {
    if(myGUI.isNull()) {
        return;
    }

    if(myOpenDialog->hasResults()) {
        if(!myOpenDialog->getPathRight().isEmpty()) {
            // meta-file
            myLoader->getPlayList().clear();
            myLoader->getPlayList().addOneFile(myOpenDialog->getPathLeft(), myOpenDialog->getPathRight());
        } else {
            myLoader->getPlayList().open(myOpenDialog->getPathLeft());
        }

        doUpdateStateLoading();
        myLoader->doLoadNext();

        StString aDummy;
        StFileNode::getFolderAndFile(myOpenDialog->getPathLeft(), params.lastFolder, aDummy);
        if(!params.lastFolder.isEmpty()) {
            mySettings->saveString(ST_SETTING_LAST_FOLDER, params.lastFolder);
        }
        myOpenDialog->resetResults();
    }

    // recreate menu event
    if(params.ScaleHiDPI->setValue(myWindow->getScaleFactor())
    || myToRecreateMenu) {
        createGui();
        myToRecreateMenu = false;
        myLoader->doLoadNext();
    }

    if(mySlideShowTimer.getElapsedTimeInSec() > mySlideShowDelay) {
        mySlideShowTimer.restart();
        doListNext();
    }

    if(myEventLoaded.checkReset()) {
        doUpdateStateLoaded();
    }

    if(myToCheckUpdates && !myUpdates.isNull() && myUpdates->isInitialized()) {
        if(myUpdates->isNeedUpdate()) {
            myGUI->showUpdatesNotify();
        }
        myToCheckUpdates = false;
    }

    const bool isFullScreen = params.isFullscreen->getValue();
    myGUI->setVisibility(myWindow->getMousePos(), myToHideUIFullScr && isFullScreen);
    bool toHideCursor = isFullScreen && myGUI->toHideCursor();
    myWindow->showCursor(!toHideCursor);
}

void StImageViewer::stglDraw(unsigned int theView) {
    if( myContext.isNull()
    || !myContext->isBound()) {
        if(theView == ST_DRAW_LEFT
        || theView == ST_DRAW_MONO) {
            if(myWindow->isPaused()) {
                if(!myInactivityTimer.isOn()) {
                    myInactivityTimer.restart();
                } else if(myInactivityTimer.getElapsedTimeInSec() > 60.0) {
                    // perform delayed destruction on long inactivity
                    exit(0);
                } else {
                    // force deep sleeps
                    StThread::sleep(100);
                }
            }
        }
        return;
    }
    myInactivityTimer.stop();

    if(myContext->core20fwd != NULL) {
        // clear the screen and the depth buffer
        myContext->core20fwd->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

    if(myGUI.isNull()) {
        return;
    }

    myGUI->getCamera()->setView(theView);
    if(theView == ST_DRAW_LEFT
    || theView == ST_DRAW_MONO) {
        if(!myWindow->isActive()) {
            // enforce deep sleeps
            StThread::sleep(200);
        }

        myGUI->stglUpdate(myWindow->getMousePos());

        // check for mono state
        StHandle<StStereoParams> aParams = myGUI->myImage->getSource();
        if(!aParams.isNull()) {
            myWindow->setStereoOutput(!aParams->isMono() && myWindow->isActive()
                                   && (myGUI->myImage->params.displayMode->getValue() == StGLImageRegion::MODE_STEREO));
        }
    }

    // draw GUI
    myGUI->stglDraw(theView);
}

void StImageViewer::doScaleGui(const int32_t ) {
    if(myGUI.isNull()) {
        return;
    }
    myToRecreateMenu = true;
}

void StImageViewer::doScaleHiDPI(const bool ) {
    if(myGUI.isNull()) {
        return;
    }
    myToRecreateMenu = true;
}

void StImageViewer::doSwitchSrcFormat(const int32_t theSrcFormat) {
    if(myLoader.isNull()) {
        return;
    }

    myLoader->setStereoFormat(StFormat(theSrcFormat));
    if(!myLoader->getPlayList().isEmpty()) {
        myLoader->doLoadNext();
    }
    myToSaveSrcFormat = true;
}

void StImageViewer::doSwitchViewMode(const int32_t theMode) {
    if(myLoader.isNull()) {
        return;
    }

    bool isChanged = false;
    StGLFrameTextures& aTexture = myGUI->myImage->getTextureQueue()->getQTexture().getFront(StGLQuadTexture::LEFT_TEXTURE);
    if(aTexture.getPlane(0).getTarget() == GL_TEXTURE_CUBE_MAP) {
        isChanged = (theMode != StStereoParams::PANORAMA_CUBEMAP);
    } else {
        isChanged = (theMode == StStereoParams::PANORAMA_CUBEMAP);
    }

    if(isChanged
    && !myLoader->getPlayList().isEmpty()) {
        myLoader->doLoadNext();
    }
}

void StImageViewer::doPanoramaOnOff(const size_t ) {
    if(myLoader.isNull()) {
        return;
    }

    StHandle<StStereoParams> aParams = myGUI->myImage->getSource();
    if(aParams.isNull()
    || aParams->Src1SizeX == 0
    || aParams->Src1SizeY == 0) {
        return;
    }

    int aMode = myGUI->myImage->params.ViewMode->getValue();
    if(aMode != StStereoParams::FLAT_IMAGE) {
        myGUI->myImage->params.ViewMode->setValue(StStereoParams::FLAT_IMAGE);
        return;
    }

    StPanorama aPano = st::probePanorama(aParams->StereoFormat,
                                         aParams->Src1SizeX, aParams->Src1SizeY,
                                         aParams->Src2SizeX, aParams->Src2SizeY);
    myGUI->myImage->params.ViewMode->setValue(aPano == StPanorama_Cubemap6_1 || aPano == StPanorama_Cubemap3_2
                                            ? StStereoParams::PANORAMA_CUBEMAP
                                            : StStereoParams::PANORAMA_SPHERE);
}

void StImageViewer::doOpen1FileFromGui(StHandle<StString> thePath) {
    myOpenDialog->setPaths(*thePath, "");
}

void StImageViewer::doOpen1FileDialog(const size_t ) {
    myOpenDialog->openDialog(1);
}

void StImageViewer::doOpen2FilesDialog(const size_t ) {
    myOpenDialog->openDialog(2);
}

void StImageViewer::doUpdateStateLoading() {
    const StString aFileToLoad = myLoader->getPlayList().getCurrentTitle();
    if(aFileToLoad.isEmpty()) {
        myWindow->setTitle(myTitle);
    } else {
        myWindow->setTitle(aFileToLoad + " Loading... - sView");
    }
}

void StImageViewer::doUpdateStateLoaded() {
    const StString aFileToLoad = myLoader->getPlayList().getCurrentTitle();
    if(aFileToLoad.isEmpty()) {
        myWindow->setTitle(myTitle);
    } else {
        myWindow->setTitle(aFileToLoad + " - sView");
    }
}

void StImageViewer::doAboutImage(const size_t ) {
    if(!myGUI.isNull()) {
        myGUI->doAboutImage(0);
    }
}

void StImageViewer::doSaveImageInfo(const size_t theToSave) {
    if(!myGUI.isNull()
    && !myFileInfo.isNull()
    &&  theToSave == 1) {
        myLoader->doSaveInfo(myFileInfo);
    }
    myFileInfo.nullify();
}

void StImageViewer::doListFirst(const size_t ) {
    if(myLoader->getPlayList().walkToFirst()) {
        myLoader->doLoadNext();
        doUpdateStateLoading();
    }
}

void StImageViewer::doListPrev(const size_t ) {
    if(myLoader->getPlayList().walkToPrev()) {
        myLoader->doLoadNext();
        doUpdateStateLoading();
    }
}

void StImageViewer::doListNext(const size_t ) {
    if(myLoader->getPlayList().walkToNext()) {
        myLoader->doLoadNext();
        doUpdateStateLoading();
    }
}

void StImageViewer::doSlideShow(const size_t ) {
    if(mySlideShowTimer.getElapsedTimeInSec() > 0.0) {
        mySlideShowTimer.stop();
        myLoader->getPlayList().setLoop(false);
    } else {
        mySlideShowTimer.restart();
        myLoader->getPlayList().setLoop(true);
    }
}

void StImageViewer::doListLast(const size_t ) {
    if(myLoader->getPlayList().walkToLast()) {
        myLoader->doLoadNext();
        doUpdateStateLoading();
    }
}

void StImageViewer::doQuit(const size_t ) {
    StApplication::exit(0);
}

void StImageViewer::doSwitchVSync(const bool theValue) {
    StApplication::params.VSyncMode->setValue(theValue ? StGLContext::VSync_ON : StGLContext::VSync_OFF);
}

void StImageViewer::doFullscreen(const bool theIsFullscreen) {
    if(!myWindow.isNull()) {
        myWindow->setFullScreen(theIsFullscreen);
    }
}

void StImageViewer::doReset(const size_t ) {
    StHandle<StStereoParams> aParams = myGUI->myImage->getSource();
    if(!aParams.isNull()) {
        aParams->reset();
    }
}

void StImageViewer::doLoaded() {
    myEventLoaded.set();
}

void StImageViewer::doShowPlayList(const bool theToShow) {
    if(myGUI.isNull()
    || myGUI->myPlayList == NULL) {
        return;
    }

    myGUI->myPlayList->setOpacity(theToShow ? 1.0f : 0.0f, false);
}

void StImageViewer::doFileNext() {
    if(myLoader.isNull()) {
        return;
    }

    myLoader->doLoadNext();
    doUpdateStateLoading();
}

bool StImageViewer::getCurrentFile(StHandle<StFileNode>&     theFileNode,
                                   StHandle<StStereoParams>& theParams,
                                   StHandle<StImageInfo>&    theInfo) {
    theInfo.nullify();
    if(!myLoader->getPlayList().getCurrentFile(theFileNode, theParams)) {
        return false;
    }
    theInfo = myLoader->getFileInfo(theParams);
    return true;
}
