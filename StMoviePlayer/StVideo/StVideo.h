/**
 * Copyright © 2007-2017 Kirill Gavrilov <kirill@sview.ru>
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

#ifndef __StVideo_h_
#define __StVideo_h_

#include "StVideoQueue.h"   // video queue class
#include "StAudioQueue.h"   // audio queue class
#include "StSubtitleQueue.h"// subtitles queue class
#include "StVideoTimer.h"   // video refresher class
#include "StParamActiveStream.h"

#include <StAV/StAVIOFileContext.h>
#include <StFile/StMIMEList.h>
#include <StThreads/StProcess.h>
#include <StThreads/StThread.h>
#include <StGL/StPlayList.h>
#include <StImage/StImageFile.h>
#include <StSettings/StTranslations.h>

// forward declarations
class StSubQueue;

struct StMovieInfo {

    StHandle<StStereoParams> Id;
    StArgumentsMap           Info;
    StArgumentsMap           Codecs;
    StString                 Path;           //!< file path
    StFormat                 StInfoStream;   //!< source format as stored in file metadata
    StFormat                 StInfoFileName; //!< source format detected from file name
    bool                     HasVideo;       //!< true if file contains video
    bool                     IsSavable;      //!< indicate that file can be saved without re-encoding

    StMovieInfo() : StInfoStream(StFormat_AUTO), StInfoFileName(StFormat_AUTO), HasVideo(false), IsSavable(false) {}

};

template<> inline void StArray< StHandle<StFileNode> >::sort() {}
template<> inline void StArray< StHandle<StAVIOContext> >::sort() {}

/**
 * Auxiliary structure.
 */
struct StStreamsInfo {

    StHandle< StArrayList<StString> > AudioList;
    StHandle< StArrayList<StString> > SubtitleList;
    double                            Duration;
    int32_t                           LoadedAudio;
    int32_t                           LoadedSubtitles;

    StStreamsInfo()
    : Duration(0.0),
      LoadedAudio(-1),
      LoadedSubtitles(-1) {
        AudioList    = new StArrayList<StString>(8);
        SubtitleList = new StArrayList<StString>(8);
    }

};

/**
 * Special class for video playback.
 */
class StVideo {

        public:

    /**
     * Video thread function that parse input messages
     * and controls decoding threads.
     */
    ST_LOCAL void mainLoop();

        public: //! @name public methods

    static const char* ST_VIDEOS_MIME_STRING;
    static const char* ST_IMAGES_MIME_STRING;

    ST_LOCAL const StMIMEList& getMimeListVideo() const { return myMimesVideo; }

    ST_LOCAL const StMIMEList& getMimeListAudio() const { return myMimesAudio; }

    ST_LOCAL const StMIMEList& getMimeListSubtitles() const { return myMimesSubs; }

    ST_LOCAL const StMIMEList& getMimeListImages() const { return myMimesImages; }

    /**
     * Main constructor.
     */
    ST_LOCAL StVideo(const std::string&                 theALDeviceName,
                     StAudioQueue::StAlHrtfRequest      theAlHrtf,
                     const StHandle<StResourceManager>& theResMgr,
                     const StHandle<StTranslations>&    theLangMap,
                     const StHandle<StPlayList>&        thePlayList,
                     const StHandle<StGLTextureQueue>&  theTextureQueue,
                     const StHandle<StSubQueue>&        theSubtitlesQueue);

    /**
     * Destructor.
     */
    ST_LOCAL ~StVideo();

    /**
     * Start releasing this class. Should be following by destructor.
     */
    ST_LOCAL void startDestruction();

    ST_LOCAL inline const StHandle<StGLTextureQueue>& getTextureQueue() const {
        return myTextureQueue;
    }

    ST_LOCAL inline const StHandle<StSubQueue>& getSubtitlesQueue() const {
        return mySubtitles->getSubtitlesQueue();
    }

    /**
     * Ignore sync rules and perform swap when ready.
     */
    ST_LOCAL void setBenchmark(bool toPerformBenchmark);

    ST_LOCAL double getAverFps() const {
        return myTargetFps;
    }

