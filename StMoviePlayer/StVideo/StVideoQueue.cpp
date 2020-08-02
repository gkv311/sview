/**
 * Copyright © 2009-2019 Kirill Gavrilov <kirill@sview.ru>
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

#if(LIBAVCODEC_VERSION_INT < AV_VERSION_INT(55, 0, 0))
    /**
     * Release the frame buffer (old API).
     */
    static void stReleaseFrameBuffer(AVCodecContext* theCodecCtx,
                                     AVFrame*        theFrame) {
    #if defined(ST_AV_OLDSYNC) && !defined(ST_USE64PTR)
        if(theFrame != NULL) {
            delete (int64_t* )theFrame->opaque;
            theFrame->opaque = NULL;
        }
    #endif
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

}

AVPixelFormat StVideoQueue::getFrameFormat(const AVPixelFormat* theFormats) {
    if(!myUseGpu || myIsGpuFailed) {
        return avcodec_default_get_format(myCodecCtx, theFormats);
    }

    for(const AVPixelFormat* aFormatIter = theFormats; *aFormatIter != stAV::PIX_FMT::NONE; ++aFormatIter) {
        /*const AVPixFmtDescriptor* aDesc = av_pix_fmt_desc_get(*aFormatIter);
        if(!(aDesc->flags & AV_PIX_FMT_FLAG_HWACCEL)) {
            return *aFormatIter;
        }*/

    #if defined(_WIN32)
        if(*aFormatIter == stAV::PIX_FMT::DXVA2_VLD) {
            if(!hwaccelInit()) {
                myIsGpuFailed = true;
                return avcodec_default_get_format(myCodecCtx, theFormats);
            }
            return *aFormatIter;
        }
    #elif defined(__APPLE__)
        if(*aFormatIter == stAV::PIX_FMT::VIDEOTOOLBOX_VLD) {
            if(!hwaccelInit()) {
                myIsGpuFailed = true;
                return avcodec_default_get_format(myCodecCtx, theFormats);
            }
            return *aFormatIter;
        }
    #elif defined(__ANDROID__)
        if(*aFormatIter == AV_PIX_FMT_MEDIACODEC) {
            // decoding into surface (like android.graphics.SurfaceTexture) is not yet support
            return *aFormatIter;
        }
    #endif
    }

    return *theFormats;
}

int StVideoQueue::getFrameBuffer(AVFrame* theFrame,
                                 int      theFlags) {
    int aResult = 0;
#if(LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(55, 0, 0))
    bool isDone = false;
    #if defined(_WIN32)
        if(theFrame->format == stAV::PIX_FMT::DXVA2_VLD) {
            if(!myHWAccelCtx.isNull()) {
                aResult = myHWAccelCtx->getFrameBuffer(*this, theFrame);
            } else {
                aResult = -1;
            }
            isDone  = true;
        }
    /*#elif defined(__APPLE__) // standard FFmpeg VideoToolbox wrapper is used - action is not needed
        if(theFrame->format == stAV::PIX_FMT::VIDEOTOOLBOX_VLD) {
            if(!myHWAccelCtx.isNull()) {
                aResult = myHWAccelCtx->getFrameBuffer(*this, theFrame);
            } else {
                aResult = -1;
            }
            isDone  = true;
        }*/
    #endif
        if(!isDone) {
            aResult = avcodec_default_get_buffer2(myCodecCtx, theFrame, theFlags);
        }

    #ifdef ST_AV_OLDSYNC
    #ifdef ST_USE64PTR
        theFrame->opaque = (void* )myVideoPktPts;
    #else
        AVFrameSideData* aSideDataSync = av_frame_new_side_data(theFrame, (AVFrameSideDataType )1000, sizeof(myVideoPktPts));
        if(aSideDataSync != NULL) {
            memcpy(aSideDataSync->data, &myVideoPktPts, sizeof(myVideoPktPts));
        }
    #endif
    #endif
#else
    aResult = avcodec_default_get_buffer(myCodecCtx, theFrame);
    #ifdef ST_AV_OLDSYNC
    #ifdef ST_USE64PTR
        theFrame->opaque = (void* )myVideoPktPts;
    #else
        int64_t* aPts = new int64_t();
        *aPts = myVideoPktPts;
        theFrame->opaque = aPts;
    #endif
    #endif
#endif
    return aResult;
}

#if !defined(_WIN32) && !defined(__APPLE__)
bool StVideoQueue::hwaccelInit() { return false; }
#endif

inline AVCodecID stFindCodecId(const char* theName) {
    AVCodec* aCodec = avcodec_find_decoder_by_name(theName);
    return aCodec != NULL ? aCodec->id : AV_CODEC_ID_NONE;
}

StVideoQueue::StVideoQueue(const StHandle<StGLTextureQueue>& theTextureQueue,
                           const StHandle<StVideoQueue>&     theMaster)
: StAVPacketQueue(512),
  CodecIdH264  (stFindCodecId("h264")),
  CodecIdHEVC  (stFindCodecId("hevc")),
  CodecIdMPEG2 (stFindCodecId("mpeg2video")),
  CodecIdWMV3  (stFindCodecId("wmv3")),
  CodecIdVC1   (stFindCodecId("vc1")),
  CodecIdJpeg2K(stFindCodecId("jpeg2000")),
  myDowntimeState(true),
  myTextureQueue(theTextureQueue),
  myHasDataState(false),
  myMaster(theMaster),
