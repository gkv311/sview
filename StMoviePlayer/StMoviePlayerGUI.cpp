/**
 * Copyright © 2009-2015 Kirill Gavrilov <kirill@sview.ru>
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

#include "StMoviePlayerGUI.h"

// OpenAL headers
#if defined(__APPLE__)
    #include <OpenAL/al.h>
    #include <OpenAL/alc.h>
#else
    #include <AL/al.h>
    #include <AL/alc.h>
#endif

#include "StMoviePlayer.h"
#include "StSeekBar.h"
#include "StTimeBox.h"

#include "StVideo/StVideo.h"

#include <StImage/StImageFile.h>
#include <StSettings/StEnumParam.h>

#include <StGLWidgets/StGLAssignHotKey.h>
#include <StGLWidgets/StGLButton.h>
#include <StGLWidgets/StGLCombobox.h>
#include <StGLWidgets/StGLCheckboxTextured.h>
#include <StGLWidgets/StGLDescription.h>
#include <StGLWidgets/StGLFpsLabel.h>
#include <StGLWidgets/StGLImageRegion.h>
#include <StGLWidgets/StGLMenu.h>
#include <StGLWidgets/StGLMenuItem.h>
#include <StGLWidgets/StGLMessageBox.h>
#include <StGLWidgets/StGLMsgStack.h>
#include <StGLWidgets/StGLPlayList.h>
#include <StGLWidgets/StGLRangeFieldFloat32.h>
#include <StGLWidgets/StGLScrollArea.h>
#include <StGLWidgets/StGLSubtitles.h>
#include <StGLWidgets/StGLSwitchTextured.h>
#include <StGLWidgets/StGLTable.h>
#include <StGLWidgets/StGLTextureButton.h>
#include <StVersion.h>

#include "StMoviePlayerStrings.h"

// auxiliary pre-processor definition
#define stCTexture(theString) getTexturePath(stCString(theString))

using namespace StMoviePlayerStrings;

namespace {
    static const int DISPL_Y_REGION_UPPER  = 32;
    static const int DISPL_X_REGION_UPPER  = 32;
    static const int DISPL_X_REGION_BOTTOM = 52;
    static const int DISPL_Y_REGION_BOTTOM = 64;

    static const StGLVec3 aBlack(0.0f, 0.0f, 0.0f);
    static const StGLVec3 aGreen(0.4f, 0.8f, 0.4f);
    static const StGLVec3 aRed  (1.0f, 0.0f, 0.0f);

}

void StMoviePlayerGUI::createDesktopUI(const StHandle<StPlayList>& thePlayList) {
    createUpperToolbar();
    createBottomToolbar();

    myDescr = new StGLDescription(this);

    myPlayList = new StGLPlayList(this, thePlayList);
    myPlayList->setCorner(StGLCorner(ST_VCORNER_CENTER, ST_HCORNER_RIGHT));
    myPlayList->setOpacity(myPlugin->params.ToShowPlayList->getValue() ? 1.0f : 0.0f, false);
    myPlayList->signals.onOpenItem = stSlot(myPlugin, &StMoviePlayer::doFileNext);

    // create main menu
    createMainMenu();
}

/**
 * Create upper toolbar
 */
void StMoviePlayerGUI::createUpperToolbar() {
    int aBtnIter = 0;
    const int aTop  = scale(DISPL_Y_REGION_UPPER);
    const int aLeft = scale(DISPL_X_REGION_UPPER);
    StMarginsI aButtonMargins;
    const IconSize anIconSize = scaleIcon(32, aButtonMargins);
    const int      anIconStep = scale(48);
    aButtonMargins.extend(scale(8));

    const StMarginsI& aMargins = getRootMargins();
    myPanelUpper = new StGLContainer(this, aMargins.left, aMargins.top, StGLCorner(ST_VCORNER_TOP, ST_HCORNER_LEFT), scale(4096), scale(128));

    // append the textured buttons
    myBtnOpen = new StGLTextureButton(myPanelUpper, aLeft + (aBtnIter++) * anIconStep, aTop);
    myBtnOpen->signals.onBtnClick.connect(myPlugin, &StMoviePlayer::doOpen1File);
    myBtnOpen->setTexturePath(iconTexture(stCString("actionOpen"), anIconSize));
    myBtnOpen->setDrawShadow(true);
    myBtnOpen->changeMargins() = aButtonMargins;

    myBtnInfo = new StGLTextureButton(myPanelUpper, aLeft + (aBtnIter++) * anIconStep, aTop);
    myBtnInfo->signals.onBtnClick += stSlot(myPlugin, &StMoviePlayer::doAboutFile);
    myBtnInfo->setTexturePath(iconTexture(stCString("actionInfo"),  anIconSize));
    myBtnInfo->setDrawShadow(true);
    myBtnInfo->changeMargins() = aButtonMargins;

    StGLTextureButton* aSrcBtn = new StGLTextureButton(myPanelUpper, aLeft + (aBtnIter++) * anIconStep, aTop,
                                                       StGLCorner(ST_VCORNER_TOP, ST_HCORNER_LEFT), StFormat_NB);
    aSrcBtn->changeMargins() = aButtonMargins;
    aSrcBtn->signals.onBtnClick += stSlot(this, &StMoviePlayerGUI::doDisplayStereoFormatCombo);
    const StString aSrcTextures[StFormat_NB] = {
        iconTexture(stCString("menuMono"),           anIconSize),
        iconTexture(stCString("menuSbsLR"),          anIconSize),
        iconTexture(stCString("menuSbsRL"),          anIconSize),
        iconTexture(stCString("menuOverUnderLR"),    anIconSize),
        iconTexture(stCString("menuOverUnderRL"),    anIconSize),
        iconTexture(stCString("menuRowLR"),          anIconSize),
        iconTexture(stCString("menuColLR"),          anIconSize),
        iconTexture(stCString("menuSrcSeparate"),    anIconSize),
        iconTexture(stCString("menuFrameSeqLR"),     anIconSize),
        iconTexture(stCString("menuRedCyanLR"),      anIconSize),
        iconTexture(stCString("menuGreenMagentaLR"), anIconSize),
        iconTexture(stCString("menuYellowBlueLR"),   anIconSize),
        iconTexture(stCString("menuTiledLR"),        anIconSize)
    };
    aSrcBtn->setTexturePath(aSrcTextures, StFormat_NB);
    aSrcBtn->setDrawShadow(true);
    myBtnSrcFrmt = aSrcBtn;

    myBtnSwapLR = new StGLCheckboxTextured(myPanelUpper, myImage->params.swapLR,
                                           iconTexture(stCString("actionSwapLROff"), anIconSize),
                                           iconTexture(stCString("actionSwapLROn"),  anIconSize),
                                           aLeft + (aBtnIter++) * anIconStep, aTop,
                                           StGLCorner(ST_VCORNER_TOP, ST_HCORNER_LEFT));
    myBtnSwapLR->setDrawShadow(true);
    myBtnSwapLR->changeMargins() = aButtonMargins;
}

/**
 * Create bottom toolbar
 */
void StMoviePlayerGUI::createBottomToolbar() {
    const StMarginsI& aMargins = getRootMargins();
    const int aTop  = scale(DISPL_Y_REGION_BOTTOM);
    const int aLeft = scale(DISPL_X_REGION_BOTTOM);
    StMarginsI aButtonMargins;
    const IconSize anIconSize = scaleIcon(64, aButtonMargins);

    myPanelBottom = new StGLContainer(this, aMargins.left, -aMargins.bottom, StGLCorner(ST_VCORNER_BOTTOM, ST_HCORNER_LEFT), scale(4096), scale(128));

    // append the textured buttons
    myBtnPlay = new StGLTextureButton(myPanelBottom, aLeft, aTop,
                                      StGLCorner(ST_VCORNER_TOP, ST_HCORNER_LEFT), 2);
    myBtnPlay->signals.onBtnClick.connect(myPlugin, &StMoviePlayer::doPlayPause);
    const StString aPaths[2] = {
        iconTexture(stCString("actionVideoPlayBlue"),  anIconSize),
        iconTexture(stCString("actionVideoPauseBlue"), anIconSize)
    };

    myBtnPlay->setTexturePath(aPaths, 2);
    myBtnPlay->changeMargins() = aButtonMargins;

    myTimeBox = new StTimeBox(myPanelBottom, aLeft + 1 * myIconStep, aTop,
                              StGLCorner(ST_VCORNER_TOP, ST_HCORNER_LEFT));
    myTimeBox->setSwitchOnClick(true);
    myTimeBox->changeRectPx().right()  = myTimeBox->getRectPx().left() + scale(256);//myIconStep * 2;
    myTimeBox->changeRectPx().bottom() = myTimeBox->getRectPx().top()  + scale(64);
    myBtnPrev = new StGLTextureButton(myPanelBottom, -aLeft - 3 * myIconStep, aTop,
                                      StGLCorner(ST_VCORNER_TOP, ST_HCORNER_RIGHT));
    myBtnPrev->signals.onBtnClick.connect(myPlugin, &StMoviePlayer::doListPrev);
    myBtnPrev->setTexturePath(iconTexture(stCString("actionVideoPreviousBlue"), anIconSize));
    myBtnPrev->changeMargins() = aButtonMargins;

    myBtnNext = new StGLTextureButton(myPanelBottom, -aLeft - 2 * myIconStep, aTop,
                                      StGLCorner(ST_VCORNER_TOP, ST_HCORNER_RIGHT));
    myBtnNext->signals.onBtnClick.connect(myPlugin, &StMoviePlayer::doListNext);
    myBtnNext->setTexturePath(iconTexture(stCString("actionVideoNextBlue"), anIconSize));
    myBtnNext->changeMargins() = aButtonMargins;

    myBtnList = new StGLTextureButton(myPanelBottom, -aLeft - myIconStep, aTop,
                                      StGLCorner(ST_VCORNER_TOP, ST_HCORNER_RIGHT));
    myBtnList->signals.onBtnClick.connect(myPlugin, &StMoviePlayer::doPlayListReverse);
    myBtnList->setTexturePath(iconTexture(stCString("actionVideoPlaylistBlue"), anIconSize));
    myBtnList->changeMargins() = aButtonMargins;

    if(myWindow->hasFullscreenMode()) {
        myBtnFullScr = new StGLTextureButton(myPanelBottom, -aLeft, aTop,
                                             StGLCorner(ST_VCORNER_TOP, ST_HCORNER_RIGHT));
        myBtnFullScr->signals.onBtnClick.connect(myPlugin->params.isFullscreen.operator->(), &StBoolParam::doReverse);
        myBtnFullScr->setTexturePath(iconTexture(stCString("actionVideoFullscreenBlue"), anIconSize));
        myBtnFullScr->changeMargins() = aButtonMargins;
    }

    mySeekBar = new StSeekBar(myPanelBottom, 0, scale(4));
    mySeekBar->signals.onSeekClick.connect(myPlugin, &StMoviePlayer::doSeek);
}

/**
 * Main menu
 */
void StMoviePlayerGUI::createMainMenu() {
    const StMarginsI& aMargins = getRootMargins();
    myMenuRoot = new StGLMenu(this, aMargins.left, aMargins.top, StGLMenu::MENU_HORIZONTAL, true);

    StGLMenu* aMenuMedia   = createMediaMenu();     // Root -> Media menu
    StGLMenu* aMenuView    = createViewMenu();      // Root -> View menu
              myMenuAudio  = createAudioMenu();     // Root -> Audio menu
           myMenuSubtitles = createSubtitlesMenu(); // Root -> Subtitles menu
    StGLMenu* aDevicesMenu = createOutputMenu();    // Root -> Output menu
    StGLMenu* aMenuHelp    = createHelpMenu();      // Root -> Help menu

    // Attach sub menus to root
    myMenuRoot->addItem(tr(MENU_MEDIA),     aMenuMedia);
    myMenuRoot->addItem(tr(MENU_VIEW),      aMenuView);
    myMenuRoot->addItem(tr(MENU_AUDIO),     myMenuAudio);
    myMenuRoot->addItem(tr(MENU_SUBTITLES), myMenuSubtitles);
    myMenuRoot->addItem(myPlugin->StApplication::params.ActiveDevice->getActiveValue(), aDevicesMenu);
    myMenuRoot->addItem(tr(MENU_HELP),      aMenuHelp);
}

/**
 * Root -> Media menu
 */
