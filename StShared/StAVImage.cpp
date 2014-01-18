/**
 * Copyright © 2011-2013 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#include <StAV/StAVImage.h>

#include <StAV/StAVPacket.h>
#include <StFile/StFileNode.h>
#include <StFile/StRawFile.h>
#include <StStrings/StLogger.h>

bool StAVImage::init() {
    return stAV::init();
}

StAVImage::StAVImage()
: StImageFile(),
  imageFormat(NULL),
  formatCtx(NULL),
  codecCtx(NULL),
  codec(NULL),
  frame(NULL) {
    StAVImage::init();
    imageFormat = av_find_input_format("image2");
    frame = avcodec_alloc_frame();
}

StAVImage::~StAVImage() {
    close();
    av_free(frame);
}

int StAVImage::getAVPixelFormat() {
    if(isPacked()) {
        switch(getPlane(0).getFormat()) {
            case StImagePlane::ImgRGB:    return stAV::PIX_FMT::RGB24;
            case StImagePlane::ImgBGR:    return stAV::PIX_FMT::BGR24;
            case StImagePlane::ImgRGBA:   return stAV::PIX_FMT::RGBA32;
            case StImagePlane::ImgBGRA:   return stAV::PIX_FMT::BGRA32;
            case StImagePlane::ImgGray:   return stAV::PIX_FMT::GRAY8;
            case StImagePlane::ImgGray16: return stAV::PIX_FMT::GRAY16;
            default:                      return stAV::PIX_FMT::NONE;
        }
    }
    switch(getColorModel()) {
        case StImage::ImgColor_YUV: {
            size_t aDelimX = (getPlane(1).getSizeX() > 0) ? (getPlane(0).getSizeX() / getPlane(1).getSizeX()) : 1;
            size_t aDelimY = (getPlane(1).getSizeY() > 0) ? (getPlane(0).getSizeY() / getPlane(1).getSizeY()) : 1;
            if(aDelimX == 1 && aDelimY == 1) {
                switch(getColorScale()) {
                    case StImage::ImgScale_Mpeg:
                        return getPlane(0).getFormat() == StImagePlane::ImgGray16
                             ? stAV::PIX_FMT::YUV444P16
                             : stAV::PIX_FMT::YUV444P;
                    case StImage::ImgScale_Mpeg9:
                    case StImage::ImgScale_Jpeg9:  return stAV::PIX_FMT::YUV444P9;
                    case StImage::ImgScale_Mpeg10:
                    case StImage::ImgScale_Jpeg10: return stAV::PIX_FMT::YUV444P10;
                    case StImage::ImgScale_Full:
                    default:
                        return getPlane(0).getFormat() == StImagePlane::ImgGray16
                             ? stAV::PIX_FMT::YUV444P16 //
                             : stAV::PIX_FMT::YUVJ444P;
                }
            } else if(aDelimX == 2 && aDelimY == 2) {
                switch(getColorScale()) {
                    case StImage::ImgScale_Mpeg:
                        return getPlane(0).getFormat() == StImagePlane::ImgGray16
                             ? stAV::PIX_FMT::YUV420P16
                             : stAV::PIX_FMT::YUV420P;
                    case StImage::ImgScale_Mpeg9:
                    case StImage::ImgScale_Jpeg9:  return stAV::PIX_FMT::YUV420P9;
                    case StImage::ImgScale_Mpeg10:
                    case StImage::ImgScale_Jpeg10: return stAV::PIX_FMT::YUV420P10;
                    case StImage::ImgScale_Full:
                    default:
                        return getPlane(0).getFormat() == StImagePlane::ImgGray16
                             ? stAV::PIX_FMT::YUV420P16 // jpeg color range ignored!
                             : stAV::PIX_FMT::YUVJ420P;
                }
            } else if(aDelimX == 2 && aDelimY == 1) {
                switch(getColorScale()) {
                    case StImage::ImgScale_Mpeg:
                        return getPlane(0).getFormat() == StImagePlane::ImgGray16
                             ? stAV::PIX_FMT::YUV422P16 // jpeg color range ignored!
                             : stAV::PIX_FMT::YUV422P;
                    case StImage::ImgScale_Mpeg9:
                    case StImage::ImgScale_Jpeg9:  return stAV::PIX_FMT::YUV422P9;
                    case StImage::ImgScale_Mpeg10:
                    case StImage::ImgScale_Jpeg10: return stAV::PIX_FMT::YUV422P10;
                    case StImage::ImgScale_Full:
                    default:
                        return getPlane(0).getFormat() == StImagePlane::ImgGray16
                             ? stAV::PIX_FMT::YUV422P16 // jpeg color range ignored!
                             : stAV::PIX_FMT::YUVJ422P;
                }
            } else if(aDelimX == 1 && aDelimY == 2) {
                return getColorScale() == StImage::ImgScale_Mpeg
                     ? stAV::PIX_FMT::YUVJ440P : stAV::PIX_FMT::YUV440P;
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

/**
 * Convert image from one format to another using swscale.
 * Image buffers should be already initialized!
 */
