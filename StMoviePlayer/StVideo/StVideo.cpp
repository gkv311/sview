/**
 * Copyright © 2007-2025 Kirill Gavrilov <kirill@sview.ru>
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

#include "../StImageViewer/StImagePluginInfo.h"

#include <StStrings/StFormatTime.h>
#include <StAV/StAVIOJniHttpContext.h>

using namespace StMoviePlayerStrings;

namespace {
    static const char ST_AUDIOS_MIME_STRING[] = ST_VIDEO_PLUGIN_AUDIO_MIME_CHAR;
    static const char ST_SUBTIT_MIME_STRING[] = ST_VIDEO_PLUGIN_SUBTIT_MIME_CHAR;

    static SV_THREAD_FUNCTION threadFunction(void* theStVideo) {
        StVideo* aStVideo  = (StVideo* )theStVideo;
        aStVideo->mainLoop();
        return SV_THREAD_RETURN 0;
    }

    /**
     * Format framerate value.
     */
    static StString formatFps(double theVal) {
        //const uint64_t aVal = lrintf(theVal * 100.0);
        const uint64_t aVal = uint64_t(theVal * 100.0 + 0.5);
        char aBuff[256];
        if(aVal == 0) {
            stsprintf(aBuff, sizeof(aBuff), "%1.4f", theVal);
        } else if(aVal % 100) {
            stsprintf(aBuff, sizeof(aBuff), "%3.2f", theVal);
        } else if(aVal % (100 * 1000)) {
            stsprintf(aBuff, sizeof(aBuff), "%1.0f", theVal);
        } else {
            stsprintf(aBuff, sizeof(aBuff), "%1.0fk", theVal / 1000);
        }
        return aBuff;
    }

    /**
     * Format stream info.
     */
    static StString formatStreamInfo(const AVStream* theStream, AVCodecContext* theCodecCtx) {
        AVCodecContext* aCodecCtx = theCodecCtx;
    #if(LIBAVFORMAT_VERSION_INT < AV_VERSION_INT(59, 0, 100))
        if(aCodecCtx == NULL) {
            aCodecCtx = stAV::getCodecCtx(theStream);
        }
    #endif
        char aFrmtBuff[4096] = {};
        if(aCodecCtx != NULL) {
            avcodec_string(aFrmtBuff, sizeof(aFrmtBuff), aCodecCtx, 0);
        }
        StString aStreamInfo(aFrmtBuff);
        //aStreamInfo = aStreamInfo + ", " + theStream->codec_info_nb_frames + ", " + theStream->time_base.num + "/" + theStream->time_base.den;
        if(theStream->sample_aspect_ratio.num && av_cmp_q(theStream->sample_aspect_ratio, theStream->codecpar->sample_aspect_ratio)) {
            AVRational aDispAspectRatio;
            av_reduce(&aDispAspectRatio.num, &aDispAspectRatio.den,
                      theStream->codecpar->width  * int64_t(theStream->sample_aspect_ratio.num),
                      theStream->codecpar->height * int64_t(theStream->sample_aspect_ratio.den),
                      1024 * 1024);
            aStreamInfo = aStreamInfo + ", SAR " + theStream->sample_aspect_ratio.num + ":" + theStream->sample_aspect_ratio.den
                                       + " DAR " + aDispAspectRatio.num + ":" + aDispAspectRatio.den;
        }

        if(theStream->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            if(theStream->avg_frame_rate.den != 0 && theStream->avg_frame_rate.num != 0) {
                aStreamInfo += StString(", ") + formatFps(av_q2d(theStream->avg_frame_rate)) + " fps";
            }
            if(theStream->r_frame_rate.den != 0 && theStream->r_frame_rate.num != 0) {
                aStreamInfo += StString(", ") + formatFps(av_q2d(theStream->r_frame_rate)) + " tbr";
            }
            if(theStream->time_base.den != 0 && theStream->time_base.num != 0) {
                aStreamInfo += StString(", ") + formatFps(1 / av_q2d(theStream->time_base)) + " tbn";
            }
            if(aCodecCtx != NULL
            && aCodecCtx->time_base.den != 0 && aCodecCtx->time_base.num != 0) {
                aStreamInfo += StString(", ") + formatFps(1 / av_q2d(aCodecCtx->time_base)) + " tbc";
            }
        }
        return aStreamInfo;
    }
}

const char* StVideo::ST_IMAGES_MIME_STRING = ST_IMAGE_PLUGIN_MIME_CHAR;
const char* StVideo::ST_VIDEOS_MIME_STRING = ST_VIDEO_PLUGIN_MIME_CHAR;

StVideo::StVideo(const std::string&                 theALDeviceName,
                 StAudioQueue::StAlHintOutput       theAlOutput,
                 StAudioQueue::StAlHintHrtf         theAlHrtf,
                 const StHandle<StResourceManager>& theResMgr,
                 const StHandle<StTranslations>&    theLangMap,
                 const StHandle<StPlayList>&        thePlayList,
                 const StHandle<StGLTextureQueue>&  theTextureQueue,
                 const StHandle<StSubQueue>&        theSubtitlesQueue1,
                 const StHandle<StSubQueue>&        theSubtitlesQueue2)
: myMimesVideo(ST_VIDEOS_MIME_STRING),
  myMimesAudio(ST_AUDIOS_MIME_STRING),
  myMimesSubs(ST_SUBTIT_MIME_STRING),
  myMimesImages(ST_IMAGES_MIME_STRING),
  myResMgr(theResMgr),
  myLangMap(theLangMap),
  mySlaveCtx(NULL),
  mySlaveStream(-1),
  myPlayList(thePlayList),
  myTextureQueue(theTextureQueue),
  myDuration(0.0),
  myPtsSeek(0.0),
  myToSeekBack(false),
  myPlayEvent(ST_PLAYEVENT_NONE),
  myTargetFps(0.0),
  //
  myAudioDelayMSec(0),
  myIsBenchmark(false),
  toSave(StImageFile::ST_TYPE_NONE),
  toQuit(false),
  myQuitEvent(false) {
    // initialize FFmpeg library if not yet performed
    stAV::init();

    myPlayList->setExtensions(myMimesVideo.getExtensionsList());
    myTracksExt = myMimesSubs.getExtensionsList();
    StArrayList<StString> anAudioExt = myMimesAudio.getExtensionsList();
    for(size_t anExtIter = 0; anExtIter < anAudioExt.size(); ++anExtIter) {
        myTracksExt.add(anAudioExt.getValue(anExtIter));
    }

    params.UseGpu           = new StBoolParam(false);
    params.UseOpenJpeg      = new StBoolParam(false);
    params.activeAudio      = new StParamActiveStream();
    params.activeSubtitles1 = new StParamActiveStream();
    params.activeSubtitles2 = new StParamActiveStream();

    myVideoMaster = new StVideoQueue(myTextureQueue);
    myVideoMaster->signals.onError.connect(this, &StVideo::doOnErrorRedirect);

    myVideoSlave  = new StVideoQueue(myTextureQueue, myVideoMaster);
    myVideoSlave->signals.onError.connect(this, &StVideo::doOnErrorRedirect);

    myAudio = new StAudioQueue(theALDeviceName, theAlOutput, theAlHrtf);
    myAudio->signals.onError.connect(this, &StVideo::doOnErrorRedirect);

    mySubtitles1 = new StSubtitleQueue(theSubtitlesQueue1);
    mySubtitles1->signals.onError.connect(this, &StVideo::doOnErrorRedirect);

    mySubtitles2 = new StSubtitleQueue(theSubtitlesQueue2);
    mySubtitles2->signals.onError.connect(this, &StVideo::doOnErrorRedirect);

    // launch working thread
    myThread = new StThread(threadFunction, (void* )this, "StVideo");
}

class ST_LOCAL StHangKiller {

        public:

    StHangKiller(const double theLimitSec,
                 const char** theStages)
    : myStages(theStages),
      myLimitSec(theLimitSec),
      myDoneEvent(false),
      myStageIter(0) {
        myThread = new StThread(threadWatcher, this, "StHangKiller");
    }

