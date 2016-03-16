/**
 * This source is a part of sView program.
 *
 * Copyright © Kirill Gavrilov, 2011-2016
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

/**
 * Root GUI widget for Tiny CAD Viewer application.
 */
class StCADViewerGUI : public StGLRootWidget {

        public: //!< StGLWidget overrides

    ST_LOCAL StCADViewerGUI(StCADViewer*    thePlugin,
                            StTranslations* theLangMap);
    ST_LOCAL virtual ~StCADViewerGUI();
    ST_LOCAL virtual void stglUpdate(const StPointD_t& theCursorZo) ST_ATTR_OVERRIDE;
    ST_LOCAL virtual void stglResize(const StGLBoxPx&  theRectPx) ST_ATTR_OVERRIDE;
    ST_LOCAL virtual void stglDraw(unsigned int theView) ST_ATTR_OVERRIDE;

    ST_LOCAL void setVisibility(const StPointD_t& theCursorZo, bool );

    /**
     * @return translation for the string with specified id
     */
    ST_LOCAL const StString& tr(const size_t theId) const {
        return myLangMap->getValue(theId);
    }

    /**
     * @return translation for the string with specified id
     */
    ST_LOCAL const StString& tr(const StString& theId) const {
        return myLangMap->getValue(theId);
    }

        public: //!< callback Slots

    ST_LOCAL void doAboutProgram(const size_t );
    ST_LOCAL void doOpenLicense(const size_t );
    ST_LOCAL void doShowFPS(const bool );

        private: //!< menus creation routines

    ST_LOCAL void      createMainMenu();         //!< Root (Main menu)
    ST_LOCAL StGLMenu* createViewMenu();         //!< Root -> View menu
    ST_LOCAL StGLMenu* createProjMenu();         //!< Root -> View menu -> Projection
    ST_LOCAL StGLMenu* createHelpMenu();         //!< Root -> Help menu
    ST_LOCAL StGLMenu* createLanguageMenu();     //!< Root -> Help -> Language menu

        private:

    StCADViewer*      myPlugin;       //!< link to the main class
    StTranslations*   myLangMap;      //!< translated strings map

    StGLDescription*  myMouseDescr;   //!< description shown near mouse cursor
    StGLMsgStack*     myMsgStack;     //!< messages stack
    StGLMenu*         myMenu0Root;    //!< main menu
    StGLFpsLabel*     myFpsWidget;    //!< FPS meter

    bool              myIsGUIVisible;

};

#endif //__StCADViewerGUI_h_
