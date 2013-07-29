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

#ifndef __StVideo_h_
#define __StVideo_h_

#include "StVideoQueue.h"   // video queue class
#include "StAudioQueue.h"   // audio queue class
#include "StSubtitleQueue.h"// subtitles queue class
#include "StVideoTimer.h"   // video refresher class
#include "StParamActiveStream.h"

#include <StFile/StMIMEList.h>
#include <StThreads/StProcess.h>
#include <StThreads/StThread.h>
#include <StGL/StPlayList.h>
#include <StImage/StImageFile.h>

// forward declarations
class StSubQueue;
class StLangMap;

struct StMovieInfo {

    StHandle<StStereoParams> myId;
    StArgumentsMap           myInfo;
    StArgumentsMap           myCodecs;

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

    ST_LOCAL const StMIMEList& getMimeListVideo() const {
        return myMimesVideo;
    }

    ST_LOCAL const StMIMEList& getMimeListAudio() const {
        return myMimesAudio;
    }

    ST_LOCAL const StMIMEList& getMimeListSubtitles() const {
        return myMimesSubs;
    }

    /**
     * Main constructor.
     */
    ST_LOCAL StVideo(const StString&                   theALDeviceName,
                     const StHandle<StLangMap>&        theLangMap,
                     const StHandle<StPlayList>&       thePlayList,
                     const StHandle<StGLTextureQueue>& theTextureQueue,
                     const StHandle<StSubQueue>&       theSubtitlesQueue);
    ST_LOCAL ~StVideo();

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

    /**
     * Access to the playlist.
     */
    ST_LOCAL StPlayList& getPlayList() {
        return *myPlayList;
    }

    ST_LOCAL double getAverFps() const {
        return myTargetFps;
    }

    /**
     * @return true if video stream loaded
     */
    ST_LOCAL bool hasVideoStream() const {
        return myVideoMaster->isInitialized();
    }

    /**
     * Get default stereoscopic format.
     */
    ST_LOCAL StFormatEnum getSrcFormat() const {
        return myVideoMaster->getSrcFormat();
    }

    /**
     * Set the stereoscopic format to be used for video
     * with ambiguous format information.
     */
    ST_LOCAL void setSrcFormat(const StFormatEnum theSrcFormat) {
        myVideoMaster->setSrcFormat(theSrcFormat);
    }

    /**
     * Retrieve information about currently played file.
     */
    ST_LOCAL StHandle<StMovieInfo> getFileInfo(const StHandle<StStereoParams>& theParams) const;

        public: //!< callback Slots

    /**
     * Interrupt the playback and load current position in playlist.
     */
    ST_LOCAL void doLoadNext() {
        pushPlayEvent(ST_PLAYEVENT_NEXT);
    }

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
    ST_LOCAL void switchAudioDevice(const StString& theAlDeviceName) {
        myAudio->switchAudioDevice(theAlDeviceName);
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

        public: //!< Properties

    struct {

        StHandle<StBoolParam>         UseGpu;          //!< use video decoding on GPU when available
        StHandle<StParamActiveStream> activeAudio;     //!< active Audio stream
        StHandle<StParamActiveStream> activeSubtitles; //!< active Subtitles stream

    } params;

        public: //!< Signals

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
        theIsVideoPlayed = myVideoMaster->isPlaying();
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
                          StHandle< StArrayList<StString> >& theStreamsListA,
                          StHandle< StArrayList<StString> >& theStreamsListS,
                          double& theMaxDuration);

    ST_LOCAL bool openSource(const StHandle<StFileNode>&     theNewSource,
                             const StHandle<StStereoParams>& theNewParams);

    /**
     * Close active played file(s).
     */
    ST_LOCAL void close();

    ST_LOCAL void packetsLoop();
    ST_LOCAL void doFlush();
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
    StHandle<StThread>            myThread;      //!< main loop thread
    StHandle<StLangMap>           myLangMap;     //!< translations dictionary

    StArrayList<AVFormatContext*> myCtxList;     //!< format context for each file
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
    StHandle<StStereoParams>      myCurrParams;   //!< paramters for active file node
    StHandle<StGLTextureQueue>    myTextureQueue; //!< decoded frames queue

    StHandle<StVideoTimer>        myVideoTimer;   //!< video refresh timer (Audio -> Video sync)
    mutable StMutex               myEventMutex;   //!< lock for thread-safety
    double                        myDuration;     //!< active file duration in seconds
    double                        myPtsSeek;      //!< seeking target
    bool                          myToSeekBack;   //!< seeking direction
    StPlayEvent_t                 myPlayEvent;    //!< playback event
    double                        myTargetFps;

    bool                          isBenchmark;
    volatile StImageFile::ImageType toSave;
    volatile bool                 toQuit;

};

#endif //__StVideo_h_
