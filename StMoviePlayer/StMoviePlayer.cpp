/**
 * Copyright © 2007-2020 Kirill Gavrilov <kirill@sview.ru>
 *
 * StMoviePlayer program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * StMoviePlayer program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program.
 * If not, see <http://www.gnu.org/licenses/>.
 */

#include "StMoviePlayer.h"

#include "StALDeviceParam.h"
#include "StMovieOpenDialog.h"
#include "StMoviePlayerGUI.h"
#include "StMoviePlayerStrings.h"
#include "StVideo/StVideo.h"
#include "StTimeBox.h"

#include <StImage/StImageFile.h>
#include <StSocket/StCheckUpdates.h>
#include <StSettings/StSettings.h>
#include <StStrings/StStringStream.h>
#include <StCore/StSearchMonitors.h>

#include <StGL/StGLContext.h>
#include <StGLCore/StGLCore20.h>
#include <StGLWidgets/StGLButton.h>
#include <StGLWidgets/StGLImageRegion.h>
#include <StGLWidgets/StGLMessageBox.h>
#include <StGLWidgets/StGLMsgStack.h>
#include <StGLWidgets/StGLPlayList.h>
#include <StGLWidgets/StGLSeekBar.h>
#include <StGLWidgets/StGLSubtitles.h>
#include <StGLWidgets/StGLTextureButton.h>

#include "../StOutAnaglyph/StOutAnaglyph.h"
#include "../StOutDual/StOutDual.h"
#include "../StOutIZ3D/StOutIZ3D.h"
#include "../StOutInterlace/StOutInterlace.h"
#include "../StOutPageFlip/StOutPageFlipExt.h"
#include "../StOutDistorted/StOutDistorted.h"

#include <cstdlib> // std::abs(int)

#ifdef ST_HAVE_MONGOOSE
    #include "mongoose.h"
#endif

const StString StMoviePlayer::ST_DRAWER_PLUGIN_NAME = "StMoviePlayer";

using namespace StMoviePlayerStrings;

namespace {

    static const char ST_SETTING_LAST_FOLDER[]   = "lastFolder";
    static const char ST_SETTING_RECENT_FILES[]  = "recent";

    static const char ST_SETTING_VIEWMODE[]      = "viewMode";
    static const char ST_SETTING_GAMMA[]         = "viewGamma";

    static const char ST_SETTING_WEBUI_CMDPORT[] = "webuiCmdPort";

    static const char ST_ARGUMENT_FILE_LEFT[]  = "left";
    static const char ST_ARGUMENT_FILE_RIGHT[] = "right";
    static const char ST_ARGUMENT_FILE_LAST[]  = "last";
    static const char ST_ARGUMENT_FILE_DEMO[]  = "demo";
    static const char ST_ARGUMENT_FILE_PAUSE[] = "pause";
    static const char ST_ARGUMENT_FILE_PAUSED[]= "paused";
    static const char ST_ARGUMENT_FILE_SEEK[]  = "seek";
    static const char ST_ARGUMENT_MONITOR[]    = "monitorId";
    static const char ST_ARGUMENT_WINLEFT[]    = "windowLeft";
    static const char ST_ARGUMENT_WINTOP[]     = "windowTop";
    static const char ST_ARGUMENT_WINWIDTH[]   = "windowWidth";
    static const char ST_ARGUMENT_WINHEIGHT[]  = "windowHeight";

}

void StMoviePlayer::doChangeDevice(const int32_t theValue) {
    StApplication::doChangeDevice(theValue);
    // update menu
}

void StMoviePlayer::doPause(const StPauseEvent& theEvent) {
    StApplication::doPause(theEvent);
    saveAllParams();
    if(myVideo.isNull()) {
        return;
    }

    // pause video but keep playing audio in background
    StWinAttr anAttribs[] = {
        StWinAttr_ToBlockSleepSystem,  (StWinAttr )0,
        StWinAttr_ToBlockSleepDisplay, (StWinAttr )0,
        StWinAttr_NULL
    };
    myWindow->getAttributes(anAttribs);
    if(anAttribs[1] == (StWinAttr )0
    || anAttribs[3] != (StWinAttr )0) {
        myVideo->pushPlayEvent(ST_PLAYEVENT_PAUSE);
    }
}

void StMoviePlayer::updateStrings() {
    params.ScaleAdjust->setName(tr(MENU_HELP_SCALE));
    params.ScaleAdjust->defineOption(StGLRootWidget::ScaleAdjust_Small,  tr(MENU_HELP_SCALE_SMALL));
    params.ScaleAdjust->defineOption(StGLRootWidget::ScaleAdjust_Normal, tr(MENU_HELP_SCALE_NORMAL));
    params.ScaleAdjust->defineOption(StGLRootWidget::ScaleAdjust_Big,    tr(MENU_HELP_SCALE_BIG));
    params.ScaleHiDPI2X->setName(tr(MENU_HELP_SCALE_HIDPI2X));
    params.SubtitlesPlace->setName(stCString("Subtitles Placement"));
    params.ToSearchSubs->setName(stCString("Search additional tracks"));
    params.SubtitlesParser->setName(tr(MENU_SUBTITLES_PARSER));
    params.SubtitlesParser->defineOption(0, tr(MENU_SUBTITLES_PLAIN_TEXT));
    params.SubtitlesParser->defineOption(1, tr(MENU_SUBTITLES_LITE_HTML));
    params.SubtitlesApplyStereo->setName(tr(MENU_SUBTITLES_STEREO));
    params.AudioAlHrtf->setName(stCString("Audio HRTF mixing"));
    params.AudioAlHrtf->defineOption(0, stCString("Auto"));
    params.AudioAlHrtf->defineOption(1, stCString("Forced ON"));
    params.AudioAlHrtf->defineOption(2, stCString("Forced OFF"));
    params.AudioMute->setName(stCString("Mute Audio"));
    params.IsFullscreen->setName(tr(MENU_VIEW_FULLSCREEN));

    params.ExitOnEscape->setName(tr(OPTION_EXIT_ON_ESCAPE));
    params.ExitOnEscape->defineOption(ActionOnEscape_Nothing,              tr(OPTION_EXIT_ON_ESCAPE_NEVER));
    params.ExitOnEscape->defineOption(ActionOnEscape_ExitOneClick,         tr(OPTION_EXIT_ON_ESCAPE_ONE_CLICK));
    params.ExitOnEscape->defineOption(ActionOnEscape_ExitDoubleClick,      tr(OPTION_EXIT_ON_ESCAPE_DOUBLE_CLICK));
    params.ExitOnEscape->defineOption(ActionOnEscape_ExitOneClickWindowed, tr(OPTION_EXIT_ON_ESCAPE_WINDOWED));

    params.ToRestoreRatio->setName(tr(MENU_VIEW_RATIO_KEEP_ON_RESTART));
    params.IsShuffle->setName(tr(MENU_MEDIA_SHUFFLE));
    params.ToLoopSingle->setName(stCString("Loop single item"));
    params.AreGlobalMKeys->setName(stCString("Use Multimedia Keys"));
    params.CheckUpdatesDays->setName(tr(MENU_HELP_UPDATES));
    params.CheckUpdatesDays->defineOption(StCheckUpdates::UpdateInteval_Never,     tr(MENU_HELP_UPDATES_NEVER));
    params.CheckUpdatesDays->defineOption(StCheckUpdates::UpdateInteval_EveryDay,  tr(MENU_HELP_UPDATES_DAY));
    params.CheckUpdatesDays->defineOption(StCheckUpdates::UpdateInteval_EveryWeek, tr(MENU_HELP_UPDATES_WEEK));
    params.CheckUpdatesDays->defineOption(StCheckUpdates::UpdateInteval_EveryYear, tr(MENU_HELP_UPDATES_YEAR));
    params.LastUpdateDay->setName(tr(MENU_HELP_UPDATES));
    params.SrcStereoFormat->setName(tr(MENU_MEDIA_SRC_FORMAT));
    params.ToShowPlayList->setName(tr(VIDEO_LIST));
    params.ToShowAdjustImage->setName(tr(MENU_VIEW_IMAGE_ADJUST));
    params.ToSwapJPS->setName(tr(OPTION_SWAP_JPS));
    params.ToStickPanorama->setName(tr(MENU_VIEW_STICK_PANORAMA360));
    params.ToTrackHead->setName(tr(MENU_VIEW_TRACK_HEAD));
    params.ToTrackHeadAudio->setName(tr(MENU_VIEW_TRACK_HEAD_AUDIO));
    params.ToForceBFormat->setName(stCString("Force B-Format"));
    params.ToShowFps->setName(tr(MENU_FPS_METER));
    params.ToShowMenu->setName(stCString("Show main menu"));
    params.ToShowTopbar->setName(stCString("Show top toolbar"));
    params.ToShowBottom->setName(stCString("Show seekbar"));
    params.ToMixImagesVideos->setName(stCString("Mix images & videos"));
    params.SlideShowDelay->setName(stCString("Slideshow delay"));
    params.IsMobileUI->setName(stCString("Mobile UI"));
    params.IsExclusiveFullScreen->setName(tr(MENU_EXCLUSIVE_FULLSCREEN));
    params.IsVSyncOn->setName(tr(MENU_FPS_VSYNC));
    params.ToLimitFps->setName(tr(MENU_FPS_BOUND));
    params.ToSmoothUploads->setName("Smooth texture uploading");
    params.StartWebUI->setName(stCString("Web UI start option"));
    params.StartWebUI->defineOption(WEBUI_OFF,  tr(MENU_MEDIA_WEBUI_OFF));
    params.StartWebUI->defineOption(WEBUI_ONCE, tr(MENU_MEDIA_WEBUI_ONCE));
    params.StartWebUI->defineOption(WEBUI_AUTO, tr(MENU_MEDIA_WEBUI_ON));
    params.ToPrintWebErrors->setName(tr(MENU_MEDIA_WEBUI_SHOW_ERRORS));
    params.IsLocalWebUI->setName(stCString("Local WebUI"));
    params.WebUIPort->setName(stCString("WebUI Port"));
    params.BlockSleeping->setName(tr(MENU_HELP_BLOCKSLP));
    params.BlockSleeping->defineOption(BLOCK_SLEEP_NEVER,      tr(MENU_HELP_BLOCKSLP_NEVER));
    params.BlockSleeping->defineOption(BLOCK_SLEEP_ALWAYS,     tr(MENU_HELP_BLOCKSLP_ALWAYS));
    params.BlockSleeping->defineOption(BLOCK_SLEEP_PLAYBACK,   tr(MENU_HELP_BLOCKSLP_PLAYBACK));
    params.BlockSleeping->defineOption(BLOCK_SLEEP_FULLSCREEN, tr(MENU_HELP_BLOCKSLP_FULLSCR));
    params.ToHideStatusBar->setName("Hide system status bar");
    params.ToHideNavBar   ->setName(tr(OPTION_HIDE_NAVIGATION_BAR));
    params.ToOpenLast     ->setName(tr(OPTION_OPEN_LAST_ON_STARTUP));
    params.ToShowExtra->setName(tr(MENU_HELP_EXPERIMENTAL));
    params.TargetFps->setName(stCString("FPS Target"));

#if defined(_WIN32)
    const StCString aGpuAcc = stCString(" (DXVA2)");
#elif defined(__APPLE__)
    const StCString aGpuAcc = stCString(" (VideoToolbox)");
#elif defined(__ANDROID__)
    const StCString aGpuAcc = stCString(""); //stCString(" (Android Media Codec)");
#else
    const StCString aGpuAcc = stCString("");
#endif
    params.UseGpu->setName(tr(MENU_MEDIA_GPU_DECODING) + aGpuAcc);
    params.UseOpenJpeg->setName(stCString("Use OpenJPEG instead of jpeg2000"));
    params.SnapshotImgType->setName(stCString("Snapshot Image Format"));
    params.Benchmark->setName(stCString("Benchmark"));
    myLangMap->params.language->setName(tr(MENU_HELP_LANGS));
}

StMoviePlayer::StMoviePlayer(const StHandle<StResourceManager>& theResMgr,
                             const StNativeWin_t                theParentWin,
                             const StHandle<StOpenInfo>&        theOpenInfo)
