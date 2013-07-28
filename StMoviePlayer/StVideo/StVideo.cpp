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

#include "StVideo.h"
#include "../StMoviePlayerInfo.h"
#include "../StMoviePlayerStrings.h"

#include <StStrings/StFormatTime.h>
#include <StStrings/StLangMap.h>

namespace {
    static const char ST_AUDIOS_MIME_STRING[] = ST_VIDEO_PLUGIN_AUDIO_MIME_CHAR;
    static const char ST_SUBTIT_MIME_STRING[] = ST_VIDEO_PLUGIN_SUBTIT_MIME_CHAR;

    static SV_THREAD_FUNCTION threadFunction(void* theStVideo) {
        StVideo* aStVideo  = (StVideo* )theStVideo;
        aStVideo->mainLoop();
        return SV_THREAD_RETURN 0;
    }
};

const char* StVideo::ST_VIDEOS_MIME_STRING = ST_VIDEO_PLUGIN_MIME_CHAR;

StVideo::StVideo(const StString&                   theALDeviceName,
                 const StHandle<StLangMap>&        theLangMap,
                 const StHandle<StPlayList>&       thePlayList,
                 const StHandle<StGLTextureQueue>& theTextureQueue,
                 const StHandle<StSubQueue>&       theSubtitlesQueue)
: myMimesVideo(ST_VIDEOS_MIME_STRING),
  myMimesAudio(ST_AUDIOS_MIME_STRING),
  myMimesSubs(ST_SUBTIT_MIME_STRING),
  myLangMap(theLangMap),
  mySlaveCtx(NULL),
  mySlaveStream(-1),
  myPlayList(thePlayList),
  myTextureQueue(theTextureQueue),
  myDuration(0.0),
  myPtsSeek(0.0),
  myToSeekBack(false),
  myPlayEvent(ST_PLAYEVENT_NONE),
  targetFps(0.0),
  //
  isBenchmark(false),
  toSave(StImageFile::ST_TYPE_NONE),
  toQuit(false) {
    // initialize FFmpeg library if not yet performed
    stAV::init();

    myPlayList->setExtensions(myMimesVideo.getExtensionsList());

    params.UseGpu          = new StBoolParam(false);
    params.activeAudio     = new StParamActiveStream();
    params.activeSubtitles = new StParamActiveStream();

    myVideoMaster = new StVideoQueue(myTextureQueue);
    myVideoMaster->signals.onError.connect(this, &StVideo::doOnErrorRedirect);

    myVideoSlave  = new StVideoQueue(myTextureQueue, myVideoMaster);
    myVideoSlave->signals.onError.connect(this, &StVideo::doOnErrorRedirect);

    myAudio = new StAudioQueue(theALDeviceName);
    myAudio->signals.onError.connect(this, &StVideo::doOnErrorRedirect);

    mySubtitles = new StSubtitleQueue(theSubtitlesQueue);
    mySubtitles->signals.onError.connect(this, &StVideo::doOnErrorRedirect);

    // launch working thread
    myThread = new StThread(threadFunction, (void* )this);
}

#include <stAssert.h>
class ST_LOCAL StHangKiller {

        private:

    StHandle<StThread> myThread;
    StHandle<StString> myState;
    const double       myLimitSec;
    StCondition        myDoneEvent;

        private:

    void waitLoop() {
        StTimer aTimer(true);
        StString aStr = "Waiting timeout, state: ";
        for(;;) {
            ///ST_DEBUG_LOG("...waiting " + aTimer.getElapsedTimeInSec() + " seconds"); ///
            if(myDoneEvent.wait(1000)) {
                return;
            }
            StHandle<StString> aState = myState;
            ST_ASSERT_SLIP(aTimer.getElapsedTimeInSec() < myLimitSec,
                           aStr + (aState.isNull() ? StString() : *aState),
                           exit(-1));
        }
    }

    static SV_THREAD_FUNCTION threadWatcher(void* theWatcher) {
        StHangKiller* aWatcher = (StHangKiller* )theWatcher;
        aWatcher->waitLoop();
        return SV_THREAD_RETURN 0;
    }

        public:

    StHangKiller(const double& theLimitSec)
    : myState(new StString("initial")),
      myLimitSec(theLimitSec),
      myDoneEvent(false) {
        myThread = new StThread(threadWatcher, this);
    }

    ~StHangKiller() {
        myDoneEvent.set();
        myThread->wait();
        myThread.nullify();
    }

    void setState(const StString& theState) {
        setState(new StString(theState));
    }

    void setState(const StHandle<StString>& theState) {
        myState = theState;
    }

    void setDone() {
        myDoneEvent.set();
        myState = new StString("done");
    }

        private:

    // no copies, please
    StHangKiller(const StHangKiller& theCopy);
    const StHangKiller& operator=(const StHangKiller& theCopy);

};

StVideo::~StVideo() {
    // stop the thread
    toQuit = true;
    toSave = StImageFile::ST_TYPE_NONE;
    pushPlayEvent(ST_PLAYEVENT_NEXT);
    myTextureQueue->clear();

    // wait main thread is quit
    StHangKiller aHangKiller(10.0);
    aHangKiller.setState("waiting for StVideo::mainLoop()");
    myThread->wait();
    myThread.nullify();
    myVideoTimer.nullify();

    // close all decoding threads
    aHangKiller.setState("waiting for StSubtitleQueue thread");
    mySubtitles.nullify();
    aHangKiller.setState("waiting for StAudioQueue thread");
    myAudio.nullify();
    aHangKiller.setState("waiting for StVideoQueue (slave) thread");
    myVideoSlave.nullify();
    aHangKiller.setState("waiting for StVideoQueue (master) thread");
    myVideoMaster.nullify();
    aHangKiller.setDone();
    close(); // we must quit or flush video/audio threads before close()!
}

