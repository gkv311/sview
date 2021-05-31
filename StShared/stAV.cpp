/**
 * Copyright © 2011-2020 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#include <StAV/stAV.h>
#include <StThreads/StMutex.h>
#include <StStrings/StLogger.h>

// libav* libraries written on pure C,
// and we must around includes manually
extern "C" {
#if(LIBAVUTIL_VERSION_INT >= AV_VERSION_INT(50, 8, 0))
    #include <libavutil/pixdesc.h>
#endif

#if(LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(53, 0, 0))
    // #include <libavresample/avresample.h>
    #include <libavutil/channel_layout.h>
#endif

#if defined(__ANDROID__)
    #include <libavcodec/jni.h>
#endif
};

#ifdef ST_LIBAV_FORK
    #ifdef _MSC_VER
        #pragma message("stAV.cpp(28) : warning C4996: LibAV detected - compatibility workarounds have been activated!")
    #else
        #warning LibAV detected - compatibility workarounds have been activated!
    #endif
#endif

#if(LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(52, 30, 0)) \
&& ((LIBAVCODEC_VERSION_INT < AV_VERSION_INT(58, 9, 100)) || defined(ST_LIBAV_FORK))
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
const int64_t stAV::NOPTS_VALUE = 0x8000000000000000LL;

/**
 * Reproduced AVPixelFormat enumeration.
 */
#if(LIBAVUTIL_VERSION_INT >= AV_VERSION_INT(50, 8, 0))
    #define ST_AV_GETPIXFMT(theName) av_get_pix_fmt(theName)
#else
    #define ST_AV_GETPIXFMT(theName) avcodec_get_pix_fmt(theName)
#endif
const AVPixelFormat stAV::PIX_FMT::NONE       = AVPixelFormat(-1);
const AVPixelFormat stAV::PIX_FMT::YUV420P    = AVPixelFormat( 0);
const AVPixelFormat stAV::PIX_FMT::YUVA420P   = ST_AV_GETPIXFMT("yuva420p");
const AVPixelFormat stAV::PIX_FMT::PAL8       = ST_AV_GETPIXFMT("pal8");
const AVPixelFormat stAV::PIX_FMT::GRAY8      = ST_AV_GETPIXFMT("gray");
const AVPixelFormat stAV::PIX_FMT::GRAY16     = ST_AV_GETPIXFMT("gray16");
const AVPixelFormat stAV::PIX_FMT::YUV422P    = ST_AV_GETPIXFMT("yuv422p");
const AVPixelFormat stAV::PIX_FMT::YUVA422P   = ST_AV_GETPIXFMT("yuva422p");
const AVPixelFormat stAV::PIX_FMT::YUV444P    = ST_AV_GETPIXFMT("yuv444p");
const AVPixelFormat stAV::PIX_FMT::YUVA444P   = ST_AV_GETPIXFMT("yuva444p");
const AVPixelFormat stAV::PIX_FMT::YUV410P    = ST_AV_GETPIXFMT("yuv410p");
const AVPixelFormat stAV::PIX_FMT::YUV411P    = ST_AV_GETPIXFMT("yuv411p");
const AVPixelFormat stAV::PIX_FMT::YUV440P    = ST_AV_GETPIXFMT("yuv440p");
const AVPixelFormat stAV::PIX_FMT::NV12       = ST_AV_GETPIXFMT("nv12");
const AVPixelFormat stAV::PIX_FMT::YUV420P9   = ST_AV_GETPIXFMT("yuv420p9");
const AVPixelFormat stAV::PIX_FMT::YUV422P9   = ST_AV_GETPIXFMT("yuv422p9");
const AVPixelFormat stAV::PIX_FMT::YUV444P9   = ST_AV_GETPIXFMT("yuv444p9");
const AVPixelFormat stAV::PIX_FMT::YUV420P10  = ST_AV_GETPIXFMT("yuv420p10");
const AVPixelFormat stAV::PIX_FMT::YUV422P10  = ST_AV_GETPIXFMT("yuv422p10");
const AVPixelFormat stAV::PIX_FMT::YUV444P10  = ST_AV_GETPIXFMT("yuv444p10");
const AVPixelFormat stAV::PIX_FMT::YUV420P16  = ST_AV_GETPIXFMT("yuv420p16");
const AVPixelFormat stAV::PIX_FMT::YUV422P16  = ST_AV_GETPIXFMT("yuv422p16");
const AVPixelFormat stAV::PIX_FMT::YUV444P16  = ST_AV_GETPIXFMT("yuv444p16");
const AVPixelFormat stAV::PIX_FMT::YUVJ420P   = ST_AV_GETPIXFMT("yuvj420p");
const AVPixelFormat stAV::PIX_FMT::YUVJ422P   = ST_AV_GETPIXFMT("yuvj422p");
const AVPixelFormat stAV::PIX_FMT::YUVJ444P   = ST_AV_GETPIXFMT("yuvj444p");
const AVPixelFormat stAV::PIX_FMT::YUVJ440P   = ST_AV_GETPIXFMT("yuvj440p");
const AVPixelFormat stAV::PIX_FMT::RGB24      = ST_AV_GETPIXFMT("rgb24");
const AVPixelFormat stAV::PIX_FMT::BGR24      = ST_AV_GETPIXFMT("bgr24");
const AVPixelFormat stAV::PIX_FMT::RGB48      = ST_AV_GETPIXFMT("rgb48");
const AVPixelFormat stAV::PIX_FMT::BGR48      = ST_AV_GETPIXFMT("bgr48");
const AVPixelFormat stAV::PIX_FMT::RGBA64     = ST_AV_GETPIXFMT("rgba64");
const AVPixelFormat stAV::PIX_FMT::BGRA64     = ST_AV_GETPIXFMT("bgra64");
const AVPixelFormat stAV::PIX_FMT::XYZ12      = ST_AV_GETPIXFMT("xyz12");
const AVPixelFormat stAV::PIX_FMT::DXVA2_VLD  = ST_AV_GETPIXFMT("dxva2_vld");
const AVPixelFormat stAV::PIX_FMT::VIDEOTOOLBOX_VLD  = ST_AV_GETPIXFMT("videotoolbox_vld");