: StApplication(theResMgr, theParentWin, theOpenInfo),
  myPlayList(new StPlayList(4, true)),
  myEventLoaded(false),
  mySeekOnLoad(-1.0),
  myAudioOnLoad(-1),
  mySubsOnLoad(-1),
  //
  myWebCtx(NULL),
  //
  myToUpdateALList(false),
  myToCheckUpdates(true),
  myToCheckPoorOrient(true) {
    mySettings = new StSettings(myResMgr, ST_DRAWER_PLUGIN_NAME);
    myLangMap  = new StTranslations(myResMgr, StMoviePlayer::ST_DRAWER_PLUGIN_NAME);
    myOpenDialog = new StMovieOpenDialog(this);
    StMoviePlayerStrings::loadDefaults(*myLangMap);
    myLangMap->params.language->signals.onChanged += stSlot(this, &StMoviePlayer::doChangeLanguage);
    myTitle = stCString("sView - Movie Player");

    params.ScaleAdjust = new StEnumParam(StGLRootWidget::ScaleAdjust_Normal, stCString("scaleAdjust"));
    params.ScaleHiDPI  = new StFloat32Param(1.0f, stCString("scaleHiDPI"));
    params.ScaleHiDPI->setMinMaxValues(0.5f, 3.0f);
    params.ScaleHiDPI->setDefValue(1.0f);
    params.ScaleHiDPI->setStep(1.0f);
    params.ScaleHiDPI->setTolerance(0.001f);
    params.ScaleHiDPI2X     = new StBoolParamNamed(false, stCString("scale2X"));
    params.SubtitlesPlace   = new StInt32ParamNamed(ST_VCORNER_BOTTOM, stCString("subsPlace"));
    params.SubtitlesTopDY   = new StFloat32Param(100.0f, stCString("subsTopDY"));
    params.SubtitlesTopDY->setMinMaxValues(0.0f, 400.0f);
    params.SubtitlesTopDY->setDefValue(100.0f);
    params.SubtitlesTopDY->setStep(5.0f);
    params.SubtitlesTopDY->setTolerance(0.1f);
    params.SubtitlesBottomDY= new StFloat32Param(100.0f, stCString("subsBottomDY"));
    params.SubtitlesBottomDY->setMinMaxValues(0.0f, 400.0f);
    params.SubtitlesBottomDY->setDefValue(100.0f);
    params.SubtitlesBottomDY->setStep(5.0f);
    params.SubtitlesBottomDY->setTolerance(0.1f);
    params.SubtitlesSize    = new StFloat32Param(28.0f, stCString("subsSize"));
    params.SubtitlesSize->setMinMaxValues(8.0f, 96.0f);
    params.SubtitlesSize->setDefValue(28.0f);
    params.SubtitlesSize->setStep(1.0f);
    params.SubtitlesSize->setTolerance(0.1f);
    params.SubtitlesParallax= new StFloat32Param(0.0f, stCString("subsParallax"));
    params.SubtitlesParallax->setMinMaxValues(-90.0f, 90.0f);
    params.SubtitlesParallax->setDefValue(0.0f);
    params.SubtitlesParallax->setStep(1.0f);
    params.SubtitlesParallax->setTolerance(0.1f);
    params.ToSearchSubs = new StBoolParamNamed(true, stCString("toSearchSubs"));
    params.SubtitlesParser = new StEnumParam(1, stCString("subsParser"));
    params.SubtitlesApplyStereo = new StBoolParamNamed(true, stCString("subsApplyStereo"));
    params.AudioAlDevice = new StALDeviceParam();
    params.AudioAlHrtf   = new StEnumParam(0, stCString("alHrtfRequest"));
    params.AudioGain = new StFloat32Param( 0.0f, // sound is unattenuated
                                         -50.0f, // almost mute
                                          10.0f, // max amplification
                                           0.0f, // default
                                           1.0f, // step
                                           0.1f);
    params.AudioGain->signals.onChanged = stSlot(this, &StMoviePlayer::doSetAudioVolume);
    params.AudioMute    = new StBoolParamNamed(false, stCString("muteAudio"));
    params.AudioMute->signals.onChanged = stSlot(this, &StMoviePlayer::doSetAudioMute);
    params.AudioDelay   = new StFloat32Param(0.0f, -5.0f, 5.0f, 0.0f, 0.100f);
    params.AudioDelay->signals.onChanged = stSlot(this, &StMoviePlayer::doSetAudioDelay);

    params.IsFullscreen     = new StBoolParamNamed(false, stCString("fullscreen"));
    params.IsFullscreen->signals.onChanged = stSlot(this, &StMoviePlayer::doFullscreen);
    params.ExitOnEscape     = new StEnumParam(ActionOnEscape_ExitOneClick, stCString("exitOnEscape"));
    params.ToRestoreRatio   = new StBoolParamNamed(false, stCString("toRestoreRatio"));
    params.IsShuffle        = new StBoolParamNamed(false, stCString("shuffle"));
    params.ToLoopSingle     = new StBoolParamNamed(false, stCString("loopSingle"));
    params.AreGlobalMKeys   = new StBoolParamNamed(true,  stCString("globalMediaKeys"));
    params.CheckUpdatesDays = new StEnumParam(StCheckUpdates::UpdateInteval_EveryWeek, stCString("updatesIntervalEnum"));
    params.LastUpdateDay    = new StInt32ParamNamed(0, stCString("updatesLastCheck"));
    params.SrcStereoFormat  = new StInt32ParamNamed(StFormat_AUTO, stCString("srcFormat"));
    params.SrcStereoFormat->signals.onChanged = stSlot(this, &StMoviePlayer::doSwitchSrcFormat);
    params.ToShowPlayList   = new StBoolParamNamed(false, stCString("showPlaylist"));
    params.ToShowPlayList->signals.onChanged = stSlot(this, &StMoviePlayer::doShowPlayList);
    params.ToShowAdjustImage = new StBoolParamNamed(false, stCString("showAdjustImage"));
    params.ToShowAdjustImage->signals.onChanged = stSlot(this, &StMoviePlayer::doShowAdjustImage);
    params.ToSwapJPS  = new StBoolParamNamed(false, stCString("toSwapJPS"));
    params.ToSwapJPS->signals.onChanged = stSlot(this, &StMoviePlayer::doChangeSwapJPS);
    params.ToStickPanorama  = new StBoolParamNamed(false, stCString("toStickPano360"));
    params.ToStickPanorama->signals.onChanged = stSlot(this, &StMoviePlayer::doChangeStickPano360);
    params.ToTrackHead      = new StBoolParamNamed(true,  stCString("toTrackHead"));
    params.ToTrackHeadAudio = new StBoolParamNamed(true,  stCString("toTrackHeadAudio"));
    params.ToForceBFormat   = new StBoolParamNamed(false, stCString("toForceBFormat"));
    params.ToShowFps   = new StBoolParamNamed(false, stCString("toShowFps"));
    params.ToShowMenu  = new StBoolParamNamed(true,  stCString("toShowMenu"));
    params.ToShowTopbar= new StBoolParamNamed(true,  stCString("toShowTopbar"));
    params.ToShowBottom= new StBoolParamNamed(true,  stCString("toShowBottom"));
    params.ToMixImagesVideos = new StBoolParamNamed(false,  stCString("toMixImagesVideos"));
    params.ToMixImagesVideos->signals.onChanged = stSlot(this, &StMoviePlayer::doChangeMixImagesVideos);
    params.SlideShowDelay = new StFloat32Param(4.0f, stCString("slideShowDelay2"));
    params.SlideShowDelay->setMinMaxValues(1.0f, 10.0f);
    params.SlideShowDelay->setDefValue(4.0f);
    params.SlideShowDelay->setStep(1.0f);
    params.SlideShowDelay->setTolerance(0.1f);
    params.SlideShowDelay->setFormat(stCString("%01.1f s"));
    params.IsMobileUI  = new StBoolParamNamed(StWindow::isMobile(), stCString("isMobileUI"));
    params.IsMobileUI->signals.onChanged = stSlot(this, &StMoviePlayer::doChangeMobileUI);
    params.IsMobileUISwitch = new StBoolParam(params.IsMobileUI->getValue());
    params.IsMobileUISwitch->signals.onChanged = stSlot(this, &StMoviePlayer::doScaleHiDPI);
    params.IsExclusiveFullScreen = new StBoolParamNamed(false, stCString("isExclusiveFullScreen"));
    params.IsVSyncOn   = new StBoolParamNamed(true, stCString("vsync"));
    params.IsVSyncOn->signals.onChanged = stSlot(this, &StMoviePlayer::doSwitchVSync);
    StApplication::params.VSyncMode->setValue(StGLContext::VSync_ON);
    params.ToLimitFps       = new StBoolParamNamed(true, stCString("toLimitFps"));
    params.ToSmoothUploads  = new StBoolParamNamed(true, stCString("toSmoothUploads"));
    params.StartWebUI       = new StEnumParam(WEBUI_OFF, stCString("webuiOn"));
    params.ToPrintWebErrors = new StBoolParamNamed(true,  stCString("webuiShowErrors"));
    params.IsLocalWebUI     = new StBoolParamNamed(false, stCString("isLocalWebUI"));
    params.WebUIPort        = new StInt32ParamNamed(8080, stCString("webuiPort"));
    params.AudioStream = new StInt32Param(-1);
    params.AudioStream->signals.onChanged = stSlot(this, &StMoviePlayer::doSwitchAudioStream);
    params.SubtitlesStream = new StInt32Param(-1);
    params.SubtitlesStream->signals.onChanged = stSlot(this, &StMoviePlayer::doSwitchSubtitlesStream);
    params.BlockSleeping = new StEnumParam(StMoviePlayer::BLOCK_SLEEP_PLAYBACK, stCString("blockSleep"));
    params.ToHideStatusBar = new StBoolParamNamed(true, stCString("toHideStatusBar"));
    params.ToHideStatusBar->signals.onChanged = stSlot(this, &StMoviePlayer::doHideSystemBars);
    params.ToHideNavBar    = new StBoolParamNamed(true, stCString("toHideNavBar"));
    params.ToHideNavBar   ->signals.onChanged = stSlot(this, &StMoviePlayer::doHideSystemBars);
    params.ToOpenLast    = new StBoolParamNamed(false, stCString("toOpenLast"));
    params.ToShowExtra   = new StBoolParamNamed(false, stCString("experimental"));
    // set rendering FPS as twice as average video FPS
    params.TargetFps = new StInt32ParamNamed(2, stCString("fpsTarget"));
    params.UseGpu = new StBoolParamNamed(false, stCString("gpuDecoding"));
    // OpenJPEG seems to be faster then built-in jpeg2000 decoder
    params.UseOpenJpeg = new StBoolParamNamed(true, stCString("openJpeg"));
    params.SnapshotImgType = new StInt32ParamNamed(StImageFile::ST_TYPE_JPEG, stCString("snapImgType"));
    params.Benchmark = new StBoolParamNamed(false, stCString("benchmark"));
    params.Benchmark->signals.onChanged = stSlot(this, &StMoviePlayer::doSetBenchmark);

    updateStrings();

    // load settings
    mySettings->loadParam(params.ExitOnEscape);
    mySettings->loadParam(params.ScaleAdjust);
    params.ScaleAdjust->signals.onChanged  = stSlot(this, &StMoviePlayer::doScaleGui);
    mySettings->loadParam(params.ScaleHiDPI2X);
    params.ScaleHiDPI2X->signals.onChanged = stSlot(this, &StMoviePlayer::doScaleHiDPI);

    mySettings->loadParam (params.TargetFps);
    mySettings->loadString(ST_SETTING_LAST_FOLDER,        params.lastFolder);
    mySettings->loadParam (params.LastUpdateDay);
    mySettings->loadParam (params.CheckUpdatesDays);
    mySettings->loadParam (params.IsShuffle);
    mySettings->loadParam (params.ToLoopSingle);
    mySettings->loadParam (params.AreGlobalMKeys);
    mySettings->loadParam (params.ToShowPlayList);
    mySettings->loadParam (params.ToShowAdjustImage);
    mySettings->loadParam (params.SubtitlesPlace);
    mySettings->loadParam (params.SubtitlesTopDY);
    mySettings->loadParam (params.SubtitlesBottomDY);
    mySettings->loadParam (params.SubtitlesSize);
    mySettings->loadParam (params.SubtitlesParallax);
    mySettings->loadParam (params.SubtitlesParser);
    mySettings->loadParam (params.SubtitlesApplyStereo);
    mySettings->loadParam (params.ToSearchSubs);

    myToCheckPoorOrient = !mySettings->loadParam(params.ToTrackHead);
    mySettings->loadParam (params.ToSwapJPS);
    mySettings->loadParam (params.ToStickPanorama);
    mySettings->loadParam (params.ToTrackHeadAudio);
    mySettings->loadParam (params.ToForceBFormat);
    mySettings->loadParam (params.AudioAlHrtf);
    mySettings->loadParam (params.ToShowFps);
    mySettings->loadParam (params.SlideShowDelay);
    mySettings->loadParam (params.ToMixImagesVideos);
    mySettings->loadParam (params.IsMobileUI);
    mySettings->loadParam (params.IsExclusiveFullScreen);
    mySettings->loadParam (params.IsVSyncOn);
    mySettings->loadParam (params.ToLimitFps);
    mySettings->loadParam (params.ToSmoothUploads);
    mySettings->loadParam (params.UseGpu);
    mySettings->loadParam (params.UseOpenJpeg);

    mySettings->loadParam (params.StartWebUI);
    mySettings->loadParam (params.WebUIPort);
    mySettings->loadParam (params.ToPrintWebErrors);
    mySettings->loadParam (params.SnapshotImgType);
    mySettings->loadParam (params.BlockSleeping);
    mySettings->loadParam (params.ToHideStatusBar);
    mySettings->loadParam (params.ToHideNavBar);
    mySettings->loadParam (params.ToOpenLast);
    mySettings->loadParam (params.ToShowExtra);
    if(params.StartWebUI->getValue() == WEBUI_ONCE) {
        params.StartWebUI->setValue(WEBUI_OFF);
    }

    StString aSavedALDevice;
    mySettings->loadString(params.AudioAlDevice->getKey(), aSavedALDevice);
    params.AudioAlDevice->init(aSavedALDevice);

    params.IsShuffle    ->signals.onChanged.connect(this, &StMoviePlayer::doSwitchShuffle);
    params.ToLoopSingle ->signals.onChanged.connect(this, &StMoviePlayer::doSwitchLoopSingle);
    params.AudioAlDevice->signals.onChanged.connect(this, &StMoviePlayer::doSwitchAudioDevice);
    params.AudioAlHrtf  ->signals.onChanged.connect(this, &StMoviePlayer::doSwitchAudioAlHrtf);
    params.ToForceBFormat->signals.onChanged = stSlot(this, &StMoviePlayer::doSetForceBFormat);

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

    anAction = new StActionIntSlot(stCString("DoQuit"), stSlot(this, &StMoviePlayer::doQuit), 0);
    addAction(Action_Quit, anAction, 0);

    anAction = new StActionBool(stCString("DoFullscreen"), params.IsFullscreen);
    addAction(Action_Fullscreen, anAction, ST_VK_F, ST_VK_RETURN);

    anAction = new StActionBool(stCString("DoShowFPS"), params.ToShowFps);
    addAction(Action_ShowFps, anAction, ST_VK_F12);

    anAction = new StActionIntSlot(stCString("DoShowGUI"), stSlot(this, &StMoviePlayer::doShowHideGUI), 0);
    addAction(Action_ShowGUI, anAction, ST_VK_TILDE);

    anAction = new StActionIntValue(stCString("DoSrcAuto"), params.SrcStereoFormat, StFormat_AUTO);
    addAction(Action_SrcAuto, anAction, ST_VK_A);

    anAction = new StActionIntValue(stCString("DoSrcMono"), params.SrcStereoFormat, StFormat_Mono);
    addAction(Action_SrcMono, anAction, ST_VK_M);

    anAction = new StActionIntValue(stCString("DoSrcOverUnder"), params.SrcStereoFormat, StFormat_TopBottom_LR);
    addAction(Action_SrcOverUnderLR, anAction, ST_VK_O);

    anAction = new StActionIntValue(stCString("DoSrcSideBySide"), params.SrcStereoFormat, StFormat_SideBySide_RL);
    addAction(Action_SrcSideBySideRL, anAction, ST_VK_S);

    anAction = new StActionIntSlot(stCString("DoFileInfo"), stSlot(this, &StMoviePlayer::doAboutFile), 0);
    addAction(Action_FileInfo, anAction, ST_VK_I);

    anAction = new StActionIntSlot(stCString("DoListFirst"), stSlot(this, &StMoviePlayer::doListFirst), 0);
    addAction(Action_ListFirst, anAction, ST_VK_HOME);

    anAction = new StActionIntSlot(stCString("DoListLast"), stSlot(this, &StMoviePlayer::doListLast), 0);
    addAction(Action_ListLast, anAction, ST_VK_END);

    anAction = new StActionIntSlot(stCString("DoListPrev"), stSlot(this, &StMoviePlayer::doListPrev), 0);
    addAction(Action_ListPrev, anAction, ST_VK_PRIOR);

    anAction = new StActionIntSlot(stCString("DoListNext"), stSlot(this, &StMoviePlayer::doListNext), 0);
    addAction(Action_ListNext, anAction, ST_VK_NEXT);

    anAction = new StActionIntSlot(stCString("DoListPrevExt"), stSlot(this, &StMoviePlayer::doListPrev), 0);
    addAction(Action_ListPrevExt, anAction, ST_VK_MEDIA_PREV_TRACK, ST_VK_BROWSER_BACK);

    anAction = new StActionIntSlot(stCString("DoListNextExt"), stSlot(this, &StMoviePlayer::doListNext), 0);
    addAction(Action_ListNextExt, anAction, ST_VK_MEDIA_NEXT_TRACK, ST_VK_BROWSER_FORWARD);

    anAction = new StActionIntSlot(stCString("DoPlayPause"), stSlot(this, &StMoviePlayer::doPlayPause), 0);
    addAction(Action_PlayPause, anAction, ST_VK_SPACE, ST_VK_MEDIA_PLAY_PAUSE);

    anAction = new StActionIntSlot(stCString("DoStop"), stSlot(this, &StMoviePlayer::doStop), 0);
    addAction(Action_Stop, anAction, ST_VK_MEDIA_STOP);

    anAction = new StActionIntSlot(stCString("DoSeekLeft"), stSlot(this, &StMoviePlayer::doSeekLeft), 0);
    addAction(Action_SeekLeft5, anAction, ST_VK_LEFT);

    anAction = new StActionIntSlot(stCString("DoSeekRight"), stSlot(this, &StMoviePlayer::doSeekRight), 0);
    addAction(Action_SeekRight5, anAction, ST_VK_RIGHT);

    anAction = new StActionIntSlot(stCString("DoOpen1File"), stSlot(this, &StMoviePlayer::doOpen1FileAction), 0);
#ifdef __APPLE__
    addAction(Action_Open1File, anAction, ST_VK_O | ST_VF_CONTROL, ST_VK_O | ST_VF_COMMAND);
#else
    addAction(Action_Open1File, anAction, ST_VK_O | ST_VF_CONTROL);
#endif

    anAction = new StActionIntSlot(stCString("DoSnapshot"), stSlot(this, &StMoviePlayer::doSnapshot), StImageFile::ST_TYPE_NONE);
#ifdef __APPLE__
    addAction(Action_SaveSnapshot, anAction, ST_VK_S | ST_VF_CONTROL, ST_VK_S | ST_VF_COMMAND);
#else
    addAction(Action_SaveSnapshot, anAction, ST_VK_S | ST_VF_CONTROL);
#endif

    anAction = new StActionIntSlot(stCString("DoDeleteFile"), stSlot(this, &StMoviePlayer::doDeleteFileBegin), 0);
    addAction(Action_DeleteFile, anAction, ST_VK_DELETE | ST_VF_SHIFT);

    anAction = new StActionBool(stCString("DoAudioMute"), params.AudioMute);
    addAction(Action_AudioMute, anAction);

    anAction = new StActionIntSlot(stCString("DoAudioDecrease"), stSlot(this, &StMoviePlayer::doAudioVolume), (size_t )-1);
    addAction(Action_AudioDecrease, anAction, ST_VK_DOWN);

    anAction = new StActionIntSlot(stCString("DoAudioIncrease"), stSlot(this, &StMoviePlayer::doAudioVolume), 1);
    addAction(Action_AudioIncrease, anAction, ST_VK_UP);

    anAction = new StActionIntSlot(stCString("DoAudioNext"), stSlot(this, &StMoviePlayer::doAudioNext), 1);
    addAction(Action_AudioNext, anAction, ST_VK_H, ST_VK_L);

    anAction = new StActionIntSlot(stCString("DoAudioPrev"), stSlot(this, &StMoviePlayer::doAudioNext), (size_t )-1);
    addAction(Action_AudioPrev, anAction, ST_VK_H | ST_VF_SHIFT, ST_VK_L | ST_VF_SHIFT);

    anAction = new StActionIntSlot(stCString("DoSubtitlesNext"), stSlot(this, &StMoviePlayer::doSubtitlesNext), 1);
    addAction(Action_SubsNext, anAction, ST_VK_U, ST_VK_T);

    anAction = new StActionIntSlot(stCString("DoSubtitlesPrev"), stSlot(this, &StMoviePlayer::doSubtitlesNext), (size_t )-1);
    addAction(Action_SubsPrev, anAction, ST_VK_U | ST_VF_SHIFT, ST_VK_T | ST_VF_SHIFT);

    anAction = new StActionIntSlot(stCString("DoSubtitlesCopy"), stSlot(this, &StMoviePlayer::doSubtitlesCopy), 0);
#ifdef __APPLE__
    addAction(Action_CopyToClipboard, anAction, ST_VK_C | ST_VF_CONTROL, ST_VK_C      | ST_VF_COMMAND);
#else
    addAction(Action_CopyToClipboard, anAction, ST_VK_C | ST_VF_CONTROL, ST_VK_INSERT | ST_VF_CONTROL);
#endif

    anAction = new StActionIntSlot(stCString("DoOpenFromClipboard"), stSlot(this, &StMoviePlayer::doFromClipboard), 0);
#ifdef __APPLE__
    addAction(Action_PasteFromClipboard, anAction, ST_VK_V | ST_VF_CONTROL, ST_VK_V      | ST_VF_COMMAND);
#else
    addAction(Action_PasteFromClipboard, anAction, ST_VK_V | ST_VF_CONTROL, ST_VK_INSERT | ST_VF_SHIFT);
#endif

    anAction = new StActionIntSlot(stCString("DoPlayListReverse"), stSlot(this, &StMoviePlayer::doPlayListReverse), 0);
    addAction(Action_ShowList, anAction, ST_VK_L | ST_VF_CONTROL);

    anAction = new StActionIntSlot(stCString("DoImageAdjustReset"), stSlot(this, &StMoviePlayer::doImageAdjustReset), 0);
    addAction(Action_ImageAdjustReset, anAction);

    anAction = new StActionIntSlot(stCString("DoPanoramaOnOff"), stSlot(this, &StMoviePlayer::doPanoramaOnOff), 0);
    addAction(Action_PanoramaOnOff, anAction, ST_VK_P);

    {
    anAction = new StActionIntSlot(stCString("DoOutStereoNormal"), stSlot(this, &StMoviePlayer::doSetStereoOutput), StGLImageRegion::MODE_STEREO);
    addAction(Action_OutStereoNormal, anAction);

    anAction = new StActionIntSlot(stCString("DoOutStereoLeftView"), stSlot(this, &StMoviePlayer::doSetStereoOutput), StGLImageRegion::MODE_ONLY_LEFT);
    addAction(Action_OutStereoLeftView, anAction);

    anAction = new StActionIntSlot(stCString("DoOutStereoRightView"), stSlot(this, &StMoviePlayer::doSetStereoOutput), StGLImageRegion::MODE_ONLY_RIGHT);
    addAction(Action_OutStereoRightView, anAction);

    anAction = new StActionIntSlot(stCString("DoOutStereoParallelPair"), stSlot(this, &StMoviePlayer::doSetStereoOutput), StGLImageRegion::MODE_PARALLEL);
    addAction(Action_OutStereoParallelPair, anAction);

    anAction = new StActionIntSlot(stCString("DoOutStereoCrossEyed"), stSlot(this, &StMoviePlayer::doSetStereoOutput), StGLImageRegion::MODE_CROSSYED);
    addAction(Action_OutStereoCrossEyed, anAction);
    }
}

