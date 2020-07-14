/**
 * Copyright © 2007-2020 Kirill Gavrilov <kirill@sview.ru>
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

#ifndef __StMoviePlayer_h_
#define __StMoviePlayer_h_

#include <StCore/StApplication.h>

#include <StSettings/StFloat32Param.h>
#include <StGLStereo/StFormatEnum.h>
#include <StThreads/StCondition.h>
#include <StThreads/StThread.h>

#include <StGLWidgets/StGLImageRegion.h>

#include <vector>

// forward declarations
class StALDeviceParam;
class StCheckUpdates;
class StFileNode;
class StGLContext;
class StMovieOpenDialog;
class StMoviePlayerGUI;
class StPlayList;
class StSettings;
class StStereoParams;
class StSubQueue;
class StVideo;
class StWindow;
struct StMovieInfo;

struct mg_context;
struct mg_connection;
struct mg_request_info;

/**
 * Movie Player application.
 */
class StMoviePlayer : public StApplication {

        public:

    static const StString ST_DRAWER_PLUGIN_NAME;

    enum {
        BLOCK_SLEEP_NEVER      = 0,
        BLOCK_SLEEP_ALWAYS     = 1,
        BLOCK_SLEEP_PLAYBACK   = 2,
        BLOCK_SLEEP_FULLSCREEN = 3,
    };

    enum {
        WEBUI_OFF  = 0, //!< do not launch Web UI
        WEBUI_ONCE = 1, //!< launch Web UI once
        WEBUI_AUTO = 2, //!< launch Web UI each time
    };

        public: //! @name interface methods' implementations

    /**
     * Main constructor.
     */
    ST_CPPEXPORT StMoviePlayer(const StHandle<StResourceManager>& theResMgr,
                               const StNativeWin_t                theParentWin = (StNativeWin_t )NULL,
                               const StHandle<StOpenInfo>&        theOpenInfo  = NULL);

    /**
     * Destructor.
     */
    ST_CPPEXPORT virtual ~StMoviePlayer();

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

        public: //! @name callback Slots

    /**
     * Handler for new file loaded event.
     */
    ST_LOCAL void doLoaded();

    ST_LOCAL void doPlayListReverse(const size_t dummy = 0);
    ST_LOCAL void doListFirst(const size_t dummy = 0);
    ST_LOCAL void doListPrev(const size_t dummy = 0);
    ST_LOCAL void doListNext(const size_t dummy = 0);
    ST_LOCAL void doListLast(const size_t dummy = 0);
    ST_LOCAL void doDeleteFileBegin(const size_t dummy = 0);
    ST_LOCAL void doDeleteFileEnd  (const size_t dummy = 0);
    ST_LOCAL void doAudioVolume(size_t theDirection);
    ST_LOCAL void doAudioNext(size_t theDirection);
    ST_LOCAL void doSubtitlesNext(size_t theDirection);
    ST_LOCAL void doSubtitlesCopy(size_t dummy = 0);
    ST_LOCAL void doFromClipboard(size_t dummy = 0);
    ST_LOCAL void doShowHideGUI(const size_t dummy = 0);

    ST_LOCAL void doQuit(const size_t dummy = 0);

    ST_LOCAL void doFileNext();
    ST_LOCAL void doOpen1FileFromGui(StHandle<StString> thePath);
    ST_LOCAL void doOpen1AudioFromGui(StHandle<StString> thePath);
    ST_LOCAL void doOpen1SubtitleFromGui(StHandle<StString> thePath);
    ST_LOCAL void doOpen1FileAction(const size_t dummy = 0);
    ST_LOCAL void doOpen2Files(const size_t dummy = 0);
    ST_LOCAL void doSaveFileInfo(const size_t theToSave);
    ST_LOCAL void doOpenRecent(const size_t theItemId);
    ST_LOCAL void doClearRecent(const size_t dummy = 0);
    ST_LOCAL void doUpdateOpenALDeviceList(const size_t dummy = 0);
    ST_LOCAL void doAddAudioStream(const size_t dummy = 0);
    ST_LOCAL void doAddSubtitleStream(const size_t dummy = 0);
    ST_LOCAL void doSeekLeft(const size_t dummy = 0);
    ST_LOCAL void doSeekRight(const size_t dummy = 0);
    ST_LOCAL void doSeek(const int mouseBtn, const double seekX);
    ST_LOCAL void doPlayPause(const size_t dummy = 0);
    ST_LOCAL void doStop(const size_t dummy = 0);