StGLMenu* StMoviePlayerGUI::createMediaMenu() {
    StGLMenu* aMenuMedia = new StGLMenu(this, 0, 0, StGLMenu::MENU_VERTICAL);
    myMenuRecent             = createRecentMenu();       // Root -> Media -> Recent files menu
#ifdef ST_HAVE_MONGOOSE
    StGLMenu* aMenuWebUI     = createWebUIMenu();        // Root -> Media -> Web UI menu
#endif
    StGLMenu* aMenuSrcFormat = createSrcFormatMenu();    // Root -> Media -> Source format menu
    myMenuOpenAL             = createOpenALDeviceMenu(); // Root -> Media -> OpenAL Device
    StGLMenu* aMenuVolume    = createAudioGainMenu();
    StGLMenu* aMenuOpenImage = createOpenMovieMenu();    // Root -> Media -> Open movie menu
    StGLMenu* aMenuSaveImage = createSaveImageMenu();    // Root -> Media -> Save snapshot menu

    aMenuMedia->addItem(tr(MENU_MEDIA_OPEN_MOVIE),       myPlugin->getAction(StMoviePlayer::Action_Open1File),    aMenuOpenImage);
    StGLMenuItem* anItem = aMenuMedia->addItem(tr(MENU_MEDIA_RECENT), myMenuRecent);
    anItem->setUserData(0);
    anItem->signals.onItemClick.connect(myPlugin, &StMoviePlayer::doOpenRecent);
    aMenuMedia->addItem(tr(MENU_MEDIA_SAVE_SNAPSHOT_AS), myPlugin->getAction(StMoviePlayer::Action_SaveSnapshot), aMenuSaveImage);
    aMenuMedia->addItem(tr(MENU_MEDIA_SRC_FORMAT), aMenuSrcFormat);
    aMenuMedia->addItem(tr(MENU_MEDIA_FILE_INFO),  myPlugin->getAction(StMoviePlayer::Action_FileInfo));

    if(myWindow->isMobile()) {
        aMenuMedia->addItem("Mobile UI", myPlugin->params.IsMobileUI);
    }

    aMenuMedia->addItem(tr(MENU_MEDIA_AL_DEVICE),  myMenuOpenAL);
    aMenuMedia->addItem("Audio Volume",            aMenuVolume);

#if defined(_WIN32)
    const StCString aGpuAcc = stCString(" (DXVA2)");
#elif defined(__APPLE__)
    const StCString aGpuAcc = stCString(" (VDA)");
#else
    //const StCString aGpuAcc = stCString("");
#endif

#if defined(_WIN32) || defined(__APPLE__)
#if defined(_WIN32)
    if(myPlugin->params.ToShowExtra->getValue()) {
#endif
        aMenuMedia->addItem(tr(MENU_MEDIA_GPU_DECODING) + aGpuAcc, myPlugin->params.UseGpu);
#if defined(_WIN32)
    }
#endif
#endif

    aMenuMedia->addItem(tr(MENU_MEDIA_SHUFFLE), myPlugin->params.isShuffle);
    if(myPlugin->params.ToShowExtra->getValue()) {
        aMenuMedia->addItem("Loop single item", myPlugin->params.ToLoopSingle);
    }

#ifdef ST_HAVE_MONGOOSE
    aMenuMedia->addItem(tr(MENU_MEDIA_WEBUI) + ":" + myPlugin->params.WebUIPort->getValue(), aMenuWebUI);
#endif
    aMenuMedia->addItem(tr(MENU_MEDIA_QUIT), myPlugin->getAction(StMoviePlayer::Action_Quit));
    return aMenuMedia;
}

/**
 * Root -> Media -> Open movie menu
 */
StGLMenu* StMoviePlayerGUI::createOpenMovieMenu() {
    StGLMenu* aMenu = new StGLMenu(this, 0, 0, StGLMenu::MENU_VERTICAL);
    aMenu->addItem(tr(MENU_MEDIA_OPEN_MOVIE_1), myPlugin->getAction(StMoviePlayer::Action_Open1File));
    aMenu->addItem(tr(MENU_MEDIA_OPEN_MOVIE_2))
         ->signals.onItemClick.connect(myPlugin, &StMoviePlayer::doOpen2Files);
    return aMenu;
}

/**
 * Root -> Media -> Save snapshot menu
 */
StGLMenu* StMoviePlayerGUI::createSaveImageMenu() {
    StGLMenu* aMenu = new StGLMenu(this, 0, 0, StGLMenu::MENU_VERTICAL);
    aMenu->addItem("JPEG stereo (*.jps)", size_t(StImageFile::ST_TYPE_JPEG))
         ->signals.onItemClick.connect(myPlugin, &StMoviePlayer::doSnapshot);
    aMenu->addItem("PNG stereo (*.pns)",  size_t(StImageFile::ST_TYPE_PNG))
         ->signals.onItemClick.connect(myPlugin, &StMoviePlayer::doSnapshot);
    return aMenu;
}

/**
 * Root -> Media -> Source format menu
 */
StGLMenu* StMoviePlayerGUI::createSrcFormatMenu() {
    StGLMenu* aMenu = new StGLMenu(this, 0, 0, StGLMenu::MENU_VERTICAL);
    fillSrcFormatMenu(aMenu);
    return aMenu;
}

void StMoviePlayerGUI::fillSrcFormatMenu(StGLMenu* theMenu) {
    const IconSize anIconSize = scaleIcon(16);
    theMenu->addItem(tr(MENU_SRC_FORMAT_AUTO),         myPlugin->params.srcFormat, StFormat_AUTO)
           ->setIcon(iconTexture(stCString("menuAuto"), anIconSize));
    theMenu->addItem(tr(MENU_SRC_FORMAT_MONO),         myPlugin->params.srcFormat, StFormat_Mono)
           ->setIcon(iconTexture(stCString("menuMono"), anIconSize));
    theMenu->addItem(tr(MENU_SRC_FORMAT_PARALLEL),     myPlugin->params.srcFormat, StFormat_SideBySide_LR)
           ->setIcon(iconTexture(stCString("menuSbsLR"), anIconSize));
    theMenu->addItem(tr(MENU_SRC_FORMAT_CROSS_EYED),   myPlugin->params.srcFormat, StFormat_SideBySide_RL)
           ->setIcon(iconTexture(stCString("menuSbsRL"), anIconSize));
    theMenu->addItem(tr(MENU_SRC_FORMAT_OVERUNDER_LR), myPlugin->params.srcFormat, StFormat_TopBottom_LR)
           ->setIcon(iconTexture(stCString("menuOverUnderLR"), anIconSize));
    theMenu->addItem(tr(MENU_SRC_FORMAT_OVERUNDER_RL), myPlugin->params.srcFormat, StFormat_TopBottom_RL)
           ->setIcon(iconTexture(stCString("menuOverUnderRL"), anIconSize));
    theMenu->addItem(tr(MENU_SRC_FORMAT_INTERLACED),   myPlugin->params.srcFormat, StFormat_Rows)
           ->setIcon(iconTexture(stCString("menuRowLR"), anIconSize));
    theMenu->addItem(tr(MENU_SRC_FORMAT_ANA_RC),       myPlugin->params.srcFormat, StFormat_AnaglyphRedCyan)
           ->setIcon(iconTexture(stCString("menuRedCyanLR"), anIconSize));
    theMenu->addItem(tr(MENU_SRC_FORMAT_ANA_RB),       myPlugin->params.srcFormat, StFormat_AnaglyphGreenMagenta)
           ->setIcon(iconTexture(stCString("menuGreenMagentaLR"), anIconSize));
    theMenu->addItem(tr(MENU_SRC_FORMAT_ANA_YB),       myPlugin->params.srcFormat, StFormat_AnaglyphYellowBlue)
           ->setIcon(iconTexture(stCString("menuYellowBlueLR"), anIconSize));
    theMenu->addItem(tr(MENU_SRC_FORMAT_PAGEFLIP),     myPlugin->params.srcFormat, StFormat_FrameSequence)
           ->setIcon(iconTexture(stCString("menuFrameSeqLR"), anIconSize));
    theMenu->addItem(tr(MENU_SRC_FORMAT_TILED_4X),     myPlugin->params.srcFormat, StFormat_Tiled4x)
           ->setIcon(iconTexture(stCString("menuTiledLR"), anIconSize));
}

void StMoviePlayerGUI::doDisplayStereoFormatCombo(const size_t ) {
    StGLCombobox::ListBuilder aBuilder(this);
    fillSrcFormatMenu(aBuilder.getMenu());
    aBuilder.display();
}

/**
 * Root -> Media -> OpenAL Device menu
 */
StGLMenu* StMoviePlayerGUI::createOpenALDeviceMenu() {
    StGLMenu* aMenu = new StGLMenu(this, 0, 0, StGLMenu::MENU_VERTICAL);
    fillOpenALDeviceMenu(aMenu);
    return aMenu;
}

void StMoviePlayerGUI::fillOpenALDeviceMenu(StGLMenu* theMenu) {
    const StArrayList<StString>& aDevList = myPlugin->params.alDevice->getList();

    theMenu->addItem("Refresh list...")
           ->signals.onItemClick.connect(myPlugin, &StMoviePlayer::doUpdateOpenALDeviceList);

    // OpenAL devices names are often very long...
    size_t aLen = 10;
    for(size_t devId = 0; devId < aDevList.size(); ++devId) {
        aLen = stMax(aLen, aDevList[devId].getLength());
    }
    aLen += 2;
    for(size_t devId = 0; devId < aDevList.size(); ++devId) {
        StGLMenuItem* anItem = theMenu->addItem(aDevList[devId],
                                                StHandle<StInt32Param>::downcast(myPlugin->params.alDevice), int32_t(devId));
        anItem->changeRectPx().right() = anItem->getRectPx().left() + scale(10 * int(aLen));
    }
}

void StMoviePlayerGUI::updateOpenALDeviceMenu() {
    if(myMenuOpenAL == NULL) {
        return;
    }
    for(StGLWidget* aChild = myMenuOpenAL->getChildren()->getStart(); aChild != NULL;) {
        StGLWidget* anItem = aChild;
        aChild = aChild->getNext();
        delete anItem;
    }
    fillOpenALDeviceMenu(myMenuOpenAL);

    // update menu representation
    myMenuOpenAL->stglInit();
}

/**
 * Root -> Media -> Recent files menu
 */
StGLMenu* StMoviePlayerGUI::createRecentMenu() {
    StGLMenu* aMenu = new StGLMenu(this, 0, 0, StGLMenu::MENU_VERTICAL);
    fillRecentMenu(aMenu);
    return aMenu;
}

void StMoviePlayerGUI::fillRecentMenu(StGLMenu* theMenu) {
    StArrayList<StString> aList;
    myPlugin->getRecentList(aList);

    theMenu->addItem(tr(MENU_MEDIA_RECENT_CLEAR))
           ->signals.onItemClick.connect(myPlugin, &StMoviePlayer::doClearRecent);
    for(size_t anIter = 0; anIter < aList.size(); ++anIter) {
        theMenu->addItem(aList[anIter], int32_t(anIter))
               ->signals.onItemClick.connect(myPlugin, &StMoviePlayer::doOpenRecent);
    }
}

/**
 * Root -> Media -> Web UI menu
 */
StGLMenu* StMoviePlayerGUI::createWebUIMenu() {
    StGLMenu* aMenu = new StGLMenu(this, 0, 0, StGLMenu::MENU_VERTICAL);
    for(size_t anIter = 0; anIter < myPlugin->params.StartWebUI->getValues().size(); ++anIter) {
        aMenu->addItem(myPlugin->params.StartWebUI->getValues()[anIter], myPlugin->params.StartWebUI, (int32_t )anIter);
    }
    aMenu->addItem(tr(MENU_MEDIA_WEBUI_SHOW_ERRORS), myPlugin->params.ToPrintWebErrors);
    return aMenu;
}

void StMoviePlayerGUI::updateRecentMenu() {
    if(myMenuRecent == NULL) {
        return;
    }
    for(StGLWidget* aChild = myMenuRecent->getChildren()->getStart(); aChild != NULL;) {
        StGLWidget* anItem = aChild;
        aChild = aChild->getNext();
        delete anItem;
    }
    fillRecentMenu(myMenuRecent);

    // update menu representation
    myMenuRecent->stglInit();
}

/**
 * Root -> View menu
 */
