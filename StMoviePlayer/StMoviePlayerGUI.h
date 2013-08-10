/**
 * Copyright © 2009-2013 Kirill Gavrilov <kirill@sview.ru>
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
class StGLTextArea;
class StGLTextureButton;
class StPlayList;
class StSeekBar;
class StTimeBox;
class StUtfLangMap;
class StWindow;

/**
 * Root GUI widget for Video Playback plugin.
 */
class StMoviePlayerGUI : public StGLRootWidget {

        public:

    enum {
        CLICKED_NONE = 0,
        CLICKED_ICON_OPEN_FILE,
        CLICKED_ICON_SWAP_LR,
        CLICKED_ICON_SRC2MONO,
        CLICKED_ICON_SRC2SIDE_BY_SIDE,
        CLICKED_ICON_SRC2SOVER_UNDER,
        CLICKED_ICON_SRC2HOR_INTERLACE,
        CLICKED_ICON_SRC2AUTODETECT,
        CLICKED_SEEKBAR,
        CLICKED_ICON_PLAY,
        CLICKED_TIMEBOX,
        CLICKED_ICON_PREV_FILE,
        CLICKED_ICON_NEXT_FILE,
        CLICKED_ICON_PLAYLIST,
        CLICKED_ICON_FULLSCREEN,
    };

    /**
     * @return translation for the string with specified id
     */
    ST_LOCAL const StString& tr(const size_t theId) const {
        return myLangMap->getValue(theId);
    }

    /**
     * @return absolute path to the texture
     */
    ST_LOCAL StString getTexturePath(const StCString& theTextureName) const {
        return myTexturesFolder + theTextureName;
    }

        public: //! @name StGLRootWidget overrides

    ST_LOCAL StMoviePlayerGUI(StMoviePlayer*  thePlugin,
                              StWindow*       theWindow,
                              StTranslations* theLangMap,
                              const StHandle<StPlayList>&       thePlayList,
                              const StHandle<StGLTextureQueue>& theTextureQueue,
                              const StHandle<StSubQueue>&       theSubQueue);
    ST_LOCAL virtual ~StMoviePlayerGUI();
    ST_LOCAL virtual void stglUpdate(const StPointD_t& thePointZo,
                                     const GLfloat     theProgress,
                                     const double      thePTS);
    ST_LOCAL virtual void stglResize(const StRectI_t& winRectPx);
    ST_LOCAL virtual void stglDraw(unsigned int theView);
    ST_LOCAL virtual void setVisibility(const StPointD_t& theCursor,
                                        bool              theIsMouseMoved);

        public:

    ST_LOCAL bool toHideCursor();
    ST_LOCAL void showUpdatesNotify();

        public: //! @name menu update routines

    ST_LOCAL void updateOpenALDeviceMenu();
    ST_LOCAL void updateAudioStreamsMenu    (const StHandle< StArrayList<StString> >& theStreamsList,
                                             const bool theHasVideo);
    ST_LOCAL void updateSubtitlesStreamsMenu(const StHandle< StArrayList<StString> >& theStreamsList);

    ST_LOCAL void updateRecentMenu();

    ST_LOCAL void doAboutFile(const size_t );

        private:

    ST_LOCAL void createUpperToolbar();
    ST_LOCAL void createBottomToolbar();

        private: //! @name menus creation routines

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
    ST_LOCAL StGLMenu* createAudioMenu();        // Root -> Audio menu
    ST_LOCAL StGLMenu* createAudioGainMenu();    // Root -> Audio menu -> Volume
    ST_LOCAL StGLMenu* createSubtitlesMenu();    // Root -> Subtitles menu
    ST_LOCAL StGLMenu* createOutputMenu();       // Root -> Output menu
    ST_LOCAL StGLMenu* createFpsMenu();          // Root -> Output -> FPS Control
    ST_LOCAL StGLMenu* createHelpMenu();         // Root -> Help menu
    ST_LOCAL StGLMenu* createBlockSleepMenu();   // Root -> Help   -> Block sleeping
    ST_LOCAL StGLMenu* createCheckUpdatesMenu(); // Root -> Help   -> Check updates menu
    ST_LOCAL StGLMenu* createLanguageMenu();     // Root -> Help   -> Language menu

    ST_LOCAL void fillOpenALDeviceMenu(StGLMenu* theMenu);
    ST_LOCAL void fillRecentMenu(StGLMenu* theMenu);

        private: //! @name callback Slots

    ST_LOCAL void doAboutProgram (const size_t );
    ST_LOCAL void doUserTips     (const size_t );
    ST_LOCAL void doAboutSystem  (const size_t );
    ST_LOCAL void doCheckUpdates (const size_t );
    ST_LOCAL void doOpenLicense  (const size_t );
    ST_LOCAL void doShowFPS(const bool );
    ST_LOCAL void doAboutRenderer(const size_t );
    ST_LOCAL void doAudioDelay   (const size_t );

        private: //! @name private fields

    StMoviePlayer*      myPlugin;           //!< link to the main Drawer class
    StWindow*           myWindow;           //!< link to the window instance
    StTranslations*     myLangMap;          //!< translated strings map
    StString            myTexturesFolder;   //!< folder with textures
    StTimer             myVisibilityTimer;  //!< minimum visible delay

    StGLImageRegion*    myImage;            //!< the main video frame
    StGLSubtitles*      mySubtitles;        //!< the subtitles
    StGLDescription*    myDescr;            //!< description text shown near mouse cursor
    StGLMsgStack*       myMsgStack;         //!< messages stack
    StGLPlayList*       myPlayList;

    StGLMenu*           myMenuRoot;         //!< root of the main menu
    StGLMenu*           myMenuOpenAL;
    StGLMenu*           myMenuRecent;
    StGLMenu*           myMenuAudio;
    StGLMenu*           myMenuSubtitles;

    StGLWidget*         myPanelUpper;       //!< upper toolbar
    StGLTextureButton*  myBtnOpen;
    StGLTextureButton*  myBtnSwapLR;
    StGLWidget*         myBtnSrcFrmt;

    StGLWidget*         myPanelBottom;      //!< bottom toolbar
    StSeekBar*          mySeekBar;
    StGLTextureButton*  myBtnPlay;
    StTimeBox*          myTimeBox;
    StGLTextureButton*  myBtnPrev;
    StGLTextureButton*  myBtnNext;
    StGLTextureButton*  myBtnList;
    StGLTextureButton*  myBtnFullScr;
    StGLFpsLabel*       myFpsWidget;

    bool                myIsVisibleGUI;
    bool                myIsExperimental;

        private:

    friend class StMoviePlayer;

};

#endif // __StMoviePlayerGUI_h_