    ~StHangKiller() {
        myDoneEvent.set();
        myThread->wait();
        myThread.nullify();
    }

    void nextStage() {
        myStageIter.increment();
    }

    void setDone() {
        myDoneEvent.set();
    }

        private:

    void waitLoop() {
        StTimer aTimer(true);
        for(;;) {
            if(myDoneEvent.wait(1000)) {
                return;
            }

            if(aTimer.getElapsedTimeInSec() >= myLimitSec) {
                const char* aState = myStages[myStageIter.getValue()];
                ST_ERROR_LOG("StHangKiller waiting for " + aState + "... " + aTimer.getElapsedTimeInSec() + " seconds elapsed, exiting!");
                exit(-1);
            }
        }
    }

    static SV_THREAD_FUNCTION threadWatcher(void* theWatcher) {
        StHangKiller* aWatcher = (StHangKiller* )theWatcher;
        aWatcher->waitLoop();
        return SV_THREAD_RETURN 0;
    }

        private: // no copies, please

    StHangKiller(const StHangKiller& theCopy);
    const StHangKiller& operator=(const StHangKiller& theCopy);

        private:

    StHandle<StThread> myThread;
    const char**       myStages;
    const double       myLimitSec;
    StCondition        myDoneEvent;
    StAtomic<int32_t>  myStageIter;

};

void StVideo::startDestruction() {
    if(toQuit) {
        return;
    }

    toQuit = true;
    toSave = StImageFile::ST_TYPE_NONE;
    pushPlayEvent(ST_PLAYEVENT_NEXT);
    myTextureQueue->clear();
    myQuitEvent.wait(1000);
}

StVideo::~StVideo() {
    // stop the thread
    startDestruction();

    // wait main thread is quit
    const char* THE_STATES[] = {
        "StVideo::mainLoop()",
        "StSubtitleQueue thread",
        "StAudioQueue thread",
        "StVideoQueue (slave) thread",
        "StVideoQueue (master) thread",
        "DONE"
    };

    StHangKiller aHangKiller(10.0, THE_STATES);
    myThread->wait();
    myThread.nullify();
    myVideoTimer.nullify();

    // close all decoding threads
    aHangKiller.nextStage();
    mySubtitles2.nullify();
    mySubtitles1.nullify();
    aHangKiller.nextStage();
    myAudio.nullify();
    aHangKiller.nextStage();
    myVideoSlave.nullify();
    aHangKiller.nextStage();
    myVideoMaster.nullify();
    aHangKiller.setDone();
    close(); // we must quit or flush video/audio threads before close()!
}

void StVideo::close() {
    if(!myVideoSlave.isNull())  { myVideoSlave->deinit(); }
    if(!myVideoMaster.isNull()) { myVideoMaster->deinit(); }
    if(!myAudio.isNull())       { myAudio->deinit(); }
    if(!mySubtitles1.isNull())  { mySubtitles1->deinit(); }
    if(!mySubtitles2.isNull())  { mySubtitles2->deinit(); }
    for(size_t ctxId = 0; ctxId < myCtxList.size(); ++ctxId) {
        AVFormatContext*& formatCtx = myCtxList[ctxId];
        if(formatCtx != NULL) {
            avformat_close_input(&formatCtx);
        }
    }
    myFileList.clear();
    myCtxList.clear();
    myFileIOList.clear();
    myPlayCtxList.clear();
    mySlaveCtx    = NULL;
    mySlaveStream = -1;

    params.activeAudio->clearList();
    params.activeSubtitles1->clearList();
    params.activeSubtitles2->clearList();
    myCurrNode.nullify();
    myCurrParams.nullify();
    myCurrPlsFile.nullify();

    myEventMutex.lock();
        myDuration = 0.0;
    myEventMutex.unlock();
}

void StVideo::setBenchmark(bool toPerformBenchmark) {
    myIsBenchmark = toPerformBenchmark;
}

void StVideo::setAudioDelay(const float theDelaySec) {
    myAudioDelayMSec = int(theDelaySec * 1000.0f + (theDelaySec > 0.0f ? 0.5f : -0.5));
    myVideoMaster->setAudioDelay(myAudioDelayMSec);
}