    /**
     * @return true if audio stream loaded
     */
    ST_LOCAL bool hasAudioStream() const {
        return myAudio->isInitialized();
    }

    /**
     * @return true if video stream loaded
     */
    ST_LOCAL bool hasVideoStream() const {
        return myVideoMaster->isInitialized();
    }

    /**
     * Set the stereoscopic format to be used for video
     * with ambiguous format information.
     */
    ST_LOCAL void setStereoFormat(const StFormat theSrcFormat) {
        myVideoMaster->setStereoFormatByUser(theSrcFormat);
    }

    /**
     * Set theater mode.
     */
    ST_LOCAL void setTheaterMode(bool theIsTheater) {
        myVideoMaster->setTheaterMode(theIsTheater);
    }

    /**
     * Stick to panorama 360 mode.
     */
    ST_LOCAL void setStickPano360(bool theToStick) {
        myVideoMaster->setStickPano360(theToStick);
    }

    /**
     * Set if JPS file should be read as Left/Right (TRUE) of as Right/Left (FALSE).
     */
    ST_LOCAL void setSwapJPS(bool theToSwap) { myVideoMaster->setSwapJPS(theToSwap); }

    /**
     * Retrieve information about currently played file.
     */
    ST_LOCAL StHandle<StMovieInfo> getFileInfo(const StHandle<StStereoParams>& theParams) const;

        public: //! @name callback Slots

    /**
     * Interrupt the playback and load current position in playlist.
     */
    ST_LOCAL void doLoadNext() {
        pushPlayEvent(ST_PLAYEVENT_NEXT);
    }

    /**
     * Add file node for removal.
     */
    ST_LOCAL void doRemovePhysically(const StHandle<StFileNode>& theFile);

    /**
     * Save current displayed frame.
     */
    ST_LOCAL void doSaveSnapshotAs(const size_t theImgType) {
        toSave = StImageFile::ImageType(theImgType);
        pushPlayEvent(ST_PLAYEVENT_NEXT);
    }

    /**
     * Switch audio device.
     */
    ST_LOCAL void switchAudioDevice(const std::string& theAlDeviceName) {
        myAudio->switchAudioDevice(theAlDeviceName);
    }

    /**
     * Return TRUE if OpenAL implementation provides HRTF mixing feature.
     */
    ST_LOCAL bool hasAlHrtf() const {
        return myAudio->hasAlHrtf();
    }

    /**
     * Setup OpenAL HRTF mixing.
     */
    ST_LOCAL void setAlHrtfRequest(StAudioQueue::StAlHrtfRequest theAlHrt) {
        myAudio->setAlHrtfRequest(theAlHrt);
    }

    /**
     * @return true if device was disconnected and OpenAL should be re-initialized
     */
    ST_LOCAL bool isDisconnected() const {
        return myAudio->isDisconnected();
    }

    /**
     * Set audio gain.
     */
    ST_LOCAL void setAudioVolume(const float theGain) {
        myAudio->setAudioVolume(theGain);
    }

    /**
     * Set head orientation.
     */
    ST_LOCAL void setHeadOrientation(const StGLQuaternion& theOrient, bool theToTrack) {
        myAudio->setHeadOrientation(theOrient, theToTrack);
    }

    /**
     * Set forcing B-Format.
     */
    ST_LOCAL void setForceBFormat(bool theToForce) {
        myAudio->setForceBFormat(theToForce);
    }

    ST_LOCAL void setAudioDelay(const float theDelaySec);

    /**
     * Return OpenAL info.
     */
    ST_LOCAL void getAlInfo(StDictionary& theInfo) {
        myAudio->getAlInfo(theInfo);
    }

        public: //! @name Properties

    struct {

        StHandle<StBoolParam>         UseGpu;          //!< use video decoding on GPU when available
        StHandle<StBoolParam>         UseOpenJpeg;     //!< use OpenJPEG (libopenjpeg) instead of built-in jpeg2000 decoder
        StHandle<StBoolParam>         ToSearchSubs;    //!< automatically search for additional subtitles/audio track files nearby video file
        StHandle<StBoolParamNamed>    ToTrackHeadAudio;//!< enable/disable head-tracking for audio listener
        StHandle<StFloat32Param>      SlideShowDelay;  //!< slideshow delay
        StHandle<StParamActiveStream> activeAudio;     //!< active Audio stream
        StHandle<StParamActiveStream> activeSubtitles; //!< active Subtitles stream

    } params;