bool StMoviePlayer::resetDevice() {
    if(myGUI.isNull()
    || myVideo.isNull()) {
        return init();
    }

    // be sure Render plugin process quit correctly
    myWindow->beforeClose();

    releaseDevice();
    myWindow->close();
    myWindow.nullify();
    return open();
}

void StMoviePlayer::saveGuiParams() {
    if(myGUI.isNull()) {
        return;
    }

    mySettings->saveParam (myGUI->myImage->params.DisplayMode);
    mySettings->saveInt32 (ST_SETTING_GAMMA,       stRound(100.0f * myGUI->myImage->params.Gamma->getValue()));
    mySettings->saveParam (myGUI->myImage->params.ToHealAnamorphicRatio);
    mySettings->saveInt32(myGUI->myImage->params.DisplayRatio->getKey(),
                          params.ToRestoreRatio->getValue()
                        ? myGUI->myImage->params.DisplayRatio->getValue()
                        : StGLImageRegion::RATIO_AUTO);
    mySettings->saveParam (myGUI->myImage->params.TextureFilter);
}

void StMoviePlayer::saveAllParams() {
    saveGuiParams();
    if(!myGUI.isNull()) {
        mySettings->saveParam (params.ExitOnEscape);
        mySettings->saveParam (params.ScaleAdjust);
        mySettings->saveParam (params.ScaleHiDPI2X);
        mySettings->saveParam (params.SubtitlesPlace);
        mySettings->saveParam (params.SubtitlesTopDY);
        mySettings->saveParam (params.SubtitlesBottomDY);
        mySettings->saveParam (params.SubtitlesSize);
        mySettings->saveParam (params.SubtitlesParallax);
        mySettings->saveParam (params.SubtitlesParser);
        mySettings->saveParam (params.SubtitlesApplyStereo);
        mySettings->saveParam (params.ToSearchSubs);
        mySettings->saveParam (params.TargetFps);
        mySettings->saveString(params.AudioAlDevice->getKey(), params.AudioAlDevice->getUtfTitle());
        mySettings->saveParam (params.AudioAlHrtf);
        mySettings->saveParam (params.LastUpdateDay);
        mySettings->saveParam (params.CheckUpdatesDays);
        mySettings->saveParam (params.SrcStereoFormat);
        mySettings->saveParam (params.IsShuffle);
        mySettings->saveParam (params.ToLoopSingle);
        mySettings->saveParam (params.AreGlobalMKeys);
        mySettings->saveParam (params.ToShowPlayList);
        mySettings->saveParam (params.ToShowAdjustImage);

        mySettings->saveParam (params.ToSwapJPS);
        mySettings->saveParam (params.ToStickPanorama);
        mySettings->saveParam (params.ToTrackHead);
        mySettings->saveParam (params.ToTrackHeadAudio);
        mySettings->saveParam (params.ToForceBFormat);
        mySettings->saveParam (params.ToShowFps);
        mySettings->saveParam (params.SlideShowDelay);
        mySettings->saveParam (params.ToMixImagesVideos);
        mySettings->saveParam (params.IsMobileUI);
        mySettings->saveParam (params.IsExclusiveFullScreen);
        mySettings->saveParam (params.IsVSyncOn);
        mySettings->saveParam (params.ToLimitFps);
        mySettings->saveParam (params.ToSmoothUploads);
        mySettings->saveParam (params.UseGpu);
        mySettings->saveParam (params.UseOpenJpeg);
        if(!params.IsLocalWebUI->getValue()) {
            mySettings->saveParam(params.WebUIPort);
            mySettings->saveParam(params.StartWebUI);
        }
        mySettings->saveParam (params.ToPrintWebErrors);
        mySettings->saveParam (params.SnapshotImgType);
        mySettings->saveParam (params.BlockSleeping);
        mySettings->saveParam (params.ToHideStatusBar);
        mySettings->saveParam (params.ToHideNavBar);
        mySettings->saveParam (params.ToOpenLast);
        mySettings->saveParam (params.ToShowExtra);

        // store hot-keys
        for(std::map< int, StHandle<StAction> >::iterator anIter = myActions.begin();
            anIter != myActions.end(); ++anIter) {
            mySettings->saveHotKey(anIter->second);
        }
    }
    myPlayList->currentToRecent();
    mySettings->saveString(ST_SETTING_RECENT_FILES, myPlayList->dumpRecentList());
    mySettings->flush();
}

