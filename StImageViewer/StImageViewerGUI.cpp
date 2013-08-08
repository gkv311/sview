/**
 * Copyright © 2009-2013 Kirill Gavrilov <kirill@sview.ru>
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

#include "StImageViewerGUI.h"
#include "StImageViewer.h"

#include <StCore/StWindow.h>

#include <StSocket/StSocket.h>
#include <StSettings/StEnumParam.h>
#include <StThreads/StThread.h>

#include <StGL/StParams.h>
#include <StGLWidgets/StGLCheckboxTextured.h>
#include <StGLWidgets/StGLDescription.h>
#include <StGLWidgets/StGLImageRegion.h>
#include <StGLWidgets/StGLMenu.h>
#include <StGLWidgets/StGLMenuItem.h>
#include <StGLWidgets/StGLMessageBox.h>
#include <StGLWidgets/StGLMsgStack.h>
#include <StGLWidgets/StGLRangeFieldFloat32.h>
#include <StGLWidgets/StGLSwitchTextured.h>
#include <StGLWidgets/StGLTextureButton.h>
#include <StGLWidgets/StGLFpsLabel.h>

#include <StImage/StImageFile.h>
#include <StVersion.h>

#include "StImageViewerStrings.h"

using namespace StImageViewerStrings;

namespace {
    static const int DISPL_Y_REGION_UPPER = 32;
    static const int DISPL_X_REGION_UPPER = 32;
    static const int ICON_WIDTH           = 64;
};

/**
 * Create upper toolbar
 */
void StImageViewerGUI::createUpperToolbar() {
    int i = 0;

    const StRectI_t& aMargins = getRootMarginsPx();
    upperRegion = new StGLWidget(this, aMargins.left(), aMargins.top(), StGLCorner(ST_VCORNER_TOP, ST_HCORNER_LEFT), 4096, 128);

    // append textured buttons
    btnOpen     = new StGLTextureButton(upperRegion, DISPL_X_REGION_UPPER + (i++) * ICON_WIDTH, DISPL_Y_REGION_UPPER);
    btnOpen->signals.onBtnClick.connectUnsafe(myPlugin, StImageViewer::doOpenFileDialog);

    btnPrev     = new StGLTextureButton(upperRegion, DISPL_X_REGION_UPPER + (i++) * ICON_WIDTH, DISPL_Y_REGION_UPPER);
    btnPrev->signals.onBtnClick.connect(myPlugin, &StImageViewer::doListPrev);

    btnNext     = new StGLTextureButton(upperRegion, DISPL_X_REGION_UPPER + (i++) * ICON_WIDTH, DISPL_Y_REGION_UPPER);
    btnNext->signals.onBtnClick.connect(myPlugin, &StImageViewer::doListNext);

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
    textPath = texturesPathRoot + "imageNext.std";
        btnNext->setTexturePath(&textPath);
    textPath = texturesPathRoot + "imagePrev.std";
        btnPrev->setTexturePath(&textPath);
}

/**
 * Main menu
 */
void StImageViewerGUI::createMainMenu() {
    const StRectI_t& aMargins = getRootMarginsPx();
    menu0Root = new StGLMenu(this, aMargins.left(), aMargins.top(), StGLMenu::MENU_HORIZONTAL, true);

    StGLMenu* aMenuMedia   = createMediaMenu();  // Root -> Media  menu
    StGLMenu* aMenuView    = createViewMenu();   // Root -> View   menu
    StGLMenu* aDevicesMenu = createOutputMenu(); // Root -> Output menu
    StGLMenu* aMenuHelp    = createHelpMenu();   // Root -> Help   menu

    // Attach sub menus to root
    menu0Root->addItem(tr(MENU_MEDIA), aMenuMedia);
    menu0Root->addItem(tr(MENU_VIEW),  aMenuView);
    menu0Root->addItem(myPlugin->StApplication::params.ActiveDevice->getActiveValue(), aDevicesMenu);
    menu0Root->addItem(tr(MENU_HELP),  aMenuHelp);
}

/**
 * Root -> Media menu
 */
