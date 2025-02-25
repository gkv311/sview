/**
 * Copyright © 2009-2025 Kirill Gavrilov <kirill@sview.ru>
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

#include "StAudioQueue.h"

#include <StGL/StGLVec.h>
#include <StThreads/StThread.h>

namespace {

    /**
     * Check OpenAL state.
     */
    bool stalCheckErrors(const StString& ST_DEBUG_VAR(theProcedure)) {
        ALenum anError = alGetError();
        switch(anError) {
            case AL_NO_ERROR: return true; // alright
            case AL_INVALID_NAME:      ST_DEBUG_LOG(theProcedure + ": AL_INVALID_NAME");      return false;
            case AL_INVALID_ENUM:      ST_DEBUG_LOG(theProcedure + ": AL_INVALID_ENUM");      return false;
            case AL_INVALID_VALUE:     ST_DEBUG_LOG(theProcedure + ": AL_INVALID_VALUE");     return false;
            case AL_INVALID_OPERATION: ST_DEBUG_LOG(theProcedure + ": AL_INVALID_OPERATION"); return false;
            case AL_OUT_OF_MEMORY:     ST_DEBUG_LOG(theProcedure + ": AL_OUT_OF_MEMORY");     return false;
            default:                   ST_DEBUG_LOG(theProcedure + ": OpenAL unknown error"); return false;
        }
    }

    static const StGLVec3 THE_POSITION_LEFT         (-1.0f, 0.0f,  0.0f);
    static const StGLVec3 THE_POSITION_RIGHT        ( 1.0f, 0.0f,  0.0f);

    static const StGLVec3 THE_POSITION_CENTER       ( 0.0f, 0.0f,  0.0f);
    static const StGLVec3 THE_POSITION_FRONT_LEFT   (-1.0f, 0.0f, -1.0f);
    static const StGLVec3 THE_POSITION_FRONT_CENTER ( 0.0f, 0.0f, -1.0f);
    static const StGLVec3 THE_POSITION_FRONT_RIGHT  ( 1.0f, 0.0f, -1.0f);
    static const StGLVec3 THE_POSITION_LFE          ( 0.0f, 0.0f,  0.0f);
    static const StGLVec3 THE_POSITION_REAR_LEFT    (-1.0f, 0.0f,  1.0f);
    static const StGLVec3 THE_POSITION_REAR_RIGHT   ( 1.0f, 0.0f,  1.0f);

    static const StGLVec3 THE_POSITION_REAR_LEFT71  (-1.0f, 0.0f,  1.0f);
    static const StGLVec3 THE_POSITION_REAR_RIGHT71 ( 1.0f, 0.0f,  1.0f);
    static const StGLVec3 THE_POSITION_SIDE_LEFT71  (-1.0f, 0.0f,  0.0f);
    static const StGLVec3 THE_POSITION_SIDE_RIGHT71 ( 1.0f, 0.0f,  0.0f);

    static const StGLVec3 THE_LISTENER_FORWARD      ( 0.0f, 0.0f, -1.0f);
    static const StGLVec3 THE_LISTENER_UP           ( 0.0f, 1.0f,  0.0f);

}

void StAudioQueue::stalConfigureSources1() {
    alSourcefv(myAlSources[0], AL_POSITION, THE_POSITION_CENTER);
    stalCheckErrors("alSource*1.0");
}

void StAudioQueue::stalConfigureSources2_0() {
    alSourcefv(myAlSources[0], AL_POSITION, THE_POSITION_LEFT);
    alSourcefv(myAlSources[1], AL_POSITION, THE_POSITION_RIGHT);
    stalCheckErrors("alSource*2.0");
}

void StAudioQueue::stalConfigureSources3_0() {
    alSourcefv(myAlSources[0], AL_POSITION, THE_POSITION_LEFT);
    alSourcefv(myAlSources[1], AL_POSITION, THE_POSITION_RIGHT);
    alSourcefv(myAlSources[2], AL_POSITION, THE_POSITION_CENTER);
    stalCheckErrors("alSource*3.0");
}

void StAudioQueue::stalConfigureSources4_0() {
    alSourcefv(myAlSources[0], AL_POSITION, THE_POSITION_FRONT_LEFT);
    alSourcefv(myAlSources[1], AL_POSITION, THE_POSITION_FRONT_RIGHT);
    alSourcefv(myAlSources[2], AL_POSITION, THE_POSITION_REAR_LEFT);
    alSourcefv(myAlSources[3], AL_POSITION, THE_POSITION_REAR_RIGHT);
    stalCheckErrors("alSource*4.0");
}

void StAudioQueue::stalConfigureSources5_0() {
    alSourcefv(myAlSources[0], AL_POSITION, THE_POSITION_FRONT_LEFT);
    alSourcefv(myAlSources[1], AL_POSITION, THE_POSITION_FRONT_RIGHT);
    alSourcefv(myAlSources[2], AL_POSITION, THE_POSITION_FRONT_CENTER);
    alSourcefv(myAlSources[3], AL_POSITION, THE_POSITION_REAR_LEFT);
    alSourcefv(myAlSources[4], AL_POSITION, THE_POSITION_REAR_RIGHT);
    stalCheckErrors("alSource*5.0");
}

void StAudioQueue::stalConfigureSources5_1() {
    alSourcefv(myAlSources[0], AL_POSITION, THE_POSITION_FRONT_LEFT);
    alSourcefv(myAlSources[1], AL_POSITION, THE_POSITION_FRONT_RIGHT);
    alSourcefv(myAlSources[2], AL_POSITION, THE_POSITION_FRONT_CENTER);
    alSourcefv(myAlSources[3], AL_POSITION, THE_POSITION_LFE);
    alSourcefv(myAlSources[4], AL_POSITION, THE_POSITION_REAR_LEFT);
    alSourcefv(myAlSources[5], AL_POSITION, THE_POSITION_REAR_RIGHT);
    stalCheckErrors("alSource*5.1");
}

void StAudioQueue::stalConfigureSources7_1() {
    alSourcefv(myAlSources[0], AL_POSITION, THE_POSITION_FRONT_LEFT);
    alSourcefv(myAlSources[1], AL_POSITION, THE_POSITION_FRONT_RIGHT);
    alSourcefv(myAlSources[2], AL_POSITION, THE_POSITION_FRONT_CENTER);
    alSourcefv(myAlSources[3], AL_POSITION, THE_POSITION_LFE);
    alSourcefv(myAlSources[4], AL_POSITION, THE_POSITION_REAR_LEFT71);
    alSourcefv(myAlSources[5], AL_POSITION, THE_POSITION_REAR_RIGHT71);
    alSourcefv(myAlSources[6], AL_POSITION, THE_POSITION_SIDE_LEFT71);
    alSourcefv(myAlSources[7], AL_POSITION, THE_POSITION_SIDE_RIGHT71);
    stalCheckErrors("alSource*7.1");
}

