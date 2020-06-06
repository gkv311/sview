/**
 * Copyright © 2007-2020 Kirill Gavrilov <kirill@sview.ru>
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

#ifndef __StImageViewer_h_
#define __StImageViewer_h_

#include <StCore/StApplication.h>
#include <StGLWidgets/StGLImageRegion.h>

#include "StImageLoader.h"
#include "StImageViewerGUI.h"

// forward declarations
class StGLContext;
class StCheckUpdates;
class StWindow;
class StImageOpenDialog;

/**
 * Image Viewer application.
 */
class StImageViewer : public StApplication {

        public:

    static const char* ST_DRAWER_PLUGIN_NAME;

        public: //! @name interface methods' implementations

    /**
     * Main constructor.
     */
    ST_CPPEXPORT StImageViewer(const StHandle<StResourceManager>& theResMgr,
                               const StNativeWin_t                theParentWin = (StNativeWin_t )NULL,
                               const StHandle<StOpenInfo>&        theOpenInfo  = NULL,
                               const StString&                    theAppName   = "");

    /**
     * Destructor.
     */
    ST_CPPEXPORT virtual ~StImageViewer();

    /**
     * Open application.
     */
    ST_CPPEXPORT virtual bool open() ST_ATTR_OVERRIDE;

    /**
     * Process callback.
     */
    ST_CPPEXPORT virtual void beforeDraw() ST_ATTR_OVERRIDE;

    /**
     * Draw frame for requested view.
     */
    ST_CPPEXPORT virtual void stglDraw(unsigned int theView) ST_ATTR_OVERRIDE;

    /**
     * Reset device - release GL resources in old window and re-create them in new window.
     */
    ST_CPPEXPORT virtual bool resetDevice() ST_ATTR_OVERRIDE;

        private: //! @name window events slots

    ST_LOCAL virtual void doChangeDevice(const int32_t theValue)     ST_ATTR_OVERRIDE;
    ST_LOCAL virtual void doPause    (const StPauseEvent&  theEvent) ST_ATTR_OVERRIDE;
    ST_LOCAL virtual void doResize   (const StSizeEvent&   theEvent) ST_ATTR_OVERRIDE;
    ST_LOCAL virtual void doKeyDown  (const StKeyEvent&    theEvent) ST_ATTR_OVERRIDE;
    ST_LOCAL virtual void doKeyHold  (const StKeyEvent&    theEvent) ST_ATTR_OVERRIDE;
    ST_LOCAL virtual void doKeyUp    (const StKeyEvent&    theEvent) ST_ATTR_OVERRIDE;
    ST_LOCAL virtual void doMouseDown(const StClickEvent&  theEvent) ST_ATTR_OVERRIDE;
    ST_LOCAL virtual void doMouseUp  (const StClickEvent&  theEvent) ST_ATTR_OVERRIDE;
    ST_LOCAL virtual void doGesture  (const StGestureEvent& theEvent) ST_ATTR_OVERRIDE;
    ST_LOCAL virtual void doScroll   (const StScrollEvent& theEvent) ST_ATTR_OVERRIDE;
    ST_LOCAL virtual void doFileDrop (const StDNDropEvent& theEvent) ST_ATTR_OVERRIDE;
    ST_LOCAL virtual void doNavigate (const StNavigEvent&  theEvent) ST_ATTR_OVERRIDE;

        public: //! @name callback Slots

    ST_LOCAL void doOpen1FileFromGui(StHandle<StString> thePath);
    ST_LOCAL void doOpen1FileAction(const size_t dummy = 0);
    ST_LOCAL void doOpen2FilesDialog(const size_t dummy = 0);
    ST_LOCAL void doSaveImageAs(const size_t theImgType) { myLoader->doSaveImageAs(theImgType); }
    ST_LOCAL void doSaveImageInfo(const size_t theToSave);
    ST_LOCAL void doSaveImageInfoBegin(const size_t dummy = 0);
    ST_LOCAL void doAboutImage(const size_t dummy = 0);
    ST_LOCAL void doListFirst(const size_t dummy = 0);
    ST_LOCAL void doListPrev(const size_t dummy = 0);
    ST_LOCAL void doListNext(const size_t dummy = 0);
    ST_LOCAL void doListLast(const size_t dummy = 0);
    ST_LOCAL void doDeleteFileBegin(const size_t dummy = 0);
    ST_LOCAL void doDeleteFileEnd(const size_t dummy = 0);
    ST_LOCAL void doShowHideGUI(const size_t dummy = 0);
    ST_LOCAL void doSlideShow(const size_t dummy = 0);
    ST_LOCAL void doQuit(const size_t dummy = 0);

