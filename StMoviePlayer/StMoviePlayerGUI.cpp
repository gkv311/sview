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
#if (defined(__APPLE__))
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

using namespace StMoviePlayerStrings;

namespace {
    static const int DISPL_Y_REGION_UPPER  = 32;
    static const int DISPL_X_REGION_UPPER  = 32;
    static const int DISPL_X_REGION_BOTTOM = 52;
    static const int DISPL_Y_REGION_BOTTOM = 64;
    static const int ICON_WIDTH            = 64;
};

/**
 * Create upper toolbar
 */
void StMoviePlayerGUI::createUpperToolbar() {
    int i = 0;

    const StRectI_t& aMargins = getRootMarginsPx();
    upperRegion = new StGLWidget(this, aMargins.left(), aMargins.top(), StGLCorner(ST_VCORNER_TOP, ST_HCORNER_LEFT), 4096, 128);

    // append the textured buttons
    btnOpen     = new StGLTextureButton(upperRegion, DISPL_X_REGION_UPPER + (i++) * ICON_WIDTH, DISPL_Y_REGION_UPPER);
    btnOpen->signals.onBtnClick.connect(myPlugin, &StMoviePlayer::doOpen1File);

    btnSwapLR   = new StGLCheckboxTextured(upperRegion, stImageRegion->params.swapLR,
                                           texturesPathRoot + "swapLRoff.std",
                                           texturesPathRoot + "swapLRon.std",
                                           DISPL_X_REGION_UPPER + (i++) * ICON_WIDTH, DISPL_Y_REGION_UPPER,
                                           StGLCorner(ST_VCORNER_TOP, ST_HCORNER_LEFT));

    StGLSwitchTextured* aSrcBtn = new StGLSwitchTextured(upperRegion, myPlugin->params.srcFormat,
                                                         DISPL_X_REGION_UPPER + (i++) * ICON_WIDTH, DISPL_Y_REGION_UPPER,
                                                         StGLCorner(ST_VCORNER_TOP, ST_HCORNER_LEFT));

    aSrcBtn->addItem(ST_V_SRC_AUTODETECT,    texturesPathRoot + "srcFrmtAuto.std");
    aSrcBtn->addItem(ST_V_SRC_MONO,          texturesPathRoot + "srcFrmtMono.std");
    aSrcBtn->addItem(ST_V_SRC_ROW_INTERLACE, texturesPathRoot + "srcFrmtInterlace.std");
    aSrcBtn->addItem(ST_V_SRC_SIDE_BY_SIDE,  texturesPathRoot + "srcFrmtSideBySide.std");
    aSrcBtn->addItem(ST_V_SRC_PARALLEL_PAIR, texturesPathRoot + "srcFrmtSideBySide.std", true);
    aSrcBtn->addItem(ST_V_SRC_OVER_UNDER_LR, texturesPathRoot + "srcFrmtOverUnder.std");
    aSrcBtn->addItem(ST_V_SRC_OVER_UNDER_RL, texturesPathRoot + "srcFrmtOverUnder.std",  true);
    myBtnSrcFrmt = aSrcBtn;

    // setup textures for the buttons
    StString textPath = texturesPathRoot + "openImage.std";
        btnOpen->setTexturePath(&textPath);
}

/**
 * Create bottom toolbar
 */
