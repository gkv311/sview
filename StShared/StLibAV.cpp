/**
 * Copyright © 2011-2013 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#include <StLibAV.h>
#include <StThreads/StMutex.h>
#include <StStrings/StLogger.h>

// libav* libraries written on pure C,
// and we must around includes manually
extern "C" {
#if(LIBAVUTIL_VERSION_INT >= AV_VERSION_INT(50, 8, 0))
    #include <libavutil/pixdesc.h>
#endif
};

#if(LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(52, 30, 0))
class StFFMpegLocker {

        public:

    StFFMpegLocker() {
        //
    }

    ~StFFMpegLocker() {
        av_lockmgr_register(NULL); // unregister to avoid usage of dead function pointer
    }

    void init() {
        if(av_lockmgr_register(stFFmpegLock) != 0) {
            ST_DEBUG_LOG("FFmpeg, fail to register own mutex!");
        }
    }

        private:

    static int stFFmpegLock(void** theMutexPtrPtr, enum AVLockOp theOperation) {
        StMutex* stMutex = (StMutex* )*theMutexPtrPtr;
        switch(theOperation) {
            case AV_LOCK_CREATE: {
                // create a mutex
                stMutex = new StMutex();
                *theMutexPtrPtr = (void* )stMutex;
                return 0;
            }
            case AV_LOCK_OBTAIN: {
                // lock the mutex
                stMutex->lock();
                return 0;
            }
            case AV_LOCK_RELEASE: {
                // unlock the mutex
                stMutex->unlock();
                return 0;
            }
            case AV_LOCK_DESTROY: {
                // free mutex resources
                delete stMutex;
                *theMutexPtrPtr = NULL;
                return 0;
            }
            default: {
                ST_DEBUG_LOG("FFmpeg, Unsupported lock operation " + theOperation);
                return 1;
            }
        }
    }

} static stFFMpegLocker;
#endif

// this is just redeclaration AV_NOPTS_VALUE
const int64_t stLibAV::NOPTS_VALUE = 0x8000000000000000LL;

/**
 * Reproduced PixelFormat enumeration.
 */
#if(LIBAVUTIL_VERSION_INT >= AV_VERSION_INT(50, 8, 0))
    #define ST_AV_GETPIXFMT(theName) av_get_pix_fmt(theName)
#else
    #define ST_AV_GETPIXFMT(theName) avcodec_get_pix_fmt(theName)
#endif
const PixelFormat stLibAV::PIX_FMT::NONE       = PixelFormat(-1);
const PixelFormat stLibAV::PIX_FMT::YUV420P    = PixelFormat( 0);
const PixelFormat stLibAV::PIX_FMT::GRAY8      = ST_AV_GETPIXFMT("gray");
const PixelFormat stLibAV::PIX_FMT::GRAY16     = ST_AV_GETPIXFMT("gray16");
const PixelFormat stLibAV::PIX_FMT::YUV422P    = ST_AV_GETPIXFMT("yuv422p");
const PixelFormat stLibAV::PIX_FMT::YUV444P    = ST_AV_GETPIXFMT("yuv444p");
const PixelFormat stLibAV::PIX_FMT::YUV410P    = ST_AV_GETPIXFMT("yuv410p");
const PixelFormat stLibAV::PIX_FMT::YUV411P    = ST_AV_GETPIXFMT("yuv411p");
const PixelFormat stLibAV::PIX_FMT::YUV440P    = ST_AV_GETPIXFMT("yuv440p");
const PixelFormat stLibAV::PIX_FMT::YUV420P9   = ST_AV_GETPIXFMT("yuv420p9");
const PixelFormat stLibAV::PIX_FMT::YUV422P9   = ST_AV_GETPIXFMT("yuv422p9");
const PixelFormat stLibAV::PIX_FMT::YUV444P9   = ST_AV_GETPIXFMT("yuv444p9");
const PixelFormat stLibAV::PIX_FMT::YUV420P10  = ST_AV_GETPIXFMT("yuv420p10");
const PixelFormat stLibAV::PIX_FMT::YUV422P10  = ST_AV_GETPIXFMT("yuv422p10");
const PixelFormat stLibAV::PIX_FMT::YUV444P10  = ST_AV_GETPIXFMT("yuv444p10");
const PixelFormat stLibAV::PIX_FMT::YUV420P16  = ST_AV_GETPIXFMT("yuv420p16");
const PixelFormat stLibAV::PIX_FMT::YUV422P16  = ST_AV_GETPIXFMT("yuv422p16");
const PixelFormat stLibAV::PIX_FMT::YUV444P16  = ST_AV_GETPIXFMT("yuv444p16");
const PixelFormat stLibAV::PIX_FMT::YUVJ420P   = ST_AV_GETPIXFMT("yuvj420p");
const PixelFormat stLibAV::PIX_FMT::YUVJ422P   = ST_AV_GETPIXFMT("yuvj422p");
const PixelFormat stLibAV::PIX_FMT::YUVJ444P   = ST_AV_GETPIXFMT("yuvj444p");
const PixelFormat stLibAV::PIX_FMT::YUVJ440P   = ST_AV_GETPIXFMT("yuvj440p");
const PixelFormat stLibAV::PIX_FMT::RGB24      = ST_AV_GETPIXFMT("rgb24");
const PixelFormat stLibAV::PIX_FMT::BGR24      = ST_AV_GETPIXFMT("bgr24");