bool StAudioQueue::stalInit() {
    std::string aDevName;
    {
        StMutexAuto aLock(mySwitchMutex);
        aDevName = myAlDeviceName;
    }
    if(!myAlCtx.create(aDevName)) {
        if(aDevName.empty()
        || !myAlCtx.create("")) {
            // retry with default device
            return false;
        }
    }
    myAlCtx.makeCurrent();
    stalResetAlHints();
    {
        StMutexAuto aLock(myAlInfoMutex);
        myAlInfo.clear();
        myAlCtx.fullInfo(myAlInfo);
    }

    alGetError(); // clear error code

    // generate the buffers
    for(size_t aSrcId = 0; aSrcId < THE_NUM_AL_SOURCES; ++aSrcId) {
        alGenBuffers(THE_NUM_AL_BUFFERS, &myAlBuffers[aSrcId][0]);
        stalCheckErrors(StString("alGenBuffers") + aSrcId);
    }

    // generate the sources
    alGenSources(THE_NUM_AL_SOURCES, myAlSources);
    stalCheckErrors("alGenSources");

    // configure sources
    const StGLVec3 aZeroVec(0.0f);
    for(size_t aSrcId = 0; aSrcId < THE_NUM_AL_SOURCES; ++aSrcId) {
        alSourcefv(myAlSources[aSrcId], AL_POSITION,        aZeroVec);
        alSourcefv(myAlSources[aSrcId], AL_VELOCITY,        aZeroVec);
        alSourcefv(myAlSources[aSrcId], AL_DIRECTION,       aZeroVec);
        alSourcef (myAlSources[aSrcId], AL_ROLLOFF_FACTOR,  0.0f);
        //alSourcei(myAlSources[aSrcId], AL_SOURCE_RELATIVE, myToOrientListener ? AL_FALSE : AL_TRUE);
        alSourcei (myAlSources[aSrcId], AL_SOURCE_RELATIVE, AL_FALSE);
        alSourcef (myAlSources[aSrcId], AL_GAIN,            1.0f);
        stalCheckErrors(StString("alSource*") + aSrcId);
    }

    // configure listener
    alListenerfv(AL_POSITION, aZeroVec);
    alListenerfv(AL_VELOCITY, aZeroVec);
    alListenerf (AL_GAIN,     myAlGain); // apply gain to all sources at-once
    stalOrientListener();
    return true;
}

void StAudioQueue::stalDeinit() {
    // clear buffers
    stalEmpty();
    alSourceStopv(THE_NUM_AL_SOURCES, myAlSources);

    alDeleteSources(THE_NUM_AL_SOURCES, myAlSources);
    stalCheckErrors("alDeleteSources");

    for(size_t aSrcId = 0; aSrcId < THE_NUM_AL_SOURCES; ++aSrcId) {
        alDeleteBuffers(THE_NUM_AL_BUFFERS, &myAlBuffers[aSrcId][0]);
        stalCheckErrors(StString("alDeleteBuffers") + aSrcId);
    }

    // close device
    myAlCtx.destroy();
}

void StAudioQueue::stalReinitialize() {
    stalDeinit(); // release OpenAL context
    myIsAlValid = (stalInit() ? ST_AL_INIT_OK : ST_AL_INIT_KO);

    myIsDisconnected = false;
    myToSwitchDev    = false;
}

void StAudioQueue::stalResetAlHints() {
    const bool wasChanged = myAlHintOut  != myAlHintOutPrev
                         || myAlHintHrtf != myAlHintHrtfPrev;
    myAlHintOutPrev  = myAlHintOut;
    myAlHintHrtfPrev = myAlHintHrtf;
    if(!myAlCtx.hasExtSoftHrtf && !myAlCtx.hasExtSoftOutMode) {
        return;
    }

    ALCint anAttrs[6] = {
        0, 0,
        0, 0,
        0, 0,
    };

    int anAttrIter = 0;
    if(myAlCtx.hasExtSoftOutMode) {
        ALCint anOutVal = ALC_ANY_SOFT;
        switch(myAlHintOut) {
            case StAlHintOutput_Any:         anOutVal = ALC_ANY_SOFT; break;
            case StAlHintOutput_Mono:        anOutVal = ALC_MONO_SOFT; break;
            case StAlHintOutput_Stereo:      anOutVal = ALC_STEREO_SOFT; break;
            case StAlHintOutput_StereoBasic: anOutVal = ALC_STEREO_BASIC_SOFT; break;
            case StAlHintOutput_StereoUHJ:   anOutVal = ALC_STEREO_UHJ_SOFT; break;
            case StAlHintOutput_StereoHRTF:  anOutVal = ALC_STEREO_HRTF_SOFT; break;
            case StAlHintOutput_Quad:        anOutVal = ALC_QUAD_SOFT; break;
            case StAlHintOutput_Surround51:  anOutVal = ALC_SURROUND_5_1_SOFT; break;
            case StAlHintOutput_Surround61:  anOutVal = ALC_SURROUND_6_1_SOFT; break;
            case StAlHintOutput_Surround71:  anOutVal = ALC_SURROUND_7_1_SOFT; break;
        }
        anAttrs[anAttrIter * 2 + 0] = ALC_OUTPUT_MODE_SOFT;
        anAttrs[anAttrIter * 2 + 1] = anOutVal;
        ++anAttrIter;
    }
    if(myAlCtx.hasExtSoftHrtf && (!myAlCtx.hasExtSoftOutMode || myAlHintHrtf != StAlHintHrtf_Auto)) {
        ALCint aHrtfVal = ALC_DONT_CARE_SOFT;
        if(myAlHintHrtf == StAlHintHrtf_ForceOn) {
            aHrtfVal = ALC_TRUE;
        } else if(myAlHintHrtf == StAlHintHrtf_ForceOff) {
            aHrtfVal = ALC_FALSE;
        }
        anAttrs[anAttrIter * 2 + 0] = ALC_HRTF_SOFT;
        anAttrs[anAttrIter * 2 + 1] = aHrtfVal;
        ++anAttrIter;
    }

    myAlCtx.alcResetDeviceSOFT(myAlCtx.getAlDevice(), anAttrs);
    ST_DEBUG_LOG(myAlCtx.toStringExtensions())
    if(wasChanged) {
        (void )wasChanged;
        //ST_DEBUG_LOG(myAlCtx.toStringExtensions())
    }
}

