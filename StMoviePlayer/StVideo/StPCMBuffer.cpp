/**
 * Copyright © 2009-2011 Kirill Gavrilov <kirill@sview.ru>
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

#include <StStrings/StLogger.h>

StChannelMap::StChannelMap(const StChannelMap::Channels theChannels,
                           const StChannelMap::OrderRules theRules)
: count(0),
  channels(theChannels) {
    switch(theChannels) {
        case StChannelMap::CH10:
            FL = FR = FC = LFE = RL = RR = 0;
            count = 1;
            return;
        case StChannelMap::CH20:
            FL = FC = LFE = RL = RR = 0; FR = 1;
            count = 2;
            return;
        case StChannelMap::CH40:
            count = 4;
            FL = 0;
            FR = 1;
            RL = 4;
            RR = 5;
            FC = LFE = 0;
            return;
        case StChannelMap::CH51:
            count = 6;
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
                    return;
                }
            }
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

StPCMBuffer::StPCMBuffer(const StPCMformat thePCMFormat,
                         const size_t theBufferSize)
: mySizeBytes(theBufferSize),
  myDataSizeBytes(0),
  myBuffer(NULL),
  mySampleSize(0),
  myPCMFormat(thePCMFormat),
  myPCMFreq(FREQ_44100),
  myChMap(StChannelMap::CH10, StChannelMap::PCM),
  mySourcesNb(1) {
    myBuffer = stMemAllocAligned<unsigned char*>(mySizeBytes, 16); // data must be aligned to 16 bytes for SSE!
    stMemSet(myBuffer, 0, mySizeBytes);
    setFormat(thePCMFormat);
}

StPCMBuffer::~StPCMBuffer() {
    stMemFreeAligned(myBuffer);
}

void StPCMBuffer::clear() {
    myDataSizeBytes = 0;
    stMemSet(myBuffer, 0, mySizeBytes);
}

void StPCMBuffer::mute() {
    if(myDataSizeBytes > 0) {
        stMemSet(myBuffer, 0, myDataSizeBytes);
    }
}

bool StPCMBuffer::setDataSize(const size_t theDataSize) {
    myDataSizeBytes = (theDataSize > mySizeBytes) ? 0 : theDataSize;
    if(myDataSizeBytes < mySizeBytes) {
        myBuffer[mySizeBytes - 1] = 0; // we set end to zero for avcodec_decode_audio2
                                       // to prevent overflow on some broken streams...
    }
    return myDataSizeBytes != 0;
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
bool StPCMBuffer::addSplitInterleaved(const StPCMBuffer& theBuffer) {
    if(mySourcesNb > 1 && mySourcesNb != myChMap.count) {
        // currently only split into mono sources supported
        ST_DEBUG_ASSERT(false);
        return false;
    } else if(theBuffer.myDataSizeBytes < theBuffer.mySampleSize * mySourcesNb) {
        // just ignore
        return true;
    }

    size_t aSamplesSrcCount = theBuffer.myDataSizeBytes / theBuffer.mySampleSize;
    const sampleSrc_t* aBufferSrc = (const sampleSrc_t* )theBuffer.myBuffer;

    size_t aSmplSrcInc = myChMap.count;
    size_t aSmplOutInc = (mySourcesNb > 1) ? 1 : myChMap.count;

    // capture the start pointer for each channel.
    // we don't care here for invalid pointers (will not be used later)...
    sampleOut_t* aBufferOutFL = (mySourcesNb > 1)
        ? (sampleOut_t* )&getData(myChMap.FL )[getDataSize(myChMap.FL)]
        : (sampleOut_t* )&myBuffer[myDataSizeBytes + sizeof(sampleOut_t) * myChMap.FL];
    sampleOut_t* aBufferOutFC = (mySourcesNb > 1)
        ? (sampleOut_t* )&getData(myChMap.FC )[getDataSize(myChMap.FC)]
        : (sampleOut_t* )&myBuffer[myDataSizeBytes + sizeof(sampleOut_t) * myChMap.FC];
    sampleOut_t* aBufferOutFR = (mySourcesNb > 1)
        ? (sampleOut_t* )&getData(myChMap.FR )[getDataSize(myChMap.FR)]
        : (sampleOut_t* )&myBuffer[myDataSizeBytes + sizeof(sampleOut_t) * myChMap.FR];
    sampleOut_t* aBufferOutRL = (mySourcesNb > 1)
        ? (sampleOut_t* )&getData(myChMap.RL )[getDataSize(myChMap.RL)]
        : (sampleOut_t* )&myBuffer[myDataSizeBytes + sizeof(sampleOut_t) * myChMap.RL];
    sampleOut_t* aBufferOutRR = (mySourcesNb > 1)
        ? (sampleOut_t* )&getData(myChMap.RR )[getDataSize(myChMap.RR)]
        : (sampleOut_t* )&myBuffer[myDataSizeBytes + sizeof(sampleOut_t) * myChMap.RR];
    sampleOut_t* aBufferOutLFE= (mySourcesNb > 1)
        ? (sampleOut_t* )&getData(myChMap.LFE)[getDataSize(myChMap.LFE)]
        : (sampleOut_t* )&myBuffer[myDataSizeBytes + sizeof(sampleOut_t) * myChMap.LFE];

    switch(myChMap.channels) {
        case StChannelMap::CH10: {
            for(size_t sampleSrcId(0), sampleOutId(0); sampleSrcId < aSamplesSrcCount; sampleSrcId += aSmplSrcInc, sampleOutId += aSmplOutInc) {
                sampleConv(aBufferSrc[sampleSrcId + theBuffer.myChMap.FL], aBufferOutFL[sampleOutId]);
            }
            myDataSizeBytes += mySampleSize * aSamplesSrcCount;
            return true;
        }
        case StChannelMap::CH20: {
            for(size_t sampleSrcId(0), sampleOutId(0); sampleSrcId < aSamplesSrcCount; sampleSrcId += aSmplSrcInc, sampleOutId += aSmplOutInc) {
                sampleConv(aBufferSrc[sampleSrcId + theBuffer.myChMap.FL], aBufferOutFL[sampleOutId]);
                sampleConv(aBufferSrc[sampleSrcId + theBuffer.myChMap.FR], aBufferOutFR[sampleOutId]);
            }
            myDataSizeBytes += mySampleSize * aSamplesSrcCount;
            return true;
        }
        case StChannelMap::CH40: {
            for(size_t sampleSrcId(0), sampleOutId(0); sampleSrcId < aSamplesSrcCount; sampleSrcId += aSmplSrcInc, sampleOutId += aSmplOutInc) {
                sampleConv(aBufferSrc[sampleSrcId + theBuffer.myChMap.FL], aBufferOutFL[sampleOutId]);
                sampleConv(aBufferSrc[sampleSrcId + theBuffer.myChMap.FR], aBufferOutFR[sampleOutId]);
                sampleConv(aBufferSrc[sampleSrcId + theBuffer.myChMap.RL], aBufferOutRL[sampleOutId]);
                sampleConv(aBufferSrc[sampleSrcId + theBuffer.myChMap.RR], aBufferOutRR[sampleOutId]);
            }
            myDataSizeBytes += mySampleSize * aSamplesSrcCount;
            return true;
        }
        case StChannelMap::CH51: {
            for(size_t sampleSrcId(0), sampleOutId(0); sampleSrcId < aSamplesSrcCount; sampleSrcId += aSmplSrcInc, sampleOutId += aSmplOutInc) {
                sampleConv(aBufferSrc[sampleSrcId + theBuffer.myChMap.FL ], aBufferOutFL [sampleOutId]);
                sampleConv(aBufferSrc[sampleSrcId + theBuffer.myChMap.FC ], aBufferOutFC [sampleOutId]);
                sampleConv(aBufferSrc[sampleSrcId + theBuffer.myChMap.FR ], aBufferOutFR [sampleOutId]);
                sampleConv(aBufferSrc[sampleSrcId + theBuffer.myChMap.RL ], aBufferOutRL [sampleOutId]);
                sampleConv(aBufferSrc[sampleSrcId + theBuffer.myChMap.RR ], aBufferOutRR [sampleOutId]);
                sampleConv(aBufferSrc[sampleSrcId + theBuffer.myChMap.LFE], aBufferOutLFE[sampleOutId]);
            }
            myDataSizeBytes += mySampleSize * aSamplesSrcCount;
            return true;
        }
        default:
            return false;
    }
}

bool StPCMBuffer::addData(const StPCMBuffer& theBuffer) {
    if(theBuffer.myDataSizeBytes == 0 || !hasDataSize(theBuffer.myDataSizeBytes)) {
        return false;
    }
    if(myChMap.count != theBuffer.myChMap.count) {
        // currently not supported
        ST_DEBUG_ASSERT(false);
        return false;
    }
    if(mySourcesNb > 1 || myChMap != theBuffer.myChMap || myPCMFormat != theBuffer.myPCMFormat) {
        // split interleaved data or remap channel order
        switch(theBuffer.myPCMFormat) {
            case PCM8_UNSIGNED:
                switch(myPCMFormat) {
                    case PCM8_UNSIGNED: return addSplitInterleaved<uint8_t, uint8_t>(theBuffer);
                    case PCM16_SIGNED:  return addSplitInterleaved<uint8_t, int16_t>(theBuffer);
                    case PCM32_SIGNED:  return addSplitInterleaved<uint8_t, int32_t>(theBuffer);
                    case PCM32FLOAT:    return addSplitInterleaved<uint8_t, float  >(theBuffer);
                    case PCM64FLOAT:    return addSplitInterleaved<uint8_t, double >(theBuffer);
                }
            case PCM16_SIGNED:
                switch(myPCMFormat) {
                    case PCM8_UNSIGNED: return addSplitInterleaved<int16_t, uint8_t>(theBuffer);
                    case PCM16_SIGNED:  return addSplitInterleaved<int16_t, int16_t>(theBuffer);
                    case PCM32_SIGNED:  return addSplitInterleaved<int16_t, int32_t>(theBuffer);
                    case PCM32FLOAT:    return addSplitInterleaved<int16_t, float  >(theBuffer);
                    case PCM64FLOAT:    return addSplitInterleaved<int16_t, double >(theBuffer);
                }
            case PCM32_SIGNED:
                switch(myPCMFormat) {
                    case PCM8_UNSIGNED: return addSplitInterleaved<int32_t, uint8_t>(theBuffer);
                    case PCM16_SIGNED:  return addSplitInterleaved<int32_t, int16_t>(theBuffer);
                    case PCM32_SIGNED:  return addSplitInterleaved<int32_t, int32_t>(theBuffer);
                    case PCM32FLOAT:    return addSplitInterleaved<int32_t, float  >(theBuffer);
                    case PCM64FLOAT:    return addSplitInterleaved<int32_t, double >(theBuffer);
                }
            case PCM32FLOAT:
                switch(myPCMFormat) {
                    case PCM8_UNSIGNED: return addSplitInterleaved<float, uint8_t>(theBuffer);
                    case PCM16_SIGNED:  return addSplitInterleaved<float, int16_t>(theBuffer);
                    case PCM32_SIGNED:  return addSplitInterleaved<float, int32_t>(theBuffer);
                    case PCM32FLOAT:    return addSplitInterleaved<float, float  >(theBuffer);
                    case PCM64FLOAT:    return addSplitInterleaved<float, double >(theBuffer);
                }
            case PCM64FLOAT:
                switch(myPCMFormat) {
                    case PCM8_UNSIGNED: return addSplitInterleaved<double, uint8_t>(theBuffer);
                    case PCM16_SIGNED:  return addSplitInterleaved<double, int16_t>(theBuffer);
                    case PCM32_SIGNED:  return addSplitInterleaved<double, int32_t>(theBuffer);
                    case PCM32FLOAT:    return addSplitInterleaved<double, float  >(theBuffer);
                    case PCM64FLOAT:    return addSplitInterleaved<double, double >(theBuffer);
                }
        }
        return false;
    } else if(myChMap == theBuffer.myChMap) {
        stMemCpy(&myBuffer[myDataSizeBytes], theBuffer.myBuffer, theBuffer.myDataSizeBytes);
        myDataSizeBytes += theBuffer.myDataSizeBytes;
        return true;
    } else {
        ST_DEBUG_ASSERT(false);
        return false;
    }
}
