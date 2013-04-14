/**
 * Copyright © 2009-2013 Kirill Gavrilov <kirill@sview.ru>
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
class StAVPacket {

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

    ST_CPPEXPORT static void avDestructPacket(AVPacket* thePkt);

        private:

    AVPacket myPacket;
    StHandle<StStereoParams> myStParams;
    double myDurationSec;
    int myType;

    /**
     * Emulates av_init_packet().
     */
    ST_CPPEXPORT void avInitPacket();

        public:

    /**
     * Empty constructor
     */
    ST_CPPEXPORT StAVPacket();

    ST_CPPEXPORT StAVPacket(const StHandle<StStereoParams>& theStParams,
                            const int theType = DATA_PACKET);

    ST_CPPEXPORT StAVPacket(const StAVPacket& theCopy);

    ST_CPPEXPORT ~StAVPacket();

    /**
     * Emulates av_free_packet().
     */
    ST_CPPEXPORT void free();

    inline AVPacket* getAVpkt() {
        return &myPacket;
    }

    ST_CPPEXPORT void setAVpkt(const AVPacket& theCopy);

    inline const StHandle<StStereoParams>& getSource() const {
        return myStParams;
    }

    inline int getType() const {
        return myType;
    }

    inline const uint8_t* getData() const {
        return myPacket.data;
    }

    inline int getSize() const {
        return myPacket.size;
    }

    inline int64_t getPts() const {
        return myPacket.pts;
    }

    inline int64_t getDts() const {
        return myPacket.dts;
    }

    inline int64_t getConvergenceDuration() const {
        return myPacket.convergence_duration;
    }

    inline int getDuration() const {
        return myPacket.duration;
    }

    inline double getDurationSeconds() const {
        return myDurationSec;
    }

    inline void setDurationSeconds(const double theDurationSec) {
        myDurationSec = theDurationSec;
    }

    inline int getStreamId() const {
        return myPacket.stream_index;
    }

    inline bool isKeyFrame() const {
        return myPacket.flags & AV_PKT_FLAG_KEY;
    }

    inline void setKeyFrame() {
        myPacket.flags |= AV_PKT_FLAG_KEY;
    }

    // dummy
    inline bool operator==(const StAVPacket& compare) const {
        return this == &compare;
    }
    inline bool operator!=(const StAVPacket& compare) const {
        return this != &compare;
    }
    inline bool operator>(const StAVPacket& compare) const {
        return this > &compare;
    }
    inline bool operator<(const StAVPacket& compare) const {
        return this < &compare;
    }
    inline bool operator>=(const StAVPacket& compare) const {
        return this >= &compare;
    }
    inline bool operator<=(const StAVPacket& compare) const {
        return this <= &compare;
    }
    inline StString toString() const {
        return StString();
    }

};

#endif //__StAVPacket_h_
