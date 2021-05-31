/**
 * Copyright © 2013-2015 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#ifndef __StMsgQueue_h__
#define __StMsgQueue_h__

#include <StThreads/StMutex.h>
#include <StStrings/StLogger.h>

#include <deque>

struct StMsg {

    StHandle<StString> Text;  //!< message text
    StLogger::Level    Level; //!< message level

};

/**
 * Queue of messages.
 */
class StMsgQueue {

        public:

    /**
     * Empty constructor.
     */
    ST_CPPEXPORT StMsgQueue();

    /**
     * Destructor.
     */
    ST_CPPEXPORT virtual ~StMsgQueue();

    /**
     * Pop message from the queue.
     */
    ST_CPPEXPORT bool pop(StMsg& theMessage);

    /**
     * Pop all messages and display them using standard dialogs.
     */
    ST_CPPEXPORT void popAll();

    /**
     * Push info message to the queue.
     */
    ST_CPPEXPORT void pushInfo(const StHandle<StString>& theMessage);

    /**
     * Push info message to the queue.
     */
    ST_LOCAL void pushInfo(const StCString& theMessage) {
        pushInfo(new StString(theMessage));
    }

    /**
     * Push error message to the queue.
     */
    ST_CPPEXPORT void pushError(const StHandle<StString>& theMessage);

    /**
     * Push error message to the queue.
     */
    ST_LOCAL void pushError(const StCString& theMessage) {
        pushError(new StString(theMessage));
    }

        public: //! @name callback Slots

    /**
     * Push message to the queue.
     */
    ST_CPPEXPORT void doPush(const StMsg& theMessage);

    /**
     * Push info message to the queue.
     */
    ST_CPPEXPORT void doPushInfo(const StCString& theMessage);

    /**
     * Push error message to the queue.
     */
    ST_CPPEXPORT void doPushError(const StCString& theMessage);

        private:

    StMutex           myMutex; //!< mutex for thread-safe access
    std::deque<StMsg> myQueue; //!< messages queue

};

#endif // __StMsgQueue_h__
