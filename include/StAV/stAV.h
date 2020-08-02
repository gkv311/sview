/**
 * Copyright © 2011-2020 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#ifndef __stAV_h_
#define __stAV_h_

#include <StGLStereo/StFormatEnum.h>

// libav* libraries written on pure C,
// and we must around includes manually
extern "C" {
#ifdef _MSC_VER
    // suppress some common warnings in FFmpeg headers
    #pragma warning(disable : 4244)
#endif

    #include <libavcodec/avcodec.h>
    #include <libavformat/avformat.h>
    #include <libavutil/avutil.h>
    #include <libswscale/swscale.h>

    #include <libavutil/mathematics.h>

    // new stereoscopic info API
#if(LIBAVUTIL_VERSION_INT >= AV_VERSION_INT(52, 56, 100))
    #define ST_AV_NEWSTEREO
    #include <libavutil/stereo3d.h>
#endif

#if(LIBAVUTIL_VERSION_INT >= AV_VERSION_INT(55, 0, 0))
    #include <libavutil/display.h>
#endif

#if(LIBAVUTIL_VERSION_INT >= AV_VERSION_INT(55, 57, 100))
    #define ST_AV_NEWSPHERICAL
    #include <libavutil/spherical.h>
#endif

#if(LIBAVFORMAT_VERSION_INT >= AV_VERSION_INT(57, 33, 100))
    #define ST_AV_NEWCODECPAR
#endif

#ifndef AV_NUM_DATA_POINTERS
    #define AV_NUM_DATA_POINTERS 4
#endif

// Compatibility with older FFmpeg (API change since 2015-07-27 - lavc 56.56.100 / 56.35.0)
#ifndef AV_INPUT_BUFFER_PADDING_SIZE
    #define AV_INPUT_BUFFER_PADDING_SIZE FF_INPUT_BUFFER_PADDING_SIZE
#endif
#ifndef AV_CODEC_FLAG_GLOBAL_HEADER
    #define AV_CODEC_FLAG_GLOBAL_HEADER CODEC_FLAG_GLOBAL_HEADER
#endif
#ifndef AV_CODEC_FLAG2_IGNORE_CROP
    #define AV_CODEC_FLAG2_IGNORE_CROP CODEC_FLAG2_IGNORE_CROP
#endif

#ifdef _MSC_VER
    #pragma warning(default : 4244)
#endif
};

// Detect FFmpeg vs. LibAV.
// Current check is unsafe - there is no guarantee that flag AVIO_FLAG_DIRECT
// (introduced in libavformat/avio.h since '2012)
// will not be removed in further FFmpeg release,
// will not be converted to enum,
// and LibAV will not merge this change
#if(LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(54, 18, 100)) && !defined(AVIO_FLAG_DIRECT)
    #define ST_LIBAV_FORK
#endif

#if(LIBAVUTIL_VERSION_INT < AV_VERSION_INT(50, 13, 0))
    // enumeration was renamed from CodecType
    enum AVMediaType {
        AVMEDIA_TYPE_UNKNOWN = -1,
        AVMEDIA_TYPE_VIDEO,
        AVMEDIA_TYPE_AUDIO,
        AVMEDIA_TYPE_DATA,
        AVMEDIA_TYPE_SUBTITLE,
        AVMEDIA_TYPE_ATTACHMENT,
        AVMEDIA_TYPE_NB
    };
#endif


#if(LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(52, 30, 0))
    // own lock function
    extern "C" int stFFmpegLock(void** theMutexPtrPtr, enum AVLockOp theOperation);
#endif

#ifdef ST_SHARED_DLL
    #define ST_SHARED_CPPEXPORT ST_CPPEXPORT extern const
#else
    #define ST_SHARED_CPPEXPORT ST_CPPIMPORT extern const
#endif

/**
 * This namespace to perform libav* initialization only once
 */
namespace stAV {

//!< entry functions

    /**
     * Should be called at application start.
     */
    ST_CPPEXPORT bool init();

