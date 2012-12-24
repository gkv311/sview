/**
 * Copyright © 2009-2012 Kirill Gavrilov <kirill@sview.ru>
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
#include <StGLStereo/StFormatEnum.h>
#include <StSettings/StTranslations.h>

// forward declarations
class StMoviePlayer;
class StGLDescription;
class StGLImageRegion;
class StGLMenu;
class StGLMenuItem;
class StGLSubtitles;
class StGLTextureButton;
class StSeekBar;
class StTimeBox;
class StUtfLangMap;
class StGLMsgStack;
class StWindow;

/**
 * Root GUI widget for Video Playback plugin.
 */
class ST_LOCAL StMoviePlayerGUI : public StGLRootWidget {

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

    StMoviePlayer*            myPlugin; //!< link to the main Drawer class
    StWindow*                 myWindow; //!< link to the window instance
    StHandle<StTranslations> myLangMap; //!< translated strings map
    StString          texturesPathRoot; // textures directory
    StTimer          stTimeVisibleLock; // minimum visible delay

    StGLImageRegion* stImageRegion; // the main video frame
    StGLSubtitles*     stSubtitles; // the main subtitles
    StGLDescription*       stDescr; // description text shown near mouse cursor
    StGLMsgStack*       myMsgStack; // messages stack

    StGLMenu*            menu0Root; // main menu
    StGLMenu*          myMenuAudio;
    StGLMenu*      myMenuSubtitles;

    StGLWidget*        upperRegion; // upper toolbar
    StGLTextureButton*     btnOpen;
    StGLTextureButton*   btnSwapLR;
    StGLWidget*       myBtnSrcFrmt;

    StGLWidget*       bottomRegion; // bottom toolbar
    StSeekBar*             seekBar;
    StGLTextureButton*     btnPlay;
    StTimeBox*           stTimeBox;
    StGLTextureButton*     btnPrev;
    StGLTextureButton*     btnNext;
    StGLTextureButton*     btnList;
    StGLTextureButton*  btnFullScr;

    bool isGUIVisible;

        private:

    void createUpperToolbar();
    void createBottomToolbar();

        private: //!< menus creation routines

    void      createMainMenu();         // Root (Main menu)
    StGLMenu* createMediaMenu();        // Root -> Media menu
    StGLMenu* createOpenMovieMenu();    // Root -> Media -> Open movie menu
    StGLMenu* createSaveImageMenu();    // Root -> Media -> Save snapshot menu
    StGLMenu* createSrcFormatMenu();    // Root -> Media -> Source format menu
    StGLMenu* createOpenALDeviceMenu(); // Root -> Media -> OpenAL Device menu
    StGLMenu* createViewMenu();         // Root -> View menu
    StGLMenu* createDisplayModeMenu();  // Root -> View menu -> Output
    StGLMenu* createDisplayRatioMenu(); // Root -> View menu -> Display Ratio
    StGLMenu* createSmoothFilterMenu(); // Root -> View menu -> Smooth Filter
    StGLMenu* createGammaMenu();        // Root -> View menu -> Gamma Correction
    StGLMenu* createAudioMenu();        // Root -> Audio menu
    StGLMenu* createAudioGainMenu();    // Root -> Audio menu -> Volume
    StGLMenu* createSubtitlesMenu();    // Root -> Subtitles menu
    StGLMenu* createHelpMenu();         // Root -> Help menu
    StGLMenu* createCheckUpdatesMenu(); // Root -> Help -> Check updates menu
    StGLMenu* createLanguageMenu();     // Root -> Help -> Language menu

        public: //!< StGLRootWidget overrides

    StMoviePlayerGUI(StMoviePlayer* thePlugin,
                     StWindow*      theWindow,
                     size_t theTextureQueueSizeMax);
    virtual ~StMoviePlayerGUI();
    virtual void stglUpdate(const StPointD_t& thePointZo,
                            const GLfloat theProgress,
                            const double thePTS);
    virtual void stglResize(const StRectI_t& winRectPx);
    virtual void setVisibility(const StPointD_t& pointZo, bool isMouseActive = false);

        public:

    bool toHideCursor();
    void showUpdatesNotify();

        public: //!< menu update routines

    void updateAudioStreamsMenu    (const StHandle< StArrayList<StString> >& theStreamsList,
                                    const bool theHasVideo);
    void updateSubtitlesStreamsMenu(const StHandle< StArrayList<StString> >& theStreamsList);

    void doAboutFile(const size_t );

        private: //!< callback Slots

    void doAboutProgram (const size_t );
    void doCheckUpdates (const size_t );
    void doOpenLicense  (const size_t );

};

#endif //__StMoviePlayerGUI_h_