void StMoviePlayer::releaseDevice() {
    saveAllParams();

    // release GUI data and GL resources before closing the window
    myKeyActions.clear();
    myGUI.nullify();
    myContext.nullify();
}

StMoviePlayer::~StMoviePlayer() {
    doStopWebUI();

    myUpdates.nullify();
    if(!myVideo.isNull()) {
        myVideo->startDestruction();
    }

    releaseDevice();
    // wait video playback thread to quit and release resources
    myVideo.nullify();
}

bool StMoviePlayer::createGui(StHandle<StGLTextureQueue>& theTextureQueue,
                              StHandle<StSubQueue>&       theSubQueue) {
    if(!myGUI.isNull()) {
        saveGuiParams();
        myGUI.nullify();
        myKeyActions.clear();
    }

    if(!myVideo.isNull()) {
        theTextureQueue = myVideo->getTextureQueue();
        theSubQueue     = myVideo->getSubtitlesQueue();
    } else {
        theTextureQueue = new StGLTextureQueue(16);
        theSubQueue     = new StSubQueue();
    }

    params.ScaleHiDPI->setValue(myWindow->getScaleFactor());
    doChangeMobileUI(params.IsMobileUI->getValue());
    myGUI = new StMoviePlayerGUI(this, myWindow.access(), myLangMap.access(), myPlayList, theTextureQueue, theSubQueue);
    myGUI->setContext(myContext);
    theTextureQueue->setDeviceCaps(myContext->getDeviceCaps());

    // load settings
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

void StMoviePlayer::doChangeLanguage(const int32_t theNewLang) {
    StApplication::doChangeLanguage(theNewLang);
    StMoviePlayerStrings::loadDefaults(*myLangMap);
    updateStrings();
}

void StMoviePlayer::doImageAdjustReset(const size_t ) {
    if(myGUI.isNull()) {
        return;
    }

    myGUI->myImage->params.Gamma     ->reset();
    myGUI->myImage->params.Brightness->reset();
    myGUI->myImage->params.Saturation->reset();
}


void StMoviePlayer::doDeleteFileBegin(const size_t ) {
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
    const StString aText = myLangMap->getValue(StMoviePlayerStrings::DIALOG_DELETE_FILE_QUESTION)
                         + (isReadOnly ? "\nWARNING! The file is READ ONLY!" : "")
                         + "\n" + myFileToDelete->getPath();

    StGLMessageBox* aDialog = new StGLMessageBox(myGUI.access(), myLangMap->getValue(StMoviePlayerStrings::DIALOG_DELETE_FILE_TITLE),
                                                 aText, myGUI->scale(512), myGUI->scale(256));
    aDialog->addButton(myLangMap->getValue(StMoviePlayerStrings::BUTTON_DELETE), true)->signals.onBtnClick += stSlot(this, &StMoviePlayer::doDeleteFileEnd);
    aDialog->addButton(myLangMap->getValue(StMoviePlayerStrings::BUTTON_CANCEL), false);
    aDialog->stglInit();
}

void StMoviePlayer::doDeleteFileEnd(const size_t ) {
    if(myFileToDelete.isNull()
    || myVideo.isNull()) {
        return;
    }

    StFileNode::removeReadOnlyFlag(myFileToDelete->getPath());
    myVideo->doRemovePhysically(myFileToDelete);
    myFileToDelete.nullify();
}

void StMoviePlayer::doStopWebUI() {
#ifdef ST_HAVE_MONGOOSE
    if(myWebCtx != NULL) {
        mg_stop(myWebCtx);
        myWebCtx = NULL;
    }
#endif
}

void StMoviePlayer::doStartWebUI() {
#ifdef ST_HAVE_MONGOOSE
    if(params.StartWebUI->getValue() == WEBUI_OFF
    || myWebCtx != NULL) {
        return;
    }

    mg_callbacks aCallbacks;
    stMemZero(&aCallbacks, sizeof(aCallbacks));
    aCallbacks.begin_request = StMoviePlayer::beginRequestHandler;
    const StString aPort = params.WebUIPort->getValue();

    StString aControlList = "+0.0.0.0/0";
    if(params.IsLocalWebUI->getValue()) {
        aControlList = "-0.0.0.0/0,+127.0.0.0/16";
    }
    const char* anOptions[] = { "listening_ports",     aPort.toCString(),
                                "access_control_list", aControlList.toCString(),
                                NULL };
    myWebCtx = mg_start(&aCallbacks, this, anOptions);
    if(myWebCtx == NULL
    && params.ToPrintWebErrors->getValue()) {
        myMsgQueue->pushError(tr(WEBUI_ERROR_PORT_BUSY).format(aPort));
    }
#endif
}

void StMoviePlayer::doSwitchWebUI(const int32_t theValue) {
#ifdef ST_HAVE_MONGOOSE
    switch(theValue) {
        case WEBUI_ONCE:
        case WEBUI_AUTO: {
            doStartWebUI();
            break;
        }
        case WEBUI_OFF:
        default: {
            doStopWebUI();
            break;
        }
    }
#endif
}

bool StMoviePlayer::init() {
    const bool isReset = !myVideo.isNull();
    if(!myContext.isNull()
    && !myGUI.isNull()) {
        return true;
    }

    // initialize GL context
    myContext = myWindow->getContext();
    myContext->setMessagesQueue(myMsgQueue);
    if(!myContext->isGlGreaterEqual(2, 0)) {
        myMsgQueue->pushError(stCString("OpenGL 2.0 is required by Movie Player!"));
        myMsgQueue->popAll();
        return false;
    }

    // load hot-keys
    if(!isReset) {
    #ifdef ST_HAVE_MONGOOSE
        // handle this argument in advance
        if(!myOpenFileInfo.isNull()) {
            StArgument anArgWebuiCmd = myOpenFileInfo->getArgumentsMap()[ST_SETTING_WEBUI_CMDPORT];
            if(anArgWebuiCmd.isValid()) {
                params.IsLocalWebUI->setValue(true);
                params.WebUIPort->setValue(::atol(anArgWebuiCmd.getValue().toCString()));
                params.StartWebUI->setValue(WEBUI_ONCE);
            }
        }
    #endif

        for(std::map< int, StHandle<StAction> >::iterator anIter = myActions.begin();
            anIter != myActions.end(); ++anIter) {
            mySettings->loadHotKey(anIter->second);
        }
    }

    // create the GUI with default values
    StHandle<StGLTextureQueue> aTextureQueue;
    StHandle<StSubQueue>       aSubQueue;
    if(!createGui(aTextureQueue, aSubQueue)) {
        myMsgQueue->pushError(stCString("Movie Player - critical error:\n"
                                        "Frame region initialization failed!"));
        myMsgQueue->popAll();
        myGUI.nullify();
        return false;
    }

    // capture multimedia keys even without window focus
    myWindow->setAttribute(StWinAttr_GlobalMediaKeys, params.AreGlobalMKeys->getValue());
    myWindow->setHideSystemBars(params.ToHideStatusBar->getValue(), params.ToHideNavBar->getValue());

    // create the video playback thread
    if(!isReset) {
        myVideo = new StVideo(params.AudioAlDevice->getCTitle(), (StAudioQueue::StAlHrtfRequest )params.AudioAlHrtf->getValue(),
                              myResMgr, myLangMap, myPlayList, aTextureQueue, aSubQueue);
        myVideo->signals.onError  = stSlot(myMsgQueue.access(), &StMsgQueue::doPushError);
        myVideo->signals.onLoaded = stSlot(this,                &StMoviePlayer::doLoaded);
        myVideo->params.UseGpu       = params.UseGpu;
        myVideo->params.UseOpenJpeg  = params.UseOpenJpeg;
        myVideo->params.ToSearchSubs = params.ToSearchSubs;
        myVideo->params.ToTrackHeadAudio = params.ToTrackHeadAudio;
        myVideo->params.SlideShowDelay = params.SlideShowDelay;
        myVideo->setSwapJPS(params.ToSwapJPS->getValue());
        myVideo->setStickPano360(params.ToStickPanorama->getValue());
        myVideo->setForceBFormat(params.ToForceBFormat->getValue());
        doChangeMixImagesVideos(params.ToMixImagesVideos->getValue());

    #ifdef ST_HAVE_MONGOOSE
        doStartWebUI();
        params.StartWebUI->signals.onChanged += stSlot(this, &StMoviePlayer::doSwitchWebUI);
    #endif
    }

    myPlayList->setShuffle   (params.IsShuffle   ->getValue());
    myPlayList->setLoopSingle(params.ToLoopSingle->getValue());

    StString aRecentList;
    mySettings->loadString(ST_SETTING_RECENT_FILES, aRecentList);
    myPlayList->loadRecentList(aRecentList);

    if(isReset) {
        if(params.IsFullscreen->getValue()) {
            myWindow->setFullScreen(true);
        }
        return true;
    }

    // load this parameter AFTER video thread creation
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

void StMoviePlayer::parseArguments(const StArgumentsMap& theArguments) {
    StArgument anArgViewMode   = theArguments[ST_SETTING_VIEWMODE];
    StArgument anArgSrcFormat  = theArguments[params.SrcStereoFormat->getKey()];
    StArgument anArgShuffle    = theArguments[params.IsShuffle->getKey()];
    StArgument anArgLoopSingle = theArguments[params.ToLoopSingle->getKey()];
    StArgument anArgBenchmark  = theArguments[params.Benchmark->getKey()];
    StArgument anArgShowMenu   = theArguments[params.ToShowMenu->getKey()];
    StArgument anArgShowTopbar = theArguments[params.ToShowTopbar->getKey()];
    StArgument anArgMixImages  = theArguments[params.ToMixImagesVideos->getKey()];

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

    if(anArgViewMode.isValid()) {
        myPlayList->changeDefParams().ViewingMode = StStereoParams::GET_VIEW_MODE_FROM_STRING(anArgViewMode.getValue());
    }
    if(anArgSrcFormat.isValid()) {
        params.SrcStereoFormat->setValue(st::formatFromString(anArgSrcFormat.getValue()));
    }
    if(anArgShuffle.isValid()) {
        params.IsShuffle->setValue(!anArgShuffle.isValueOff());
    }
    if(anArgLoopSingle.isValid()) {
        params.ToLoopSingle->setValue(!anArgLoopSingle.isValueOff());
    }
    if(anArgBenchmark.isValid()) {
        params.Benchmark->setValue(!anArgBenchmark.isValueOff());
    }
    if(anArgShowMenu.isValid()) {
        params.ToShowMenu->setValue(!anArgShowMenu.isValueOff());
    }
    if(anArgShowTopbar.isValid()) {
        params.ToShowTopbar->setValue(!anArgShowTopbar.isValueOff());
    }
    if(anArgMixImages.isValid()) {
        params.ToMixImagesVideos->setValue(!anArgMixImages.isValueOff());
    }
}

bool StMoviePlayer::open() {
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
        //myVideo->doLoadNext();
        doUpdateStateLoaded();
        return true;
    }

    parseArguments(myOpenFileInfo->getArgumentsMap());
    const StMIME     anOpenMIME  = myOpenFileInfo->getMIME();
    const StArgument anArgPause  = myOpenFileInfo->getArgumentsMap()[ST_ARGUMENT_FILE_PAUSE];
    const StArgument anArgPaused = myOpenFileInfo->getArgumentsMap()[ST_ARGUMENT_FILE_PAUSED];
    const StArgument anArgLast   = myOpenFileInfo->getArgumentsMap()[ST_ARGUMENT_FILE_LAST];
    const StArgument anArgFileDemo = myOpenFileInfo->getArgumentsMap()[ST_ARGUMENT_FILE_DEMO];
    const bool       isPaused    = (anArgPause .isValid() && !anArgPause .isValueOff())
                                || (anArgPaused.isValid() && !anArgPaused.isValueOff());
    const bool toOpenLast = anArgLast.isValid() ? !anArgLast.isValueOff() : params.ToOpenLast->getValue();
    if(myOpenFileInfo->getPath().isEmpty() || (toOpenLast && anArgFileDemo.isValid())) {
        // open drawer without files
        if(toOpenLast) {
            doOpenRecent(0); // open last opened file
            if(isPaused) {
                myVideo->pushPlayEvent(ST_PLAYEVENT_PAUSE);
            } else if(!anArgLast.isValid() && !anArgPause.isValid() && !anArgPaused.isValid()) {
                myVideo->pushPlayEvent(ST_PLAYEVENT_PAUSE);
            }
        }
        return true;
    }

    // clear playlist first
    myPlayList->clear();

    //StArgument argFile1     = myOpenFileInfo->getArgumentsMap()[ST_ARGUMENT_FILE + 1]; // playlist?
    const StArgument argFileLeft  = myOpenFileInfo->getArgumentsMap()[ST_ARGUMENT_FILE_LEFT];
    const StArgument argFileRight = myOpenFileInfo->getArgumentsMap()[ST_ARGUMENT_FILE_RIGHT];
    const StArgument anArgSeek    = myOpenFileInfo->getArgumentsMap()[ST_ARGUMENT_FILE_SEEK];
    if(anArgSeek.isValid()) {
        StCLocale aCLocale;
        mySeekOnLoad = stStringToDouble(anArgSeek.getValue().toCString(), aCLocale);
    }
    if(argFileLeft.isValid() && argFileRight.isValid()) {
        // meta-file
        const size_t aRecent = myPlayList->findRecent(argFileLeft.getValue(), argFileRight.getValue());
        if(aRecent != size_t(-1)) {
            doOpenRecent(aRecent);
            if(isPaused) {
                myVideo->pushPlayEvent(ST_PLAYEVENT_PAUSE);
            }
            return true;
        }
        myPlayList->addOneFile(argFileLeft.getValue(), argFileRight.getValue());
    } else if(!anOpenMIME.isEmpty()) {
        // handle URI with #t=SECONDS tail
        StString aFilePath = myOpenFileInfo->getPath();
        StHandle< StArrayList<StString> > anUriParams = aFilePath.split('#', 2);
        if(StFileNode::isRemoteProtocolPath(aFilePath)
        && anUriParams->size() == 2) {
            aFilePath = anUriParams->getFirst();
            StString aParams = anUriParams->getLast();
            if(aParams.isStartsWith(stCString("t="))) {
                StCLocale aCLocale;
                mySeekOnLoad = stStringToDouble(aParams.toCString() + 2, aCLocale);
            }
        }

        // create just one-file playlist
        myPlayList->addOneFile(myOpenFileInfo->getPath(), anOpenMIME);
    } else {
        // handle URI with #t=SECONDS tail
        StString aFilePath = myOpenFileInfo->getPath();
        double aSeekPos = -1.0;
        StHandle< StArrayList<StString> > anUriParams = aFilePath.split('#', 2);
        if(StFileNode::isRemoteProtocolPath(aFilePath)
        && anUriParams->size() == 2) {
            aFilePath = anUriParams->getFirst();
            StString aParams = anUriParams->getLast();
            if(aParams.isStartsWith(stCString("t="))) {
                StCLocale aCLocale;
                aSeekPos = stStringToDouble(aParams.toCString() + 2, aCLocale);
            }
        }

        // create playlist from file's folder
        const size_t aRecent = myPlayList->findRecent(aFilePath);
        if(aRecent != size_t(-1)) {
            doOpenRecent(aRecent);
            if(aSeekPos >= 0.0) {
                mySeekOnLoad = aSeekPos;
            }
            if(isPaused) {
                myVideo->pushPlayEvent(ST_PLAYEVENT_PAUSE);
            }
            return true;
        }
        if(aSeekPos >= 0.0) {
            mySeekOnLoad = aSeekPos;
        }
        myPlayList->open(aFilePath);
    }

    if(!myPlayList->isEmpty()) {
        doUpdateStateLoading();
        myVideo->pushPlayEvent(ST_PLAYEVENT_RESUME);
        myVideo->doLoadNext();
        if(isPaused) {
            myVideo->pushPlayEvent(ST_PLAYEVENT_PAUSE);
        }
    }
    return true;
}

void StMoviePlayer::doResize(const StSizeEvent& ) {
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

void StMoviePlayer::doKeyDown(const StKeyEvent& theEvent) {
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
            if(!doExitOnEscape((ActionOnEscape )params.ExitOnEscape->getValue())) {
                if(myWindow->hasFullscreenMode()
                && myWindow->isFullScreen()) {
                    params.IsFullscreen->setValue(false);
                }
            }
            return;
        }
        case ST_VK_B: {
            if(theEvent.Flags == ST_VF_SHIFT) {
                myGUI->myImage->params.Brightness->increment();
            } else if(theEvent.Flags == ST_VF_CONTROL) {
                myGUI->myImage->params.Brightness->decrement();
            }
            return;
        }
        default: break;
    }
}

