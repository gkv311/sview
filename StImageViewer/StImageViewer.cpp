/**
 * Copyright © 2007-2012 Kirill Gavrilov <kirill@sview.ru>
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

#include <StThreads/StThreads.h> // threads header (mutexes, threads,...)
#include <StGL/StGLContext.h>
#include <StGLCore/StGLCore20.h>
#include <StGLWidgets/StGLTextureButton.h>
#include <StGLWidgets/StGLImageRegion.h>
#include <StGLWidgets/StGLMsgStack.h>
#include <StSocket/StCheckUpdates.h>
#include <StImage/StImageFile.h>

#include <StCore/StCore.h>
#include <StCore/StWindow.h>

#include "StImageViewerStrings.h"

#include <cstdlib> // std::abs(int)

const char* StImageViewer::ST_DRAWER_PLUGIN_NAME = "StImageViewer";

namespace {

    static const char ST_SETTING_SLIDESHOW_DELAY[] = "slideShowDelay";
    static const char ST_SETTING_FPSBOUND[]    = "fpsbound";
    static const char ST_SETTING_SRCFORMAT[]   = "srcFormat";
    static const char ST_SETTING_LAST_FOLDER[] = "lastFolder";
    static const char ST_SETTING_COMPRESS[]    = "toCompress";
    static const char ST_SETTING_ESCAPENOQUIT[]= "escNoQuit";

    static const char ST_SETTING_FULLSCREEN[]  = "fullscreen";
    static const char ST_SETTING_SLIDESHOW[]   = "slideshow";
    static const char ST_SETTING_VIEWMODE[]    = "viewMode";
    static const char ST_SETTING_STEREO_MODE[] = "viewStereoMode";
    static const char ST_SETTING_TEXFILTER[]   = "viewTexFilter";
    static const char ST_SETTING_GAMMA[]       = "viewGamma";
    static const char ST_SETTING_RATIO[]       = "ratio";
    static const char ST_SETTING_UPDATES_LAST_CHECK[] = "updatesLastCheck";
    static const char ST_SETTING_UPDATES_INTERVAL[]   = "updatesInterval";
    static const char ST_SETTING_IMAGELIB[]    = "imageLib";

    static const char ST_ARGUMENT_FILE[]       = "file";
    static const char ST_ARGUMENT_FILE_LEFT[]  = "left";
    static const char ST_ARGUMENT_FILE_RIGHT[] = "right";
};

StImageViewer::StImageViewer()
: myEventDialog(false),
  myEventLoaded(false),
  //
  mySlideShowTimer(false),
  mySlideShowDelay(4.0),
  //
  myLastUpdateDay(0),
  myToCheckUpdates(true),
  myToSaveSrcFormat(false),
  myEscNoQuit(false),
  myToQuit(false) {
    //
    params.isFullscreen = new StBoolParam(false);
    params.isFullscreen->signals.onChanged.connect(this, &StImageViewer::doFullscreen);
    params.toRestoreRatio   = new StBoolParam(false);
    params.checkUpdatesDays = new StInt32Param(7);
    params.srcFormat        = new StInt32Param(ST_V_SRC_AUTODETECT);
    params.srcFormat->signals.onChanged.connect(this, &StImageViewer::doSwitchSrcFormat);
    params.imageLib = StImageFile::ST_LIBAV,
    params.fpsBound = 0;
}

StImageViewer::~StImageViewer() {
    myUpdates.nullify();
    if(!mySettings.isNull() && !myGUI.isNull()) {
        mySettings->saveParam(ST_SETTING_STEREO_MODE, myGUI->stImageRegion->params.displayMode);
        mySettings->saveInt32(ST_SETTING_GAMMA, stRound(100.0f * myGUI->stImageRegion->params.gamma->getValue()));
        if(params.toRestoreRatio->getValue()) {
            mySettings->saveParam(ST_SETTING_RATIO, myGUI->stImageRegion->params.displayRatio);
        } else {
            mySettings->saveInt32(ST_SETTING_RATIO, StGLImageRegion::RATIO_AUTO);
        }
        mySettings->saveParam(ST_SETTING_TEXFILTER, myGUI->stImageRegion->params.textureFilter);
        mySettings->saveInt32(ST_SETTING_FPSBOUND, params.fpsBound);
        mySettings->saveInt32(ST_SETTING_SLIDESHOW_DELAY, int(mySlideShowDelay));
        mySettings->saveInt32(ST_SETTING_UPDATES_LAST_CHECK, myLastUpdateDay);
        mySettings->saveParam(ST_SETTING_UPDATES_INTERVAL, params.checkUpdatesDays);
        mySettings->saveString(ST_SETTING_IMAGELIB, StImageFile::imgLibToString(params.imageLib));
        if(myToSaveSrcFormat) {
            mySettings->saveParam(ST_SETTING_SRCFORMAT, params.srcFormat);
        }
    }

    // release GUI data and GL resorces before closing the window
    myGUI.nullify();
    // wait image loading thread to quit and release resources
    myLoader.nullify();
    // destroy other objects
    mySettings.nullify();
    // now destroy the window
    myWindow.nullify();
    // release libraries
    StCore::FREE();
}

bool StImageViewer::init(StWindowInterface* theWindow) {
    if(!StVersionInfo::checkTimeBomb("sView - Image Viewer plugin")) {
        // timebomb for alpha versions
        return false;
    } else if(theWindow == NULL) {
        stError("ImagePlugin, Invalid window from StRenderer plugin!");
        return false;
    } else if(StCore::INIT() != STERROR_LIBNOERROR) {
        stError("ImagePlugin, Core library not available!");
        return false;
    }

    // create window wrapper
    myWindow = new StWindow(theWindow);
    myWindow->setTitle("sView - Image Viewer");

    // initialize GL context
    myContext = new StGLContext();
    if(!myContext->stglInit()) {
        stError("ImagePlugin, OpenGL context is broken!\n(OpenGL library internal error?)");
        return false;
    } else if(!myContext->isGlGreaterEqual(2, 0)) {
        stError("ImagePlugin, OpenGL2.0+ not available!");
        return false;
    }

    // create the GUI with default values
    myGUI = new StImageViewerGUI(this, myWindow.access());
    myGUI->setContext(myContext);

    // load settings
    mySettings = new StSettings(ST_DRAWER_PLUGIN_NAME);
    mySettings->loadInt32 (ST_SETTING_FPSBOUND, params.fpsBound);
    myWindow->stglSetTargetFps(double(params.fpsBound));
    mySettings->loadString(ST_SETTING_LAST_FOLDER,        params.lastFolder);
    mySettings->loadParam (ST_SETTING_STEREO_MODE,        myGUI->stImageRegion->params.displayMode);
    mySettings->loadParam (ST_SETTING_TEXFILTER,          myGUI->stImageRegion->params.textureFilter);
    mySettings->loadParam (ST_SETTING_RATIO,              myGUI->stImageRegion->params.displayRatio);
    mySettings->loadInt32 (ST_SETTING_UPDATES_LAST_CHECK, myLastUpdateDay);
    mySettings->loadParam (ST_SETTING_UPDATES_INTERVAL,   params.checkUpdatesDays);
    params.toRestoreRatio->setValue(myGUI->stImageRegion->params.displayRatio->getValue() != StGLImageRegion::RATIO_AUTO);
    int32_t loadedGamma = 100; // 1.0f
        mySettings->loadInt32(ST_SETTING_GAMMA, loadedGamma);
        myGUI->stImageRegion->params.gamma->setValue(0.01f * loadedGamma);
    int32_t aSlideShowDelayInt = int32_t(mySlideShowDelay);
    mySettings->loadInt32 (ST_SETTING_SLIDESHOW_DELAY,    aSlideShowDelayInt);
    mySlideShowDelay = double(aSlideShowDelayInt);

    // initialize frame region early to show dedicated error description
    if(!myGUI->stImageRegion->stglInit()) {
        stError("ImagePlugin, frame region initialization failed!");
        return false;
    }
    myGUI->stglInit();
    if(!mySettings->isValid()) {
        myGUI->myMsgStack->doPushMessage("Settings plugin is not available!\nAll changes will be lost after restart.");
    }

    // create the image loader thread
    StString imageLibString;
    mySettings->loadString(ST_SETTING_IMAGELIB, imageLibString);
    params.imageLib = StImageFile::imgLibFromString(imageLibString);
    myLoader = new StImageLoader(params.imageLib, myGUI->myLangMap, myGUI->stImageRegion->getTextureQueue());
    myLoader->signals.onError.connect(myGUI->myMsgStack, &StGLMsgStack::doPushMessage);
    myLoader->signals.onLoaded.connect(this, &StImageViewer::doLoaded);

    // load this parameter AFTER image thread creation
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

void StImageViewer::parseArguments(const StArgumentsMap& theArguments) {
    StArgument argFullscreen = theArguments[ST_SETTING_FULLSCREEN];
    StArgument argSlideshow  = theArguments[ST_SETTING_SLIDESHOW];
    StArgument argViewMode   = theArguments[ST_SETTING_VIEWMODE];
    StArgument argSrcFormat  = theArguments[ST_SETTING_SRCFORMAT];
    StArgument argImgLibrary = theArguments[ST_SETTING_IMAGELIB];
    StArgument argToCompress = theArguments[ST_SETTING_COMPRESS];
    StArgument argEscNoQuit  = theArguments[ST_SETTING_ESCAPENOQUIT];
    if(argToCompress.isValid()) {
        myLoader->setCompressMemory(!argToCompress.isValueOff());
    }
    if(argEscNoQuit.isValid()) {
        myEscNoQuit = !argEscNoQuit.isValueOff();
    }
    if(argFullscreen.isValid()) {
        params.isFullscreen->setValue(!argFullscreen.isValueOff());
    }
    if(argSlideshow.isValid() && !argSlideshow.isValueOff()) {
        doSlideShow();
    }
    if(argViewMode.isValid()) {
        myLoader->getPlayList().changeDefParams().setViewMode(StStereoParams::GET_VIEW_MODE_FROM_STRING(argViewMode.getValue()));
    }
    if(argSrcFormat.isValid()) {
        params.srcFormat->setValue(st::formatFromString(argSrcFormat.getValue()));
        myToSaveSrcFormat = false; // this setting is temporary!
    }
    if(argImgLibrary.isValid()) {
        params.imageLib = StImageFile::imgLibFromString(argImgLibrary.getValue());
        myLoader->setImageLib(params.imageLib);
    }
}

bool StImageViewer::open(const StOpenInfo& theOpenInfo) {
    parseArguments(theOpenInfo.getArgumentsMap());
    StMIME anOpenMIME = theOpenInfo.getMIME();
    if(anOpenMIME == StDrawerInfo::DRAWER_MIME() || theOpenInfo.getPath().isEmpty()) {
        // open drawer without files
        return true;
    }

    // clear playlist first
    myLoader->getPlayList().clear();

    //StArgument argFile1     = theOpenInfo.getArgumentsMap()[ST_ARGUMENT_FILE + 1]; // playlist?
    StArgument argFileLeft  = theOpenInfo.getArgumentsMap()[ST_ARGUMENT_FILE_LEFT];
    StArgument argFileRight = theOpenInfo.getArgumentsMap()[ST_ARGUMENT_FILE_RIGHT];
    if(argFileLeft.isValid() && argFileRight.isValid()) {
        // meta-file
        /// TODO (Kirill Gavrilov#4) we should use MIME type!
        myLoader->getPlayList().addOneFile(argFileLeft.getValue(), argFileRight.getValue());
    } else if(!anOpenMIME.isEmpty()) {
        // create just one-file playlist
        myLoader->getPlayList().addOneFile(theOpenInfo.getPath(), anOpenMIME);
    } else {
        // create playlist from file's folder
        myLoader->getPlayList().open(theOpenInfo.getPath());
    }

    if(!myLoader->getPlayList().isEmpty()) {
        doUpdateStateLoading();
        myLoader->doLoadNext();
    }
    return true;
}

void StImageViewer::parseCallback(StMessage_t* stMessages) {
    bool isMouseMove = false;
    if(myToQuit) {
        stMessages[0].uin = StMessageList::MSG_EXIT;
        stMessages[1].uin = StMessageList::MSG_NULL;
    }
    size_t evId(0);
    for(; stMessages[evId].uin != StMessageList::MSG_NULL; ++evId) {
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
                    stUtf8_t buffFile[4096];
                    stMemSet(buffFile, 0, sizeof(buffFile));
                    if(myWindow->getDragNDropFile(0, buffFile, (4096 * sizeof(stUtf8_t))) == 0) {
                        StString buffString(buffFile);
                        if(myLoader->getPlayList().checkExtension(buffString)) {
                            myLoader->getPlayList().open(buffString);
                            doUpdateStateLoading();
                            myLoader->doLoadNext();
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
                    if(!myEscNoQuit) {
                        stMessages[0].uin = StMessageList::MSG_EXIT;
                        stMessages[1].uin = StMessageList::MSG_NULL;
                        return;
                    }
                    if(myWindow->isFullScreen()) {
                        params.isFullscreen->setValue(false);
                    }
                }
                keysCommon(keysMap); break;
            }
            case StMessageList::MSG_MOUSE_MOVE: {
                isMouseMove = true; break;
            }
            case StMessageList::MSG_MOUSE_DOWN: {
                StPointD_t pt;
                int aMouseBtn = myWindow->getMouseDown(&pt);
                if(myEscNoQuit
                && !myWindow->isFullScreen()
                && (aMouseBtn == ST_MOUSE_SCROLL_V_UP || aMouseBtn == ST_MOUSE_SCROLL_V_DOWN)) {
                    // ignore scrolling as well
                    break;
                }
                myGUI->tryClick(pt, aMouseBtn);
                break;
            }
            case StMessageList::MSG_MOUSE_UP: {
                StPointD_t pt;
                int aMouseBtn = myWindow->getMouseUp(&pt);
                if(aMouseBtn == ST_MOUSE_MIDDLE) {
                    params.isFullscreen->reverse();
                } else if(myEscNoQuit
                       && !myWindow->isFullScreen()
                       && (aMouseBtn == ST_MOUSE_SCROLL_V_UP || aMouseBtn == ST_MOUSE_SCROLL_V_DOWN)) {
                    // ignore scrolling as well
                    break;
                }
                myGUI->tryUnClick(pt, aMouseBtn);
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

    myGUI->setVisibility(myWindow->getMousePos(), isMouseMove);
    bool toHideCursor = params.isFullscreen->getValue() && myGUI->toHideCursor();
    myWindow->showCursor(!toHideCursor);
}

void StImageViewer::stglDraw(unsigned int theView) {
    myGUI->getCamera()->setView(theView);
    if(theView == ST_DRAW_LEFT) {
        if(!myWindow->isActive()) {
            // enforce deep sleeps
            StThread::sleep(200);
        }

        myGUI->stglUpdate(myWindow->getMousePos());

        // check for mono state
        StHandle<StStereoParams> aParams = myGUI->stImageRegion->getSource();
        if(!aParams.isNull()) {
            myWindow->setStereoOutput(!aParams->isMono() && myWindow->isActive()
                                   && (myGUI->stImageRegion->params.displayMode->getValue() == StGLImageRegion::MODE_STEREO));
        }
    }

    // clear the screen and the depth buffer
    myContext->core20fwd->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // draw GUI
    myGUI->stglDraw(theView);
}

void StImageViewer::doSwitchSrcFormat(const int32_t theSrcFormat) {
    myLoader->setSrcFormat(StFormatEnum(theSrcFormat));
    myLoader->doLoadNext();
    myToSaveSrcFormat = true;
}

void StImageViewer::doOpen2FilesDialog(const size_t ) {
    doOpenFileDialog(this, 2);
}

void StImageViewer::doOpenFileDialog(const size_t filesCount) {
    if(myEventDialog.check()) {
        return;
    }
    myEventDialog.set();

    if(params.lastFolder.isEmpty()) {
        // TODO
        StHandle<StFileNode> aCurrFile = myLoader->getPlayList().getCurrentFile();
        if(!aCurrFile.isNull()) {
            params.lastFolder = aCurrFile->isEmpty() ? aCurrFile->getFolderPath() : aCurrFile->getValue(0)->getFolderPath();
        }
    }
    StString title;
    if(filesCount == 2) {
        title = myGUI->myLangMap->changeValueId(StImageViewerStrings::DIALOG_OPEN_LEFT,
                                                "Choose LEFT image file to open");
    } else {
        title = myGUI->myLangMap->changeValueId(StImageViewerStrings::DIALOG_OPEN_FILE,
                                                "Choose the image file to open");
    }

    StString aFilePath, aDummy;
    if(StFileNode::openFileDialog(params.lastFolder, title, myLoader->getMimeList(), aFilePath, false)) {
        if(filesCount == 2) {
            title = myGUI->myLangMap->changeValueId(StImageViewerStrings::DIALOG_OPEN_RIGHT,
                                                    "Choose RIGHT image file to open");
            StFileNode::getFolderAndFile(aFilePath, params.lastFolder, aDummy);
            StString filePathRToOpen;
            if(StFileNode::openFileDialog(params.lastFolder, title, myLoader->getMimeList(), filePathRToOpen, false)) {
                // meta-file
                myLoader->getPlayList().clear();
                myLoader->getPlayList().addOneFile(aFilePath, filePathRToOpen);
            }
        } else {
            myLoader->getPlayList().open(aFilePath);
        }

        doUpdateStateLoading();
        myLoader->doLoadNext();

        if(!aFilePath.isEmpty()) {
            StString aDummy;
            StFileNode::getFolderAndFile(aFilePath, params.lastFolder, aDummy);
        }
        if(!params.lastFolder.isEmpty()) {
            mySettings->saveString(ST_SETTING_LAST_FOLDER, params.lastFolder);
        }
    }
    myEventDialog.reset();
}

void StImageViewer::doUpdateStateLoading() {
    const StString aFileToLoad = myLoader->getPlayList().getCurrentTitle();
    if(aFileToLoad.isEmpty()) {
        myWindow->setTitle("sView - Image Viewer");
    } else {
        /// TODO (Kirill Gavrilov#4) - show Loading... after delay
        myWindow->setTitle(aFileToLoad + " Loading... - sView");
    }
}

void StImageViewer::doUpdateStateLoaded() {
    const StString aFileToLoad = myLoader->getPlayList().getCurrentTitle();
    if(aFileToLoad.isEmpty()) {
        myWindow->setTitle("sView - Image Viewer");
    } else {
        myWindow->setTitle(aFileToLoad + " - sView");
    }
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
    myToQuit = true;
}

void StImageViewer::doFullscreen(const bool theIsFullscreen) {
    if(!myWindow.isNull()) {
        myWindow->setFullScreen(theIsFullscreen);
    }
}

void StImageViewer::doReset(const size_t ) {
    StHandle<StStereoParams> aParams = myGUI->stImageRegion->getSource();
    if(!aParams.isNull()) {
        aParams->reset();
    }
}

void StImageViewer::doLoaded() {
    myEventLoaded.set();
}

void StImageViewer::keysStereo(bool* keysMap) {
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

    // ========= Positioning factor =======
    if(keysMap[ST_VK_LEFT]) {
        aParams->moveToRight();
    }
    if(keysMap[ST_VK_RIGHT]) {
        aParams->moveToLeft();
    }
    if(keysMap[ST_VK_UP]) {
        aParams->moveToDown();
    }
    if(keysMap[ST_VK_DOWN]) {
        aParams->moveToUp();
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
    // reset stereo attributes
    if(keysMap[ST_VK_BACK]) {
        doReset();
    }

    if(keysMap[ST_VK_P]) {
        aParams->nextViewMode();
        keysMap[ST_VK_P] = false;
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

    /// TODO (Kirill Gavrilov#9) remove this hot key
    if(keysMap[ST_VK_T] && keysMap[ST_VK_CONTROL]) {
        myGUI->stImageRegion->params.saturation->decrement();
        keysMap[ST_VK_T] = false;
    }
    if(keysMap[ST_VK_T] && keysMap[ST_VK_SHIFT]) {
        myGUI->stImageRegion->params.saturation->increment();
        keysMap[ST_VK_T] = false;
    }

}

void StImageViewer::keysSrcFormat(bool* keysMap) {
    // A (auto)/M (mono)/S (side by side)/O (over under)/I (horizontal interlace)
    if(keysMap[ST_VK_A]) {
        params.srcFormat->setValue(ST_V_SRC_AUTODETECT);
        keysMap[ST_VK_A] = false;
    }
    if(keysMap[ST_VK_M]) {
        params.srcFormat->setValue(ST_V_SRC_MONO);
        keysMap[ST_VK_M] = false;
    }
    if(keysMap[ST_VK_S] && !keysMap[ST_VK_CONTROL] && !keysMap[ST_VK_SHIFT]) {
        params.srcFormat->setValue(ST_V_SRC_SIDE_BY_SIDE);
        keysMap[ST_VK_S] = false;
    }
    if(keysMap[ST_VK_O] && !keysMap[ST_VK_CONTROL]) {
        params.srcFormat->setValue(ST_V_SRC_OVER_UNDER_RL);
        keysMap[ST_VK_O] = false;
    }
}

void StImageViewer::keysFileWalk(bool* keysMap) {
    if(keysMap[ST_VK_I]) {
        myGUI->doAboutImage(0);
        keysMap[ST_VK_I] = false;
    }

    if(keysMap[ST_VK_O] && keysMap[ST_VK_CONTROL]) {
        doOpenFileDialog(this, 1);
        keysMap[ST_VK_O] = false;
    }

    if(keysMap[ST_VK_SPACE]) {
        doSlideShow();
        keysMap[ST_VK_SPACE] = false;
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

    if(keysMap[ST_VK_SHIFT] && keysMap[ST_VK_DELETE]) {
        StHandle<StFileNode> aCurrFile = myLoader->getPlayList().getCurrentFile();
        if(!aCurrFile.isNull() && aCurrFile->size() == 0) {
            if(stQuestion(StString("Are you sure you want to completely remove the file\n'") + aCurrFile->getPath() + "'?")) {
                myLoader->getPlayList().removePhysically(aCurrFile);
                if(!myLoader->getPlayList().isEmpty()) {
                    doUpdateStateLoading();
                    myLoader->doLoadNext();
                }
            }
        }
        keysMap[ST_VK_DELETE] = false;
    }
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

void StImageViewer::keysCommon(bool* keysMap) {
    if(keysMap[ST_VK_F]) {
        params.isFullscreen->reverse();
        keysMap[ST_VK_F] = false;
    }
    if(keysMap[ST_VK_RETURN]) {
        params.isFullscreen->reverse();
        keysMap[ST_VK_RETURN] = false;
    }

    if(keysMap[ST_VK_S] && keysMap[ST_VK_CONTROL]) {
        myLoader->doSaveImageAs(StImageFile::ST_TYPE_PNG);
        keysMap[ST_VK_S] = false;
    }

    keysStereo(keysMap);
    keysSrcFormat(keysMap);
    keysFileWalk(keysMap);
}

ST_EXPORT StDrawerInterface* StDrawer_new() {
    return new StImageViewer(); }
ST_EXPORT void StDrawer_del(StDrawerInterface* inst) {
    delete (StImageViewer* )inst; }
ST_EXPORT stBool_t StDrawer_init(StDrawerInterface* inst, StWindowInterface* stWin) {
    return ((StImageViewer* )inst)->init(stWin); }
ST_EXPORT stBool_t StDrawer_open(StDrawerInterface* inst, const StOpenInfo_t* stOpenInfo) {
    return ((StImageViewer* )inst)->open(StOpenInfo(stOpenInfo)); }
ST_EXPORT void StDrawer_parseCallback(StDrawerInterface* inst, StMessage_t* stMessages) {
    ((StImageViewer* )inst)->parseCallback(stMessages); }
ST_EXPORT void StDrawer_stglDraw(StDrawerInterface* inst, unsigned int view) {
    ((StImageViewer* )inst)->stglDraw(view); }

// SDK version was used
ST_EXPORT void getSDKVersion(StVersion* ver) {
    *ver = StVersionInfo::getSDKVersion();
}

// plugin version
ST_EXPORT void getPluginVersion(StVersion* ver) {
    *ver = StVersionInfo::getSDKVersion();
}

ST_EXPORT const stUtf8_t* getMIMEDescription() {
    return StImageLoader::ST_IMAGES_MIME_STRING;
}
