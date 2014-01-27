/**
 * Copyright © 2007-2014 Kirill Gavrilov <kirill@sview.ru>
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

#include "StMoviePlayerGUI.h"
#include "StMoviePlayerStrings.h"
#include "StVideo/StVideo.h"
#include "StTimeBox.h"

#include <StSocket/StCheckUpdates.h>
#include <StImage/StImageFile.h>
#include <StStrings/StStringStream.h>

#include <StGL/StGLContext.h>
#include <StGLCore/StGLCore20.h>
#include <StGLWidgets/StGLButton.h>
#include <StGLWidgets/StGLImageRegion.h>
#include <StGLWidgets/StGLMessageBox.h>
#include <StGLWidgets/StGLMsgStack.h>
#include <StGLWidgets/StGLPlayList.h>
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

    static const char ST_SETTING_FPSTARGET[]     = "fpsTarget";
    static const char ST_SETTING_SRCFORMAT[]     = "srcFormat";
    static const char ST_SETTING_LAST_FOLDER[]   = "lastFolder";
    static const char ST_SETTING_OPENAL_DEVICE[] = "alDevice";
    static const char ST_SETTING_RECENT_FILES[]  = "recent";
    static const char ST_SETTING_SHOW_LIST[]     = "showPlaylist";
    static const char ST_SETTING_SHOW_FPS[]      = "toShowFps";
    static const char ST_SETTING_LIMIT_FPS[]     = "toLimitFps";
    static const char ST_SETTING_GPU_DECODING[]  = "gpuDecoding";
    static const char ST_SETTING_VSYNC[]         = "vsync";

    static const char ST_SETTING_SCALE_ADJUST[]  = "scaleAdjust";
    static const char ST_SETTING_SCALE_FORCE2X[] = "scale2X";
    static const char ST_SETTING_SUBTITLES_SIZE[]= "subsSize";
    static const char ST_SETTING_SUBTITLES_PARALLAX[] = "subsParallax";
    static const char ST_SETTING_SUBTITLES_PARSER[] = "subsParser";
    static const char ST_SETTING_SEARCH_SUBS[]   = "toSearchSubs";
    static const char ST_SETTING_FULLSCREEN[]    = "fullscreen";
    static const char ST_SETTING_VIEWMODE[]      = "viewMode";
    static const char ST_SETTING_STEREO_MODE[]   = "viewStereoMode";
    static const char ST_SETTING_TEXFILTER[]     = "viewTexFilter";
    static const char ST_SETTING_GAMMA[]         = "viewGamma";
    static const char ST_SETTING_SHUFFLE[]       = "shuffle";
    static const char ST_SETTING_LOOP_SINGLE[]   = "loopSingle";
    static const char ST_SETTING_GLOBAL_MKEYS[]  = "globalMediaKeys";
    static const char ST_SETTING_RATIO[]         = "ratio";
    static const char ST_SETTING_UPDATES_LAST_CHECK[] = "updatesLastCheck";
    static const char ST_SETTING_UPDATES_INTERVAL[]   = "updatesInterval";
    static const char ST_SETTING_SAVE_IMG_TYPE[] = "snapImgType";
    static const char ST_SETTING_EXPERIMENTAL[]  = "experimental";

    static const char ST_SETTING_WEBUI_ON[]      = "webuiOn";
    static const char ST_SETTING_WEBUI_PORT[]    = "webuiPort";
    static const char ST_SETTING_WEBUI_ERRORS[]  = "webuiShowErrors";

    static const char ST_ARGUMENT_FILE[]       = "file";
    static const char ST_ARGUMENT_FILE_LEFT[]  = "left";
    static const char ST_ARGUMENT_FILE_RIGHT[] = "right";
    static const char ST_ARGUMENT_FILE_LAST[]  = "last";
    static const char ST_ARGUMENT_BENCHMARK[]  = "benchmark";
};

StALDeviceParam::StALDeviceParam()
: StInt32Param(0) {
    initList();
}

void StALDeviceParam::initList() {
    myValue = 0;
    myDevicesList.clear();
    StString aName;
    if(alcIsExtensionPresent(NULL, "ALC_ENUMERATE_ALL_EXT") != AL_TRUE) {
        // ansient OpenAL implementations supports only single device (like from apples)
        const ALchar* aDefDevice = alcGetString(NULL, ALC_DEFAULT_DEVICE_SPECIFIER);
    #ifdef _WIN32
        aName.fromLocale(aDefDevice);
    #else
        aName.fromUnicode(aDefDevice);
    #endif
        myDevicesList.add(aName);
        return;
    }

    const ALchar* aDevicesNames = alcGetString(NULL, ALC_ALL_DEVICES_SPECIFIER);
    while(aDevicesNames && *aDevicesNames) {
    #ifdef _WIN32
        aName.fromLocale(aDevicesNames);
    #else
        aName.fromUnicode(aDevicesNames);
    #endif
        myDevicesList.add(aName);
        aDevicesNames += strlen(aDevicesNames) + 1;
    }
    if(myDevicesList.isEmpty()) {
        myDevicesList.add("None"); // append dummy device
    }
}

StALDeviceParam::~StALDeviceParam() {}

int32_t StALDeviceParam::getValueFromName(const StString& theName) {
    for(size_t anId = 0; anId < myDevicesList.size(); ++anId) {
        if(myDevicesList[anId] == theName) {
            return int32_t(anId);
        }
    }
    return -1;
}

bool StALDeviceParam::init(const StString& theActive) {
    myValue = getValueFromName(theActive);
    return myValue >= 0;
}

StString StALDeviceParam::getTitle() const {
    if(myDevicesList.isEmpty()) {
        return StString();
    }
    int32_t anActive = getValue();
    return myDevicesList[(anActive >= 0 && size_t(anActive) < myDevicesList.size()) ? size_t(anActive) : 0];
}

void StMoviePlayer::doChangeDevice(const int32_t theValue) {
    StApplication::doChangeDevice(theValue);
    // update menu
}

StMoviePlayer::StMoviePlayer(const StNativeWin_t         theParentWin,
                             const StHandle<StOpenInfo>& theOpenInfo)
: StApplication(theParentWin, theOpenInfo),
  mySettings(new StSettings(ST_DRAWER_PLUGIN_NAME)),
  myLangMap(new StTranslations(StMoviePlayer::ST_DRAWER_PLUGIN_NAME)),
  myPlayList(new StPlayList(4, true)),
  myEventDialog(false),
  myEventLoaded(false),
  mySeekOnLoad(-1.0),
  myAudioOnLoad(-1),
  mySubsOnLoad(-1),
  //
  myWebCtx(NULL),
  //
  myLastUpdateDay(0),
  myToRecreateMenu(false),
  myToUpdateALList(false),
  myIsBenchmark(false),
  myToCheckUpdates(true) {
    StMoviePlayerStrings::loadDefaults(*myLangMap);
    myTitle = "sView - Movie Player";

    params.ScaleAdjust      = new StInt32Param(StGLRootWidget::ScaleAdjust_Normal);
    mySettings->loadParam (ST_SETTING_SCALE_ADJUST, params.ScaleAdjust);
    params.ScaleAdjust->signals.onChanged = stSlot(this, &StMoviePlayer::doScaleGui);
    params.ScaleHiDPI       = new StFloat32Param(1.0f,       // initial value
                                                 0.5f, 3.0f, // min, max values
                                                 1.0f,       // default value
                                                 1.0f,       // incremental step
                                                 0.001f);    // equality tolerance
    params.ScaleHiDPI2X     = new StBoolParam(false);
    mySettings->loadParam (ST_SETTING_SCALE_FORCE2X, params.ScaleHiDPI2X);
    params.ScaleHiDPI2X->signals.onChanged = stSlot(this, &StMoviePlayer::doScaleHiDPI);
    params.SubtitlesSize    = new StFloat32Param(28.0f,       // initial value
                                                 8.0f, 96.0f, // min, max values
                                                 28.0f,       // default value
                                                 1.0f,        // incremental step
                                                 0.1f);       // equality tolerance
    params.SubtitlesParallax= new StFloat32Param(0.0f,        // initial value
                                                -90.0f, 90.0f,// min, max values
                                                 0.0f,        // default value
                                                 1.0f,        // incremental step
                                                 0.1f);       // equality tolerance
    params.ToSearchSubs = new StBoolParam(true);
    params.SubtitlesParser = new StEnumParam(1, tr(MENU_SUBTITLES_PARSER));
    params.SubtitlesParser->changeValues().add(tr(MENU_SUBTITLES_PLAIN_TEXT));
    params.SubtitlesParser->changeValues().add(tr(MENU_SUBTITLES_LITE_HTML));

    params.alDevice = new StALDeviceParam();
    params.AudioGain = new StFloat32Param( 0.0f, // sound is unattenuated
                                         -50.0f, // almost mute
                                          10.0f, // max amplification
                                           0.0f, // default
                                           1.0f, // step
                                         1.e-3f);
    params.AudioGain->signals.onChanged = stSlot(this, &StMoviePlayer::doSetAudioVolume);
    params.AudioMute    = new StBoolParam(false);
    params.AudioMute->signals.onChanged = stSlot(this, &StMoviePlayer::doSetAudioMute);
    params.AudioDelay   = new StFloat32Param(0.0f, -5.0f, 5.0f, 0.0f, 0.100f);
    params.AudioDelay->signals.onChanged = stSlot(this, &StMoviePlayer::doSetAudioDelay);

    params.isFullscreen = new StBoolParam(false);
    params.isFullscreen->signals.onChanged = stSlot(this, &StMoviePlayer::doFullscreen);
    params.toRestoreRatio   = new StBoolParam(false);
    params.isShuffle        = new StBoolParam(false);
    params.ToLoopSingle     = new StBoolParam(false);
    params.areGlobalMKeys   = new StBoolParam(true);
    params.checkUpdatesDays = new StInt32Param(7);
    params.srcFormat        = new StInt32Param(ST_V_SRC_AUTODETECT);
    params.srcFormat->signals.onChanged = stSlot(this, &StMoviePlayer::doSwitchSrcFormat);
    params.ToShowPlayList   = new StBoolParam(false);
    params.ToShowPlayList->signals.onChanged = stSlot(this, &StMoviePlayer::doShowPlayList);
    params.ToShowFps   = new StBoolParam(false);
    params.IsVSyncOn   = new StBoolParam(true);
    params.IsVSyncOn->signals.onChanged = stSlot(this, &StMoviePlayer::doSwitchVSync);
    StApplication::params.VSyncMode->setValue(StGLContext::VSync_ON);
    params.ToLimitFps       = new StBoolParam(true);
    params.StartWebUI       = new StEnumParam(WEBUI_OFF, "Web UI start option");
    params.StartWebUI->changeValues().add(tr(MENU_MEDIA_WEBUI_OFF));
    params.StartWebUI->changeValues().add(tr(MENU_MEDIA_WEBUI_ONCE));
    params.StartWebUI->changeValues().add(tr(MENU_MEDIA_WEBUI_ON));
    params.ToPrintWebErrors = new StBoolParam(true);
    params.WebUIPort        = new StInt32Param(8080);
    params.audioStream = new StInt32Param(-1);
    params.audioStream->signals.onChanged = stSlot(this, &StMoviePlayer::doSwitchAudioStream);
    params.subtitlesStream = new StInt32Param(-1);
    params.subtitlesStream->signals.onChanged = stSlot(this, &StMoviePlayer::doSwitchSubtitlesStream);
    params.blockSleeping = new StInt32Param(StMoviePlayer::BLOCK_SLEEP_PLAYBACK);
    params.ToShowExtra   = new StBoolParam(false);
    params.TargetFps = 2; // set rendering FPS as twice as average video FPS
    params.UseGpu = new StBoolParam(false);
    params.SnapshotImgType = new StInt32Param(StImageFile::ST_TYPE_JPEG);

    // load settings
    mySettings->loadInt32 (ST_SETTING_FPSTARGET,          params.TargetFps);
    mySettings->loadString(ST_SETTING_LAST_FOLDER,        params.lastFolder);
    mySettings->loadInt32 (ST_SETTING_UPDATES_LAST_CHECK, myLastUpdateDay);
    mySettings->loadParam (ST_SETTING_UPDATES_INTERVAL,   params.checkUpdatesDays);
    mySettings->loadParam (ST_SETTING_SHUFFLE,            params.isShuffle);
    mySettings->loadParam (ST_SETTING_LOOP_SINGLE,        params.ToLoopSingle);
    mySettings->loadParam (ST_SETTING_GLOBAL_MKEYS,       params.areGlobalMKeys);
    mySettings->loadParam (ST_SETTING_SHOW_LIST,          params.ToShowPlayList);
    mySettings->loadParam (ST_SETTING_SUBTITLES_SIZE,     params.SubtitlesSize);
    mySettings->loadParam (ST_SETTING_SUBTITLES_PARALLAX, params.SubtitlesParallax);
    mySettings->loadParam (ST_SETTING_SUBTITLES_PARSER,   params.SubtitlesParser);
    mySettings->loadParam (ST_SETTING_SEARCH_SUBS,        params.ToSearchSubs);

    mySettings->loadParam (ST_SETTING_SHOW_FPS,           params.ToShowFps);
    mySettings->loadParam (ST_SETTING_VSYNC,              params.IsVSyncOn);
    mySettings->loadParam (ST_SETTING_LIMIT_FPS,          params.ToLimitFps);
    mySettings->loadParam (ST_SETTING_GPU_DECODING,       params.UseGpu);

    mySettings->loadParam (ST_SETTING_WEBUI_ON,           params.StartWebUI);
    mySettings->loadParam (ST_SETTING_WEBUI_PORT,         params.WebUIPort);
    mySettings->loadParam (ST_SETTING_WEBUI_ERRORS,       params.ToPrintWebErrors);
    mySettings->loadParam (ST_SETTING_SAVE_IMG_TYPE,      params.SnapshotImgType);
    mySettings->loadParam (ST_SETTING_EXPERIMENTAL,       params.ToShowExtra);
    if(params.StartWebUI->getValue() == WEBUI_ONCE) {
        params.StartWebUI->setValue(WEBUI_OFF);
    }
    params.StartWebUI->signals.onChanged += stSlot(this, &StMoviePlayer::doSwitchWebUI);

    StString aSavedALDevice;
    mySettings->loadString(ST_SETTING_OPENAL_DEVICE,      aSavedALDevice);
    params.alDevice->init(aSavedALDevice);

    params.isShuffle   ->signals.onChanged.connect(this, &StMoviePlayer::doSwitchShuffle);
    params.ToLoopSingle->signals.onChanged.connect(this, &StMoviePlayer::doSwitchLoopSingle);
    params.alDevice    ->signals.onChanged.connect(this, &StMoviePlayer::doSwitchAudioDevice);

    addRenderer(new StOutAnaglyph(theParentWin));
    addRenderer(new StOutDual(theParentWin));
    addRenderer(new StOutIZ3D(theParentWin));
    addRenderer(new StOutInterlace(theParentWin));
    addRenderer(new StOutDistorted(theParentWin));
    addRenderer(new StOutPageFlipExt(theParentWin));

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

    anAction = new StActionIntSlot(stCString("DoQuit"), stSlot(this, &StMoviePlayer::doQuit), 0);
    addAction(Action_Quit, anAction, ST_VK_ESCAPE);

    anAction = new StActionBool(stCString("DoFullscreen"), params.isFullscreen);
    addAction(Action_Fullscreen, anAction, ST_VK_F, ST_VK_RETURN);

    anAction = new StActionBool(stCString("DoShowFPS"), params.ToShowFps);
    addAction(Action_ShowFps, anAction, ST_VK_F12);

    anAction = new StActionIntValue(stCString("DoSrcAuto"), params.srcFormat, ST_V_SRC_AUTODETECT);
    addAction(Action_SrcAuto, anAction, ST_VK_A);

    anAction = new StActionIntValue(stCString("DoSrcMono"), params.srcFormat, ST_V_SRC_MONO);
    addAction(Action_SrcMono, anAction, ST_VK_M);

    anAction = new StActionIntValue(stCString("DoSrcOverUnder"), params.srcFormat, ST_V_SRC_OVER_UNDER_LR);
    addAction(Action_SrcOverUnderLR, anAction, ST_VK_O);

    anAction = new StActionIntValue(stCString("DoSrcSideBySide"), params.srcFormat, ST_V_SRC_SIDE_BY_SIDE);
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

    anAction = new StActionIntSlot(stCString("DoOpen1File"), stSlot(this, &StMoviePlayer::doOpen1File), 0);
    addAction(Action_Open1File, anAction, ST_VK_O | ST_VF_CONTROL);

    anAction = new StActionIntSlot(stCString("DoSnapshot"), stSlot(this, &StMoviePlayer::doSnapshot), StImageFile::ST_TYPE_NONE);
    addAction(Action_SaveSnapshot, anAction, ST_VK_S | ST_VF_CONTROL);

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
    addAction(Action_CopyToClipboard, anAction, ST_VK_C | ST_VF_CONTROL, ST_VK_INSERT | ST_VF_CONTROL);

    anAction = new StActionIntSlot(stCString("DoPlayListReverse"), stSlot(this, &StMoviePlayer::doPlayListReverse), 0);
    addAction(Action_ShowList, anAction, ST_VK_L | ST_VF_CONTROL);

    anAction = new StActionIntSlot(stCString("DoImageAdjustReset"), stSlot(this, &StMoviePlayer::doImageAdjustReset), 0);
    addAction(Action_ImageAdjustReset, anAction);
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

    mySettings->saveParam (ST_SETTING_STEREO_MODE, myGUI->myImage->params.displayMode);
    mySettings->saveInt32 (ST_SETTING_GAMMA,       stRound(100.0f * myGUI->myImage->params.gamma->getValue()));
    if(params.toRestoreRatio->getValue()) {
        mySettings->saveParam(ST_SETTING_RATIO,    myGUI->myImage->params.displayRatio);
    } else {
        mySettings->saveInt32(ST_SETTING_RATIO,    StGLImageRegion::RATIO_AUTO);
    }
    mySettings->saveParam (ST_SETTING_TEXFILTER,   myGUI->myImage->params.textureFilter);
}

void StMoviePlayer::releaseDevice() {
    saveGuiParams();
    if(!myGUI.isNull()) {
        mySettings->saveParam (ST_SETTING_SCALE_ADJUST,       params.ScaleAdjust);
        mySettings->saveParam (ST_SETTING_SCALE_FORCE2X,      params.ScaleHiDPI2X);
        mySettings->saveParam (ST_SETTING_SUBTITLES_SIZE,     params.SubtitlesSize);
        mySettings->saveParam (ST_SETTING_SUBTITLES_PARALLAX, params.SubtitlesParallax);
        mySettings->saveParam (ST_SETTING_SUBTITLES_PARSER,   params.SubtitlesParser);
        mySettings->saveParam (ST_SETTING_SEARCH_SUBS,        params.ToSearchSubs);
        mySettings->saveInt32 (ST_SETTING_FPSTARGET,          params.TargetFps);
        mySettings->saveString(ST_SETTING_OPENAL_DEVICE,      params.alDevice->getTitle());
        mySettings->saveInt32 (ST_SETTING_UPDATES_LAST_CHECK, myLastUpdateDay);
        mySettings->saveParam (ST_SETTING_UPDATES_INTERVAL,   params.checkUpdatesDays);
        mySettings->saveParam (ST_SETTING_SRCFORMAT,          params.srcFormat);
        mySettings->saveParam (ST_SETTING_SHUFFLE,            params.isShuffle);
        mySettings->saveParam (ST_SETTING_LOOP_SINGLE,        params.ToLoopSingle);
        mySettings->saveParam (ST_SETTING_GLOBAL_MKEYS,       params.areGlobalMKeys);
        mySettings->saveParam (ST_SETTING_SHOW_LIST,          params.ToShowPlayList);

        mySettings->saveParam (ST_SETTING_SHOW_FPS,           params.ToShowFps);
        mySettings->saveParam (ST_SETTING_VSYNC,              params.IsVSyncOn);
        mySettings->saveParam (ST_SETTING_LIMIT_FPS,          params.ToLimitFps);
        mySettings->saveParam (ST_SETTING_GPU_DECODING,       params.UseGpu);

        mySettings->saveParam (ST_SETTING_WEBUI_ON,           params.StartWebUI);
        mySettings->saveParam (ST_SETTING_WEBUI_PORT,         params.WebUIPort);
        mySettings->saveParam (ST_SETTING_WEBUI_ERRORS,       params.ToPrintWebErrors);
        mySettings->saveParam (ST_SETTING_SAVE_IMG_TYPE,      params.SnapshotImgType);
        mySettings->saveParam (ST_SETTING_EXPERIMENTAL,       params.ToShowExtra);

        if(!myVideo.isNull()) {
            mySettings->saveString(ST_SETTING_RECENT_FILES,   myPlayList->dumpRecentList());
        }

        // store hot-keys
        for(std::map< int, StHandle<StAction> >::iterator anIter = myActions.begin();
            anIter != myActions.end(); ++anIter) {
            mySettings->saveHotKey(anIter->second);
        }
    }

    // release GUI data and GL resources before closing the window
    myKeyActions.clear();
    myGUI.nullify();
    myContext.nullify();
}

StMoviePlayer::~StMoviePlayer() {
    doStopWebUI();

    myUpdates.nullify();
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
    myGUI = new StMoviePlayerGUI(this, myWindow.access(), myLangMap.access(), myPlayList, theTextureQueue, theSubQueue);
    myGUI->setContext(myContext);

    // load settings
    mySettings->loadParam (ST_SETTING_STEREO_MODE, myGUI->myImage->params.displayMode);
    mySettings->loadParam (ST_SETTING_TEXFILTER,   myGUI->myImage->params.textureFilter);
    mySettings->loadParam (ST_SETTING_RATIO,       myGUI->myImage->params.displayRatio);
    params.toRestoreRatio->setValue(myGUI->myImage->params.displayRatio->getValue() != StGLImageRegion::RATIO_AUTO);
    int32_t loadedGamma = 100; // 1.0f
        mySettings->loadInt32(ST_SETTING_GAMMA, loadedGamma);
        myGUI->myImage->params.gamma->setValue(0.01f * loadedGamma);

    // initialize frame region early to show dedicated error description
    if(!myGUI->myImage->stglInit()) {
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

void StMoviePlayer::doImageAdjustReset(const size_t ) {
    if(myGUI.isNull()) {
        return;
    }

    myGUI->myImage->params.gamma     ->reset();
    myGUI->myImage->params.brightness->reset();
    myGUI->myImage->params.saturation->reset();
}


void StMoviePlayer::doDeleteFileBegin(const size_t ) {
    //if(!myFileToDelete.isNull()) {
    //    return;
    //}

    myFileToDelete = myVideo->getPlayList().getCurrentFile();
    if(myFileToDelete.isNull()
    || myFileToDelete->size() != 0) {
        myFileToDelete.nullify();
        return;
    }

    const StString aText = StString("Do you really want to completely remove this file?\n")
                         + myFileToDelete->getPath() + "";

    StGLMessageBox* aDialog = new StGLMessageBox(myGUI.access(), "Confirmation", aText, 512, 256);
    aDialog->addButton("Delete", true,  96)->signals.onBtnClick += stSlot(this, &StMoviePlayer::doDeleteFileEnd);
    aDialog->addButton("Cancel", false, 96);
    aDialog->setVisibility(true, true);
    aDialog->stglInit();
}

void StMoviePlayer::doDeleteFileEnd(const size_t ) {
    if(myFileToDelete.isNull()
    || myVideo.isNull()) {
        return;
    }

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
    const char* anOptions[] = { "listening_ports", aPort.toCString(), NULL };
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
    myWindow->setAttribute(StWinAttr_GlobalMediaKeys, params.areGlobalMKeys->getValue());

    // create the video playback thread
    if(!isReset) {
        myVideo = new StVideo(params.alDevice->getTitle(), myLangMap, myPlayList, aTextureQueue, aSubQueue);
        myVideo->signals.onError  = stSlot(myMsgQueue.access(), &StMsgQueue::doPushError);
        myVideo->signals.onLoaded = stSlot(this,                &StMoviePlayer::doLoaded);
        myVideo->params.UseGpu       = params.UseGpu;
        myVideo->params.ToSearchSubs = params.ToSearchSubs;

    #ifdef ST_HAVE_MONGOOSE
        doStartWebUI();
    #endif
    }

    myPlayList->setShuffle   (params.isShuffle   ->getValue());
    myPlayList->setLoopSingle(params.ToLoopSingle->getValue());

    StString aRecentList;
    mySettings->loadString(ST_SETTING_RECENT_FILES, aRecentList);
    myPlayList->loadRecentList(aRecentList);

    if(isReset) {
        return true;
    }

    // load this parameter AFTER video thread creation
    mySettings->loadParam(ST_SETTING_SRCFORMAT, params.srcFormat);

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

    return true;
}

void StMoviePlayer::parseArguments(const StArgumentsMap& theArguments) {
    StArgument anArgFullscr    = theArguments[ST_SETTING_FULLSCREEN];
    StArgument anArgViewMode   = theArguments[ST_SETTING_VIEWMODE];
    StArgument anArgSrcFormat  = theArguments[ST_SETTING_SRCFORMAT];
    StArgument anArgShuffle    = theArguments[ST_SETTING_SHUFFLE];
    StArgument anArgLoopSingle = theArguments[ST_SETTING_LOOP_SINGLE];
    StArgument anArgBenchmark  = theArguments[ST_ARGUMENT_BENCHMARK];

    if(anArgFullscr.isValid()) {
        params.isFullscreen->setValue(!anArgFullscr.isValueOff());
    }
    if(anArgViewMode.isValid()) {
        myPlayList->changeDefParams().setViewMode(StStereoParams::GET_VIEW_MODE_FROM_STRING(anArgViewMode.getValue()));
    }
    if(anArgSrcFormat.isValid()) {
        params.srcFormat->setValue(st::formatFromString(anArgSrcFormat.getValue()));
    }
    if(anArgShuffle.isValid()) {
        params.isShuffle->setValue(!anArgShuffle.isValueOff());
    }
    if(anArgLoopSingle.isValid()) {
        params.ToLoopSingle->setValue(!anArgLoopSingle.isValueOff());
    }
    if(anArgBenchmark.isValid()) {
        myIsBenchmark = !anArgBenchmark.isValueOff();
        myVideo->setBenchmark(myIsBenchmark);
    }
}

bool StMoviePlayer::open() {
    const bool isReset = !mySwitchTo.isNull();
    if(!StApplication::open()
    || !init()) {
        myMsgQueue->popAll();
        return false;
    }

    if(isReset) {
        //myVideo->doLoadNext();
        doUpdateStateLoaded();
        return true;
    }

    parseArguments(myOpenFileInfo->getArgumentsMap());
    const StMIME anOpenMIME = myOpenFileInfo->getMIME();
    if(myOpenFileInfo->getPath().isEmpty()) {
        // open drawer without files
        const StArgument anArgLast = myOpenFileInfo->getArgumentsMap()[ST_ARGUMENT_FILE_LAST];
        if(anArgLast.isValid() && !anArgLast.isValueOff()) {
            doOpenRecent(0); // open last opened file
        }
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
        myVideo->pushPlayEvent(ST_PLAYEVENT_RESUME);
        myVideo->doLoadNext();
    }
    return true;
}

void StMoviePlayer::doResize(const StSizeEvent& ) {
    if(myGUI.isNull()) {
        return;
    }

    myGUI->stglResize(myWindow->stglViewport(ST_WIN_MASTER));
}

void StMoviePlayer::doKeyDown(const StKeyEvent& theEvent) {
    if(myGUI.isNull()) {
        return;
    }

    if(myGUI->getFocus() != NULL) {
        myGUI->doKeyDown(theEvent);
        return;
    }

    StApplication::doKeyDown(theEvent);
    switch(theEvent.VKey) {
        // post process keys
        case ST_VK_B: {
            if(theEvent.Flags == ST_VF_SHIFT) {
                myGUI->myImage->params.brightness->increment();
            } else if(theEvent.Flags == ST_VF_CONTROL) {
                myGUI->myImage->params.brightness->decrement();
            } else {
            #ifdef __ST_DEBUG__
                myIsBenchmark = !myIsBenchmark;
                myVideo->setBenchmark(myIsBenchmark);
            #endif
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

    if(myGUI->getFocus() != NULL) {
        myGUI->doKeyUp(theEvent);
    } else {
        myGUI->myImage->doKeyUp(theEvent);
    }
}

void StMoviePlayer::doMouseDown(const StClickEvent& theEvent) {
    if(myGUI.isNull()) {
        return;
    }

    myGUI->tryClick(StPointD_t(theEvent.PointX, theEvent.PointY), theEvent.Button);
}

void StMoviePlayer::doMouseUp(const StClickEvent& theEvent) {
    if(myGUI.isNull()) {
        return;
    }

    const StPointD_t aPnt(theEvent.PointX, theEvent.PointY);
    const bool isNeutral = myGUI->getFocus() == NULL
                       || !myGUI->getFocus()->isPointIn(aPnt);
    if(!isNeutral) {
        myGUI->tryUnClick(aPnt, theEvent.Button);
        return;
    }

    switch(theEvent.Button) {
        case ST_MOUSE_MIDDLE: {
            params.isFullscreen->reverse();
            break;
        }
        case ST_MOUSE_SCROLL_LEFT:
        case ST_MOUSE_SCROLL_RIGHT: {
            // limit seeking by scroll to lower corner
            if(theEvent.PointY > 0.75) {
                if(theEvent.Button == ST_MOUSE_SCROLL_RIGHT) {
                    doSeekRight();
                } else {
                    doSeekLeft();
                }
            }
        }
        case ST_MOUSE_SCROLL_V_UP:
        case ST_MOUSE_SCROLL_V_DOWN: {
            if(theEvent.PointY > 0.75) {
                break;
            }
        }
        default: {
            myGUI->tryUnClick(aPnt, theEvent.Button);
            break;
        }
    }
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
    params.audioStream->setValue(aValue);
}

void StMoviePlayer::doSubtitlesNext(size_t theDirection) {
    if(myVideo.isNull()) {
        return;
    }

    const int32_t aValue = myVideo->params.activeSubtitles->nextValue(theDirection == 1 ? 1 : -1);
    params.subtitlesStream->setValue(aValue);
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

void StMoviePlayer::doFileNext() {
    if(myVideo.isNull()) {
        return;
    }

    doUpdateStateLoading();
    myVideo->pushPlayEvent(ST_PLAYEVENT_RESUME);
    myVideo->doLoadNext();
}

void StMoviePlayer::doFileDrop(const StDNDropEvent& theEvent) {
    const StString aFilePath = theEvent.File;
    if(myPlayList->checkExtension(aFilePath)) {
        myPlayList->open(aFilePath);
        doUpdateStateLoading();
        myVideo->pushPlayEvent(ST_PLAYEVENT_RESUME);
        myVideo->doLoadNext();
    }
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

    if(params.ScaleHiDPI->setValue(myWindow->getScaleFactor())
    || myToRecreateMenu) {
        StHandle<StGLTextureQueue> aTextureQueue;
        StHandle<StSubQueue>       aSubQueue;
        createGui(aTextureQueue, aSubQueue);

        myGUI->updateAudioStreamsMenu(myVideo->params.activeAudio->getList(),
                                      myVideo->hasVideoStream());
        myGUI->updateSubtitlesStreamsMenu(myVideo->params.activeSubtitles->getList(),
                                          myVideo->hasAudioStream() || myVideo->hasVideoStream());

        myToRecreateMenu = false;
    }

    const bool isMouseMove = myWindow->isMouseMoved();
    if(myEventLoaded.checkReset()) {
        doUpdateStateLoaded();
    }

    if(myIsBenchmark
    || !params.ToLimitFps->getValue()) {
        // do not limit FPS
        myWindow->setTargetFps(-1.0);
    } else if(params.TargetFps >= 1
           && params.TargetFps <= 3) {
        // set rendering FPS to 2x averageFPS
        double aTargetFps = myVideo->getAverFps();
        if(aTargetFps < 17.0
        || aTargetFps > 120.0) {
            aTargetFps = 0.0;
        } else if(aTargetFps < 40.0) {
            aTargetFps *= double(params.TargetFps);
        }
        myWindow->setTargetFps(aTargetFps);
    } else {
        // set rendering FPS to set value in settings
        myWindow->setTargetFps(double(params.TargetFps));
    }

    if(myToCheckUpdates && !myUpdates.isNull() && myUpdates->isInitialized()) {
        if(myUpdates->isNeedUpdate()) {
            myGUI->showUpdatesNotify();
        }
        myToCheckUpdates = false;
    }
    myGUI->setVisibility(myWindow->getMousePos(), isMouseMove);
}

void StMoviePlayer::doUpdateOpenALDeviceList(const size_t ) {
    myToUpdateALList = true;
}

void StMoviePlayer::stglDraw(unsigned int theView) {
    if(!myContext.isNull()
    && myContext->core20fwd != NULL) {
        // clear the screen and the depth buffer
        myContext->core20fwd->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

    if(myGUI.isNull()) {
        return;
    }

    if(myVideo->isDisconnected() || myToUpdateALList) {
        const StString aPrevDev = params.alDevice->getTitle();
        params.alDevice->initList();
        myGUI->updateOpenALDeviceMenu();
        // switch audio device
        if(!params.alDevice->init(aPrevDev)) {
            // select first existing device if any
            params.alDevice->init(params.alDevice->getTitle());
        }
        myVideo->switchAudioDevice(params.alDevice->getTitle());
        myToUpdateALList = false;
    }
    if(myPlayList->isRecentChanged()) {
        myGUI->updateRecentMenu();
    }

    myGUI->getCamera()->setView(theView);
    if(theView == ST_DRAW_LEFT
    || theView == ST_DRAW_MONO) {
        double aDuration = 0.0;
        double aPts      = 0.0;
        bool isVideoPlayed = false, isAudioPlayed = false;
        bool isPlaying = myVideo->getPlaybackState(aDuration, aPts, isVideoPlayed, isAudioPlayed);
        double aPosition = (aDuration > 0.0) ? (aPts / aDuration) : 0.0;
        if(myGUI->myBtnPlay != NULL) {
            myGUI->myBtnPlay->setFaceId(isPlaying ? 1 : 0); // set play/pause
        }
        if(myGUI->myTimeBox != NULL) {
            myGUI->myTimeBox->setTime(aPts, aDuration);
        }
        myGUI->stglUpdate(myWindow->getMousePos(), GLfloat(aPosition), aPts);

        // prevent display going to sleep
        bool toBlockSleepDisplay = false;
        bool toBlockSleepSystem  = false;
        if(myIsBenchmark) {
            toBlockSleepDisplay = true;
            toBlockSleepSystem  = true;
        } else {
            switch(params.blockSleeping->getValue()) {
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
                    toBlockSleepDisplay = myWindow->isFullScreen();;
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
        StHandle<StStereoParams> aParams = myGUI->myImage->getSource();
        if(!aParams.isNull()) {
            myWindow->setStereoOutput(!aParams->isMono()
                                && (myGUI->myImage->params.displayMode->getValue() == StGLImageRegion::MODE_STEREO));
        }
    }

    // draw GUI
    myGUI->stglDraw(theView);
}

void StMoviePlayer::doShowPlayList(const bool theToShow) {
    if(myGUI.isNull()) {
        return;
    }

    myGUI->myPlayList->setVisibility(theToShow, true);
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
        myVideo->switchAudioDevice(params.alDevice->getTitle());
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
    myGUI->updateAudioStreamsMenu(myVideo->params.activeAudio->getList(),
                                  myVideo->hasVideoStream());
    params.audioStream->setValue(myVideo->params.activeAudio->getValue());
    myGUI->updateSubtitlesStreamsMenu(myVideo->params.activeSubtitles->getList(),
                                      myVideo->hasAudioStream() || myVideo->hasVideoStream());
    params.subtitlesStream->setValue(myVideo->params.activeSubtitles->getValue());
    if(mySeekOnLoad > 0.0) {
        myVideo->pushPlayEvent(ST_PLAYEVENT_SEEK, mySeekOnLoad);
        mySeekOnLoad = -1.0;
    }
    if(myAudioOnLoad >= 0) {
        myVideo->params.activeAudio->setValue(myAudioOnLoad);
        params.audioStream->setValue(myAudioOnLoad);
        myAudioOnLoad = -1;
    }
    if(mySubsOnLoad >= 0) {
        myVideo->params.activeSubtitles->setValue(mySubsOnLoad);
        params.subtitlesStream->setValue(mySubsOnLoad);
        mySubsOnLoad = -1;
    }
}

void StMoviePlayer::doAboutFile(const size_t ) {
    if(!myGUI.isNull()) {
        myGUI->doAboutFile(0);
    }
}

void StMoviePlayer::doSwitchSrcFormat(const int32_t theSrcFormat) {
    myVideo->setSrcFormat(StFormatEnum(theSrcFormat));
}

void StMoviePlayer::doReset(const size_t ) {
    StHandle<StStereoParams> aParams = myGUI->myImage->getSource();
    if(!aParams.isNull()) {
        aParams->reset();
    }
}

void StMoviePlayer::doScaleGui(const int32_t ) {
    if(myGUI.isNull()) {
        return;
    }
    myToRecreateMenu = true;
}

void StMoviePlayer::doScaleHiDPI(const bool ) {
    if(myGUI.isNull()) {
        return;
    }
    myToRecreateMenu = true;
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

void StMoviePlayer::doQuit(const size_t ) {
    StApplication::exit(0);
}

struct ST_LOCAL OpenFileArgs {
    StMoviePlayer* receiverPtr; size_t openType;
};

extern SV_THREAD_FUNCTION openFileThread(void* theArg) {
    OpenFileArgs* aThreadArgs = (OpenFileArgs* )theArg;
    aThreadArgs->receiverPtr->doOpenFileDialog(aThreadArgs->openType);
    delete aThreadArgs;
    return SV_THREAD_RETURN 0;
}

static void doOpenFileThreaded(void*  theReceiverPtr,
                               size_t theOpenType) {
    OpenFileArgs* aThreadArgs = new OpenFileArgs();
    aThreadArgs->receiverPtr = (StMoviePlayer* )theReceiverPtr;
    aThreadArgs->openType    = theOpenType;
    aThreadArgs->receiverPtr->params.isFullscreen->setValue(false); // workaround
    StThread(openFileThread, (void* )aThreadArgs);
}

void StMoviePlayer::doOpen1File(const size_t ) {
    doOpenFileThreaded(this, OPEN_FILE_MOVIE);
}

void StMoviePlayer::doOpen2Files(const size_t ) {
    doOpenFileThreaded(this, OPEN_FILE_2MOVIES);
}

void StMoviePlayer::doOpenRecent(const size_t theItemId) {
    if(myVideo.isNull()) {
        return;
    }
    myPlayList->openRecent(theItemId);
    doUpdateStateLoading();
    myVideo->pushPlayEvent(ST_PLAYEVENT_RESUME);
    myVideo->doLoadNext();
}

void StMoviePlayer::doClearRecent(const size_t ) {
    myPlayList->clearRecent();
}

void StMoviePlayer::doAddAudioStream(const size_t ) {
    doOpenFileThreaded(this, OPEN_STREAM_AUDIO);
}

void StMoviePlayer::doAddSubtitleStream(const size_t ) {
    doOpenFileThreaded(this, OPEN_STREAM_SUBTITLES);
}

void StMoviePlayer::doOpenFileDialog(const size_t theOpenType) {
    if(myEventDialog.check()) {
        return;
    }
    myEventDialog.set();

    StHandle<StFileNode> aCurrFile = myPlayList->getCurrentFile();
    if(params.lastFolder.isEmpty()) {
        if(!aCurrFile.isNull()) {
            params.lastFolder = aCurrFile->isEmpty() ? aCurrFile->getFolderPath() : aCurrFile->getValue(0)->getFolderPath();
        }
    }
    StString aTitle;
    switch(theOpenType) {
        case OPEN_FILE_2MOVIES: {
            aTitle = tr(DIALOG_OPEN_LEFT);
            break;
        }
        case OPEN_STREAM_AUDIO: {
            aTitle = "Choose audio file to attach";
            if(aCurrFile.isNull()) {
                myEventDialog.reset();
                return;
            }
            break;
        }
        case OPEN_STREAM_SUBTITLES: {
            aTitle = "Choose subtitles file to attach";
            if(aCurrFile.isNull()) {
                myEventDialog.reset();
                return;
            }
            break;
        }
        case OPEN_FILE_MOVIE:
        default: {
            aTitle = tr(DIALOG_OPEN_FILE);
        }
    }

    const StMIMEList& aMimeList =  (theOpenType == OPEN_STREAM_AUDIO)     ? myVideo->getMimeListAudio()
                                : ((theOpenType == OPEN_STREAM_SUBTITLES) ? myVideo->getMimeListSubtitles()
                                : myVideo->getMimeListVideo());

    StString aFilePath, aDummy;
    if(!StFileNode::openFileDialog(params.lastFolder, aTitle, aMimeList, aFilePath, false)) {
        myEventDialog.reset();
        return;
    }

    switch(theOpenType) {
        case OPEN_FILE_2MOVIES: {
            aTitle = tr(DIALOG_OPEN_RIGHT);
            StFileNode::getFolderAndFile(aFilePath, params.lastFolder, aDummy);
            StString aFilePathR;
            if(StFileNode::openFileDialog(params.lastFolder, aTitle, myVideo->getMimeListVideo(), aFilePathR, false)) {
                // meta-file
                myPlayList->clear();
                myPlayList->addOneFile(aFilePath, aFilePathR);
            }
            break;
        }
        case OPEN_STREAM_AUDIO: {
            myPlayList->addToNode(aCurrFile, aFilePath);
            myAudioOnLoad = myVideo->params.activeAudio->getListSize();
            mySubsOnLoad  = myVideo->params.activeSubtitles->getValue();
            mySeekOnLoad  = myVideo->getPts();
            break;
        }
        case OPEN_STREAM_SUBTITLES: {
            myPlayList->addToNode(aCurrFile, aFilePath);
            myAudioOnLoad = myVideo->params.activeAudio->getValue();
            mySubsOnLoad  = myVideo->params.activeSubtitles->getListSize();
            mySeekOnLoad  = myVideo->getPts();
            break;
        }
        case OPEN_FILE_MOVIE:
        default: {
            myPlayList->open(aFilePath);
        }
    }

    doUpdateStateLoading();
    myVideo->pushPlayEvent(ST_PLAYEVENT_RESUME);
    myVideo->doLoadNext();

    if(!aFilePath.isEmpty()) {
        StFileNode::getFolderAndFile(aFilePath, params.lastFolder, aDummy);
    }
    if(!params.lastFolder.isEmpty()) {
        mySettings->saveString(ST_SETTING_LAST_FOLDER, params.lastFolder);
    }
    myEventDialog.reset();
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

void StMoviePlayer::doSeek(const int , const double theSeekX) {
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
        const StString aPath = StProcess::getStShareFolder() + "web" + SYS_FS_SPLITTER + "index.htm";
        mg_send_file(theConnection, aPath.toCString());
        return 1;
    } else if(anURI.isStartsWith(stCString("/web"))) {
        // return Web UI files
        const StString aSubPath = anURI.subString(5, size_t(-1));
        const StString aPath    = StProcess::getStShareFolder() + "web" + SYS_FS_SPLITTER + aSubPath;
        mg_send_file(theConnection, aPath.toCString());
        return 1;
    } else if(anURI.isStartsWith(stCString("/textures"))) {
        // return textures images
        const StString aSubPath = anURI.subString(10, size_t(-1));
        const StString aPath    = StProcess::getStShareFolder() + "textures" + SYS_FS_SPLITTER + aSubPath;
        mg_send_file(theConnection, aPath.toCString());
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
