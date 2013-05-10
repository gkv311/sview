/**
 * Copyright © 2007-2013 Kirill Gavrilov <kirill@sview.ru>
 *
 * StCore library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * StCore library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.
 * If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __StEventsList_h_
#define __StEventsList_h_

#include <StTemplates/StVec2.h>
#include <StThreads/StMutex.h>
#include <StThreads/StTimer.h>
#include "StVirtualKeys.h"

typedef struct tagStMessage {
    size_t uin; // unique identification number
    void* data; // message data
} StMessage_t;

typedef struct tagStMouseMessage {
    StPointD_t point; // click point
    int       button; // mouse button
} StMouseMessage_t;

/**
 * Special class for StWindow callback events list.
 */
class StMessageList {

        public:

    static const size_t BUFFER_SIZE = 2048U;
    enum {
        MSG_NULL = 0,   // last 'NULL' event in query
        MSG_NONE = 1,   // just ignored event
        MSG_EXIT = 2,
        MSG_CLOSE = 3,
        MSG_INIT = 4,   // reserved
        MSG_RESIZE = 6,
        MSG_MOUSE_MOVE = 10,
        MSG_OPEN_FILE = 11,
        MSG_DEVICE_INFO = 12,
        MSG_DEVICE_OPTION = 13,
        MSG_WIN_ON_NEW_MONITOR = 14,
        MSG_FULLSCREEN_SWITCH = 15,
        MSG_GO_TOP      = 44,
        MSG_GO_BOTTOM   = 45,
        MSG_GO_BACKWARD = 46,
        MSG_GO_FORWARD  = 47,
    };

        private:

    //size_t eventsId[BUFFER_SIZE + 1];
    StMessage_t messageList[BUFFER_SIZE + 1];

    StMutex      stMutex; // thread-safe
    size_t         count; // events counter

        public:

    ST_LOCAL StMessageList()
    : count(0) {
        reset();
    }

    ST_LOCAL void reset() {
        stMemSet(messageList, 0, sizeof(messageList));
    }

    ST_LOCAL bool append(const size_t& msgUIN, void* msgData = NULL) {
        stMutex.lock();
        if(msgUIN == MSG_EXIT) {
            count = 1;
            messageList[0].uin = MSG_EXIT;
        } else if(count < BUFFER_SIZE) {
            messageList[count].uin = msgUIN;
            messageList[count].data = msgData;
            messageList[++count].uin = MSG_NULL;
        } else {
            stMutex.unlock();
            return false;
        }
        stMutex.unlock();
        return true;
    }

    ST_LOCAL bool append(const StMessage_t& stMessage) {
        return append(stMessage.uin, stMessage.data);
    }

    ST_LOCAL bool hasExitMessage() {
        stMutex.lock();
            bool res = messageList[0].uin == MSG_EXIT;
        stMutex.unlock();
        return res;
    }

    ST_LOCAL void popList(StMessage_t outList[BUFFER_SIZE + 1]) {
        outList[0].uin = StMessageList::MSG_NULL; // at first - mark 'empty' callback

        if(stMutex.tryLock()) {
            for(size_t i = 0; i <= count; ++i) {
                outList[i] = messageList[i];
            }
            messageList[0].uin = MSG_NULL;
            count = 0;
            stMutex.unlock();
        }
    }

};

#endif //__StEventsList_h_
