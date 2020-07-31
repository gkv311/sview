/**
 * Copyright © 2009-2020 Kirill Gavrilov <kirill@sview.ru>
 *
 * StMoviePlayer program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * StMoviePlayer program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program.
 * If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __StMoviePlayerGUI_h_
#define __StMoviePlayerGUI_h_

#include <StGLWidgets/StGLWidget.h>
#include <StGLWidgets/StGLRootWidget.h>
#include <StGLWidgets/StSubQueue.h>
#include <StGLStereo/StFormatEnum.h>
#include <StGLStereo/StGLTextureQueue.h>
#include <StSettings/StTranslations.h>

// forward declarations
class StMoviePlayer;
class StGLFpsLabel;
class StGLDescription;
class StGLImageRegion;
class StGLMenu;
class StGLMenuItem;
class StGLMsgStack;
class StGLRangeFieldFloat32;
class StGLSubtitles;
class StGLPlayList;
class StGLTable;
class StGLTextArea;
class StGLTextureButton;
class StGLCheckboxTextured;
class StPlayList;
class StGLSeekBar;
class StTimeBox;
class StUtfLangMap;
class StWindow;

/**
 * Root GUI widget for Video Playback plugin.
 */
class StMoviePlayerGUI : public StGLRootWidget {

        public:

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

    ST_LOCAL static size_t trSrcFormatId(const StFormat theSrcFormat);

