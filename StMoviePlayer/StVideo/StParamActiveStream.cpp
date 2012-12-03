/**
 * Copyright © 2011 Kirill Gavrilov <kirill@sview.ru>
 *
 * StMoviePlayer program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * StMoviePlayer program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program.
 * If not, see <http://www.gnu.org/licenses/>.
 */

#include "StParamActiveStream.h"

StParamActiveStream::StParamActiveStream()
: StInt32Param(-1),
  myList(new StArrayList<StString>(1)),
  myMutex(),
  myIsChanged(false) {}

StHandle< StArrayList<StString> > StParamActiveStream::getList() const {
    myMutex.lock();
    StHandle< StArrayList<StString> > aList = myList;
    myMutex.unlock();
    return aList;
}

void StParamActiveStream::clearList() {
    myMutex.lock();
    myList  = new StArrayList<StString>(1);
    myValue = -1;
    myIsChanged = false;
    myMutex.unlock();
}

void StParamActiveStream::setList(const StHandle< StArrayList<StString> >& theList,
                                  const int32_t theValue) {
    myMutex.lock();
    myList  = theList;
    myValue = theValue;
    myIsChanged = false;
    myMutex.unlock();
}

int32_t StParamActiveStream::getValue() const {
    myMutex.lock();
    int32_t aStreamId = myValue;
    myMutex.unlock();
    return aStreamId;
}

bool StParamActiveStream::setValue(const int32_t theValue) {
    myMutex.lock();
    bool toSwitch = myValue != theValue
                && (myValue == -1 || (theValue < int32_t(myList->size())));
    if(toSwitch) {
        myValue = theValue;
        myIsChanged = true;
    }
    myMutex.unlock();
    if(toSwitch) {
        signals.onChanged(theValue);
        return true;
    }
    return false;
}

bool StParamActiveStream::wasChanged() const {
    myMutex.lock();
    bool aChanged = myIsChanged;
    myIsChanged = false;
    myMutex.unlock();
    return aChanged;
}
