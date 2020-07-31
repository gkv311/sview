/**
 * Copyright © 2009-2020 Kirill Gavrilov <kirill@sview.ru>
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

#include "StALDeviceParam.h"
#include "StMoviePlayer.h"
#include "StTimeBox.h"

#include "StVideo/StALContext.h"
#include "StVideo/StVideo.h"
#include "StMovieOpenDialog.h"

#include <StCore/StSearchMonitors.h>
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
#include <StGLWidgets/StGLOpenFile.h>
#include <StGLWidgets/StGLPlayList.h>
#include <StGLWidgets/StGLRangeFieldFloat32.h>
#include <StGLWidgets/StGLSeekBar.h>
#include <StGLWidgets/StGLScrollArea.h>
#include <StGLWidgets/StGLSubtitles.h>
#include <StGLWidgets/StGLSwitchTextured.h>
#include <StGLWidgets/StGLTable.h>
#include <StGLWidgets/StGLTextureButton.h>
#include <StVersion.h>

#include "StMoviePlayerStrings.h"

// auxiliary pre-processor definition
#define stCTexture(theString) getTexturePath(stCString(theString))
#define stCMenuIcon(theString) iconTexture(stCString(theString), myMenuIconSize)

using namespace StMoviePlayerStrings;

namespace {

    static const float THE_VISIBILITY_IDLE_TIME = 2.0f;

    static const int DISPL_Y_REGION_UPPER  = 32;
    static const int DISPL_X_REGION_UPPER  = 32;
    static const int DISPL_X_REGION_BOTTOM = 52;

    static const StGLVec3 aBlack (0.0f, 0.0f, 0.0f);
    static const StGLVec3 aGreen (0.0f, 0.6f, 0.4f);
    static const StGLVec3 aRed   (1.0f, 0.0f, 0.0f);

}

void StMoviePlayerGUI::createDesktopUI(const StHandle<StPlayList>& thePlayList) {
    createImageAdjustments();
    createUpperToolbar();
    createBottomToolbar(64, 32);

    mySeekBar = new StGLSeekBar(myPanelBottom, 0, scale(18));
    mySeekBar->setMoveTolerance(scale(isMobile() ? 16 : 8));
    mySeekBar->signals.onSeekClick.connect(myPlugin, &StMoviePlayer::doSeek);

    myTimeBox = new StTimeBox(myPanelBottom, myBottomBarNbLeft * myIconStep, 0,
                              StGLCorner(ST_VCORNER_TOP, ST_HCORNER_RIGHT));
    myTimeBox->setSwitchOnClick(true);
    myTimeBox->changeRectPx().right()  = myTimeBox->getRectPx().left() + scale(128);
    myTimeBox->changeRectPx().bottom() = myTimeBox->getRectPx().top()  + scale(64);

    myDescr = new StGLDescription(this);

    myPlayList = new StGLPlayList(this, thePlayList);
    myPlayList->setCorner(StGLCorner(ST_VCORNER_TOP, ST_HCORNER_RIGHT));
    myPlayList->changeFitMargins().top    = scale(110);
    myPlayList->changeFitMargins().bottom = scale(110);
    myPlayList->changeMargins().bottom    = scale(32);
    myPlayList->setOpacity(myPlugin->params.ToShowPlayList->getValue() ? 1.0f : 0.0f, false);
    myPlayList->signals.onOpenItem = stSlot(myPlugin, &StMoviePlayer::doFileNext);

    StMarginsI aButtonMargins;
    const IconSize anIconSize = scaleIcon(32, aButtonMargins);
    aButtonMargins.extend(scale(8));
    myBtnShuffle = new StGLCheckboxTextured(myPlayList, myPlugin->params.IsShuffle,
                                            iconTexture(stCString("actionVideoShuffle"), anIconSize),
                                            iconTexture(stCString("actionVideoShuffle"), anIconSize),
                                            scale(24), 0,
                                            StGLCorner(ST_VCORNER_BOTTOM, ST_HCORNER_CENTER));
    myBtnShuffle->changeMargins() = aButtonMargins;

    myBtnLoop = new StGLCheckboxTextured(myPlayList, myPlugin->params.ToLoopSingle,
                                         iconTexture(stCString("actionVideoLoopSingle"), anIconSize),
                                         iconTexture(stCString("actionVideoLoopSingle"), anIconSize),
                                         -scale(24), 0,
                                         StGLCorner(ST_VCORNER_BOTTOM, ST_HCORNER_CENTER));
    myBtnLoop->changeMargins() = aButtonMargins;

    // create main menu
    createMainMenu();
}

/**
 * Create upper toolbar
 */
void StMoviePlayerGUI::createUpperToolbar() {
    int aBtnIter = 0;
    int aNbBtnRight = 0;
    const int aTop  = scale(DISPL_Y_REGION_UPPER);
    const int aLeft = scale(DISPL_X_REGION_UPPER);
    StMarginsI aButtonMargins;
    const IconSize anIconSize = scaleIcon(32, aButtonMargins);
    const int      anIconStep = scale(48);
    aButtonMargins.extend(scale(8));

    myPanelUpper = new StGLContainer(this, aLeft, aTop, StGLCorner(ST_VCORNER_TOP, ST_HCORNER_LEFT), scale(4096), scale(128));

    // append the textured buttons
    myBtnOpen = new StGLTextureButton(myPanelUpper, (aBtnIter++) * anIconStep, 0);
    myBtnOpen->signals.onBtnClick.connect(myPlugin, &StMoviePlayer::doOpen1FileAction);
    myBtnOpen->setTexturePath(iconTexture(stCString("actionOpen"), anIconSize));
    myBtnOpen->setDrawShadow(true);
    myBtnOpen->changeMargins() = aButtonMargins;

    myBtnInfo = new StGLTextureButton(myPanelUpper, (aBtnIter++) * anIconStep, 0);
    myBtnInfo->signals.onBtnClick += stSlot(myPlugin, &StMoviePlayer::doAboutFile);
    myBtnInfo->setTexturePath(iconTexture(stCString("actionInfo"),  anIconSize));
    myBtnInfo->setDrawShadow(true);
    myBtnInfo->changeMargins() = aButtonMargins;

    StGLTextureButton* aSrcBtn = new StGLTextureButton(myPanelUpper, (aBtnIter++) * anIconStep, 0,
                                                       StGLCorner(ST_VCORNER_TOP, ST_HCORNER_LEFT), StFormat_NB + 1);
    aSrcBtn->changeMargins() = aButtonMargins;
    aSrcBtn->signals.onBtnClick += stSlot(this, &StMoviePlayerGUI::doDisplayStereoFormatCombo);
    const StString aSrcTextures[StFormat_NB + 1] = {
        iconTexture(stCString("menuMono"),           anIconSize),
        iconTexture(stCString("menuSbsLR"),          anIconSize),
        iconTexture(stCString("menuSbsRL"),          anIconSize),
        iconTexture(stCString("menuOverUnderLR"),    anIconSize),
        iconTexture(stCString("menuOverUnderRL"),    anIconSize),
        iconTexture(stCString("menuRowLR"),          anIconSize),
        iconTexture(stCString("menuColLR"),          anIconSize),
        iconTexture(stCString("menuDual"),           anIconSize),
        iconTexture(stCString("menuFrameSeqLR"),     anIconSize),
        iconTexture(stCString("menuRedCyanLR"),      anIconSize),
        iconTexture(stCString("menuGreenMagentaLR"), anIconSize),
        iconTexture(stCString("menuYellowBlueLR"),   anIconSize),
        iconTexture(stCString("menuTiledLR"),        anIconSize),
        iconTexture(stCString("menuAuto"),           anIconSize)
    };
    aSrcBtn->setTexturePath(aSrcTextures, StFormat_NB + 1);
    aSrcBtn->setDrawShadow(true);
    myBtnSrcFrmt = aSrcBtn;

    myBtnSwapLR = new StGLCheckboxTextured(myPanelUpper, myImage->params.SwapLR,
                                           iconTexture(stCString("actionSwapLROff"), anIconSize),
                                           iconTexture(stCString("actionSwapLROn"),  anIconSize),
                                           (aBtnIter++) * anIconStep, 0,
                                           StGLCorner(ST_VCORNER_TOP, ST_HCORNER_LEFT));
    myBtnSwapLR->setDrawShadow(true);
    myBtnSwapLR->changeMargins() = aButtonMargins;

    StHandle<StBoolParam> aTrackedPano = new StBoolParam(false);
    myBtnPanorama = new StGLCheckboxTextured(myPanelUpper, aTrackedPano,
                                             iconTexture(stCString("actionPanoramaOff"), anIconSize),
                                             iconTexture(stCString("actionPanorama"),    anIconSize),
                                             (aBtnIter++) * anIconStep, 0,
                                             StGLCorner(ST_VCORNER_TOP, ST_HCORNER_LEFT));
    myBtnPanorama->signals.onBtnClick += stSlot(this, &StMoviePlayerGUI::doPanoramaCombo);
    myBtnPanorama->setDrawShadow(true);
    myBtnPanorama->changeMargins() = aButtonMargins;

    myBtnAdjust = new StGLCheckboxTextured(myPanelUpper, myPlugin->params.ToShowAdjustImage,
                                           iconTexture(stCString("actionColorAdjustOff"), anIconSize),
                                           iconTexture(stCString("actionColorAdjust"),    anIconSize),
                                           (aBtnIter++) * anIconStep, 0,
                                           StGLCorner(ST_VCORNER_TOP, ST_HCORNER_LEFT));
    myBtnAdjust->setDrawShadow(true);
    myBtnAdjust->changeMargins() = aButtonMargins;

    // right buttons
    StHandle<StBoolParam> aTrackedSubs = new StBoolParam(false);
    myBtnSubs = new StGLCheckboxTextured(myPanelUpper, aTrackedSubs,
                                         iconTexture(stCString("actionStreamSubtitlesOff"), anIconSize),
                                         iconTexture(stCString("actionStreamSubtitles"),    anIconSize),
                                         (aNbBtnRight++) * (-anIconStep), 0,
                                         StGLCorner(ST_VCORNER_TOP, ST_HCORNER_RIGHT));
    myBtnSubs->signals.onBtnClick = stSlot(this, &StMoviePlayerGUI::doSubtitlesStreamsCombo);
    myBtnSubs->setDrawShadow(true);
    myBtnSubs->changeMargins() = aButtonMargins;

    StHandle<StBoolParam> aTrackedAudio = new StBoolParam(false);
    myBtnAudio = new StGLCheckboxTextured(myPanelUpper, aTrackedAudio,
                                          iconTexture(stCString("actionStreamAudioOff"), anIconSize),
                                          iconTexture(stCString("actionStreamAudio"),    anIconSize),
                                          (aNbBtnRight++) * (-anIconStep), 0,
                                          StGLCorner(ST_VCORNER_TOP, ST_HCORNER_RIGHT));
    myBtnAudio->signals.onBtnClick = stSlot(this, &StMoviePlayerGUI::doAudioStreamsCombo);
    myBtnAudio->setDrawShadow(true);
    myBtnAudio->changeMargins() = aButtonMargins;
}

/**
 * Create bottom toolbar
 */