    /**
     * Returns string description for AVError code.
     */
    ST_CPPEXPORT StString getAVErrorDescription(int avErrCode);

    /**
     * Version-control. This version may be used to provide information to user
     * and to perform run-time compatibility checks.
     */
    struct Version {

        unsigned myMajor; //!< Major version number, indicates API changes
        unsigned myMinor;
        unsigned myMicro;

            public:

        /**
         * Initialize the version structure from integer, returned by libav* library.
         */
        ST_CPPEXPORT Version(const unsigned theVersionInt);

        /**
         * Returns the string representation for version.
         */
        ST_CPPEXPORT StString toString() const;

            public: //!< return the version for each library

        ST_CPPEXPORT static Version libavutil();
        ST_CPPEXPORT static Version libavcodec();
        ST_CPPEXPORT static Version libavformat();
        ST_CPPEXPORT static Version libavdevice();
        ST_CPPEXPORT static Version libswscale();

    };

    // this is just redeclaration AV_NOPTS_VALUE
    ST_SHARED_CPPEXPORT int64_t NOPTS_VALUE;

    /**
     * Convert time units into seconds. Returns zero for invalid value.
     * @param theTimeBase  the timebase
     * @param theTimeUnits value to convert
     * @return converted time units in seconds
     */
    ST_CPPEXPORT double unitsToSeconds(const AVRational& theTimeBase,
                                       const int64_t     theTimeUnits);

    /**
     * Convert seconds into time units.
     * @param theTimeBase    the timebase
     * @param theTimeSeconds value to convert
     * @return time units
     */
    ST_CPPEXPORT int64_t secondsToUnits(const AVRational& theTimeBase,
                                        const double      theTimeSeconds);

    /**
     * Convert time units into seconds using stream base.
     * @param theStream    (const AVStream* ) - the stream;
     * @param theTimeUnits (const int64_t   ) - value to convert;
     * @return converted time units in seconds.
     */
    inline double unitsToSeconds(const AVStream* theStream,
                                 const int64_t   theTimeUnits) {
        return stAV::unitsToSeconds(theStream->time_base, theTimeUnits);
    }

    /**
     * Convert seconds into time units for stream.
     * @param theStream      the stream
     * @param theTimeSeconds value to convert
     * @return time units
     */
    inline int64_t secondsToUnits(const AVStream* theStream,
                                  const double    theTimeSeconds) {
        return stAV::secondsToUnits(theStream->time_base, theTimeSeconds);
    }

    /**
     * Convert time units into seconds for context.
     * @param theTimeUnits value to convert
     * @return converted time units in seconds
     */
    ST_CPPEXPORT double unitsToSeconds(const int64_t theTimeUnits);

    /**
     * Convert seconds into time units for context.
     * @param theTimeSeconds value to convert
     * @return time units
     */
    ST_CPPEXPORT int64_t secondsToUnits(const double theTimeSeconds);

    /**
     * Image frame pixel formats. Originally defined as enumerations
     * but because them not maintained in future-compatible way (thus - changed between not major versions)
     * redefined as extern values.
     *
     * Colorscale values:
     *  - full,  8 bits:               0..255
     *  - MPEG,  8 bits:              16..235   for Y,   16..240   for U and V
     *  - full,  9 bits in 16 bits:    0..511
     *  - MPEG,  9 bits in 16 bits:   32..470   for Y,   32..480   for U and V
     *  - full, 10 bits in 16 bits:    0..1023
     *  - MPEG, 10 bits in 16 bits:   64..940   for Y,   64..960   for U and V
     *  - full, 16 bits:               0..65535
     *  - MPEG, 16 bits:            4096..60160 for Y, 4096..61440 for U and V
     */
    namespace PIX_FMT {
        ST_SHARED_CPPEXPORT AVPixelFormat NONE;
        ST_SHARED_CPPEXPORT AVPixelFormat PAL8;      //!< palette

