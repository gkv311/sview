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

#ifndef __StParamActiveStream_h_
#define __StParamActiveStream_h_

#include <StSettings/StParam.h>
#include <StTemplates/StArrayList.h>
#include <StThreads/StMutex.h>

/**
 * Parameter to control stream list.
 */
class StParamActiveStream : public StInt32Param {

        public:

    /**
     * Main constructor.
     */
    ST_LOCAL StParamActiveStream();

    /**
     * Returns list of streams.
     */
    ST_LOCAL StHandle< StArrayList<StString> > getList() const;

    /**
     * Update configuration.
     */
    ST_LOCAL void clearList();

    /**
     * Update configuration.
     */
    ST_LOCAL void setList(const StHandle< StArrayList<StString> >& theList,
                          const int32_t theValue);

    /**
     * Overridden thread-safe method.
     */
    ST_LOCAL virtual int32_t getValue() const;

    /**
     * Overridden thread-safe method.
     */
    ST_LOCAL virtual bool setValue(const int32_t theValue);

    /**
     * Switch to next value in list.
     * @param theIncrement Increment to current value
     * @return new value
     */
    ST_LOCAL int32_t nextValue(const int32_t theIncrement);

    /**
     * Returns true if value was changed since last call
     * and automatically reset this state.
     */
    ST_LOCAL bool wasChanged() const;

        private:

    StHandle< StArrayList<StString> > myList;
    mutable StMutex  myMutex;
    mutable bool     myIsChanged;

};

#endif //__StParamActiveStream_h_