        public: //! @name Signals

    /**
     * All callback handlers should be thread-safe.
     */
    struct {
        /**
         * Emit callback Slot on file load.
         */
        StSignal<void ()> onLoaded;

        /**
         * Emit callback Slot on error.
         * @param theUserData (const StString& ) - error description.
         */
        StSignal<void (const StCString& )> onError;
    } signals;

    ST_LOCAL bool getPlaybackState(double& theDuration,
                                   double& thePts,
                                   bool&   theIsVideoPlayed,
                                   bool&   theIsAudioPlayed) const {
        myEventMutex.lock();
            theDuration = myDuration;
        myEventMutex.unlock();
        thePts = getPts();
        theIsVideoPlayed = myVideoMaster->isPlaying()
                       && !myVideoMaster->isAttachedPicture();
        theIsAudioPlayed = myAudio->isPlaying();
        return theIsVideoPlayed || theIsAudioPlayed;
    }

    ST_LOCAL double getDuration() const {
        myEventMutex.lock();
            double aDuration = myDuration;
        myEventMutex.unlock();
        return aDuration;
    }

    ST_LOCAL double getPts() const {
        double aPts = myAudio->getPts();
        if(aPts <= 0.0)
            aPts = myVideoMaster->getPts();
        return (aPts > 0.0) ? aPts : 0.0;
    }

    ST_LOCAL bool isPlaying() const {
        return myVideoMaster->isPlaying() || myAudio->isPlaying();
    }

    ST_LOCAL void pushPlayEvent(const StPlayEvent_t theEventId,
                                const double        theSeekParam = 0.0) {
        if(theEventId == ST_PLAYEVENT_NEXT) {
            myEventMutex.lock();
                myPlayEvent = theEventId;
            myEventMutex.unlock();
            return;
        }
        double aPrevPts = getPts();
        myVideoMaster->pushPlayEvent(theEventId, theSeekParam);
        myAudio->pushPlayEvent(theEventId, theSeekParam);
        if(theEventId == ST_PLAYEVENT_SEEK) {
            myEventMutex.lock();
                myPlayEvent  = theEventId;
                myPtsSeek    = theSeekParam;
                myToSeekBack = myPtsSeek < aPrevPts;
            myEventMutex.unlock();
        }
    }

        private: //! @name auxiliary methods

    ST_LOCAL const StString& tr(const size_t theId) const {
        return myLangMap->getValue(theId);
    }

    /**
     * Just redirect callback slot.
     */
    ST_LOCAL void doOnErrorRedirect(const StCString& theMsgText) {
        signals.onError(theMsgText);
    }

    /**
     * Private method to append one format context (one file).
     */
    ST_LOCAL bool addFile(const StString& theFileToLoad,
                          const StHandle<StStereoParams>& theNewParams,
                          StStreamsInfo&  theInfo);

    ST_LOCAL bool openSource(const StHandle<StFileNode>&     theNewSource,
                             const StHandle<StStereoParams>& theNewParams,
                             const StHandle<StFileNode>&     theNewPlsFile);

    /**
     * Close active played file(s).
     */
    ST_LOCAL void close();

    /**
     * Dispatch packets from format contexts to decoding queues.
     */
    ST_LOCAL void packetsLoop();

    /**
     * Clear packets queue and push Flush event to decoders.
     */
    ST_LOCAL void doFlush();

    /**
     * Clear packets queue.
     * Does not flush decoder for attached picture.
     */
    ST_LOCAL void doFlushSoft();
    ST_LOCAL void doSeek(const double theSeekPts,
                         const bool   toSeekBack);
    ST_LOCAL void doSeekContext(AVFormatContext* theFormatCtx,
                                const double     theSeekPts,
                                const bool       toSeekBack);
    ST_LOCAL bool doSeekStream (AVFormatContext* theFormatCtx,
                                const signed int theStreamId,
                                const double     theSeekPts,
                                const bool       toSeekBack);
    ST_LOCAL bool pushPacket(StHandle<StAVPacketQueue>& theAVPacketQueue,
                             StAVPacket& thePacket);

