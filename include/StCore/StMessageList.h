/**
 * Copyright © 2007-2012 Kirill Gavrilov <kirill@sview.ru>
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
#include <StThreads/StThreads.h> // threads header (mutexes, threads,...)
#include "StVirtualKeys.h"       // VIRTUAL keyboard codes

typedef struct tagStMessage {
    size_t uin; // unique identification number
    void* data; // message data
} StMessage_t;

typedef struct tagStMouseMessage {
    StPointD_t point; // click point
    int       button; // mouse button
} StMouseMessage_t;

// TODO (Kirill Gavrilov#5#) name of the class is ambiguous, may tangle as StEvent list
/**
 * Special class for StWindow callback events list.
 */
class ST_LOCAL StMessageList {

        public:

    static const size_t BUFFER_SIZE = 2048U;
    enum {
        MSG_NULL = 0,   // last 'NULL' event in query
        MSG_NONE = 1,   // just ignored event
        MSG_EXIT = 2,
        MSG_CLOSE = 3,
        MSG_INIT = 4,   // reserved
        MSG_KEYS = 5,
        MSG_RESIZE = 6,
        MSG_DRAGNDROP_IN = 7,
        MSG_MOUSE_DOWN = 8,
        MSG_MOUSE_UP = 9,
        MSG_MOUSE_MOVE = 10,
        MSG_OPEN_FILE = 11,
        MSG_DEVICE_INFO = 12,
        MSG_DEVICE_OPTION = 13,
        MSG_WIN_ON_NEW_MONITOR = 14,
        MSG_FULLSCREEN_SWITCH = 15,
        MSG_MOUSE_DOWN_APPEND = 40,
        MSG_MOUSE_UP_APPEND = 41,
        MSG_KEY_DOWN_APPEND = 42,
        MSG_KEY_UP_APPEND = 43,
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
    StTimer     keyDelay; // special key-delay timer
    bool    keysMap[256]; // VKEYS map

        public:

    StMessageList()
    : stMutex(),
      count(0),
      keyDelay(true) {
        //
        stMemSet(messageList, 0, sizeof(messageList));
        resetKeysMap();
    }

    ~StMessageList() {
        //
    }

    bool* getKeysMap() {
        // TODO (Kirill Gavrilov#3#) not thread-safe operation
        return keysMap;
    }

    void resetKeysMap() {
        // TODO (Kirill Gavrilov#3#) not thread-safe operation
        stMemSet(keysMap, (int )false, sizeof(keysMap));
    }

    bool append(const size_t& msgUIN, void* msgData = NULL) {
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

    bool append(const StMessage_t& stMessage) {
        return append(stMessage.uin, stMessage.data);
    }

    bool hasExitMessage() {
        stMutex.lock();
            bool res = messageList[0].uin == MSG_EXIT;
        stMutex.unlock();
        return res;
    }

    void popList(StMessage_t outList[BUFFER_SIZE + 1]) {
        outList[0].uin = StMessageList::MSG_NULL; // at first - mark 'empty' callback

        // TODO (Kirill Gavrilov#4#) - this is not best place for keyMap
        // prevent too offten ST_CALLBACK_KEYS events
        // 60Hz is max for this
        if(keyDelay.getElapsedTimeInMilliSec() >= 16.6) {
            keyDelay.restart();
            append(MSG_KEYS, keysMap);
        }

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