void StAudioQueue::stalOrientListener() {
    if(myToOrientListener) {
        StGLQuaternion aHeadOrient;
        {
            StMutexAuto aLock(mySwitchMutex);
            aHeadOrient = myHeadOrient;
            aHeadOrient.reverse();
            aHeadOrient.normalize();
        }

        const StGLVec3 aFwdVec = aHeadOrient.multiply(THE_LISTENER_FORWARD);
        const StGLVec3 anUpVec = aHeadOrient.multiply(THE_LISTENER_UP);
        const StGLVec3 aListenerOri[2] = { aFwdVec, anUpVec };

        //static StGLVec3 aPrev; if(aFwdVec != aPrev) { aPrev = aFwdVec; ST_DEBUG_LOG("FWD: " + aFwdVec.toString() + "\n[UP: " + anUpVec.toString() + "]\n") }

        alListenerfv(AL_ORIENTATION, (const ALfloat* )aListenerOri);
        myAlIsListOrient = true;
    } else if(myAlIsListOrient) {
        const StGLVec3 aListenerOri[2] = { THE_LISTENER_FORWARD, THE_LISTENER_UP };
        alListenerfv(AL_ORIENTATION, (const ALfloat* )aListenerOri);
        myAlIsListOrient = false;
    }
}

void StAudioQueue::stalEmpty() {
    alSourceStopv(THE_NUM_AL_SOURCES, myAlSources);

    ALint aBufQueued = 0;
    ALuint alBuffIdToUnqueue = 0;
    for(size_t aSrcId = 0; aSrcId < THE_NUM_AL_SOURCES; ++aSrcId) {
        alGetSourcei(myAlSources[aSrcId], AL_BUFFERS_QUEUED, &aBufQueued);
        for(ALint aBufIter = 0; aBufIter < aBufQueued; ++aBufIter) {
            alSourceUnqueueBuffers(myAlSources[aSrcId], 1, &alBuffIdToUnqueue);
            stalCheckErrors(StString("alSourceUnqueueBuffers") + aSrcId);
        }
        alSourcei(myAlSources[aSrcId], AL_BUFFER, 0);
    }
    ///alSourceRewindv(THE_NUM_AL_SOURCES, myAlSources);
}

ALenum StAudioQueue::stalGetSourceState() {
    ALenum aState;
    alGetSourcei(myAlSources[0], AL_SOURCE_STATE, &aState);
    if(myDbgPrevSrcState != aState) {
        switch(aState) {
            case AL_INITIAL: ST_DEBUG_LOG("OpenAL source state: INITIAL"); break;
            case AL_PLAYING: ST_DEBUG_LOG("OpenAL source state: PLAYING"); break;
            case AL_PAUSED:  ST_DEBUG_LOG("OpenAL source state: PAUSED") ; break;
            case AL_STOPPED: ST_DEBUG_LOG("OpenAL source state: STOPPED"); break;
            default:         ST_DEBUG_LOG("OpenAL source state: UNKNOWN"); break;
        }
    }
    myDbgPrevSrcState = aState;
    return aState;
}

/**
 * Simple thread function which just call decodeLoop().
 */
static SV_THREAD_FUNCTION threadFunction(void* audioQueue) {
    StAudioQueue* stAudioQueue = (StAudioQueue* )audioQueue;
    stAudioQueue->decodeLoop();
    return SV_THREAD_RETURN 0;
}

StAudioQueue::StAudioQueue(const std::string& theAlDeviceName,
                           StAudioQueue::StAlHintOutput theAlOut,
                           StAudioQueue::StAlHintHrtf theAlHrtf)
: StAVPacketQueue(512),
  myPlaybackTimer(false),
  myDowntimeEvent(true),
  myAvSrcFormat(-1),
  myAvSampleRate(-1),
  myAvNbChannels(-1),
  myBufferSrc(StPcmFormat_Int16),
  myBufferOut(StPcmFormat_Int16),
  myIsAlValid(ST_AL_INIT_NA),
  myToSwitchDev(false),
  myIsDisconnected(false),
  myToOrientListener(false),
  myToForceBFormat(false),
  myAlDeviceName(theAlDeviceName),
  myAlFormat(AL_FORMAT_STEREO16),
  myPrevFormat(AL_FORMAT_STEREO16),
  myPrevFrequency(0),
  myAlGain(1.0f),
  myAlGainPrev(1.0f),
  myAlSoftLayout(true),
  myAlIsListOrient(false),
  myAlCanBFormat(false),
  myAlIsBFormat(false),
  myAlHintOut(theAlOut),
  myAlHintOutPrev(theAlOut),
  myAlHintHrtf(theAlHrtf),
  myAlHintHrtfPrev(theAlHrtf),
  myDbgPrevQueued(-1),
  myDbgPrevSrcState(-1) {
    stMemSet(myAlSources, 0, sizeof(myAlSources));

    // launch thread parse incoming packets from queue
    myThread = new StThread(threadFunction, (void* )this, "StAudioQueue");
}

StAudioQueue::~StAudioQueue() {
    myToQuit = true;
    pushQuit();

    myThread->wait();
    myThread.nullify();

    deinit();
}

bool StAudioQueue::setupOutMonoFormat() {
    switch(myBufferSrc.getFormat()) {
        case StPcmFormat_Float64: {
            if(myAlCtx.hasExtFloat64) {
                // use float64 extension
                myAlFormat = alGetEnumValue("AL_FORMAT_MONO_DOUBLE_EXT");
                myBufferOut.setFormat(StPcmFormat_Float64);
                return true;
            }
        }
        ST_FALLTHROUGH
        case StPcmFormat_Int32:
        case StPcmFormat_Float32: {
            if(myAlCtx.hasExtFloat32) {
                // use float32 extension to preserve more quality
                myAlFormat = alGetEnumValue("AL_FORMAT_MONO_FLOAT32");
                myBufferOut.setFormat(StPcmFormat_Float32);
                return true;
            }
        }
        ST_FALLTHROUGH
        case StPcmFormat_Int16: {
            // default - int16_t
            myAlFormat = AL_FORMAT_MONO16;
            myBufferOut.setFormat(StPcmFormat_Int16);
            return true;
        }
        case StPcmFormat_UInt8: {
            myAlFormat = AL_FORMAT_MONO8;
            myBufferOut.setFormat(StPcmFormat_UInt8);
            return true;
        }
    }
    return false;
}

bool StAudioQueue::initOutMono() {
    if(!setupOutMonoFormat()) {
        return false;
    }

    myBufferSrc.setupChannels(StChannelMap::CH10, StChannelMap::PCM, 1);
    myBufferOut.setupChannels(StChannelMap::CH10, StChannelMap::PCM, 1);
    stalConfigureSources1();
    return true;
}