StGLMenu* StMoviePlayerGUI::createViewMenu() {
    StGLMenu* aMenuView = new StGLMenu(this, 0, 0, StGLMenu::MENU_VERTICAL);
    StGLMenu* aMenuDispMode  = createDisplayModeMenu();
    StGLMenu* aMenuDispRatio = createDisplayRatioMenu();
    StGLMenu* aMenuSurface   = createSurfaceMenu();
    StGLMenu* aMenuTexFilter = createSmoothFilterMenu();
    StGLMenu* aMenuImgAdjust = createImageAdjustMenu();

    aMenuView->addItem(tr(MENU_VIEW_DISPLAY_MODE),  aMenuDispMode);
    if(myWindow->hasFullscreenMode()) {
        aMenuView->addItem(tr(MENU_VIEW_FULLSCREEN),    myPlugin->params.isFullscreen);
    }
    aMenuView->addItem(tr(MENU_VIEW_RESET))
             ->signals.onItemClick.connect(myPlugin, &StMoviePlayer::doReset);
    aMenuView->addItem(tr(MENU_VIEW_SWAP_LR),       myImage->params.swapLR);
    aMenuView->addItem(tr(MENU_VIEW_DISPLAY_RATIO), aMenuDispRatio);
    aMenuView->addItem(tr(MENU_VIEW_SURFACE),       aMenuSurface);
    aMenuView->addItem(tr(MENU_VIEW_TEXFILTER),     aMenuTexFilter);
    aMenuView->addItem(tr(MENU_VIEW_IMAGE_ADJUST),  aMenuImgAdjust);
    return aMenuView;
}

/**
 * Root -> View menu -> Output
 */
StGLMenu* StMoviePlayerGUI::createDisplayModeMenu() {
    StGLMenu* aMenu = new StGLMenu(this, 0, 0, StGLMenu::MENU_VERTICAL);
    const StArrayList<StString>& aValuesList = myImage->params.displayMode->getValues();
    for(size_t aValIter = 0; aValIter < aValuesList.size(); ++aValIter) {
        aMenu->addItem(aValuesList[aValIter], myImage->params.displayMode, int32_t(aValIter));
    }
    return aMenu;
}

/**
 * Root -> View menu -> Display Ratio
 */
StGLMenu* StMoviePlayerGUI::createDisplayRatioMenu() {
    StGLMenu* aMenu = new StGLMenu(this, 0, 0, StGLMenu::MENU_VERTICAL);
    fillDisplayRatioMenu(aMenu);
    aMenu->addItem("Keep on restart", myPlugin->params.toRestoreRatio);
    return aMenu;
}

void StMoviePlayerGUI::fillDisplayRatioMenu(StGLMenu* theMenu) {
    const IconSize anIconSize = scaleIcon(16);
    theMenu->addItem("Source", myImage->params.displayRatio, StGLImageRegion::RATIO_AUTO)
           ->setIcon(iconTexture(stCString("menuAuto"), anIconSize));
    theMenu->addItem("2.21:1", myImage->params.displayRatio, StGLImageRegion::RATIO_221_1)
           ->setIcon(iconTexture(stCString("menuRatio2_1_"), anIconSize));
    theMenu->addItem("16:9",   myImage->params.displayRatio, StGLImageRegion::RATIO_16_9)
           ->setIcon(iconTexture(stCString("menuRatio16_9_"), anIconSize));
    theMenu->addItem("16:10",  myImage->params.displayRatio, StGLImageRegion::RATIO_16_10)
           ->setIcon(iconTexture(stCString("menuRatio16_10_"), anIconSize));
    theMenu->addItem("4:3",    myImage->params.displayRatio, StGLImageRegion::RATIO_4_3)
           ->setIcon(iconTexture(stCString("menuRatio4_3_"), anIconSize));
    theMenu->addItem("5:4",    myImage->params.displayRatio, StGLImageRegion::RATIO_5_4)
           ->setIcon(iconTexture(stCString("menuRatio5_4_"), anIconSize));
    theMenu->addItem("1:1",    myImage->params.displayRatio, StGLImageRegion::RATIO_1_1)
           ->setIcon(iconTexture(stCString("menuRatio1_1_"), anIconSize));
}

void StMoviePlayerGUI::doDisplayRatioCombo(const size_t ) {
    StGLCombobox::ListBuilder aBuilder(this);
    fillDisplayRatioMenu(aBuilder.getMenu());
    aBuilder.display();
}

/**
 * Root -> View menu -> Surface
 */
StGLMenu* StMoviePlayerGUI::createSurfaceMenu() {
    StGLMenu* aMenu = new StGLMenu(this, 0, 0, StGLMenu::MENU_VERTICAL);
    aMenu->addItem(tr(MENU_VIEW_SURFACE_PLANE),
                   myImage->params.ViewMode, StStereoParams::FLAT_IMAGE);
    aMenu->addItem(tr(MENU_VIEW_SURFACE_SPHERE),
                   myImage->params.ViewMode, StStereoParams::PANORAMA_SPHERE);
    return aMenu;
}

/**
 * Root -> View menu -> Smooth Filter
 */
StGLMenu* StMoviePlayerGUI::createSmoothFilterMenu() {
    StGLMenu* aMenu = new StGLMenu(this, 0, 0, StGLMenu::MENU_VERTICAL);
    aMenu->addItem(tr(MENU_VIEW_TEXFILTER_NEAREST),
                   myImage->params.textureFilter, StGLImageProgram::FILTER_NEAREST);
    aMenu->addItem(tr(MENU_VIEW_TEXFILTER_LINEAR),
                   myImage->params.textureFilter, StGLImageProgram::FILTER_LINEAR);
    aMenu->addItem(tr(MENU_VIEW_TEXFILTER_BLEND),
                   myImage->params.textureFilter, StGLImageProgram::FILTER_BLEND);
    return aMenu;
}

/**
 * Root -> View menu -> Image Adjust
 */
StGLMenu* StMoviePlayerGUI::createImageAdjustMenu() {
    StGLMenu* aMenu = new StGLMenu(this, 0, 0, StGLMenu::MENU_VERTICAL);
    aMenu->addItem(tr(MENU_VIEW_ADJUST_RESET), myPlugin->getAction(StMoviePlayer::Action_ImageAdjustReset));

    StGLMenuItem* anItem = aMenu->addItem(tr(MENU_VIEW_ADJUST_GAMMA));
    anItem->changeMargins().right = scale(100 + 16);
    StGLRangeFieldFloat32* aRange = new StGLRangeFieldFloat32(anItem, myImage->params.gamma,
                                                              -scale(16), 0, StGLCorner(ST_VCORNER_CENTER, ST_HCORNER_RIGHT));
    aRange->setFormat(stCString("%+01.2f"));
    aRange->setColor(StGLRangeFieldFloat32::FieldColor_Default,  aBlack);
    aRange->setColor(StGLRangeFieldFloat32::FieldColor_Positive, aGreen);
    aRange->setColor(StGLRangeFieldFloat32::FieldColor_Negative, aRed);

    anItem = aMenu->addItem(tr(MENU_VIEW_ADJUST_BRIGHTNESS));
    anItem->changeMargins().right = scale(100 + 16);
    aRange = new StGLRangeFieldFloat32(anItem, myImage->params.brightness,
                                       -scale(16), 0, StGLCorner(ST_VCORNER_CENTER, ST_HCORNER_RIGHT));
    aRange->setFormat(stCString("%+01.2f"));
    aRange->setColor(StGLRangeFieldFloat32::FieldColor_Default,  aBlack);
    aRange->setColor(StGLRangeFieldFloat32::FieldColor_Positive, aGreen);
    aRange->setColor(StGLRangeFieldFloat32::FieldColor_Negative, aRed);

    anItem = aMenu->addItem(tr(MENU_VIEW_ADJUST_SATURATION));
    anItem->changeMargins().right = scale(100 + 16);
    aRange = new StGLRangeFieldFloat32(anItem, myImage->params.saturation,
                                       -scale(16), 0, StGLCorner(ST_VCORNER_CENTER, ST_HCORNER_RIGHT));
    aRange->changeRectPx().bottom() = aRange->getRectPx().top() + aMenu->getItemHeight();
    aRange->setFormat(stCString("%+01.2f"));
    aRange->setColor(StGLRangeFieldFloat32::FieldColor_Default,  aBlack);
    aRange->setColor(StGLRangeFieldFloat32::FieldColor_Positive, aGreen);
    aRange->setColor(StGLRangeFieldFloat32::FieldColor_Negative, aRed);
    return aMenu;
}

/**
 * Root -> Audio menu
 */
StGLMenu* StMoviePlayerGUI::createAudioMenu() {
    StGLMenu* aMenu = new StGLMenu(this, 0, 0, StGLMenu::MENU_VERTICAL);
    aMenu->addItem(tr(MENU_AUDIO_NONE), myPlugin->params.audioStream, -1);
    return aMenu;
}

/**
 * Root -> Audio menu -> Volume
 */
StGLMenu* StMoviePlayerGUI::createAudioGainMenu() {
    StGLMenu* aMenu = new StGLMenu(this, 0, 0, StGLMenu::MENU_VERTICAL);

    StGLMenuItem* anItem = aMenu->addItem("Volume");
    anItem->changeMargins().right = scale(110 + 16);
    StGLRangeFieldFloat32* aRange = new StGLRangeFieldFloat32(anItem, myPlugin->params.AudioGain,
                                                              -scale(16), 0, StGLCorner(ST_VCORNER_CENTER, ST_HCORNER_RIGHT));
    aRange->setFormat(stCString("%+03.0f dB"));
    aRange->setColor(StGLRangeFieldFloat32::FieldColor_Default,  aBlack);
    aRange->setColor(StGLRangeFieldFloat32::FieldColor_Positive, aGreen);
    aRange->setColor(StGLRangeFieldFloat32::FieldColor_Negative, aRed);

    aMenu->addItem("Mute", myPlugin->params.AudioMute);
    return aMenu;
}

/**
 * Dialog for Audio/Video synchronization control.
 */
class ST_LOCAL StDelayControl : public StGLMessageBox {

        public:

    StDelayControl(StMoviePlayerGUI*               theParent,
                   const StHandle<StFloat32Param>& theTrackedValue)
    : StGLMessageBox(theParent, theParent->tr(DIALOG_AUDIO_DELAY_TITLE), "", theParent->scale(400), theParent->scale(260)),
      myRange(NULL) {
        changeRectPx().moveX( myRoot->scale( 64));
        changeRectPx().moveY(-myRoot->scale(128));
        setCorner(StGLCorner(ST_VCORNER_BOTTOM, ST_HCORNER_LEFT));
        StGLButton* aResetBtn = addButton(theParent->tr(BUTTON_RESET));
        addButton(theParent->tr(BUTTON_CLOSE));

        StGLWidget* aContent = new StGLContainer(getContent(), 0, 0, StGLCorner(ST_VCORNER_TOP, ST_HCORNER_LEFT),
                                                 getContent()->getRectPx().width(), getContent()->getRectPx().height());

        const StGLVec3 aWhite(1.0f, 1.0f, 1.0f);
        StGLTextArea* aText = new StGLTextArea(aContent, 0, 0, StGLCorner(ST_VCORNER_TOP, ST_HCORNER_LEFT),
                                               aContent->getRectPx().width(), myRoot->scale(10));
        aText->setText(theParent->tr(DIALOG_AUDIO_DELAY_DESC) + "\n");
        aText->setTextColor(aWhite);
        aText->stglInitAutoHeight();

        StGLTextArea* aLabel = new StGLTextArea(aContent, 0, aText->getRectPx().bottom(), StGLCorner(ST_VCORNER_TOP, ST_HCORNER_LEFT),
                                                -myRoot->scale(1), myRoot->scale(10));
        aLabel->setText(theParent->tr(DIALOG_AUDIO_DELAY_LABEL));
        aLabel->setTextColor(aWhite);
        aLabel->stglInitAutoHeightWidth();

        myRange = new StGLRangeFieldFloat32(aContent, theTrackedValue,
                                            aLabel->getRectPx().right() + myRoot->scale(10), aLabel->getRectPx().top());
        myRange->setFormat(stCString("%+01.3f"));
        myRange->setColor(StGLRangeFieldFloat32::FieldColor_Default,  StGLVec3(1.0f, 1.0f, 1.0f));
        myRange->setColor(StGLRangeFieldFloat32::FieldColor_Positive, StGLVec3(0.4f, 0.8f, 0.4f));
        myRange->setColor(StGLRangeFieldFloat32::FieldColor_Negative, StGLVec3(1.0f, 0.0f, 0.0f));
        myRange->stglInit();

        StGLTextArea* aLabUnits = new StGLTextArea(aContent, myRange->getRectPx().right() + myRoot->scale(10), aLabel->getRectPx().top(),
                                                   StGLCorner(ST_VCORNER_TOP, ST_HCORNER_LEFT), -myRoot->scale(1), myRoot->scale(10));
        aLabUnits->setText(theParent->tr(DIALOG_AUDIO_DELAY_UNITS));
        aLabUnits->setTextColor(aWhite);
        aLabUnits->stglInitAutoHeightWidth();

        aResetBtn->signals.onBtnClick = stSlot(myRange, &StGLRangeFieldFloat32::doResetValue);
    }

