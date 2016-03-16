/**
 * This source is a part of sView program.
 *
 * Copyright © Kirill Gavrilov, 2011-2016
 */

#include "StCADViewerGUI.h"
#include "StCADViewer.h"

#include <StGLWidgets/StGLDescription.h>
#include <StGLWidgets/StGLFpsLabel.h>
#include <StGLWidgets/StGLMenu.h>
#include <StGLWidgets/StGLMenuItem.h>
#include <StGLWidgets/StGLMessageBox.h>
#include <StGLWidgets/StGLMsgStack.h>
#include <StGLWidgets/StGLScrollArea.h>
#include <StGLWidgets/StGLTable.h>
#include <StGLWidgets/StGLTextureButton.h>

#include <StVersion.h>

#include "StCADViewerStrings.h"
using namespace StCADViewerStrings;

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
    aMenu->addItem(tr(MENU_VIEW_FULLSCREEN), myPlugin->params.isFullscreen);
#endif
    aMenu->addItem(tr(MENU_VIEW_TRIHEDRON),  myPlugin->params.toShowTrihedron);
    aMenu->addItem(tr(MENU_VIEW_PROJECTION), aMenuProj);
    aMenu->addItem(tr(MENU_VIEW_FITALL), myPlugin->getAction(StCADViewer::Action_FitAll));
    return aMenu;
}

/**
 * Root -> View menu -> Projection
 */
StGLMenu* StCADViewerGUI::createProjMenu() {
    StGLMenu* aMenu = new StGLMenu(this, 0, 0, StGLMenu::MENU_VERTICAL);
    aMenu->addItem(tr(MENU_VIEW_PROJ_ORTHO),
                   myPlugin->params.projectMode, ST_PROJ_ORTHO);
    aMenu->addItem(tr(MENU_VIEW_PROJ_PERSP),
                   myPlugin->params.projectMode, ST_PROJ_PERSP);
    aMenu->addItem(tr(MENU_VIEW_PROJ_STEREO),
                   myPlugin->params.projectMode, ST_PROJ_STEREO);
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

    aMenu->addItem("Show FPS", myPlugin->params.ToShowFps);

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

StCADViewerGUI::StCADViewerGUI(StCADViewer*    thePlugin,
                               StTranslations* theLangMap)
: StGLRootWidget(thePlugin->myResMgr),
  myPlugin(thePlugin),
  myLangMap(theLangMap),
  myMouseDescr(NULL),
  myMsgStack(NULL),
  myMenu0Root(NULL),
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
    createMainMenu();

    myMsgStack = new StGLMsgStack(this, myPlugin->getMessagesQueue());
    if(myPlugin->params.ToShowFps->getValue()) {
        myFpsWidget = new StGLFpsLabel(this);
    }
}

StCADViewerGUI::~StCADViewerGUI() {
    //
}

void StCADViewerGUI::setVisibility(const StPointD_t& , bool ) {
    if(myMenu0Root != NULL) {
        myMenu0Root->setOpacity(myIsGUIVisible ? 1.0f : 0.0f, false);
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
    if(myLangMap->wasReloaded()) {
        StGLMenu::DeleteWithSubMenus(myMenu0Root); myMenu0Root = NULL;
        createMainMenu();
        myMenu0Root->stglUpdateSubmenuLayout();
        myLangMap->resetReloaded();
    }
}

void StCADViewerGUI::stglResize(const StGLBoxPx& theRectPx) {
    const StMarginsI& aMargins = myPlugin->getMainWindow()->getMargins();
    const bool areNewMargins = aMargins != getRootMargins();
    if(areNewMargins) {
        changeRootMargins() = aMargins;
    }

    if(areNewMargins) {
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
}

void StCADViewerGUI::doOpenLicense(const size_t ) {
    StProcess::openURL(StProcess::getStShareFolder() + "info" + SYS_FS_SPLITTER + "license.txt");
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
