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

#include "StPCMBuffer.h"

#include <stAssert.h>
#include <StStrings/StLogger.h>

/**
 * 1 second of 48khz 32bit audio (old AVCODEC_MAX_AUDIO_FRAME_SIZE).
 */
#define ST_MAX_AUDIO_FRAME_SIZE 192000

StChannelMap::StChannelMap(const StChannelMap::Channels   theChannels,
                           const StChannelMap::OrderRules theRules)
: count(0),
  channels(theChannels) {
    for(size_t aChIter = 0; aChIter < ST_AUDIO_CHANNELS_MAX; ++aChIter) {
        Order[aChIter] = aChIter;
    }
    switch(theChannels) {
        case StChannelMap::CH10:
            FL = FR = FC = LFE = RL = RR = SL = SR = 0;
            count = 1;
            return;
        case StChannelMap::CH20:
            FL = FC = LFE = RL = RR = SL = SR = 0; FR = 1;
            count = 2;
            return;
        case StChannelMap::CH30:
            count = 3;
            FL = 0;
            FR = 1;
            FC = 2;
            LFE = RL = RR = SL = SR = 0;
            return;
        case StChannelMap::CH40:
            count = 4;
            FL = 0;
            FR = 1;
            RL = 2;
            RR = 3;
            FC = LFE = SL = SR = 0;
            return;
        case StChannelMap::CH50:
            count = 5;
            FL  = 0;
            FR  = 1;
            FC  = 2;
            RL  = 3;
            RR  = 4;
            LFE = SL = SR = 0;
            return;
        case StChannelMap::CH51:
            count = 6;
            SL = SR = 0;
            switch(theRules) {
                case StChannelMap::PCM: {
                    FL  = 0;
                    FR  = 1;
                    FC  = 2;
                    LFE = 3;
                    RL  = 4;
                    RR  = 5;
                    return;
                }
                case StChannelMap::AC3:
                case StChannelMap::OGG: {
                    FL  = 0;
                    FC  = 1;
                    FR  = 2;
                    RL  = 3;
                    RR  = 4;
                    LFE = 5;
                    Order[0] = FL;
                    Order[1] = FR;
                    Order[2] = FC;
                    Order[3] = LFE;
                    Order[4] = RL;
                    Order[5] = RR;
                    return;
                }
            }
        case StChannelMap::CH71:
            count = 8;
            FL  = 0;
            FR  = 1;
            FC  = 2;
            LFE = 3;
            RL  = 4;
            RR  = 5;
            SL  = 6;
            SR  = 7;
            return;
    }
}

void StPCMBuffer::setFormat(const StPCMformat thePCMFormat) {
    switch(thePCMFormat) {
        case PCM8_UNSIGNED: mySampleSize = sizeof(uint8_t); break;
        case PCM16_SIGNED:  mySampleSize = sizeof(int16_t); break;
        case PCM32_SIGNED:  mySampleSize = sizeof(int32_t); break;
        case PCM32FLOAT:    mySampleSize = sizeof(float);   break;
        case PCM64FLOAT:    mySampleSize = sizeof(double);  break;
    }
    myPCMFormat = thePCMFormat;
}

StPCMBuffer::StPCMBuffer(const StPCMformat thePCMFormat)
: myBuffer(NULL),
  mySizeBytes(ST_MAX_AUDIO_FRAME_SIZE),
  myPlaneSize(0),
  myPlanesNb(1),
  mySampleSize(0),
  myPCMFormat(thePCMFormat),
  myPCMFreq(FREQ_44100),
  myChMap(StChannelMap::CH10, StChannelMap::PCM) {
    myBuffer = stMemAllocAligned<uint8_t*>(mySizeBytes, 16); // data must be aligned to 16 bytes for SSE!
    stMemZero(myBuffer, mySizeBytes);
    stMemZero(myPlanes, sizeof(myPlanes));
    myPlanes[0] = myBuffer; // single plane for interleaved data
    setFormat(thePCMFormat);
}

StPCMBuffer::~StPCMBuffer() {
    stMemFreeAligned(myBuffer);
}

void StPCMBuffer::clear() {
    myPlaneSize = 0;
    stMemZero(myBuffer, mySizeBytes);
}

void StPCMBuffer::setupChannels(const StChannelMap& theChMap,
                                const size_t        thePlanesNb) {
    myChMap     = theChMap;
    myPlanesNb  = (thePlanesNb != 0) ? thePlanesNb : 1;
    myPlaneSize = 0;

    const size_t aPlaneSizeMax = (mySizeBytes / myPlanesNb);
    for(size_t aPlaneIter = 0; aPlaneIter < ST_AUDIO_CHANNELS_MAX; ++aPlaneIter) {
        myPlanes[aPlaneIter] = (aPlaneIter < myPlanesNb)
                             ? &myBuffer[aPlaneIter * aPlaneSizeMax]
                             : NULL;
    }
}

