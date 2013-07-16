/**
 * Copyright © 2009-2013 Kirill Gavrilov <kirill@sview.ru>
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

#include "StVideoQueue.h"

#include <StStrings/StStringStream.h>
#include <StThreads/StThread.h>

#if (defined(_WIN64) || defined(__WIN64__))\
 || (defined(_LP64)  || defined(__LP64__))
    #define ST_USE64PTR
#endif

namespace {

#if(LIBAVCODEC_VERSION_INT < AV_VERSION_INT(54, 18, 100))
    /*
     * These are called whenever we allocate a frame
     * buffer. We use this to store the global_pts in
     * a frame at the time it is allocated.
     */
    static int ourGetBuffer(AVCodecContext* theCodecCtx,
                            AVFrame*        theFrame) {
        StVideoQueue* aVideoQueue = (StVideoQueue* )theCodecCtx->opaque;
        const int aResult = avcodec_default_get_buffer(theCodecCtx, theFrame);
    #ifdef ST_USE64PTR
        theFrame->opaque = (void* )aVideoQueue->getVideoPktPts();
    #else
        int64_t* aPts = new int64_t();
        *aPts = aVideoQueue->getVideoPktPts();
        theFrame->opaque = aPts;
    #endif
        return aResult;
    }

    static void ourReleaseBuffer(AVCodecContext* theCodecCtx,
                                 AVFrame*        theFrame) {
        if(theFrame != NULL) {
        #ifdef ST_USE64PTR
            theFrame->opaque = (void* )stLibAV::NOPTS_VALUE;
        #else
            delete (int64_t* )theFrame->opaque;
            theFrame->opaque = NULL;
        #endif
        }
        avcodec_default_release_buffer(theCodecCtx, theFrame);
    }
#endif

    /**
     * Thread function just call decodeLoop() function.
     */
    static SV_THREAD_FUNCTION threadFunction(void* theVideoQueue) {
        StVideoQueue* aVideoQueue = (StVideoQueue* )theVideoQueue;
        aVideoQueue->decodeLoop();
        return SV_THREAD_RETURN 0;
    }

};

StVideoQueue::StVideoQueue(const StHandle<StGLTextureQueue>& theTextureQueue,
                           const StHandle<StVideoQueue>&     theMaster)
: StAVPacketQueue(512),
  myDowntimeState(true),
  myTextureQueue(theTextureQueue),
  myHasDataState(false),
  myMaster(theMaster),
  //
  myAvDiscard(AVDISCARD_DEFAULT),
  myFrame(NULL),
  myFrameRGB(NULL),
  myBufferRGB(NULL),
  myToRgbCtx(NULL),
  myFramePts(0.0),
  myPixelRatio(1.0f),
  //
  myVideoClock(0.0),
  //
  myVideoPktPts(stLibAV::NOPTS_VALUE),
  //
  myAudioClock(0.0),
  myFramesCounter(1),
  myWasFlushed(false),
  mySrcFormat(ST_V_SRC_AUTODETECT),
  mySrcFormatInfo(ST_V_SRC_AUTODETECT) {
    // allocate an AVFrame structure, avfreep() should be called to free memory
    myFrame    = avcodec_alloc_frame();
    myFrameRGB = avcodec_alloc_frame();
#ifdef ST_USE64PTR
    myFrame->opaque = (void* )stLibAV::NOPTS_VALUE;
#else
    myFrame->opaque = NULL;
#endif
    myThread   = new StThread(threadFunction, (void* )this);
}

StVideoQueue::~StVideoQueue() {
    myTextureQueue->clear();

    myToQuit = true;
    pushQuit();

    myThread->wait();
    myThread.nullify();

    deinit();
    stMemFreeAligned(myBufferRGB);
    av_free(myFrame);
    av_free(myFrameRGB);
}

namespace {

    struct StFFmpegStereoFormat {
        StFormatEnum stID;
        const char*  name;
    };