StGLMenu* StImageViewerGUI::createMediaMenu() {
    StGLMenu* aMenuMedia = new StGLMenu(this, 0, 0, StGLMenu::MENU_VERTICAL);
    StGLMenu* aMenuSrcFormat = createSrcFormatMenu(); // Root -> Media -> Source format menu
    StGLMenu* aMenuOpenImage = createOpenImageMenu(); // Root -> Media -> Open image menu
    StGLMenu* aMenuSaveImage = createSaveImageMenu(); // Root -> Media -> Save image menu

    aMenuMedia->addItem(tr(MENU_MEDIA_OPEN_IMAGE),    aMenuOpenImage);
    aMenuMedia->addItem(tr(MENU_MEDIA_SAVE_IMAGE_AS), aMenuSaveImage);

    aMenuMedia->addItem("First File in folder")
              ->signals.onItemClick.connect(myPlugin, &StImageViewer::doListFirst);
    aMenuMedia->addItem("Prev File in folder")
              ->signals.onItemClick.connect(myPlugin, &StImageViewer::doListPrev);
    aMenuMedia->addItem("Next File in folder")
              ->signals.onItemClick.connect(myPlugin, &StImageViewer::doListNext);
    aMenuMedia->addItem("Last File in folder")
              ->signals.onItemClick.connect(myPlugin, &StImageViewer::doListLast);

    aMenuMedia->addItem(tr(MENU_MEDIA_SRC_FORMAT), aMenuSrcFormat);
    aMenuMedia->addItem(tr(MENU_MEDIA_QUIT))
              ->signals.onItemClick.connect(myPlugin, &StImageViewer::doQuit);
    return aMenuMedia;
}

/**
 * Root -> Media -> Open image menu
 */
StGLMenu* StImageViewerGUI::createOpenImageMenu() {
    StGLMenu* menu = new StGLMenu(this, 0, 0, StGLMenu::MENU_VERTICAL);
    menu->addItem(tr(MENU_MEDIA_OPEN_IMAGE_1), 1)
        ->signals.onItemClick.connectUnsafe(myPlugin, StImageViewer::doOpenFileDialog);
    menu->addItem(tr(MENU_MEDIA_OPEN_IMAGE_2), 2)
        ->signals.onItemClick.connect(myPlugin, &StImageViewer::doOpen2FilesDialog);
    return menu;
}

/**
 * Root -> Media -> Save image menu
 */
StGLMenu* StImageViewerGUI::createSaveImageMenu() {
    StGLMenu* menu = new StGLMenu(this, 0, 0, StGLMenu::MENU_VERTICAL);
    menu->addItem("JPEG stereo (*.jps)", size_t(StImageFile::ST_TYPE_JPEG))
        ->signals.onItemClick.connect(myPlugin, &StImageViewer::doSaveImageAs);
    menu->addItem("PNG stereo (*.pns)", size_t(StImageFile::ST_TYPE_PNG))
        ->signals.onItemClick.connect(myPlugin, &StImageViewer::doSaveImageAs);
    return menu;
}

/**
 * Root -> Media -> Source format menu
 */
StGLMenu* StImageViewerGUI::createSrcFormatMenu() {
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
    return aMenu;
}

/**
 * Root -> View menu
 */
StGLMenu* StImageViewerGUI::createViewMenu() {
    StGLMenu* aMenuView = new StGLMenu(this, 0, 0, StGLMenu::MENU_VERTICAL);
    StGLMenu* aMenuDispMode  = createDisplayModeMenu();  // Root -> View menu -> Output
    StGLMenu* aMenuDispRatio = createDisplayRatioMenu(); // Root -> View menu -> Display Ratio
    StGLMenu* aMenuTexFilter = createSmoothFilterMenu(); // Root -> View menu -> Smooth Filter
    StGLMenu* aMenuImgAdjust = createImageAdjustMenu();  // Root -> View menu -> Image Adjust

    aMenuView->addItem(tr(MENU_VIEW_DISPLAY_MODE),  aMenuDispMode);
    aMenuView->addItem(tr(MENU_VIEW_FULLSCREEN),    myPlugin->params.isFullscreen);
    aMenuView->addItem(tr(MENU_VIEW_RESET))
             ->signals.onItemClick.connect(myPlugin, &StImageViewer::doReset);
    aMenuView->addItem(tr(MENU_VIEW_SWAP_LR),       stImageRegion->params.swapLR);
    aMenuView->addItem(tr(MENU_VIEW_DISPLAY_RATIO), aMenuDispRatio);
    aMenuView->addItem(tr(MENU_VIEW_TEXFILTER),     aMenuTexFilter);
    aMenuView->addItem(tr(MENU_VIEW_IMAGE_ADJUST),  aMenuImgAdjust);
    return aMenuView;
}

/**
 * Root -> View menu -> Output
 */
