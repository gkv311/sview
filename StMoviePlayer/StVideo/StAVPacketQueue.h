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

#ifndef __StAVPacketQueue_h_
#define __StAVPacketQueue_h_

#include <StThreads/StMutex.h>
#include <StTemplates/StHandle.h>
#include <StSlots/StSignal.h>

#include <StAV/StAVPacket.h>

typedef enum {
    ST_PLAYEVENT_NONE = 0,
    ST_PLAYEVENT_RESET,
    ST_PLAYEVENT_PLAY,
    ST_PLAYEVENT_STOP,
    ST_PLAYEVENT_PAUSE,
    ST_PLAYEVENT_RESUME,
    ST_PLAYEVENT_SEEK,
    ST_PLAYEVENT_NEXT,
} StPlayEvent_t;

/**
 * This is a simple thread safe queue implementation
 * specialized for AVPacketClass
 */
class StAVPacketQueue {

        public: //! @name Public API

    ST_LOCAL static double detectPtsStartBase(const AVFormatContext* theFormatCtx);

    /**
     * @param theSizeLimit (const size_t& ) - queue size limit.
     */
    ST_LOCAL StAVPacketQueue(const size_t theSizeLimit);

    ST_LOCAL virtual ~StAVPacketQueue();

    /**
     * Clean up the queue.
     */
    ST_LOCAL void clear();

    /**
     * Open stream.
     */
    ST_LOCAL virtual bool init(AVFormatContext*   theFormatCtx,
                               const unsigned int theStreamId,
                               const StString&    theFileName);

    /**
     * Close stream.
     */
    ST_LOCAL virtual void deinit();

    /**
     * @return packet (StAVPacket* ) - first packet in queue.
     */
    ST_LOCAL StHandle<StAVPacket> pop();

    /**
     * @param thePacket (StAVPacket& ) - packet to add (will be copied with content).
     * @return true on success.
     */
    ST_LOCAL void push(const StAVPacket& thePacket);

    ST_LOCAL void pushStart();
    ST_LOCAL void pushEnd();
    ST_LOCAL void pushQuit();
    ST_LOCAL void pushFlush();

    /**
     * Returns true if queue is empty.
     */
    ST_LOCAL bool isEmpty() const {
        myMutex.lock();
            bool aResult = myFront == NULL;
        myMutex.unlock();
        return aResult;
    }

    /**
     * Returns true if queue is full.
     */
    ST_LOCAL bool isFull() const {
        myMutex.lock();
            bool aResult = (mySize >= mySizeLimit) || (mySizeSeconds >= 5.0);
            //if(mySize >= mySizeLimit) { ST_DEBUG_LOG("stream" + streamId + " sizeSeconds= " + sizeSeconds + "; mySize= " + mySize); }
        myMutex.unlock();
        return aResult;
    }

    ST_LOCAL size_t getSize() const {
        myMutex.lock();
            size_t aSize = mySize;
        myMutex.unlock();
        return aSize;
    }

    ST_LOCAL size_t getSizeMax() const {
        myMutex.lock();
            size_t aSize = mySizeLimit;
        myMutex.unlock();
        return aSize;
    }

    /**
     * @return true if queue initialized.
     */
    ST_LOCAL bool isInitialized() const {
        return myStreamId >= 0;
    }

    ST_LOCAL bool isInContext(AVFormatContext* theFormatCtx) const {
        return myFormatCtx == theFormatCtx;
    }

    ST_LOCAL bool isInContext(AVFormatContext* theFormatCtx, signed int theStreamId) const {
        return (myFormatCtx == theFormatCtx) && (myStreamId == theStreamId);
    }

    /**
     * @return stream id in videofile or -1 if none
     */
    ST_LOCAL signed int getId() const {
        return myStreamId;
    }

    /**
     * @return format context
     */
    ST_LOCAL AVFormatContext* getContext() const {
        return myFormatCtx;
    }

    ST_LOCAL AVStream* getStream() {
        return myStream;
    }

