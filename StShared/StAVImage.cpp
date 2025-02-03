/**
 * Copyright © 2011-2025 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#include <StAV/StAVImage.h>

#include <StAV/StAVPacket.h>
#include <StFile/StFileNode.h>
#include <StFile/StRawFile.h>
#include <StImage/StJpegParser.h>
#include <StStrings/StLogger.h>
#include <StAV/StAVIOMemContext.h>

bool StAVImage::init() {
    return stAV::init();
}

StAVImage::StAVImage()
: myFormatCtx(NULL),
  myCodecCtx(NULL),
  myCodec(NULL) {
    StAVImage::init();
}

StAVImage::~StAVImage() {
    close();
}

int StAVImage::getAVPixelFormat(const StImage& theImage) {
    const StImagePlane& aPlane0 = theImage.getPlane(0);
    if(theImage.isPacked()) {
        switch(aPlane0.getFormat()) {
            case StImagePlane::ImgRGB:    return stAV::PIX_FMT::RGB24;
            case StImagePlane::ImgBGR:    return stAV::PIX_FMT::BGR24;
            case StImagePlane::ImgRGBA:   return stAV::PIX_FMT::RGBA32;
            case StImagePlane::ImgBGRA:   return stAV::PIX_FMT::BGRA32;
            case StImagePlane::ImgGray:   return stAV::PIX_FMT::GRAY8;
            case StImagePlane::ImgGray16: return stAV::PIX_FMT::GRAY16;
            case StImagePlane::ImgGrayF:  return stAV::PIX_FMT::GRAYF32;
            default:                      return stAV::PIX_FMT::NONE;
        }
    }
    switch(theImage.getColorModel()) {
        case StImage::ImgColor_YUV:
        case StImage::ImgColor_YUVA: {
            size_t aDelimX = (theImage.getPlane(1).getSizeX() > 0) ? (aPlane0.getSizeX() / theImage.getPlane(1).getSizeX()) : 1;
            size_t aDelimY = (theImage.getPlane(1).getSizeY() > 0) ? (aPlane0.getSizeY() / theImage.getPlane(1).getSizeY()) : 1;
            if(theImage.getPlane(1).getFormat() == StImagePlane::ImgUV) {
                return stAV::PIX_FMT::NV12;
            } else if(aDelimX == 1 && aDelimY == 1) {
                switch(theImage.getColorScale()) {
                    case StImage::ImgScale_Mpeg:
                        return aPlane0.getFormat() == StImagePlane::ImgGray16
                             ? stAV::PIX_FMT::YUV444P16
                             : stAV::PIX_FMT::YUV444P;
                    case StImage::ImgScale_Mpeg9:
                    case StImage::ImgScale_Jpeg9:  return stAV::PIX_FMT::YUV444P9;
                    case StImage::ImgScale_Mpeg10:
                    case StImage::ImgScale_Jpeg10: return stAV::PIX_FMT::YUV444P10;
                    case StImage::ImgScale_Full:
                    default:
                        return aPlane0.getFormat() == StImagePlane::ImgGray16
                             ? stAV::PIX_FMT::YUV444P16 //
                             : stAV::PIX_FMT::YUVJ444P;
                }
            } else if(aDelimX == 2 && aDelimY == 2) {
                switch(theImage.getColorScale()) {
                    case StImage::ImgScale_Mpeg:
                        return aPlane0.getFormat() == StImagePlane::ImgGray16
                             ? stAV::PIX_FMT::YUV420P16
                             : stAV::PIX_FMT::YUV420P;
                    case StImage::ImgScale_Mpeg9:
                    case StImage::ImgScale_Jpeg9:  return stAV::PIX_FMT::YUV420P9;
                    case StImage::ImgScale_Mpeg10:
                    case StImage::ImgScale_Jpeg10: return stAV::PIX_FMT::YUV420P10;
                    case StImage::ImgScale_Full:
                    default:
                        return aPlane0.getFormat() == StImagePlane::ImgGray16
                             ? stAV::PIX_FMT::YUV420P16 // jpeg color range ignored!
                             : stAV::PIX_FMT::YUVJ420P;
                }
            } else if(aDelimX == 2 && aDelimY == 1) {
                switch(theImage.getColorScale()) {
                    case StImage::ImgScale_Mpeg:
                        return aPlane0.getFormat() == StImagePlane::ImgGray16
                             ? stAV::PIX_FMT::YUV422P16 // jpeg color range ignored!
                             : stAV::PIX_FMT::YUV422P;
                    case StImage::ImgScale_Mpeg9:
                    case StImage::ImgScale_Jpeg9:  return stAV::PIX_FMT::YUV422P9;
                    case StImage::ImgScale_Mpeg10:
                    case StImage::ImgScale_Jpeg10: return stAV::PIX_FMT::YUV422P10;
                    case StImage::ImgScale_Full:
                    default:
                        return aPlane0.getFormat() == StImagePlane::ImgGray16
                             ? stAV::PIX_FMT::YUV422P16 // jpeg color range ignored!
                             : stAV::PIX_FMT::YUVJ422P;
                }
            } else if(aDelimX == 1 && aDelimY == 2) {
                return theImage.getColorScale() == StImage::ImgScale_Mpeg
                     ? stAV::PIX_FMT::YUV440P : stAV::PIX_FMT::YUVJ440P;
            } else if(aDelimX == 4 && aDelimY == 1) {
                return stAV::PIX_FMT::YUV411P;
            } else if(aDelimX == 4 && aDelimY == 4) {
                return stAV::PIX_FMT::YUV410P;
            }
            return stAV::PIX_FMT::NONE;
        }
        default: return stAV::PIX_FMT::NONE;
    }
}

static void fillPointersAV(const StImage& theImage,
                           uint8_t* theData[], int theLinesize[]) {
    for(size_t aPlaneId = 0; aPlaneId < 4; ++aPlaneId) {
        theData[aPlaneId] = !theImage.getPlane(aPlaneId).isNull() ? (uint8_t* )theImage.getPlane(aPlaneId).getData() : NULL;
        theLinesize[aPlaneId] = (int )theImage.getPlane(aPlaneId).getSizeRowBytes();
    }
}

static const int THE_SWSCALE_FLAGS_FAST    = SWS_BICUBIC;
static const int THE_SWSCALE_FLAGS_QUALITY = SWS_LANCZOS | SWS_ACCURATE_RND | SWS_FULL_CHR_H_INT;

/**
 * Convert image from one format to another using swscale.
 * Image buffers should be already initialized!
 */