// TODO (Kirill Gavrilov#9) remove this stuff
namespace {
    static const PixelFormat AvPixFmtRGBA  = ST_AV_GETPIXFMT("rgba");
    static const PixelFormat AvPixFmtBGRA  = ST_AV_GETPIXFMT("bgra");
    static const PixelFormat AvPixFmtRGB32 = ST_AV_GETPIXFMT("rgb32"); // compatibility with old FFmpeg
    static const PixelFormat AvPixFmtBGR32 = ST_AV_GETPIXFMT("bgr32"); // compatibility with old FFmpeg

    static const AVRational ST_AV_TIME_BASE_Q = {1, AV_TIME_BASE};
    static const double     ST_AV_TIME_BASE_D = av_q2d(ST_AV_TIME_BASE_Q);
};

const PixelFormat stLibAV::PIX_FMT::RGBA32 = (AvPixFmtRGBA != stLibAV::PIX_FMT::NONE) ? AvPixFmtRGBA : AvPixFmtBGR32;
const PixelFormat stLibAV::PIX_FMT::BGRA32 = (AvPixFmtBGRA != stLibAV::PIX_FMT::NONE) ? AvPixFmtBGRA : AvPixFmtRGB32;

StCString stLibAV::PIX_FMT::getString(const PixelFormat theFrmt) {
    if(theFrmt == stLibAV::PIX_FMT::NONE) {
        return stCString("none");
    } else if(theFrmt == stLibAV::PIX_FMT::YUV420P) {
        return stCString("yuv420p");
    } else if(theFrmt == stLibAV::PIX_FMT::GRAY8) {
        return stCString("gray8");
    } else if(theFrmt == stLibAV::PIX_FMT::GRAY16) {
        return stCString("gray16");
    } else if(theFrmt == stLibAV::PIX_FMT::YUV422P) {
        return stCString("yuv422p");
    } else if(theFrmt == stLibAV::PIX_FMT::YUV444P) {
        return stCString("yuv444p");
    } else if(theFrmt == stLibAV::PIX_FMT::YUV410P) {
        return stCString("yuv410p");
    } else if(theFrmt == stLibAV::PIX_FMT::YUV411P) {
        return stCString("yuv411p");
    } else if(theFrmt == stLibAV::PIX_FMT::YUV440P) {
        return stCString("yuv440p");
    } else if(theFrmt == stLibAV::PIX_FMT::YUV420P9) {
        return stCString("yuv420p9");
    } else if(theFrmt == stLibAV::PIX_FMT::YUV422P9) {
        return stCString("yuv422p9");
    } else if(theFrmt == stLibAV::PIX_FMT::YUV444P9) {
        return stCString("yuv444p9");
    } else if(theFrmt == stLibAV::PIX_FMT::YUV420P10) {
        return stCString("yuv420p10");
    } else if(theFrmt == stLibAV::PIX_FMT::YUV422P10) {
        return stCString("yuv422p10");
    } else if(theFrmt == stLibAV::PIX_FMT::YUV444P10) {
        return stCString("yuv444p10");
    } else if(theFrmt == stLibAV::PIX_FMT::YUV420P16) {
        return stCString("yuv420p16");
    } else if(theFrmt == stLibAV::PIX_FMT::YUV422P16) {
        return stCString("yuv422p16");
    } else if(theFrmt == stLibAV::PIX_FMT::YUV444P16) {
        return stCString("yuv444p16");
    } else if(theFrmt == stLibAV::PIX_FMT::YUVJ420P) {
        return stCString("yuvj420p");
    } else if(theFrmt == stLibAV::PIX_FMT::YUVJ422P) {
        return stCString("yuvj422p");
    } else if(theFrmt == stLibAV::PIX_FMT::YUVJ444P) {
        return stCString("yuvj444p");
    } else if(theFrmt == stLibAV::PIX_FMT::YUVJ440P) {
        return stCString("yuvj440p");
    } else if(theFrmt == stLibAV::PIX_FMT::RGB24) {
        return stCString("rgb24");
    } else if(theFrmt == stLibAV::PIX_FMT::BGR24) {
        return stCString("bgr24");
    } else {
        return stCString("unknown");
    }
}

