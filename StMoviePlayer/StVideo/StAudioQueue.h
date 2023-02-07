/**
 * Copyright © 2009-2023 Kirill Gavrilov <kirill@sview.ru>
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
#include <StThreads/StCondition.h>
#include <StThreads/StTimer.h>

#include <StAV/StAVFrame.h>

#include "StAVPacketQueue.h"// StAVPacketQueue class
#include "StPCMBuffer.h"    // audio PCM buffer class
#include "StALContext.h"

// forward declarations
class StAudioQueue;
class StThread;

// define StHandle template specialization
ST_DEFINE_HANDLE(StAudioQueue, StAVPacketQueue);

/**
 * This is Audio playback class (OpenAL is used)
 * which feed with packets (StAVPacket),
 * so it also implements StAVPacketQueue.
 */
class StAudioQueue : public StAVPacketQueue {

        public:

    enum StAlHintHrtf {
        StAlHintHrtf_Auto     = 0,
        StAlHintHrtf_ForceOn  = 1,
        StAlHintHrtf_ForceOff = 2,
    };

    enum StAlHintOutput {
        StAlHintOutput_Any         = 0, // ALC_ANY_SOFT
        StAlHintOutput_Mono        = 1, // ALC_MONO_SOFT
        StAlHintOutput_Stereo      = 2, // ALC_STEREO_SOFT
        StAlHintOutput_StereoBasic = 3, // ALC_STEREO_BASIC_SOFT
        StAlHintOutput_StereoUHJ   = 4, // ALC_STEREO_UHJ_SOFT
        StAlHintOutput_StereoHRTF  = 5, // ALC_STEREO_HRTF_SOFT
        StAlHintOutput_Quad        = 6, // ALC_QUAD_SOFT
        StAlHintOutput_Surround51  = 7, // ALC_SURROUND_5_1_SOFT
        StAlHintOutput_Surround61  = 8, // ALC_SURROUND_6_1_SOFT
        StAlHintOutput_Surround71  = 9, // ALC_SURROUND_7_1_SOFT
    };

        public: //! @name public API

    ST_LOCAL StAudioQueue(const std::string& theAlDeviceName,
                          StAudioQueue::StAlHintOutput theAlOut,
                          StAudioQueue::StAlHintHrtf theAlHrtf);
    ST_LOCAL virtual ~StAudioQueue();

    ST_LOCAL bool isInDowntime() {
        return myDowntimeEvent.check();
    }

    /**
     * Return codec type.
     */
    ST_LOCAL virtual AVMediaType getCodecType() const ST_ATTR_OVERRIDE { return AVMEDIA_TYPE_AUDIO; }

    /**
     * Initialization function.
     * @param theFormatCtx pointer to video format context
     * @param theStreamId  stream id in video format context
     * @return true if no error
     */
    ST_LOCAL bool init(AVFormatContext*   theFormatCtx,
                       const unsigned int theStreamId,
                       const StString&    theFileName);

    /**
     * Clean function.
     */
    ST_LOCAL virtual void deinit() ST_ATTR_OVERRIDE;

    /**
     * Main decoding and playback loop.
     * Give packets from queue, decode them and fill OpenAL buffers
     * for playback.
     */
    ST_LOCAL void decodeLoop();

    /**
     * @return true if audio is played.
     */
    ST_LOCAL bool stalIsAudioPlaying() {
        return (stalGetSourceState() == AL_PLAYING);
    }

        public: //!< @name playback control methods

    ST_LOCAL virtual void pushPlayEvent(const StPlayEvent_t theEventId,
                                        const double        theSeekParam = 0.0) ST_ATTR_OVERRIDE;

    ST_LOCAL double getPts() const {
        myEventMutex.lock();
        if(!isPlaying()) {
            myPlaybackTimer.pause();
        }
        const double aPts = isInitialized() ? myPlaybackTimer.getElapsedTimeInSec() : -1.0;
        myEventMutex.unlock();
        return aPts;
    }

    /**
     * Setup OpenAL mixing hints.
     */
    ST_LOCAL void setAlHints(StAlHintOutput theAlOut,
                             StAlHintHrtf theAlHrt) {
        myAlHintOut  = theAlOut;
        myAlHintHrtf = theAlHrt;
    }

    /**
     * Set audio gain.
     */
    ST_LOCAL void setAudioVolume(const float theGain) {
        myAlGain = theGain;
    }

    /**
     * Set head orientation.
     */
    ST_LOCAL void setHeadOrientation(const StGLQuaternion& theOrient, bool theToTrack) {
        if(!theToTrack) {
            myToOrientListener = false;
            return;
        }

        StMutexAuto aLock(mySwitchMutex);
        myHeadOrient = theOrient;
        myToOrientListener = true;
    }