bool StVideo::addFile(const StString& theFileToLoad,
                      const StHandle<StStereoParams>& theNewParams,
                      StStreamsInfo&  theInfo) {
    // open video file
    StString aFileName, aDummy;
    StFileNode::getFolderAndFile(theFileToLoad, aDummy, aFileName);

    StHandle<StAVIOContext> anIOContext;
    if(StFileNode::isContentProtocolPath(theFileToLoad)) {
        int aFileDescriptor = myResMgr->openFileDescriptor(theFileToLoad);
        if(aFileDescriptor != -1) {
            StHandle<StAVIOFileContext> aFileCtx = new StAVIOFileContext();
            if(aFileCtx->openFromDescriptor(aFileDescriptor, "rb")) {
                anIOContext = aFileCtx;
            }
        }
    }
#if defined(__ANDROID__)
    else if(theFileToLoad.isStartsWith(stCString("https://"))) {
        static const bool hasHttpsProtocol = stAV::isEnabledInputProtocol("https");
        if(!hasHttpsProtocol) {
            StHandle<StAVIOJniHttpContext> aHttpCtx = new StAVIOJniHttpContext();
            if(aHttpCtx->open(theFileToLoad)) {
                anIOContext = aHttpCtx;
            }
        }
    }
#endif
    AVFormatContext* aFormatCtx = NULL;
    if(!anIOContext.isNull()) {
        aFormatCtx = avformat_alloc_context();
        aFormatCtx->pb = anIOContext->getAvioContext();
    }

    int avErrCode = avformat_open_input(&aFormatCtx, theFileToLoad.toCString(), NULL, NULL);
    if(avErrCode != 0) {
        signals.onError(StString("FFmpeg: Couldn't open video file '") + theFileToLoad
                      + "'\nError: " + stAV::getAVErrorDescription(avErrCode));
        if(aFormatCtx != NULL) {
            avformat_close_input(&aFormatCtx);
        }
        return false;
    }

    // retrieve stream information
    if(avformat_find_stream_info(aFormatCtx, NULL) < 0) {
        signals.onError(StString("FFmpeg: Couldn't find stream information in '") + theFileToLoad + "'");
        if(aFormatCtx != NULL) {
            avformat_close_input(&aFormatCtx);
        }
        return false;
    }

#ifdef ST_DEBUG
    av_dump_format(aFormatCtx, 0, theFileToLoad.toCString(), false);
#endif

    StString aTitleString, aFolder;
    StFileNode::getFolderAndFile(theFileToLoad, aFolder, aTitleString);
    if(myFileInfoTmp->Path.isEmpty()) {
        myFileInfoTmp->Path = theFileToLoad;
    }
    StDictEntry& aFileNamePair = myFileInfoTmp->Info.addChange(tr(INFO_FILE_NAME));
    if(!aFileNamePair.getValue().isEmpty()) {
        aFileNamePair.changeValue() += "\n";
    }
    aFileNamePair.changeValue() += aTitleString;

    // collect metadata
    for(stAV::meta::Tag* aTag = stAV::meta::findTag(aFormatCtx->metadata, "", NULL, stAV::meta::SEARCH_IGNORE_SUFFIX);
        aTag != NULL;
        aTag = stAV::meta::findTag(aFormatCtx->metadata, "", aTag, stAV::meta::SEARCH_IGNORE_SUFFIX)) {
        myFileInfoTmp->Info.add(StArgument(aTag->key, aTag->value));
    }

    const StString& aPrefLangAudio = myLangMap->getLanguageCode();
    int32_t anAudioStreamId = (int32_t )theInfo.AudioList->size();
    int32_t aSubsStreamId = (int32_t )theInfo.SubtitleList->size();

    theInfo.Duration = stMax(theInfo.Duration, stAV::unitsToSeconds(aFormatCtx->duration));
    for(unsigned int aStreamId = 0; aStreamId < aFormatCtx->nb_streams; ++aStreamId) {
        AVStream*         aStream    = aFormatCtx->streams[aStreamId];
        const AVMediaType aCodecType = stAV::getCodecType(aStream);
        theInfo.Duration = stMax(theInfo.Duration, stAV::unitsToSeconds(aStream, aStream->duration));

        StString aLang = stAV::meta::readLang(aStream);
        if(aLang == "und") {
            aLang.clear();
        }

        if(aCodecType == AVMEDIA_TYPE_VIDEO) {
            // video track
            if(!myVideoMaster->isInitialized()) {
                myVideoMaster->init(aFormatCtx, aStreamId, aTitleString, theNewParams);
                myVideoMaster->setSlave(NULL);

                if(myVideoMaster->isInitialized()) {
                    myAudio->setTrackHeadOrientation(params.ToTrackHeadAudio->getValue() && theNewParams->ViewingMode != StViewSurface_Plain);

                    const int aSizeX      = myVideoMaster->sizeX();
                    const int aSizeY      = myVideoMaster->sizeY();
                    const int aCodedSizeX = myVideoMaster->getCodedSizeX();
                    const int aCodedSizeY = myVideoMaster->getCodedSizeY();
                    StString  aDimsStr    = StString() + aSizeX + " x " + aSizeY;
                    if((aCodedSizeX != aSizeX && aCodedSizeX != 0)
                    || (aCodedSizeY != aSizeY && aCodedSizeY != 0)) {
                        aDimsStr += StString(" [") + aCodedSizeX + " x " + aCodedSizeY + "]";
                    }

                    StDictEntry& aDimInfo = myFileInfoTmp->Info.addChange(tr(INFO_DIMENSIONS));
                    aDimInfo.changeValue() = aDimsStr;

                    myFileInfoTmp->Info.add(StArgument(tr(INFO_PIXEL_FORMAT),
                        myVideoMaster->getPixelFormatString()));
                    myFileInfoTmp->Info.add(StArgument(tr(INFO_PIXEL_RATIO),
                        StString() + myVideoMaster->getPixelRatio()));
                    myFileInfoTmp->Info.add(StArgument(tr(INFO_DURATION),
                        StFormatTime::formatSeconds(theInfo.Duration)));
                }
            } else if(!myVideoSlave->isInitialized()
                   && !stAV::isAttachedPicture(aStream)) {
                myVideoSlave->init(aFormatCtx, aStreamId, aTitleString, theNewParams);
                if(myVideoSlave->isInitialized()) {
                    mySlaveCtx    = aFormatCtx;
                    mySlaveStream = aStreamId;

                    const int aSizeX      = myVideoSlave->sizeX();
                    const int aSizeY      = myVideoSlave->sizeY();
                    const int aCodedSizeX = myVideoSlave->getCodedSizeX();
                    const int aCodedSizeY = myVideoSlave->getCodedSizeY();
                    StString  aDimsStr    = StString() + aSizeX + " x " + aSizeY;
                    if((aCodedSizeX != aSizeX && aCodedSizeX != 0)
                    || (aCodedSizeY != aSizeY && aCodedSizeY != 0)) {
                        aDimsStr += StString(" [") + aCodedSizeX + " x " + aCodedSizeY + "]";
                    }
                    aDimsStr += " [2]";

                    StDictEntry& aDimInfo = myFileInfoTmp->Info.addChange(tr(INFO_DIMENSIONS));
                    aDimInfo.changeValue() += "\n";
                    aDimInfo.changeValue() += aDimsStr;

                    if(myVideoMaster->getStereoFormatByUser() == StFormat_AUTO) {
                        myVideoMaster->setSlave(myVideoSlave);
                    } else {
                        myVideoSlave->deinit();
                    }
                }
            }
        } else if(aCodecType == AVMEDIA_TYPE_AUDIO) {
            // audio track
            StString aCodecName;
            const AVCodec* aCodec = avcodec_find_decoder(stAV::getCodecId(aStream));
            if(aCodec != NULL) {
                aCodecName = aCodec->name;
            }

            const char* aSampleFormatStr = av_get_sample_fmt_name((AVSampleFormat )aStream->codecpar->format);
            const StString aSampleFormat  = aSampleFormatStr != NULL ? StString(aSampleFormatStr) : StString("");
            const StString aSampleRate    = StString(aStream->codecpar->sample_rate) + " Hz";
            const StString aChannelLayout = stAV::audio::getChannelLayoutString(aStream);
            StString aStreamTitle = aCodecName;
            if(!aSampleRate.isEmpty()) {
                aStreamTitle += StString(", ") + aSampleRate;
            }
            aStreamTitle     += StString(", ") + aChannelLayout;
            if(!aSampleFormat.isEmpty()) {
                aStreamTitle += StString(", ") + aSampleFormat;
            }
            if(!aLang.isEmpty()) {
                aStreamTitle += StString(" (") + aLang + ")";
            }

            if( aFormatCtx->nb_streams == 1
            && !myCtxList.isEmpty()) {
                aStreamTitle += (aFileName.getLength() > 24)
                              ? (StString(" ...") + aFileName.subString(aFileName.getLength() - 24, aFileName.getLength()))
                              : (StString(" ") + aFileName);
            }
            theInfo.AudioList->add(aStreamTitle);

            if(!myAudio->isInitialized()
            && (aPrefLangAudio.isEmpty() || aLang == aPrefLangAudio)
            &&  myAudio->init(aFormatCtx, aStreamId, aTitleString)) {
                theInfo.LoadedAudio = (int32_t )(theInfo.AudioList->size() - 1);
            }
        } else if(aCodecType == AVMEDIA_TYPE_SUBTITLE) {
            // subtitles track
            StString aCodecName("PLAIN");
            const AVCodec* aCodec = avcodec_find_decoder(stAV::getCodecId(aStream));
            if(aCodec != NULL) {
                aCodecName = aCodec->name;
            }

            StString aStreamTitle = aCodecName;
            if(!aLang.isEmpty()) {
                aStreamTitle += StString(" (") + aLang + ")";
            }
            if(aFormatCtx->nb_streams == 1) {
                aStreamTitle += StString(", ") + aFileName;
            }
            theInfo.SubtitleList->add(aStreamTitle);

            if(params.ToAutoLoadSubs->getValue()
            && !mySubtitles1->isInitialized()
            && (aPrefLangAudio.isEmpty() || aLang == aPrefLangAudio)
            &&  mySubtitles1->init(aFormatCtx, aStreamId, aTitleString)) {
                theInfo.LoadedSubtitles1 = (int32_t )(theInfo.SubtitleList->size() - 1);
            }
        }
    }

    // load first audio stream if preferred language is unavailable
    if(!myAudio->isInitialized()
    && !aPrefLangAudio.isEmpty()
    && !theInfo.AudioList->isEmpty()) {
        for(unsigned int aStreamId = 0; aStreamId < aFormatCtx->nb_streams; ++aStreamId) {
            AVStream* aStream = aFormatCtx->streams[aStreamId];
            if(stAV::getCodecType(aStream) != AVMEDIA_TYPE_AUDIO) {
                continue;
            }

            StString aLanguage;
            stAV::meta::readTag(aStream, stCString("language"), aLanguage);
            if(aLanguage != aPrefLangAudio
            && myAudio->init(aFormatCtx, aStreamId, aTitleString)) {
                theInfo.LoadedAudio = anAudioStreamId;
                break;
            }
            ++anAudioStreamId;
        }
    }

    // load subtitles stream
    if(params.ToAutoLoadSubs->getValue()
    && !mySubtitles1->isInitialized()
    && !aPrefLangAudio.isEmpty()
    && !theInfo.SubtitleList->isEmpty()) {
        for(unsigned int aStreamId = 0; aStreamId < aFormatCtx->nb_streams; ++aStreamId) {
            AVStream* aStream = aFormatCtx->streams[aStreamId];
            if(stAV::getCodecType(aStream) != AVMEDIA_TYPE_SUBTITLE) {
                continue;
            }

            StString aLanguage;
            stAV::meta::readTag(aStream, stCString("language"), aLanguage);
            if(aLanguage != aPrefLangAudio
            && mySubtitles1->init(aFormatCtx, aStreamId, aTitleString)) {
                theInfo.LoadedSubtitles1 = aSubsStreamId;
                break;
            }
            ++aSubsStreamId;
        }
    }

    myFileIOList.add(anIOContext);
    myCtxList.add(aFormatCtx);
    myFileList.add(theFileToLoad);
    return true;
}

