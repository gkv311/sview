/**
 * Copyright © 2009-2012 Kirill Gavrilov <kirill@sview.ru>
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

#ifndef __StAudioQueue_h_
#define __StAudioQueue_h_

#include <StStrings/StString.h>
#include <StSettings/StFloat32Param.h>
#include <StThreads/StThreads.h>

#include "StAVPacketQueue.h"// StAVPacketQueue class
#include "StPCMBuffer.h"    // audio PCM buffer class
#include "StALContext.h"

// forward declarations
class StAudioQueue;

// define StHandle template specialization
ST_DEFINE_HANDLE(StAudioQueue, StAVPacketQueue);

/**
 * This is Audio playback class (OpenAL is used)
 * which feeded with packets (StAVPacket),
 * so it also implements StAVPacketQueue.
 */
class ST_LOCAL StAudioQueue : public StAVPacketQueue {

        public: //! @name public API

    StAudioQueue(const StString& theAlDeviceName);
    virtual ~StAudioQueue();

    bool isInDowntime() {
        return myDowntimeEvent.check();
    }

    /**
     * Initialization function.
     * @param theFormatCtx (AVFormatContext* )  - pointer to video format context;
     * @param theStreamId (const unsigned int ) - stream id in video format context;
     * @return true if no error.
     */
    virtual bool init(AVFormatContext*   theFormatCtx,
                      const unsigned int theStreamId);

    /**
     * Clean function.
     */
    virtual void deinit();

    /**
     * Main decoding and playback loop.
     * Give packets from queue, decode them and fill OpenAL buffers
     * for playback.
     */
    void decodeLoop();

    /**
     * @return true if audio is played.
     */
    bool stalIsAudioPlaying() {
        return (stalGetSourceState() == AL_PLAYING);
    }

        public: //!< @name playback control methods

    virtual void pushPlayEvent(const StPlayEvent_t theEventId,
                               const double        theSeekParam = 0.0);

    double getPts() const {
        myEventMutex.lock();
            if(!isPlaying()) {
                myPlaybackTimer.pause();
            }
            double aPts = myPlaybackTimer.getElapsedTimeInSec();
        myEventMutex.unlock();
        return aPts;
    }

    /**
     * Set audio gain.
     */
    void setAudioVolume(const float theGain) {
        myAlGain = theGain;
    }

    /**
     * Switch audio device.
     */
    void switchAudioDevice(const StString& theAlDeviceName) {
        myAlDeviceName = new StString(theAlDeviceName);
        myToSwitchDev  = true;
    }

        private: //! @name private methods

    bool stalInit();
    void stalDeinit();

    void stalConfigureSources1();
    void stalConfigureSources4_0();
    void stalConfigureSources5_1();

    bool stalQueue(const double thePts);

    /**
     * This function do fill OpenAL buffers.
     * @param pts (const double& ) - PTS for last decoded frame.
     */
    void stalFillBuffers(const double thePts,
                         const bool   toIgnoreEvents);

    void stalEmpty();

    ALenum stalGetSourceState();

    bool parseEvents();

    void decodePacket(const StHandle<StAVPacket>& thePacket,
                      double& thePts);

        private:

    void playTimerStart(const double thePts) {
        myEventMutex.lock();
            // timer operate within microseconds
            myPlaybackTimer.restart(thePts * 1000000.0);
        myEventMutex.unlock();
    }

    void playTimerPause() {
        myEventMutex.lock();
            myPlaybackTimer.pause();
        myEventMutex.unlock();
    }

    void playTimerResume() {
        myEventMutex.lock();
            myPlaybackTimer.resume();
        myEventMutex.unlock();
    }

        private: //! @name private fields

    // This constant sets count of OpenAL buffers, used in loop
    // for gapless playback
    #define NUM_AL_BUFFERS 4
    #define NUM_AL_SOURCES 8

    /**
     * This is help class, used to store buffer-sizes
     * that was decoded last.
     */
    class DataLoop {

            public:

        DataLoop()
        : myLast(NUM_AL_BUFFERS - 1) {
            stMemSet(myDataSizes, 0, sizeof(myDataSizes));
        }

        void clear() {
            stMemSet(myDataSizes, 0, sizeof(myDataSizes));
        }

        void push(const size_t theDataSize) {
            ++myLast;
            if(myLast >= NUM_AL_BUFFERS) {
               myLast = 0;
            }
            myDataSizes[myLast] = theDataSize;
        }

        size_t summ() const {
            size_t aSumm = 0;
            for(size_t aBuffIter = 0; aBuffIter < NUM_AL_BUFFERS; ++aBuffIter) {
                aSumm += myDataSizes[aBuffIter];
            }
            return aSumm;
        }

            private:

        size_t myDataSizes[NUM_AL_BUFFERS];
        size_t myLast;

    } myAlDataLoop;

    typedef enum {
        ST_AL_INIT_NA,
        ST_AL_INIT_OK,
        ST_AL_INIT_KO,
    } IState_t;

    StHandle<StThread> myThread;        //!< decoding loop thread
    mutable StTimer    myPlaybackTimer; //!< timer used for current PTS calculation
    StEvent            myDowntimeEvent;
    StPCMBuffer        myBufferSrc;     //!< decoded PCM audio buffer
    StPCMBuffer        myBufferOut;     //!< output  PCM audio buffer
    StTimer            myLimitTimer;
    volatile IState_t  myIsAlValid;     //!< OpenAL initialization state
    volatile bool      myToSwitchDev;   //!< switch audio device flag
    volatile bool      myToQuit;        //!< quiting flag

        private: //! @name OpenAL items

    StHandle<StString> myAlDeviceName;  //!< Output audio device name for OpenAL context initialization
    StALContext        myAlCtx;         //!< OpenAL context
    ALuint             myAlBuffers[NUM_AL_SOURCES][NUM_AL_BUFFERS]; //!< audio buffers
    ALuint             myAlSources[NUM_AL_SOURCES];                 //!< audio sources
    ALenum             myAlFormat;      //!< buffer data internal format
    ALenum             myPrevFormat;    //!< previous format (to correctly reinitialize AL buffer on change)
    ALsizei            myPrevFrequency; //!< previous audio frequency
    ALfloat            myAlGain;        //!< volume factor
    ALfloat            myAlGainPrev;    //!< volume factor (currently active)

        private: //! @name debug items

    ALint              myDbgPrevQueued;
    ALenum             myDbgPrevSrcState;

};

#endif //__StAudioQueue_h_