void StMoviePlayer::doKeyHold(const StKeyEvent& theEvent) {
    if(myGUI.isNull()) {
        return;
    }

    if(myGUI->getFocus() == NULL) {
        StApplication::doKeyHold(theEvent);
    } else {
        myGUI->doKeyHold(theEvent);
    }
}

void StMoviePlayer::doKeyUp(const StKeyEvent& theEvent) {
    if(myGUI.isNull()) {
        return;
    }

    myGUI->myImage->doKeyUp(theEvent);
    if(myGUI->getFocus() != NULL) {
        myGUI->doKeyUp(theEvent);
    }
}

void StMoviePlayer::doMouseDown(const StClickEvent& theEvent) {
    if(myGUI.isNull()) {
        return;
    }

    myGUI->tryClick(theEvent);
}

void StMoviePlayer::doMouseUp(const StClickEvent& theEvent) {
    if(myGUI.isNull()) {
        return;
    }

    const StPointD_t aPnt(theEvent.PointX, theEvent.PointY);
    const bool isNeutral = myGUI->getFocus() == NULL
                       || !myGUI->getFocus()->isPointIn(aPnt);
    if(!isNeutral) {
        myGUI->tryUnClick(theEvent);
        return;
    }

    switch(theEvent.Button) {
        case ST_MOUSE_MIDDLE: {
            params.IsFullscreen->reverse();
            break;
        }
        default: {
            myGUI->tryUnClick(theEvent);
            break;
        }
    }
}

void StMoviePlayer::doGesture(const StGestureEvent& theEvent) {
    if(!myGUI.isNull()) {
        myGUI->doGesture(theEvent);
    }
}

void StMoviePlayer::doScroll(const StScrollEvent& theEvent) {
    if(myGUI.isNull()) {
        return;
    }

    const StPointD_t aPnt(theEvent.PointX, theEvent.PointY);
    const bool isNeutral = myGUI->getFocus() == NULL
                       || !myGUI->getFocus()->isPointIn(aPnt);
    if(!isNeutral) {
        myGUI->doScroll(theEvent);
        return;
    }

    // limit seeking by scroll to lower corner
    if(myGUI->mySeekBar != NULL
    && myGUI->mySeekBar->isVisibleAndPointIn(aPnt)) {
        if(theEvent.StepsX >= 1) {
            doSeekRight();
        } else if(theEvent.StepsX <= -1) {
            doSeekLeft();
        }
        return;
    }
    myGUI->doScroll(theEvent);
}

void StMoviePlayer::doAudioVolume(size_t theDirection) {
    if(myVideo.isNull()) {
        return;
    }

    if(theDirection == 1) {
        params.AudioGain->increment();
    } else {
        params.AudioGain->decrement();
    }
}

void StMoviePlayer::doAudioNext(size_t theDirection) {
    if(myVideo.isNull()) {
        return;
    }

    const int32_t aValue = myVideo->params.activeAudio->nextValue(theDirection == 1 ? 1 : -1);
    params.AudioStream->setValue(aValue);
}

void StMoviePlayer::doSubtitlesNext(size_t theDirection) {
    if(myVideo.isNull()) {
        return;
    }

    const int32_t aValue = myVideo->params.activeSubtitles->nextValue(theDirection == 1 ? 1 : -1);
    params.SubtitlesStream->setValue(aValue);
}

void StMoviePlayer::doSubtitlesCopy(size_t ) {
    if(myVideo.isNull()
    || myGUI.isNull()
    || myGUI->mySubtitles == NULL) {
        return;
    }

    const StString& aText = myGUI->mySubtitles->getText();
    if(aText.isEmpty()) {
        return;
    }
    myWindow->toClipboard(aText);
}