void StVideo::close() {
    if(!myVideoSlave.isNull())  myVideoSlave->deinit();
    if(!myVideoMaster.isNull()) myVideoMaster->deinit();
    if(!myAudio.isNull())       myAudio->deinit();
    if(!mySubtitles.isNull())   mySubtitles->deinit();
    for(size_t ctxId = 0; ctxId < myCtxList.size(); ++ctxId) {
        AVFormatContext*& formatCtx = myCtxList[ctxId];
        if(formatCtx != NULL) {
        #if(LIBAVFORMAT_VERSION_INT >= AV_VERSION_INT(53, 17, 0))
            avformat_close_input(&formatCtx);
        #else
            av_close_input_file(formatCtx); // close video file at all
        #endif
        }
    }
    myCtxList.clear();
    myPlayCtxList.clear();
    mySlaveCtx    = NULL;
    mySlaveStream = -1;

    params.activeAudio->clearList();
    params.activeSubtitles->clearList();
    myCurrNode.nullify();
    myCurrParams.nullify();

    myEventMutex.lock();
        myDuration = 0.0;
    myEventMutex.unlock();
}

void StVideo::setBenchmark(bool toPerformBenchmark) {
    isBenchmark = toPerformBenchmark;
    if(!myVideoTimer.isNull()) {
        myVideoTimer->setBenchmark(isBenchmark);
    }
}