#if defined(__ANDROID__)
  myCodecH264HW(avcodec_find_decoder_by_name("h264_mediacodec")),
  myCodecHevcHW(avcodec_find_decoder_by_name("hevc_mediacodec")),
  myCodecVp9HW (avcodec_find_decoder_by_name("vp9_mediacodec")),
#endif
  myCodecOpenJpeg(avcodec_find_decoder_by_name("libopenjpeg")),
  myUseGpu(false),
  myIsGpuFailed(false),
  myUseOpenJpeg(false),
  //
  myToRgbCtx(NULL),
  myToRgbPixFmt(stAV::PIX_FMT::NONE),
  myToRgbIsBroken(false),
  //
  myAvDiscard(AVDISCARD_DEFAULT),
  myFramePts(0.0),
  myPixelRatio(1.0f),
  myHParallax(0),
  myRotateDeg(0),
  //
  myVideoClock(0.0),
  //
  myVideoPktPts(stAV::NOPTS_VALUE),
  //
  myAudioClock(0.0),
  myAudioDelayMSec(0),
  myFramesCounter(1),
  myWasFlushed(false),
  myStFormatByUser(StFormat_AUTO),
  myStFormatByName(StFormat_AUTO),
  myStFormatInStream(StFormat_AUTO),
  myIsTheaterMode(false),
  myToStickPano360(false),
  myToSwapJps(false) {
#ifdef ST_USE64PTR
    myFrame.Frame->opaque = (void* )stAV::NOPTS_VALUE;
#else
    myFrame.Frame->opaque = NULL;
#endif

#if(LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(55, 45, 101))
    myFrameBufRef = new StAVFrameCounter();
#endif

    myThread = new StThread(threadFunction, (void* )this, theMaster.isNull() ? "StVideoQueueM" : "StVideoQueueS");
}

StVideoQueue::~StVideoQueue() {
    myTextureQueue->clear();

    myToQuit = true;
    pushQuit();

    myThread->wait();
    myThread.nullify();

    deinit();
    myHWAccelCtx.nullify();
}

namespace {

    struct StFFmpegStereoFormat {
        StFormat    stID;
        const char* name;
    };

    static const StCString THE_ROTATE_KEY       = stCString("rotate");

    static const StCString THE_SRC_MODE_KEY     = stCString("STEREO_MODE");
    static const StCString THE_SRC_MODE_KEY_WMV = stCString("StereoscopicLayout");

    static const StFFmpegStereoFormat STEREOFLAGS[] = {
        // MKV stereoscopic mode decoded by FFmpeg into STEREO_MODE metadata tag
        {StFormat_Mono,                 "mono"},
        {StFormat_SideBySide_RL,        "right_left"},
        {StFormat_SideBySide_LR,        "left_right"},
        {StFormat_TopBottom_RL,         "bottom_top"},
        {StFormat_TopBottom_LR,         "top_bottom"},
        {StFormat_Rows,                 "row_interleaved_rl"},
        {StFormat_Rows,                 "row_interleaved_lr"},
        {StFormat_Columns,              "col_interleaved_rl"},
        {StFormat_Columns,              "col_interleaved_lr"},
        {StFormat_FrameSequence,        "block_lr"}, /// ???
        {StFormat_FrameSequence,        "block_rl"},
        {StFormat_AnaglyphRedCyan,      "anaglyph_cyan_red"},
        {StFormat_AnaglyphGreenMagenta, "anaglyph_green_magenta"},
        // values in WMV StereoscopicLayout tag
        {StFormat_SideBySide_RL,        "SideBySideRF"}, // Right First
        {StFormat_SideBySide_LR,        "SideBySideLF"},
        {StFormat_TopBottom_LR,         "OverUnderLT"},  // Left Top
        {StFormat_TopBottom_RL,         "OverUnderRT"},
        // NULL-terminate array
        {StFormat_AUTO, NULL}
    };

}

