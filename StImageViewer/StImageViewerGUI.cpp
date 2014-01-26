/**
 * Copyright © 2009-2014 Kirill Gavrilov <kirill@sview.ru>
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
#include <StGLWidgets/StGLScrollArea.h>
#include <StGLWidgets/StGLSwitchTextured.h>
#include <StGLWidgets/StGLTable.h>
#include <StGLWidgets/StGLTextureButton.h>
#include <StGLWidgets/StGLFpsLabel.h>

#include <StImage/StImageFile.h>
#include <StVersion.h>

#include "StImageViewerStrings.h"

// auxiliary pre-processor definition
#define stCTexture(theString) getTexturePath(stCString(theString))

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
    int aBtnIter = 0;
    const int aTop  = scale(DISPL_Y_REGION_UPPER);
    const int aLeft = scale(DISPL_X_REGION_UPPER);

    const StRectI_t& aMargins = getRootMarginsPx();
    myPanelUpper = new StGLWidget(this, aMargins.left(), aMargins.top(), StGLCorner(ST_VCORNER_TOP, ST_HCORNER_LEFT), scale(4096), scale(128));

    // append textured buttons
    myBtnOpen   = new StGLTextureButton(myPanelUpper, aLeft + (aBtnIter++) * ICON_WIDTH, aTop);
    myBtnOpen->signals.onBtnClick.connectUnsafe(myPlugin, StImageViewer::doOpenFileDialog);
    myBtnOpen->setTexturePath(stCTexture("openImage.png"));

    myBtnPrev   = new StGLTextureButton(myPanelUpper, aLeft + (aBtnIter++) * ICON_WIDTH, aTop);
    myBtnPrev->signals.onBtnClick.connect(myPlugin, &StImageViewer::doListPrev);
    myBtnPrev->setTexturePath(stCTexture("imagePrev.png"));

    myBtnNext   = new StGLTextureButton(myPanelUpper, aLeft + (aBtnIter++) * ICON_WIDTH, aTop);
    myBtnNext->signals.onBtnClick.connect(myPlugin, &StImageViewer::doListNext);
    myBtnNext->setTexturePath(stCTexture("imageNext.png"));

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
 * Main menu
 */
void StImageViewerGUI::createMainMenu() {
    const StRectI_t& aMargins = getRootMarginsPx();
    myMenuRoot = new StGLMenu(this, aMargins.left(), aMargins.top(), StGLMenu::MENU_HORIZONTAL, true);

    StGLMenu* aMenuMedia   = createMediaMenu();  // Root -> Media  menu
    StGLMenu* aMenuView    = createViewMenu();   // Root -> View   menu
    StGLMenu* aDevicesMenu = createOutputMenu(); // Root -> Output menu
    StGLMenu* aMenuHelp    = createHelpMenu();   // Root -> Help   menu

    // Attach sub menus to root
    myMenuRoot->addItem(tr(MENU_MEDIA), aMenuMedia);
    myMenuRoot->addItem(tr(MENU_VIEW),  aMenuView);
    myMenuRoot->addItem(myPlugin->StApplication::params.ActiveDevice->getActiveValue(), aDevicesMenu);
    myMenuRoot->addItem(tr(MENU_HELP),  aMenuHelp);
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
    aMenuMedia->addItem(tr(MENU_MEDIA_SRC_FORMAT),    aMenuSrcFormat);
    aMenuMedia->addItem(tr(MENU_MEDIA_FILE_INFO),     myPlugin->getAction(StImageViewer::Action_FileInfo));

    aMenuMedia->addItem("First File in folder")
              ->signals.onItemClick.connect(myPlugin, &StImageViewer::doListFirst);
    aMenuMedia->addItem("Prev File in folder")
              ->signals.onItemClick.connect(myPlugin, &StImageViewer::doListPrev);
    aMenuMedia->addItem("Next File in folder")
              ->signals.onItemClick.connect(myPlugin, &StImageViewer::doListNext);
    aMenuMedia->addItem("Last File in folder")
              ->signals.onItemClick.connect(myPlugin, &StImageViewer::doListLast);

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
    const IconSize anIconSize = scaleIcon(16);
    StGLMenu* aMenu  = new StGLMenu(this, 0, 0, StGLMenu::MENU_VERTICAL);
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
    return aMenu;
}

