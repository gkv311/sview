/**
 * Copyright © 2009-2020 Kirill Gavrilov <kirill@sview.ru>
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

#include <StGLWidgets/StGLMessageBox.h>
#include <StGLWidgets/StGLRootWidget.h>
#include <StGLStereo/StGLTextureQueue.h>
#include <StGLStereo/StFormatEnum.h>
#include <StSettings/StTranslations.h>
#include <StThreads/StCondition.h>

// forward declarations
class StGLCheckboxTextured;
class StGLDescription;
class StGLTextureButton;
class StImageViewer;
class StGLMenu;
class StGLMenuItem;
class StGLImageRegion;
class StGLMsgStack;
class StGLFpsLabel;
class StGLTable;
class StGLPlayList;
class StGLRangeFieldFloat32;
class StPlayList;
class StWindow;

/**
 * Customized message box.
 */
class ST_LOCAL StInfoDialog : public StGLMessageBox {

        public:

    ST_LOCAL StInfoDialog(StImageViewer*  thePlugin,
                          StGLWidget*     theParent,
                          const StString& theTitle,
                          const int       theWidth,
                          const int       theHeight)
    : StGLMessageBox(theParent, theTitle, "", theWidth, theHeight),
      myPlugin(thePlugin) {}

    ST_LOCAL virtual ~StInfoDialog();

        private:

    StImageViewer* myPlugin;

};

/**
 * Root GUI widget for Image Viewer plugin.
 */
class StImageViewerGUI : public StGLRootWidget {

        public:

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

        public: //! @name StGLRootWidget overrides

    ST_LOCAL StImageViewerGUI(StImageViewer*  thePlugin,
                              StWindow*       theWindow,
                              StTranslations* theLangMap,
                              const StHandle<StPlayList>&       thePlayList,
                              const StHandle<StGLTextureQueue>& theTextureQueue);
    ST_LOCAL virtual ~StImageViewerGUI();
    ST_LOCAL virtual void stglUpdate(const StPointD_t& thePointZo,
                                     bool theIsPreciseInput) ST_ATTR_OVERRIDE;
    ST_LOCAL virtual void stglResize(const StGLBoxPx& theViewPort,
                                     const StMarginsI& theMargins,
                                     float theAspect) ST_ATTR_OVERRIDE;
    ST_LOCAL virtual void stglDraw(unsigned int theView) ST_ATTR_OVERRIDE;

    /**
     * Handle gesture.
     */
    ST_LOCAL void doGesture(const StGestureEvent& theEvent);
    ST_LOCAL bool isVisibleGUI() const { return myVisLerp.getValue() > 0.0; }
    ST_LOCAL void setVisibility(const StPointD_t& theCursor,
                                bool theToForceHide,
                                bool theToForceShow = false);

        public:

    ST_LOCAL bool toHideCursor() const;
    ST_LOCAL void showUpdatesNotify();

    ST_LOCAL void doAboutImage(const size_t );

    ST_LOCAL static size_t trSrcFormatId(const StFormat theSrcFormat);

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

    ST_LOCAL const StString& trSrcFormat(const StFormat theSrcFormat) const {
        return tr(trSrcFormatId(theSrcFormat));
    }

        private: //! @name desktop interface creation routines

    /**
     * Create normal (desktop) interface.
     */
    ST_LOCAL void createDesktopUI(const StHandle<StPlayList>& thePlayList);

    /**
     * Create upper tool-bar.
     */
    ST_LOCAL void createUpperToolbar();

    ST_LOCAL void      createMainMenu();         // Root (Main menu)
    ST_LOCAL StGLMenu* createMediaMenu();        // Root -> Media menu
    ST_LOCAL StGLMenu* createOpenImageMenu();    // Root -> Media -> Open image menu
    ST_LOCAL StGLMenu* createSaveImageMenu();    // Root -> Media -> Save image menu
    ST_LOCAL StGLMenu* createSrcFormatMenu();    // Root -> Media -> Source format menu
    ST_LOCAL StGLMenu* createViewMenu();         // Root -> View menu
    ST_LOCAL StGLMenu* createDisplayModeMenu();  // Root -> View menu -> Output
    ST_LOCAL StGLMenu* createDisplayRatioMenu(); // Root -> View menu -> Display Ratio
    ST_LOCAL StGLMenu* createSmoothFilterMenu(); // Root -> View menu -> Smooth Filter
    ST_LOCAL StGLMenu* createImageAdjustMenu();  // Root -> View menu -> Image Adjust
    ST_LOCAL StGLMenu* create3dAdjustMenu();     // Root -> View menu -> Stereo 3D Adjust
    ST_LOCAL StGLMenu* createOutputMenu();       // Root -> Output menu
    ST_LOCAL StGLMenu* createHelpMenu();         // Root -> Help menu
    ST_LOCAL StGLMenu* createScaleMenu();        // Root -> Scale Interface menu
    ST_LOCAL StGLMenu* createLanguageMenu();     // Root -> Help -> Language menu