void StMoviePlayerGUI::createBottomToolbar(const int theIconSize,
                                           const int theIconSizeSmall) {
    StMarginsI aButtonMargins, aButtonMarginsSmall;
    const IconSize anIconSize      = scaleIcon(theIconSize,      aButtonMargins);
    const IconSize anIconSizeSmall = scaleIcon(theIconSizeSmall, aButtonMarginsSmall);
    const int      anIconStep      = scale(theIconSize);

    myBottomBarNbLeft  = 0;
    myBottomBarNbRight = 0;
    myPanelBottom = new StGLContainer(this, scale(DISPL_X_REGION_BOTTOM), 0,
                                      StGLCorner(ST_VCORNER_BOTTOM, ST_HCORNER_LEFT),
                                      scale(4096), anIconStep);

    myBtnPlay = new StGLTextureButton(myPanelBottom, (myBottomBarNbLeft++) * anIconStep, 0,
                                      StGLCorner(ST_VCORNER_TOP, ST_HCORNER_LEFT), 2);
    myBtnPlay->signals.onBtnClick.connect(myPlugin, &StMoviePlayer::doPlayPause);
    const StString aPlayPaths[2] = {
        iconTexture(stCString("actionVideoPlay"),  anIconSize),
        iconTexture(stCString("actionVideoPause"), anIconSize)
    };

    myBtnPlay->setTexturePath(aPlayPaths, 2);
    myBtnPlay->setDrawShadow(true);
    myBtnPlay->changeMargins() = aButtonMargins;

    if(myWindow->hasFullscreenMode()) {
        myBtnFullScr = new StGLTextureButton(myPanelBottom, (myBottomBarNbRight++) * (-anIconStep), 0,
                                             StGLCorner(ST_VCORNER_TOP, ST_HCORNER_RIGHT), 4);
        myBtnFullScr->setAction(myPlugin->getAction(StMoviePlayer::Action_Fullscreen));
        const StString aSrcTextures[4] = {
            iconTexture(stCString("actionVideoFullscreenOff"),   anIconSize),
            iconTexture(stCString("actionVideoFullscreenOn"),    anIconSize),
            iconTexture(stCString("actionVideoFullscreen3dOff"), anIconSize),
            iconTexture(stCString("actionVideoFullscreen3dOn"),  anIconSize)
        };
        myBtnFullScr->setTexturePath(aSrcTextures, 4);
        myBtnFullScr->setDrawShadow(true);
        myBtnFullScr->changeMargins() = aButtonMargins;
    }

    myBtnList = new StGLCheckboxTextured(myPanelBottom, myPlugin->params.ToShowPlayList,
                                            iconTexture(stCString("actionVideoPlaylistOff"), anIconSize),
                                            iconTexture(stCString("actionVideoPlaylist"),    anIconSize),
                                            (myBottomBarNbRight++) * (-anIconStep), 0,
                                            StGLCorner(ST_VCORNER_TOP, ST_HCORNER_RIGHT));
    myBtnList->setDrawShadow(true);
    myBtnList->changeMargins() = aButtonMargins;

    myBtnNext = new StGLTextureButton(myPanelBottom,(myBottomBarNbRight++) * (-anIconStep), 0,
                                      StGLCorner(ST_VCORNER_TOP, ST_HCORNER_RIGHT));
    myBtnNext->signals.onBtnClick.connect(myPlugin, &StMoviePlayer::doListNext);
    myBtnNext->setTexturePath(iconTexture(stCString("actionVideoNext"), anIconSize));
    myBtnNext->setDrawShadow(true);
    myBtnNext->changeMargins() = aButtonMargins;

    myBtnPrev = new StGLTextureButton(myPanelBottom, (myBottomBarNbRight++) * (-anIconStep), 0,
                                      StGLCorner(ST_VCORNER_TOP, ST_HCORNER_RIGHT));
    myBtnPrev->signals.onBtnClick.connect(myPlugin, &StMoviePlayer::doListPrev);
    myBtnPrev->setTexturePath(iconTexture(stCString("actionVideoPrevious"), anIconSize));
    myBtnPrev->setDrawShadow(true);
    myBtnPrev->changeMargins() = aButtonMargins;

    myVolumeBar = new StGLSeekBar(myPanelBottom, 0, scale(4));
    myVolumeBar->changeRectPx().left()  = (myBottomBarNbRight++) * (-anIconStep) - scale(8);
    myVolumeBar->changeRectPx().right() = myVolumeBar->getRectPx().left() + 2 * anIconStep + scale(8);
    myVolumeBar->changeRectPx().moveTopTo(0 + (anIconStep - myVolumeBar->getRectPx().height()) / 2);
    myVolumeBar->setCorner(StGLCorner(ST_VCORNER_TOP, ST_HCORNER_RIGHT));
    myVolumeBar->signals.onSeekClick  = stSlot(this, &StMoviePlayerGUI::doAudioGain);
    myVolumeBar->signals.onSeekScroll = stSlot(this, &StMoviePlayerGUI::doAudioGainScroll);
    myVolumeBar->changeMargins().left  = scale(8);
    myVolumeBar->changeMargins().right = scale(8);

    myVolumeLab = new StGLTextArea(myVolumeBar, 0, 0, StGLCorner(ST_VCORNER_TOP, ST_HCORNER_LEFT),
                                   myVolumeBar->getRectPx().width(), myVolumeBar->getRectPx().height(), StGLTextArea::SIZE_NORMAL);
    myVolumeLab->setBorder(false);
    myVolumeLab->setTextColor(StGLVec3(1.0f, 1.0f, 1.0f));
    myVolumeLab->setupAlignment(StGLTextFormatter::ST_ALIGN_X_CENTER,
                                StGLTextFormatter::ST_ALIGN_Y_CENTER);
    myVolumeLab->setDrawShadow(true);

    ++myBottomBarNbRight;
    myBtnVolume = new StGLCheckboxTextured(myPanelBottom, myPlugin->params.AudioMute,
                                           iconTexture(stCString("actionVolume"),    anIconSizeSmall),
                                           iconTexture(stCString("actionVolumeOff"), anIconSizeSmall),
                                           (myBottomBarNbRight++) * (-anIconStep) - scale(32), scale(16),
                                           StGLCorner(ST_VCORNER_TOP, ST_HCORNER_RIGHT));
    myBtnVolume->setDrawShadow(true);
    myBtnVolume->setFalseOpacity(1.0f);
    myBtnVolume->setTrueOpacity(0.5f);
    myBtnVolume->changeMargins() = aButtonMarginsSmall;
}

/**
 * Main menu
 */
void StMoviePlayerGUI::createMainMenu() {
    myMenuRoot = new StGLMenu(this, 0, 0, StGLMenu::MENU_HORIZONTAL, true);

    StGLMenu* aMenuMedia   = createMediaMenu();     // Root -> Media menu
    StGLMenu* aMenuView    = createViewMenu();      // Root -> View menu
    StGLMenu* aDevicesMenu = createOutputMenu();    // Root -> Output menu
    StGLMenu* aMenuHelp    = createHelpMenu();      // Root -> Help menu

    // Attach sub menus to root
    myMenuRoot->addItem(tr(MENU_MEDIA),     aMenuMedia);
    myMenuRoot->addItem(tr(MENU_VIEW),      aMenuView);
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
    StGLMenu* aMenuOpenImage = createOpenMovieMenu();    // Root -> Media -> Open movie menu
    StGLMenu* aMenuSaveImage = createSaveImageMenu();    // Root -> Media -> Save snapshot menu

    aMenuMedia->addItem(tr(MENU_MEDIA_OPEN_MOVIE), myPlugin->getAction(StMoviePlayer::Action_Open1File),    aMenuOpenImage)
              ->setIcon(stCMenuIcon("actionOpen"), false);
    StGLMenuItem* anItem = aMenuMedia->addItem(tr(MENU_MEDIA_RECENT), myMenuRecent);
    anItem->setUserData(0);
    anItem->signals.onItemClick.connect(myPlugin, &StMoviePlayer::doOpenRecent);
    aMenuMedia->addItem(tr(MENU_MEDIA_SAVE_SNAPSHOT_AS), myPlugin->getAction(StMoviePlayer::Action_SaveSnapshot), aMenuSaveImage)
              ->setIcon(stCMenuIcon("actionSave"), false);
    aMenuMedia->addItem(tr(MENU_MEDIA_SRC_FORMAT), aMenuSrcFormat)
              ->setIcon(stCMenuIcon("actionSourceFormat"), false);

    if(myWindow->isMobile()) {
        aMenuMedia->addItem(myPlugin->params.IsMobileUI);
    }

    aMenuMedia->addItem(tr(MENU_MEDIA_AL_DEVICE),  myMenuOpenAL);

#ifdef ST_HAVE_MONGOOSE
    StString aWebUiItem = tr(MENU_MEDIA_WEBUI) + ":" + myPlugin->params.WebUIPort->getValue();
    if(myPlugin->params.IsLocalWebUI->getValue()) {
        aWebUiItem += " [CMD]";
    }
    aMenuMedia->addItem(aWebUiItem, aMenuWebUI);
#endif

    if(myPlugin->params.ToShowExtra->getValue()) {
        aMenuMedia->addItem(myPlugin->params.Benchmark->getName(), myPlugin->params.Benchmark);
    }

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
    theMenu->addItem(tr(MENU_SRC_FORMAT_AUTO),         myPlugin->params.SrcStereoFormat, StFormat_AUTO)
           ->setIcon(stCMenuIcon("menuAuto"));
    theMenu->addItem(tr(MENU_SRC_FORMAT_MONO),         myPlugin->params.SrcStereoFormat, StFormat_Mono)
           ->setIcon(stCMenuIcon("menuMono"));
    theMenu->addItem(tr(MENU_SRC_FORMAT_PARALLEL),     myPlugin->params.SrcStereoFormat, StFormat_SideBySide_LR)
           ->setIcon(stCMenuIcon("menuSbsLR"));
    theMenu->addItem(tr(MENU_SRC_FORMAT_CROSS_EYED),   myPlugin->params.SrcStereoFormat, StFormat_SideBySide_RL)
           ->setIcon(stCMenuIcon("menuSbsRL"));
    theMenu->addItem(tr(MENU_SRC_FORMAT_OVERUNDER_LR), myPlugin->params.SrcStereoFormat, StFormat_TopBottom_LR)
           ->setIcon(stCMenuIcon("menuOverUnderLR"));
    theMenu->addItem(tr(MENU_SRC_FORMAT_OVERUNDER_RL), myPlugin->params.SrcStereoFormat, StFormat_TopBottom_RL)
           ->setIcon(stCMenuIcon("menuOverUnderRL"));
    theMenu->addItem(tr(MENU_SRC_FORMAT_INTERLACED),   myPlugin->params.SrcStereoFormat, StFormat_Rows)
           ->setIcon(stCMenuIcon("menuRowLR"));
    theMenu->addItem(tr(MENU_SRC_FORMAT_ANA_RC),       myPlugin->params.SrcStereoFormat, StFormat_AnaglyphRedCyan)
           ->setIcon(stCMenuIcon("menuRedCyanLR"));
    theMenu->addItem(tr(MENU_SRC_FORMAT_ANA_RB),       myPlugin->params.SrcStereoFormat, StFormat_AnaglyphGreenMagenta)
           ->setIcon(stCMenuIcon("menuGreenMagentaLR"));
    theMenu->addItem(tr(MENU_SRC_FORMAT_ANA_YB),       myPlugin->params.SrcStereoFormat, StFormat_AnaglyphYellowBlue)
           ->setIcon(stCMenuIcon("menuYellowBlueLR"));
    theMenu->addItem(tr(MENU_SRC_FORMAT_PAGEFLIP),     myPlugin->params.SrcStereoFormat, StFormat_FrameSequence)
           ->setIcon(stCMenuIcon("menuFrameSeqLR"));
    theMenu->addItem(tr(MENU_SRC_FORMAT_TILED_4X),     myPlugin->params.SrcStereoFormat, StFormat_Tiled4x)
           ->setIcon(stCMenuIcon("menuTiledLR"));
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
    const StArrayList<StString>& aDevList = myPlugin->params.AudioAlDevice->getList();

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
                                                StHandle<StInt32Param>::downcast(myPlugin->params.AudioAlDevice), int32_t(devId));
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

    theMenu->addItem(myPlugin->params.ToOpenLast);
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
        if(myPlugin->params.IsLocalWebUI->getValue()
        && anIter == StMoviePlayer::WEBUI_AUTO) {
            continue;
        }
        aMenu->addItem(myPlugin->params.StartWebUI->getValues()[anIter], myPlugin->params.StartWebUI, (int32_t )anIter);
    }
    aMenu->addItem(myPlugin->params.ToPrintWebErrors);
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
    StGLMenu* aMenuPanorama  = new StGLMenu(this, 0, 0, StGLMenu::MENU_VERTICAL);
    fillPanoramaMenu(aMenuPanorama);
    StGLMenu* aMenuTexFilter = createSmoothFilterMenu();
    StGLMenu* aMenuImgAdjust = createImageAdjustMenu();
    StGLMenu* aMenu3dAdjust  = create3dAdjustMenu();

    aMenuView->addItem(tr(MENU_VIEW_DISPLAY_MODE),  aMenuDispMode);
    if(myWindow->hasFullscreenMode()) {
        aMenuView->addItem(myPlugin->params.IsFullscreen);
    }
    aMenuView->addItem(tr(MENU_VIEW_RESET), myImage->getActions()[StGLImageRegion::Action_Reset])
             ->setIcon(stCMenuIcon("actionResetPlacement"), false);
    aMenuView->addItem(tr(MENU_VIEW_SWAP_LR),       myImage->params.SwapLR);
    aMenuView->addItem(tr(MENU_VIEW_DISPLAY_RATIO), aMenuDispRatio)
             ->setIcon(stCMenuIcon("actionDisplayRatio"), false);
    aMenuView->addItem(tr(MENU_VIEW_PANORAMA),      aMenuPanorama)
             ->setIcon(stCMenuIcon("actionPanorama"), false);
    aMenuView->addItem(tr(MENU_VIEW_TEXFILTER),     aMenuTexFilter)
             ->setIcon(stCMenuIcon("actionInterpolation"), false);
    aMenuView->addItem(tr(MENU_VIEW_IMAGE_ADJUST),  aMenuImgAdjust)
             ->setIcon(stCMenuIcon("actionColorAdjust"), false);
    aMenuView->addItem("3D Stereo",  aMenu3dAdjust)
             ->setIcon(stCMenuIcon("actionStereo3dSettings"), false);
    return aMenuView;
}

/**
 * Root -> View menu -> Output
 */
StGLMenu* StMoviePlayerGUI::createDisplayModeMenu() {
    StGLMenu* aMenu = new StGLMenu(this, 0, 0, StGLMenu::MENU_VERTICAL);
    const StArrayList<StString>& aValuesList = myImage->params.DisplayMode->getValues();
    for(size_t aValIter = 0; aValIter < aValuesList.size(); ++aValIter) {
        aMenu->addItem(aValuesList[aValIter], myImage->params.DisplayMode, int32_t(aValIter));
    }
    return aMenu;
}

/**
 * Root -> View menu -> Display Ratio
 */
StGLMenu* StMoviePlayerGUI::createDisplayRatioMenu() {
    StGLMenu* aMenu = new StGLMenu(this, 0, 0, StGLMenu::MENU_VERTICAL);
    fillDisplayRatioMenu(aMenu);
    aMenu->addItem(myPlugin->params.ToRestoreRatio);
    aMenu->addItem(tr(MENU_VIEW_RATIO_HEAL_ANAMORPHIC), myImage->params.ToHealAnamorphicRatio);
    return aMenu;
}

