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

#ifndef __StRendererPlugin_h_
#define __StRendererPlugin_h_

#include "StRendererInterface.h"
#include "StStereoDeviceInfo_t.h"
#include "StOpenInfo.h"

#include <StLibrary.h>

struct tagStRendererInfo;
typedef tagStRendererInfo StRendererInfo_t;

class ST_LOCAL StRendererPlugin : public StRendererInterface {

        protected:

    // allow use type definitions
    friend class StCore;

    // typedef pointer-to-class
    typedef void* StRenderer_t;

    // types definitions - needed for each exported function
    typedef StRenderer_t (*StRenderer_new_t)();
    typedef void (*StRenderer_del_t)(StRenderer_t );
    typedef StWindowInterface* (*StRenderer_getStWindow_t)(StRenderer_t );
    typedef stBool_t (*StRenderer_init_t)(StRenderer_t , const stUtf8_t* , const int& , const StNativeWin_t );
    typedef stBool_t (*StRenderer_open_t)(StRenderer_t , const StOpenInfo_t* );
    typedef void (*StRenderer_callback_t)(StRenderer_t , StMessage_t* );
    typedef void (*StRenderer_stglDraw_t)(StRenderer_t , unsigned int );

        public:

    // definitions of special functions
    typedef const stUtf8_t* (*StRenderer_getAboutInfo_t)();
    typedef const StRendererInfo_t* (*StRenderer_getDevicesInfo_t)(const stBool_t theToDetectPriority);

        private:

    StLibrary stLib;

    // plugin exported functions' pointers
    StRenderer_new_t StRenderer_new;
    StRenderer_del_t StRenderer_del;
    StRenderer_getStWindow_t StRenderer_getStWindow;
    StRenderer_init_t StRenderer_init;
    StRenderer_open_t StRenderer_open;
    StRenderer_callback_t StRenderer_callback;
    StRenderer_stglDraw_t StRenderer_stglDraw;
    StRenderer_getDevicesInfo_t StRenderer_getDevicesInfo;

    StRendererInterface* instance;
    StWindowInterface* stWindow;

        public:

    /**
     * Empty constructor. Doesn't create class instance!
     */
    StRendererPlugin();

    /**
     * Open the plugin and retrieve function pointers.
     */
    bool InitLibrary(const StString& thePluginPath);

    void Instantiate() {
        instance = (StRendererInterface* )StRenderer_new();
    }

    StRendererInterface* getLibImpl() {
        return instance;
    }

    StWindowInterface* getStWindow() {
        return stWindow;
    }

    bool init(const int& deviceId, const StNativeWin_t nativeParent = (StNativeWin_t )NULL);
    bool init(const StString& rendererPath, const int& deviceId, const StNativeWin_t nativeParent = (StNativeWin_t )NULL);

    bool open(const StOpenInfo& stOpenInfo = StOpenInfo()) {
        const StOpenInfo_t stOpenInfoStruct = stOpenInfo.getStruct();
        return StRenderer_open(instance, &stOpenInfoStruct);
    }

    void callback(StMessage_t* stMessages) {
        StRenderer_callback(instance, stMessages);
    }

    void stglDraw(unsigned int views) {
        StRenderer_stglDraw(instance, views);
    }

    ~StRendererPlugin() {
        Destruct();
    }

    void Destruct();

    // auxiliary function
    const StRendererInfo_t* GetDevicesInfo(const bool theToDetectPriority) const {
        if(StRenderer_getDevicesInfo == NULL) {
            return NULL;
        }
        return StRenderer_getDevicesInfo(theToDetectPriority);
    }

};

#endif //__StRendererPlugin_h_