void StPCMBuffer::resize(const size_t theSizeMin,
                         const bool   theToReduce) {
    if(mySizeBytes >= theSizeMin
    && !theToReduce) {
        return; // do not reduce buffer
    }

    mySizeBytes = theSizeMin;
    stMemZero(myPlanes, sizeof(myPlanes));
    stMemFreeAligned(myBuffer);
    myBuffer = stMemAllocAligned<uint8_t*>(mySizeBytes, 16);
    stMemZero(myBuffer, mySizeBytes);
    setupChannels(myChMap, myPlanesNb);
}

bool StPCMBuffer::setDataSize(const size_t theDataSize) {
    const size_t aPlaneSize    = theDataSize / myPlanesNb;
    const size_t aPlaneSizeMax = mySizeBytes / myPlanesNb;
    myPlaneSize = (aPlaneSize > aPlaneSizeMax) ? 0 : aPlaneSize;
    if(aPlaneSize < aPlaneSizeMax) {
        // we set end to zero for avcodec_decode_audio2
        // to prevent overflow on some broken streams...
        for(size_t aPlaneIter = 0; aPlaneIter < myPlanesNb; ++aPlaneIter) {
            myBuffer[aPlaneSizeMax - 1] = 0;
        }
    }
    return myPlaneSize != 0;
}

// useful constants
static const float  ST_INT16_MAX_F = 32768.0f;
static const double ST_INT16_MAX_D = 32768.0;
static const float  ST_INT32_MAX_F = 2147483648.0f;
static const double ST_INT32_MAX_D = 2147483648.0;
static const float  ST_INT8_MAX_INV_F  = 1.0f / 128.0f;
static const double ST_INT8_MAX_INV_D  = 1.0  / 128.0;
static const float  ST_INT16_MAX_INV_F = 1.0f / ST_INT16_MAX_F;
static const double ST_INT16_MAX_INV_D = 1.0  / ST_INT16_MAX_D;
static const float  ST_INT32_MAX_INV_F = 1.0f / ST_INT32_MAX_F;
static const double ST_INT32_MAX_INV_D = 1.0  / ST_INT32_MAX_D;

// uint8_t -> uint8_t, lossless
inline void sampleConv(const uint8_t& theSrcSample, uint8_t& theOutSample) {
    theOutSample = theSrcSample;
}

// uint8_t -> int16_t, lossless
inline void sampleConv(const uint8_t& theSrcSample, int16_t& theOutSample) {
    theOutSample = (int16_t(theSrcSample) - 127) << 8;
}

// uint8_t -> int32_t, lossless
inline void sampleConv(const uint8_t& theSrcSample, int32_t& theOutSample) {
    theOutSample = (int32_t(theSrcSample) - 127) << 16;
}

// uint8_t -> float
inline void sampleConv(const uint8_t& theSrcSample, float& theOutSample) {
    theOutSample = (float )theSrcSample * ST_INT8_MAX_INV_F - 1.0f;
}

// uint8_t -> double
inline void sampleConv(const uint8_t& theSrcSample, double& theOutSample) {
    theOutSample = (double )theSrcSample * ST_INT8_MAX_INV_D - 1.0;
}

// int16_t -> uint8_t, lossy
inline void sampleConv(const int16_t& theSrcSample, uint8_t& theOutSample) {
    theOutSample = uint8_t((theSrcSample >> 8) + 127);
}

// int16_t -> int16_t, lossless
inline void sampleConv(const int16_t& theSrcSample, int16_t& theOutSample) {
    theOutSample = theSrcSample;
}

// int16_t -> int32_t, lossless
inline void sampleConv(const int16_t& theSrcSample, int32_t& theOutSample) {
    theOutSample = theSrcSample << 16;
}

// int16_t -> float
inline void sampleConv(const int16_t& theSrcSample, float& theOutSample) {
    theOutSample = float(theSrcSample) * ST_INT16_MAX_INV_F;
}

// int16_t -> double
inline void sampleConv(const int16_t& theSrcSample, double& theOutSample) {
    theOutSample = double(theSrcSample) * ST_INT16_MAX_INV_D;
}

// int32_t -> uint8_t, lossy
inline void sampleConv(const int32_t& theSrcSample, uint8_t& theOutSample) {
    theOutSample = uint8_t((theSrcSample >> 16) + 127);
}

