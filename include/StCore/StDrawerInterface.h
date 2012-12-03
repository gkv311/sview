/**
 * Copyright © 2007-2010 Kirill Gavrilov <kirill@sview.ru>
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

#ifndef __StDrawerInterface_h_
#define __StDrawerInterface_h_

#include <stTypes.h>
#include <StCore/StMessageList.h>
#include <StCore/StOpenInfo.h>
#include <StGL/StGLEnums.h>

class StWindowInterface;

/**
 * Applications / Drawer Plugins should implement this interface
 */
class StDrawerInterface {

        public:

    virtual StDrawerInterface* getLibImpl() = 0;

    virtual ~StDrawerInterface() {} // never forget it!

    virtual bool init(StWindowInterface* ) = 0;

    virtual bool open(const StOpenInfo& stOpenInfo = StOpenInfo()) = 0;

    virtual void parseCallback(StMessage_t* ) = 0;

    virtual void stglDraw(unsigned int ) = 0;

};

#endif //__StDrawerInterface_h_
