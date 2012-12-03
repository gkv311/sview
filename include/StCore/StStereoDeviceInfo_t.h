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

#ifndef __StStereoDeviceInfo_t_h_
#define __StStereoDeviceInfo_t_h_

#include <stTypes.h>

enum {
    ST_DEVICE_SUPPORT_IGNORE =-1,
    ST_DEVICE_SUPPORT_NONE   = 0,
    ST_DEVICE_SUPPORT_LOW    = 1,
    ST_DEVICE_SUPPORT_MIDDLE = 2,
    ST_DEVICE_SUPPORT_HIGHT  = 3,
    ST_DEVICE_SUPPORT_FULL   = 4,
    ST_DEVICE_SUPPORT_PREFER = 5,
};

typedef struct tagStStereoDeviceInfo {
    const stUtf8_t* stringId;
    const stUtf8_t* name;
    const stUtf8_t* description;
    int             detectionLevel;
} StStereoDeviceInfo_t;

typedef struct tagStSDOption {
    stUtf8_t*    title;
    size_t  optionType;
} StSDOption_t;

typedef struct tagStSDOnOff {
    stUtf8_t*    title;
    size_t  optionType;
    stBool_t     value;
} StSDOnOff_t;

typedef struct tagStSDSwitch {
    stUtf8_t*         title;
    size_t       optionType;
    size_t            value;
    size_t      valuesCount;
    stUtf8_t** valuesTitles;
} StSDSwitch_t;

enum {
    ST_DEVICE_OPTION_ON_OFF,
    ST_DEVICE_OPTION_SWITCH,
};

typedef struct tagStSDOptionsList {
    stUtf8_t* curRendererPath;
    int           curDeviceId;
    size_t       optionsCount;
    StSDOption_t**    options;
} StSDOptionsList_t;


inline void stMemFree(StSDOnOff_t* optionOnOffStruct, stMemFree_t freeFunction) {
    if(optionOnOffStruct == NULL) {
        return;
    }
    freeFunction(optionOnOffStruct->title);
    freeFunction(optionOnOffStruct);
}

inline void stMemFree(StSDSwitch_t* optionSwitchStruct, stMemFree_t freeFunction) {
    if(optionSwitchStruct == NULL) {
        return;
    }
    freeFunction(optionSwitchStruct->title);
    for(size_t valID = 0; valID < optionSwitchStruct->valuesCount; valID++) {
        freeFunction(optionSwitchStruct->valuesTitles[valID]);
    }
    freeFunction(optionSwitchStruct->valuesTitles);
    freeFunction(optionSwitchStruct);
}

inline void stMemFree(StSDOptionsList_t* optionsStruct, stMemFree_t freeFunction) {
    if(optionsStruct == NULL) {
        return;
    }
    freeFunction(optionsStruct->curRendererPath);
    for(size_t optId = 0; optId < optionsStruct->optionsCount; optId++) {
        switch(optionsStruct->options[optId]->optionType) {
            case ST_DEVICE_OPTION_ON_OFF: {
                stMemFree((StSDOnOff_t* )optionsStruct->options[optId], freeFunction);
                break;
            }
            case ST_DEVICE_OPTION_SWITCH: {
                stMemFree((StSDSwitch_t* )optionsStruct->options[optId], freeFunction);
                break;
            }
            //default: ST_DEBUG_LOG_AT("Unknown option type: " + optionsStruct->options[optId]->optionType);
        }
    }
    freeFunction(optionsStruct);
}

#endif //__StStereoDeviceInfo_t_h_
