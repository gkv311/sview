/**
 * Copyright © 2007-2015 Kirill Gavrilov <kirill@sview.ru>
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

/**
 * Image Viewer application.
 */
class StImageViewer : public StApplication {

    friend class StImageViewGUI;
    class StOpenImage;

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
    ST_CPPEXPORT virtual bool open();

    /**
     * Process callback.
     */
    ST_CPPEXPORT virtual void beforeDraw();

    /**
     * Draw frame for requested view.
     */
    ST_CPPEXPORT virtual void stglDraw(unsigned int theView);

    /**
     * Reset device - release GL resources in old window and re-create them in new window.
     */
    ST_CPPEXPORT virtual bool resetDevice();

        private: //! @name window events slots

    ST_LOCAL virtual void doChangeDevice(const int32_t theValue);
    ST_LOCAL virtual void doResize   (const StSizeEvent&   theEvent);
    ST_LOCAL virtual void doKeyDown  (const StKeyEvent&    theEvent);
    ST_LOCAL virtual void doKeyHold  (const StKeyEvent&    theEvent);
    ST_LOCAL virtual void doKeyUp    (const StKeyEvent&    theEvent);
    ST_LOCAL virtual void doMouseDown(const StClickEvent&  theEvent);
    ST_LOCAL virtual void doMouseUp  (const StClickEvent&  theEvent);
    ST_LOCAL virtual void doFileDrop (const StDNDropEvent& theEvent);
    ST_LOCAL virtual void doNavigate (const StNavigEvent&  theEvent);

        public: //! @name callback Slots

    ST_LOCAL void doOpen1FileDialog(const size_t dummy = 0);
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
    ST_LOCAL void doSlideShow(const size_t dummy = 0);
    ST_LOCAL void doQuit(const size_t dummy = 0);

    ST_LOCAL void doImageAdjustReset(const size_t dummy = 0);
    ST_LOCAL void doReset(const size_t dummy = 0);

    ST_LOCAL void doUpdateStateLoading();
    ST_LOCAL void doUpdateStateLoaded();

    /**
     * Handler for new file loaded event.
     */
    ST_LOCAL void doLoaded();

        public: //! @name Properties

    struct {

        StHandle<StBoolParam>    isFullscreen;     //!< fullscreen state
        StHandle<StBoolParam>    toRestoreRatio;   //!< restore ratio on restart
        StHandle<StInt32Param>   ScaleAdjust;      //!< adjust GUI size, see StGLRootWidget::ScaleAdjust
        StHandle<StFloat32Param> ScaleHiDPI;       //!< adapt  GUI size for HiDPI resolution
        StHandle<StBoolParam>    ScaleHiDPI2X;     //!< option to set HiDPI resolution to 2.0
        StHandle<StInt32Param>   checkUpdatesDays; //!< days count between updates checks
        StHandle<StInt32Param>   srcFormat;        //!< source format
        StHandle<StBoolParam>    ToTrackHead;      //!< enable/disable head-tracking
        StHandle<StBoolParam>    ToShowToolbar;    //!< show/hide toolbar
        StHandle<StBoolParam>    ToShowFps;        //!< display FPS meter
        StHandle<StBoolParam>    IsMobileUI;       //!< display mobile interface
        StHandle<StBoolParam>    IsVSyncOn;        //!< flag to use VSync
        StString                 lastFolder;       //!< laster folder used to open / save file
        StImageFile::ImageClass  imageLib;         //!< preferred image library
        int                      TargetFps;        //!< limit or not rendering FPS

    } params;

    /**
     * Retrieve current playlist item.
     */
    ST_LOCAL bool getCurrentFile(StHandle<StFileNode>&     theFileNode,
                                 StHandle<StStereoParams>& theParams,
                                 StHandle<StImageInfo>&    theInfo);

        private: //! @name private callback Slots

    ST_LOCAL void doScaleGui(const int32_t );
    ST_LOCAL void doScaleHiDPI(const bool );
    ST_LOCAL void doSwitchVSync(const bool theValue);
    ST_LOCAL void doFullscreen(const bool theIsFullscreen);
    ST_LOCAL void doSwitchSrcFormat(const int32_t theSrcFormat);
    ST_LOCAL void doSwitchViewMode(const int32_t theMode);
    ST_LOCAL void doPanoramaOnOff(const size_t );

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
    };

        private:

    /**
     * Initialization routines.
     */
    ST_LOCAL bool init();
    ST_LOCAL bool createGui();
    ST_LOCAL void saveGuiParams();

    /**
     * Parse arguments.
     */
    ST_LOCAL void parseArguments(const StArgumentsMap& theArguments);

    /**
     * Release GL resources.
     */
    ST_LOCAL void releaseDevice();

    ST_LOCAL const StString& tr(const size_t theId) const {
        return myLangMap->getValue(theId);
    }

        private: //! @name private fields

    StHandle<StGLContext>      myContext;
    StHandle<StSettings>       mySettings;        //!< settings manager for Image Viewer plugin
    StHandle<StTranslations>   myLangMap;         //!< translated strings map
    StHandle<StImageViewerGUI> myGUI;             //!< GUI root widget
    StHandle<StImageLoader>    myLoader;          //!< main image loader class
    StHandle<StCheckUpdates>   myUpdates;         //!< check updates utility
    StHandle<StFileNode>       myFileToDelete;    //!< file node for removal
    StHandle<StImageInfo>      myFileInfo;        //!< file info for opened dialog
    StHandle<StOpenImage>      myOpenDialog;      //!< file open dialog
    StString                   myAppName;         //!< name of customized application

    StCondition                myEventLoaded;     //!< indicate that new file was open

    StTimer                    mySlideShowTimer;  //!< slideshow stuff
    double                     mySlideShowDelay;

    int32_t                    myLastUpdateDay;
    bool                       myToCheckUpdates;
    bool                       myToRecreateMenu;
    bool                       myToSaveSrcFormat; //!< indicates that active source format should be saved or not
    bool                       myEscNoQuit;       //!< if true then Escape will not quit application
    bool                       myToHideUIFullScr; //!< if true then GUI will be hidden in full-screen mode
    bool                       myToCheckPoorOrient; //!< switch off orientation sensor with poor quality

        private:

    friend class StImageViewerGUI;

};

#endif //__StImageViewer_h_
