/**
 * This source is a part of sView program.
 *
 * Copyright © Kirill Gavrilov, 2011-2016
 */

#include "StCADViewerGUI.h"

#include "StCADViewer.h"
#include "StCADLoader.h"

#include <StGLWidgets/StGLCheckboxTextured.h>
#include <StGLWidgets/StGLDescription.h>
#include <StGLWidgets/StGLFpsLabel.h>
#include <StGLWidgets/StGLMenu.h>
#include <StGLWidgets/StGLMenuItem.h>
#include <StGLWidgets/StGLMessageBox.h>
#include <StGLWidgets/StGLMsgStack.h>
#include <StGLWidgets/StGLOpenFile.h>
#include <StGLWidgets/StGLPlayList.h>
#include <StGLWidgets/StGLSeekBar.h>
#include <StGLWidgets/StGLScrollArea.h>
#include <StGLWidgets/StGLTable.h>
#include <StGLWidgets/StGLTextureButton.h>

#include <StVersion.h>

#include "StCADViewerStrings.h"
using namespace StCADViewerStrings;

// auxiliary pre-processor definition
#define stCTexture(theString) getTexturePath(stCString(theString))
#define stCMenuIcon(theString) iconTexture(stCString(theString), myMenuIconSize)

StInfoDialog::~StInfoDialog() {
    //myPlugin->doSaveImageInfo(0);
}

void StCADViewerGUI::createToolbarOnTop() {
    StMarginsI aButtonMargins;
    const IconSize anIconSize = scaleIcon(32, aButtonMargins);
    const int      anIconStep = scale(56);
    aButtonMargins.extend(scale(12));

    const StMarginsI& aRootMargins = getRootMargins();
    myPanelUpper = new StGLContainer(this, aRootMargins.left, aRootMargins.top, StGLCorner(ST_VCORNER_TOP, ST_HCORNER_LEFT), scale(4096), scale(56));

    // left side
    {
        int aBtnIter = 0;
        StGLTextureButton* aBtnOpen = new StGLTextureButton(myPanelUpper, (aBtnIter++) * anIconStep, 0);
        aBtnOpen->signals.onBtnClick.connect(this, &StCADViewerGUI::doOpenFile);
        aBtnOpen->setTexturePath(iconTexture(stCString("actionOpen"), anIconSize));
        aBtnOpen->setDrawShadow(true);
        aBtnOpen->changeMargins() = aButtonMargins;
     }

    // right side
    {
        int aBtnIter = 0;
        StGLTextureButton* aBtnEx = new StGLTextureButton(myPanelUpper, (aBtnIter--) * (-anIconStep), 0,
                                                          StGLCorner(ST_VCORNER_TOP, ST_HCORNER_RIGHT));
        aBtnEx->changeMargins() = aButtonMargins;
        aBtnEx->setTexturePath(iconTexture(stCString("actionOverflow"), anIconSize));
        aBtnEx->setDrawShadow(true);
        aBtnEx->signals.onBtnClick += stSlot(this, &StCADViewerGUI::doShowMobileExMenu);
    }
}

