/**
 * Copyright © 2009-2015 Kirill Gavrilov <kirill@sview.ru>
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

#include <StSettings/StEnumParam.h>
#include <StThreads/StThread.h>

#include <StGL/StParams.h>
#include <StGLWidgets/StGLButton.h>
#include <StGLWidgets/StGLCombobox.h>
#include <StGLWidgets/StGLCheckboxTextured.h>
#include <StGLWidgets/StGLDescription.h>
#include <StGLWidgets/StGLImageRegion.h>
#include <StGLWidgets/StGLMenu.h>
#include <StGLWidgets/StGLMenuItem.h>
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
}

StInfoDialog::~StInfoDialog() {
    myPlugin->doSaveImageInfo(0);
}

void StImageViewerGUI::createDesktopUI() {
    if(myPlugin->params.ToShowFps->getValue()) {
        myFpsWidget = new StGLFpsLabel(this);
        myFpsWidget->setVisibility(true, true);
    }

    createUpperToolbar();

    const StMarginsI& aMargins = getRootMargins();
    myBtnPlayList = new StGLTextureButton(this, -aMargins.right - scale(8 + 8 + 32), -aMargins.bottom - scale(8),
                                          StGLCorner(ST_VCORNER_BOTTOM, ST_HCORNER_RIGHT));
    myBtnPlayList->setTexturePath(iconTexture(stCString("playList"), scaleIcon(32)));

    // fullscreen button
    if(myWindow->hasFullscreenMode()) {
        myBtnFull = new StGLTextureButton(this, -aMargins.right - scale(8), -aMargins.bottom - scale(8),
                                          StGLCorner(ST_VCORNER_BOTTOM, ST_HCORNER_RIGHT));
        myBtnFull->signals.onBtnClick.connect(myPlugin->params.isFullscreen.operator->(), &StBoolParam::doReverse);
        myBtnFull->setTexturePath(iconTexture(stCString("fullScreen"), scaleIcon(32)));
    }

    myDescr = new StGLDescription(this);

    // create Main menu
    createMainMenu();
}

/**
 * Create upper toolbar
 */
void StImageViewerGUI::createUpperToolbar() {
    int aBtnIter = 0;
    const int aTop  = scale(DISPL_Y_REGION_UPPER);
    const int aLeft = scale(DISPL_X_REGION_UPPER);

    const StMarginsI& aMargins = getRootMargins();
    myPanelUpper = new StGLWidget(this, aMargins.left, aMargins.top, StGLCorner(ST_VCORNER_TOP, ST_HCORNER_LEFT), scale(4096), scale(128));

    // append textured buttons
    myBtnOpen   = new StGLTextureButton(myPanelUpper, aLeft + (aBtnIter++) * ICON_WIDTH, aTop);
    myBtnOpen->signals.onBtnClick.connect(myPlugin, &StImageViewer::doOpen1FileDialog);
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
    aSrcBtn->addItem(StFormat_AUTO,          stCTexture("srcFrmtAuto.png"));
    aSrcBtn->addItem(StFormat_Mono,          stCTexture("srcFrmtMono.png"));
    aSrcBtn->addItem(StFormat_Rows,          stCTexture("srcFrmtInterlace.png"));
    aSrcBtn->addItem(StFormat_SideBySide_LR, stCTexture("srcFrmtSideBySide.png"));
    aSrcBtn->addItem(StFormat_SideBySide_RL, stCTexture("srcFrmtSideBySide.png"), true);
    aSrcBtn->addItem(StFormat_TopBottom_LR,  stCTexture("srcFrmtOverUnder.png"));
    aSrcBtn->addItem(StFormat_TopBottom_RL,  stCTexture("srcFrmtOverUnder.png"),  true);
    myBtnSrcFrmt = aSrcBtn;
}

/**
 * Main menu
 */
