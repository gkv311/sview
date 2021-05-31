/**
 * StGLWidgets, small C++ toolkit for writing GUI using OpenGL.
 * Copyright © 2010-2014 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#ifndef __StSubQueue_h_
#define __StSubQueue_h_

#include <StTemplates/StHandle.h>
#include <StTemplates/StArrayList.h>
#include <StThreads/StMutex.h>
#include <StImage/StImagePlane.h>

/**
 * Subtitle primitive (Text that bound to one time interval).
 */
class StSubItem {

        public:

    StString     Text;      //!< subtitle textual representation
    StImagePlane Image;     //!< subtitle image   representation
    double       TimeStart; //!< PTS to show subtitle item
    double       TimeEnd;   //!< PTS to hide subtitle item
    float        Scale;     //!< image scale factor

        public:

    ST_LOCAL StSubItem(double theTimeStart,
                       double theTimeEnd)
    : TimeStart(theTimeStart),
      TimeEnd(theTimeEnd),
      Scale(1.0f) {
        //
    }

};

/**
 * Thread-safe subtitles queue.
 */
class StSubQueue {

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

        private:

    struct QueueItem {

        StHandle<StSubItem> myItem;
        QueueItem* myNext;

        ST_LOCAL QueueItem(const StHandle<StSubItem>& theItem)
        : myItem(theItem),
          myNext(NULL) {}

    };

        private: //! @name private fields

    QueueItem* myFront; //!< queue front item
    QueueItem* myBack;  //!< queue back item
    StMutex    myMutex; //!< lock for thread safety

};

#endif // __StSubQueue_h_