bool StAudioQueue::initOut20Soft(const bool theIsPlanar) {
    if(!setupOutMonoFormat()) {
        return false;
    }

    myBufferOut.setupChannels(StChannelMap::CH20, StChannelMap::PCM, 2);
    myBufferSrc.setupChannels(StChannelMap::CH20, StChannelMap::PCM, theIsPlanar ? stAV::audio::getNbChannels(myCodecCtx) : 1);
    stalConfigureSources2_0();
    return true;
}

bool StAudioQueue::initOut30Soft(const bool theIsPlanar) {
    if(!setupOutMonoFormat()) {
        return false;
    }

    myBufferOut.setupChannels(StChannelMap::CH30, StChannelMap::PCM, 3);
    myBufferSrc.setupChannels(StChannelMap::CH30, StChannelMap::PCM, theIsPlanar ? stAV::audio::getNbChannels(myCodecCtx) : 1);
    stalConfigureSources3_0();
    return true;
}

bool StAudioQueue::initOutStereo(const bool theIsPlanar) {
    switch(myBufferSrc.getFormat()) {
        case StPcmFormat_Float64: {
            if(myAlCtx.hasExtFloat64) {
                // use float64 extension
                myAlFormat = alGetEnumValue("AL_FORMAT_STEREO_DOUBLE_EXT");
                myBufferOut.setFormat(StPcmFormat_Float64);
                break;
            }
        }
        ST_FALLTHROUGH
        case StPcmFormat_Int32:
        case StPcmFormat_Float32: {
            if(myAlCtx.hasExtFloat32) {
                myAlFormat = alGetEnumValue("AL_FORMAT_STEREO_FLOAT32");
                myBufferOut.setFormat(StPcmFormat_Float32);
                break;
            }
        }
        ST_FALLTHROUGH
        case StPcmFormat_Int16: {
            // default - int16_t
            myAlFormat = AL_FORMAT_STEREO16;
            myBufferOut.setFormat(StPcmFormat_Int16);
            break;
        }
        case StPcmFormat_UInt8: {
            myAlFormat = AL_FORMAT_STEREO8;
            myBufferOut.setFormat(StPcmFormat_UInt8);
            break;
        }
        default: return false;
    }

    myBufferSrc.setupChannels(StChannelMap::CH20, StChannelMap::PCM, theIsPlanar ? stAV::audio::getNbChannels(myCodecCtx) : 1);
    myBufferOut.setupChannels(StChannelMap::CH20, StChannelMap::PCM, 1);
    stalConfigureSources1();
    return true;
}

bool StAudioQueue::initOut40Soft(const bool theIsPlanar) {
    if(!setupOutMonoFormat()) {
        return false;
    }

    myBufferOut.setupChannels(StChannelMap::CH40, StChannelMap::PCM, 4);
    myBufferSrc.setupChannels(StChannelMap::CH40, StChannelMap::PCM, theIsPlanar ? stAV::audio::getNbChannels(myCodecCtx) : 1);
    stalConfigureSources4_0();
    return true;
}

bool StAudioQueue::initOut40Ext(const bool theIsPlanar) {
    switch(myBufferSrc.getFormat()) {
        case StPcmFormat_Int32:
        case StPcmFormat_Float32:
        case StPcmFormat_Float64: {
            myAlFormat = alGetEnumValue("AL_FORMAT_QUAD32");
            myBufferOut.setFormat(StPcmFormat_Float32);
            break;
        }
        case StPcmFormat_Int16: {
            myAlFormat = alGetEnumValue("AL_FORMAT_QUAD16");
            myBufferOut.setFormat(StPcmFormat_Int16);
            break;
        }
        case StPcmFormat_UInt8: {
            myAlFormat = alGetEnumValue("AL_FORMAT_QUAD8");
            myBufferOut.setFormat(StPcmFormat_UInt8);
            break;
        }
        default: return false;
    }

    myBufferSrc.setupChannels(StChannelMap::CH40, StChannelMap::PCM, theIsPlanar ? stAV::audio::getNbChannels(myCodecCtx) : 1);
    myBufferOut.setupChannels(StChannelMap::CH40, StChannelMap::PCM, 1);
    stalConfigureSources1();
    return true;
}

bool StAudioQueue::initOut40BFormat(const bool theIsPlanar) {
    switch(myBufferSrc.getFormat()) {
        case StPcmFormat_Int32:
        case StPcmFormat_Float32:
        case StPcmFormat_Float64: {
            myAlFormat = alGetEnumValue("AL_FORMAT_BFORMAT3D_FLOAT32");
            myBufferOut.setFormat(StPcmFormat_Float32);
            break;
        }
        case StPcmFormat_Int16: {
            myAlFormat = alGetEnumValue("AL_FORMAT_BFORMAT3D_16");
            myBufferOut.setFormat(StPcmFormat_Int16);
            break;
        }
        case StPcmFormat_UInt8: {
            myAlFormat = alGetEnumValue("AL_FORMAT_BFORMAT3D_8");
            myBufferOut.setFormat(StPcmFormat_UInt8);
            break;
        }
        default: return false;
    }

    myBufferSrc.setupChannels(StChannelMap::CH40, StChannelMap::WYZX, theIsPlanar ? stAV::audio::getNbChannels(myCodecCtx) : 1);
    myBufferOut.setupChannels(StChannelMap::CH40, StChannelMap::PCM, 1);
    stalConfigureSources1();
    return true;
}

bool StAudioQueue::initOut50Soft(const bool theIsPlanar) {
    if(!setupOutMonoFormat()) {
        return false;
    }

    myBufferOut.setupChannels(StChannelMap::CH50, StChannelMap::PCM, 5);
    myBufferSrc.setupChannels(StChannelMap::CH50, StChannelMap::PCM, theIsPlanar ? stAV::audio::getNbChannels(myCodecCtx) : 1);
    stalConfigureSources5_0();
    return true;
}

bool StAudioQueue::initOut51Soft(const bool theIsPlanar) {
    if(!setupOutMonoFormat()) {
        return false;
    }

    myBufferOut.setupChannels(StChannelMap::CH51, StChannelMap::PCM, 6);
    myBufferSrc.setupChannels(StChannelMap::CH51, StChannelMap::PCM, theIsPlanar ? stAV::audio::getNbChannels(myCodecCtx) : 1);
    stalConfigureSources5_1();
    return true;
}