    ST_LOCAL void fillSrcFormatMenu(StGLMenu* theMenu);
    ST_LOCAL void fillPanoramaMenu (StGLMenu* theMenu);

        private: //! @name mobile interface creation routines

    /**
     * Create mobile interface.
     */
    ST_LOCAL void      createMobileUI(const StHandle<StPlayList>& thePlayList);

    ST_LOCAL void      createMobileUpperToolbar();
    ST_LOCAL void      createMobileBottomToolbar();
    ST_LOCAL void      createImageAdjustments();

        private: //! @name callback Slots

    ST_LOCAL void doAction(const size_t theActionId,
                           const double theDuration);

    ST_LOCAL void doAboutProgram(const size_t );
    ST_LOCAL void doUserTips    (const size_t );
    ST_LOCAL void doCheckUpdates(const size_t );
    ST_LOCAL void doOpenLicense (const size_t );
    ST_LOCAL void doShowFPS(const bool );
    ST_LOCAL void doAboutRenderer(const size_t );

    ST_LOCAL void doChangeHotKey1(const size_t );
    ST_LOCAL void doChangeHotKey2(const size_t );
    ST_LOCAL void doResetHotKeys(const size_t );
    ST_LOCAL void doListHotKeys(const size_t );

    ST_LOCAL void doOpenFile(const size_t );
    ST_LOCAL void doShowMobileExMenu(const size_t );
    ST_LOCAL void doMobileSettings(const size_t );
    ST_LOCAL void doDisplayStereoFormatCombo(const size_t );
    ST_LOCAL void doPanoramaCombo(const size_t );

        private: //! @name private fields

    StImageViewer*      myPlugin;           //!< link to the main Drawer class
    StWindow*           myWindow;           //!< link to the window instance
    StTranslations*     myLangMap;          //!< translated strings map
    StTimer             myVisibilityTimer;  //!< minimum visible delay
    StTimer             myEmptyTimer;       //!< empty list delay
    StTimer             myTapTimer;         //!< single tap delay
    StGLAnimationLerp   myVisLerp;

    StGLImageRegion*    myImage;            //!< the main image
    StGLDescription*    myDescr;            //!< description text shown near mouse cursor
    StGLMsgStack*       myMsgStack;         //!< messages stack
    StGLPlayList*       myPlayList;

    StGLMenu*           myMenuRoot;         //!< main menu

    StGLWidget*         myPanelUpper;       //!< upper  toolbar
    StGLWidget*         myPanelBottom;      //!< bottom toolbar

    StGLWidget*         myAdjustOverlay;    //!< image adjustments control
    StGLRangeFieldFloat32* myBtnSepDx;
    StGLRangeFieldFloat32* myBtnSepDy;
    StGLRangeFieldFloat32* myBtnSepRot;
    StGLTextureButton*     myBtnReset3d;
    StGLTextureButton*     myBtnResetColor1;
    StGLTextureButton*     myBtnResetColor2;

    StGLTextureButton*  myBtnOpen;
    StGLTextureButton*  myBtnPrev;
    StGLTextureButton*  myBtnNext;
    StGLTextureButton*  myBtnInfo;
    StGLTextureButton*  myBtnAdjust;
    StGLTextureButton*  myBtnSwapLR;
    StGLCheckboxTextured* myBtnPanorama;
    StGLTextureButton*  myBtnSrcFrmt;
    StGLTextureButton*  myBtnList;
    StGLTextureButton*  myBtnFull;
    StGLFpsLabel*       myFpsWidget;

    StGLTable*          myHKeysTable;

    bool                myIsVisibleGUI;
    bool                myIsMinimalGUI;

        private:

    friend class StImageViewer;
    friend class StHotKeyControl;

};

#endif // __StImageViewerGUI_h_
