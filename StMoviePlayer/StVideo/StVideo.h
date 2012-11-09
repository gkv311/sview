/**
 * Copyright © 2007-2012 Kirill Gavrilov <kirill@sview.ru>
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
#include <StGL/StPlayList.h>
#include <StImage/StImageFile.h>

// forward declarations
class StSubQueue;
class StLangMap;

struct ST_LOCAL StMovieInfo {

    StHandle<StStereoParams> myId;
    StArgumentsMap           myInfo;

};

/**
 * Special class for video playback.
 */
class ST_LOCAL StVideo {

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

    StPlayList                    myPlayList;     //!< play list
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
    double                        targetFps;

    bool                          isBenchmark;
    volatile StImageFile::ImageType toSave;
    volatile bool                 toQuit;

        private: //! @name auxiliary methods

    /**
     * Just redirect callback slot.
     */
    void doOnErrorRedirect(const StString& theMsgText) {
        signals.onError(theMsgText);
    }

    /**
     * Private method to append one format context (one file).
     */
    bool addFile(const StString& theFileToLoad,
                 StHandle< StArrayList<StString> >& theStreamsListA,
                 StHandle< StArrayList<StString> >& theStreamsListS,
                 double& theMaxDuration);

    bool openSource(const StHandle<StFileNode>&     theNewSource,
                    const StHandle<StStereoParams>& theNewParams);

    /**
     * Close active played file(s).
     */
    void close();

    void packetsLoop();
    void doFlush();
    void doSeek(const double theSeekPts,
                const bool   toSeekBack);
    void doSeekContext(AVFormatContext* theFormatCtx,
                       const double     theSeekPts,
                       const bool       toSeekBack);
    bool doSeekStream (AVFormatContext* theFormatCtx,
                       const signed int theStreamId,
                       const double     theSeekPts,
                       const bool       toSeekBack);
    bool pushPacket(StHandle<StAVPacketQueue>& theAVPacketQueue,
                    StAVPacket& thePacket);

    /**
     * Save current frame to file.
     */
    bool saveSnapshotAs(StImageFile::ImageType theImgType);

    /**
     * @return event (StPlayEvent_t ) - event in wait state.
     */
    StPlayEvent_t popPlayEvent(double& theSeekPts,
                               bool&   toSeekBack) {
        myEventMutex.lock();
            StPlayEvent_t anEventId = myPlayEvent;
            theSeekPts = myPtsSeek;
            toSeekBack = myToSeekBack;
            myPlayEvent = ST_PLAYEVENT_NONE;
        myEventMutex.unlock();
        return anEventId;
    }

    void waitEvent() {
        double aSeekPts;
        bool toSeekBack;
        for(;;) {
            if(popPlayEvent(aSeekPts, toSeekBack) != ST_PLAYEVENT_NONE) {
                return;
            }
            StThread::sleep(10);
        }
    }

        public:

    /**
     * Video thread function that parse input messages
     * and controls decoding threads.
     */
    void mainLoop();

        public: //! @name public methods

    static const char* ST_VIDEOS_MIME_STRING;

    const StMIMEList& getMimeListVideo() const {
        return myMimesVideo;
    }

    const StMIMEList& getMimeListAudio() const {
        return myMimesAudio;
    }

    const StMIMEList& getMimeListSubtitles() const {
        return myMimesSubs;
    }

    /**
     * Main constructor.
     */
    StVideo(const StString&                   theALDeviceName,
            const StHandle<StLangMap>&        theLangMap,
            const StHandle<StGLTextureQueue>& theTextureQueue,
            const StHandle<StSubQueue>&       theSubtitlesQueue);
    ~StVideo();

    /**
     * Ignore sync rules and perform swap when ready.
     */
    void setBenchmark(bool toPerformBenchmark);

    /**
     * Access to the playlist.
     */
    StPlayList& getPlayList() {
        return myPlayList;
    }

    double getAverFps() const {
        return targetFps;
    }

    /**
     * Get default stereoscopic format.
     */
    StFormatEnum getSrcFormat() const {
        return myVideoMaster->getSrcFormat();
    }

    /**
     * Set the stereoscopic format to be used for video
     * with ambiguous format information.
     */
    void setSrcFormat(const StFormatEnum theSrcFormat) {
        myVideoMaster->setSrcFormat(theSrcFormat);
    }

    StHandle<StMovieInfo> getFileInfo(const StHandle<StStereoParams>& theParams) const {
        StHandle<StMovieInfo> anInfo = myFileInfo;
        return (!anInfo.isNull() && anInfo->myId == theParams) ? anInfo : NULL;
    }

        public: //!< callback Slots

    /**
     * Interrupt the playback and load current position in playlist.
     */
    void doLoadNext() {
        pushPlayEvent(ST_PLAYEVENT_NEXT);
    }

    /**
     * Save current displayed frame.
     */
    void doSaveSnapshotAs(const size_t theImgType) {
        toSave = StImageFile::ImageType(theImgType);
        pushPlayEvent(ST_PLAYEVENT_NEXT);
    }

    /**
     * Switch audio device.
     */
    void switchAudioDevice(const StString& theAlDeviceName) {
        myAudio->switchAudioDevice(theAlDeviceName);
    }

    /**
     * Set audio gain.
     */
    void setAudioVolume(const float theGain) {
        myAudio->setAudioVolume(theGain);
    }

        public: //!< Properties

    struct {

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
        StSignal<void (const StString& )> onError;
    } signals;

    bool getPlaybackState(double& theDuration,
                          double& thePts) const {
        myEventMutex.lock();
            theDuration = myDuration;
        myEventMutex.unlock();
        thePts = getPts();
        return isPlaying();
    }

    double getDuration() const {
        myEventMutex.lock();
            double aDuration = myDuration;
        myEventMutex.unlock();
        return aDuration;
    }

    double getPts() const {
        double aPts = myAudio->getPts();
        if(aPts <= 0.0)
            aPts = myVideoMaster->getPts();
        return (aPts > 0.0) ? aPts : 0.0;
    }

    bool isPlaying() const {
        return myVideoMaster->isPlaying() || myAudio->isPlaying();
    }

    virtual void pushPlayEvent(const StPlayEvent_t theEventId,
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

};

#endif //__StVideo_h_
