/**
 * Copyright © 2009-2012 Kirill Gavrilov <kirill@sview.ru>
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

#include <StCore/StCore.h>
#include <StCore/StWindow.h>

#include <StImage/StImageFile.h>
#include <StSocket/StSocket.h>

#include <StGLWidgets/StGLCheckboxTextured.h>
#include <StGLWidgets/StGLImageRegion.h>
#include <StGLWidgets/StGLTextureButton.h>
#include <StGLWidgets/StGLDescription.h>
#include <StGLWidgets/StGLDevicesMenu.h>
#include <StGLWidgets/StGLMenu.h>
#include <StGLWidgets/StGLMenuItem.h>
#include <StGLWidgets/StGLMessageBox.h>
#include <StGLWidgets/StGLMsgStack.h>
#include <StGLWidgets/StGLSubtitles.h>
#include <StGLWidgets/StGLSwitchTextured.h>

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

    upperRegion = new StGLWidget(this, 0, 0, StGLCorner(ST_VCORNER_TOP, ST_HCORNER_LEFT), 4096, 128);

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
    bottomRegion = new StGLWidget(this, 0, 0, StGLCorner(ST_VCORNER_BOTTOM, ST_HCORNER_LEFT), 4096, 128);

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
    //btnList->signals.onBtnClick

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
    menu0Root = new StGLMenu(this, 0, 0, StGLMenu::MENU_HORIZONTAL, true);

    StGLMenu* aMenuMedia  = createMediaMenu();     // Root -> Media menu
    StGLMenu* aMenuView   = createViewMenu();      // Root -> View menu
              myMenuAudio = createAudioMenu();     // Root -> Audio menu
          myMenuSubtitles = createSubtitlesMenu(); // Root -> Subtitles menu
    // Root -> Output menu
    StGLDevicesMenu* aDevicesMenu = new StGLDevicesMenu(this, myWindow,
        myLangMap->changeValueId(MENU_CHANGE_DEVICE,  "Change Device"),
        myLangMap->changeValueId(MENU_ABOUT_RENDERER, "About Plugin..."));
    StGLMenu* aMenuHelp   = createHelpMenu();    // Root -> Help menu

    // Attach sub menus to root
    menu0Root->addItem(myLangMap->changeValueId(MENU_MEDIA,
                       "Media"), aMenuMedia);
    menu0Root->addItem(myLangMap->changeValueId(MENU_VIEW,
                       "View"),  aMenuView);
    menu0Root->addItem(myLangMap->changeValueId(MENU_AUDIO,
                       "Audio"), myMenuAudio);
    menu0Root->addItem(myLangMap->changeValueId(MENU_SUBTITLES,
                       "Subtitles"), myMenuSubtitles);
    aDevicesMenu->setTrackedItem(menu0Root->addItem(aDevicesMenu->getTitle(), aDevicesMenu));
    menu0Root->addItem(myLangMap->changeValueId(MENU_HELP,
                       "Help"),  aMenuHelp);
}

/**
 * Root -> Media menu
 */
