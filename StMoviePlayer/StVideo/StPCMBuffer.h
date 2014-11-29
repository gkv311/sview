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

#ifndef __StPCMBuffer_h_
#define __StPCMBuffer_h_

#include <stTypes.h>

#define ST_AUDIO_CHANNELS_MAX 8

/**
 * All sample formats are in native-endian
 */
enum StPcmFormat {
    StPcmFormat_UInt8   = 0, //!< uint8_t
    StPcmFormat_Int16   = 1, //!< int16_t
    StPcmFormat_Int32   = 2, //!< int32_t
    StPcmFormat_Float32 = 3, //!< signed 32-bit float, should be -1.0 .. 1.0
    StPcmFormat_Float64 = 4, //!< signed 64-bit float, should be -1.0 .. 1.0
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
class StChannelMap {

        public:

    typedef enum tagChannels {
        CH10, // mono
        CH20, // stereo
        //CH21, // stereo + LFE
        CH30, // left + right + center
        //CH31,
        CH40,
        //CH41,
        CH50, // 3 Front + 2 Rear
        CH51, // 3 Front + 2 Rear + LFE
        //CH61,
        CH71
    } Channels;

    typedef enum tagOrderRules {
        PCM, //!< used PCM channel order rules
        AC3, //!< used AC3 channel order rules
        OGG  //!< used OGG Vorbis channel order rules
    } OrderRules;

        public: //!< fields are public for simple access

    size_t count; //!< channels number
    Channels channels;

    size_t Order[ST_AUDIO_CHANNELS_MAX];

    size_t FL;  //!< Front Left
    size_t FR;  //!< Front Right
    size_t FC;  //!< Front Center
    size_t LFE; //!< Low Frequency
    size_t RL;  //!< Rear Left
    size_t RR;  //!< Rear Right
    size_t SL;  //!< Side Left
    size_t SR;  //!< Side Right

        public:

    ST_LOCAL StChannelMap(const Channels theChannels, const OrderRules theRules);

    ST_LOCAL bool operator==(const StChannelMap& theOther) const {
        return channels == theOther.channels
            && FL  == theOther.FL
            && FR  == theOther.FR
            && FC  == theOther.FC
            && LFE == theOther.LFE
            && RL  == theOther.RL
            && RR  == theOther.RR
            && SL  == theOther.SL
            && SR  == theOther.SR;
    }

    ST_LOCAL bool operator!=(const StChannelMap& theOther) const {
        return (*this == theOther) == false;
    }

};

/**
 * This class represent Audio PCM buffer.
 */
class StPCMBuffer {

        public:

    /**
     * @param thePCMFormat PCM format
     */
    ST_LOCAL StPCMBuffer(const StPcmFormat thePCMFormat);

    /**
     * Destructor.
     */
    ST_LOCAL ~StPCMBuffer();

    /**
     * Clear data in buffer (nulling), set datasize to zero.
     */
    ST_LOCAL void clear();

    /**
     * Resize buffer to fit bigger packets.
     * @param theSizeMin buffer size in bytes
     */
    ST_LOCAL void resize(const size_t theSizeMin,
                         const bool   theToReduce);

    /**
     * @return one second size in bytes for current format
     */
    ST_LOCAL size_t getSecondSize() const {
        return mySampleSize * myChMap.count * myPCMFreq;
    }

    /**
     * @return data for specified plane
     */
    ST_LOCAL uint8_t* getPlane(const size_t thePlaneId) const {
        return myPlanes[thePlaneId];
    }

    /**
     * @return data size for single plane (should be same for all planes)
     */
    ST_LOCAL size_t getPlaneSize() const {
        return myPlaneSize;
    }

    /**
     * Wrap pointer to the alien plane.
     * Caller should ensure that pointer will remain valid.
     */
    ST_LOCAL void wrapPlane(const size_t thePlaneId,
                            uint8_t*     thePlane) {
        myPlanes[thePlaneId] = thePlane;
    }

    /**
     * @param thePlaneSize data size (should be same for all planes)
     */
    ST_LOCAL void setPlaneSize(const size_t thePlaneSize) {
        myPlaneSize = thePlaneSize;
    }

    /**
     * @return true if there no data in the buffer
     */
    ST_LOCAL bool isEmpty() const {
        return myPlaneSize == 0;
    }

    ST_LOCAL size_t getBufferSizeWhole() const {
        return mySizeBytes;
    }

    ST_LOCAL size_t getDataSizeWhole() const {
        return myPlaneSize * myPlanesNb;
    }