// int32_t -> int16_t, lossy
inline void sampleConv(const int32_t& theSrcSample, int16_t& theOutSample) {
    theOutSample = theSrcSample >> 16;
}

// int32_t -> int32_t, lossless
inline void sampleConv(const int32_t& theSrcSample, int32_t& theOutSample) {
    theOutSample = theSrcSample;
}

// int32_t -> float
inline void sampleConv(const int32_t& theSrcSample, float& theOutSample) {
    theOutSample = float(theSrcSample) * ST_INT32_MAX_INV_F;
}

// int32_t -> double
inline void sampleConv(const int32_t& theSrcSample, double& theOutSample) {
    theOutSample = double(theSrcSample) * ST_INT32_MAX_INV_D;
}

// float -> uint8_t, lossy
inline void sampleConv(const float& theSrcSample, uint8_t& theOutSample) {
    theOutSample = uint8_t(theSrcSample * 128.0f + 127.0f);
}

// float -> int16_t, lossy
inline void sampleConv(const float& theSrcSample, int16_t& theOutSample) {
    theOutSample = int16_t(theSrcSample * ST_INT16_MAX_F);
}

// float -> int32_t
inline void sampleConv(const float& theSrcSample, int32_t& theOutSample) {
    theOutSample = int32_t(theSrcSample * ST_INT32_MAX_F);
}

// float -> float, lossless
inline void sampleConv(const float& theSrcSample, float& theOutSample) {
    theOutSample = theSrcSample;
}

// float -> double, lossless
inline void sampleConv(const float& theSrcSample, double& theOutSample) {
    theOutSample = (double )theSrcSample;
}

// double -> uint8_t, lossy
inline void sampleConv(const double& theSrcSample, uint8_t& theOutSample) {
    theOutSample = uint8_t(theSrcSample * 128.0 + 127.0);
}

// double -> int16_t, lossy
inline void sampleConv(const double& theSrcSample, int16_t& theOutSample) {
    theOutSample = int16_t(theSrcSample * ST_INT16_MAX_D);
}

// double -> int32_t, lossy
inline void sampleConv(const double& theSrcSample, int32_t& theOutSample) {
    theOutSample = int32_t(theSrcSample * ST_INT32_MAX_D);
}

// double -> float, lossy
inline void sampleConv(const double& theSrcSample, float& theOutSample) {
    theOutSample = (float )theSrcSample;
}

// double -> double, lossless
inline void sampleConv(const double& theSrcSample, double& theOutSample) {
    theOutSample = theSrcSample;
}

