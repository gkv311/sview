/**
 * Copyright © 2010-2013 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#ifndef __StSubQueue_h_
#define __StSubQueue_h_

#include <StTemplates/StHandle.h>
#include <StTemplates/StArrayList.h>
#include <StThreads/StMutex.h>

/**
 * Subtitle primitive (Text that bound to one time interval).
 */
class StSubItem {

        public:

    StString myText;      //!< subtitle textual representation
    double   myTimeStart; //!< PTS to show subtitle item
    double   myTimeEnd;   //!< PTS to hide subtitle item

        public:

    StSubItem(const StString& theText,
              const double    theTimeStart,
              const double    theTimeEnd)
    : myText(theText),
      myTimeStart(theTimeStart),
      myTimeEnd(theTimeEnd) {
        //
    }

};

/**
 * Thread-safe subtitles queue.
 */
class StSubQueue {

        private:

    struct QueueItem {

        StHandle<StSubItem> myItem;
        QueueItem* myNext;

        QueueItem(const StHandle<StSubItem>& theItem)
        : myItem(theItem),
          myNext(NULL) {}

    };

        private: //!< private fields

    QueueItem* myFront; //!< queue front item
    QueueItem*  myBack; //!< queue back item
    StMutex    myMutex; //!< lock for thread safety

        public:

    /**
     * Default constructor.
     */
    ST_CPPEXPORT StSubQueue();

    /**
     * Destructor.
     */
    ST_CPPEXPORT virtual ~StSubQueue();

    /**
     * Returns true if queue is empty.
     */
    ST_CPPEXPORT bool isEmpty();

    /**
     * Clean up the queue.
     */
    ST_CPPEXPORT void clear();

    /**
     * Return new subtitle item to show according to current presentation timestamp
     * and automatically remove outdated items.
     * @param thePTS current presentation timestamp
     * @return new subtitle item to show or NULL handle
     */
    ST_CPPEXPORT StHandle<StSubItem> pop(const double thePTS);

    /**
     * Append subtitle item to the queue.
     * @param theSubItem item to add
     */
    ST_CPPEXPORT void push(const StHandle<StSubItem>& theSubItem);

};

#endif //__StSubQueue_h_
