/**
 * This source is a part of sView program.
 *
 * Copyright © Kirill Gavrilov, 2011-2016
 */

#ifndef __StCADViewerGUI_h_
#define __StCADViewerGUI_h_

#include <StGLWidgets/StGLMessageBox.h>
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
 * Customized message box.
 */
class ST_LOCAL StInfoDialog : public StGLMessageBox {

        public:

    ST_LOCAL StInfoDialog(StCADViewer*    thePlugin,
                          StGLWidget*     theParent,
                          const StString& theTitle,
                          const int       theWidth,
                          const int       theHeight)
    : StGLMessageBox(theParent, theTitle, "", theWidth, theHeight), myPlugin(thePlugin) {}
    ST_LOCAL virtual ~StInfoDialog();

        private:

    StCADViewer* myPlugin;

};

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
     * @return relative path to the texture
     */
    ST_LOCAL StString getTexturePath(const StCString& theTextureName) const {
        return StString("textures" ST_FILE_SPLITTER) + theTextureName;
    }

    /**
     * @return relative path to the texture
     */
    ST_LOCAL StString iconTexture(const StCString& theName,
                                  const IconSize   theSize) const {
        return StGLRootWidget::iconTexture(StString("textures" ST_FILE_SPLITTER) + theName, theSize);
    }

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
    ST_LOCAL void doAction(const size_t theActionId,
                           const double theDuration);
    ST_LOCAL void doShowFPS(const bool );

    /**
     * Show settings dialog.
     */
    ST_LOCAL void doMobileSettings(const size_t );

    /**
     * Show context menu.
     */
    ST_LOCAL void doShowMobileExMenu(const size_t );

        private: //!< menus creation routines

    ST_LOCAL void      createMainMenu();         //!< Root (Main menu)
    ST_LOCAL StGLMenu* createViewMenu();         //!< Root -> View menu
    ST_LOCAL StGLMenu* createProjMenu();         //!< Root -> View menu -> Projection
    ST_LOCAL StGLMenu* createHelpMenu();         //!< Root -> Help menu
    ST_LOCAL StGLMenu* createLanguageMenu();     //!< Root -> Help -> Language menu

    /**
     * Define content of the top toolbar.
     */
    ST_LOCAL void createToolbarOnTop();

    /**
     * Define content of the bottom toolbar.
     */
    ST_LOCAL void createToolbarOnBottom();

        private:

    StCADViewer*      myPlugin;       //!< link to the main class
    StTranslations*   myLangMap;      //!< translated strings map

    StGLDescription*  myMouseDescr;   //!< description shown near mouse cursor
    StGLMsgStack*     myMsgStack;     //!< messages stack
    StGLMenu*         myMenu0Root;    //!< main menu
    StGLWidget*       myPanelUpper;   //!< upper  toolbar
    StGLWidget*       myPanelBottom;  //!< bottom toolbar
    StGLFpsLabel*     myFpsWidget;    //!< FPS meter

    bool              myIsGUIVisible;

};

#endif //__StCADViewerGUI_h_
