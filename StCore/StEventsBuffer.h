/**
 * StCore, window system independent C++ toolkit for writing OpenGL applications.
 * Copyright © 2007-2013 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#ifndef __StEventsBuffer_h_
#define __StEventsBuffer_h_

#include <StCore/StEvent.h>

/**
 * Double buffer for StWindow callback events.
 * One buffer is read-only (could be read by StWindow thread without lock)
 * and another one is write-only (access is protected by mutex).
 * Swap method reverse buffers mission.
 *
 * Current implementation is lossy - buffers created with limited size
 * and if not swaped in time, new events will be lost.
 */
class StEventsBuffer {

        public:

    static const size_t BUFFER_SIZE = 2048U;

        public:

    /**
     * Create empty buffer.
     */
    ST_LOCAL StEventsBuffer()
    : myEventsRead (new StEvent[BUFFER_SIZE]),
      myEventsWrite(new StEvent[BUFFER_SIZE]),
      mySizeRead (0),
      mySizeWrite(0) {
        //
    }

    /**
     * Destructor.
     */
    ST_LOCAL ~StEventsBuffer() {
        // release dynamically allocated resources
        swapBuffers();
        delete[] myEventsRead;
        delete[] myEventsWrite;
    }

    /**
     * Reset both buffers.
     */
    ST_LOCAL void reset() {
        StMutexAuto aLock(myMutex);
        swapBuffers();
        mySizeRead  = 0;
        mySizeWrite = 0;
    }

    /**
     * @return number of events in read-only buffer
     */
    ST_LOCAL size_t getSize() const {
        return mySizeRead;
    }

    /**
     * @param theId Index of event to retrieve
     * @return event from read-only buffer
     */
    ST_LOCAL const StEvent& getEvent(const size_t theId) const {
        return myEventsRead[theId];
    }

    /**
     * @param theId Index of event to retrieve
     * @return event from read-only buffer
     */
    ST_LOCAL StEvent& changeEvent(const size_t theId) {
        return myEventsRead[theId];
    }

    /**
     * Append one more event to the write buffer.
     */
    ST_LOCAL void append(const StEvent& theEvent) {
        StMutexAuto aLock(myMutex);
        if(mySizeWrite >= BUFFER_SIZE) {
            return;
        }

        StEvent& anEvent = myEventsWrite[mySizeWrite++];
        anEvent = theEvent;
        if(theEvent.Type == stEvent_FileDrop) {
            if(theEvent.DNDrop.NbFiles == 0) {
                anEvent.DNDrop.Files = NULL;
                return;
            }

            // make a copy in C-style
            anEvent.DNDrop.Files = stMemAlloc<const char**>(sizeof(const char* ) * theEvent.DNDrop.NbFiles);
            if(anEvent.DNDrop.Files == NULL) {
                anEvent.DNDrop.NbFiles = 0;
                return;
            }

            stMemZero(anEvent.DNDrop.Files, sizeof(const char* ) * theEvent.DNDrop.NbFiles);
            for(uint32_t aFileIter = 0; aFileIter < theEvent.DNDrop.NbFiles; ++aFileIter) {
                const char*  aBufferSrc = theEvent.DNDrop.Files[aFileIter];
                const size_t aSize      = std::strlen(aBufferSrc);
                char*        aBufferDst = stMemAlloc<char*>(sizeof(char) * aSize + 1);
                if(aBufferDst == NULL) {
                    anEvent.DNDrop.NbFiles = aFileIter;
                    return;
                }

                stMemCpy(aBufferDst, aBufferSrc, aSize);
                aBufferDst[aSize] = '\0';
                anEvent.DNDrop.Files[aFileIter] = aBufferDst;
            }
        }
    }

    /**
     * Swap read/write buffers. Write buffer become empty as result.
     */
    ST_LOCAL void swapBuffers() {
        // release dynamically allocated resources
        for(size_t anIter = 0; anIter < mySizeRead; ++anIter) {
            StEvent& anEvent = myEventsRead[anIter];
            if(anEvent.Type == stEvent_FileDrop) {
                for(uint32_t aFileIter = 0; aFileIter < anEvent.DNDrop.NbFiles; ++aFileIter) {
                    stMemFree((void* )anEvent.DNDrop.Files[aFileIter]);
                }
                stMemFree(anEvent.DNDrop.Files);
                anEvent.DNDrop.Files   = NULL;
                anEvent.DNDrop.NbFiles = 0;
            }
        }

        StMutexAuto aLock(myMutex);
        std::swap(myEventsRead, myEventsWrite);
        mySizeRead  = mySizeWrite;
        mySizeWrite = 0;
    }

        private: //! @name private fields

    StMutex  myMutex;       //!< mutex for thread-safe access
    StEvent* myEventsRead;  //!< read-only  events buffer, could be accessed by StWindow thread without lock
    StEvent* myEventsWrite; //!< write-only events buffer, each insertion operation is protected with mutex
    size_t   mySizeRead;    //!< number of events in read-only  buffer
    size_t   mySizeWrite;   //!< number of events in write-only buffer

};

#endif // __StEventsBuffer_h_
