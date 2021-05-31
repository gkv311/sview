/**
 * StGLWidgets, small C++ toolkit for writing GUI using OpenGL.
 * Copyright © 2010-2014 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
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
        if(anItem->myItem->TimeEnd < thePTS) {
            // remove outdated items
            myFront = myFront->myNext;
            delete anItem;
            anItem = myFront;
        } else if(anItem->myItem->TimeStart <= thePTS) {
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