// TODO (Kirill Gavrilov#9) remove this stuff
namespace {
    static const AVPixelFormat AvPixFmtRGBA  = ST_AV_GETPIXFMT("rgba");
    static const AVPixelFormat AvPixFmtBGRA  = ST_AV_GETPIXFMT("bgra");
    static const AVPixelFormat AvPixFmtRGB32 = ST_AV_GETPIXFMT("rgb32"); // compatibility with old FFmpeg
    static const AVPixelFormat AvPixFmtBGR32 = ST_AV_GETPIXFMT("bgr32"); // compatibility with old FFmpeg

    static const AVRational ST_AV_TIME_BASE_Q = {1, AV_TIME_BASE};
    static const double     ST_AV_TIME_BASE_D = av_q2d(ST_AV_TIME_BASE_Q);
};

const AVPixelFormat stAV::PIX_FMT::RGBA32 = (AvPixFmtRGBA != stAV::PIX_FMT::NONE) ? AvPixFmtRGBA : AvPixFmtBGR32;
const AVPixelFormat stAV::PIX_FMT::BGRA32 = (AvPixFmtBGRA != stAV::PIX_FMT::NONE) ? AvPixFmtBGRA : AvPixFmtRGB32;

StCString stAV::PIX_FMT::getString(const AVPixelFormat theFrmt) {
    if(theFrmt == stAV::PIX_FMT::NONE) {
        return stCString("none");
    } else if(theFrmt == stAV::PIX_FMT::YUV420P) {
        return stCString("yuv420p");
    } else if(theFrmt == stAV::PIX_FMT::YUVA420P) {
        return stCString("yuva420p");
    } else if(theFrmt == stAV::PIX_FMT::PAL8) {
        return stCString("pal8");
    } else if(theFrmt == stAV::PIX_FMT::GRAY8) {
        return stCString("gray8");
    } else if(theFrmt == stAV::PIX_FMT::GRAY16) {
        return stCString("gray16");
    } else if(theFrmt == stAV::PIX_FMT::YUV422P) {
        return stCString("yuv422p");
    } else if(theFrmt == stAV::PIX_FMT::YUVA422P) {
        return stCString("yuva422p");
    } else if(theFrmt == stAV::PIX_FMT::YUV444P) {
        return stCString("yuv444p");
    } else if(theFrmt == stAV::PIX_FMT::YUVA444P) {
        return stCString("yuva444p");
    } else if(theFrmt == stAV::PIX_FMT::YUV410P) {
        return stCString("yuv410p");
    } else if(theFrmt == stAV::PIX_FMT::YUV411P) {
        return stCString("yuv411p");
    } else if(theFrmt == stAV::PIX_FMT::YUV440P) {
        return stCString("yuv440p");
    } else if(theFrmt == stAV::PIX_FMT::YUV420P9) {
        return stCString("yuv420p9");
    } else if(theFrmt == stAV::PIX_FMT::YUV422P9) {
        return stCString("yuv422p9");
    } else if(theFrmt == stAV::PIX_FMT::YUV444P9) {
        return stCString("yuv444p9");
    } else if(theFrmt == stAV::PIX_FMT::YUV420P10) {
        return stCString("yuv420p10");
    } else if(theFrmt == stAV::PIX_FMT::YUV422P10) {
        return stCString("yuv422p10");
    } else if(theFrmt == stAV::PIX_FMT::YUV444P10) {
        return stCString("yuv444p10");
    } else if(theFrmt == stAV::PIX_FMT::YUV420P16) {
        return stCString("yuv420p16");
    } else if(theFrmt == stAV::PIX_FMT::YUV422P16) {
        return stCString("yuv422p16");
    } else if(theFrmt == stAV::PIX_FMT::YUV444P16) {
        return stCString("yuv444p16");
    } else if(theFrmt == stAV::PIX_FMT::YUVJ420P) {
        return stCString("yuvj420p");
    } else if(theFrmt == stAV::PIX_FMT::YUVJ422P) {
        return stCString("yuvj422p");
    } else if(theFrmt == stAV::PIX_FMT::YUVJ444P) {
        return stCString("yuvj444p");
    } else if(theFrmt == stAV::PIX_FMT::YUVJ440P) {
        return stCString("yuvj440p");
    } else if(theFrmt == stAV::PIX_FMT::RGB24) {
        return stCString("rgb24");
    } else if(theFrmt == stAV::PIX_FMT::BGR24) {
        return stCString("bgr24");
    } else if(theFrmt == stAV::PIX_FMT::RGB48) {
        return stCString("rgb48");
    } else if(theFrmt == stAV::PIX_FMT::BGR48) {
        return stCString("bgr48");
    } else if(theFrmt == stAV::PIX_FMT::RGBA32) {
        return stCString("rgba32");
    } else if(theFrmt == stAV::PIX_FMT::BGRA32) {
        return stCString("bgra32");
    } else if(theFrmt == stAV::PIX_FMT::RGBA64) {
        return stCString("rgba64");
    } else if(theFrmt == stAV::PIX_FMT::BGRA64) {
        return stCString("bgra64");
    } else if(theFrmt == stAV::PIX_FMT::NV12) {
        return stCString("nv12");
    } else if(theFrmt == stAV::PIX_FMT::XYZ12) {
        return stCString("xyz12");
    } else if(theFrmt == stAV::PIX_FMT::DXVA2_VLD) {
        return stCString("dxva2_vld");
    } else {
        return stCString("unknown");
    }
}

