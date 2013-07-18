/**
 * Copyright © 2013 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#include <StAV/StAVFrame.h>

StAVFrame::StAVFrame()
: Frame(avcodec_alloc_frame()) {
    avcodec_get_frame_defaults(Frame);
}

StAVFrame::~StAVFrame() {
    av_free(Frame);
}

void StAVFrame::reset() {
    avcodec_get_frame_defaults(Frame);
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