namespace {

    static bool initOnce() {
        // register own mutex to prevent multithreading errors
        // while using FFmpeg functions
    #if(LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(52, 30, 0))
        stFFMpegLocker.init();
    #else
        #warning Ancient FFmpeg used, initialization performed in thread-unsafe manner!
    #endif
        // Notice, this call is absolutely not thread safe!
        // you should never call it first time from concurrent threads.
        // But after first initialization is done this is safe to call it anyhow
        av_register_all();
    #if(LIBAVFORMAT_VERSION_INT >= AV_VERSION_INT(53, 13, 0))
        avformat_network_init();
    #endif
        ST_DEBUG_LOG("FFmpeg initialized:");

        // show up information about dynamically linked libraries
        ST_DEBUG_LOG("  libavutil\t"   + stLibAV::Version::libavutil().toString());
        ST_DEBUG_LOG("  libavcodec\t"  + stLibAV::Version::libavcodec().toString());
        ST_DEBUG_LOG("  libavformat\t" + stLibAV::Version::libavformat().toString());
        ST_DEBUG_LOG("  libavdevice\t" + stLibAV::Version::libavdevice().toString());
        ST_DEBUG_LOG("  libswscale\t"  + stLibAV::Version::libswscale().toString());
        return true;
    }

};

bool stLibAV::init() {
    static const bool isFFmpegInitiailed = initOnce();
    return isFFmpegInitiailed;
}

bool stLibAV::isFormatYUVPlanar(const AVCodecContext* theCtx) {
    return theCtx->pix_fmt == stLibAV::PIX_FMT::YUV420P
        || theCtx->pix_fmt == stLibAV::PIX_FMT::YUVJ420P
        || theCtx->pix_fmt == stLibAV::PIX_FMT::YUV422P
        || theCtx->pix_fmt == stLibAV::PIX_FMT::YUVJ422P
        || theCtx->pix_fmt == stLibAV::PIX_FMT::YUV444P
        || theCtx->pix_fmt == stLibAV::PIX_FMT::YUVJ444P
        || theCtx->pix_fmt == stLibAV::PIX_FMT::YUV440P
        || theCtx->pix_fmt == stLibAV::PIX_FMT::YUVJ440P
        || theCtx->pix_fmt == stLibAV::PIX_FMT::YUV411P
        || theCtx->pix_fmt == stLibAV::PIX_FMT::YUV410P
        || theCtx->pix_fmt == stLibAV::PIX_FMT::YUV420P9
        || theCtx->pix_fmt == stLibAV::PIX_FMT::YUV422P9
        || theCtx->pix_fmt == stLibAV::PIX_FMT::YUV444P9
        || theCtx->pix_fmt == stLibAV::PIX_FMT::YUV420P10
        || theCtx->pix_fmt == stLibAV::PIX_FMT::YUV422P10
        || theCtx->pix_fmt == stLibAV::PIX_FMT::YUV444P10
        || theCtx->pix_fmt == stLibAV::PIX_FMT::YUV420P16
        || theCtx->pix_fmt == stLibAV::PIX_FMT::YUV422P16
        || theCtx->pix_fmt == stLibAV::PIX_FMT::YUV444P16;
}