        ST_SHARED_CPPEXPORT AVPixelFormat GRAY8;     //!< Y,  8bpp
        ST_SHARED_CPPEXPORT AVPixelFormat GRAY16;    //!< Y, 16bpp
        // planar YUV formats
        ST_SHARED_CPPEXPORT AVPixelFormat YUV420P;   //!< planar YUV 4:2:0, 12bpp, (1 Cr & Cb sample per 2x2 Y samples)
        ST_SHARED_CPPEXPORT AVPixelFormat YUVA420P;  //!< planar YUV 4:2:0, 20bpp, (1 Cr & Cb sample per 2x2 Y & A samples)
        ST_SHARED_CPPEXPORT AVPixelFormat YUV422P;   //!< planar YUV 4:2:2, 16bpp, (1 Cr & Cb sample per 2x1 Y samples)
        ST_SHARED_CPPEXPORT AVPixelFormat YUVA422P;  //!< planar YUV 4:2:2  24bpp, (1 Cr & Cb sample per 2x1 Y & A samples)
        ST_SHARED_CPPEXPORT AVPixelFormat YUV444P;   //!< planar YUV 4:4:4, 24bpp, (1 Cr & Cb sample per 1x1 Y samples)
        ST_SHARED_CPPEXPORT AVPixelFormat YUVA444P;  //!< planar YUV 4:4:4  32bpp, (1 Cr & Cb sample per 1x1 Y & A samples)
        ST_SHARED_CPPEXPORT AVPixelFormat YUV410P;   //!< planar YUV 4:1:0,  9bpp, (1 Cr & Cb sample per 4x4 Y samples)
        ST_SHARED_CPPEXPORT AVPixelFormat YUV411P;   //!< planar YUV 4:1:1, 12bpp, (1 Cr & Cb sample per 4x1 Y samples)
        ST_SHARED_CPPEXPORT AVPixelFormat YUV440P;   //!< planar YUV 4:4:0 (1 Cr & Cb sample per 1x2 Y samples)
        ST_SHARED_CPPEXPORT AVPixelFormat NV12;      //!< YUV420, Y plane + interleaved UV plane oh half width and height
        // wide planar YUV formats (9,10,14,16 bits stored in 16 bits)
        ST_SHARED_CPPEXPORT AVPixelFormat YUV420P9;
        ST_SHARED_CPPEXPORT AVPixelFormat YUV422P9;
        ST_SHARED_CPPEXPORT AVPixelFormat YUV444P9;
        ST_SHARED_CPPEXPORT AVPixelFormat YUV420P10;
        ST_SHARED_CPPEXPORT AVPixelFormat YUV422P10;
        ST_SHARED_CPPEXPORT AVPixelFormat YUV444P10;
        ST_SHARED_CPPEXPORT AVPixelFormat YUV420P16;
        ST_SHARED_CPPEXPORT AVPixelFormat YUV422P16;
        ST_SHARED_CPPEXPORT AVPixelFormat YUV444P16;
        //extern const AVPixelFormat YUVA420P; ///< planar YUV 4:2:0, 20bpp, (1 Cr & Cb sample per 2x2 Y & A samples)
        // fullscale YUV formats (deprecated?)
        ST_SHARED_CPPEXPORT AVPixelFormat YUVJ420P;  //!< planar YUV 4:2:0, 12bpp, full scale (JPEG)
        ST_SHARED_CPPEXPORT AVPixelFormat YUVJ422P;  //!< planar YUV 4:2:2, 16bpp, full scale (JPEG)
        ST_SHARED_CPPEXPORT AVPixelFormat YUVJ444P;  //!< planar YUV 4:4:4, 24bpp, full scale (JPEG)
        ST_SHARED_CPPEXPORT AVPixelFormat YUVJ440P;  //!< planar YUV 4:4:0 full scale (JPEG)
        // RGB formats
        ST_SHARED_CPPEXPORT AVPixelFormat RGB24;     //!< packed RGB 8:8:8, 24bpp, RGBRGB...
        ST_SHARED_CPPEXPORT AVPixelFormat BGR24;     //!< packed RGB 8:8:8, 24bpp, BGRBGR...
        ST_SHARED_CPPEXPORT AVPixelFormat RGB48;
        ST_SHARED_CPPEXPORT AVPixelFormat BGR48;
        ST_SHARED_CPPEXPORT AVPixelFormat RGBA32;
        ST_SHARED_CPPEXPORT AVPixelFormat BGRA32;
        ST_SHARED_CPPEXPORT AVPixelFormat RGBA64;
        ST_SHARED_CPPEXPORT AVPixelFormat BGRA64;
        // XYZ formats
        ST_SHARED_CPPEXPORT AVPixelFormat XYZ12;
        // HWAccel formats
        ST_SHARED_CPPEXPORT AVPixelFormat DXVA2_VLD; // Windows
        ST_SHARED_CPPEXPORT AVPixelFormat VIDEOTOOLBOX_VLD; // macOS

