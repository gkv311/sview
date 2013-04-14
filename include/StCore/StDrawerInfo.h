/**
 * Copyright © 2009-2013 Kirill Gavrilov <kirill@sview.ru>
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

#ifndef __StDrawerInfo_h_
#define __StDrawerInfo_h_

#include <stTypes.h>
#include <StFile/StMIMEList.h>

class StDrawerInfo {

        public:

    ST_CPPEXPORT static const StMIME& DRAWER_MIME();
    ST_CPPEXPORT static const StMIME& CLOSE_MIME();

        public:

    ST_CPPEXPORT StDrawerInfo();
    ST_CPPEXPORT StDrawerInfo(const StDrawerInfo& toCopy);
    ST_CPPEXPORT StDrawerInfo(const StString& drawerPath);
    ST_CPPEXPORT ~StDrawerInfo();

    const StDrawerInfo& operator=(const StDrawerInfo& toCopy) {
        if(this != &toCopy) {
            drawerPath = toCopy.drawerPath;
            mimeList = toCopy.mimeList;
        }
        return (*this);
    }

    const StString& getPath() const {
        return drawerPath;
    }

    const StMIMEList& getMIMEList() const {
        return mimeList;
    }

    bool isValid() const {
        return bValid;
    }

    bool operator==(const StDrawerInfo& compare) const {
        if(&compare == this) {
            return true;
        }
        return compare.drawerPath == this->drawerPath;
    }

    bool operator!=(const StDrawerInfo& compare) const {
        if(&compare == this) {
            return false;
        }
        return compare.drawerPath != this->drawerPath;
    }

    bool operator>(const StDrawerInfo& compare) const {
        if(&compare == this) {
            return false;
        }
        return this->mimeList.size() > compare.mimeList.size();
    }

    bool operator<(const StDrawerInfo& compare) const {
        if(&compare == this) {
            return false;
        }
        return this->mimeList.size() < compare.mimeList.size();
    }

    bool operator>=(const StDrawerInfo& compare) const {
        if(&compare == this) {
            return true;
        }
        return this->mimeList.size() >= compare.mimeList.size();
    }

    bool operator<=(const StDrawerInfo& compare) const {
        if(&compare == this) {
            return true;
        }
        return this->mimeList.size() <= compare.mimeList.size();
    }

    StString toString() const {
        return StString("Drawer Path = '") + drawerPath + "'. Full MIME list:\n" + mimeList.toString();
    }

        private:

    StString drawerPath;
    StMIMEList    mimeList;
    bool            bValid;

};

#endif //__StDrawerInfo_h_