bool stLibAV::isFormatYUVPlanar(const PixelFormat thePixFmt,
                                const int         theWidth,
                                const int         theHeight,
                                dimYUV&           theDims) {
    if(thePixFmt == stLibAV::PIX_FMT::NONE) {
        return false;
    } else if(thePixFmt == stLibAV::PIX_FMT::YUV420P
           || thePixFmt == stLibAV::PIX_FMT::YUVJ420P
           || thePixFmt == stLibAV::PIX_FMT::YUV420P9
           || thePixFmt == stLibAV::PIX_FMT::YUV420P10
           || thePixFmt == stLibAV::PIX_FMT::YUV420P16) {
        theDims.widthY  = theWidth;
        theDims.heightY = theHeight;
        theDims.widthU  = theDims.widthV  = theDims.widthY  / 2;
        theDims.heightU = theDims.heightV = theDims.heightY / 2;
        theDims.isFullScale = (thePixFmt == stLibAV::PIX_FMT::YUVJ420P);
    } else if(thePixFmt == stLibAV::PIX_FMT::YUV422P
           || thePixFmt == stLibAV::PIX_FMT::YUVJ422P
           || thePixFmt == stLibAV::PIX_FMT::YUV422P9
           || thePixFmt == stLibAV::PIX_FMT::YUV422P10
           || thePixFmt == stLibAV::PIX_FMT::YUV422P16) {
        theDims.widthY  = theWidth;
        theDims.heightY = theDims.heightU = theDims.heightV = theHeight;
        theDims.widthU  = theDims.widthV  = theDims.widthY  / 2;
        theDims.isFullScale = (thePixFmt == stLibAV::PIX_FMT::YUVJ422P);
    } else if(thePixFmt == stLibAV::PIX_FMT::YUV444P
           || thePixFmt == stLibAV::PIX_FMT::YUVJ444P
           || thePixFmt == stLibAV::PIX_FMT::YUV444P9
           || thePixFmt == stLibAV::PIX_FMT::YUV444P10
           || thePixFmt == stLibAV::PIX_FMT::YUV444P16) {
        theDims.widthY  = theDims.widthU  = theDims.widthV  = theWidth;
        theDims.heightY = theDims.heightU = theDims.heightV = theHeight;
        theDims.isFullScale = (thePixFmt == stLibAV::PIX_FMT::YUVJ444P);
    } else if(thePixFmt == stLibAV::PIX_FMT::YUV440P
           || thePixFmt == stLibAV::PIX_FMT::YUVJ440P) {
        theDims.widthY  = theDims.widthU  = theDims.widthV  = theWidth;
        theDims.heightY = theHeight;
        theDims.heightU = theDims.heightV = theDims.heightY / 2;
        theDims.isFullScale = (thePixFmt == stLibAV::PIX_FMT::YUVJ440P);
    } else if(thePixFmt == stLibAV::PIX_FMT::YUV411P) {
        theDims.widthY  = theDims.widthU  = theDims.widthV  = theWidth;
        theDims.heightY = theDims.heightU = theDims.heightV = theHeight;
        theDims.widthU  = theDims.widthV  = theDims.widthY  / 4;
        theDims.isFullScale = false;
    } else if(thePixFmt == stLibAV::PIX_FMT::YUV410P) {
        theDims.widthY  = theDims.widthU  = theDims.widthV  = theWidth;
        theDims.heightY = theHeight;
        theDims.widthU  = theDims.widthV  = theDims.widthY  / 4;
        theDims.heightU = theDims.heightV = theDims.heightY / 4;
        theDims.isFullScale = false;
    } else {
        return false;
    }

    if(thePixFmt == stLibAV::PIX_FMT::YUV420P9
    || thePixFmt == stLibAV::PIX_FMT::YUV422P9
    || thePixFmt == stLibAV::PIX_FMT::YUV444P9) {
        theDims.bitsPerComp = 9;
    } else if(thePixFmt == stLibAV::PIX_FMT::YUV420P10
           || thePixFmt == stLibAV::PIX_FMT::YUV422P10
           || thePixFmt == stLibAV::PIX_FMT::YUV444P10) {
        theDims.bitsPerComp = 10;
    } else if(thePixFmt == stLibAV::PIX_FMT::YUV420P16
           || thePixFmt == stLibAV::PIX_FMT::YUV422P16
           || thePixFmt == stLibAV::PIX_FMT::YUV444P16) {
        theDims.bitsPerComp = 16;
    } else {
        theDims.bitsPerComp = 8;
    }

    return true;
}

