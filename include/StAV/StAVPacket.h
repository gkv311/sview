/**
 * Copyright © 2009-2025 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#ifndef __StAVPacket_h_
#define __StAVPacket_h_

#include <StAV/stAV.h>

#include <StGL/StParams.h>

#include <memory>

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
        LAST_PACKET,
        END_PACKET,
        QUIT_PACKET,
    };

    #ifndef AV_PKT_FLAG_KEY
        #define AV_PKT_FLAG_KEY     0x0001 ///< The packet contains a keyframe
        #define AV_PKT_FLAG_CORRUPT 0x0002 ///< The packet content is corrupted
    #endif

    ST_CPPEXPORT static void avDestructPacket(AVPacket* thePkt);

        public:

    /**
     * Empty constructor
     */
    ST_CPPEXPORT StAVPacket();

    ST_CPPEXPORT StAVPacket(const std::shared_ptr<StStereoParams>& theStParams,
                            const int theType = DATA_PACKET);

    ST_CPPEXPORT StAVPacket(const StAVPacket& theCopy);

    ST_CPPEXPORT ~StAVPacket();

    /**
     * Emulates av_free_packet().
     */
    ST_CPPEXPORT void free();

    ST_LOCAL AVPacket* getAVpkt() {
        return &myPacket;
    }

    ST_CPPEXPORT void setAVpkt(const AVPacket& theCopy);

    ST_LOCAL const std::shared_ptr<StStereoParams>& getSource() const {
        return myStParams;
    }

    ST_LOCAL int getType() const {
        return myType;
    }

    ST_LOCAL const uint8_t* getData() const {
        return myPacket.data;
    }

    ST_LOCAL uint8_t* changeData() {
        return myPacket.data;
    }

    ST_LOCAL int getSize() const {
        return myPacket.size;
    }

    ST_LOCAL int64_t getPts() const {
        return myPacket.pts;
    }

    ST_LOCAL int64_t getDts() const {
        return myPacket.dts;
    }

    ST_LOCAL int64_t getConvergenceDuration() const {
        return myPacket.duration;
    }

    ST_LOCAL int64_t getDuration() const {
        return myPacket.duration;
    }

    ST_LOCAL double getDurationSeconds() const {
        return myDurationSec;
    }

    ST_LOCAL void setDurationSeconds(const double theDurationSec) {
        myDurationSec = theDurationSec;
    }

    ST_LOCAL int getStreamId() const {
        return myPacket.stream_index;
    }

    ST_LOCAL bool isKeyFrame() const {
        return myPacket.flags & AV_PKT_FLAG_KEY;
    }

    ST_LOCAL void setKeyFrame() {
        myPacket.flags |= AV_PKT_FLAG_KEY;
    }

    // dummy
    ST_LOCAL bool operator==(const StAVPacket& compare) const {
        return this == &compare;
    }
    ST_LOCAL bool operator!=(const StAVPacket& compare) const {
        return this != &compare;
    }
    ST_LOCAL bool operator>(const StAVPacket& compare) const {
        return this > &compare;
    }
    ST_LOCAL bool operator<(const StAVPacket& compare) const {
        return this < &compare;
    }
    ST_LOCAL bool operator>=(const StAVPacket& compare) const {
        return this >= &compare;
    }
    ST_LOCAL bool operator<=(const StAVPacket& compare) const {
        return this <= &compare;
    }
    ST_LOCAL StString toString() const {
        return StString();
    }

        private:

    /**
     * Emulates av_init_packet().
     */
    ST_CPPEXPORT void avInitPacket();

        private:

    AVPacket myPacket;

    std::shared_ptr<StStereoParams> myStParams;

    double myDurationSec = 0.0;
    int    myType = DATA_PACKET;
    bool   myIsOwn = false;

};

#endif //__StAVPacket_h_