namespace {

#if defined(ST_DEBUG_FFMPEG) || defined(ST_DEBUG_FFMPEG_VERBOSE)
    /**
     * Logger callback.
     */
    static void stAvLogCallback(void* thePtr, int theLevel, const char* theFormat, va_list theArgs) {
    #if !defined(ST_DEBUG_FFMPEG_VERBOSE)
        if(theLevel > AV_LOG_INFO) {
            return;
        }
    #endif

        //static int aPrintPrefix = 1;
        int aPrintPrefix = 1;
        char aLine[1024];
        av_log_format_line(thePtr, theLevel, theFormat, theArgs,
                           aLine, sizeof(aLine), &aPrintPrefix);

        StLogger::Level aLevelSt = StLogger::ST_TRACE;
        switch(theLevel) {
            case AV_LOG_QUIET:   aLevelSt = StLogger::ST_QUIET;   break;
            case AV_LOG_PANIC:   aLevelSt = StLogger::ST_PANIC;   break;
            case AV_LOG_FATAL:   aLevelSt = StLogger::ST_FATAL;   break;
            case AV_LOG_ERROR:   aLevelSt = StLogger::ST_ERROR;   break;
            case AV_LOG_WARNING: aLevelSt = StLogger::ST_WARNING; break;
            case AV_LOG_INFO:    aLevelSt = StLogger::ST_INFO;    break;
            case AV_LOG_VERBOSE: aLevelSt = StLogger::ST_VERBOSE; break;
            case AV_LOG_DEBUG:
        #ifdef AV_LOG_TRACE
            case AV_LOG_TRACE:
        #endif
            default: aLevelSt = StLogger::ST_TRACE; break;
        }

        StLogger::GetDefault().write(StString(aLine), aLevelSt);
    }
#endif