void StMoviePlayer::doFromClipboard(size_t ) {
    if(myVideo.isNull()
    || myGUI.isNull()) {
        return;
    }

    StString aText;
    if(!myWindow->fromClipboard(aText)
    ||  aText.isEmpty()) {
        return;
    }

    if(!StFileNode::isAbsolutePath(aText)) {
        return;
    }

    // handle URI with #t=SECONDS tail
    double aSeekPos = -1.0;
    StHandle< StArrayList<StString> > anUriParams = aText.split('#', 2);
    if(StFileNode::isRemoteProtocolPath(aText)
    && anUriParams->size() == 2) {
        aText = anUriParams->getFirst();
        StString aParams = anUriParams->getLast();
        if(aParams.isStartsWith(stCString("t="))) {
            StCLocale aCLocale;
            aSeekPos = stStringToDouble(aParams.toCString() + 2, aCLocale);
        }
    }

    const size_t aRecent = myPlayList->findRecent(aText);
    if(aRecent != size_t(-1)) {
        doOpenRecent(aRecent);
        if(aSeekPos >= 0.0) {
            mySeekOnLoad = aSeekPos;
        }
        return;
    }

    if(aSeekPos >= 0.0) {
        mySeekOnLoad = aSeekPos;
    }
    myPlayList->open(aText);
    if(!myPlayList->isEmpty()) {
        doUpdateStateLoading();
        myVideo->pushPlayEvent(ST_PLAYEVENT_RESUME);
        myVideo->doLoadNext();
    }
}

void StMoviePlayer::doFileNext() {
    if(myVideo.isNull()) {
        return;
    }

    doUpdateStateLoading();
    myVideo->pushPlayEvent(ST_PLAYEVENT_RESUME);
    myVideo->doLoadNext();
}

void StMoviePlayer::doFileDrop(const StDNDropEvent& theEvent) {
    if(theEvent.NbFiles == 0) {
        return;
    }

    const StString aFile1 = theEvent.Files[0];
    const StString aFile2 = theEvent.NbFiles > 1 ? theEvent.Files[1] : "";
    const StString anExt1 = StFileNode::getExtension(aFile1);
    if(theEvent.NbFiles == 1
    || StFolder::isFolder(aFile1)) {
        // attach subtitle to currently opened item
        for(size_t anExtId = 0; anExtId < myVideo->getMimeListSubtitles().size(); ++anExtId) {
            if(!anExt1.isEqualsIgnoreCase(myVideo->getMimeListSubtitles()[anExtId].getExtension())) {
                continue;
            }

            StHandle<StFileNode> aCurrFile = myPlayList->getCurrentFile();
            if(aCurrFile.isNull()) {
                ST_ERROR_LOG("Can not attach subtitles");
                return;
            }

            myPlayList->addToNode(aCurrFile, aFile1);
            myAudioOnLoad = myVideo->params.activeAudio->getValue();
            mySubsOnLoad  = myVideo->params.activeSubtitles->getListSize();
            mySeekOnLoad  = myVideo->getPts();
            doUpdateStateLoading();
            myVideo->pushPlayEvent(ST_PLAYEVENT_RESUME);
            myVideo->doLoadNext();
            return;
        }

        if(!myPlayList->checkExtension(aFile1)
         && myVideo->getMimeListImages().checkExtension(anExt1)) {
            // redirect to StMoviePlayer
            myOpenFileOtherApp = new StOpenInfo();
            StArgumentsMap anArgs;
            anArgs.add(StArgument("in", "image"));
            myOpenFileOtherApp->setArgumentsMap(anArgs);
            myOpenFileOtherApp->setPath(aFile1);
            exit(0);
            return;
        }

        // just open the path
        const size_t aRecent = myPlayList->findRecent(aFile1);
        if(aRecent != size_t(-1)) {
            doOpenRecent(aRecent);
            return;
        }

        myPlayList->open(aFile1);
        doUpdateStateLoading();
        myVideo->pushPlayEvent(ST_PLAYEVENT_RESUME);
        myVideo->doLoadNext();
        return;
    } else if(theEvent.NbFiles == 2
          && !StFolder::isFolder(aFile2)
          &&  myPlayList->checkExtension(aFile2)) {
        // handle stereopair
        StString anExt2 = StFileNode::getExtension(aFile2);
        int aNbVideos = 0;
        for(size_t anExtId = 0; anExtId < myVideo->getMimeListVideo().size(); ++anExtId) {
            if(!anExt1.isEqualsIgnoreCase(myVideo->getMimeListVideo()[anExtId].getExtension())) {
                ++aNbVideos;
                break;
            }
        }
        for(size_t anExtId = 0; anExtId < myVideo->getMimeListVideo().size(); ++anExtId) {
            if(!anExt2.isEqualsIgnoreCase(myVideo->getMimeListVideo()[anExtId].getExtension())) {
                ++aNbVideos;
                break;
            }
        }

        if(aNbVideos == 2) {
            myPlayList->clear();
            myPlayList->addOneFile(aFile1, aFile2);
            doUpdateStateLoading();
            myVideo->pushPlayEvent(ST_PLAYEVENT_RESUME);
            myVideo->doLoadNext();
            return;
        }
    }

    myPlayList->clear();
    for(uint32_t aFileIter = 0; aFileIter < theEvent.NbFiles; ++aFileIter) {
        StString aPath(theEvent.Files[aFileIter]);
        if(!StFolder::isFolder(aPath)
        && myPlayList->checkExtension(aPath)) {
            myPlayList->addOneFile(aPath, StMIME());
        }
    }

    doUpdateStateLoading();
    myVideo->pushPlayEvent(ST_PLAYEVENT_RESUME);
    myVideo->doLoadNext();
}

void StMoviePlayer::doNavigate(const StNavigEvent& theEvent) {
    switch(theEvent.Target) {
        case stNavigate_Backward: doListPrev(); break;
        case stNavigate_Forward:  doListNext(); break;
        default: break;
    }
}