    ST_LOCAL void doImageAdjustReset(const size_t dummy = 0);

    ST_LOCAL void doUpdateStateLoading();
    ST_LOCAL void doUpdateStateLoaded();

    /**
     * Handler for new file loaded event.
     */
    ST_LOCAL void doLoaded();

        public: //! @name Properties

    struct {

        StHandle<StBoolParamNamed>    IsFullscreen;     //!< fullscreen state
        StHandle<StEnumParam>         ExitOnEscape;     //!< exit action on escape
        StHandle<StBoolParamNamed>    ToRestoreRatio;   //!< restore ratio on restart
        StHandle<StEnumParam>         ScaleAdjust;      //!< adjust GUI size, see StGLRootWidget::ScaleAdjust
        StHandle<StFloat32Param>      ScaleHiDPI;       //!< adapt  GUI size for HiDPI resolution
        StHandle<StBoolParamNamed>    ScaleHiDPI2X;     //!< option to set HiDPI resolution to 2.0
        StHandle<StEnumParam>         CheckUpdatesDays; //!< days count between updates checks
        StHandle<StInt32ParamNamed>   LastUpdateDay;    //!< the last time update has been checked
        StHandle<StInt32ParamNamed>   SrcStereoFormat;  //!< source format
        StHandle<StBoolParamNamed>    ToSwapJPS;        //!< swap JPS views order
        StHandle<StBoolParamNamed>    ToStickPanorama;  //!< force panorama input for all files
        StHandle<StBoolParamNamed>    ToFlipCubeZ6x1;   //!< flip Z coordinate within Cube map 6x1
        StHandle<StBoolParamNamed>    ToFlipCubeZ3x2;   //!< flip Z coordinate within Cube map 3x2
        StHandle<StBoolParamNamed>    ToTrackHead;      //!< enable/disable head-tracking
        StHandle<StBoolParamNamed>    ToShowMenu;       //!< show main menu
        StHandle<StBoolParamNamed>    ToShowTopbar;     //!< show topbar
        StHandle<StBoolParamNamed>    ToShowBottom;     //!< show bottom
        StHandle<StBoolParamNamed>    ToShowPlayList;   //!< display playlist
        StHandle<StBoolParamNamed>    ToShowAdjustImage;//!< display image adjustment overlay
        StHandle<StBoolParamNamed>    ToShowFps;        //!< display FPS meter
        StHandle<StFloat32Param>      SlideShowDelay;   //!< slideshow delay
        StHandle<StBoolParamNamed>    IsMobileUI;       //!< display mobile interface (user option)
        StHandle<StBoolParam>         IsMobileUISwitch; //!< display mobile interface (actual value)
        StHandle<StBoolParamNamed>    ToHideStatusBar;  //!< hide system-provided status bar
        StHandle<StBoolParamNamed>    ToHideNavBar;     //!< hide system-provided navigation bar
        StHandle<StBoolParamNamed>    IsExclusiveFullScreen; //!< exclusive fullscreen mode
        StHandle<StBoolParamNamed>    IsVSyncOn;        //!< flag to use VSync
        StHandle<StBoolParamNamed>    ToOpenLast;       //!< option to open last file from recent list by default
        StHandle<StBoolParamNamed>    ToSaveRecent;     //!< load/save recent file
        StString                      lastFolder;       //!< laster folder used to open / save file
        StImageFile::ImageClass       imageLib;         //!< preferred image library
        StHandle<StInt32ParamNamed>   TargetFps;        //!< limit or not rendering FPS

    } params;

    /**
     * Return true if mobile UI should be enabled considering user option and window margins.
     */
    ST_LOCAL bool toUseMobileUI() const {
        return toUseMobileUI(!myWindow.isNull() ? myWindow->getMargins() : StMarginsI());
    }

    /**
     * Return true if mobile UI should be enabled considering user option and window margins.
     */
    ST_LOCAL bool toUseMobileUI(const StMarginsI& theMargins) const {
        return params.IsMobileUI->getValue()
            || theMargins.top > 0;
    }

