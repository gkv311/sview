/**
 * Copyright © 2020 Kirill Gavrilov <kirill@sview.ru>
 *
 * StMoviePlayer program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * StMoviePlayer program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program.
 * If not, see <http://www.gnu.org/licenses/>.
 */

#include "StVideoQueue.h"

#if defined(__APPLE__)

#include <StAV/StAVPacket.h>
#include <StAV/StAVFrame.h>
#include <StAV/StAVBufferPool.h>
#include <StLibrary.h>

#include <vector>

#include <CoreServices/CoreServices.h>

extern "C" {
  #include <libavcodec/videotoolbox.h>
  #include <libavutil/buffer.h>
  #include <libavutil/imgutils.h>
};

/**
 * VideoToolbox HWAccel context.
 */
class StVideoToolboxContext : public StHWAccelContext {

        public:

    /**
     * Empty constructor.
     */
    StVideoToolboxContext()
    : myFrameTmp(NULL),
      myHasDevice(false){
        //
    }

    /**
     * Destructor.
     */
    virtual ~StVideoToolboxContext() {
        release();
    }

    /**
     * Release context.
     */
    void release();

    /**
     * Return true if VideoToolbox has been successfully initialized.
     */
    virtual bool isValid() const ST_ATTR_OVERRIDE { return myHasDevice; }

    /**
     * Create context.
     */
    virtual bool create(StVideoQueue& theVideo) ST_ATTR_OVERRIDE;

    /**
     * Destroy decoder.
     */
    virtual void decoderDestroy(AVCodecContext* theCodecCtx) ST_ATTR_OVERRIDE {
        myPoolsTmp[0].release();
        myPoolsTmp[1].release();
        myPoolsTmp[2].release();
        myPoolsTmp[3].release();
        if(theCodecCtx != NULL && myHasDevice) {
            av_videotoolbox_default_free(theCodecCtx);
        }
        myHasDevice = false;
    }

    /**
     * Create decoder.
     */
    virtual bool decoderCreate(StVideoQueue&   theVideo,
                               AVCodecContext* theCodecCtx) ST_ATTR_OVERRIDE {
        const StSignal<void (const StCString& )>& onError = theVideo.signals.onError;
        decoderDestroy(theCodecCtx);

        const int anAvErr = av_videotoolbox_default_init(theCodecCtx);
        if(anAvErr < 0) {
            onError(stCString("StVideoQueue: Error creating Videotoolbox decoder"));
            return false;
        }
        myHasDevice = true;
        return true;
    }

    /**
     * AVFrame initialization callback.
     */
    virtual int getFrameBuffer(StVideoQueue& ,
                               AVFrame* ) ST_ATTR_OVERRIDE {
        return -1; // method is unused
    }

    /**
     * Fetch decoded results into specified frame.
     */
    virtual bool retrieveFrame(StVideoQueue& theVideo,
                               AVFrame* theFrame) ST_ATTR_OVERRIDE;

        private:

    StAVBufferPool myPoolsTmp[4];
    AVFrame*       myFrameTmp;
    bool           myHasDevice;

};

void StVideoToolboxContext::release() {
    decoderDestroy(NULL);
    av_frame_free(&myFrameTmp);
}

bool StVideoToolboxContext::create(StVideoQueue& ) {
    myFrameTmp = av_frame_alloc();
    if(myFrameTmp == NULL) {
        release();
        return false;
    }
    return true;
}