void StImageViewerGUI::createMainMenu() {
    const StMarginsI& aMargins = getRootMargins();
    myMenuRoot = new StGLMenu(this, aMargins.left, aMargins.top, StGLMenu::MENU_HORIZONTAL, true);

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

    if(myWindow->isMobile()) {
        aMenuMedia->addItem("Mobile UI", myPlugin->params.IsMobileUI);
    }

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
        ->signals.onItemClick.connect(myPlugin, &StImageViewer::doOpen1FileDialog);
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
    StGLMenu* aMenu  = new StGLMenu(this, 0, 0, StGLMenu::MENU_VERTICAL);
    fillSrcFormatMenu(aMenu);
    return aMenu;
}

void StImageViewerGUI::fillSrcFormatMenu(StGLMenu* theMenu) {
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
}

void StImageViewerGUI::doDisplayStereoFormatCombo(const size_t ) {
    StGLCombobox::ListBuilder aBuilder(this);
    fillSrcFormatMenu(aBuilder.getMenu());
    aBuilder.display();
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
    if(myWindow->hasFullscreenMode()) {
        aMenuView->addItem(tr(MENU_VIEW_FULLSCREEN),    myPlugin->params.isFullscreen);
    }
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
    const StArrayList<StString>& aValuesList = myImage->params.displayMode->getValues();
    for(size_t aValIter = 0; aValIter < aValuesList.size(); ++aValIter) {
        aMenu->addItem(aValuesList[aValIter], myImage->params.displayMode, int32_t(aValIter));
    }
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
    anItem->changeMargins().right = scale(100 + 16);
    StGLRangeFieldFloat32* aRange = new StGLRangeFieldFloat32(anItem, myImage->params.gamma,
                                                              -scale(16), 0, StGLCorner(ST_VCORNER_CENTER, ST_HCORNER_RIGHT));
    aRange->setFormat(stCString("%+01.2f"));
    aRange->setColor(StGLRangeFieldFloat32::FieldColor_Default,  aBlack);
    aRange->setColor(StGLRangeFieldFloat32::FieldColor_Positive, aGreen);
    aRange->setColor(StGLRangeFieldFloat32::FieldColor_Negative, aRed);
    aRange->setVisibility(true, true);

    anItem = aMenu->addItem(tr(MENU_VIEW_ADJUST_BRIGHTNESS));
    anItem->changeMargins().right = scale(100 + 16);
    aRange = new StGLRangeFieldFloat32(anItem, myImage->params.brightness,
                                       -scale(16), 0, StGLCorner(ST_VCORNER_CENTER, ST_HCORNER_RIGHT));
    aRange->setFormat(stCString("%+01.2f"));
    aRange->setColor(StGLRangeFieldFloat32::FieldColor_Default,  aBlack);
    aRange->setColor(StGLRangeFieldFloat32::FieldColor_Positive, aGreen);
    aRange->setColor(StGLRangeFieldFloat32::FieldColor_Negative, aRed);
    aRange->setVisibility(true, true);

    anItem = aMenu->addItem(tr(MENU_VIEW_ADJUST_SATURATION));
    anItem->changeMargins().right = scale(100 + 16);
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
        + tr(ABOUT_VERSION) + " " + StVersionInfo::getSDKVersionString()
        + "\n \n" + tr(ABOUT_DESCRIPTION).format("2007-2015", "kirill@sview.ru", "www.sview.ru"),
        scale(512), scale(300));
    aDialog->addButton(tr(BUTTON_CLOSE));
    aDialog->setVisibility(true, true);
    aDialog->stglInit();
}

void StImageViewerGUI::doUserTips(const size_t ) {
    StProcess::openURL("http://sview.ru/sview2009/usertips");
}

void StImageViewerGUI::doAboutSystem(const size_t ) {
    const StString aTitle = tr(ABOUT_SYSTEM);
    StGLMessageBox* aDialog = new StGLMessageBox(this, aTitle, "", scale(512), scale(256));

    StArgumentsMap anInfo;
    anInfo.add(StDictEntry("CPU cores", StString(StThread::countLogicalProcessors()) + StString(" logical processor(s)")));
    getContext().stglFullInfo(anInfo);
    StGLTable* aTable = new StGLTable(aDialog->getContent(), 0, 0, StGLCorner(ST_VCORNER_TOP, ST_HCORNER_CENTER));
    aTable->setVisibility(true, true);
    aTable->fillFromMap(anInfo, StGLVec3(1.0f, 1.0f, 1.0f), aDialog->getContent()->getRectPx().width(), aDialog->getContent()->getRectPx().width() / 2);

    aDialog->addButton(tr(BUTTON_CLOSE));
    aDialog->setVisibility(true, true);
    aDialog->stglInit();
}