static bool convert(const StImage& theImageFrom, AVPixelFormat theFormatFrom,
                          StImage& theImageTo,   AVPixelFormat theFormatTo,
                    int theSwsFlags) {
    ST_DEBUG_LOG("StAVImage, convert from " + stAV::PIX_FMT::getString(theFormatFrom) + " " + theImageFrom.getSizeX() + "x" + theImageFrom.getSizeY()
               + " to " + stAV::PIX_FMT::getString(theFormatTo) + " " + theImageTo.getSizeX() + "x" + theImageTo.getSizeY());
    if(theFormatFrom == stAV::PIX_FMT::NONE
    || theFormatTo   == stAV::PIX_FMT::NONE) {
        return false;
    }

    SwsContext* aCtxToRgb = sws_getContext((int )theImageFrom.getSizeX(), (int )theImageFrom.getSizeY(), theFormatFrom, // source
                                           (int )theImageTo.getSizeX(),   (int )theImageTo.getSizeY(),   theFormatTo,   // destination
                                           theSwsFlags, NULL, NULL, NULL);
    if(aCtxToRgb == NULL) {
        return false;
    }

    uint8_t* aSrcData[4]; int aSrcLinesize[4];
    fillPointersAV(theImageFrom, aSrcData, aSrcLinesize);

    uint8_t* aDstData[4]; int aDstLinesize[4];
    fillPointersAV(theImageTo, aDstData, aDstLinesize);

    sws_scale(aCtxToRgb,
              aSrcData, aSrcLinesize,
              0, (int )theImageFrom.getSizeY(),
              aDstData, aDstLinesize);

    sws_freeContext(aCtxToRgb);
    return true;
}

/**
 * Return AV pixel format for an image plane.
 */
static int getAVPixelFormatForPlane(const StImagePlane& theImage) {
    switch(theImage.getFormat()) {
        case StImagePlane::ImgRGB:    return stAV::PIX_FMT::RGB24;
        case StImagePlane::ImgBGR:    return stAV::PIX_FMT::BGR24;
        case StImagePlane::ImgRGBA:   return stAV::PIX_FMT::RGBA32;
        case StImagePlane::ImgBGRA:   return stAV::PIX_FMT::BGRA32;
        case StImagePlane::ImgGray:   return stAV::PIX_FMT::GRAY8;
        case StImagePlane::ImgGray16: return stAV::PIX_FMT::GRAY16;
        case StImagePlane::ImgGrayF:  return stAV::PIX_FMT::GRAYF32;
        default:                      return stAV::PIX_FMT::NONE;
    }
}

bool StAVImage::resizePlane(const StImagePlane& theImageFrom,
                            StImagePlane&       theImageTo) {
    if(theImageFrom.isNull()
    || theImageFrom.getSizeX() < 1
    || theImageFrom.getSizeY() < 1
    || theImageTo.isNull()
    || theImageTo.getSizeX() < 1
    || theImageTo.getSizeY() < 1) {
        return false;
    }

    StAVImage::init();
    const AVPixelFormat aFormatFrom = (AVPixelFormat )getAVPixelFormatForPlane(theImageFrom);
    const AVPixelFormat aFormatTo   = (AVPixelFormat )getAVPixelFormatForPlane(theImageTo);
    if(aFormatFrom == stAV::PIX_FMT::NONE
    || aFormatTo   == stAV::PIX_FMT::NONE) {
        return false;
    }

    SwsContext* aCtxToRgb = sws_getContext((int )theImageFrom.getSizeX(), (int )theImageFrom.getSizeY(), aFormatFrom,
                                           (int )theImageTo.getSizeX(),   (int )theImageTo.getSizeY(),   aFormatTo,
                                           THE_SWSCALE_FLAGS_FAST, NULL, NULL, NULL);
    if(aCtxToRgb == NULL) {
        return false;
    }

    uint8_t* aSrcData[4] = { (uint8_t* )theImageFrom.getData(), NULL, NULL, NULL };
    int  aSrcLinesize[4] = { (int )theImageFrom.getSizeRowBytes(), 0, 0, 0 };
    uint8_t* aDstData[4] = { (uint8_t* )theImageTo.getData(), NULL, NULL, NULL };
    int  aDstLinesize[4] = { (int )theImageTo.getSizeRowBytes(), 0, 0, 0 };
    sws_scale(aCtxToRgb,
              aSrcData, aSrcLinesize,
              0, (int )theImageFrom.getSizeY(),
              aDstData, aDstLinesize);
    sws_freeContext(aCtxToRgb);
    return true;
}