    ST_LOCAL void doSnapshot(const size_t theImgType);
    ST_LOCAL void doAboutFile(const size_t dummy = 0);

        public: //! @name Properties

    ST_LOCAL bool getCurrentFile(StHandle<StFileNode>&     theFileNode,
                                 StHandle<StStereoParams>& theParams,
                                 StHandle<StMovieInfo>&    theInfo);

    ST_LOCAL void getRecentList(StArrayList<StString>& theList);

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
     * Return TRUE if OpenAL implementation provides HRTF mixing feature.
     */
    ST_CPPEXPORT bool hasAlHrtf() const;

    struct {

        StHandle<StEnumParam>         ScaleAdjust;       //!< adjust GUI size, see StGLRootWidget::ScaleAdjust
        StHandle<StFloat32Param>      ScaleHiDPI;        //!< adapt  GUI size for HiDPI resolution
        StHandle<StBoolParamNamed>    ScaleHiDPI2X;      //!< option to set HiDPI resolution to 2.0
        StHandle<StInt32ParamNamed>   SubtitlesPlace;    //!< subtitles placement
        StHandle<StFloat32Param>      SubtitlesTopDY;    //!< subtitles vertical displacement
        StHandle<StFloat32Param>      SubtitlesBottomDY; //!< subtitles vertical displacement
        StHandle<StFloat32Param>      SubtitlesSize;     //!< subtitles font size
        StHandle<StFloat32Param>      SubtitlesParallax; //!< subtitles parallax
        StHandle<StBoolParamNamed>    ToSearchSubs;      //!< automatically search for additional subtitles/audio track files nearby video file
        StHandle<StEnumParam>         SubtitlesParser;   //!< subtitles parser
        StHandle<StBoolParamNamed>    SubtitlesApplyStereo; //!<  apply stereoscopic format of video to image subtitles
        StHandle<StALDeviceParam>     AudioAlDevice;     //!< active OpenAL device
        StHandle<StEnumParam>         AudioAlHrtf;       //!< OpenAL HRTF flag
        StHandle<StFloat32Param>      AudioGain;         //!< volume factor
        StHandle<StBoolParamNamed>    AudioMute;         //!< volume mute flag
        StHandle<StFloat32Param>      AudioDelay;        //!< audio/video synchronization delay
        StHandle<StBoolParamNamed>    IsFullscreen;      //!< fullscreen state
        StHandle<StEnumParam>         ExitOnEscape;     //!< exit action on escape
        StHandle<StBoolParamNamed>    ToRestoreRatio;    //!< restore ratio on restart
        StHandle<StBoolParamNamed>    IsShuffle;         //!< shuffle playback order
        StHandle<StBoolParamNamed>    ToLoopSingle;      //!< play single playlist item in loop
        StHandle<StBoolParamNamed>    AreGlobalMKeys;    //!< capture global multimedia keys
        StHandle<StEnumParam>         CheckUpdatesDays;  //!< days count between updates checks
        StHandle<StInt32ParamNamed>   LastUpdateDay;     //!< the last time update has been checked
        StHandle<StInt32ParamNamed>   SrcStereoFormat;   //!< source format
        StHandle<StBoolParamNamed>    ToSwapJPS;         //!< swap JPS views order
        StHandle<StBoolParamNamed>    ToStickPanorama;   //!< force panorama input for all files
        StHandle<StBoolParamNamed>    ToTrackHead;       //!< enable/disable head-tracking
        StHandle<StBoolParamNamed>    ToTrackHeadAudio;  //!< enable/disable head-tracking for audio listener
        StHandle<StBoolParamNamed>    ToForceBFormat;    //!< force B-Format for any 4-channels audio stream
        StHandle<StBoolParamNamed>    ToShowPlayList;    //!< display playlist
        StHandle<StBoolParamNamed>    ToShowAdjustImage; //!< display image adjustment overlay
        StHandle<StBoolParamNamed>    ToShowFps;         //!< display FPS meter
        StHandle<StBoolParamNamed>    ToShowMenu;        //!< show main menu
        StHandle<StBoolParamNamed>    ToShowTopbar;      //!< show topbar
        StHandle<StBoolParamNamed>    ToShowBottom;      //!< show bottom (seekbar)
        StHandle<StBoolParamNamed>    ToMixImagesVideos; //!< mix videos and images
        StHandle<StFloat32Param>      SlideShowDelay;    //!< slideshow delay
        StHandle<StBoolParamNamed>    IsMobileUI;        //!< display mobile interface (user option)
        StHandle<StBoolParam>         IsMobileUISwitch;  //!< display mobile interface (actual value)
        StHandle<StBoolParamNamed>    IsExclusiveFullScreen; //!< exclusive fullscreen mode
        StHandle<StBoolParamNamed>    ToLimitFps;        //!< limit CPU usage or not
        StHandle<StBoolParamNamed>    ToSmoothUploads;   //!< smooth texture uploads
        StHandle<StBoolParamNamed>    IsVSyncOn;         //!< flag to use VSync
        StHandle<StEnumParam>         StartWebUI;        //!< to start Web UI or not
        StHandle<StBoolParamNamed>    ToPrintWebErrors;  //!< print Web UI starting errors
        StHandle<StBoolParamNamed>    IsLocalWebUI;      //!< restrict remote access to 127.0.0.0
        StHandle<StInt32ParamNamed>   WebUIPort;         //!< port to start Web UI
        StHandle<StInt32Param>        AudioStream;       //!< active Audio stream
        StHandle<StInt32Param>        SubtitlesStream;   //!< active Subtitles stream
        StHandle<StEnumParam>         BlockSleeping;     //!< active Audio stream
        StHandle<StBoolParamNamed>    ToHideStatusBar;   //!< hide system-provided status bar
        StHandle<StBoolParamNamed>    ToHideNavBar;      //!< hide system-provided navigation bar
        StHandle<StBoolParamNamed>    ToOpenLast;        //!< option to open last file from recent list by default
        StHandle<StBoolParamNamed>    ToShowExtra;       //!< show experimental menu items
        StHandle<StInt32ParamNamed>   SnapshotImgType;   //!< default snapshot image type
        StString                      lastFolder;        //!< laster folder used to open / save file
        StHandle<StInt32ParamNamed>   TargetFps;         //!< rendering FPS limit (0 - max FPS with less CPU, 1,2,3 - adjust to video FPS)
        StHandle<StBoolParamNamed>    UseGpu;            //!< use video decoding on GPU when available
        StHandle<StBoolParamNamed>    UseOpenJpeg;       //!< use OpenJPEG (libopenjpeg) instead of built-in jpeg2000 decoder
        StHandle<StBoolParamNamed>    Benchmark;         //!< benchmark flag

    } params;

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