void StMoviePlayerGUI::createBottomToolbar() {
    const StRectI_t& aMargins = getRootMarginsPx();
    bottomRegion = new StGLWidget(this, aMargins.left(), -aMargins.bottom(), StGLCorner(ST_VCORNER_BOTTOM, ST_HCORNER_LEFT), 4096, 128);

    // append the textured buttons
    btnPlay      = new StGLTextureButton(bottomRegion, DISPL_X_REGION_BOTTOM, DISPL_Y_REGION_BOTTOM,
                                         StGLCorner(ST_VCORNER_TOP, ST_HCORNER_LEFT), 2);
    btnPlay->signals.onBtnClick.connect(myPlugin, &StMoviePlayer::doPlayPause);

    stTimeBox    = new StTimeBox(bottomRegion, DISPL_X_REGION_BOTTOM + 1 * ICON_WIDTH, DISPL_Y_REGION_BOTTOM);
    btnPrev      = new StGLTextureButton(bottomRegion, -DISPL_X_REGION_BOTTOM - 3 * ICON_WIDTH, DISPL_Y_REGION_BOTTOM,
                                         StGLCorner(ST_VCORNER_TOP, ST_HCORNER_RIGHT));
    btnPrev->signals.onBtnClick.connect(myPlugin, &StMoviePlayer::doListPrev);

    btnNext      = new StGLTextureButton(bottomRegion, -DISPL_X_REGION_BOTTOM - 2 * ICON_WIDTH, DISPL_Y_REGION_BOTTOM,
                                         StGLCorner(ST_VCORNER_TOP, ST_HCORNER_RIGHT));
    btnNext->signals.onBtnClick.connect(myPlugin, &StMoviePlayer::doListNext);

    btnList      = new StGLTextureButton(bottomRegion, -DISPL_X_REGION_BOTTOM - ICON_WIDTH, DISPL_Y_REGION_BOTTOM,
                                         StGLCorner(ST_VCORNER_TOP, ST_HCORNER_RIGHT));
    btnList->signals.onBtnClick.connect(myPlugin, &StMoviePlayer::doPlayListReverse);

    btnFullScr   = new StGLTextureButton(bottomRegion, -DISPL_X_REGION_BOTTOM, DISPL_Y_REGION_BOTTOM,
                                         StGLCorner(ST_VCORNER_TOP, ST_HCORNER_RIGHT));
    btnFullScr->signals.onBtnClick.connect(myPlugin->params.isFullscreen.operator->(), &StBoolParam::doReverse);

    // setup textures for the buttons
    StString textPathsDuo[2];
    textPathsDuo[0] = texturesPathRoot + "moviePlay.std";
    textPathsDuo[1] = texturesPathRoot + "moviePause.std";
        btnPlay->setTexturePath(textPathsDuo, 2);
    StString textPath = texturesPathRoot + "timebox.std";
        stTimeBox->setTexturePath(&textPath);
    textPath = texturesPathRoot + "moviePrior.std";
        btnPrev->setTexturePath(&textPath);
    textPath = texturesPathRoot + "movieNext.std";
        btnNext->setTexturePath(&textPath);
    textPath = texturesPathRoot + "moviePlaylist.std";
        btnList->setTexturePath(&textPath);
    textPath = texturesPathRoot + "movieFullScr.std";
        btnFullScr->setTexturePath(&textPath);
}

/**
 * Main menu
 */
