/**
 * Copyright © 2011-2013 Kirill Gavrilov <kirill@sview.ru>
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
class StLibAVImage : public StImageFile {

        public:

    /**
     * Should be called at application start.
     * Short-link to the stLibAV::init().
     */
    ST_CPPEXPORT static bool init();

        public:

    ST_CPPEXPORT StLibAVImage();
    ST_CPPEXPORT virtual ~StLibAVImage();

    ST_CPPEXPORT virtual void close();
    ST_CPPEXPORT virtual bool load(const StString& theFilePath,
                                   ImageType theImageType = ST_TYPE_NONE,
                                   uint8_t* theDataPtr = NULL, int theDataSize = 0);
    ST_CPPEXPORT virtual bool save(const StString& theFilePath,
                                   ImageType theImageType);
    ST_CPPEXPORT virtual bool resize(size_t , size_t );

        private:

    ST_CPPEXPORT int getAVPixelFormat();

        private:

    AVInputFormat*   imageFormat; //!< image format
    AVFormatContext* formatCtx;   //!< file context
    AVCodecContext*  codecCtx;    //!< codec context
    AVCodec*         codec;       //!< codec
    AVFrame*         frame;

};

#endif //__StLibAVImage_h_