StGLMenu* StMoviePlayerGUI::createMediaMenu() {
    StGLMenu* aMenuMedia = new StGLMenu(this, 0, 0, StGLMenu::MENU_VERTICAL);
    StGLMenu* aMenuSrcFormat = createSrcFormatMenu();    // Root -> Media -> Source format menu
    StGLMenu* aMenuOpenAL    = createOpenALDeviceMenu(); // Root -> Media -> OpenAL Device
    StGLMenu* aMenuVolume    = createAudioGainMenu();
    StGLMenu* aMenuOpenImage = createOpenMovieMenu();    // Root -> Media -> Open movie menu
    StGLMenu* aMenuSaveImage = createSaveImageMenu();    // Root -> Media -> Save snapshot menu

    aMenuMedia->addItem(myLangMap->changeValueId(MENU_MEDIA_OPEN_MOVIE,
                        "Open Movie..."), aMenuOpenImage);
    aMenuMedia->addItem(myLangMap->changeValueId(MENU_MEDIA_SAVE_SNAPSHOT_AS,
                        "Save Snapshot As..."), aMenuSaveImage);

    //aMenuMedia->addItem("First File in folder", myPlugin, StMoviePlayer::doListFirst);
    //aMenuMedia->addItem("Prev File in folder",  myPlugin, StMoviePlayer::doListPrev);
    //aMenuMedia->addItem("Next File in folder",  myPlugin, StMoviePlayer::doListNext);
    //aMenuMedia->addItem("Last File in folder",  myPlugin, StMoviePlayer::doListLast);

    aMenuMedia->addItem(myLangMap->changeValueId(MENU_MEDIA_SRC_FORMAT,
                        "Source stereo format"), aMenuSrcFormat);

    aMenuMedia->addItem(myLangMap->changeValueId(MENU_MEDIA_AL_DEVICE,
                        "Audio Device"), aMenuOpenAL);

    aMenuMedia->addItem("Audio Volume", aMenuVolume);

    aMenuMedia->addItem(myLangMap->changeValueId(MENU_MEDIA_SHUFFLE,
                        "Shuffle"), myPlugin->params.isShuffle);

    aMenuMedia->addItem(myLangMap->changeValueId(MENU_MEDIA_QUIT, "Quit"))
              ->signals.onItemClick.connect(myPlugin, &StMoviePlayer::doQuit);
    return aMenuMedia;
}

/**
 * Root -> Media -> Open movie menu
 */
StGLMenu* StMoviePlayerGUI::createOpenMovieMenu() {
    StGLMenu* aMenu = new StGLMenu(this, 0, 0, StGLMenu::MENU_VERTICAL);
    aMenu->addItem(myLangMap->changeValueId(MENU_MEDIA_OPEN_MOVIE_1, "From One file"))
         ->signals.onItemClick.connect(myPlugin, &StMoviePlayer::doOpen1File);
    aMenu->addItem(myLangMap->changeValueId(MENU_MEDIA_OPEN_MOVIE_2, "Left+Right files"))
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
    aMenu->addItem(myLangMap->changeValueId(MENU_SRC_FORMAT_AUTO,
                   "Autodetection"),           myPlugin->params.srcFormat, ST_V_SRC_AUTODETECT);
    aMenu->addItem(myLangMap->changeValueId(MENU_SRC_FORMAT_MONO,
                   "Mono"),                    myPlugin->params.srcFormat, ST_V_SRC_MONO);
    aMenu->addItem(myLangMap->changeValueId(MENU_SRC_FORMAT_CROSS_EYED,
                   "Cross-eyed"),              myPlugin->params.srcFormat, ST_V_SRC_SIDE_BY_SIDE);
    aMenu->addItem(myLangMap->changeValueId(MENU_SRC_FORMAT_PARALLEL,
                   "Parallel Pair"),           myPlugin->params.srcFormat, ST_V_SRC_PARALLEL_PAIR);
    aMenu->addItem(myLangMap->changeValueId(MENU_SRC_FORMAT_OVERUNDER_RL,
                   "Over/Under (R/L)"),        myPlugin->params.srcFormat, ST_V_SRC_OVER_UNDER_RL);
    aMenu->addItem(myLangMap->changeValueId(MENU_SRC_FORMAT_OVERUNDER_LR,
                   "Over/Under (L/R)"),        myPlugin->params.srcFormat, ST_V_SRC_OVER_UNDER_LR);
    aMenu->addItem(myLangMap->changeValueId(MENU_SRC_FORMAT_INTERLACED,
                   "Interlaced"),              myPlugin->params.srcFormat, ST_V_SRC_ROW_INTERLACE);
    aMenu->addItem(myLangMap->changeValueId(MENU_SRC_FORMAT_ANA_RC,
                   "Anaglyph Red/Cyan"),       myPlugin->params.srcFormat, ST_V_SRC_ANAGLYPH_RED_CYAN);
    aMenu->addItem(myLangMap->changeValueId(MENU_SRC_FORMAT_ANA_RB,
                   "Anaglyph Green/Red+Blue"), myPlugin->params.srcFormat, ST_V_SRC_ANAGLYPH_G_RB);
    aMenu->addItem(myLangMap->changeValueId(MENU_SRC_FORMAT_ANA_YB,
                   "Anaglyph Yellow/Blue"),    myPlugin->params.srcFormat, ST_V_SRC_ANAGLYPH_YELLOW_BLUE);
    aMenu->addItem(myLangMap->changeValueId(MENU_SRC_FORMAT_PAGEFLIP,
                   "Frame-sequential"),        myPlugin->params.srcFormat, ST_V_SRC_PAGE_FLIP);
    aMenu->addItem(myLangMap->changeValueId(MENU_SRC_FORMAT_TILED_4X,
                   "Tiled 4X"),                myPlugin->params.srcFormat, ST_V_SRC_TILED_4X);
    return aMenu;
}

