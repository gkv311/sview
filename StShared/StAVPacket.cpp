/**
 * Copyright © 2009-2013 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#include <StAV/StAVPacket.h>

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
    stMemZero(&myPacket, sizeof(AVPacket));
    myPacket.pts = stAV::NOPTS_VALUE;
    myPacket.dts = stAV::NOPTS_VALUE;
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
  myType(DATA_PACKET),
  myIsOwn(false) {
    avInitPacket();
}

StAVPacket::StAVPacket(const StHandle<StStereoParams>& theStParams,
                       const int theType)
: myStParams(theStParams),
  myDurationSec(0.0),
  myType(theType),
  myIsOwn(false) {
    avInitPacket();
}

StAVPacket::StAVPacket(const StAVPacket& theCopy)
: myStParams(theCopy.myStParams),
  myDurationSec(theCopy.myDurationSec),
  myType(theCopy.myType),
  myIsOwn(false) {
    avInitPacket();
    if(myType == DATA_PACKET) {
    #if(LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(57, 106, 102))
        av_packet_ref(&myPacket, &theCopy.myPacket); // copy by reference
    #else
        setAVpkt(theCopy.myPacket);
    #endif
    }
}

StAVPacket::~StAVPacket() {
    free();
}

void StAVPacket::free() {
    if(myIsOwn) {
        avDestructPacket(&myPacket);
        avInitPacket();
    } else {
    #if(LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(57, 12, 100))
        av_packet_unref(&myPacket);
    #elif(LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(55, 0, 0))
        av_free_packet(&myPacket);
        avInitPacket();
    #else
        if(myPacket.destruct != NULL) {
            myPacket.destruct(&myPacket);
            myPacket.destruct = NULL;
        }
        avInitPacket();
    #endif
    }
    myIsOwn = false;
}

void StAVPacket::setAVpkt(const AVPacket& theCopy) {
    // free old data
    free();
    if(theCopy.data == NULL) {
        return;
    }

    // copy values
    myIsOwn  = true;
    myPacket = theCopy;
#if(LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(55, 0, 0))
    myPacket.buf = NULL;
#endif

    // now copy data with special padding space
    myPacket.data = stMemAllocAligned<uint8_t*>((theCopy.size + AV_INPUT_BUFFER_PADDING_SIZE), 16); // data must be aligned to 16 bytes for SSE!
    stMemCpy (myPacket.data, theCopy.data, theCopy.size);
    stMemZero(myPacket.data + (ptrdiff_t )theCopy.size, AV_INPUT_BUFFER_PADDING_SIZE);

#if(LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(52, 118, 0))
    if(myPacket.side_data_elems > 0) {
        size_t aSize = theCopy.side_data_elems * sizeof(*theCopy.side_data);
        // weird anonymouse structure...
        uint8_t** aPtr = (uint8_t** )&myPacket.side_data;
        *aPtr = stMemAllocZeroAligned<uint8_t*>(aSize, 16);
        for(int anIter = 0; anIter < theCopy.side_data_elems; ++anIter) {
            aSize = theCopy.side_data[anIter].size;
            myPacket.side_data[anIter] = theCopy.side_data[anIter];
            myPacket.side_data[anIter].data = stMemAllocAligned<uint8_t*>(aSize + AV_INPUT_BUFFER_PADDING_SIZE, 16);
            stMemCpy (myPacket.side_data[anIter].data, theCopy.side_data[anIter].data, aSize);
            stMemZero(myPacket.side_data[anIter].data + (ptrdiff_t )aSize, AV_INPUT_BUFFER_PADDING_SIZE);
        }
    }
#endif
}