    virtual ~StDelayControl() {}

    virtual bool doKeyDown(const StKeyEvent& theEvent) {
        if(StGLMessageBox::doKeyDown(theEvent)) {
            return true;
        }
        switch(theEvent.VKey) {
            case ST_VK_ADD:
            case ST_VK_OEM_PLUS:
                myRange->doIncrement(0);
                return true;
            case ST_VK_SUBTRACT:
            case ST_VK_OEM_MINUS:
                myRange->doDecrement(0);
                return true;
            default:
                return false;
        }
    }

        private:

    StGLRangeFieldFloat32* myRange;

};

void StMoviePlayerGUI::doAudioDelay(const size_t ) {
    StGLMessageBox* aDialog = new StDelayControl(this, myPlugin->params.AudioDelay);
    aDialog->stglInit();
}

/**
 * Root -> Subtitles menu
 */
StGLMenu* StMoviePlayerGUI::createSubtitlesMenu() {
    StGLMenu* aMenu = new StGLMenu(this, 0, 0, StGLMenu::MENU_VERTICAL);
    aMenu->addItem(tr(MENU_SUBTITLES_NONE), myPlugin->params.subtitlesStream, -1);
    return aMenu;
}

/**
 * Root -> Output menu
 */
StGLMenu* StMoviePlayerGUI::createOutputMenu() {
    StGLMenu* aMenu = new StGLMenu(this, 0, 0, StGLMenu::MENU_VERTICAL);

    StGLMenu* aMenuChangeDevice = new StGLMenu(this, 0, 0, StGLMenu::MENU_VERTICAL);
    StGLMenu* aMenuFpsControl   = createFpsMenu();

    const StHandle<StEnumParam>& aDevicesEnum = myPlugin->StApplication::params.ActiveDevice;
    const StArrayList<StString>& aValuesList  = aDevicesEnum->getValues();
    for(size_t aValIter = 0; aValIter < aValuesList.size(); ++aValIter) {
        aMenuChangeDevice->addItem(aValuesList[aValIter], aDevicesEnum, int32_t(aValIter));
    }

    aMenu->addItem(tr(MENU_CHANGE_DEVICE), aMenuChangeDevice);
    aMenu->addItem(tr(MENU_ABOUT_RENDERER))
         ->signals.onItemClick.connect(this, &StMoviePlayerGUI::doAboutRenderer);
    aMenu->addItem(tr(MENU_FPS),           aMenuFpsControl);

    const StHandle<StWindow>& aRend = myPlugin->getMainWindow();
    StParamsList aParams;
    aRend->getOptions(aParams);
    StHandle<StBoolParamNamed> aBool;
    StHandle<StEnumParam>      anEnum;
    for(size_t anIter = 0; anIter < aParams.size(); ++anIter) {
        const StHandle<StParamBase>& aParam = aParams[anIter];
        if(aBool.downcastFrom(aParam)) {
            aMenu->addItem(aBool->getName(), aBool);
        } else if(anEnum.downcastFrom(aParam)) {
            StGLMenu* aChildMenu = new StGLMenu(this, 0, 0, StGLMenu::MENU_VERTICAL);
            const StArrayList<StString>& aValues = anEnum->getValues();
            for(size_t aValIter = 0; aValIter < aValues.size(); ++aValIter) {
                aChildMenu->addItem(aValues[aValIter], anEnum, int32_t(aValIter));
            }
            aMenu->addItem(anEnum->getName(), aChildMenu);
        }
    }
    return aMenu;
}

/**
 * Root -> Output -> FPS Control
 */
StGLMenu* StMoviePlayerGUI::createFpsMenu() {
    StGLMenu* aMenu = new StGLMenu(this, 0, 0, StGLMenu::MENU_VERTICAL);
    aMenu->addItem(tr(MENU_FPS_VSYNC), myPlugin->params.IsVSyncOn);
    aMenu->addItem(tr(MENU_FPS_METER), myPlugin->params.ToShowFps);
    aMenu->addItem(tr(MENU_FPS_BOUND), myPlugin->params.ToLimitFps);
    return aMenu;
}

void StMoviePlayerGUI::doAboutProgram(const size_t ) {
    StGLMessageBox* aDialog = new StGLMessageBox(this, "",
          tr(ABOUT_DPLUGIN_NAME) + '\n'
        + tr(ABOUT_VERSION) + " " + StVersionInfo::getSDKVersionString()
        + "\n \n" + tr(ABOUT_DESCRIPTION).format("2007-2015", "kirill@sview.ru", "www.sview.ru"),
        scale(512), scale(300));
    aDialog->addButton(tr(BUTTON_CLOSE));
    aDialog->stglInit();
}

void StMoviePlayerGUI::doUserTips(const size_t ) {
    StProcess::openURL("http://sview.ru/sview2009/usertips");
}

void StMoviePlayerGUI::doAboutSystem(const size_t ) {
    const StString aTitle = tr(ABOUT_SYSTEM);
    StGLMessageBox* aDialog = new StGLMessageBox(this, aTitle, "", scale(512), scale(256));

    StArgumentsMap anInfo;
    anInfo.add(StDictEntry("CPU cores", StString(StThread::countLogicalProcessors()) + StString(" logical processor(s)")));
    getContext().stglFullInfo(anInfo);
    StGLTable* aTable = new StGLTable(aDialog->getContent(), 0, 0, StGLCorner(ST_VCORNER_TOP, ST_HCORNER_CENTER));
    aTable->fillFromMap(anInfo, StGLVec3(1.0f, 1.0f, 1.0f), aDialog->getContent()->getRectPx().width(), aDialog->getContent()->getRectPx().width() / 2);

    aDialog->addButton(tr(BUTTON_CLOSE));
    aDialog->stglInit();
}

/**
 * Customized message box.
 */
class ST_LOCAL StInfoDialog : public StGLMessageBox {

        public:

    ST_LOCAL StInfoDialog(StMoviePlayer*  thePlugin,
                          StGLWidget*     theParent,
                          const StString& theTitle,
                          const int       theWidth,
                          const int       theHeight)
    : StGLMessageBox(theParent, theTitle, "", theWidth, theHeight),
      myPlugin(thePlugin) {}

    ST_LOCAL virtual ~StInfoDialog() {
        myPlugin->doSaveFileInfo(0);
    }

        private:

    StMoviePlayer* myPlugin;

};

void StMoviePlayerGUI::doAboutFile(const size_t ) {
    StHandle<StMovieInfo>& anExtraInfo = myPlugin->myFileInfo;
    if(!anExtraInfo.isNull()) {
        return; // already opened
    }

    StHandle<StFileNode>     aFileNode;
    StHandle<StStereoParams> aParams;
    if(!myPlugin->getCurrentFile(aFileNode, aParams, anExtraInfo)
    ||  anExtraInfo.isNull()) {
        StHandle<StMsgQueue> aQueue = myPlugin->getMessagesQueue();
        aQueue->pushInfo(tr(DIALOG_FILE_NOINFO));
        anExtraInfo.nullify();
        return;
    }

    const int THE_MIN_WIDTH = scale(512);
    const StString aTitle  = tr(DIALOG_FILE_INFO);
    const int      aWidth  = stMax(int(double(getRectPx().width()) * 0.6), THE_MIN_WIDTH);
    StInfoDialog*  aDialog = new StInfoDialog(myPlugin, this, aTitle, aWidth, scale(300));

    // translate known metadata tag names
    for(size_t aMapIter = 0; aMapIter < anExtraInfo->Info.size(); ++aMapIter) {
        StDictEntry& anEntry = anExtraInfo->Info.changeValue(aMapIter);
        StString     aKey    = anEntry.getKey().lowerCased();
        anEntry.changeName() = myLangMap->getValue(aKey);
    }

    const StGLVec3 aWhite(1.0f, 1.0f, 1.0f);
    const int    aWidthMax  = aDialog->getContent()->getRectPx().width();
    StGLTable*   aTable     = new StGLTable(aDialog->getContent(), 0, 0, StGLCorner(ST_VCORNER_TOP, ST_HCORNER_CENTER));
    int          aRowLast   = (int )anExtraInfo->Info.size();
    const int    aNbRowsMax = aRowLast + (int )anExtraInfo->Codecs.size() + 3;
    aTable->setupTable((int )aNbRowsMax, 2);
    aTable->fillFromMap(anExtraInfo->Info, aWhite,
                        aWidthMax, aWidthMax / 2);

    const int aTextMaxWidth = aWidthMax - (aTable->getItemMargins().left + aTable->getItemMargins().right);

    if(anExtraInfo->HasVideo) {
        // add stereoscopic format info
        const StFormat anActiveSrcFormat = aParams->ToSwapLR
                                         ? st::formatReversed(aParams->StereoFormat)
                                         : aParams->StereoFormat;
        StGLTableItem& aSrcFormatItem = aTable->changeElement(aRowLast++, 0); aSrcFormatItem.setColSpan(2);
        StGLTextArea*  aSrcFormatText = new StGLTextArea(&aSrcFormatItem, 0, 0, StGLCorner(ST_VCORNER_CENTER, ST_HCORNER_CENTER));
        aSrcFormatText->setupAlignment(StGLTextFormatter::ST_ALIGN_X_CENTER,
                                       StGLTextFormatter::ST_ALIGN_Y_TOP);
        aSrcFormatText->setText(StString("\n") + tr(BTN_SRC_FORMAT) + " " + trSrcFormat(anActiveSrcFormat));
        aSrcFormatText->setTextColor(aWhite);
        aSrcFormatText->stglInitAutoHeightWidth(aTextMaxWidth);

        // warn about wrong/missing stereoscopic format information
        StString aSrcInfo;
        StGLVec3 anExtraColor = aWhite;
        if(anExtraInfo->StInfoStream == StFormat_AUTO
        && anActiveSrcFormat != StFormat_Mono
        && anActiveSrcFormat != StFormat_SeparateFrames) {
            anExtraColor = StGLVec3(1.0f, 1.0f, 0.8f);
            if(anExtraInfo->StInfoFileName != StFormat_AUTO
            && anExtraInfo->StInfoFileName == anActiveSrcFormat) {
                aSrcInfo = tr(INFO_NO_SRCFORMAT_EX);
            } else {
                aSrcInfo = tr(INFO_NO_SRCFORMAT);
            }
        } else if(anExtraInfo->StInfoStream != StFormat_AUTO
               && anExtraInfo->StInfoStream != anActiveSrcFormat) {
            aSrcInfo     = tr(INFO_WRONG_SRCFORMAT);
            anExtraColor = StGLVec3(1.0f, 0.0f, 0.0f);
        }
        if(!aSrcInfo.isEmpty()) {
            StGLTableItem& aTabItem = aTable->changeElement(aRowLast++, 0); aTabItem.setColSpan(2);
            StGLTextArea*  aText    = new StGLTextArea(&aTabItem, 0, 0, StGLCorner(ST_VCORNER_CENTER, ST_HCORNER_CENTER));
            aText->setupAlignment(StGLTextFormatter::ST_ALIGN_X_CENTER,
                                  StGLTextFormatter::ST_ALIGN_Y_TOP);
            aText->setText(aSrcInfo);
            aText->setTextColor(anExtraColor);
            aText->stglInitAutoHeightWidth(aTextMaxWidth);
        }
    }

    // append information about active decoders
    StGLTableItem& aCodecItem = aTable->changeElement(aRowLast++, 0);
    aCodecItem.setColSpan(2);

    StGLTextArea* aCodecsText = new StGLTextArea(&aCodecItem, 0, 0, StGLCorner(ST_VCORNER_TOP, ST_HCORNER_CENTER));
    aCodecsText->setupAlignment(StGLTextFormatter::ST_ALIGN_X_LEFT,
                                StGLTextFormatter::ST_ALIGN_Y_TOP);
    aCodecsText->setText(StString('\n') + tr(DIALOG_FILE_DECODERS) + '\n');
    aCodecsText->setTextColor(aWhite);
    aCodecsText->stglInitAutoHeightWidth(aTextMaxWidth);
    for(size_t aKeyIter = 0; aKeyIter < anExtraInfo->Codecs.size(); ++aKeyIter) {
        const StArgument& aPair = anExtraInfo->Codecs.getFromIndex(aKeyIter);
        if(aPair.getValue().isEmpty()) {
            continue;
        }

        StGLTableItem& anItem = aTable->changeElement(aRowLast++, 0);
        anItem.setColSpan(2);

        StGLTextArea* aText = new StGLTextArea(&anItem, 0, 0, StGLCorner(ST_VCORNER_TOP, ST_HCORNER_CENTER));
        aText->setupAlignment(StGLTextFormatter::ST_ALIGN_X_LEFT,
                              StGLTextFormatter::ST_ALIGN_Y_TOP);
        aText->setText(aPair.getValue());
        aText->setTextColor(aWhite);
        aText->stglInitAutoHeightWidth(aTextMaxWidth);
    }
    aTable->updateLayout();
    const int aWidthAdjust = aWidth - stMax(aTable->getRectPx().width() + aDialog->getMarginLeft() + aDialog->getMarginRight(), THE_MIN_WIDTH);
    if(aWidthAdjust > 0) {
        aDialog->changeRectPx().right()               -= aWidthAdjust;
        aDialog->getContent()->changeRectPx().right() -= aWidthAdjust;
    }

    aDialog->addButton(tr(BUTTON_CLOSE));
    aDialog->stglInit();
}

