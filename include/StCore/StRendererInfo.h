/**
 * Copyright © 2009-2012 Kirill Gavrilov <kirill@sview.ru>
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

#ifndef __StRendererInfo_h_
#define __StRendererInfo_h_

#include "StStereoDeviceInfoList.h"

class ST_LOCAL StRendererInfo {

        public:

    enum {
        DEVICE_AUTO = -1,
    };

    StRendererInfo();
    StRendererInfo(const StRendererInfo&   theCopy);
    StRendererInfo(const StRendererInfo_t& theCopy);
    StRendererInfo(const StString& theRendererPath,
                   const bool      theToDetectPriority);

    const StRendererInfo& operator=(const StRendererInfo& toCopy);

    StString getTitle() const;

    const StString& getPath() const {
        return myRendererPath;
    }

    const StString& getAboutString() const {
        return myAboutString;
    }

    const StStereoDeviceInfoList& getDeviceList() const {
        return myDeviceList;
    }

    bool isValid() const {
        return myIsValid;
    }

    int getSupportLevel() const;

    bool operator==(const StRendererInfo& compare) const;

    bool operator!=(const StRendererInfo& compare) const;

    bool operator>(const StRendererInfo& compare) const {
        if(&compare == this) {
            return false;
        }
        return this->getSupportLevel() > compare.getSupportLevel();
    }

    bool operator<(const StRendererInfo& compare) const {
        if(&compare == this) {
            return false;
        }
        return this->getSupportLevel() < compare.getSupportLevel();
    }

    bool operator>=(const StRendererInfo& compare) const {
        if(&compare == this) {
            return true;
        }
        return this->getSupportLevel() >= compare.getSupportLevel();
    }

    bool operator<=(const StRendererInfo& compare) const {
        if(&compare == this) {
            return true;
        }
        return this->getSupportLevel() <= compare.getSupportLevel();
    }

    StString toString() const;

        private:

    StString               myRendererPath; //!< path to this plugin
    StString               myAboutString;  //!< plugin description
    StStereoDeviceInfoList myDeviceList;   //!< devices list
    bool                   myIsValid;      //!< flag indicates that this plugin can be loaded

};

#endif //__StRendererInfo_h_