    /**
     * Retrieve current playlist item.
     */
    ST_LOCAL bool getCurrentFile(StHandle<StFileNode>&     theFileNode,
                                 StHandle<StStereoParams>& theParams,
                                 StHandle<StImageInfo>&    theInfo);

        private: //! @name private callback Slots

    ST_LOCAL virtual void doChangeLanguage(const int32_t ) ST_ATTR_OVERRIDE;
    ST_LOCAL void doScaleGui(const int32_t );
    ST_LOCAL void doChangeMobileUI(const bool theIsOn);
    ST_LOCAL void doHideSystemBars(const bool theToHide);
    ST_LOCAL void doScaleHiDPI(const bool );
    ST_LOCAL void doSwitchVSync(const bool theValue);
    ST_LOCAL void doFullscreen(const bool theIsFullscreen);
    ST_LOCAL void doSwitchSrcFormat(const int32_t theSrcFormat);
    ST_LOCAL void doSwitchViewMode(const int32_t theMode);
    ST_LOCAL void doSetStereoOutput(const size_t theMode);
    ST_LOCAL void doPanoramaOnOff(const size_t );
    ST_LOCAL void doChangeSwapJPS(const bool );
    ST_LOCAL void doChangeStickPano360(const bool );
    ST_LOCAL void doChangeFlipCubeZ(const bool );
    ST_LOCAL void doShowPlayList(const bool theToShow);
    ST_LOCAL void doShowAdjustImage(const bool theToShow);
    ST_LOCAL void doFileNext();

        public:

    /**
     * Actions identifiers.
     */
    enum ActionId {
        Action_Fullscreen,
        Action_ShowFps,
        Action_SrcAuto,
        Action_SrcMono,
        Action_SrcOverUnderLR,
        Action_SrcSideBySideRL,
        Action_FileInfo,
        Action_ListFirst,
        Action_ListLast,
        Action_ListPrev,
        Action_ListNext,
        Action_SlideShow,
        Action_SavePng,
        Action_SaveJpeg,
        Action_SaveFileInfo,
        Action_DeleteFile,
        Action_ImageAdjustReset,
        Action_StereoParamsBegin,
        Action_StereoParamsEnd = Action_StereoParamsBegin + StGLImageRegion::ActionsNb - 1,
        Action_PanoramaOnOff,
        Action_ShowGUI,
        Action_OutStereoNormal,
        Action_OutStereoLeftView,
        Action_OutStereoRightView,
        Action_OutStereoParallelPair,
        Action_OutStereoCrossEyed,
    };

        private:

    /**
     * Initialization routines.
     */
    ST_LOCAL bool init();
    ST_LOCAL void updateStrings();
    ST_LOCAL bool createGui();
    ST_LOCAL void saveGuiParams();
    ST_LOCAL void saveAllParams();

    /**
     * Parse arguments.
     */
    ST_LOCAL void parseArguments(const StArgumentsMap& theArguments);

    /**
     * Release GL resources.
     */
    ST_LOCAL void releaseDevice();

        private: //! @name private fields

    StHandle<StGLContext>       myContext;
    StHandle<StSettings>        mySettings;        //!< settings manager for Image Viewer plugin
    StHandle<StPlayList>        myPlayList;        //!< play list
    StHandle<StImageViewerGUI>  myGUI;             //!< GUI root widget
    StHandle<StImageLoader>     myLoader;          //!< main image loader class
    StHandle<StCheckUpdates>    myUpdates;         //!< check updates utility
    StHandle<StFileNode>        myFileToDelete;    //!< file node for removal
    StHandle<StImageInfo>       myFileInfo;        //!< file info for opened dialog
    StHandle<StImageOpenDialog> myOpenDialog;      //!< file open dialog
    StString                    myAppName;         //!< name of customized application

    StCondition                 myEventLoaded;     //!< indicate that new file was open
    StTimer                     myInactivityTimer; //!< timer initialized when application goes into paused state
    StTimer                     mySlideShowTimer;  //!< slideshow timer

    bool                        myToCheckUpdates;
    bool                        myToSaveSrcFormat; //!< indicates that active source format should be saved or not
    bool                        myEscNoQuit;       //!< if true then Escape will not quit application
    bool                        myToHideUIFullScr; //!< if true then GUI will be hidden in full-screen mode
    bool                        myToCheckPoorOrient; //!< switch off orientation sensor with poor quality

        private:

    friend class StImageViewerGUI;
    friend class StImageOpenDialog;

};

#endif //__StImageViewer_h_
