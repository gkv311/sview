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

#include "StMoviePlayerGUI.h"
#include "StMoviePlayerStrings.h"
#include "StVideo/StVideo.h"
#include "StTimeBox.h"

#include <StSocket/StCheckUpdates.h>
#include <StImage/StImageFile.h>

#include <StGL/StGLContext.h>
#include <StGLCore/StGLCore20.h>
#include <StGLWidgets/StGLImageRegion.h>
#include <StGLWidgets/StGLMsgStack.h>
#include <StGLWidgets/StGLSubtitles.h>
#include <StGLWidgets/StGLTextureButton.h>

#include "../StOutAnaglyph/StOutAnaglyph.h"
#include "../StOutDual/StOutDual.h"
#include "../StOutIZ3D/StOutIZ3D.h"
#include "../StOutInterlace/StOutInterlace.h"
#include "../StOutPageFlip/StOutPageFlipExt.h"
#include "../StOutDistorted/StOutDistorted.h"

#include <cstdlib> // std::abs(int)

const StString StMoviePlayer::ST_DRAWER_PLUGIN_NAME = "StMoviePlayer";

namespace {

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
  myEventDialog(false),
  myEventLoaded(false),
  mySeekOnLoad(-1.0),
  //
  myLastUpdateDay(0),
  myToUpdateALList(false),
  myIsBenchmark(false),
  myToCheckUpdates(true) {
    //
    myTitle = "sView - Movie Player";
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
    params.ToShowFps   = new StBoolParam(false);
    params.audioStream = new StInt32Param(-1);
    params.audioStream->signals.onChanged.connect(this, &StMoviePlayer::doSwitchAudioStream);
    params.subtitlesStream = new StInt32Param(-1);
    params.subtitlesStream->signals.onChanged.connect(this, &StMoviePlayer::doSwitchSubtitlesStream);
    params.blockSleeping = new StInt32Param(StMoviePlayer::BLOCK_SLEEP_PLAYBACK);
    params.fpsBound = 1;

    // load settings
    mySettings->loadInt32 (ST_SETTING_FPSBOUND,           params.fpsBound);
    mySettings->loadString(ST_SETTING_LAST_FOLDER,        params.lastFolder);
    mySettings->loadInt32 (ST_SETTING_UPDATES_LAST_CHECK, myLastUpdateDay);
    mySettings->loadParam (ST_SETTING_UPDATES_INTERVAL,   params.checkUpdatesDays);
    mySettings->loadParam (ST_SETTING_SHUFFLE,            params.isShuffle);
    mySettings->loadParam (ST_SETTING_GLOBAL_MKEYS,       params.areGlobalMKeys);

    StString aSavedALDevice;
    mySettings->loadString(ST_SETTING_OPENAL_DEVICE,      aSavedALDevice);
    params.alDevice->init(aSavedALDevice);

    params.isShuffle->signals.onChanged.connect(this, &StMoviePlayer::doSwitchShuffle);
    params.alDevice ->signals.onChanged.connect(this, &StMoviePlayer::doSwitchAudioDevice);

    addRenderer(new StOutAnaglyph(theParentWin));
    addRenderer(new StOutDual(theParentWin));
    addRenderer(new StOutIZ3D(theParentWin));
    addRenderer(new StOutInterlace(theParentWin));
    addRenderer(new StOutPageFlipExt(theParentWin));
    addRenderer(new StOutDistorted(theParentWin));

    // no need in Depth buffer
    const StWinAttr anAttribs[] = {
        StWinAttr_GlDepthSize, (StWinAttr )0,
        StWinAttr_NULL
    };
    for(size_t aRendIter = 0; aRendIter < myRenderers.size(); ++aRendIter) {
        StHandle<StWindow>& aRend = myRenderers[aRendIter];
        aRend->setAttributes(anAttribs);
    }
}

bool StMoviePlayer::resetDevice() {
    if(myGUI.isNull()
    || myVideo.isNull()) {
        return init();
    }

    // be sure Render plugin process quit correctly
    myWindow->beforeClose();

    myVideo->doRelease();
    releaseDevice();
    myWindow->close();
    myWindow.nullify();
    return open();
}

void StMoviePlayer::releaseDevice() {
    if(!myGUI.isNull()) {
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

    // release GUI data and GL resources before closing the window
    myGUI.nullify();
    myContext.nullify();
}

StMoviePlayer::~StMoviePlayer() {
    myUpdates.nullify();
    releaseDevice();
    // wait video playback thread to quit and release resources
    myVideo.nullify();
}

bool StMoviePlayer::init() {
    const bool isReset = !myVideo.isNull();
    if(!myContext.isNull()
    && !myGUI.isNull()) {
        return true;
    }

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
    StHandle<StGLTextureQueue> aTextureQueue;
    StHandle<StSubQueue>       aSubQueue;
    if(!myVideo.isNull()) {
        aTextureQueue = myVideo->getTextureQueue();
        aSubQueue     = myVideo->getSubtitlesQueue();
    } else {
        aTextureQueue = new StGLTextureQueue(16);
        aSubQueue     = new StSubQueue();
    }
    myGUI = new StMoviePlayerGUI(this, myWindow.access(), myLangMap.access(), aTextureQueue, aSubQueue);
    myGUI->setContext(myContext);

    // load settings
    mySettings->loadParam (ST_SETTING_STEREO_MODE, myGUI->stImageRegion->params.displayMode);
    mySettings->loadParam (ST_SETTING_TEXFILTER,   myGUI->stImageRegion->params.textureFilter);
    mySettings->loadParam (ST_SETTING_RATIO,       myGUI->stImageRegion->params.displayRatio);
    params.toRestoreRatio->setValue(myGUI->stImageRegion->params.displayRatio->getValue() != StGLImageRegion::RATIO_AUTO);
    int32_t loadedGamma = 100; // 1.0f
        mySettings->loadInt32(ST_SETTING_GAMMA, loadedGamma);
        myGUI->stImageRegion->params.gamma->setValue(0.01f * loadedGamma);

    // capture multimedia keys even without window focus
    myWindow->setAttribute(StWinAttr_GlobalMediaKeys, params.areGlobalMKeys->getValue());

    // initialize frame region early to show dedicated error description
    if(!myGUI->stImageRegion->stglInit()) {
        stError("VideoPlugin, frame region initialization failed!");
        return false;
    }
    myGUI->stglInit();
    myGUI->stglResize(myWindow->getPlacement());

    // create the video playback thread
    if(!isReset) {
        myVideo = new StVideo(params.alDevice->getTitle(), myLangMap, aTextureQueue, aSubQueue);
    }
    myVideo->signals.onError.connect(myGUI->myMsgStack, &StGLMsgStack::doPushMessage);
    myVideo->signals.onLoaded.connect(this, &StMoviePlayer::doLoaded);
    myVideo->getPlayList().setShuffle(params.isShuffle->getValue());

    StString aRecentList;
    mySettings->loadString(ST_SETTING_RECENT_FILES, aRecentList);
    myVideo->getPlayList().loadRecentList(aRecentList);

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

bool StMoviePlayer::open() {
    const bool isReset = !mySwitchTo.isNull();
    if(!StApplication::open()
    || !init()) {
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
        return true;
    }

    // clear playlist first
    myVideo->getPlayList().clear();

    //StArgument argFile1     = myOpenFileInfo->getArgumentsMap()[ST_ARGUMENT_FILE + 1]; // playlist?
    StArgument argFileLeft  = myOpenFileInfo->getArgumentsMap()[ST_ARGUMENT_FILE_LEFT];
    StArgument argFileRight = myOpenFileInfo->getArgumentsMap()[ST_ARGUMENT_FILE_RIGHT];
    if(argFileLeft.isValid() && argFileRight.isValid()) {
        // meta-file
        /// TODO (Kirill Gavrilov#4) we should use MIME type!
        myVideo->getPlayList().addOneFile(argFileLeft.getValue(), argFileRight.getValue());
    } else if(!anOpenMIME.isEmpty()) {
        // create just one-file playlist
        myVideo->getPlayList().addOneFile(myOpenFileInfo->getPath(), anOpenMIME);
    } else {
        // create playlist from file's folder
        myVideo->getPlayList().open(myOpenFileInfo->getPath());
    }

    if(!myVideo->getPlayList().isEmpty()) {
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

    myGUI->stglResize(myWindow->getPlacement());
}

void StMoviePlayer::doKeyDown(const StKeyEvent& theEvent) {
    if(myGUI.isNull()) {
        return;
    }

    switch(theEvent.VKey) {
        case ST_VK_ESCAPE: {
            StApplication::exit(0);
            return;
        }
        case ST_VK_F:
        case ST_VK_RETURN:
            params.isFullscreen->reverse();
            return;
        case ST_VK_F12:
            params.ToShowFps->reverse();
            return;
        case ST_VK_W:
            myGUI->stImageRegion->params.swapLR->reverse();
            return;

        // file walk
        case ST_VK_I:
            myGUI->doAboutFile(0);
            return;
        case ST_VK_HOME:
            doListFirst();
            return;
        case ST_VK_PRIOR:
        case ST_VK_MEDIA_PREV_TRACK:
        case ST_VK_BROWSER_BACK:
            doListPrev();
            return;
        case ST_VK_END:
            doListLast();
            return;
        case ST_VK_NEXT:
        case ST_VK_MEDIA_NEXT_TRACK:
        case ST_VK_BROWSER_FORWARD:
            doListNext();
            return;
        case ST_VK_O: {
            if(theEvent.Flags == ST_VF_CONTROL) {
                doOpen1File();
            } else if(theEvent.Flags == ST_VF_NONE) {
                params.srcFormat->setValue(ST_V_SRC_OVER_UNDER_RL);
            }
            return;
        }

        // source format
        case ST_VK_A:
            params.srcFormat->setValue(ST_V_SRC_AUTODETECT);
            return;
        case ST_VK_M:
            params.srcFormat->setValue(ST_V_SRC_MONO);
            return;
        case ST_VK_R:
            params.srcFormat->setValue(ST_V_SRC_ANAGLYPH_RED_CYAN);
            return;

        // playback control
        case ST_VK_SPACE:
        case ST_VK_MEDIA_PLAY_PAUSE:
            doPlayPause();
            return;
        case ST_VK_MEDIA_STOP:
            doStop();
            return;
        case ST_VK_LEFT:
            doSeekLeft();
            return;
        case ST_VK_RIGHT:
            doSeekRight();
            return;

        case ST_VK_H:
        case ST_VK_L: {
            const int32_t aValue = myVideo->params.activeAudio->nextValue(theEvent.Flags == ST_VF_SHIFT ? -1 : 1);
            params.audioStream->setValue(aValue);
            return;
        }
        case ST_VK_U:
        case ST_VK_T: {
            const int32_t aValue = myVideo->params.activeSubtitles->nextValue(theEvent.Flags == ST_VF_SHIFT ? -1 : 1);
            params.subtitlesStream->setValue(aValue);
            return;
        }

        case ST_VK_S: {
            if(theEvent.Flags == ST_VF_CONTROL) {
                doSnapshot(StImageFile::ST_TYPE_JPEG);
            } else if(theEvent.Flags == ST_VF_NONE) {
                params.srcFormat->setValue(ST_V_SRC_SIDE_BY_SIDE);
            }
            return;
        }

        // reset stereo attributes
        case ST_VK_BACK:
            doReset();
            return;

        // post process keys
        case ST_VK_G: {
            if(theEvent.Flags == ST_VF_SHIFT) {
                myGUI->stImageRegion->params.gamma->increment();
            } else if(theEvent.Flags == ST_VF_CONTROL) {
                myGUI->stImageRegion->params.gamma->decrement();
            }
            return;
        }
        case ST_VK_B: {
            if(theEvent.Flags == ST_VF_SHIFT) {
                myGUI->stImageRegion->params.brightness->increment();
            } else if(theEvent.Flags == ST_VF_CONTROL) {
                myGUI->stImageRegion->params.brightness->decrement();
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

    StHandle<StStereoParams> aParams = myGUI->stImageRegion->getSource();
    if(aParams.isNull()) {
        return;
    }

    switch(theEvent.VKey) {
        case ST_VK_COMMA:
        case ST_VK_DIVIDE: {
            if(theEvent.Flags == ST_VF_CONTROL) {
                aParams->decSeparationDy();
            } else {
                aParams->decSeparationDx();
            }
            return;
        }
        case ST_VK_PERIOD:
        case ST_VK_MULTIPLY: {
            if(theEvent.Flags == ST_VF_CONTROL) {
                aParams->incSeparationDy();
            } else {
                aParams->incSeparationDx();
            }
            return;
        }
        case ST_VK_P:
            aParams->nextViewMode();
            return;
        case ST_VK_BRACKETLEFT: {
            if(theEvent.Flags == ST_VF_NONE) {
                aParams->decZRotate();
            }
            return;
        }
        case ST_VK_BRACKETRIGHT: {
            if(theEvent.Flags == ST_VF_NONE) {
                aParams->incZRotate();
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

    StHandle<StStereoParams> aParams = myGUI->stImageRegion->getSource();
    if(aParams.isNull()) {
        return;
    }

    const GLfloat aDuration = (GLfloat )theEvent.Progress;
    switch(theEvent.VKey) {
        // zooming
        case ST_VK_ADD:
        case ST_VK_OEM_PLUS:
            aParams->scaleIn(aDuration);
            break;
        case ST_VK_SUBTRACT:
        case ST_VK_OEM_MINUS:
            aParams->scaleOut(aDuration);
            break;
        // rotation
        case ST_VK_BRACKETLEFT: {
            if(theEvent.Flags == ST_VF_CONTROL) {
                aParams->decZRotateL(aDuration);
            }
            break;
        }
        case ST_VK_BRACKETRIGHT: {
            if(theEvent.Flags == ST_VF_CONTROL) {
                aParams->incZRotateL(aDuration);
            }
            break;
        }
        case ST_VK_SEMICOLON: {
            if(theEvent.Flags == ST_VF_CONTROL) {
                aParams->incSepRotation(aDuration);
            }
            break;
        }
        case ST_VK_APOSTROPHE: {
            if(theEvent.Flags == ST_VF_CONTROL) {
                aParams->decSepRotation(aDuration);
            }
            break;
        }
        default: break;
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
            myGUI->tryUnClick(StPointD_t(theEvent.PointX, theEvent.PointY), theEvent.Button);
            break;
        }
    }
}

void StMoviePlayer::doFileDrop(const StDNDropEvent& theEvent) {
    const StString aFilePath = theEvent.File;
    if(myVideo->getPlayList().checkExtension(aFilePath)) {
        myVideo->getPlayList().open(aFilePath);
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
    const bool isMouseMove = myWindow->isMouseMoved();
    if(myEventLoaded.checkReset()) {
        doUpdateStateLoaded();
    }

    if(myIsBenchmark) {
        // full unbounded
        myWindow->setTargetFps(-1.0);
    } else if(params.fpsBound == 1) {
        // set rendering FPS to 2x averageFPS
        double targetFps = myVideo->getAverFps();
        if(targetFps < 18.0) {
            targetFps = 0.0;
        } else if(targetFps < 40.0) {
            targetFps *= 2.0;
        }
        myWindow->setTargetFps(targetFps);
    } else {
        // set rendering FPS to setted value in settings
        myWindow->setTargetFps(double(params.fpsBound));
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
    if(!myContext.isNull()) {
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

        const StWinAttr anAttribs[] = {
            StWinAttr_ToBlockSleepSystem,  (StWinAttr )toBlockSleepSystem,
            StWinAttr_ToBlockSleepDisplay, (StWinAttr )toBlockSleepDisplay,
            StWinAttr_NULL
        };
        myWindow->setAttributes(anAttribs);

        myWindow->showCursor(!myGUI->toHideCursor());

        // check for mono state
        StHandle<StStereoParams> aParams = myGUI->stImageRegion->getSource();
        if(!aParams.isNull()) {
            myWindow->setStereoOutput(!aParams->isMono()
                                && (myGUI->stImageRegion->params.displayMode->getValue() == StGLImageRegion::MODE_STEREO));
        }
    }

    // draw GUI
    myGUI->stglDraw(view);
}

void StMoviePlayer::doSwitchShuffle(const bool theShuffleOn) {
    if(!myVideo.isNull()) {
        myVideo->getPlayList().setShuffle(theShuffleOn);
    }
}

void StMoviePlayer::doSwitchAudioDevice(const int32_t /*theDevId*/) {
    if(!myVideo.isNull()) {
        myVideo->switchAudioDevice(params.alDevice->getTitle());
    }
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
