/**
 * Copyright © 2009-2013 Kirill Gavrilov <kirill@sview.ru>
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
#include <StSocket/StSocket.h>
#include <StSettings/StEnumParam.h>

#include <StGLWidgets/StGLButton.h>
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
    static const int ICON_WIDTH            = 64;

    static const StGLVec3 aBlack(0.0f, 0.0f, 0.0f);
    static const StGLVec3 aGreen(0.4f, 0.8f, 0.4f);
    static const StGLVec3 aRed  (1.0f, 0.0f, 0.0f);

};

/**
 * Create upper toolbar
 */
void StMoviePlayerGUI::createUpperToolbar() {
    int aBtnIter = 0;
    const int aTop  = scale(DISPL_Y_REGION_UPPER);
    const int aLeft = scale(DISPL_X_REGION_UPPER);

    const StRectI_t& aMargins = getRootMarginsPx();
    myPanelUpper = new StGLWidget(this, aMargins.left(), aMargins.top(), StGLCorner(ST_VCORNER_TOP, ST_HCORNER_LEFT), scale(4096), scale(128));

    // append the textured buttons
    myBtnOpen = new StGLTextureButton(myPanelUpper, aLeft + (aBtnIter++) * ICON_WIDTH, aTop);
    myBtnOpen->signals.onBtnClick.connect(myPlugin, &StMoviePlayer::doOpen1File);
    myBtnOpen->setTexturePath(stCTexture("openImage.png"));

    myBtnSwapLR = new StGLCheckboxTextured(myPanelUpper, myImage->params.swapLR,
                                           stCTexture("swapLRoff.png"),
                                           stCTexture("swapLRon.png"),
                                           aLeft + (aBtnIter++) * ICON_WIDTH, aTop,
                                           StGLCorner(ST_VCORNER_TOP, ST_HCORNER_LEFT));

    StGLSwitchTextured* aSrcBtn = new StGLSwitchTextured(myPanelUpper, myPlugin->params.srcFormat,
                                                         aLeft + (aBtnIter++) * ICON_WIDTH, aTop,
                                                         StGLCorner(ST_VCORNER_TOP, ST_HCORNER_LEFT));

    aSrcBtn->addItem(ST_V_SRC_AUTODETECT,    stCTexture("srcFrmtAuto.png"));
    aSrcBtn->addItem(ST_V_SRC_MONO,          stCTexture("srcFrmtMono.png"));
    aSrcBtn->addItem(ST_V_SRC_ROW_INTERLACE, stCTexture("srcFrmtInterlace.png"));
    aSrcBtn->addItem(ST_V_SRC_SIDE_BY_SIDE,  stCTexture("srcFrmtSideBySide.png"));
    aSrcBtn->addItem(ST_V_SRC_PARALLEL_PAIR, stCTexture("srcFrmtSideBySide.png"), true);
    aSrcBtn->addItem(ST_V_SRC_OVER_UNDER_LR, stCTexture("srcFrmtOverUnder.png"));
    aSrcBtn->addItem(ST_V_SRC_OVER_UNDER_RL, stCTexture("srcFrmtOverUnder.png"),  true);
    myBtnSrcFrmt = aSrcBtn;
}

/**
 * Create bottom toolbar
 */
void StMoviePlayerGUI::createBottomToolbar() {
    const StRectI_t& aMargins = getRootMarginsPx();
    const int aTop  = scale(DISPL_Y_REGION_BOTTOM);
    const int aLeft = scale(DISPL_X_REGION_BOTTOM);
    myPanelBottom = new StGLWidget(this, aMargins.left(), -aMargins.bottom(), StGLCorner(ST_VCORNER_BOTTOM, ST_HCORNER_LEFT), scale(4096), scale(128));

    // append the textured buttons
    myBtnPlay = new StGLTextureButton(myPanelBottom, aLeft, aTop,
                                      StGLCorner(ST_VCORNER_TOP, ST_HCORNER_LEFT), 2);
    myBtnPlay->signals.onBtnClick.connect(myPlugin, &StMoviePlayer::doPlayPause);
    const StString aPaths[2] = { stCTexture("moviePlay.png"), stCTexture("moviePause.png") };
    myBtnPlay->setTexturePath(aPaths, 2);

    myTimeBox = new StTimeBox(myPanelBottom, aLeft + 1 * ICON_WIDTH, aTop);
    myTimeBox->setTexturePath(stCTexture("timebox.png"));
    myBtnPrev = new StGLTextureButton(myPanelBottom, -aLeft - 3 * ICON_WIDTH, aTop,
                                      StGLCorner(ST_VCORNER_TOP, ST_HCORNER_RIGHT));
    myBtnPrev->signals.onBtnClick.connect(myPlugin, &StMoviePlayer::doListPrev);
    myBtnPrev->setTexturePath(stCTexture("moviePrior.png"));

    myBtnNext = new StGLTextureButton(myPanelBottom, -aLeft - 2 * ICON_WIDTH, aTop,
                                      StGLCorner(ST_VCORNER_TOP, ST_HCORNER_RIGHT));
    myBtnNext->signals.onBtnClick.connect(myPlugin, &StMoviePlayer::doListNext);
    myBtnNext->setTexturePath(stCTexture("movieNext.png"));

    myBtnList = new StGLTextureButton(myPanelBottom, -aLeft - ICON_WIDTH, aTop,
                                      StGLCorner(ST_VCORNER_TOP, ST_HCORNER_RIGHT));
    myBtnList->signals.onBtnClick.connect(myPlugin, &StMoviePlayer::doPlayListReverse);
    myBtnList->setTexturePath(stCTexture("moviePlaylist.png"));

    myBtnFullScr = new StGLTextureButton(myPanelBottom, -aLeft, aTop,
                                         StGLCorner(ST_VCORNER_TOP, ST_HCORNER_RIGHT));
    myBtnFullScr->signals.onBtnClick.connect(myPlugin->params.isFullscreen.operator->(), &StBoolParam::doReverse);
    myBtnFullScr->setTexturePath(stCTexture("movieFullScr.png"));
}

