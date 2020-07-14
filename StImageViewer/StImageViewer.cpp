/**
 * Copyright © 2007-2020 Kirill Gavrilov <kirill@sview.ru>
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

#include "StImageOpenDialog.h"
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

    static const char ST_SETTING_LAST_FOLDER[] = "lastFolder";
    static const char ST_SETTING_RECENT_L[]    = "recentL";
    static const char ST_SETTING_RECENT_R[]    = "recentR";
    static const char ST_SETTING_COMPRESS[]    = "toCompress";
    static const char ST_SETTING_ESCAPENOQUIT[]= "escNoQuit";
    static const char ST_SETTING_FULLSCREENUI[]= "fullScreenUI";

    static const char ST_SETTING_SLIDESHOW[]   = "slideshow";
    static const char ST_SETTING_VIEWMODE[]    = "viewMode";
    static const char ST_SETTING_GAMMA[]       = "viewGamma";
    static const char ST_SETTING_IMAGELIB[]    = "imageLib";

    static const char ST_ARGUMENT_FILE_LEFT[]  = "left";
    static const char ST_ARGUMENT_FILE_RIGHT[] = "right";
    static const char ST_ARGUMENT_FILE_LAST[]  = "last";
    static const char ST_ARGUMENT_FILE_DEMO[]  = "demo";

    static const char ST_ARGUMENT_MONITOR[]    = "monitorId";
    static const char ST_ARGUMENT_WINLEFT[]    = "windowLeft";
    static const char ST_ARGUMENT_WINTOP[]     = "windowTop";
    static const char ST_ARGUMENT_WINWIDTH[]   = "windowWidth";
    static const char ST_ARGUMENT_WINHEIGHT[]  = "windowHeight";

}

void StImageViewer::updateStrings() {
    using namespace StImageViewerStrings;
    params.IsFullscreen->setName(tr(MENU_VIEW_FULLSCREEN));

    params.ExitOnEscape->setName(tr(OPTION_EXIT_ON_ESCAPE));
    params.ExitOnEscape->defineOption(ActionOnEscape_Nothing,              tr(OPTION_EXIT_ON_ESCAPE_NEVER));
    params.ExitOnEscape->defineOption(ActionOnEscape_ExitOneClick,         tr(OPTION_EXIT_ON_ESCAPE_ONE_CLICK));
    params.ExitOnEscape->defineOption(ActionOnEscape_ExitDoubleClick,      tr(OPTION_EXIT_ON_ESCAPE_DOUBLE_CLICK));
    params.ExitOnEscape->defineOption(ActionOnEscape_ExitOneClickWindowed, tr(OPTION_EXIT_ON_ESCAPE_WINDOWED));

    params.ToRestoreRatio->setName(tr(MENU_VIEW_RATIO_KEEP_ON_RESTART));

    params.ScaleAdjust->setName(tr(MENU_HELP_SCALE));
    params.ScaleAdjust->defineOption(StGLRootWidget::ScaleAdjust_Small,  tr(MENU_HELP_SCALE_SMALL));
    params.ScaleAdjust->defineOption(StGLRootWidget::ScaleAdjust_Normal, tr(MENU_HELP_SCALE_NORMAL));
    params.ScaleAdjust->defineOption(StGLRootWidget::ScaleAdjust_Big,    tr(MENU_HELP_SCALE_BIG));
    params.ScaleHiDPI2X->setName(tr(MENU_HELP_SCALE_HIDPI2X));
    params.CheckUpdatesDays->setName(tr(MENU_HELP_UPDATES));
    params.CheckUpdatesDays->defineOption(StCheckUpdates::UpdateInteval_Never,     tr(MENU_HELP_UPDATES_NEVER));
    params.CheckUpdatesDays->defineOption(StCheckUpdates::UpdateInteval_EveryDay,  tr(MENU_HELP_UPDATES_DAY));
    params.CheckUpdatesDays->defineOption(StCheckUpdates::UpdateInteval_EveryWeek, tr(MENU_HELP_UPDATES_WEEK));
    params.CheckUpdatesDays->defineOption(StCheckUpdates::UpdateInteval_EveryYear, tr(MENU_HELP_UPDATES_YEAR));
    params.LastUpdateDay->setName(tr(MENU_HELP_UPDATES));
    params.SrcStereoFormat->setName(tr(MENU_MEDIA_SRC_FORMAT));
    params.ToShowPlayList->setName(tr(PLAYLIST));
    params.ToShowAdjustImage->setName(tr(MENU_VIEW_IMAGE_ADJUST));
    params.ToSwapJPS->setName(tr(OPTION_SWAP_JPS));
    params.ToStickPanorama->setName(tr(MENU_VIEW_STICK_PANORAMA360));
    params.ToFlipCubeZ6x1->setName(tr(MENU_VIEW_FLIPZ_CUBE6x1));
    params.ToFlipCubeZ3x2->setName(tr(MENU_VIEW_FLIPZ_CUBE3x2));
    params.ToTrackHead->setName(tr(MENU_VIEW_TRACK_HEAD));
    params.ToShowFps->setName(tr(MENU_SHOW_FPS));
    params.ToShowMenu->setName(stCString("Show main menu"));
    params.ToShowTopbar->setName(stCString("Show top toolbar"));
    params.ToShowBottom->setName(stCString("Show bottom toolbar"));
    params.SlideShowDelay->setName(stCString("Slideshow delay"));
    params.IsMobileUI->setName(stCString("Mobile UI"));
    params.ToHideStatusBar->setName("Hide system status bar");
    params.ToHideNavBar   ->setName(tr(OPTION_HIDE_NAVIGATION_BAR));
    params.IsExclusiveFullScreen->setName(tr(MENU_EXCLUSIVE_FULLSCREEN));
    params.IsVSyncOn->setName(tr(MENU_VSYNC));
    params.ToOpenLast->setName(tr(OPTION_OPEN_LAST_ON_STARTUP));
    params.ToSaveRecent->setName(stCString("Remember recent file"));
    params.TargetFps->setName(stCString("FPS Target"));
    myLangMap->params.language->setName(tr(MENU_HELP_LANGS));
}

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
  //
  myToCheckUpdates(true),
  myToSaveSrcFormat(false),
  myEscNoQuit(false),
  myToHideUIFullScr(false),
  myToCheckPoorOrient(true) {
    mySettings = new StSettings(myResMgr, myAppName);
    myLangMap  = new StTranslations(myResMgr, StImageViewer::ST_DRAWER_PLUGIN_NAME);
    myOpenDialog = new StImageOpenDialog(this);
    StImageViewerStrings::loadDefaults(*myLangMap);
    myLangMap->params.language->signals.onChanged += stSlot(this, &StImageViewer::doChangeLanguage);

    myTitle = stCString("sView - Image Viewer");
    if(!theAppName.isEmpty()) {
        myTitle = theAppName;
    }
    //
    params.IsFullscreen = new StBoolParamNamed(false, stCString("fullscreen"));
    params.IsFullscreen->signals.onChanged.connect(this, &StImageViewer::doFullscreen);
    params.ExitOnEscape = new StEnumParam(ActionOnEscape_ExitOneClick, stCString("exitOnEscape"));
    params.ToRestoreRatio = new StBoolParamNamed(false, stCString("toRestoreRatio"));
    params.ScaleAdjust = new StEnumParam(StGLRootWidget::ScaleAdjust_Normal, stCString("scaleAdjust"));
    params.ScaleHiDPI       = new StFloat32Param(1.0f, stCString("scaleHiDPI"));
    params.ScaleHiDPI->setMinMaxValues(0.5f, 3.0f);
    params.ScaleHiDPI->setDefValue(1.0f);
    params.ScaleHiDPI->setStep(1.0f);
    params.ScaleHiDPI->setTolerance(0.001f);
    params.ScaleHiDPI2X     = new StBoolParamNamed(false, stCString("scale2X"));
    params.CheckUpdatesDays = new StEnumParam(StCheckUpdates::UpdateInteval_EveryWeek, stCString("updatesIntervalEnum"));
    params.LastUpdateDay    = new StInt32ParamNamed(0, stCString("updatesLastCheck"));
    params.SrcStereoFormat  = new StInt32ParamNamed(StFormat_AUTO, stCString("srcFormat"));
    params.SrcStereoFormat->signals.onChanged.connect(this, &StImageViewer::doSwitchSrcFormat);
    params.ToShowPlayList   = new StBoolParamNamed(false, stCString("showPlaylist"));
    params.ToShowPlayList->signals.onChanged = stSlot(this, &StImageViewer::doShowPlayList);
    params.ToShowAdjustImage = new StBoolParamNamed(false, stCString("showAdjustImage"));
    params.ToShowAdjustImage->signals.onChanged = stSlot(this, &StImageViewer::doShowAdjustImage);
    params.ToSwapJPS = new StBoolParamNamed(false, stCString("toSwapJPS"));
    params.ToSwapJPS->signals.onChanged = stSlot(this, &StImageViewer::doChangeSwapJPS);
    params.ToStickPanorama = new StBoolParamNamed(false, stCString("toStickPano360"));
    params.ToStickPanorama->signals.onChanged = stSlot(this, &StImageViewer::doChangeStickPano360);
    params.ToFlipCubeZ6x1= new StBoolParamNamed(true,  stCString("toFlipCube6x1"));
    params.ToFlipCubeZ6x1->signals.onChanged = stSlot(this, &StImageViewer::doChangeFlipCubeZ);
    params.ToFlipCubeZ3x2= new StBoolParamNamed(false, stCString("toFlipCube3x2"));
    params.ToFlipCubeZ3x2->signals.onChanged = stSlot(this, &StImageViewer::doChangeFlipCubeZ);
    params.ToTrackHead   = new StBoolParamNamed(true,  stCString("toTrackHead"));
    params.ToShowFps     = new StBoolParamNamed(false, stCString("toShowFps"));
    params.ToShowMenu    = new StBoolParamNamed(true,  stCString("toShowMenu"));
    params.ToShowTopbar  = new StBoolParamNamed(true,  stCString("toShowTopbar"));
    params.ToShowBottom  = new StBoolParamNamed(true,  stCString("toShowBottom"));
    params.SlideShowDelay = new StFloat32Param(4.0f, stCString("slideShowDelay2"));
    params.SlideShowDelay->setMinMaxValues(1.0f, 10.0f);
    params.SlideShowDelay->setDefValue(4.0f);
    params.SlideShowDelay->setStep(1.0f);
    params.SlideShowDelay->setTolerance(0.1f);
    params.SlideShowDelay->setFormat(stCString("%01.1f s"));
    params.IsMobileUI    = new StBoolParamNamed(StWindow::isMobile(), stCString("isMobileUI"));
    params.IsMobileUI->signals.onChanged = stSlot(this, &StImageViewer::doChangeMobileUI);
    params.IsMobileUISwitch = new StBoolParam(params.IsMobileUI->getValue());
    params.IsMobileUISwitch->signals.onChanged = stSlot(this, &StImageViewer::doScaleHiDPI);
    params.ToHideStatusBar = new StBoolParamNamed(true, stCString("toHideStatusBar"));
    params.ToHideStatusBar->signals.onChanged = stSlot(this, &StImageViewer::doHideSystemBars);
    params.ToHideNavBar    = new StBoolParamNamed(true, stCString("toHideNavBar"));
    params.ToHideNavBar   ->signals.onChanged = stSlot(this, &StImageViewer::doHideSystemBars);
    params.IsExclusiveFullScreen = new StBoolParamNamed(false, stCString("isExclusiveFullScreen"));
    params.IsVSyncOn     = new StBoolParamNamed(true,  stCString("vsync"));
    params.IsVSyncOn->signals.onChanged = stSlot(this, &StImageViewer::doSwitchVSync);
    StApplication::params.VSyncMode->setValue(StGLContext::VSync_ON);
    params.ToOpenLast   = new StBoolParamNamed(false, stCString("toOpenLast"));
    params.ToSaveRecent = new StBoolParamNamed(false, stCString("toSaveRecent"));
    params.imageLib = StImageFile::ST_LIBAV,
    params.TargetFps = new StInt32ParamNamed(0, stCString("fpsTarget"));
    updateStrings();

    mySettings->loadParam(params.ExitOnEscape);
    mySettings->loadParam(params.ScaleAdjust);
    params.ScaleAdjust->signals.onChanged  = stSlot(this, &StImageViewer::doScaleGui);
    mySettings->loadParam (params.ScaleHiDPI2X);
    params.ScaleHiDPI2X->signals.onChanged = stSlot(this, &StImageViewer::doScaleHiDPI);
    mySettings->loadParam (params.TargetFps);
    mySettings->loadString(ST_SETTING_LAST_FOLDER,        params.lastFolder);
    mySettings->loadParam (params.LastUpdateDay);
    mySettings->loadParam (params.CheckUpdatesDays);
    mySettings->loadParam (params.ToSwapJPS);
    mySettings->loadParam (params.ToStickPanorama);
    mySettings->loadParam (params.ToFlipCubeZ6x1);
    mySettings->loadParam (params.ToFlipCubeZ3x2);
    myToCheckPoorOrient = !mySettings->loadParam(params.ToTrackHead);
    mySettings->loadParam (params.ToShowFps);
    mySettings->loadParam (params.SlideShowDelay);
    mySettings->loadParam (params.IsMobileUI);
    mySettings->loadParam (params.ToHideStatusBar);
    mySettings->loadParam (params.ToHideNavBar);
    mySettings->loadParam (params.ToOpenLast);
    mySettings->loadParam (params.IsExclusiveFullScreen);
    mySettings->loadParam (params.IsVSyncOn);
    mySettings->loadParam (params.ToShowPlayList);
    mySettings->loadParam (params.ToShowAdjustImage);

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
        StWinAttr_GlDepthSize,   (StWinAttr )0,
        StWinAttr_GlStencilSize, (StWinAttr )0,
        StWinAttr_NULL
    };
    for(size_t aRendIter = 0; aRendIter < myRenderers.size(); ++aRendIter) {
        StHandle<StWindow>& aRend = myRenderers[aRendIter];
        aRend->setAttributes(anAttribs);
    }

    // create actions
    StHandle<StAction> anAction;
    anAction = new StActionBool(stCString("DoFullscreen"), params.IsFullscreen);
    addAction(Action_Fullscreen, anAction, ST_VK_F, ST_VK_RETURN);

    anAction = new StActionBool(stCString("DoShowFPS"), params.ToShowFps);
    addAction(Action_ShowFps, anAction, ST_VK_F12);

    anAction = new StActionIntSlot(stCString("DoShowGUI"), stSlot(this, &StImageViewer::doShowHideGUI), 0);
    addAction(Action_ShowGUI, anAction, ST_VK_TILDE);

    anAction = new StActionIntValue(stCString("DoSrcAuto"), params.SrcStereoFormat, StFormat_AUTO);
    addAction(Action_SrcAuto, anAction, ST_VK_A);

    anAction = new StActionIntValue(stCString("DoSrcMono"), params.SrcStereoFormat, StFormat_Mono);
    addAction(Action_SrcMono, anAction, ST_VK_M);

    anAction = new StActionIntValue(stCString("DoSrcOverUnder"), params.SrcStereoFormat, StFormat_TopBottom_LR);
    addAction(Action_SrcOverUnderLR, anAction, ST_VK_O);

    anAction = new StActionIntValue(stCString("DoSrcSideBySide"), params.SrcStereoFormat, StFormat_SideBySide_RL);
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
    addAction(Action_ListPrev, anAction, ST_VK_PRIOR, StWindow::isMobile() ? ST_VK_VOLUME_UP : 0);

    anAction = new StActionIntSlot(stCString("DoListNext"), stSlot(this, &StImageViewer::doListNext), 0);
    addAction(Action_ListNext, anAction, ST_VK_NEXT,  StWindow::isMobile() ? ST_VK_VOLUME_DOWN : 0);

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

    {
    anAction = new StActionIntSlot(stCString("DoOutStereoNormal"), stSlot(this, &StImageViewer::doSetStereoOutput), StGLImageRegion::MODE_STEREO);
    addAction(Action_OutStereoNormal, anAction);

    anAction = new StActionIntSlot(stCString("DoOutStereoLeftView"), stSlot(this, &StImageViewer::doSetStereoOutput), StGLImageRegion::MODE_ONLY_LEFT);
    addAction(Action_OutStereoLeftView, anAction);

    anAction = new StActionIntSlot(stCString("DoOutStereoRightView"), stSlot(this, &StImageViewer::doSetStereoOutput), StGLImageRegion::MODE_ONLY_RIGHT);
    addAction(Action_OutStereoRightView, anAction);

    anAction = new StActionIntSlot(stCString("DoOutStereoParallelPair"), stSlot(this, &StImageViewer::doSetStereoOutput), StGLImageRegion::MODE_PARALLEL);
    addAction(Action_OutStereoParallelPair, anAction);

    anAction = new StActionIntSlot(stCString("DoOutStereoCrossEyed"), stSlot(this, &StImageViewer::doSetStereoOutput), StGLImageRegion::MODE_CROSSYED);
    addAction(Action_OutStereoCrossEyed, anAction);
    }
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

    mySettings->saveParam(myGUI->myImage->params.DisplayMode);
    mySettings->saveInt32(ST_SETTING_GAMMA, stRound(100.0f * myGUI->myImage->params.Gamma->getValue()));
    mySettings->saveParam(myGUI->myImage->params.ToHealAnamorphicRatio);
    mySettings->saveInt32(myGUI->myImage->params.DisplayRatio->getKey(),
                          params.ToRestoreRatio->getValue()
                        ? myGUI->myImage->params.DisplayRatio->getValue()
                        : StGLImageRegion::RATIO_AUTO);
    mySettings->saveParam(myGUI->myImage->params.TextureFilter);
}

void StImageViewer::saveAllParams() {
    saveGuiParams();
    if(!myGUI.isNull()) {
        mySettings->saveParam (params.ExitOnEscape);
        mySettings->saveParam (params.ScaleAdjust);
        mySettings->saveParam (params.ScaleHiDPI2X);
        mySettings->saveParam (params.TargetFps);
        mySettings->saveParam(params.LastUpdateDay);
        mySettings->saveParam(params.CheckUpdatesDays);
        mySettings->saveString(ST_SETTING_IMAGELIB,  StImageFile::imgLibToString(params.imageLib));
        mySettings->saveParam (params.ToSwapJPS);
        mySettings->saveParam (params.ToStickPanorama);
        mySettings->saveParam (params.ToFlipCubeZ6x1);
        mySettings->saveParam (params.ToFlipCubeZ3x2);
        mySettings->saveParam (params.ToTrackHead);
        mySettings->saveParam (params.ToShowFps);
        mySettings->saveParam (params.SlideShowDelay);
        mySettings->saveParam (params.IsMobileUI);
        mySettings->saveParam (params.ToHideStatusBar);
        mySettings->saveParam (params.ToHideNavBar);
        mySettings->saveParam (params.ToOpenLast);
        mySettings->saveParam (params.IsExclusiveFullScreen);
        mySettings->saveParam (params.IsVSyncOn);
        mySettings->saveParam (params.ToShowPlayList);
        mySettings->saveParam (params.ToShowAdjustImage);
        if(myToSaveSrcFormat) {
            mySettings->saveParam(params.SrcStereoFormat);
        }

        // store hot-keys
        for(std::map< int, StHandle<StAction> >::iterator anIter = myActions.begin();
            anIter != myActions.end(); ++anIter) {
            mySettings->saveHotKey(anIter->second);
        }
    }

    StString aLastL, aLastR;
    StHandle<StFileNode> aFile = myPlayList->getCurrentFile();
    if((params.ToSaveRecent->getValue() || params.ToOpenLast->getValue())
    && !aFile.isNull()) {
        if(aFile->isEmpty()) {
            aLastL = aFile->getPath();
        } else if(aFile->size() == 2) {
            aLastL = aFile->getValue(0)->getPath();
            aLastR = aFile->getValue(1)->getPath();
        }
    }

    // skip temporary URLs
    if(StFileNode::isContentProtocolPath(aLastL)) {
        aLastL.clear();
        aLastR.clear();
    } else if(StFileNode::isContentProtocolPath(aLastR)) {
        aLastR.clear();
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
    StGLDeviceCaps aDevCaps = myContext->getDeviceCaps();
    // better slow-down GPU memory copy but avoid extra memory usage
    aDevCaps.hasUnpack = true;
    myGUI->myImage->getTextureQueue()->setDeviceCaps(aDevCaps);

    // load settings
    doChangeMobileUI(params.IsMobileUI->getValue());
    myWindow->setTargetFps(double(params.TargetFps->getValue()));
    mySettings->loadParam (myGUI->myImage->params.DisplayMode);
    mySettings->loadParam (myGUI->myImage->params.TextureFilter);
    mySettings->loadParam (myGUI->myImage->params.DisplayRatio);
    mySettings->loadParam (myGUI->myImage->params.ToHealAnamorphicRatio);
    params.ToRestoreRatio->setValue(myGUI->myImage->params.DisplayRatio->getValue() != StGLImageRegion::RATIO_AUTO);
    int32_t loadedGamma = 100; // 1.0f
        mySettings->loadInt32(ST_SETTING_GAMMA, loadedGamma);
        myGUI->myImage->params.Gamma->setValue(0.01f * loadedGamma);

    // initialize frame region early to show dedicated error description
    if(!myGUI->myImage->stglInit()) {
        myMsgQueue->pushError(stCString("Image Viewer - critical error:\nFrame region initialization failed!"));
        myMsgQueue->popAll();
        myGUI.nullify();
        return false;
    }
    myGUI->stglInit();
    StRectF_t aFrustL, aFrustR;
    if(myWindow->getCustomProjection(aFrustL, aFrustR)) {
        myGUI->changeCamera()->setCustomProjection(aFrustL, aFrustR);
    } else {
        myGUI->changeCamera()->resetCustomProjection();
    }
    myGUI->stglResize(myWindow->stglViewport(ST_WIN_MASTER), myWindow->getMargins(), (float )myWindow->stglAspectRatio());

    for(size_t anIter = 0; anIter < myGUI->myImage->getActions().size(); ++anIter) {
        StHandle<StAction>& anAction = myGUI->myImage->changeActions()[anIter];
        mySettings->loadHotKey(anAction);
        addAction(Action_StereoParamsBegin + int(anIter), anAction);
    }
    registerHotKeys();
    return true;
}

void StImageViewer::doChangeLanguage(const int32_t theNewLang) {
    StApplication::doChangeLanguage(theNewLang);
    StImageViewerStrings::loadDefaults(*myLangMap);
    updateStrings();
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
    myWindow->setHideSystemBars(params.ToHideStatusBar->getValue(), params.ToHideNavBar->getValue());
    if(isReset) {
        if(params.IsFullscreen->getValue()) {
            myWindow->setFullScreen(true);
        }
        return true;
    }

    StString anImgLibStr;
    mySettings->loadString(ST_SETTING_IMAGELIB, anImgLibStr);
    params.imageLib = StImageFile::imgLibFromString(anImgLibStr);
    myLoader = new StImageLoader(params.imageLib, myResMgr, myMsgQueue, myLangMap, myPlayList,
                                 myGUI->myImage->getTextureQueue(), myContext->getMaxTextureSize());
    myLoader->signals.onLoaded.connect(this, &StImageViewer::doLoaded);
    myLoader->setCompressMemory(myWindow->isMobile());
    myLoader->setSwapJPS(params.ToSwapJPS->getValue());
    myLoader->setStickPano360(params.ToStickPanorama->getValue());
    myLoader->setFlipCubeZ6x1(params.ToFlipCubeZ6x1->getValue());
    myLoader->setFlipCubeZ3x2(params.ToFlipCubeZ3x2->getValue());

    // load this parameter AFTER image thread creation
    mySettings->loadParam(params.SrcStereoFormat);

#if defined(ST_UPDATES_CHECK)
    // read the current time
    time_t aRawtime;
    time(&aRawtime);
    struct tm* aTimeinfo = localtime(&aRawtime);
    int32_t aCurrentDayInYear = aTimeinfo->tm_yday;

    const int aNbDays = StCheckUpdates::getNbDaysFromInterval((StCheckUpdates::UpdateInteval )params.CheckUpdatesDays->getValue());
    if(aNbDays > 0
    && std::abs(aCurrentDayInYear - params.LastUpdateDay->getValue()) > aNbDays) {
        myUpdates = new StCheckUpdates();
        myUpdates->init();
        params.LastUpdateDay->setValue(aCurrentDayInYear);
        mySettings->saveParam(params.LastUpdateDay);
    }
#endif
    return true;
}

void StImageViewer::parseArguments(const StArgumentsMap& theArguments) {
    StArgument anArgSlideshow  = theArguments[ST_SETTING_SLIDESHOW];
    StArgument anArgViewMode   = theArguments[ST_SETTING_VIEWMODE];
    StArgument anArgSrcFormat  = theArguments[params.SrcStereoFormat->getKey()];
    StArgument anArgImgLibrary = theArguments[ST_SETTING_IMAGELIB];
    StArgument anArgToCompress = theArguments[ST_SETTING_COMPRESS];
    StArgument anArgEscNoQuit  = theArguments[ST_SETTING_ESCAPENOQUIT];
    StArgument anArgFullScreenUI = theArguments[ST_SETTING_FULLSCREENUI];
    StArgument anArgShowMenu   = theArguments[params.ToShowMenu->getKey()];
    StArgument anArgShowTopbar = theArguments[params.ToShowTopbar->getKey()];
    StArgument anArgSaveRecent = theArguments[params.ToSaveRecent->getKey()];

    StArgument anArgFullscreen = theArguments[params.IsFullscreen->getKey()];
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
        params.IsFullscreen->setValue(!anArgFullscreen.isValueOff());
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
        myPlayList->changeDefParams().ViewingMode = StStereoParams::GET_VIEW_MODE_FROM_STRING(anArgViewMode.getValue());
    }
    if(anArgSrcFormat.isValid()) {
        params.SrcStereoFormat->setValue(st::formatFromString(anArgSrcFormat.getValue()));
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
    const StArgument anArgLast     = myOpenFileInfo->getArgumentsMap()[ST_ARGUMENT_FILE_LAST];
    const StArgument anArgFileDemo = myOpenFileInfo->getArgumentsMap()[ST_ARGUMENT_FILE_DEMO];
    const bool toOpenLast = anArgLast.isValid() ? !anArgLast.isValueOff() : params.ToOpenLast->getValue();
    if(myOpenFileInfo->getPath().isEmpty() || (toOpenLast && anArgFileDemo.isValid())) {
        if(toOpenLast) {
            StString aLastL, aLastR;
            mySettings->loadString(ST_SETTING_RECENT_L, aLastL);
            mySettings->loadString(ST_SETTING_RECENT_R, aLastR);
            if(!aLastL.isEmpty()) {
                if(!aLastR.isEmpty()) {
                    myPlayList->clear();
                    myPlayList->addOneFile(aLastL, aLastR);
                } else {
                    myPlayList->open(aLastL);
                }

                if(!myPlayList->isEmpty()) {
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
    myPlayList->clear();

    //StArgument argFile1     = myOpenFileInfo->getArgumentsMap()[ST_ARGUMENT_FILE + 1]; // playlist?
    StArgument argFileLeft  = myOpenFileInfo->getArgumentsMap()[ST_ARGUMENT_FILE_LEFT];
    StArgument argFileRight = myOpenFileInfo->getArgumentsMap()[ST_ARGUMENT_FILE_RIGHT];
    if(argFileLeft.isValid() && argFileRight.isValid()) {
        // meta-file
        /// TODO (Kirill Gavrilov#4) we should use MIME type!
        myPlayList->addOneFile(argFileLeft.getValue(), argFileRight.getValue());
    } else if(!anOpenMIME.isEmpty()) {
        // create just one-file playlist
        myPlayList->addOneFile(myOpenFileInfo->getPath(), anOpenMIME);
    } else {
        // create playlist from file's folder
        myPlayList->open(myOpenFileInfo->getPath());
    }

    if(!myPlayList->isEmpty()) {
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

    const StMarginsI& aMargins = myWindow->getMargins();
    const bool wasMobileGui = myGUI->isMobile();
    const bool toMobileGui  = toUseMobileUI(aMargins);
    if(toMobileGui != wasMobileGui) {
        doChangeMobileUI(params.IsMobileUI->getValue());
    } else {
        StRectF_t aFrustL, aFrustR;
        if(myWindow->getCustomProjection(aFrustL, aFrustR)) {
            myGUI->changeCamera()->setCustomProjection(aFrustL, aFrustR);
        } else {
            myGUI->changeCamera()->resetCustomProjection();
        }
        myGUI->stglResize(myWindow->stglViewport(ST_WIN_MASTER), myWindow->getMargins(), (float )myWindow->stglAspectRatio());
    }
}

void StImageViewer::doImageAdjustReset(const size_t ) {
    if(myGUI.isNull()) {
        return;
    }

    myGUI->myImage->params.Gamma     ->reset();
    myGUI->myImage->params.Brightness->reset();
    myGUI->myImage->params.Saturation->reset();
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

    myFileToDelete = myPlayList->getCurrentFile();
    if(myFileToDelete.isNull()
    || myFileToDelete->size() != 0) {
        myFileToDelete.nullify();
        return;
    }

    const bool isReadOnly = StFileNode::isFileReadOnly(myFileToDelete->getPath());
    const StString aText = myLangMap->getValue(StImageViewerStrings::DIALOG_DELETE_FILE_QUESTION)
                         + (isReadOnly ? "\nWARNING! The file is READ ONLY!" : "")
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

    StFileNode::removeReadOnlyFlag(myFileToDelete->getPath());
    myPlayList->removePhysically(myFileToDelete);
    if(!myPlayList->isEmpty()) {
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
            if(!doExitOnEscape(!myEscNoQuit ? (ActionOnEscape )params.ExitOnEscape->getValue() : ActionOnEscape_Nothing)) {
                if(myWindow->hasFullscreenMode()
                && myWindow->isFullScreen()) {
                    params.IsFullscreen->setValue(false);
                }
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
                myGUI->myImage->params.Brightness->increment();
            } else if(theEvent.Flags == ST_VF_CONTROL) {
                myGUI->myImage->params.Brightness->decrement();
            }
            return;
        }
        case ST_VK_T: {
            /// TODO (Kirill Gavrilov#9) remove this hot key
            if(theEvent.Flags == ST_VF_SHIFT) {
                myGUI->myImage->params.Saturation->increment();
            } else if(theEvent.Flags == ST_VF_CONTROL) {
                myGUI->myImage->params.Saturation->decrement();
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

    myGUI->tryClick(theEvent);
}

void StImageViewer::doMouseUp(const StClickEvent& theEvent) {
    if(myGUI.isNull()) {
        return;
    }

    if(theEvent.Button == ST_MOUSE_MIDDLE) {
        params.IsFullscreen->reverse();
    }
    myGUI->tryUnClick(theEvent);
}

void StImageViewer::doGesture(const StGestureEvent& theEvent) {
    if(!myGUI.isNull()) {
        myGUI->doGesture(theEvent);
    }
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
    if(!myPlayList->checkExtension(aFile1)
     && myLoader->getMimeListVideo().checkExtension(StFileNode::getExtension(aFile1))) {
        // redirect to StMoviePlayer
        myOpenFileOtherApp = new StOpenInfo();
        StArgumentsMap anArgs;
        anArgs.add(StArgument("in", "video"));
        myOpenFileOtherApp->setArgumentsMap(anArgs);
        myOpenFileOtherApp->setPath(aFile1);
        exit(0);
        return;
    }

    if(theEvent.NbFiles == 1) {
        myPlayList->open(aFile1);
        doUpdateStateLoading();
        myLoader->doLoadNext();
        return;
    } else if(theEvent.NbFiles == 2
          && !StFolder::isFolder(aFile1)
          && !StFolder::isFolder(StString(theEvent.Files[1]))) {
        myPlayList->clear();
        myPlayList->addOneFile(aFile1, StString(theEvent.Files[1]));
        doUpdateStateLoading();
        myLoader->doLoadNext();
        return;
    }

    myPlayList->clear();
    for(uint32_t aFileIter = 0; aFileIter < theEvent.NbFiles; ++aFileIter) {
        StString aPath(theEvent.Files[aFileIter]);
        if(!StFolder::isFolder(aPath)) {
            myPlayList->addOneFile(aPath, StMIME());
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
            myPlayList->clear();
            myPlayList->addOneFile(myOpenDialog->getPathLeft(), myOpenDialog->getPathRight());
        } else {
            if(!myPlayList->checkExtension(myOpenDialog->getPathLeft())
             && myLoader->getMimeListVideo().checkExtension(StFileNode::getExtension(myOpenDialog->getPathLeft()))) {
                // redirect to StMoviePlayer
                myOpenFileOtherApp = new StOpenInfo();
                StArgumentsMap anArgs;
                anArgs.add(StArgument("in", "video"));
                myOpenFileOtherApp->setArgumentsMap(anArgs);
                myOpenFileOtherApp->setPath(myOpenDialog->getPathLeft());
                exit(0);
            } else {
                myPlayList->open(myOpenDialog->getPathLeft());
            }
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

    if(mySlideShowTimer.getElapsedTimeInSec() > params.SlideShowDelay->getValue()) {
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

    const bool isFullScreen = params.IsFullscreen->getValue();
    myGUI->setVisibility(myWindow->getMousePos(), myToHideUIFullScr && isFullScreen);
    bool toHideCursor = isFullScreen && myGUI->toHideCursor();
    myWindow->showCursor(!toHideCursor);

    // for image viewer it is OK to make longer smoothed uploads
    myGUI->myImage->getTextureQueue()->getUploadParams().MaxUploadIterations = 10;
}

void StImageViewer::stglDraw(unsigned int theView) {
    const bool hasCtx = !myContext.isNull() && myContext->isBound();
    if(!hasCtx || myWindow->isPaused()) {
        if(theView == ST_DRAW_LEFT
        || theView == ST_DRAW_MONO) {
            if(myWindow->isPaused()) {
                const double aTimeout = hasCtx ? 300.0 : 60.0;
                if(!myInactivityTimer.isOn()) {
                    myInactivityTimer.restart();
                } else if(myInactivityTimer.getElapsedTimeInSec() > aTimeout) {
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

    myGUI->changeCamera()->setView(theView);
    if(theView == ST_DRAW_LEFT
    || theView == ST_DRAW_MONO) {
        if(!myWindow->isActive()) {
            // enforce deep sleeps
            StThread::sleep(200);
        }

        myGUI->stglUpdate(myWindow->getMousePos(), myWindow->isPreciseCursor());

        // check for mono state
        bool hasStereoSource = false;
        StHandle<StStereoParams> aParams = myGUI->myImage->getSource();
        if(!aParams.isNull()) {
            hasStereoSource =!aParams->isMono()
                           && myGUI->myImage->hasVideoStream()
                           && myGUI->myImage->params.DisplayMode->getValue() == StGLImageRegion::MODE_STEREO;
        }
        myWindow->setStereoOutput(hasStereoSource);
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

void StImageViewer::doChangeMobileUI(const bool ) {
    params.IsMobileUISwitch->setValue(toUseMobileUI());
}

void StImageViewer::doHideSystemBars(const bool ) {
    if(myWindow.isNull()) {
        return;
    }

    myWindow->setHideSystemBars(params.ToHideStatusBar->getValue(), params.ToHideNavBar->getValue());
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
    if(!myPlayList->isEmpty()) {
        myLoader->doLoadNext();
    }
    myToSaveSrcFormat = true;
}

void StImageViewer::doSwitchViewMode(const int32_t theMode) {
    if(myLoader.isNull()) {
        return;
    }

    myLoader->setTheaterMode(theMode == StViewSurface_Theater);

    bool isChanged = false;
    StGLFrameTextures& aTexture = myGUI->myImage->getTextureQueue()->getQTexture().getFront(StGLQuadTexture::LEFT_TEXTURE);
    if(aTexture.getPlane(0).getTarget() == GL_TEXTURE_CUBE_MAP) {
        isChanged = (theMode != StViewSurface_Cubemap && theMode != StViewSurface_CubemapEAC);
    } else {
        isChanged = (theMode == StViewSurface_Cubemap || theMode == StViewSurface_CubemapEAC);
    }

    if(isChanged
    && !myPlayList->isEmpty()) {
        myLoader->doLoadNext();
    }
}

void StImageViewer::doSetStereoOutput(const size_t theMode) {
    if(myLoader.isNull()) {
        return;
    }
    myGUI->myImage->params.DisplayMode->setValue((int32_t )theMode);
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
    if(aMode != StViewSurface_Plain) {
        myGUI->myImage->params.ViewMode->setValue(StViewSurface_Plain);
        return;
    }

    StPanorama aPano = st::probePanorama(aParams->StereoFormat,
                                         aParams->Src1SizeX, aParams->Src1SizeY,
                                         aParams->Src2SizeX, aParams->Src2SizeY);
    if(aPano == StPanorama_OFF) {
        size_t aSizeX = aParams->Src1SizeX;
        size_t aSizeY = aParams->Src1SizeY;
        StPairRatio aPairRatio = st::formatToPairRatio(aParams->StereoFormat);
        if(aPairRatio == StPairRatio_HalfWidth) {
            aSizeX /= 2;
        } else if(aPairRatio == StPairRatio_HalfHeight) {
            aSizeY /= 2;
        }
        if(aSizeX > 8 && aSizeY > 8) {
            if(double(aSizeX)/double(aSizeY) > 3.5) {
                myGUI->myImage->params.ViewMode->setValue(StViewSurface_Cylinder);
                return;
            }
        }
    }
    myGUI->myImage->params.ViewMode->setValue(StStereoParams::getViewSurfaceForPanoramaSource(aPano, true));
}

void StImageViewer::doChangeSwapJPS(const bool ) {
    if(!myLoader.isNull()) {
        myLoader->setSwapJPS(params.ToSwapJPS->getValue());
        StHandle<StStereoParams> aParams = myGUI->myImage->getSource();
        if(!aParams.isNull()
        && !myPlayList->isEmpty()) {
            StString aCurrFile = myPlayList->getCurrentTitle();
            aCurrFile.toLowerCase();
            if(aCurrFile.isEndsWith(stCString(".jps"))
            || aCurrFile.isEndsWith(stCString(".pps"))) {
                myLoader->doLoadNext();
            }
        }
    }
}

void StImageViewer::doChangeStickPano360(const bool ) {
    if(myLoader.isNull()) {
        return;
    }

    myLoader->setStickPano360(params.ToStickPanorama->getValue());
    if(!params.ToStickPanorama->getValue()) {
        return;
    }

    StHandle<StStereoParams> aParams = myGUI->myImage->getSource();
    if(!aParams.isNull()
    &&  myGUI->myImage->params.ViewMode->getValue() == StViewSurface_Plain
    && !myPlayList->isEmpty()) {
        myLoader->doLoadNext();
    }
}

void StImageViewer::doChangeFlipCubeZ(const bool ) {
    if(myLoader.isNull()) {
        return;
    }

    myLoader->setFlipCubeZ6x1(params.ToFlipCubeZ6x1->getValue());
    myLoader->setFlipCubeZ3x2(params.ToFlipCubeZ3x2->getValue());
}

void StImageViewer::doOpen1FileFromGui(StHandle<StString> thePath) {
    myOpenDialog->setPaths(*thePath, "");
}

void StImageViewer::doOpen1FileAction(const size_t ) {
    if(!myGUI.isNull() && (myWindow->isFullScreen() || myGUI->isMobile())) {
        myGUI->doOpenFile(0);
        return;
    }

    myOpenDialog->openDialog(1);
}

void StImageViewer::doOpen2FilesDialog(const size_t ) {
    myOpenDialog->openDialog(2);
}

void StImageViewer::doUpdateStateLoading() {
    const StString aFileToLoad = myPlayList->getCurrentTitle();
    if(aFileToLoad.isEmpty()) {
        myWindow->setTitle(myTitle);
    } else {
        myWindow->setTitle(aFileToLoad + " Loading... - sView");
    }
}

void StImageViewer::doUpdateStateLoaded() {
    const StString aFileToLoad = myPlayList->getCurrentTitle();
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
    if(myPlayList->walkToFirst()) {
        myLoader->doLoadNext();
        doUpdateStateLoading();
    }
}

void StImageViewer::doListPrev(const size_t ) {
    if(myPlayList->walkToPrev()) {
        myLoader->doLoadNext();
        doUpdateStateLoading();
    }
}

void StImageViewer::doListNext(const size_t ) {
    if(myPlayList->walkToNext()) {
        myLoader->doLoadNext();
        doUpdateStateLoading();
    }
}

void StImageViewer::doShowHideGUI(const size_t ) {
    const bool toShow = !params.ToShowMenu->getValue() || (!myGUI.isNull() && !myGUI->isVisibleGUI());
    params.ToShowMenu->setValue(toShow);
    params.ToShowTopbar->setValue(toShow);
    params.ToShowBottom->setValue(toShow);
    if(toShow && !myGUI.isNull()) {
        myGUI->setVisibility(myWindow->getMousePos(), false, true);
    }
}

void StImageViewer::doSlideShow(const size_t ) {
    if(mySlideShowTimer.getElapsedTimeInSec() > 0.0) {
        mySlideShowTimer.stop();
        myPlayList->setLoop(false);
    } else {
        mySlideShowTimer.restart();
        myPlayList->setLoop(true);
    }
}

void StImageViewer::doListLast(const size_t ) {
    if(myPlayList->walkToLast()) {
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
        myWindow->setAttribute(StWinAttr_ExclusiveFullScreen, params.IsExclusiveFullScreen->getValue());
        myWindow->setFullScreen(theIsFullscreen);
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

void StImageViewer::doShowAdjustImage(const bool theToShow) {
    if(myGUI.isNull()
    || myGUI->myAdjustOverlay == NULL) {
        return;
    }

    myGUI->myAdjustOverlay->setOpacity(theToShow ? 1.0f : 0.0f, false);
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
    if(!myPlayList->getCurrentFile(theFileNode, theParams)) {
        return false;
    }
    theInfo = myLoader->getFileInfo(theParams);
    return true;
}
