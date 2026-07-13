/**
 * Copyright © 2013-2025 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#include <StAV/StAVFrame.h>

#include <StAV/StAVImage.h>

StAVFrame::StAVFrame()
: Frame(av_frame_alloc())
{
    reset();
}

StAVFrame::~StAVFrame() {
    av_frame_free(&Frame);
}

void StAVFrame::reset() {
    av_frame_unref(Frame);
}

int64_t StAVFrame::getBestEffortTimestamp() const {
    return Frame->best_effort_timestamp;
}

void StAVFrame::getImageInfo(const AVCodecContext* theCodecCtx,
                             int&           theSizeX,
                             int&           theSizeY,
                             AVPixelFormat& thePixFmt) const {
    (void )theCodecCtx;
    theSizeX  = Frame->width;
    theSizeY  = Frame->height;
    thePixFmt = (AVPixelFormat )Frame->format;
}

StAVFrameCounter::StAVFrameCounter()
: myFrame(NULL),
  myIsProxy(false) {
    myFrame = av_frame_alloc();
}

StAVFrameCounter::~StAVFrameCounter() {
    av_frame_free(&myFrame);
}

void StAVFrameCounter::createReference(StHandle<StBufferCounter>& theOther) const {
    StHandle<StAVFrameCounter> anAvRef = StHandle<StAVFrameCounter>::downcast(theOther);
    if(anAvRef.isNull()) {
        anAvRef = new StAVFrameCounter();
        theOther = anAvRef;
    }

    av_frame_unref(anAvRef->myFrame);
    if(myIsProxy) {
        // just steal the reference
        av_frame_move_ref(anAvRef->myFrame, myFrame);
    } else {
        av_frame_ref(anAvRef->myFrame, myFrame);
    }
}

void StAVFrameCounter::releaseReference() {
    av_frame_unref(myFrame);
}

void StAVFrameCounter::moveReferenceFrom(AVFrame* theFrame) {
    myIsProxy = true;
    av_frame_unref(myFrame);
    av_frame_move_ref(myFrame, theFrame);
}
