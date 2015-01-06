/**
 * Copyright © 2013-2014 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#include <StAV/StAVFrame.h>

StAVFrame::StAVFrame()
#if(LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(55, 45, 101))
: Frame(av_frame_alloc())
#else
: Frame(avcodec_alloc_frame())
#endif
{
    reset();
}

StAVFrame::~StAVFrame() {
#if(LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(55, 45, 101))
    av_frame_free(&Frame);
#else
    av_free(Frame);
#endif
}

void StAVFrame::reset() {
#if(LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(55, 45, 101))
    av_frame_unref(Frame);
#else
    avcodec_get_frame_defaults(Frame);
#endif
}

void StAVFrame::getImageInfo(const AVCodecContext* theCodecCtx,
                             int&         theSizeX,
                             int&         theSizeY,
                             PixelFormat& thePixFmt) const {
#if(LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(53, 5, 0))
    (void )theCodecCtx;
    theSizeX  = Frame->width;
    theSizeY  = Frame->height;
    thePixFmt = (PixelFormat )Frame->format;
#else
    theSizeX  = theCodecCtx->width;
    theSizeY  = theCodecCtx->height;
    thePixFmt = theCodecCtx->pix_fmt;
#endif
}
