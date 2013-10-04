/**
 * This source is a part of sView program.
 *
 * Copyright © Kirill Gavrilov, 2011-2013
 */

#include "StCADViewerGUI.h"
#include "StCADViewer.h"

#include <StGLWidgets/StGLDescription.h>
#include <StGLWidgets/StGLFpsLabel.h>
#include <StGLWidgets/StGLMenu.h>
#include <StGLWidgets/StGLMenuItem.h>
#include <StGLWidgets/StGLMessageBox.h>
#include <StGLWidgets/StGLTextureButton.h>
#include <StGLWidgets/StGLMsgStack.h>

#include <StSocket/StSocket.h>
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
    myMenu0Root->addItem(myLangMap->changeValueId(MENU_VIEW,
                         "View"), aMenuView);
    myMenu0Root->addItem(myLangMap->changeValueId(MENU_HELP,
                         "Help"), aMenuHelp);
}

/**
 * Root -> View menu
 */
StGLMenu* StCADViewerGUI::createViewMenu() {
    StGLMenu* aMenu = new StGLMenu(this, 0, 0, StGLMenu::MENU_VERTICAL);

    StGLMenu* aMenuProj = createProjMenu(); // Root -> View menu -> Projection
    StGLMenu* aMenuFill = createFillMenu(); // Root -> View menu -> Fill Mode

    aMenu->addItem(myLangMap->changeValueId(MENU_VIEW_FULLSCREEN, "Fullscreen"),
                   myPlugin->params.isFullscreen);
    aMenu->addItem(myLangMap->changeValueId(MENU_VIEW_NORMALS,    "Show Normals"),
                   myPlugin->params.toShowNormals);
    aMenu->addItem(myLangMap->changeValueId(MENU_VIEW_TRIHEDRON,  "Show Trihedron"),
                   myPlugin->params.toShowTrihedron);
    aMenu->addItem(myLangMap->changeValueId(MENU_VIEW_TWOSIDES,   "Two sides lighting"),
                   myPlugin->params.isLightTwoSides);
    aMenu->addItem(myLangMap->changeValueId(MENU_VIEW_PROJECTION, "Projection"), aMenuProj);
    aMenu->addItem(myLangMap->changeValueId(MENU_VIEW_FILLMODE,   "Fill Mode"),  aMenuFill);
    aMenu->addItem(myLangMap->changeValueId(MENU_VIEW_FITALL,     "Fit ALL"))
              ->signals.onItemClick.connect(myPlugin, &StCADViewer::doFitALL);
    return aMenu;
}

/**
 * Root -> View menu -> Projection
 */
StGLMenu* StCADViewerGUI::createProjMenu() {
    StGLMenu* aMenu = new StGLMenu(this, 0, 0, StGLMenu::MENU_VERTICAL);
    aMenu->addItem(myLangMap->changeValueId(MENU_VIEW_PROJ_ORTHO,  "Orthogonal"),
                   myPlugin->params.projectMode, ST_PROJ_ORTHO);
    aMenu->addItem(myLangMap->changeValueId(MENU_VIEW_PROJ_PERSP,  "Perspective"),
                   myPlugin->params.projectMode, ST_PROJ_PERSP);
    aMenu->addItem(myLangMap->changeValueId(MENU_VIEW_PROJ_STEREO, "Stereo"),
                   myPlugin->params.projectMode, ST_PROJ_STEREO);
    return aMenu;
}

/**
 * Root -> View menu -> Fill Mode
 */
StGLMenu* StCADViewerGUI::createFillMenu() {
    StGLMenu* aMenu = new StGLMenu(this, 0, 0, StGLMenu::MENU_VERTICAL);
    aMenu->addItem(myLangMap->changeValueId(MENU_VIEW_FILL_MESH,        "Mesh"),
                   myPlugin->params.fillMode, ST_FILL_MESH);
    aMenu->addItem(myLangMap->changeValueId(MENU_VIEW_FILL_SHADED,      "Shaded"),
                   myPlugin->params.fillMode, ST_FILL_SHADING);
    aMenu->addItem(myLangMap->changeValueId(MENU_VIEW_FILL_SHADED_MESH, "Shaded + Mesh"),
                   myPlugin->params.fillMode, ST_FILL_SHADED_MESH);
    return aMenu;
}

/**
 * Root -> Help menu
 */