void StMoviePlayerGUI::doCheckUpdates(const size_t ) {
    StProcess::openURL("http://www.sview.ru/download");
}

void StMoviePlayerGUI::doOpenLicense(const size_t ) {
    StProcess::openURL(StProcess::getStShareFolder()
                       + "info" + SYS_FS_SPLITTER
                       + "license.txt");
}

/**
 * Root -> Help menu
 */
StGLMenu* StMoviePlayerGUI::createHelpMenu() {
    StGLMenu* aMenu = new StGLMenu(this, 0, 0, StGLMenu::MENU_VERTICAL);
    StGLMenu* aMenuScale        = createScaleMenu();        // Root -> Help -> Scale Interface menu
    StGLMenu* aMenuBlockSleep   = createBlockSleepMenu();   // Root -> Help -> Block sleeping
#if !defined(ST_NO_UPDATES_CHECK)
    StGLMenu* aMenuCheckUpdates = createCheckUpdatesMenu(); // Root -> Help -> Check updates menu
#endif
    StGLMenu* aMenuLanguage     = createLanguageMenu();     // Root -> Help -> Language menu

    aMenu->addItem(tr(MENU_HELP_ABOUT))
         ->signals.onItemClick.connect(this, &StMoviePlayerGUI::doAboutProgram);
    aMenu->addItem(tr(MENU_HELP_USERTIPS))
         ->signals.onItemClick.connect(this, &StMoviePlayerGUI::doUserTips);
    aMenu->addItem(tr(MENU_HELP_HOTKEYS))
         ->signals.onItemClick.connect(this, &StMoviePlayerGUI::doListHotKeys);
    aMenu->addItem(tr(MENU_HELP_LICENSE))
         ->signals.onItemClick.connect(this, &StMoviePlayerGUI::doOpenLicense);
    aMenu->addItem(tr(MENU_HELP_SYSINFO))
         ->signals.onItemClick.connect(this, &StMoviePlayerGUI::doAboutSystem);
    aMenu->addItem(tr(MENU_HELP_EXPERIMENTAL), myPlugin->params.ToShowExtra);
    aMenu->addItem(tr(MENU_HELP_SCALE),        aMenuScale);
    aMenu->addItem(tr(MENU_HELP_BLOCKSLP),     aMenuBlockSleep);
#if !defined(ST_NO_UPDATES_CHECK)
    aMenu->addItem(tr(MENU_HELP_UPDATES),      aMenuCheckUpdates);
#endif
    aMenu->addItem(tr(MENU_HELP_LANGS),        aMenuLanguage);
    return aMenu;
}

/**
 * Root -> Help -> Scale Interface menu
 */
StGLMenu* StMoviePlayerGUI::createScaleMenu() {
    StGLMenu* aMenu = new StGLMenu(this, 0, 0, StGLMenu::MENU_VERTICAL);
    aMenu->addItem(tr(MENU_HELP_SCALE_SMALL),   myPlugin->params.ScaleAdjust,  StGLRootWidget::ScaleAdjust_Small);
    aMenu->addItem(tr(MENU_HELP_SCALE_NORMAL),  myPlugin->params.ScaleAdjust,  StGLRootWidget::ScaleAdjust_Normal);
    aMenu->addItem(tr(MENU_HELP_SCALE_BIG),     myPlugin->params.ScaleAdjust,  StGLRootWidget::ScaleAdjust_Big);
    aMenu->addItem(tr(MENU_HELP_SCALE_HIDPI2X), myPlugin->params.ScaleHiDPI2X);
    aMenu->addItem("Mobile UI",                 myPlugin->params.IsMobileUI);
    return aMenu;
}

/**
 * Root -> Help -> Block sleeping
 */
StGLMenu* StMoviePlayerGUI::createBlockSleepMenu() {
    StGLMenu* aMenu = new StGLMenu(this, 0, 0, StGLMenu::MENU_VERTICAL);
    aMenu->addItem(tr(MENU_HELP_BLOCKSLP_NEVER),
                   myPlugin->params.blockSleeping, StMoviePlayer::BLOCK_SLEEP_NEVER);
    aMenu->addItem(tr(MENU_HELP_BLOCKSLP_PLAYBACK),
                   myPlugin->params.blockSleeping, StMoviePlayer::BLOCK_SLEEP_PLAYBACK);
    aMenu->addItem(tr(MENU_HELP_BLOCKSLP_FULLSCR),
                   myPlugin->params.blockSleeping, StMoviePlayer::BLOCK_SLEEP_FULLSCREEN);
    aMenu->addItem(tr(MENU_HELP_BLOCKSLP_ALWAYS),
                   myPlugin->params.blockSleeping, StMoviePlayer::BLOCK_SLEEP_ALWAYS);
    return aMenu;
}

/**
 * Root -> Help -> Check updates menu
 */
StGLMenu* StMoviePlayerGUI::createCheckUpdatesMenu() {
    StGLMenu* aMenu = new StGLMenu(this, 0, 0, StGLMenu::MENU_VERTICAL);
    aMenu->addItem(tr(MENU_HELP_UPDATES_NOW))
         ->signals.onItemClick.connect(this, &StMoviePlayerGUI::doCheckUpdates);
    aMenu->addItem(tr(MENU_HELP_UPDATES_DAY),   myPlugin->params.checkUpdatesDays, 1);
    aMenu->addItem(tr(MENU_HELP_UPDATES_WEEK),  myPlugin->params.checkUpdatesDays, 7);
    aMenu->addItem(tr(MENU_HELP_UPDATES_YEAR),  myPlugin->params.checkUpdatesDays, 355);
    aMenu->addItem(tr(MENU_HELP_UPDATES_NEVER), myPlugin->params.checkUpdatesDays, 0);
    return aMenu;
}

/**
 * Root -> Help -> Language menu
 */
StGLMenu* StMoviePlayerGUI::createLanguageMenu() {
    StGLMenu* aMenu = new StGLMenu(this, 0, 0, StGLMenu::MENU_VERTICAL);
    for(size_t aLangId = 0; aLangId < myLangMap->getLanguagesList().size(); ++aLangId) {
        aMenu->addItem(myLangMap->getLanguagesList()[aLangId], myLangMap->params.language, int32_t(aLangId));
    }
    return aMenu;
}

void StMoviePlayerGUI::createMobileUI(const StHandle<StPlayList>& thePlayList) {
    createMobileUpperToolbar();
    createMobileBottomToolbar();

    myPlayList = new StGLPlayList(this, thePlayList);
    myPlayList->setCorner(StGLCorner(ST_VCORNER_CENTER, ST_HCORNER_RIGHT));
    myPlayList->setOpacity(myPlugin->params.ToShowPlayList->getValue() ? 1.0f : 0.0f, false);
    myPlayList->signals.onOpenItem = stSlot(myPlugin, &StMoviePlayer::doFileNext);
}

/**
 * Create upper toolbar
 */
void StMoviePlayerGUI::createMobileUpperToolbar() {
    StMarginsI aButtonMargins;
    const IconSize anIconSize = scaleIcon(32, aButtonMargins);
    aButtonMargins.extend(scale(12));

    const StMarginsI& aRootMargins = getRootMargins();
    myPanelUpper = new StGLContainer(this, aRootMargins.left, aRootMargins.top, StGLCorner(ST_VCORNER_TOP, ST_HCORNER_LEFT), scale(4096), scale(56));

    int aBtnIter = 0;

    StGLTextureButton* aSrcBtn = new StGLTextureButton(myPanelUpper, (aBtnIter++) * myIconStep, 0,
                                                       StGLCorner(ST_VCORNER_TOP, ST_HCORNER_LEFT), StFormat_NB);
    aSrcBtn->changeMargins() = aButtonMargins;
    aSrcBtn->signals.onBtnClick += stSlot(this, &StMoviePlayerGUI::doDisplayStereoFormatCombo);
    const StString aSrcTextures[StFormat_NB] = {
        iconTexture(stCString("menuMono"),           anIconSize),
        iconTexture(stCString("menuSbsLR"),          anIconSize),
        iconTexture(stCString("menuSbsRL"),          anIconSize),
        iconTexture(stCString("menuOverUnderLR"),    anIconSize),
        iconTexture(stCString("menuOverUnderRL"),    anIconSize),
        iconTexture(stCString("menuRowLR"),          anIconSize),
        iconTexture(stCString("menuColLR"),          anIconSize),
        iconTexture(stCString("menuSrcSeparate"),    anIconSize),
        iconTexture(stCString("menuFrameSeqLR"),     anIconSize),
        iconTexture(stCString("menuRedCyanLR"),      anIconSize),
        iconTexture(stCString("menuGreenMagentaLR"), anIconSize),
        iconTexture(stCString("menuYellowBlueLR"),   anIconSize),
        iconTexture(stCString("menuTiledLR"),        anIconSize)
    };
    aSrcBtn->setTexturePath(aSrcTextures, StFormat_NB);
    aSrcBtn->setDrawShadow(true);
    myBtnSrcFrmt = aSrcBtn;

    myBtnSwapLR = new StGLCheckboxTextured(myPanelUpper, myImage->params.swapLR,
                                           iconTexture(stCString("actionSwapLROff"), anIconSize),
                                           iconTexture(stCString("actionSwapLROn"),  anIconSize),
                                           (aBtnIter++) * myIconStep, 0,
                                           StGLCorner(ST_VCORNER_TOP, ST_HCORNER_LEFT));
    myBtnSwapLR->setDrawShadow(true);
    myBtnSwapLR->changeMargins() = aButtonMargins;

    aBtnIter = 0;
    StGLTextureButton* aBtnEx = new StGLTextureButton(myPanelUpper, (aBtnIter--) * (-myIconStep), 0,
                                                      StGLCorner(ST_VCORNER_TOP, ST_HCORNER_RIGHT));
    aBtnEx->changeMargins() = aButtonMargins;
    aBtnEx->setTexturePath(iconTexture(stCString("actionOverflow"), anIconSize));
    aBtnEx->setDrawShadow(true);
    aBtnEx->signals.onBtnClick += stSlot(this, &StMoviePlayerGUI::doShowMobileExMenu);
}

/**
 * Create bottom toolbar
 */
