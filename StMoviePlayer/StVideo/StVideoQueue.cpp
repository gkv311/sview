/**
 * Copyright © 2009-2011 Kirill Gavrilov <kirill@sview.ru>
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

/*
 * These are called whenever we allocate a frame
 * buffer. We use this to store the global_pts in
 * a frame at the time it is allocated.
 */
static int ourGetBuffer(struct AVCodecContext* codecCtx, AVFrame* pic) {
    StVideoQueue* stVideoQueue = (StVideoQueue* )codecCtx->opaque;
    int ret = avcodec_default_get_buffer(codecCtx, pic);
    int64_t* pts = new int64_t();
    *pts = stVideoQueue->getVideoPktPts();
    pic->opaque = pts;
    return ret;
}

static void ourReleaseBuffer(struct AVCodecContext* codecCtx, AVFrame* pic) {
    if(pic != NULL) {
        delete (int64_t* )pic->opaque;
        pic->opaque = NULL;
    }
    avcodec_default_release_buffer(codecCtx, pic);
}

/**
 * Thread function just call decodeLoop() function.
 */
static SV_THREAD_FUNCTION threadFunction(void* videoQueue) {
    StVideoQueue* stVideoQueue = (StVideoQueue* )videoQueue;
    stVideoQueue->decodeLoop();
    return SV_THREAD_RETURN 0;
}

StVideoQueue::StVideoQueue(const StHandle<StGLTextureQueue>& theTextureQueue,
                           const StHandle<StVideoQueue>&     theMaster)
: StAVPacketQueue(512),
  evDowntime(true),
  myTextureQueue(theTextureQueue),
  evHasData(false),
  myMaster(theMaster),
  mySlave(),
  //
  avDiscard(AVDISCARD_DEFAULT),
  frame(NULL),
  frameRGB(NULL),
  bufferRGB(NULL),
  dataAdp(),
  pToRgbCtx(NULL),
  myFramePts(0.0),
  pixelRatio(1.0f),
  //
  videoClock(0.0),
  //
  videoPktPts(stLibAV::NOPTS_VALUE),
  //
  aClockMutex(),
  aClock(0.0),
  myFramesCounter(1),
  myCachedFrame(),
  mySrcFormat(ST_V_SRC_AUTODETECT),
  mySrcFormatInfo(ST_V_SRC_AUTODETECT),
  toQuit(false) {
    //
    // allocate an AVFrame structure, avfreep() should be called to free memory
    frame    = avcodec_alloc_frame();
    frameRGB = avcodec_alloc_frame();
    myThread = new StThread(threadFunction, (void* )this);
}

StVideoQueue::~StVideoQueue() {
    myTextureQueue->clear();

    toQuit = true;
    pushQuit();

    myThread->wait();
    myThread.nullify();

    deinit();
    stMemFreeAligned(bufferRGB);
    av_free(frame);
    av_free(frameRGB);
}

namespace {

    struct StFFmpegStereoFormat {
        StFormatEnum stID;
        const char*  name;
    };

    static const StFFmpegStereoFormat STEREOFLAGS[] = {
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
        {ST_V_SRC_AUTODETECT, NULL}
    };

};