/**
 * Main menu
 */
void StMoviePlayerGUI::createMainMenu() {
    const StRectI_t& aMargins = getRootMarginsPx();
    myMenuRoot = new StGLMenu(this, aMargins.left(), aMargins.top(), StGLMenu::MENU_HORIZONTAL, true);

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
    aMenuMedia->addItem(tr(MENU_MEDIA_AL_DEVICE),  myMenuOpenAL);

    aMenuMedia->addItem("Audio Volume", aMenuVolume);

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
    const IconSize anIconSize = scaleIcon(16);
    StGLMenu* aMenu = new StGLMenu(this, 0, 0, StGLMenu::MENU_VERTICAL);
    aMenu->addItem(tr(MENU_SRC_FORMAT_AUTO),         myPlugin->params.srcFormat, ST_V_SRC_AUTODETECT)
         ->setIcon(iconTexture(stCString("menuAuto"), anIconSize));
    aMenu->addItem(tr(MENU_SRC_FORMAT_MONO),         myPlugin->params.srcFormat, ST_V_SRC_MONO)
         ->setIcon(iconTexture(stCString("menuMono"), anIconSize));
    aMenu->addItem(tr(MENU_SRC_FORMAT_CROSS_EYED),   myPlugin->params.srcFormat, ST_V_SRC_SIDE_BY_SIDE)
         ->setIcon(iconTexture(stCString("menuSbsRL"), anIconSize));
    aMenu->addItem(tr(MENU_SRC_FORMAT_PARALLEL),     myPlugin->params.srcFormat, ST_V_SRC_PARALLEL_PAIR)
         ->setIcon(iconTexture(stCString("menuSbsLR"), anIconSize));
    aMenu->addItem(tr(MENU_SRC_FORMAT_OVERUNDER_RL), myPlugin->params.srcFormat, ST_V_SRC_OVER_UNDER_RL)
         ->setIcon(iconTexture(stCString("menuOverUnderRL"), anIconSize));
    aMenu->addItem(tr(MENU_SRC_FORMAT_OVERUNDER_LR), myPlugin->params.srcFormat, ST_V_SRC_OVER_UNDER_LR)
         ->setIcon(iconTexture(stCString("menuOverUnderLR"), anIconSize));
    aMenu->addItem(tr(MENU_SRC_FORMAT_INTERLACED),   myPlugin->params.srcFormat, ST_V_SRC_ROW_INTERLACE)
         ->setIcon(iconTexture(stCString("menuRowLR"), anIconSize));
    aMenu->addItem(tr(MENU_SRC_FORMAT_ANA_RC),       myPlugin->params.srcFormat, ST_V_SRC_ANAGLYPH_RED_CYAN)
         ->setIcon(iconTexture(stCString("menuRedCyanLR"), anIconSize));
    aMenu->addItem(tr(MENU_SRC_FORMAT_ANA_RB),       myPlugin->params.srcFormat, ST_V_SRC_ANAGLYPH_G_RB)
         ->setIcon(iconTexture(stCString("menuGreenMagentaLR"), anIconSize));
    aMenu->addItem(tr(MENU_SRC_FORMAT_ANA_YB),       myPlugin->params.srcFormat, ST_V_SRC_ANAGLYPH_YELLOW_BLUE)
         ->setIcon(iconTexture(stCString("menuYellowBlueLR"), anIconSize));
    aMenu->addItem(tr(MENU_SRC_FORMAT_PAGEFLIP),     myPlugin->params.srcFormat, ST_V_SRC_PAGE_FLIP)
         ->setIcon(iconTexture(stCString("menuFrameSeqLR"), anIconSize));
    aMenu->addItem(tr(MENU_SRC_FORMAT_TILED_4X),     myPlugin->params.srcFormat, ST_V_SRC_TILED_4X)
         ->setIcon(iconTexture(stCString("menuTiledLR"), anIconSize));
    return aMenu;
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
    aMenuView->addItem(tr(MENU_VIEW_FULLSCREEN),    myPlugin->params.isFullscreen);
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
    aMenu->addItem(tr(MENU_VIEW_DISPLAY_MODE_STEREO),
                   myImage->params.displayMode, StGLImageRegion::MODE_STEREO);
    aMenu->addItem(tr(MENU_VIEW_DISPLAY_MODE_LEFT),
                   myImage->params.displayMode, StGLImageRegion::MODE_ONLY_LEFT);
    aMenu->addItem(tr(MENU_VIEW_DISPLAY_MODE_RIGHT),
                   myImage->params.displayMode, StGLImageRegion::MODE_ONLY_RIGHT);
    aMenu->addItem(tr(MENU_VIEW_DISPLAY_MODE_PARALLEL),
                   myImage->params.displayMode, StGLImageRegion::MODE_PARALLEL);
    aMenu->addItem(tr(MENU_VIEW_DISPLAY_MODE_CROSSYED),
                   myImage->params.displayMode, StGLImageRegion::MODE_CROSSYED);
    return aMenu;
}

