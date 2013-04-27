/**
 * Copyright © 2011 Kirill Gavrilov <kirill@sview.ru>
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

#ifndef __StSubtitleQueue_h_
#define __StSubtitleQueue_h_

#include <StGLStereo/StGLTextureQueue.h>

#include "StAVPacketQueue.h"
#include "StSubtitlesASS.h"

// forward declarations
class StSubQueue;
class StSubtitleQueue;

// define StHandle template specialization
ST_DEFINE_HANDLE(StSubtitleQueue, StAVPacketQueue);

/**
 * Subtitles decoding thread.
 */
class StSubtitleQueue : public StAVPacketQueue {

        public:

    ST_LOCAL bool isInDowntime() {
        return evDowntime.check();
    }

    ST_LOCAL StSubtitleQueue(const StHandle<StSubQueue>& theSubtitlesQueue);
    ST_LOCAL virtual ~StSubtitleQueue();

    ST_LOCAL inline const StHandle<StSubQueue>& getSubtitlesQueue() const {
        return myOutQueue;
    }

    /**
     * Initialization function.
     * @param theFormatCtx pointer to video format context
     * @param streamId     stream id in video format context
     * @return true if no error
     */
    ST_LOCAL bool init(AVFormatContext*   theFormatCtx,
                       const unsigned int theStreamId);

    /**
     * Clean function.
     */
    ST_LOCAL void deinit();

    /**
     * Main decoding loop.
     */
    ST_LOCAL void decodeLoop();

        private:

    StHandle<StSubQueue> myOutQueue;
    StThread*            myThread;   //!< decoding loop thread
    StSubtitlesASS       myASS;      //!< ASS subtitles parser
    StEvent              evDowntime;
    volatile bool        toQuit;

};

#endif //__StSubtitleQueue_h_
