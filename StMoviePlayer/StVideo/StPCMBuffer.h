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

#ifndef __StPCMBuffer_h_
#define __StPCMBuffer_h_

#include <stTypes.h>

/**
 * All sample formats are in native-endian
 */
typedef enum tagStPCMformat {
    PCM8_UNSIGNED = 0,
    PCM16_SIGNED = 1,
    PCM32_SIGNED = 2,
    PCM32FLOAT = 3,    // signed, should be -1.0 .. 1.0
    PCM64FLOAT = 4,
} StPCMformat;

template<StPCMformat pcmFormat>
class StPCMformatType;

template<>
class StPCMformatType<(StPCMformat )1> {
    typedef int16_t type;
};

/**
 * Just enumeration for standard frequency values.
 */
typedef enum tagStPCMfreq {
    FREQ_5500 = 5500,
    FREQ_6000 = 6000,
    FREQ_7333 = 7333,
    FREQ_8000 = 8000,
    FREQ_11025 = 11025,
    FREQ_16000 = 16000,
    FREQ_22050 = 22050,
    FREQ_32000 = 32000,
    FREQ_44100 = 44100,
    FREQ_48000 = 48000,
    FREQ_64000 = 64000,
    FREQ_88200 = 88200,
    FREQ_96000 = 96000,
    FREQ_192000 = 192000,
} StPCMfreq;

/**
 * Channel order in interleaved PCM form:
 * 1. Front Left - FL
 * 2. Front Right - FR
 * 3. Front Center - FC
 * 4. Low Frequency - LF
 * 5. Back Left - BL
 * 6. Back Right - BR
 * 7. Front Left of Center - FLC
 * 8. Front Right of Center - FRC
 * 9. Back Center - BC
 * 10.Side Left - SL
 * 11.Side Right - SR
 * 12.Top Center - TC
 * 13.Top Front Left - TFL
 * 14.Top Front Center - TFC
 * 15.Top Front Right - TFR
 * 16.Top Back Left - TBL
 * 17.Top Back Center - TBC
 * 18.Top Back Right - TBR
 **/
class ST_LOCAL StChannelMap {

        public:

    typedef enum tagChannels {
        CH10, // mono
        CH20, // stereo
        //CH21, // stereo + LFE
        //CH30,
        //CH31,
        CH40,
        //CH41,
        CH51, // 3 Front + 2 Rear + LFE
        //CH61,
        //CH71
    } Channels;

    typedef enum tagOrderRules {
        PCM, //!< used PCM channel order rules
        AC3, //!< used AC3 channel order rules
        OGG  //!< used OGG Vorbis channel order rules
    } OrderRules;

        public: //!< fields are public for simple access

    size_t count; //!< channels number
    Channels channels;

    size_t FL;  //!< Front Left
    size_t FR;  //!< Front Right
    size_t FC;  //!< Front Center
    size_t LFE; //!< Low Frequency
    size_t RL;  //!< Rear Left
    size_t RR;  //!< Rear Right

        public:

    StChannelMap(const Channels theChannels, const OrderRules theRules);

    bool operator==(const StChannelMap& theOther) const {
        return channels == theOther.channels
            && FL  == theOther.FL
            && FR  == theOther.FR
            && FC  == theOther.FC
            && LFE == theOther.LFE
            && RL  == theOther.RL
            && RR  == theOther.RR;
    }

    bool operator!=(const StChannelMap& theOther) const {
        return (*this == theOther) == false;
    }

};

/**
 * This class represent Audio PCM buffer.
 */
class ST_LOCAL StPCMBuffer {

        private:

    size_t      mySizeBytes; //!< buffer size in bytes
    size_t  myDataSizeBytes; //!< data (engaged part of buffer) size in bytes
    unsigned char* myBuffer; //!< allocated buffer
    size_t     mySampleSize; //!< sample size (for 1 channel)
    StPCMformat myPCMFormat; //!< sample format
    int           myPCMFreq; //!< frequency
    StChannelMap    myChMap; //!< channel order rules
    size_t      mySourcesNb; //!< ALSA source number

        public:

    /**
     * @param pcmFormat (const StPCMformat ) - PCM format from PCMformat enum;
     * @param theBufferSize (const size_t ) - buffer size in bytes.
     */
    StPCMBuffer(const StPCMformat thePCMFormat,
                const size_t theBufferSize);

    ~StPCMBuffer();

    /**
     * Clear data in buffer (nulling), set datasize to zero.
     */
    void clear();

    /**
     * Only clear data in buffer (nulling), datasize not changed.
     */
    void mute();

    /**
     * @return one second size in bytes for current format.
     */
    size_t getSecondSize() const {
        return mySampleSize * myChMap.count * myPCMFreq;
    }

    unsigned char* getData(size_t theSourceId = 0) const {
        return &myBuffer[theSourceId * (mySizeBytes / mySourcesNb)];
    }

    size_t getBufferSizeWhole() const {
        return mySizeBytes;
    }

    size_t getBufferSize(size_t /*sourceId*/) const {
        return mySizeBytes / mySourcesNb;
    }

    size_t getDataSizeWhole() const {
        return myDataSizeBytes;
    }

    size_t getDataSize(size_t /*sourceId*/) const {
        return myDataSizeBytes / mySourcesNb;
    }

    bool hasDataSize(const size_t thePushDataSize) const {
        return ((mySizeBytes - myDataSizeBytes) >= thePushDataSize);
    }

    /**
     * @param theDataSize (size_t ) - new data size;
     * @return true if setted size correct.
     */
    bool setDataSize(const size_t theDataSize);

    template<typename sampleSrc_t, typename sampleOut_t>
    bool addSplitInterleaved(const StPCMBuffer& theBuffer);

    bool addData(const StPCMBuffer& theBuffer);

    /**
     * This parameter measures how many samples/channel are played each second.
     * Frequency is measured in samples/second (Hz).
     * @return (int ) Frequency (Sample Rate).
     */
    int getFreq() const {
        return myPCMFreq;
    }

    void setFreq(int theFreq) {
        //ST_DEBUG_ASSERT(theFreq > 0);
        myPCMFreq = theFreq;
    }

    StPCMformat getFormat() const {
        return myPCMFormat;
    }

    void setFormat(const StPCMformat thePCMFormat);

    size_t getSourcesCount() const {
        return mySourcesNb;
    }

    void setupChannels(const StChannelMap::Channels theChannels,
                       const StChannelMap::OrderRules theRules,
                       const size_t theSourcesNb) {
        myChMap = StChannelMap(theChannels, theRules);
        mySourcesNb = (theSourcesNb > 0) ? theSourcesNb : 1;
    }

};

#endif //__StPCMBuffer_h_