        private:

    /**
     * Initialization routines.
     */
    ST_LOCAL bool init();
    ST_LOCAL void updateStrings();
    ST_LOCAL void saveGuiParams();
    ST_LOCAL void saveAllParams();
    ST_LOCAL bool createGui(StHandle<StGLTextureQueue>& theTextureQueue,
                            StHandle<StSubQueue>&       theSubQueue);

    /**
     * Parse arguments.
     */
    ST_LOCAL void parseArguments(const StArgumentsMap& theArguments);

    /**
     * Release GL resources.
     */
    ST_LOCAL void releaseDevice();

    ST_LOCAL static GLfloat gainToVolume(const StHandle<StFloat32Param>& theGain) {
        return (theGain->getMinValue() - theGain->getValue()) / theGain->getMinValue();
    }

    ST_LOCAL static GLfloat volumeToGain(const StHandle<StFloat32Param>& theGain,
                                         const GLfloat                   theVol) {
        return theGain->getMinValue() - theVol * theGain->getMinValue();
    }

        private: //! @name private callback Slots

    ST_LOCAL virtual void doChangeLanguage(const int32_t theNewLang) ST_ATTR_OVERRIDE;
    ST_LOCAL void doScaleGui(const int32_t );
    ST_LOCAL void doChangeMobileUI(const bool );
    ST_LOCAL void doScaleHiDPI(const bool );
    ST_LOCAL void doChangeMixImagesVideos(const bool );
    ST_LOCAL void doSwitchVSync(const bool theValue);
    ST_LOCAL void doSwitchAudioDevice(const int32_t theDevId);
    ST_LOCAL void doSwitchAudioAlHrtf(const int32_t theValue);
    ST_LOCAL void doSetForceBFormat(const bool theToForce);
    ST_LOCAL void doSetAudioVolume(const float theGain);
    ST_LOCAL void doSetAudioMute(const bool theToMute);
    ST_LOCAL void doSetAudioDelay(const float theDelaySec);
    ST_LOCAL void doSwitchShuffle(const bool theShuffleOn);
    ST_LOCAL void doSwitchLoopSingle(const bool theValue);
    ST_LOCAL void doFullscreen(const bool theIsFullscreen);
    ST_LOCAL void doSwitchSrcFormat(const int32_t theSrcFormat);
    ST_LOCAL void doSetStereoOutput(const size_t theMode);
    ST_LOCAL void doSwitchViewMode(const int32_t theMode);
    ST_LOCAL void doPanoramaOnOff(const size_t );
    ST_LOCAL void doChangeStickPano360(const bool );
    ST_LOCAL void doChangeSwapJPS(const bool );
    ST_LOCAL void doSwitchAudioStream(const int32_t theStreamId);
    ST_LOCAL void doSwitchSubtitlesStream(const int32_t theStreamId);
    ST_LOCAL void doShowPlayList(const bool theToShow);
    ST_LOCAL void doShowAdjustImage(const bool theToShow);
    ST_LOCAL void doUpdateStateLoading();
    ST_LOCAL void doUpdateStateLoaded();
    ST_LOCAL void doImageAdjustReset(const size_t dummy = 0);
    ST_LOCAL void doHideSystemBars(const bool theToHide);
    ST_LOCAL void doSetBenchmark(const bool theValue);