static bool convert(const StImage& theImageFrom, PixelFormat theFormatFrom,
                          StImage& theImageTo,   PixelFormat theFormatTo) {
    ST_DEBUG_LOG("StAVImage, convert from " + stAV::PIX_FMT::getString(theFormatFrom)
               + " to " + stAV::PIX_FMT::getString(theFormatTo) + " using swscale");
    SwsContext* pToRgbCtx = sws_getContext((int )theImageFrom.getSizeX(), (int )theImageFrom.getSizeY(), theFormatFrom, // source
                                           (int )theImageTo.getSizeX(),   (int )theImageTo.getSizeY(),   theFormatTo,   // destination
                                           SWS_BICUBIC, NULL, NULL, NULL);
    if(pToRgbCtx == NULL) {
        return false;
    }

    uint8_t* aSrcData[4]; int aSrcLinesize[4];
    fillPointersAV(theImageFrom, aSrcData, aSrcLinesize);

    uint8_t* aDstData[4]; int aDstLinesize[4];
    fillPointersAV(theImageTo, aDstData, aDstLinesize);

    sws_scale(pToRgbCtx,
              aSrcData, aSrcLinesize,
              0, (int )theImageFrom.getSizeY(),
              aDstData, aDstLinesize);

    sws_freeContext(pToRgbCtx);
    return true;
}

void StAVImage::close() {
    if(codec != NULL && codecCtx != NULL) {
        avcodec_close(codecCtx); // close VIDEO codec
    }
    codec = NULL;
    if(formatCtx != NULL) {
    #if(LIBAVFORMAT_VERSION_INT >= AV_VERSION_INT(53, 17, 0))
        avformat_close_input(&formatCtx);
    #else
        av_close_input_file(formatCtx); // close video file
        formatCtx = NULL;
    #endif
        // codec context allocated by av_open_input_file() function
        codecCtx = NULL;
    } else if(codecCtx != NULL) {
        // codec context allocated by ourself with avcodec_alloc_context() function
        av_freep(&codecCtx);
    }
}