    static bool initOnce() {
    #if(LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(52, 30, 0)) \
    && ((LIBAVCODEC_VERSION_INT < AV_VERSION_INT(58, 9, 100)) || defined(ST_LIBAV_FORK))
        // register own mutex to prevent multithreading errors
        // while using FFmpeg functions
        stFFMpegLocker.init();
    #endif

    #if(LIBAVFORMAT_VERSION_INT < AV_VERSION_INT(58, 9, 100)) || defined(ST_LIBAV_FORK)
        // Notice, this call is absolutely not thread safe!
        // you should never call it first time from concurrent threads.
        // But after first initialization is done this is safe to call it anyhow
        av_register_all();
    #endif

    #if(LIBAVFORMAT_VERSION_INT >= AV_VERSION_INT(53, 13, 0))
        avformat_network_init();
    #endif
        ST_DEBUG_LOG("FFmpeg initialized:");

    #if defined(ST_DEBUG_FFMPEG) || defined(ST_DEBUG_FFMPEG_VERBOSE)
        av_log_set_callback(stAvLogCallback);
    #endif

        // show up information about dynamically linked libraries
        ST_DEBUG_LOG("  libavutil\t"   + stAV::Version::libavutil().toString());
        ST_DEBUG_LOG("  libavcodec\t"  + stAV::Version::libavcodec().toString());
        ST_DEBUG_LOG("  libavformat\t" + stAV::Version::libavformat().toString());
        ST_DEBUG_LOG("  libavdevice\t" + stAV::Version::libavdevice().toString());
        ST_DEBUG_LOG("  libswscale\t"  + stAV::Version::libswscale().toString());
        return true;
    }

};

bool stAV::init() {
    static const bool isFFmpegInitiailed = initOnce();
    return isFFmpegInitiailed;
}

bool stAV::isFormatYUVPlanar(const AVCodecContext* theCtx) {
    return theCtx->pix_fmt == stAV::PIX_FMT::YUV420P
        || theCtx->pix_fmt == stAV::PIX_FMT::YUVA420P
        || theCtx->pix_fmt == stAV::PIX_FMT::YUVJ420P
        || theCtx->pix_fmt == stAV::PIX_FMT::YUV422P
        || theCtx->pix_fmt == stAV::PIX_FMT::YUVA422P
        || theCtx->pix_fmt == stAV::PIX_FMT::YUVJ422P
        || theCtx->pix_fmt == stAV::PIX_FMT::YUV444P
        || theCtx->pix_fmt == stAV::PIX_FMT::YUVA444P
        || theCtx->pix_fmt == stAV::PIX_FMT::YUVJ444P
        || theCtx->pix_fmt == stAV::PIX_FMT::YUV440P
        || theCtx->pix_fmt == stAV::PIX_FMT::YUVJ440P
        || theCtx->pix_fmt == stAV::PIX_FMT::YUV411P
        || theCtx->pix_fmt == stAV::PIX_FMT::YUV410P
        || theCtx->pix_fmt == stAV::PIX_FMT::YUV420P9
        || theCtx->pix_fmt == stAV::PIX_FMT::YUV422P9
        || theCtx->pix_fmt == stAV::PIX_FMT::YUV444P9
        || theCtx->pix_fmt == stAV::PIX_FMT::YUV420P10
        || theCtx->pix_fmt == stAV::PIX_FMT::YUV422P10
        || theCtx->pix_fmt == stAV::PIX_FMT::YUV444P10
        || theCtx->pix_fmt == stAV::PIX_FMT::YUV420P16
        || theCtx->pix_fmt == stAV::PIX_FMT::YUV422P16
        || theCtx->pix_fmt == stAV::PIX_FMT::YUV444P16;
}