void StMoviePlayerGUI::createMobileBottomToolbar() {
    myBottomBarNbLeft  = 0;
    myBottomBarNbRight = 0;

    StMarginsI aButtonMargins;
    const IconSize anIconSize = scaleIcon(32, aButtonMargins);
    aButtonMargins.extend(scale(12));
    const StMarginsI& aRootMargins = getRootMargins();
    myPanelBottom = new StGLContainer(this, aRootMargins.left, -aRootMargins.bottom, StGLCorner(ST_VCORNER_BOTTOM, ST_HCORNER_LEFT), scale(4096), scale(56));

    const StGLCorner aLeftCorner = StGLCorner(ST_VCORNER_TOP, ST_HCORNER_LEFT);
    myBtnPrev = new StGLTextureButton(myPanelBottom, (myBottomBarNbLeft++) * myIconStep, 0);
    myBtnPrev->signals.onBtnClick += stSlot(myPlugin, &StMoviePlayer::doListPrev);
    myBtnPrev->setTexturePath(iconTexture(stCString("actionVideoPrevious"), anIconSize));
    myBtnPrev->setDrawShadow(true);
    myBtnPrev->changeMargins() = aButtonMargins;

    myBtnPlay = new StGLTextureButton(myPanelBottom, (myBottomBarNbLeft++) * myIconStep, 0, aLeftCorner, 2);
    myBtnPlay->signals.onBtnClick += stSlot(myPlugin, &StMoviePlayer::doPlayPause);
    const StString aPaths[2] = {
        iconTexture(stCString("actionVideoPlay"),  anIconSize),
        iconTexture(stCString("actionVideoPause"), anIconSize)
    };
    myBtnPlay->setTexturePath(aPaths, 2);
    myBtnPlay->setDrawShadow(true);
    myBtnPlay->changeMargins() = aButtonMargins;

    myBtnNext = new StGLTextureButton(myPanelBottom, (myBottomBarNbLeft++) * myIconStep, 0);
    myBtnNext->signals.onBtnClick += stSlot(myPlugin, &StMoviePlayer::doListNext);
    myBtnNext->setTexturePath(iconTexture(stCString("actionVideoNext"), anIconSize));
    myBtnNext->setDrawShadow(true);
    myBtnNext->changeMargins() = aButtonMargins;

    const StGLCorner aRightCorner = StGLCorner(ST_VCORNER_TOP, ST_HCORNER_RIGHT);
    myBtnList = new StGLTextureButton(myPanelBottom, (myBottomBarNbRight++) * (-myIconStep), 0, aRightCorner);
    myBtnList->signals.onBtnClick += stSlot(myPlugin, &StMoviePlayer::doPlayListReverse);
    myBtnList->setTexturePath(iconTexture(stCString("actionVideoPlaylist"), anIconSize));
    myBtnList->setDrawShadow(true);
    myBtnList->changeMargins() = aButtonMargins;

    StGLTextureButton* aBtnInfo = new StGLTextureButton(myPanelBottom, (myBottomBarNbRight++) * (-myIconStep), 0, aRightCorner);
    aBtnInfo->signals.onBtnClick += stSlot(myPlugin, &StMoviePlayer::doAboutFile);
    aBtnInfo->setTexturePath(iconTexture(stCString("actionInfo"),  anIconSize));
    aBtnInfo->setDrawShadow(true);
    aBtnInfo->changeMargins() = aButtonMargins;

    mySeekBar = new StSeekBar(myPanelBottom, 0, scale(18));
    mySeekBar->signals.onSeekClick.connect(myPlugin, &StMoviePlayer::doSeek);

    myTimeBox = new StTimeBox(myPanelBottom, myBottomBarNbRight * (-myIconStep), 0, aRightCorner, StGLTextArea::SIZE_SMALL);
    myTimeBox->setSwitchOnClick(true);
    myTimeBox->changeRectPx().right()  = myTimeBox->getRectPx().left() + myIconStep * 2;
    myTimeBox->changeRectPx().bottom() = myTimeBox->getRectPx().top()  + scale(56);
}

void StMoviePlayerGUI::doShowMobileExMenu(const size_t ) {
    const IconSize anIconSize = scaleIcon(16);
    const int aTop = scale(56);

    StHandle<StMovieInfo>&   anExtraInfo = myPlugin->myFileInfo;
    StHandle<StFileNode>     aFileNode;
    StHandle<StStereoParams> aParams;
    if(anExtraInfo.isNull()
    && !myPlugin->getCurrentFile(aFileNode, aParams, anExtraInfo)) {
        anExtraInfo.nullify();
    }

    StGLMenu*     aMenu  = new StGLMenu(this, 0, aTop, StGLMenu::MENU_VERTICAL_COMPACT, true);
    StGLMenuItem* anItem = NULL;
    aMenu->setCorner(StGLCorner(ST_VCORNER_TOP, ST_HCORNER_RIGHT));
    aMenu->setContextual(true);
    if(!anExtraInfo.isNull()) {
        anItem = aMenu->addItem(tr(BUTTON_DELETE), myPlugin->getAction(StMoviePlayer::Action_DeleteFile));
        anItem->setIcon(iconTexture(stCString("actionDiscard"), anIconSize));

        const StHandle< StArrayList<StString> >& anAudioStreams = myPlugin->myVideo->params.activeAudio->getList();
        if(!anAudioStreams.isNull()
        && !anAudioStreams->isEmpty()) {
            anItem = aMenu->addItem(tr(MENU_AUDIO));
            anItem->setIcon(iconTexture(stCString("actionStreamAudio"), anIconSize));
            anItem->signals.onItemClick += stSlot(this, &StMoviePlayerGUI::doAudioStreamsCombo);
        }

        const StHandle< StArrayList<StString> >& aSubsStreams = myPlugin->myVideo->params.activeSubtitles->getList();
        if(!aSubsStreams.isNull()
        && !aSubsStreams->isEmpty()) {
            anItem = aMenu->addItem(tr(MENU_SUBTITLES));
            anItem->setIcon(iconTexture(stCString("actionStreamSubtitles"), anIconSize));
            anItem->signals.onItemClick += stSlot(this, &StMoviePlayerGUI::doSubtitlesStreamsCombo);
        }

        if(myPlugin->myVideo->hasVideoStream()) {
            anItem = aMenu->addItem(tr(MENU_VIEW_DISPLAY_RATIO));
            anItem->setIcon(iconTexture(stCString("actionDisplayRatio"), anIconSize));
            anItem->signals.onItemClick += stSlot(this, &StMoviePlayerGUI::doDisplayRatioCombo);
        }

        anExtraInfo.nullify();
    }
    anItem = aMenu->addItem(tr(MENU_HELP_ABOUT));
    anItem->setIcon(iconTexture(stCString("actionHelp"),      anIconSize));
    anItem->signals.onItemClick += stSlot(this, &StMoviePlayerGUI::doAboutProgram);
    //anItem = aMenu->addItem(myPlugin->StApplication::params.ActiveDevice->getActiveValue());
    anItem = aMenu->addItem("Settings");
    anItem->setIcon(iconTexture(stCString("actionSettings"),  anIconSize));
    anItem->signals.onItemClick += stSlot(this, &StMoviePlayerGUI::doMobileSettings);
    aMenu->stglInit();
}

StMoviePlayerGUI::StMoviePlayerGUI(StMoviePlayer*  thePlugin,
                                   StWindow*       theWindow,
                                   StTranslations* theLangMap,
                                   const StHandle<StPlayList>&       thePlayList,
                                   const StHandle<StGLTextureQueue>& theTextureQueue,
                                   const StHandle<StSubQueue>&       theSubQueue)
: StGLRootWidget(thePlugin->myResMgr),
  myPlugin(thePlugin),
  myWindow(theWindow),
  myLangMap(theLangMap),
  myVisibilityTimer(true),
  //
  myImage(NULL),
  mySubtitles(NULL),
  myDescr(NULL),
  myMsgStack(NULL),
  myPlayList(NULL),
  // main menu
  myMenuRoot(NULL),
  myMenuOpenAL(NULL),
  myMenuRecent(NULL),
  myMenuAudio(NULL),
  myMenuSubtitles(NULL),
  // upper toolbar
  myPanelUpper(NULL),
  myBtnOpen(NULL),
  myBtnInfo(NULL),
  myBtnSwapLR(NULL),
  myBtnSrcFrmt(NULL),
  // bottom toolbar
  myPanelBottom(NULL),
  mySeekBar(NULL),
  myBtnPlay(NULL),
  myTimeBox(NULL),
  myBtnPrev(NULL),
  myBtnNext(NULL),
  myBtnList(NULL),
  myBtnFullScr(NULL),
  //
  myFpsWidget(NULL),
  myHKeysTable(NULL),
  //
  myIsVisibleGUI(true),
  myIsExperimental(myPlugin->params.ToShowExtra->getValue()),
  myIconStep(64),
  myBottomBarNbLeft(0),
  myBottomBarNbRight(0) {
    const GLfloat aScale = myPlugin->params.ScaleHiDPI2X->getValue() ? 2.0f : myPlugin->params.ScaleHiDPI ->getValue();
    setScale(aScale, (StGLRootWidget::ScaleAdjust )myPlugin->params.ScaleAdjust->getValue());
    setMobile(myPlugin->params.IsMobileUI->getValue());

    myIconStep = isMobile() ? scale(56) : scale(64);

    changeRootMargins() = myWindow->getMargins();
    myPlugin->params.ToShowFps->signals.onChanged.connect(this, &StMoviePlayerGUI::doShowFPS);

    myImage = new StGLImageRegion(this, theTextureQueue, false);
    myImage->setDragDelayMs(500.0);
    myImage->params.displayMode->setName(tr(MENU_VIEW_DISPLAY_MODE));
    myImage->params.displayMode->changeValues()[StGLImageRegion::MODE_STEREO]     = tr(MENU_VIEW_DISPLAY_MODE_STEREO);
    myImage->params.displayMode->changeValues()[StGLImageRegion::MODE_ONLY_LEFT]  = tr(MENU_VIEW_DISPLAY_MODE_LEFT);
    myImage->params.displayMode->changeValues()[StGLImageRegion::MODE_ONLY_RIGHT] = tr(MENU_VIEW_DISPLAY_MODE_RIGHT);
    myImage->params.displayMode->changeValues()[StGLImageRegion::MODE_PARALLEL]   = tr(MENU_VIEW_DISPLAY_MODE_PARALLEL);
    myImage->params.displayMode->changeValues()[StGLImageRegion::MODE_CROSSYED]   = tr(MENU_VIEW_DISPLAY_MODE_CROSSYED);

    mySubtitles = new StGLSubtitles  (this, theSubQueue,
                                      myPlugin->params.SubtitlesSize,
                                      myPlugin->params.SubtitlesParallax,
                                      myPlugin->params.SubtitlesParser);

    if(myPlugin->params.ToShowFps->getValue()) {
        myFpsWidget = new StGLFpsLabel(this);
    }

    if(isMobile()) {
        createMobileUI(thePlayList);
    } else {
        createDesktopUI(thePlayList);
    }

    myMsgStack = new StGLMsgStack(this, myPlugin->getMessagesQueue());
}

StMoviePlayerGUI::~StMoviePlayerGUI() {
    //
}

void StMoviePlayerGUI::stglUpdate(const StPointD_t& thePointZo,
                                  const GLfloat theProgress,
                                  const double thePTS) {
    StGLRootWidget::stglUpdate(thePointZo);
    if(mySubtitles != NULL) {
        mySubtitles->setPTS(thePTS);
    }
    if(mySeekBar != NULL) {
        mySeekBar->setProgress(theProgress);
    }
    if(myDescr != NULL) {
        myDescr->setPoint(thePointZo);
    }

    if(myLangMap->wasReloaded()) {
        myPlugin->myToRecreateMenu = true;
        myLangMap->resetReloaded();
        StMoviePlayerStrings::loadDefaults(*myLangMap);
    } else if(myIsExperimental != myPlugin->params.ToShowExtra->getValue()) {
        StGLMenu::DeleteWithSubMenus(myMenuRoot); myMenuRoot = NULL;
        createMainMenu();
        myMenuRoot->stglUpdateSubmenuLayout();
        myLangMap->resetReloaded();
        myIsExperimental = myPlugin->params.ToShowExtra->getValue();
        // turn back topmost position
        getChildren()->moveToTop(myMsgStack);
    }
}

