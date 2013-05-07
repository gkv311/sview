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

#include <StSettings/StFloat32Param.h>
#include <StGLStereo/StFormatEnum.h>
#include <StSettings/StTranslations.h>

// forward declarations
class StGLContext;
class StSettings;
class StWindow;
class StVideo;
class StMoviePlayerGUI;
class StCheckUpdates;
class StStereoParams;
class StFileNode;
struct StMovieInfo;

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
    ST_CPPEXPORT virtual void processEvents(const StMessage_t* theEvents);

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

    ST_LOCAL void doListFirst(const size_t dummy = 0);
    ST_LOCAL void doListPrev(const size_t dummy = 0);
    ST_LOCAL void doListNext(const size_t dummy = 0);
    ST_LOCAL void doListLast(const size_t dummy = 0);

    ST_LOCAL void doQuit(const size_t dummy = 0);

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

    // callback keys
    ST_LOCAL void keysStereo(bool* keysMap);
    ST_LOCAL void keysSrcFormat(bool* keysMap);
    ST_LOCAL void keysFileWalk(bool* keysMap);
    ST_LOCAL void keysCommon(bool* keysMap);

        public: //! @name Properties

    ST_LOCAL bool getCurrentFile(StHandle<StFileNode>&     theFileNode,
                                 StHandle<StStereoParams>& theParams,
                                 StHandle<StMovieInfo>&    theInfo);

    ST_LOCAL void getRecentList(StArrayList<StString>& theList);

    struct {

        StHandle<StALDeviceParam> alDevice;         //!< active OpenAL device
        StHandle<StFloat32Param>  audioGain;        //!< volume factor
        StHandle<StBoolParam>     isFullscreen;     //!< fullscreen state
        StHandle<StBoolParam>     toRestoreRatio;   //!< restore ratio on restart
        StHandle<StBoolParam>     isShuffle;        //!< shuffle playback order
        StHandle<StBoolParam>     areGlobalMKeys;   //!< capture global multimedia keys
        StHandle<StInt32Param>    checkUpdatesDays; //!< days count between updates checks
        StHandle<StInt32Param>    srcFormat;        //!< source format
        StHandle<StBoolParam>     ToShowFps;        //!< display FPS meter
        StHandle<StInt32Param>    audioStream;      //!< active Audio stream
        StHandle<StInt32Param>    subtitlesStream;  //!< active Subtitles stream
        StHandle<StInt32Param>    blockSleeping;    //!< active Audio stream
        StString                  lastFolder;       //!< laster folder used to open / save file
        int                       fpsBound;         //!< limit or not rendering FPS

    } params;

        private:

    /**
     * Process device change.
     */
    ST_LOCAL virtual void doChangeDevice(const int32_t theValue);

    /**
     * Initialization routines.
     */
    ST_LOCAL bool init();

    /**
     * Parse arguments.
     */
    ST_LOCAL void parseArguments(const StArgumentsMap& theArguments);

    /**
     * Release GL resources.
     */
    ST_LOCAL void releaseDevice();

        private: //! @name private callback Slots

    ST_LOCAL void doSwitchAudioDevice(const int32_t theDevId);
    ST_LOCAL void doSetAudioVolume(const float theGain);
    ST_LOCAL void doSwitchShuffle(const bool theShuffleOn);
    ST_LOCAL void doFullscreen(const bool theIsFullscreen);
    ST_LOCAL void doSwitchSrcFormat(const int32_t theSrcFormat);
    ST_LOCAL void doSwitchAudioStream(const int32_t theStreamId);
    ST_LOCAL void doSwitchSubtitlesStream(const int32_t theStreamId);
    ST_LOCAL void doUpdateStateLoading();
    ST_LOCAL void doUpdateStateLoaded();
    ST_LOCAL friend SV_THREAD_FUNCTION openFileThread(void* theArg);
    ST_LOCAL void doOpenFileDialog(const size_t theOpenType);

        private: //! @name private fields

    StHandle<StGLContext>      myContext;
    StHandle<StSettings>       mySettings;        //!< settings manager for Image Viewer plugin
    StHandle<StTranslations>   myLangMap;         //!< translated strings map
    StHandle<StMoviePlayerGUI> myGUI;             //!< GUI root widget
    StHandle<StVideo>          myVideo;           //!< main video playback class
    StHandle<StCheckUpdates>   myUpdates;         //!< check updates utility

    StCondition                myEventDialog;     //!< event to prevent showing multiple open/save file dialogs
    StCondition                myEventLoaded;     //!< indicate that new file was open
    double                     mySeekOnLoad;      //!< seeking target

    int32_t                    myLastUpdateDay;
    bool                       myToUpdateALList;
    bool                       myIsBenchmark;
    bool                       myToCheckUpdates;

};

#endif //__StMoviePlayer_h_