bool stAV::isFormatYUVPlanar(const AVPixelFormat thePixFmt,
                             const int           theWidth,
                             const int           theHeight,
                             dimYUV&             theDims) {
    if(thePixFmt == stAV::PIX_FMT::NONE) {
        return false;
    } else if(thePixFmt == stAV::PIX_FMT::YUV420P
           || thePixFmt == stAV::PIX_FMT::YUVA420P
           || thePixFmt == stAV::PIX_FMT::YUVJ420P
           || thePixFmt == stAV::PIX_FMT::YUV420P9
           || thePixFmt == stAV::PIX_FMT::YUV420P10
           || thePixFmt == stAV::PIX_FMT::YUV420P16) {
        theDims.widthY  = theWidth;
        theDims.heightY = theHeight;
        theDims.widthU  = theDims.widthV  = theDims.widthY  / 2;
        theDims.heightU = theDims.heightV = theDims.heightY / 2;
        theDims.isFullScale = (thePixFmt == stAV::PIX_FMT::YUVJ420P);
        theDims.hasAlpha = thePixFmt == stAV::PIX_FMT::YUVA420P;
    } else if(thePixFmt == stAV::PIX_FMT::YUV422P
           || thePixFmt == stAV::PIX_FMT::YUVA422P
           || thePixFmt == stAV::PIX_FMT::YUVJ422P
           || thePixFmt == stAV::PIX_FMT::YUV422P9
           || thePixFmt == stAV::PIX_FMT::YUV422P10
           || thePixFmt == stAV::PIX_FMT::YUV422P16) {
        theDims.widthY  = theWidth;
        theDims.heightY = theDims.heightU = theDims.heightV = theHeight;
        theDims.widthU  = theDims.widthV  = theDims.widthY  / 2;
        theDims.isFullScale = (thePixFmt == stAV::PIX_FMT::YUVJ422P);
        theDims.hasAlpha = false;
    } else if(thePixFmt == stAV::PIX_FMT::YUV444P
           || thePixFmt == stAV::PIX_FMT::YUVA444P
           || thePixFmt == stAV::PIX_FMT::YUVJ444P
           || thePixFmt == stAV::PIX_FMT::YUV444P9
           || thePixFmt == stAV::PIX_FMT::YUV444P10
           || thePixFmt == stAV::PIX_FMT::YUV444P16) {
        theDims.widthY  = theDims.widthU  = theDims.widthV  = theWidth;
        theDims.heightY = theDims.heightU = theDims.heightV = theHeight;
        theDims.isFullScale = (thePixFmt == stAV::PIX_FMT::YUVJ444P);
        theDims.hasAlpha = false;
    } else if(thePixFmt == stAV::PIX_FMT::YUV440P
           || thePixFmt == stAV::PIX_FMT::YUVJ440P) {
        theDims.widthY  = theDims.widthU  = theDims.widthV  = theWidth;
        theDims.heightY = theHeight;
        theDims.heightU = theDims.heightV = theDims.heightY / 2;
        theDims.isFullScale = (thePixFmt == stAV::PIX_FMT::YUVJ440P);
        theDims.hasAlpha = false;
    } else if(thePixFmt == stAV::PIX_FMT::YUV411P) {
        theDims.widthY  = theDims.widthU  = theDims.widthV  = theWidth;
        theDims.heightY = theDims.heightU = theDims.heightV = theHeight;
        theDims.widthU  = theDims.widthV  = theDims.widthY  / 4;
        theDims.isFullScale = false;
        theDims.hasAlpha = false;
    } else if(thePixFmt == stAV::PIX_FMT::YUV410P) {
        theDims.widthY  = theDims.widthU  = theDims.widthV  = theWidth;
        theDims.heightY = theHeight;
        theDims.widthU  = theDims.widthV  = theDims.widthY  / 4;
        theDims.heightU = theDims.heightV = theDims.heightY / 4;
        theDims.isFullScale = false;
        theDims.hasAlpha = false;
    } else {
        return false;
    }

    if(thePixFmt == stAV::PIX_FMT::YUV420P9
    || thePixFmt == stAV::PIX_FMT::YUV422P9
    || thePixFmt == stAV::PIX_FMT::YUV444P9) {
        theDims.bitsPerComp = 9;
    } else if(thePixFmt == stAV::PIX_FMT::YUV420P10
           || thePixFmt == stAV::PIX_FMT::YUV422P10
           || thePixFmt == stAV::PIX_FMT::YUV444P10) {
        theDims.bitsPerComp = 10;
    } else if(thePixFmt == stAV::PIX_FMT::YUV420P16
           || thePixFmt == stAV::PIX_FMT::YUV422P16
           || thePixFmt == stAV::PIX_FMT::YUV444P16) {
        theDims.bitsPerComp = 16;
    } else {
        theDims.bitsPerComp = 8;
    }

    return true;
}