        /**
         * @sa StImage::formatImgPixelFormat()
         */
        ST_CPPEXPORT StCString getString(const AVPixelFormat theFrmt);
    }

    /**
     * Simple structure for planar YUV frame dimensions.
     */
    struct dimYUV {
        int  widthY;
        int  heightY;
        int  widthU;
        int  heightU;
        int  widthV;
        int  heightV;
        int  bitsPerComp;
        bool isFullScale;
        bool hasAlpha;
    };

    /**
     * Auxiliary function to check that frame is in one of the YUV planar pixel format.
     * @return true if AVPixelFormat is planar YUV
     */
    ST_CPPEXPORT bool isFormatYUVPlanar(const AVCodecContext* theCtx);

    /**
     * Same as above but provide width/height information per component's plane.
     * @return true if AVPixelFormat is planar YUV
     */
    ST_CPPEXPORT bool isFormatYUVPlanar(const AVPixelFormat thePixFmt,
                                        const int           theWidth,
                                        const int           theHeight,
                                        dimYUV&             theDims);

    /**
     * @return true if AVPixelFormat is planar YUV
     */
    inline bool isFormatYUVPlanar(const AVCodecContext* theCtx,
                                  dimYUV&               theDims) {
        return isFormatYUVPlanar(theCtx->pix_fmt,
                                 theCtx->width,
                                 theCtx->height,
                                 theDims);
    }

#if(LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(53, 5, 0))
    inline bool isFormatYUVPlanar(const AVFrame* theFrame,
                                  dimYUV&        theDims) {
        return isFormatYUVPlanar((AVPixelFormat )theFrame->format,
                                 theFrame->width,
                                 theFrame->height,
                                 theDims);
    }
#endif

    /**
     * Check is stream represents attached picture (e.g. NOT a video stream).
     */
    inline bool isAttachedPicture(const AVStream* theStream) {
        if(theStream == NULL) {
            return false;
        }

    #if(LIBAVFORMAT_VERSION_INT >= AV_VERSION_INT(54, 2, 100))
        return (theStream->disposition & AV_DISPOSITION_ATTACHED_PIC) != 0;
    #else
        return false;
    #endif
    }

    /**
     * Return codec context from the stream using deprecated API.
     * The new API suggests using AVStream::codecpar, but this is confusing.
     * So we would better waiting until old API is completely removed.
     */
    inline AVCodecContext* getCodecCtx(const AVStream* theStream) {
    ST_DISABLE_DEPRECATION_WARNINGS
        return theStream->codec;
    ST_ENABLE_DEPRECATION_WARNINGS
    }

    /**
     * Get codec type from the stream.
     */
    inline AVMediaType getCodecType(const AVStream* theStream) {
    #ifdef ST_AV_NEWCODECPAR
        return theStream->codecpar->codec_type;
    #else
        return theStream->codec->codec_type;
    #endif
    }

    /**
     * Get codec id from the stream.
     */
    inline AVCodecID getCodecId(const AVStream* theStream) {
    #ifdef ST_AV_NEWCODECPAR
        return theStream->codecpar->codec_id;
    #else
        return theStream->codec->codec_id;
    #endif
    }