StString stLibAV::getAVErrorDescription(int avErrCode) {
#if(LIBAVUTIL_VERSION_INT >= AV_VERSION_INT(50, 13, 0))
    char aBuff[4096];
    stMemSet(aBuff, 0, sizeof(aBuff));
    if(av_strerror(avErrCode, aBuff, 4096) == -1) {
    #ifdef _MSC_VER
        wchar_t aBuffW[4096];
        stMemSet(aBuffW, 0, sizeof(aBuffW));
        if(_wcserror_s(aBuffW, 4096, AVUNERROR(avErrCode)) == 0) {
            return aBuffW;
        }
    #elif defined(_WIN32)
        // MinGW has only thread-unsafe variant
        char* anErrDesc = strerror(AVUNERROR(avErrCode));
        if(anErrDesc != NULL) {
            return StString(anErrDesc);
        }
    #endif
    }
    return aBuff;
#else
    return StString(" #") + avErrCode;
#endif
}

stLibAV::Version::Version(const unsigned theVersionInt)
: myMajor(theVersionInt >> 16 & 0xFF),
  myMinor(theVersionInt >>  8 & 0xFF),
  myMicro(theVersionInt       & 0xFF) {
    //
}

inline StString formatVerSub(const unsigned theVerSub) {
    return (theVerSub > 10) ? StString(theVerSub) : (StString('0') + StString(theVerSub));
}

StString stLibAV::Version::toString() const {
    if(myMajor == 0 && myMinor == 0 && myMicro == 0) {
        return "N/A";
    }
    return formatVerSub(myMajor) + '.'
         + formatVerSub(myMinor) + '.'
         + formatVerSub(myMicro);
}

stLibAV::Version stLibAV::Version::libavutil() {
    return stLibAV::Version(::avutil_version());
}

stLibAV::Version stLibAV::Version::libavcodec() {
    return stLibAV::Version(::avcodec_version());
}

stLibAV::Version stLibAV::Version::libavformat() {
    // function available only from version 52.20.0!
    //return stLibAV::Version(::avformat_version());
    return stLibAV::Version(0);
}

stLibAV::Version stLibAV::Version::libavdevice() {
    // we didn't linked to this library, yet
    //return stLibAV::Version(::avdevice_version());
    return stLibAV::Version(0);
}

stLibAV::Version stLibAV::Version::libswscale() {
    return stLibAV::Version(::swscale_version());
}

/// TODO (Kirill Gavrilov#9) replace with av_get_sample_fmt() on next libavcodec major version
#if(LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(53, 0, 0))
    const AVSampleFormat stLibAV::audio::SAMPLE_FMT::U8   = AV_SAMPLE_FMT_U8;  //= av_get_sample_fmt("u8");
    const AVSampleFormat stLibAV::audio::SAMPLE_FMT::S16  = AV_SAMPLE_FMT_S16; //= av_get_sample_fmt("s16");
    const AVSampleFormat stLibAV::audio::SAMPLE_FMT::S32  = AV_SAMPLE_FMT_S32; //= av_get_sample_fmt("s32");
    const AVSampleFormat stLibAV::audio::SAMPLE_FMT::FLT  = AV_SAMPLE_FMT_FLT; //= av_get_sample_fmt("flt");
    const AVSampleFormat stLibAV::audio::SAMPLE_FMT::DBL  = AV_SAMPLE_FMT_DBL; //= av_get_sample_fmt("dbl");
    #if(LIBAVUTIL_VERSION_INT >= AV_VERSION_INT(51, 17, 0))
    const AVSampleFormat stLibAV::audio::SAMPLE_FMT::U8P  = AV_SAMPLE_FMT_U8P;
    const AVSampleFormat stLibAV::audio::SAMPLE_FMT::S16P = AV_SAMPLE_FMT_S16P;
    const AVSampleFormat stLibAV::audio::SAMPLE_FMT::S32P = AV_SAMPLE_FMT_S32P;
    const AVSampleFormat stLibAV::audio::SAMPLE_FMT::FLTP = AV_SAMPLE_FMT_FLTP;
    const AVSampleFormat stLibAV::audio::SAMPLE_FMT::DBLP = AV_SAMPLE_FMT_DBLP;
    #endif
#else
    const SampleFormat   stLibAV::audio::SAMPLE_FMT::U8   =    SAMPLE_FMT_U8;
    const SampleFormat   stLibAV::audio::SAMPLE_FMT::S16  =    SAMPLE_FMT_S16;
    const SampleFormat   stLibAV::audio::SAMPLE_FMT::S32  =    SAMPLE_FMT_S32;
    const SampleFormat   stLibAV::audio::SAMPLE_FMT::FLT  =    SAMPLE_FMT_FLT;
    const SampleFormat   stLibAV::audio::SAMPLE_FMT::DBL  =    SAMPLE_FMT_DBL;