    /**
     * @return relative path to the texture
     */
    ST_LOCAL inline StString getTexturePath(const StCString& theTextureName) const {
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
     * Convert dB to amplitude ratio
     */
    ST_LOCAL static inline double dBellToRatio(const double thedB) {
        return std::pow(10.0, thedB * 0.05);
    }



        public: //! @name StGLRootWidget overrides

    ST_LOCAL StMoviePlayerGUI(StMoviePlayer*  thePlugin,
                              StWindow*       theWindow,
                              StTranslations* theLangMap,
                              const StHandle<StPlayList>&       thePlayList,
                              const StHandle<StGLTextureQueue>& theTextureQueue,
                              const StHandle<StSubQueue>&       theSubQueue);
    ST_LOCAL virtual ~StMoviePlayerGUI();

    ST_LOCAL virtual void stglResize(const StGLBoxPx& theViewPort,
                                     const StMarginsI& theMargins,
                                     float theAspect) ST_ATTR_OVERRIDE;
    ST_LOCAL virtual void stglDraw(unsigned int theView) ST_ATTR_OVERRIDE;
    ST_LOCAL virtual void stglUpdate(const StPointD_t& thePointZo,
                                     bool theIsPreciseInput) ST_ATTR_OVERRIDE;

    /**
     * Handle gesture.
     */
    ST_LOCAL void doGesture(const StGestureEvent& theEvent);
    ST_LOCAL bool isVisibleGUI() const { return myVisLerp.getValue() > 0.0; }
    ST_LOCAL void setVisibility(const StPointD_t& theCursor,
                                bool theToForceHide = false,
                                bool theToForceShow = false);

        public:

    ST_LOCAL bool toHideCursor();
    ST_LOCAL void showUpdatesNotify();

        public: //! @name menu update routines

    ST_LOCAL void stglResizeSeekBar();
    ST_LOCAL void updateOpenALDeviceMenu();
    ST_LOCAL void updateRecentMenu();

    ST_LOCAL void doAboutFile(const size_t );

        private: //! @name desktop interface creation routines

    /**
     * Create normal (desktop) interface.
     */
    ST_LOCAL void createDesktopUI(const StHandle<StPlayList>& thePlayList);
    ST_LOCAL void createUpperToolbar();
    ST_LOCAL void createBottomToolbar(const int theIconSize,
                                      const int theIconSizeSmall);

    ST_LOCAL void      createMainMenu();         // Root (Main menu)
    ST_LOCAL StGLMenu* createMediaMenu();        // Root -> Media menu
    ST_LOCAL StGLMenu* createOpenMovieMenu();    // Root -> Media  -> Open movie menu
    ST_LOCAL StGLMenu* createSaveImageMenu();    // Root -> Media  -> Save snapshot menu
    ST_LOCAL StGLMenu* createSrcFormatMenu();    // Root -> Media  -> Source format menu
    ST_LOCAL StGLMenu* createOpenALDeviceMenu(); // Root -> Media  -> OpenAL Device menu
    ST_LOCAL StGLMenu* createRecentMenu();       // Root -> Media  -> Recent files menu
    ST_LOCAL StGLMenu* createWebUIMenu();        // Root -> Media  -> Web UI menu
    ST_LOCAL StGLMenu* createViewMenu();         // Root -> View menu
    ST_LOCAL StGLMenu* createDisplayModeMenu();  // Root -> View   -> Output
    ST_LOCAL StGLMenu* createDisplayRatioMenu(); // Root -> View   -> Display Ratio
    ST_LOCAL StGLMenu* createSmoothFilterMenu(); // Root -> View   -> Smooth Filter
    ST_LOCAL StGLMenu* createImageAdjustMenu();  // Root -> View   -> Image Adjust
    ST_LOCAL StGLMenu* create3dAdjustMenu();     // Root -> View   -> Stereo 3D Adjust
    ST_LOCAL StGLMenu* createAudioMenu();        // Root -> Audio menu
    ST_LOCAL StGLMenu* createSubtitlesMenu();    // Root -> Subtitles menu
    ST_LOCAL StGLMenu* createOutputMenu();       // Root -> Output menu
    ST_LOCAL StGLMenu* createFpsMenu();          // Root -> Output -> FPS Control
    ST_LOCAL StGLMenu* createHelpMenu();         // Root -> Help menu
    ST_LOCAL StGLMenu* createScaleMenu();        // Root -> Scale Interface menu
    ST_LOCAL StGLMenu* createLanguageMenu();     // Root -> Help   -> Language menu

    ST_LOCAL void fillOpenALDeviceMenu(StGLMenu* theMenu);
    ST_LOCAL void fillRecentMenu(StGLMenu* theMenu);
    ST_LOCAL void fillDisplayRatioMenu(StGLMenu* theMenu);
    ST_LOCAL void fillSrcFormatMenu(StGLMenu* theMenu);
    ST_LOCAL void fillPanoramaMenu (StGLMenu* theMenu);
    ST_LOCAL void fillSubtitlesFontSize(StGLMenu* theMenu);
    ST_LOCAL void fillSubtitlesPlacement(StGLMenu* theMenu);

        private: //! @name mobile interface creation routines

    /**
     * Create mobile interface.
     */
    ST_LOCAL void createMobileUI(const StHandle<StPlayList>& thePlayList);

    ST_LOCAL void createMobileUpperToolbar();
    ST_LOCAL void createMobileBottomToolbar();
    ST_LOCAL void createImageAdjustments();

        private: //! @name callback Slots

    ST_LOCAL void doAboutProgram (const size_t );
    ST_LOCAL void doUserTips     (const size_t );
    ST_LOCAL void doCheckUpdates (const size_t );
    ST_LOCAL void doOpenLicense  (const size_t );
    ST_LOCAL void doShowFPS(const bool );
    ST_LOCAL void doAboutRenderer(const size_t );
    ST_LOCAL void doAudioGain    (const int theMouseBtn, const double theVolume);
    ST_LOCAL void doAudioGainScroll(const double theDelta);
    ST_LOCAL void doAudioDelay   (const size_t );

    ST_LOCAL void doListHotKeys(const size_t );
    ST_LOCAL void doResetHotKeys(const size_t );
    ST_LOCAL void doChangeHotKey1(const size_t );
    ST_LOCAL void doChangeHotKey2(const size_t );

    ST_LOCAL void doOpenFile(const size_t theFileType);
    ST_LOCAL void doShowMobileExMenu(const size_t );
    ST_LOCAL void doMobileSettings(const size_t );
    ST_LOCAL void doAudioStreamsCombo(const size_t );
    ST_LOCAL void doSubtitlesStreamsCombo(const size_t );
    ST_LOCAL void doSubtitlesPlacement(const size_t );
    ST_LOCAL void doDisplayRatioCombo(const size_t );
    ST_LOCAL void doDisplayStereoFormatCombo(const size_t );
    ST_LOCAL void doPanoramaCombo(const size_t );

        private: //! @name private fields

    StMoviePlayer*      myPlugin;           //!< link to the main Drawer class
    StWindow*           myWindow;           //!< link to the window instance
    StTranslations*     myLangMap;          //!< translated strings map
    StTimer             myVisibilityTimer;  //!< minimum visible delay
    StTimer             myEmptyTimer;       //!< empty list delay
    StTimer             myTapTimer;         //!< single tap delay
    StGLAnimationLerp   myVisLerp;

    StGLImageRegion*    myImage;            //!< the main video frame
    StGLSubtitles*      mySubtitles;        //!< the subtitles
    StGLDescription*    myDescr;            //!< description text shown near mouse cursor
    StGLMsgStack*       myMsgStack;         //!< messages stack
    StGLPlayList*       myPlayList;

    StGLMenu*           myMenuRoot;         //!< root of the main menu
    StGLMenu*           myMenuOpenAL;
    StGLMenu*           myMenuRecent;

    StGLWidget*         myPanelUpper;       //!< upper toolbar
    StGLTextureButton*  myBtnOpen;
    StGLTextureButton*  myBtnInfo;
    StGLTextureButton*  myBtnAdjust;
    StGLTextureButton*  myBtnSwapLR;
    StGLCheckboxTextured* myBtnPanorama;
    StGLTextureButton*  myBtnSrcFrmt;

    StGLCheckboxTextured* myBtnAudio;
    StGLCheckboxTextured* myBtnSubs;

    StGLWidget*         myPanelBottom;      //!< bottom toolbar
    StGLSeekBar*        mySeekBar;
    StGLSeekBar*        myVolumeBar;
    StGLTextArea*       myVolumeLab;
    StGLTextureButton*  myBtnPlay;
    StTimeBox*          myTimeBox;
    StGLCheckboxTextured* myBtnVolume;
    StGLTextureButton*  myBtnPrev;
    StGLTextureButton*  myBtnNext;
    StGLTextureButton*  myBtnList;
    StGLCheckboxTextured* myBtnShuffle;
    StGLCheckboxTextured* myBtnLoop;
    StGLTextureButton*  myBtnFullScr;
    StGLFpsLabel*       myFpsWidget;

    StGLWidget*            myAdjustOverlay; //!< image adjustments control
    StGLRangeFieldFloat32* myBtnSepDx;
    StGLRangeFieldFloat32* myBtnSepDy;
    StGLRangeFieldFloat32* myBtnSepRot;
    StGLTextureButton*     myBtnReset3d;
    StGLTextureButton*     myBtnResetColor1;
    StGLTextureButton*     myBtnResetColor2;

    StGLTable*          myHKeysTable;

    bool                myIsVisibleGUI;
    bool                myIsExperimental;
    int                 myIconStep;         //!< step between icons, in pixels
    int                 myBottomBarNbLeft;  //!< number of icons at left  side of bottom bar, in pixels
    int                 myBottomBarNbRight; //!< number of icons at right side of bottom bar, in pixels

        private:

    friend class StMoviePlayer;

};

#endif // __StMoviePlayerGUI_h_
