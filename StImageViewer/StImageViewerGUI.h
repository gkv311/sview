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

#ifndef __StImageViewerGUI_h_
#define __StImageViewerGUI_h_

#include <StGLWidgets/StGLWidget.h>
#include <StGLWidgets/StGLRootWidget.h>
#include <StGLStereo/StGLTextureQueue.h>
#include <StGLStereo/StFormatEnum.h>
#include <StSettings/StTranslations.h>
#include <StThreads/StCondition.h>

// forward declarations
class StGLDescription;
class StGLTextureButton;
class StImageViewer;
class StGLMenu;
class StGLMenuItem;
class StGLImageRegion;
class StGLMsgStack;
class StGLFpsLabel;
class StWindow;

/**
 * Root GUI widget for Image Viewer plugin.
 */
class StImageViewerGUI : public StGLRootWidget {

        public:

    enum {
        CLICKED_NONE = 0,
        CLICKED_ICON_OPEN_FILE,
        CLICKED_ICON_PREV_FILE,
        CLICKED_ICON_NEXT_FILE,
        CLICKED_ICON_SWAP_LR,
        CLICKED_ICON_SRC2MONO,
        CLICKED_ICON_SRC2SIDE_BY_SIDE,
        CLICKED_ICON_SRC2SOVER_UNDER,
        CLICKED_ICON_SRC2HOR_INTERLACE,
        CLICKED_ICON_SRC2AUTODETECT,
    };

    StImageViewer*           myPlugin;  //!< link to the main Drawer class
    StWindow*                myWindow;  //!< link to the window instance
    StTranslations*          myLangMap; //!< translated strings map
    StString      texturesPathRoot; // textures directory
    StTimer      stTimeVisibleLock; // minimum visible delay

    StGLImageRegion* stImageRegion; // the main image
    StGLDescription*   stTextDescr; // description text shown near mouse cursor
    StGLMsgStack*       myMsgStack; // messages stack

    StGLMenu*            menu0Root; // main menu

    StGLWidget*        upperRegion; // upper toolbar
    StGLTextureButton*     btnOpen;
    StGLTextureButton*     btnPrev;
    StGLTextureButton*     btnNext;
    StGLTextureButton*   btnSwapLR;
    StGLWidget*       myBtnSrcFrmt;
    StGLTextureButton*   myBtnFull;
    StGLFpsLabel*      myFpsWidget;

    bool isGUIVisible;
    bool isGUIMinimal;

        private:

    ST_LOCAL void createUpperToolbar();

        private: //!< menus creation routines

    ST_LOCAL void      createMainMenu();         // Root (Main menu)
    ST_LOCAL StGLMenu* createMediaMenu();        // Root -> Media menu
    ST_LOCAL StGLMenu* createOpenImageMenu();    // Root -> Media -> Open image menu
    ST_LOCAL StGLMenu* createSaveImageMenu();    // Root -> Media -> Save image menu
    ST_LOCAL StGLMenu* createSrcFormatMenu();    // Root -> Media -> Source format menu
    ST_LOCAL StGLMenu* createViewMenu();         // Root -> View menu
    ST_LOCAL StGLMenu* createDisplayModeMenu();  // Root -> View menu -> Output
    ST_LOCAL StGLMenu* createDisplayRatioMenu(); // Root -> View menu -> Display Ratio
    ST_LOCAL StGLMenu* createSmoothFilterMenu(); // Root -> View menu -> Smooth Filter
    ST_LOCAL StGLMenu* createGammaMenu();        // Root -> View menu -> Gamma Correction
    ST_LOCAL StGLMenu* createOutputMenu();       // Root -> Output menu
    ST_LOCAL StGLMenu* createHelpMenu();         // Root -> Help menu
    ST_LOCAL StGLMenu* createCheckUpdatesMenu(); // Root -> Help -> Check updates menu
    ST_LOCAL StGLMenu* createLanguageMenu();     // Root -> Help -> Language menu

        public: //!< StGLRootWidget overrides

    ST_LOCAL StImageViewerGUI(StImageViewer*  thePlugin,
                              StWindow*       theWindow,
                              StTranslations* theLangMap,
                              const StHandle<StGLTextureQueue>& theTextureQueue);
    ST_LOCAL virtual ~StImageViewerGUI();
    ST_LOCAL virtual void stglUpdate(const StPointD_t& pointZo);
    ST_LOCAL virtual void stglResize(const StRectI_t& winRectPx);
    ST_LOCAL virtual void stglDraw(unsigned int theView);
    ST_LOCAL virtual void setVisibility(const StPointD_t& pointZo, bool isMouseActive = false);

        public:

    ST_LOCAL bool toHideCursor() const;
    ST_LOCAL void showUpdatesNotify();

    ST_LOCAL void doAboutImage(const size_t );

        private: //!< callback Slots

    ST_LOCAL void doAboutProgram(const size_t );
    ST_LOCAL void doUserTips    (const size_t );
    ST_LOCAL void doAboutSystem (const size_t );
    ST_LOCAL void doCheckUpdates(const size_t );
    ST_LOCAL void doOpenLicense (const size_t );
    ST_LOCAL void doShowFPS(const bool );
    ST_LOCAL void doAboutRenderer(const size_t );

};

#endif //__StImageViewerGUI_h_