bool StVideo::openSource(const StHandle<StFileNode>&     theNewSource,
                         const StHandle<StStereoParams>& theNewParams,
                         const StHandle<StFileNode>&     theNewPlsFile) {
    // just for safe - close previously opened video
    close();

    const bool toUseGpu      = params.UseGpu->getValue();
    const bool toUseOpenJpeg = params.UseOpenJpeg->getValue();
    myVideoMaster->setUseGpu(toUseGpu);
    myVideoSlave ->setUseGpu(toUseGpu);
    myVideoMaster->setUseOpenJpeg(toUseOpenJpeg);
    myVideoSlave ->setUseOpenJpeg(toUseOpenJpeg);
    myAudio->setTrackHeadOrientation(false);

    myFileInfoTmp = new StMovieInfo();

    StStreamsInfo aStreamsInfo;
    aStreamsInfo.AudioList    = new StArrayList<StString>(8);
    aStreamsInfo.SubtitleList = new StArrayList<StString>(8);
    if(!theNewSource->isEmpty()) {
        bool isLoaded = false;
        for(size_t aNode = 0; aNode < theNewSource->size(); ++aNode) {
            isLoaded = addFile(theNewSource->getValue(aNode)->getPath(), theNewParams, aStreamsInfo) || isLoaded;
        }
        if(!isLoaded) {
            return false;
        }
    } else {
        const StString aFullPath = theNewSource->getPath();
        if(!addFile(aFullPath, theNewParams, aStreamsInfo)) {
            return false;
        }

        // search for additional tracks
        if(params.ToSearchSubs->getValue()
        && myVideoMaster->isInitialized()
        && !StFileNode::isRemoteProtocolPath(aFullPath)) {
            StString aFolder, aFileName;
            StFileNode::getFolderAndFile(aFullPath, aFolder, aFileName);
            if(aFileName.getLength() > 8) { // ignore too short names
                StString aName, anExtension, aTrackName, aTrackExtension;
                StFileNode::getNameAndExtension(aFileName, aName, anExtension);
                if(myTracksFolder.getPath() != aFolder) {
                    // notice that playlist re-loading is not checked here...
                    myTracksFolder.setSubPath(aFolder);
                    myTracksFolder.init(myTracksExt, 1);
                }
                for(size_t aNodeIter = 0; aNodeIter < myTracksFolder.size(); ++aNodeIter) {
                    const StFileNode* aNode          = myTracksFolder.getValue(aNodeIter);
                    const StString&   aTrackFileName = aNode->getSubPath();
                    StFileNode::getNameAndExtension(aTrackFileName, aTrackName, aTrackExtension);
                    if(aFileName != aTrackFileName
                    && aTrackName.isStartsWithIgnoreCase(aName)) {
                        //myPlayList->addToNode(aCurrFile, aFilePath);
                        //myPlayList->getCurrentFile(theNewSource, theNewParams)
                        addFile(aNode->getPath(), theNewParams, aStreamsInfo);
                    }
                }
            }
        }
    }

    // read general information about streams
    for(size_t aCtxIter = 0; aCtxIter < myCtxList.size(); ++aCtxIter) {
        AVFormatContext* aFormatCtx = myCtxList[aCtxIter];

        StString aFrmtInfo;
        if(aFormatCtx->bit_rate != 0) {
            aFrmtInfo += StString("bitrate: ") + (int64_t(aFormatCtx->bit_rate) / 1000) + " kb/s";
        }
        if(!aFrmtInfo.isEmpty()) {
            StString aStreamInfoKey = StString("input ") + aCtxIter;
            myFileInfoTmp->Info.add(StArgument(aStreamInfoKey, aFrmtInfo));
        }

        for(unsigned int aStreamId = 0; aStreamId < aFormatCtx->nb_streams; ++aStreamId) {
            AVStream* aStream = aFormatCtx->streams[aStreamId];
            StString aLang = stAV::meta::readLang(aStream);
            if(aLang.isEmpty()) {
                aLang = "und";
            }

            StString aStreamInfoKey = StString("steam ")
                                    + (myCtxList.size() > 1 ? (StString() + aCtxIter + ":" + aStreamId) : (StString() + aStreamId))
                                    + " [" + aLang + "]";
            AVCodecContext* aCodecCtx = NULL;
            if(myVideoMaster->isInContext(aFormatCtx, aStreamId)) {
                aCodecCtx = myVideoMaster->getCodecContext();
            } else if(myVideoSlave->isInContext(aFormatCtx, aStreamId)) {
                aCodecCtx = myVideoSlave->getCodecContext();
            } else if(myAudio->isInContext(aFormatCtx, aStreamId)) {
                aCodecCtx = myAudio->getCodecContext();
            } else if(mySubtitles1->isInContext(aFormatCtx, aStreamId)) {
                aCodecCtx = mySubtitles1->getCodecContext();
            } else if(mySubtitles2->isInContext(aFormatCtx, aStreamId)) {
                aCodecCtx = mySubtitles2->getCodecContext();
            }

            myFileInfoTmp->Info.add(StArgument(aStreamInfoKey, formatStreamInfo(aStream, aCodecCtx)));
        }
    }

    if(!myVideoMaster->isInitialized() && !myAudio->isInitialized()) {
        signals.onError(stCString("FFmpeg: Didn't found any video or audio streams"));
        return false;
    }

    StArgument aTitle = myFileInfoTmp->Info["TITLE"];
    if(!aTitle.isValid()) {
        aTitle = myFileInfoTmp->Info["title"];
    }
    StArgument anArtist = myFileInfoTmp->Info["ARTIST"];
    if(!anArtist.isValid()) {
        anArtist = myFileInfoTmp->Info["artist"];
    }

    if(anArtist.isValid() && aTitle.isValid()) {
        StString anArtistStr = anArtist.getValue();
        StString aTitleStr   = aTitle.getValue();
        anArtistStr.rightAdjust();
        aTitleStr  .rightAdjust();
        myPlayList->setTitle(theNewParams, anArtistStr + " - " + aTitleStr);
    } else if(aTitle.isValid()) {
        StString aTitleStr = aTitle.getValue();
        aTitleStr.rightAdjust();
        if(aTitleStr.getLength() < 20) {
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
            myPlayList->setTitle(theNewParams, aTitleStr + " (" + aFileName + ")");
        } else {
            myPlayList->setTitle(theNewParams, aTitleStr);
        }
    }

    myCurrNode    = theNewSource;
    myCurrParams  = theNewParams;
    myCurrPlsFile = theNewPlsFile;
    myFileInfoTmp->Id = myCurrParams;

    params.activeAudio     ->setList(aStreamsInfo.AudioList,    aStreamsInfo.LoadedAudio);
    params.activeSubtitles1->setList(aStreamsInfo.SubtitleList, aStreamsInfo.LoadedSubtitles1);
    params.activeSubtitles2->setList(aStreamsInfo.SubtitleList, aStreamsInfo.LoadedSubtitles2);

    myEventMutex.lock();
        myDuration = aStreamsInfo.Duration;
        myFileInfo = myFileInfoTmp;
    myEventMutex.unlock();

    return true;
}

