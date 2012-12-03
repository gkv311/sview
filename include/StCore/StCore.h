/**
 * Copyright © 2007-2012 Kirill Gavrilov <kirill@sview.ru>
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

#ifndef __StCore_h_
#define __StCore_h_

#include <StCore/StDrawerInfo.h>
#include <StCore/StRendererPlugin.h>
#include <StCore/StRendererInfo.h>
#include <StCore/StMonitor.h>

class ST_LOCAL StCore : public StRendererInterface {

        public:

    typedef int (*StCore_getStMonitors_t)(StMonitor_t* monList, const int& inSize, stBool_t toForceUpdate);
    typedef StRenderersArray_t* (*StCore_getStRenderers_t)(const stBool_t theToDetectPriority);

    class CoreFunctions {

            public:

        StRendererPlugin::StRenderer_new_t StCore_new;
        StRendererPlugin::StRenderer_del_t StCore_del;
        StRendererPlugin::StRenderer_getStWindow_t StCore_getStWindow;
        StRendererPlugin::StRenderer_init_t StCore_init;
        StRendererPlugin::StRenderer_open_t StCore_open;
        StRendererPlugin::StRenderer_callback_t StCore_callback;
        StRendererPlugin::StRenderer_stglDraw_t StCore_stglDraw;
        StCore_getStMonitors_t  StCore_getStMonitors;
        StCore_getStRenderers_t StCore_getStRenderers;

            public:

        CoreFunctions();
        ~CoreFunctions();

        void load(StLibrary& theLib);
        bool isNull() const;
        void nullify();

    };

    // core exported functions' pointers
    static CoreFunctions& GetFunctions();
    static StLibrary& GetLibrary();
    static StMutex&   GetMutex();      // private mutex to process multiple library INIT/FREE
    static size_t&    GetUseCounter(); // private counter to process multiple library INIT/FREE

        public:

    // INIT library function - called by host application to use library classes
    static int INIT();
    static void FREE();

    static StString getDrawersDir();
    static StArrayList<StDrawerInfo> getDrawersList();

    static StString getRenderersDir();
    static void getRenderersList(StArrayList<StRendererInfo>& theList,
                                 const bool                   theToDetectPriority);

    static StArrayList<StMonitor> getStMonitors();
    static StMonitor getMonitorFromPoint(const StPointI_t& iPoint);

        private:

    bool isPointer;
    StRendererInterface* instance;
    StWindowInterface* stWindow;

        public:

    StCore();
    StCore(StRendererInterface* inst);

    StRendererInterface* getLibImpl() {
        return instance;
    }

    StWindowInterface* getStWindow() {
        return stWindow;
    }

    bool init(const StString& theRendererPath, const int& theDeviceId, const StNativeWin_t* theNativeParent = NULL) {
        return GetFunctions().StCore_init(instance, theRendererPath.toCString(), theDeviceId, theNativeParent);
    }

    bool open(const StOpenInfo& stOpenInfo = StOpenInfo()) {
        const StOpenInfo_t stOpenInfoStruct = stOpenInfo.getStruct();
        return GetFunctions().StCore_open(instance, &stOpenInfoStruct);
    }

    /**
     * Callback order:
     *   - StWindow   (intial -> buffer empting and filled with window's messages);
     *   - StDrawer   (could parse StWindow messages, add own messages);
     *   - StRenderer (could parse StWindow and StDrawer messages, add own messages);
     *   - out (caller).
     */
    void callback(StMessage_t* stMessages) {
        GetFunctions().StCore_callback(instance, stMessages);
    }

    void stglDraw(unsigned int views) {
        GetFunctions().StCore_stglDraw(instance, views);
    }

    ~StCore();

};

#endif //__StCore_h_
