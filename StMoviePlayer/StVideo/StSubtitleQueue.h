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
class ST_LOCAL StSubtitleQueue : public StAVPacketQueue {

        private:

    StHandle<StSubQueue> myOutQueue;
    StThread*   myThread; //!< decoding loop thread
    StSubtitlesASS myASS; //!< ASS subtitles parser
    StEvent   evDowntime;
    volatile bool toQuit;

        public:

    bool isInDowntime() {
        return evDowntime.check();
    }

    StSubtitleQueue(const StHandle<StSubQueue>& theSubtitlesQueue);
    virtual ~StSubtitleQueue();

    /**
     * Initialization function.
     * @param theFormatCtx (AVFormatContext* ) - pointer to video format context;
     * @param streamId (const unsigned int )   - stream id in video format context;
     * @return true if no error.
     */
    bool init(AVFormatContext*   theFormatCtx,
              const unsigned int theStreamId);

    /**
     * Clean function.
     */
    void deinit();

    /**
     * Main decoding loop.
     */
    void decodeLoop();

};

#endif //__StSubtitleQueue_h_