/**
 * Root -> Media -> OpenAL Device menu
 */
StGLMenu* StMoviePlayerGUI::createOpenALDeviceMenu() {
    StGLMenu* aMenu = new StGLMenu(this, 0, 0, StGLMenu::MENU_VERTICAL);
    const StArrayList<StString>& aDevList = myPlugin->params.alDevice->getList();
    // OpenAL devices names are often very long...
    size_t aLen = 10;
    for(size_t devId = 0; devId < aDevList.size(); ++devId) {
        aLen = stMax(aLen, aDevList[devId].getLength());
    }
    aLen += 2;
    for(size_t devId = 0; devId < aDevList.size(); ++devId) {
        StGLMenuItem* anItem = aMenu->addItem(aDevList[devId],
                                              StHandle<StInt32Param>::downcast(myPlugin->params.alDevice), int32_t(devId));
        anItem->changeRectPx().right() = anItem->getRectPx().left() + 10 * int(aLen);
    }
    return aMenu;
}

/**
 * Root -> View menu
 */
StGLMenu* StMoviePlayerGUI::createViewMenu() {
    StGLMenu* aMenuView = new StGLMenu(this, 0, 0, StGLMenu::MENU_VERTICAL);
    StGLMenu* aMenuDispMode  = createDisplayModeMenu();  // Root -> View menu -> Output
    StGLMenu* aMenuDispRatio = createDisplayRatioMenu(); // Root -> View menu -> Display Ratio
    StGLMenu* aMenuTexFilter = createSmoothFilterMenu(); // Root -> View menu -> Smooth Filter
    StGLMenu* aMenuGamma     = createGammaMenu();        // Root -> View menu -> Gamma Correction

    aMenuView->addItem(myLangMap->changeValueId(MENU_VIEW_DISPLAY_MODE,
                       "Stereo Output"), aMenuDispMode);

    aMenuView->addItem(myLangMap->changeValueId(MENU_VIEW_FULLSCREEN, "Fullscreen"),
                       myPlugin->params.isFullscreen);
    aMenuView->addItem(myLangMap->changeValueId(MENU_VIEW_RESET, "Reset"))
             ->signals.onItemClick.connect(myPlugin, &StMoviePlayer::doReset);
    aMenuView->addItem(myLangMap->changeValueId(MENU_VIEW_SWAP_LR, "Swap Left/Right"),
                       stImageRegion->params.swapLR);
    aMenuView->addItem(myLangMap->changeValueId(MENU_VIEW_DISPLAY_RATIO,
                       "Display Ratio"),    aMenuDispRatio);
    aMenuView->addItem(myLangMap->changeValueId(MENU_VIEW_TEXFILTER,
                       "Smooth Filter"),    aMenuTexFilter);
    aMenuView->addItem(myLangMap->changeValueId(MENU_VIEW_GAMMA,
                       "Gamma Correction"), aMenuGamma);
    return aMenuView;
}

/**
 * Root -> View menu -> Output
 */
