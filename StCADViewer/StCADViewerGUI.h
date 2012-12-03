/**
 * This source is a part of sView program.
 *
 * Copyright © Kirill Gavrilov, 2011
 */

#ifndef __StCADViewerGUI_h_
#define __StCADViewerGUI_h_

#include <StGLWidgets/StGLWidget.h>
#include <StGLWidgets/StGLRootWidget.h>
#include <StSettings/StTranslations.h>

class StCADViewer;
class StGLDescription;
class StGLMenu;
class StGLMenuItem;
class StGLTextureButton;
class StGLMsgStack;

class ST_LOCAL StCADViewerGUI : public StGLRootWidget {

        public:

    StCADViewer*              myPlugin; //!< link to the main class
    StHandle<StTranslations> myLangMap; //!< translated strings map
    StString            myTexturesRoot; //!< textures path root

    StGLDescription*      myMouseDescr; //!< description shown near mouse cursor
    StGLMsgStack*           myMsgStack; //!< messages stack
    StGLMenu*              myMenu0Root; //!< main menu

    bool                myIsGUIVisible;

        private: //!< menus creation routines

    void      createMainMenu();         //!< Root (Main menu)
    StGLMenu* createViewMenu();         //!< Root -> View menu
    StGLMenu* createProjMenu();         //!< Root -> View menu -> Projection
    StGLMenu* createFillMenu();         //!< Root -> View menu -> Fill Mode
    StGLMenu* createHelpMenu();         //!< Root -> Help menu
    StGLMenu* createLanguageMenu();     //!< Root -> Help -> Language menu

        public: //!< StGLWidget overrides

    StCADViewerGUI(StCADViewer* thePlugin);
    virtual ~StCADViewerGUI();
    virtual void stglUpdate(const StPointD_t& theCursorZo);
    virtual void stglResize(const StRectI_t& winRectPx);
    virtual void setVisibility(const StPointD_t& theCursorZo, bool );

        public: //!< callback Slots

    void doAboutProgram(const size_t );
    void doOpenLicense(const size_t );

};

#endif //__StCADViewerGUI_h_