    /**
     * Convert time units into seconds.
     */
    ST_LOCAL double unitsToSeconds(const int64_t theTimeUnits) const {
        return (myStream != NULL) ? stAV::unitsToSeconds(myStream, theTimeUnits) : 0.0;
    }

    /**
     * Update codec description.
     */
    ST_LOCAL void fillCodecInfo(AVCodec* theCodec);

    /**
     * Get codec description.
     */
    ST_LOCAL void getCodecInfo(StString& theName,
                               StString& theDesc) const;

    /**
     * Get codec description.
     */
    ST_LOCAL StString getCodecInfo() const;

    /**
     * @return file name
     */
    ST_LOCAL const StString& getFileName() const {
        return myFileName;
    }

        public: //! @name playback control methods

    /**
     * @return true if control in playback state.
     */
    ST_LOCAL bool isPlaying() const {
        myEventMutex.lock();
            bool aRes = myIsPlaying;
        myEventMutex.unlock();
        return aRes;
    }

    /**
     * @param theEventId (const StPlayEvent_t ) - event from enum;
     * @param theSeekParam (const double ) - additional parameter.
     */
    ST_LOCAL virtual void pushPlayEvent(const StPlayEvent_t theEventId,
                                        const double        theSeekParam = 0.0);

    /**
     * @return event in wait state
     */
    ST_LOCAL StPlayEvent_t popPlayEvent(double& theSeekPts) {
        myEventMutex.lock();
            StPlayEvent_t anEventId = myPlayEvent;
            theSeekPts = myPtsSeek;
            myPlayEvent = ST_PLAYEVENT_NONE;
        myEventMutex.unlock();
        return anEventId;
    }

    struct {
        /**
         * Emit callback Slot on error.
         * @param theUserData (const StString& ) - error description.
         */
        StSignal<void (const StCString& )> onError;
    } signals;

        protected: //! @name Fields should be full-controlled by heirs

    StString         myFileName;       //!< file name
    AVFormatContext* myFormatCtx;      //!< pointer to video context
    AVStream*        myStream;         //!< pointer to stream in video context
    AVCodecContext*  myCodecCtx;       //!< codec context
    AVCodec*         myCodec;          //!< codec
    AVCodec*         myCodecAuto;      //!< original codec (autodetected - before overriding)
    typedef PixelFormat (*aGetFrmt_t)(AVCodecContext* , const PixelFormat* );
    typedef int         (*aGetBuf2_t)(AVCodecContext* , AVFrame* frame, int );
    aGetFrmt_t       myGetFrmtInit;
    aGetBuf2_t       myGetBuffInit;
    double           myPtsStartBase;   //!< starting PTS in context
    double           myPtsStartStream; //!< starting PTS in the stream
    signed int       myStreamId;       //!< stream ID
    volatile bool    myToFlush;        //!< flag indicates FLUSH event was pushed in packets queue
    volatile bool    myToQuit;         //!< flag to terminate decoding loop

        protected: //! @name Playback control fields

    mutable StMutex  myEventMutex;     //!< lock for thread-safety
    double           myPtsSeek;        //!< seeking targert in seconds
    StPlayEvent_t    myPlayEvent;      //!< playback control event
    bool             myIsPlaying;      //!< playback state

        private: //! @name Private fields

    struct QueueItem;

    QueueItem*       myFront;          //!< queue front packet (first to pop)
    QueueItem*       myBack;           //!< queue back  packet (last  to pop)
    size_t           mySize;           //!< packets number in queue
    size_t           mySizeLimit;      //!< packets limit
    double           mySizeSeconds;    //!< cumulative packets length in seconds
    mutable StMutex  myMutex;          //!< lock for thread-safety

    StString         myCodecName;      //!< active codec name
    StString         myCodecDesc;      //!< active codec description
    StString         myCodecStr;       //!< active codec description
    mutable StMutex  myMutexInfo;      //!< lock for thread-safety

};

#endif //__StAVPacketQueue_h_