void StVideo::doFlush() {
    // clear packet queues from obsolete data
    mySubtitles1->clear();
    mySubtitles2->clear();
    myAudio->clear();
    myVideoMaster->clear();
    myVideoSlave->clear();

    // push FLUSH packet to queues so they must flush FFmpeg codec buffers
    if(myVideoMaster->isInitialized()) { myVideoMaster->pushFlush(); }
    if(myVideoSlave->isInitialized())  { myVideoSlave->pushFlush(); }
    if(myAudio->isInitialized())       { myAudio->pushFlush(); }
    if(mySubtitles1->isInitialized())  { mySubtitles1->pushFlush(); }
    if(mySubtitles2->isInitialized())  { mySubtitles2->pushFlush(); }
}

void StVideo::doFlushSoft() {
    // clear packet queues from obsolete data
    mySubtitles1->clear();
    mySubtitles2->clear();
    myAudio->clear();
    myVideoMaster->clear();
    myVideoSlave->clear();

    // push FLUSH packet to queues so they must flush FFmpeg codec buffers
    if( myVideoMaster->isInitialized()
    && !myVideoMaster->isAttachedPicture()) {
        myVideoMaster->pushFlush();
    }
    if( myVideoSlave->isInitialized()
    && !myVideoSlave->isAttachedPicture()) {
        myVideoSlave->pushFlush();
    }
    if(myAudio->isInitialized())      { myAudio->pushFlush(); }
    if(mySubtitles1->isInitialized()) { mySubtitles1->pushFlush(); }
    if(mySubtitles2->isInitialized()) { mySubtitles2->pushFlush(); }
}

void StVideo::doSeek(const double theSeekPts,
                     const bool   toSeekBack) {
    for(size_t ctxId = 0; ctxId < myPlayCtxList.size(); ++ctxId) {
        doSeekContext(myPlayCtxList[ctxId], theSeekPts, toSeekBack);
    }

    // clear packet queues from obsolete data
    doFlushSoft();
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
    } else if(myAudio->isInContext(theFormatCtx)) {
        //
    } else if(mySubtitles1->isInContext(theFormatCtx)) {
        if(mySubtitles1->getFileName().isEndsWithIgnoreCase(stCString(".srt"))) {
            // workaround SRT seeking issues - make a heavy seek (usual size of SRT file is not greater than 200 KiB)
            isSeekDone = doSeekStream(theFormatCtx, mySubtitles1->getId(), 0.0, true);
        } else {
            isSeekDone = doSeekStream(theFormatCtx, mySubtitles1->getId(), theSeekPts, toSeekBack);
        }
    } else if(mySubtitles2->isInContext(theFormatCtx)) {
        if(mySubtitles2->getFileName().isEndsWithIgnoreCase(stCString(".srt"))) {
            // workaround SRT seeking issues - make a heavy seek (usual size of SRT file is not greater than 200 KiB)
            isSeekDone = doSeekStream(theFormatCtx, mySubtitles2->getId(), 0.0, true);
        } else {
            isSeekDone = doSeekStream(theFormatCtx, mySubtitles2->getId(), theSeekPts, toSeekBack);
        }
    }
    if(!isSeekDone && myAudio->isInContext(theFormatCtx)) {
        // if no video stream or seeking was failed - try to seek Audio stream
        isSeekDone = doSeekStream(theFormatCtx, myAudio->getId(), theSeekPts, toSeekBack);
    }
    if(!isSeekDone) {
        // at last - try to seek the format context itself...
        const int aFlags      = toSeekBack ? AVSEEK_FLAG_BACKWARD : 0;
        int64_t   aSeekTarget = stAV::secondsToUnits(theSeekPts);
        isSeekDone = av_seek_frame(theFormatCtx, -1, aSeekTarget, aFlags) >= 0;
        if(!isSeekDone) {
        #ifdef ST_DEBUG
            const char* aFileName = theFormatCtx->url;
            ST_ERROR_LOG("Disaster! Seeking to " + theSeekPts + " [" + aFileName + "] has failed.");
        #endif
        }
    }
}

