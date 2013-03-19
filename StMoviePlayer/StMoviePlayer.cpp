/**
 * Copyright © 2007-2013 Kirill Gavrilov <kirill@sview.ru>
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

#include <StSocket/StCheckUpdates.h>

#include <StCore/StCore.h>
#include <StCore/StWindow.h>

#include "StMoviePlayerGUI.h"

#include <StImage/StImageFile.h>

#include <StGL/StGLContext.h>
#include <StGLCore/StGLCore20.h>
#include <StGLWidgets/StGLImageRegion.h>
#include <StGLWidgets/StGLMsgStack.h>
#include <StGLWidgets/StGLSubtitles.h>
#include <StGLWidgets/StGLTextureButton.h>
#include "StTimeBox.h"

#include "StVideo/StVideo.h"
#include "StMoviePlayerStrings.h"

#include <cstdlib> // std::abs(int)

const StString StMoviePlayer::ST_DRAWER_PLUGIN_NAME = "StMoviePlayer";

namespace {
    static bool HAS_LOGGER_ID = StLogger::IdentifyModule("StMoviePlayer");

    static const char ST_SETTING_FPSBOUND[]      = "fpsbound";
    static const char ST_SETTING_SRCFORMAT[]     = "srcFormat";
    static const char ST_SETTING_LAST_FOLDER[]   = "lastFolder";
    static const char ST_SETTING_OPENAL_DEVICE[] = "alDevice";
    static const char ST_SETTING_RECENT_FILES[]  = "recent";

    static const char ST_SETTING_FULLSCREEN[]    = "fullscreen";
    static const char ST_SETTING_VIEWMODE[]      = "viewMode";
    static const char ST_SETTING_STEREO_MODE[]   = "viewStereoMode";
    static const char ST_SETTING_TEXFILTER[]     = "viewTexFilter";
    static const char ST_SETTING_GAMMA[]         = "viewGamma";
    static const char ST_SETTING_SHUFFLE[]       = "shuffle";
    static const char ST_SETTING_GLOBAL_MKEYS[]  = "globalMediaKeys";
    static const char ST_SETTING_RATIO[]         = "ratio";
    static const char ST_SETTING_UPDATES_LAST_CHECK[] = "updatesLastCheck";
    static const char ST_SETTING_UPDATES_INTERVAL[]   = "updatesInterval";

    static const char ST_ARGUMENT_FILE[]       = "file";
    static const char ST_ARGUMENT_FILE_LEFT[]  = "left";
    static const char ST_ARGUMENT_FILE_RIGHT[] = "right";
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
    #if(defined(_WIN32) || defined(__WIN32__))
        aName.fromLocale(aDefDevice);
    #else
        aName.fromUnicode(aDefDevice);
    #endif
        myDevicesList.add(aName);
        return;
    }

    const ALchar* aDevicesNames = alcGetString(NULL, ALC_ALL_DEVICES_SPECIFIER);
    while(aDevicesNames && *aDevicesNames) {
    #if(defined(_WIN32) || defined(__WIN32__))
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

StMoviePlayer::StMoviePlayer()
: myEventDialog(false),
  myEventLoaded(false),
  mySeekOnLoad(-1.0),
  //
  myLastUpdateDay(0),
  myToUpdateALList(false),
  myIsBenchmark(false),
  myToCheckUpdates(true),
  myToQuit(false) {
    //
    params.alDevice = new StALDeviceParam();
    params.audioGain = new StFloat32Param( 1.0f, // sound is unattenuated
                                           0.0f, // mute
                                          10.0f, // max amplification
                                           1.0f, // default
                                         0.001f, // step
                                         1.e-7f);
    params.audioGain->signals.onChanged.connect(this, &StMoviePlayer::doSetAudioVolume);
    params.isFullscreen = new StBoolParam(false);
    params.isFullscreen->signals.onChanged.connect(this, &StMoviePlayer::doFullscreen);
    params.toRestoreRatio   = new StBoolParam(false);
    params.isShuffle        = new StBoolParam(false);
    params.areGlobalMKeys   = new StBoolParam(true);
    params.checkUpdatesDays = new StInt32Param(7);
    params.srcFormat        = new StInt32Param(ST_V_SRC_AUTODETECT);
    params.srcFormat->signals.onChanged.connect(this, &StMoviePlayer::doSwitchSrcFormat);
    params.audioStream = new StInt32Param(-1);
    params.audioStream->signals.onChanged.connect(this, &StMoviePlayer::doSwitchAudioStream);
    params.subtitlesStream = new StInt32Param(-1);
    params.subtitlesStream->signals.onChanged.connect(this, &StMoviePlayer::doSwitchSubtitlesStream);
    params.blockSleeping = new StInt32Param(StMoviePlayer::BLOCK_SLEEP_PLAYBACK);
    params.fpsBound = 1;
}

StMoviePlayer::~StMoviePlayer() {
    myUpdates.nullify();
    if(!mySettings.isNull() && !myGUI.isNull()) {
        mySettings->saveParam (ST_SETTING_STEREO_MODE,        myGUI->stImageRegion->params.displayMode);
        mySettings->saveInt32 (ST_SETTING_GAMMA,              stRound(100.0f * myGUI->stImageRegion->params.gamma->getValue()));
        if(params.toRestoreRatio->getValue()) {
            mySettings->saveParam(ST_SETTING_RATIO,           myGUI->stImageRegion->params.displayRatio);
        } else {
            mySettings->saveInt32(ST_SETTING_RATIO,           StGLImageRegion::RATIO_AUTO);
        }
        mySettings->saveParam (ST_SETTING_TEXFILTER,          myGUI->stImageRegion->params.textureFilter);
        mySettings->saveInt32 (ST_SETTING_FPSBOUND,           params.fpsBound);
        mySettings->saveString(ST_SETTING_OPENAL_DEVICE,      params.alDevice->getTitle());
        mySettings->saveInt32 (ST_SETTING_UPDATES_LAST_CHECK, myLastUpdateDay);
        mySettings->saveParam (ST_SETTING_UPDATES_INTERVAL,   params.checkUpdatesDays);
        mySettings->saveParam (ST_SETTING_SRCFORMAT,          params.srcFormat);
        mySettings->saveParam (ST_SETTING_SHUFFLE,            params.isShuffle);
        mySettings->saveParam (ST_SETTING_GLOBAL_MKEYS,       params.areGlobalMKeys);

        if(!myVideo.isNull()) {
            mySettings->saveString(ST_SETTING_RECENT_FILES,   myVideo->getPlayList().dumpRecentList());
        }
    }
    // release GUI data and GL resorces before closing the window
    myGUI.nullify();
    // wait video playback thread to quit and release resources
    myVideo.nullify();
    // destroy other objects
    mySettings.nullify();
    // now destroy the window
    myWindow.nullify();
    // release libraries
    StCore::FREE();
}

bool StMoviePlayer::init(StWindowInterface* theWindow) {
    if(!StVersionInfo::checkTimeBomb("sView - Video playback plugin")) {
        // timebomb for alpha versions
        return false;
    } else if(theWindow == NULL) {
        stError("VideoPlugin, Invalid window from StRenderer plugin!");
        return false;
    } else if(StCore::INIT() != STERROR_LIBNOERROR) {
        stError("VideoPlugin, Core library not available!");
        return false;
    }

    // create window wrapper
    myWindow = new StWindow(theWindow);
    myWindow->setTitle("sView - Media Player");
    myWindow->stglMakeCurrent(ST_WIN_MASTER);

    // initialize GL context
    myContext = new StGLContext();
    if(!myContext->stglInit()) {
        stError("VideoPlugin, OpenGL context is broken!\n(OpenGL library internal error?)");
        return false;
    } else if(!myContext->isGlGreaterEqual(2, 0)) {
        stError("VideoPlugin, OpenGL2.0+ not available!");
        return false;
    }

    // create the GUI with default values
    myGUI = new StMoviePlayerGUI(this, myWindow.access(), 16);
    myGUI->setContext(myContext);

    // load settings
    mySettings = new StSettings(ST_DRAWER_PLUGIN_NAME);
    mySettings->loadInt32 (ST_SETTING_FPSBOUND,           params.fpsBound);
    mySettings->loadString(ST_SETTING_LAST_FOLDER,        params.lastFolder);
    StString aSavedALDevice;
    mySettings->loadString(ST_SETTING_OPENAL_DEVICE,      aSavedALDevice);
    params.alDevice->init(aSavedALDevice);
    mySettings->loadInt32 (ST_SETTING_UPDATES_LAST_CHECK, myLastUpdateDay);
    mySettings->loadParam (ST_SETTING_UPDATES_INTERVAL,   params.checkUpdatesDays);
    mySettings->loadParam (ST_SETTING_STEREO_MODE,        myGUI->stImageRegion->params.displayMode);
    mySettings->loadParam (ST_SETTING_TEXFILTER,          myGUI->stImageRegion->params.textureFilter);
    mySettings->loadParam (ST_SETTING_RATIO,              myGUI->stImageRegion->params.displayRatio);
    mySettings->loadParam (ST_SETTING_SHUFFLE,            params.isShuffle);
    mySettings->loadParam (ST_SETTING_GLOBAL_MKEYS,       params.areGlobalMKeys);
    params.toRestoreRatio->setValue(myGUI->stImageRegion->params.displayRatio->getValue() != StGLImageRegion::RATIO_AUTO);
    int32_t loadedGamma = 100; // 1.0f
        mySettings->loadInt32(ST_SETTING_GAMMA, loadedGamma);
        myGUI->stImageRegion->params.gamma->setValue(0.01f * loadedGamma);

    // capture multimedia keys even without window focus
    StWinAttributes_t anAttribs = stDefaultWinAttributes();
    myWindow->getAttributes(&anAttribs);
    if(anAttribs.areGlobalMediaKeys != params.areGlobalMKeys->getValue()) {
        anAttribs.areGlobalMediaKeys = params.areGlobalMKeys->getValue();
        myWindow->setAttributes(&anAttribs);
    }

    // initialize frame region early to show dedicated error description
    if(!myGUI->stImageRegion->stglInit()) {
        stError("VideoPlugin, frame region initialization failed!");
        return false;
    }
    myGUI->stglInit();
    if(!mySettings->isValid()) {
        myGUI->myMsgStack->doPushMessage("Settings plugin is not available!\nAll changes will be lost after restart.");
    }

    // create the video playback thread
    myVideo = new StVideo(params.alDevice->getTitle(),
                          myGUI->myLangMap,
                          myGUI->stImageRegion->getTextureQueue(),
                          myGUI->stSubtitles->getQueue());
    myVideo->signals.onError.connect(myGUI->myMsgStack, &StGLMsgStack::doPushMessage);
    myVideo->signals.onLoaded.connect(this, &StMoviePlayer::doLoaded);
    myVideo->getPlayList().setShuffle(params.isShuffle->getValue());
    params.isShuffle->signals.onChanged.connect(this, &StMoviePlayer::doSwitchShuffle);
    params.alDevice->signals.onChanged.connect(this, &StMoviePlayer::doSwitchAudioDevice);

    StString aRecentList;
    mySettings->loadString(ST_SETTING_RECENT_FILES, aRecentList);
    myVideo->getPlayList().loadRecentList(aRecentList);

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
    StArgument argFullscreen = theArguments[ST_SETTING_FULLSCREEN];
    StArgument argViewMode   = theArguments[ST_SETTING_VIEWMODE];
    StArgument argSrcFormat  = theArguments[ST_SETTING_SRCFORMAT];
    StArgument argShuffle    = theArguments[ST_SETTING_SHUFFLE];
    StArgument argBenchmark  = theArguments[ST_ARGUMENT_BENCHMARK];

    if(argFullscreen.isValid()) {
        params.isFullscreen->setValue(!argFullscreen.isValueOff());
    }
    if(argViewMode.isValid()) {
        myVideo->getPlayList().changeDefParams().setViewMode(StStereoParams::GET_VIEW_MODE_FROM_STRING(argViewMode.getValue()));
    }
    if(argSrcFormat.isValid()) {
        params.srcFormat->setValue(st::formatFromString(argSrcFormat.getValue()));
    }
    if(argShuffle.isValid()) {
        params.isShuffle->setValue(!argShuffle.isValueOff());
    }
    if(argBenchmark.isValid()) {
        myIsBenchmark = !argShuffle.isValueOff();
        myVideo->setBenchmark(myIsBenchmark);
    }
}

bool StMoviePlayer::open(const StOpenInfo& stOpenInfo) {
    parseArguments(stOpenInfo.getArgumentsMap());
    StMIME stOpenMIME = stOpenInfo.getMIME();
    if(stOpenMIME == StDrawerInfo::DRAWER_MIME() || stOpenInfo.getPath().isEmpty()) {
        // open drawer without files
        return true;
    }

    // clear playlist first
    myVideo->getPlayList().clear();

    //StArgument argFile1     = stOpenInfo.getArgumentsMap()[ST_ARGUMENT_FILE + 1]; // playlist?
    StArgument argFileLeft  = stOpenInfo.getArgumentsMap()[ST_ARGUMENT_FILE_LEFT];
    StArgument argFileRight = stOpenInfo.getArgumentsMap()[ST_ARGUMENT_FILE_RIGHT];
    if(argFileLeft.isValid() && argFileRight.isValid()) {
        // meta-file
        /// TODO (Kirill Gavrilov#4) we should use MIME type!
        myVideo->getPlayList().addOneFile(argFileLeft.getValue(), argFileRight.getValue());
    } else if(!stOpenMIME.isEmpty()) {
        // create just one-file playlist
        myVideo->getPlayList().addOneFile(stOpenInfo.getPath(), stOpenMIME);
    } else {
        // create playlist from file's folder
        myVideo->getPlayList().open(stOpenInfo.getPath());
    }

    if(!myVideo->getPlayList().isEmpty()) {
        doUpdateStateLoading();
        myVideo->pushPlayEvent(ST_PLAYEVENT_RESUME);
        myVideo->doLoadNext();
    }
    return true;
}

void StMoviePlayer::parseCallback(StMessage_t* stMessages) {
    if(myToQuit) {
        stMessages[0].uin = StMessageList::MSG_EXIT;
        stMessages[1].uin = StMessageList::MSG_NULL;
    }
    bool isMouseMove = false;
    for(size_t evId = 0; stMessages[evId].uin != StMessageList::MSG_NULL; ++evId) {
        switch(stMessages[evId].uin) {
            case StMessageList::MSG_RESIZE: {
                myGUI->stglResize(myWindow->getPlacement());
                break;
            }
            case StMessageList::MSG_FULLSCREEN_SWITCH: {
                params.isFullscreen->setValue(myWindow->isFullScreen());
                break;
            }
            case StMessageList::MSG_DRAGNDROP_IN: {
                int filesCount = myWindow->getDragNDropFile(-1, NULL, 0);
                if(filesCount > 0) {
                    stUtf8_t aBuffFile[4096];
                    stMemSet(aBuffFile, 0, sizeof(aBuffFile));
                    if(myWindow->getDragNDropFile(0, aBuffFile, (4096 * sizeof(stUtf8_t))) == 0) {
                        StString aBuffString(aBuffFile);
                        if(myVideo->getPlayList().checkExtension(aBuffString)) {
                            myVideo->getPlayList().open(aBuffString);
                            doUpdateStateLoading();
                            myVideo->pushPlayEvent(ST_PLAYEVENT_RESUME);
                            myVideo->doLoadNext();
                        }
                    }
                }
                break;
            }
            case StMessageList::MSG_CLOSE:
            case StMessageList::MSG_EXIT: {
                stMessages[0].uin = StMessageList::MSG_EXIT;
                stMessages[1].uin = StMessageList::MSG_NULL;
                break;
            }
            case StMessageList::MSG_KEYS: {
                bool* keysMap = (bool* )stMessages[evId].data;
                if(keysMap[ST_VK_ESCAPE]) {
                    // we could parse Escape key in other way
                    stMessages[0].uin = StMessageList::MSG_EXIT;
                    stMessages[1].uin = StMessageList::MSG_NULL;
                    return;
                }
                keysCommon((bool* )stMessages[evId].data); break;
            }
            case StMessageList::MSG_MOUSE_MOVE: {
                isMouseMove = true; break;
            }
            case StMessageList::MSG_MOUSE_DOWN: {
                StPointD_t pt;
                int mouseBtn = myWindow->getMouseDown(&pt);
                myGUI->tryClick(pt, mouseBtn);
                break;
            }
            case StMessageList::MSG_MOUSE_UP: {
                StPointD_t aPoint;
                int aMouseBtn = myWindow->getMouseUp(&aPoint);
                switch(aMouseBtn) {
                    case ST_MOUSE_MIDDLE: {
                        params.isFullscreen->reverse();
                        break;
                    }
                    case ST_MOUSE_SCROLL_LEFT:
                    case ST_MOUSE_SCROLL_RIGHT: {
                        // limit seeking by scroll to lower corner
                        if(aPoint.y() > 0.75) {
                            if(aMouseBtn == ST_MOUSE_SCROLL_RIGHT) {
                                doSeekRight();
                            } else {
                                doSeekLeft();
                            }
                        }
                    }
                    case ST_MOUSE_SCROLL_V_UP:
                    case ST_MOUSE_SCROLL_V_DOWN: {
                        if(aPoint.y() > 0.75) {
                            break;
                        }
                    }
                    default: {
                        myGUI->tryUnClick(aPoint, aMouseBtn);
                        break;
                    }
                }
                break;
            }
            case StMessageList::MSG_GO_BACKWARD: {
                doListPrev();
                break;
            }
            case StMessageList::MSG_GO_FORWARD: {
                doListNext();
                break;
            }
        }
    }

    if(myEventLoaded.checkReset()) {
        doUpdateStateLoaded();
    }

    if(myIsBenchmark) {
        // full unbounded
        myWindow->stglSetTargetFps(-1.0);
    } else if(params.fpsBound == 1) {
        // set rendering FPS to 2x averageFPS
        double targetFps = myVideo->getAverFps();
        if(targetFps < 18.0) {
            targetFps = 0.0;
        } else if(targetFps < 40.0) {
            targetFps *= 2.0;
        }
        myWindow->stglSetTargetFps(targetFps);
    } else {
        // set rendering FPS to setted value in settings
        myWindow->stglSetTargetFps(double(params.fpsBound));
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

void StMoviePlayer::stglDraw(unsigned int view) {
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
    if(myVideo->getPlayList().isRecentChanged()) {
        myGUI->updateRecentMenu();
    }

    myGUI->getCamera()->setView(view);
    if(view == ST_DRAW_LEFT) {
        double aDuration = 0.0;
        double aPts      = 0.0;
        bool isVideoPlayed = false, isAudioPlayed = false;
        bool isPlaying = myVideo->getPlaybackState(aDuration, aPts, isVideoPlayed, isAudioPlayed);
        double aPosition = (aDuration > 0.0) ? (aPts / aDuration) : 0.0;
        if(myGUI->btnPlay != NULL) {
            myGUI->btnPlay->setFaceId(isPlaying ? 1 : 0); // set play/pause
        }
        if(myGUI->stTimeBox != NULL) {
            myGUI->stTimeBox->setTime(aPts, aDuration);
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
        StWinAttributes_t anAttribs = stDefaultWinAttributes();
        myWindow->getAttributes(&anAttribs);
        if(anAttribs.toBlockSleepSystem  != toBlockSleepSystem
        || anAttribs.toBlockSleepDisplay != toBlockSleepDisplay) {
            anAttribs.toBlockSleepSystem  = toBlockSleepSystem;
            anAttribs.toBlockSleepDisplay = toBlockSleepDisplay;
            myWindow->setAttributes(&anAttribs);
        }

        myWindow->showCursor(!myGUI->toHideCursor());

        // check for mono state
        StHandle<StStereoParams> aParams = myGUI->stImageRegion->getSource();
        if(!aParams.isNull()) {
            myWindow->setStereoOutput(!aParams->isMono()
                                && (myGUI->stImageRegion->params.displayMode->getValue() == StGLImageRegion::MODE_STEREO));
        }
    }

    // clear the screen and the depth buffer
    myContext->core20fwd->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // draw GUI
    myGUI->stglDraw(view);
}

void StMoviePlayer::doSwitchShuffle(const bool theShuffleOn) {
    myVideo->getPlayList().setShuffle(theShuffleOn);
}

void StMoviePlayer::doSwitchAudioDevice(const int32_t /*theDevId*/) {
    myVideo->switchAudioDevice(params.alDevice->getTitle());
}

