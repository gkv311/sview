/**
 * Copyright © 2009 Kirill Gavrilov <kirill@sview.ru>
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

#ifndef __StStereoDeviceInfoList_h_
#define __StStereoDeviceInfoList_h_

#ifdef __cplusplus

#include "StStereoDeviceInfo.h"

#include <StTemplates/StArrayList.h>

typedef struct tagStRendererInfo {

    stUtf8_t*             rendererPath; //!< path to this plugin
    stUtf8_t*             aboutString;  //!< plugin description
    StStereoDeviceInfo_t* devices;      //!< devices array
    size_t                count;        //!< number of devices

} StRendererInfo_t;

typedef struct tagStRenderersArray {

    StRendererInfo_t* array;
    size_t            count;

} StRenderersArray_t;

class ST_LOCAL StStereoDeviceInfoList : public StArrayList<StStereoDeviceInfo> {

        public:

    // inherited constructors
    StStereoDeviceInfoList(size_t theInitialSize = 8) : StArrayList<StStereoDeviceInfo>(theInitialSize) {}
    StStereoDeviceInfoList(const StStereoDeviceInfoList& theCopy) : StArrayList<StStereoDeviceInfo>(theCopy) {}

    StStereoDeviceInfoList(const StRendererInfo_t& theDevInfo)
    : StArrayList<StStereoDeviceInfo>(theDevInfo.count) {
        for(size_t anIter = 0; anIter < theDevInfo.count; ++anIter) {
            StArrayList<StStereoDeviceInfo>::add(StStereoDeviceInfo(theDevInfo.devices[anIter]));
        }
    }

};

#endif //__cplusplus
#endif //__StStereoDeviceInfoList_h_