bool StVideoToolboxContext::retrieveFrame(StVideoQueue& ,
                                          AVFrame* theFrame) {
    if(!isValid()
     || theFrame->format != stAV::PIX_FMT::VIDEOTOOLBOX_VLD) {
        return false;
    }

    av_frame_unref(myFrameTmp);

    CVPixelBufferRef aPixBuf = (CVPixelBufferRef )theFrame->data[3];
    const OSType aPixBufFormat = CVPixelBufferGetPixelFormatType(aPixBuf);
    switch(aPixBufFormat) {
        case kCVPixelFormatType_420YpCbCr8Planar: myFrameTmp->format = AV_PIX_FMT_YUV420P; break;
        case kCVPixelFormatType_422YpCbCr8:       myFrameTmp->format = AV_PIX_FMT_UYVY422; break;
        case kCVPixelFormatType_32BGRA:           myFrameTmp->format = AV_PIX_FMT_BGRA; break;
        case kCVPixelFormatType_420YpCbCr8BiPlanarVideoRange:
        case kCVPixelFormatType_420YpCbCr8BiPlanarFullRange: myFrameTmp->format = AV_PIX_FMT_NV12; break;
        case kCVPixelFormatType_420YpCbCr10BiPlanarVideoRange:
        case kCVPixelFormatType_420YpCbCr10BiPlanarFullRange: myFrameTmp->format = AV_PIX_FMT_P010; break;
        default: {
            ST_ERROR_LOG("StVideoToolboxContext: unsupported pixel format " + int(aPixBufFormat));
            return false;
        }
    }

    CVReturn err = CVPixelBufferLockBaseAddress(aPixBuf, kCVPixelBufferLock_ReadOnly);
    if(err != kCVReturnSuccess) {
        ST_ERROR_LOG("StVideoToolboxContext: Error locking the pixel buffer");
        return false;
    }

    myFrameTmp->width  = theFrame->width;
    myFrameTmp->height = theFrame->height;
    size_t aDataSize[4] = { 0 };

    uint8_t*   aData[4] = { 0 };
    int    aLineSize[4] = { 0 };
    const int aNbPlanes = CVPixelBufferIsPlanar(aPixBuf) ? CVPixelBufferGetPlaneCount(aPixBuf) : 1;
    if(CVPixelBufferIsPlanar(aPixBuf)) {
        for(int aPlaneIter = 0; aPlaneIter < aNbPlanes; ++aPlaneIter) {
            aData[aPlaneIter] = (uint8_t* )CVPixelBufferGetBaseAddressOfPlane(aPixBuf, aPlaneIter);
            aLineSize[aPlaneIter] = CVPixelBufferGetBytesPerRowOfPlane(aPixBuf, aPlaneIter);

            const int aTmpStride = aLineSize[aPlaneIter]; //(int )getAligned(CVPixelBufferGetWidthOfPlane(aPixBuf, aPlaneIter) * bytesPerPixel, 32);
            myFrameTmp->linesize[aPlaneIter] = aTmpStride;
            aDataSize[aPlaneIter] = size_t(aTmpStride) * size_t(CVPixelBufferGetHeightOfPlane(aPixBuf, aPlaneIter));
        }
    } else {
        aData[0] = (uint8_t* )CVPixelBufferGetBaseAddress(aPixBuf);
        aLineSize[0] = CVPixelBufferGetBytesPerRow(aPixBuf);

        const int aTmpStride = aLineSize[0]; //(int )getAligned(CVPixelBufferGetWidth(aPixBuf) * bytesPerPixel, 32);
        myFrameTmp->linesize[0] = aTmpStride;
        aDataSize[0] = size_t(aTmpStride) * size_t(CVPixelBufferGetHeight(aPixBuf));
    }

    for(int aPlaneIter = 0; aPlaneIter < aNbPlanes; ++aPlaneIter) {
        myFrameTmp->buf[aPlaneIter] = myPoolsTmp[aPlaneIter].init(aDataSize[aPlaneIter]) ? myPoolsTmp[aPlaneIter].getBuffer() : NULL;
        if(myFrameTmp->buf[aPlaneIter] == NULL) {
            ST_ERROR_LOG("StVideoToolboxContext: unable to allocate frame buffers");
            av_frame_unref(myFrameTmp);
            CVPixelBufferUnlockBaseAddress(aPixBuf, kCVPixelBufferLock_ReadOnly);
            return false;
        }
        myFrameTmp->data[aPlaneIter] = myFrameTmp->buf[aPlaneIter]->data;
    }
    //int anAvErrBuf = av_frame_get_buffer(myFrameTmp, 0);
    //if(anAvErrBuf < 0) { return false; }

    av_image_copy(myFrameTmp->data, myFrameTmp->linesize,
                  (const uint8_t** )aData, aLineSize, (AVPixelFormat )myFrameTmp->format,
                  theFrame->width, theFrame->height);

    const int anAvErr = av_frame_copy_props(myFrameTmp, theFrame);
    CVPixelBufferUnlockBaseAddress(aPixBuf, kCVPixelBufferLock_ReadOnly);
    if(anAvErr < 0) {
        ST_ERROR_LOG("StVideoToolboxContext: Error copying frame properties");
        av_frame_unref(myFrameTmp);
        return false;
    }

    av_frame_unref   (theFrame);
    av_frame_move_ref(theFrame, myFrameTmp);
    return true;
}

bool StVideoQueue::hwaccelInit() {
    if(myCodecCtx->codec_id == AV_CODEC_ID_NONE) {
        return false;
    }

    if(myHWAccelCtx.isNull()) {
        myHWAccelCtx = new StVideoToolboxContext();
        if(!myHWAccelCtx->create(*this)) {
            return false;
        }
    }

    if(!myHWAccelCtx->decoderCreate(*this, myCodecCtx)) {
        return false;
    }
    fillCodecInfo(myCodecCtx->codec, " (VideoToolbox)");
    return true;
}

#endif // __APPLE__