bool StVideoQueue::init(AVFormatContext*   theFormatCtx,
                        const unsigned int theStreamId) {
    if(!StAVPacketQueue::init(theFormatCtx, theStreamId)
    || myCodecCtx->codec_type != AVMEDIA_TYPE_VIDEO) {
        signals.onError("FFmpeg: invalid stream");
        deinit();
        return false;
    }

    if(frame == NULL || frameRGB == NULL) {
        // should never happens
        signals.onError("FFmpeg: Could not allocate an AVFrame");
        deinit();
        return false;
    }

    // find the decoder for the video stream
    myCodec = avcodec_find_decoder(myCodecCtx->codec_id);
    if(myCodec == NULL) {
        signals.onError("FFmpeg: Video codec not found");
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
        signals.onError("FFmpeg: Could not open video codec");
        deinit();
        return false;
    }

    // determine required frameRGB size and allocate frameRGB
    if(sizeX() == 0 || sizeY() == 0) {
        signals.onError("FFmpeg: Codec return wrong frame size");
        deinit();
        return false;
    }

    size_t numBytes = avpicture_get_size(stLibAV::PIX_FMT::RGB24, sizeX(), sizeY());
    stMemFreeAligned(bufferRGB); bufferRGB = stMemAllocAligned<uint8_t*>(numBytes);

    // assign appropriate parts of frameRGB to image planes in frameRGB
    avpicture_fill((AVPicture* )frameRGB, bufferRGB, stLibAV::PIX_FMT::RGB24,
                   sizeX(), sizeY());

    // reset AVFrame structure
    avcodec_get_frame_defaults(frame);

    if(myCodecCtx->pix_fmt != stLibAV::PIX_FMT::RGB24 && !stLibAV::isFormatYUVPlanar(myCodecCtx)) {
        // initialize software scaler/converter
        pToRgbCtx = sws_getContext(sizeX(), sizeY(), myCodecCtx->pix_fmt,       // source
                                   sizeX(), sizeY(), stLibAV::PIX_FMT::RGB24, // destination
                                   SWS_BICUBIC, NULL, NULL, NULL);
        if(pToRgbCtx == NULL) {
            signals.onError("FFmpeg: Failed to create SWScaler context");
            deinit();
            return false;
        }
    }

    // compute PAR
    if(myStream->sample_aspect_ratio.num && av_cmp_q(myStream->sample_aspect_ratio, myStream->codec->sample_aspect_ratio)) {
        pixelRatio = GLfloat(myStream->sample_aspect_ratio.num) / GLfloat(myStream->sample_aspect_ratio.den);
    } else {
        if(myCodecCtx->sample_aspect_ratio.num == 0 ||
           myCodecCtx->sample_aspect_ratio.den == 0) {
            pixelRatio = 1.0f;
        } else {
            pixelRatio = GLfloat(myCodecCtx->sample_aspect_ratio.num) / GLfloat(myCodecCtx->sample_aspect_ratio.den);
        }
    }

    // read stereoscopic tags if available
    mySrcFormatInfo = ST_V_SRC_AUTODETECT;
#if(LIBAVUTIL_VERSION_INT >= AV_VERSION_INT(51, 5, 0))
    AVDictionaryEntry* aTag = av_dict_get(myStream->metadata, "STEREO_MODE", NULL, 0);
#else
    AVMetadataTag* aTag = av_metadata_get(myStream->metadata, "STEREO_MODE", NULL, 0);
#endif
    if(aTag == NULL) {
    #if(LIBAVUTIL_VERSION_INT >= AV_VERSION_INT(51, 5, 0))
        aTag =     av_dict_get(myFormatCtx->metadata, "STEREO_MODE", NULL, 0);
    #else
        aTag = av_metadata_get(myFormatCtx->metadata, "STEREO_MODE", NULL, 0);
    #endif
    }
    if(aTag != NULL) {
        for(size_t aSrcId = 0;; ++aSrcId) {
            const StFFmpegStereoFormat& aFlag = STEREOFLAGS[aSrcId];
            if(aFlag.stID == ST_V_SRC_AUTODETECT || aFlag.name == NULL) {
                break;
            } else if(stAreEqual(aTag->value, aFlag.name, strlen(aFlag.name))) {
                mySrcFormatInfo = aFlag.stID;
                break;
            }
        }
    }

    // override buffers' functions for getting true PTS rootines
    myCodecCtx->opaque = this;
    myCodecCtx->get_buffer = ourGetBuffer;
    myCodecCtx->release_buffer = ourReleaseBuffer;
    return true;
}

void StVideoQueue::deinit() {
    if(myMaster.isNull()) {
        myTextureQueue->clear();
    }
    mySlave.nullify();
    pixelRatio = 1.0f;

    stMemFreeAligned(bufferRGB); bufferRGB = NULL;
    dataAdp.nullify();

    if(pToRgbCtx != NULL) {
        // TODO (Kirill Gavrilov#5#) we got random crashes with this call
        ///sws_freeContext(pToRgbCtx);
    }
    myFramesCounter = 1;
    myCachedFrame.nullify();

    StAVPacketQueue::deinit();
}

void StVideoQueue::syncVideo(AVFrame* srcFrame, double* pts) {
    if(*pts != 0.0) {
        // if we have pts, set video clock to it
        videoClock = *pts;
    } else {
        // if we aren't given a pts, set it to the clock
        *pts = videoClock;
    }

    // update the video clock
    double frameDelay = av_q2d(myCodecCtx->time_base);
    // if we are repeating a frame, adjust clock accordingly
    frameDelay += srcFrame->repeat_pict * (frameDelay * 0.5);
    videoClock += frameDelay;
}

