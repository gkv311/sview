/**
 * Copyright © 2007-2013 Kirill Gavrilov <kirill@sview.ru>
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

#include <StSettings/StTranslations.h>
#include <StSettings/StFloat32Param.h>
#include <StGLStereo/StFormatEnum.h>
#include <StThreads/StCondition.h>
#include <StThreads/StThread.h>

#include <StGLWidgets/StGLImageRegion.h>

// forward declarations
class StCheckUpdates;
class StFileNode;
class StGLContext;
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

class StALDeviceParam : public StInt32Param {

        public:

    /**
     * Main constructor.
     */
    ST_LOCAL StALDeviceParam();

    /**
     * Desctructor.
     */
    ST_LOCAL ~StALDeviceParam();

    ST_LOCAL void initList();

    ST_LOCAL bool init(const StString& theActive);

    ST_LOCAL int32_t getValueFromName(const StString& theName);

    /**
     * Returns title for active AL device.
     */
    ST_LOCAL StString getTitle() const;

    /**
     * Return list of available translations.
     */
    ST_LOCAL const StArrayList<StString>& getList() const {
        return myDevicesList;
    }

        private:

    StArrayList<StString> myDevicesList;

};

/**
 * Movie Player application.
 */
class StMoviePlayer : public StApplication {

        public:

    static const StString ST_DRAWER_PLUGIN_NAME;

    enum {
        OPEN_FILE_MOVIE       = 0,
        OPEN_FILE_2MOVIES     = 1,
        OPEN_STREAM_AUDIO     = 2,
        OPEN_STREAM_SUBTITLES = 3,
    };

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
    ST_CPPEXPORT StMoviePlayer(const StNativeWin_t         theParentWin = (StNativeWin_t )NULL,
                               const StHandle<StOpenInfo>& theOpenInfo  = NULL);

    /**
     * Destructor.
     */
    ST_CPPEXPORT virtual ~StMoviePlayer();

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
    ST_LOCAL void doAudioNext(size_t theDirection);
    ST_LOCAL void doSubtitlesNext(size_t theDirection);

    ST_LOCAL void doQuit(const size_t dummy = 0);

    ST_LOCAL void doFileNext();
    ST_LOCAL void doOpen1File(const size_t dummy = 0);
    ST_LOCAL void doOpen2Files(const size_t dummy = 0);
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
    ST_LOCAL void doReset(const size_t dummy = 0);

    ST_LOCAL void doSnapshot(const size_t theImgType);

        public: //! @name Properties

    ST_LOCAL bool getCurrentFile(StHandle<StFileNode>&     theFileNode,
                                 StHandle<StStereoParams>& theParams,
                                 StHandle<StMovieInfo>&    theInfo);

    ST_LOCAL void getRecentList(StArrayList<StString>& theList);

    struct {

        StHandle<StInt32Param>    ScaleAdjust;      //!< adjust GUI size, see StGLRootWidget::ScaleAdjust
        StHandle<StFloat32Param>  ScaleHiDPI;       //!< adapt  GUI size for HiDPI resolution
        StHandle<StBoolParam>     ScaleHiDPI2X;     //!< option to set HiDPI resolution to 2.0
        StHandle<StFloat32Param>  SubtitlesSize;    //!< subtitles font size
        StHandle<StFloat32Param>  SubtitlesParallax;//!< subtitles parallax
        StHandle<StALDeviceParam> alDevice;         //!< active OpenAL device
        StHandle<StFloat32Param>  AudioGain;        //!< volume factor
        StHandle<StBoolParam>     AudioMute;        //!< volume mute flag
        StHandle<StFloat32Param>  AudioDelay;       //!< audio/video synchronization delay
        StHandle<StBoolParam>     isFullscreen;     //!< fullscreen state
        StHandle<StBoolParam>     toRestoreRatio;   //!< restore ratio on restart
        StHandle<StBoolParam>     isShuffle;        //!< shuffle playback order
        StHandle<StBoolParam>     ToLoopSingle;     //!< play single playlist item in loop
        StHandle<StBoolParam>     areGlobalMKeys;   //!< capture global multimedia keys
        StHandle<StInt32Param>    checkUpdatesDays; //!< days count between updates checks
        StHandle<StInt32Param>    srcFormat;        //!< source format
        StHandle<StBoolParam>     ToShowPlayList;   //!< display playlist
        StHandle<StBoolParam>     ToShowFps;        //!< display FPS meter
        StHandle<StBoolParam>     ToLimitFps;       //!< limit CPU usage or not
        StHandle<StBoolParam>     IsVSyncOn;        //!< flag to use VSync
        StHandle<StEnumParam>     StartWebUI;       //!< to start Web UI or not
        StHandle<StBoolParam>     ToPrintWebErrors; //!< print Web UI starting errors
        StHandle<StInt32Param>    WebUIPort;        //!< port to start Web UI
        StHandle<StInt32Param>    audioStream;      //!< active Audio stream
        StHandle<StInt32Param>    subtitlesStream;  //!< active Subtitles stream
        StHandle<StInt32Param>    blockSleeping;    //!< active Audio stream
        StHandle<StBoolParam>     ToShowExtra;      //!< show experimental menu items
        StHandle<StInt32Param>    SnapshotImgType;  //!< default snapshot image type
        StString                  lastFolder;       //!< laster folder used to open / save file
        int                       TargetFps;        //!< rendering FPS limit (0 - max FPS with less CPU, 1,2,3 - adjust to video FPS)
        StHandle<StBoolParam>     UseGpu;           //!< use video decoding on GPU when available

    } params;

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