/**
 * Root -> View menu -> Display Ratio
 */
StGLMenu* StMoviePlayerGUI::createDisplayRatioMenu() {
    const IconSize anIconSize = scaleIcon(16);
    StGLMenu* aMenu = new StGLMenu(this, 0, 0, StGLMenu::MENU_VERTICAL);
    aMenu->addItem("Source", myImage->params.displayRatio, StGLImageRegion::RATIO_AUTO)
         ->setIcon(iconTexture(stCString("menuAuto"), anIconSize));
    aMenu->addItem("2.21:1", myImage->params.displayRatio, StGLImageRegion::RATIO_221_1)
         ->setIcon(iconTexture(stCString("menuRatio2_1_"), anIconSize));
    aMenu->addItem("16:9",   myImage->params.displayRatio, StGLImageRegion::RATIO_16_9)
         ->setIcon(iconTexture(stCString("menuRatio16_9_"), anIconSize));
    aMenu->addItem("16:10",  myImage->params.displayRatio, StGLImageRegion::RATIO_16_10)
         ->setIcon(iconTexture(stCString("menuRatio16_10_"), anIconSize));
    aMenu->addItem("4:3",    myImage->params.displayRatio, StGLImageRegion::RATIO_4_3)
         ->setIcon(iconTexture(stCString("menuRatio4_3_"), anIconSize));
    aMenu->addItem("5:4",    myImage->params.displayRatio, StGLImageRegion::RATIO_5_4)
         ->setIcon(iconTexture(stCString("menuRatio5_4_"), anIconSize));
    aMenu->addItem("1:1",    myImage->params.displayRatio, StGLImageRegion::RATIO_1_1)
         ->setIcon(iconTexture(stCString("menuRatio1_1_"), anIconSize));
    aMenu->addItem("Keep on restart", myPlugin->params.toRestoreRatio);
    return aMenu;
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
    anItem->setMarginRight(scale(100 + 16));
    StGLRangeFieldFloat32* aRange = new StGLRangeFieldFloat32(anItem, myImage->params.gamma,
                                                              -scale(16), 0, StGLCorner(ST_VCORNER_CENTER, ST_HCORNER_RIGHT));
    aRange->setFormat(stCString("%+01.2f"));
    aRange->setColor(StGLRangeFieldFloat32::FieldColor_Default,  aBlack);
    aRange->setColor(StGLRangeFieldFloat32::FieldColor_Positive, aGreen);
    aRange->setColor(StGLRangeFieldFloat32::FieldColor_Negative, aRed);
    aRange->setVisibility(true, true);

    anItem = aMenu->addItem(tr(MENU_VIEW_ADJUST_BRIGHTNESS));
    anItem->setMarginRight(scale(100 + 16));
    aRange = new StGLRangeFieldFloat32(anItem, myImage->params.brightness,
                                       -scale(16), 0, StGLCorner(ST_VCORNER_CENTER, ST_HCORNER_RIGHT));
    aRange->setFormat(stCString("%+01.2f"));
    aRange->setColor(StGLRangeFieldFloat32::FieldColor_Default,  aBlack);
    aRange->setColor(StGLRangeFieldFloat32::FieldColor_Positive, aGreen);
    aRange->setColor(StGLRangeFieldFloat32::FieldColor_Negative, aRed);
    aRange->setVisibility(true, true);

    anItem = aMenu->addItem(tr(MENU_VIEW_ADJUST_SATURATION));
    anItem->setMarginRight(scale(100 + 16));
    aRange = new StGLRangeFieldFloat32(anItem, myImage->params.saturation,
                                       -scale(16), 0, StGLCorner(ST_VCORNER_CENTER, ST_HCORNER_RIGHT));
    aRange->changeRectPx().bottom() = aRange->getRectPx().top() + aMenu->getItemHeight();
    aRange->setFormat(stCString("%+01.2f"));
    aRange->setColor(StGLRangeFieldFloat32::FieldColor_Default,  aBlack);
    aRange->setColor(StGLRangeFieldFloat32::FieldColor_Positive, aGreen);
    aRange->setColor(StGLRangeFieldFloat32::FieldColor_Negative, aRed);
    aRange->setVisibility(true, true);
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
    anItem->setMarginRight(scale(110 + 16));
    StGLRangeFieldFloat32* aRange = new StGLRangeFieldFloat32(anItem, myPlugin->params.AudioGain,
                                                              -scale(16), 0, StGLCorner(ST_VCORNER_CENTER, ST_HCORNER_RIGHT));
    aRange->setFormat(stCString("%+03.0f dB"));
    aRange->setColor(StGLRangeFieldFloat32::FieldColor_Default,  aBlack);
    aRange->setColor(StGLRangeFieldFloat32::FieldColor_Positive, aGreen);
    aRange->setColor(StGLRangeFieldFloat32::FieldColor_Negative, aRed);
    aRange->setVisibility(true, true);

    aMenu->addItem("Mute", myPlugin->params.AudioMute);
    return aMenu;
}

