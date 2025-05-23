/**
 * Copyright © 2011-2025 Kirill Gavrilov <kirill@sview.ru>
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

#include "StSubtitleQueue.h"

#include <StGLWidgets/StSubQueue.h>
#include <StThreads/StThread.h>

#include <StAV/StAVImage.h>

namespace {
    static const StString ST_CRLF_REDUNDANT   = "\x0D\x0A";
    static const StString ST_CRLF_REPLACEMENT = " \x0A";
}

/**
 * Thread function just call decodeLoop() function.
 */
static SV_THREAD_FUNCTION threadFunction(void* theSubtitleQueue) {
    StSubtitleQueue* aSubtitleQueue = (StSubtitleQueue* )theSubtitleQueue;
    aSubtitleQueue->decodeLoop();
    return SV_THREAD_RETURN 0;
}

StSubtitleQueue::StSubtitleQueue(const StHandle<StSubQueue>& theSubtitlesQueue)
: StAVPacketQueue(512),
  myOutQueue(theSubtitlesQueue),
  myThread(NULL),
  evDowntime(true),
  myImageScale(1.0f),
  toQuit(false) {
    myThread = new StThread(threadFunction, (void* )this, "StSubtitleQueue");
}

StSubtitleQueue::~StSubtitleQueue() {
    toQuit = true;
    pushQuit();

    myThread->wait();
    delete myThread;

    deinit();
}

bool StSubtitleQueue::init(AVFormatContext*   theFormatCtx,
                           const unsigned int theStreamId,
                           const StString&    theFileName) {
    myImageScale = 1.0f;
    if(!StAVPacketQueue::init(theFormatCtx, theStreamId, theFileName)) {
        signals.onError(stCString("FFmpeg: invalid stream"));
        deinit();
        return false;
    }

    if(myCodecAutoId != AV_CODEC_ID_TEXT) {
        // open SUBTITLE codec
        if(avcodec_open2(myCodecCtx, myCodecAuto, NULL) < 0) {
            signals.onError(stCString("FFmpeg: Could not open subtitle codec"));
            deinit();
            return false;
        }
        myCodec = myCodecAuto;

        // initialize ASS parser
        /// TODO (Kirill Gavrilov#6) when we should do that?
        myASS.init((char* )myCodecCtx->subtitle_header,
                   myCodecCtx->subtitle_header_size);
    }
    if(myCodecAuto != NULL && StString(myCodecAuto->name).isEquals(stCString("pgssub"))) {
        myImageScale = 0.5f;
    }
    fillCodecInfo(myCodec);
    return true;
}

void StSubtitleQueue::deinit() {
    StAVPacketQueue::deinit();
    myASS.init(NULL, 0);
}