/**
 * Root -> View menu
 */
StGLMenu* StImageViewerGUI::createViewMenu() {
    StGLMenu* aMenuView = new StGLMenu(this, 0, 0, StGLMenu::MENU_VERTICAL);
    StGLMenu* aMenuDispMode  = createDisplayModeMenu();
    StGLMenu* aMenuDispRatio = createDisplayRatioMenu();
    StGLMenu* aMenuSurface   = createSurfaceMenu();
    StGLMenu* aMenuTexFilter = createSmoothFilterMenu();
    StGLMenu* aMenuImgAdjust = createImageAdjustMenu();

    aMenuView->addItem(tr(MENU_VIEW_DISPLAY_MODE),  aMenuDispMode);
    aMenuView->addItem(tr(MENU_VIEW_FULLSCREEN),    myPlugin->params.isFullscreen);
    aMenuView->addItem(tr(MENU_VIEW_RESET))
             ->signals.onItemClick.connect(myPlugin, &StImageViewer::doReset);
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
StGLMenu* StImageViewerGUI::createDisplayModeMenu() {
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
StGLMenu* StImageViewerGUI::createDisplayRatioMenu() {
    const IconSize anIconSize = scaleIcon(16);
    StGLMenu* aMenu = new StGLMenu(this, 0, 0, StGLMenu::MENU_VERTICAL);
    aMenu->addItem(tr(MENU_VIEW_DISPLAY_RATIO_SRC), myImage->params.displayRatio, StGLImageRegion::RATIO_AUTO)
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
    aMenu->addItem(tr(MENU_VIEW_KEEP_ON_RESTART),   myPlugin->params.toRestoreRatio);
    return aMenu;
}

/**
 * Root -> View menu -> Surface
 */
StGLMenu* StImageViewerGUI::createSurfaceMenu() {
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
StGLMenu* StImageViewerGUI::createSmoothFilterMenu() {
    StGLMenu* aMenu = new StGLMenu(this, 0, 0, StGLMenu::MENU_VERTICAL);
    aMenu->addItem(tr(MENU_VIEW_TEXFILTER_NEAREST),
                   myImage->params.textureFilter, StGLImageProgram::FILTER_NEAREST);
    aMenu->addItem(tr(MENU_VIEW_TEXFILTER_LINEAR),
                   myImage->params.textureFilter, StGLImageProgram::FILTER_LINEAR);
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

    aMenu->addItem(tr(MENU_VIEW_ADJUST_RESET), myPlugin->getAction(StImageViewer::Action_ImageAdjustReset));

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
    StGLMessageBox* aDialog = new StGLMessageBox(this, "",
          tr(ABOUT_DPLUGIN_NAME) + '\n'
        + tr(ABOUT_VERSION) + ": " + StVersionInfo::getSDKVersionString()
        + " " + StThread::getArchString()
        + "\n \n" + tr(ABOUT_DESCRIPTION),
        scale(512), scale(300));
    aDialog->addButton("Close");
    aDialog->setVisibility(true, true);
    aDialog->stglInit();
}

void StImageViewerGUI::doUserTips(const size_t ) {
    StSocket::openURL("http://sview.ru/sview2009/usertips");
}

void StImageViewerGUI::doAboutSystem(const size_t ) {
    const StString aTitle = "System Info";
    StGLMessageBox* aDialog = new StGLMessageBox(this, aTitle, "", scale(512), scale(256));

    StArgumentsMap anInfo;
    getContext().stglFullInfo(anInfo);
    StGLTable* aTable = new StGLTable(aDialog->getContent(), 0, 0, StGLCorner(ST_VCORNER_TOP, ST_HCORNER_CENTER));
    aTable->setVisibility(true, true);
    aTable->fillFromMap(anInfo, StGLVec3(1.0f, 1.0f, 1.0f), aDialog->getContent()->getRectPx().width(), aDialog->getContent()->getRectPx().width() / 2);

    aDialog->addButton("Close");
    aDialog->setVisibility(true, true);
    aDialog->stglInit();
}

void StImageViewerGUI::doAboutImage(const size_t ) {
    const StString aTitle = "Image Info";
    StGLMessageBox* aDialog = new StGLMessageBox(this, aTitle, "", scale(512), scale(300));

    StHandle<StFileNode>     aFileNode;
    StHandle<StStereoParams> aParams;
    StHandle<StImageInfo>    anExtraInfo;
    if(myPlugin->getCurrentFile(aFileNode, aParams, anExtraInfo)
    && !anExtraInfo.isNull()) {
        StGLTable* aTable = new StGLTable(aDialog->getContent(), 0, 0, StGLCorner(ST_VCORNER_TOP, ST_HCORNER_CENTER));
        aTable->setVisibility(true, true);
        aTable->fillFromMap(anExtraInfo->myInfo, StGLVec3(1.0f, 1.0f, 1.0f), aDialog->getContent()->getRectPx().width(), aDialog->getContent()->getRectPx().width() / 2);
        if(aTable->getRectPx().height() <= aDialog->getContent()->getRectPx().height()) {
            aTable->setCorner(StGLCorner(ST_VCORNER_CENTER, ST_HCORNER_CENTER));
        }
    } else {
        aDialog->setText("Information is unavailable");
    }

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
    StGLMenu* aMenuScale        = createScaleMenu();        // Root -> Help -> Scale Interface menu
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
    aMenu->addItem(tr(MENU_HELP_SCALE),   aMenuScale);
    aMenu->addItem(tr(MENU_HELP_UPDATES), aMenuCheckUpdates);
    aMenu->addItem(tr(MENU_HELP_LANGS),   aMenuLanguage);
    return aMenu;
}

/**
 * Root -> Help -> Check updates menu
 */
StGLMenu* StImageViewerGUI::createScaleMenu() {
    StGLMenu* aMenu = new StGLMenu(this, 0, 0, StGLMenu::MENU_VERTICAL);
    aMenu->addItem(tr(MENU_HELP_SCALE_SMALL),   myPlugin->params.ScaleAdjust,  StGLRootWidget::ScaleAdjust_Small);
    aMenu->addItem(tr(MENU_HELP_SCALE_NORMAL),  myPlugin->params.ScaleAdjust,  StGLRootWidget::ScaleAdjust_Normal);
    aMenu->addItem(tr(MENU_HELP_SCALE_BIG),     myPlugin->params.ScaleAdjust,  StGLRootWidget::ScaleAdjust_Big);
    aMenu->addItem(tr(MENU_HELP_SCALE_HIDPI2X), myPlugin->params.ScaleHiDPI2X);
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
  myTexturesFolder(StProcess::getStShareFolder() + "textures" + SYS_FS_SPLITTER),
  myVisibilityTimer(true),
  //
  myImage(NULL),
  myDescr(NULL),
  myMsgStack(NULL),
  //
  myMenuRoot(NULL),
  //
  myPanelUpper(NULL),
  myBtnOpen(NULL),
  myBtnPrev(NULL),
  myBtnNext(NULL),
  myBtnSwapLR(NULL),
  myBtnSrcFrmt(NULL),
  myBtnPlayList(NULL),
  myBtnFull(NULL),
  //
  myFpsWidget(NULL),
  //
  myIsVisibleGUI(true),
  myIsMinimalGUI(true) {
    const GLfloat aScale = myPlugin->params.ScaleHiDPI2X->getValue() ? 2.0f : myPlugin->params.ScaleHiDPI ->getValue();
    StGLRootWidget::setScale(aScale, (StGLRootWidget::ScaleAdjust )myPlugin->params.ScaleAdjust->getValue());

    setRootMarginsPx(myWindow->getMargins());
    const StRectI_t& aMargins = getRootMarginsPx();
    myPlugin->params.ToShowFps->signals.onChanged.connect(this, &StImageViewerGUI::doShowFPS);

    StHandle<StGLTextureQueue> aTextureQueue = theTextureQueue;
    if(aTextureQueue.isNull()) {
        aTextureQueue = new StGLTextureQueue(2);
    }
    myImage = new StGLImageRegion(this, aTextureQueue, true);

    createUpperToolbar();

    myBtnPlayList = new StGLTextureButton(this, -aMargins.right() - scale(8 + 8 + 32), -aMargins.bottom() - scale(8),
                                          StGLCorner(ST_VCORNER_BOTTOM, ST_HCORNER_RIGHT));
    myBtnPlayList->setTexturePath(iconTexture(stCString("playList"), scaleIcon(32)));

    // fullscreen button
    myBtnFull = new StGLTextureButton(this, -aMargins.right() - scale(8), -aMargins.bottom() - scale(8),
                                      StGLCorner(ST_VCORNER_BOTTOM, ST_HCORNER_RIGHT));
    myBtnFull->signals.onBtnClick.connect(myPlugin->params.isFullscreen.operator->(), &StBoolParam::doReverse);
    myBtnFull->setTexturePath(iconTexture(stCString("fullScreen"), scaleIcon(32)));

    myDescr = new StGLDescription(this);

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
    return (myMenuRoot != NULL) && !myMenuRoot->isVisible();
}

namespace {

    inline bool isPointIn(const StGLWidget* theWidget,
                          const StPointD_t& theCursorZo) {
        return theWidget != NULL
            && theWidget->isVisible()
            && theWidget->isPointIn(theCursorZo);
    }

};

void StImageViewerGUI::setVisibility(const StPointD_t& theCursor,
                                     bool              isMouseActive) {
    myIsVisibleGUI = isMouseActive
        || myVisibilityTimer.getElapsedTime() < 2.0
        || (myPanelUpper != NULL && !myIsMinimalGUI && myPanelUpper->isPointIn(theCursor))
        || (myMenuRoot   != NULL && myMenuRoot->isActive());
    if(isMouseActive) {
        myVisibilityTimer.restart();
    }
    const bool toShowAll = !myIsMinimalGUI && myIsVisibleGUI;

    // always visible
    StGLRootWidget::setVisibility(true, true);
    myImage->setVisibility(true, true);

    if(myMenuRoot != NULL) {
        myMenuRoot->setVisibility(toShowAll, false);
    }

    if(myPanelUpper != NULL) {
        myPanelUpper->setVisibility(toShowAll);
        for(StGLWidget* child = myPanelUpper->getChildren()->getStart();
            child != NULL; child = child->getNext()) {
            child->setVisibility(toShowAll);
        }
    }
    if(myBtnPlayList != NULL) {
        //myBtnPlayList->setVisibility(myIsMinimalGUI || toShowAll);
    }
    if(myBtnFull != NULL) {
        myBtnFull->setVisibility(myIsMinimalGUI || toShowAll);
    }

    if(myDescr != NULL) {
        myDescr->setVisibility(true, true);
        if(::isPointIn(myBtnOpen, theCursor)) {
            myDescr->setText(tr(IMAGE_OPEN));
        } else if(::isPointIn(myBtnPrev, theCursor)) {
            myDescr->setText(tr(IMAGE_PREVIOUS));
        } else if(::isPointIn(myBtnNext, theCursor)) {
            myDescr->setText(tr(IMAGE_NEXT));
        } else if(::isPointIn(myBtnSwapLR, theCursor)) {
            size_t aLngId = myImage->params.swapLR->getValue() ? SWAP_LR_ON : SWAP_LR_OFF;
            myDescr->setText(tr(aLngId));
        } else if(::isPointIn(myBtnPlayList, theCursor)) {
            myDescr->setText(tr(PLAYLIST));
        } else if(::isPointIn(myBtnFull, theCursor)) {
            myDescr->setText(tr(FULLSCREEN));
        } else if(::isPointIn(myBtnSrcFrmt, theCursor)) {
            size_t aLngId = MENU_SRC_FORMAT_AUTO;
            StFormatEnum aSrcFormat = (StFormatEnum )myPlugin->params.srcFormat->getValue();
            if(aSrcFormat == ST_V_SRC_AUTODETECT
            && !myImage->params.stereoFile.isNull()) {
                aSrcFormat = myImage->params.stereoFile->getSrcFormat();
            }
            switch(aSrcFormat) {
                case ST_V_SRC_MONO:                 aLngId = MENU_SRC_FORMAT_MONO;         break;
                case ST_V_SRC_SIDE_BY_SIDE:         aLngId = MENU_SRC_FORMAT_CROSS_EYED;   break;
                case ST_V_SRC_PARALLEL_PAIR:        aLngId = MENU_SRC_FORMAT_PARALLEL;     break;
                case ST_V_SRC_OVER_UNDER_RL:        aLngId = MENU_SRC_FORMAT_OVERUNDER_RL; break;
                case ST_V_SRC_OVER_UNDER_LR:        aLngId = MENU_SRC_FORMAT_OVERUNDER_LR; break;
                case ST_V_SRC_ROW_INTERLACE:        aLngId = MENU_SRC_FORMAT_INTERLACED;   break;
                case ST_V_SRC_ANAGLYPH_G_RB:        aLngId = MENU_SRC_FORMAT_ANA_RB;       break;
                case ST_V_SRC_ANAGLYPH_RED_CYAN:    aLngId = MENU_SRC_FORMAT_ANA_RC;       break;
                case ST_V_SRC_ANAGLYPH_YELLOW_BLUE: aLngId = MENU_SRC_FORMAT_ANA_YB;       break;
                case ST_V_SRC_SEPARATE_FRAMES:      aLngId = MENU_SRC_FORMAT_SEPARATE;     break;
                default:
                case ST_V_SRC_AUTODETECT:           aLngId = MENU_SRC_FORMAT_AUTO;         break;
            }
            myDescr->setText(tr(BTN_SRC_FORMAT) + tr(aLngId));
        } else {
            myDescr->setVisibility(false, true);
        }
    }
}

void StImageViewerGUI::stglUpdate(const StPointD_t& pointZo) {
    StGLRootWidget::stglUpdate(pointZo);
    if(myDescr != NULL) {
        myDescr->setPoint(pointZo);
    }
    if(myLangMap->wasReloaded()) {
        myPlugin->myToRecreateMenu = true;
        myLangMap->resetReloaded();
        StImageViewerStrings::loadDefaults(*myLangMap);
    }
}

void StImageViewerGUI::stglResize(const StGLBoxPx& theRectPx) {
    const int aSizeX = theRectPx.width();
    const int aSizeY = theRectPx.height();
    myImage->changeRectPx().bottom() = aSizeY;
    myImage->changeRectPx().right()  = aSizeX;

    const StRectI_t& aMargins = myWindow->getMargins();
    const bool areNewMargins = aMargins != getRootMarginsPx();
    if(areNewMargins) {
        setRootMarginsPx(aMargins);
    }

    if(myPanelUpper != NULL) {
        myPanelUpper->changeRectPx().right() = aSizeX;
        myIsMinimalGUI = (aSizeY < scale(400) || aSizeX < scale(400));
    }
    if(areNewMargins) {
        if(myPanelUpper != NULL) {
            myPanelUpper->changeRectPx().left() = aMargins.left();
            myPanelUpper->changeRectPx().top()  = aMargins.top();
        }
        if(myMenuRoot != NULL) {
            myMenuRoot->changeRectPx().left() = aMargins.left();
            myMenuRoot->changeRectPx().top()  = aMargins.top();
            myMenuRoot->stglUpdateSubmenuLayout();
        }
    }

    StGLRootWidget::stglResize(theRectPx);
}

void StImageViewerGUI::stglDraw(unsigned int theView) {
    setLensDist(myPlugin->getMainWindow()->getLensDist());
    if((theView == ST_DRAW_LEFT || theView == ST_DRAW_MONO)
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

    StGLMessageBox* aDialog = new StGLMessageBox(this, "", anAboutText, scale(512), scale(300));
    aDialog->addButton("Close");
    aDialog->setVisibility(true, true);
    aDialog->stglInit();
}

void StImageViewerGUI::showUpdatesNotify() {
    StGLMessageBox* aDialog = new StGLMessageBox(this, "", tr(UPDATES_NOTIFY));
    aDialog->addButton("Close");
    aDialog->setVisibility(true, true);
    aDialog->stglInit();
}