template<typename sampleSrc_t, typename sampleOut_t>
bool StPCMBuffer::addConvert(const StPCMBuffer& theBuffer) {
    if(myPlanesNb > 1 && myPlanesNb != myChMap.count) {
        // currently only split into mono sources supported
        ST_DEBUG_ASSERT(false);
        return false;
    } else if(theBuffer.myPlaneSize * theBuffer.myPlanesNb < theBuffer.mySampleSize * myPlanesNb) {
        // just ignore
        return true;
    }

    const size_t aSamplesSrcCount = theBuffer.myPlaneSize / theBuffer.mySampleSize;
    const size_t anAddedPlaneSize = mySampleSize * ((aSamplesSrcCount * theBuffer.myPlanesNb) / myPlanesNb);
    const size_t aSmplSrcInc      = (theBuffer.myPlanesNb > 1) ? 1 : theBuffer.myChMap.count;
    const size_t aSmplOutInc      = (myPlanesNb           > 1) ? 1 : myChMap.count;

    // capture the start pointer for each channel
    sampleSrc_t* aBuffersSrc[ST_AUDIO_CHANNELS_MAX];
    sampleOut_t* aBuffersOut[ST_AUDIO_CHANNELS_MAX];
    for(size_t aChIter = 0; aChIter < theBuffer.myChMap.count; ++aChIter) {
        theBuffer.getChannelDataStart(aChIter, aBuffersSrc[aChIter]);
    }
    for(size_t aChIter = 0; aChIter < myChMap.count; ++aChIter) {
        getChannelDataEnd(aChIter, aBuffersOut[aChIter]);
    }

    switch(myChMap.channels) {
        case StChannelMap::CH10: {
            for(size_t sampleSrcId(0), sampleOutId(0); sampleSrcId < aSamplesSrcCount; sampleSrcId += aSmplSrcInc, sampleOutId += aSmplOutInc) {
                sampleConv(aBuffersSrc[0][sampleSrcId], aBuffersOut[0][sampleOutId]);
            }
            myPlaneSize += anAddedPlaneSize;
            return true;
        }
        case StChannelMap::CH20: {
            for(size_t sampleSrcId(0), sampleOutId(0); sampleSrcId < aSamplesSrcCount; sampleSrcId += aSmplSrcInc, sampleOutId += aSmplOutInc) {
                sampleConv(aBuffersSrc[0][sampleSrcId], aBuffersOut[0][sampleOutId]);
                sampleConv(aBuffersSrc[1][sampleSrcId], aBuffersOut[1][sampleOutId]);
            }
            myPlaneSize += anAddedPlaneSize;
            return true;
        }
        case StChannelMap::CH30: {
            for(size_t sampleSrcId(0), sampleOutId(0); sampleSrcId < aSamplesSrcCount; sampleSrcId += aSmplSrcInc, sampleOutId += aSmplOutInc) {
                sampleConv(aBuffersSrc[0][sampleSrcId], aBuffersOut[0][sampleOutId]);
                sampleConv(aBuffersSrc[1][sampleSrcId], aBuffersOut[1][sampleOutId]);
                sampleConv(aBuffersSrc[2][sampleSrcId], aBuffersOut[2][sampleOutId]);
            }
            myPlaneSize += anAddedPlaneSize;
            return true;
        }
        case StChannelMap::CH40: {
            for(size_t sampleSrcId(0), sampleOutId(0); sampleSrcId < aSamplesSrcCount; sampleSrcId += aSmplSrcInc, sampleOutId += aSmplOutInc) {
                sampleConv(aBuffersSrc[0][sampleSrcId], aBuffersOut[0][sampleOutId]);
                sampleConv(aBuffersSrc[1][sampleSrcId], aBuffersOut[1][sampleOutId]);
                sampleConv(aBuffersSrc[2][sampleSrcId], aBuffersOut[2][sampleOutId]);
                sampleConv(aBuffersSrc[3][sampleSrcId], aBuffersOut[3][sampleOutId]);
            }
            myPlaneSize += anAddedPlaneSize;
            return true;
        }
        case StChannelMap::CH50: {
            for(size_t sampleSrcId(0), sampleOutId(0); sampleSrcId < aSamplesSrcCount; sampleSrcId += aSmplSrcInc, sampleOutId += aSmplOutInc) {
                sampleConv(aBuffersSrc[0][sampleSrcId], aBuffersOut[0][sampleOutId]);
                sampleConv(aBuffersSrc[1][sampleSrcId], aBuffersOut[1][sampleOutId]);
                sampleConv(aBuffersSrc[2][sampleSrcId], aBuffersOut[2][sampleOutId]);
                sampleConv(aBuffersSrc[3][sampleSrcId], aBuffersOut[3][sampleOutId]);
                sampleConv(aBuffersSrc[4][sampleSrcId], aBuffersOut[4][sampleOutId]);
            }
            myPlaneSize += anAddedPlaneSize;
            return true;
        }
        case StChannelMap::CH51: {
            for(size_t sampleSrcId(0), sampleOutId(0); sampleSrcId < aSamplesSrcCount; sampleSrcId += aSmplSrcInc, sampleOutId += aSmplOutInc) {
                sampleConv(aBuffersSrc[0][sampleSrcId], aBuffersOut[0][sampleOutId]);
                sampleConv(aBuffersSrc[1][sampleSrcId], aBuffersOut[1][sampleOutId]);
                sampleConv(aBuffersSrc[2][sampleSrcId], aBuffersOut[2][sampleOutId]);
                sampleConv(aBuffersSrc[3][sampleSrcId], aBuffersOut[3][sampleOutId]);
                sampleConv(aBuffersSrc[4][sampleSrcId], aBuffersOut[4][sampleOutId]);
                sampleConv(aBuffersSrc[5][sampleSrcId], aBuffersOut[5][sampleOutId]);
            }
            myPlaneSize += anAddedPlaneSize;
            return true;
        }
        case StChannelMap::CH71: {
            for(size_t sampleSrcId(0), sampleOutId(0); sampleSrcId < aSamplesSrcCount; sampleSrcId += aSmplSrcInc, sampleOutId += aSmplOutInc) {
                sampleConv(aBuffersSrc[0][sampleSrcId], aBuffersOut[0][sampleOutId]);
                sampleConv(aBuffersSrc[1][sampleSrcId], aBuffersOut[1][sampleOutId]);
                sampleConv(aBuffersSrc[2][sampleSrcId], aBuffersOut[2][sampleOutId]);
                sampleConv(aBuffersSrc[3][sampleSrcId], aBuffersOut[3][sampleOutId]);
                sampleConv(aBuffersSrc[4][sampleSrcId], aBuffersOut[4][sampleOutId]);
                sampleConv(aBuffersSrc[5][sampleSrcId], aBuffersOut[5][sampleOutId]);
                sampleConv(aBuffersSrc[6][sampleSrcId], aBuffersOut[6][sampleOutId]);
                sampleConv(aBuffersSrc[7][sampleSrcId], aBuffersOut[7][sampleOutId]);
            }
            myPlaneSize += anAddedPlaneSize;
            return true;
        }
        default:
            return false;
    }
}