void StMoviePlayerGUI::stglResize(const StGLBoxPx& theRectPx) {
    myImage->changeRectPx().bottom() = theRectPx.height();
    myImage->changeRectPx().right()  = theRectPx.width();

    const StMarginsI& aMargins = myWindow->getMargins();
    const bool areNewMargins = aMargins != getRootMargins();
    if(areNewMargins) {
        changeRootMargins() = aMargins;
    }

    if(myPanelUpper != NULL) {
        myPanelUpper->changeRectPx().right()  = stMax(theRectPx.width() - aMargins.right, 2);
    }
    if(myPanelBottom != NULL) {
        myPanelBottom->changeRectPx().right() = stMax(theRectPx.width() - aMargins.right, 2);
    }

    if(areNewMargins) {
        if(myPanelUpper != NULL) {
            myPanelUpper->changeRectPx().left() = aMargins.left;
            myPanelUpper->changeRectPx().moveTopTo(aMargins.top);
        }
        if(myPanelBottom != NULL) {
            myPanelBottom->changeRectPx().left() =  aMargins.left;
            myPanelBottom->changeRectPx().moveTopTo(-aMargins.bottom);
        }
        if(myMenuRoot != NULL) {
            myMenuRoot->changeRectPx().left() = aMargins.left;
            myMenuRoot->changeRectPx().top()  = aMargins.top;
            myMenuRoot->stglUpdateSubmenuLayout();
        }
    }

    if(mySeekBar != NULL
    && myPanelBottom != NULL) {
        const int aPanelSizeY = myPanelBottom->getRectPx().height();
        const int aSeekSizeY  = mySeekBar->getRectPx().height();
        if(isMobile()) {
            const int anXOffset = scale(24);
            const int anXSpace  = theRectPx.width() - (myBottomBarNbLeft + myBottomBarNbRight) * myIconStep;
            const int anXSpace2 = anXSpace - myBottomBarNbRight * myIconStep * 2;
            const int aBoxWidth = myTimeBox->getRectPx().width();

            if(anXSpace >= scale(250)) {
                mySeekBar->changeRectPx().moveTopTo((aPanelSizeY - aSeekSizeY) / 2);
                mySeekBar->changeRectPx().left()  = anXOffset + myBottomBarNbLeft * myIconStep;
                mySeekBar->changeRectPx().right() = theRectPx.width() - anXOffset - myBottomBarNbRight * myIconStep;
                if(anXSpace2 >= scale(250)) {
                    mySeekBar->changeRectPx().right() -= myIconStep * 2;
                    myTimeBox->changeRectPx().moveTopTo(0);
                    myTimeBox->changeRectPx().moveLeftTo(myBottomBarNbRight * (-myIconStep));
                    myTimeBox->setOverlay(false);
                } else {
                    myTimeBox->changeRectPx().moveTopTo(0);
                    myTimeBox->changeRectPx().moveLeftTo(myBottomBarNbRight * (-myIconStep) - anXSpace / 2 + aBoxWidth / 2);
                    myTimeBox->setOverlay(true);
                }
            } else {
                mySeekBar->changeRectPx().moveTopTo(-aPanelSizeY + (aPanelSizeY - aSeekSizeY) / 2);
                mySeekBar->changeRectPx().left()  = anXOffset;
                mySeekBar->changeRectPx().right() = theRectPx.width() - anXOffset;
                myTimeBox->changeRectPx().moveTopTo(-aPanelSizeY);
                myTimeBox->changeRectPx().moveLeftTo(-theRectPx.width() / 2 + aBoxWidth / 2);
                myTimeBox->setOverlay(true);
            }
        } else {
            const int anXOffset = scale(64);
            mySeekBar->changeRectPx().moveTopTo(scale(34));
            mySeekBar->changeRectPx().left()  = anXOffset;
            mySeekBar->changeRectPx().right() = theRectPx.width() - anXOffset;
        }
    }

    StGLRootWidget::stglResize(theRectPx);
}

bool StMoviePlayerGUI::toHideCursor() {
    if(myPanelBottom == NULL) {
        return false;
    }
    StGLWidget* child = myPanelBottom->getChildren()->getStart();
    return child != NULL && !child->isVisible();
}

size_t StMoviePlayerGUI::trSrcFormatId(const StFormat theSrcFormat) {
    switch(theSrcFormat) {
        case StFormat_Mono:                 return MENU_SRC_FORMAT_MONO;
        case StFormat_SideBySide_LR:        return MENU_SRC_FORMAT_PARALLEL;
        case StFormat_SideBySide_RL:        return MENU_SRC_FORMAT_CROSS_EYED;
        case StFormat_TopBottom_LR:         return MENU_SRC_FORMAT_OVERUNDER_LR;
        case StFormat_TopBottom_RL:         return MENU_SRC_FORMAT_OVERUNDER_RL;
        case StFormat_Rows:                 return MENU_SRC_FORMAT_INTERLACED;
        //case StFormat_Columns:
        case StFormat_SeparateFrames:       return MENU_SRC_FORMAT_SEPARATE;
        case StFormat_FrameSequence:        return MENU_SRC_FORMAT_PAGEFLIP;
        case StFormat_AnaglyphRedCyan:      return MENU_SRC_FORMAT_ANA_RC;
        case StFormat_AnaglyphGreenMagenta: return MENU_SRC_FORMAT_ANA_RB;
        case StFormat_AnaglyphYellowBlue:   return MENU_SRC_FORMAT_ANA_YB;
        case StFormat_Tiled4x:              return MENU_SRC_FORMAT_TILED_4X;
        default:
        case StFormat_AUTO:                 return MENU_SRC_FORMAT_AUTO;
    }
}

namespace {

    inline bool isPointIn(const StGLWidget* theWidget,
                          const StPointD_t& theCursorZo) {
        return theWidget != NULL
            && theWidget->isVisible()
            && theWidget->isPointIn(theCursorZo);
    }

}

void StMoviePlayerGUI::setVisibility(const StPointD_t& theCursor,
                                     bool              theIsMouseMoved) {
    const bool toShowPlayList = myPlugin->params.ToShowPlayList->getValue();
    myIsVisibleGUI = theIsMouseMoved
        || myVisibilityTimer.getElapsedTime() < 2.0
        || (myPanelUpper  != NULL && myPanelUpper ->isPointIn(theCursor))
        || (myPanelBottom != NULL && myPanelBottom->isPointIn(theCursor))
        || (mySeekBar     != NULL && mySeekBar    ->isPointIn(theCursor))
        || (myPlayList    != NULL && toShowPlayList && myPlayList->isPointIn(theCursor))
        || (myMenuRoot    != NULL && myMenuRoot->isActive());
    const float anOpacity = (float )myVisLerp.perform(myIsVisibleGUI, false);
    if(theIsMouseMoved) {
        myVisibilityTimer.restart();
    }

    if(myMenuRoot != NULL) {
        myMenuRoot->setOpacity(anOpacity, false);
    }
    if(mySeekBar != NULL) {
        mySeekBar->setOpacity(anOpacity, false);
    }
    if(myPanelUpper != NULL) {
        myPanelUpper->setOpacity(anOpacity, true);
    }
    if(myPanelBottom != NULL) {
        myPanelBottom->setOpacity(anOpacity, true);
    }

    if(myPlayList != NULL
    && toShowPlayList) {
        myPlayList->setOpacity(anOpacity, true);
    }

    StFormat aSrcFormat = (StFormat )myPlugin->params.srcFormat->getValue();
    if(aSrcFormat == StFormat_AUTO
    && !myImage->params.stereoFile.isNull()) {
        aSrcFormat = myImage->params.stereoFile->StereoFormat;
    }
    if(!myImage->params.stereoFile.isNull()
     && myImage->params.swapLR->getValue()) {
        aSrcFormat = st::formatReversed(aSrcFormat);
    }
    if(myBtnSrcFrmt != NULL) {
        myBtnSrcFrmt->setFaceId(aSrcFormat != StFormat_AUTO ? aSrcFormat : StFormat_Mono);
    }
    if(myBtnSwapLR != NULL) {
        myBtnSwapLR->setOpacity(aSrcFormat != StFormat_Mono ? anOpacity : 0.0f, false);
    }

    if(myDescr != NULL) {
        myDescr->setOpacity(1.0f, false);
        if(::isPointIn(myBtnOpen, theCursor)) {
            myDescr->setText(tr(FILE_VIDEO_OPEN));
        } else if(::isPointIn(myBtnInfo,   theCursor)) {
            myDescr->setText(tr(MENU_MEDIA_FILE_INFO));
        } else if(::isPointIn(myBtnSwapLR, theCursor)) {
            size_t aLngId = myImage->params.swapLR->getValue() ? SWAP_LR_ON : SWAP_LR_OFF;
            myDescr->setText(tr(aLngId));
        } else if(::isPointIn(myBtnSrcFrmt, theCursor)) {
            myDescr->setText(tr(BTN_SRC_FORMAT) + "\n" + trSrcFormat(aSrcFormat));
        } else if(::isPointIn(myBtnPlay, theCursor)) {
            myDescr->setText(tr(VIDEO_PLAYPAUSE));
        } else if(::isPointIn(myBtnPrev, theCursor)) {
            myDescr->setText(tr(VIDEO_LIST_PREV));
        } else if(::isPointIn(myBtnNext, theCursor)) {
            myDescr->setText(tr(VIDEO_LIST_NEXT));
        } else if(::isPointIn(myBtnList, theCursor)) {
            myDescr->setText(tr(VIDEO_LIST));
        } else if(::isPointIn(myBtnFullScr, theCursor)) {
            myDescr->setText(tr(FULLSCREEN));
        } else {
            myDescr->setOpacity(0.0f, false);
        }
    }
}

void StMoviePlayerGUI::doAudioStreamsCombo(const size_t ) {
    const StHandle< StArrayList<StString> >& aStreams = myPlugin->myVideo->params.activeAudio->getList();
    const bool hasVideo = myPlugin->myVideo->hasVideoStream();

    StGLCombobox::ListBuilder aBuilder(this);
    if(hasVideo || aStreams.isNull() || aStreams->isEmpty()) {
        aBuilder.getMenu()->addItem(tr(MENU_AUDIO_NONE), myPlugin->params.audioStream, -1);
    }
    if(!aStreams.isNull()) {
        for(size_t aStreamId = 0; aStreamId < aStreams->size(); ++aStreamId) {
            aBuilder.getMenu()->addItem(aStreams->getValue(aStreamId), myPlugin->params.audioStream, int32_t(aStreamId));
        }
    }
    aBuilder.display();
}

void StMoviePlayerGUI::updateAudioStreamsMenu(const StHandle< StArrayList<StString> >& theStreamsList,
                                              const bool theHasVideo) {
    if(myMenuAudio == NULL) {
        return;
    }
    for(StGLWidget* aChild = myMenuAudio->getChildren()->getStart(); aChild != NULL;) {
        StGLWidget* anItem = aChild;
        aChild = aChild->getNext();
        delete anItem;
    }

    if(theHasVideo || theStreamsList.isNull() || theStreamsList->isEmpty()) {
        myMenuAudio->addItem(tr(MENU_AUDIO_NONE), myPlugin->params.audioStream, -1);
    }
    if(!theStreamsList.isNull()) {
        for(size_t aStreamId = 0; aStreamId < theStreamsList->size(); ++aStreamId) {
            myMenuAudio->addItem(theStreamsList->getValue(aStreamId), myPlugin->params.audioStream, int32_t(aStreamId));
        }
    }

    //myMenuAudio->addSplitter();
    StGLMenuItem* aDelayItem = NULL;
    StGLRangeFieldFloat32* aDelayRange = NULL;
    if(theHasVideo) {
        if(!theStreamsList.isNull()
        && !theStreamsList->isEmpty()) {
            aDelayItem = myMenuAudio->addItem(tr(MENU_AUDIO_DELAY));
            aDelayItem->changeMargins().right = scale(100 + 16);
            aDelayItem->signals.onItemClick.connect(this, &StMoviePlayerGUI::doAudioDelay);
            aDelayRange = new StGLRangeFieldFloat32(aDelayItem, myPlugin->params.AudioDelay,
                                                    -scale(16), 0, StGLCorner(ST_VCORNER_CENTER, ST_HCORNER_RIGHT));
            aDelayRange->changeRectPx().bottom() = aDelayRange->getRectPx().top()  + myMenuAudio->getItemHeight();
            aDelayRange->setFormat(stCString("%+01.3f"));
            aDelayRange->setColor(StGLRangeFieldFloat32::FieldColor_Default,  StGLVec3(0.0f, 0.0f, 0.0f));
            aDelayRange->setColor(StGLRangeFieldFloat32::FieldColor_Positive, StGLVec3(0.4f, 0.8f, 0.4f));
            aDelayRange->setColor(StGLRangeFieldFloat32::FieldColor_Negative, StGLVec3(1.0f, 0.0f, 0.0f));
        }
        myMenuAudio->addItem(tr(MENU_AUDIO_ATTACH))
                   ->signals.onItemClick.connect(myPlugin, &StMoviePlayer::doAddAudioStream);
    }

    // update menu representation
    myMenuAudio->stglInit();
}