#endif

StString stLibAV::audio::getSampleFormatString(const AVCodecContext* theCtx) {
#if(LIBAVUTIL_VERSION_INT >= AV_VERSION_INT(51, 17, 0))
    const char* aStr = av_get_sample_fmt_name(theCtx->sample_fmt);
    return aStr != NULL ? StString(aStr) : StString("");
#else
    switch(theCtx->sample_fmt) {
        case stLibAV::audio::SAMPLE_FMT::U8:  return stCString("u8");
        case stLibAV::audio::SAMPLE_FMT::S16: return stCString("s16");
        case stLibAV::audio::SAMPLE_FMT::S32: return stCString("s32");
        case stLibAV::audio::SAMPLE_FMT::FLT: return stCString("flt");
        case stLibAV::audio::SAMPLE_FMT::DBL: return stCString("dbl");
        default: return stCString("??");
    }
#endif
}

StString stLibAV::audio::getSampleRateString(const AVCodecContext* theCtx) {
    return StString(theCtx->sample_rate) + " Hz";
}

// copied from avcodec_get_channel_layout_string()
StString stLibAV::audio::getChannelLayoutString(const AVCodecContext* theCtx) {
#if(LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(53, 0, 0))
    if(theCtx->channel_layout == 0) {
        if(theCtx->channels == 1) {
            return "mono";
        } else if(theCtx->channels == 2) {
            return "stereo";
        }
    }
    char aBuff[128]; aBuff[0] = '\0';
    av_get_channel_layout_string(aBuff, sizeof(aBuff),
                                 theCtx->channels, theCtx->channel_layout);
    return aBuff;
#else
    switch(theCtx->channels) {
        case 1: return "mono";
        case 2: return "stereo";
        case 4: {
            switch(theCtx->channel_layout) {
                //case CH_LAYOUT_4POINT0: return "4.0");
                case CH_LAYOUT_QUAD:    return "quad";
                default: return "4.0";
            }
        }
        case 5: return "5.0";
        case 6: return "5.1";
        case 8: {
            switch(theCtx->channel_layout) {
                case CH_LAYOUT_5POINT1|CH_LAYOUT_STEREO_DOWNMIX:
                    return "5.1+downmix";
                case CH_LAYOUT_7POINT1:
                    return "7.1";
                case CH_LAYOUT_7POINT1_WIDE:
                    return "7.1(wide)";
                default:
                    return "unknown 8.0";
            }
        }
        case 10: return "7.1+downmix";
        default: return StString("unknown") + theCtx->channels;
    }
#endif
}

stLibAV::meta::Tag* stLibAV::meta::findTag(stLibAV::meta::Dict*      theDict,
                                           const char*               theKey,
                                           const stLibAV::meta::Tag* thePrevTag,
                                           const int                 theFlags) {
#if(LIBAVUTIL_VERSION_INT >= AV_VERSION_INT(51, 5, 0))
    return av_dict_get    (theDict, theKey, thePrevTag, theFlags);
#else
    return av_metadata_get(theDict, theKey, thePrevTag, theFlags);
#endif
}

bool stLibAV::meta::readTag(stLibAV::meta::Dict* theDict,
                            const StString&      theKey,
                            StString&            theValue) {
    stLibAV::meta::Tag* aTag = stLibAV::meta::findTag(theDict, theKey.toCString(), NULL, 0);
    if(aTag == NULL) {
        return false;
    }
    theValue = aTag->value;
    return true;
}

double stLibAV::unitsToSeconds(const AVRational& theTimeBase,
                               const int64_t     theTimeUnits) {
    return (theTimeUnits != stLibAV::NOPTS_VALUE)
         ? (av_q2d(theTimeBase) * theTimeUnits) : 0.0;
}

double stLibAV::unitsToSeconds(const int64_t theTimeUnits) {
    return (theTimeUnits != stLibAV::NOPTS_VALUE)
         ? (ST_AV_TIME_BASE_D * theTimeUnits) : 0.0;
}

int64_t stLibAV::secondsToUnits(const AVRational& theTimeBase,
                                const double      theTimeSeconds) {
    return int64_t(theTimeSeconds / av_q2d(theTimeBase));
}

int64_t stLibAV::secondsToUnits(const double theTimeSeconds) {
    return int64_t(theTimeSeconds / ST_AV_TIME_BASE_D);
}