void StMoviePlayer::doSetAudioVolume(const float theGain) {
    myVideo->setAudioVolume(theGain);
}

void StMoviePlayer::doUpdateStateLoading() {
    const StString aFileToLoad = myVideo->getPlayList().getCurrentTitle();
    if(aFileToLoad.isEmpty()) {
        myWindow->setTitle("sView - Movie Player");
    } else {
        /// TODO (Kirill Gavrilov#4) - show Loading... after delay
        myWindow->setTitle(aFileToLoad + " Loading... - sView");
    }
}

void StMoviePlayer::doUpdateStateLoaded() {
    const StString aFileToLoad = myVideo->getPlayList().getCurrentTitle();
    if(aFileToLoad.isEmpty()) {
        myWindow->setTitle("sView - Movie Player");
    } else {
        myWindow->setTitle(aFileToLoad + " - sView");
    }
    myGUI->updateAudioStreamsMenu(myVideo->params.activeAudio->getList(),
                                  myVideo->hasVideoStream());
    params.audioStream->setValue(myVideo->params.activeAudio->getValue());
    myGUI->updateSubtitlesStreamsMenu(myVideo->params.activeSubtitles->getList());
    params.subtitlesStream->setValue(myVideo->params.activeSubtitles->getValue());
    if(mySeekOnLoad > 0.0) {
        myVideo->pushPlayEvent(ST_PLAYEVENT_SEEK, mySeekOnLoad);
        mySeekOnLoad = -1.0;
    }
}

