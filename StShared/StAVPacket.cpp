/**
 * Copyright © 2009-2012 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#include <StAVPacket.h>

void StAVPacket::avDestructPacket(AVPacket* thePkt) {
    // use own deallocation
    stMemFreeAligned(thePkt->data);
    thePkt->data = NULL;
    thePkt->size = 0;

#if(LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(52, 118, 0))
    for(int anIter = 0; anIter < thePkt->side_data_elems; ++anIter) {
        stMemFreeAligned(thePkt->side_data[anIter].data);
    }
    stMemFreeAligned(thePkt->side_data);
    thePkt->side_data       = NULL;
    thePkt->side_data_elems = 0;
#endif
}

void StAVPacket::avInitPacket() {
    stMemSet(&myPacket, 0, sizeof(AVPacket));
    myPacket.pts = stLibAV::NOPTS_VALUE;
    myPacket.dts = stLibAV::NOPTS_VALUE;
    myPacket.pos = -1;
    /*myPacket.duration = 0;
    myPacket.convergence_duration = 0;
    myPacket.flags = 0;
    myPacket.stream_index = 0;
    myPacket.destruct = NULL; //av_destruct_packet_nofree;
    myPacket.data = NULL;
    myPacket.size = 0;
    myPacket.side_data = NULL;
    myPacket.side_data_elems = 0;*/
}

StAVPacket::StAVPacket()
: myStParams(),
  myDurationSec(0.0),
  myType(DATA_PACKET) {
    avInitPacket();
}

StAVPacket::StAVPacket(const StHandle<StStereoParams>& theStParams,
                       const int theType)
: myStParams(theStParams),
  myDurationSec(0.0),
  myType(theType) {
    avInitPacket();
}

StAVPacket::StAVPacket(const StAVPacket& theCopy)
: myStParams(theCopy.myStParams),
  myDurationSec(theCopy.myDurationSec),
  myType(theCopy.myType) {
    avInitPacket();
    if(myType == DATA_PACKET) {
        setAVpkt(theCopy.myPacket);
    }
}

StAVPacket::~StAVPacket() {
    free();
}

void StAVPacket::free() {
    if(myPacket.destruct != NULL) {
        myPacket.destruct(&myPacket);
    }
    myPacket.data = NULL;
    myPacket.size = 0;
}

void StAVPacket::setAVpkt(const AVPacket& theCopy) {
    // free old data
    free();

    // copy values
    myPacket = theCopy;

    if(theCopy.data != NULL) {
        // now copy data with special padding space
        myPacket.data = stMemAllocAligned<uint8_t*>((theCopy.size + FF_INPUT_BUFFER_PADDING_SIZE), 16); // data must be aligned to 16 bytes for SSE!
        stMemCpy(myPacket.data, theCopy.data, theCopy.size);
        stMemSet(myPacket.data + (ptrdiff_t )theCopy.size, 0, FF_INPUT_BUFFER_PADDING_SIZE);

        // set our own deallocate function
        myPacket.destruct = avDestructPacket;
    }

#if(LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(52, 118, 0))
    if(myPacket.side_data_elems > 0) {
        size_t aSize = theCopy.side_data_elems * sizeof(*theCopy.side_data);
        // weird anonymouse structure...
        uint8_t** aPtr = (uint8_t** )&myPacket.side_data;
        *aPtr = stMemAllocZeroAligned<uint8_t*>(aSize, 16);
        for(int anIter = 0; anIter < theCopy.side_data_elems; ++anIter) {
            aSize = theCopy.side_data[anIter].size;
            myPacket.side_data[anIter].data = stMemAllocAligned<uint8_t*>(aSize + FF_INPUT_BUFFER_PADDING_SIZE, 16);
            stMemCpy(myPacket.side_data[anIter].data, theCopy.side_data[anIter].data, aSize);
            stMemSet(myPacket.side_data[anIter].data + (ptrdiff_t )aSize, 0, FF_INPUT_BUFFER_PADDING_SIZE);
        }
    }
#endif
}
