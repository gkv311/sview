/**
 * This source is a part of sView program.
 *
 * Copyright © Kirill Gavrilov, 2011-2013
 */

#ifndef __StCADViewerGUI_h_
#define __StCADViewerGUI_h_

#include <StGLWidgets/StGLWidget.h>
#include <StGLWidgets/StGLRootWidget.h>
#include <StSettings/StTranslations.h>

class StCADViewer;
class StGLDescription;
class StGLFpsLabel;
class StGLMenu;
class StGLMenuItem;
class StGLTextureButton;
class StGLMsgStack;

class StCADViewerGUI : public StGLRootWidget {

        public:

    StCADViewer*              myPlugin; //!< link to the main class
    StHandle<StTranslations> myLangMap; //!< translated strings map

    StGLDescription*      myMouseDescr; //!< description shown near mouse cursor
    StGLMsgStack*           myMsgStack; //!< messages stack
    StGLMenu*              myMenu0Root; //!< main menu
    StGLFpsLabel*          myFpsWidget; //!< FPS meter

    bool                myIsGUIVisible;

        private: //!< menus creation routines

    ST_LOCAL void      createMainMenu();         //!< Root (Main menu)
    ST_LOCAL StGLMenu* createViewMenu();         //!< Root -> View menu
    ST_LOCAL StGLMenu* createProjMenu();         //!< Root -> View menu -> Projection
    ST_LOCAL StGLMenu* createFillMenu();         //!< Root -> View menu -> Fill Mode
    ST_LOCAL StGLMenu* createHelpMenu();         //!< Root -> Help menu
    ST_LOCAL StGLMenu* createLanguageMenu();     //!< Root -> Help -> Language menu

        public: //!< StGLWidget overrides

    ST_LOCAL StCADViewerGUI(StCADViewer* thePlugin);
    ST_LOCAL virtual ~StCADViewerGUI();
    ST_LOCAL virtual void stglUpdate(const StPointD_t& theCursorZo);
    ST_LOCAL virtual void stglResize(const StGLBoxPx&  theRectPx);
    ST_LOCAL virtual void stglDraw(unsigned int theView);

    using StGLRootWidget::setVisibility;
    ST_LOCAL void setVisibility(const StPointD_t& theCursorZo, bool );

        public: //!< callback Slots

    ST_LOCAL void doAboutProgram(const size_t );
    ST_LOCAL void doOpenLicense(const size_t );
    ST_LOCAL void doShowFPS(const bool );

};

#endif //__StCADViewerGUI_h_
