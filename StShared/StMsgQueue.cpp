/**
 * Copyright © 2013-2015 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#include <StStrings/StMsgQueue.h>

StMsgQueue::StMsgQueue() {
    //
}

StMsgQueue::~StMsgQueue() {
    //
}

void StMsgQueue::popAll() {
    myMutex.lock();
    StString aText;
    bool hasErrors = false;
    bool isFirst   = true;
    while(!myQueue.empty()) {
        const StMsg& aMsg = myQueue.front();
        if(!isFirst) {
            aText += "\n\n";
        }
        aText += *aMsg.Text;
        isFirst = false;

        if(aMsg.Level == StLogger::ST_ERROR) {
            hasErrors = true;
        }
        myQueue.pop_front();
    }
    myMutex.unlock();
    if(aText.isEmpty()) {
        return;
    }

    if(hasErrors) {
        stError(aText);
    } else {
        stInfo(aText);
    }
}

bool StMsgQueue::pop(StMsg& theMessage) {
    myMutex.lock();
    if(myQueue.empty()) {
        myMutex.unlock();
        return false;
    }

    theMessage = myQueue.front();
    myQueue.pop_front();
    myMutex.unlock();
    return true;
}

void StMsgQueue::doPush(const StMsg& theMessage) {
    myMutex.lock();
    myQueue.push_back(theMessage);
    myMutex.unlock();
}

void StMsgQueue::pushInfo(const StHandle<StString>& theMessage) {
    StMsg aMsg;
    aMsg.Level = StLogger::ST_INFO;
    aMsg.Text  = theMessage;
    //ST_DEBUG_LOG(*theMessage);
    doPush(aMsg);
}

void StMsgQueue::pushError(const StHandle<StString>& theMessage) {
    StMsg aMsg;
    aMsg.Level = StLogger::ST_ERROR;
    aMsg.Text  = theMessage;
    //ST_ERROR_LOG(*theMessage);
    doPush(aMsg);
}

void StMsgQueue::doPushInfo(const StCString& theMessage) {
    StMsg aMsg;
    aMsg.Level = StLogger::ST_INFO;
    aMsg.Text  = new StString(theMessage);
    //ST_DEBUG_LOG(*theMessage);
    doPush(aMsg);
}

void StMsgQueue::doPushError(const StCString& theMessage) {
    StMsg aMsg;
    aMsg.Level = StLogger::ST_ERROR;
    aMsg.Text  = new StString(theMessage);
    //ST_ERROR_LOG(*theMessage);
    doPush(aMsg);
}