void StMoviePlayer::beforeDraw() {
    if(myGUI.isNull()) {
        return;
    }

    if(myVideo->isDisconnected() || myToUpdateALList) {
        const StString aPrevDev = params.AudioAlDevice->getUtfTitle();
        params.AudioAlDevice->initList();
        myGUI->updateOpenALDeviceMenu();
        // switch audio device
        if(!params.AudioAlDevice->init(aPrevDev)) {
            // select first existing device if any
            params.AudioAlDevice->init(params.AudioAlDevice->getUtfTitle());
        }
        myVideo->switchAudioDevice(params.AudioAlDevice->getCTitle());
        myToUpdateALList = false;
    }
    if(myPlayList->isRecentChanged()) {
        myGUI->updateRecentMenu();
    }

    // fetch Open File operation results
    if(myOpenDialog->hasResults()) {
        StHandle<StFileNode> aCurrFile = myPlayList->getCurrentFile();
        StString aFilePath;
        if(!myOpenDialog->getPathAudio().isEmpty()) {
            aFilePath = myOpenDialog->getPathAudio();
            myPlayList->addToNode(aCurrFile, aFilePath);
            myAudioOnLoad = myVideo->params.activeAudio->getListSize();
            mySubsOnLoad  = myVideo->params.activeSubtitles->getValue();
            mySeekOnLoad  = myVideo->getPts();
        } else if(!myOpenDialog->getPathSubtitles().isEmpty()) {
            aFilePath = myOpenDialog->getPathSubtitles();
            myPlayList->addToNode(aCurrFile, aFilePath);
            myAudioOnLoad = myVideo->params.activeAudio->getValue();
            mySubsOnLoad  = myVideo->params.activeSubtitles->getListSize();
            mySeekOnLoad  = myVideo->getPts();
        } else if(!myOpenDialog->getPathRight().isEmpty()) {
            // meta-file
            aFilePath = myOpenDialog->getPathLeft();
            myPlayList->clear();
            myPlayList->addOneFile(myOpenDialog->getPathLeft(), myOpenDialog->getPathRight());
        } else {
            if(!myPlayList->checkExtension(myOpenDialog->getPathLeft())
             && myVideo->getMimeListImages().checkExtension(StFileNode::getExtension(myOpenDialog->getPathLeft()))) {
                // redirect to StImageViewer
                myOpenFileOtherApp = new StOpenInfo();
                StArgumentsMap anArgs;
                anArgs.add(StArgument("in", "image"));
                myOpenFileOtherApp->setArgumentsMap(anArgs);
                myOpenFileOtherApp->setPath(myOpenDialog->getPathLeft());
                exit(0);
            } else {
                aFilePath = myOpenDialog->getPathLeft();
                myPlayList->open(myOpenDialog->getPathLeft());
            }
        }

        doUpdateStateLoading();
        myVideo->pushPlayEvent(ST_PLAYEVENT_RESUME);
        myVideo->doLoadNext();

        StString aDummy;
        StFileNode::getFolderAndFile(aFilePath, params.lastFolder, aDummy);
        if(!params.lastFolder.isEmpty()) {
            mySettings->saveString(ST_SETTING_LAST_FOLDER, params.lastFolder);
        }
        myOpenDialog->resetResults();
    }

    // re-create GUI when necessary
    if(params.ScaleHiDPI->setValue(myWindow->getScaleFactor())
    || myToRecreateMenu) {
        StHandle<StGLTextureQueue> aTextureQueue;
        StHandle<StSubQueue>       aSubQueue;
        createGui(aTextureQueue, aSubQueue);
        myToRecreateMenu = false;
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

    double aDuration = 0.0;
    double aPts      = 0.0;
    bool isVideoPlayed = false, isAudioPlayed = false;
    const bool   isPlaying = myVideo->getPlaybackState(aDuration, aPts, isVideoPlayed, isAudioPlayed);
    const double aPosition = (aDuration > 0.0) ? (aPts / aDuration) : 0.0;
    if(myGUI->myBtnPlay != NULL) {
        myGUI->myBtnPlay->setFaceId(isPlaying ? 1 : 0); // set play/pause
    }
    if(myGUI->myTimeBox != NULL) {
        myGUI->myTimeBox->stglUpdateTime(aPts, aDuration);
    }
    if(myGUI->mySubtitles != NULL) {
        myGUI->mySubtitles->setPTS(aPts);
    }
    if(myGUI->mySeekBar != NULL) {
        myGUI->mySeekBar->setProgress(GLfloat(aPosition));
    }
    myGUI->stglUpdate(myWindow->getMousePos(), myWindow->isPreciseCursor());

    // prevent display going to sleep
    bool toBlockSleepDisplay = false;
    bool toBlockSleepSystem  = false;
    if(params.Benchmark->getValue()) {
        toBlockSleepDisplay = true;
        toBlockSleepSystem  = true;
    } else {
        switch(params.BlockSleeping->getValue()) {
            case BLOCK_SLEEP_NEVER: {
                toBlockSleepDisplay = false;
                toBlockSleepSystem  = false;
                break;
            }
            case BLOCK_SLEEP_ALWAYS: {
                toBlockSleepDisplay = true;
                toBlockSleepSystem  = true;
                break;
            }
            case BLOCK_SLEEP_PLAYBACK: {
                toBlockSleepDisplay = isVideoPlayed;
                toBlockSleepSystem  = isPlaying;
                break;
            }
            case BLOCK_SLEEP_FULLSCREEN: {
                toBlockSleepDisplay = myWindow->isFullScreen();
                toBlockSleepSystem  = toBlockSleepDisplay;
                break;
            }
        }
    }

    const StWinAttr anAttribs[] = {
        StWinAttr_ToBlockSleepSystem,  (StWinAttr )toBlockSleepSystem,
        StWinAttr_ToBlockSleepDisplay, (StWinAttr )toBlockSleepDisplay,
        StWinAttr_NULL
    };
    myWindow->setAttributes(anAttribs);
    myWindow->showCursor(!myGUI->toHideCursor());

    // check for mono state
    bool hasStereoSource = false;
    StHandle<StStereoParams> aParams = myGUI->myImage->getSource();
    if(!aParams.isNull()) {
        StGLQuaternion aHeadOrient;
        const bool toTrackHead = params.ToTrackHeadAudio->getValue()
                              && myGUI->myImage->getHeadOrientation(aHeadOrient, ST_DRAW_MONO, false);
        myVideo->setHeadOrientation(aHeadOrient, toTrackHead);

        hasStereoSource =!aParams->isMono()
                       && myGUI->myImage->hasVideoStream()
                       && myGUI->myImage->params.DisplayMode->getValue() == StGLImageRegion::MODE_STEREO;
    }
    myWindow->setStereoOutput(hasStereoSource);

    const double aDispMaxFps = myWindow->getMaximumTargetFps();
    double aTargetFps = myVideo->getAverFps();
    int aMaxUploadFrames = int((double(aDispMaxFps) + 0.1) / stMax(aTargetFps, 1.0));
    if(params.Benchmark->getValue()
    || !params.ToLimitFps->getValue()) {
        // do not limit FPS
        myWindow->setTargetFps(-1.0);
    } else if(params.TargetFps->getValue() >= 1
           && params.TargetFps->getValue() <= 3) {
        // set rendering FPS to 2x averageFPS
        const bool toTrackOrientation = myWindow->toTrackOrientation();
        if(aTargetFps < 17.0
        || aTargetFps > 120.0) {
            aTargetFps = 0.0;
        } else if(aTargetFps < 40.0) {
            if(!toTrackOrientation) {
                aMaxUploadFrames = 2; // maximum 2 frames to upload single frame
            }
            aTargetFps *= double(params.TargetFps->getValue());
        } else {
            if(!toTrackOrientation) {
                aMaxUploadFrames = 1; // uploads should be done within single frame
            }
        }

        if(toTrackOrientation) {
            // do not limit FPS within head-tracking mode
            aTargetFps = 0.0;
        } else if(aTargetFps > aDispMaxFps) {
            aTargetFps = 0.0;
        }

        myWindow->setTargetFps(aTargetFps);
    } else {
        // set rendering FPS to set value in settings
        myWindow->setTargetFps(double(params.TargetFps->getValue()));
        aMaxUploadFrames = 1; // uploads should be done within single frame
    }

    if(!params.ToSmoothUploads->getValue()) {
        aMaxUploadFrames = 1;
    }
    myVideo->getTextureQueue()->getUploadParams().MaxUploadIterations = stMax(stMin(aMaxUploadFrames, 3), 1);
}

void StMoviePlayer::doUpdateOpenALDeviceList(const size_t ) {
    myToUpdateALList = true;
}

void StMoviePlayer::stglDraw(unsigned int theView) {
    const bool hasCtx = !myContext.isNull() && myContext->isBound();
    if(!hasCtx || myWindow->isPaused()) {
        if(!myGUI.isNull()
         && myGUI->myImage != NULL) {
            myGUI->myImage->stglSkipFrames();
        }

        if(theView == ST_DRAW_LEFT
        || theView == ST_DRAW_MONO) {
            if(myWindow->isPaused()) {
                double aDuration = 0.0;
                double aPts      = 0.0;
                bool isVideoPlayed = false, isAudioPlayed = false;
                const bool isPlaying = !myVideo.isNull()
                                     && myVideo->getPlaybackState(aDuration, aPts, isVideoPlayed, isAudioPlayed);
                const double aTimeout = hasCtx ? 300.0 : 60.0;
                if(!myInactivityTimer.isOn()) {
                    myInactivityTimer.restart();
                } else if(myInactivityTimer.getElapsedTimeInSec() > aTimeout
                      && !isPlaying) {
                    // perform delayed destruction on long inactivity
                    exit(0);
                } else if(!isVideoPlayed) {
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
    myGUI->stglDraw(theView);
}

void StMoviePlayer::doShowPlayList(const bool theToShow) {
    if(myGUI.isNull()
    || myGUI->myPlayList == NULL) {
        return;
    }

    myGUI->myPlayList->setOpacity(theToShow ? 1.0f : 0.0f, false);
}

void StMoviePlayer::doShowAdjustImage(const bool theToShow) {
    if(myGUI.isNull()
    || myGUI->myAdjustOverlay == NULL) {
        return;
    }

    myGUI->myAdjustOverlay->setOpacity(theToShow ? 1.0f : 0.0f, false);
}

void StMoviePlayer::doPlayListReverse(const size_t ) {
    if(myGUI.isNull()) {
        return;
    }

    params.ToShowPlayList->reverse();
}

void StMoviePlayer::doSwitchShuffle(const bool theShuffleOn) {
    myPlayList->setShuffle(theShuffleOn);
}

void StMoviePlayer::doSwitchLoopSingle(const bool theValue) {
    myPlayList->setLoopSingle(theValue);
}

void StMoviePlayer::doSwitchVSync(const bool theValue) {
    StApplication::params.VSyncMode->setValue(theValue ? StGLContext::VSync_ON : StGLContext::VSync_OFF);
}

void StMoviePlayer::doSwitchAudioDevice(const int32_t /*theDevId*/) {
    if(!myVideo.isNull()) {
        myVideo->switchAudioDevice(params.AudioAlDevice->getCTitle());
    }
}

bool StMoviePlayer::hasAlHrtf() const {
    return !myVideo.isNull()
         && myVideo->hasAlHrtf();
}

void StMoviePlayer::doSwitchAudioAlHrtf(const int32_t theValue) {
    if(!myVideo.isNull()) {
        myVideo->setAlHrtfRequest((StAudioQueue::StAlHrtfRequest )theValue);
    }
}

void StMoviePlayer::doSetForceBFormat(const bool theValue) {
    if(!myVideo.isNull()) {
        myVideo->setForceBFormat(theValue);
    }
}

void StMoviePlayer::doSetAudioVolume(const float theGaindB) {
    if(!myVideo.isNull()
    && !params.AudioMute->getValue()) {
        const GLfloat aGain = params.AudioGain->isMinValue()
                            ? 0.0f
                            : GLfloat(StMoviePlayerGUI::dBellToRatio(theGaindB));
        myVideo->setAudioVolume(aGain);
    }
}

void StMoviePlayer::doSetAudioMute(const bool theToMute) {
    if(!myVideo.isNull()) {
        const GLfloat aGain = (theToMute || params.AudioGain->isMinValue())
                            ? 0.0f
                            : GLfloat(StMoviePlayerGUI::dBellToRatio(params.AudioGain->getValue()));
        myVideo->setAudioVolume(aGain);
    }
}

void StMoviePlayer::doSetAudioDelay(const float theDelaySec) {
    if(!myVideo.isNull()) {
        myVideo->setAudioDelay(theDelaySec);
    }
}

void StMoviePlayer::doUpdateStateLoading() {
    const StString aFileToLoad = myPlayList->getCurrentTitle();
    if(aFileToLoad.isEmpty()) {
        myWindow->setTitle("sView - Movie Player");
    } else {
        /// TODO (Kirill Gavrilov#4) - show Loading... after delay
        myWindow->setTitle(aFileToLoad + " Loading... - sView");
    }
}

void StMoviePlayer::doUpdateStateLoaded() {
    const StString aFileToLoad = myPlayList->getCurrentTitle();
    if(aFileToLoad.isEmpty()) {
        myWindow->setTitle("sView - Movie Player");
    } else {
        myWindow->setTitle(aFileToLoad + " - sView");
    }
    params.AudioStream    ->setValue(myVideo->params.activeAudio->getValue());
    params.SubtitlesStream->setValue(myVideo->params.activeSubtitles->getValue());
    if(mySeekOnLoad > 0.0) {
        myVideo->pushPlayEvent(ST_PLAYEVENT_SEEK, mySeekOnLoad);
        mySeekOnLoad = -1.0;
    }
    if(myAudioOnLoad >= 0) {
        myVideo->params.activeAudio->setValue(myAudioOnLoad);
        params.AudioStream->setValue(myAudioOnLoad);
        myAudioOnLoad = -1;
    }
    if(mySubsOnLoad >= 0) {
        myVideo->params.activeSubtitles->setValue(mySubsOnLoad);
        params.SubtitlesStream->setValue(mySubsOnLoad);
        mySubsOnLoad = -1;
    }
}

void StMoviePlayer::doAboutFile(const size_t ) {
    if(!myGUI.isNull()) {
        myGUI->doAboutFile(0);
    }
}

void StMoviePlayer::doSwitchViewMode(const int32_t theMode) {
    if(myVideo.isNull()) {
        return;
    }

    myVideo->setTheaterMode(theMode == StViewSurface_Theater);
}

void StMoviePlayer::doPanoramaOnOff(const size_t ) {
    if(myVideo.isNull()) {
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
    myGUI->myImage->params.ViewMode->setValue(StStereoParams::getViewSurfaceForPanoramaSource(aPano, true));
}

void StMoviePlayer::doChangeStickPano360(const bool ) {
    if(myVideo.isNull()) {
        return;
    }

    myVideo->setStickPano360(params.ToStickPanorama->getValue());
}

void StMoviePlayer::doChangeSwapJPS(const bool ) {
    if(myVideo.isNull()) {
        return;
    }

    myVideo->setSwapJPS(params.ToSwapJPS->getValue());
}

void StMoviePlayer::doSwitchSrcFormat(const int32_t theSrcFormat) {
    myVideo->setStereoFormat(StFormat(theSrcFormat));
    double aDuration = 0.0;
    double aPts = 0.0;
    bool   isVideoPlayed = false;
    bool   isAudioPlayed = false;
    bool   isPlaying = myVideo->getPlaybackState(aDuration, aPts, isVideoPlayed, isAudioPlayed);
    StHandle<StStereoParams> aParams = myGUI->myImage->getSource();
    if(!isPlaying && !aParams.isNull() && myGUI->myImage->hasVideoStream()) {
        myVideo->pushPlayEvent(ST_PLAYEVENT_SEEK, aPts - 0.01);
    }
}

void StMoviePlayer::doSetStereoOutput(const size_t theMode) {
    if(myVideo.isNull()) {
        return;
    }
    myGUI->myImage->params.DisplayMode->setValue((int32_t )theMode);
}

void StMoviePlayer::doScaleGui(const int32_t ) {
    if(myGUI.isNull()) {
        return;
    }
    myToRecreateMenu = true;
}

void StMoviePlayer::doChangeMobileUI(const bool ) {
    params.IsMobileUISwitch->setValue(toUseMobileUI());
}

void StMoviePlayer::doScaleHiDPI(const bool ) {
    if(myGUI.isNull()) {
        return;
    }
    myToRecreateMenu = true;
}

void StMoviePlayer::doChangeMixImagesVideos(const bool theToMix) {
    if(myVideo.isNull()) {
        return;
    }
    if(theToMix) {
        StArrayList<StString> aMediaExt = myVideo->getMimeListVideo().getExtensionsList();
        StArrayList<StString> anImgExt  = myVideo->getMimeListImages().getExtensionsList();
        for(size_t anExtIter = 0; anExtIter < anImgExt.size(); ++anExtIter) {
            aMediaExt.add(anImgExt.getValue(anExtIter));
        }
        myPlayList->setExtensions(aMediaExt);
    } else {
        myPlayList->setExtensions(myVideo->getMimeListVideo().getExtensionsList());
    }
}

void StMoviePlayer::doLoaded() {
    myEventLoaded.set();
}

void StMoviePlayer::doListFirst(const size_t ) {
    if(myPlayList->walkToFirst()) {
        myVideo->doLoadNext();
        doUpdateStateLoading();
    }
}

void StMoviePlayer::doListPrev(const size_t ) {
    if(myPlayList->walkToPrev()) {
        myVideo->doLoadNext();
        doUpdateStateLoading();
    }
}

void StMoviePlayer::doListNext(const size_t ) {
    if(myPlayList->walkToNext()) {
        myVideo->doLoadNext();
        doUpdateStateLoading();
    }
}

void StMoviePlayer::doListLast(const size_t ) {
    if(myPlayList->walkToLast()) {
        myVideo->doLoadNext();
        doUpdateStateLoading();
    }
}

void StMoviePlayer::doShowHideGUI(const size_t ) {
    const bool toShow = !params.ToShowMenu->getValue() || (!myGUI.isNull() && !myGUI->isVisibleGUI());
    params.ToShowMenu->setValue(toShow);
    params.ToShowTopbar->setValue(toShow);
    params.ToShowBottom->setValue(toShow);
    if(toShow && !myGUI.isNull()) {
        myGUI->setVisibility(myWindow->getMousePos(), false, true);
    }
}

void StMoviePlayer::doQuit(const size_t ) {
    StApplication::exit(0);
}

void StMoviePlayer::doOpen1FileFromGui(StHandle<StString> thePath) {
    myOpenDialog->setPaths(*thePath, "", StMovieOpenDialog::Dialog_SingleMovie);
}

void StMoviePlayer::doOpen1AudioFromGui(StHandle<StString> thePath) {
    myOpenDialog->setPaths(*thePath, "", StMovieOpenDialog::Dialog_Audio);
}

void StMoviePlayer::doOpen1SubtitleFromGui(StHandle<StString> thePath) {
    myOpenDialog->setPaths(*thePath, "", StMovieOpenDialog::Dialog_Subtitles);
}

void StMoviePlayer::doOpen1FileAction(const size_t ) {
    if(!myGUI.isNull() && (myWindow->isFullScreen() || myGUI->isMobile())) {
        myGUI->doOpenFile(StMovieOpenDialog::Dialog_SingleMovie);
        return;
    }
    myOpenDialog->openDialog(StMovieOpenDialog::Dialog_SingleMovie);
}

void StMoviePlayer::doOpen2Files(const size_t ) {
    if(!myGUI.isNull() && (myWindow->isFullScreen() || myGUI->isMobile())) {
        //myGUI->doOpenFile(StMovieOpenDialog::Dialog_DoubleMovie);
        //return;
    }
    myOpenDialog->openDialog(StMovieOpenDialog::Dialog_DoubleMovie);
}

void StMoviePlayer::doSaveFileInfo(const size_t theToSave) {
    if(!myGUI.isNull()
    && !myFileInfo.isNull()
    &&  theToSave == 1) {
        //myLoader->doSaveInfo(myFileInfo);
    }
    myFileInfo.nullify();
}

void StMoviePlayer::doOpenRecent(const size_t theItemId) {
    if(myVideo.isNull()) {
        return;
    }

    StHandle<StStereoParams> anOldParams = myPlayList->openRecent(theItemId);
    doUpdateStateLoading();
    myVideo->pushPlayEvent(ST_PLAYEVENT_RESUME);
    myVideo->doLoadNext();
    if(!anOldParams.isNull()) {
        mySeekOnLoad = anOldParams->Timestamp;
    }
}

void StMoviePlayer::doClearRecent(const size_t ) {
    myPlayList->clearRecent();
}

void StMoviePlayer::doAddAudioStream(const size_t ) {
    if(!myGUI.isNull() && (myWindow->isFullScreen() || myGUI->isMobile())) {
        myGUI->doOpenFile(StMovieOpenDialog::Dialog_Audio);
        return;
    }
    myOpenDialog->openDialog(StMovieOpenDialog::Dialog_Audio);
}

void StMoviePlayer::doAddSubtitleStream(const size_t ) {
    if(!myGUI.isNull() && (myWindow->isFullScreen() || myGUI->isMobile())) {
        myGUI->doOpenFile(StMovieOpenDialog::Dialog_Subtitles);
        return;
    }
    myOpenDialog->openDialog(StMovieOpenDialog::Dialog_Subtitles);
}

void StMoviePlayer::doSeekLeft(const size_t ) {
    double aSeekPts = (myVideo->getPts() - 5.0);
    if(aSeekPts < 0.0) {
        aSeekPts = 0.0;
    }
    myVideo->pushPlayEvent(ST_PLAYEVENT_SEEK, aSeekPts);
}

void StMoviePlayer::doSeekRight(const size_t ) {
    double aSeekPts = (myVideo->getPts() + 5.0);
    if(aSeekPts < 0.0) {
        aSeekPts = 0.0;
    }
    myVideo->pushPlayEvent(ST_PLAYEVENT_SEEK, aSeekPts);
}

void StMoviePlayer::doSeek(const int theMouseBnt, const double theSeekX) {
    if(theMouseBnt != ST_MOUSE_LEFT) {
        return;
    }

    double aSeekPts = myVideo->getDuration() * theSeekX;
    if(aSeekPts < 0.0) {
        aSeekPts = 0.0;
    }
    myVideo->pushPlayEvent(ST_PLAYEVENT_SEEK, aSeekPts);
}

void StMoviePlayer::doPlayPause(const size_t ) {
    myVideo->pushPlayEvent(myVideo->isPlaying() ? ST_PLAYEVENT_PAUSE : ST_PLAYEVENT_RESUME);
}

void StMoviePlayer::doStop(const size_t ) {
    if(!myVideo->isPlaying()) {
        myVideo->pushPlayEvent(ST_PLAYEVENT_RESUME);
    }
    myVideo->doLoadNext(); // TODO (Kirill Gavrilov#9#) implement correct stop
}

void StMoviePlayer::doSwitchAudioStream(const int32_t theStreamId) {
    myVideo->params.activeAudio->setValue(theStreamId);
}

void StMoviePlayer::doSwitchSubtitlesStream(const int32_t theStreamId) {
    myVideo->params.activeSubtitles->setValue(theStreamId);
}

void StMoviePlayer::doFullscreen(const bool theIsFullscreen) {
    if(!myWindow.isNull()) {
        myWindow->setAttribute(StWinAttr_ExclusiveFullScreen, params.IsExclusiveFullScreen->getValue());
        myWindow->setFullScreen(theIsFullscreen);
    }
}

void StMoviePlayer::doSnapshot(const size_t theImgType) {
    size_t aType = theImgType;
    if(theImgType == StImageFile::ST_TYPE_NONE) {
        aType = params.SnapshotImgType->getValue();
    } else {
        params.SnapshotImgType->setValue((int32_t )theImgType);
    }
    myVideo->doSaveSnapshotAs(aType);
}

void StMoviePlayer::doHideSystemBars(const bool ) {
    if(myWindow.isNull()) {
        return;
    }

    myWindow->setHideSystemBars(params.ToHideStatusBar->getValue(), params.ToHideNavBar->getValue());
}

void StMoviePlayer::doSetBenchmark(const bool theValue) {
    if(myVideo.isNull()) {
        return;
    }

    myVideo->setBenchmark(theValue);
}

bool StMoviePlayer::getCurrentFile(StHandle<StFileNode>&     theFileNode,
                                   StHandle<StStereoParams>& theParams,
                                   StHandle<StMovieInfo>&    theInfo) {
    theInfo.nullify();
    if(!myPlayList->getCurrentFile(theFileNode, theParams)) {
        return false;
    }
    theInfo = myVideo->getFileInfo(theParams);
    return true;
}

void StMoviePlayer::getRecentList(StArrayList<StString>& theList) {
    myPlayList->getRecentList(theList);
}

int StMoviePlayer::beginRequest(mg_connection*         theConnection,
                                const mg_request_info& theRequestInfo) {
#ifdef ST_HAVE_MONGOOSE
    const StString anURI  = theRequestInfo.uri;
    const StString aQuery = theRequestInfo.query_string != NULL ? theRequestInfo.query_string : "";

    // process general requests
    if(anURI.isEquals(stCString("/"))) {
        // return index page
        StHandle<StResource> aRes = myResMgr->getResource(StString("web") + SYS_FS_SPLITTER + "index.htm");
        mg_send_file(theConnection, !aRes.isNull() ? aRes->getPath().toCString() : "");
        return 1;
    } else if(anURI.isStartsWith(stCString("/web"))) {
        // return Web UI files
        const StString aSubPath = anURI.subString(5, size_t(-1));
        StHandle<StResource> aRes = myResMgr->getResource(StString("web") + SYS_FS_SPLITTER + aSubPath);
        mg_send_file(theConnection, !aRes.isNull() ? aRes->getPath().toCString() : "");
        return 1;
    } else if(anURI.isStartsWith(stCString("/textures"))) {
        // return textures images
        const StString aSubPath = anURI.subString(10, size_t(-1));
        StHandle<StResource> aRes = myResMgr->getResource(StString("textures") + SYS_FS_SPLITTER + aSubPath);
        mg_send_file(theConnection, !aRes.isNull() ? aRes->getPath().toCString() : "");
        return 1;
    }

    // process AJAX requests
    StString aContent;
    if(anURI.isEquals(stCString("/prev"))) {
        invokeAction(Action_ListPrev);
        aContent = "open previous item in playlist...";
    } else if(anURI.isEquals(stCString("/next"))) {
        invokeAction(Action_ListNext);
        aContent = "open next item in playlist...";
    } else if(anURI.isEquals(stCString("/play_pause"))) {
        invokeAction(Action_PlayPause);
        aContent = "play/pause playback...";
    } else if(anURI.isEquals(stCString("/stop"))) {
        invokeAction(Action_Stop);
        aContent = "stop playback...";
    } else if(anURI.isEquals(stCString("/mute"))) {
        invokeAction(Action_AudioMute);
        aContent = "audio mute/unmute...";
    } else if(anURI.isEquals(stCString("/vol"))) {
        StCLocale aCLocale;
        const long aVol = stStringToLong(aQuery.toCString(), 10, aCLocale);
        params.AudioGain->setValue(volumeToGain(params.AudioGain, GLfloat(aVol) * 0.01f));
        aContent = "audio set volume...";
    } else if(anURI.isEquals(stCString("/seek"))) {
        StCLocale aCLocale;
        const double aPosSec = stStringToDouble(aQuery.toCString(), aCLocale);
        myVideo->pushPlayEvent(ST_PLAYEVENT_SEEK, aPosSec);
        aContent = "seek to position...";
    } else if(anURI.isEquals(stCString("/fullscr_win"))) {
        invokeAction(Action_Fullscreen);
        aContent = "switch fullscreen/windowed...";
    } else if(anURI.isEquals(stCString("/current"))) {
        if(aQuery.isEquals(stCString("id"))) {
            aContent = StString(myPlayList->getSerial())
                     + ":" + myPlayList->getCurrentId()
                     + ":" + int(gainToVolume(params.AudioGain) * 100.0f);
        } else if(aQuery.isEquals(stCString("title"))) {
            aContent = myPlayList->getCurrentTitle();
        }
    } else if(anURI.isEquals(stCString("/item"))) {
        StCLocale aCLocale;
        const long anItem = stStringToLong(aQuery.toCString(), 10, aCLocale);
        myPlayList->walkToPosition(anItem);
        myVideo->pushPlayEvent(ST_PLAYEVENT_RESUME);
        myVideo->doLoadNext();
        aContent = "open item...";
    } else if(anURI.isEquals(stCString("/version"))) {
        aContent = StVersionInfo::getSDKVersionString();
    } else if(anURI.isEquals(stCString("/playlist"))) {
        // return current playlist
        StArrayList<StString> aList;
        myPlayList->getSubList(aList, 0, size_t(-1));
        if(!aList.isEmpty()) {
            for(size_t anIter = 0;;) {
                aContent += aList[anIter++];
                if(anIter < aList.size()) {
                    aContent += "\n";
                } else {
                    break;
                }
            }
        }
    } else if(anURI.isEquals(stCString("/action"))) {
        if(!params.IsLocalWebUI->getValue()) {
            aContent = "Error: command interface is disabled!";
        } else {
            const int anActionId = getActionIdFromName(aQuery);
            if(anActionId != -1) {
                invokeAction(anActionId);
                aContent = "Action has been invoked";
            } else {
                aContent = "Error: unknown action";
            }
        }
    } else {
        aContent = StString("query_string: '") + aQuery + "'\n"
                 + StString("uri: '") + anURI + "'";
        //return 0;
    }

    const StString anAnswer = StString("HTTP/1.1 200 OK\r\n"
                                       //"Content-Type: text/html; charset=utf-8\r\n"
                                       "Content-Type: text/plain; charset=utf-8\r\n"
                                       "Content-Length: ") + aContent.getSize() + "\r\n"
                                        "\r\n" + aContent;

    // send HTTP reply to the client
    mg_write(theConnection, anAnswer.toCString(), anAnswer.getSize());
#endif
    // returning non-zero tells mongoose that our function has replied to
    // the client, and mongoose should not send client any more data.
    return 1;
}

int StMoviePlayer::beginRequestHandler(mg_connection* theConnection) {
#ifdef ST_HAVE_MONGOOSE
    const mg_request_info* aRequestInfo = mg_get_request_info(theConnection);
    StMoviePlayer* aPlugin = (StMoviePlayer* )aRequestInfo->user_data;
    return aPlugin->beginRequest(theConnection, *aRequestInfo);
#else
    return 0;
#endif
}
