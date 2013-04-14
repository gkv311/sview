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

#ifndef __StRendererInfo_h_
#define __StRendererInfo_h_

#include "StStereoDeviceInfoList.h"

class StRendererInfo {

        public:

    enum {
        DEVICE_AUTO = -1,
    };

    ST_CPPEXPORT StRendererInfo();
    ST_CPPEXPORT StRendererInfo(const StRendererInfo&   theCopy);
    ST_CPPEXPORT StRendererInfo(const StRendererInfo_t& theCopy);
    ST_CPPEXPORT StRendererInfo(const StString& theRendererPath,
                                const bool      theToDetectPriority);

    ST_CPPEXPORT const StRendererInfo& operator=(const StRendererInfo& toCopy);

    ST_CPPEXPORT StString getTitle() const;

    inline const StString& getPath() const {
        return myRendererPath;
    }

    inline const StString& getAboutString() const {
        return myAboutString;
    }

    inline const StStereoDeviceInfoList& getDeviceList() const {
        return myDeviceList;
    }

    inline bool isValid() const {
        return myIsValid;
    }

    ST_CPPEXPORT int getSupportLevel() const;

    ST_CPPEXPORT bool operator==(const StRendererInfo& compare) const;

    ST_CPPEXPORT bool operator!=(const StRendererInfo& compare) const;

    inline bool operator>(const StRendererInfo& compare) const {
        if(&compare == this) {
            return false;
        }
        return this->getSupportLevel() > compare.getSupportLevel();
    }

    inline bool operator<(const StRendererInfo& compare) const {
        if(&compare == this) {
            return false;
        }
        return this->getSupportLevel() < compare.getSupportLevel();
    }

    inline bool operator>=(const StRendererInfo& compare) const {
        if(&compare == this) {
            return true;
        }
        return this->getSupportLevel() >= compare.getSupportLevel();
    }

    inline bool operator<=(const StRendererInfo& compare) const {
        if(&compare == this) {
            return true;
        }
        return this->getSupportLevel() <= compare.getSupportLevel();
    }

    ST_CPPEXPORT StString toString() const;

        private:

    StString               myRendererPath; //!< path to this plugin
    StString               myAboutString;  //!< plugin description
    StStereoDeviceInfoList myDeviceList;   //!< devices list
    bool                   myIsValid;      //!< flag indicates that this plugin can be loaded

};

#endif //__StRendererInfo_h_