void StMoviePlayerGUI::createMainMenu() {
    const StRectI_t& aMargins = getRootMarginsPx();
    menu0Root = new StGLMenu(this, aMargins.left(), aMargins.top(), StGLMenu::MENU_HORIZONTAL, true);

    StGLMenu* aMenuMedia   = createMediaMenu();     // Root -> Media menu
    StGLMenu* aMenuView    = createViewMenu();      // Root -> View menu
              myMenuAudio  = createAudioMenu();     // Root -> Audio menu
           myMenuSubtitles = createSubtitlesMenu(); // Root -> Subtitles menu
    StGLMenu* aDevicesMenu = createOutputMenu();    // Root -> Output menu
    StGLMenu* aMenuHelp    = createHelpMenu();      // Root -> Help menu

    // Attach sub menus to root
    menu0Root->addItem(tr(MENU_MEDIA),     aMenuMedia);
    menu0Root->addItem(tr(MENU_VIEW),      aMenuView);
    menu0Root->addItem(tr(MENU_AUDIO),     myMenuAudio);
    menu0Root->addItem(tr(MENU_SUBTITLES), myMenuSubtitles);
    menu0Root->addItem(myPlugin->StApplication::params.ActiveDevice->getActiveValue(), aDevicesMenu);
    menu0Root->addItem(tr(MENU_HELP),      aMenuHelp);
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

    StGLMenuItem* anItem = aMenuMedia->addItem(tr(MENU_MEDIA_RECENT), myMenuRecent);
    anItem->setUserData(0);
    anItem->signals.onItemClick.connect(myPlugin, &StMoviePlayer::doOpenRecent);

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
    aMenu->addItem(tr(MENU_SRC_FORMAT_AUTO),         myPlugin->params.srcFormat, ST_V_SRC_AUTODETECT);
    aMenu->addItem(tr(MENU_SRC_FORMAT_MONO),         myPlugin->params.srcFormat, ST_V_SRC_MONO);
    aMenu->addItem(tr(MENU_SRC_FORMAT_CROSS_EYED),   myPlugin->params.srcFormat, ST_V_SRC_SIDE_BY_SIDE);
    aMenu->addItem(tr(MENU_SRC_FORMAT_PARALLEL),     myPlugin->params.srcFormat, ST_V_SRC_PARALLEL_PAIR);
    aMenu->addItem(tr(MENU_SRC_FORMAT_OVERUNDER_RL), myPlugin->params.srcFormat, ST_V_SRC_OVER_UNDER_RL);
    aMenu->addItem(tr(MENU_SRC_FORMAT_OVERUNDER_LR), myPlugin->params.srcFormat, ST_V_SRC_OVER_UNDER_LR);
    aMenu->addItem(tr(MENU_SRC_FORMAT_INTERLACED),   myPlugin->params.srcFormat, ST_V_SRC_ROW_INTERLACE);
    aMenu->addItem(tr(MENU_SRC_FORMAT_ANA_RC),       myPlugin->params.srcFormat, ST_V_SRC_ANAGLYPH_RED_CYAN);
    aMenu->addItem(tr(MENU_SRC_FORMAT_ANA_RB),       myPlugin->params.srcFormat, ST_V_SRC_ANAGLYPH_G_RB);
    aMenu->addItem(tr(MENU_SRC_FORMAT_ANA_YB),       myPlugin->params.srcFormat, ST_V_SRC_ANAGLYPH_YELLOW_BLUE);
    aMenu->addItem(tr(MENU_SRC_FORMAT_PAGEFLIP),     myPlugin->params.srcFormat, ST_V_SRC_PAGE_FLIP);
    aMenu->addItem(tr(MENU_SRC_FORMAT_TILED_4X),     myPlugin->params.srcFormat, ST_V_SRC_TILED_4X);
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
        anItem->changeRectPx().right() = anItem->getRectPx().left() + 10 * int(aLen);
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
 * Root -> Media  -> Web UI menu
 */
StGLMenu* StMoviePlayerGUI::createWebUIMenu() {
    StGLMenu* aMenu = new StGLMenu(this, 0, 0, StGLMenu::MENU_VERTICAL);
    for(size_t anIter = 0; anIter < myPlugin->params.StartWebUI->getValues().size(); ++anIter) {
        aMenu->addItem(myPlugin->params.StartWebUI->getValues()[anIter], myPlugin->params.StartWebUI, anIter);
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
    StGLMenu* aMenuDispMode  = createDisplayModeMenu();  // Root -> View menu -> Output
    StGLMenu* aMenuDispRatio = createDisplayRatioMenu(); // Root -> View menu -> Display Ratio
    StGLMenu* aMenuTexFilter = createSmoothFilterMenu(); // Root -> View menu -> Smooth Filter
    StGLMenu* aMenuImgAdjust = createImageAdjustMenu();  // Root -> View menu -> Image Adjust

    aMenuView->addItem(tr(MENU_VIEW_DISPLAY_MODE),  aMenuDispMode);
    aMenuView->addItem(tr(MENU_VIEW_FULLSCREEN),    myPlugin->params.isFullscreen);
    aMenuView->addItem(tr(MENU_VIEW_RESET))
             ->signals.onItemClick.connect(myPlugin, &StMoviePlayer::doReset);
    aMenuView->addItem(tr(MENU_VIEW_SWAP_LR),       stImageRegion->params.swapLR);
    aMenuView->addItem(tr(MENU_VIEW_DISPLAY_RATIO), aMenuDispRatio);
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
                   stImageRegion->params.displayMode, StGLImageRegion::MODE_STEREO);
    aMenu->addItem(tr(MENU_VIEW_DISPLAY_MODE_LEFT),
                   stImageRegion->params.displayMode, StGLImageRegion::MODE_ONLY_LEFT);
    aMenu->addItem(tr(MENU_VIEW_DISPLAY_MODE_RIGHT),
                   stImageRegion->params.displayMode, StGLImageRegion::MODE_ONLY_RIGHT);
    aMenu->addItem(tr(MENU_VIEW_DISPLAY_MODE_PARALLEL),
                   stImageRegion->params.displayMode, StGLImageRegion::MODE_PARALLEL);
    aMenu->addItem(tr(MENU_VIEW_DISPLAY_MODE_CROSSYED),
                   stImageRegion->params.displayMode, StGLImageRegion::MODE_CROSSYED);
    return aMenu;
}

/**
 * Root -> View menu -> Display Ratio
 */
StGLMenu* StMoviePlayerGUI::createDisplayRatioMenu() {
    StGLMenu* aMenu = new StGLMenu(this, 0, 0, StGLMenu::MENU_VERTICAL);
    aMenu->addItem("Auto",   stImageRegion->params.displayRatio, StGLImageRegion::RATIO_AUTO);
    aMenu->addItem("1:1",    stImageRegion->params.displayRatio, StGLImageRegion::RATIO_1_1);
    aMenu->addItem("4:3",    stImageRegion->params.displayRatio, StGLImageRegion::RATIO_4_3);
    aMenu->addItem("16:9",   stImageRegion->params.displayRatio, StGLImageRegion::RATIO_16_9);
    aMenu->addItem("16:10",  stImageRegion->params.displayRatio, StGLImageRegion::RATIO_16_10);
    aMenu->addItem("2.21:1", stImageRegion->params.displayRatio, StGLImageRegion::RATIO_221_1);
    aMenu->addItem("5:4",    stImageRegion->params.displayRatio, StGLImageRegion::RATIO_5_4);
    aMenu->addItem("Keep on restart", myPlugin->params.toRestoreRatio);
    return aMenu;
}

/**
 * Root -> View menu -> Smooth Filter
 */
StGLMenu* StMoviePlayerGUI::createSmoothFilterMenu() {
    StGLMenu* aMenu = new StGLMenu(this, 0, 0, StGLMenu::MENU_VERTICAL);
    aMenu->addItem(tr(MENU_VIEW_TEXFILTER_NEAREST),
                   stImageRegion->params.textureFilter, StGLImageProgram::FILTER_NEAREST);
    aMenu->addItem(tr(MENU_VIEW_TEXFILTER_LINEAR),
                   stImageRegion->params.textureFilter, StGLImageProgram::FILTER_LINEAR);
    aMenu->addItem(tr(MENU_VIEW_TEXFILTER_BLEND),
                   stImageRegion->params.textureFilter, StGLImageProgram::FILTER_BLEND);
    return aMenu;
}

/**
 * Root -> View menu -> Image Adjust
 */
StGLMenu* StMoviePlayerGUI::createImageAdjustMenu() {
    StGLMenu* aMenu = new StGLMenu(this, 0, 0, StGLMenu::MENU_VERTICAL);
    const StGLVec3 aBlack(0.0f, 0.0f, 0.0f);
    const StGLVec3 aGreen(0.4f, 0.8f, 0.4f);
    const StGLVec3 aRed  (1.0f, 0.0f, 0.0f);

    aMenu->addItem(tr(MENU_VIEW_ADJUST_RESET), myPlugin->getAction(StMoviePlayer::Action_ImageAdjustReset));

    StGLMenuItem* anItem = aMenu->addItem(tr(MENU_VIEW_ADJUST_GAMMA));
    anItem->setMarginRight(100 + 16);
    StGLRangeFieldFloat32* aRange = new StGLRangeFieldFloat32(anItem, stImageRegion->params.gamma,
                                                              -16, 0, StGLCorner(ST_VCORNER_CENTER, ST_HCORNER_RIGHT));
    aRange->setFormat(stCString("%+01.2f"));
    aRange->setColor(StGLRangeFieldFloat32::FieldColor_Default,  aBlack);
    aRange->setColor(StGLRangeFieldFloat32::FieldColor_Positive, aGreen);
    aRange->setColor(StGLRangeFieldFloat32::FieldColor_Negative, aRed);
    aRange->setVisibility(true, true);

    anItem = aMenu->addItem(tr(MENU_VIEW_ADJUST_BRIGHTNESS));
    anItem->setMarginRight(100 + 16);
    aRange = new StGLRangeFieldFloat32(anItem, stImageRegion->params.brightness,
                                       -16, 0, StGLCorner(ST_VCORNER_CENTER, ST_HCORNER_RIGHT));
    aRange->setFormat(stCString("%+01.2f"));
    aRange->setColor(StGLRangeFieldFloat32::FieldColor_Default,  aBlack);
    aRange->setColor(StGLRangeFieldFloat32::FieldColor_Positive, aGreen);
    aRange->setColor(StGLRangeFieldFloat32::FieldColor_Negative, aRed);
    aRange->setVisibility(true, true);

    anItem = aMenu->addItem(tr(MENU_VIEW_ADJUST_SATURATION));
    anItem->setMarginRight(100 + 16);
    aRange = new StGLRangeFieldFloat32(anItem, stImageRegion->params.saturation,
                                       -16, 0, StGLCorner(ST_VCORNER_CENTER, ST_HCORNER_RIGHT));
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
    stUtf8_t aBuff[256];

    //aGainFactor = -2.0;
    //aGain    = std::pow(2.0, aGainFactor);
    //aGain_dB = 6.0 * aGainFactor;
    stsprintf(aBuff, 256, "%01.2f", 1.0);
    aMenu->addItem(aBuff, myPlugin->params.AudioGain, 1.0f);
    stsprintf(aBuff, 256, "%01.2f (-3 dB)", 0.71);
    aMenu->addItem(aBuff, myPlugin->params.AudioGain, std::pow(2.0f, -0.5f));
    stsprintf(aBuff, 256, "%01.2f (-6 dB)", 0.5);
    aMenu->addItem(aBuff, myPlugin->params.AudioGain, 0.5f);
    stsprintf(aBuff, 256, "%01.2f (-9 dB)", 0.35);
    aMenu->addItem(aBuff, myPlugin->params.AudioGain, std::pow(2.0f, -1.5f));
    stsprintf(aBuff, 256, "%01.2f (-12 dB)", 0.25);
    aMenu->addItem(aBuff, myPlugin->params.AudioGain, 0.25f);
    stsprintf(aBuff, 256, "%01.2f (-15 dB)", 0.18);
    aMenu->addItem(aBuff, myPlugin->params.AudioGain, std::pow(2.0f, -2.5f));
    stsprintf(aBuff, 256, "%01.2f (-18 dB)", 0.125);
    aMenu->addItem(aBuff, myPlugin->params.AudioGain, 0.125f);
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
    : StGLMessageBox(theParent, "", 400, 260),
      myRange(NULL) {
        changeRectPx().moveX(  64);
        changeRectPx().moveY(-128);
        setCorner(StGLCorner(ST_VCORNER_BOTTOM, ST_HCORNER_LEFT));
        StGLButton* aResetBtn = addButton(theParent->tr(BUTTON_RESET));
        addButton(theParent->tr(BUTTON_CLOSE));
        setVisibility(true, true);

        StGLWidget* aContent = new StGLWidget(getContent(), 0, 0, StGLCorner(ST_VCORNER_TOP, ST_HCORNER_LEFT),
                                              getContent()->getRectPx().width(), getContent()->getRectPx().height());
        aContent->setVisibility(true, true);

        const StGLVec3 aWhite(1.0f, 1.0f, 1.0f);
        StGLTextArea* aTitle = new StGLTextArea(aContent, 0, 0, StGLCorner(ST_VCORNER_TOP, ST_HCORNER_LEFT),
                                                aContent->getRectPx().width(), 10);
        aTitle->setupAlignment(StGLTextFormatter::ST_ALIGN_X_CENTER, StGLTextFormatter::ST_ALIGN_Y_TOP);
        aTitle->setText(theParent->tr(DIALOG_AUDIO_DELAY_TITLE));
        aTitle->setTextColor(aWhite);
        aTitle->setVisibility(true, true);
        aTitle->stglInitAutoHeight();

        StGLTextArea* aText = new StGLTextArea(aContent, 0, aTitle->getRectPx().bottom(), StGLCorner(ST_VCORNER_TOP, ST_HCORNER_LEFT),
                                               aContent->getRectPx().width(), 10);
        aText->setText(StString("\n\n") + theParent->tr(DIALOG_AUDIO_DELAY_DESC) + "\n");
        aText->setTextColor(aWhite);
        aText->setVisibility(true, true);
        aText->stglInitAutoHeight();

        StGLTextArea* aLabel = new StGLTextArea(aContent, 0, aText->getRectPx().bottom(), StGLCorner(ST_VCORNER_TOP, ST_HCORNER_LEFT), -1, 10);
        aLabel->setText(theParent->tr(DIALOG_AUDIO_DELAY_LABEL));
        aLabel->setTextColor(aWhite);
        aLabel->setVisibility(true, true);
        aLabel->stglInitAutoHeightWidth();

        myRange = new StGLRangeFieldFloat32(aContent, theTrackedValue,
                                            aLabel->getRectPx().right() + 10, aLabel->getRectPx().top());
        myRange->setFormat(stCString("%+01.3f"));
        myRange->setColor(StGLRangeFieldFloat32::FieldColor_Default,  StGLVec3(1.0f, 1.0f, 1.0f));
        myRange->setColor(StGLRangeFieldFloat32::FieldColor_Positive, StGLVec3(0.4f, 0.8f, 0.4f));
        myRange->setColor(StGLRangeFieldFloat32::FieldColor_Negative, StGLVec3(1.0f, 0.0f, 0.0f));
        myRange->setVisibility(true, true);
        myRange->stglInit();

        StGLTextArea* aLabUnits = new StGLTextArea(aContent, myRange->getRectPx().right() + 10, aLabel->getRectPx().top(),
                                                   StGLCorner(ST_VCORNER_TOP, ST_HCORNER_LEFT), -1, 10);
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
        512, 300);
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
    StGLMessageBox* aDialog = new StGLMessageBox(this, aString, 512, 256);
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
    StGLMessageBox* aDialog = new StGLMessageBox(this, aString, 512, 300);
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
    aMenu->addItem(tr(MENU_HELP_BLOCKSLP),     aMenuBlockSleep);
    aMenu->addItem(tr(MENU_HELP_UPDATES),      aMenuCheckUpdates);
    aMenu->addItem(tr(MENU_HELP_LANGS),        aMenuLanguage);
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
  texturesPathRoot(StProcess::getStShareFolder() + "textures" + SYS_FS_SPLITTER),
  stTimeVisibleLock(true),
  //
  stImageRegion(NULL),
  stSubtitles(NULL),
  stDescr(NULL),
  myMsgStack(NULL),
  myPlayList(NULL),
  // main menu
  menu0Root(NULL),
  myMenuOpenAL(NULL),
  myMenuRecent(NULL),
  myMenuAudio(NULL),
  myMenuSubtitles(NULL),
  // upper toolbar
  upperRegion(NULL),
  btnOpen(NULL),
  btnSwapLR(NULL),
  myBtnSrcFrmt(NULL),
  // bottom toolbar
  bottomRegion(NULL),
  seekBar(NULL),
  btnPlay(NULL),
  stTimeBox(NULL),
  btnPrev(NULL),
  btnNext(NULL),
  btnList(NULL),
  btnFullScr(NULL),
  //
  myFpsWidget(NULL),
  //
  isGUIVisible(true),
  myIsExperimental(myPlugin->params.ToShowExtra->getValue()) {
    setRootMarginsPx(myWindow->getMargins());
    const StRectI_t& aMargins = getRootMarginsPx();
    myPlugin->params.ToShowFps->signals.onChanged.connect(this, &StMoviePlayerGUI::doShowFPS);
    stImageRegion = new StGLImageRegion(this, theTextureQueue, false);
    stSubtitles   = new StGLSubtitles  (this, theSubQueue);

    createUpperToolbar();

    seekBar = new StSeekBar(this, -aMargins.bottom() - 78);
    seekBar->signals.onSeekClick.connect(myPlugin, &StMoviePlayer::doSeek);

    createBottomToolbar();

    stDescr = new StGLDescription(this);

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
    if(stSubtitles != NULL) {
        stSubtitles->setPTS(thePTS);
    }
    if(seekBar != NULL) {
        seekBar->setProgress(theProgress);
    }
    if(stDescr != NULL) {
        stDescr->setPoint(thePointZo);
    }

    if(myLangMap->wasReloaded()
    || myIsExperimental != myPlugin->params.ToShowExtra->getValue()) {
        StGLMenu::DeleteWithSubMenus(menu0Root); menu0Root = NULL;
        createMainMenu();
        menu0Root->stglUpdateSubmenuLayout();
        myLangMap->resetReloaded();
        myIsExperimental = myPlugin->params.ToShowExtra->getValue();
        // turn back topmost position
        getChildren()->moveToTop(myMsgStack);
    }
}

void StMoviePlayerGUI::stglResize(const StRectI_t& winRectPx) {
    stImageRegion->changeRectPx().bottom() = winRectPx.height();
    stImageRegion->changeRectPx().right()  = winRectPx.width();

    const StRectI_t& aMargins = myWindow->getMargins();
    const bool areNewMargins = aMargins != getRootMarginsPx();
    if(areNewMargins) {
        setRootMarginsPx(aMargins);
    }

    if(upperRegion != NULL) {
        upperRegion->changeRectPx().right()  = stMax(winRectPx.width() - aMargins.right(), 2);
    }
    if(bottomRegion != NULL) {
        bottomRegion->changeRectPx().right() = stMax(winRectPx.width() - aMargins.right(), 2);
    }

    if(areNewMargins) {
        if(upperRegion != NULL) {
            upperRegion->changeRectPx().left() = aMargins.left();
            upperRegion->changeRectPx().moveTopTo(aMargins.top());
        }
        if(bottomRegion != NULL) {
            bottomRegion->changeRectPx().left() =  aMargins.left();
            bottomRegion->changeRectPx().moveTopTo(-aMargins.bottom());
        }
        if(seekBar != NULL) {
            seekBar->changeRectPx().moveTopTo(-aMargins.bottom() - 78);
        }
        if(menu0Root != NULL) {
            menu0Root->changeRectPx().left() = aMargins.left();
            menu0Root->changeRectPx().top()  = aMargins.top();
            menu0Root->stglUpdateSubmenuLayout();
        }
    }

    StGLRootWidget::stglResize(winRectPx);
}

bool StMoviePlayerGUI::toHideCursor() {
    if(bottomRegion == NULL) {
        return false;
    }
    StGLWidget* child = bottomRegion->getChildren()->getStart();
    return child != NULL && !child->isVisible();
}

void StMoviePlayerGUI::setVisibility(const StPointD_t& cursorZo, bool isMouseActive) {
    const bool toShowPlayList = myPlugin->params.ToShowPlayList->getValue();
    isGUIVisible = isMouseActive
        || stTimeVisibleLock.getElapsedTime() < 2.0
        || (upperRegion  != NULL && upperRegion->isPointIn(cursorZo))
        || (bottomRegion != NULL && bottomRegion->isPointIn(cursorZo))
        || (seekBar   != NULL && seekBar->isPointIn(cursorZo))
        || (toShowPlayList    && myPlayList->isPointIn(cursorZo))
        || (menu0Root != NULL && menu0Root->isActive());
    if(isMouseActive) {
        stTimeVisibleLock.restart();
    }

    // always visible
    StGLRootWidget::setVisibility(true, true);
    stImageRegion->setVisibility(true, true);
    stSubtitles->setVisibility(true, true);

    if(menu0Root != NULL) {
        menu0Root->setVisibility(isGUIVisible, false);
    }

    if(seekBar != NULL) {
        seekBar->setVisibility(isGUIVisible);
    }

    if(upperRegion != NULL) {
        upperRegion->setVisibility(isGUIVisible);
        for(StGLWidget* child = upperRegion->getChildren()->getStart(); child != NULL; child = child->getNext()) {
            child->setVisibility(isGUIVisible);
        }
    }

    if(bottomRegion != NULL) {
        bottomRegion->setVisibility(isGUIVisible);
        for(StGLWidget* child = bottomRegion->getChildren()->getStart(); child != NULL; child = child->getNext()) {
            child->setVisibility(isGUIVisible);
        }
    }

    if(toShowPlayList) {
        myPlayList->setVisibility(isGUIVisible, false);
    }

    if(stDescr != NULL) {
        stDescr->setVisibility(true, true);
        if(btnOpen->isPointIn(cursorZo)) {
            stDescr->setText(tr(FILE_VIDEO_OPEN));
        } else if(btnSwapLR->isPointIn(cursorZo)) {
            size_t aLngId = stImageRegion->params.swapLR->getValue() ? SWAP_LR_ON : SWAP_LR_OFF;
            stDescr->setText(tr(aLngId));
        } else if(myBtnSrcFrmt->isPointIn(cursorZo)) {
            size_t aLngId = MENU_SRC_FORMAT_AUTO;
            switch(myPlugin->params.srcFormat->getValue()) {
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
                //case ST_V_SRC_SEPARATE_FRAMES:      aLngId = MENU_SRC_FORMAT_SEPARATE; break;
                default:
                case ST_V_SRC_AUTODETECT:           aLngId = MENU_SRC_FORMAT_AUTO; break;
            }
            stDescr->setText(tr(BTN_SRC_FORMAT) + tr(aLngId));
        } else if(btnPlay->isPointIn(cursorZo)) {
            stDescr->setText(tr(VIDEO_PLAYPAUSE));
        } else if(btnPrev->isPointIn(cursorZo)) {
            stDescr->setText(tr(VIDEO_LIST_PREV));
        } else if(btnNext->isPointIn(cursorZo)) {
            stDescr->setText(tr(VIDEO_LIST_NEXT));
        } else if(btnList->isPointIn(cursorZo)) {
            stDescr->setText(tr(VIDEO_LIST));
        } else if(btnFullScr->isPointIn(cursorZo)) {
            stDescr->setText(tr(FULLSCREEN));
        } else {
            stDescr->setVisibility(false, true);
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
            aDelayItem->setMarginRight(100 + 16);
            aDelayItem->signals.onItemClick.connect(this, &StMoviePlayerGUI::doAudioDelay);
            aDelayRange = new StGLRangeFieldFloat32(aDelayItem, myPlugin->params.AudioDelay,
                                                    -16, 0, StGLCorner(ST_VCORNER_CENTER, ST_HCORNER_RIGHT));
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

void StMoviePlayerGUI::updateSubtitlesStreamsMenu(const StHandle< StArrayList<StString> >& theStreamsList) {
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

    //myMenuSubtitles->addSplitter();
    myMenuSubtitles->addItem(tr(MENU_SUBTITLES_ATTACH))
                   ->signals.onItemClick.connect(myPlugin, &StMoviePlayer::doAddSubtitleStream);

    // update menu representation
    myMenuSubtitles->stglInit();
}

void StMoviePlayerGUI::stglDraw(unsigned int theView) {
    setLensDist(myPlugin->getMainWindow()->getLensDist());
    if(theView == ST_DRAW_LEFT
    && myFpsWidget != NULL) {
        stImageRegion->getTextureQueue()->getQueueInfo(myFpsWidget->changePlayQueued(),
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

    StGLMessageBox* aDialog = new StGLMessageBox(this, anAboutText, 512, 300);
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