bool StAVImage::resize(const StImage& theImageFrom,
                       StImage&       theImageTo) {
    if(theImageFrom.isNull()
    || theImageFrom.getSizeX() < 1
    || theImageFrom.getSizeY() < 1
    || theImageTo.isNull()
    || theImageTo.getSizeX() < 1
    || theImageTo.getSizeY() < 1) {
        return false;
    }

    StAVImage::init();
    const AVPixelFormat aFormatFrom = (AVPixelFormat )StAVImage::getAVPixelFormat(theImageFrom);
    const AVPixelFormat aFormatTo   = (AVPixelFormat )StAVImage::getAVPixelFormat(theImageTo);
    return aFormatFrom != stAV::PIX_FMT::NONE
        && aFormatTo   != stAV::PIX_FMT::NONE
        && convert(theImageFrom, aFormatFrom,
                   theImageTo,   aFormatTo,
                   THE_SWSCALE_FLAGS_FAST);
}

void StAVImage::close() {
    myMetadata.clear();
    closeAvCtx();
}

void StAVImage::closeAvCtx() {
    myFrame.reset();
    if(myCodec != NULL && myCodecCtx != NULL) {
        avcodec_close(myCodecCtx); // close VIDEO codec
    }
    myCodec = NULL;
    if(myFormatCtx != NULL) {
        avformat_close_input(&myFormatCtx);
        // codec context allocated by av_open_input_file() function
        myCodecCtx = NULL;
    } else if(myCodecCtx != NULL) {
        // codec context allocated by ourself with avcodec_alloc_context() function
        av_freep(&myCodecCtx);
    }
}

