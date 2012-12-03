/**
 * Copyright © 2011 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#ifndef __StLibAVImage_h_
#define __StLibAVImage_h_

#include "StImageFile.h"

struct AVInputFormat;
struct AVFormatContext;
struct AVCodecContext;
struct AVCodec;
struct AVFrame;

/**
 * This class implements image load/save operation using libav* libraries.
 */
class ST_LOCAL StLibAVImage : public StImageFile {

        private:

    // FFmpeg staff
    AVInputFormat* imageFormat; // image format
    AVFormatContext* formatCtx; // file context
    AVCodecContext*   codecCtx; // codec context
    AVCodec*             codec; // codec
    AVFrame*             frame;

        private:

    int getAVPixelFormat();

        public:

    /**
     * Should be called at application start.
     * Short-link to the stLibAV::init().
     */
    static bool init();

        public:

    StLibAVImage();
    virtual ~StLibAVImage();

    virtual void close();
    virtual bool load(const StString& theFilePath,
                      ImageType theImageType = ST_TYPE_NONE,
                      uint8_t* theDataPtr = NULL, int theDataSize = 0);
    virtual bool save(const StString& theFilePath,
                      ImageType theImageType);
    virtual bool resize(size_t , size_t ) { return false; }
};

#endif //__StLibAVImage_h_