bool StAVImage::load(const StString& theFilePath, ImageType theImageType,
                     uint8_t* theDataPtr, int theDataSize) {

    // reset current data
    StImage::nullify();
    setState();
    close();

    switch(theImageType) {
        case ST_TYPE_PNG:
        case ST_TYPE_PNS: {
            codec = avcodec_find_decoder_by_name("png");
            break;
        }
        case ST_TYPE_JPEG:
        case ST_TYPE_MPO:
        case ST_TYPE_JPS: {
            codec = avcodec_find_decoder_by_name("mjpeg");
            break;
        }
        default: {
            break;
        }
    }

    if(theImageType == ST_TYPE_NONE || !StFileNode::isFileExists(theFilePath)) {
        // open image file and detect its type, its could be non local file!
    #if(LIBAVFORMAT_VERSION_INT >= AV_VERSION_INT(53, 2, 0))
        int avErrCode = avformat_open_input(&formatCtx, theFilePath.toCString(), imageFormat, NULL);
    #else
        int avErrCode = av_open_input_file(&formatCtx, theFilePath.toCString(), imageFormat, 0, NULL);
    #endif
        if(avErrCode != 0
        || formatCtx->nb_streams < 1
        || formatCtx->streams[0]->codec->codec_id == 0) {
            if(formatCtx != NULL) {
            #if(LIBAVFORMAT_VERSION_INT >= AV_VERSION_INT(53, 17, 0))
                avformat_close_input(&formatCtx);
            #else
                av_close_input_file(formatCtx);
                formatCtx = NULL;
            #endif
            }

        #if(LIBAVFORMAT_VERSION_INT >= AV_VERSION_INT(53, 2, 0))
            avErrCode = avformat_open_input(&formatCtx, theFilePath.toCString(), NULL, NULL);
        #else
            avErrCode = av_open_input_file(&formatCtx, theFilePath.toCString(), NULL, 0, NULL);
        #endif
        }

        if(avErrCode != 0
        || formatCtx->nb_streams < 1) {
            setState(StString("AVFormat library, couldn't open image file. Error: ") + stAV::getAVErrorDescription(avErrCode));
            close();
            return false;
        }

        // find the decoder for the video stream
        codecCtx = formatCtx->streams[0]->codec;
        if(theImageType == ST_TYPE_NONE) {
            codec = avcodec_find_decoder(codecCtx->codec_id);
        }
    }

    if(codec == NULL) {
        setState("AVCodec library, video codec not found");
        close();
        return false;
    } else if(formatCtx == NULL) {
        // use given image type to load decoder
    #if(LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(53, 8, 0))
        codecCtx = avcodec_alloc_context3(codec);
    #else
        codecCtx = avcodec_alloc_context();
    #endif
    }

    // stupid check
    if(codecCtx == NULL) {
        setState("AVCodec library, codec context is NULL");
        close();
        return false;
    }

    // open VIDEO codec
#if(LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(53, 8, 0))
    if(avcodec_open2(codecCtx, codec, NULL) < 0) {
#else
    if(avcodec_open(codecCtx, codec) < 0) {
#endif
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
        if(formatCtx != NULL) {
            if(av_read_frame(formatCtx, anAvPkt.getAVpkt()) < 0) {
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
#if(LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(52, 23, 0))
    avcodec_decode_video2(codecCtx, frame, &isFrameFinished, anAvPkt.getAVpkt());
#else
    avcodec_decode_video(codecCtx, frame, &isFrameFinished,
                         theDataPtr, theDataSize);
#endif

    if(isFrameFinished == 0) {
        // thats not an image!!! try to decode more packets???
        setState("AVCodec library, input file is not an Image!");
        close();
        return false;
    }

    // check frame size
    if(codecCtx->width <= 0 || codecCtx->height <= 0) {
        setState("AVCodec library, codec returns wrong frame size");
        close();
        return false;
    }

    // read aspect ratio
    if(codecCtx->sample_aspect_ratio.num == 0
    || codecCtx->sample_aspect_ratio.den == 0) {
        setPixelRatio(1.0f);
    } else {
        setPixelRatio(GLfloat(codecCtx->sample_aspect_ratio.num) / GLfloat(codecCtx->sample_aspect_ratio.den));
    }

    stAV::dimYUV aDimsYUV;
    if(codecCtx->pix_fmt == stAV::PIX_FMT::RGB24) {
        setColorModel(StImage::ImgColor_RGB);
        changePlane(0).initWrapper(StImagePlane::ImgRGB, frame->data[0],
                                   codecCtx->width, codecCtx->height,
                                   frame->linesize[0]);
    } else if(codecCtx->pix_fmt == stAV::PIX_FMT::BGR24) {
        setColorModel(StImage::ImgColor_RGB);
        changePlane(0).initWrapper(StImagePlane::ImgBGR, frame->data[0],
                                   codecCtx->width, codecCtx->height,
                                   frame->linesize[0]);
    } else if(codecCtx->pix_fmt == stAV::PIX_FMT::RGBA32) {
        setColorModel(StImage::ImgColor_RGBA);
        changePlane(0).initWrapper(StImagePlane::ImgRGBA, frame->data[0],
                                   codecCtx->width, codecCtx->height,
                                   frame->linesize[0]);
    } else if(codecCtx->pix_fmt == stAV::PIX_FMT::BGRA32) {
        setColorModel(StImage::ImgColor_RGBA);
        changePlane(0).initWrapper(StImagePlane::ImgBGRA, frame->data[0],
                                   codecCtx->width, codecCtx->height,
                                   frame->linesize[0]);
    } else if(codecCtx->pix_fmt == stAV::PIX_FMT::GRAY8) {
        setColorModel(StImage::ImgColor_GRAY);
        changePlane(0).initWrapper(StImagePlane::ImgGray, frame->data[0],
                                   codecCtx->width, codecCtx->height,
                                   frame->linesize[0]);
    } else if(codecCtx->pix_fmt == stAV::PIX_FMT::GRAY16) {
        setColorModel(StImage::ImgColor_GRAY);
        changePlane(0).initWrapper(StImagePlane::ImgGray16, frame->data[0],
                                   codecCtx->width, codecCtx->height,
                                   frame->linesize[0]);
    } else if(stAV::isFormatYUVPlanar(codecCtx, aDimsYUV)) {
    #if(LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(52, 29, 0))
        if(codecCtx->color_range == AVCOL_RANGE_JPEG) {
            aDimsYUV.isFullScale = true;
        }
    #endif
        setColorModel(StImage::ImgColor_YUV);
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

        changePlane(0).initWrapper(aPlaneFrmt, frame->data[0],
                                   size_t(aDimsYUV.widthY), size_t(aDimsYUV.heightY), frame->linesize[0]);
        changePlane(1).initWrapper(aPlaneFrmt, frame->data[1],
                                   size_t(aDimsYUV.widthU), size_t(aDimsYUV.heightU), frame->linesize[1]);
        changePlane(2).initWrapper(aPlaneFrmt, frame->data[2],
                                   size_t(aDimsYUV.widthV), size_t(aDimsYUV.heightV), frame->linesize[2]);
    } else {
        ///ST_DEBUG_LOG("StAVImage, perform conversion from Pixel format '" + avcodec_get_pix_fmt_name(codecCtx->pix_fmt) + "' to RGB");
        // initialize software scaler/converter
        SwsContext* pToRgbCtx = sws_getContext(codecCtx->width, codecCtx->height, codecCtx->pix_fmt,    // source
                                               codecCtx->width, codecCtx->height, stAV::PIX_FMT::RGB24, // destination
                                               SWS_BICUBIC, NULL, NULL, NULL);
        if(pToRgbCtx == NULL) {
            setState("SWScale library, failed to create SWScaler context");
            close();
            return false;
        }

        // initialize additional buffer for converted RGB data
        setColorModel(StImage::ImgColor_RGB);
        changePlane(0).initTrash(StImagePlane::ImgRGB,
                                 codecCtx->width, codecCtx->height);

        uint8_t* rgbData[4]; stMemZero(rgbData,     sizeof(rgbData));
        int  rgbLinesize[4]; stMemZero(rgbLinesize, sizeof(rgbLinesize));
        rgbData[0]     = changePlane(0).changeData();
        rgbLinesize[0] = (int )changePlane(0).getSizeRowBytes();

        sws_scale(pToRgbCtx,
                  frame->data, frame->linesize,
                  0, codecCtx->height,
                  rgbData, rgbLinesize);

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
                     ImageType       theImageType) {
    close();
    setState();
    if(isNull()) {
        return false;
    }

    PixelFormat aPFormatAV = (PixelFormat )getAVPixelFormat();
    StImage anImage;
    switch(theImageType) {
        case ST_TYPE_PNG:
        case ST_TYPE_PNS: {
            codec = avcodec_find_encoder_by_name("png");
            if(codec == NULL) {
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
                PixelFormat aPFrmtTarget = stAV::PIX_FMT::RGB24;
                if(!convert(*this,   aPFormatAV,
                            anImage, aPFrmtTarget)) {
                    setState("SWScale library, failed to create SWScaler context");
                    close();
                    return false;
                }
                aPFormatAV = aPFrmtTarget;
            }
        #if(LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(53, 8, 0))
            codecCtx = avcodec_alloc_context3(codec);
        #else
            codecCtx = avcodec_alloc_context();
        #endif

            // setup encoder
            codecCtx->pix_fmt = aPFormatAV;
            codecCtx->width   = (int )anImage.getSizeX();
            codecCtx->height  = (int )anImage.getSizeY();
            codecCtx->compression_level = 9; // 0..9
            break;
        }
        case ST_TYPE_JPEG:
        case ST_TYPE_MPO:
        case ST_TYPE_JPS: {
            codec = avcodec_find_encoder_by_name("mjpeg");
            if(codec == NULL) {
                setState("AVCodec library, video codec 'mjpeg' not found");
                close();
                return false;
            }

            if(aPFormatAV == stAV::PIX_FMT::YUVJ420P
            || aPFormatAV == stAV::PIX_FMT::YUVJ422P
            //|| aPFormatAV == stAV::PIX_FMT::YUVJ444P not supported by FFmpeg... yet?
            //|| aPFormatAV == stAV::PIX_FMT::YUVJ440P
               ) {
                anImage.initWrapper(*this);
            } else {
                // convert to compatible pixel format
                PixelFormat aPFrmtTarget = stAV::PIX_FMT::YUVJ422P;
                anImage.setColorModel(StImage::ImgColor_YUV);
                anImage.setColorScale(StImage::ImgScale_Mpeg);
                anImage.changePlane(0).initTrash(StImagePlane::ImgGray, getSizeX(), getSizeY(), getAligned(getSizeX()));
                stMemSet(anImage.changePlane(0).changeData(), '\0', anImage.getPlane(0).getSizeBytes());
                anImage.changePlane(1).initTrash(StImagePlane::ImgGray, getSizeX(), getSizeY(), getAligned(getSizeX()));
                stMemSet(anImage.changePlane(1).changeData(), '\0', anImage.getPlane(1).getSizeBytes());
                anImage.changePlane(2).initTrash(StImagePlane::ImgGray, getSizeX(), getSizeY(), getAligned(getSizeX()));
                stMemSet(anImage.changePlane(2).changeData(), '\0', anImage.getPlane(2).getSizeBytes());
                if(!convert(*this,   aPFormatAV,
                            anImage, aPFrmtTarget)) {
                    setState("SWScale library, failed to create SWScaler context");
                    close();
                    return false;
                }
                aPFormatAV = aPFrmtTarget;
            }

        #if(LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(53, 8, 0))
            codecCtx = avcodec_alloc_context3(codec);
        #else
            codecCtx = avcodec_alloc_context();
        #endif
            codecCtx->pix_fmt = aPFormatAV;
            codecCtx->width   = (int )anImage.getSizeX();
            codecCtx->height  = (int )anImage.getSizeY();
            codecCtx->time_base.num = 1;
            codecCtx->time_base.den = 1;
            codecCtx->qmin = codecCtx->qmax = 10; // quality factor - lesser is better
            break;
        }
        case ST_TYPE_NONE:
        default:
            close();
            return false;
    }

    // open VIDEO codec
#if(LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(53, 8, 0))
    if(avcodec_open2(codecCtx, codec, NULL) < 0) {
#else
    if(avcodec_open(codecCtx, codec) < 0) {
#endif
        setState("AVCodec library, could not open video codec");
        close();
        return false;
    }

    // wrap own data into AVFrame
    fillPointersAV(anImage, frame->data, frame->linesize);

    StRawFile aRawFile(theFilePath);
    if(!aRawFile.openFile(StRawFile::WRITE)) {
        setState("Can not open the file for writing");
        close();
        return false;
    }

    // allocate the buffer, large enough (stupid formula copied from ffmpeg.c)
    int aBuffSize = int(getSizeX() * getSizeY() * 10);
    aRawFile.initBuffer(aBuffSize);

    // encode the image
    int anEncSize = avcodec_encode_video(codecCtx, (uint8_t* )aRawFile.changeBuffer(), aBuffSize, frame);
    if(anEncSize <= 0) {
        setState("AVCodec library, fail to encode the image");
        close();
        return false;
    }

    // store current content
    aRawFile.writeFile(anEncSize);
    // and finally close the file handle
    aRawFile.closeFile();

    close();

    // set debug information
    StString aDummy, aFileName;
    StFileNode::getFolderAndFile(theFilePath, aDummy, aFileName);
    setState(StString("AVCodec library, saved image '") + aFileName + "' " + getDescription());

    return true;
}

bool StAVImage::resize(size_t , size_t ) {
    return false;
}