bool StVideo::doSeekStream(AVFormatContext* theFormatCtx,
                           const signed int theStreamId,
                           const double     theSeekPts,
                           const bool       toSeekBack) {
    const int aFlags = toSeekBack ? AVSEEK_FLAG_BACKWARD : 0;
    AVStream* aStream = theFormatCtx->streams[theStreamId];
    if(stAV::isAttachedPicture(aStream)) {
        return false;
    }

    int64_t aSeekTarget = stAV::secondsToUnits(aStream, theSeekPts + stAV::unitsToSeconds(aStream, aStream->start_time));
    bool isSeekDone = av_seek_frame(theFormatCtx, theStreamId, aSeekTarget, aFlags) >= 0;

    // try 10 more times in backward direction to work-around huge duration between key frames
#if(LIBAVFORMAT_VERSION_INT >= AV_VERSION_INT(59, 3, 100))
    /// TODO AVStream::cur_dts has been removed without libavformat version bump and without entry in APIchanges
    /// with comment "no reason to have them exposed in a public header"
#else
    // will not work for some streams with undefined cur_dts (AV_NOPTS_VALUE)!!!
    for(int aTries = 10; isSeekDone && toSeekBack && aTries > 0 && (aStream->cur_dts > aSeekTarget); --aTries) {
        aSeekTarget -= stAV::secondsToUnits(aStream, 1.0);
        isSeekDone = av_seek_frame(theFormatCtx, theStreamId, aSeekTarget, aFlags) >= 0;
    }
#endif

    if(!isSeekDone) {
        ST_DEBUG_LOG("Error while seeking"
                   + (stAV::getCodecType(aStream) == AVMEDIA_TYPE_VIDEO ? " Video"
                   : (stAV::getCodecType(aStream) == AVMEDIA_TYPE_AUDIO ? " Audio" : " "))
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
    const bool toDecodeSlave = myVideoMaster->getStereoFormatByUser() == StFormat_AUTO
                            && mySlaveStream >= 0;
    // keep failed flag
    const bool isGpuFailed   = myVideoMaster->isGpuFailed()
                           || (myVideoSlave->isInitialized() && myVideoSlave->isGpuFailed());
    if(toUseGpu      != myVideoMaster->toUseGpu()
    || toDecodeSlave != myVideoSlave->isInitialized()) {
        doFlush();
        if(myVideoMaster->isInitialized()) {
            const StString   aFileNameMaster = myVideoMaster->getFileName();
            const StString   aFileNameSlave  = myVideoSlave ->getFileName();
            AVFormatContext* aCtxMaster      = myVideoMaster->getContext();
            const signed int aStreamIdMaster = myVideoMaster->getId();
            myVideoMaster->pushEnd();
            if(myVideoSlave->isInitialized()) {
                myVideoSlave->pushEnd();
            }
            while(!myVideoMaster->isEmpty() || !myVideoMaster->isInDowntime()
               || !myVideoSlave->isEmpty()  || !myVideoSlave->isInDowntime()) {
                if(toQuit) {
                    break;
                }
                StThread::sleep(10);
            }
            myVideoMaster->deinit();
            if(myVideoSlave->isInitialized()) {
                myVideoSlave->deinit();
            }

            myVideoMaster->setUseGpu(toUseGpu, isGpuFailed);
            myVideoSlave ->setUseGpu(toUseGpu, isGpuFailed);
            myVideoMaster->init(aCtxMaster, aStreamIdMaster, aFileNameMaster, myCurrParams);
            myVideoMaster->setSlave(NULL);
            if(toDecodeSlave) {
                myVideoSlave->init(mySlaveCtx, mySlaveStream, aFileNameSlave, myCurrParams);
                myVideoMaster->setSlave(myVideoSlave);
            }
            myVideoMaster->pushStart();
            if(toDecodeSlave) {
                myVideoSlave->pushStart();
            }
        } else {
            myVideoMaster->setUseGpu(toUseGpu);
            myVideoSlave ->setUseGpu(toUseGpu);
        }
    }
}

void StVideo::doSwitchAudioStream(std::vector<StAVPacket>& theAVPackets,
                                  std::vector<bool>& theQueueIsFull,
                                  size_t& theEmptyQueues) {
    double aCurrPts = getPts();
    doFlushSoft();
    const bool toPlayNewAudio = isPlaying();
    if(myAudio->isInitialized()) {
        myAudio->pushEnd();
        while(!myAudio->isEmpty() || !myAudio->isInDowntime()) {
            if(toQuit) {
                myQuitEvent.set();
                break;
            }
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
        for(size_t aCtxId = 0; aCtxId < myCtxList.size() && !myAudio->isInitialized(); ++aCtxId) {
            AVFormatContext* aFormatCtx = myCtxList[aCtxId];
            for(unsigned int aStreamId = 0; aStreamId < aFormatCtx->nb_streams; ++aStreamId) {
                if(stAV::getCodecType(aFormatCtx->streams[aStreamId]) == AVMEDIA_TYPE_AUDIO) {
                    if(aCounter == anActiveStreamId) {
                        myAudio->init(aFormatCtx, aStreamId, myFileList[aCtxId]);
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
    theAVPackets.clear();
    theQueueIsFull.clear();
    theEmptyQueues = 0;
    for(size_t aCtxId = 0; aCtxId < myCtxList.size(); ++aCtxId) {
        AVFormatContext* aFormatCtx = myCtxList[aCtxId];
        if(!myVideoMaster->isInContext(aFormatCtx)
        && !myVideoSlave ->isInContext(aFormatCtx)
        && !myAudio      ->isInContext(aFormatCtx)
        && !mySubtitles1 ->isInContext(aFormatCtx)
        && !mySubtitles2 ->isInContext(aFormatCtx)) {
            continue;
        }
        myPlayCtxList.add(aFormatCtx);
        theAVPackets.push_back(StAVPacket(myCurrParams));
        theQueueIsFull.push_back(false);
    }

    pushPlayEvent(ST_PLAYEVENT_SEEK, aCurrPts);
    if(toPlayNewAudio) {
        myAudio->pushPlayEvent(ST_PLAYEVENT_PLAY);
    }
}

void StVideo::doSwitchSubtitlesStream(std::vector<StAVPacket>& theAVPackets,
                                      std::vector<bool>& theQueueIsFull,
                                      size_t& theEmptyQueues,
                                      const int theIndex) {
    double aCurrPts = getPts();
    doFlushSoft();
    StHandle<StSubtitleQueue>& aSubsQueue = theIndex == 0 ? mySubtitles1 : mySubtitles2;
    if(aSubsQueue->isInitialized()) {
        aSubsQueue->pushEnd();
        while(!aSubsQueue->isEmpty() || !aSubsQueue->isInDowntime()) {
            if(toQuit) {
                myQuitEvent.set();
                break;
            }
            StThread::sleep(10);
        }
        aSubsQueue->deinit();
    }
    size_t anActiveStreamId = size_t(theIndex == 0 ? params.activeSubtitles1->getValue() : params.activeSubtitles2->getValue());
    if(anActiveStreamId != size_t(-1)) {
        size_t aCounter = 0;
        for(size_t aCtxId = 0; aCtxId < myCtxList.size() && !aSubsQueue->isInitialized(); ++aCtxId) {
            AVFormatContext* aFormatCtx = myCtxList[aCtxId];
            for(unsigned int aStreamId = 0; aStreamId < aFormatCtx->nb_streams; ++aStreamId) {
                if(stAV::getCodecType(aFormatCtx->streams[aStreamId]) == AVMEDIA_TYPE_SUBTITLE) {
                    if(aCounter == anActiveStreamId) {
                        aSubsQueue->init(aFormatCtx, aStreamId, myFileList[aCtxId]);
                        aSubsQueue->pushStart();
                        break;
                    }
                    ++aCounter;
                }
            }
        }
    }

    // exclude inactive contexts
    myPlayCtxList.clear();
    theAVPackets.clear();
    theQueueIsFull.clear();
    theEmptyQueues = 0;
    for(size_t aCtxId = 0; aCtxId < myCtxList.size(); ++aCtxId) {
      AVFormatContext* aFormatCtx = myCtxList[aCtxId];
      if(!myVideoMaster->isInContext(aFormatCtx)
      && !myVideoSlave ->isInContext(aFormatCtx)
      && !myAudio      ->isInContext(aFormatCtx)
      && !mySubtitles1 ->isInContext(aFormatCtx)
      && !mySubtitles2 ->isInContext(aFormatCtx)) {
        continue;
      }
      myPlayCtxList.add(aFormatCtx);
      theAVPackets.push_back(StAVPacket(myCurrParams));
      theQueueIsFull.push_back(false);
    }

    pushPlayEvent(ST_PLAYEVENT_SEEK, aCurrPts);
}

void StVideo::packetsLoop() {
#ifdef ST_DEBUG
    double aPtsbar  = 10.0;
#endif
    double aSeekPts = 0.0;
    bool toSeekBack = false;
    StPlayEvent_t aPlayEvent = ST_PLAYEVENT_NONE;

    // wake up threads
    if(myVideoMaster->isInitialized()) { myVideoMaster->pushStart(); }
    if(myVideoSlave->isInitialized())  { myVideoSlave->pushStart(); }
    if(myAudio->isInitialized())       { myAudio->pushStart(); }
    if(mySubtitles1->isInitialized())  { mySubtitles1->pushStart(); }
    if(mySubtitles2->isInitialized())  { mySubtitles2->pushStart(); }

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

    std::vector<StAVPacket> anAVPackets;
    std::vector<bool> aQueueIsFull;
    std::vector<bool> aQueueIsEmpty;
    anAVPackets.reserve(myCtxList.size());
    aQueueIsFull.reserve(myCtxList.size());
    aQueueIsEmpty.reserve(myCtxList.size());
    myPlayCtxList.clear();
    size_t anEmptyQueues = 0;
    for(size_t aCtxId = 0; aCtxId < myCtxList.size(); ++aCtxId) {
        AVFormatContext* aFormatCtx = myCtxList[aCtxId];
        if(!myVideoMaster->isInContext(aFormatCtx)
        && !myVideoSlave->isInContext(aFormatCtx)
        && !myAudio->isInContext(aFormatCtx)
        && !mySubtitles1->isInContext(aFormatCtx)
        && !mySubtitles2->isInContext(aFormatCtx)) {
            continue;
        }

        myPlayCtxList.add(aFormatCtx);
        anAVPackets.push_back(StAVPacket(myCurrParams));
        aQueueIsFull.push_back(false);
        aQueueIsEmpty.push_back(false);
    }

    // reset target FPS
    myEventMutex.lock();
    myTargetFps = 0.0;
    myEventMutex.unlock();

    for(;;) {
        anEmptyQueues = 0;
        for(size_t aCtxId = 0; aCtxId < myPlayCtxList.size(); ++aCtxId) {
            AVFormatContext* aFormatCtx = myPlayCtxList[aCtxId];
            StAVPacket& aPacket = anAVPackets[aCtxId];
            if(!aQueueIsFull[aCtxId]) {
                // read next packet
                if(av_read_frame(aFormatCtx, aPacket.getAVpkt()) < 0) {
                    ++anEmptyQueues;
                    if(!aQueueIsEmpty[aCtxId]) {
                        aQueueIsEmpty[aCtxId] = true;
                        // force decoding of last frame
                        const StAVPacket aDummyLastPacket(myCurrParams, StAVPacket::LAST_PACKET);
                        if(myVideoMaster->isInContext(aFormatCtx)) { myVideoMaster->push(aDummyLastPacket); }
                        if(myVideoSlave ->isInContext(aFormatCtx)) { myVideoSlave ->push(aDummyLastPacket); }
                        if(myAudio      ->isInContext(aFormatCtx)) { myAudio      ->push(aDummyLastPacket); }
                        if(mySubtitles1 ->isInContext(aFormatCtx)) { mySubtitles1 ->push(aDummyLastPacket); }
                        if(mySubtitles2 ->isInContext(aFormatCtx)) { mySubtitles2 ->push(aDummyLastPacket); }
                    }
                    continue;
                }
            }

            // push packet to appropriate queue
            if(myVideoMaster->isInContext(aFormatCtx, aPacket.getStreamId())) {
                aQueueIsFull[aCtxId] = !pushPacket(myVideoMaster, aPacket);
                if(aQueueIsFull[aCtxId]) { continue; }
                const double aTagerFpsNew = myVideoTimer->getAverFps();
                if(myTargetFps != aTagerFpsNew) {
                    myEventMutex.lock();
                    myTargetFps = aTagerFpsNew;
                    myEventMutex.unlock();
                }
            } else if(myVideoSlave->isInContext(aFormatCtx, aPacket.getStreamId())) {
                aQueueIsFull[aCtxId] = !pushPacket(myVideoSlave, aPacket);
                if(aQueueIsFull[aCtxId]) { continue; }
            } else if(myAudio->isInContext(aFormatCtx, aPacket.getStreamId())) {
                aQueueIsFull[aCtxId] = !pushPacket(myAudio, aPacket);
                if(aQueueIsFull[aCtxId]) { continue; }
            } else if(mySubtitles1->isInContext(aFormatCtx, aPacket.getStreamId())) {
                aQueueIsFull[aCtxId] = !pushPacket(mySubtitles1, aPacket);
                if(aQueueIsFull[aCtxId]) { continue; }
            } else if(mySubtitles2->isInContext(aFormatCtx, aPacket.getStreamId())) {
                aQueueIsFull[aCtxId] = !pushPacket(mySubtitles2, aPacket);
                if(aQueueIsFull[aCtxId]) { continue; }
            }
            aPacket.free();
        }

        // check events
        checkInitVideoStreams();

        if(!myVideoTimer.isNull()) {
            myVideoTimer->setAudioDelay(myAudioDelayMSec);
            myVideoTimer->setBenchmark(myIsBenchmark);
        }

        aPlayEvent = popPlayEvent(aSeekPts, toSeekBack);
        if(aPlayEvent == ST_PLAYEVENT_NEXT || toQuit) {
            if(toSave != StImageFile::ST_TYPE_NONE) {
                // save snapshot
                StImageFile::ImageType anImgType = toSave;
                toSave = StImageFile::ST_TYPE_NONE;
                saveSnapshotAs(anImgType);
            } else {
                // load next file
                const double aPts = getPts();
                const double aDur = getDuration();
                if(aPts > 300.0
                && aPts < aDur - 300.0) {
                    myCurrParams->Timestamp = (GLfloat )aPts;
                } else {
                    myCurrParams->Timestamp = 0.0f;
                }

                myPlayList->updateRecent(myCurrPlsFile.isNull() ? myCurrNode : myCurrPlsFile, myCurrParams);
                if(toQuit) {
                    myQuitEvent.set();
                }

                doFlush();
                if(myAudio->isInitialized()) {
                    myAudio->pushPlayEvent(ST_PLAYEVENT_SEEK, 0.0);
                }
                // resend event after it was pop out
                pushPlayEvent(ST_PLAYEVENT_NEXT);
                break;
            }
        } else if(params.activeAudio->wasChanged()) {
            doSwitchAudioStream(anAVPackets, aQueueIsFull, anEmptyQueues);
        } else if(params.activeSubtitles1->wasChanged()) {
            doSwitchSubtitlesStream(anAVPackets, aQueueIsFull, anEmptyQueues, 0);
        } else if(params.activeSubtitles2->wasChanged()) {
            doSwitchSubtitlesStream(anAVPackets, aQueueIsFull, anEmptyQueues, 1);
        } else if(aPlayEvent == ST_PLAYEVENT_SEEK) {
            doSeek(aSeekPts, toSeekBack);
            // ignore current packet
            for(size_t aCtxId = 0; aCtxId < myPlayCtxList.size(); ++aCtxId) {
                aQueueIsFull[aCtxId] = false;
                anAVPackets[aCtxId].free();
            }
        }

        ///
        if(aQueueIsFull[0]) {
            StThread::sleep(2);
        }

    #ifdef ST_DEBUG
        const double aPts = getPts();
        if(aPts > aPtsbar) {
            aPtsbar = aPts + 10.0;
            ST_DEBUG_LOG("Current position: " + StFormatTime::formatSeconds(aPts)
                      + " from "              + StFormatTime::formatSeconds(myDuration));
        }
    #endif

        // All packets sent
        bool isPendingPlayNext = false;
        if(anEmptyQueues == myPlayCtxList.size()) {
            bool areFlushed = false;
            // It seems FFmpeg fail to seek the stream after all packets were read...
            // Thus - we just wait until queues process all packets
            while(!myVideoMaster->isEmpty() || !myVideoMaster->isInDowntime()
               || !myAudio->isEmpty()       || !myAudio->isInDowntime()
               || !myVideoSlave->isEmpty()  || !myVideoSlave->isInDowntime()
               || !mySubtitles1->isEmpty()  || !mySubtitles1->isInDowntime()
               || !mySubtitles2->isEmpty()  || !mySubtitles2->isInDowntime()) {
                StThread::sleep(10);
                if(!areFlushed && (popPlayEvent(aSeekPts, toSeekBack) == ST_PLAYEVENT_NEXT)) {
                    isPendingPlayNext = true;
                    doFlush();
                    if(myAudio->isInitialized()) {
                        myAudio->pushPlayEvent(ST_PLAYEVENT_SEEK, 0.0);
                    }
                    areFlushed = true;
                    isPendingPlayNext = true;
                }
            }
            // If video is played - always wait until audio played
            if(myVideoMaster->isInitialized()) {
                if(myAudio->isInitialized()) {
                    while(myAudio->stalIsAudioPlaying()) {
                        StThread::sleep(10);
                        if(!areFlushed && (popPlayEvent(aSeekPts, toSeekBack) == ST_PLAYEVENT_NEXT)) {
                            isPendingPlayNext = true;
                            doFlush();
                            if(myAudio->isInitialized()) {
                                myAudio->pushPlayEvent(ST_PLAYEVENT_SEEK, 0.0);
                            }
                            areFlushed = true;
                            isPendingPlayNext = true;
                            break;
                        }
                    }
                } else if(myDuration < (double )params.SlideShowDelay->getValue()) {
                    StTimer aDelayTimer;
                    aDelayTimer.restart(myDuration * 1000.0);
                    while(aDelayTimer.getElapsedTimeInSec() < (double )params.SlideShowDelay->getValue()) {
                        StThread::sleep(10);
                        if((popPlayEvent(aSeekPts, toSeekBack) == ST_PLAYEVENT_NEXT)) {
                            isPendingPlayNext = true;
                            break;
                        }
                        const bool isPaused = !isPlaying();
                        if(isPaused) {
                            aDelayTimer.pause();
                        } else {
                            aDelayTimer.resume();
                        }
                    }
                }
            }
            if(isPendingPlayNext) {
                // resend event after it was pop out
                pushPlayEvent(ST_PLAYEVENT_NEXT);
                break;
            }

            // end when any one in format context finished
            myCurrParams->Timestamp = 0.0f;
            break;
        }
    }

    // now send 'end-packet'
    if(myVideoMaster->isInitialized()) { myVideoMaster->pushEnd(); }
    if(myVideoSlave ->isInitialized()) { myVideoSlave ->pushEnd(); }
    if(myAudio      ->isInitialized()) { myAudio      ->pushEnd(); }
    if(mySubtitles1 ->isInitialized()) { mySubtitles1 ->pushEnd(); }
    if(mySubtitles2 ->isInitialized()) { mySubtitles2 ->pushEnd(); }

    // wait for queues receive 'end-packet'
    while(!myVideoMaster->isEmpty() || !myVideoMaster->isInDowntime()
       || !myAudio      ->isEmpty() || !myAudio      ->isInDowntime()
       || !myVideoSlave ->isEmpty() || !myVideoSlave ->isInDowntime()
       || !mySubtitles1 ->isEmpty() || !mySubtitles1 ->isInDowntime()
       || !mySubtitles2 ->isEmpty() || !mySubtitles2 ->isInDowntime()) {
        StThread::sleep(10);
    }
}

bool StVideo::saveSnapshotAs(StImageFile::ImageType theImgType) {
    if(myCurrParams.isNull() || myCurrNode.isNull()) {
        stInfo(myLangMap->getValue(StMoviePlayerStrings::DIALOG_NOTHING_TO_SAVE));
        return false;
    }

    pushPlayEvent(ST_PLAYEVENT_PAUSE);

    StImage dataLeft;
    StImage dataRight;
    int result = StGLTextureQueue::SNAPSHOT_NO_NEW;
    if(!myCurrParams->ToSwapLR) {
        result = myTextureQueue->getSnapshot(&dataLeft, &dataRight, true);
    } else {
        result = myTextureQueue->getSnapshot(&dataRight, &dataLeft, true);
    }

    if(result == StGLTextureQueue::SNAPSHOT_NO_NEW || dataLeft.isNull()) {
        stInfo(myLangMap->getValue(StMoviePlayerStrings::DIALOG_NO_SNAPSHOT));
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

    StOpenFileName anOpenInfo;
    anOpenInfo.Title = myLangMap->getValue(StMoviePlayerStrings::DIALOG_SAVE_SNAPSHOT);
    StString saveExt;
    if(toSaveStereo) {
        switch(theImgType) {
            case StImageFile::ST_TYPE_PNG:
                saveExt = "pns";
                anOpenInfo.Filter.add(StMIME("image/pns", saveExt,
                                             "PNS - png  stereo image, lossless"));
                break;
            case StImageFile::ST_TYPE_JPEG:
                saveExt = "jps";
                anOpenInfo.Filter.add(StMIME("image/jps", saveExt,
                                             "JPS - jpeg stereo image, lossy"));
                break;
            default:
                return false;
        }
    } else {
        switch(theImgType) {
            case StImageFile::ST_TYPE_PNG:
                saveExt = "png";
                anOpenInfo.Filter.add(StMIME("image/png", saveExt,
                                             "PNG image, lossless"));
                break;
            case StImageFile::ST_TYPE_JPEG:
                saveExt = "jpg";
                anOpenInfo.Filter.add(StMIME("image/jpg", saveExt,
                                             "JPEG image, lossy"));
                break;
            default:
                return false;
        }
    }

    StString fileToSave;
    // get path from the first file in case of multi-file input
    anOpenInfo.Folder = myCurrNode->size() >= 2 ? myCurrNode->getValue(0)->getFolderPath() : myCurrNode->getFolderPath();
    if(StFileNode::openFileDialog(fileToSave, anOpenInfo, true)) {
        if(StFileNode::getExtension(fileToSave) != saveExt) {
            fileToSave += StString('.') + saveExt;
        }
        ST_DEBUG_LOG("Save snapshot to the path '" + fileToSave + '\'');
        StImageFile::SaveImageParams aSaveParams;
        aSaveParams.SaveImageType = theImgType;
        aSaveParams.StereoFormat = toSaveStereo ? StFormat_SideBySide_RL : StFormat_AUTO;
        if(!dataResult->save(fileToSave, aSaveParams)) {
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
    if(anInfo.isNull() || anInfo->Id != theParams) {
        return NULL;
    }

    // continuously read source format since it can be stored in frame
    anInfo->StInfoStream   = myVideoMaster->getStereoFormatFromStream();
    anInfo->StInfoFileName = myVideoMaster->getStereoFormatFromName();
    anInfo->HasVideo  = myVideoMaster->isInitialized();

    anInfo->Codecs.clear();
    anInfo->Codecs.add(StArgument("vcodec1",    myVideoMaster->getCodecInfo()));
    anInfo->Codecs.add(StArgument("vcodec2",    myVideoSlave ->getCodecInfo()));
    anInfo->Codecs.add(StArgument("audio",      myAudio      ->getCodecInfo()));
    anInfo->Codecs.add(StArgument("subtitles",  mySubtitles1 ->getCodecInfo()));
    anInfo->Codecs.add(StArgument("subtitles2", mySubtitles2 ->getCodecInfo()));

    return anInfo;
}

void StVideo::doRemovePhysically(const StHandle<StFileNode>& theFile) {
    if(theFile.isNull()
    || theFile->size() != 0) {
        return;
    }

    const StHandle<StFileNode> aCurrent = myPlayList->getCurrentFile();
    const bool toPlayNext = !aCurrent.isNull()
                          && aCurrent->size() == 0
                          && aCurrent->getPath().isEquals(theFile->getPath());

    myEventMutex.lock();
    myFilesToDelete.add(theFile);
    myEventMutex.unlock();

    if(toPlayNext) {
        doLoadNext();
    }
}

void StVideo::mainLoop() {
    bool isOpenSuccess = false;
    StHandle<StFileNode> aFileToLoad, aPlsFile;
    StHandle<StStereoParams> aFileParams;
    double aDummy;
    bool aDummyBool;
    for(;;) {
        // wait for initial message
        waitEvent();
        if(toQuit) {
            close();
            myQuitEvent.set();
            return;
        }

        if(!myPlayList->getCurrentFile(aFileToLoad, aFileParams, aPlsFile)) {
            continue;
        }
        isOpenSuccess = openSource(aFileToLoad, aFileParams, aPlsFile);
        if(isOpenSuccess) {
            break;
        }
    }

    // initial play event
    for(;;) {

        if(myVideoMaster->isInContext(myCtxList[0])) {
            myVideoTimer = new StVideoTimer(myVideoMaster, myAudio,
                1000.0 * av_q2d(myVideoMaster->getCodecContext()->time_base));
            myVideoTimer->setAudioDelay(myAudioDelayMSec);
            myVideoTimer->setBenchmark(myIsBenchmark);
        } else if(myCtxList.size() > 1 && myVideoMaster->isInContext(myCtxList[1])) {
            myVideoTimer = new StVideoTimer(myVideoMaster, myAudio,
                1000.0 * av_q2d(myVideoMaster->getCodecContext()->time_base));
            myVideoTimer->setAudioDelay(myAudioDelayMSec);
            myVideoTimer->setBenchmark(myIsBenchmark);
        } else {
            myVideoTimer.nullify();
        }

        // decoding processed in other threads
        // here we just send packets into queues
        // and manipulate times
        packetsLoop();

        myVideoTimer.nullify();

        myEventMutex.lock();
        if(!myFilesToDelete.isEmpty()) {
            const StHandle<StFileNode> aCurrent = myPlayList->getCurrentFile();
            if(!aCurrent.isNull()
             && aCurrent->getPath().isEquals(myFilesToDelete[0]->getPath())) {
                close();
            }
            for(size_t anIter = 0; anIter < myFilesToDelete.size(); ++anIter) {
                const StHandle<StFileNode>& aNode = myFilesToDelete[anIter];
                if(!myPlayList->removePhysically(aNode)) {
                    signals.onError(StString("File can not be deleted!\n" + aNode->getPath()));
                }
            }
            myFilesToDelete.clear();
        }
        myEventMutex.unlock();

        for(;;) {
            if(popPlayEvent(aDummy, aDummyBool) != ST_PLAYEVENT_NEXT) {
                myPlayList->walkToNext(false);
            }
            if(toQuit) {
                // make sure to close AVIO contexts from the same working thread,
                // because some of them can be attached to specific thread (like StAVIOJniHttpContext to JavaVM)
                close();
                myQuitEvent.set();
                return;
            }
            isOpenSuccess = false;
            if(myPlayList->getCurrentFile(aFileToLoad, aFileParams, aPlsFile)) {
                isOpenSuccess = openSource(aFileToLoad, aFileParams, aPlsFile);
            }
            if(!isOpenSuccess) {
                waitEvent();
            } else {
                break;
            }
        }
    }
}