void StCADViewerGUI::createToolbarOnBottom() {
    StMarginsI aButtonMargins;
    const IconSize anIconSize = scaleIcon(32, aButtonMargins);
    const int      anIconStep = scale(56);
    aButtonMargins.extend(scale(12));

    const StMarginsI& aRootMargins = getRootMargins();
    myPanelBottom = new StGLContainer(this, aRootMargins.left, -aRootMargins.bottom, StGLCorner(ST_VCORNER_BOTTOM, ST_HCORNER_LEFT), scale(4096), scale(56));

    // left side
    {
      //int aBtnIter = 0;
      //myBtnPrev = new StGLTextureButton(myPanelBottom, (aBtnIter++) * anIconStep, 0);
    }

    // right side
    {
      int aBtnIter = 0;
      StGLTextureButton* aBtnZoomIn = new StGLTextureButton(myPanelBottom, (aBtnIter++) * (-anIconStep), 0,
                                                            StGLCorner(ST_VCORNER_TOP, ST_HCORNER_RIGHT));
      aBtnZoomIn->changeMargins() = aButtonMargins;
      aBtnZoomIn->setTexturePath(iconTexture(stCString("actionZoomIn"), anIconSize));
      aBtnZoomIn->setDrawShadow(true);
      aBtnZoomIn->setUserData(StCADViewer::Action_ZoomIn);
      aBtnZoomIn->signals.onBtnHold += stSlot(this, &StCADViewerGUI::doAction);

      StGLTextureButton* aBtnZoomOut = new StGLTextureButton(myPanelBottom, (aBtnIter++) * (-anIconStep), 0,
          StGLCorner(ST_VCORNER_TOP, ST_HCORNER_RIGHT));
      aBtnZoomOut->changeMargins() = aButtonMargins;
      aBtnZoomOut->setTexturePath(iconTexture(stCString("actionZoomOut"), anIconSize));
      aBtnZoomOut->setDrawShadow(true);
      aBtnZoomOut->setUserData(StCADViewer::Action_ZoomOut);
      aBtnZoomOut->signals.onBtnHold += stSlot(this, &StCADViewerGUI::doAction);

      StGLTextureButton* aBtnFitAll = new StGLTextureButton(myPanelBottom, (aBtnIter++) * (-anIconStep), 0,
          StGLCorner(ST_VCORNER_TOP, ST_HCORNER_RIGHT));
      aBtnFitAll->changeMargins() = aButtonMargins;
      aBtnFitAll->setTexturePath(iconTexture(stCString("actionFitAll"), anIconSize));
      aBtnFitAll->setDrawShadow(true);
      aBtnFitAll->setUserData(StCADViewer::Action_FitAll);
      aBtnFitAll->signals.onBtnHold += stSlot(this, &StCADViewerGUI::doAction);

      StGLCheckboxTextured* aBtnList = new StGLCheckboxTextured(myPanelBottom, myPlugin->params.ToShowPlayList,
                                           iconTexture(stCString("actionVideoPlaylistOff"), anIconSize),
                                           iconTexture(stCString("actionVideoPlaylist"),    anIconSize),
                                           (aBtnIter++) * (-anIconStep), 0,
                                           StGLCorner(ST_VCORNER_TOP, ST_HCORNER_RIGHT));
      aBtnList->setDrawShadow(true);
      aBtnList->changeMargins() = aButtonMargins;
    }
}

/**
 * Main menu
 */
void StCADViewerGUI::createMainMenu() {
    myMenu0Root = new StGLMenu(this, 0, 0, StGLMenu::MENU_HORIZONTAL, true);

    StGLMenu* aMenuView = createViewMenu(); // Root -> View menu
    StGLMenu* aMenuHelp = createHelpMenu(); // Root -> Help menu

    // Attach sub menus to root
    myMenu0Root->addItem(tr(MENU_VIEW), aMenuView);
    myMenu0Root->addItem(tr(MENU_HELP), aMenuHelp);
}

/**
 * Root -> View menu
 */
StGLMenu* StCADViewerGUI::createViewMenu() {
    StGLMenu* aMenu = new StGLMenu(this, 0, 0, StGLMenu::MENU_VERTICAL);

    StGLMenu* aMenuProj = createProjMenu(); // Root -> View menu -> Projection

#if !defined(__ANDROID__)
    aMenu->addItem(myPlugin->params.IsFullscreen);
#endif
    aMenu->addItem(myPlugin->params.ToShowTrihedron);
    aMenu->addItem(tr(MENU_VIEW_PROJECTION), aMenuProj);
    aMenu->addItem(tr(MENU_VIEW_FITALL), myPlugin->getAction(StCADViewer::Action_FitAll));
    return aMenu;
}

/**
 * Root -> View menu -> Projection
 */
StGLMenu* StCADViewerGUI::createProjMenu() {
    StGLMenu* aMenu = new StGLMenu(this, 0, 0, StGLMenu::MENU_VERTICAL);
    aMenu->addItem(myPlugin->params.ProjectMode, ST_PROJ_ORTHO);
    aMenu->addItem(myPlugin->params.ProjectMode, ST_PROJ_PERSP);
    aMenu->addItem(myPlugin->params.ProjectMode, ST_PROJ_STEREO);
    return aMenu;
}

/**
 * Root -> Help menu
 */
