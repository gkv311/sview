/**
 * Copyright © 2009-2014 Kirill Gavrilov <kirill@sview.ru>
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

    /**
     * @return absolute path to the texture
     */
    ST_LOCAL StString getTexturePath(const StCString& theTextureName) const {
        return myTexturesFolder + theTextureName;
    }

    /**
     * @return absolute path to the texture
     */
    ST_LOCAL StString iconTexture(const StCString& theName,
                                  const IconSize   theSize) const {
        return StGLRootWidget::iconTexture(myTexturesFolder + theName, theSize);
    }

        public: //! @name StGLRootWidget overrides

    ST_LOCAL StImageViewerGUI(StImageViewer*  thePlugin,
                              StWindow*       theWindow,
                              StTranslations* theLangMap,
                              const StHandle<StGLTextureQueue>& theTextureQueue);
    ST_LOCAL virtual ~StImageViewerGUI();
    ST_LOCAL virtual void stglUpdate(const StPointD_t& pointZo);
    ST_LOCAL virtual void stglResize(const StGLBoxPx& theRectPx);
    ST_LOCAL virtual void stglDraw(unsigned int theView);
    ST_LOCAL virtual void setVisibility(const StPointD_t& theCursor,
                                        bool              isMouseActive);

        public:

    ST_LOCAL bool toHideCursor() const;
    ST_LOCAL void showUpdatesNotify();

    ST_LOCAL void doAboutImage(const size_t );

    ST_LOCAL static size_t trSrcFormatId(const StFormatEnum theSrcFormat);

        private:

    ST_LOCAL void createUpperToolbar();

    ST_LOCAL const StString& tr(const size_t theId) const {
        return myLangMap->getValue(theId);
    }

    ST_LOCAL const StString& trSrcFormat(const StFormatEnum theSrcFormat) const {
        return tr(trSrcFormatId(theSrcFormat));
    }

        private: //! @name menus creation routines

    ST_LOCAL void      createMainMenu();         // Root (Main menu)
    ST_LOCAL StGLMenu* createMediaMenu();        // Root -> Media menu
    ST_LOCAL StGLMenu* createOpenImageMenu();    // Root -> Media -> Open image menu
    ST_LOCAL StGLMenu* createSaveImageMenu();    // Root -> Media -> Save image menu
    ST_LOCAL StGLMenu* createSrcFormatMenu();    // Root -> Media -> Source format menu
    ST_LOCAL StGLMenu* createViewMenu();         // Root -> View menu
    ST_LOCAL StGLMenu* createDisplayModeMenu();  // Root -> View menu -> Output
    ST_LOCAL StGLMenu* createDisplayRatioMenu(); // Root -> View menu -> Display Ratio
    ST_LOCAL StGLMenu* createSurfaceMenu();      // Root -> View   -> Surface
    ST_LOCAL StGLMenu* createSmoothFilterMenu(); // Root -> View menu -> Smooth Filter
    ST_LOCAL StGLMenu* createImageAdjustMenu();  // Root -> View menu -> Image Adjust
    ST_LOCAL StGLMenu* createOutputMenu();       // Root -> Output menu
    ST_LOCAL StGLMenu* createHelpMenu();         // Root -> Help menu
    ST_LOCAL StGLMenu* createScaleMenu();        // Root -> Scale Interface menu
    ST_LOCAL StGLMenu* createCheckUpdatesMenu(); // Root -> Help -> Check updates menu
    ST_LOCAL StGLMenu* createLanguageMenu();     // Root -> Help -> Language menu

        private: //! @name callback Slots

    ST_LOCAL void doAboutProgram(const size_t );
    ST_LOCAL void doUserTips    (const size_t );
    ST_LOCAL void doAboutSystem (const size_t );
    ST_LOCAL void doCheckUpdates(const size_t );
    ST_LOCAL void doOpenLicense (const size_t );
    ST_LOCAL void doShowFPS(const bool );
    ST_LOCAL void doAboutRenderer(const size_t );

        private: //! @name private fields

    StImageViewer*      myPlugin;           //!< link to the main Drawer class
    StWindow*           myWindow;           //!< link to the window instance
    StTranslations*     myLangMap;          //!< translated strings map
    StString            myTexturesFolder;   //!< textures directory
    StTimer             myVisibilityTimer;  //!< minimum visible delay

    StGLImageRegion*    myImage;            //!< the main image
    StGLDescription*    myDescr;            //!< description text shown near mouse cursor
    StGLMsgStack*       myMsgStack;         //!< messages stack

    StGLMenu*           myMenuRoot;         //!< main menu

    StGLWidget*         myPanelUpper;       //!< upper toolbar
    StGLTextureButton*  myBtnOpen;
    StGLTextureButton*  myBtnPrev;
    StGLTextureButton*  myBtnNext;
    StGLTextureButton*  myBtnSwapLR;
    StGLWidget*         myBtnSrcFrmt;
    StGLTextureButton*  myBtnPlayList;
    StGLTextureButton*  myBtnFull;
    StGLFpsLabel*       myFpsWidget;

    bool                myIsVisibleGUI;
    bool                myIsMinimalGUI;

        private:

    friend class StImageViewer;

};

#endif // __StImageViewerGUI_h_