bool StAudioQueue::initOut51Ext(const bool theIsPlanar) {
    switch(myBufferSrc.getFormat()) {
        case StPcmFormat_Int32:
        case StPcmFormat_Float32:
        case StPcmFormat_Float64: {
            myAlFormat = alGetEnumValue("AL_FORMAT_51CHN32");
            myBufferOut.setFormat(StPcmFormat_Float32);
            break;
        }
        case StPcmFormat_Int16: {
            myAlFormat = alGetEnumValue("AL_FORMAT_51CHN16");
            myBufferOut.setFormat(StPcmFormat_Int16);
            break;
        }
        case StPcmFormat_UInt8: {
            myAlFormat = alGetEnumValue("AL_FORMAT_51CHN8");
            myBufferOut.setFormat(StPcmFormat_UInt8);
            break;
        }
        default: return false;
    }

    myBufferOut.setupChannels(StChannelMap::CH51, StChannelMap::PCM, 1);
    myBufferSrc.setupChannels(StChannelMap::CH51, StChannelMap::PCM, theIsPlanar ? stAV::audio::getNbChannels(myCodecCtx) : 1);
    stalConfigureSources1();
    return true;
}

bool StAudioQueue::initOut71Soft(const bool theIsPlanar) {
    if(!setupOutMonoFormat()) {
        return false;
    }

    myBufferOut.setupChannels(StChannelMap::CH71, StChannelMap::PCM, 8);
    myBufferSrc.setupChannels(StChannelMap::CH71, StChannelMap::PCM, theIsPlanar ? stAV::audio::getNbChannels(myCodecCtx) : 1);
    stalConfigureSources7_1();
    return true;
}

bool StAudioQueue::initOut71Ext(const bool theIsPlanar) {
    if(!myAlCtx.hasExtMultiChannel
     || stAV::audio::getNbChannels(myCodecCtx) != 8) {
        return false;
    }

    switch(myBufferSrc.getFormat()) {
        case StPcmFormat_Int32:
        case StPcmFormat_Float32:
        case StPcmFormat_Float64: {
            myAlFormat = alGetEnumValue("AL_FORMAT_71CHN32");
            myBufferOut.setFormat(StPcmFormat_Float32);
            break;
        }
        case StPcmFormat_Int16: {
            myAlFormat = alGetEnumValue("AL_FORMAT_71CHN16");
            myBufferOut.setFormat(StPcmFormat_Int16);
            break;
        }
        case StPcmFormat_UInt8: {
            myAlFormat = alGetEnumValue("AL_FORMAT_71CHN8");
            myBufferOut.setFormat(StPcmFormat_UInt8);
            break;
        }
        default: return false;
    }

    myBufferSrc.setupChannels(StChannelMap::CH71, StChannelMap::PCM, theIsPlanar ? stAV::audio::getNbChannels(myCodecCtx) : 1);
    myBufferOut.setupChannels(StChannelMap::CH71, StChannelMap::PCM, 1);
    stalConfigureSources1();
    return true;
}

bool StAudioQueue::initOutChannels() {
    const bool isPlanar = av_sample_fmt_is_planar(myCodecCtx->sample_fmt) != 0;

    myAlCanBFormat = false;
    myAlIsBFormat  = false;
    const int aNbChannels = stAV::audio::getNbChannels(myCodecCtx);
    switch(aNbChannels) {
        case 1: {
            myAlSoftLayout = true; // just unsupported
            return initOutMono();
        }
        case 2: {
            if(!myToOrientListener) {
                myAlSoftLayout = false;
                return initOutStereo(isPlanar);
            } else {
                myAlSoftLayout = true;
                return initOut20Soft(isPlanar);
            }
        }
        case 3: {
            myAlSoftLayout = true;
            return initOut30Soft(isPlanar);
        }
        case 4: {
            myAlCanBFormat = myAlCtx.hasExtBFormat;
            bool isAmbisonic = false;
        #ifdef ST_AV_NEW_CHANNEL_LAYOUT
            isAmbisonic = myCodecCtx->ch_layout.order == AV_CHANNEL_ORDER_AMBISONIC;
        #endif
            if(myToForceBFormat && myAlCtx.hasExtBFormat) {
                myAlSoftLayout = true;
                myAlIsBFormat  = true;
                return initOut40BFormat(isPlanar);
            }
            if(isAmbisonic && myAlCtx.hasExtBFormat) {
                myAlSoftLayout = true;
                //myAlIsBFormat  = true; // not forced
                return initOut40BFormat(isPlanar);
            }
            if(myAlCtx.hasExtMultiChannel && !myToOrientListener) {
                myAlSoftLayout = false;
                return initOut40Ext(isPlanar);
            }
            ST_DEBUG_LOG("OpenAL: multichannel extension (AL_FORMAT_QUAD16) is unavailable");
            myAlSoftLayout = true;
            return initOut40Soft(isPlanar);
        }
        case 5: {
            myAlSoftLayout = true;
            return initOut50Soft(isPlanar);
        }
        case 6: {
            if(myAlCtx.hasExtMultiChannel && !myToOrientListener) {
                myAlSoftLayout = false;
                return initOut51Ext(isPlanar);
            }
            ST_DEBUG_LOG("OpenAL: multichannel extension (AL_FORMAT_51CHN16) is unavailable");
            myAlSoftLayout = true;
            return initOut51Soft(isPlanar);
        }
        case 8: {
            if(myAlCtx.hasExtMultiChannel && !myToOrientListener) {
                myAlSoftLayout = false;
                return initOut71Ext(isPlanar);
            }

            ST_DEBUG_LOG("OpenAL: multichannel extension (AL_FORMAT_71CHN16) is unavailable");
            myAlSoftLayout = true;
            return initOut71Soft(isPlanar);
        }
        default: {
            myAlSoftLayout = true;
            return false;
        }
    }
}

bool StAudioQueue::init(AVFormatContext*   theFormatCtx,
                        const unsigned int theStreamId,
                        const StString&    theFileName) {
    while(myIsAlValid == ST_AL_INIT_NA) {
        StThread::sleep(10);
        continue;
    }

    if(myIsAlValid != ST_AL_INIT_OK) {
        signals.onError(stCString("OpenAL: no playback device available"));
        deinit();
        return false;
    }

    if(!StAVPacketQueue::init(theFormatCtx, theStreamId, theFileName)) {
        signals.onError(stCString("FFmpeg: invalid stream"));
        deinit();
        return false;
    }

    if(avcodec_open2(myCodecCtx, myCodecAuto, NULL) < 0) {
        signals.onError(stCString("FFmpeg: could not open audio codec"));
        deinit();
        return false;
    }
    myCodec = myCodecAuto;

    // setup buffers
    if(!initBuffers()) {
        deinit();
        return false;
    }

    fillCodecInfo(myCodec);
    return true;
}