StGLMenu* StCADViewerGUI::createHelpMenu() {
    StGLMenu* aMenu = new StGLMenu(this, 0, 0, StGLMenu::MENU_VERTICAL);
    StGLMenu* aLangMenu = createLanguageMenu(); // Root -> Help -> Language menu

    aMenu->addItem(myLangMap->changeValueId(MENU_HELP_ABOUT,   "About..."))
         ->signals.onItemClick.connect(this, &StCADViewerGUI::doAboutProgram);

    aMenu->addItem(myLangMap->changeValueId(MENU_HELP_LICENSE, "License text"))
         ->signals.onItemClick.connect(this, &StCADViewerGUI::doOpenLicense);

    aMenu->addItem("Show FPS", myPlugin->params.ToShowFps);

    aMenu->addItem(myLangMap->changeValueId(MENU_HELP_LANGS,   "Language"), aLangMenu);
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

StCADViewerGUI::StCADViewerGUI(StCADViewer* thePlugin)
: StGLRootWidget(),
  myPlugin(thePlugin),
  myLangMap(new StTranslations(StCADViewer::ST_DRAWER_PLUGIN_NAME)),
  myTexturesRoot(StProcess::getStShareFolder() + "textures" + SYS_FS_SPLITTER),
  myMouseDescr(NULL),
  myMsgStack(NULL),
  myMenu0Root(NULL),
  myFpsWidget(NULL),
  myIsGUIVisible(true) {
    myPlugin->params.ToShowFps->signals.onChanged.connect(this, &StCADViewerGUI::doShowFPS);

    myMouseDescr = new StGLDescription(this);

    // create Main menu
    createMainMenu();

    myMsgStack = new StGLMsgStack(this, myPlugin->getMessagesQueue());
    myMsgStack->setVisibility(true, true);

    if(myPlugin->params.ToShowFps->getValue()) {
        myFpsWidget = new StGLFpsLabel(this);
        myFpsWidget->setVisibility(true, true);
    }
}

StCADViewerGUI::~StCADViewerGUI() {
    //
}

void StCADViewerGUI::setVisibility(const StPointD_t& , bool ) {
    // always visible
    StGLRootWidget::setVisibility(true, true);
    if(myMenu0Root != NULL) {
        myMenu0Root->setVisibility(myIsGUIVisible, false);
    }

    if(myMouseDescr != NULL) {
        myMouseDescr->setVisibility(false, true);
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
    const StRectI_t& aMargins = myPlugin->getMainWindow()->getMargins();
    const bool areNewMargins = aMargins != getRootMarginsPx();
    if(areNewMargins) {
        setRootMarginsPx(aMargins);
    }

    if(areNewMargins) {
        if(myMenu0Root != NULL) {
            myMenu0Root->changeRectPx().left() = aMargins.left();
            myMenu0Root->changeRectPx().top()  = aMargins.top();
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
                            myPlugin->getMainWindow()->getTargetFps());
    }
    StGLRootWidget::stglDraw(theView);
}

void StCADViewerGUI::doAboutProgram(const size_t ) {
    const StString& aTitle     = myLangMap->changeValueId(ABOUT_DPLUGIN_NAME, "sView - Tiny CAD Viewer");
    const StString& aVerString = myLangMap->changeValueId(ABOUT_VERSION,      "version");
    const StString& aDescr     = myLangMap->changeValueId(ABOUT_DESCRIPTION,
        "CAD viewer allows you to view CAD files in formats IGES, STEP, BREP using OCCT.\n"
        "(C) 2011-2013 Kirill Gavrilov (kirill@sview.ru).\nOfficial site: www.sview.ru");
    StGLMessageBox* anAboutDialog = new StGLMessageBox(this, aTitle + '\n'
        + aVerString + ": " + StVersionInfo::getSDKVersionString() + "\n \n" + aDescr,
        512, 300);
    anAboutDialog->addButton("Close");
    anAboutDialog->setVisibility(true, true);
    anAboutDialog->stglInit();
}

void StCADViewerGUI::doOpenLicense(const size_t ) {
    StSocket::openURL(StProcess::getStShareFolder() + "info" + SYS_FS_SPLITTER + "license.txt");
}

void StCADViewerGUI::doShowFPS(const bool ) {
    if(myFpsWidget != NULL) {
        delete myFpsWidget;
        myFpsWidget = NULL;
        return;
    }

    myFpsWidget = new StGLFpsLabel(this);
    myFpsWidget->setVisibility(true, true);
    myFpsWidget->stglInit();
}