void StMoviePlayerGUI::fillDisplayRatioMenu(StGLMenu* theMenu) {
    theMenu->addItem(tr(MENU_VIEW_DISPLAY_RATIO_SRC), myImage->params.DisplayRatio, StGLImageRegion::RATIO_AUTO)
           ->setIcon(stCMenuIcon("menuAuto"));
    theMenu->addItem("2.21:1", myImage->params.DisplayRatio, StGLImageRegion::RATIO_221_1)
           ->setIcon(stCMenuIcon("menuRatio2_1_"));
    theMenu->addItem("16:9",   myImage->params.DisplayRatio, StGLImageRegion::RATIO_16_9)
           ->setIcon(stCMenuIcon("menuRatio16_9_"));
    theMenu->addItem("16:10",  myImage->params.DisplayRatio, StGLImageRegion::RATIO_16_10)
           ->setIcon(stCMenuIcon("menuRatio16_10_"));
    theMenu->addItem("4:3",    myImage->params.DisplayRatio, StGLImageRegion::RATIO_4_3)
           ->setIcon(stCMenuIcon("menuRatio4_3_"));
    theMenu->addItem("5:4",    myImage->params.DisplayRatio, StGLImageRegion::RATIO_5_4)
           ->setIcon(stCMenuIcon("menuRatio5_4_"));
    theMenu->addItem("1:1",    myImage->params.DisplayRatio, StGLImageRegion::RATIO_1_1)
           ->setIcon(stCMenuIcon("menuRatio1_1_"));
}

void StMoviePlayerGUI::doDisplayRatioCombo(const size_t ) {
    StGLCombobox::ListBuilder aBuilder(this);
    fillDisplayRatioMenu(aBuilder.getMenu());
    aBuilder.getMenu()->addItem(tr(MENU_VIEW_RATIO_HEAL_ANAMORPHIC), myImage->params.ToHealAnamorphicRatio);
    aBuilder.display();
}

void StMoviePlayerGUI::fillPanoramaMenu(StGLMenu* theMenu) {
    theMenu->addItem(tr(MENU_VIEW_SURFACE_PLANE),
                     myImage->params.ViewMode, StViewSurface_Plain);
    theMenu->addItem(tr(MENU_VIEW_SURFACE_THEATER),
                     myImage->params.ViewMode, StViewSurface_Theater);
    theMenu->addItem(tr(MENU_VIEW_SURFACE_CYLINDER),
                     myImage->params.ViewMode, StViewSurface_Cylinder);
    theMenu->addItem(tr(MENU_VIEW_SURFACE_HEMISPHERE),
                     myImage->params.ViewMode, StViewSurface_Hemisphere);
    theMenu->addItem(tr(MENU_VIEW_SURFACE_SPHERE),
                     myImage->params.ViewMode, StViewSurface_Sphere);
    theMenu->addItem(tr(MENU_VIEW_SURFACE_CUBEMAP),
                     myImage->params.ViewMode, StViewSurface_Cubemap);
    theMenu->addItem(tr(MENU_VIEW_SURFACE_CUBEMAP_EAC),
                     myImage->params.ViewMode, StViewSurface_CubemapEAC);
    if(myWindow->hasOrientationSensor()) {
        theMenu->addItem(tr(myWindow->isPoorOrientationSensor() ? MENU_VIEW_TRACK_HEAD_POOR : MENU_VIEW_TRACK_HEAD),
                         myPlugin->params.ToTrackHead);
    }
    theMenu->addItem(tr(MENU_VIEW_TRACK_HEAD_AUDIO),
                     myPlugin->params.ToTrackHeadAudio);
    theMenu->addItem(myPlugin->params.ToStickPanorama);
}

void StMoviePlayerGUI::doPanoramaCombo(const size_t ) {
    StGLCombobox::ListBuilder aBuilder(this);
    fillPanoramaMenu(aBuilder.getMenu());
    aBuilder.display();
}

/**
 * Root -> View menu -> Smooth Filter
 */
StGLMenu* StMoviePlayerGUI::createSmoothFilterMenu() {
    StGLMenu* aMenu = new StGLMenu(this, 0, 0, StGLMenu::MENU_VERTICAL);
    aMenu->addItem(tr(MENU_VIEW_TEXFILTER_NEAREST),
                   myImage->params.TextureFilter, StGLImageProgram::FILTER_NEAREST);
    aMenu->addItem(tr(MENU_VIEW_TEXFILTER_LINEAR),
                   myImage->params.TextureFilter, StGLImageProgram::FILTER_LINEAR);
    aMenu->addItem(tr(MENU_VIEW_TEXFILTER_TRILINEAR),
                   myImage->params.TextureFilter, StGLImageProgram::FILTER_TRILINEAR);
    aMenu->addItem(tr(MENU_VIEW_TEXFILTER_BLEND),
                   myImage->params.TextureFilter, StGLImageProgram::FILTER_BLEND);
    return aMenu;
}

/**
 * Root -> View menu -> Image Adjust
 */
StGLMenu* StMoviePlayerGUI::createImageAdjustMenu() {
    StGLMenu* aMenu = new StGLMenu(this, 0, 0, StGLMenu::MENU_VERTICAL);
    aMenu->addItem(tr(MENU_VIEW_ADJUST_RESET), myPlugin->getAction(StMoviePlayer::Action_ImageAdjustReset))
         ->setIcon(stCMenuIcon("actionColorReset"), false);

    StGLMenuItem* anItem = aMenu->addItem(tr(MENU_VIEW_ADJUST_GAMMA));
    anItem->changeMargins().right = scale(100 + 16);
    StGLRangeFieldFloat32* aRange = new StGLRangeFieldFloat32(anItem, myImage->params.Gamma,
                                                              -scale(16), 0, StGLCorner(ST_VCORNER_CENTER, ST_HCORNER_RIGHT));
    aRange->setFormat(stCString("%+01.2f"));
    aRange->setColor(StGLRangeFieldFloat32::FieldColor_Default,  aBlack);
    aRange->setColor(StGLRangeFieldFloat32::FieldColor_Positive, aGreen);
    aRange->setColor(StGLRangeFieldFloat32::FieldColor_Negative, aRed);

    anItem = aMenu->addItem(tr(MENU_VIEW_ADJUST_BRIGHTNESS));
    anItem->setIcon(stCMenuIcon("actionBrightness"), false);
    anItem->changeMargins().right = scale(100 + 16);
    aRange = new StGLRangeFieldFloat32(anItem, myImage->params.Brightness,
                                       -scale(16), 0, StGLCorner(ST_VCORNER_CENTER, ST_HCORNER_RIGHT));
    aRange->setFormat(stCString("%+01.2f"));
    aRange->setColor(StGLRangeFieldFloat32::FieldColor_Default,  aBlack);
    aRange->setColor(StGLRangeFieldFloat32::FieldColor_Positive, aGreen);
    aRange->setColor(StGLRangeFieldFloat32::FieldColor_Negative, aRed);

    anItem = aMenu->addItem(tr(MENU_VIEW_ADJUST_SATURATION));
    anItem->setIcon(stCMenuIcon("actionSaturation"), false);
    anItem->changeMargins().right = scale(100 + 16);
    aRange = new StGLRangeFieldFloat32(anItem, myImage->params.Saturation,
                                       -scale(16), 0, StGLCorner(ST_VCORNER_CENTER, ST_HCORNER_RIGHT));
    aRange->changeRectPx().bottom() = aRange->getRectPx().top() + aMenu->getItemHeight();
    aRange->setFormat(stCString("%+01.2f"));
    aRange->setColor(StGLRangeFieldFloat32::FieldColor_Default,  aBlack);
    aRange->setColor(StGLRangeFieldFloat32::FieldColor_Positive, aGreen);
    aRange->setColor(StGLRangeFieldFloat32::FieldColor_Negative, aRed);
    return aMenu;
}

/**
 * Root -> View -> Stereo 3D Adjust
 */
StGLMenu* StMoviePlayerGUI::create3dAdjustMenu() {
    StGLMenu* aMenu = new StGLMenu(this, 0, 0, StGLMenu::MENU_VERTICAL);

    StGLMenuItem* anItem = NULL;
    StGLRangeFieldFloat32* aRange = NULL;

    anItem = aMenu->addItem("DX separation");
    anItem->changeMargins().right = scale(100 + 16);
    aRange = new StGLRangeFieldFloat32(anItem, myImage->params.SeparationDX,
                                       -scale(16), 0, StGLCorner(ST_VCORNER_CENTER, ST_HCORNER_RIGHT));
    aRange->setFormat(stCString("%+01.2f"));
    aRange->setColor(StGLRangeFieldFloat32::FieldColor_Default,  aBlack);
    aRange->setColor(StGLRangeFieldFloat32::FieldColor_Positive, aGreen);
    aRange->setColor(StGLRangeFieldFloat32::FieldColor_Negative, aRed);

    anItem = aMenu->addItem("DY separation");
    anItem->changeMargins().right = scale(100 + 16);
    aRange = new StGLRangeFieldFloat32(anItem, myImage->params.SeparationDY,
                                       -scale(16), 0, StGLCorner(ST_VCORNER_CENTER, ST_HCORNER_RIGHT));
    aRange->setFormat(stCString("%+01.2f"));
    aRange->setColor(StGLRangeFieldFloat32::FieldColor_Default,  aBlack);
    aRange->setColor(StGLRangeFieldFloat32::FieldColor_Positive, aGreen);
    aRange->setColor(StGLRangeFieldFloat32::FieldColor_Negative, aRed);

    anItem = aMenu->addItem("Angular separation");
    anItem->changeMargins().right = scale(100 + 16);
    aRange = new StGLRangeFieldFloat32(anItem, myImage->params.SeparationRot,
                                       -scale(16), 0, StGLCorner(ST_VCORNER_CENTER, ST_HCORNER_RIGHT));
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
    aMenu->addItem(tr(MENU_AUDIO_NONE), myPlugin->params.AudioStream, -1);
    return aMenu;
}

/**
 * Dialog for Audio/Video synchronization control.
 */
class ST_LOCAL StDelayControl : public StGLMessageBox {

        public:

