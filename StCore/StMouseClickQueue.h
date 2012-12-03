/**
 * Copyright © 2007-2009 Kirill Gavrilov <kirill@sview.ru>
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

#ifndef __StMouseClickQueue_h_
#define __StMouseClickQueue_h_

#include <StThreads/StMutex.h>

/**
 * Helper-class.
 */
class ST_LOCAL StMouseClickQueue {

        private:

    static const size_t MOUSECLICK_BUFF_SIZE = 8;

    StPointD_t arrayPt[MOUSECLICK_BUFF_SIZE];
    int arrayBt[MOUSECLICK_BUFF_SIZE];
    size_t front;
    size_t back;
    size_t size;
    StMutex stMutex;

        public:


    StMouseClickQueue() : front(0), back(0), size(0), stMutex() {
        memset(arrayBt, 0, sizeof(arrayBt));
    }

    ~StMouseClickQueue() {
        //
    }

    void clear() {
        stMutex.lock();
        while(size != 0) {
            front++;
            if(front >= MOUSECLICK_BUFF_SIZE) {
                front = 0;
            }
            size--;
        }
        stMutex.unlock();
    }

    int pop(StPointD_t& point) {
        stMutex.lock();
        if(size != 0) {
            point = arrayPt[front];
            int btId = arrayBt[front];
            front++;
            if(front >= MOUSECLICK_BUFF_SIZE) {
                front = 0;
            }
            size--;
            stMutex.unlock();
            return btId;
        }
        stMutex.unlock();
        return ST_NOMOUSE;
    }

    void push(const StPointD_t& point, const int& buttonId) {
        stMutex.lock();
        if(size != MOUSECLICK_BUFF_SIZE) {
            arrayPt[back] = point;
            arrayBt[back] = buttonId;
            back++;
            if(back >= MOUSECLICK_BUFF_SIZE) {
                back = 0;
            }
            size++;
        }
        stMutex.unlock();
    }

};

#endif //__StMouseClickQueue_h_
