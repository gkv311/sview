/**
 * Copyright © 2009-2012 Kirill Gavrilov <kirill@sview.ru>
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
#include <StGLStereo/StFormatEnum.h>
#include <StSettings/StTranslations.h>
#include <StThreads/StEvent.h>

// forward declarations
class StGLDescription;
class StGLTextureButton;
class StImageViewer;
class StGLMenu;
class StGLMenuItem;
class StGLImageRegion;
class StGLMsgStack;
class StWindow;

/**
 * Root GUI widget for Image Viewer plugin.
 */
class ST_LOCAL StImageViewerGUI : public StGLRootWidget {

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

    StImageViewer*           myPlugin; //!< link to the main Drawer class
    StWindow*                myWindow; //!< link to the window instance
    StHandle<StTranslations> myLangMap; //!< translated strings map
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

    bool isGUIVisible;
    bool isGUIMinimal;

        private:

    void createUpperToolbar();

        private: //!< menus creation routines

    void      createMainMenu();         // Root (Main menu)
    StGLMenu* createMediaMenu();        // Root -> Media menu
    StGLMenu* createOpenImageMenu();    // Root -> Media -> Open image menu
    StGLMenu* createSaveImageMenu();    // Root -> Media -> Save image menu
    StGLMenu* createSrcFormatMenu();    // Root -> Media -> Source format menu
    StGLMenu* createViewMenu();         // Root -> View menu
    StGLMenu* createDisplayModeMenu();  // Root -> View menu -> Output
    StGLMenu* createDisplayRatioMenu(); // Root -> View menu -> Display Ratio
    StGLMenu* createSmoothFilterMenu(); // Root -> View menu -> Smooth Filter
    StGLMenu* createGammaMenu();        // Root -> View menu -> Gamma Correction
    StGLMenu* createHelpMenu();         // Root -> Help menu
    StGLMenu* createCheckUpdatesMenu(); // Root -> Help -> Check updates menu
    StGLMenu* createLanguageMenu();     // Root -> Help -> Language menu

        public: //!< StGLRootWidget overrides

    StImageViewerGUI(StImageViewer* thePlugin,
                     StWindow*      theWindow);
    virtual ~StImageViewerGUI();
    virtual void stglUpdate(const StPointD_t& pointZo);
    virtual void stglResize(const StRectI_t& winRectPx);
    virtual void setVisibility(const StPointD_t& pointZo, bool isMouseActive = false);

        public:

    bool toHideCursor() const;
    void showUpdatesNotify();

        private: //!< callback Slots

    void doAboutProgram(const size_t );
    void doAboutSystem (const size_t );
    void doCheckUpdates(const size_t );
    void doOpenLicense (const size_t );

};

#endif //__StImageViewerGUI_h_