    /**
     * This method always return false for wrapper over alien data.
     * @return true if there is unused allocated space available in own buffer
     */
    ST_LOCAL bool hasDataSize(const size_t thePushDataSize) const {
        return (myPlanes[0] == myBuffer)
            && ((mySizeBytes - myPlaneSize * myPlanesNb) >= thePushDataSize);
    }

    /**
     * @return free memory available for appending data
     */
    ST_LOCAL size_t getFreeDataSize() const {
        if(myPlanes[0] != myBuffer) {
            return 0;
        }
        return mySizeBytes - myPlaneSize * myPlanesNb;
    }

    /**
     * @param theDataSize new data size
     * @return true if setted size correct
     */
    ST_LOCAL bool setDataSize(const size_t theDataSize);

    /**
     * This method is for reading data from the beginning.
     * @param theChannel channel id in PCM order
     * @return pointer to the first sample in specified channel
     */
    template<typename sample_t>
    ST_LOCAL inline void getChannelDataStart(const size_t theChannel,
                                             sample_t*&   thePointer) const {
        const size_t aChannel = myChMap.Order[theChannel];
        thePointer = (myPlanesNb > 1)
                   ? (sample_t* )getPlane(aChannel)
                   : (sample_t* )&getPlane(0)[sizeof(sample_t) * aChannel];
    }

    /**
     * This method is for appending data.
     * @param theChannel channel id in PCM order
     * @return pointer to the first unused sample in specified channel
     */
    template<typename sample_t>
    ST_LOCAL inline void getChannelDataEnd(const size_t theChannel,
                                           sample_t*&   thePointer) const {
        const size_t aChannel = myChMap.Order[theChannel];
        thePointer = (myPlanesNb > 1)
                   ? (sample_t* )&getPlane(aChannel)[myPlaneSize]
                   : (sample_t* )&getPlane(0)[myPlaneSize + sizeof(sample_t) * aChannel];
    }

    /**
     * Add data with remapping and/or conversion.
     */
    template<typename sampleSrc_t, typename sampleOut_t>
    ST_LOCAL bool addConvert(const StPCMBuffer& theBuffer);

    /**
     * Add data buffer.
     */
    ST_LOCAL bool addData(const StPCMBuffer& theBuffer);

    /**
     * This parameter measures how many samples/channel are played each second.
     * Frequency is measured in samples/second (Hz).
     * @return Frequency (Sample Rate)
     */
    ST_LOCAL int getFreq() const {
        return myPCMFreq;
    }

    ST_LOCAL void setFreq(int theFreq) {
        //ST_DEBUG_ASSERT(theFreq > 0);
        myPCMFreq = theFreq;
    }

    ST_LOCAL StPcmFormat getFormat() const {
        return myPCMFormat;
    }

    ST_LOCAL void setFormat(const StPcmFormat thePCMFormat);

    /**
     * @return planes number (1 for interleaved data)
     */
    ST_LOCAL size_t getPlanesNb() const {
        return myPlanesNb;
    }

    /**
     * Initialize buffer configuration.
     * @param theChannels pre-defined channels configuration
     * @param theRules    (re)ordering rules
     * @param thePlanesNb planes number (1 for interleaved data, >=2 for planar data)
     */
    ST_LOCAL void setupChannels(const StChannelMap::Channels   theChannels,
                                const StChannelMap::OrderRules theRules,
                                const size_t                   thePlanesNb) {
        setupChannels(StChannelMap(theChannels, theRules), thePlanesNb);
    }

    /**
     * Initialize buffer configuration.
     * @param theChMap    channels configuration
     * @param thePlanesNb planes number (1 for interleaved data, >=2 for planar data)
     */
    ST_LOCAL void setupChannels(const StChannelMap& theChMap,
                                const size_t        thePlanesNb);

        private:

    uint8_t*     myBuffer;         //!< allocated buffer
    size_t       mySizeBytes;      //!< buffer size in bytes

    uint8_t*     myPlanes[ST_AUDIO_CHANNELS_MAX]; //!< array of planes
    size_t       myPlaneSize;      //!< (data) plane size in bytes, should be same for all planes
    size_t       myPlanesNb;       //!< number of planes (either - 1 for interleaved data or >= 2 for array of mono sources)

    size_t       mySampleSize;     //!< sample size (for 1 channel)
    StPcmFormat  myPCMFormat;      //!< sample format
    int          myPCMFreq;        //!< frequency
    StChannelMap myChMap;          //!< channel order rules

};

#endif // __StPCMBuffer_h_
