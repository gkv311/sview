/**
 * Copyright © 2013-2015 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#ifndef __StAVFrame_h_
#define __StAVFrame_h_

#include <StAV/stAV.h>

/**
 * This is just a wrapper over AVFrame structure.
 */
class StAVFrame {

        public:

    /**
     * Empty constructor
     */
    ST_CPPEXPORT StAVFrame();

    /**
     * Destructor
     */
    ST_CPPEXPORT ~StAVFrame();

    /**
     * Return true if frame does not contain any data.
     */
    ST_LOCAL bool isEmpty() const { return Frame->format == -1; } // AV_PIX_FMT_NONE

    /**
     * Reset frame to default state (empty).
     */
    ST_CPPEXPORT void reset();

    /**
     * Retrieve image dimensions and pixel format
     * from this frame or codec context.
     */
    ST_CPPEXPORT void getImageInfo(const AVCodecContext* theCodecCtx,
                                   int&           theSizeX,
                                   int&           theSizeY,
                                   AVPixelFormat& thePixFmt) const;

    /**
     * Access data plane for specified Id.
     */
    ST_LOCAL inline uint8_t* getPlane(const size_t thePlaneId) const {
        return Frame->data[thePlaneId];
    }

    /**
     * @return linesize in bytes for specified data plane
     */
    ST_LOCAL inline int getLineSize(const size_t thePlaneId) const {
        return Frame->linesize[thePlaneId];
    }

    /**
     * @return frame timestamp estimated using various heuristics, in stream time base
     */
    ST_CPPEXPORT int64_t getBestEffortTimestamp() const;

        public:

    AVFrame* Frame;

        private:

    StAVFrame(const StAVFrame& theCopy);

};

#endif // __StAVFrame_h_