    /**
     * Re-initialize video streams if needed (source format change, GPU decoding).
     */
    ST_LOCAL void checkInitVideoStreams();

    /**
     * Save current frame to file.
     */
    ST_LOCAL bool saveSnapshotAs(StImageFile::ImageType theImgType);

    /**
     * @return event (StPlayEvent_t ) - event in wait state.
     */
    ST_LOCAL StPlayEvent_t popPlayEvent(double& theSeekPts,
                                        bool&   toSeekBack) {
        myEventMutex.lock();
            StPlayEvent_t anEventId = myPlayEvent;
            theSeekPts = myPtsSeek;
            toSeekBack = myToSeekBack;
            myPlayEvent = ST_PLAYEVENT_NONE;
        myEventMutex.unlock();
        return anEventId;
    }

    ST_LOCAL void waitEvent() {
        double aSeekPts;
        bool toSeekBack;
        for(;;) {
            if(popPlayEvent(aSeekPts, toSeekBack) != ST_PLAYEVENT_NONE) {
                return;
            }
            StThread::sleep(10);
        }
    }

        private: //! @name private fields

    StMIMEList                    myMimesVideo;
    StMIMEList                    myMimesAudio;
    StMIMEList                    myMimesSubs;
    StMIMEList                    myMimesImages;
    StHandle<StThread>            myThread;      //!< main loop thread
    StHandle<StResourceManager>   myResMgr;      //!< resource manager
    StHandle<StTranslations>      myLangMap;     //!< translations dictionary

    StArrayList<StString>         myFileList;    //!< file list
    StArrayList<AVFormatContext*> myCtxList;     //!< format context for each file
    StArrayList< StHandle<StAVIOContext> >
                                  myFileIOList;  //!< associated IO context
    StArrayList<AVFormatContext*> myPlayCtxList; //!< currently played contexts

    StHandle<StVideoQueue>        myVideoMaster;  //!< Master video decoding thread
    StHandle<StVideoQueue>        myVideoSlave;   //!< Slave  video decoding thread
    StHandle<StAudioQueue>        myAudio;        //!< audio decoding thread
    StHandle<StSubtitleQueue>     mySubtitles;    //!< subtitles decoding thread
    AVFormatContext*              mySlaveCtx;     //!< Slave video format context
    signed int                    mySlaveStream;  //!< Slave video stream id

    StHandle<StPlayList>          myPlayList;     //!< play list
    StHandle<StMovieInfo>         myFileInfo;     //!< info about currently loaded file
    StHandle<StMovieInfo>         myFileInfoTmp;
    StHandle<StFileNode>          myCurrNode;     //!< active (played) file node
    StHandle<StStereoParams>      myCurrParams;   //!< parameters for active file node
    StHandle<StFileNode>          myCurrPlsFile;  //!< active playlist file node
    StHandle<StGLTextureQueue>    myTextureQueue; //!< decoded frames queue

    StArrayList<StString>         myTracksExt;    //!< extra tracks extensions list
    StFolder                      myTracksFolder; //!< cached list of subtitles/audio tracks in the current folder
    StArrayList< StHandle<StFileNode> >
                                  myFilesToDelete;//!< file nodes for removal

    StHandle<StVideoTimer>        myVideoTimer;   //!< video refresh timer (Audio -> Video sync)
    mutable StMutex               myEventMutex;   //!< lock for thread-safety
    double                        myDuration;     //!< active file duration in seconds
    double                        myPtsSeek;      //!< seeking target
    bool                          myToSeekBack;   //!< seeking direction
    StPlayEvent_t                 myPlayEvent;    //!< playback event
    double                        myTargetFps;
    volatile int                  myAudioDelayMSec;//!< audio/video sync delay
    volatile bool                 myIsBenchmark;
    volatile StImageFile::ImageType toSave;
    volatile bool                 toQuit;         //!< flag indicating that all working threads should be closed
    StCondition                   myQuitEvent;    //!< condition indicating that working thread has saved playback state to playlist

};

#endif // __StVideo_h_