void StImageViewerGUI::doAboutImage(const size_t ) {
    StHandle<StImageInfo>& anExtraInfo = myPlugin->myFileInfo;
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

    const StString aTitle  = tr(DIALOG_FILE_INFO);
    StInfoDialog*  aDialog = new StInfoDialog(myPlugin, this, aTitle, scale(512), scale(300));

    // translate known metadata tag names
    for(size_t aMapIter = 0; aMapIter < anExtraInfo->Info.size(); ++aMapIter) {
        StDictEntry& anEntry = anExtraInfo->Info.changeValue(aMapIter);
        anEntry.changeName() = myLangMap->getValue(anEntry.getKey());
    }
    const int aWidthMax  = aDialog->getContent()->getRectPx().width();
    int       aRowLast   = (int )anExtraInfo->Info.size();
    const int aNbRowsMax = aRowLast + 2;

    StGLTable* aTable = new StGLTable(aDialog->getContent(), 0, 0, StGLCorner(ST_VCORNER_TOP, ST_HCORNER_CENTER));
    aTable->setupTable(aNbRowsMax, 2);
    aTable->setVisibility(true, true);
    aTable->fillFromMap(anExtraInfo->Info, StGLVec3(1.0f, 1.0f, 1.0f), aWidthMax, aWidthMax / 2);

    // add stereoscopic format info
    const StFormat anActiveSrcFormat = aParams->ToSwapLR
                                     ? st::formatReversed(aParams->StereoFormat)
                                     : aParams->StereoFormat;
    const int aTextMaxWidth = aWidthMax - (aTable->getItemMargins().left + aTable->getItemMargins().right);
    StGLTableItem& aSrcFormatItem = aTable->changeElement(aRowLast++, 0); aSrcFormatItem.setColSpan(2);
    StGLTextArea*  aSrcFormatText = new StGLTextArea(&aSrcFormatItem, 0, 0, StGLCorner(ST_VCORNER_CENTER, ST_HCORNER_CENTER));
    aSrcFormatText->setupAlignment(StGLTextFormatter::ST_ALIGN_X_CENTER,
                                   StGLTextFormatter::ST_ALIGN_Y_TOP);
    aSrcFormatText->setText(StString("\n") + tr(BTN_SRC_FORMAT) + " " + trSrcFormat(anActiveSrcFormat));
    aSrcFormatText->setTextColor(StGLVec3(1.0f, 1.0f, 1.0f));
    aSrcFormatText->setVisibility(true, true);
    aSrcFormatText->stglInitAutoHeightWidth(aTextMaxWidth);

    // warn about wrong/missing stereoscopic format information
    StString aSrcInfo;
    StGLVec3 anExtraColor(1.0f, 1.0f, 1.0f);
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
        aText->setVisibility(true, true);
        aText->stglInitAutoHeightWidth(aTextMaxWidth);
    }
    aTable->updateLayout();

    if(anExtraInfo->IsSavable
    && !aSrcInfo.isEmpty()) {
        StGLButton* aSaveBtn = aDialog->addButton(tr(BUTTON_SAVE_METADATA));
        aSaveBtn->setUserData(1);
        aSaveBtn->signals.onBtnClick += stSlot(myPlugin, &StImageViewer::doSaveImageInfo);
    }

    aDialog->addButton(tr(BUTTON_CLOSE), true);
    aDialog->setVisibility(true, true);
    aDialog->stglInit();
}

void StImageViewerGUI::doMobileSettings(const size_t ) {
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
    aTable->setVisibility(true, true);
    aTable->fillFromParams(aParams, StGLVec3(1.0f, 1.0f, 1.0f), aWidthMax);

    aDialog->addButton(tr(BUTTON_CLOSE), true);
    aDialog->setVisibility(true, true);
    aDialog->stglInit();
}

