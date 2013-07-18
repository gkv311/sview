/**
 * Copyright © 2011-2013 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#ifndef __stAV_h_
#define __stAV_h_

#include <StStrings/StString.h>

// libav* libraries written on pure C,
// and we must around includes manually
extern "C" {
#ifdef _MSC_VER
    // suppress some common warnings in FFmpeg headers
    #pragma warning(disable : 4244)
#endif

    #include <libavcodec/avcodec.h>
    #include <libavformat/avformat.h>
    #include <libswscale/swscale.h>

#ifdef _MSC_VER
    #pragma warning(default : 4244)
#endif
};

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

#ifdef __ST_SHARED_DLL__
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
        ST_SHARED_CPPEXPORT PixelFormat NONE;
        ST_SHARED_CPPEXPORT PixelFormat GRAY8;     //!< Y,  8bpp
        ST_SHARED_CPPEXPORT PixelFormat GRAY16;    //!< Y, 16bpp
        // planar YUV formats
        ST_SHARED_CPPEXPORT PixelFormat YUV420P;   //!< planar YUV 4:2:0, 12bpp, (1 Cr & Cb sample per 2x2 Y samples)
        ST_SHARED_CPPEXPORT PixelFormat YUV422P;   //!< planar YUV 4:2:2, 16bpp, (1 Cr & Cb sample per 2x1 Y samples)
        ST_SHARED_CPPEXPORT PixelFormat YUV444P;   //!< planar YUV 4:4:4, 24bpp, (1 Cr & Cb sample per 1x1 Y samples)
        ST_SHARED_CPPEXPORT PixelFormat YUV410P;   //!< planar YUV 4:1:0,  9bpp, (1 Cr & Cb sample per 4x4 Y samples)
        ST_SHARED_CPPEXPORT PixelFormat YUV411P;   //!< planar YUV 4:1:1, 12bpp, (1 Cr & Cb sample per 4x1 Y samples)
        ST_SHARED_CPPEXPORT PixelFormat YUV440P;   //!< planar YUV 4:4:0 (1 Cr & Cb sample per 1x2 Y samples)
        // wide planar YUV formats (9,10,14,16 bits stored in 16 bits)
        ST_SHARED_CPPEXPORT PixelFormat YUV420P9;
        ST_SHARED_CPPEXPORT PixelFormat YUV422P9;
        ST_SHARED_CPPEXPORT PixelFormat YUV444P9;
        ST_SHARED_CPPEXPORT PixelFormat YUV420P10;
        ST_SHARED_CPPEXPORT PixelFormat YUV422P10;
        ST_SHARED_CPPEXPORT PixelFormat YUV444P10;
        ST_SHARED_CPPEXPORT PixelFormat YUV420P16;
        ST_SHARED_CPPEXPORT PixelFormat YUV422P16;
        ST_SHARED_CPPEXPORT PixelFormat YUV444P16;
        //extern const PixelFormat YUVA420P; ///< planar YUV 4:2:0, 20bpp, (1 Cr & Cb sample per 2x2 Y & A samples)
        // fullscale YUV formats (deprecated?)
        ST_SHARED_CPPEXPORT PixelFormat YUVJ420P;  //!< planar YUV 4:2:0, 12bpp, full scale (JPEG)
        ST_SHARED_CPPEXPORT PixelFormat YUVJ422P;  //!< planar YUV 4:2:2, 16bpp, full scale (JPEG)
        ST_SHARED_CPPEXPORT PixelFormat YUVJ444P;  //!< planar YUV 4:4:4, 24bpp, full scale (JPEG)
        ST_SHARED_CPPEXPORT PixelFormat YUVJ440P;  //!< planar YUV 4:4:0 full scale (JPEG)
        // RGB formats
        ST_SHARED_CPPEXPORT PixelFormat RGB24;     //!< packed RGB 8:8:8, 24bpp, RGBRGB...
        ST_SHARED_CPPEXPORT PixelFormat BGR24;     //!< packed RGB 8:8:8, 24bpp, BGRBGR...
        ST_SHARED_CPPEXPORT PixelFormat RGBA32;
        ST_SHARED_CPPEXPORT PixelFormat BGRA32;

        ST_CPPEXPORT StCString getString(const PixelFormat theFrmt);
    };

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
    };

    /**
     * Auxiliary function to check that frame is in one of the YUV planar pixel format.
     * @return true if PixelFormat is planar YUV
     */
    ST_CPPEXPORT bool isFormatYUVPlanar(const AVCodecContext* theCtx);

    /**
     * Same as above but provide width/height information per component's plane.
     * @return true if PixelFormat is planar YUV
     */
    ST_CPPEXPORT bool isFormatYUVPlanar(const PixelFormat thePixFmt,
                                        const int         theWidth,
                                        const int         theHeight,
                                        dimYUV&           theDims);

    /**
     * @return true if PixelFormat is planar YUV
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
        return isFormatYUVPlanar((PixelFormat )theFrame->format,
                                 theFrame->width,
                                 theFrame->height,
                                 theDims);
    }
#endif

    /**
     * Audio functions
     */
    namespace audio {

        ST_CPPEXPORT StString getSampleFormatString (const AVCodecContext* theCtx);
        ST_CPPEXPORT StString getSampleRateString   (const AVCodecContext* theCtx);
        ST_CPPEXPORT StString getChannelLayoutString(const AVCodecContext* theCtx);

        namespace SAMPLE_FMT {
            static const AVSampleFormat NONE = (AVSampleFormat )-1;
        #if(LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(53, 0, 0))
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
        };

    };

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
         * Alias to av_dict_get.
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
        ST_CPPEXPORT bool readTag(Dict*           theDict,
                                  const StString& theKey,
                                  StString&       theValue);

        inline bool readTag(AVFormatContext* theFormatCtx,
                            const StString&  theKey,
                            StString&        theValue) {
            return readTag(theFormatCtx->metadata, theKey, theValue);
        }

        inline bool readTag(AVStream*        theStream,
                            const StString&  theKey,
                            StString&        theValue) {
            return readTag(theStream->metadata, theKey, theValue);
        }

    };

};

#endif // __stAV_h_
