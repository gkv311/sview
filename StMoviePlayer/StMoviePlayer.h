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

#include <StCore/StDrawerInterface.h>

#include <StSettings/StFloat32Param.h>
#include <StGLStereo/StFormatEnum.h>

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

class ST_LOCAL StALDeviceParam : public StInt32Param {

        private:

    StArrayList<StString> myDevicesList;

        public:

    /**
     * Main constructor.
     */
    StALDeviceParam();

    /**
     * Desctructor.
     */
    ~StALDeviceParam();

    void initList();

    bool init(const StString& theActive);

    /**
     * Returns title for active AL device.
     */
    StString getTitle() const;

    /**
     * Return list of available translations.
     */
    const StArrayList<StString>& getList() const {
        return myDevicesList;
    }

};

/**
 * Base Drawer class for Movie Player plugin.
 */
class ST_LOCAL StMoviePlayer : public StDrawerInterface {

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

        private:

    void parseArguments(const StArgumentsMap& theArguments);

        public: //! @name interface methods' implementations

    StMoviePlayer();
    virtual ~StMoviePlayer();
    virtual StDrawerInterface* getLibImpl() { return this; }
    virtual bool init(StWindowInterface* theWindow);
    virtual bool open(const StOpenInfo& stOpenInfo);
    virtual void parseCallback(StMessage_t* stMessages);
    virtual void stglDraw(unsigned int theView);

        public: //! @name callback Slots

    /**
     * Handler for new file loaded event.
     */
    void doLoaded();

    void doListFirst(const size_t dummy = 0);
    void doListPrev(const size_t dummy = 0);
    void doListNext(const size_t dummy = 0);
    void doListLast(const size_t dummy = 0);

    void doQuit(const size_t dummy = 0);

    void doOpen1File(const size_t dummy = 0);
    void doOpen2Files(const size_t dummy = 0);
    void doAddAudioStream(const size_t dummy = 0);
    void doAddSubtitleStream(const size_t dummy = 0);
    void doSeekLeft(const size_t dummy = 0);
    void doSeekRight(const size_t dummy = 0);
    void doSeek(const int mouseBtn, const double seekX);
    void doPlayPause(const size_t dummy = 0);
    void doStop(const size_t dummy = 0);
    void doReset(const size_t dummy = 0);

    void doSnapshot(const size_t theImgType);

    // callback keys
    void keysStereo(bool* keysMap);
    void keysSrcFormat(bool* keysMap);
    void keysFileWalk(bool* keysMap);
    void keysCommon(bool* keysMap);

        public: //! @name Properties

    bool getCurrentFile(StHandle<StFileNode>&     theFileNode,
                        StHandle<StStereoParams>& theParams,
                        StHandle<StMovieInfo>&    theInfo);

    struct {

        StHandle<StALDeviceParam> alDevice;         //!< active OpenAL device
        StHandle<StFloat32Param>  audioGain;        //!< volume factor
        StHandle<StBoolParam>     isFullscreen;     //!< fullscreen state
        StHandle<StBoolParam>     toRestoreRatio;   //!< restore ratio on restart
        StHandle<StBoolParam>     isShuffle;        //!< shuffle playback order
        StHandle<StInt32Param>    checkUpdatesDays; //!< days count between updates checks
        StHandle<StInt32Param>    srcFormat;        //!< source format
        StHandle<StInt32Param>    audioStream;      //!< active Audio stream
        StHandle<StInt32Param>    subtitlesStream;  //!< active Subtitles stream
        StHandle<StInt32Param>    blockSleeping;    //!< active Audio stream
        StString                  lastFolder;       //!< laster folder used to open / save file
        int                       fpsBound;         //!< limit or not rendering FPS

    } params;

        private: //! @name private callback Slots

    void doSwitchAudioDevice(const int32_t theDevId);
    void doSetAudioVolume(const float theGain);
    void doSwitchShuffle(const bool theShuffleOn);
    void doFullscreen(const bool theIsFullscreen);
    void doSwitchSrcFormat(const int32_t theSrcFormat);
    void doSwitchAudioStream(const int32_t theStreamId);
    void doSwitchSubtitlesStream(const int32_t theStreamId);
    void doUpdateStateLoading();
    void doUpdateStateLoaded();
    friend SV_THREAD_FUNCTION openFileThread(void* theArg);
    void doOpenFileDialog(const size_t theOpenType);

        private: //! @name private fields

    StHandle<StGLContext>      myContext;
    StHandle<StWindow>         myWindow;          //!< wrapper over Output plugin's StWindow instance
    StHandle<StSettings>       mySettings;        //!< settings manager for Image Viewer plugin
    StHandle<StMoviePlayerGUI> myGUI;             //!< GUI root widget
    StHandle<StVideo>          myVideo;           //!< main video playback class
    StHandle<StCheckUpdates>   myUpdates;         //!< check updates utility

    StEvent                    myEventDialog;     //!< event to prevent showing multiple open/save file dialogs
    StEvent                    myEventLoaded;     //!< indicate that new file was open
    double                     mySeekOnLoad;      //!< seeking target

    int32_t                    myLastUpdateDay;
    bool                       myIsBenchmark;
    bool                       myToCheckUpdates;
    bool                       myToQuit;


};

#endif //__StMoviePlayer_h_