void StMoviePlayerGUI::doSubtitlesStreamsCombo(const size_t ) {
    const StHandle< StArrayList<StString> >& aStreams = myPlugin->myVideo->params.activeSubtitles->getList();

    StGLCombobox::ListBuilder aBuilder(this);
    aBuilder.getMenu()->addItem(tr(MENU_SUBTITLES_NONE), myPlugin->params.subtitlesStream, -1);
    if(!aStreams.isNull()) {
        for(size_t aStreamId = 0; aStreamId < aStreams->size(); ++aStreamId) {
            aBuilder.getMenu()->addItem(aStreams->getValue(aStreamId), myPlugin->params.subtitlesStream, int32_t(aStreamId));
        }
    }
    aBuilder.display();
}

void StMoviePlayerGUI::updateSubtitlesStreamsMenu(const StHandle< StArrayList<StString> >& theStreamsList,
                                                  const bool theIsFilePlayed) {
    if(myMenuSubtitles == NULL) {
        return;
    }
    for(StGLWidget* aChild = myMenuSubtitles->getChildren()->getStart(); aChild != NULL;) {
        StGLWidget* anItem = aChild;
        aChild = aChild->getNext();
        delete anItem;
    }

    // create text parser menu
    StGLMenu* aParserMenu = new StGLMenu(this, 0, 0, StGLMenu::MENU_VERTICAL);
    for(size_t anIter = 0; anIter < myPlugin->params.SubtitlesParser->getValues().size(); ++anIter) {
        aParserMenu->addItem(myPlugin->params.SubtitlesParser->getValues()[anIter], myPlugin->params.SubtitlesParser, (int32_t )anIter);
    }
    aParserMenu->stglInit();

    myMenuSubtitles->addItem(tr(MENU_SUBTITLES_NONE), myPlugin->params.subtitlesStream, -1);
    if(!theStreamsList.isNull()) {
        for(size_t aStreamId = 0; aStreamId < theStreamsList->size(); ++aStreamId) {
            myMenuSubtitles->addItem(theStreamsList->getValue(aStreamId), myPlugin->params.subtitlesStream, int32_t(aStreamId));
        }
    }

    if(!theStreamsList.isNull()
    && !theStreamsList->isEmpty()) {
        //myMenuSubtitles->addSplitter();
        StGLMenuItem* anItem = myMenuSubtitles->addItem(tr(MENU_SUBTITLES_SIZE));
        anItem->changeMargins().right = scale(100 + 16);
        StGLRangeFieldFloat32* aRange = new StGLRangeFieldFloat32(anItem, myPlugin->params.SubtitlesSize,
                                                                 -scale(16), 0, StGLCorner(ST_VCORNER_CENTER, ST_HCORNER_RIGHT));
        aRange->changeRectPx().bottom() = aRange->getRectPx().top() + myMenuSubtitles->getItemHeight();
        aRange->setFormat(stCString("%02.0f"));
        aRange->setColor(StGLRangeFieldFloat32::FieldColor_Default,  aBlack);
        aRange->setColor(StGLRangeFieldFloat32::FieldColor_Positive, aBlack);
        aRange->setColor(StGLRangeFieldFloat32::FieldColor_Negative, aBlack);

        anItem = myMenuSubtitles->addItem(tr(MENU_SUBTITLES_PARALLAX));
        anItem->changeMargins().right  = scale(100 + 16);
        aRange = new StGLRangeFieldFloat32(anItem, myPlugin->params.SubtitlesParallax,
                                          -scale(16), 0, StGLCorner(ST_VCORNER_CENTER, ST_HCORNER_RIGHT));
        aRange->changeRectPx().bottom() = aRange->getRectPx().top() + myMenuSubtitles->getItemHeight();
        aRange->setFormat(stCString("%+03.0f"));
        aRange->setColor(StGLRangeFieldFloat32::FieldColor_Default,  aBlack);
        aRange->setColor(StGLRangeFieldFloat32::FieldColor_Positive, aBlack);
        aRange->setColor(StGLRangeFieldFloat32::FieldColor_Negative, aBlack);
    }

    myMenuSubtitles->addItem(tr(MENU_SUBTITLES_PARSER), aParserMenu);
    if(theIsFilePlayed) {
        //myMenuSubtitles->addSplitter();
        myMenuSubtitles->addItem(tr(MENU_SUBTITLES_ATTACH))
                       ->signals.onItemClick.connect(myPlugin, &StMoviePlayer::doAddSubtitleStream);
    }

    // update menu representation
    myMenuSubtitles->stglInit();
}

void StMoviePlayerGUI::stglDraw(unsigned int theView) {
    setLensDist(myPlugin->getMainWindow()->getLensDist());
    if((theView == ST_DRAW_LEFT || theView == ST_DRAW_MONO)
    && myFpsWidget != NULL) {
        myImage->getTextureQueue()->getQueueInfo(myFpsWidget->changePlayQueued(),
                                                 myFpsWidget->changePlayQueueLength(),
                                                 myFpsWidget->changePlayFps());
        myFpsWidget->update(myPlugin->getMainWindow()->isStereoOutput(),
                            myPlugin->getMainWindow()->getTargetFps());
    }
    StGLRootWidget::stglDraw(theView);
}

void StMoviePlayerGUI::doShowFPS(const bool ) {
    if(myFpsWidget != NULL) {
        delete myFpsWidget;
        myFpsWidget = NULL;
        return;
    }

    myFpsWidget = new StGLFpsLabel(this);
    myFpsWidget->stglInit();
}

void StMoviePlayerGUI::doAboutRenderer(const size_t ) {
    StString anAboutText = myPlugin->getMainWindow()->getRendererAbout();
    if(anAboutText.isEmpty()) {
        anAboutText = StString() + "Plugin '" + myPlugin->getMainWindow()->getRendererId() + "' doesn't provide description";
    }

    StGLMessageBox* aDialog = new StGLMessageBox(this, "", anAboutText, scale(512), scale(300));
    aDialog->addButton(tr(BUTTON_CLOSE));
    aDialog->stglInit();
}

void StMoviePlayerGUI::showUpdatesNotify() {
    StGLMessageBox* aDialog = new StGLMessageBox(this, "", tr(UPDATES_NOTIFY));
    aDialog->addButton(tr(BUTTON_CLOSE));
    aDialog->stglInit();
}

/**
 * Widget to assign new hot key.
 */
class ST_LOCAL StHotKeyControl : public StGLAssignHotKey {

        public:

    /**
     * Main constructor.
     */
    ST_LOCAL StHotKeyControl(StApplication*            thePlugin,
                             StGLTable*                theHKeysTable,
                             StMoviePlayerGUI*         theParent,
                             const StHandle<StAction>& theAction,
                             const int                 theHKeyIndex)
    : StGLAssignHotKey(theParent, theAction, theHKeyIndex),
      myPlugin(thePlugin),
      myHKeysTable(theHKeysTable) {
        myTitleFrmt    = theParent->tr(DIALOG_ASSIGN_HOT_KEY).format(theParent->tr(theAction->getName()));
        myConflictFrmt = theParent->tr(DIALOG_CONFLICTS_WITH);
        myAssignLab    = theParent->tr(BUTTON_ASSIGN);
        myDefaultLab   = theParent->tr(BUTTON_DEFAULT);
        myCancelLab    = theParent->tr(BUTTON_CANCEL);
        create();
    }

    /**
     * Destructor.
     */
    ST_LOCAL virtual ~StHotKeyControl() {
        myPlugin->registerHotKeys();
        myHKeysTable->updateHotKeys(myPlugin->getActions());
    }

    /**
     * Retrieve action for specified hot key.
     */
    ST_LOCAL virtual StHandle<StAction> getActionForKey(unsigned int theHKey) const {
        return myPlugin->getActionForKey(theHKey);
    }

        private:

    StApplication* myPlugin;
    StGLTable*     myHKeysTable;

};

void StMoviePlayerGUI::doResetHotKeys(const size_t ) {
    if(myHKeysTable == NULL) {
        return;
    }

    for(std::map< int, StHandle<StAction> >::iterator anActionIter = myPlugin->changeActions().begin();
        anActionIter != myPlugin->changeActions().end(); ++anActionIter) {
        StHandle<StAction>& anAction = anActionIter->second;
        anAction->setHotKey1(anAction->getDefaultHotKey1());
        anAction->setHotKey2(anAction->getDefaultHotKey2());
    }
    myPlugin->registerHotKeys();
    myHKeysTable->updateHotKeys(myPlugin->getActions());
}

void StMoviePlayerGUI::doListHotKeys(const size_t ) {
    const StHandle<StWindow>& aRend = myPlugin->getMainWindow();
    StParamsList aParams;
    aParams.add(myPlugin->StApplication::params.ActiveDevice);
    aParams.add(myImage->params.displayMode);
    aRend->getOptions(aParams);
    aParams.add(myPlugin->params.ToShowFps);
    aParams.add(myLangMap->params.language);
    aParams.add(myPlugin->params.IsMobileUI);
    myLangMap->params.language->setName(tr(MENU_HELP_LANGS));

    const StString aTitle  = tr(MENU_HELP_HOTKEYS);
    StInfoDialog*  aDialog = new StInfoDialog(myPlugin, this, aTitle, scale(650), scale(300));

    std::map< int, StHandle<StAction> >& anActionsMap = myPlugin->changeActions();

    StGLTable* aTable = new StGLTable(aDialog->getContent(), 0, 0, StGLCorner(ST_VCORNER_TOP, ST_HCORNER_CENTER));
    aTable->changeItemMargins().top    = scale(4);
    aTable->changeItemMargins().bottom = scale(4);
    aTable->setupTable(anActionsMap.size(), 3);

    StHandle< StSlot<void (const size_t )> > aSlot1 = new StSlotMethod<StMoviePlayerGUI, void (const size_t )>(this, &StMoviePlayerGUI::doChangeHotKey1);
    StHandle< StSlot<void (const size_t )> > aSlot2 = new StSlotMethod<StMoviePlayerGUI, void (const size_t )>(this, &StMoviePlayerGUI::doChangeHotKey2);
    aTable->fillFromHotKeys(anActionsMap, *myLangMap, aSlot1, aSlot2);
    myHKeysTable = aTable;

    aDialog->addButton(tr(BUTTON_DEFAULTS), false)->signals.onBtnClick = stSlot(this, &StMoviePlayerGUI::doResetHotKeys);
    aDialog->addButton(tr(BUTTON_CLOSE), true);
    aDialog->stglInit();
}

void StMoviePlayerGUI::doChangeHotKey1(const size_t theId) {
    const StHandle<StAction>& anAction = myPlugin->getAction(theId);
    StHotKeyControl* aKeyChanger = new StHotKeyControl(myPlugin, myHKeysTable, this, anAction, 1);
    aKeyChanger->stglInit();
}

void StMoviePlayerGUI::doChangeHotKey2(const size_t theId) {
    const StHandle<StAction>& anAction = myPlugin->getAction(theId);
    StHotKeyControl* aKeyChanger = new StHotKeyControl(myPlugin, myHKeysTable, this, anAction, 2);
    aKeyChanger->stglInit();
}

void StMoviePlayerGUI::doMobileSettings(const size_t ) {
    const StHandle<StWindow>& aRend = myPlugin->getMainWindow();
    StParamsList aParams;
    aParams.add(myPlugin->StApplication::params.ActiveDevice);
    aParams.add(myImage->params.displayMode);
    aRend->getOptions(aParams);
    aParams.add(myPlugin->params.ToShowFps);
    aParams.add(myLangMap->params.language);
    aParams.add(myPlugin->params.IsMobileUI);
    myLangMap->params.language->setName(tr(MENU_HELP_LANGS));

    const StString aTitle  = "Settings";
    StInfoDialog*  aDialog = new StInfoDialog(myPlugin, this, aTitle, scale(512), scale(300));

    const int aWidthMax  = aDialog->getContent()->getRectPx().width();
    int       aRowLast   = (int )aParams.size();
    const int aNbRowsMax = aRowLast + 2;

    StGLTable* aTable = new StGLTable(aDialog->getContent(), 0, 0, StGLCorner(ST_VCORNER_TOP, ST_HCORNER_CENTER));
    aTable->changeItemMargins().top    = scale(4);
    aTable->changeItemMargins().bottom = scale(4);
    aTable->setupTable(aNbRowsMax, 2);
    aTable->fillFromParams(aParams, StGLVec3(1.0f, 1.0f, 1.0f), aWidthMax);

    aDialog->addButton(tr(BUTTON_CLOSE), true);
    aDialog->stglInit();
}