bool StVideo::addFile(const StString& theFileToLoad,
                      StHandle< StArrayList<StString> >& theStreamsListA,
                      StHandle< StArrayList<StString> >& theStreamsListS,
                      double& theMaxDuration) {
    StString aFileName, aDummy;
    StFileNode::getFolderAndFile(theFileToLoad, aDummy, aFileName);

    // open video file
    AVFormatContext* aFormatCtx = NULL;
#if(LIBAVFORMAT_VERSION_INT >= AV_VERSION_INT(53, 2, 0))
    int avErrCode = avformat_open_input(&aFormatCtx, theFileToLoad.toCString(), NULL, NULL);
#else
    int avErrCode = av_open_input_file (&aFormatCtx, theFileToLoad.toCString(), NULL, 0, NULL);
#endif
    if(avErrCode != 0) {
        signals.onError(StString("FFmpeg: Couldn't open video file '") + theFileToLoad
                      + "'\nError: " + stAV::getAVErrorDescription(avErrCode));
        if(aFormatCtx != NULL) {
        #if(LIBAVFORMAT_VERSION_INT >= AV_VERSION_INT(53, 17, 0))
            avformat_close_input(&aFormatCtx);
        #else
            av_close_input_file(aFormatCtx); // close video file
        #endif
        }
        return false;
    }

    // retrieve stream information
#if(LIBAVFORMAT_VERSION_INT >= AV_VERSION_INT(53, 6, 0))
    if(avformat_find_stream_info(aFormatCtx, NULL) < 0) {
#else
    if(av_find_stream_info(aFormatCtx) < 0) {
#endif
        signals.onError(StString("FFmpeg: Couldn't find stream information in '") + theFileToLoad + "'");
        if(aFormatCtx != NULL) {
        #if(LIBAVFORMAT_VERSION_INT >= AV_VERSION_INT(53, 17, 0))
            avformat_close_input(&aFormatCtx);
        #else
            av_close_input_file(aFormatCtx); // close video file at all
        #endif
        }
        return false;
    }

#ifdef __ST_DEBUG__
#if(LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(52, 101, 0))
    av_dump_format(aFormatCtx, 0, theFileToLoad.toCString(), false);
#else
    dump_format   (aFormatCtx, 0, theFileToLoad.toCString(), false);
#endif
#endif

    StString aTitleString, aFolder;
    StFileNode::getFolderAndFile(theFileToLoad, aFolder, aTitleString);
    myFileInfoTmp->myInfo.add(StArgument("File name", aTitleString));

    // collect metadata
    for(stAV::meta::Tag* aTag = stAV::meta::findTag(aFormatCtx->metadata, "", NULL, stAV::meta::SEARCH_IGNORE_SUFFIX);
        aTag != NULL;
        aTag = stAV::meta::findTag(aFormatCtx->metadata, "", aTag, stAV::meta::SEARCH_IGNORE_SUFFIX)) {
        myFileInfoTmp->myInfo.add(StArgument(aTag->key, aTag->value));
    }

    theMaxDuration = stMax(theMaxDuration, stAV::unitsToSeconds(aFormatCtx->duration));
    for(unsigned int aStreamId = 0; aStreamId < aFormatCtx->nb_streams; ++aStreamId) {
        AVStream* aStream = aFormatCtx->streams[aStreamId];
        theMaxDuration = stMax(theMaxDuration, stAV::unitsToSeconds(aStream, aStream->duration));

        if(aStream->codec->codec_type == AVMEDIA_TYPE_VIDEO) {
            // video track
            if(!myVideoMaster->isInitialized()) {
                myVideoMaster->init(aFormatCtx, aStreamId);
                myVideoMaster->setSlave(NULL);

                if(myVideoMaster->isInitialized()) {
                    const int aSizeX      = myVideoMaster->sizeX();
                    const int aSizeY      = myVideoMaster->sizeY();
                    StString aDimsStr = StString() + aSizeX + " x " + aSizeY;
                    const int aCodedSizeX = myVideoMaster->getCodedSizeX();
                    const int aCodedSizeY = myVideoMaster->getCodedSizeY();
                    if(aCodedSizeX != aSizeX
                    || aCodedSizeY != aSizeY) {
                        aDimsStr += StString(" (") + aCodedSizeX + " x " + aCodedSizeY + ")";
                    }
                    myFileInfoTmp->myInfo.add(StArgument("Video Dimensions", aDimsStr));
                    myFileInfoTmp->myInfo.add(StArgument("Pixel Format",
                        myVideoMaster->getPixelFormatString()));
                    myFileInfoTmp->myInfo.add(StArgument("Pixel Ratio",
                        StString() + myVideoMaster->getPixelRatio()));
                    myFileInfoTmp->myInfo.add(StArgument("Duration",
                        StFormatTime::formatSeconds(theMaxDuration)));
                }
            } else if(!myVideoSlave->isInitialized()) {
                myVideoSlave->init(aFormatCtx, aStreamId);
                if(myVideoSlave->isInitialized()) {
                    mySlaveCtx    = aFormatCtx;
                    mySlaveStream = aStreamId;

                    const int aSizeX      = myVideoSlave->sizeX();
                    const int aSizeY      = myVideoSlave->sizeY();
                    StString aDimsStr = StString() + aSizeX + " x " + aSizeY;
                    const int aCodedSizeX = myVideoSlave->getCodedSizeX();
                    const int aCodedSizeY = myVideoSlave->getCodedSizeY();
                    if(aCodedSizeX != aSizeX
                    || aCodedSizeY != aSizeY) {
                        aDimsStr += StString(" (") + aCodedSizeX + " x " + aCodedSizeY + ")";
                    }
                    myFileInfoTmp->myInfo.add(StArgument("Video Dimensions (slave)", aDimsStr));

                    if(myVideoMaster->getSrcFormat() == ST_V_SRC_AUTODETECT) {
                        myVideoMaster->setSlave(myVideoSlave);
                    } else {
                        myVideoSlave->deinit();
                    }
                }
            }
        } else if(aStream->codec->codec_type == AVMEDIA_TYPE_AUDIO) {
            // audio track
            AVCodecContext* aCodecCtx = aStream->codec;
            StString aCodecName;
            AVCodec* aCodec = avcodec_find_decoder(aCodecCtx->codec_id);
            if(aCodec != NULL) {
                aCodecName = aCodec->name;
            }

            StString aSampleFormat  = stAV::audio::getSampleFormatString (aCodecCtx);
            StString aSampleRate    = stAV::audio::getSampleRateString   (aCodecCtx);
            StString aChannelLayout = stAV::audio::getChannelLayoutString(aCodecCtx);

            StString aLanguage;
        #if(LIBAVUTIL_VERSION_INT >= AV_VERSION_INT(51, 5, 0))
            stAV::meta::readTag(aStream, "language", aLanguage);
        #else
            aLanguage = aStream->language;
        #endif

            if(aFormatCtx->nb_streams == 1) {
                aLanguage = (aFileName.getLength() > 24) ? (StString("...") + aFileName.subString(aFileName.getLength() - 24, aFileName.getLength())) : aFileName;
            }
            theStreamsListA->add(aCodecName
                               + (!aSampleRate.isEmpty() ? StString(", ") : StString()) + aSampleRate
                               + ", " + aChannelLayout
                               + (!aSampleFormat.isEmpty() ? StString(", ") : StString()) + aSampleFormat
                               + (!aLanguage.isEmpty() ? (StString(" (") + aLanguage + ')') : StString()));

            if(!myAudio->isInitialized()) {
                myAudio->init(aFormatCtx, aStreamId);
            }
        } else if(aStream->codec->codec_type == AVMEDIA_TYPE_SUBTITLE) {
            // subtitles track
            AVCodecContext* aCodecCtx = aStream->codec;
            StString aCodecName("PLAIN");
            AVCodec* aCodec = avcodec_find_decoder(aCodecCtx->codec_id);
            if(aCodec != NULL) {
                aCodecName = aCodec->name;
            }

            StString aStreamTitle;
            if(aFormatCtx->nb_streams == 1) {
                aStreamTitle = aCodecName + ", " + aFileName;
            } else {
                StString aLanguage;
            #if(LIBAVUTIL_VERSION_INT >= AV_VERSION_INT(51, 5, 0))
                stAV::meta::readTag(aStream, "language", aLanguage);
            #else
                aLanguage = aStream->language;
            #endif
                aStreamTitle = aCodecName + (!aLanguage.isEmpty() ? (StString(" (") + aLanguage + ')') : StString());
            }
            theStreamsListS->add(aStreamTitle);
        }
    }

    myCtxList.add(aFormatCtx);
    return true;
}

bool StVideo::openSource(const StHandle<StFileNode>&     theNewSource,
                         const StHandle<StStereoParams>& theNewParams) {
    // just for safe - close previously opened video
    close();

    const bool toUseGpu = params.UseGpu->getValue();
    myVideoMaster->setUseGpu(toUseGpu);
    myVideoSlave ->setUseGpu(toUseGpu);

    myFileInfoTmp = new StMovieInfo();

    double aDuration = 0.0;
    StHandle< StArrayList<StString> > aStreamsListA = new StArrayList<StString>(8);
    StHandle< StArrayList<StString> > aStreamsListS = new StArrayList<StString>(8);
    if(!theNewSource->isEmpty()) {
        bool isLoaded = false;
        for(size_t aNode = 0; aNode < theNewSource->size(); ++aNode) {
            isLoaded = addFile(theNewSource->getValue(aNode)->getPath(),
                               aStreamsListA, aStreamsListS, aDuration) || isLoaded;
        }
        if(!isLoaded) {
            return false;
        }
    } else {
        if(!addFile(theNewSource->getPath(),
                    aStreamsListA, aStreamsListS, aDuration)) {
            return false;
        }
    }

    if(!myVideoMaster->isInitialized() && !myAudio->isInitialized()) {
        signals.onError(stCString("FFmpeg: Didn't found any video or audio streams"));
        return false;
    }

    StArgument aTitle = myFileInfoTmp->myInfo["TITLE"];
    if(!aTitle.isValid()) {
        aTitle = myFileInfoTmp->myInfo["title"];
    }
    StArgument anArtist = myFileInfoTmp->myInfo["ARTIST"];
    if(!anArtist.isValid()) {
        anArtist = myFileInfoTmp->myInfo["artist"];
    }

    if(anArtist.isValid() && aTitle.isValid()) {
        myPlayList->setTitle(theNewParams, anArtist.getValue() + " - " + aTitle.getValue());
    } else if(aTitle.isValid()) {
        if(aTitle.getValue().getLength() < 20) {
            // protection against messed title
            StString aFileName;
            StString aFolder;
            StString aPath;
            if(!theNewSource->isEmpty()) {
                aPath = theNewSource->getValue(0)->getPath();
            } else {
                aPath = theNewSource->getPath();
            }
            StFileNode::getFolderAndFile(aPath, aFolder, aFileName);
            myPlayList->setTitle(theNewParams, aTitle.getValue() + " (" + aFileName + ")");
        } else {
            myPlayList->setTitle(theNewParams, aTitle.getValue());
        }
    }

    myCurrNode   = theNewSource;
    myCurrParams = theNewParams;
    myFileInfoTmp->myId = myCurrParams;

    params.activeAudio->setList(aStreamsListA, aStreamsListA->isEmpty() ? -1 : 0);
    params.activeSubtitles->setList(aStreamsListS, -1); // do not show subtitles by default

    myEventMutex.lock();
        myDuration = aDuration;
        myFileInfo = myFileInfoTmp;
    myEventMutex.unlock();

    return true;
}

void StVideo::doFlush() {
    // clear packet queues from obsolete data
    mySubtitles->clear();
    myAudio->clear();
    myVideoMaster->clear();
    myVideoSlave->clear();

    // push FLUSH packet to queues so they must flush FFmpeg codec buffers
    if(myVideoMaster->isInitialized()) myVideoMaster->pushFlush();
    if(myVideoSlave->isInitialized())  myVideoSlave->pushFlush();
    if(myAudio->isInitialized())       myAudio->pushFlush();
    if(mySubtitles->isInitialized())   mySubtitles->pushFlush();
}

void StVideo::doSeek(const double theSeekPts,
                     const bool   toSeekBack) {
    for(size_t ctxId = 0; ctxId < myPlayCtxList.size(); ++ctxId) {
        doSeekContext(myPlayCtxList[ctxId], theSeekPts, toSeekBack);
    }

    // clear packet queues from obsolete data
    doFlush();
}

void StVideo::doSeekContext(AVFormatContext* theFormatCtx,
                            const double     theSeekPts,
                            const bool       toSeekBack) {
    // try seek the Video stream first to got key frame
    bool isSeekDone = false;
    if(myVideoMaster->isInContext(theFormatCtx)) {
        isSeekDone = doSeekStream(theFormatCtx, myVideoMaster->getId(), theSeekPts, toSeekBack);
    } else if(myVideoSlave->isInContext(theFormatCtx)) {
        isSeekDone = doSeekStream(theFormatCtx, myVideoSlave->getId(), theSeekPts, toSeekBack);
    }
    if(!isSeekDone && myAudio->isInContext(theFormatCtx)) {
        // if no video stream or seeking was failed - try to seek Audio stream
        isSeekDone = doSeekStream(theFormatCtx, myAudio->getId(), theSeekPts, toSeekBack);
    }
    if(!isSeekDone) {
        // at last - try to seek the format context itself...
        int64_t aSeekTarget = stAV::secondsToUnits(theSeekPts);
        isSeekDone = av_seek_frame(theFormatCtx, -1, aSeekTarget, 0) >= 0;
        if(!isSeekDone) {
            ST_DEBUG_LOG("Seeking disaster!");
        }
    }
}

bool StVideo::doSeekStream(AVFormatContext* theFormatCtx,
                           const signed int theStreamId,
                           const double     theSeekPts,
                           const bool       toSeekBack) {
    AVStream* aStream = theFormatCtx->streams[theStreamId];
    int64_t aSeekTarget = stAV::secondsToUnits(aStream, theSeekPts + stAV::unitsToSeconds(aStream, aStream->start_time));

    bool isSeekDone = av_seek_frame(theFormatCtx, theStreamId, aSeekTarget, 0) >= 0;

    // try 10 more times in backward direction to work-around huge duration between key frames
    // will not work for some streams with undefined cur_dts (AV_NOPTS_VALUE)!!!
    for(int aTries = 10; isSeekDone && toSeekBack && aTries > 0 && (aStream->cur_dts > aSeekTarget); --aTries) {
        aSeekTarget -= stAV::secondsToUnits(aStream, 1.0);
        isSeekDone = av_seek_frame(theFormatCtx, theStreamId, aSeekTarget, 0) >= 0;
    }

    if(!isSeekDone) {
        ST_DEBUG_LOG("Error while seeking"
                   + (aStream->codec->codec_type == AVMEDIA_TYPE_VIDEO ? " Video"
                   : (aStream->codec->codec_type == AVMEDIA_TYPE_AUDIO ? " Audio" : " "))
                   +  "stream to " + theSeekPts + "sec(" + (theSeekPts + stAV::unitsToSeconds(aStream, aStream->start_time)) + "sec)");
    }
    return isSeekDone;
}

bool StVideo::pushPacket(StHandle<StAVPacketQueue>& theAVPacketQueue,
                         StAVPacket& thePacket) {
    if(theAVPacketQueue->isFull()) {
        return false;
    }
    thePacket.setDurationSeconds(theAVPacketQueue->unitsToSeconds(thePacket.getDuration()));
    theAVPacketQueue->push(thePacket);
    return true;
}

void StVideo::checkInitVideoStreams() {
    const bool toUseGpu      = params.UseGpu->getValue();
    const bool toDecodeSlave = myVideoMaster->getSrcFormat() == ST_V_SRC_AUTODETECT
                            && mySlaveStream >= 0;
    if(toUseGpu      != myVideoMaster->toUseGpu()
    || toDecodeSlave != myVideoSlave->isInitialized()) {
        myVideoMaster->setUseGpu(toUseGpu);
        myVideoSlave ->setUseGpu(toUseGpu);
        doFlush();
        if(myVideoMaster->isInitialized()) {
            AVFormatContext* aCtxMaster      = myVideoMaster->getContext();
            const signed int aStreamIdMaster = myVideoMaster->getId();
            myVideoMaster->pushEnd();
            if(myVideoSlave->isInitialized()) {
                myVideoSlave->pushEnd();
            }
            while(!myVideoMaster->isEmpty() || !myVideoMaster->isInDowntime()
               || !myVideoSlave->isEmpty()  || !myVideoSlave->isInDowntime()) {
                StThread::sleep(10);
            }
            myVideoMaster->deinit();
            if(myVideoSlave->isInitialized()) {
                myVideoSlave->deinit();
            }
            myVideoMaster->init(aCtxMaster, aStreamIdMaster);
            myVideoMaster->setSlave(NULL);
            if(toDecodeSlave) {
                myVideoSlave->init(mySlaveCtx, mySlaveStream);
                myVideoMaster->setSlave(myVideoSlave);
            }
            myVideoMaster->pushStart();
            if(toDecodeSlave) {
                myVideoSlave->pushStart();
            }
        }
    }
}

void StVideo::packetsLoop() {
    double aPts     = 0.0;
    double aPtsbar  = 10.0; /// debug variable
    double aSeekPts = 0.0;
    bool toSeekBack = false;
    StPlayEvent_t aPlayEvent = ST_PLAYEVENT_NONE;
    AVFormatContext* aFormatCtx = NULL;

    // wake up threads
    if(myVideoMaster->isInitialized()) myVideoMaster->pushStart();
    if(myVideoSlave->isInitialized())  myVideoSlave->pushStart();
    if(myAudio->isInitialized())       myAudio->pushStart();
    if(mySubtitles->isInitialized())   mySubtitles->pushStart();

    const bool toKeepPlaying = isPlaying();

    // reset seeking events for previous file
    myVideoMaster->pushPlayEvent(ST_PLAYEVENT_NONE);
    myAudio->pushPlayEvent(ST_PLAYEVENT_NONE);
    if(!myVideoMaster->isInitialized()) {
        myVideoMaster->pushPlayEvent(ST_PLAYEVENT_RESET);
    } else if(toKeepPlaying && !myVideoMaster->isPlaying()) {
        myVideoMaster->pushPlayEvent(ST_PLAYEVENT_PLAY);
    }
    if(!myAudio->isInitialized()) {
        myAudio->pushPlayEvent(ST_PLAYEVENT_RESET);
    } else if(toKeepPlaying && !myAudio->isPlaying()) {
        myAudio->pushPlayEvent(ST_PLAYEVENT_PLAY);
    }

    // indicate new file opened
    signals.onLoaded();

    StArrayList<StAVPacket> anAVPackets(myCtxList.size());
    StArrayList<bool> aQueueIsFull(myCtxList.size());
    myPlayCtxList.clear();
    size_t anEmptyQueues = 0;
    size_t aCtxId = 0;
    for(aCtxId = 0; aCtxId < myCtxList.size(); ++aCtxId) {
        aFormatCtx = myCtxList[aCtxId];
        if(!myVideoMaster->isInContext(aFormatCtx)
        && !myVideoSlave->isInContext(aFormatCtx)
        && !myAudio->isInContext(aFormatCtx)
        && !mySubtitles->isInContext(aFormatCtx)) {
            continue;
        }

        myPlayCtxList.add(aFormatCtx);
        anAVPackets.add(StAVPacket(myCurrParams));
        aQueueIsFull.add(false);
    }

    for(;;) {
        anEmptyQueues = 0;
        for(aCtxId = 0; aCtxId < myPlayCtxList.size(); ++aCtxId) {
            aFormatCtx = myPlayCtxList[aCtxId];
            StAVPacket& aPacket = anAVPackets[aCtxId];
            if(!aQueueIsFull[aCtxId]) {
                // read next packet
                if(av_read_frame(aFormatCtx, aPacket.getAVpkt()) < 0) {
                    ++anEmptyQueues;
                    continue;
                }
            }

            // push packet to appropriate queue
            if(myVideoMaster->isInContext(aFormatCtx, aPacket.getStreamId())) {
                aQueueIsFull[aCtxId] = !pushPacket(myVideoMaster, aPacket);
                if(aQueueIsFull[aCtxId]) {
                    continue;
                }
                const double aTagerFpsNew = myVideoTimer->getAverFps();
                if(targetFps != aTagerFpsNew) {
                    myEventMutex.lock();
                    targetFps = aTagerFpsNew;
                    myEventMutex.unlock();
                }
            } else if(myVideoSlave->isInContext(aFormatCtx, aPacket.getStreamId())) {
                aQueueIsFull[aCtxId] = !pushPacket(myVideoSlave, aPacket);
                if(aQueueIsFull[aCtxId]) {
                    continue;
                }
            } else if(myAudio->isInContext(aFormatCtx, aPacket.getStreamId())) {
                aQueueIsFull[aCtxId] = !pushPacket(myAudio, aPacket);
                if(aQueueIsFull[aCtxId]) {
                    continue;
                }
            } else if(mySubtitles->isInContext(aFormatCtx, aPacket.getStreamId())) {
                aQueueIsFull[aCtxId] = !pushPacket(mySubtitles, aPacket);
                if(aQueueIsFull[aCtxId]) {
                    continue;
                }
            }
            aPacket.free();
        }

        // check events
        checkInitVideoStreams();

        aPlayEvent = popPlayEvent(aSeekPts, toSeekBack);
        if(aPlayEvent == ST_PLAYEVENT_NEXT || toQuit) {
            if(toSave != StImageFile::ST_TYPE_NONE) {
                // save snapshot
                StImageFile::ImageType anImgType = toSave;
                toSave = StImageFile::ST_TYPE_NONE;
                saveSnapshotAs(anImgType);
            } else {
                // load next file
                doFlush();
                if(myAudio->isInitialized()) {
                    myAudio->pushPlayEvent(ST_PLAYEVENT_SEEK, 0.0);
                }
                // resend event after it was pop out
                pushPlayEvent(ST_PLAYEVENT_NEXT);
                break;
            }
        } else if(params.activeAudio->wasChanged()) {
            double aCurrPts = getPts();
            doFlush();
            if(myAudio->isInitialized()) {
                myAudio->pushEnd();
                while(!myAudio->isEmpty() || !myAudio->isInDowntime()) {
                    StThread::sleep(10);
                }
                myAudio->deinit();
            }
            size_t anActiveStreamId = size_t(params.activeAudio->getValue());
            if(!myVideoMaster->isInitialized() && anActiveStreamId == size_t(-1)) {
                anActiveStreamId = 0; // just prevent crash - should be protected in GUI
            }
            if(anActiveStreamId != size_t(-1)) {
                size_t aCounter = 0;
                for(aCtxId = 0; aCtxId < myCtxList.size() && !myAudio->isInitialized(); ++aCtxId) {
                    aFormatCtx = myCtxList[aCtxId];
                    for(unsigned int aStreamId = 0; aStreamId < aFormatCtx->nb_streams; ++aStreamId) {
                        if(aFormatCtx->streams[aStreamId]->codec->codec_type == AVMEDIA_TYPE_AUDIO) {
                            if(aCounter == anActiveStreamId) {
                                myAudio->init(aFormatCtx, aStreamId);
                                myAudio->pushStart();
                                break;
                            }
                            ++aCounter;
                        }
                    }
                }
            }

            // exclude inactive contexts
            myPlayCtxList.clear();
            anAVPackets.clear();
            aQueueIsFull.clear();
            anEmptyQueues = 0;
            for(aCtxId = 0; aCtxId < myCtxList.size(); ++aCtxId) {
                aFormatCtx = myCtxList[aCtxId];
                if(!myVideoMaster->isInContext(aFormatCtx)
                && !myVideoSlave->isInContext(aFormatCtx)
                && !myAudio->isInContext(aFormatCtx)
                && !mySubtitles->isInContext(aFormatCtx)) {
                    continue;
                }
                myPlayCtxList.add(aFormatCtx);
                anAVPackets.add(StAVPacket(myCurrParams));
                aQueueIsFull.add(false);
            }

            pushPlayEvent(ST_PLAYEVENT_SEEK, aCurrPts);
        } else if(params.activeSubtitles->wasChanged()) {
            double aCurrPts = getPts();
            doFlush();
            if(mySubtitles->isInitialized()) {
                mySubtitles->pushEnd();
                while(!mySubtitles->isEmpty() || !mySubtitles->isInDowntime()) {
                    StThread::sleep(10);
                }
                mySubtitles->deinit();
            }
            size_t anActiveStreamId = size_t(params.activeSubtitles->getValue());
            if(anActiveStreamId != size_t(-1)) {
                size_t aCounter = 0;
                for(aCtxId = 0; aCtxId < myCtxList.size() && !mySubtitles->isInitialized(); ++aCtxId) {
                    aFormatCtx = myCtxList[aCtxId];
                    for(unsigned int aStreamId = 0; aStreamId < aFormatCtx->nb_streams; ++aStreamId) {
                        if(aFormatCtx->streams[aStreamId]->codec->codec_type == AVMEDIA_TYPE_SUBTITLE) {
                            if(aCounter == anActiveStreamId) {
                                mySubtitles->init(aFormatCtx, aStreamId);
                                mySubtitles->pushStart();
                                break;
                            }
                            ++aCounter;
                        }
                    }
                }
            }

            // exclude inactive contexts
            myPlayCtxList.clear();
            anAVPackets.clear();
            aQueueIsFull.clear();
            anEmptyQueues = 0;
            for(aCtxId = 0; aCtxId < myCtxList.size(); ++aCtxId) {
                aFormatCtx = myCtxList[aCtxId];
                if(!myVideoMaster->isInContext(aFormatCtx)
                && !myVideoSlave->isInContext(aFormatCtx)
                && !myAudio->isInContext(aFormatCtx)
                && !mySubtitles->isInContext(aFormatCtx)) {
                    continue;
                }
                myPlayCtxList.add(aFormatCtx);
                anAVPackets.add(StAVPacket(myCurrParams));
                aQueueIsFull.add(false);
            }

            pushPlayEvent(ST_PLAYEVENT_SEEK, aCurrPts);
        } else if(aPlayEvent == ST_PLAYEVENT_SEEK) {
            doSeek(aSeekPts, toSeekBack);
            // ignore current packet
            for(aCtxId = 0; aCtxId < myPlayCtxList.size(); ++aCtxId) {
                aQueueIsFull[aCtxId] = false;
                anAVPackets[aCtxId].free();
            }
        }

        ///
        if(aQueueIsFull[0]) {
            StThread::sleep(2);
        }

        aPts = getPts();
        if(aPts > aPtsbar) {
            aPtsbar = aPts + 10.0;
            ST_DEBUG_LOG("Current position: " + StFormatTime::formatSeconds(aPts)
                      + " from "              + StFormatTime::formatSeconds(myDuration));
        }

        // All packets sent
        if(anEmptyQueues == myPlayCtxList.size()) {
            bool areFlushed = false;
            // It seems FFmpeg fail to seek the stream after all packets were read...
            // Thus - we just wait until queues process all packets
            while(!myVideoMaster->isEmpty() || !myVideoMaster->isInDowntime()
               || !myAudio->isEmpty()       || !myAudio->isInDowntime()
               || !myVideoSlave->isEmpty()  || !myVideoSlave->isInDowntime()
               || !mySubtitles->isEmpty()   || !mySubtitles->isInDowntime()) {
                StThread::sleep(10);
                if(!areFlushed && (popPlayEvent(aSeekPts, toSeekBack) == ST_PLAYEVENT_NEXT)) {
                    doFlush();
                    if(myAudio->isInitialized()) {
                        myAudio->pushPlayEvent(ST_PLAYEVENT_SEEK, 0.0);
                    }
                    areFlushed = true;
                }
            }
            // If video is played - always wait until audio played
            if(myVideoMaster->isInitialized() && myAudio->isInitialized()) {
                while(myAudio->stalIsAudioPlaying()) {
                    StThread::sleep(10);
                    if(!areFlushed && (popPlayEvent(aSeekPts, toSeekBack) == ST_PLAYEVENT_NEXT)) {
                        doFlush();
                        if(myAudio->isInitialized()) {
                            myAudio->pushPlayEvent(ST_PLAYEVENT_SEEK, 0.0);
                        }
                        areFlushed = true;
                    }
                }
            }
            // end when any one in format context finished
            break;
        }
    }

    // now send 'end-packet'
    if(myVideoMaster->isInitialized()) myVideoMaster->pushEnd();
    if(myVideoSlave->isInitialized())  myVideoSlave->pushEnd();
    if(myAudio->isInitialized())       myAudio->pushEnd();
    if(mySubtitles->isInitialized())   mySubtitles->pushEnd();

    // what for queues receive 'end-packet'
    while(!myVideoMaster->isEmpty() || !myVideoMaster->isInDowntime()
       || !myAudio->isEmpty()       || !myAudio->isInDowntime()
       || !myVideoSlave->isEmpty()  || !myVideoSlave->isInDowntime()
       || !mySubtitles->isEmpty()   || !mySubtitles->isInDowntime()) {
        StThread::sleep(10);
    }
}

bool StVideo::saveSnapshotAs(StImageFile::ImageType theImgType) {
    if(myCurrParams.isNull() || myCurrNode.isNull()) {
        stInfo(myLangMap->changeValueId(StMoviePlayerStrings::DIALOG_NOTHING_TO_SAVE,
                                        "Nothing to save!"));
        return false;
    }

    pushPlayEvent(ST_PLAYEVENT_PAUSE);

    StImage dataLeft;
    StImage dataRight;
    int result = StGLTextureQueue::SNAPSHOT_NO_NEW;
    if(!myCurrParams->isSwapLR()) {
        result = myTextureQueue->getSnapshot(&dataLeft, &dataRight, true);
    } else {
        result = myTextureQueue->getSnapshot(&dataRight, &dataLeft, true);
    }

    if(result == StGLTextureQueue::SNAPSHOT_NO_NEW || dataLeft.isNull()) {
        stInfo(myLangMap->changeValueId(StMoviePlayerStrings::DIALOG_NO_SNAPSHOT,
                                        "Snapshot not available!"));
        return false;
    }
    StHandle<StImageFile> dataResult = StImageFile::create();
    if(dataResult.isNull()) {
        signals.onError(stCString("No any image library was found!"));
        return false;
    }

    bool toSaveStereo = !dataRight.isNull();
    if(toSaveStereo && dataResult->initSideBySide(dataLeft, dataRight,
                                                  myCurrParams->getSeparationDx(),
                                                  myCurrParams->getSeparationDy())) {
        dataLeft.nullify();
        dataRight.nullify();
    } else {
        dataResult->initWrapper(dataLeft);
    }

    StString title = myLangMap->changeValueId(StMoviePlayerStrings::DIALOG_SAVE_SNAPSHOT,
                                              "Choose location to save snapshot");
    StMIMEList filter;
    StString saveExt;
    if(toSaveStereo) {
        switch(theImgType) {
            case StImageFile::ST_TYPE_PNG:
                saveExt = "pns";
                filter.add(StMIME("image/pns", saveExt,
                                  "PNS - png  stereo image, lossless"));
                break;
            case StImageFile::ST_TYPE_JPEG:
                saveExt = "jps";
                filter.add(StMIME("image/jps", saveExt,
                                  "JPS - jpeg stereo image, lossy"));
                break;
            default:
                return false;
        }
    } else {
        switch(theImgType) {
            case StImageFile::ST_TYPE_PNG:
                saveExt = "png";
                filter.add(StMIME("image/png", saveExt,
                                  "PNG image, lossless"));
                break;
            case StImageFile::ST_TYPE_JPEG:
                saveExt = "jpg";
                filter.add(StMIME("image/jpg", saveExt,
                                  "JPEG image, lossy"));
                break;
            default:
                return false;
        }
    }

    StString fileToSave;
    if(StFileNode::openFileDialog(myCurrNode->getFolderPath(), title, filter, fileToSave, true)) {
        if(StFileNode::getExtension(fileToSave) != saveExt) {
            fileToSave += StString('.') + saveExt;
        }
        ST_DEBUG_LOG("Save snapshot to the path '" + fileToSave + '\'');
        if(!dataResult->save(fileToSave, theImgType)) {
            // TODO (Kirill Gavrilov#7)
            signals.onError(dataResult->getState());
            return false;
        } else if(!dataResult->getState().isEmpty()) {
            ST_DEBUG_LOG(dataResult->getState());
        }
        // TODO (Kirill Gavrilov#8) - update playlist
    }
    return true;
}

StHandle<StMovieInfo> StVideo::getFileInfo(const StHandle<StStereoParams>& theParams) const {
    myEventMutex.lock();
    StHandle<StMovieInfo> anInfo = myFileInfo;
    myEventMutex.unlock();
    if(anInfo.isNull() || anInfo->myId != theParams) {
        return NULL;
    }

    anInfo->myCodecs.clear();
    anInfo->myCodecs.add(StArgument("vcodec1",   myVideoMaster->getCodecInfo()));
    anInfo->myCodecs.add(StArgument("vcodec2",   myVideoSlave->getCodecInfo()));
    anInfo->myCodecs.add(StArgument("audio",     myAudio->getCodecInfo()));
    anInfo->myCodecs.add(StArgument("subtitles", mySubtitles->getCodecInfo()));

    return anInfo;
}

void StVideo::mainLoop() {
    bool isOpenSuccess = false;
    StHandle<StFileNode> aFileToLoad;
    StHandle<StStereoParams> aFileParams;
    double aDummy;
    bool aDummyBool;
    for(;;) {
        // wait for initial message
        waitEvent();
        if(toQuit) {
            return;
        }

        if(!myPlayList->getCurrentFile(aFileToLoad, aFileParams)) {
            continue;
        }
        isOpenSuccess = openSource(aFileToLoad, aFileParams);
        if(isOpenSuccess) {
            break;
        }
    }

    // initial play event
    for(;;) {

        if(myVideoMaster->isInContext(myCtxList[0])) {
            myVideoTimer = new StVideoTimer(myVideoMaster, myAudio,
                1000.0 * av_q2d(myCtxList[0]->streams[myVideoMaster->getId()]->codec->time_base));
            myVideoTimer->setBenchmark(isBenchmark);
        } else if(myCtxList.size() > 1 && myVideoMaster->isInContext(myCtxList[1])) {
            myVideoTimer = new StVideoTimer(myVideoMaster, myAudio,
                1000.0 * av_q2d(myCtxList[1]->streams[myVideoMaster->getId()]->codec->time_base));
            myVideoTimer->setBenchmark(isBenchmark);
        } else {
            myVideoTimer.nullify();
        }

        // decoding processed in other threads
        // here we just send packets into queues
        // and manipulate times
        packetsLoop();

        myVideoTimer.nullify();

        for(;;) {
            if(popPlayEvent(aDummy, aDummyBool) == ST_PLAYEVENT_NEXT) {
                if(toQuit) {
                    return;
                }
            } else {
                myPlayList->walkToNext(false);
            }
            isOpenSuccess = false;
            if(myPlayList->getCurrentFile(aFileToLoad, aFileParams)) {
                isOpenSuccess = openSource(aFileToLoad, aFileParams);
            }
            if(!isOpenSuccess) {
                waitEvent();
            } else {
                break;
            }
        }
    }
}