void StSubtitleQueue::decodeLoop() {
    int isFrameFinished = 0;
    double aPts = 0.0;
    double aDuration = 0.0;
    AVSubtitle aSubtitle;

    for(;;) {
        if(isEmpty()) {
            evDowntime.set();
            StThread::sleep(10);
            continue;
        }
        evDowntime.reset();

        StHandle<StAVPacket> aPacket = pop();
        if(aPacket.isNull()) {
            continue;
        }
        switch(aPacket->getType()) {
            case StAVPacket::FLUSH_PACKET: {
                // got the special FLUSH packet - flush FFmpeg codec buffers
                if(myCodecCtx != NULL && myCodec != NULL) {
                    avcodec_flush_buffers(myCodecCtx);
                }
                myOutQueue->clear();
                continue;
            }
            case StAVPacket::START_PACKET: {
                myOutQueue->clear();
                continue;
            }
            case StAVPacket::DATA_PACKET: {
                break;
            }
            case StAVPacket::LAST_PACKET: {
                continue; // ignore as avcodec_send_packet() is not used
            }
            case StAVPacket::END_PACKET: {
                if(toQuit) {
                    return;
                }
                continue;
            }
            case StAVPacket::QUIT_PACKET: {
                return;
            }
        }

        aPts      = unitsToSeconds(aPacket->getPts()) - myPtsStartBase;
        aDuration = unitsToSeconds(aPacket->getConvergenceDuration());
        if(myCodec != NULL) {
            // decode subtitle item
            avcodec_decode_subtitle2(myCodecCtx, &aSubtitle,
                                     &isFrameFinished, aPacket->getAVpkt());
            if(isFrameFinished != 0 && aPacket->getPts() != stAV::NOPTS_VALUE) {
                for(unsigned aRectId = 0; aRectId < aSubtitle.num_rects; ++aRectId) {
                    AVSubtitleRect* aRect = aSubtitle.rects[aRectId];
                    if(aRect == NULL) {
                        // should not happens
                        continue;
                    }

                    switch(aRect->type) {
                        case SUBTITLE_BITMAP: {
                            if(aDuration < 0.001) {
                                aDuration = 3.0; // duration is always zero here...
                            }

                            StHandle<StSubItem> aNewSubItem = new StSubItem(aPts, aPts + aDuration);
                            aNewSubItem->Image.initTrash(StImagePlane::ImgRGBA, aRect->w, aRect->h);
                            aNewSubItem->Scale = myImageScale;

                            uint8_t** anImgData = aRect->data;
                            int* anImgLineSizes = aRect->linesize;

                            SwsContext* aCtxToRgb = sws_getContext(aRect->w, aRect->h, stAV::PIX_FMT::PAL8,
                                                                   aRect->w, aRect->h, stAV::PIX_FMT::RGBA32,
                                                                   SWS_BICUBIC, NULL, NULL, NULL);
                            if(aCtxToRgb == NULL) {
                                break;
                            }

                            uint8_t* aDstData[4] = {
                                (uint8_t* )aNewSubItem->Image.getData(), NULL, NULL, NULL
                            };
                            /*const*/ int aDstLinesize[4] = {
                                (int )aNewSubItem->Image.getSizeRowBytes(), 0, 0, 0
                            };

                            sws_scale(aCtxToRgb,
                                      anImgData, anImgLineSizes,
                                      0, aRect->h,
                                      aDstData, aDstLinesize);
                            sws_freeContext(aCtxToRgb);

                            /*ST_DEBUG_LOG("  |" + aRectId + "/" + aSubtitle.num_rects + "| " //+ aRect->x + "x" + aRect->y + " WH= "
                                            + aRect->w + "x" + aRect->h + " c= " + aRect->nb_colors
                                            + " pts= " + aPts
                                            + " dur= " + aDuration);*/
                            myOutQueue->push(aNewSubItem);
                            break;
                        }
                        case SUBTITLE_TEXT: {
                            StHandle<StSubItem> aNewSubItem = new StSubItem(aPts, aPts + aDuration);
                            aNewSubItem->Text = aRect->text;
                            aNewSubItem->Text.replaceFast(ST_CRLF_REDUNDANT, ST_CRLF_REPLACEMENT); // remove redundant CR symbols
                            myOutQueue->push(aNewSubItem);
                            break;
                        }
                        case SUBTITLE_ASS: {
                            StString aTextData = aRect->ass;
                            StHandle<StSubItem> aNewSubItem = myASS.parseEvent(aTextData, aPts, aDuration);
                            if(!aNewSubItem.isNull()) {
                                myOutQueue->push(aNewSubItem);
                            }
                            break;
                        }
                        case SUBTITLE_NONE:
                        default:
                            break;
                    }
                }
            }
            avsubtitle_free(&aSubtitle);
        } else {
            // just plain text
            StHandle<StSubItem> aNewSubItem = new StSubItem(aPts, aPts + aDuration);
            aNewSubItem->Text = (const char* )aPacket->getData();
            aNewSubItem->Text.replaceFast(ST_CRLF_REDUNDANT, ST_CRLF_REPLACEMENT); // remove redundant CR symbols
            myOutQueue->push(aNewSubItem);
        }

        // and now packet finished
        aPacket.nullify();
    }
}
