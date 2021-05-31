/**
 * Copyright © 2015 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#ifndef __StAVBufferPool_h_
#define __StAVBufferPool_h_

#include <StAV/stAV.h>

struct AVBufferPool;
struct AVBufferRef;

/**
 * This is just a wrapper over AVBufferPool structure.
 */
class StAVBufferPool {

        public:

    /**
     * Empty constructor
     */
    ST_LOCAL StAVBufferPool()
    : myPool(NULL),
      myBufferSize(0) {
        //
    }

    /**
     * Destructor
     */
    ST_LOCAL ~StAVBufferPool() {
        release();
    }

    /**
     * Release the pool (reference-counted buffer will be released when needed).
     */
    ST_LOCAL void release() {
    #if(LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(55, 45, 101))
        if(myPool != NULL) {
            av_buffer_pool_uninit(&myPool);
            myPool       = NULL;
            myBufferSize = 0;
         }
    #endif
    }

    /**
     * (Re-)initialize the pool.
     */
    ST_LOCAL bool init(const int theBufferSize) {
        if(myBufferSize == theBufferSize) {
            return true;
        }

        release();
        if(theBufferSize == 0) {
            return true;
        }

    #if(LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(55, 45, 101))
        myPool       = av_buffer_pool_init(theBufferSize, NULL);
        myBufferSize = theBufferSize;
        return myPool != NULL;
    #else
        return false;
    #endif
    }

    /**
     * Return buffer size within the pool.
     */
    int getBufferSize() const {
        return myBufferSize;
    }

    /**
     * Get new buffer from the pool.
     */
    ST_LOCAL AVBufferRef* getBuffer() {
    #if(LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(55, 45, 101))
        return av_buffer_pool_get(myPool);
    #else
        return NULL;
    #endif
    }

        private:

    StAVBufferPool(const StAVBufferPool& theCopy);

        private:

    AVBufferPool* myPool;
    int           myBufferSize;

};

#endif // __StAVBufferPool_h_