    /**
     * av_version_info() wrapper; return FFmpeg version info.
     */
    ST_CPPEXPORT StString getVersionInfo();

    /**
     * avutil_license() wrapper; return FFmpeg license info.
     */
    ST_CPPEXPORT StString getLicenseInfo();

    /**
     * av_jni_set_java_vm() wrapper.
     */
    ST_CPPEXPORT bool setJavaVM(void* theJavaVM);

    /**
     * avio_enum_protocols() wrapper for checking input protocol.
     */
    ST_CPPEXPORT bool isEnabledInputProtocol(const StString& theProtocol);

    /**
     * Audio functions
     */
    namespace audio {

        ST_CPPEXPORT StString getSampleFormatString (const AVCodecContext* theCtx);
        ST_CPPEXPORT StString getSampleRateString   (const AVCodecContext* theCtx);
        ST_CPPEXPORT StString getChannelLayoutString(const AVCodecContext* theCtx);

        namespace SAMPLE_FMT {
        #if(LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(53, 0, 0))
            static const AVSampleFormat NONE = (AVSampleFormat )-1;
            ST_SHARED_CPPEXPORT AVSampleFormat U8;
            ST_SHARED_CPPEXPORT AVSampleFormat S16;
            ST_SHARED_CPPEXPORT AVSampleFormat S32;
            ST_SHARED_CPPEXPORT AVSampleFormat FLT;
            ST_SHARED_CPPEXPORT AVSampleFormat DBL;
            ST_SHARED_CPPEXPORT AVSampleFormat U8P;
            ST_SHARED_CPPEXPORT AVSampleFormat S16P;
            ST_SHARED_CPPEXPORT AVSampleFormat S32P;
            ST_SHARED_CPPEXPORT AVSampleFormat FLTP;
            ST_SHARED_CPPEXPORT AVSampleFormat DBLP;
        #else
            static const SampleFormat NONE = (SampleFormat )-1;
            ST_SHARED_CPPEXPORT SampleFormat   U8;
            ST_SHARED_CPPEXPORT SampleFormat   S16;
            ST_SHARED_CPPEXPORT SampleFormat   S32;
            ST_SHARED_CPPEXPORT SampleFormat   FLT;
            ST_SHARED_CPPEXPORT SampleFormat   DBL;
            static const SampleFormat U8P  = (SampleFormat )-1;
            static const SampleFormat S16P = (SampleFormat )-1;
            static const SampleFormat S32P = (SampleFormat )-1;
            static const SampleFormat FLTP = (SampleFormat )-1;
            static const SampleFormat DBLP = (SampleFormat )-1;
        #endif
        }

    }

    /**
     * Metadata functions
     */
    namespace meta {

    #if(LIBAVUTIL_VERSION_INT >= AV_VERSION_INT(51, 5, 0))
        typedef AVDictionaryEntry Tag;
        typedef AVDictionary      Dict;
        enum {
            SEARCH_MATCH_CASE    = AV_DICT_MATCH_CASE,    // 1
            SEARCH_IGNORE_SUFFIX = AV_DICT_IGNORE_SUFFIX, // 2
        };
    #else
        typedef AVMetadataTag     Tag;
        typedef AVMetadata        Dict;
        enum {
            SEARCH_MATCH_CASE    = AV_METADATA_MATCH_CASE,    // 1
            SEARCH_IGNORE_SUFFIX = AV_METADATA_IGNORE_SUFFIX, // 2
        };
    #endif

        /**
         * Alias for av_frame_get_metadata().
         */
        ST_CPPEXPORT Dict* getFrameMetadata(AVFrame* theFrame);

        /**
         * Alias for av_dict_get().
         */
        ST_CPPEXPORT Tag* findTag(Dict*       theDict,
                                  const char* theKey,
                                  const Tag*  thePrevTag,
                                  const int   theFlags);

