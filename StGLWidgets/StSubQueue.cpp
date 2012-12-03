/**
 * Copyright © 2010-2011 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#include <StGLWidgets/StSubQueue.h>

StSubQueue::StSubQueue()
: myFront(NULL),
  myBack(NULL),
  myMutex() {
    //
}

StSubQueue::~StSubQueue() {
    for(QueueItem* anItem = myFront; anItem != NULL; anItem = myFront) {
        myFront = myFront->myNext;
        delete anItem;
    }
}

bool StSubQueue::isEmpty() {
    myMutex.lock();
    bool aResult = myFront == NULL;
    myMutex.unlock();
    return aResult;
}

void StSubQueue::clear() {
    myMutex.lock();
    for(QueueItem* anItem = myFront; anItem != NULL; anItem = myFront) {
        myFront = myFront->myNext;
        delete anItem;
    }
    myMutex.unlock();
}

StHandle<StSubItem> StSubQueue::pop(const double thePTS) {
    myMutex.lock();
    for(QueueItem* anItem = myFront; anItem != NULL;) {
        StHandle<StSubItem> aSubItem = anItem->myItem;
        if(anItem->myItem->myTimeEnd < thePTS) {
            // remove outdated items
            myFront = myFront->myNext;
            delete anItem;
            anItem = myFront;
        } else if(anItem->myItem->myTimeStart <= thePTS) {
            // pop the item
            StHandle<StSubItem> aSubItem = anItem->myItem;
            myFront = myFront->myNext;
            delete anItem;
            myMutex.unlock();
            return aSubItem;
        } else {
            // no more items to show
            myMutex.unlock();
            return StHandle<StSubItem>();
        }
    }
    myMutex.unlock();
    return StHandle<StSubItem>();
}

void StSubQueue::push(const StHandle<StSubItem>& theSubItem) {
    myMutex.lock();
    QueueItem* anItem = new QueueItem(theSubItem);
    if(myFront == NULL) {
        myFront = myBack = anItem;
    } else {
        myBack->myNext = anItem;
        myBack = anItem;
    }
    myMutex.unlock();
}