bool StAVImage::loadExtra(const StString& theFilePath,
                          ImageType       theImageType,
                          uint8_t*        theDataPtr,
                          int             theDataSize,
                          bool            theIsOnlyRGB) {

    // reset current data
    StImage::nullify();
    setState();
    close();
    myMetadata.clear();

    AVInputFormat* anImgFormat = NULL;
    bool isForcedCodec = false;
    switch(theImageType) {
        case ST_TYPE_PNG:
        case ST_TYPE_PNS: {
            myCodec = avcodec_find_decoder_by_name("png");
            isForcedCodec = true;
            break;
        }
        case ST_TYPE_JPEG:
        case ST_TYPE_MPO:
        case ST_TYPE_JPS: {
            myCodec = avcodec_find_decoder_by_name("mjpeg");
            isForcedCodec = true;
            break;
        }
        case ST_TYPE_EXR: {
            myCodec = avcodec_find_decoder_by_name("exr");
            isForcedCodec = true;
            break;
        }
        case ST_TYPE_WEBP:
        case ST_TYPE_WEBPLL: {
            myCodec = avcodec_find_decoder_by_name("webp");
            isForcedCodec = true;
            break;
        }
        case ST_TYPE_DDS: {
            myCodec = avcodec_find_decoder_by_name("dds");
            isForcedCodec = true;
            break;
        }
        case ST_TYPE_PSD: {
            myCodec = avcodec_find_decoder_by_name("psd");
            isForcedCodec = true;
            break;
        }
        case ST_TYPE_ICO: {
            // .ico is a container format referring to multiple codecs
            static AVInputFormat* anIcoFormat = (AVInputFormat* )av_find_input_format("ico");
            anImgFormat = anIcoFormat;
            isForcedCodec = false;
            break;
        }
        default: {
            break;
        }
    }

    StHandle<StAVIOMemContext> aMemIoCtx;
    if(!isForcedCodec
    || (theDataPtr == NULL && !StFileNode::isFileExists(theFilePath))) {
        if (anImgFormat != NULL) {
            //
        } else if (theDataPtr != NULL) {
            static AVInputFormat* anImg2Format = (AVInputFormat* )av_find_input_format("image2");
            anImgFormat = anImg2Format;
            //anImgFormat->flags |= AVFMT_NOFILE;

            aMemIoCtx = new StAVIOMemContext();
            aMemIoCtx->wrapBuffer(theDataPtr, theDataSize);
            myFormatCtx = avformat_alloc_context();
            myFormatCtx->pb = aMemIoCtx->getAvioContext();
        } else {
            static AVInputFormat* anImg2PipeFormat = (AVInputFormat* )av_find_input_format("image2pipe");
            anImgFormat = anImg2PipeFormat;
            //anImgFormat->flags &= ~AVFMT_NOFILE;
        }

        // open image file and detect its type, it could be non local file!
        int avErrCode = avformat_open_input(&myFormatCtx, theFilePath.toCString(), anImgFormat, NULL);
        if(avErrCode != 0
        || myFormatCtx->nb_streams < 1
        || stAV::getCodecId(myFormatCtx->streams[0]) == AV_CODEC_ID_NONE) {
            if(myFormatCtx != NULL) {
                avformat_close_input(&myFormatCtx);
            }

            if(theDataPtr != NULL) {
                aMemIoCtx = new StAVIOMemContext();
                aMemIoCtx->wrapBuffer(theDataPtr, theDataSize);
                myFormatCtx = avformat_alloc_context();
                myFormatCtx->pb = aMemIoCtx->getAvioContext();
            }
            avErrCode = avformat_open_input(&myFormatCtx, theFilePath.toCString(), NULL, NULL);
        }

        if(avErrCode != 0
        || myFormatCtx->nb_streams < 1) {
            setState(StString("AVFormat library, couldn't open image file. Error: ") + stAV::getAVErrorDescription(avErrCode));
            close();
            return false;
        }

        // find the decoder for the video stream
    #if(LIBAVFORMAT_VERSION_INT < AV_VERSION_INT(59, 0, 100))
        myCodecCtx = stAV::getCodecCtx(myFormatCtx->streams[0]);
        if (!isForcedCodec) {
            myCodec = avcodec_find_decoder(myCodecCtx->codec_id);
        }
    #else
        if (!isForcedCodec) {
            myCodec = avcodec_find_decoder(myFormatCtx->streams[0]->codecpar->codec_id);
        }
    #endif
    }

    if(myCodec == NULL) {
        setState("AVCodec library, video codec not found");
        close();
        return false;
    } else if(myFormatCtx == NULL || myCodecCtx == NULL) {
        // use given image type to load decoder
        myCodecCtx = avcodec_alloc_context3(myCodec);
    }

    // stupid check
    if(myCodecCtx == NULL) {
        setState("AVCodec library, codec context is NULL");
        close();
        return false;
    }

    // open VIDEO codec
    if(avcodec_open2(myCodecCtx, myCodec, NULL) < 0) {
        setState("AVCodec library, could not open video codec");
        close();
        return false;
    }

    // read one packet or file
    StRawFile aRawFile(theFilePath);
    StAVPacket anAvPkt;
    if(theDataPtr != NULL && theDataSize != 0) {
        anAvPkt.getAVpkt()->data = theDataPtr;
        anAvPkt.getAVpkt()->size = theDataSize;
    } else {
        if(myFormatCtx != NULL) {
            if(av_read_frame(myFormatCtx, anAvPkt.getAVpkt()) < 0) {
                setState("AVFormat library, could not read first packet");
                close();
                return false;
            }
        } else {
            if(!aRawFile.readFile()) {
                setState("StAVImage, could not read the file");
                close();
                return false;
            }
            anAvPkt.getAVpkt()->data = (uint8_t* )aRawFile.getBuffer();
            anAvPkt.getAVpkt()->size = (int )aRawFile.getSize();
        }
    }
    anAvPkt.setKeyFrame();

    // decode one frame
    int isFrameFinished = 0;
    if(avcodec_send_packet(myCodecCtx, anAvPkt.getAVpkt()) == 0
    && avcodec_receive_frame(myCodecCtx, myFrame.Frame) == 0) {
        isFrameFinished = 1;
    }

    if(isFrameFinished == 0) {
        const bool toRetry = myFormatCtx == NULL && theImageType != ST_TYPE_NONE;
        setState("AVCodec library, input file is not an Image!");
        close();
        if(toRetry) {
            // try to detect codec from file content
            return loadExtra(theFilePath, ST_TYPE_NONE, anAvPkt.getAVpkt()->data, anAvPkt.getAVpkt()->size, theIsOnlyRGB);
        }
        return false;
    }

    // check frame size
    if(myCodecCtx->width <= 0 || myCodecCtx->height <= 0) {
        setState("AVCodec library, codec returns wrong frame size");
        close();
        return false;
    }

    // read aspect ratio
    if(myCodecCtx->sample_aspect_ratio.num == 0
    || myCodecCtx->sample_aspect_ratio.den == 0) {
        setPixelRatio(1.0f);
    } else {
        const GLfloat aRatio = GLfloat(myCodecCtx->sample_aspect_ratio.num) / GLfloat(myCodecCtx->sample_aspect_ratio.den);
        if(aRatio > 70.0f) {
            ST_DEBUG_LOG("AVCodec library, ignoring wrong PAR " + myCodecCtx->sample_aspect_ratio.num + ":" + myCodecCtx->sample_aspect_ratio.den);
            setPixelRatio(1.0f);
        } else {
            setPixelRatio(aRatio);
        }
    }

    // currently it is unlikely... but maybe in future?
    AVFrameSideData* aSideData = av_frame_get_side_data(myFrame.Frame, AV_FRAME_DATA_STEREO3D);
    if(aSideData != NULL) {
        AVStereo3D* aStereo = (AVStereo3D* )aSideData->data;
        mySrcFormat = stAV::stereo3dAvToSt(aStereo->type);
        if(aStereo->flags & AV_STEREO3D_FLAG_INVERT) {
            mySrcFormat = st::formatReversed(mySrcFormat);
        }
    } else {
        mySrcFormat = StFormat_AUTO;
    }

    // it is unlikely that there would be any metadata from format...
    // but lets try
    if(myFormatCtx != NULL) {
        for(stAV::meta::Tag* aTag = stAV::meta::findTag(myFormatCtx->metadata, "", NULL, stAV::meta::SEARCH_IGNORE_SUFFIX);
            aTag != NULL;
            aTag = stAV::meta::findTag(myFormatCtx->metadata, "", aTag, stAV::meta::SEARCH_IGNORE_SUFFIX)) {
            myMetadata.add(StDictEntry(aTag->key, aTag->value));
        }
        for(stAV::meta::Tag* aTag = stAV::meta::findTag(myFormatCtx->streams[0]->metadata, "", NULL, stAV::meta::SEARCH_IGNORE_SUFFIX);
            aTag != NULL;
            aTag = stAV::meta::findTag(myFormatCtx->streams[0]->metadata, "", aTag, stAV::meta::SEARCH_IGNORE_SUFFIX)) {
            myMetadata.add(StDictEntry(aTag->key, aTag->value));
        }
    }

    // collect metadata from the frame
    stAV::meta::Dict* aFrameMetadata = stAV::meta::getFrameMetadata(myFrame.Frame);
    for(stAV::meta::Tag* aTag = stAV::meta::findTag(aFrameMetadata, "", NULL, stAV::meta::SEARCH_IGNORE_SUFFIX);
        aTag != NULL;
        aTag = stAV::meta::findTag(aFrameMetadata, "", aTag, stAV::meta::SEARCH_IGNORE_SUFFIX)) {
        myMetadata.add(StDictEntry(aTag->key, aTag->value));
    }

    stAV::dimYUV aDimsYUV;
    if(myCodecCtx->pix_fmt == stAV::PIX_FMT::RGB24) {
        setColorModel(StImage::ImgColor_RGB);
        changePlane(0).initWrapper(StImagePlane::ImgRGB, myFrame.getPlane(0),
                                   myCodecCtx->width, myCodecCtx->height,
                                   myFrame.getLineSize(0));
    } else if(myCodecCtx->pix_fmt == stAV::PIX_FMT::BGR24) {
        setColorModel(StImage::ImgColor_RGB);
        changePlane(0).initWrapper(StImagePlane::ImgBGR, myFrame.getPlane(0),
                                   myCodecCtx->width, myCodecCtx->height,
                                   myFrame.getLineSize(0));
    } else if(myCodecCtx->pix_fmt == stAV::PIX_FMT::RGBA32) {
        setColorModel(StImage::ImgColor_RGBA);
        changePlane(0).initWrapper(StImagePlane::ImgRGBA, myFrame.getPlane(0),
                                   myCodecCtx->width, myCodecCtx->height,
                                   myFrame.getLineSize(0));
    } else if(myCodecCtx->pix_fmt == stAV::PIX_FMT::BGRA32) {
        setColorModel(StImage::ImgColor_RGBA);
        changePlane(0).initWrapper(StImagePlane::ImgBGRA, myFrame.getPlane(0),
                                   myCodecCtx->width, myCodecCtx->height,
                                   myFrame.getLineSize(0));
    } else if(myCodecCtx->pix_fmt == stAV::PIX_FMT::GRAY8) {
        setColorModel(StImage::ImgColor_GRAY);
        changePlane(0).initWrapper(StImagePlane::ImgGray, myFrame.getPlane(0),
                                   myCodecCtx->width, myCodecCtx->height,
                                   myFrame.getLineSize(0));
    } else if(myCodecCtx->pix_fmt == stAV::PIX_FMT::GRAY16) {
        setColorModel(StImage::ImgColor_GRAY);
        changePlane(0).initWrapper(StImagePlane::ImgGray16, myFrame.getPlane(0),
                                   myCodecCtx->width, myCodecCtx->height,
                                   myFrame.getLineSize(0));
    } else if(myCodecCtx->pix_fmt == stAV::PIX_FMT::GRAYF32) {
        setColorModel(StImage::ImgColor_GRAY);
        changePlane(0).initWrapper(StImagePlane::ImgGrayF, myFrame.getPlane(0),
                                   myCodecCtx->width, myCodecCtx->height,
                                   myFrame.getLineSize(0));
    } else if(myCodecCtx->pix_fmt == stAV::PIX_FMT::RGB48) {
        setColorModel(StImage::ImgColor_RGB);
        changePlane(0).initWrapper(StImagePlane::ImgRGB48, myFrame.getPlane(0),
                                   myCodecCtx->width, myCodecCtx->height,
                                   myFrame.getLineSize(0));
    } else if(myCodecCtx->pix_fmt == stAV::PIX_FMT::RGBA64) {
        setColorModel(StImage::ImgColor_RGBA);
        changePlane(0).initWrapper(StImagePlane::ImgRGBA64, myFrame.getPlane(0),
                                   myCodecCtx->width, myCodecCtx->height,
                                   myFrame.getLineSize(0));
    } else if(myCodecCtx->pix_fmt == stAV::PIX_FMT::GBRPF32 || myCodecCtx->pix_fmt == stAV::PIX_FMT::GBRAPF32) {
        // planar RGB(A) is somewhat unusual and suboptimal - always convert to packed RGB(A)
        const bool hasAlpha = myCodecCtx->pix_fmt == stAV::PIX_FMT::GBRAPF32;

        StImagePlane aPlanes[4];
        aPlanes[0].initWrapper(StImagePlane::ImgGrayF, myFrame.getPlane(2),
                               myCodecCtx->width, myCodecCtx->height, myFrame.getLineSize(2));
        aPlanes[1].initWrapper(StImagePlane::ImgGrayF, myFrame.getPlane(0),
                               myCodecCtx->width, myCodecCtx->height, myFrame.getLineSize(0));
        aPlanes[2].initWrapper(StImagePlane::ImgGrayF, myFrame.getPlane(1),
                               myCodecCtx->width, myCodecCtx->height, myFrame.getLineSize(1));
        if (hasAlpha) {
            aPlanes[3].initWrapper(StImagePlane::ImgGrayF, myFrame.getPlane(3),
                                   myCodecCtx->width, myCodecCtx->height, myFrame.getLineSize(3));
        }

        setColorModel(hasAlpha ? StImage::ImgColor_RGBA : StImage::ImgColor_RGB);
        changePlane(0).initZero(hasAlpha ? StImagePlane::ImgRGBAF : StImagePlane::ImgRGBF, myCodecCtx->width, myCodecCtx->height);
        const int aNbComps = hasAlpha ? 4 : 3;
        for (int aPlnIter = 0; aPlnIter < 4; ++aPlnIter) {
            const StImagePlane& aSrcPln = aPlanes[aPlnIter];
            if (aSrcPln.isNull()) {
                continue;
            }
            for (int aRow = 0; aRow < myCodecCtx->height; ++aRow) {
                float* aDstRow = (float*)changePlane(0).changeData(aRow) + aPlnIter;
                const float* aSrcRow = (const float*)aSrcPln.getData(aRow);
                for(int aCol = 0; aCol < myCodecCtx->width; ++aCol) {
                    aDstRow[aCol * aNbComps] = aSrcRow[aCol];
                }
            }
        }
    } else if(stAV::isFormatYUVPlanar(myCodecCtx, aDimsYUV) && !theIsOnlyRGB) {
        if(myCodecCtx->color_range == AVCOL_RANGE_JPEG) {
            aDimsYUV.isFullScale = true;
        }

        setColorModel(aDimsYUV.hasAlpha ? StImage::ImgColor_YUVA : StImage::ImgColor_YUV);
        setColorScale(aDimsYUV.isFullScale ? StImage::ImgScale_Full : StImage::ImgScale_Mpeg);
        StImagePlane::ImgFormat aPlaneFrmt = StImagePlane::ImgGray;
        if(aDimsYUV.bitsPerComp == 9) {
            aPlaneFrmt = StImagePlane::ImgGray16;
            setColorScale(aDimsYUV.isFullScale ? StImage::ImgScale_Jpeg9  : StImage::ImgScale_Mpeg9);
        } else if(aDimsYUV.bitsPerComp == 10) {
            aPlaneFrmt = StImagePlane::ImgGray16;
            setColorScale(aDimsYUV.isFullScale ? StImage::ImgScale_Jpeg10 : StImage::ImgScale_Mpeg10);
        } else if(aDimsYUV.bitsPerComp == 16) {
            aPlaneFrmt = StImagePlane::ImgGray16;
        }

        changePlane(0).initWrapper(aPlaneFrmt, myFrame.getPlane(0),
                                   size_t(aDimsYUV.widthY), size_t(aDimsYUV.heightY), myFrame.getLineSize(0));
        changePlane(1).initWrapper(aPlaneFrmt, myFrame.getPlane(1),
                                   size_t(aDimsYUV.widthU), size_t(aDimsYUV.heightU), myFrame.getLineSize(1));
        changePlane(2).initWrapper(aPlaneFrmt, myFrame.getPlane(2),
                                   size_t(aDimsYUV.widthV), size_t(aDimsYUV.heightV), myFrame.getLineSize(2));
        if(aDimsYUV.hasAlpha) {
            changePlane(3).initWrapper(aPlaneFrmt, myFrame.getPlane(3),
                                       size_t(aDimsYUV.widthY), size_t(aDimsYUV.heightY), myFrame.getLineSize(3));
        }
    } else {
        ///ST_DEBUG_LOG("StAVImage, perform conversion from Pixel format '" + avcodec_get_pix_fmt_name(myCodecCtx->pix_fmt) + "' to RGB");

        const AVPixFmtDescriptor* aDesc = av_pix_fmt_desc_get(myCodecCtx->pix_fmt);
        const bool hasAlpha = aDesc != NULL && (aDesc->flags & AV_PIX_FMT_FLAG_ALPHA) != 0;

        // initialize software scaler/converter
        SwsContext* pToRgbCtx = sws_getContext(myCodecCtx->width, myCodecCtx->height, myCodecCtx->pix_fmt,    // source
                                               myCodecCtx->width, myCodecCtx->height, hasAlpha ? stAV::PIX_FMT::RGBA32 : stAV::PIX_FMT::RGB24, // destination
                                               SWS_BICUBIC, NULL, NULL, NULL);
        if(pToRgbCtx == NULL) {
            setState("SWScale library, failed to create SWScaler context");
            close();
            return false;
        }

        // initialize additional buffer for converted RGB data
        setColorModel(hasAlpha ? StImage::ImgColor_RGBA : StImage::ImgColor_RGB);
        changePlane(0).initTrash(hasAlpha ? StImagePlane::ImgRGBA : StImagePlane::ImgRGB,
                                 myCodecCtx->width, myCodecCtx->height);

        uint8_t* rgbData[4]; stMemZero(rgbData,     sizeof(rgbData));
        int  rgbLinesize[4]; stMemZero(rgbLinesize, sizeof(rgbLinesize));
        rgbData[0]     = changePlane(0).changeData();
        rgbLinesize[0] = (int )changePlane(0).getSizeRowBytes();

        sws_scale(pToRgbCtx,
                  myFrame.Frame->data, myFrame.Frame->linesize,
                  0, myCodecCtx->height,
                  rgbData, rgbLinesize);
        // reset original data
        closeAvCtx();

        sws_freeContext(pToRgbCtx);
    }

    // set debug information
    StString aDummy, aFileName;
    StFileNode::getFolderAndFile(theFilePath, aDummy, aFileName);
    setState(StString("AVCodec library, loaded image '") + aFileName + "' " + getDescription());

    // we should not close the file because decoded image data is in codec context cache
    return true;
}