        private:

    /**
     * Initialization routines.
     */
    ST_LOCAL bool init();
    ST_LOCAL void saveGuiParams();
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

    ST_LOCAL const StString& tr(const size_t theId) const {
        return myLangMap->getValue(theId);
    }

    ST_LOCAL static GLfloat gainToVolume(const StHandle<StFloat32Param>& theGain) {
        return (theGain->getMinValue() - theGain->getValue()) / theGain->getMinValue();
    }

    ST_LOCAL static GLfloat volumeToGain(const StHandle<StFloat32Param>& theGain,
                                         const GLfloat                   theVol) {
        return theGain->getMinValue() - theVol * theGain->getMinValue();
    }

        private: //! @name private callback Slots

    ST_LOCAL void doScaleGui(const int32_t );
    ST_LOCAL void doScaleHiDPI(const bool );
    ST_LOCAL void doSwitchVSync(const bool theValue);
    ST_LOCAL void doSwitchAudioDevice(const int32_t theDevId);
    ST_LOCAL void doSetAudioVolume(const float theGain);
    ST_LOCAL void doSetAudioMute(const bool theToMute);
    ST_LOCAL void doSetAudioDelay(const float theDelaySec);
    ST_LOCAL void doSwitchShuffle(const bool theShuffleOn);
    ST_LOCAL void doSwitchLoopSingle(const bool theValue);
    ST_LOCAL void doFullscreen(const bool theIsFullscreen);
    ST_LOCAL void doSwitchSrcFormat(const int32_t theSrcFormat);
    ST_LOCAL void doSwitchAudioStream(const int32_t theStreamId);
    ST_LOCAL void doSwitchSubtitlesStream(const int32_t theStreamId);
    ST_LOCAL void doShowPlayList(const bool theToShow);
    ST_LOCAL void doUpdateStateLoading();
    ST_LOCAL void doUpdateStateLoaded();
    ST_LOCAL friend SV_THREAD_FUNCTION openFileThread(void* theArg);
    ST_LOCAL void doOpenFileDialog(const size_t theOpenType);
    ST_LOCAL void doImageAdjustReset(const size_t dummy = 0);

        private:

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
        Action_AudioPrev,
        Action_AudioNext,
        Action_SubsPrev,
        Action_SubsNext,
        Action_ShowList,
        Action_ImageAdjustReset,
        Action_StereoParamsBegin,
        Action_StereoParamsEnd = Action_StereoParamsBegin + StGLImageRegion::ActionsNb,
    };

        private: //! @name Web UI methods

    ST_LOCAL static int beginRequestHandler(mg_connection* theConnection);

    ST_LOCAL int beginRequest(mg_connection*         theConnection,
                              const mg_request_info& theRequestInfo);

    ST_LOCAL void doStopWebUI();
    ST_LOCAL void doStartWebUI();
    ST_LOCAL void doSwitchWebUI(const int32_t theValue);

        private: //! @name private fields

    StHandle<StGLContext>      myContext;
    StHandle<StSettings>       mySettings;        //!< settings manager for Image Viewer plugin
    StHandle<StTranslations>   myLangMap;         //!< translated strings map
    StHandle<StPlayList>       myPlayList;        //!< play list
    StHandle<StMoviePlayerGUI> myGUI;             //!< GUI root widget
    StHandle<StVideo>          myVideo;           //!< main video playback class
    StHandle<StCheckUpdates>   myUpdates;         //!< check updates utility
    StHandle<StFileNode>       myFileToDelete;    //!< file node for removal

    StCondition                myEventDialog;     //!< event to prevent showing multiple open/save file dialogs
    StCondition                myEventLoaded;     //!< indicate that new file was open
    double                     mySeekOnLoad;      //!< seeking target

    mg_context*                myWebCtx;          //!< web UI context

    int32_t                    myLastUpdateDay;
    bool                       myToRecreateMenu;
    bool                       myToUpdateALList;
    bool                       myIsBenchmark;
    bool                       myToCheckUpdates;

    friend class StMoviePlayerGUI;

};

#endif // __StMoviePlayer_h_
