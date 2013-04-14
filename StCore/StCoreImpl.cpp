/**
 * Copyright © 2007-2013 Kirill Gavrilov <kirill@sview.ru>
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

#include "StCoreImpl.h"

#include <StFile/StFolder.h>

StCoreImpl::StCoreImpl()
: myWindow(NULL),
  myStCorePath(StProcess::getStCoreFolder()) {
    myWindow = StWindow_new();

    // Adjust system timer
    // By default Windows2K+ timer has ugly precision
    // Thus - Sleep(1) may be long 14ms!
    // We force best available precision to make Sleep() more adequate
    // This affect whole system while running application!
#if(defined(_WIN32) || defined(__WIN32__))
    TIMECAPS ptc; ptc.wPeriodMax = 0; ptc.wPeriodMin = 0;
    if(timeGetDevCaps(&ptc, sizeof(ptc)) == TIMERR_NOERROR) {
        timeBeginPeriod(ptc.wPeriodMin);
    } else {
        timeBeginPeriod(1);
    }
#endif

    // add additional paths
    // TODO (Kirill Gavrilov#1) linux library path
#if(defined(_WIN32) || defined(__WIN32__))
    // requires Windows XP with SP1 or higher!
    const StStringUtfWide aCoreFolder = StProcess::getStCoreFolder().toUtfWide();
    SetDllDirectoryW(aCoreFolder.toCString());
#endif
}

StCoreImpl::~StCoreImpl() {
    closeDrawer();
    StWindow_del(myWindow);
#if(defined(_WIN32) || defined(__WIN32__))
    TIMECAPS ptc; ptc.wPeriodMax = 0; ptc.wPeriodMin = 0;
    if(timeGetDevCaps(&ptc, sizeof(ptc)) == TIMERR_NOERROR) {
        timeEndPeriod(ptc.wPeriodMin);
    } else {
        timeEndPeriod(1);
    }
#endif
}

bool StCoreImpl::init(const StString& , const int& , const StNativeWin_t ) {
    if(!StVersionInfo::checkTimeBomb("sView - Core library")) {
        return false;
    }
    // TODO (Kirill Gavrilov#1) function doesn't called from StRenderer plugins!!!
    return true;
}

void StCoreImpl::closeDrawer() {
    myDrawer.nullify();
    myDrawerPath = StString();
}

bool StCoreImpl::open(const StOpenInfo& theOpenInfo) {
    StString aPluginFullPath;
    const StMIME anOpenMIME = theOpenInfo.getMIME();
    if(anOpenMIME == StDrawerInfo::CLOSE_MIME()) {
        // we got close drawer command
        closeDrawer();
        return true;
    } else if(anOpenMIME == StDrawerInfo::DRAWER_MIME()) {
        // we should open specified drawer
        aPluginFullPath = theOpenInfo.getPath();
    } else if(!myDrawer.isNull()) {
        // use already opened drawer
        return myDrawer->open(theOpenInfo);
    } else {
        // we should autodetect best drawer plugin
        ST_DEBUG_LOG_AT("StCoreImpl::open, filePath= '" + theOpenInfo.getPath() + "'; mime= '" + theOpenInfo.getMIME().toString() + "'");
        StArrayList<StDrawerInfo> aDrawerInfoList = StCore::getDrawersList();
        StString aFileExtension = StFileNode::getExtension(theOpenInfo.getPath());

        if(anOpenMIME == StMIME() && !theOpenInfo.hasPath()) {
            stError("Nothing to open with sView!\nCheck program arguments.");
            return false;
        }
        bool isDrawerFound = false;
        // firstly - search by MIME
        if(anOpenMIME != StMIME()) {
            for(size_t aDrId = 0; !isDrawerFound && (aDrId < aDrawerInfoList.size()); ++aDrId) {
                for(size_t drMimeId = 0; drMimeId < aDrawerInfoList[aDrId].getMIMEList().size(); ++drMimeId) {
                    if(anOpenMIME == aDrawerInfoList[aDrId].getMIMEList()[drMimeId]) {
                        aPluginFullPath = aDrawerInfoList[aDrId].getPath();
                        isDrawerFound = true;
                        break;
                    }
                }
            }
        }
        // if MIME not available or failed - search by extension
        for(size_t aDrId = 0; !isDrawerFound && (aDrId < aDrawerInfoList.size()); ++aDrId) {
            for(size_t drMimeId = 0; drMimeId < aDrawerInfoList[aDrId].getMIMEList().size(); ++drMimeId) {
                if(aFileExtension.isEqualsIgnoreCase(aDrawerInfoList[aDrId].getMIMEList()[drMimeId].getExtension())) {
                    aPluginFullPath = aDrawerInfoList[aDrId].getPath();
                    isDrawerFound = true;
                    break;
                }
            }
        }
        if(!isDrawerFound) {
            stError(StString("sView, Extension '") + aFileExtension + "' not supported");
            return false;
        }
    }

    // check Drawer already runned
    if(myDrawerPath == aPluginFullPath) {
        return myDrawer->open(theOpenInfo);
    }

    if(!myDrawer.isNull()) {
        // have active Drawer instance
        closeDrawer();
    }

    myDrawer = new StDrawer();
    if(!myDrawer->InitLibrary(aPluginFullPath)) {
        StString errString = StString("StDrawer Plugin \"") + aPluginFullPath + "\" loading Failed!";
        stError(errString);
        closeDrawer();
        return false;
    }
    myDrawer->Instantiate();

    myWindow->stglMakeCurrent(ST_WIN_MASTER);
    if(!myDrawer->init(myWindow)) {
        myDrawerPath = StString();
        return false;
    }
    if(!myDrawer->open(theOpenInfo)) {
        myDrawerPath = aPluginFullPath;
        return false;
    }
    // save current plugin path
    myDrawerPath = aPluginFullPath;
    return true;
}

// SDK version was used
ST_EXPORT void getSDKVersion(StVersion* theVer) {
    *theVer = StVersionInfo::getSDKVersion();
}

namespace {
    static StRenderersArray_t ST_RENDERERS_ARRAY = {NULL, 0};
    static StArrayList<StRendererInfo> ST_RENDERERS_LIST;
    static StMutex ST_LOCK_REND_DETECTION;
};

ST_EXPORT StRenderersArray_t* StCore_getStRenderers(const stBool_t theToDetectPriority) {
    StMutexAuto aLock(ST_LOCK_REND_DETECTION);
    if(ST_RENDERERS_ARRAY.count != 0) {
        return &ST_RENDERERS_ARRAY;
    }

    const StString aRenderersPath = StProcess::getStCoreFolder() + StCore::getRenderersDir();

    StLibrary::suppressSystemErrors(true); // suppress system errors
    StFolder aPluginsList(aRenderersPath);
    StArrayList<StString> anExtList(1); anExtList.add(StString(ST_DLIB_EXTENSION));
    aPluginsList.init(anExtList);
    for(size_t anIter = 0; anIter < aPluginsList.size(); ++anIter) {
        StRendererInfo aRendererInfo(aPluginsList[anIter]->getPath(), theToDetectPriority);
        if(aRendererInfo.isValid()) {
            ST_RENDERERS_LIST.add(aRendererInfo);
        }
    }
    StLibrary::suppressSystemErrors(false);

    if(ST_RENDERERS_LIST.isEmpty()) {
        return &ST_RENDERERS_ARRAY;
    }

    ST_RENDERERS_ARRAY.array = (StRendererInfo_t* )stMemAllocZeroAligned(sizeof(StRendererInfo_t) * ST_RENDERERS_LIST.size());
    ST_RENDERERS_ARRAY.count = ST_RENDERERS_LIST.size();
    for(size_t anIter = 0; anIter < ST_RENDERERS_LIST.size(); ++anIter) {
        StRendererInfo&   aRendInfo   = ST_RENDERERS_LIST[anIter];
        StRendererInfo_t& aRendStruct = ST_RENDERERS_ARRAY.array[anIter];
        aRendStruct.rendererPath = (stUtf8_t* )aRendInfo.getPath().toCString();
        aRendStruct.aboutString  = (stUtf8_t* )aRendInfo.getAboutString().toCString();

        const StStereoDeviceInfoList& aDevices = aRendInfo.getDeviceList();
        aRendStruct.devices = (StStereoDeviceInfo_t* )stMemAllocZeroAligned(sizeof(StStereoDeviceInfo_t) * aDevices.size());
        aRendStruct.count   = aDevices.size();
        for(size_t aDevIter = 0; aDevIter < aDevices.size(); ++aDevIter) {
            const StStereoDeviceInfo& aDevInfo   = aDevices[aDevIter];
            StStereoDeviceInfo_t&     aDevStruct = aRendStruct.devices[aDevIter];
            aDevStruct.stringId       = (stUtf8_t* )aDevInfo.getStringId().toCString();
            aDevStruct.name           = (stUtf8_t* )aDevInfo.getName().toCString();
            aDevStruct.description    = (stUtf8_t* )aDevInfo.getDescription().toCString();
            aDevStruct.detectionLevel = aDevInfo.getDetectionLevel();
        }
    }

    return &ST_RENDERERS_ARRAY;
}

ST_EXPORT StRendererInterface* StCore_new() {
    return new StCoreImpl(); }
ST_EXPORT void StCore_del(StRendererInterface* theInst) {
    delete (StCoreImpl* )theInst; }
ST_EXPORT StWindowInterface* StCore_getStWindow(StRendererInterface* theInst) {
    return ((StCoreImpl* )theInst)->getStWindow(); }
ST_EXPORT stBool_t StCore_init(StRendererInterface* theInst,
                               const stUtf8_t*      theRendererPath,
                               const int&           theDeviceId,
                               const StNativeWin_t  theNativeParent) {
    return ((StCoreImpl* )theInst)->init(StString(theRendererPath), theDeviceId, theNativeParent); }
ST_EXPORT stBool_t StCore_open(StRendererInterface* theInst, const StOpenInfo_t* theOpenInfo) {
    return ((StCoreImpl* )theInst)->open(StOpenInfo(theOpenInfo)); }
ST_EXPORT void StCore_callback(StRendererInterface* theInst, StMessage_t* theMessages) {
    ((StCoreImpl* )theInst)->callback(theMessages); }
ST_EXPORT void StCore_stglDraw(StRendererInterface* theInst, unsigned int theViews) {
    ((StCoreImpl* )theInst)->stglDraw(theViews); }