bool StVideoQueue::initCodec(AVCodec*   theCodec,
                             const bool theToUseGpu) {
    // close previous codec
    if(myCodec != NULL) {
        myCodec = NULL;
        fillCodecInfo(NULL);
    #ifdef ST_AV_NEWCODECPAR
        avcodec_free_context(&myCodecCtx);
        myCodecCtx = avcodec_alloc_context3(NULL);
        if(avcodec_parameters_to_context(myCodecCtx, myStream->codecpar) < 0) {
            signals.onError(stCString("Internal error: unable to copy codec parameters"));
            return false;
        }
    #else
        avcodec_close(myCodecCtx);
    #endif
    }

    myCodecCtx->opaque         = this;
    myCodecCtx->get_format     = stGetFrameFormat;
#if(LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(55, 0, 0))
    if(check720in1080()) {
        myCodecCtx->flags2 |= AV_CODEC_FLAG2_IGNORE_CROP;
    }
    myCodecCtx->get_buffer2    = stGetFrameBuffer2;
#else
    myCodecCtx->get_buffer     = stGetFrameBuffer1;
    myCodecCtx->release_buffer = stReleaseFrameBuffer;
#endif

    // configure the codec
    myCodecCtx->codec_id = theCodec->id;
    stAV::meta::Dict* anOpts = NULL;
#if(LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(55, 45, 101))
    av_dict_set(&anOpts, "refcounted_frames", "1", 0);
#endif

    // attached pics are sparse, therefore we would not want to delay their decoding till EOF
    int aNbThreads = theToUseGpu || isAttachedPicture()
                   ? 1
                   : StThread::countLogicalProcessors();
    myCodecCtx->thread_count = aNbThreads;
#if(LIBAVCODEC_VERSION_INT < AV_VERSION_INT(52, 112, 0))
    avcodec_thread_init(myCodecCtx, aNbThreads);
#endif

    // open codec
#if(LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(53, 8, 0))
    if(avcodec_open2(myCodecCtx, theCodec, &anOpts) < 0) {
#else
    if(avcodec_open(myCodecCtx, theCodec) < 0) {
#endif
        return false;
    }

    myCodec = theCodec;
    fillCodecInfo(theCodec);
    ST_DEBUG_LOG("FFmpeg: Setup AVcodec to use " + aNbThreads + " threads");
    return true;
}

bool StVideoQueue::init(AVFormatContext*   theFormatCtx,
                        const unsigned int theStreamId,
                        const StString&    theFileName,
                        const StHandle<StStereoParams>& theNewParams) {
    if(!StAVPacketQueue::init(theFormatCtx, theStreamId, theFileName)) {
        signals.onError(stCString("FFmpeg: invalid stream"));
        deinit();
        return false;
    }

#if defined(_WIN32)
    myIsGpuFailed = myCodecAutoId != CodecIdMPEG2
                 && myCodecAutoId != CodecIdH264
                 && myCodecAutoId != CodecIdVC1
                 && myCodecAutoId != CodecIdWMV3
                 && myCodecAutoId != CodecIdHEVC;
#elif defined(__APPLE__)
    myIsGpuFailed = myCodecAutoId != CodecIdMPEG2
                 && myCodecAutoId != CodecIdH264
                 && myCodecAutoId != CodecIdHEVC;
#endif

    bool isCodecOverridden = false;
    if(myUseOpenJpeg
    && myCodecAutoId == CodecIdJpeg2K
    && myCodecOpenJpeg != NULL) {
        isCodecOverridden = initCodec(myCodecOpenJpeg, false);
    }

    // open VIDEO codec
#if defined(__ANDROID__)
    AVCodec* aCodecGpu = NULL;
    if(myUseGpu
    && myCodecCtx->pix_fmt == stAV::PIX_FMT::YUV420P) {
        const StString anAutoCodecName(myCodecAuto->name);
        if(anAutoCodecName.isEquals(stCString("h264"))) {
            aCodecGpu = myCodecH264HW;
        } else if(anAutoCodecName.isEquals(stCString("hevc"))) {
            aCodecGpu = myCodecHevcHW;
        } else if(anAutoCodecName.isEquals(stCString("vp9"))) {
            aCodecGpu = myCodecVp9HW;
        }
    }

    if(aCodecGpu != NULL) {
        isCodecOverridden = initCodec(aCodecGpu, true);
    }
    if(!isCodecOverridden && !initCodec(myCodecAuto, false)) {
        signals.onError(stCString("FFmpeg: Could not open video codec"));
        deinit();
        return false;
    }
#else
    if(!isCodecOverridden && !initCodec(myCodecAuto, myUseGpu && !myIsGpuFailed)) {
        signals.onError(stCString("FFmpeg: Could not open video codec"));
        deinit();
        return false;
    }
#endif

    // determine required myFrameRGB size and allocate it
    if(sizeX() == 0 || sizeY() == 0) {
        signals.onError(stCString("FFmpeg: Codec return wrong frame size"));
        deinit();
        return false;
    }

    // reset AVFrame structure
    myFrame.reset();

    // compute PAR
    if(myStream->sample_aspect_ratio.num && av_cmp_q(myStream->sample_aspect_ratio, myCodecCtx->sample_aspect_ratio)) {
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
    if(stAV::meta::readTag(myFormatCtx, aHalfHeightKeyWMV, aValue)) {
        if(aValue == "1") {
            myPixelRatio *= 0.5;
        }
    } else if(stAV::meta::readTag(myFormatCtx, aHalfWidthKeyWMV, aValue)) {
        if(aValue == "1") {
            myPixelRatio *= 2.0;
        }
    }
    myHParallax = 0;
    if(stAV::meta::readTag(myFormatCtx, aHorParallaxKeyWMV, aValue)) {
        StCLocale aCLocale;
        myHParallax = (int )stStringToLong(aValue.toCString(), 10, aCLocale);
    }

    // we can read information from Display Matrix in side data or from metadata key
    myRotateDeg = 0;

#ifdef ST_AV_NEWSPHERICAL
    if(const AVSphericalMapping* aSpherical = (AVSphericalMapping* )av_stream_get_side_data(myStream, AV_PKT_DATA_SPHERICAL, NULL)) {
        switch(aSpherical->projection) {
            case AV_SPHERICAL_EQUIRECTANGULAR: {
                theNewParams->ViewingMode = StViewSurface_Sphere;
                if(sizeX() == sizeY()) {
                    // TODO - hemisphere information should be somewhere in the file
                    //theNewParams->ViewingMode = StViewSurface_Hemisphere;
                }
                break;
            }
            case AV_SPHERICAL_CUBEMAP: {
                theNewParams->ViewingMode = StViewSurface_Cubemap;
                //spherical->padding
                break;
            }
            //case AV_SPHERICAL_MESH: { theNewParams->ViewingMode = StViewSurface_CubemapEAC; break; }
            case AV_SPHERICAL_EQUIRECTANGULAR_TILE: {
                // unsupported
                //av_spherical_tile_bounds(aSpherical, par->width, par->height, &l, &t, &r, &b);
                break;
            }
        }
        if(theNewParams->ViewingMode != StViewSurface_Plain
        && theNewParams->isZeroRotate()) {
            const double aYaw   = (double )aSpherical->yaw   / (1 << 16);
            const double aPitch = (double )aSpherical->pitch / (1 << 16);
            const double aRoll  = (double )aSpherical->roll  / (1 << 16);
            myRotateDeg = (int )-aRoll;
            theNewParams->setRotateZero((float )-aYaw, (float )aPitch, (float )myRotateDeg);
        }
    }
#endif

#if(LIBAVUTIL_VERSION_INT >= AV_VERSION_INT(55, 0, 0))
    if(const uint8_t* aDispMatrix = av_stream_get_side_data(myStream, AV_PKT_DATA_DISPLAYMATRIX, NULL)) {
        const double aRotDeg = -av_display_rotation_get((const int32_t* )aDispMatrix);
        if(!st::isNaN(aRotDeg)) {
            myRotateDeg = -int(aRotDeg - 360 * std::floor(aRotDeg / 360 + 0.9 / 360));
        }
    }
#else
    if(stAV::meta::readTag(myStream, THE_ROTATE_KEY, aValue)) {
        StCLocale aCLocale;
        myRotateDeg = (int )-stStringToLong(aValue.toCString(), 10, aCLocale);
    }
#endif

    // stereoscopic mode tags
    myStFormatInStream = check720in1080() ? StFormat_Tiled4x : StFormat_AUTO;
    if(stAV::meta::readTag(myFormatCtx, THE_SRC_MODE_KEY,     aValue)
    || stAV::meta::readTag(myStream,    THE_SRC_MODE_KEY,     aValue)
    || stAV::meta::readTag(myFormatCtx, THE_SRC_MODE_KEY_WMV, aValue)) {
        for(size_t aSrcId = 0;; ++aSrcId) {
            const StFFmpegStereoFormat& aFlag = STEREOFLAGS[aSrcId];
            if(aFlag.stID == StFormat_AUTO || aFlag.name == NULL) {
                break;
            } else if(aValue == aFlag.name) {
                myStFormatInStream = aFlag.stID;
                //ST_DEBUG_LOG("  read srcFormat from tags= " + myStFormatInStream);
                break;
            }
        }
    }

    // detect information from file name
    bool isAnamorphByName = false;
    myStFormatByName = st::formatFromName(myFileName, myToSwapJps, isAnamorphByName);
    if(myStFormatInStream == StFormat_AUTO
    && isAnamorphByName) {
        if(myStFormatByName == StFormat_SideBySide_LR
        || myStFormatByName == StFormat_SideBySide_RL) {
            myPixelRatio *= 2.0;
        } else if(myStFormatByName == StFormat_TopBottom_LR
               || myStFormatByName == StFormat_TopBottom_RL) {
            myPixelRatio *= 0.5;
        }
    }
    return true;
}

void StVideoQueue::deinit() {
    myIsGpuFailed = false;
    if(myMaster.isNull()) {
        myTextureQueue->clear();
        myTextureQueue->setConnectedStream(false);
    }
    mySlave.nullify();
    myPixelRatio = 1.0f;
    myDataAdp.nullify();

    myDataRGB.nullify();
    sws_freeContext(myToRgbCtx);
    myToRgbCtx      = NULL;
    myToRgbPixFmt   = stAV::PIX_FMT::NONE;
    myToRgbIsBroken = false;

    myFramesCounter = 1;
    myCachedFrame.nullify();

#if !defined(ST_AV_NEWCODECPAR)
    if(myCodecCtx != NULL) { myCodecCtx->codec_id = myCodecAutoId; }
#endif
    StAVPacketQueue::deinit();
    if(!myHWAccelCtx.isNull()) {
        myHWAccelCtx->decoderDestroy(myCodecCtx);
    }
    if(myCodecCtx != NULL) {
        myCodecCtx->hwaccel_context = NULL;
    }
}

#ifdef ST_AV_OLDSYNC
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

void StVideoQueue::prepareFrame(const StFormat theSrcFormat) {
    int           aFrameSizeX = 0;
    int           aFrameSizeY = 0;
    AVPixelFormat aPixFmt     = stAV::PIX_FMT::NONE;
    stAV::dimYUV  aDimsYUV;
    myFrame.getImageInfo(myCodecCtx, aFrameSizeX, aFrameSizeY, aPixFmt);
    myDataAdp.setBufferCounter(NULL);
    if(aPixFmt == stAV::PIX_FMT::XYZ12
    && myTextureQueue->getDeviceCaps().isSupportedFormat(StImagePlane::ImgRGB48)) {
        myDataAdp.setColorModel(StImage::ImgColor_XYZ);
        myDataAdp.setColorScale(StImage::ImgScale_Full);
        myDataAdp.setPixelRatio(getPixelRatio());
        myDataAdp.changePlane(0).initWrapper(StImagePlane::ImgRGB48, myFrame.getPlane(0),
                                             size_t(aFrameSizeX), size_t(aFrameSizeY),
                                             myFrame.getLineSize(0));
        return;
    } else if(aPixFmt == stAV::PIX_FMT::RGB24
           && myTextureQueue->getDeviceCaps().isSupportedFormat(StImagePlane::ImgRGB)) {
        myDataAdp.setColorModel(StImage::ImgColor_RGB);
        myDataAdp.setColorScale(StImage::ImgScale_Full);
        myDataAdp.setPixelRatio(getPixelRatio());
        myDataAdp.changePlane(0).initWrapper(StImagePlane::ImgRGB, myFrame.getPlane(0),
                                             size_t(aFrameSizeX), size_t(aFrameSizeY),
                                             myFrame.getLineSize(0));
        return;
    } else if(aPixFmt == stAV::PIX_FMT::RGBA32
           && myTextureQueue->getDeviceCaps().isSupportedFormat(StImagePlane::ImgRGBA)) {
        myDataAdp.setColorModel(StImage::ImgColor_RGBA);
        myDataAdp.setColorScale(StImage::ImgScale_Full);
        myDataAdp.setPixelRatio(getPixelRatio());
        myDataAdp.changePlane(0).initWrapper(StImagePlane::ImgRGBA, myFrame.getPlane(0),
                                             size_t(aFrameSizeX), size_t(aFrameSizeY),
                                             myFrame.getLineSize(0));
        return;
#if(LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(53, 5, 0))
    } else if(stAV::isFormatYUVPlanar(myFrame.Frame,
#else
    } else if(stAV::isFormatYUVPlanar(myCodecCtx,
#endif
                                      aDimsYUV)) {

        /// TODO (Kirill Gavrilov#5) remove hack
        // workaround for incorrect frame dimensions information in some files
        // critical for tiled source format that should be 1080p
        if(theSrcFormat == StFormat_Tiled4x
        && aPixFmt      == stAV::PIX_FMT::YUV420P
        && aFrameSizeX >= 1906 && aFrameSizeX <= 1920
        && myFrame.getLineSize(0) >= 1920
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

        if(myTextureQueue->getDeviceCaps().isSupportedFormat(aPlaneFrmt)) {
            myDataAdp.setColorModel(aDimsYUV.hasAlpha ? StImage::ImgColor_YUVA : StImage::ImgColor_YUV);
            myDataAdp.setPixelRatio(getPixelRatio());
            myDataAdp.changePlane(0).initWrapper(aPlaneFrmt, myFrame.getPlane(0),
                                                 size_t(aDimsYUV.widthY), size_t(aDimsYUV.heightY), myFrame.getLineSize(0));
            myDataAdp.changePlane(1).initWrapper(aPlaneFrmt, myFrame.getPlane(1),
                                                 size_t(aDimsYUV.widthU), size_t(aDimsYUV.heightU), myFrame.getLineSize(1));
            myDataAdp.changePlane(2).initWrapper(aPlaneFrmt, myFrame.getPlane(2),
                                                 size_t(aDimsYUV.widthV), size_t(aDimsYUV.heightV), myFrame.getLineSize(2));
            if(aDimsYUV.hasAlpha) {
                myDataAdp.changePlane(3).initWrapper(aPlaneFrmt, myFrame.getPlane(3),
                                                     size_t(aDimsYUV.widthY), size_t(aDimsYUV.heightY), myFrame.getLineSize(3));
            }

            myFrameBufRef->moveReferenceFrom(myFrame.Frame);
            myDataAdp.setBufferCounter(myFrameBufRef);
            return;
        }
    } else if(aPixFmt == stAV::PIX_FMT::NV12) {
        aDimsYUV.isFullScale = false;
    #if(LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(52, 29, 0))
        if(myCodecCtx->color_range == AVCOL_RANGE_JPEG) {
            // there no color range information in the AVframe (yet)
            aDimsYUV.isFullScale = true;
        }
    #endif
        myDataAdp.setColorScale(aDimsYUV.isFullScale ? StImage::ImgScale_NvFull : StImage::ImgScale_NvMpeg);
        myDataAdp.setColorModel(StImage::ImgColor_YUV);
        myDataAdp.setPixelRatio(getPixelRatio());
        myDataAdp.changePlane(0).initWrapper(StImagePlane::ImgGray, myFrame.getPlane(0),
                                             size_t(aFrameSizeX), size_t(aFrameSizeY), myFrame.getLineSize(0));
        myDataAdp.changePlane(1).initWrapper(StImagePlane::ImgUV, myFrame.getPlane(1),
                                             size_t(aFrameSizeX / 2), size_t(aFrameSizeY / 2), myFrame.getLineSize(1));

        myFrameBufRef->moveReferenceFrom(myFrame.Frame);
        myDataAdp.setBufferCounter(myFrameBufRef);
        return;
    }

    if(!myToRgbIsBroken) {
        if(myToRgbCtx    == NULL
        || myToRgbPixFmt != aPixFmt
        || size_t(aFrameSizeX) != myDataRGB.getSizeX()
        || size_t(aFrameSizeY) != myDataRGB.getSizeY()) {
            // initialize software scaler/converter
        //#if LIBSWSCALE_VERSION_MAJOR >= 3
            //myToRgbCtx = sws_getCachedContext(myToRgbCtx,
            //                                  aFrameSizeX, aFrameSizeY, aPixFmt,
            //                                  aFrameSizeX, aFrameSizeY, stAV::PIX_FMT::RGB24
            //                                  SWS_BICUBIC, NULL, NULL, NULL);
        //#else
            sws_freeContext(myToRgbCtx);
            myToRgbCtx = sws_getContext(aFrameSizeX, aFrameSizeY, aPixFmt,              // source
                                        aFrameSizeX, aFrameSizeY, stAV::PIX_FMT::RGB24, // destination
                                        SWS_BICUBIC, NULL, NULL, NULL);
        //#endif
            myToRgbPixFmt = aPixFmt;
            if(myToRgbCtx == NULL
            || aFrameSizeX <= 0
            || aFrameSizeY <= 0) {
                signals.onError(stCString("FFmpeg: Failed to create SWScaler context"));
                myToRgbIsBroken = true;
            } else {
                if(!myDataRGB.initTrash(StImagePlane::ImgRGB, size_t(aFrameSizeX), size_t(aFrameSizeY))) {
                    signals.onError(stCString("FFmpeg: Failed allocation of RGB frame (out of memory)"));
                    myToRgbIsBroken = true;
                } else {
                    ST_DEBUG_LOG(" !!! Performance warning! Using SWScaler for " + stAV::PIX_FMT::getString(aPixFmt) + " pixel format.");
                    {
                        StMutexAuto aLock(myMutexInfo);
                        myCodecStr += StString("\n[SWScaler] Software converter (from ") + stAV::PIX_FMT::getString(aPixFmt) + stCString(" into RGB)");
                    }

                    myFrameRGB.Frame->data[0]     = (uint8_t* )myDataRGB.changeData();
                    myFrameRGB.Frame->linesize[0] = (int      )myDataRGB.getSizeRowBytes();
                    for(int aPlaneIter = 1; aPlaneIter < AV_NUM_DATA_POINTERS; ++aPlaneIter) {
                        myFrameRGB.Frame->data    [aPlaneIter] = NULL;
                        myFrameRGB.Frame->linesize[aPlaneIter] = 0;
                    }
                }
            }
        }

        if(!myToRgbIsBroken) {
            sws_scale(myToRgbCtx,
                      myFrame.Frame->data, myFrame.Frame->linesize,
                      0, aFrameSizeY,
                      myFrameRGB.Frame->data, myFrameRGB.Frame->linesize);

            myDataAdp.setColorModel(StImage::ImgColor_RGB);
            myDataAdp.setColorScale(StImage::ImgScale_Full);
            myDataAdp.setPixelRatio(getPixelRatio());
            myDataAdp.changePlane(0).initWrapper(StImagePlane::ImgRGB, myDataRGB.changeData(),
                                                 size_t(aFrameSizeX), size_t(aFrameSizeY));
        }
    } else {
        //ST_DEBUG_LOG("Frame skipped - unsupported pixel format!");
    }
}

void StVideoQueue::pushFrame(const StImage&     theSrcDataLeft,
                             const StImage&     theSrcDataRight,
                             const StHandle<StStereoParams>& theStParams,
                             const StFormat     theSrcFormat,
                             const StCubemap    theCubemapFormat,
                             const double       theSrcPTS) {
    while(!myToFlush && myTextureQueue->isFull()) {
        StThread::sleep(10);
    }

    if(myToFlush) {
        myToFlush = false;
        return;
    }

    if(!theSrcDataLeft.isNull()) {
        theStParams->Src1SizeX = theSrcDataLeft.getSizeX();
        theStParams->Src1SizeY = theSrcDataLeft.getSizeY();
    } else {
        theStParams->Src1SizeX = 0;
        theStParams->Src1SizeY = 0;
    }
    if(!theSrcDataRight.isNull()) {
        theStParams->Src2SizeX = theSrcDataRight.getSizeX();
        theStParams->Src2SizeY = theSrcDataRight.getSizeY();
    } else {
        theStParams->Src2SizeX = 0;
        theStParams->Src2SizeY = 0;
    }

    if(myToStickPano360
    && theStParams->ViewingMode == StViewSurface_Plain) {
        StPanorama aPano = st::probePanorama(theSrcFormat,
                                             theStParams->Src1SizeX, theStParams->Src1SizeY,
                                             theStParams->Src2SizeX, theStParams->Src2SizeY);
        theStParams->ViewingMode = StStereoParams::getViewSurfaceForPanoramaSource(aPano, true);
    }
    if(myIsTheaterMode && theStParams->ViewingMode == StViewSurface_Plain) {
        theStParams->ViewingMode = StViewSurface_Theater;
    } else if(!myIsTheaterMode && theStParams->ViewingMode == StViewSurface_Theater) {
        theStParams->ViewingMode = StViewSurface_Plain;
    }

    myTextureQueue->push(theSrcDataLeft, theSrcDataRight, theStParams, theSrcFormat, theCubemapFormat, theSrcPTS);
    myTextureQueue->setConnectedStream(true);
    if(myWasFlushed) {
        // force frame update after seeking regardless playback timer
        myTextureQueue->stglSwapFB(0);
        myWasFlushed = false;
    }
}

void StVideoQueue::decodeLoop() {
    double anAverageDelaySec = 40.0;
    double aPrevPts  = 0.0;
    myFramePts = 0.0;
    StHandle<StAVPacket> aPacket;
    StString aTagValue;
    bool isStarted = false;
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
                aPrevPts = 0.0;
                myWasFlushed = true; // force displaying the first frame
                continue;
            }
            case StAVPacket::DATA_PACKET: {
                break;
            }
            case StAVPacket::LAST_PACKET: {
            #if(LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(57, 106, 102))
                break; // redirect NULL packet to avcodec_send_packet()break;
            #else
                continue;
            #endif
            }
            case StAVPacket::END_PACKET: {
                // TODO at the end of the stream it might be needed calling
                // avcodec_send_packet with NULL to initiate draining data followed by avcodec_receive_frame
                // to recieve last frames.

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

        bool toSendPacket = true;
        for(;;) {
            if(!decodeFrame(aPacket, toSendPacket, isStarted, aTagValue, anAverageDelaySec, aPrevPts)) {
                break;
            }
        }
        aPacket.nullify();
    }
}

bool StVideoQueue::decodeFrame(const StHandle<StAVPacket>& thePacket,
                               bool& theToSendPacket,
                               bool& theIsStarted,
                               StString& theTagValue,
                               double& theAverageDelaySec,
                               double& thePrevPts) {
    bool toTryMoreFrames = false;
    (void )theToSendPacket;
    const bool toTryGpu = myUseGpu && !myIsGpuFailed;
#if(LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(57, 106, 102))
    if(theToSendPacket) {
        theToSendPacket = false;
        const int aRes = avcodec_send_packet(myCodecCtx, thePacket->getType() == StAVPacket::DATA_PACKET ? thePacket->getAVpkt() : NULL);
        if(aRes == AVERROR(EAGAIN)) {
            // special case used by some hardware decoders - new packet cannot be sent until decoded frame is retrieved
            theToSendPacket = true;
        } else if(aRes < 0 && aRes != AVERROR_EOF) {
            return false;
        }
    }

    const int aRes2 = avcodec_receive_frame(myCodecCtx, myFrame.Frame);
    const bool isGpuUsed = myUseGpu && !myIsGpuFailed;
    if(isGpuUsed != toTryGpu) {
        if(!initCodec(myCodecAuto, isGpuUsed)) {
            signals.onError(stCString("FFmpeg: Could not re-open video codec"));
            deinit();
            return false;
        }
        theToSendPacket = true;
        return true;
    }

    if(aRes2 < 0) {
        // polling - if packet was not sent and new frame is not yet ready, we need to try again and again...
        if(theToSendPacket) {
            StThread::sleep(10);
        }
        return theToSendPacket;
    }
    toTryMoreFrames = true;
#elif(LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(52, 23, 0))
    int isFrameFinished = 0;
    avcodec_decode_video2(myCodecCtx, myFrame.Frame, &isFrameFinished, thePacket->getAVpkt());
    const bool isGpuUsed = myUseGpu && !myIsGpuFailed;
    if(isGpuUsed != toTryGpu) {
        if(!initCodec(myCodecAuto, isGpuUsed)) {
            signals.onError(stCString("FFmpeg: Could not re-open video codec"));
            deinit();
            return false;
        }
        return true;
    }
    if(isFrameFinished == 0) {
        // need more packets to decode whole frame
        return false;
    }
#else
    int isFrameFinished = 0;
    avcodec_decode_video(myCodecCtx, myFrame.Frame, &isFrameFinished,
                         thePacket->getData(), thePacket->getSize());
    if(isFrameFinished == 0) {
        // need more packets to decode whole frame
        return false;
    }
#endif
    if(thePacket->isKeyFrame()) { // !theToSentPacket?
        myFramesCounter = 1;
    }

#ifndef ST_AV_OLDSYNC
    myVideoPktPts = myFrame.getBestEffortTimestamp();
    if(myVideoPktPts == stAV::NOPTS_VALUE) {
        // TODO why best_effort_timestamp is invalid in case of h264_mediacodec?
        myVideoPktPts = myFrame.Frame->pts;
    }
    if(myVideoPktPts == stAV::NOPTS_VALUE) {
        myVideoPktPts = 0;
    }

    myFramePts  = double(myVideoPktPts) * av_q2d(myStream->time_base);
    myFramePts -= myPtsStartBase; // normalize PTS
#else
    // Save global pts to be stored in pFrame in first call
    myVideoPktPts = thePacket->getPts();

    myFramePts = 0.0;
    if(thePacket->getDts() != stAV::NOPTS_VALUE) {
        myFramePts = double(thePacket->getDts());
    } else {
        int64_t aPktPtsSync = stAV::NOPTS_VALUE;
    #ifdef ST_USE64PTR
        aPktPtsSync = (int64_t )myFrame.Frame->opaque;
    #else
        AVFrameSideData* aSideDataSync = av_frame_get_side_data(myFrame.Frame, (AVFrameSideDataType )1000);
        if(aSideDataSync != NULL) {
            memcpy(&aPktPtsSync, &aSideDataSync->data, sizeof(myVideoPktPts));
        }
    #endif
        if(aPktPtsSync != stAV::NOPTS_VALUE) {
            myFramePts = double(aPktPtsSync);
        }
    }
    myFramePts *= av_q2d(myStream->time_base);
    myFramePts -= myPtsStartBase; // normalize PTS

    syncVideo(myFrame.Frame, &myFramePts);
#endif

    const double aDelay = myFramePts - thePrevPts;
    if(aDelay > 0.0 && aDelay < 1.0) {
        theAverageDelaySec = aDelay;
    }
    thePrevPts = myFramePts;

    // do we need to skip frames or not
    static const double OVERR_LIMIT = 0.2;
    static const double GREATER_LIMIT = 100.0;
    if(myMaster.isNull()) {
        const double anAudioClock = getAClock() + double(myAudioDelayMSec) * 0.001;
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

    // copy frame back from GPU to CPU memory
    if(!myHWAccelCtx.isNull()) {
        myHWAccelCtx->retrieveFrame(*this, myFrame.Frame);
    }

    // we currently allow to override source format stored in metadata
#ifdef ST_AV_NEWSTEREO
    if(AVFrameSideData* aSideDataS3d = av_frame_get_side_data(myFrame.Frame, AV_FRAME_DATA_STEREO3D)) {
        AVStereo3D* aStereo = (AVStereo3D* )aSideDataS3d->data;
        myStFormatInStream = stAV::stereo3dAvToSt(aStereo->type);
        if(aStereo->flags & AV_STEREO3D_FLAG_INVERT) {
            myStFormatInStream = st::formatReversed(myStFormatInStream);
        }
    }
#endif
#if(LIBAVUTIL_VERSION_INT >= AV_VERSION_INT(55, 0, 0))
    if(const AVFrameSideData* aSideDataRot = av_frame_get_side_data(myFrame.Frame, AV_FRAME_DATA_DISPLAYMATRIX)) {
        if(aSideDataRot->size >= int(9 * sizeof(int32_t))) {
            const double aRotDeg = -av_display_rotation_get((const int32_t* )aSideDataRot->data);
            if(!st::isNaN(aRotDeg)) {
                myRotateDeg = -int(aRotDeg - 360 * std::floor(aRotDeg / 360 + 0.9 / 360));
            }
        }
    }
#endif
    if(stAV::meta::readTag(myFrame.Frame, THE_SRC_MODE_KEY, theTagValue)) {
        for(size_t aSrcId = 0;; ++aSrcId) {
            const StFFmpegStereoFormat& aFlag = STEREOFLAGS[aSrcId];
            if(aFlag.stID == StFormat_AUTO || aFlag.name == NULL) {
                break;
            } else if(theTagValue == aFlag.name) {
                myStFormatInStream = aFlag.stID;
                //ST_DEBUG_LOG("  read srcFormat from tags= " + myStFormatInStream);
                break;
            }
        }
    }
    // override source format stored in metadata
    StFormat  aSrcFormat     = myStFormatByUser;
    StCubemap aCubemapFormat = StCubemap_OFF;
    if(thePacket->getSource()->ViewingMode == StViewSurface_Cubemap) {
        aCubemapFormat = StCubemap_Packed;
    } else if(thePacket->getSource()->ViewingMode == StViewSurface_CubemapEAC) {
        aCubemapFormat = StCubemap_PackedEAC;
    }

    if(aSrcFormat == StFormat_AUTO) {
        // prefer info stored in the stream itself
        aSrcFormat = myStFormatInStream;
    }
    if(aSrcFormat == StFormat_AUTO) {
        // try using format detected from file name
        aSrcFormat = myStFormatByName;
    }
    /*if(aSrcFormat == StFormat_AUTO
    && sizeY() != 0) {
        // try detection based on aspect ratio
        aSrcFormat = st::formatFromRatio(GLfloat(sizeX()) / GLfloat(sizeY()));
    }*/

    prepareFrame(aSrcFormat);

    if(!mySlave.isNull()) {
        if(theIsStarted) {
            StHandle<StStereoParams> aParams = thePacket->getSource();
            if(!aParams.isNull()) {
                aParams->setSeparationNeutral(myHParallax);
                aParams->setZRotateZero((float )myRotateDeg);
            }
            theIsStarted = false;
        }

        StImage* aSlaveData = NULL;
        double aSlavePts = 0.0;
        for(;;) {
            // wait data from Slave
            if(aSlaveData == NULL) {
                aSlaveData = mySlave->waitData(aSlavePts);
            }
            if(aSlaveData != NULL) {
                const double aPtsDiff = myFramePts - aSlavePts;
                if(aPtsDiff > 0.5 * theAverageDelaySec) {
                    // wait for more recent frame from slave thread
                    mySlave->unlockData();
                    aSlaveData = NULL;
                    StThread::sleep(10);
                    continue;
                } else if(aPtsDiff < -0.5 * theAverageDelaySec) {
                    // too far...
                    if(aPtsDiff < -6.0) {
                        // result of seeking?
                        mySlave->unlockData();
                        aSlaveData = NULL;
                    }
                    break;
                }

                pushFrame(myDataAdp, *aSlaveData, thePacket->getSource(), StFormat_SeparateFrames, aCubemapFormat, myFramePts);

                aSlaveData = NULL;
                mySlave->unlockData();
            } else {
                pushFrame(myDataAdp, myEmptyImage, thePacket->getSource(), aSrcFormat, aCubemapFormat, myFramePts);
            }
            break;
        }
    } else if(!myMaster.isNull()) {
        // push data to Master
        myHasDataState.set();
    } else {
        if(theIsStarted) {
            StHandle<StStereoParams> aParams = thePacket->getSource();
            if(!aParams.isNull()) {
                aParams->setSeparationNeutral(myHParallax);
                aParams->setZRotateZero((float )myRotateDeg);
            }
            theIsStarted = false;
        }

        // simple one-stream case
        if(aSrcFormat == StFormat_FrameSequence) {
            if(isOddNumber(myFramesCounter)) {
                myCachedFrame.fill(myDataAdp, false);
            } else {
                pushFrame(myCachedFrame, myDataAdp, thePacket->getSource(), StFormat_FrameSequence, aCubemapFormat, myFramePts);
            }
            ++myFramesCounter;
        } else {
            pushFrame(myDataAdp, myEmptyImage, thePacket->getSource(), aSrcFormat, aCubemapFormat, myFramePts);
        }
    }

    myFrame.reset();
    return toTryMoreFrames;
}
