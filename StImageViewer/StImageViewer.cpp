/**
 * Copyright © 2007-2013 Kirill Gavrilov <kirill@sview.ru>
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
#include <StGLWidgets/StGLTextureButton.h>
#include <StGLWidgets/StGLImageRegion.h>
#include <StGLWidgets/StGLMsgStack.h>
#include <StSocket/StCheckUpdates.h>
#include <StThreads/StThreads.h>
#include <StImage/StImageFile.h>

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

void StImageViewer::doChangeDevice(const int32_t theValue) {
    StApplication::doChangeDevice(theValue);
    // update menu
}

StImageViewer::StImageViewer(const StNativeWin_t         theParentWin,
                             const StHandle<StOpenInfo>& theOpenInfo)
: StApplication(theParentWin, theOpenInfo),
  mySettings(new StSettings(ST_DRAWER_PLUGIN_NAME)),
  myLangMap(new StTranslations(StImageViewer::ST_DRAWER_PLUGIN_NAME)),
  myEventDialog(false),
  myEventLoaded(false),
  //
  mySlideShowTimer(false),
  mySlideShowDelay(4.0),
  //
  myLastUpdateDay(0),
  myToCheckUpdates(true),
  myToSaveSrcFormat(false),
  myEscNoQuit(false) {
    //
    myTitle = "sView - Image Viewer";
    //
    params.isFullscreen = new StBoolParam(false);
    params.isFullscreen->signals.onChanged.connect(this, &StImageViewer::doFullscreen);
    params.toRestoreRatio   = new StBoolParam(false);
    params.checkUpdatesDays = new StInt32Param(7);
    params.srcFormat        = new StInt32Param(ST_V_SRC_AUTODETECT);
    params.srcFormat->signals.onChanged.connect(this, &StImageViewer::doSwitchSrcFormat);
    params.ToShowFps = new StBoolParam(false);
    params.imageLib = StImageFile::ST_LIBAV,
    params.fpsBound = 0;

    mySettings->loadInt32 (ST_SETTING_FPSBOUND,           params.fpsBound);
    mySettings->loadString(ST_SETTING_LAST_FOLDER,        params.lastFolder);
    mySettings->loadInt32 (ST_SETTING_UPDATES_LAST_CHECK, myLastUpdateDay);
    mySettings->loadParam (ST_SETTING_UPDATES_INTERVAL,   params.checkUpdatesDays);

    int32_t aSlideShowDelayInt = int32_t(mySlideShowDelay);
    mySettings->loadInt32 (ST_SETTING_SLIDESHOW_DELAY,    aSlideShowDelayInt);
    mySlideShowDelay = double(aSlideShowDelayInt);

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

bool StImageViewer::resetDevice() {
    if(myGUI.isNull()
    || myLoader.isNull()) {
        return init();
    }

    // be sure Render plugin process quit correctly
    myMessages[0].uin = StMessageList::MSG_EXIT;
    myMessages[1].uin = StMessageList::MSG_NULL;
    myWindow->processEvents(myMessages);

    myLoader->doRelease();
    releaseDevice();
    myWindow->close();
    myWindow.nullify();
    return open();
}

void StImageViewer::releaseDevice() {
    if(!myGUI.isNull()) {
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

    // release GUI data and GL resources before closing the window
    myGUI.nullify();
    myContext.nullify();
}

StImageViewer::~StImageViewer() {
    myUpdates.nullify();
    releaseDevice();
    // wait image loading thread to quit and release resources
    myLoader.nullify();
}

bool StImageViewer::init() {
    const bool isReset = !myLoader.isNull();
    if(!myContext.isNull()
    && !myGUI.isNull()) {
        return true;
    }

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
    myGUI = new StImageViewerGUI(this, myWindow.access(), myLangMap.access(),
                                 myLoader.isNull() ? NULL : myLoader->getTextureQueue());
    myGUI->setContext(myContext);

    // load settings
    myWindow->setTargetFps(double(params.fpsBound));
    mySettings->loadParam (ST_SETTING_STEREO_MODE,        myGUI->stImageRegion->params.displayMode);
    mySettings->loadParam (ST_SETTING_TEXFILTER,          myGUI->stImageRegion->params.textureFilter);
    mySettings->loadParam (ST_SETTING_RATIO,              myGUI->stImageRegion->params.displayRatio);
    params.toRestoreRatio->setValue(myGUI->stImageRegion->params.displayRatio->getValue() != StGLImageRegion::RATIO_AUTO);
    int32_t loadedGamma = 100; // 1.0f
        mySettings->loadInt32(ST_SETTING_GAMMA, loadedGamma);
        myGUI->stImageRegion->params.gamma->setValue(0.01f * loadedGamma);

    // initialize frame region early to show dedicated error description
    if(!myGUI->stImageRegion->stglInit()) {
        stError("ImagePlugin, frame region initialization failed!");
        return false;
    }
    myGUI->stglInit();
    myGUI->stglResize(myWindow->getPlacement());

    // create the image loader thread
    if(!isReset) {
        StString imageLibString;
        mySettings->loadString(ST_SETTING_IMAGELIB, imageLibString);
        params.imageLib = StImageFile::imgLibFromString(imageLibString);
        myLoader = new StImageLoader(params.imageLib, myLangMap, myGUI->stImageRegion->getTextureQueue());
        myLoader->signals.onLoaded.connect(this, &StImageViewer::doLoaded);
    }
    myLoader->signals.onError.connect(myGUI->myMsgStack, &StGLMsgStack::doPushMessage);

    if(isReset) {
        return true;
    }

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

bool StImageViewer::open() {
    const bool isReset = !mySwitchTo.isNull();
    if(!StApplication::open()
    || !init()) {
        return false;
    }

    if(isReset) {
        myLoader->doLoadNext();
        return true;
    }

    parseArguments(myOpenFileInfo->getArgumentsMap());
    const StMIME anOpenMIME = myOpenFileInfo->getMIME();
    if(myOpenFileInfo->getPath().isEmpty()) {
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

void StImageViewer::processEvents(const StMessage_t* theEvents) {
    bool isMouseMove = false;
    size_t evId(0);
    for(; theEvents[evId].uin != StMessageList::MSG_NULL; ++evId) {
        switch(theEvents[evId].uin) {
            case StMessageList::MSG_RESIZE: {
                myGUI->stglResize(myWindow->getPlacement());
                break;
            }
            case StMessageList::MSG_FULLSCREEN_SWITCH: {
                params.isFullscreen->setValue(myWindow->isFullScreen());
                break;
            }
            case StMessageList::MSG_DRAGNDROP_IN: {
                StString aFilePath;
                int aFilesNb = myWindow->getDragNDropFile(-1, aFilePath);
                if(aFilesNb > 0) {
                    myWindow->getDragNDropFile(0, aFilePath);
                    if(myLoader->getPlayList().checkExtension(aFilePath)) {
                        myLoader->getPlayList().open(aFilePath);
                        doUpdateStateLoading();
                        myLoader->doLoadNext();
                    }
                }
                break;
            }
            case StMessageList::MSG_CLOSE:
            case StMessageList::MSG_EXIT: {
                StApplication::exit(0);
                break;
            }
            case StMessageList::MSG_KEYS: {
                bool* keysMap = (bool* )theEvents[evId].data;
                if(keysMap[ST_VK_ESCAPE]) {
                    // we could parse Escape key in other way
                    if(!myEscNoQuit) {
                        StApplication::exit(0);
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
                int aMouseBtn = myWindow->getMouseDown(pt);
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
                int aMouseBtn = myWindow->getMouseUp(pt);
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
    StApplication::exit(0);
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
    if(keysMap[ST_VK_F12]) {
        params.ToShowFps->reverse();
        keysMap[ST_VK_F12] = false;
    }

    if(keysMap[ST_VK_S] && keysMap[ST_VK_CONTROL]) {
        myLoader->doSaveImageAs(StImageFile::ST_TYPE_PNG);
        keysMap[ST_VK_S] = false;
    }

    keysStereo(keysMap);
    keysSrcFormat(keysMap);
    keysFileWalk(keysMap);
}