StGLMenu* StImageViewerGUI::createDisplayModeMenu() {
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
StGLMenu* StImageViewerGUI::createDisplayRatioMenu() {
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
StGLMenu* StImageViewerGUI::createSmoothFilterMenu() {
    StGLMenu* aMenu = new StGLMenu(this, 0, 0, StGLMenu::MENU_VERTICAL);
    aMenu->addItem(tr(MENU_VIEW_TEXFILTER_NEAREST),
                   stImageRegion->params.textureFilter, StGLImageProgram::FILTER_NEAREST);
    aMenu->addItem(tr(MENU_VIEW_TEXFILTER_LINEAR),
                   stImageRegion->params.textureFilter, StGLImageProgram::FILTER_LINEAR);
    return aMenu;
}

/**
 * Root -> View menu -> Image Adjust
 */
StGLMenu* StImageViewerGUI::createImageAdjustMenu() {
    StGLMenu* aMenu = new StGLMenu(this, 0, 0, StGLMenu::MENU_VERTICAL);

    const StGLVec3 aBlack(0.0f, 0.0f, 0.0f);
    const StGLVec3 aGreen(0.4f, 0.8f, 0.4f);
    const StGLVec3 aRed  (1.0f, 0.0f, 0.0f);

    //aMenu->addItem(tr(MENU_VIEW_ADJUST_RESET));

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
 * Root -> Output menu
 */
StGLMenu* StImageViewerGUI::createOutputMenu() {
    StGLMenu* aMenu = new StGLMenu(this, 0, 0, StGLMenu::MENU_VERTICAL);

    StGLMenu* aMenuChangeDevice = new StGLMenu(this, 0, 0, StGLMenu::MENU_VERTICAL);

    const StHandle<StEnumParam>& aDevicesEnum = myPlugin->StApplication::params.ActiveDevice;
    const StArrayList<StString>& aValuesList  = aDevicesEnum->getValues();
    for(size_t aValIter = 0; aValIter < aValuesList.size(); ++aValIter) {
        aMenuChangeDevice->addItem(aValuesList[aValIter], aDevicesEnum, int32_t(aValIter));
    }

    aMenu->addItem(tr(MENU_CHANGE_DEVICE), aMenuChangeDevice);
    aMenu->addItem(tr(MENU_ABOUT_RENDERER))
         ->signals.onItemClick.connect(this, &StImageViewerGUI::doAboutRenderer);
    aMenu->addItem(tr(MENU_SHOW_FPS),      myPlugin->params.ToShowFps);
    aMenu->addItem(tr(MENU_VSYNC),         myPlugin->params.IsVSyncOn);

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

void StImageViewerGUI::doAboutProgram(const size_t ) {
    StGLMessageBox* aDialog = new StGLMessageBox(this,
          tr(ABOUT_DPLUGIN_NAME) + '\n'
        + tr(ABOUT_VERSION) + ": " + StVersionInfo::getSDKVersionString()
        + " " + StThread::getArchString()
        + "\n \n" + tr(ABOUT_DESCRIPTION),
        512, 300);
    aDialog->addButton("Close");
    aDialog->setVisibility(true, true);
    aDialog->stglInit();
}

void StImageViewerGUI::doUserTips(const size_t ) {
    StSocket::openURL("http://sview.ru/sview2009/usertips");
}

void StImageViewerGUI::doAboutSystem(const size_t ) {
    StString aTitle = "System Info";
    StString anInfo = getContext().stglFullInfo();
    StString aString = aTitle + "\n\n \n" + anInfo;
    StGLMessageBox* aDialog = new StGLMessageBox(this, aString, 512, 256);
    aDialog->addButton("Close");
    aDialog->setVisibility(true, true);
    aDialog->stglInit();
}

void StImageViewerGUI::doAboutImage(const size_t ) {
    StHandle<StFileNode>     aFileNode;
    StHandle<StStereoParams> aParams;
    StHandle<StImageInfo>    anExtraInfo;
    StArrayList<StString> anInfoList(10);
    if(myPlugin->getCurrentFile(aFileNode, aParams, anExtraInfo) && !anExtraInfo.isNull()) {
        for(size_t aKeyIter = 0; aKeyIter < anExtraInfo->myInfo.size(); ++aKeyIter) {
            const StArgument& aPair = anExtraInfo->myInfo.getFromIndex(aKeyIter);
            anInfoList.add(aPair.getKey() + ": " + aPair.getValue() + "\n");
        }
    }

    StString aTitle = "Image Info";
    StString anInfo;
    for(size_t anIter = 0; anIter < anInfoList.size(); ++anIter) {
        anInfo += anInfoList[anIter];
    }
    StString aString = aTitle + "\n\n \n" + anInfo;
    StGLMessageBox* aDialog = new StGLMessageBox(this, aString, 512, 300);
    aDialog->addButton("Close");
    aDialog->setVisibility(true, true);
    aDialog->stglInit();

}

void StImageViewerGUI::doCheckUpdates(const size_t ) {
    StSocket::openURL("http://www.sview.ru/download");
}

void StImageViewerGUI::doOpenLicense(const size_t ) {
    StSocket::openURL(StProcess::getStShareFolder()
                      + "info" + SYS_FS_SPLITTER
                      + "license.txt");
}

/**
 * Root -> Help menu
 */
StGLMenu* StImageViewerGUI::createHelpMenu() {
    StGLMenu* aMenu = new StGLMenu(this, 0, 0, StGLMenu::MENU_VERTICAL);
    StGLMenu* aMenuCheckUpdates = createCheckUpdatesMenu(); // Root -> Help -> Check updates menu
    StGLMenu* aMenuLanguage     = createLanguageMenu();     // Root -> Help -> Language menu

    aMenu->addItem(tr(MENU_HELP_ABOUT))
         ->signals.onItemClick.connect(this, &StImageViewerGUI::doAboutProgram);
    aMenu->addItem(tr(MENU_HELP_USERTIPS))
         ->signals.onItemClick.connect(this, &StImageViewerGUI::doUserTips);
    aMenu->addItem(tr(MENU_HELP_LICENSE))
         ->signals.onItemClick.connect(this, &StImageViewerGUI::doOpenLicense);
    aMenu->addItem(tr(MENU_HELP_SYSINFO))
         ->signals.onItemClick.connect(this, &StImageViewerGUI::doAboutSystem);
    aMenu->addItem(tr(MENU_HELP_UPDATES), aMenuCheckUpdates);
    aMenu->addItem(tr(MENU_HELP_LANGS),   aMenuLanguage);
    return aMenu;
}

/**
 * Root -> Help -> Check updates menu
 */
StGLMenu* StImageViewerGUI::createCheckUpdatesMenu() {
    StGLMenu* aMenu = new StGLMenu(this, 0, 0, StGLMenu::MENU_VERTICAL);
    aMenu->addItem(tr(MENU_HELP_UPDATES_NOW))
         ->signals.onItemClick.connect(this, &StImageViewerGUI::doCheckUpdates);
    aMenu->addItem(tr(MENU_HELP_UPDATES_DAY),   myPlugin->params.checkUpdatesDays, 1);
    aMenu->addItem(tr(MENU_HELP_UPDATES_WEEK),  myPlugin->params.checkUpdatesDays, 7);
    aMenu->addItem(tr(MENU_HELP_UPDATES_YEAR),  myPlugin->params.checkUpdatesDays, 355);
    aMenu->addItem(tr(MENU_HELP_UPDATES_NEVER), myPlugin->params.checkUpdatesDays, 0);
    return aMenu;
}

/**
 * Root -> Help -> Language menu
 */
StGLMenu* StImageViewerGUI::createLanguageMenu() {
    StGLMenu* aMenu = new StGLMenu(this, 0, 0, StGLMenu::MENU_VERTICAL);
    for(size_t aLangId = 0; aLangId < myLangMap->getLanguagesList().size(); ++aLangId) {
        aMenu->addItem(myLangMap->getLanguagesList()[aLangId], myLangMap->params.language, int32_t(aLangId));
    }
    return aMenu;
}

StImageViewerGUI::StImageViewerGUI(StImageViewer*  thePlugin,
                                   StWindow*       theWindow,
                                   StTranslations* theLangMap,
                                   const StHandle<StGLTextureQueue>& theTextureQueue)
: StGLRootWidget(),
  myPlugin(thePlugin),
  myWindow(theWindow),
  myLangMap(theLangMap),
  texturesPathRoot(StProcess::getStShareFolder() + "textures" + SYS_FS_SPLITTER),
  stTimeVisibleLock(true),
  //
  stImageRegion(NULL),
  stTextDescr(NULL),
  myMsgStack(NULL),
  //
  menu0Root(NULL),
  //
  upperRegion(NULL),
  btnOpen(NULL),
  btnPrev(NULL),
  btnNext(NULL),
  btnSwapLR(NULL),
  myBtnSrcFrmt(NULL),
  myBtnFull(NULL),
  //
  myFpsWidget(NULL),
  //
  isGUIVisible(true),
  isGUIMinimal(true) {
    setRootMarginsPx(myWindow->getMargins());
    const StRectI_t& aMargins = getRootMarginsPx();
    myPlugin->params.ToShowFps->signals.onChanged.connect(this, &StImageViewerGUI::doShowFPS);

    StHandle<StGLTextureQueue> aTextureQueue = theTextureQueue;
    if(aTextureQueue.isNull()) {
        aTextureQueue = new StGLTextureQueue(2);
    }
    stImageRegion = new StGLImageRegion(this, aTextureQueue, true);

    createUpperToolbar();

    // fullscreen button
    myBtnFull = new StGLTextureButton(this, -aMargins.right() - 8, -aMargins.bottom() - 8,
                                      StGLCorner(ST_VCORNER_BOTTOM, ST_HCORNER_RIGHT));
    myBtnFull->signals.onBtnClick.connect(myPlugin->params.isFullscreen.operator->(), &StBoolParam::doReverse);
    StString aTextPath = texturesPathRoot + "fullscr.std";
    myBtnFull->setTexturePath(&aTextPath);

    stTextDescr = new StGLDescription(this);

    // create Main menu
    createMainMenu();

    myMsgStack = new StGLMsgStack(this, myPlugin->getMessagesQueue());
    myMsgStack->setVisibility(true, true);

    if(myPlugin->params.ToShowFps->getValue()) {
        myFpsWidget = new StGLFpsLabel(this);
        myFpsWidget->setVisibility(true, true);
    }
}

StImageViewerGUI::~StImageViewerGUI() {
    //
}

bool StImageViewerGUI::toHideCursor() const {
    return (menu0Root != NULL) && !menu0Root->isVisible();
}

namespace {

    inline bool isPointIn(const StGLWidget* theWidget,
                          const StPointD_t& theCursorZo) {
        return theWidget != NULL
            && theWidget->isVisible()
            && theWidget->isPointIn(theCursorZo);
    }

};

void StImageViewerGUI::setVisibility(const StPointD_t& cursorZo, bool isMouseActive) {
    isGUIVisible = isMouseActive
        || stTimeVisibleLock.getElapsedTime() < 2.0
        || (upperRegion != NULL && !isGUIMinimal && upperRegion->isPointIn(cursorZo))
        || (menu0Root != NULL && menu0Root->isActive());
    if(isMouseActive) {
        stTimeVisibleLock.restart();
    }
    const bool toShowAll = !isGUIMinimal && isGUIVisible;

    // always visible
    StGLRootWidget::setVisibility(true, true);
    stImageRegion->setVisibility(true, true);

    if(menu0Root != NULL) {
        ///menu0Root->setVisibility(isGUIVisible, false);
        menu0Root->setVisibility(toShowAll, false);
    }

    if(upperRegion != NULL) {
        upperRegion->setVisibility(toShowAll);
        for(StGLWidget* child = upperRegion->getChildren()->getStart();
            child != NULL; child = child->getNext()) {
            child->setVisibility(toShowAll);
        }
    }
    if(myBtnFull != NULL) {
        myBtnFull->setVisibility(isGUIMinimal || toShowAll);
    }

    if(stTextDescr != NULL) {
        stTextDescr->setVisibility(true, true);
        if(::isPointIn(btnOpen, cursorZo)) {
            stTextDescr->setText(tr(IMAGE_OPEN));
        } else if(::isPointIn(btnPrev, cursorZo)) {
            stTextDescr->setText(tr(IMAGE_PREVIOUS));
        } else if(::isPointIn(btnNext, cursorZo)) {
            stTextDescr->setText(tr(IMAGE_NEXT));
        } else if(::isPointIn(btnSwapLR, cursorZo)) {
            size_t aLngId = stImageRegion->params.swapLR->getValue() ? SWAP_LR_ON : SWAP_LR_OFF;
            stTextDescr->setText(tr(aLngId));
        } else if(::isPointIn(myBtnFull, cursorZo)) {
            stTextDescr->setText(tr(FULLSCREEN));
        } else if(::isPointIn(myBtnSrcFrmt, cursorZo)) {
            size_t aLngId = MENU_SRC_FORMAT_AUTO;
            switch(myPlugin->params.srcFormat->getValue()) {
                case ST_V_SRC_MONO:                 aLngId = MENU_SRC_FORMAT_MONO;         break;
                case ST_V_SRC_SIDE_BY_SIDE:         aLngId = MENU_SRC_FORMAT_CROSS_EYED;   break;
                case ST_V_SRC_PARALLEL_PAIR:        aLngId = MENU_SRC_FORMAT_PARALLEL;     break;
                case ST_V_SRC_OVER_UNDER_RL:        aLngId = MENU_SRC_FORMAT_OVERUNDER_RL; break;
                case ST_V_SRC_OVER_UNDER_LR:        aLngId = MENU_SRC_FORMAT_OVERUNDER_LR; break;
                case ST_V_SRC_ROW_INTERLACE:        aLngId = MENU_SRC_FORMAT_INTERLACED;   break;
                case ST_V_SRC_ANAGLYPH_G_RB:        aLngId = MENU_SRC_FORMAT_ANA_RB;       break;
                case ST_V_SRC_ANAGLYPH_RED_CYAN:    aLngId = MENU_SRC_FORMAT_ANA_RC;       break;
                case ST_V_SRC_ANAGLYPH_YELLOW_BLUE: aLngId = MENU_SRC_FORMAT_ANA_YB;       break;
                //case ST_V_SRC_SEPARATE_FRAMES:      aLngId = MENU_SRC_FORMAT_SEPARATE;     break;
                default:
                case ST_V_SRC_AUTODETECT:           aLngId = MENU_SRC_FORMAT_AUTO;         break;
            }
            stTextDescr->setText(tr(BTN_SRC_FORMAT) + tr(aLngId));
        } else {
            stTextDescr->setVisibility(false, true);
        }
    }
}

void StImageViewerGUI::stglUpdate(const StPointD_t& pointZo) {
    StGLRootWidget::stglUpdate(pointZo);
    if(stTextDescr != NULL) {
        stTextDescr->setPoint(pointZo);
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

void StImageViewerGUI::stglResize(const StRectI_t& winRectPx) {
    const int aSizeX = winRectPx.width();
    const int aSizeY = winRectPx.height();
    stImageRegion->changeRectPx().bottom() = aSizeY;
    stImageRegion->changeRectPx().right()  = aSizeX;

    const StRectI_t& aMargins = myWindow->getMargins();
    const bool areNewMargins = aMargins != getRootMarginsPx();
    if(areNewMargins) {
        setRootMarginsPx(aMargins);
    }

    if(upperRegion != NULL) {
        upperRegion->changeRectPx().right() = aSizeX;
        isGUIMinimal = (aSizeY < 400 || aSizeX < 400);
    }
    if(areNewMargins) {
        if(upperRegion != NULL) {
            upperRegion->changeRectPx().left() = aMargins.left();
            upperRegion->changeRectPx().top()  = aMargins.top();
        }
        if(menu0Root != NULL) {
            menu0Root->changeRectPx().left() = aMargins.left();
            menu0Root->changeRectPx().top()  = aMargins.top();
            menu0Root->stglUpdateSubmenuLayout();
        }
    }

    StGLRootWidget::stglResize(winRectPx);
}

void StImageViewerGUI::stglDraw(unsigned int theView) {
    setLensDist(myPlugin->getMainWindow()->getLensDist());
    if(theView == ST_DRAW_LEFT
    && myFpsWidget != NULL) {
        myFpsWidget->update(myPlugin->getMainWindow()->isStereoOutput(),
                            myPlugin->getMainWindow()->getTargetFps());
    }
    StGLRootWidget::stglDraw(theView);
}

void StImageViewerGUI::doShowFPS(const bool ) {
    if(myFpsWidget != NULL) {
        delete myFpsWidget;
        myFpsWidget = NULL;
        return;
    }

    myFpsWidget = new StGLFpsLabel(this);
    myFpsWidget->setVisibility(true, true);
    myFpsWidget->stglInit();
}

void StImageViewerGUI::doAboutRenderer(const size_t ) {
    StString anAboutText = myPlugin->getMainWindow()->getRendererAbout();
    if(anAboutText.isEmpty()) {
        anAboutText = StString() + "Plugin '" + myPlugin->getMainWindow()->getRendererId() + "' doesn't provide description";
    }

    StGLMessageBox* aDialog = new StGLMessageBox(this, anAboutText, 512, 300);
    aDialog->addButton("Close");
    aDialog->setVisibility(true, true);
    aDialog->stglInit();
}

void StImageViewerGUI::showUpdatesNotify() {
    StGLMessageBox* aDialog = new StGLMessageBox(this, tr(UPDATES_NOTIFY));
    aDialog->addButton("Close");
    aDialog->setVisibility(true, true);
    aDialog->stglInit();
}
