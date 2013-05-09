/**
 * Copyright © 2007-2013 Kirill Gavrilov <kirill@sview.ru>
 *
 * StCore library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * StCore library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.
 * If not, see <http://www.gnu.org/licenses/>.
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
            // copy buffer
            const size_t aSize = std::strlen(theEvent.DNDrop.File);
            char* aBuffer = new char[aSize + 1];
            stMemCpy(aBuffer, theEvent.DNDrop.File, aSize);
            aBuffer[aSize] = '\0';
            anEvent.DNDrop.File = aBuffer;
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
                delete[] anEvent.DNDrop.File;
                anEvent.DNDrop.File = NULL;
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
