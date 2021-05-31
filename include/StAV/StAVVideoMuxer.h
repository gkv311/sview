/**
 * Copyright © 2015 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#ifndef __StAVVideoMuxer_h_
#define __StAVVideoMuxer_h_

#include <StGLStereo/StFormatEnum.h>
#include <StSlots/StSignal.h>
#include <StTemplates/StArrayList.h>

struct AVFormatContext;
struct AVCodecContext;
struct AVCodec;
struct AVFrame;
struct AVStream;

/**
 * This class implements video re-muxing operation using libav* libraries.
 */
class StAVVideoMuxer {

        protected:

    struct StRemuxContext {
        AVFormatContext*          Context;
        bool                      State;
        StArrayList<unsigned int> Streams;

        StRemuxContext() : Context(NULL), State(true) {}
    };

        public:

    /**
     * Initialize empty image.
     */
    ST_CPPEXPORT StAVVideoMuxer();

    /**
     * Destructor.
     */
    ST_CPPEXPORT virtual ~StAVVideoMuxer();

    /**
     * Close currently opened files.
     */
    ST_CPPEXPORT void close();

    /**
     * Return stereo format.
     */
    ST_LOCAL StFormat getStereoFormat() const { return myStereoFormat; }

    /**
     * Set stereo format.
     */
    ST_LOCAL void setStereoFormat(const StFormat theStereoFormat) { myStereoFormat = theStereoFormat; }

    /**
     * Add input file.
     */
    ST_CPPEXPORT bool addFile(const StString& theFileToLoad);

    /**
     * Save to the file.
     */
    ST_CPPEXPORT virtual bool save(const StString& theFile);

        public: //! @name signals

    /**
     * All callback handlers should be thread-safe.
     */
    struct {
        /**
         * Emit callback Slot on error.
         * @param theUserData (const StString& ) - error description.
         */
        StSignal<void (const StCString& )> onError;
    } signals;

        protected:

    /**
     * Create output stream from input stream.
     */
    ST_CPPEXPORT bool addStream(AVFormatContext* theContext,
                                AVStream*        theStream);

        private:

    StArrayList<AVFormatContext*> myCtxListSrc;
    StFormat                      myStereoFormat;

};

#endif // __StAVVideoMuxer_h_