void StMoviePlayer::doSwitchSrcFormat(const int32_t theSrcFormat) {
    myVideo->setSrcFormat(StFormatEnum(theSrcFormat));
}

void StMoviePlayer::doReset(const size_t ) {
    StHandle<StStereoParams> aParams = myGUI->stImageRegion->getSource();
    if(!aParams.isNull()) {
        aParams->reset();
    }
}

void StMoviePlayer::doLoaded() {
    myEventLoaded.set();
}

void StMoviePlayer::doListFirst(const size_t ) {
    if(myVideo->getPlayList().walkToFirst()) {
        myVideo->doLoadNext();
        doUpdateStateLoading();
    }
}

void StMoviePlayer::doListPrev(const size_t ) {
    if(myVideo->getPlayList().walkToPrev()) {
        myVideo->doLoadNext();
        doUpdateStateLoading();
    }
}

void StMoviePlayer::doListNext(const size_t ) {
    if(myVideo->getPlayList().walkToNext()) {
        myVideo->doLoadNext();
        doUpdateStateLoading();
    }
}

void StMoviePlayer::doListLast(const size_t ) {
    if(myVideo->getPlayList().walkToLast()) {
        myVideo->doLoadNext();
        doUpdateStateLoading();
    }
}

void StMoviePlayer::doQuit(const size_t ) {
    myToQuit = true;
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
    myVideo->getPlayList().openRecent(theItemId);
    doUpdateStateLoading();
    myVideo->pushPlayEvent(ST_PLAYEVENT_RESUME);
    myVideo->doLoadNext();
}