    StDelayControl(StMoviePlayerGUI*               theParent,
                   const StHandle<StFloat32Param>& theTrackedValue)
    : StGLMessageBox(theParent),
      myRange(NULL) {
        int aWidth  = stMin(theParent->scale(400), myRoot->getRectPx().width());
        int aHeight = stMin(theParent->scale(220), myRoot->getRectPx().height());
        const bool isCompact = myRoot->getRectPx().width()  <= myRoot->scale(450)
                            || myRoot->getRectPx().height() <= myRoot->scale(450);
        if(isCompact) {
            aHeight = stMin(theParent->scale(150), aHeight);
        } else {
            changeRectPx().left() = myRoot->scale(64);
            changeRectPx().top() = -myRoot->scale(128);
        }
        setCorner(StGLCorner(ST_VCORNER_BOTTOM, ST_HCORNER_LEFT));
        changeRectPx().right()  = getRectPx().left() + aWidth;
        changeRectPx().bottom() = getRectPx().top()  + aHeight;
        create(theParent->tr(DIALOG_AUDIO_DELAY_TITLE), "", aWidth, aHeight);
        if(isCompact) {
            myMinSizeY = theParent->scale(150);
        }

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

void StMoviePlayerGUI::doAudioGain(const int    theMouseBtn,
                                   const double theVolume) {
    if(theMouseBtn == ST_MOUSE_LEFT) {
        myPlugin->params.AudioGain->setValue(myPlugin->volumeToGain(myPlugin->params.AudioGain, GLfloat(theVolume)));
    }
}

void StMoviePlayerGUI::doAudioGainScroll(const double theDelta) {
    if(theDelta > 0.001) {
        myPlugin->params.AudioGain->increment();
    } else if(theDelta < -0.001) {
        myPlugin->params.AudioGain->decrement();
    }
}

void StMoviePlayerGUI::doAudioDelay(const size_t ) {
    StGLMessageBox* aDialog = new StDelayControl(this, myPlugin->params.AudioDelay);
    aDialog->stglInit();
    setModalDialog(aDialog);
}

/**
 * Root -> Subtitles menu
 */
StGLMenu* StMoviePlayerGUI::createSubtitlesMenu() {
    StGLMenu* aMenu = new StGLMenu(this, 0, 0, StGLMenu::MENU_VERTICAL);
    aMenu->addItem(tr(MENU_SUBTITLES_NONE), myPlugin->params.SubtitlesStream, -1);
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
         ->setIcon(stCMenuIcon("actionHelp"), false)
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
    aMenu->addItem(myPlugin->params.IsVSyncOn);
    aMenu->addItem(myPlugin->params.ToShowFps);
    aMenu->addItem(myPlugin->params.ToLimitFps);
    return aMenu;
}

void StMoviePlayerGUI::doAboutProgram(const size_t ) {
    const StGLVec3 THE_WHITE(1.0f, 1.0f, 1.0f);
    const StString anAbout = tr(ABOUT_DPLUGIN_NAME) + '\n'
                           + tr(ABOUT_VERSION) + " " + StVersionInfo::getSDKVersionString()
                           + "\n \n" + tr(ABOUT_DESCRIPTION).format("2007-2020", "kirill@sview.ru", "www.sview.ru")
                           + "\n\n<b><i>Used projects</i></b>"
                           + "\n \nFFmpeg " + stAV::getVersionInfo() + " (" + stAV::getLicenseInfo() + ")\nhttps://ffmpeg.org/"
                           + "\n \nOpenAL Soft (LGPL)\nhttp://kcat.strangesoft.net/openal.html"
                           + "\n \nFreeType \nhttp://freetype.org/";

    StDictionary anInfo;
    anInfo.add(StDictEntry("CPU cores", StString(StThread::countLogicalProcessors()) + StString(" logical processor(s)")));
    getContext().stglFullInfo(anInfo);
    anInfo.add(StDictEntry("Display Scale", StString(myWindow->getMonitors()[myWindow->getPlacement().center()].getScale()) + "x"));

    // OpenAL info
    myPlugin->myVideo->getAlInfo(anInfo);

    StGLMessageBox* aDialog = new StGLMessageBox(this, tr(MENU_HELP_ABOUT), "", scale(512), scale(300));
    StGLTable* aTable = new StGLTable(aDialog->getContent(), 0, 0, StGLCorner(ST_VCORNER_TOP, ST_HCORNER_CENTER));
    aTable->setupTable((int )anInfo.size() + 1, 2);

    const int aTextMaxWidth = aDialog->getContent()->getRectPx().width() - 2 * (aTable->getItemMargins().left + aTable->getItemMargins().right);
    StGLTableItem& anAboutItem = aTable->changeElement(0, 0); anAboutItem.setColSpan(2);
    StGLTextArea*  anAboutLab  = new StGLTextArea(&anAboutItem, 0, 0, StGLCorner(ST_VCORNER_CENTER, ST_HCORNER_CENTER));
    anAboutLab->setupAlignment(StGLTextFormatter::ST_ALIGN_X_CENTER,
                               StGLTextFormatter::ST_ALIGN_Y_TOP);
    anAboutLab->setText(anAbout + "\n\n<b><i>" + tr(ABOUT_SYSTEM) + "</i></b>\n");
    anAboutLab->setTextColor(THE_WHITE);
    anAboutLab->stglInitAutoHeightWidth(aTextMaxWidth);

    aTable->fillFromMap(anInfo, THE_WHITE,
                        aDialog->getContent()->getRectPx().width(),
                        aDialog->getContent()->getRectPx().width() / 2, 1);

    aDialog->addButton(stCString("Website"))->signals.onBtnClick += stSlot(this, &StMoviePlayerGUI::doCheckUpdates);
    aDialog->addButton(tr(BUTTON_CLOSE), true);
    aDialog->stglInit();
    setModalDialog(aDialog);
}

void StMoviePlayerGUI::doUserTips(const size_t ) {
    StProcess::openURL("http://sview.ru/sview/usertips");
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
    anExtraInfo.nullify();

    StHandle<StFileNode>     aFileNode;
    StHandle<StStereoParams> aParams;
    if(!myPlugin->getCurrentFile(aFileNode, aParams, anExtraInfo)
    ||  anExtraInfo.isNull()) {
        anExtraInfo.nullify();
        StGLMessageBox* aMsgBox = new StGLMessageBox(this, tr(DIALOG_FILE_INFO), tr(DIALOG_FILE_NOINFO));
        aMsgBox->addButton(tr(BUTTON_CLOSE), true);
        aMsgBox->stglInit();
        setModalDialog(aMsgBox);
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
        const size_t aSize = anEntry.getValue().getSize();
        anEntry.changeName() = myLangMap->getValue(aKey);
        if(aSize > 16384) {
            // cut too long strings
            anEntry.changeValue() = anEntry.getValue().subString(0, 128) + "\n...[" + (aSize / 1024) + " KiB]";
        }
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
    setModalDialog(aDialog);
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
    StGLMenu* aMenuLanguage     = createLanguageMenu();     // Root -> Help -> Language menu

    aMenu->addItem(tr(MENU_HELP_ABOUT))
         ->setIcon(stCMenuIcon("actionHelp"), false)
         ->signals.onItemClick.connect(this, &StMoviePlayerGUI::doAboutProgram);
    aMenu->addItem(tr(MENU_HELP_USERTIPS))
         ->signals.onItemClick.connect(this, &StMoviePlayerGUI::doUserTips);
    aMenu->addItem(tr(MENU_HELP_HOTKEYS))
         ->setIcon(stCMenuIcon("actionKeyboard"), false)
         ->signals.onItemClick.connect(this, &StMoviePlayerGUI::doListHotKeys);
    aMenu->addItem(tr(MENU_HELP_SETTINGS))
         ->setIcon(stCMenuIcon("actionSettings"), false)
         ->signals.onItemClick.connect(this, &StMoviePlayerGUI::doMobileSettings);
    aMenu->addItem(tr(MENU_HELP_LICENSE))
         ->signals.onItemClick.connect(this, &StMoviePlayerGUI::doOpenLicense);
    aMenu->addItem(tr(MENU_HELP_SCALE),        aMenuScale)
         ->setIcon(stCMenuIcon("actionFontSize"), false);
    aMenu->addItem(tr(MENU_HELP_LANGS),        aMenuLanguage)
         ->setIcon(stCMenuIcon("actionLanguage"), false);
    return aMenu;
}

/**
 * Root -> Help -> Scale Interface menu
 */
StGLMenu* StMoviePlayerGUI::createScaleMenu() {
    StGLMenu* aMenu = new StGLMenu(this, 0, 0, StGLMenu::MENU_VERTICAL);
    aMenu->addItem(myPlugin->params.ScaleAdjust,  StGLRootWidget::ScaleAdjust_Small);
    aMenu->addItem(myPlugin->params.ScaleAdjust,  StGLRootWidget::ScaleAdjust_Normal);
    aMenu->addItem(myPlugin->params.ScaleAdjust,  StGLRootWidget::ScaleAdjust_Big);
    aMenu->addItem(myPlugin->params.ScaleHiDPI2X);
    aMenu->addItem(myPlugin->params.IsMobileUI);
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
    createImageAdjustments();
    createMobileUpperToolbar();
    createMobileBottomToolbar();

    myPlayList = new StGLPlayList(this, thePlayList);
    myPlayList->setCorner(StGLCorner(ST_VCORNER_TOP, ST_HCORNER_RIGHT));
    myPlayList->changeFitMargins().top    = scale(56);
    myPlayList->changeFitMargins().bottom = scale(100);
    myPlayList->changeMargins().bottom    = scale(56);
    myPlayList->setOpacity(myPlugin->params.ToShowPlayList->getValue() ? 1.0f : 0.0f, false);
    myPlayList->signals.onOpenItem = stSlot(myPlugin, &StMoviePlayer::doFileNext);

    StMarginsI aButtonMargins;
    const IconSize anIconSize = scaleIcon(32, aButtonMargins);
    aButtonMargins.extend(scale(12));
    StGLCheckboxTextured* aBtnShuffle = new StGLCheckboxTextured(myPlayList, myPlugin->params.IsShuffle,
                                                                 iconTexture(stCString("actionVideoShuffle"), anIconSize),
                                                                 iconTexture(stCString("actionVideoShuffle"), anIconSize),
                                                                 scale(28), 0,
                                                                 StGLCorner(ST_VCORNER_BOTTOM, ST_HCORNER_CENTER));
    aBtnShuffle->changeMargins() = aButtonMargins;

    StGLCheckboxTextured* aBtnLoop = new StGLCheckboxTextured(myPlayList, myPlugin->params.ToLoopSingle,
                                                              iconTexture(stCString("actionVideoLoopSingle"), anIconSize),
                                                              iconTexture(stCString("actionVideoLoopSingle"), anIconSize),
                                                              -scale(28), 0,
                                                              StGLCorner(ST_VCORNER_BOTTOM, ST_HCORNER_CENTER));
    aBtnLoop->changeMargins() = aButtonMargins;
}

/**
 * Create upper toolbar
 */
void StMoviePlayerGUI::createMobileUpperToolbar() {
    StMarginsI aButtonMargins;
    const IconSize anIconSize = scaleIcon(32, aButtonMargins);
    aButtonMargins.extend(scale(12));

    myPanelUpper = new StGLContainer(this, 0, 0, StGLCorner(ST_VCORNER_TOP, ST_HCORNER_LEFT), scale(4096), scale(56));

    int aBtnIter = 0;

    myBtnOpen = new StGLTextureButton(myPanelUpper, (aBtnIter++) * myIconStep, 0);
    myBtnOpen->signals.onBtnClick.connect(this, &StMoviePlayerGUI::doOpenFile);
    myBtnOpen->setTexturePath(iconTexture(stCString("actionOpen"), anIconSize));
    myBtnOpen->setDrawShadow(true);
    myBtnOpen->changeMargins() = aButtonMargins;

    StGLTextureButton* aSrcBtn = new StGLTextureButton(myPanelUpper, (aBtnIter++) * myIconStep, 0,
                                                       StGLCorner(ST_VCORNER_TOP, ST_HCORNER_LEFT), StFormat_NB + 1);
    aSrcBtn->changeMargins() = aButtonMargins;
    aSrcBtn->signals.onBtnClick += stSlot(this, &StMoviePlayerGUI::doDisplayStereoFormatCombo);
    const StString aSrcTextures[StFormat_NB + 1] = {
        iconTexture(stCString("menuMono"),           anIconSize),
        iconTexture(stCString("menuSbsLR"),          anIconSize),
        iconTexture(stCString("menuSbsRL"),          anIconSize),
        iconTexture(stCString("menuOverUnderLR"),    anIconSize),
        iconTexture(stCString("menuOverUnderRL"),    anIconSize),
        iconTexture(stCString("menuRowLR"),          anIconSize),
        iconTexture(stCString("menuColLR"),          anIconSize),
        iconTexture(stCString("menuDual"),           anIconSize),
        iconTexture(stCString("menuFrameSeqLR"),     anIconSize),
        iconTexture(stCString("menuRedCyanLR"),      anIconSize),
        iconTexture(stCString("menuGreenMagentaLR"), anIconSize),
        iconTexture(stCString("menuYellowBlueLR"),   anIconSize),
        iconTexture(stCString("menuTiledLR"),        anIconSize),
        iconTexture(stCString("menuAuto"),           anIconSize)
    };
    aSrcBtn->setTexturePath(aSrcTextures, StFormat_NB + 1);
    aSrcBtn->setDrawShadow(true);
    myBtnSrcFrmt = aSrcBtn;

    myBtnSwapLR = new StGLCheckboxTextured(myPanelUpper, myImage->params.SwapLR,
                                           iconTexture(stCString("actionSwapLROff"), anIconSize),
                                           iconTexture(stCString("actionSwapLROn"),  anIconSize),
                                           (aBtnIter++) * myIconStep, 0,
                                           StGLCorner(ST_VCORNER_TOP, ST_HCORNER_LEFT));
    myBtnSwapLR->setDrawShadow(true);
    myBtnSwapLR->changeMargins() = aButtonMargins;

    StHandle<StBoolParam> aTrackedPano = new StBoolParam(false);
    myBtnPanorama = new StGLCheckboxTextured(myPanelUpper, aTrackedPano,
                                             iconTexture(stCString("actionPanoramaOff"), anIconSize),
                                             iconTexture(stCString("actionPanorama"),    anIconSize),
                                             (aBtnIter++) * myIconStep, 0,
                                             StGLCorner(ST_VCORNER_TOP, ST_HCORNER_LEFT));
    myBtnPanorama->signals.onBtnClick += stSlot(this, &StMoviePlayerGUI::doPanoramaCombo);
    myBtnPanorama->setDrawShadow(true);
    myBtnPanorama->changeMargins() = aButtonMargins;

    myBtnAdjust = new StGLCheckboxTextured(myPanelUpper, myPlugin->params.ToShowAdjustImage,
                                           iconTexture(stCString("actionColorAdjustOff"), anIconSize),
                                           iconTexture(stCString("actionColorAdjust"),    anIconSize),
                                           (aBtnIter++) * myIconStep, 0,
                                           StGLCorner(ST_VCORNER_TOP, ST_HCORNER_LEFT));
    myBtnAdjust->setDrawShadow(true);
    myBtnAdjust->changeMargins() = aButtonMargins;

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

    ///
    ///const int aBotOffset = scale(56);
    myPanelBottom = new StGLContainer(this, 0, 0, StGLCorner(ST_VCORNER_BOTTOM, ST_HCORNER_LEFT), scale(4096), scale(56));

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
    if(myWindow->hasFullscreenMode()) {
        myBtnFullScr = new StGLTextureButton(myPanelBottom, (myBottomBarNbRight++) * (-myIconStep), 0,
                                             aRightCorner, 4);
        myBtnFullScr->setAction(myPlugin->getAction(StMoviePlayer::Action_Fullscreen));
        const StString aSrcTextures[4] = {
            iconTexture(stCString("actionVideoFullscreenOff"),   anIconSize),
            iconTexture(stCString("actionVideoFullscreenOn"),    anIconSize),
            iconTexture(stCString("actionVideoFullscreen3dOff"), anIconSize),
            iconTexture(stCString("actionVideoFullscreen3dOn"),  anIconSize)
        };
        myBtnFullScr->setTexturePath(aSrcTextures, 4);
        myBtnFullScr->setDrawShadow(true);
        myBtnFullScr->changeMargins() = aButtonMargins;
    }

    myBtnList = new StGLCheckboxTextured(myPanelBottom, myPlugin->params.ToShowPlayList,
                                         iconTexture(stCString("actionVideoPlaylistOff"), anIconSize),
                                         iconTexture(stCString("actionVideoPlaylist"),    anIconSize),
                                         (myBottomBarNbRight++) * (-myIconStep), 0,
                                         aRightCorner);
    myBtnList->setDrawShadow(true);
    myBtnList->changeMargins() = aButtonMargins;

    StGLTextureButton* aBtnInfo = new StGLTextureButton(myPanelBottom, (myBottomBarNbRight++) * (-myIconStep), 0, aRightCorner);
    aBtnInfo->signals.onBtnClick += stSlot(myPlugin, &StMoviePlayer::doAboutFile);
    aBtnInfo->setTexturePath(iconTexture(stCString("actionInfo"),  anIconSize));
    aBtnInfo->setDrawShadow(true);
    aBtnInfo->changeMargins() = aButtonMargins;

    mySeekBar = new StGLSeekBar(myPanelBottom, 0, scale(18));
    mySeekBar->setMoveTolerance(scale(isMobile() ? 16 : 8));
    mySeekBar->signals.onSeekClick.connect(myPlugin, &StMoviePlayer::doSeek);

    myTimeBox = new StTimeBox(myPanelBottom, myBottomBarNbRight * (-myIconStep), 0, aRightCorner, StGLTextArea::SIZE_SMALL);
    myTimeBox->setSwitchOnClick(true);
    myTimeBox->changeRectPx().right()  = myTimeBox->getRectPx().left() + myIconStep * 2;
    myTimeBox->changeRectPx().bottom() = myTimeBox->getRectPx().top()  + scale(56);
}

/**
 * Create image adjustments control
 */
void StMoviePlayerGUI::createImageAdjustments() {
    StMarginsI aButtonMargins;
    const IconSize anIconSize = scaleIcon(32, aButtonMargins);
    const int      anIconStep = scale(isMobile() ? 56 : 48);
    const int      aCtrlStep  = scale(36);
    const int      aSlideWidth = anIconStep * 4;
    aButtonMargins.extend(scale(isMobile() ? 12 : 8));

    myAdjustOverlay = new StGLContainer(this,
                                        isMobile() ? anIconStep / 2 : scale(DISPL_X_REGION_UPPER),
                                        isMobile() ? scale(56) : scale(72),
                                        StGLCorner(ST_VCORNER_TOP, ST_HCORNER_LEFT),
                                        scale(4096), aCtrlStep * 6 + anIconStep);
    myAdjustOverlay->setOpacity(myPlugin->params.ToShowAdjustImage->getValue() ? 1.0f : 0.0f, false);

    int aBtnIter = 0;
    {
        StGLRangeFieldFloat32* aRange = new StGLRangeFieldFloat32(myAdjustOverlay, myImage->params.Gamma,
                                                                  0, 0, StGLCorner(ST_VCORNER_CENTER, ST_HCORNER_RIGHT),
                                                                  StGLRangeFieldFloat32::RangeStyle_Seekbar, scale(18));
        aRange->changeRectPx().right() = aRange->getRectPx().left() + aSlideWidth;
        aRange->changeRectPx().moveTopTo(aCtrlStep * (aBtnIter++));
        aRange->changeMargins().left   = scale(8);
        aRange->changeMargins().right  = scale(8);
        aRange->setCorner(StGLCorner(ST_VCORNER_TOP, ST_HCORNER_LEFT));
        aRange->setFormat(tr(MENU_VIEW_ADJUST_GAMMA) + ": %+01.2f");
    }
    {
        StGLRangeFieldFloat32* aRange = new StGLRangeFieldFloat32(myAdjustOverlay, myImage->params.Brightness,
                                                                  0, 0, StGLCorner(ST_VCORNER_CENTER, ST_HCORNER_RIGHT),
                                                                  StGLRangeFieldFloat32::RangeStyle_Seekbar, scale(18));
        aRange->changeRectPx().right() = aRange->getRectPx().left() + aSlideWidth;
        aRange->changeRectPx().moveTopTo(aCtrlStep * (aBtnIter++));
        aRange->changeMargins().left   = scale(8);
        aRange->changeMargins().right  = scale(8);
        aRange->setCorner(StGLCorner(ST_VCORNER_TOP, ST_HCORNER_LEFT));
        aRange->setFormat(tr(MENU_VIEW_ADJUST_BRIGHTNESS) + ": %+01.2f");
    }
    {
        StGLRangeFieldFloat32* aRange = new StGLRangeFieldFloat32(myAdjustOverlay, myImage->params.Saturation,
                                                                  0, 0, StGLCorner(ST_VCORNER_CENTER, ST_HCORNER_RIGHT),
                                                                  StGLRangeFieldFloat32::RangeStyle_Seekbar, scale(18));
        aRange->changeRectPx().right() = aRange->getRectPx().left() + aSlideWidth;
        aRange->changeRectPx().moveTopTo(aCtrlStep * (aBtnIter++));
        aRange->changeMargins().left   = scale(8);
        aRange->changeMargins().right  = scale(8);
        aRange->setCorner(StGLCorner(ST_VCORNER_TOP, ST_HCORNER_LEFT));
        aRange->setFormat(tr(MENU_VIEW_ADJUST_SATURATION) + ": %+01.2f");
    }
    myBtnResetColor1 = new StGLTextureButton(myAdjustOverlay, anIconStep * 1, aCtrlStep * aBtnIter);
    myBtnResetColor1->setAction(myPlugin->getAction(StMoviePlayer::Action_ImageAdjustReset));
    myBtnResetColor1->setTexturePath(iconTexture(stCString("actionColorReset"), anIconSize));
    myBtnResetColor1->setDrawShadow(true);
    myBtnResetColor1->changeMargins() = aButtonMargins;
    myBtnResetColor1->setOpacity(0.0f, false);

    myBtnSepDx = new StGLRangeFieldFloat32(myAdjustOverlay, myImage->params.SeparationDX,
                                           0, 0, StGLCorner(ST_VCORNER_CENTER, ST_HCORNER_RIGHT),
                                           StGLRangeFieldFloat32::RangeStyle_Seekbar, scale(18));
    myBtnSepDx->changeRectPx().right() = myBtnSepDx->getRectPx().left() + aSlideWidth;
    myBtnSepDx->changeRectPx().moveTopTo(aCtrlStep * (aBtnIter++));
    myBtnSepDx->changeMargins().left   = scale(8);
    myBtnSepDx->changeMargins().right  = scale(8);
    myBtnSepDx->setCorner(StGLCorner(ST_VCORNER_TOP, ST_HCORNER_LEFT));
    myBtnSepDx->setFormat(stCString("DX Separation: %+01.0f"));

    myBtnSepDy = new StGLRangeFieldFloat32(myAdjustOverlay, myImage->params.SeparationDY,
                                           0, 0, StGLCorner(ST_VCORNER_CENTER, ST_HCORNER_RIGHT),
                                           StGLRangeFieldFloat32::RangeStyle_Seekbar, scale(18));
    myBtnSepDy->changeRectPx().right() = myBtnSepDy->getRectPx().left() + aSlideWidth;
    myBtnSepDy->changeRectPx().moveTopTo(aCtrlStep * (aBtnIter++));
    myBtnSepDy->changeMargins().left   = scale(8);
    myBtnSepDy->changeMargins().right  = scale(8);
    myBtnSepDy->setCorner(StGLCorner(ST_VCORNER_TOP, ST_HCORNER_LEFT));
    myBtnSepDy->setFormat(stCString("DY Separation: %+01.0f"));

    myBtnSepRot = new StGLRangeFieldFloat32(myAdjustOverlay, myImage->params.SeparationRot,
                                            0, 0, StGLCorner(ST_VCORNER_CENTER, ST_HCORNER_RIGHT),
                                            StGLRangeFieldFloat32::RangeStyle_Seekbar, scale(18));
    myBtnSepRot->changeRectPx().right() = myBtnSepRot->getRectPx().left() + aSlideWidth;
    myBtnSepRot->changeRectPx().moveTopTo(aCtrlStep * (aBtnIter++));
    myBtnSepRot->changeMargins().left   = scale(8);
    myBtnSepRot->changeMargins().right  = scale(8);
    myBtnSepRot->setCorner(StGLCorner(ST_VCORNER_TOP, ST_HCORNER_LEFT));
    myBtnSepRot->setFormat(stCString("Angular Sep.: %+01.2f"));

    myBtnResetColor2 = new StGLTextureButton(myAdjustOverlay, anIconStep * 1, aCtrlStep * aBtnIter);
    myBtnResetColor2->setAction(myPlugin->getAction(StMoviePlayer::Action_ImageAdjustReset));
    myBtnResetColor2->setTexturePath(iconTexture(stCString("actionColorReset"), anIconSize));
    myBtnResetColor2->setDrawShadow(true);
    myBtnResetColor2->changeMargins() = aButtonMargins;

    myBtnReset3d = new StGLTextureButton(myAdjustOverlay, anIconStep * 2, aCtrlStep * aBtnIter);
    myBtnReset3d->setAction(myImage->getActions()[StGLImageRegion::Action_Reset]);
    myBtnReset3d->setTexturePath(iconTexture(stCString("actionResetPlacement"), anIconSize));
    myBtnReset3d->setDrawShadow(true);
    myBtnReset3d->changeMargins() = aButtonMargins;
}

void StMoviePlayerGUI::doOpenFile(const size_t theFileType) {
    StString aTitle;
    switch(theFileType) {
        case StMovieOpenDialog::Dialog_Audio: {
            aTitle = tr(DIALOG_OPEN_AUDIO);
            break;
        }
        case StMovieOpenDialog::Dialog_Subtitles: {
            aTitle = tr(DIALOG_OPEN_SUBTITLES);
            break;
        }
        case StMovieOpenDialog::Dialog_SingleMovie:
        case StMovieOpenDialog::Dialog_DoubleMovie:
        default: {
            aTitle = tr(DIALOG_OPEN_FILE);
            break;
        }
    }

    StGLOpenFile* aDialog = new StGLOpenFile(this, aTitle, tr(BUTTON_CLOSE));
    const StString anSdCardPath = getResourceManager()->getFolder(StResourceManager::FolderId_SdCard);
    if(!anSdCardPath.isEmpty()) {
        StString aFolder, aName;
        StFileNode::getFolderAndFile(anSdCardPath, aFolder, aName);
        aDialog->addHotItem(anSdCardPath, stUtfTools::isInteger(aName) ? (StString("sdcard") + aName) : aName);
    }
    aDialog->addHotItem(getResourceManager()->getFolder(StResourceManager::FolderId_Downloads));
    aDialog->addHotItem(getResourceManager()->getFolder(StResourceManager::FolderId_Documents));
    aDialog->addHotItem(getResourceManager()->getFolder(StResourceManager::FolderId_Videos));
    aDialog->addHotItem(getResourceManager()->getFolder(StResourceManager::FolderId_Music));

    switch(theFileType) {
        case StMovieOpenDialog::Dialog_Audio: {
            aDialog->signals.onFileSelected = stSlot(myPlugin, &StMoviePlayer::doOpen1AudioFromGui);
            aDialog->setMimeList(myPlugin->myVideo->getMimeListAudio(), "Audio", false);
            break;
        }
        case StMovieOpenDialog::Dialog_Subtitles: {
            aDialog->signals.onFileSelected = stSlot(myPlugin, &StMoviePlayer::doOpen1SubtitleFromGui);
            aDialog->setMimeList(myPlugin->myVideo->getMimeListSubtitles(), "Subtitles", false);
            break;
        }
        case StMovieOpenDialog::Dialog_SingleMovie:
        case StMovieOpenDialog::Dialog_DoubleMovie:
        default: {
            aDialog->signals.onFileSelected = stSlot(myPlugin, &StMoviePlayer::doOpen1FileFromGui);
            aDialog->setDisplayExtra(myPlugin->params.ToMixImagesVideos->getValue());
            aDialog->setMimeList(myPlugin->myVideo->getMimeListVideo(),  "Videos", false);
            aDialog->setMimeList(myPlugin->myVideo->getMimeListImages(), "Images", true);
            aDialog->addHotCheckbox(myPlugin->params.ToMixImagesVideos, myPlugin->params.ToMixImagesVideos->getName());
            break;
        }
    }

    if(myPlugin->params.lastFolder.isEmpty()) {
        StHandle<StFileNode> aCurrFile = myPlugin->myPlayList->getCurrentFile();
        if(!aCurrFile.isNull()) {
            myPlugin->params.lastFolder = aCurrFile->isEmpty() ? aCurrFile->getFolderPath() : aCurrFile->getValue(0)->getFolderPath();
        }
    }
    aDialog->openFolder(myPlugin->params.lastFolder);
    setModalDialog(aDialog);
}

void StMoviePlayerGUI::doShowMobileExMenu(const size_t ) {
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
        anItem->setIcon(stCMenuIcon("actionDiscard"));

        const bool hasVideo = myPlugin->myVideo->hasVideoStream();
        const StHandle< StArrayList<StString> >& anAudioStreams = myPlugin->myVideo->params.activeAudio->getList();
        if(!anAudioStreams.isNull()
        && !anAudioStreams->isEmpty()) {
            anItem = aMenu->addItem(tr(MENU_AUDIO));
            anItem->setIcon(stCMenuIcon("actionStreamAudio"));
            anItem->signals.onItemClick += stSlot(this, &StMoviePlayerGUI::doAudioStreamsCombo);
        } else if(hasVideo) {
            anItem = aMenu->addItem(tr(MENU_AUDIO));
            anItem->setIcon(stCMenuIcon("actionStreamAudio"));
            //anItem->signals.onItemClick += stSlot(myPlugin,  &StMoviePlayer::doAddAudioStream);
            anItem->signals.onItemClick += stSlot(this, &StMoviePlayerGUI::doAudioStreamsCombo);
        }

        const StHandle< StArrayList<StString> >& aSubsStreams = myPlugin->myVideo->params.activeSubtitles->getList();
        if(!aSubsStreams.isNull()
        && !aSubsStreams->isEmpty()) {
            anItem = aMenu->addItem(tr(MENU_SUBTITLES));
            anItem->setIcon(stCMenuIcon("actionStreamSubtitles"));
            anItem->signals.onItemClick += stSlot(this, &StMoviePlayerGUI::doSubtitlesStreamsCombo);
        } else if(myPlugin->myVideo->hasAudioStream()
               || hasVideo) {
            anItem = aMenu->addItem(tr(MENU_SUBTITLES));
            anItem->setIcon(stCMenuIcon("actionStreamSubtitles"));
            //anItem->signals.onItemClick += stSlot(myPlugin,  &StMoviePlayer::doAddSubtitleStream);
            anItem->signals.onItemClick += stSlot(this, &StMoviePlayerGUI::doSubtitlesStreamsCombo);
        }

        if(hasVideo) {
            anItem = aMenu->addItem(tr(MENU_VIEW_DISPLAY_RATIO));
            anItem->setIcon(stCMenuIcon("actionDisplayRatio"));
            anItem->signals.onItemClick += stSlot(this, &StMoviePlayerGUI::doDisplayRatioCombo);
        }

        anExtraInfo.nullify();
    }
    anItem = aMenu->addItem(tr(MENU_HELP_ABOUT));
    anItem->setIcon(stCMenuIcon("actionHelp"));
    anItem->signals.onItemClick += stSlot(this, &StMoviePlayerGUI::doAboutProgram);
    anItem = aMenu->addItem(tr(MENU_HELP_SETTINGS));
    anItem->setIcon(stCMenuIcon("actionSettings"));
    anItem->signals.onItemClick += stSlot(this, &StMoviePlayerGUI::doMobileSettings);
    aMenu->stglInit();
    setFocus(aMenu);
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
  // upper toolbar
  myPanelUpper(NULL),
  myBtnOpen(NULL),
  myBtnInfo(NULL),
  myBtnAdjust(NULL),
  myBtnSwapLR(NULL),
  myBtnPanorama(NULL),
  myBtnSrcFrmt(NULL),
  myBtnAudio(NULL),
  myBtnSubs(NULL),
  // bottom toolbar
  myPanelBottom(NULL),
  mySeekBar(NULL),
  myVolumeBar(NULL),
  myVolumeLab(NULL),
  myBtnPlay(NULL),
  myTimeBox(NULL),
  myBtnVolume(NULL),
  myBtnPrev(NULL),
  myBtnNext(NULL),
  myBtnList(NULL),
  myBtnShuffle(NULL),
  myBtnLoop(NULL),
  myBtnFullScr(NULL),
  //
  myFpsWidget(NULL),
  myAdjustOverlay(NULL),
  myBtnSepDx(NULL),
  myBtnSepDy(NULL),
  myBtnSepRot(NULL),
  myBtnReset3d(NULL),
  myBtnResetColor1(NULL),
  myBtnResetColor2(NULL),
  myHKeysTable(NULL),
  //
  myIsVisibleGUI(true),
  myIsExperimental(myPlugin->params.ToShowExtra->getValue()),
  myIconStep(64),
  myBottomBarNbLeft(0),
  myBottomBarNbRight(0) {
    const GLfloat aScale = myPlugin->params.ScaleHiDPI2X->getValue() ? 2.0f : myPlugin->params.ScaleHiDPI ->getValue();
    setScale(aScale, (StGLRootWidget::ScaleAdjust )myPlugin->params.ScaleAdjust->getValue());
    setMobile(myPlugin->params.IsMobileUISwitch->getValue());

    myIconStep = isMobile() ? scale(56) : scale(64);

    myPlugin->params.ToShowFps->signals.onChanged.connect(this, &StMoviePlayerGUI::doShowFPS);

    myImage = new StGLImageRegion(this, theTextureQueue, false);
    myImage->changeIconPrev()->setTexturePath(iconTexture(stCString("actionBack"), scaleIcon(64)));
    myImage->changeIconPrev()->setDrawShadow(true);
    myImage->changeIconNext()->setTexturePath(iconTexture(stCString("actionNext"), scaleIcon(64)));
    myImage->changeIconNext()->setDrawShadow(true);
    myImage->signals.onOpenItem = stSlot(myPlugin, &StMoviePlayer::doFileNext);
    myImage->setPlayList(thePlayList);
    //myImage->setDragDelayMs(500.0);
    myImage->params.DisplayMode->setName(tr(MENU_VIEW_DISPLAY_MODE));
    myImage->params.DisplayMode->changeValues()[StGLImageRegion::MODE_STEREO]     = tr(MENU_VIEW_DISPLAY_MODE_STEREO);
    myImage->params.DisplayMode->changeValues()[StGLImageRegion::MODE_ONLY_LEFT]  = tr(MENU_VIEW_DISPLAY_MODE_LEFT);
    myImage->params.DisplayMode->changeValues()[StGLImageRegion::MODE_ONLY_RIGHT] = tr(MENU_VIEW_DISPLAY_MODE_RIGHT);
    myImage->params.DisplayMode->changeValues()[StGLImageRegion::MODE_PARALLEL]   = tr(MENU_VIEW_DISPLAY_MODE_PARALLEL);
    myImage->params.DisplayMode->changeValues()[StGLImageRegion::MODE_CROSSYED]   = tr(MENU_VIEW_DISPLAY_MODE_CROSSYED);
    myImage->params.ToHealAnamorphicRatio->setValue(true);
    myImage->params.ViewMode->signals.onChanged += stSlot(myPlugin, &StMoviePlayer::doSwitchViewMode);

    mySubtitles = new StGLSubtitles  (myImage, theSubQueue,
                                      myPlugin->params.SubtitlesPlace,
                                      myPlugin->params.SubtitlesSize);
    mySubtitles->params.TopDY    = myPlugin->params.SubtitlesTopDY;
    mySubtitles->params.BottomDY = myPlugin->params.SubtitlesBottomDY;
    mySubtitles->params.Parallax = myPlugin->params.SubtitlesParallax;
    mySubtitles->params.Parser   = myPlugin->params.SubtitlesParser;
    mySubtitles->params.ToApplyStereo = myPlugin->params.SubtitlesApplyStereo;

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
                                  bool theIsPreciseInput) {
    if(mySeekBar != NULL
    && myPanelBottom != NULL
    && myTimeBox->wasResized()) {
        stglResizeSeekBar();
    }

    if(myBtnAudio != NULL) {
        myBtnAudio->getTrackedValue()->setValue(myPlugin->params.AudioStream->getValue() != -1);
    }
    if(myBtnSubs != NULL) {
        myBtnSubs->getTrackedValue()->setValue(myPlugin->params.SubtitlesStream->getValue() != -1);
    }

    setVisibility(thePointZo);
    StGLRootWidget::stglUpdate(thePointZo, theIsPreciseInput);
    if(myVolumeBar != NULL) {
        char aBuff[128];
        stsprintf(aBuff, 128, "%+03.0f dB", myPlugin->params.AudioGain->getValue());
        myVolumeBar->setProgress(myPlugin->gainToVolume(myPlugin->params.AudioGain));
        myVolumeLab->setText(aBuff);
    }
    if(myDescr != NULL) {
        myDescr->setPoint(thePointZo);
    }

    if(myIsExperimental != myPlugin->params.ToShowExtra->getValue()) {
        StGLMenu::DeleteWithSubMenus(myMenuRoot); myMenuRoot = NULL;
        createMainMenu();
        myMenuRoot->stglUpdateSubmenuLayout();
        myIsExperimental = myPlugin->params.ToShowExtra->getValue();
        // turn back topmost position
        getChildren()->moveToTop(myMsgStack);
    }
}

void StMoviePlayerGUI::stglResize(const StGLBoxPx& theViewPort,
                                  const StMarginsI& theMargins,
                                  float theAspect) {
    const int aNewSizeX = theViewPort.width();
    const int aNewSizeY = theViewPort.height();
    int aGapTopX = 0, aGapTopY = 0, aGapBotX = 0, aGapBotY = 0;
    if(isMobile()) {
        // add gap for hidden system navigation buttons
        static const int THE_NAVIGATION_GAPX = 32;
        static const int THE_NAVIGATION_GAPY = 16;
        if(theAspect < 9.0 / 16.0 && theAspect > 0.0) {
            aGapTopY = aGapBotY = stMax(0, scale(stMin(THE_NAVIGATION_GAPY, int((1.0 / theAspect * 360) - 360 * 2))));
        } else if(theAspect > 9.0 / 16.0) {
            aGapTopX = aGapBotX = stMax(0, scale(stMin(THE_NAVIGATION_GAPX, int((theAspect * 360) - 360 * 2))));
        }
    } else {
        aGapTopY = scale(DISPL_Y_REGION_UPPER);
        aGapTopX = scale(DISPL_X_REGION_UPPER);
        aGapBotX = scale(DISPL_X_REGION_BOTTOM);
    }

    // image should fit entire view
    myImage->changeRectPx().top()    = -theMargins.top;
    myImage->changeRectPx().bottom() = -theMargins.top + aNewSizeY;
    myImage->changeRectPx().left()   = -theMargins.left;
    myImage->changeRectPx().right()  = -theMargins.left + aNewSizeX;

    if(myPanelUpper != NULL) {
        myPanelUpper->changeRectPx().top()   = aGapTopY;
        myPanelUpper->changeRectPx().left()  = aGapTopX;
        myPanelUpper->changeRectPx().right() = aGapTopX + stMax(aNewSizeX - theMargins.right - theMargins.left - 2 * aGapTopX, 2);
    }
    if(myPanelBottom != NULL) {
        myPanelBottom->changeRectPx().top()   = -aGapBotY;
        myPanelBottom->changeRectPx().left()  = aGapBotX;
        myPanelBottom->changeRectPx().right() = aGapBotX + stMax(aNewSizeX - theMargins.right - theMargins.left - 2 * aGapBotX, 2);
    }

    stglResizeSeekBar();
    StGLRootWidget::stglResize(theViewPort, theMargins, theAspect);
}

void StMoviePlayerGUI::stglResizeSeekBar() {
    if(mySeekBar != NULL
    && myPanelBottom != NULL) {
        const int aPanelSizeY = myPanelBottom->getRectPx().top() + myPanelBottom->getRectPx().height();
        const int aPanelSizeX = myPanelBottom->getRectPx().width();
        const int aSeekSizeY  = mySeekBar->getRectPx().height();
        const int aBoxWidth   = myTimeBox->getRectPx().width();
        const int anXSpace    = aPanelSizeX - (myBottomBarNbLeft + myBottomBarNbRight) * myIconStep;
        const int anXSpace2   = anXSpace - aBoxWidth;
        const int anXOffset   = scale(isMobile() ? 24 : 12);
        const int anMinXSpace = scale(isMobile() ? 250 : 400);
        myTimeBox->changeRectPx().moveTopTo(0);
        if(anXSpace >= anMinXSpace) {
            // normal mode
            if(myPlayList != NULL && isMobile()) {
                myPlayList->changeFitMargins().bottom = scale(56);
            }
            mySeekBar->changeRectPx().moveTopTo((aPanelSizeY - aSeekSizeY) / 2);
            mySeekBar->changeRectPx().left()  = anXOffset + myBottomBarNbLeft * myIconStep;
            mySeekBar->changeRectPx().right() = aPanelSizeX - anXOffset - myBottomBarNbRight * myIconStep;
            if(anXSpace2 >= anMinXSpace) {
                // wide mode
                mySeekBar->changeRectPx().right() -= aBoxWidth;
                myTimeBox->changeRectPx().moveLeftTo(myBottomBarNbRight * (-myIconStep));
                myTimeBox->setOverlay(false);
            } else {
                myTimeBox->changeRectPx().moveLeftTo(myBottomBarNbRight * (-myIconStep) - anXSpace / 2 + aBoxWidth / 2);
                myTimeBox->setOverlay(true);
            }
        } else {
            // narrow mode
            if(myPlayList != NULL && isMobile()) {
                myPlayList->changeFitMargins().bottom = scale(100);
            }
            mySeekBar->changeRectPx().moveTopTo(-aSeekSizeY);
            mySeekBar->changeRectPx().left()  = anXOffset;
            mySeekBar->changeRectPx().right() = aPanelSizeX - anXOffset;
            if(anXSpace2 >= 0) {
                myTimeBox->changeRectPx().moveLeftTo(myBottomBarNbRight * (-myIconStep) - anXSpace / 2 + aBoxWidth / 2);
                myTimeBox->setOverlay(false);
            } else {
                myTimeBox->changeRectPx().moveTopTo(-aPanelSizeY + (aPanelSizeY - aSeekSizeY) / 2);
                myTimeBox->changeRectPx().moveLeftTo(-aPanelSizeX / 2 + aBoxWidth / 2);
                myTimeBox->setOverlay(true);
            }
        }
    }
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
            && theWidget->isVisibleWithParents()
            && theWidget->isPointIn(theCursorZo);
    }

}

void StMoviePlayerGUI::doGesture(const StGestureEvent& theEvent) {
    if(myImage == NULL) {
        return;
    }

    if(theEvent.Type == stEvent_Gesture1Tap) {
        myTapTimer.restart();
    } else if(theEvent.Type == stEvent_Gesture1DoubleTap) {
        myTapTimer.stop();
    }

    for(StGLWidget *aChildIter(getChildren()->getLast()), *aChildActive(NULL); aChildIter != NULL;) {
        aChildActive = aChildIter;
        aChildIter   = aChildIter->getPrev();
        if(aChildActive->isVisibleAndPointIn(getCursorZo())) {
            if(aChildActive == myImage) {
                myImage->doGesture(theEvent);
            }
            return;
        }
    }
}

void StMoviePlayerGUI::setVisibility(const StPointD_t& theCursor,
                                     bool theToForceHide,
                                     bool theToForceShow) {
    const bool toShowAdjust   = myPlugin->params.ToShowAdjustImage->getValue();
    const bool toShowPlayList = myPlugin->params.ToShowPlayList->getValue();
    const bool hasMainMenu    = myPlugin->params.ToShowMenu->getValue()
                             && myMenuRoot != NULL;
    const bool hasUpperPanel  = myPlugin->params.ToShowTopbar->getValue()
                             && myPanelUpper != NULL;
    const bool hasBottomPanel = myPlugin->params.ToShowBottom->getValue()
                             && (myPanelBottom != NULL || mySeekBar != NULL);

    const int  aRootSizeY     = getRectPx().height();
    const bool hasVideo       = myPlugin->myVideo->hasVideoStream();
    if(!hasVideo && !myTapTimer.isOn()
    && !myPlugin->myPlayList->isEmpty()) {
        myEmptyTimer.restart();
    } else {
        myEmptyTimer.stop();
    }
    if(myEmptyTimer.getElapsedTime() >= 2.5) {
        myVisibilityTimer.restart();
        myEmptyTimer.stop();
    }
    if(myTapTimer.getElapsedTime() >= 0.5) {
        myVisibilityTimer.restart();
        myTapTimer.stop();
    }
    if(theToForceShow) {
        myVisibilityTimer.restart();
    } else if(theToForceHide) {
        myVisibilityTimer.restart(THE_VISIBILITY_IDLE_TIME + 0.001);
    }
    const bool isMouseActive  = myWindow->isMouseMoved();
    const double aStillTime   = myVisibilityTimer.getElapsedTime();

    StHandle<StStereoParams> aParams = myImage->getSource();
    StFormat aSrcFormat = (StFormat )myPlugin->params.SrcStereoFormat->getValue();
    if( aSrcFormat == StFormat_AUTO
    && !aParams.isNull()
    &&  hasVideo) {
        aSrcFormat = aParams->StereoFormat;
    }
    if(!aParams.isNull()
     && myImage->params.SwapLR->getValue()
     && hasVideo) {
        aSrcFormat = st::formatReversed(aSrcFormat);
    }
    const bool has3dInput = hasVideo
                         && aSrcFormat != StFormat_Mono
                         && aSrcFormat != StFormat_AUTO;

    myIsVisibleGUI = isMouseActive
        || aParams.isNull()
        || aStillTime < THE_VISIBILITY_IDLE_TIME
        || (hasUpperPanel && myPanelUpper->isPointIn(theCursor))
        || (hasBottomPanel && myPanelBottom != NULL
         && int(aRootSizeY * theCursor.y()) > (aRootSizeY - 2 * myPanelBottom->getRectPx().height())
         && theCursor.y() < 1.0)
        || (hasBottomPanel && mySeekBar != NULL && mySeekBar->isPointIn(theCursor))
        || (hasBottomPanel && myPlayList != NULL && toShowPlayList && myPlayList->isPointIn(theCursor))
        || (hasMainMenu           && myMenuRoot->isActive());
    if(!myIsVisibleGUI
     && myBtnPlay != NULL
     && myBtnPlay->getFaceId() == 0
     && (theCursor.x() < 0.0 || theCursor.x() > 1.0
      || theCursor.y() < 0.0 || theCursor.y() > 1.0)) {
        myIsVisibleGUI = true;
    }
    const float anOpacity = (float )myVisLerp.perform(myIsVisibleGUI, theToForceHide || theToForceShow);
    if(isMouseActive) {
        myVisibilityTimer.restart();
    }

    if(myMenuRoot != NULL) {
        myMenuRoot->setOpacity(hasMainMenu ? anOpacity : 0.0f, false);
    }
    if(myPanelUpper != NULL) {
        myPanelUpper->setOpacity(hasUpperPanel ? anOpacity : 0.0f, true);
    }
    if(mySeekBar != NULL) {
        mySeekBar->setOpacity(hasBottomPanel ? anOpacity : 0.0f, false);
    }
    if(myPanelBottom != NULL) {
        myPanelBottom->setOpacity(hasBottomPanel ? anOpacity : 0.0f, true);
    }

    if(myAdjustOverlay != NULL
    && toShowAdjust) {
        myAdjustOverlay->setOpacity(anOpacity, true);
        if(!has3dInput) {
            myBtnSepDx  ->setOpacity(0.0f, false);
            myBtnSepDy  ->setOpacity(0.0f, false);
            myBtnSepRot ->setOpacity(0.0f, false);
            myBtnReset3d->setOpacity(0.0f, false);
            myBtnResetColor2->setOpacity(0.0f, false);
        } else {
            myBtnResetColor1->setOpacity(0.0f, false);
        }
    }
    if(myPlayList != NULL
    && toShowPlayList) {
        myPlayList->setOpacity(hasBottomPanel ? anOpacity : 0.0f, true);
    }

    const StPlayList::CurrentPosition aCurrPos = myPlugin->myPlayList->getCurrentPosition();
    if(myBtnPrev != NULL) {
        myBtnPrev->setOpacityScale(aCurrPos == StPlayList::CurrentPosition_Middle
                                || aCurrPos == StPlayList::CurrentPosition_Last ? 1.0f : 0.5f);
    }
    if(myBtnNext != NULL) {
        myBtnNext->setOpacityScale(aCurrPos == StPlayList::CurrentPosition_Middle
                                || aCurrPos == StPlayList::CurrentPosition_First ? 1.0f : 0.5f);
    }
    if(myBtnPlay != NULL) {
        myBtnPlay->setOpacityScale(aCurrPos != StPlayList::CurrentPosition_NONE ? 1.0f : 0.5f);
    }

    if(myBtnSrcFrmt != NULL) {
        size_t aFaceId = aSrcFormat;
        if(aSrcFormat == StFormat_AUTO) {
            aFaceId = hasVideo ? StFormat_Mono : StFormat_NB;
        }
        myBtnSrcFrmt->setOpacityScale(hasVideo ? 1.0f : 0.5f);
        myBtnSrcFrmt->setFaceId(aFaceId);
    }
    if(myBtnFullScr != NULL) {
        int aFirstFace = 0;
        if(myWindow->isStereoFullscreenOnly() && myWindow->isStereoOutput()) {
            aFirstFace = 2;
        }
        myBtnFullScr->setFaceId(aFirstFace + (myPlugin->params.IsFullscreen->getValue() ? 1 : 0));
    }
    if(myBtnSwapLR != NULL) {
        myBtnSwapLR->setOpacity(has3dInput ? anOpacity : 0.0f, false);
    }

    ///

    const StViewSurface aViewMode = hasVideo && !aParams.isNull()
                                  ? aParams->ViewingMode
                                  : StViewSurface_Plain;
    bool toShowPano = aViewMode != StViewSurface_Plain;
    if(!toShowPano
    &&  hasVideo
    && !aParams.isNull()
    /*&&  st::probePanorama(aParams->StereoFormat,
                          aParams->Src1SizeX, aParams->Src1SizeY,
                          aParams->Src2SizeX, aParams->Src2SizeY) != StPanorama_OFF*/) {
        toShowPano = true;
    }
    if(myBtnPanorama != NULL) {
        myBtnPanorama->getTrackedValue()->setValue(aViewMode != StViewSurface_Plain);
        myBtnPanorama->setOpacity(toShowPano ? anOpacity : 0.0f, false);
    }
    myWindow->setTrackOrientation(aViewMode != StViewSurface_Plain
                               && myPlugin->params.ToTrackHead->getValue());
    StQuaternion<double> aQ = myWindow->getDeviceOrientation();
    myImage->setDeviceOrientation(StGLQuaternion((float )aQ.x(), (float )aQ.y(), (float )aQ.z(), (float )aQ.w()));

    if(myDescr != NULL) {
        bool wasEmpty = myDescr->getText().isEmpty();
        if(::isPointIn(myBtnOpen, theCursor)) {
            myDescr->setText(tr(FILE_VIDEO_OPEN));
        } else if(::isPointIn(myBtnInfo,   theCursor)) {
            myDescr->setText(tr(MENU_MEDIA_FILE_INFO));
        } else if(::isPointIn(myBtnSwapLR, theCursor)) {
            size_t aLngId = myImage->params.SwapLR->getValue() ? SWAP_LR_ON : SWAP_LR_OFF;
            myDescr->setText(tr(aLngId));
        } else if(::isPointIn(myBtnSrcFrmt, theCursor)) {
            myDescr->setText(tr(BTN_SRC_FORMAT) + "\n" + trSrcFormat(aSrcFormat));
        } else if(::isPointIn(myBtnPanorama, theCursor)) {
            size_t aTrPano = MENU_VIEW_SURFACE_PLANE;
            switch(aViewMode) {
                case StViewSurface_Plain:      aTrPano = MENU_VIEW_SURFACE_PLANE;   break;
                case StViewSurface_Theater:    aTrPano = MENU_VIEW_SURFACE_THEATER; break;
                case StViewSurface_Sphere:     aTrPano = MENU_VIEW_SURFACE_SPHERE;  break;
                case StViewSurface_Hemisphere: aTrPano = MENU_VIEW_SURFACE_HEMISPHERE;  break;
                case StViewSurface_Cubemap:    aTrPano = MENU_VIEW_SURFACE_CUBEMAP;  break;
                case StViewSurface_CubemapEAC: aTrPano = MENU_VIEW_SURFACE_CUBEMAP_EAC; break;
                case StViewSurface_Cylinder:   aTrPano = MENU_VIEW_SURFACE_CYLINDER; break;
            }
            myDescr->setText(tr(MENU_VIEW_PANORAMA) + "\n" + tr(aTrPano));
        } else if(::isPointIn(myBtnAdjust, theCursor)) {
            myDescr->setText(tr(MENU_VIEW_IMAGE_ADJUST));
        } else if(::isPointIn(myBtnAudio, theCursor)) {
            myDescr->setText(tr(MENU_AUDIO));
        } else if(::isPointIn(myBtnSubs, theCursor)) {
            myDescr->setText(tr(MENU_SUBTITLES));
        } else if(::isPointIn(myBtnVolume, theCursor)) {
            myDescr->setText("Mute");
        } else if(::isPointIn(myBtnPlay, theCursor)) {
            myDescr->setText(tr(VIDEO_PLAYPAUSE));
        } else if(::isPointIn(myBtnPrev, theCursor)) {
            myDescr->setText(tr(VIDEO_LIST_PREV));
        } else if(::isPointIn(myBtnNext, theCursor)) {
            myDescr->setText(tr(VIDEO_LIST_NEXT));
        } else if(::isPointIn(myBtnList, theCursor)) {
            myDescr->setText(tr(VIDEO_LIST));
        } else if(::isPointIn(myBtnShuffle, theCursor)) {
            myDescr->setText(tr(MENU_MEDIA_SHUFFLE));
        } else if(::isPointIn(myBtnLoop, theCursor)) {
            myDescr->setText("Loop single item");
        } else if(::isPointIn(myBtnFullScr, theCursor)) {
            myDescr->setText(tr(FULLSCREEN));
        } else {
            myDescr->setText("");
        }

        if(wasEmpty
        && aStillTime < 1.0) {
            myDescr->setText("");
        } else if(getFocus() != NULL
               || (myMenuRoot != NULL && myMenuRoot->isActive())) {
            // hide within active dialog - should be replaced by z-layer check
            myDescr->setText("");
        }

        myDescr->setOpacity(!myDescr->getText().isEmpty() ? 1.0f : 0.0f, false);
    }
}

void StMoviePlayerGUI::doAudioStreamsCombo(const size_t ) {
    const StHandle< StArrayList<StString> >& aStreams = myPlugin->myVideo->params.activeAudio->getList();
    const bool hasVideo = myPlugin->myVideo->hasVideoStream();

    StGLCombobox::ListBuilder aBuilder(this);
    if(hasVideo || aStreams.isNull() || aStreams->isEmpty()) {
        aBuilder.getMenu()->addItem(tr(MENU_AUDIO_NONE), myPlugin->params.AudioStream, -1);
    }
    if(!aStreams.isNull()) {
        bool hasQuadAudio = false;
        for(size_t aStreamId = 0; aStreamId < aStreams->size(); ++aStreamId) {
            const StString& aStreamName = aStreams->getValue(aStreamId);
            aBuilder.getMenu()->addItem(aStreamName, myPlugin->params.AudioStream, int32_t(aStreamId));
            hasQuadAudio = hasQuadAudio
                        || aStreamName.isContains(stCString("quad,"))
                        || aStreamName.isContains(stCString("4.0,"));
        }
        if(hasQuadAudio) {
            aBuilder.getMenu()->addItem(myPlugin->params.ToForceBFormat);
        }
    }

    //aBuilder.getMenu()->addSplitter();
    StGLMenuItem* aDelayItem = NULL;
    StGLRangeFieldFloat32* aDelayRange = NULL;
    if(hasVideo) {
        if(!aStreams.isNull()
        && !aStreams->isEmpty()) {
            aDelayItem = aBuilder.getMenu()->addItem(tr(MENU_AUDIO_DELAY));
            aDelayItem->changeMargins().right = scale(100 + 16);
            aDelayItem->signals.onItemClick.connect(this, &StMoviePlayerGUI::doAudioDelay);
            aDelayRange = new StGLRangeFieldFloat32(aDelayItem, myPlugin->params.AudioDelay,
                                                    -scale(16), 0, StGLCorner(ST_VCORNER_CENTER, ST_HCORNER_RIGHT));
            aDelayRange->changeRectPx().bottom() = aDelayRange->getRectPx().top()  + aBuilder.getMenu()->getItemHeight();
            aDelayRange->setFormat(stCString("%+01.3f"));
            aDelayRange->setColor(StGLRangeFieldFloat32::FieldColor_Default,  aBlack);
            aDelayRange->setColor(StGLRangeFieldFloat32::FieldColor_Positive, aGreen);
            aDelayRange->setColor(StGLRangeFieldFloat32::FieldColor_Negative, aRed);
        }
        aBuilder.getMenu()->addItem(tr(MENU_AUDIO_ATTACH))
                          ->setIcon(stCMenuIcon("actionOpen"), false)
                          ->signals.onItemClick.connect(myPlugin, &StMoviePlayer::doAddAudioStream);
    }

    aBuilder.display();
}

void StMoviePlayerGUI::doSubtitlesPlacement(const size_t ) {
    StGLMenu* aMenu  = new StGLMenu(this, 0, 0, StGLMenu::MENU_VERTICAL, true);
    aMenu->setCorner(StGLCorner(ST_VCORNER_BOTTOM, ST_HCORNER_RIGHT));
    aMenu->setContextual(true);
    fillSubtitlesFontSize(aMenu);
    fillSubtitlesPlacement(aMenu);
    aMenu->stglInit();
    setFocus(aMenu);
}

void StMoviePlayerGUI::fillSubtitlesFontSize(StGLMenu* theMenu) {
    StGLMenuItem* anItem = theMenu->addItem(tr(MENU_SUBTITLES_SIZE));
    anItem->setIcon(stCMenuIcon("actionFontSize"), false);
    anItem->changeMargins().right = scale(100 + 16);
    StGLRangeFieldFloat32* aRange = new StGLRangeFieldFloat32(anItem, myPlugin->params.SubtitlesSize,
                                                              -scale(16), 0, StGLCorner(ST_VCORNER_CENTER, ST_HCORNER_RIGHT));
    aRange->changeRectPx().bottom() = aRange->getRectPx().top() + theMenu->getItemHeight();
    aRange->setFormat(stCString("%02.0f"));
    aRange->setColor(StGLRangeFieldFloat32::FieldColor_Default,  aBlack);
    aRange->setColor(StGLRangeFieldFloat32::FieldColor_Positive, aBlack);
    aRange->setColor(StGLRangeFieldFloat32::FieldColor_Negative, aBlack);
}

void StMoviePlayerGUI::fillSubtitlesPlacement(StGLMenu* theMenu) {
    StGLMenuItem* anItem = theMenu->addItem(tr(MENU_SUBTITLES_TOP), myPlugin->params.SubtitlesPlace, ST_VCORNER_TOP);
    anItem->changeMargins().right  = scale(100 + 16);
    StGLRangeFieldFloat32* aRange = new StGLRangeFieldFloat32(anItem, myPlugin->params.SubtitlesTopDY,
                                                              -scale(16), 0, StGLCorner(ST_VCORNER_CENTER, ST_HCORNER_RIGHT));
    aRange->changeRectPx().bottom() = aRange->getRectPx().top() + theMenu->getItemHeight();
    aRange->setFormat(stCString("%+03.0f"));
    aRange->setColor(StGLRangeFieldFloat32::FieldColor_Default,  aBlack);
    aRange->setColor(StGLRangeFieldFloat32::FieldColor_Positive, aGreen);
    aRange->setColor(StGLRangeFieldFloat32::FieldColor_Negative, aRed);

    anItem = theMenu->addItem(tr(MENU_SUBTITLES_BOTTOM), myPlugin->params.SubtitlesPlace, ST_VCORNER_BOTTOM);
    anItem->changeMargins().right  = scale(100 + 16);
    aRange = new StGLRangeFieldFloat32(anItem, myPlugin->params.SubtitlesBottomDY,
                                       -scale(16), 0, StGLCorner(ST_VCORNER_CENTER, ST_HCORNER_RIGHT));
    aRange->changeRectPx().bottom() = aRange->getRectPx().top() + theMenu->getItemHeight();
    aRange->setFormat(stCString("%+03.0f"));
    aRange->setColor(StGLRangeFieldFloat32::FieldColor_Default,  aBlack);
    aRange->setColor(StGLRangeFieldFloat32::FieldColor_Positive, aGreen);
    aRange->setColor(StGLRangeFieldFloat32::FieldColor_Negative, aRed);

    anItem = theMenu->addItem(tr(MENU_SUBTITLES_PARALLAX));
    anItem->changeMargins().right  = scale(100 + 16);
    aRange = new StGLRangeFieldFloat32(anItem, myPlugin->params.SubtitlesParallax,
                                       -scale(16), 0, StGLCorner(ST_VCORNER_CENTER, ST_HCORNER_RIGHT));
    aRange->changeRectPx().bottom() = aRange->getRectPx().top() + theMenu->getItemHeight();
    aRange->setFormat(stCString("%+03.0f"));
    aRange->setColor(StGLRangeFieldFloat32::FieldColor_Default,  aBlack);
    aRange->setColor(StGLRangeFieldFloat32::FieldColor_Positive, aBlack);
    aRange->setColor(StGLRangeFieldFloat32::FieldColor_Negative, aBlack);
}

void StMoviePlayerGUI::doSubtitlesStreamsCombo(const size_t ) {
    const StHandle< StArrayList<StString> >& aStreams = myPlugin->myVideo->params.activeSubtitles->getList();
    const bool hasAudioStream = myPlugin->myVideo->hasAudioStream();
    const bool hasVideoStream = myPlugin->myVideo->hasVideoStream();

    StGLCombobox::ListBuilder aBuilder(this);

    // create text parser menu
    StGLMenu* aParserMenu = NULL;
    if(!myWindow->isMobile()) {
        aParserMenu = new StGLMenu(this, 0, 0, StGLMenu::MENU_VERTICAL);
        for(size_t anIter = 0; anIter < myPlugin->params.SubtitlesParser->getValues().size(); ++anIter) {
            aParserMenu->addItem(myPlugin->params.SubtitlesParser->getValues()[anIter], myPlugin->params.SubtitlesParser, (int32_t )anIter);
        }
    }

    aBuilder.getMenu()->addItem(tr(MENU_SUBTITLES_NONE), myPlugin->params.SubtitlesStream, -1);
    if(!aStreams.isNull()) {
        for(size_t aStreamId = 0; aStreamId < aStreams->size(); ++aStreamId) {
            aBuilder.getMenu()->addItem(aStreams->getValue(aStreamId), myPlugin->params.SubtitlesStream, int32_t(aStreamId));
        }
    }

    if(!aStreams.isNull()
    && !aStreams->isEmpty()) {
        //myMenuSubtitles->addSplitter();
        if(!myWindow->isMobile()) {
            fillSubtitlesFontSize(aBuilder.getMenu());
        }

        StGLMenu* aPlaceMenu = new StGLMenu(this, 0, 0, StGLMenu::MENU_VERTICAL);
        fillSubtitlesPlacement(aPlaceMenu);
        aBuilder.getMenu()->addItem(tr(MENU_SUBTITLES_PLACEMENT), aPlaceMenu)
                          ->signals.onItemClick.connect(this, &StMoviePlayerGUI::doSubtitlesPlacement);
    }

    if(!aStreams.isNull()
    && !aStreams->isEmpty()
    && hasVideoStream) {
        StHandle<StStereoParams> aParams = myImage->getSource();
        StFormat aSrcFormat = (StFormat )myPlugin->params.SrcStereoFormat->getValue();
        if(aSrcFormat == StFormat_AUTO && !aParams.isNull()) {
            aSrcFormat = aParams->StereoFormat;
        }
        if(aSrcFormat == StFormat_SideBySide_LR
        || aSrcFormat == StFormat_SideBySide_RL
        || aSrcFormat == StFormat_TopBottom_LR
        || aSrcFormat == StFormat_TopBottom_RL) {
            aBuilder.getMenu()->addItem(tr(MENU_SUBTITLES_STEREO), myPlugin->params.SubtitlesApplyStereo);
        }
    }
    if(!myWindow->isMobile()) {
        aBuilder.getMenu()->addItem(tr(MENU_SUBTITLES_PARSER), aParserMenu)
                          ->setIcon(stCMenuIcon("actionTextFormat"), false);
    }
    if(hasAudioStream || hasVideoStream) {
        //aBuilder.getMenu()->addSplitter();
        aBuilder.getMenu()->addItem(tr(MENU_SUBTITLES_ATTACH))
                          ->setIcon(stCMenuIcon("actionOpen"), false)
                          ->signals.onItemClick.connect(myPlugin, &StMoviePlayer::doAddSubtitleStream);
    }

    aBuilder.display();
}

void StMoviePlayerGUI::stglDraw(unsigned int theView) {
    setLensDist(myPlugin->getMainWindow()->getLensDist());
    if((theView == ST_DRAW_LEFT || theView == ST_DRAW_MONO)
    && myFpsWidget != NULL) {
        myImage->getTextureQueue()->getQueueInfo(myFpsWidget->changePlayQueued(),
                                                 myFpsWidget->changePlayQueueLength(),
                                                 myFpsWidget->changePlayFps());
        myFpsWidget->update(myPlugin->getMainWindow()->isStereoOutput(),
                            myPlugin->getMainWindow()->getTargetFps(),
                            myPlugin->getMainWindow()->getStatistics());
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
    setModalDialog(aDialog);
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
    aParams.add(myImage->params.DisplayMode);
    aRend->getOptions(aParams);
    aParams.add(myPlugin->params.ToShowFps);
    aParams.add(myLangMap->params.language);
    aParams.add(myPlugin->params.IsMobileUI);

    const StString aTitle  = tr(MENU_HELP_HOTKEYS);
    StInfoDialog*  aDialog = new StInfoDialog(myPlugin, this, aTitle, scale(650), scale(300));

    std::map< int, StHandle<StAction> >& anActionsMap = myPlugin->changeActions();

    StGLTable* aTable = new StGLTable(aDialog->getContent(), 0, 0, StGLCorner(ST_VCORNER_TOP, ST_HCORNER_CENTER));
    aTable->changeItemMargins().top    = scale(4);
    aTable->changeItemMargins().bottom = scale(4);
    aTable->setupTable((int )anActionsMap.size(), 3);

    StHandle< StSlot<void (const size_t )> > aSlot1 = new StSlotMethod<StMoviePlayerGUI, void (const size_t )>(this, &StMoviePlayerGUI::doChangeHotKey1);
    StHandle< StSlot<void (const size_t )> > aSlot2 = new StSlotMethod<StMoviePlayerGUI, void (const size_t )>(this, &StMoviePlayerGUI::doChangeHotKey2);
    aTable->fillFromHotKeys(anActionsMap, *myLangMap, aSlot1, aSlot2);
    myHKeysTable = aTable;

    aDialog->addButton(tr(BUTTON_DEFAULTS), false)->signals.onBtnClick = stSlot(this, &StMoviePlayerGUI::doResetHotKeys);
    aDialog->addButton(tr(BUTTON_CLOSE), true);
    aDialog->stglInit();
    setModalDialog(aDialog);
}

void StMoviePlayerGUI::doChangeHotKey1(const size_t theId) {
    const StHandle<StAction>& anAction = myPlugin->getAction((int )theId);
    StHotKeyControl* aKeyChanger = new StHotKeyControl(myPlugin, myHKeysTable, this, anAction, 1);
    aKeyChanger->stglInit();
}

void StMoviePlayerGUI::doChangeHotKey2(const size_t theId) {
    const StHandle<StAction>& anAction = myPlugin->getAction((int )theId);
    StHotKeyControl* aKeyChanger = new StHotKeyControl(myPlugin, myHKeysTable, this, anAction, 2);
    aKeyChanger->stglInit();
}

void StMoviePlayerGUI::doMobileSettings(const size_t ) {
    const StHandle<StWindow>& aRend = myPlugin->getMainWindow();
    StParamsList aParams;
    aParams.add(myPlugin->StApplication::params.ActiveDevice);
    aParams.add(myImage->params.DisplayMode);
    aParams.add(myPlugin->params.ToStickPanorama);
    aParams.add(myPlugin->params.ToSwapJPS);
    aRend->getOptions(aParams);
    aParams.add(myPlugin->params.ToShowFps);
    aParams.add(myPlugin->params.UseGpu);
    if(myPlugin->hasAlHrtf()) {
        aParams.add(myPlugin->params.AudioAlHrtf);
    }

    if(avcodec_find_decoder_by_name("libopenjpeg") != NULL) {
        aParams.add(myPlugin->params.UseOpenJpeg);
    }

    aParams.add(myLangMap->params.language);
    aParams.add(myPlugin->params.ToMixImagesVideos);
    aParams.add(myPlugin->params.SlideShowDelay);
    aParams.add(myPlugin->params.IsMobileUI);
#if defined(_WIN32) || defined(__APPLE__) // implemented only on Windows and macOS
    aParams.add(myPlugin->params.IsExclusiveFullScreen);
#endif
    aParams.add(myPlugin->params.ToSmoothUploads);
    if(isMobile()) {
        //aParams.add(myPlugin->params.ToHideStatusBar);
        aParams.add(myPlugin->params.ToHideNavBar);
    }
    aParams.add(myPlugin->params.ExitOnEscape);
    if(!isMobile()) {
        aParams.add(myPlugin->params.ToShowExtra);
    }
    if(!isMobile()) {
        aParams.add(myPlugin->params.BlockSleeping);
        aParams.add(myPlugin->params.ToOpenLast);
    }
#if defined(ST_UPDATES_CHECK)
    aParams.add(myPlugin->params.CheckUpdatesDays);
#endif

    if(!myWindow->isMobile()) {
        StHandle<StBoolParamNamed> aDefDrawerParam = myPlugin->createDefaultDrawerParam(stCString("StMoviePlayer"),
                                                                                        stCString("sView launcher starts Movie Player"));
        aParams.add(aDefDrawerParam);
    }

    StInfoDialog* aDialog = new StInfoDialog(myPlugin, this, tr(MENU_HELP_SETTINGS), scale(768), scale(300));

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
    setModalDialog(aDialog);
}
