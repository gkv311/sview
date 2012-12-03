/**
 * Copyright © 2007-2012 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#include <StCore/StCore.h>
#include <StCore/StApplication.h>
#include <StCore/StWindow.h>

#include <StFile/StFolder.h>

StCore::CoreFunctions::CoreFunctions()
: StCore_new(NULL),
  StCore_del(NULL),
  StCore_getStWindow(NULL),
  StCore_init(NULL),
  StCore_open(NULL),
  StCore_callback(NULL),
  StCore_stglDraw(NULL),
  StCore_getStMonitors(NULL),
  StCore_getStRenderers(NULL) {
    //
}

StCore::CoreFunctions::~CoreFunctions() {}

void StCore::CoreFunctions::load(StLibrary& theLib) {
    theLib("StCore_new",            StCore_new);
    theLib("StCore_del",            StCore_del);
    theLib("StCore_getStWindow",    StCore_getStWindow);
    theLib("StCore_init",           StCore_init);
    theLib("StCore_open",           StCore_open);
    theLib("StCore_callback",       StCore_callback);
    theLib("StCore_stglDraw",       StCore_stglDraw);
    theLib("StCore_getStMonitors",  StCore_getStMonitors);
    theLib("StCore_getStRenderers", StCore_getStRenderers);
}

bool StCore::CoreFunctions::isNull() const {
    return StCore_new  == NULL || StCore_del == NULL
        || StCore_open == NULL || StCore_stglDraw == NULL
        || StCore_callback == NULL
        || StCore_getStWindow == NULL || StCore_init == NULL
        || StCore_getStMonitors  == NULL
        || StCore_getStRenderers == NULL;
}

void StCore::CoreFunctions::nullify() {
    StCore_new = NULL;
    StCore_del = NULL;
    StCore_getStWindow = NULL;
    StCore_init = NULL;
    StCore_open = NULL;
    StCore_callback = NULL;
    StCore_stglDraw = NULL;
    StCore_getStMonitors  = NULL;
    StCore_getStRenderers = NULL;
}

namespace {
    static StCore::CoreFunctions ST_CORE_FUNCTIONS;
    static StLibrary ST_CORE_LIB;
    static StMutex ST_CORE_MUTEX;
    static size_t ST_CORE_COUNTER = 0;
};

StCore::CoreFunctions& StCore::GetFunctions() {
    return ST_CORE_FUNCTIONS;
}

StLibrary& StCore::GetLibrary() {
    return ST_CORE_LIB;
}

StMutex& StCore::GetMutex() {
    return ST_CORE_MUTEX;
}

size_t& StCore::GetUseCounter() {
    return ST_CORE_COUNTER;
}

StArrayList<StMonitor> StCore::getStMonitors() {
    StArrayList<StMonitor> stMonitors;
    if(GetFunctions().StCore_getStMonitors != NULL) {
        StMonitor_t* monArr = NULL;
        int monCount = GetFunctions().StCore_getStMonitors(monArr, 0, ST_FALSE);
        if(monCount <= 0) {
            return stMonitors;
        }
        monArr = new StMonitor_t[monCount];
        GetFunctions().StCore_getStMonitors(monArr, monCount, ST_FALSE);

        // create class instances from structs array
        for(int m = 0; m < monCount; ++m) {
            stMonitors.add(StMonitor(monArr[m]));
        }
        delete[] monArr;
    }
    return stMonitors;
}

StMonitor StCore::getMonitorFromPoint(const StPointI_t& iPoint) {
    StArrayList<StMonitor> stMonitors = getStMonitors();
    for(size_t id = 0; id < stMonitors.size(); ++id) {
        if(stMonitors[id].getVRect().isPointIn(iPoint)) {
            return stMonitors[id];
        }
    }
    // return first anyway...
    return !stMonitors.isEmpty() ? stMonitors[0] : StMonitor();
}

int StCore::INIT() {
    GetMutex().lock();
    if(GetUseCounter() > 0) {
        ++GetUseCounter();
        GetMutex().unlock();
        return STERROR_LIBNOERROR;
    }

    static const stUtf8_t LIB_NAME[] = "StCore"; // library name
    if(!GetLibrary().load(StProcess::getStCoreFolder() + LIB_NAME)) {
        GetMutex().unlock();
        return STERROR_LIBLOADFAILED;
    }

    // get functions addresses in library
    StWindow::GetFunctions().load(GetLibrary());
    GetFunctions().load(GetLibrary());
    StApplication::GetFunctions().load(GetLibrary());

    if (StWindow::GetFunctions().isNull() || GetFunctions().isNull() || StApplication::GetFunctions().isNull()) {
        GetMutex().unlock();
        FREE();
        return STERROR_LIBFUNCTIONNOTFOUND;
    }
    ++GetUseCounter();
    GetMutex().unlock();
    // initialization success
    return STERROR_LIBNOERROR;
}

void StCore::FREE() {
    GetMutex().lock();
    if(GetUseCounter() > 0) {
        --GetUseCounter();
    }
    if(GetUseCounter() > 0) {
        // library still in use
        GetMutex().unlock();
        return;
    }

    GetLibrary().close();

    StWindow::GetFunctions().nullify();
    GetFunctions().nullify();
    StApplication::GetFunctions().nullify();

    GetMutex().unlock();
}

StString StCore::getDrawersDir() {
    return "StDrawers";
}

StArrayList<StDrawerInfo> StCore::getDrawersList() {
    StString stCorePath = StProcess::getStCoreFolder();
    StString stDrawersPath = stCorePath + StCore::getDrawersDir();

    StLibrary::suppressSystemErrors(true); // suppress system errors
    StArrayList<StDrawerInfo> stDrawersInfoList;
    StFolder pluginsList(stDrawersPath);
    StArrayList<StString> anExtList(1); anExtList.add(StString(ST_DLIB_EXTENSION));
    pluginsList.init(anExtList);
    for(size_t iter = 0; iter < pluginsList.size(); ++iter) {
        StDrawerInfo stDrawerInfo(pluginsList[iter]->getPath());
        if(stDrawerInfo.isValid()) {
            stDrawersInfoList.add(stDrawerInfo);
        }
    }
    StLibrary::suppressSystemErrors(false);
    return stDrawersInfoList;
}

StString StCore::getRenderersDir() {
    return "StRenderers";
}

void StCore::getRenderersList(StArrayList<StRendererInfo>& theList,
                              const bool                   theToDetectPriority) {
    theList.clear();
    if(GetFunctions().StCore_getStRenderers == NULL) {
        return;
    }

    StRenderersArray_t* anArray = GetFunctions().StCore_getStRenderers(theToDetectPriority);
    if(anArray == NULL || anArray->count == 0) {
        return;
    }

    for(size_t aRendIter = 0; aRendIter < anArray->count; ++aRendIter) {
        theList.add(StRendererInfo(anArray->array[aRendIter]));
    }
}

StCore::StCore() {
    isPointer = false;
    instance = (StRendererInterface* )GetFunctions().StCore_new();
    stWindow = new StWindow(GetFunctions().StCore_getStWindow(instance));
}

StCore::StCore(StRendererInterface* inst) {
    isPointer = true;
    instance = inst;
    stWindow = new StWindow(GetFunctions().StCore_getStWindow(instance));
}

StCore::~StCore() {
    if(!isPointer) {
        GetFunctions().StCore_del(instance);
    }
    delete stWindow;
}