StGLMenu* StMoviePlayerGUI::createDisplayModeMenu() {
    StGLMenu* aMenu = new StGLMenu(this, 0, 0, StGLMenu::MENU_VERTICAL);
    aMenu->addItem(myLangMap->changeValueId(MENU_VIEW_DISPLAY_MODE_STEREO,   "Stereo"),
                   stImageRegion->params.displayMode, StGLImageRegion::MODE_STEREO);
    aMenu->addItem(myLangMap->changeValueId(MENU_VIEW_DISPLAY_MODE_LEFT,     "Left view"),
                   stImageRegion->params.displayMode, StGLImageRegion::MODE_ONLY_LEFT);
    aMenu->addItem(myLangMap->changeValueId(MENU_VIEW_DISPLAY_MODE_RIGHT,    "Right view"),
                   stImageRegion->params.displayMode, StGLImageRegion::MODE_ONLY_RIGHT);
    aMenu->addItem(myLangMap->changeValueId(MENU_VIEW_DISPLAY_MODE_PARALLEL, "Parallel pair"),
                   stImageRegion->params.displayMode, StGLImageRegion::MODE_PARALLEL);
    aMenu->addItem(myLangMap->changeValueId(MENU_VIEW_DISPLAY_MODE_CROSSYED, "Cross-eyed pair"),
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
    aMenu->addItem(myLangMap->changeValueId(MENU_VIEW_TEXFILTER_NEAREST, "Nearest"),
                   stImageRegion->params.textureFilter, StGLImageProgram::FILTER_NEAREST);
    aMenu->addItem(myLangMap->changeValueId(MENU_VIEW_TEXFILTER_LINEAR,  "Linear"),
                   stImageRegion->params.textureFilter, StGLImageProgram::FILTER_LINEAR);
    aMenu->addItem(myLangMap->changeValueId(MENU_VIEW_TEXFILTER_BLEND,   "Blend Deinterlace"),
                   stImageRegion->params.textureFilter, StGLImageProgram::FILTER_BLEND);
    return aMenu;
}

/**
 * Root -> View menu -> Gamma Correction
 */
StGLMenu* StMoviePlayerGUI::createGammaMenu() {
    StGLMenu* aMenu = new StGLMenu(this, 0, 0, StGLMenu::MENU_VERTICAL);
    ///menu->addItem("Coeff. *.*", size_t(StGLImageRegion::GAMMA_MAN))
    ///    ->signals.onItemClick.connect(stImageRegion, &StGLImageRegion::doGammaMenu);
    stUtf8_t aBuff[256];
    stsprintf(aBuff, 256, "%01.1f", 0.8);
    aMenu->addItem(aBuff, stImageRegion->params.gamma, 0.8f);
    aMenu->addItem("Off", stImageRegion->params.gamma, 1.0f);
    stsprintf(aBuff, 256, "%01.1f", 1.2);
    aMenu->addItem(aBuff, stImageRegion->params.gamma, 1.2f);
    stsprintf(aBuff, 256, "%01.1f", 1.4);
    aMenu->addItem(aBuff, stImageRegion->params.gamma, 1.4f);
    return aMenu;
}

/**
 * Root -> Audio menu
 */
StGLMenu* StMoviePlayerGUI::createAudioMenu() {
    StGLMenu* aMenu = new StGLMenu(this, 0, 0, StGLMenu::MENU_VERTICAL);
    aMenu->addItem("None", myPlugin->params.audioStream, -1);
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
    aMenu->addItem(aBuff, myPlugin->params.audioGain, 1.0f);
    stsprintf(aBuff, 256, "%01.2f (-3 dB)", 0.71);
    aMenu->addItem(aBuff, myPlugin->params.audioGain, std::pow(2.0f, -0.5f));
    stsprintf(aBuff, 256, "%01.2f (-6 dB)", 0.5);
    aMenu->addItem(aBuff, myPlugin->params.audioGain, 0.5f);
    stsprintf(aBuff, 256, "%01.2f (-9 dB)", 0.35);
    aMenu->addItem(aBuff, myPlugin->params.audioGain, std::pow(2.0f, -1.5f));
    stsprintf(aBuff, 256, "%01.2f (-12 dB)", 0.25);
    aMenu->addItem(aBuff, myPlugin->params.audioGain, 0.25f);
    stsprintf(aBuff, 256, "%01.2f (-15 dB)", 0.18);
    aMenu->addItem(aBuff, myPlugin->params.audioGain, std::pow(2.0f, -2.5f));
    stsprintf(aBuff, 256, "%01.2f (-18 dB)", 0.125);
    aMenu->addItem(aBuff, myPlugin->params.audioGain, 0.125f);
    aMenu->addItem("Mute", myPlugin->params.audioGain, 0.0f);
    return aMenu;
}

/**
 * Root -> Subtitles menu
 */
StGLMenu* StMoviePlayerGUI::createSubtitlesMenu() {
    StGLMenu* aMenu = new StGLMenu(this, 0, 0, StGLMenu::MENU_VERTICAL);
    aMenu->addItem("None", myPlugin->params.subtitlesStream, -1);

    return aMenu;
}

void StMoviePlayerGUI::doAboutProgram(const size_t ) {
    StString& aTitle = myLangMap->changeValueId(ABOUT_DPLUGIN_NAME,
        "sView - Movie Player");
    StString& aVerString = myLangMap->changeValueId(ABOUT_VERSION, "version");
    StString& aDescr = myLangMap->changeValueId(ABOUT_DESCRIPTION, StString()
        + "Movie player allows you to play stereoscopic video.\n"
        + "(C) 2007-2012 Kirill Gavrilov <kirill@sview.ru>\nOfficial site: www.sview.ru\n\nThis program distributed under GPL3.0");
    StGLMessageBox* aboutDialog = new StGLMessageBox(this, aTitle + '\n'
        + aVerString + ": " + StVersionInfo::getSDKVersionString()
        + " "+ StThread::getArchString()
        + "\n \n" + aDescr,
        512, 256);

    aboutDialog->setVisibility(true, true);
    aboutDialog->stglInit();
    aboutDialog->signals.onClickLeft.connect(aboutDialog,  &StGLMessageBox::doKillSelf);
    aboutDialog->signals.onClickRight.connect(aboutDialog, &StGLMessageBox::doKillSelf);
}

void StMoviePlayerGUI::doAboutFile(const size_t ) {
    StHandle<StFileNode>     aFileNode;
    StHandle<StStereoParams> aParams;
    StHandle<StMovieInfo>    anExtraInfo;
    StArrayList<StString> anInfoList(10);
    if(myPlugin->getCurrentFile(aFileNode, aParams, anExtraInfo) && !anExtraInfo.isNull()) {
        for(size_t aKeyIter = 0; aKeyIter < anExtraInfo->myInfo.size(); ++aKeyIter) {
            const StArgument& aPair = anExtraInfo->myInfo.getFromIndex(aKeyIter);
            anInfoList.add(aPair.getKey() + ": " + aPair.getValue() + "\n");
        }
    }

    StString aTitle = "File Info";
    StString anInfo;
    for(size_t anIter = 0; anIter < anInfoList.size(); ++anIter) {
        anInfo += anInfoList[anIter];
    }
    StString aString = aTitle + "\n\n \n" + anInfo;
    StGLMessageBox* anInfoDialog = new StGLMessageBox(this, aString, 512, 256);

    anInfoDialog->setVisibility(true, true);
    anInfoDialog->stglInit();
    anInfoDialog->signals.onClickLeft.connect(anInfoDialog,  &StGLMessageBox::doKillSelf);
    anInfoDialog->signals.onClickRight.connect(anInfoDialog, &StGLMessageBox::doKillSelf);
}

void StMoviePlayerGUI::doCheckUpdates(const size_t ) {
    StSocket::openURL("http://www.sview.ru/download");
}

void StMoviePlayerGUI::doOpenLicense(const size_t ) {
    StSocket::openURL(StProcess::getStCoreFolder()
                      + "info" + SYS_FS_SPLITTER
                      + "license.txt");
}

/**
 * Root -> Help menu
 */
StGLMenu* StMoviePlayerGUI::createHelpMenu() {
    StGLMenu* aMenu = new StGLMenu(this, 0, 0, StGLMenu::MENU_VERTICAL);
    StGLMenu* aMenuCheckUpdates = createCheckUpdatesMenu(); // Root -> Help -> Check updates menu
    StGLMenu* aMenuLanguage     = createLanguageMenu();     // Root -> Help -> Language menu

    aMenu->addItem(myLangMap->changeValueId(MENU_HELP_ABOUT,   "About..."))
         ->signals.onItemClick.connect(this, &StMoviePlayerGUI::doAboutProgram);

    aMenu->addItem(myLangMap->changeValueId(MENU_HELP_LICENSE, "License text"))
         ->signals.onItemClick.connect(this, &StMoviePlayerGUI::doOpenLicense);

    aMenu->addItem(myLangMap->changeValueId(MENU_HELP_UPDATES, "Check for updates"), aMenuCheckUpdates);
    aMenu->addItem(myLangMap->changeValueId(MENU_HELP_LANGS,   "Language"),          aMenuLanguage);
    return aMenu;
}

/**
 * Root -> Help -> Check updates menu
 */
StGLMenu* StMoviePlayerGUI::createCheckUpdatesMenu() {
    StGLMenu* aMenu = new StGLMenu(this, 0, 0, StGLMenu::MENU_VERTICAL);

    aMenu->addItem(myLangMap->changeValueId(MENU_HELP_UPDATES_NOW, "Now"))
         ->signals.onItemClick.connect(this, &StMoviePlayerGUI::doCheckUpdates);

    aMenu->addItem(myLangMap->changeValueId(MENU_HELP_UPDATES_DAY,   "Each day"),
                   myPlugin->params.checkUpdatesDays, 1);
    aMenu->addItem(myLangMap->changeValueId(MENU_HELP_UPDATES_WEEK,  "Each week"),
                   myPlugin->params.checkUpdatesDays, 7);
    aMenu->addItem(myLangMap->changeValueId(MENU_HELP_UPDATES_YEAR,  "Each year"),
                   myPlugin->params.checkUpdatesDays, 355);
    aMenu->addItem(myLangMap->changeValueId(MENU_HELP_UPDATES_NEVER, "Never"),
                   myPlugin->params.checkUpdatesDays, 0);
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

StMoviePlayerGUI::StMoviePlayerGUI(StMoviePlayer* thePlugin,
                                   StWindow*      theWindow,
                                   size_t theTextureQueueSizeMax)
: StGLRootWidget(),
  myPlugin(thePlugin),
  myWindow(theWindow),
  myLangMap(new StTranslations(StMoviePlayer::ST_DRAWER_PLUGIN_NAME)),
  texturesPathRoot(StProcess::getStCoreFolder() + "textures" + SYS_FS_SPLITTER),
  stTimeVisibleLock(true),
  //
  stImageRegion(NULL),
  stSubtitles(NULL),
  stDescr(NULL),
  myMsgStack(NULL),
  // main menu
  menu0Root(NULL),
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
  isGUIVisible(true) {
    //
    stImageRegion = new StGLImageRegion(this, theTextureQueueSizeMax);
    stSubtitles = new StGLSubtitles(this);

    createUpperToolbar();

    seekBar = new StSeekBar(this, -78);
    seekBar->signals.onSeekClick.connect(myPlugin, &StMoviePlayer::doSeek);

    createBottomToolbar();

    stDescr = new StGLDescription(this);

    // create main menu
    createMainMenu();

    myMsgStack = new StGLMsgStack(this);
    myMsgStack->setVisibility(true, true);
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
    if(myLangMap->wasReloaded()) {
        StGLMenu::DeleteWithSubMenus(menu0Root); menu0Root = NULL;
        createMainMenu();
        menu0Root->stglUpdateSubmenuLayout();
        myLangMap->resetReloaded();
        // turn back topmost position
        getChildren()->moveToTop(myMsgStack);
    }
}

void StMoviePlayerGUI::stglResize(const StRectI_t& winRectPx) {
    stImageRegion->changeRectPx().bottom() = winRectPx.height();
    stImageRegion->changeRectPx().right()  = winRectPx.width();
    if(upperRegion != NULL) {
        upperRegion->changeRectPx().right() = winRectPx.width();
    }
    if(bottomRegion != NULL) {
        bottomRegion->changeRectPx().right() = winRectPx.width();
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
    isGUIVisible = isMouseActive
        || stTimeVisibleLock.getElapsedTime() < 2.0
        || (upperRegion  != NULL && upperRegion->isPointIn(cursorZo))
        || (bottomRegion != NULL && bottomRegion->isPointIn(cursorZo))
        || (seekBar   != NULL && seekBar->isPointIn(cursorZo))
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

    if(stDescr != NULL) {
        stDescr->setVisibility(true, true);
        if(btnOpen->isPointIn(cursorZo)) {
            stDescr->setText(myLangMap->changeValueId(FILE_VIDEO_OPEN,
                             "Open another movie"));
        } else if(btnSwapLR->isPointIn(cursorZo)) {
            size_t aLngId = stImageRegion->params.swapLR->getValue() ? SWAP_LR_ON : SWAP_LR_OFF;
            stDescr->setText(myLangMap->changeValueId(aLngId, StString()));
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
            StString text = myLangMap->changeValueId(BTN_SRC_FORMAT, "Source format:\n")
                          + myLangMap->changeValueId(aLngId, StString());
            stDescr->setText(text);
        } else if(btnPlay->isPointIn(cursorZo)) {
            stDescr->setText(myLangMap->changeValueId(VIDEO_PLAYPAUSE,
                             "Play/Pause"));
        } else if(btnPrev->isPointIn(cursorZo)) {
            stDescr->setText(myLangMap->changeValueId(VIDEO_LIST_PREV,
                             "Play Previous File"));
        } else if(btnNext->isPointIn(cursorZo)) {
            stDescr->setText(myLangMap->changeValueId(VIDEO_LIST_NEXT,
                             "Play Next File"));
        } else if(btnList->isPointIn(cursorZo)) {
            stDescr->setText(myLangMap->changeValueId(VIDEO_LIST,
                             "Show/Hide playlist"));
        } else if(btnFullScr->isPointIn(cursorZo)) {
            stDescr->setText(myLangMap->changeValueId(FULLSCREEN,
                             "Switch\nfullscreen/windowed"));
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
        myMenuAudio->addItem("None", myPlugin->params.audioStream, -1);
    }
    if(!theStreamsList.isNull()) {
        for(size_t aStreamId = 0; aStreamId < theStreamsList->size(); ++aStreamId) {
            myMenuAudio->addItem(theStreamsList->getValue(aStreamId), myPlugin->params.audioStream, int32_t(aStreamId));
        }
    }

    //myMenuAudio->addSplitter();
    if(theHasVideo) {
        myMenuAudio->addItem("Attach from file")
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

    myMenuSubtitles->addItem("None", myPlugin->params.subtitlesStream, -1);
    if(!theStreamsList.isNull()) {
        for(size_t aStreamId = 0; aStreamId < theStreamsList->size(); ++aStreamId) {
            myMenuSubtitles->addItem(theStreamsList->getValue(aStreamId), myPlugin->params.subtitlesStream, int32_t(aStreamId));
        }
    }

    //myMenuSubtitles->addSplitter();
    myMenuSubtitles->addItem("Attach from file")
                   ->signals.onItemClick.connect(myPlugin, &StMoviePlayer::doAddSubtitleStream);

    // update menu representation
    myMenuSubtitles->stglInit();
}

void StMoviePlayerGUI::showUpdatesNotify() {
    StGLMessageBox* notifyMsg = new StGLMessageBox(this, myLangMap->changeValueId(UPDATES_NOTIFY,
        "A new version of sView is available on the official site www.sview.ru.\nPlease update your program."));
    notifyMsg->setVisibility(true, true);
    notifyMsg->stglInit();
    notifyMsg->signals.onClickLeft.connect(notifyMsg,  &StGLMessageBox::doKillSelf);
    notifyMsg->signals.onClickRight.connect(notifyMsg, &StGLMessageBox::doKillSelf);
}