    static const StFFmpegStereoFormat STEREOFLAGS[] = {
        // MKV stereoscopic mode decoded by FFmpeg into STEREO_MODE metadata tag
        {ST_V_SRC_MONO,               "mono"},
        {ST_V_SRC_SIDE_BY_SIDE,       "right_left"},
        {ST_V_SRC_PARALLEL_PAIR,      "left_right"},
        {ST_V_SRC_OVER_UNDER_RL,      "bottom_top"},
        {ST_V_SRC_OVER_UNDER_LR,      "top_bottom"},
        {ST_V_SRC_ROW_INTERLACE,      "row_interleaved_rl"},
        {ST_V_SRC_ROW_INTERLACE,      "row_interleaved_lr"},
        {ST_V_SRC_VERTICAL_INTERLACE, "col_interleaved_rl"},
        {ST_V_SRC_VERTICAL_INTERLACE, "col_interleaved_lr"},
        {ST_V_SRC_PAGE_FLIP,          "block_lr"}, /// ???
        {ST_V_SRC_PAGE_FLIP,          "block_rl"},
        {ST_V_SRC_ANAGLYPH_RED_CYAN,  "anaglyph_cyan_red"},
        {ST_V_SRC_ANAGLYPH_G_RB,      "anaglyph_green_magenta"},
        // values in WMV StereoscopicLayout tag
        {ST_V_SRC_SIDE_BY_SIDE,       "SideBySideRF"}, // Right First
        {ST_V_SRC_PARALLEL_PAIR,      "SideBySideLF"},
        {ST_V_SRC_OVER_UNDER_LR,      "OverUnderLT"},  // Left Top
        {ST_V_SRC_OVER_UNDER_RL,      "OverUnderRT"},
        // NULL-terminate array
        {ST_V_SRC_AUTODETECT, NULL}
    };

};