bool StAudioQueue::initBuffers() {
    myAvSrcFormat  = myCodecCtx->sample_fmt;
    myAvSampleRate = myCodecCtx->sample_rate;
    myAvNbChannels = stAV::audio::getNbChannels(myCodecCtx);
    if(myCodecCtx->sample_rate < FREQ_5500) {
        signals.onError(StString("FFmpeg: wrong audio frequency ") + myCodecCtx->sample_rate);
        deinit();
        return false;
    }

    // setup source sample format (bitness)
    if(myCodecCtx->sample_fmt == stAV::audio::SAMPLE_FMT::NONE) {
        signals.onError(stCString("Invalid audio sample format!"));
        deinit();
        return false;
    } else if(myCodecCtx->sample_fmt == stAV::audio::SAMPLE_FMT::U8
           || myCodecCtx->sample_fmt == stAV::audio::SAMPLE_FMT::U8P) {
        myBufferSrc.setFormat(StPcmFormat_UInt8);
    } else if(myCodecCtx->sample_fmt == stAV::audio::SAMPLE_FMT::S16
           || myCodecCtx->sample_fmt == stAV::audio::SAMPLE_FMT::S16P) {
        myBufferSrc.setFormat(StPcmFormat_Int16);
    } else if(myCodecCtx->sample_fmt == stAV::audio::SAMPLE_FMT::S32
           || myCodecCtx->sample_fmt == stAV::audio::SAMPLE_FMT::S32P) {
        myBufferSrc.setFormat(StPcmFormat_Int32);
    } else if(myCodecCtx->sample_fmt == stAV::audio::SAMPLE_FMT::FLT
           || myCodecCtx->sample_fmt == stAV::audio::SAMPLE_FMT::FLTP) {
        myBufferSrc.setFormat(StPcmFormat_Float32);
    } else if(myCodecCtx->sample_fmt == stAV::audio::SAMPLE_FMT::DBL
           || myCodecCtx->sample_fmt == stAV::audio::SAMPLE_FMT::DBLP) {
        myBufferSrc.setFormat(StPcmFormat_Float64);
    } else {
        signals.onError(StString("Audio sample format '") + stAV::audio::getSampleFormatString(myCodecCtx)
                      + "' not supported");
        deinit();
        return false;
    }

    // setup frequency
    myBufferSrc.setFreq(myCodecCtx->sample_rate);
    myBufferOut.setFreq(myCodecCtx->sample_rate);

    // setup channel order
    if(!initOutChannels()) {
        deinit();
        signals.onError(stCString("OpenAL: unsupported format or channels configuration"));
        return false;
    }
    return true;
}

void StAudioQueue::deinit() {
    myBufferSrc.clear();
    myBufferOut.clear();
    myAvSrcFormat  = -1;
    myAvSampleRate = -1;
    myAvNbChannels = -1;
    myAlSoftLayout = true;
    myAlCanBFormat = false;
    myAlIsBFormat  = false;
    StAVPacketQueue::deinit();
}

bool StAudioQueue::parseEvents() {
    double aPtsSeek = 0.0;

    bool toResetBuffers = false;
    if(myToSwitchDev) {
        stalReinitialize();
        return true;
    } else if(myAlHintOut  != myAlHintOutPrev
           || myAlHintHrtf != myAlHintHrtfPrev) {
        stalResetAlHints();
        StMutexAuto aLock(myAlInfoMutex);
        myAlInfo.clear();
        myAlCtx.fullInfo(myAlInfo);
    } else if(myAlIsBFormat != myToForceBFormat
           && myAlCanBFormat) {
        toResetBuffers = true;
    }

    if(myToOrientListener) {
        if(!myAlSoftLayout) {
            toResetBuffers = true;
        } else {
            stalOrientListener();
        }
    } else if(myAlIsListOrient) {
        stalOrientListener();
    }

    if(toResetBuffers) {
        myBufferSrc.clear();
        myBufferOut.clear();
        initBuffers();
        return true;
    }

    if(!stAreEqual(myAlGain, myAlGainPrev, 1.e-7f)) {
        ST_DEBUG_LOG("Audio volume changed from " + myAlGainPrev + " to " + myAlGain);
        myAlGainPrev = myAlGain;
        alListenerf(AL_GAIN, myAlGain); // apply gain to all sources at-once
    }

    switch(popPlayEvent(aPtsSeek)) {
        case ST_PLAYEVENT_PLAY: {
            stalEmpty();
            playTimerStart(0.0);
            playTimerPause();
            return false;
        }
        case ST_PLAYEVENT_STOP: {
            playTimerPause();
            stalEmpty();
            return false;
        }
        case ST_PLAYEVENT_PAUSE: {
            playTimerPause();
            alSourcePausev(THE_NUM_AL_SOURCES, myAlSources);
            return false;
        }
        case ST_PLAYEVENT_RESUME: {
            playTimerResume();
            alSourcePlayv(THE_NUM_AL_SOURCES, myAlSources);
            return false;
        }
        case ST_PLAYEVENT_SEEK: {
            stalEmpty();
            playTimerStart(aPtsSeek);
            playTimerPause();
            myBufferSrc.setDataSize(0);
            myBufferOut.setDataSize(0);
            // return special flag to skip "resume playback from" in loop
            return true;
        }
        case ST_PLAYEVENT_NONE:
        default:
            return false;
    }
}