/**
 * Dialog for Audio/Video synchronization control.
 */
class StDelayControl : public StGLMessageBox {

        public:

    StDelayControl(StMoviePlayerGUI*               theParent,
                   const StHandle<StFloat32Param>& theTrackedValue)
    : StGLMessageBox(theParent, "", theParent->scale(400), theParent->scale(260)),
      myRange(NULL) {
        changeRectPx().moveX( myRoot->scale( 64));
        changeRectPx().moveY(-myRoot->scale(128));
        setCorner(StGLCorner(ST_VCORNER_BOTTOM, ST_HCORNER_LEFT));
        StGLButton* aResetBtn = addButton(theParent->tr(BUTTON_RESET));
        addButton(theParent->tr(BUTTON_CLOSE));
        setVisibility(true, true);

        StGLWidget* aContent = new StGLWidget(getContent(), 0, 0, StGLCorner(ST_VCORNER_TOP, ST_HCORNER_LEFT),
                                              getContent()->getRectPx().width(), getContent()->getRectPx().height());
        aContent->setVisibility(true, true);

        const StGLVec3 aWhite(1.0f, 1.0f, 1.0f);
        StGLTextArea* aTitle = new StGLTextArea(aContent, 0, 0, StGLCorner(ST_VCORNER_TOP, ST_HCORNER_LEFT),
                                                aContent->getRectPx().width(), myRoot->scale(10));
        aTitle->setupAlignment(StGLTextFormatter::ST_ALIGN_X_CENTER, StGLTextFormatter::ST_ALIGN_Y_TOP);
        aTitle->setText(theParent->tr(DIALOG_AUDIO_DELAY_TITLE));
        aTitle->setTextColor(aWhite);
        aTitle->setVisibility(true, true);
        aTitle->stglInitAutoHeight();

        StGLTextArea* aText = new StGLTextArea(aContent, 0, aTitle->getRectPx().bottom(), StGLCorner(ST_VCORNER_TOP, ST_HCORNER_LEFT),
                                               aContent->getRectPx().width(), myRoot->scale(10));
        aText->setText(StString("\n\n") + theParent->tr(DIALOG_AUDIO_DELAY_DESC) + "\n");
        aText->setTextColor(aWhite);
        aText->setVisibility(true, true);
        aText->stglInitAutoHeight();

        StGLTextArea* aLabel = new StGLTextArea(aContent, 0, aText->getRectPx().bottom(), StGLCorner(ST_VCORNER_TOP, ST_HCORNER_LEFT),
                                                -myRoot->scale(1), myRoot->scale(10));
        aLabel->setText(theParent->tr(DIALOG_AUDIO_DELAY_LABEL));
        aLabel->setTextColor(aWhite);
        aLabel->setVisibility(true, true);
        aLabel->stglInitAutoHeightWidth();

        myRange = new StGLRangeFieldFloat32(aContent, theTrackedValue,
                                            aLabel->getRectPx().right() + myRoot->scale(10), aLabel->getRectPx().top());
        myRange->setFormat(stCString("%+01.3f"));
        myRange->setColor(StGLRangeFieldFloat32::FieldColor_Default,  StGLVec3(1.0f, 1.0f, 1.0f));
        myRange->setColor(StGLRangeFieldFloat32::FieldColor_Positive, StGLVec3(0.4f, 0.8f, 0.4f));
        myRange->setColor(StGLRangeFieldFloat32::FieldColor_Negative, StGLVec3(1.0f, 0.0f, 0.0f));
        myRange->setVisibility(true, true);
        myRange->stglInit();

        StGLTextArea* aLabUnits = new StGLTextArea(aContent, myRange->getRectPx().right() + myRoot->scale(10), aLabel->getRectPx().top(),
                                                   StGLCorner(ST_VCORNER_TOP, ST_HCORNER_LEFT), -myRoot->scale(1), myRoot->scale(10));
        aLabUnits->setText(theParent->tr(DIALOG_AUDIO_DELAY_UNITS));
        aLabUnits->setTextColor(aWhite);
        aLabUnits->setVisibility(true, true);
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
    StGLMessageBox* aDialog = new StGLMessageBox(this,
          tr(ABOUT_DPLUGIN_NAME) + '\n'
        + tr(ABOUT_VERSION) + ": " + StVersionInfo::getSDKVersionString()
        + " "+ StThread::getArchString()
        + "\n \n" + tr(ABOUT_DESCRIPTION),
        scale(512), scale(300));
    aDialog->addButton(tr(BUTTON_CLOSE));
    aDialog->setVisibility(true, true);
    aDialog->stglInit();
}

void StMoviePlayerGUI::doUserTips(const size_t ) {
    StSocket::openURL("http://sview.ru/sview2009/usertips");
}

void StMoviePlayerGUI::doAboutSystem(const size_t ) {
    StString aTitle = "System Info";
    StString anInfo = getContext().stglFullInfo();
    StString aString = aTitle + "\n\n \n" + anInfo;
    StGLMessageBox* aDialog = new StGLMessageBox(this, aString, scale(512), scale(256));
    aDialog->addButton(tr(BUTTON_CLOSE));
    aDialog->setVisibility(true, true);
    aDialog->stglInit();
}

void StMoviePlayerGUI::doAboutFile(const size_t ) {
    StHandle<StFileNode>     aFileNode;
    StHandle<StStereoParams> aParams;
    StHandle<StMovieInfo>    anExtraInfo;
    StString                 aCodecsInfo;
    StArrayList<StString>    anInfoList(10);
    if(myPlugin->getCurrentFile(aFileNode, aParams, anExtraInfo) && !anExtraInfo.isNull()) {
        for(size_t aKeyIter = 0; aKeyIter < anExtraInfo->myInfo.size(); ++aKeyIter) {
            const StArgument& aPair = anExtraInfo->myInfo.getFromIndex(aKeyIter);
            anInfoList.add(aPair.getKey() + ": " + aPair.getValue() + "\n");
        }

        aCodecsInfo = "\nActive codecs\n \n";
        for(size_t aKeyIter = 0; aKeyIter < anExtraInfo->myCodecs.size(); ++aKeyIter) {
            const StArgument& aPair = anExtraInfo->myCodecs.getFromIndex(aKeyIter);
            if(!aPair.getValue().isEmpty()) {
                aCodecsInfo += aPair.getValue() + "\n";
            }
        }
    }

    StString aTitle = "File Info";
    StString anInfo;
    for(size_t anIter = 0; anIter < anInfoList.size(); ++anIter) {
        anInfo += anInfoList[anIter];
    }
    StString aString = aTitle + "\n\n \n" + anInfo + aCodecsInfo;
    StGLMessageBox* aDialog = new StGLMessageBox(this, aString, scale(512), scale(300));
    aDialog->addButton(tr(BUTTON_CLOSE));
    aDialog->setVisibility(true, true);
    aDialog->stglInit();
}

void StMoviePlayerGUI::doCheckUpdates(const size_t ) {
    StSocket::openURL("http://www.sview.ru/download");
}

void StMoviePlayerGUI::doOpenLicense(const size_t ) {
    StSocket::openURL(StProcess::getStShareFolder()
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
    StGLMenu* aMenuCheckUpdates = createCheckUpdatesMenu(); // Root -> Help -> Check updates menu
    StGLMenu* aMenuLanguage     = createLanguageMenu();     // Root -> Help -> Language menu

    aMenu->addItem(tr(MENU_HELP_ABOUT))
         ->signals.onItemClick.connect(this, &StMoviePlayerGUI::doAboutProgram);
    aMenu->addItem(tr(MENU_HELP_USERTIPS))
         ->signals.onItemClick.connect(this, &StMoviePlayerGUI::doUserTips);
    aMenu->addItem(tr(MENU_HELP_LICENSE))
         ->signals.onItemClick.connect(this, &StMoviePlayerGUI::doOpenLicense);
    aMenu->addItem(tr(MENU_HELP_SYSINFO))
         ->signals.onItemClick.connect(this, &StMoviePlayerGUI::doAboutSystem);
    aMenu->addItem(tr(MENU_HELP_EXPERIMENTAL), myPlugin->params.ToShowExtra);
    aMenu->addItem(tr(MENU_HELP_SCALE),        aMenuScale);
    aMenu->addItem(tr(MENU_HELP_BLOCKSLP),     aMenuBlockSleep);
    aMenu->addItem(tr(MENU_HELP_UPDATES),      aMenuCheckUpdates);
    aMenu->addItem(tr(MENU_HELP_LANGS),        aMenuLanguage);
    return aMenu;
}

/**
 * Root -> Help -> Check updates menu
 */
StGLMenu* StMoviePlayerGUI::createScaleMenu() {
    StGLMenu* aMenu = new StGLMenu(this, 0, 0, StGLMenu::MENU_VERTICAL);
    aMenu->addItem(tr(MENU_HELP_SCALE_SMALL),   myPlugin->params.ScaleAdjust,  StGLRootWidget::ScaleAdjust_Small);
    aMenu->addItem(tr(MENU_HELP_SCALE_NORMAL),  myPlugin->params.ScaleAdjust,  StGLRootWidget::ScaleAdjust_Normal);
    aMenu->addItem(tr(MENU_HELP_SCALE_BIG),     myPlugin->params.ScaleAdjust,  StGLRootWidget::ScaleAdjust_Big);
    aMenu->addItem(tr(MENU_HELP_SCALE_HIDPI2X), myPlugin->params.ScaleHiDPI2X);
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

StMoviePlayerGUI::StMoviePlayerGUI(StMoviePlayer*  thePlugin,
                                   StWindow*       theWindow,
                                   StTranslations* theLangMap,
                                   const StHandle<StPlayList>&       thePlayList,
                                   const StHandle<StGLTextureQueue>& theTextureQueue,
                                   const StHandle<StSubQueue>&       theSubQueue)
: StGLRootWidget(),
  myPlugin(thePlugin),
  myWindow(theWindow),
  myLangMap(theLangMap),
  myTexturesFolder(StProcess::getStShareFolder() + "textures" + SYS_FS_SPLITTER),
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
  //
  myIsVisibleGUI(true),
  myIsExperimental(myPlugin->params.ToShowExtra->getValue()) {
    const GLfloat aScale = myPlugin->params.ScaleHiDPI2X->getValue() ? 2.0f : myPlugin->params.ScaleHiDPI ->getValue();
    StGLRootWidget::setScale(aScale, (StGLRootWidget::ScaleAdjust )myPlugin->params.ScaleAdjust->getValue());

    setRootMarginsPx(myWindow->getMargins());
    const StRectI_t& aMargins = getRootMarginsPx();
    myPlugin->params.ToShowFps->signals.onChanged.connect(this, &StMoviePlayerGUI::doShowFPS);
    myImage     = new StGLImageRegion(this, theTextureQueue, false);
    mySubtitles = new StGLSubtitles  (this, theSubQueue,
                                      myPlugin->params.SubtitlesSize,
                                      myPlugin->params.SubtitlesParallax);

    createUpperToolbar();

    mySeekBar = new StSeekBar(this, -aMargins.bottom() - scale(78));
    mySeekBar->signals.onSeekClick.connect(myPlugin, &StMoviePlayer::doSeek);

    createBottomToolbar();

    myDescr = new StGLDescription(this);

    myPlayList = new StGLPlayList(this, thePlayList);
    myPlayList->setCorner(StGLCorner(ST_VCORNER_CENTER, ST_HCORNER_RIGHT));
    myPlayList->setVisibility(myPlugin->params.ToShowPlayList->getValue(), true);
    myPlayList->signals.onOpenItem = stSlot(myPlugin, &StMoviePlayer::doFileNext);

    // create main menu
    createMainMenu();

    myMsgStack = new StGLMsgStack(this, myPlugin->getMessagesQueue());
    myMsgStack->setVisibility(true, true);

    if(myPlugin->params.ToShowFps->getValue()) {
        myFpsWidget = new StGLFpsLabel(this);
        myFpsWidget->setVisibility(true, true);
    }
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

    const StRectI_t& aMargins = myWindow->getMargins();
    const bool areNewMargins = aMargins != getRootMarginsPx();
    if(areNewMargins) {
        setRootMarginsPx(aMargins);
    }

    if(myPanelUpper != NULL) {
        myPanelUpper->changeRectPx().right()  = stMax(theRectPx.width() - aMargins.right(), 2);
    }
    if(myPanelBottom != NULL) {
        myPanelBottom->changeRectPx().right() = stMax(theRectPx.width() - aMargins.right(), 2);
    }

    if(areNewMargins) {
        if(myPanelUpper != NULL) {
            myPanelUpper->changeRectPx().left() = aMargins.left();
            myPanelUpper->changeRectPx().moveTopTo(aMargins.top());
        }
        if(myPanelBottom != NULL) {
            myPanelBottom->changeRectPx().left() =  aMargins.left();
            myPanelBottom->changeRectPx().moveTopTo(-aMargins.bottom());
        }
        if(mySeekBar != NULL) {
            mySeekBar->changeRectPx().moveTopTo(-aMargins.bottom() - scale(78));
        }
        if(myMenuRoot != NULL) {
            myMenuRoot->changeRectPx().left() = aMargins.left();
            myMenuRoot->changeRectPx().top()  = aMargins.top();
            myMenuRoot->stglUpdateSubmenuLayout();
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

void StMoviePlayerGUI::setVisibility(const StPointD_t& theCursor,
                                     bool              theIsMouseMoved) {
    const bool toShowPlayList = myPlugin->params.ToShowPlayList->getValue();
    myIsVisibleGUI = theIsMouseMoved
        || myVisibilityTimer.getElapsedTime() < 2.0
        || (myPanelUpper  != NULL && myPanelUpper ->isPointIn(theCursor))
        || (myPanelBottom != NULL && myPanelBottom->isPointIn(theCursor))
        || (mySeekBar     != NULL && mySeekBar    ->isPointIn(theCursor))
        || (toShowPlayList        && myPlayList   ->isPointIn(theCursor))
        || (myMenuRoot    != NULL && myMenuRoot->isActive());
    if(theIsMouseMoved) {
        myVisibilityTimer.restart();
    }

    // always visible
    StGLRootWidget::setVisibility(true, true);
    myImage->setVisibility(true, true);
    mySubtitles->setVisibility(true, true);

    if(myMenuRoot != NULL) {
        myMenuRoot->setVisibility(myIsVisibleGUI, false);
    }

    if(mySeekBar != NULL) {
        mySeekBar->setVisibility(myIsVisibleGUI);
    }

    if(myPanelUpper != NULL) {
        myPanelUpper->setVisibility(myIsVisibleGUI);
        for(StGLWidget* child = myPanelUpper->getChildren()->getStart(); child != NULL; child = child->getNext()) {
            child->setVisibility(myIsVisibleGUI);
        }
    }

    if(myPanelBottom != NULL) {
        myPanelBottom->setVisibility(myIsVisibleGUI);
        for(StGLWidget* child = myPanelBottom->getChildren()->getStart(); child != NULL; child = child->getNext()) {
            child->setVisibility(myIsVisibleGUI);
        }
    }

    if(toShowPlayList) {
        myPlayList->setVisibility(myIsVisibleGUI, false);
    }

    if(myDescr != NULL) {
        myDescr->setVisibility(true, true);
        if(myBtnOpen->isPointIn(theCursor)) {
            myDescr->setText(tr(FILE_VIDEO_OPEN));
        } else if(myBtnSwapLR->isPointIn(theCursor)) {
            size_t aLngId = myImage->params.swapLR->getValue() ? SWAP_LR_ON : SWAP_LR_OFF;
            myDescr->setText(tr(aLngId));
        } else if(myBtnSrcFrmt->isPointIn(theCursor)) {
            size_t aLngId = MENU_SRC_FORMAT_AUTO;
            StFormatEnum aSrcFormat = (StFormatEnum )myPlugin->params.srcFormat->getValue();
            if(aSrcFormat == ST_V_SRC_AUTODETECT
            && !myImage->params.stereoFile.isNull()) {
                aSrcFormat = myImage->params.stereoFile->getSrcFormat();
            }
            switch(aSrcFormat) {
                case ST_V_SRC_MONO:                 aLngId = MENU_SRC_FORMAT_MONO; break;
                case ST_V_SRC_SIDE_BY_SIDE:         aLngId = MENU_SRC_FORMAT_CROSS_EYED; break;
                case ST_V_SRC_PARALLEL_PAIR:        aLngId = MENU_SRC_FORMAT_PARALLEL;   break;
                case ST_V_SRC_OVER_UNDER_RL:        aLngId = MENU_SRC_FORMAT_OVERUNDER_RL; break;
                case ST_V_SRC_OVER_UNDER_LR:        aLngId = MENU_SRC_FORMAT_OVERUNDER_LR; break;
                case ST_V_SRC_ROW_INTERLACE:        aLngId = MENU_SRC_FORMAT_INTERLACED;   break;
                case ST_V_SRC_ANAGLYPH_G_RB:        aLngId = MENU_SRC_FORMAT_ANA_RB;   break;
                case ST_V_SRC_ANAGLYPH_RED_CYAN:    aLngId = MENU_SRC_FORMAT_ANA_RC;   break;
                case ST_V_SRC_ANAGLYPH_YELLOW_BLUE: aLngId = MENU_SRC_FORMAT_ANA_YB;   break;
                case ST_V_SRC_PAGE_FLIP:            aLngId = MENU_SRC_FORMAT_PAGEFLIP; break;
                case ST_V_SRC_TILED_4X:             aLngId = MENU_SRC_FORMAT_TILED_4X; break;
                case ST_V_SRC_SEPARATE_FRAMES:      aLngId = MENU_SRC_FORMAT_SEPARATE; break;
                default:
                case ST_V_SRC_AUTODETECT:           aLngId = MENU_SRC_FORMAT_AUTO; break;
            }
            myDescr->setText(tr(BTN_SRC_FORMAT) + tr(aLngId));
        } else if(myBtnPlay->isPointIn(theCursor)) {
            myDescr->setText(tr(VIDEO_PLAYPAUSE));
        } else if(myBtnPrev->isPointIn(theCursor)) {
            myDescr->setText(tr(VIDEO_LIST_PREV));
        } else if(myBtnNext->isPointIn(theCursor)) {
            myDescr->setText(tr(VIDEO_LIST_NEXT));
        } else if(myBtnList->isPointIn(theCursor)) {
            myDescr->setText(tr(VIDEO_LIST));
        } else if(myBtnFullScr->isPointIn(theCursor)) {
            myDescr->setText(tr(FULLSCREEN));
        } else {
            myDescr->setVisibility(false, true);
        }
    }
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
            aDelayItem->setMarginRight(scale(100 + 16));
            aDelayItem->signals.onItemClick.connect(this, &StMoviePlayerGUI::doAudioDelay);
            aDelayRange = new StGLRangeFieldFloat32(aDelayItem, myPlugin->params.AudioDelay,
                                                    -scale(16), 0, StGLCorner(ST_VCORNER_CENTER, ST_HCORNER_RIGHT));
            aDelayRange->changeRectPx().bottom() = aDelayRange->getRectPx().top()  + myMenuAudio->getItemHeight();
            aDelayRange->setFormat(stCString("%+01.3f"));
            aDelayRange->setColor(StGLRangeFieldFloat32::FieldColor_Default,  StGLVec3(0.0f, 0.0f, 0.0f));
            aDelayRange->setColor(StGLRangeFieldFloat32::FieldColor_Positive, StGLVec3(0.4f, 0.8f, 0.4f));
            aDelayRange->setColor(StGLRangeFieldFloat32::FieldColor_Negative, StGLVec3(1.0f, 0.0f, 0.0f));
            aDelayRange->setVisibility(true, true);
        }
        myMenuAudio->addItem(tr(MENU_AUDIO_ATTACH))
                   ->signals.onItemClick.connect(myPlugin, &StMoviePlayer::doAddAudioStream);
    }

    // update menu representation
    myMenuAudio->stglInit();
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
        anItem->setMarginRight(scale(100 + 16));
        StGLRangeFieldFloat32* aRange = new StGLRangeFieldFloat32(anItem, myPlugin->params.SubtitlesSize,
                                                                 -scale(16), 0, StGLCorner(ST_VCORNER_CENTER, ST_HCORNER_RIGHT));
        aRange->changeRectPx().bottom() = aRange->getRectPx().top() + myMenuSubtitles->getItemHeight();
        aRange->setFormat(stCString("%02.0f"));
        aRange->setColor(StGLRangeFieldFloat32::FieldColor_Default,  aBlack);
        aRange->setColor(StGLRangeFieldFloat32::FieldColor_Positive, aBlack);
        aRange->setColor(StGLRangeFieldFloat32::FieldColor_Negative, aBlack);
        aRange->setVisibility(true, true);

        anItem = myMenuSubtitles->addItem(tr(MENU_SUBTITLES_PARALLAX));
        anItem->setMarginRight(scale(100 + 16));
        aRange = new StGLRangeFieldFloat32(anItem, myPlugin->params.SubtitlesParallax,
                                          -scale(16), 0, StGLCorner(ST_VCORNER_CENTER, ST_HCORNER_RIGHT));
        aRange->changeRectPx().bottom() = aRange->getRectPx().top() + myMenuSubtitles->getItemHeight();
        aRange->setFormat(stCString("%+03.0f"));
        aRange->setColor(StGLRangeFieldFloat32::FieldColor_Default,  aBlack);
        aRange->setColor(StGLRangeFieldFloat32::FieldColor_Positive, aBlack);
        aRange->setColor(StGLRangeFieldFloat32::FieldColor_Negative, aBlack);
        aRange->setVisibility(true, true);
    }

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
    if(theView == ST_DRAW_LEFT
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
    myFpsWidget->setVisibility(true, true);
    myFpsWidget->stglInit();
}

void StMoviePlayerGUI::doAboutRenderer(const size_t ) {
    StString anAboutText = myPlugin->getMainWindow()->getRendererAbout();
    if(anAboutText.isEmpty()) {
        anAboutText = StString() + "Plugin '" + myPlugin->getMainWindow()->getRendererId() + "' doesn't provide description";
    }

    StGLMessageBox* aDialog = new StGLMessageBox(this, anAboutText, scale(512), scale(300));
    aDialog->addButton(tr(BUTTON_CLOSE));
    aDialog->setVisibility(true, true);
    aDialog->stglInit();
}

void StMoviePlayerGUI::showUpdatesNotify() {
    StGLMessageBox* aDialog = new StGLMessageBox(this, tr(UPDATES_NOTIFY));
    aDialog->addButton(tr(BUTTON_CLOSE));
    aDialog->setVisibility(true, true);
    aDialog->stglInit();
}