        public:

    /**
     * Actions identifiers.
     */
    enum ActionId {
        Action_Quit,
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
        Action_ListPrevExt,
        Action_ListNextExt,
        Action_PlayPause,
        Action_Stop,
        Action_SeekLeft5,
        Action_SeekRight5,
        Action_Open1File,
        Action_SaveSnapshot,
        Action_DeleteFile,
        Action_AudioMute,
        Action_AudioDecrease,
        Action_AudioIncrease,
        Action_AudioPrev,
        Action_AudioNext,
        Action_SubsPrev,
        Action_SubsNext,
        Action_CopyToClipboard,
        Action_PasteFromClipboard,
        Action_ShowList,
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

        private: //! @name Web UI methods

    ST_LOCAL static int beginRequestHandler(mg_connection* theConnection);

    ST_LOCAL int beginRequest(mg_connection*         theConnection,
                              const mg_request_info& theRequestInfo);

    ST_LOCAL void doStopWebUI();
    ST_LOCAL void doStartWebUI();
    ST_LOCAL void doSwitchWebUI(const int32_t theValue);

        private: //! @name private fields

    StHandle<StGLContext>       myContext;
    StHandle<StSettings>        mySettings;        //!< settings manager for Image Viewer plugin
    StHandle<StPlayList>        myPlayList;        //!< play list
    StHandle<StMoviePlayerGUI>  myGUI;             //!< GUI root widget
    StHandle<StVideo>           myVideo;           //!< main video playback class
    StHandle<StCheckUpdates>    myUpdates;         //!< check updates utility
    StHandle<StFileNode>        myFileToDelete;    //!< file node for removal
    StHandle<StMovieInfo>       myFileInfo;        //!< file info for opened dialog
    StHandle<StMovieOpenDialog> myOpenDialog;      //!< file open dialog

    StCondition                 myEventLoaded;     //!< indicate that new file was open
    StTimer                     myInactivityTimer; //!< timer initialized when application goes into paused state
    double                      mySeekOnLoad;      //!< seeking target
    int32_t                     myAudioOnLoad;     //!< audio     track on load
    int32_t                     mySubsOnLoad;      //!< subtitles track on load

    mg_context*                 myWebCtx;          //!< web UI context

    bool                        myToUpdateALList;
    bool                        myToCheckUpdates;
    bool                        myToCheckPoorOrient; //!< switch off orientation sensor with poor quality

    friend class StMoviePlayerGUI;
    friend class StMovieOpenDialog;

};

#endif // __StMoviePlayer_h_