    /**
     * Turn ON/OFF listener head orientation tracking.
     */
    ST_LOCAL void setTrackHeadOrientation(bool theToTrack) {
        myToOrientListener = theToTrack;
    }

    /**
     * Set forcing B-Format.
     */
    ST_LOCAL void setForceBFormat(bool theToForce) {
        myToForceBFormat = theToForce;
    }

    /**
     * Switch audio device.
     */
    ST_LOCAL void switchAudioDevice(const std::string& theAlDeviceName) {
        StMutexAuto aLock(mySwitchMutex);
        myAlDeviceName = theAlDeviceName;
        myToSwitchDev  = true;
    }

    /**
     * Return TRUE if OpenAL implementation supports output mode hints.
     */
    ST_LOCAL bool hasAlHintOutput() const { return myAlCtx.hasExtSoftOutMode; }

    /**
     * Return TRUE if OpenAL implementation provides HRTF mixing feature.
     */
    ST_LOCAL bool hasAlHintHrtf() const { return myAlCtx.hasExtSoftHrtf; }

    /**
     * @return true if device was disconnected and OpenAL should be re-initialized
     */
    ST_LOCAL bool isDisconnected() const { return myIsDisconnected; }

    /**
     * Return OpenAL info.
     */
    ST_LOCAL void getAlInfo(StDictionary& theInfo) {
        StMutexAuto aLock(myAlInfoMutex);
        for(size_t aPairIter = 0; aPairIter < myAlInfo.size(); ++aPairIter) {
            theInfo.add(myAlInfo.getFromIndex(aPairIter));
        }
    }

        private: //! @name private methods

    ST_LOCAL bool initBuffers();

    ST_LOCAL bool stalInit();
    ST_LOCAL void stalDeinit();
    ST_LOCAL void stalReinitialize();
    ST_LOCAL void stalResetAlHints();
    ST_LOCAL void stalOrientListener();

    ST_LOCAL void stalConfigureSources1();
    ST_LOCAL void stalConfigureSources2_0();
    ST_LOCAL void stalConfigureSources3_0();
    ST_LOCAL void stalConfigureSources4_0();
    ST_LOCAL void stalConfigureSources5_0();
    ST_LOCAL void stalConfigureSources5_1();
    ST_LOCAL void stalConfigureSources7_1();

    ST_LOCAL bool stalQueue(const double thePts);

    /**
     * This function do fill OpenAL buffers.
     * @param thePts PTS for last decoded frame
     */
    ST_LOCAL void stalFillBuffers(const double thePts,
                                  const bool   toIgnoreEvents);

    ST_LOCAL void stalEmpty();

    ST_LOCAL ALenum stalGetSourceState();

    ST_LOCAL bool parseEvents();

    ST_LOCAL void decodePacket(const StHandle<StAVPacket>& thePacket,
                               double& thePts);

        private:

    //! Setup output format for mono source.
    ST_LOCAL bool setupOutMonoFormat();

    //! Setup output format and channels.
    ST_LOCAL bool initOutChannels();

    //! Initialize 1-channel stream.
    ST_LOCAL bool initOutMono();

    //! Initialize 2-channels stream.
    ST_LOCAL bool initOutStereo(const bool theIsPlanar);

    //! Initialize 2.0 stream by configuring 2 sources in 3D.
    ST_LOCAL bool initOut20Soft(const bool theIsPlanar);

    //! Initialize 3.0 stream by configuring 3 sources in 3D.
    ST_LOCAL bool initOut30Soft(const bool theIsPlanar);

    //! Initialize 4.0 stream by configuring 4 sources in 3D.
    ST_LOCAL bool initOut40Soft(const bool theIsPlanar);

    //! Initialize 4.0 stream using extension (AL_FORMAT_QUAD).
    ST_LOCAL bool initOut40Ext(const bool theIsPlanar);

    //! Initialize 4.0 Ambisonics WXYZ stream using extension (AL_FORMAT_BFORMAT3D).
    ST_LOCAL bool initOut40BFormat(const bool theIsPlanar);

    //! Initialize 5.0 stream by configuring 5 sources in 3D.
    ST_LOCAL bool initOut50Soft(const bool theIsPlanar);

    //! Initialize 5.1 stream by configuring 6 sources in 3D.
    ST_LOCAL bool initOut51Soft(const bool theIsPlanar);

    //! Initialize 5.1 stream using extension (AL_FORMAT_51CHN).
    ST_LOCAL bool initOut51Ext(const bool theIsPlanar);

    //! Initialize 7.1 stream by configuring 8 sources in 3D.
    ST_LOCAL bool initOut71Soft(const bool theIsPlanar);

    //! Initialize 7.1 stream using extension (AL_FORMAT_71CHN).
    ST_LOCAL bool initOut71Ext(const bool theIsPlanar);

        private:

    ST_LOCAL void playTimerStart(const double thePts) {
        myEventMutex.lock();
            // timer operate within microseconds
            myPlaybackTimer.restart(thePts * 1000000.0);
        myEventMutex.unlock();
    }

    ST_LOCAL void playTimerPause() {
        myEventMutex.lock();
            myPlaybackTimer.pause();
        myEventMutex.unlock();
    }

    ST_LOCAL void playTimerResume() {
        myEventMutex.lock();
            myPlaybackTimer.resume();
        myEventMutex.unlock();
    }

    ST_LOCAL bool stalCheckConnected();

        private: //! @name private fields

    // This constant sets count of OpenAL buffers, used in loop
    // for gapless playback
    #define THE_NUM_AL_BUFFERS 4
    #define THE_NUM_AL_SOURCES 8

    /**
     * This is help class, used to store buffer-sizes
     * that was decoded last.
     */
    class DataLoop {

            public:

        ST_LOCAL DataLoop()
        : myLast(THE_NUM_AL_BUFFERS - 1) {
            stMemSet(myDataSizes, 0, sizeof(myDataSizes));
        }

        ST_LOCAL void clear() {
            stMemSet(myDataSizes, 0, sizeof(myDataSizes));
        }

        ST_LOCAL void push(const size_t theDataSize) {
            ++myLast;
            if(myLast >= THE_NUM_AL_BUFFERS) {
               myLast = 0;
            }
            myDataSizes[myLast] = theDataSize;
        }

        ST_LOCAL size_t summ() const {
            size_t aSumm = 0;
            for(size_t aBuffIter = 0; aBuffIter < THE_NUM_AL_BUFFERS; ++aBuffIter) {
                aSumm += myDataSizes[aBuffIter];
            }
            return aSumm;
        }

            private:

        size_t myDataSizes[THE_NUM_AL_BUFFERS];
        size_t myLast;

    } myAlDataLoop;

    typedef enum {
        ST_AL_INIT_NA,
        ST_AL_INIT_OK,
        ST_AL_INIT_KO,
    } IState_t;

    StHandle<StThread> myThread;        //!< decoding loop thread
    mutable StTimer    myPlaybackTimer; //!< timer used for current PTS calculation
    StCondition        myDowntimeEvent;
    StAVFrame          myFrame;         //!< decoded audio frame
    int                myAvSrcFormat;   //!< myCodecCtx->sample_fmt
    int                myAvSampleRate;  //!< myCodecCtx->sample_rate
    int                myAvNbChannels;  //!< myCodecCtx->channels
    StPCMBuffer        myBufferSrc;     //!< decoded PCM audio buffer
    StPCMBuffer        myBufferOut;     //!< output  PCM audio buffer
    StTimer            myLimitTimer;
    volatile IState_t  myIsAlValid;     //!< OpenAL initialization state
    StMutex            mySwitchMutex;   //!< switch audio device lock
    volatile bool      myToSwitchDev;   //!< switch audio device flag
    volatile bool      myIsDisconnected;//!< audio device disconnection flag
    volatile bool      myToOrientListener; //!< track listener orientation
    volatile bool      myToForceBFormat;//!< force using B-Format for any 4-channels input
    StGLQuaternion     myHeadOrient;    //!< head orientation

        private: //! @name OpenAL items

    std::string        myAlDeviceName;  //!< Output audio device name for OpenAL context initialization
    StDictionary       myAlInfo;        //!< OpenAL info
    StMutex            myAlInfoMutex;
    StALContext        myAlCtx;         //!< OpenAL context
    ALuint             myAlBuffers[THE_NUM_AL_SOURCES][THE_NUM_AL_BUFFERS]; //!< audio buffers
    ALuint             myAlSources[THE_NUM_AL_SOURCES];                     //!< audio sources
    ALenum             myAlFormat;      //!< buffer data internal format
    ALenum             myPrevFormat;    //!< previous format (to correctly reinitialize AL buffer on change)
    ALsizei            myPrevFrequency; //!< previous audio frequency
    ALfloat            myAlGain;        //!< volume factor
    ALfloat            myAlGainPrev;    //!< volume factor (currently active)
    bool               myAlSoftLayout;  //!< flag indicating soft multichannel layout
    bool               myAlIsListOrient;//!< flag indicating that listener orientation is not identity
    bool               myAlCanBFormat;  //!< flag indicating that B-Format can be forced (e.g. 4-channels input and extension is available)
    bool               myAlIsBFormat;   //!< flag indicating that using B-Format is enabled (forcibly) for 4-channels input

    StAlHintOutput     myAlHintOut;
    StAlHintOutput     myAlHintOutPrev;

    StAlHintHrtf       myAlHintHrtf;
    StAlHintHrtf       myAlHintHrtfPrev;

        private: //! @name debug items

    ALint              myDbgPrevQueued;
    ALenum             myDbgPrevSrcState;

};

#endif //__StAudioQueue_h_