        /**
         * Find unique tag in the dictionary and retrieve it's value.
         * @param theDict  Dictionary
         * @param theKey   Unique key within this dictionary
         * @param theValue Read value (will be untouched if tag not found)
         * @return true if tag was found
         */
        ST_CPPEXPORT bool readTag(Dict*            theDict,
                                  const StCString& theKey,
                                  StString&        theValue);

        /**
         * Read tag from Format Context metadata.
         */
        inline bool readTag(AVFormatContext* theFormatCtx,
                            const StCString& theKey,
                            StString&        theValue) {
            return readTag(theFormatCtx->metadata, theKey, theValue);
        }

        /**
         * Read tag from Stream metadata.
         */
        inline bool readTag(AVStream*        theStream,
                            const StCString& theKey,
                            StString&        theValue) {
            return readTag(theStream->metadata, theKey, theValue);
        }

        /**
         * Read tag from Frame metadata.
         */
        inline bool readTag(AVFrame*         theFrame,
                            const StCString& theKey,
                            StString&        theValue) {
            return readTag(getFrameMetadata(theFrame), theKey, theValue);
        }

        /**
         * Retrieve language id from the stream.
         */
        inline StString readLang(AVStream* theStream) {
        #if(LIBAVUTIL_VERSION_INT >= AV_VERSION_INT(51, 5, 0))
            StString aLang;
            stAV::meta::readTag(theStream, stCString("language"), aLang);
            return aLang;
        #else
            return theStream->language;
        #endif
        }

    }

#ifdef ST_AV_NEWSTEREO
    /**
     * Convert stereo3d enumeration from FFmpeg to sView definition.
     */
    ST_LOCAL inline StFormat stereo3dAvToSt(const AVStereo3DType theType) {
        switch(theType) {
            case AV_STEREO3D_2D:            return StFormat_Mono;
            case AV_STEREO3D_SIDEBYSIDE:    return StFormat_SideBySide_LR;
            case AV_STEREO3D_TOPBOTTOM:     return StFormat_TopBottom_LR;
            case AV_STEREO3D_FRAMESEQUENCE: return StFormat_FrameSequence;
            //case AV_STEREO3D_CHECKERBOARD:        return StFormat_Checkboard;
            //case AV_STEREO3D_SIDEBYSIDE_QUINCUNX: return StFormat_Checkboard;
            case AV_STEREO3D_LINES:         return StFormat_Rows;
            case AV_STEREO3D_COLUMNS:       return StFormat_Columns;
            default:                        return StFormat_AUTO;
        }
    }

    /**
     * Convert stereo3d enumeration from sView to FFmpeg definition.
     */
    ST_LOCAL inline AVStereo3DType stereo3dStToAv(const StFormat theType,
                                                  bool&          theIsReversed) {
        theIsReversed = false;
        switch(theType) {
            default:
            case StFormat_AUTO:
                return (AVStereo3DType )-1;
            case StFormat_Mono:
                return AV_STEREO3D_2D;
            case StFormat_SideBySide_RL:
                theIsReversed = true;
                return AV_STEREO3D_SIDEBYSIDE;
            case StFormat_SideBySide_LR:
                return AV_STEREO3D_SIDEBYSIDE;
            case StFormat_TopBottom_RL:
                theIsReversed = true;
                return AV_STEREO3D_TOPBOTTOM;
            case StFormat_TopBottom_LR:
                return AV_STEREO3D_TOPBOTTOM;
            case StFormat_Rows:
                return AV_STEREO3D_LINES;
            case StFormat_Columns:
                return AV_STEREO3D_COLUMNS;
            //case StFormat_SeparateFrames:
            case StFormat_FrameSequence:
                return AV_STEREO3D_FRAMESEQUENCE;
            //case StFormat_AnaglyphRedCyan:
            //case StFormat_AnaglyphGreenMagenta:
            //case StFormat_AnaglyphYellowBlue:
            //case StFormat_Tiled4x:
        }
    }
#endif

}

#endif // __stAV_h_