StString stAV::getAVErrorDescription(int avErrCode) {
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

stAV::Version::Version(const unsigned theVersionInt)
: myMajor(theVersionInt >> 16 & 0xFF),
  myMinor(theVersionInt >>  8 & 0xFF),
  myMicro(theVersionInt       & 0xFF) {
    //
}

inline StString formatVerSub(const unsigned theVerSub) {
    return (theVerSub > 10) ? StString(theVerSub) : (StString('0') + StString(theVerSub));
}

StString stAV::Version::toString() const {
    if(myMajor == 0 && myMinor == 0 && myMicro == 0) {
        return "N/A";
    }
    return formatVerSub(myMajor) + '.'
         + formatVerSub(myMinor) + '.'
         + formatVerSub(myMicro);
}

stAV::Version stAV::Version::libavutil() {
    return stAV::Version(::avutil_version());
}

stAV::Version stAV::Version::libavcodec() {
    return stAV::Version(::avcodec_version());
}

stAV::Version stAV::Version::libavformat() {
    // function available only from version 52.20.0!
    //return stAV::Version(::avformat_version());
    return stAV::Version(0);
}

stAV::Version stAV::Version::libavdevice() {
    // we didn't linked to this library, yet
    //return stAV::Version(::avdevice_version());
    return stAV::Version(0);
}

stAV::Version stAV::Version::libswscale() {
    return stAV::Version(::swscale_version());
}

/// TODO (Kirill Gavrilov#9) replace with av_get_sample_fmt() on next libavcodec major version
#if(LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(53, 0, 0))
    const AVSampleFormat stAV::audio::SAMPLE_FMT::U8   = AV_SAMPLE_FMT_U8;  //= av_get_sample_fmt("u8");
    const AVSampleFormat stAV::audio::SAMPLE_FMT::S16  = AV_SAMPLE_FMT_S16; //= av_get_sample_fmt("s16");
    const AVSampleFormat stAV::audio::SAMPLE_FMT::S32  = AV_SAMPLE_FMT_S32; //= av_get_sample_fmt("s32");
    const AVSampleFormat stAV::audio::SAMPLE_FMT::FLT  = AV_SAMPLE_FMT_FLT; //= av_get_sample_fmt("flt");
    const AVSampleFormat stAV::audio::SAMPLE_FMT::DBL  = AV_SAMPLE_FMT_DBL; //= av_get_sample_fmt("dbl");
    #if(LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(53, 40, 0))
    const AVSampleFormat stAV::audio::SAMPLE_FMT::U8P  = AV_SAMPLE_FMT_U8P;
    const AVSampleFormat stAV::audio::SAMPLE_FMT::S16P = AV_SAMPLE_FMT_S16P;
    const AVSampleFormat stAV::audio::SAMPLE_FMT::S32P = AV_SAMPLE_FMT_S32P;
    const AVSampleFormat stAV::audio::SAMPLE_FMT::FLTP = AV_SAMPLE_FMT_FLTP;
    const AVSampleFormat stAV::audio::SAMPLE_FMT::DBLP = AV_SAMPLE_FMT_DBLP;
    #else
    const AVSampleFormat stAV::audio::SAMPLE_FMT::U8P  = (AVSampleFormat )-1;
    const AVSampleFormat stAV::audio::SAMPLE_FMT::S16P = (AVSampleFormat )-1;
    const AVSampleFormat stAV::audio::SAMPLE_FMT::S32P = (AVSampleFormat )-1;
    const AVSampleFormat stAV::audio::SAMPLE_FMT::FLTP = (AVSampleFormat )-1;
    const AVSampleFormat stAV::audio::SAMPLE_FMT::DBLP = (AVSampleFormat )-1;
    #endif
#else
    const SampleFormat   stAV::audio::SAMPLE_FMT::U8   = SAMPLE_FMT_U8;
    const SampleFormat   stAV::audio::SAMPLE_FMT::S16  = SAMPLE_FMT_S16;
    const SampleFormat   stAV::audio::SAMPLE_FMT::S32  = SAMPLE_FMT_S32;
    const SampleFormat   stAV::audio::SAMPLE_FMT::FLT  = SAMPLE_FMT_FLT;
    const SampleFormat   stAV::audio::SAMPLE_FMT::DBL  = SAMPLE_FMT_DBL;
#endif

StString stAV::audio::getSampleFormatString(const AVCodecContext* theCtx) {
#if(LIBAVUTIL_VERSION_INT >= AV_VERSION_INT(51, 17, 0))
    const char* aStr = av_get_sample_fmt_name(theCtx->sample_fmt);
    return aStr != NULL ? StString(aStr) : StString("");
#else
    switch(theCtx->sample_fmt) {
        case stAV::audio::SAMPLE_FMT::U8:  return stCString("u8");
        case stAV::audio::SAMPLE_FMT::S16: return stCString("s16");
        case stAV::audio::SAMPLE_FMT::S32: return stCString("s32");
        case stAV::audio::SAMPLE_FMT::FLT: return stCString("flt");
        case stAV::audio::SAMPLE_FMT::DBL: return stCString("dbl");
        default: return stCString("??");
    }
#endif
}

StString stAV::audio::getSampleRateString(const AVCodecContext* theCtx) {
    return StString(theCtx->sample_rate) + " Hz";
}

// copied from avcodec_get_channel_layout_string()
StString stAV::audio::getChannelLayoutString(const AVCodecContext* theCtx) {
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

stAV::meta::Tag* stAV::meta::findTag(stAV::meta::Dict*      theDict,
                                     const char*            theKey,
                                     const stAV::meta::Tag* thePrevTag,
                                     const int              theFlags) {
#if(LIBAVUTIL_VERSION_INT >= AV_VERSION_INT(51, 5, 0))
    return av_dict_get    (theDict, theKey, thePrevTag, theFlags);
#else
    return av_metadata_get(theDict, theKey, thePrevTag, theFlags);
#endif
}

bool stAV::meta::readTag(stAV::meta::Dict* theDict,
                         const StCString&  theKey,
                         StString&         theValue) {
    if(theDict == NULL) {
        return false;
    }

    stAV::meta::Tag* aTag = stAV::meta::findTag(theDict, theKey.toCString(), NULL, 0);
    if(aTag == NULL) {
        return false;
    }
    theValue = aTag->value;
    return true;
}

stAV::meta::Dict* stAV::meta::getFrameMetadata(AVFrame* theFrame) {
#if(LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(57, 81, 102)) && !defined(ST_LIBAV_FORK)
    return theFrame->metadata;
#elif(LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(54, 38, 100)) && !defined(ST_LIBAV_FORK)
    return av_frame_get_metadata(theFrame);
#else
    return NULL;
#endif
}

double stAV::unitsToSeconds(const AVRational& theTimeBase,
                            const int64_t     theTimeUnits) {
    return (theTimeUnits != stAV::NOPTS_VALUE)
         ? (av_q2d(theTimeBase) * theTimeUnits) : 0.0;
}

double stAV::unitsToSeconds(const int64_t theTimeUnits) {
    return (theTimeUnits != stAV::NOPTS_VALUE)
         ? (ST_AV_TIME_BASE_D * theTimeUnits) : 0.0;
}

int64_t stAV::secondsToUnits(const AVRational& theTimeBase,
                             const double      theTimeSeconds) {
    return int64_t(theTimeSeconds / av_q2d(theTimeBase));
}

int64_t stAV::secondsToUnits(const double theTimeSeconds) {
    return int64_t(theTimeSeconds / ST_AV_TIME_BASE_D);
}

bool stAV::setJavaVM(void* theJavaVM) {
#if defined(__ANDROID__)
    return av_jni_set_java_vm(theJavaVM, NULL) == 0;
#else
    (void )theJavaVM;
    return false;
#endif
}

bool stAV::isEnabledInputProtocol(const StString& theProtocol) {
    void* anOpaque = NULL;
    for(const char* aName = avio_enum_protocols(&anOpaque, 0); aName != NULL; aName = avio_enum_protocols(&anOpaque, 0)) {
        if(stAreEqual(theProtocol.toCString(), aName, theProtocol.Size + 1)) {
            return true;
        }
    }
    return false;
}

StString stAV::getVersionInfo() {
#if(LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(57, 24, 0))
    return av_version_info();
#else
    return "";
#endif
}

StString stAV::getLicenseInfo() {
#if(LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(57, 24, 0))
    return avutil_license();
#else
    return "";
#endif
}
