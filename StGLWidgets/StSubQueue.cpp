/**
 * StGLWidgets, small C++ toolkit for writing GUI using OpenGL.
 * Copyright © 2010-2014 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#include <StGLWidgets/StSubQueue.h>

StSubQueue::StSubQueue() {
    //
}

StSubQueue::~StSubQueue() {
    for (QueueItem* anItem = myFront; anItem != nullptr; anItem = myFront) {
        myFront = myFront->myNext;
        delete anItem;
    }
}

bool StSubQueue::isEmpty() {
    myMutex.lock();
    bool aResult = myFront == nullptr;
    myMutex.unlock();
    return aResult;
}

void StSubQueue::clear() {
    myMutex.lock();
    for (QueueItem* anItem = myFront; anItem != nullptr; anItem = myFront) {
        myFront = myFront->myNext;
        delete anItem;
    }
    myMutex.unlock();
}

std::shared_ptr<StSubItem> StSubQueue::pop(const double thePTS) {
    myMutex.lock();
    for (QueueItem* anItem = myFront; anItem != nullptr;) {
        if (anItem->myItem->TimeEnd < thePTS) {
            // remove outdated items
            myFront = myFront->myNext;
            delete anItem;
            anItem = myFront;
        } else if (anItem->myItem->TimeStart <= thePTS) {
            // pop the item
            std::shared_ptr<StSubItem> aSubItem = anItem->myItem;
            myFront = myFront->myNext;
            delete anItem;
            myMutex.unlock();
            return aSubItem;
        } else {
            // no more items to show
            myMutex.unlock();
            return std::shared_ptr<StSubItem>();
        }
    }
    myMutex.unlock();
    return std::shared_ptr<StSubItem>();
}

void StSubQueue::push(const std::shared_ptr<StSubItem>& theSubItem) {
    myMutex.lock();
    QueueItem* anItem = new QueueItem(theSubItem);
    if (myFront == nullptr) {
        myFront = myBack = anItem;
    } else {
        myBack->myNext = anItem;
        myBack = anItem;
    }
    myMutex.unlock();
}