StGLMenu* StCADViewerGUI::createHelpMenu() {
    StGLMenu* aMenu = new StGLMenu(this, 0, 0, StGLMenu::MENU_VERTICAL);
    StGLMenu* aLangMenu = createLanguageMenu(); // Root -> Help -> Language menu

    aMenu->addItem(tr(MENU_HELP_ABOUT))
         ->signals.onItemClick.connect(this, &StCADViewerGUI::doAboutProgram);

    aMenu->addItem(tr(MENU_HELP_LICENSE))
         ->signals.onItemClick.connect(this, &StCADViewerGUI::doOpenLicense);

    aMenu->addItem(myPlugin->params.ToShowFps);

    aMenu->addItem(tr(MENU_HELP_LANGS), aLangMenu);
    return aMenu;
}

/**
 * Root -> Help -> Language menu
 */
StGLMenu* StCADViewerGUI::createLanguageMenu() {
    StGLMenu* aMenu = new StGLMenu(this, 0, 0, StGLMenu::MENU_VERTICAL);
    for(size_t aLangId = 0; aLangId < myLangMap->getLanguagesList().size(); ++aLangId) {
        aMenu->addItem(myLangMap->getLanguagesList()[aLangId], myLangMap->params.language, int32_t(aLangId));
    }
    return aMenu;
}

