/**
 * Copyright © 2009-2012 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#ifndef __StAVPacket_h_
#define __StAVPacket_h_

#include <StLibAV.h>

#include <StGL/StParams.h>

/**
 * This is just a wrapper to AVPacket structure
 * with some useful copy functionality inside.
 */
class ST_LOCAL StAVPacket {

        public:

    enum {
        DATA_PACKET,
        FLUSH_PACKET,
        START_PACKET,
        END_PACKET,
        QUIT_PACKET,
    };

    #ifndef AV_PKT_FLAG_KEY
        #define AV_PKT_FLAG_KEY     0x0001 ///< The packet contains a keyframe
        #define AV_PKT_FLAG_CORRUPT 0x0002 ///< The packet content is corrupted
    #endif

    static void avDestructPacket(AVPacket* thePkt);

        private:

    AVPacket myPacket;
    StHandle<StStereoParams> myStParams;
    double myDurationSec;
    int myType;

    /**
     * Emulates av_init_packet().
     */
    void avInitPacket();

        public:

    /**
     * Empty constructor
     */
    StAVPacket();

    StAVPacket(const StHandle<StStereoParams>& theStParams,
               const int theType = DATA_PACKET);

    StAVPacket(const StAVPacket& theCopy);

    ~StAVPacket();

    /**
     * Emulates av_free_packet().
     */
    void free();

    AVPacket* getAVpkt() {
        return &myPacket;
    }

    void setAVpkt(const AVPacket& theCopy);

    const StHandle<StStereoParams>& getSource() const {
        return myStParams;
    }

    int getType() const {
        return myType;
    }

    const uint8_t* getData() const {
        return myPacket.data;
    }

    int getSize() const {
        return myPacket.size;
    }

    int64_t getPts() const {
        return myPacket.pts;
    }

    int64_t getDts() const {
        return myPacket.dts;
    }

    int64_t getConvergenceDuration() const {
        return myPacket.convergence_duration;
    }

    int getDuration() const {
        return myPacket.duration;
    }

    double getDurationSeconds() const {
        return myDurationSec;
    }

    void setDurationSeconds(const double theDurationSec) {
        myDurationSec = theDurationSec;
    }

    int getStreamId() const {
        return myPacket.stream_index;
    }

    bool isKeyFrame() const {
        return myPacket.flags & AV_PKT_FLAG_KEY;
    }

    void setKeyFrame() {
        myPacket.flags |= AV_PKT_FLAG_KEY;
    }

    // dummy
    bool operator==(const StAVPacket& compare) const {
        return this == &compare;
    }
    bool operator!=(const StAVPacket& compare) const {
        return this != &compare;
    }
    bool operator>(const StAVPacket& compare) const {
        return this > &compare;
    }
    bool operator<(const StAVPacket& compare) const {
        return this < &compare;
    }
    bool operator>=(const StAVPacket& compare) const {
        return this >= &compare;
    }
    bool operator<=(const StAVPacket& compare) const {
        return this <= &compare;
    }
    StString toString() const {
        return StString();
    }

};

#endif //__StAVPacket_h_