bool StAudioQueue::stalQueue(const double thePts) {
    ALint aQueued = 0;
    ALint aProcessed = 0;
    ALenum aState = stalGetSourceState();
    alGetSourcei(myAlSources[0], AL_BUFFERS_PROCESSED, &aProcessed);
    alGetSourcei(myAlSources[0], AL_BUFFERS_QUEUED,    &aQueued);

#ifdef ST_DEBUG
    if(myDbgPrevQueued != aQueued) {
        ST_DEBUG_LOG("OpenAL buffers: " + aQueued + " queued + "
            + aProcessed + " processed from " + THE_NUM_AL_BUFFERS
        );
    }
    myDbgPrevQueued = aQueued;
#else
    (void )myDbgPrevQueued;
#endif

    if((aState == AL_PLAYING
     || aState == AL_PAUSED)
    && (myPrevFormat    != myAlFormat
     || myPrevFrequency != myBufferOut.getFreq()))
    {
        return false; // wait until tail of previous stream played
    }

    if(myPrevFormat    != myAlFormat
    || myPrevFrequency != myBufferOut.getFreq()
    || (aState  == AL_STOPPED
     && aQueued == THE_NUM_AL_BUFFERS)) {
        ST_DEBUG_LOG("AL, reinitialize buffers per source , plane size= " + myBufferOut.getPlaneSize()
                            + "; freq= " + myBufferOut.getFreq());
        stalEmpty();
        stalCheckErrors("reset state");
        aProcessed = 0;
        aQueued = 0;
    }

    bool toTryToPlay = false;
    bool isQueued = false;
    if(aProcessed == 0 && aQueued < THE_NUM_AL_BUFFERS) {
        if(myBufferOut.isEmpty()) {
            ST_DEBUG_LOG(" EMPTY BUFFER ");
            return true;
        }

        stalCheckErrors("reset state");
        ///ST_DEBUG_LOG("AL, queue more buffers " + aQueued + " / " + NUM_AL_BUFFERS);
        myPrevFormat    = myAlFormat;
        myPrevFrequency = myBufferOut.getFreq();
        for(size_t aSrcId = 0; aSrcId < myBufferOut.getPlanesNb(); ++aSrcId) {
            alBufferData(myAlBuffers[aSrcId][aQueued], myAlFormat,
                         myBufferOut.getPlane(aSrcId), (ALsizei )myBufferOut.getPlaneSize(),
                         myBufferOut.getFreq());
            stalCheckErrors("alBufferData1");
            alSourceQueueBuffers(myAlSources[aSrcId], 1, &myAlBuffers[aSrcId][aQueued]);
            stalCheckErrors("alSourceQueueBuffers");
        }
        toTryToPlay = ((aQueued + 1) == THE_NUM_AL_BUFFERS);
        isQueued = true;
    } else if(aProcessed != 0
           && (aState == AL_PLAYING
            || aState == AL_PAUSED)) {
        ALuint alBuffIdToFill = 0;
        ///ST_DEBUG_LOG("queue buffer " + thePts + "; state= " + stalGetSourceState());
        if(myBufferOut.isEmpty()) {
            ST_DEBUG_LOG(" EMPTY BUFFER ");
            return true;
        }

        myPrevFormat    = myAlFormat;
        myPrevFrequency = myBufferOut.getFreq();
        for(size_t aSrcId = 0; aSrcId < myBufferOut.getPlanesNb(); ++aSrcId) {

            // wait other sources for processed buffers
            if(aSrcId != 0) {
                myLimitTimer.restart();
                for(;;) {
                    alGetSourcei(myAlSources[aSrcId], AL_BUFFERS_PROCESSED, &aProcessed);
                    if(aProcessed != 0) {
                        break;
                    }
                    if(myLimitTimer.getElapsedTimeInSec() > 2.0) {
                        // just avoid dead loop - should never happens
                        return false;
                    }
                    StThread::sleep(10);
                }
            }

            alSourceUnqueueBuffers(myAlSources[aSrcId], 1, &alBuffIdToFill);
            stalCheckErrors("alSourceUnqueueBuffers");
            if(alBuffIdToFill != 0) {
                alBufferData(alBuffIdToFill, myAlFormat,
                             myBufferOut.getPlane(aSrcId), (ALsizei )myBufferOut.getPlaneSize(),
                             myBufferOut.getFreq());
                stalCheckErrors("alBufferData2");
                alSourceQueueBuffers(myAlSources[aSrcId], 1, &alBuffIdToFill);
                stalCheckErrors("alSourceQueueBuffers");
            } else {
                ST_DEBUG_LOG("OpenAL, unqueue FAILED");
            }
        }
        toTryToPlay = true;
        isQueued = true;
    }

    if(aState == AL_STOPPED
    && toTryToPlay) {
        double diffSecs = double(myAlDataLoop.summ() + myBufferOut.getDataSizeWhole()) / double(myBufferOut.getSecondSize());
        if((thePts - diffSecs) < 100000.0) {
            playTimerStart(thePts - diffSecs);
        } else {
            playTimerStart(0.0);
        }
        alSourcePlayv(THE_NUM_AL_SOURCES, myAlSources);
        if(stalCheckConnected()) {
            ST_DEBUG_LOG("!!! OpenAL was in stopped state, now resume playback from " + (thePts - diffSecs));
        }

        // pause playback if not in playing state
        myEventMutex.lock();
        const bool toPause = !myIsPlaying;
        myEventMutex.unlock();
        if(toPause) {
            alSourcePausev(THE_NUM_AL_SOURCES, myAlSources);
        }
    }

    return isQueued;
}

bool StAudioQueue::stalCheckConnected() {
    if(!myIsDisconnected && myAlCtx.isConnected()) {
        return true;
    }

    myAlDeviceName.clear();
    stalDeinit(); // release OpenAL context
    myIsAlValid = (stalInit() ? ST_AL_INIT_OK : ST_AL_INIT_KO);
    myIsDisconnected = true;
    ST_DEBUG_LOG("!!! OpenAL device was disconnected !!!");
    return false;
}

void StAudioQueue::stalFillBuffers(const double thePts,
                                   const bool   toIgnoreEvents) {
    if(!toIgnoreEvents) {
        parseEvents();
    }

    bool toSkipPlaybackFrom = false;
    while(!stalQueue(thePts)) {
        // AL queue is full
        if(!toIgnoreEvents) {
            toSkipPlaybackFrom = parseEvents();
        }
        if(myToQuit) {
            return;
        }

        if(!toSkipPlaybackFrom && !stalIsAudioPlaying() && isPlaying()) {
            // this position means:
            // 1) buffers were empty and playback was stopped
            //    now we have all buffers full and could play them
            double diffSecs = double(myAlDataLoop.summ() + myBufferOut.getDataSizeWhole()) / double(myBufferOut.getSecondSize());
            if((thePts - diffSecs) < 100000.0) {
                playTimerStart(thePts - diffSecs);
            } else {
                playTimerStart(0.0);
            }
            alSourcePlayv(THE_NUM_AL_SOURCES, myAlSources);
            if(stalCheckConnected()) {
                ST_DEBUG_LOG("!!! OpenAL was in stopped state, now resume playback from " + (thePts - diffSecs));
            }
        } else {
            // TODO (Kirill Gavrilov#3#) often updates may prevent normal video playback
            // on files with broken audio/video PTS
            ALfloat aPos = 0.0f;
            alGetSourcef(myAlSources[0], AL_SEC_OFFSET, &aPos);
            double diffSecs = double(myAlDataLoop.summ() + myBufferOut.getDataSizeWhole()) / double(myBufferOut.getSecondSize());
            diffSecs -= aPos;
            if((thePts - diffSecs) < 100000.0) {
                 static double oldPts = 0.0;
                 if(thePts != oldPts) {
                    /**ST_DEBUG_LOG("set AAApts " + (thePts - diffSecs)
                        + " from " + getPts()
                        + "(" + thePts + ", " + diffSecs + ")"
                    );*/
                    playTimerStart(thePts - diffSecs);
                    oldPts = thePts;
                }
                ///playTimerStart(thePts - diffSecs);
            }
        }
        StThread::sleep(1);
    }
}