bool StPCMBuffer::addData(const StPCMBuffer& theBuffer) {
    if(theBuffer.isEmpty() || !hasDataSize(theBuffer.getDataSizeWhole())) {
        return false;
    } else if(myChMap.count != theBuffer.myChMap.count) {
        // currently not supported
        ST_DEBUG_ASSERT(false);
        return false;
    }

    if(myPlanesNb  != theBuffer.myPlanesNb
    || myChMap     != theBuffer.myChMap
    || myPCMFormat != theBuffer.myPCMFormat) {
        // split interleaved data or remap channel order or convert format
        switch(theBuffer.myPCMFormat) {
            case PCM8_UNSIGNED:
                switch(myPCMFormat) {
                    case PCM8_UNSIGNED: return addConvert<uint8_t, uint8_t>(theBuffer);
                    case PCM16_SIGNED:  return addConvert<uint8_t, int16_t>(theBuffer);
                    case PCM32_SIGNED:  return addConvert<uint8_t, int32_t>(theBuffer);
                    case PCM32FLOAT:    return addConvert<uint8_t, float  >(theBuffer);
                    case PCM64FLOAT:    return addConvert<uint8_t, double >(theBuffer);
                }
            case PCM16_SIGNED:
                switch(myPCMFormat) {
                    case PCM8_UNSIGNED: return addConvert<int16_t, uint8_t>(theBuffer);
                    case PCM16_SIGNED:  return addConvert<int16_t, int16_t>(theBuffer);
                    case PCM32_SIGNED:  return addConvert<int16_t, int32_t>(theBuffer);
                    case PCM32FLOAT:    return addConvert<int16_t, float  >(theBuffer);
                    case PCM64FLOAT:    return addConvert<int16_t, double >(theBuffer);
                }
            case PCM32_SIGNED:
                switch(myPCMFormat) {
                    case PCM8_UNSIGNED: return addConvert<int32_t, uint8_t>(theBuffer);
                    case PCM16_SIGNED:  return addConvert<int32_t, int16_t>(theBuffer);
                    case PCM32_SIGNED:  return addConvert<int32_t, int32_t>(theBuffer);
                    case PCM32FLOAT:    return addConvert<int32_t, float  >(theBuffer);
                    case PCM64FLOAT:    return addConvert<int32_t, double >(theBuffer);
                }
            case PCM32FLOAT:
                switch(myPCMFormat) {
                    case PCM8_UNSIGNED: return addConvert<float, uint8_t>(theBuffer);
                    case PCM16_SIGNED:  return addConvert<float, int16_t>(theBuffer);
                    case PCM32_SIGNED:  return addConvert<float, int32_t>(theBuffer);
                    case PCM32FLOAT:    return addConvert<float, float  >(theBuffer);
                    case PCM64FLOAT:    return addConvert<float, double >(theBuffer);
                }
            case PCM64FLOAT:
                switch(myPCMFormat) {
                    case PCM8_UNSIGNED: return addConvert<double, uint8_t>(theBuffer);
                    case PCM16_SIGNED:  return addConvert<double, int16_t>(theBuffer);
                    case PCM32_SIGNED:  return addConvert<double, int32_t>(theBuffer);
                    case PCM32FLOAT:    return addConvert<double, float  >(theBuffer);
                    case PCM64FLOAT:    return addConvert<double, double >(theBuffer);
                }
        }
        return false;
    } else if(myChMap == theBuffer.myChMap) {
        // fast copy
        for(size_t aPlaneIter = 0; aPlaneIter < myPlanesNb; ++aPlaneIter) {
            stMemCpy(getPlane(aPlaneIter) + myPlaneSize,
                     theBuffer.getPlane(aPlaneIter),
                     theBuffer.getPlaneSize());
        }
        myPlaneSize += theBuffer.getPlaneSize();
        return true;
    } else {
        ST_DEBUG_ASSERT(false);
        return false;
    }
}