void StMoviePlayer::doClearRecent(const size_t ) {
    if(myVideo.isNull()) {
        return;
    }
    myVideo->getPlayList().clearRecent();
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

    StHandle<StFileNode> aCurrFile = myVideo->getPlayList().getCurrentFile();
    if(params.lastFolder.isEmpty()) {
        if(!aCurrFile.isNull()) {
            params.lastFolder = aCurrFile->isEmpty() ? aCurrFile->getFolderPath() : aCurrFile->getValue(0)->getFolderPath();
        }
    }
    StString aTitle;
    switch(theOpenType) {
        case OPEN_FILE_2MOVIES: {
            aTitle = myGUI->myLangMap->changeValueId(StMoviePlayerStrings::DIALOG_OPEN_LEFT,
                                                     "Choose LEFT video file to open");
            break;
        }
        case OPEN_STREAM_AUDIO: {
            aTitle = "Choose audio file to attach";
                     ///myGUI->myLangMap->changeValueId(StMoviePlayerStrings::DIALOG_ADD_AUDIO,
                     ///                               "Choose audio stream to attach");
            if(aCurrFile.isNull()) {
                myEventDialog.reset();
                return;
            }
            break;
        }
        case OPEN_STREAM_SUBTITLES: {
            aTitle = "Choose subtitles file to attach";
                     ///myGUI->myLangMap->changeValueId(StMoviePlayerStrings::DIALOG_ADD_AUDIO,
                     ///                               "Choose audio stream to attach");
            if(aCurrFile.isNull()) {
                myEventDialog.reset();
                return;
            }
            break;
        }
        case OPEN_FILE_MOVIE:
        default: {
            aTitle = myGUI->myLangMap->changeValueId(StMoviePlayerStrings::DIALOG_OPEN_FILE,
                                                     "Choose the video file to open");
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
            aTitle = myGUI->myLangMap->changeValueId(StMoviePlayerStrings::DIALOG_OPEN_RIGHT,
                                                     "Choose RIGHT video file to open");
            StFileNode::getFolderAndFile(aFilePath, params.lastFolder, aDummy);
            StString aFilePathR;
            if(StFileNode::openFileDialog(params.lastFolder, aTitle, myVideo->getMimeListVideo(), aFilePathR, false)) {
                // meta-file
                myVideo->getPlayList().clear();
                myVideo->getPlayList().addOneFile(aFilePath, aFilePathR);
            }
            break;
        }
        case OPEN_STREAM_AUDIO:
        case OPEN_STREAM_SUBTITLES: {
            myVideo->getPlayList().addToNode(aCurrFile, aFilePath);
            mySeekOnLoad = myVideo->getPts();
            break;
        }
        case OPEN_FILE_MOVIE:
        default: {
            myVideo->getPlayList().open(aFilePath);
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
    myVideo->doSaveSnapshotAs(theImgType);
}

void StMoviePlayer::keysStereo(bool* keysMap) {
    StHandle<StStereoParams> aParams = myGUI->stImageRegion->getSource();
    if(aParams.isNull()) {
        return;
    }

    if(keysMap['W']) {
        myGUI->stImageRegion->params.swapLR->reverse();
        keysMap['W'] = false;
    }

    // ========= ZOOM factor: + - =========
    if(keysMap[ST_VK_ADD] || keysMap[ST_VK_OEM_PLUS]) {
        aParams->scaleIn();
    }
    if(keysMap[ST_VK_SUBTRACT] || keysMap[ST_VK_OEM_MINUS]) {
        aParams->scaleOut();
    }

    // ========= Separation factor ========
    if(keysMap[ST_VK_CONTROL]) {
        if(keysMap[ST_VK_DIVIDE]) {
            aParams->decSeparationDy();
            keysMap[ST_VK_DIVIDE] = false;
        }
        if(keysMap[ST_VK_COMMA]) {
            aParams->decSeparationDy();
            keysMap[ST_VK_COMMA] = false;
        }
        if(keysMap[ST_VK_MULTIPLY]) {
            aParams->incSeparationDy();
            keysMap[ST_VK_MULTIPLY] = false;
        }
        if(keysMap[ST_VK_PERIOD]) {
            aParams->incSeparationDy();
            keysMap[ST_VK_PERIOD] = false;
        }
    } else {
        if(keysMap[ST_VK_DIVIDE]) {
            aParams->decSeparationDx();
            keysMap[ST_VK_DIVIDE] = false;
        }
        if(keysMap[ST_VK_COMMA]) {
            aParams->decSeparationDx();
            keysMap[ST_VK_COMMA] = false;
        }
        if(keysMap[ST_VK_MULTIPLY]) {
            aParams->incSeparationDx();
            keysMap[ST_VK_MULTIPLY] = false;
        }
        if(keysMap[ST_VK_PERIOD]) {
            aParams->incSeparationDx();
            keysMap[ST_VK_PERIOD] = false;
        }
    }

    // ========= Rotation =======
    if(keysMap[ST_VK_BRACKETLEFT] && keysMap[ST_VK_CONTROL]) { // [
        aParams->decZRotateL();
    }
    if(keysMap[ST_VK_BRACKETRIGHT] && keysMap[ST_VK_CONTROL]) { // ]
        aParams->incZRotateL();
    }
    if(keysMap[ST_VK_BRACKETLEFT] && !keysMap[ST_VK_CONTROL]) { // [
        aParams->decZRotate();
        keysMap[ST_VK_BRACKETLEFT] = false;
    }
    if(keysMap[ST_VK_BRACKETRIGHT] && !keysMap[ST_VK_CONTROL]) { // ]
        aParams->incZRotate();
        keysMap[ST_VK_BRACKETRIGHT] = false;
    }
    if(keysMap[ST_VK_SEMICOLON] && keysMap[ST_VK_CONTROL]) { // ;
        aParams->incSepRotation();
    }
    if(keysMap[ST_VK_APOSTROPHE] && keysMap[ST_VK_CONTROL]) { // '
        aParams->decSepRotation();
    }
    //
    if(keysMap[ST_VK_BACK]) {
        doReset();
    }

    if(keysMap[ST_VK_P]) {
        aParams->nextViewMode();
        keysMap[ST_VK_P] = false;
    }

}

void StMoviePlayer::keysSrcFormat(bool* keysMap) {
    // A (auto)/M (mono)/S (side by side)/O (over under)/I (horizontal interlace)
    if(keysMap[ST_VK_A]) {
        params.srcFormat->setValue(ST_V_SRC_AUTODETECT);
        keysMap[ST_VK_A] = false;
    }
    if(keysMap[ST_VK_M]) {
        params.srcFormat->setValue(ST_V_SRC_MONO);
        keysMap[ST_VK_M] = false;
    }
    if(keysMap[ST_VK_S] && !keysMap[ST_VK_CONTROL]) {
        params.srcFormat->setValue(ST_V_SRC_SIDE_BY_SIDE);
        keysMap[ST_VK_S] = false;
    }
    if(keysMap[ST_VK_O] && !keysMap[ST_VK_CONTROL]) {
        params.srcFormat->setValue(ST_V_SRC_OVER_UNDER_RL);
        keysMap[ST_VK_O] = false;
    }
    if(keysMap[ST_VK_R]) {
        params.srcFormat->setValue(ST_V_SRC_ANAGLYPH_RED_CYAN);
        keysMap[ST_VK_R] = false;
    }

    // Post process keys
    if(keysMap[ST_VK_G] && keysMap[ST_VK_CONTROL]) {
        myGUI->stImageRegion->params.gamma->decrement();
        keysMap[ST_VK_G] = false;
    }
    if(keysMap[ST_VK_G] && keysMap[ST_VK_SHIFT]) {
        myGUI->stImageRegion->params.gamma->increment();
        keysMap[ST_VK_G] = false;
    }

    if(keysMap[ST_VK_B] && keysMap[ST_VK_CONTROL]) {
        myGUI->stImageRegion->params.brightness->decrement();
        keysMap[ST_VK_B] = false;
    }
    if(keysMap[ST_VK_B] && keysMap[ST_VK_SHIFT]) {
        myGUI->stImageRegion->params.brightness->increment();
        keysMap[ST_VK_B] = false;
    }
}

void StMoviePlayer::keysFileWalk(bool* keysMap) {
    if(keysMap[ST_VK_I]) {
        myGUI->doAboutFile(0);
        keysMap[ST_VK_I] = false;
    }

    if(keysMap[ST_VK_O] && keysMap[ST_VK_CONTROL]) {
        doOpen1File();
        keysMap[ST_VK_O] = false;
    }

    // PgDown/PgUp/Home/End
    if(keysMap[ST_VK_PRIOR]) {
        doListPrev();
        keysMap[ST_VK_PRIOR] = false;
    }
    if(keysMap[ST_VK_MEDIA_PREV_TRACK]) {
        doListPrev();
        keysMap[ST_VK_MEDIA_PREV_TRACK] = false;
    }
    if(keysMap[ST_VK_BROWSER_BACK]) {
        doListPrev();
        keysMap[ST_VK_BROWSER_BACK] = false;
    }
    if(keysMap[ST_VK_NEXT]) {
        doListNext();
        keysMap[ST_VK_NEXT] = false;
    }
    if(keysMap[ST_VK_MEDIA_NEXT_TRACK]) {
        doListNext();
        keysMap[ST_VK_MEDIA_NEXT_TRACK] = false;
    }
    if(keysMap[ST_VK_BROWSER_FORWARD]) {
        doListNext();
        keysMap[ST_VK_BROWSER_FORWARD] = false;
    }
    if(keysMap[ST_VK_HOME]) {
        doListFirst();
        keysMap[ST_VK_HOME] = false;
    }
    if(keysMap[ST_VK_END]) {
        doListLast();
        keysMap[ST_VK_END] = false;
    }
}

bool StMoviePlayer::getCurrentFile(StHandle<StFileNode>&     theFileNode,
                                   StHandle<StStereoParams>& theParams,
                                   StHandle<StMovieInfo>&    theInfo) {
    theInfo.nullify();
    if(!myVideo->getPlayList().getCurrentFile(theFileNode, theParams)) {
        return false;
    }
    theInfo = myVideo->getFileInfo(theParams);
    return true;
}

void StMoviePlayer::getRecentList(StArrayList<StString>& theList) {
    if(myVideo.isNull()) {
        return;
    }
    myVideo->getPlayList().getRecentList(theList);
}

void StMoviePlayer::keysCommon(bool* keysMap) {
    if(keysMap[ST_VK_F]) {
        params.isFullscreen->reverse();
        keysMap[ST_VK_F] = false;
    }
    if(keysMap[ST_VK_RETURN]) {
        params.isFullscreen->reverse();
        keysMap[ST_VK_RETURN] = false;
    }

    if(keysMap[ST_VK_SPACE]) {
        doPlayPause();
        keysMap[ST_VK_SPACE] = false;
    }
    if(keysMap[ST_VK_MEDIA_PLAY_PAUSE]) {
        doPlayPause();
        keysMap[ST_VK_MEDIA_PLAY_PAUSE] = false;
    }
    if(keysMap[ST_VK_MEDIA_STOP]) {
        doStop();
        keysMap[ST_VK_MEDIA_STOP] = false;
    }

    if(keysMap[ST_VK_LEFT]) {
        doSeekLeft();
        keysMap[ST_VK_LEFT] = false;
    }
    if(keysMap[ST_VK_RIGHT]) {
        doSeekRight();
        keysMap[ST_VK_RIGHT] = false;
    }

    if(keysMap[ST_VK_S] && keysMap[ST_VK_CONTROL]) {
        doSnapshot(StImageFile::ST_TYPE_JPEG);
        keysMap[ST_VK_S] = false;
    }

#ifdef __ST_DEBUG__
    if(keysMap[ST_VK_B] && !keysMap[ST_VK_CONTROL] && !keysMap[ST_VK_SHIFT]) {
        myIsBenchmark = !myIsBenchmark;
        myVideo->setBenchmark(myIsBenchmark);
        keysMap[ST_VK_B] = false;
ST_DEBUG_LOG("myIsBenchmark= " + int(myIsBenchmark)); ///
    }
#endif

    keysStereo(keysMap);
    keysSrcFormat(keysMap);
    keysFileWalk(keysMap);
}

ST_EXPORT StDrawerInterface* StDrawer_new() {
    return new StMoviePlayer(); }
ST_EXPORT void StDrawer_del(StDrawerInterface* inst) {
    delete (StMoviePlayer* )inst; }
ST_EXPORT stBool_t StDrawer_init(StDrawerInterface* inst, StWindowInterface* stWin) {
    return ((StMoviePlayer* )inst)->init(stWin); }
ST_EXPORT stBool_t StDrawer_open(StDrawerInterface* inst, const StOpenInfo_t* stOpenInfo) {
    return ((StMoviePlayer* )inst)->open(StOpenInfo(stOpenInfo)); }
ST_EXPORT void StDrawer_parseCallback(StDrawerInterface* inst, StMessage_t* stMessages) {
    ((StMoviePlayer* )inst)->parseCallback(stMessages); }
ST_EXPORT void StDrawer_stglDraw(StDrawerInterface* inst, unsigned int view) {
    ((StMoviePlayer* )inst)->stglDraw(view); }

// SDK version was used
ST_EXPORT void getSDKVersion(StVersion* ver) {
    *ver = StVersionInfo::getSDKVersion();
}

// plugin version
ST_EXPORT void getPluginVersion(StVersion* ver) {
    *ver = StVersionInfo::getSDKVersion();
}

ST_EXPORT const stUtf8_t* getMIMEDescription() {
    return StVideo::ST_VIDEOS_MIME_STRING;
}