void StVideoQueue::decodeLoop() {
    int isFrameFinished = 0;
    double averDelaySec = 40.0;
    double prevPts = 0.0;
    double slavePts = 0.0;
    myFramePts = 0.0;
    StImage* slaveData = NULL;
    StHandle<StAVPacket> aPacket;
    StImage anEmptyImg;
    for(;;) {
        if(isEmpty()) {
            evDowntime.set();
            StThread::sleep(10);
            continue;
        }
        evDowntime.reset();

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
                aClock = 0.0;
                videoClock = 0.0;
                continue;
            }
            case StAVPacket::START_PACKET: {
                aClock = 0.0;
                videoClock = 0.0;
                evHasData.reset();
                continue;
            }
            case StAVPacket::END_PACKET: {
                if(!myMaster.isNull()) {
                    while(evHasData.check() && !myMaster->isInDowntime()) {
                        StThread::sleep(10);
                    }
                    // wake up Master
                    dataAdp.nullify();
                    evHasData.set();
                } else {
                    if(!mySlave.isNull()) {
                        mySlave->unlockData();
                    }
                    StTimer stTimerWaitEmpty(true);
                    double waitTime = averDelaySec * myTextureQueue->getSize() + 0.1;
                    while(!myTextureQueue->isEmpty() && stTimerWaitEmpty.getElapsedTimeInSec() < waitTime && !toQuit) {
                        StThread::sleep(2);
                    }
                }
                if(toQuit) {
                    return;
                }
                continue;
            }
            case StAVPacket::QUIT_PACKET: {
                return;
            }
        }

        // wait master retrieve previous data
        while(!myMaster.isNull() && evHasData.check()) {
            //
        }

        // Save global pts to be stored in pFrame in first call
        videoPktPts = aPacket->getPts();

        if(aPacket->getDts() == stLibAV::NOPTS_VALUE
           && frame->opaque && *(int64_t* )frame->opaque != stLibAV::NOPTS_VALUE) {
            myFramePts = double(*(int64_t* )frame->opaque);
        } else if(aPacket->getDts() != stLibAV::NOPTS_VALUE) {
            myFramePts = double(aPacket->getDts());
        } else {
            myFramePts = 0.0;
        }
        myFramePts *= av_q2d(myStream->time_base);
        myFramePts -= myPtsStartBase; // normalize PTS

        syncVideo(frame, &myFramePts);
        double delay = myFramePts - prevPts;
        if(delay > 0.0 && delay < 1.0) {
            averDelaySec = delay;
        }
        prevPts = myFramePts;

        // do we need to skip frames or not
        static const double OVERR_LIMIT = 0.2;
        static const double GREATER_LIMIT = 100.0;
        if(myMaster.isNull()) {
            double diff = getAClock() - myFramePts;
            if(diff > OVERR_LIMIT && diff < GREATER_LIMIT) {
                if(avDiscard != AVDISCARD_NONREF) {
                    ST_DEBUG_LOG("skip frames: AVDISCARD_NONREF (on)"
                        + " (aClock " + getAClock()
                        + " vClock " + myFramePts +
                        + " diff " + diff + ")"
                    );
                    avDiscard = AVDISCARD_NONREF;
                    ///myCodecCtx->skip_frame = AVDISCARD_NONKEY;
                    myCodecCtx->skip_frame = avDiscard;
                    if(!mySlave.isNull()) {
                        mySlave->myCodecCtx->skip_frame = avDiscard;
                    }
                }
            } else {
                if(avDiscard != AVDISCARD_DEFAULT) {
                    ST_DEBUG_LOG("skip frames: AVDISCARD_DEFAULT (off)");
                    avDiscard = AVDISCARD_DEFAULT;
                    myCodecCtx->skip_frame = avDiscard;
                    if(!mySlave.isNull()) {
                        mySlave->myCodecCtx->skip_frame = avDiscard;
                    }
                }
            }
        }

        // decode video frame
    #if(LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(52, 23, 0))
        avcodec_decode_video2(myCodecCtx, frame, &isFrameFinished, aPacket->getAVpkt());
    #else
        avcodec_decode_video(myCodecCtx, frame, &isFrameFinished,
                             aPacket->getData(), aPacket->getSize());
    #endif

        // did we get a video frame?
        if(isFrameFinished != 0) {
            if(aPacket->isKeyFrame()) {
                myFramesCounter = 1;
            }

            size_t aWidthY, aHeightY, aWidthU, aHeightU, aWidthV, aHeightV;
            bool isFullScale = false;

            // we currently allow to override source format stored in metadata
            StFormatEnum aSrcFormat = (mySrcFormat == ST_V_SRC_AUTODETECT) ? mySrcFormatInfo : mySrcFormat;

            if(myCodecCtx->pix_fmt == stLibAV::PIX_FMT::RGB24) {
                dataAdp.setColorModel(StImage::ImgColor_RGB);
                dataAdp.setPixelRatio(getPixelRatio());
                dataAdp.changePlane(0).initWrapper(StImagePlane::ImgRGB, frame->data[0],
                                                   sizeX(), sizeY(),
                                                   frame->linesize[0]);
            } else if(stLibAV::isFormatYUVPlanar(myCodecCtx,
                                                 aWidthY, aHeightY,
                                                 aWidthU, aHeightU,
                                                 aWidthV, aHeightV,
                                                 isFullScale)) {

                /// TODO (Kirill Gavrilov#5) remove hack
                // workaround for incorrect frame dimensions information in some files
                // critical for tiled source format that should be 1080p
                if(aSrcFormat == ST_V_SRC_TILED_4X
                && myCodecCtx->pix_fmt == stLibAV::PIX_FMT::YUV420P
                && myCodecCtx->width >= 1906 && myCodecCtx->width <= 1920
                && frame->linesize[0] >= 1920
                && myCodecCtx->height >= 1074) {
                    aWidthY  = 1920;
                    aHeightY = 1080;
                    aWidthU  = aWidthV  = aWidthY  / 2;
                    aHeightU = aHeightV = aHeightY / 2;
                }

                dataAdp.setColorModel(isFullScale ? StImage::ImgColor_YUVjpeg : StImage::ImgColor_YUV);
                dataAdp.setPixelRatio(getPixelRatio());
                dataAdp.changePlane(0).initWrapper(StImagePlane::ImgGray, frame->data[0],
                                                   aWidthY, aHeightY, frame->linesize[0]);
                dataAdp.changePlane(1).initWrapper(StImagePlane::ImgGray, frame->data[1],
                                                   aWidthU, aHeightU, frame->linesize[1]);
                dataAdp.changePlane(2).initWrapper(StImagePlane::ImgGray, frame->data[2],
                                                   aWidthV, aHeightV, frame->linesize[2]);
            } else {
                sws_scale(pToRgbCtx,
                          frame->data, frame->linesize,
                          0, myCodecCtx->height,
                          frameRGB->data, frameRGB->linesize);

                dataAdp.setColorModel(StImage::ImgColor_RGB);
                dataAdp.setPixelRatio(getPixelRatio());
                dataAdp.changePlane(0).initWrapper(StImagePlane::ImgRGB, bufferRGB,
                                                   size_t(sizeX()), size_t(sizeY()));
            }

            if(!mySlave.isNull()) {
                for(;;) {
                    // wait data from Slave
                    if(slaveData == NULL) {
                        slaveData = mySlave->waitData(slavePts);
                    }
                    if(slaveData != NULL) {
                        double ptsDiff = myFramePts - slavePts;
                        if(ptsDiff > 0.5 * averDelaySec) {
                            // wait for more recent frame from slave thread
                            mySlave->unlockData();
                            slaveData = NULL;
                            StThread::sleep(10);
                            continue;
                        } else if(ptsDiff < -0.5 * averDelaySec) {
                            // too far...
                            if(ptsDiff < -6.0) {
                                // result of seeking?
                                mySlave->unlockData();
                                slaveData = NULL;
                            }
                            //ST_DEBUG_LOG("ptsDiff= " + ptsDiff + "; averDelaySec= " + averDelaySec + "; framePts= " + myFramePts + "; slavePts= " + slavePts);
                            break;
                        }

                        while(myTextureQueue->isFull()) { StThread::sleep(10); }
                        myTextureQueue->push(dataAdp, *slaveData, aPacket->getSource(), ST_V_SRC_SEPARATE_FRAMES, myFramePts);

                        slaveData = NULL;
                        mySlave->unlockData();
                    } else {
                        while(myTextureQueue->isFull()) { StThread::sleep(10); }
                        myTextureQueue->push(dataAdp, anEmptyImg, aPacket->getSource(), aSrcFormat, myFramePts);
                    }
                    break;
                }
            } else if(!myMaster.isNull()) {
                // push data to Master
                evHasData.set();
            } else {
                // simple one-stream case
                if(aSrcFormat == ST_V_SRC_PAGE_FLIP) {
                    if(isOddNumber(myFramesCounter)) {
                        myCachedFrame.fill(dataAdp);
                    } else {
                        while(myTextureQueue->isFull()) { StThread::sleep(10); }
                        myTextureQueue->push(myCachedFrame, dataAdp, aPacket->getSource(), ST_V_SRC_SEPARATE_FRAMES, myFramePts);
                    }
                    ++myFramesCounter;
                } else {
                    while(myTextureQueue->isFull()) { StThread::sleep(10); }
                    myTextureQueue->push(dataAdp, anEmptyImg, aPacket->getSource(), aSrcFormat, myFramePts);
                }
            }
        }
        aPacket.nullify(); // and now packet finished
    }
}