void StCADViewerGUI::doMobileSettings(const size_t ) {
    const StHandle<StWindow>& aRend = myPlugin->getMainWindow();
    StParamsList aParams;
    aParams.add(myPlugin->StApplication::params.ActiveDevice);
    aRend->getOptions(aParams);
    aParams.add(myPlugin->params.ToShowFps);
    aParams.add(myLangMap->params.language);
    //aParams.add(myPlugin->params.IsMobileUI);
    aParams.add(myPlugin->params.ToShowTrihedron);
    aParams.add(myPlugin->params.ProjectMode);

    StInfoDialog* aDialog = new StInfoDialog(myPlugin, this, tr(MENU_HELP_SETTINGS), scale(512), scale(300));

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

void StCADViewerGUI::doOpenFile(const size_t ) {
    StGLOpenFile* aDialog = new StGLOpenFile(this, tr(DIALOG_OPEN_FILE), tr(BUTTON_CLOSE));
    aDialog->setMimeList(StCADLoader::ST_CAD_MIME_LIST);
#if defined(_WIN32)
    //
#else
    aDialog->addHotItem("/", "Root");
#endif
    aDialog->addHotItem(getResourceManager()->getFolder(StResourceManager::FolderId_SdCard));
    aDialog->addHotItem(getResourceManager()->getFolder(StResourceManager::FolderId_Downloads));
    aDialog->addHotItem(getResourceManager()->getFolder(StResourceManager::FolderId_Pictures));
    aDialog->addHotItem(getResourceManager()->getFolder(StResourceManager::FolderId_Photos));
    aDialog->signals.onFileSelected = stSlot(myPlugin, &StCADViewer::doOpen1FileFromGui);

    if(myPlugin->params.LastFolder.isEmpty()) {
        StHandle<StFileNode> aCurrFile = myPlugin->myPlayList->getCurrentFile();
        if(!aCurrFile.isNull()) {
            myPlugin->params.LastFolder = aCurrFile->isEmpty() ? aCurrFile->getFolderPath() : aCurrFile->getValue(0)->getFolderPath();
        }
    }
    aDialog->openFolder(myPlugin->params.LastFolder);
    setModalDialog(aDialog);
}

void StCADViewerGUI::doShowMobileExMenu(const size_t ) {
    const int aTop = scale(56);

    StGLMenu*     aMenu  = new StGLMenu(this, 0, aTop, StGLMenu::MENU_VERTICAL_COMPACT, true);
    StGLMenuItem* anItem = NULL;
    aMenu->setCorner(StGLCorner(ST_VCORNER_TOP, ST_HCORNER_RIGHT));
    aMenu->setContextual(true);

    anItem = aMenu->addItem(tr(MENU_HELP_ABOUT));
    anItem->setIcon(stCMenuIcon("actionHelp"));
    anItem->signals.onItemClick += stSlot(this, &StCADViewerGUI::doAboutProgram);
    //anItem = aMenu->addItem(myPlugin->StApplication::params.ActiveDevice->getActiveValue());
    anItem = aMenu->addItem(tr(MENU_HELP_SETTINGS));
    anItem->setIcon(stCMenuIcon("actionSettings"));
    anItem->signals.onItemClick += stSlot(this, &StCADViewerGUI::doMobileSettings);
    aMenu->stglInit();
    setFocus(aMenu);
}

StCADViewerGUI::StCADViewerGUI(StCADViewer*    thePlugin,
                               StTranslations* theLangMap,
                               const StHandle<StPlayList>& thePlayList)
: StGLRootWidget(thePlugin->myResMgr),
  myPlugin(thePlugin),
  myLangMap(theLangMap),
  myMouseDescr(NULL),
  myMsgStack(NULL),
  myPlayList(NULL),
  myMenu0Root(NULL),
  myPanelUpper(NULL),
  myPanelBottom(NULL),
  myStereoIODBar(NULL),
  myStereoIODLab(NULL),
  myZFocusBar(NULL),
  myZFocusLab(NULL),
  myFpsWidget(NULL),
  myIsGUIVisible(true) {
    //const GLfloat aScale = myPlugin->params.ScaleHiDPI2X->getValue() ? 2.0f : myPlugin->params.ScaleHiDPI ->getValue();
    //setScale(aScale, (StGLRootWidget::ScaleAdjust )myPlugin->params.ScaleAdjust->getValue());
    //setMobile(myPlugin->params.IsMobileUI->getValue());
    const GLfloat aScale = myPlugin->myWindow->getScaleFactor();
    setScale(aScale, StGLRootWidget::ScaleAdjust_Normal);
    setMobile(StWindow::isMobile());
    changeRootMargins() = myPlugin->myWindow->getMargins();

    myPlugin->params.ToShowFps->signals.onChanged.connect(this, &StCADViewerGUI::doShowFPS);

    myMouseDescr = new StGLDescription(this);

    // create Main menu
    //createMainMenu();

    createToolbarOnTop();
    createToolbarOnBottom();

    StMarginsI aButtonMargins;
    const int  anIconStep = scale(56);
    aButtonMargins.extend(scale(12));

    myZFocusBar = new StGLSeekBar(this, 0, scale(18));
    myZFocusBar->changeRectPx().left()  = scale(8);
    myZFocusBar->changeRectPx().right() = myZFocusBar->getRectPx().left() + 4 * anIconStep + scale(8);
    myZFocusBar->changeRectPx().moveTopTo(-(anIconStep - myZFocusBar->getRectPx().height()) / 2);
    myZFocusBar->setCorner(StGLCorner(ST_VCORNER_BOTTOM, ST_HCORNER_LEFT));
    myZFocusBar->signals.onSeekClick  = stSlot(this, &StCADViewerGUI::doZFocusSet);
    myZFocusBar->signals.onSeekScroll = stSlot(this, &StCADViewerGUI::doZFocusScroll);
    myZFocusBar->setMoveTolerance(1);
    myZFocusBar->changeMargins().left  = scale(8);
    myZFocusBar->changeMargins().right = scale(8);

    myZFocusLab = new StGLTextArea(myZFocusBar, 0, 0, StGLCorner(ST_VCORNER_TOP, ST_HCORNER_LEFT),
                                   myZFocusBar->getRectPx().width(), myZFocusBar->getRectPx().height(), StGLTextArea::SIZE_NORMAL);
    myZFocusLab->setBorder(false);
    myZFocusLab->setTextColor(StGLVec3(1.0f, 1.0f, 1.0f));
    myZFocusLab->setupAlignment(StGLTextFormatter::ST_ALIGN_X_CENTER,
                                StGLTextFormatter::ST_ALIGN_Y_CENTER);
    myZFocusLab->setDrawShadow(true);

    myStereoIODBar = new StGLSeekBar(this, 0, scale(4));
    myStereoIODBar->changeRectPx().left()  = scale(8);
    myStereoIODBar->changeRectPx().right() = myStereoIODBar->getRectPx().left() + 4 * anIconStep + scale(8);
    myStereoIODBar->changeRectPx().moveTopTo(-(anIconStep * 2 - myStereoIODBar->getRectPx().height()) / 2);
    myStereoIODBar->setCorner(StGLCorner(ST_VCORNER_BOTTOM, ST_HCORNER_LEFT));
    myStereoIODBar->signals.onSeekClick  = stSlot(this, &StCADViewerGUI::doStereoIODSet);
    myStereoIODBar->signals.onSeekScroll = stSlot(this, &StCADViewerGUI::doStereoIODScroll);
    myStereoIODBar->setMoveTolerance(1);
    myStereoIODBar->changeMargins().left  = scale(8);
    myStereoIODBar->changeMargins().right = scale(8);

    myStereoIODLab = new StGLTextArea(myStereoIODBar, 0, 0, StGLCorner(ST_VCORNER_TOP, ST_HCORNER_LEFT),
                                      myStereoIODBar->getRectPx().width(), myStereoIODBar->getRectPx().height(), StGLTextArea::SIZE_NORMAL);
    myStereoIODLab->setBorder(false);
    myStereoIODLab->setTextColor(StGLVec3(1.0f, 1.0f, 1.0f));
    myStereoIODLab->setupAlignment(StGLTextFormatter::ST_ALIGN_X_CENTER,
                                   StGLTextFormatter::ST_ALIGN_Y_CENTER);
    myStereoIODLab->setDrawShadow(true);

    myPlayList = new StGLPlayList(this, thePlayList);
    myPlayList->setCorner(StGLCorner(ST_VCORNER_TOP, ST_HCORNER_RIGHT));
    myPlayList->changeFitMargins().top    = scale(56);
    myPlayList->changeFitMargins().bottom = scale(100);
    //myPlayList->changeMargins().bottom    = scale(56);
    myPlayList->setOpacity(myPlugin->params.ToShowPlayList->getValue() ? 1.0f : 0.0f, false);
    myPlayList->signals.onOpenItem = stSlot(myPlugin, &StCADViewer::doFileNext);

    myMsgStack = new StGLMsgStack(this, myPlugin->getMessagesQueue());
    if(myPlugin->params.ToShowFps->getValue()) {
        myFpsWidget = new StGLFpsLabel(this);
    }
}

StCADViewerGUI::~StCADViewerGUI() {
    //
}

void StCADViewerGUI::setVisibility(const StPointD_t& , bool ) {
    const bool toShowPlayList = myPlugin->params.ToShowPlayList->getValue();
    //const float anOpacity = (float )myVisLerp.perform(toShowAll, toForceHide);
    const float anOpacity = myIsGUIVisible ? 1.0f : 0.0f;

    if(myMenu0Root != NULL) {
        myMenu0Root->setOpacity(anOpacity, false);
    }
    if(myPanelUpper != NULL) {
        myPanelUpper->setOpacity(anOpacity, true);
    }
    if(myPanelBottom != NULL) {
        myPanelBottom->setOpacity(anOpacity, true);
    }
    if(myPlayList != NULL) {
        myPlayList->setOpacity(toShowPlayList ? anOpacity : 0.0f, true);
    }

    if(myMouseDescr != NULL) {
        myMouseDescr->setOpacity(0.0f, true);
    }
}

void StCADViewerGUI::stglUpdate(const StPointD_t& theCursorZo) {
    StGLRootWidget::stglUpdate(theCursorZo);
    if(myMouseDescr != NULL) {
        myMouseDescr->setPoint(theCursorZo);
    }
    if(myZFocusBar != NULL) {
        char aBuff[128];
        stsprintf(aBuff, 128, "ZFocus: %3.0f%%", 100.0f * myPlugin->params.ZFocus->getNormalizedValue());
        myZFocusBar->setProgress(myPlugin->params.ZFocus->getNormalizedValue());
        myZFocusLab->setText(aBuff);
        myZFocusBar->setOpacity(myPlugin->params.ProjectMode->getValue() == ST_PROJ_STEREO ? 1.0f : 0.0f, false);
    }
    if(myStereoIODBar != NULL) {
        char aBuff[128];
        stsprintf(aBuff, 128, "IOD: %3.0f%%", 100.0f * myPlugin->params.StereoIOD->getNormalizedValue());
        myStereoIODBar->setProgress(myPlugin->params.StereoIOD->getNormalizedValue());
        myStereoIODLab->setText(aBuff);
        myStereoIODBar->setOpacity(myPlugin->params.ProjectMode->getValue() == ST_PROJ_STEREO ? 1.0f : 0.0f, false);
    }
}

void StCADViewerGUI::stglResize(const StGLBoxPx& theRectPx) {
    const int aSizeX = theRectPx.width();
    const StMarginsI& aMargins = myPlugin->getMainWindow()->getMargins();
    const bool areNewMargins = aMargins != getRootMargins();
    if(areNewMargins) {
        changeRootMargins() = aMargins;
    }

    if(myPanelUpper != NULL) {
        myPanelUpper->changeRectPx().right() = aSizeX;
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
        if(myMenu0Root != NULL) {
            myMenu0Root->changeRectPx().left() = aMargins.left;
            myMenu0Root->changeRectPx().top()  = aMargins.top;
            myMenu0Root->stglUpdateSubmenuLayout();
        }
    }

    StGLRootWidget::stglResize(theRectPx);
}

void StCADViewerGUI::stglDraw(unsigned int theView) {
    setLensDist(myPlugin->getMainWindow()->getLensDist());
    if((theView == ST_DRAW_LEFT || theView == ST_DRAW_MONO)
    && myFpsWidget != NULL) {
        myFpsWidget->update(myPlugin->getMainWindow()->isStereoOutput(),
                            myPlugin->getMainWindow()->getTargetFps(),
                            myPlugin->getMainWindow()->getStatistics());
    }
    StGLRootWidget::stglDraw(theView);
}

void StCADViewerGUI::doAboutProgram(const size_t ) {
    const StGLVec3 THE_WHITE(1.0f, 1.0f, 1.0f);
    const StString anAbout = tr(ABOUT_DPLUGIN_NAME) + '\n'
                           + tr(ABOUT_VERSION) + " " + StVersionInfo::getSDKVersionString()
                           + "\n \n" + tr(ABOUT_DESCRIPTION).format("2011-2016", "kirill@sview.ru", "www.sview.ru");

    StArgumentsMap anInfo;
    anInfo.add(StDictEntry("CPU cores", StString(StThread::countLogicalProcessors()) + StString(" logical processor(s)")));
    getContext().stglFullInfo(anInfo);

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

    aDialog->addButton(tr(BUTTON_CLOSE));
    aDialog->stglInit();
    setModalDialog(aDialog);
}

void StCADViewerGUI::doOpenLicense(const size_t ) {
    StProcess::openURL(StProcess::getStShareFolder() + "info" + SYS_FS_SPLITTER + "license.txt");
}

void StCADViewerGUI::doAction(const size_t theActionId,
                              const double theDuration) {
    myPlugin->invokeAction((int )theActionId, theDuration);
}

void StCADViewerGUI::doShowFPS(const bool ) {
    if(myFpsWidget != NULL) {
        delete myFpsWidget;
        myFpsWidget = NULL;
        return;
    }

    myFpsWidget = new StGLFpsLabel(this);
    myFpsWidget->stglInit();
}

void StCADViewerGUI::doZFocusSet(const int    theMouseBtn,
                                 const double theValue) {
    if(theMouseBtn == ST_MOUSE_LEFT) {
        myPlugin->params.ZFocus->setNormalizedValue((float )theValue);
    }
}

void StCADViewerGUI::doZFocusScroll(const double theDelta) {
    if(theDelta > 0.001) {
        myPlugin->params.ZFocus->increment();
    } else if(theDelta < -0.001) {
        myPlugin->params.ZFocus->decrement();
    }
}

void StCADViewerGUI::doStereoIODSet(const int    theMouseBtn,
                                    const double theValue) {
    if(theMouseBtn == ST_MOUSE_LEFT) {
        myPlugin->params.StereoIOD->setNormalizedValue((float )theValue);
    }
}

void StCADViewerGUI::doStereoIODScroll(const double theDelta) {
    if(theDelta > 0.001) {
        myPlugin->params.StereoIOD->increment();
    } else if(theDelta < -0.001) {
        myPlugin->params.StereoIOD->decrement();
    }
}