void StAudioQueue::decodePacket(const StHandle<StAVPacket>& thePacket,
                                double&                     thePts) {
    int anAudioPktSize = thePacket->getSize();
    bool checkMoreFrames = false;
    bool toSendPacket = true;
    // packet could store multiple frames
    for(;;) {
        while(anAudioPktSize > 0) {
            int aDataSize = (int )myBufferSrc.getBufferSizeWhole();
            (void)aDataSize;
            if(toSendPacket) {
                const int aRes = avcodec_send_packet(myCodecCtx, thePacket->getType() == StAVPacket::DATA_PACKET ? thePacket->getAVpkt() : NULL);
                if(aRes < 0 && aRes != AVERROR_EOF) {
                    anAudioPktSize = 0;
                    break;
                }
                toSendPacket = false;
            }

            myFrame.reset();
            const int aRes2 = avcodec_receive_frame(myCodecCtx, myFrame.Frame);
            if(aRes2 < 0) {
                anAudioPktSize = 0;
                break;
            }

            if(myAvSrcFormat  != myCodecCtx->sample_fmt
            || myAvNbChannels != stAV::audio::getNbChannels(myCodecCtx)
            || myAvSampleRate != myCodecCtx->sample_rate) {
                ST_DEBUG_LOG("Parameters of the Audio stream has been changed,"
                           + " Nb. channels: " + stAV::audio::getNbChannels(myCodecCtx)    + " (was " + myAvNbChannels + ")"
                           + " Sample Rate: "  + myCodecCtx->sample_rate + " (was " + myAvSampleRate + ")");
                myBufferSrc.clear();
                myBufferOut.clear();
                initBuffers();
                checkMoreFrames = true;
                break;
            }

            for(size_t aPlaneIter = 0; aPlaneIter < myBufferSrc.getPlanesNb(); ++aPlaneIter) {
                myBufferSrc.wrapPlane(aPlaneIter, myFrame.getPlane(aPlaneIter));
            }

            int aPlaneSize = 0;
            aDataSize = av_samples_get_buffer_size(&aPlaneSize, stAV::audio::getNbChannels(myCodecCtx),
                                                   myFrame.Frame->nb_samples,
                                                   myCodecCtx->sample_fmt, 1);
            myBufferSrc.setPlaneSize(aPlaneSize); // notice that myFrame.getLineSize(0) contains extra alignment

            checkMoreFrames = true;
            if(myBufferOut.addData(myBufferSrc)) {
                // 'big buffer' still not full
                break;
            }

            if(!myBufferOut.isEmpty()) {
                int64_t aPtsU = myFrame.Frame->pts;
                if(aPtsU == stAV::NOPTS_VALUE) {
                    aPtsU = thePacket->getPts();
                }

                if(aPtsU != stAV::NOPTS_VALUE) {
                    const double aNewPts = unitsToSeconds(aPtsU) - myPtsStartBase;
                    if(aNewPts <= thePts) {
                        ST_DEBUG_LOG("Got the AUDIO packet with pts in past; "
                            + "new PTS= "   + aNewPts
                            + "; old PTS= " + thePts
                        );
                    }
                    thePts = aNewPts;
                }

                // now fill OpenAL buffers
                stalFillBuffers(thePts, false);
                if(myToQuit) {
                    return;
                }

                // save the history for filled AL buffers sizes
                myAlDataLoop.push(myBufferOut.getDataSizeWhole());
            }

            myBufferOut.setDataSize(0);                         // clear 'big' buffer
            myBufferOut.resize(myBufferSrc.getDataSizeWhole(),  // make sure buffer is big enough
                               false);
            myBufferOut.addData(myBufferSrc);
            break;
        }

        if(checkMoreFrames) {
            checkMoreFrames = false;
            continue;
        }
        break;
    }
}

void StAudioQueue::decodeLoop() {
    myIsAlValid = (stalInit() ? ST_AL_INIT_OK : ST_AL_INIT_KO);

    double aPts = 0.0;
    StHandle<StAVPacket> aPacket;
    for(;;) {
        // wait for upcoming packets
        if(isEmpty()) {
            myDowntimeEvent.set();
            parseEvents();
            StThread::sleep(10);
            ///ST_DEBUG_LOG_AT("AQ is empty");
            continue;
        }
        myDowntimeEvent.reset();

        aPacket = pop();
        if(aPacket.isNull()) {
            continue;
        }
        switch(aPacket->getType()) {
            case StAVPacket::FLUSH_PACKET: {
                // got the special FLUSH packet - flush FFmpeg codec buffers
                if(myCodecCtx != NULL && myCodec != NULL) {
                    avcodec_flush_buffers(myCodecCtx);
                }
                // at this moment we clear current data from our buffers too
                myBufferOut.setDataSize(0);
                myBufferSrc.setDataSize(0);
                stalEmpty();
                continue;
            }
            case StAVPacket::START_PACKET: {
                playTimerStart(myPtsStartStream - myPtsStartBase);
                aPts = 0.0;
                continue;
            }
            case StAVPacket::DATA_PACKET: {
                break;
            }
            case StAVPacket::LAST_PACKET: {
                break; // redirect NULL packet to avcodec_send_packet()
            }
            case StAVPacket::END_PACKET: {
                pushPlayEvent(ST_PLAYEVENT_NONE);
                // TODO (Kirill Gavrilov#3#) improve file-by-file playback
                if(!myBufferOut.isEmpty()) {
                    stalFillBuffers(aPts, true);
                }
                myBufferOut.setDataSize(0);
                myBufferSrc.setDataSize(0);
                if(myToQuit) {
                    stalDeinit(); // release OpenAL context
                    return;
                }
                continue;
            }
            case StAVPacket::QUIT_PACKET: {
                stalDeinit(); // release OpenAL context
                return;
            }
        }

        // we got the data packet, so decode it
        decodePacket(aPacket, aPts);
        aPacket.nullify();
    }
}

void StAudioQueue::pushPlayEvent(const StPlayEvent_t theEventId,
                                 const double        theSeekParam) {
    myEventMutex.lock();
    StAVPacketQueue::pushPlayEvent(theEventId, theSeekParam);
    if(theEventId == ST_PLAYEVENT_SEEK) {
        myPlaybackTimer.restart(theSeekParam * 1000000.0);
    }
    myEventMutex.unlock();
}