bool StVideoQueue::init(AVFormatContext*   theFormatCtx,
                        const unsigned int theStreamId) {
    if(!StAVPacketQueue::init(theFormatCtx, theStreamId)
    || myCodecCtx->codec_type != AVMEDIA_TYPE_VIDEO) {
        signals.onError(stCString("FFmpeg: invalid stream"));
        deinit();
        return false;
    }

    if(myFrame == NULL || myFrameRGB == NULL) {
        // should never happens
        signals.onError(stCString("FFmpeg: Could not allocate an AVFrame"));
        deinit();
        return false;
    }

    // find the decoder for the video stream
    myCodec = avcodec_find_decoder(myCodecCtx->codec_id);
    if(myCodec == NULL) {
        signals.onError(stCString("FFmpeg: Video codec not found"));
        deinit();
        return false;
    }

    // configure the codec
    //myCodecCtx->debug_mv = debug_mv;
    //myCodecCtx->debug = debug;
    //myCodecCtx->workaround_bugs = workaround_bugs;
    //myCodecCtx->lowres = 1;
    //if(lowres) myCodecCtx->flags |= CODEC_FLAG_EMU_EDGE;
    //myCodecCtx->idct_algo= idct;
    //if(fast) myCodecCtx->flags2 |= CODEC_FLAG2_FAST;
    //myCodecCtx->skip_idct= skip_idct;
    //myCodecCtx->skip_loop_filter= skip_loop_filter;
    //myCodecCtx->error_recognition= error_recognition;
    //myCodecCtx->error_concealment= error_concealment;
    int threadsCount = StThread::countLogicalProcessors();
    myCodecCtx->thread_count = threadsCount;
#if(LIBAVCODEC_VERSION_INT < AV_VERSION_INT(52, 112, 0))
    avcodec_thread_init(myCodecCtx, threadsCount);
#endif
    ST_DEBUG_LOG("FFmpeg: Setup AVcodec to use " + threadsCount + " threads");

    // open VIDEO codec
#if(LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(53, 8, 0))
    if(avcodec_open2(myCodecCtx, myCodec, NULL) < 0) {
#else
    if(avcodec_open(myCodecCtx, myCodec) < 0) {
#endif
        signals.onError(stCString("FFmpeg: Could not open video codec"));
        deinit();
        return false;
    }

    // determine required myFrameRGB size and allocate it
    if(sizeX() == 0 || sizeY() == 0) {
        signals.onError(stCString("FFmpeg: Codec return wrong frame size"));
        deinit();
        return false;
    }

    // reset AVFrame structure
    avcodec_get_frame_defaults(myFrame);

    if(myCodecCtx->pix_fmt != stLibAV::PIX_FMT::RGB24
    && !stLibAV::isFormatYUVPlanar(myCodecCtx)) {
        // initialize software scaler/converter
        myToRgbCtx = sws_getContext(sizeX(), sizeY(), myCodecCtx->pix_fmt,       // source
                                    sizeX(), sizeY(), stLibAV::PIX_FMT::RGB24, // destination
                                    SWS_BICUBIC, NULL, NULL, NULL);
        if(myToRgbCtx == NULL) {
            signals.onError(stCString("FFmpeg: Failed to create SWScaler context"));
            deinit();
            return false;
        }

        // assign appropriate parts of myFrameRGB to image planes in myFrameRGB
        const size_t aBufferSize = avpicture_get_size(stLibAV::PIX_FMT::RGB24, sizeX(), sizeY());
        stMemFreeAligned(myBufferRGB); myBufferRGB = stMemAllocAligned<uint8_t*>(aBufferSize);
        if(myBufferRGB == NULL) {
            signals.onError(stCString("FFmpeg: Failed allocation of RGB frame"));
            deinit();
            return false;
        }
        avpicture_fill((AVPicture* )myFrameRGB, myBufferRGB, stLibAV::PIX_FMT::RGB24,
                       sizeX(), sizeY());
    }

    // compute PAR
    if(myStream->sample_aspect_ratio.num && av_cmp_q(myStream->sample_aspect_ratio, myStream->codec->sample_aspect_ratio)) {
        myPixelRatio = GLfloat(myStream->sample_aspect_ratio.num) / GLfloat(myStream->sample_aspect_ratio.den);
    } else {
        if(myCodecCtx->sample_aspect_ratio.num == 0 ||
           myCodecCtx->sample_aspect_ratio.den == 0) {
            myPixelRatio = 1.0f;
        } else {
            myPixelRatio = GLfloat(myCodecCtx->sample_aspect_ratio.num) / GLfloat(myCodecCtx->sample_aspect_ratio.den);
        }
    }

    // special WMV tags
    StString aValue;
    const StString aHalfHeightKeyWMV  = "StereoscopicHalfHeight";
    const StString aHalfWidthKeyWMV   = "StereoscopicHalfWidth";
    const StString aHorParallaxKeyWMV = "StereoscopicHorizontalParallax";
    if(stLibAV::meta::readTag(myFormatCtx, aHalfHeightKeyWMV, aValue)) {
        if(aValue == "1") {
            myPixelRatio *= 0.5;
        }
    } else if(stLibAV::meta::readTag(myFormatCtx, aHalfWidthKeyWMV, aValue)) {
        if(aValue == "1") {
            myPixelRatio *= 2.0;
        }
    }
    myHParallax = 0;
    if(stLibAV::meta::readTag(myFormatCtx, aHorParallaxKeyWMV, aValue)) {
        StCLocale aCLocale;
        myHParallax = (int )stStringToLong(aValue.toCString(), 10, aCLocale);
    }

    // stereoscopic mode tags
    mySrcFormatInfo = ST_V_SRC_AUTODETECT;
    const StString aSrcModeKeyMKV = "STEREO_MODE";
    const StString aSrcModeKeyWMV = "StereoscopicLayout";
    if(stLibAV::meta::readTag(myFormatCtx, aSrcModeKeyMKV, aValue)
    || stLibAV::meta::readTag(myStream,    aSrcModeKeyMKV, aValue)
    || stLibAV::meta::readTag(myFormatCtx, aSrcModeKeyWMV, aValue)) {
        for(size_t aSrcId = 0;; ++aSrcId) {
            const StFFmpegStereoFormat& aFlag = STEREOFLAGS[aSrcId];
            if(aFlag.stID == ST_V_SRC_AUTODETECT || aFlag.name == NULL) {
                break;
            } else if(aValue == aFlag.name) {
                mySrcFormatInfo = aFlag.stID;
                //ST_DEBUG_LOG("  read srcFormat from tags= " + mySrcFormatInfo);
                break;
            }
        }
    }

#if(LIBAVCODEC_VERSION_INT < AV_VERSION_INT(54, 18, 100))
    // override buffers' functions for getting true PTS rootines
    myCodecCtx->opaque = this;
    myCodecCtx->get_buffer = ourGetBuffer;
    myCodecCtx->release_buffer = ourReleaseBuffer;
#endif
    return true;
}

void StVideoQueue::deinit() {
    if(myMaster.isNull()) {
        myTextureQueue->clear();
        myTextureQueue->setConnectedStream(false);
    }
    mySlave.nullify();
    myPixelRatio = 1.0f;

    stMemFreeAligned(myBufferRGB); myBufferRGB = NULL;
    myDataAdp.nullify();

    if(myToRgbCtx != NULL) {
        // TODO (Kirill Gavrilov#5#) we got random crashes with this call
        ///sws_freeContext(myToRgbCtx);
    }
    myFramesCounter = 1;
    myCachedFrame.nullify();

    StAVPacketQueue::deinit();
}

#if(LIBAVCODEC_VERSION_INT < AV_VERSION_INT(54, 18, 100))
void StVideoQueue::syncVideo(AVFrame* theSrcFrame,
                             double*  thePts) {
    if(*thePts != 0.0) {
        // if we have pts, set video clock to it
        myVideoClock = *thePts;
    } else {
        // if we aren't given a pts, set it to the clock
        *thePts = myVideoClock;
    }

    // update the video clock
    double aFrameDelay = av_q2d(myCodecCtx->time_base);
    // if we are repeating a frame, adjust clock accordingly
    aFrameDelay  += theSrcFrame->repeat_pict * (aFrameDelay * 0.5);
    myVideoClock += aFrameDelay;
}
#endif

void StVideoQueue::pushFrame(const StImage&     theSrcDataLeft,
                             const StImage&     theSrcDataRight,
                             const StHandle<StStereoParams>& theStParams,
                             const StFormatEnum theSrcFormat,
                             const double       theSrcPTS) {
    while(!myToFlush && myTextureQueue->isFull()) {
        StThread::sleep(10);
    }

    if(myToFlush) {
        myToFlush = false;
        return;
    }

    myTextureQueue->push(theSrcDataLeft, theSrcDataRight, theStParams, theSrcFormat, theSrcPTS);
    myTextureQueue->setConnectedStream(true);
    if(myWasFlushed) {
        // force frame update after seeking regardless playback timer
        myTextureQueue->stglSwapFB(0);
        myWasFlushed = false;
    }
}

void StVideoQueue::decodeLoop() {
    int isFrameFinished = 0;
    double anAverageDelaySec = 40.0;
    double aPrevPts  = 0.0;
    double aSlavePts = 0.0;
    myFramePts = 0.0;
    StImage* aSlaveData = NULL;
    StHandle<StAVPacket> aPacket;
    StImage anEmptyImg;
    bool isStarted   = false;
    stLibAV::dimYUV aDimsYUV;
    for(;;) {
        if(isEmpty()) {
            myDowntimeState.set();
            StThread::sleep(10);
            continue;
        }
        myDowntimeState.reset();

        aPacket = pop();
        if(aPacket.isNull()) {
            continue;
        }
        switch(aPacket->getType()) {
            case StAVPacket::FLUSH_PACKET: {
                // got the special FLUSH packet - flush FFMPEG codec buffers
                if(myCodecCtx != NULL && myCodec != NULL) {
                    avcodec_flush_buffers(myCodecCtx);
                }
                // now we clear our sttextures buffer
                if(myMaster.isNull()) {
                    myTextureQueue->clear();
                }
                myAudioClock = 0.0;
                myVideoClock = 0.0;
                myToFlush    = false;
                myWasFlushed = true;
                continue;
            }
            case StAVPacket::START_PACKET: {
                myAudioClock = 0.0;
                myVideoClock = 0.0;
                myHasDataState.reset();
                isStarted = true;
                continue;
            }
            case StAVPacket::END_PACKET: {
                if(!myMaster.isNull()) {
                    while(myHasDataState.check() && !myMaster->isInDowntime()) {
                        StThread::sleep(10);
                    }
                    // wake up Master
                    myDataAdp.nullify();
                    myHasDataState.set();
                } else {
                    if(!mySlave.isNull()) {
                        mySlave->unlockData();
                    }
                    StTimer stTimerWaitEmpty(true);
                    double waitTime = anAverageDelaySec * myTextureQueue->getSize() + 0.1;
                    while(!myTextureQueue->isEmpty() && stTimerWaitEmpty.getElapsedTimeInSec() < waitTime && !myToQuit) {
                        StThread::sleep(2);
                    }
                }
                if(myToQuit) {
                    return;
                }
                continue;
            }
            case StAVPacket::QUIT_PACKET: {
                return;
            }
        }

        // wait master retrieve previous data
        while(!myMaster.isNull() && myHasDataState.check()) {
            //
        }

        // decode video frame
    #if(LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(52, 23, 0))
        avcodec_decode_video2(myCodecCtx, myFrame, &isFrameFinished, aPacket->getAVpkt());
    #else
        avcodec_decode_video(myCodecCtx, myFrame, &isFrameFinished,
                             aPacket->getData(), aPacket->getSize());
    #endif
        if(isFrameFinished == 0) {
            // need more packets to decode whole frame
            aPacket.nullify();
            continue;
        }

        if(aPacket->isKeyFrame()) {
            myFramesCounter = 1;
        }

    #if(LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(54, 18, 100))
        myVideoPktPts = av_frame_get_best_effort_timestamp(myFrame);
        if(myVideoPktPts == stLibAV::NOPTS_VALUE) {
            myVideoPktPts = 0;
        }

        myFramePts  = double(myVideoPktPts) * av_q2d(myStream->time_base);
        myFramePts -= myPtsStartBase; // normalize PTS
    #else
        // Save global pts to be stored in pFrame in first call
        myVideoPktPts = aPacket->getPts();

    #ifdef ST_USE64PTR
        if(aPacket->getDts() == stLibAV::NOPTS_VALUE
        && (int64_t )myFrame->opaque != stLibAV::NOPTS_VALUE) {
            myFramePts = double((int64_t )myFrame->opaque);
    #else
        if(aPacket->getDts() == stLibAV::NOPTS_VALUE
        && myFrame->opaque != NULL
        && *(int64_t* )myFrame->opaque != stLibAV::NOPTS_VALUE) {
            myFramePts = double(*(int64_t* )myFrame->opaque);
    #endif
        } else if(aPacket->getDts() != stLibAV::NOPTS_VALUE) {
            myFramePts = double(aPacket->getDts());
        } else {
            myFramePts = 0.0;
        }
        myFramePts *= av_q2d(myStream->time_base);
        myFramePts -= myPtsStartBase; // normalize PTS

        syncVideo(myFrame, &myFramePts);
    #endif

        const double aDelay = myFramePts - aPrevPts;
        if(aDelay > 0.0 && aDelay < 1.0) {
            anAverageDelaySec = aDelay;
        }
        aPrevPts = myFramePts;

        // do we need to skip frames or not
        static const double OVERR_LIMIT = 0.2;
        static const double GREATER_LIMIT = 100.0;
        if(myMaster.isNull()) {
            const double anAudioClock = getAClock();
            double diff = anAudioClock - myFramePts;
            if(diff > OVERR_LIMIT && diff < GREATER_LIMIT) {
                if(myAvDiscard != AVDISCARD_NONREF) {
                    ST_DEBUG_LOG("skip frames: AVDISCARD_NONREF (on)"
                        + " (aClock " + anAudioClock
                        + " vClock " + myFramePts
                        + " diff " + diff + ")"
                    );
                    myAvDiscard = AVDISCARD_NONREF;
                    ///myCodecCtx->skip_frame = AVDISCARD_NONKEY;
                    myCodecCtx->skip_frame = myAvDiscard;
                    if(!mySlave.isNull()) {
                        mySlave->myCodecCtx->skip_frame = myAvDiscard;
                    }
                }
            } else {
                if(myAvDiscard != AVDISCARD_DEFAULT) {
                    ST_DEBUG_LOG("skip frames: AVDISCARD_DEFAULT (off)");
                    myAvDiscard = AVDISCARD_DEFAULT;
                    myCodecCtx->skip_frame = myAvDiscard;
                    if(!mySlave.isNull()) {
                        mySlave->myCodecCtx->skip_frame = myAvDiscard;
                    }
                }
            }
        }

        // we currently allow to override source format stored in metadata
        const StFormatEnum aSrcFormat = (mySrcFormat == ST_V_SRC_AUTODETECT) ? mySrcFormatInfo : mySrcFormat;

        int         aFrameSizeX = myCodecCtx->width;
        int         aFrameSizeY = myCodecCtx->height;
        PixelFormat aPixFmt     = myCodecCtx->pix_fmt;
    #if(LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(53, 5, 0))
        aFrameSizeX = myFrame->width;
        aFrameSizeY = myFrame->height;
        aPixFmt     = (PixelFormat )myFrame->format;
    #endif

        if(aPixFmt == stLibAV::PIX_FMT::RGB24) {
            myDataAdp.setColorModel(StImage::ImgColor_RGB);
            myDataAdp.setColorScale(StImage::ImgScale_Full);
            myDataAdp.setPixelRatio(getPixelRatio());
            myDataAdp.changePlane(0).initWrapper(StImagePlane::ImgRGB, myFrame->data[0],
                                                 size_t(aFrameSizeX), size_t(aFrameSizeY),
                                                 myFrame->linesize[0]);
    #if(LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(53, 5, 0))
        } else if(stLibAV::isFormatYUVPlanar(myFrame,
    #else
        } else if(stLibAV::isFormatYUVPlanar(myCodecCtx,
    #endif
                                             aDimsYUV)) {

            /// TODO (Kirill Gavrilov#5) remove hack
            // workaround for incorrect frame dimensions information in some files
            // critical for tiled source format that should be 1080p
            if(aSrcFormat == ST_V_SRC_TILED_4X
            && aPixFmt    == stLibAV::PIX_FMT::YUV420P
            && aFrameSizeX >= 1906 && aFrameSizeX <= 1920
            && myFrame->linesize[0] >= 1920
            && aFrameSizeY >= 1074) {
                aDimsYUV.widthY  = 1920;
                aDimsYUV.heightY = 1080;
                aDimsYUV.widthU  = aDimsYUV.widthV  = aDimsYUV.widthY  / 2;
                aDimsYUV.heightU = aDimsYUV.heightV = aDimsYUV.heightY / 2;
            }

            StImagePlane::ImgFormat aPlaneFrmt = StImagePlane::ImgGray;
        #if(LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(52, 29, 0))
            if(myCodecCtx->color_range == AVCOL_RANGE_JPEG) {
                // there no color range information in the AVframe (yet)
                aDimsYUV.isFullScale = true;
            }
        #endif
            myDataAdp.setColorScale(aDimsYUV.isFullScale ? StImage::ImgScale_Full : StImage::ImgScale_Mpeg);
            if(aDimsYUV.bitsPerComp == 9) {
                aPlaneFrmt = StImagePlane::ImgGray16;
                myDataAdp.setColorScale(aDimsYUV.isFullScale ? StImage::ImgScale_Jpeg9  : StImage::ImgScale_Mpeg9);
            } else if(aDimsYUV.bitsPerComp == 10) {
                aPlaneFrmt = StImagePlane::ImgGray16;
                myDataAdp.setColorScale(aDimsYUV.isFullScale ? StImage::ImgScale_Jpeg10 : StImage::ImgScale_Mpeg10);
            } else if(aDimsYUV.bitsPerComp == 16) {
                aPlaneFrmt = StImagePlane::ImgGray16;
            }
            myDataAdp.setColorModel(StImage::ImgColor_YUV);
            myDataAdp.setPixelRatio(getPixelRatio());
            myDataAdp.changePlane(0).initWrapper(aPlaneFrmt, myFrame->data[0],
                                                 size_t(aDimsYUV.widthY), size_t(aDimsYUV.heightY), myFrame->linesize[0]);
            myDataAdp.changePlane(1).initWrapper(aPlaneFrmt, myFrame->data[1],
                                                 size_t(aDimsYUV.widthU), size_t(aDimsYUV.heightU), myFrame->linesize[1]);
            myDataAdp.changePlane(2).initWrapper(aPlaneFrmt, myFrame->data[2],
                                                 size_t(aDimsYUV.widthV), size_t(aDimsYUV.heightV), myFrame->linesize[2]);
        } else if(myToRgbCtx != NULL) {
            if(aPixFmt     == myCodecCtx->pix_fmt
            && aFrameSizeX == myCodecCtx->width
            && aFrameSizeY == myCodecCtx->height) {
                sws_scale(myToRgbCtx,
                          myFrame->data, myFrame->linesize,
                          0, aFrameSizeY,
                          myFrameRGB->data, myFrameRGB->linesize);

                myDataAdp.setColorModel(StImage::ImgColor_RGB);
                myDataAdp.setColorScale(StImage::ImgScale_Full);
                myDataAdp.setPixelRatio(getPixelRatio());
                myDataAdp.changePlane(0).initWrapper(StImagePlane::ImgRGB, myBufferRGB,
                                                     size_t(aFrameSizeX), size_t(aFrameSizeY));
            }
        }

        if(!mySlave.isNull()) {
            if(isStarted) {
                StHandle<StStereoParams> aParams = aPacket->getSource();
                if(!aParams.isNull()) {
                    aParams->setSeparationNeutral(myHParallax);
                }
                isStarted = false;
            }

            for(;;) {
                // wait data from Slave
                if(aSlaveData == NULL) {
                    aSlaveData = mySlave->waitData(aSlavePts);
                }
                if(aSlaveData != NULL) {
                    const double aPtsDiff = myFramePts - aSlavePts;
                    if(aPtsDiff > 0.5 * anAverageDelaySec) {
                        // wait for more recent frame from slave thread
                        mySlave->unlockData();
                        aSlaveData = NULL;
                        StThread::sleep(10);
                        continue;
                    } else if(aPtsDiff < -0.5 * anAverageDelaySec) {
                        // too far...
                        if(aPtsDiff < -6.0) {
                            // result of seeking?
                            mySlave->unlockData();
                            aSlaveData = NULL;
                        }
                        break;
                    }

                    pushFrame(myDataAdp, *aSlaveData, aPacket->getSource(), ST_V_SRC_SEPARATE_FRAMES, myFramePts);

                    aSlaveData = NULL;
                    mySlave->unlockData();
                } else {
                    pushFrame(myDataAdp, anEmptyImg, aPacket->getSource(), aSrcFormat, myFramePts);
                }
                break;
            }
        } else if(!myMaster.isNull()) {
            // push data to Master
            myHasDataState.set();
        } else {
            if(isStarted) {
                StHandle<StStereoParams> aParams = aPacket->getSource();
                if(!aParams.isNull()) {
                    aParams->setSeparationNeutral(myHParallax);
                }
                isStarted = false;
            }

            // simple one-stream case
            if(aSrcFormat == ST_V_SRC_PAGE_FLIP) {
                if(isOddNumber(myFramesCounter)) {
                    myCachedFrame.fill(myDataAdp);
                } else {
                    pushFrame(myCachedFrame, myDataAdp, aPacket->getSource(), ST_V_SRC_SEPARATE_FRAMES, myFramePts);
                }
                ++myFramesCounter;
            } else {
                pushFrame(myDataAdp, anEmptyImg, aPacket->getSource(), aSrcFormat, myFramePts);
            }
        }

        aPacket.nullify(); // and now packet finished
    }
}
