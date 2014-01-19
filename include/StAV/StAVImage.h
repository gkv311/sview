/**
 * Copyright © 2011-2014 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#ifndef __StAVImage_h_
#define __StAVImage_h_

#include <StImage/StImageFile.h>

struct AVInputFormat;
struct AVFormatContext;
struct AVCodecContext;
struct AVCodec;
struct AVFrame;

/**
 * This class implements image load/save operation using libav* libraries.
 */
class StAVImage : public StImageFile {

        public:

    /**
     * Should be called at application start.
     * Short-link to the stLibAV::init().
     */
    ST_CPPEXPORT static bool init();

        public:

    ST_CPPEXPORT StAVImage();
    ST_CPPEXPORT virtual ~StAVImage();

    ST_CPPEXPORT virtual void close();
    ST_CPPEXPORT virtual bool load(const StString& theFilePath,
                                   ImageType       theImageType = ST_TYPE_NONE,
                                   uint8_t* theDataPtr = NULL, int theDataSize = 0);
    ST_CPPEXPORT virtual bool save(const StString& theFilePath,
                                   ImageType       theImageType);
    ST_CPPEXPORT virtual bool resize(size_t , size_t );

        private:

    ST_CPPEXPORT int getAVPixelFormat();

        private:

    AVInputFormat*   myImageFormat; //!< image format
    AVFormatContext* myFormatCtx;   //!< file context
    AVCodecContext*  myCodecCtx;    //!< codec context
    AVCodec*         myCodec;       //!< codec
    AVFrame*         myFrame;

};

#endif // __StAVImage_h_
