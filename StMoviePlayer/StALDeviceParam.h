/**
 * Copyright © 2016 Kirill Gavrilov <kirill@sview.ru>
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

#ifndef __StALDeviceParam_h_
#define __StALDeviceParam_h_

#include <StSettings/StParam.h>

#include <vector>

class StALDeviceParam : public StInt32ParamNamed {

        public:

    /**
     * Main constructor.
     */
    ST_LOCAL StALDeviceParam();

    /**
     * Destructor.
     */
    ST_LOCAL virtual ~StALDeviceParam();

    ST_LOCAL void initList();

    ST_LOCAL bool init(const StString& theActive);

    ST_LOCAL int32_t getValueFromName(const StString& theName);

    /**
     * Returns title for active AL device.
     */
    ST_LOCAL StString getUtfTitle() const;

    /**
     * Returns title for active AL device.
     */
    ST_LOCAL std::string getCTitle() const;

    /**
     * Return list of available translations.
     */
    ST_LOCAL const StArrayList<StString>& getList() const {
        return myDevicesUtf;
    }

        private:

    std::vector<std::string> myDevicesLoc;
    StArrayList<StString>    myDevicesUtf;

};

#endif // __StALDeviceParam_h_
