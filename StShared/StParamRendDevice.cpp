/**
 * Copyright © 2011-2012 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#include <StSettings/StParamRendDevice.h>

#include <StCore/StStereoDeviceInfo_t.h>
#include <StCore/StWindow.h>

StSDOptionsList_t* StParamRendDevice::getSharedInfo() const {
    size_t aPtrToStruct = 0;
    if(myWindow == NULL || !myWindow->getValue(ST_WIN_DATAKEYS_RENDERER, &aPtrToStruct)) {
        return NULL;
    }
    return (StSDOptionsList_t* )aPtrToStruct;
}

const StArrayList<StRendererInfo>& StParamRendDevice::getRenderers() const {
    return myRenderers;
}

StParamRendDevice::StParamRendDevice(StWindow* theWindow)
: StInt32Param(0),
  myWindow(theWindow),
  myPluginBase(0),
  myPluginLim(0) {
    // support level not yet supported here
    StCore::getRenderersList(myRenderers, false);

    StSDOptionsList_t* anOptions = getSharedInfo();
    if(anOptions == NULL) {
        // something wrong
        return;
    }
    StString anActivePlugin(anOptions->curRendererPath);
    for(size_t aRendId = 0; aRendId < myRenderers.size(); ++aRendId) {
        const StRendererInfo& aRenderer = myRenderers[aRendId];
    #if(defined(_WIN32) || defined(__WIN32__))
        if(aRenderer.getPath().isEqualsIgnoreCase(anActivePlugin)) {
    #else
        if(aRenderer.getPath() == anActivePlugin) {
    #endif
            myPluginLim = int32_t(aRenderer.getDeviceList().size());
            return;
        }
        myPluginBase += aRenderer.getDeviceList().size();
    }
    // something wrong...
}

int32_t StParamRendDevice::getValue() const {
    StSDOptionsList_t* anOptions = getSharedInfo();
    if(anOptions == NULL) {
        // something wrong...
        return 0;
    }

    // got the device id in active plugin
    int aDevId = anOptions->curDeviceId;
    if(aDevId < 0) {
        aDevId = 0;
    } else if(aDevId >= myPluginLim) {
        aDevId = myPluginLim - 1;
    }

    // return device id in global devices list
    return int32_t(myPluginBase) + aDevId;
}

bool StParamRendDevice::setValue(const int32_t theValue) {
    if(getValue() == theValue) {
        return false;
    }

    StSDOptionsList_t* anOptions = getSharedInfo();
    if(anOptions == NULL) {
        // something wrong...
        return false;
    }

    size_t aNewDevId = size_t(theValue);
    size_t aPluginBase = 0;
    size_t aNextPluginBase = 0;
    StString anActivePlugin(anOptions->curRendererPath);
    for(size_t aRendId = 0; aRendId < myRenderers.size(); ++aRendId) {
        const StRendererInfo& aRenderer = myRenderers[aRendId];
        aNextPluginBase = aPluginBase + aRenderer.getDeviceList().size();
        if(aNextPluginBase > aNewDevId) {
            anOptions->curDeviceId = int(aNewDevId - aPluginBase);
        #if(defined(_WIN32) || defined(__WIN32__))
            if(!anActivePlugin.isEqualsIgnoreCase(aRenderer.getPath())) {
        #else
            if(anActivePlugin != aRenderer.getPath()) {
        #endif
                StWindow::memFree(anOptions->curRendererPath);
                anOptions->curRendererPath = StWindow::memAllocNCopy(aRenderer.getPath());
            }
            StMessage_t aMsg; aMsg.uin = StMessageList::MSG_DEVICE_INFO; aMsg.data = NULL;
            myWindow->appendMessage(aMsg);
            break;
        }
        aPluginBase = aNextPluginBase;
    }
    // emit connected slots
    signals.onChanged(theValue);
    return true;
}