bool StAVImage::save(const StString& theFilePath,
                     const SaveImageParams& theParams) {
    close();
    setState();
    if(isNull()) {
        return false;
    }

    AVPixelFormat aPFormatAV = (AVPixelFormat )getAVPixelFormat(*this);
    if(aPFormatAV == stAV::PIX_FMT::NONE) {
        setState("Specific pixel format conversion is not supported");
        return false;
    }

    StImage anImage;
    switch(theParams.SaveImageType) {
        case ST_TYPE_PNG:
        case ST_TYPE_PNS: {
            myCodec = avcodec_find_encoder_by_name("png");
            if(myCodec == NULL) {
                setState("AVCodec library, video codec 'png' not found");
                close();
                return false;
            }
            if(aPFormatAV == stAV::PIX_FMT::RGB24
            || aPFormatAV == stAV::PIX_FMT::RGBA32
            || aPFormatAV == stAV::PIX_FMT::GRAY8) {
                anImage.initWrapper(*this);
            } else {
                // convert to compatible pixel format
                anImage.changePlane().initTrash(StImagePlane::ImgRGB, getSizeX(), getSizeY(), getAligned(getSizeX() * 3));
                AVPixelFormat aPFrmtTarget = stAV::PIX_FMT::RGB24;
                if(!convert(*this,   aPFormatAV,
                            anImage, aPFrmtTarget,
                            THE_SWSCALE_FLAGS_QUALITY)) {
                    setState("SWScale library, failed to create SWScaler context");
                    close();
                    return false;
                }
                aPFormatAV = aPFrmtTarget;
            }
            myCodecCtx = avcodec_alloc_context3(myCodec);

            // setup encoder
            myCodecCtx->pix_fmt = aPFormatAV;
            myCodecCtx->width   = (int )anImage.getSizeX();
            myCodecCtx->height  = (int )anImage.getSizeY();
            myCodecCtx->time_base.num = 1;
            myCodecCtx->time_base.den = 1;

            // PNG compression level is within [0..9] range
            static const int PNG_QLOWER = 0;
            static const int PNG_QUPPER = 9;
            const float aRatio = stClamp(theParams.Compression >= 0.0f ? theParams.Compression : 1.0f, 0.0f, 1.0f);
            int aQLevel = stRound(stLerp(float(PNG_QLOWER), float(PNG_QUPPER), aRatio));
            myCodecCtx->compression_level = aQLevel;
            break;
        }
        case ST_TYPE_JPEG:
        case ST_TYPE_MPO:
        case ST_TYPE_JPS: {
            myCodec = avcodec_find_encoder_by_name("mjpeg");
            if(myCodec == NULL) {
                setState("AVCodec library, video codec 'mjpeg' not found");
                close();
                return false;
            }

            if(aPFormatAV == stAV::PIX_FMT::YUVJ420P
            || aPFormatAV == stAV::PIX_FMT::YUVJ422P
            || aPFormatAV == stAV::PIX_FMT::YUVJ444P
            || aPFormatAV == stAV::PIX_FMT::YUVJ440P) {
                anImage.initWrapper(*this);
            } else {
                // convert to compatible pixel format
                AVPixelFormat aPFrmtTarget = aPFormatAV == stAV::PIX_FMT::YUV420P ? stAV::PIX_FMT::YUVJ420P : stAV::PIX_FMT::YUVJ422P;
                anImage.setColorModel(StImage::ImgColor_YUV);
                anImage.setColorScale(StImage::ImgScale_Mpeg);
                anImage.changePlane(0).initTrash(StImagePlane::ImgGray, getSizeX(), getSizeY(), getAligned(getSizeX()));
                stMemSet(anImage.changePlane(0).changeData(), '\0', anImage.getPlane(0).getSizeBytes());
                anImage.changePlane(1).initTrash(StImagePlane::ImgGray, getSizeX(), getSizeY(), getAligned(getSizeX()));
                stMemSet(anImage.changePlane(1).changeData(), '\0', anImage.getPlane(1).getSizeBytes());
                anImage.changePlane(2).initTrash(StImagePlane::ImgGray, getSizeX(), getSizeY(), getAligned(getSizeX()));
                stMemSet(anImage.changePlane(2).changeData(), '\0', anImage.getPlane(2).getSizeBytes());
                if(!convert(*this,   aPFormatAV,
                            anImage, aPFrmtTarget,
                            THE_SWSCALE_FLAGS_QUALITY)) {
                    setState("SWScale library, failed to create SWScaler context");
                    close();
                    return false;
                }
                aPFormatAV = aPFrmtTarget;
            }

            myCodecCtx = avcodec_alloc_context3(myCodec);
            myCodecCtx->pix_fmt = aPFormatAV;
            myCodecCtx->width   = (int )anImage.getSizeX();
            myCodecCtx->height  = (int )anImage.getSizeY();
            myCodecCtx->time_base.num = 1;
            myCodecCtx->time_base.den = 1;

            // quantizer factor within 1..31 range, lesser is better
            static const int JPEG_QLOWER = 1;
            static const int JPEG_QUPPER = 31;
            const float aRatio = stClamp(theParams.Compression >= 0.0f ? theParams.Compression : 0.10f, 0.0f, 1.0f);
            int aQFactor = stRound(stLerp(float(JPEG_QLOWER), float(JPEG_QUPPER), aRatio));
            myCodecCtx->qmin = myCodecCtx->qmax = aQFactor;
            break;
        }
        case ST_TYPE_NONE:
        default:
            close();
            return false;
    }

    // open VIDEO codec
    if(avcodec_open2(myCodecCtx, myCodec, NULL) < 0) {
        setState("AVCodec library, could not open video codec");
        close();
        return false;
    }

    // wrap own data into AVFrame
    myFrame.Frame->format = myCodecCtx->pix_fmt;
    myFrame.Frame->width  = myCodecCtx->width;
    myFrame.Frame->height = myCodecCtx->height;
    fillPointersAV(anImage, myFrame.Frame->data, myFrame.Frame->linesize);

    bool isReversed = false;
    AVStereo3DType anAvStereoType = stAV::stereo3dStToAv(theParams.StereoFormat, isReversed);
    if(anAvStereoType != (AVStereo3DType )-1) {
        AVStereo3D* aStereo = av_stereo3d_create_side_data(myFrame.Frame);
        if(aStereo != NULL) {
            aStereo->type = anAvStereoType;
            if(isReversed) {
                aStereo->flags |= AV_STEREO3D_FLAG_INVERT;
            }
        }
    }

    StJpegParser aRawFile(theFilePath);
    if(!aRawFile.openFile(StRawFile::WRITE)) {
        setState("Can not open the file for writing");
        close();
        return false;
    }

    // encode the image
    StAVPacket aPacket;
    int anEncSize = 0;
    if(avcodec_send_frame(myCodecCtx, myFrame.Frame) == 0
    && avcodec_receive_packet(myCodecCtx, aPacket.getAVpkt()) == 0
    && aPacket.getAVpkt()->data != NULL) {
        anEncSize = aPacket.getSize();
        aRawFile.wrapBuffer(aPacket.getAVpkt()->data, anEncSize);
    }

    if(anEncSize <= 0) {
        setState("AVCodec library, fail to encode the image");
        close();
        return false;
    }
    aRawFile.setDataSize((size_t )anEncSize);

    // save metadata when possible
    if(theParams.SaveImageType == ST_TYPE_JPEG
    || theParams.SaveImageType == ST_TYPE_JPS) {
        if(aRawFile.parse()) {
            if(theParams.StereoFormat != StFormat_AUTO) {
                aRawFile.setupJps(theParams.StereoFormat);
            }
        } else {
            ST_ERROR_LOG("AVCodec library, created JPEG can not be parsed!");
        }
    }

    // store current content
    aRawFile.writeFile();
    // and finally close the file handle
    aRawFile.closeFile();

    close();

    // set debug information
    StString aDummy, aFileName;
    StFileNode::getFolderAndFile(theFilePath, aDummy, aFileName);
    setState(StString("AVCodec library, saved image '") + aFileName + "' " + getDescription());

    return true;
}