void StImageViewerGUI::doCheckUpdates(const size_t ) {
    StProcess::openURL("http://www.sview.ru/download");
}

void StImageViewerGUI::doOpenLicense(const size_t ) {
    StProcess::openURL(StProcess::getStShareFolder()
                       + "info" + SYS_FS_SPLITTER
                       + "license.txt");
}

/**
 * Root -> Help menu
 */
StGLMenu* StImageViewerGUI::createHelpMenu() {
    StGLMenu* aMenu = new StGLMenu(this, 0, 0, StGLMenu::MENU_VERTICAL);
    StGLMenu* aMenuScale        = createScaleMenu();        // Root -> Help -> Scale Interface menu
#if !defined(ST_NO_UPDATES_CHECK)
    StGLMenu* aMenuCheckUpdates = createCheckUpdatesMenu(); // Root -> Help -> Check updates menu
#endif
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
#if !defined(ST_NO_UPDATES_CHECK)
    aMenu->addItem(tr(MENU_HELP_UPDATES), aMenuCheckUpdates);
#endif
    aMenu->addItem(tr(MENU_HELP_LANGS),   aMenuLanguage);
    return aMenu;
}

/**
 * Root -> Help -> Scale Interface menu
 */
StGLMenu* StImageViewerGUI::createScaleMenu() {
    StGLMenu* aMenu = new StGLMenu(this, 0, 0, StGLMenu::MENU_VERTICAL);
    aMenu->addItem(tr(MENU_HELP_SCALE_SMALL),   myPlugin->params.ScaleAdjust,  StGLRootWidget::ScaleAdjust_Small);
    aMenu->addItem(tr(MENU_HELP_SCALE_NORMAL),  myPlugin->params.ScaleAdjust,  StGLRootWidget::ScaleAdjust_Normal);
    aMenu->addItem(tr(MENU_HELP_SCALE_BIG),     myPlugin->params.ScaleAdjust,  StGLRootWidget::ScaleAdjust_Big);
    aMenu->addItem(tr(MENU_HELP_SCALE_HIDPI2X), myPlugin->params.ScaleHiDPI2X);
    aMenu->addItem("Mobile UI",                 myPlugin->params.IsMobileUI);
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

void StImageViewerGUI::createMobileUI() {
    createMobileUpperToolbar();
    createMobileBottomToolbar();

    if(myPlugin->params.ToShowFps->getValue()) {
        myFpsWidget = new StGLFpsLabel(this);
        myFpsWidget->setVisibility(true, true);
    }
}

/**
 * Create upper toolbar
 */
void StImageViewerGUI::createMobileUpperToolbar() {
    const IconSize anIconSize = scaleIcon(32);
    const int      anIconStep = scale(56);
    StMarginsI aButtonMargins;
    aButtonMargins.setValues(12);

    const StMarginsI& aRootMargins = getRootMargins();
    myPanelUpper = new StGLWidget(this, aRootMargins.left, aRootMargins.top, StGLCorner(ST_VCORNER_TOP, ST_HCORNER_LEFT), scale(4096), scale(56));

    int aBtnIter = 0;

    StGLTextureButton* aSrcBtn = new StGLTextureButton(myPanelUpper, (aBtnIter++) * anIconStep, 0,
                                                       StGLCorner(ST_VCORNER_TOP, ST_HCORNER_LEFT), StFormat_NB);
    aSrcBtn->changeMargins() = aButtonMargins;
    aSrcBtn->signals.onBtnClick += stSlot(this, &StImageViewerGUI::doDisplayStereoFormatCombo);
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
    myBtnActualSrcFrmt = aSrcBtn;

    aBtnIter = 0;
    StGLTextureButton* aBtnEx = new StGLTextureButton(myPanelUpper, (aBtnIter--) * (-anIconStep), 0,
                                                      StGLCorner(ST_VCORNER_TOP, ST_HCORNER_RIGHT));
    aBtnEx->changeMargins() = aButtonMargins;
    aBtnEx->setTexturePath(iconTexture(stCString("actionOverflow"), anIconSize));
    aBtnEx->signals.onBtnClick += stSlot(this, &StImageViewerGUI::doShowMobileExMenu);

    /**myBtnSwapLR = new StGLCheckboxTextured(myPanelUpper, myImage->params.swapLR,
                                           stCTexture("swapLRoff.png"),
                                           stCTexture("swapLRon.png"),
                                           aLeft + (aBtnIter++) * ICON_WIDTH, aTop,
                                           StGLCorner(ST_VCORNER_TOP, ST_HCORNER_LEFT));*/
}

/**
 * Create bottom toolbar
 */
void StImageViewerGUI::createMobileBottomToolbar() {
    const IconSize anIconSize = scaleIcon(32);
    const int      anIconStep = scale(56);
    StMarginsI aButtonMargins;
    aButtonMargins.setValues(12);

    const StMarginsI& aRootMargins = getRootMargins();
    myPanelBottom = new StGLWidget(this, aRootMargins.left, -aRootMargins.bottom, StGLCorner(ST_VCORNER_BOTTOM, ST_HCORNER_LEFT), scale(4096), scale(56));

    int aBtnIter = 0;
    myBtnPrev = new StGLTextureButton(myPanelBottom, (aBtnIter++) * anIconStep, 0);
    myBtnPrev->signals.onBtnClick += stSlot(myPlugin, &StImageViewer::doListPrev);
    myBtnPrev->setTexturePath(iconTexture(stCString("actionBack"), anIconSize));
    myBtnPrev->changeMargins() = aButtonMargins;

    myBtnNext = new StGLTextureButton(myPanelBottom, (aBtnIter++) * anIconStep, 0);
    myBtnNext->signals.onBtnClick += stSlot(myPlugin, &StImageViewer::doListNext);
    myBtnNext->setTexturePath(iconTexture(stCString("actionNext"), anIconSize));
    myBtnNext->changeMargins() = aButtonMargins;

    StGLTextureButton* aBtnInfo = new StGLTextureButton(myPanelBottom, (aBtnIter++) * anIconStep, 0);
    aBtnInfo->signals.onBtnClick += stSlot(myPlugin, &StImageViewer::doAboutImage);
    aBtnInfo->setTexturePath(iconTexture(stCString("actionInfo"),  anIconSize));
    aBtnInfo->changeMargins() = aButtonMargins;
}

void StImageViewerGUI::doShowMobileExMenu(const size_t ) {
    const IconSize anIconSize = scaleIcon(16);
    const int aTop = scale(56);

    StHandle<StImageInfo>&   anExtraInfo = myPlugin->myFileInfo;
    StHandle<StFileNode>     aFileNode;
    StHandle<StStereoParams> aParams;
    if(anExtraInfo.isNull()
    && !myPlugin->getCurrentFile(aFileNode, aParams, anExtraInfo)) {
        anExtraInfo.nullify();
    }

    StGLMenu*     aMenu  = new StGLMenu(this, 0, aTop, StGLMenu::MENU_VERTICAL_COMPACT);
    StGLMenuItem* anItem = NULL;
    aMenu->setCorner(StGLCorner(ST_VCORNER_TOP, ST_HCORNER_RIGHT));
    aMenu->setContextual(true);
    if(!anExtraInfo.isNull()) {
        anItem = aMenu->addItem(tr(BUTTON_DELETE), myPlugin->getAction(StImageViewer::Action_DeleteFile));
        anItem->setIcon(iconTexture(stCString("actionDiscard"), anIconSize));
        anExtraInfo.nullify();
    }
    anItem = aMenu->addItem(tr(MENU_HELP_ABOUT));
    anItem->setIcon(iconTexture(stCString("actionHelp"),      anIconSize));
    anItem->signals.onItemClick += stSlot(this, &StImageViewerGUI::doAboutProgram);
    //anItem = aMenu->addItem(myPlugin->StApplication::params.ActiveDevice->getActiveValue());
    anItem = aMenu->addItem("Settings");
    anItem->setIcon(iconTexture(stCString("actionSettings"),  anIconSize));
    anItem->signals.onItemClick += stSlot(this, &StImageViewerGUI::doMobileSettings);
    anItem = aMenu->addItem("Slideshow", myPlugin->getAction(StImageViewer::Action_SlideShow));
    anItem->setIcon(iconTexture(stCString("actionSlideShow"), anIconSize));
    aMenu->setVisibility(true, true);
    aMenu->stglInit();
}

StImageViewerGUI::StImageViewerGUI(StImageViewer*  thePlugin,
                                   StWindow*       theWindow,
                                   StTranslations* theLangMap,
                                   const StHandle<StGLTextureQueue>& theTextureQueue)
: StGLRootWidget(thePlugin->myResMgr),
  myPlugin(thePlugin),
  myWindow(theWindow),
  myLangMap(theLangMap),
  myVisibilityTimer(true),
  //
  myImage(NULL),
  myDescr(NULL),
  myMsgStack(NULL),
  //
  myMenuRoot(NULL),
  //
  myPanelUpper(NULL),
  myPanelBottom(NULL),
  myBtnOpen(NULL),
  myBtnPrev(NULL),
  myBtnNext(NULL),
  myBtnSwapLR(NULL),
  myBtnSrcFrmt(NULL),
  myBtnActualSrcFrmt(NULL),
  myBtnPlayList(NULL),
  myBtnFull(NULL),
  //
  myFpsWidget(NULL),
  //
  myIsVisibleGUI(true),
  myIsMinimalGUI(true) {
    const GLfloat aScale = myPlugin->params.ScaleHiDPI2X->getValue() ? 2.0f : myPlugin->params.ScaleHiDPI ->getValue();
    setScale(aScale, (StGLRootWidget::ScaleAdjust )myPlugin->params.ScaleAdjust->getValue());
    setMobile(myPlugin->params.IsMobileUI->getValue());

    changeRootMargins() = myWindow->getMargins();
    myPlugin->params.ToShowFps->signals.onChanged.connect(this, &StImageViewerGUI::doShowFPS);

    StHandle<StGLTextureQueue> aTextureQueue = theTextureQueue;
    if(aTextureQueue.isNull()) {
        aTextureQueue = new StGLTextureQueue(2);
    }

    myImage = new StGLImageRegion(this, aTextureQueue, true);
    myImage->params.displayMode->setName(tr(MENU_VIEW_DISPLAY_MODE));
    myImage->params.displayMode->changeValues()[StGLImageRegion::MODE_STEREO]     = tr(MENU_VIEW_DISPLAY_MODE_STEREO);
    myImage->params.displayMode->changeValues()[StGLImageRegion::MODE_ONLY_LEFT]  = tr(MENU_VIEW_DISPLAY_MODE_LEFT);
    myImage->params.displayMode->changeValues()[StGLImageRegion::MODE_ONLY_RIGHT] = tr(MENU_VIEW_DISPLAY_MODE_RIGHT);
    myImage->params.displayMode->changeValues()[StGLImageRegion::MODE_PARALLEL]   = tr(MENU_VIEW_DISPLAY_MODE_PARALLEL);
    myImage->params.displayMode->changeValues()[StGLImageRegion::MODE_CROSSYED]   = tr(MENU_VIEW_DISPLAY_MODE_CROSSYED);

    if(isMobile()) {
        createMobileUI();
    } else {
        createDesktopUI();
    }

    myMsgStack = new StGLMsgStack(this, myPlugin->getMessagesQueue());
    myMsgStack->setVisibility(true, true);
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

}

size_t StImageViewerGUI::trSrcFormatId(const StFormat theSrcFormat) {
    switch(theSrcFormat) {
        case StFormat_Mono:                 return MENU_SRC_FORMAT_MONO;
        case StFormat_SideBySide_LR:        return MENU_SRC_FORMAT_PARALLEL;
        case StFormat_SideBySide_RL:        return MENU_SRC_FORMAT_CROSS_EYED;
        case StFormat_TopBottom_LR:         return MENU_SRC_FORMAT_OVERUNDER_LR;
        case StFormat_TopBottom_RL:         return MENU_SRC_FORMAT_OVERUNDER_RL;
        case StFormat_Rows:                 return MENU_SRC_FORMAT_INTERLACED;
        case StFormat_AnaglyphGreenMagenta: return MENU_SRC_FORMAT_ANA_RB;
        case StFormat_AnaglyphRedCyan:      return MENU_SRC_FORMAT_ANA_RC;
        case StFormat_AnaglyphYellowBlue:   return MENU_SRC_FORMAT_ANA_YB;
        case StFormat_SeparateFrames:       return MENU_SRC_FORMAT_SEPARATE;
        default:
        case StFormat_AUTO:                 return MENU_SRC_FORMAT_AUTO;
    }
}

void StImageViewerGUI::setVisibility(const StPointD_t& theCursor,
                                     bool              isMouseActive) {
    myIsVisibleGUI = isMouseActive
        || myVisibilityTimer.getElapsedTime() < 2.0
        || (myPanelUpper  != NULL && !myIsMinimalGUI && myPanelUpper ->isPointIn(theCursor))
        || (myPanelBottom != NULL && !myIsMinimalGUI && myPanelBottom->isPointIn(theCursor))
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
        for(StGLWidget* aChildIter = myPanelUpper->getChildren()->getStart();
            aChildIter != NULL; aChildIter = aChildIter->getNext()) {
            aChildIter->setVisibility(toShowAll);
        }
    }
    if(myPanelBottom != NULL) {
        myPanelBottom->setVisibility(toShowAll);
        for(StGLWidget* aChildIter = myPanelBottom->getChildren()->getStart();
            aChildIter != NULL; aChildIter = aChildIter->getNext()) {
            aChildIter->setVisibility(toShowAll);
        }
    }
    if(myBtnPlayList != NULL) {
        //myBtnPlayList->setVisibility(myIsMinimalGUI || toShowAll);
    }
    if(myBtnFull != NULL) {
        myBtnFull->setVisibility(myIsMinimalGUI || toShowAll);
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
    if(myBtnActualSrcFrmt != NULL) {
        myBtnActualSrcFrmt->setFaceId(aSrcFormat != StFormat_AUTO ? aSrcFormat : StFormat_Mono);
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
            myDescr->setText(tr(BTN_SRC_FORMAT) + "\n" + trSrcFormat(aSrcFormat));
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

    const StMarginsI& aMargins = myWindow->getMargins();
    const bool areNewMargins = aMargins != getRootMargins();
    if(areNewMargins) {
        changeRootMargins() = aMargins;
    }

    if(myPanelUpper != NULL) {
        myPanelUpper->changeRectPx().right() = aSizeX;
        myIsMinimalGUI = myWindow->isMovable() && !isMobile()
                     && (aSizeY < scale(400) || aSizeX < scale(400));
    }
    if(myPanelBottom != NULL) {
        myPanelBottom->changeRectPx().right() = aSizeX;
    }
    if(areNewMargins) {
        if(myPanelUpper != NULL) {
            myPanelUpper->changeRectPx().left() = aMargins.left;
            myPanelUpper->changeRectPx().top()  = aMargins.top;
        }
        if(myPanelBottom != NULL) {
            myPanelBottom->changeRectPx().left() = aMargins.left;
            myPanelBottom->changeRectPx().top()  = aMargins.top;
        }
        if(myMenuRoot != NULL) {
            myMenuRoot->changeRectPx().left() = aMargins.left;
            myMenuRoot->changeRectPx().top()  = aMargins.top;
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
    aDialog->addButton(tr(BUTTON_CLOSE));
    aDialog->setVisibility(true, true);
    aDialog->stglInit();
}

void StImageViewerGUI::showUpdatesNotify() {
    StGLMessageBox* aDialog = new StGLMessageBox(this, "", tr(UPDATES_NOTIFY));
    aDialog->addButton(tr(BUTTON_CLOSE));
    aDialog->setVisibility(true, true);
    aDialog->stglInit();
}
