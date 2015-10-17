/**
 * Copyright © 2009-2014 Kirill Gavrilov <kirill@sview.ru>
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

#include "StAVPacketQueue.h"

namespace {

    const StAVPacket ST_START_PACKET(NULL, StAVPacket::START_PACKET);
    const StAVPacket ST_END_PACKET  (NULL, StAVPacket::END_PACKET);
    const StAVPacket ST_FLUSH_PACKET(NULL, StAVPacket::FLUSH_PACKET);
    const StAVPacket ST_QUIT_PACKET (NULL, StAVPacket::QUIT_PACKET);

}

// auxiliary structure
struct StAVPacketQueue::QueueItem {

    StHandle<StAVPacket> myItem; //!< handle for packet
    QueueItem* myNext; //!< link to the next queue item

    ST_LOCAL QueueItem(const StAVPacket& thePacket)
    : myItem(new StAVPacket(thePacket)), // copy with content
      myNext(NULL) {}

};

StAVPacketQueue::StAVPacketQueue(const size_t theSizeLimit)
: myFormatCtx(NULL),
  myStream(NULL),
  myCodecCtx(NULL),
  myCodec(NULL),
  myCodecAuto(NULL),
  myGetFrmtInit(NULL),
  myGetBuffInit(NULL),
  myPtsStartBase(0.0),
  myPtsStartStream(0.0),
  myStreamId(-1),
  myToFlush(false),
  myToQuit(false),
  // playback control
  myEventMutex(),
  myPtsSeek(0.0),
  myPlayEvent(ST_PLAYEVENT_NONE),
  myIsPlaying(false),
  myIsAttachedPic(false),
  // queue
  myFront(NULL),
  myBack(NULL),
  mySize(0),
  mySizeLimit(theSizeLimit),
  mySizeSeconds(0.0),
  myMutex() {
    //
}

StAVPacketQueue::~StAVPacketQueue() {
    while(!isEmpty()) {
        pop();
    }
    deinit();
}

void StAVPacketQueue::clear() {
    myMutex.lock();
    while(!isEmpty()) {
        pop();
    }
    mySizeSeconds = 0.0;
    myMutex.unlock();
}

double StAVPacketQueue::detectPtsStartBase(const AVFormatContext* theFormatCtx) {
    if(theFormatCtx->nb_streams == 0) {
        return 0.0;
    }
    double aMinPts = 2.e+100;
    for(unsigned int aStreamId = 0; aStreamId < theFormatCtx->nb_streams; ++aStreamId) {
        AVStream* aStream = theFormatCtx->streams[aStreamId];
        aMinPts = stMin(aMinPts, stAV::unitsToSeconds(aStream, aStream->start_time));
    }
    return aMinPts;
}

bool StAVPacketQueue::init(AVFormatContext*   theFormatCtx,
                           const unsigned int theStreamId,
                           const StString&    theFileName) {
    myFileName       = theFileName;
    myFormatCtx      = theFormatCtx;
    myStream         = myFormatCtx->streams[theStreamId];
    myStreamId       = theStreamId;
    myCodecCtx       = myStream->codec;
    myPtsStartBase   = detectPtsStartBase(theFormatCtx);
    myPtsStartStream = unitsToSeconds(myStream->start_time);
    myGetFrmtInit    = myCodecCtx->get_format;
#if(LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(55, 0, 0))
    myGetBuffInit    = myCodecCtx->get_buffer2;
#endif
#if(LIBAVFORMAT_VERSION_INT >= AV_VERSION_INT(54, 2, 100))
    myIsAttachedPic = myStream != NULL
                  && (myStream->disposition & AV_DISPOSITION_ATTACHED_PIC) != 0;
#endif
    return true;
}

void StAVPacketQueue::deinit() {
    myFileName.clear();
    myFormatCtx = NULL;
    myStream    = NULL;
    if(myCodec != NULL && myCodecCtx != NULL) {
        avcodec_close(myCodecCtx);
        myCodecCtx->get_format  = myGetFrmtInit;
    #if(LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(55, 0, 0))
        myCodecCtx->get_buffer2 = myGetBuffInit;
    #endif
        fillCodecInfo(NULL);
    }
    myCodec       = NULL;
    myCodecAuto   = NULL;
    myCodecCtx    = NULL;
    myGetFrmtInit = NULL;
    myGetBuffInit = NULL;
    myStreamId    = -1;
    myIsAttachedPic = false;
}

void StAVPacketQueue::fillCodecInfo(AVCodec* theCodec) {
    StMutexAuto aLock(myMutexInfo);
    if(theCodec == NULL) {
        myCodecName.clear();
        myCodecDesc.clear();
        myCodecStr.clear();
    } else {
        myCodecName = theCodec->name;
        myCodecDesc = theCodec->long_name;
        myCodecStr  = StString("[") + myCodecName + stCString("] ") + myCodecDesc;
    }
}

void StAVPacketQueue::getCodecInfo(StString& theName,
                                   StString& theDesc) const {
    StMutexAuto aLock(myMutexInfo);
    theName = myCodecName;
    theDesc = myCodecDesc;
}

StString StAVPacketQueue::getCodecInfo() const {
    StMutexAuto aLock(myMutexInfo);
    const StString anInfo = myCodecStr;
    return anInfo;
}

StHandle<StAVPacket> StAVPacketQueue::pop() {
    myMutex.lock();
        if(isEmpty()) {
            myMutex.unlock();
            return StHandle<StAVPacket>();
        }
        QueueItem* anItem = myFront;
        myFront = myFront->myNext;
        StHandle<StAVPacket> aPacket = anItem->myItem;
        delete anItem;
        --mySize;
        mySizeSeconds -= aPacket->getDurationSeconds();
    myMutex.unlock();
    return aPacket;
}

void StAVPacketQueue::push(const StAVPacket& thePacket) {
    myMutex.lock();
        QueueItem* anItem = new QueueItem(thePacket);
        if(isEmpty()) {
            myFront = myBack = anItem;
        } else {
            myBack->myNext = anItem;
            myBack = anItem;
        }
        ++mySize;
        mySizeSeconds += thePacket.getDurationSeconds();
    myMutex.unlock();
}

void StAVPacketQueue::pushStart() {
    StAVPacketQueue::push(ST_START_PACKET);
}

void StAVPacketQueue::pushEnd() {
    StAVPacketQueue::push(ST_END_PACKET);
}

void StAVPacketQueue::pushQuit() {
    StAVPacketQueue::push(ST_QUIT_PACKET);
}

void StAVPacketQueue::pushFlush() {
    StAVPacketQueue::push(ST_FLUSH_PACKET);
    myToFlush = true;
}

void StAVPacketQueue::pushPlayEvent(const StPlayEvent_t theEventId,
                                    const double        theSeekParam) {
    myEventMutex.lock();
    switch(theEventId) {
        case ST_PLAYEVENT_PLAY: {
            myIsPlaying = true;
            break;
        }
        case ST_PLAYEVENT_RESUME: {
            if(myIsPlaying) {
                myEventMutex.unlock();
                return; // ignore duplicate messages
            }
            myIsPlaying = true;
            break;
        }
        case ST_PLAYEVENT_STOP: {
            myIsPlaying = false;
            break;
        }
        case ST_PLAYEVENT_PAUSE: {
            if(!myIsPlaying) {
                myEventMutex.unlock();
                return; // ignore duplicate messages
            }
            myIsPlaying = false;
            break;
        }
        case ST_PLAYEVENT_SEEK: {
            myPtsSeek = theSeekParam;
            break;
        }
        case ST_PLAYEVENT_RESET: {
            myIsPlaying = false;
            break;
        }
        case ST_PLAYEVENT_NEXT:
        case ST_PLAYEVENT_NONE: {
            break;
        }
    }
    myPlayEvent = theEventId;
    myEventMutex.unlock();
}
