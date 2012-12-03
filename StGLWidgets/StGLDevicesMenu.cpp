/**
 * Copyright © 2011-2012 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#include <StGLWidgets/StGLDevicesMenu.h>
#include <StGLWidgets/StGLMenuItem.h>
#include <StGLWidgets/StGLMessageBox.h>

#include <StCore/StCore.h>
#include <StCore/StStereoDeviceInfo_t.h>
#include <StCore/StWindow.h>

#include <StSettings/StParamRendDevice.h>
#include <StSettings/StParamDevOptOnOff.h>
#include <StSettings/StParamDevOptSwitch.h>

namespace {
    static const StString CLASS_NAME("StGLDevicesMenu");
};

const StString& StGLDevicesMenu::getClassName() {
    return CLASS_NAME;
}

StSDOptionsList_t* StGLDevicesMenu::getSharedInfo() const {
    size_t aPtrToStruct = 0;
    if(myWindow == NULL || !myWindow->getValue(ST_WIN_DATAKEYS_RENDERER, &aPtrToStruct)) {
        return NULL;
    }
    return (StSDOptionsList_t* )aPtrToStruct;
}

StGLDevicesMenu::StGLDevicesMenu(StGLWidget* theParent,
                                 StWindow* theWindow,
                                 const StString& theLabelChangeDevice,
                                 const StString& theLabelAboutPlugin,
                                 const int theOrient)
: StGLMenu(theParent, 0, 0, theOrient, false),
  myWindow(theWindow),
  myActiveDevice("Output"),
  myActiveDeviceId(0) {
    StGLMenu* aMenuChangeDevice = createChangeDeviceMenu(theParent);
    addItem(theLabelChangeDevice, aMenuChangeDevice);
    addItem(theLabelAboutPlugin)->signals.onItemClick.connect(this, &StGLDevicesMenu::doAboutRenderer);

    // add renderer device options
    StSDOptionsList_t* anOptions = getSharedInfo();
    if(anOptions == NULL) {
        return;
    }
    myActiveDeviceId = anOptions->curDeviceId;
    for(size_t optId = 0; optId < anOptions->optionsCount; ++optId) {
        switch(anOptions->options[optId]->optionType) {
            case ST_DEVICE_OPTION_ON_OFF: {
                StSDOnOff_t* anOption = (StSDOnOff_t* )anOptions->options[optId];
                addItem(StString(anOption->title), new StParamDevOptOnOff(myWindow, anOption));
                break;
            }
            case ST_DEVICE_OPTION_SWITCH: {
                StSDSwitch_t* anOption = (StSDSwitch_t* )anOptions->options[optId];
                StGLMenu* aSwitchMenu = new StGLMenu(theParent, 0, 0, StGLMenu::MENU_VERTICAL);
                for(size_t subId = 0; subId < anOption->valuesCount; subId++) {
                    aSwitchMenu->addItem(StString(anOption->valuesTitles[subId]),
                                         new StParamDevOptSwitch(myWindow, anOption), int32_t(subId));
                }
                addItem(StString(anOption->title), aSwitchMenu);
                break;
            }
            default: ST_DEBUG_LOG_AT("Unknown option type: " + anOptions->options[optId]->optionType);
        }
    }
}

StGLMenu* StGLDevicesMenu::createChangeDeviceMenu(StGLWidget* theParent) {
    StGLMenu* aMenu = new StGLMenu(theParent, 0, 0, StGLMenu::MENU_VERTICAL);
    size_t aDevGlobalId = 0;

    myActiveDevParam = new StParamRendDevice(myWindow);
    const StArrayList<StRendererInfo>& aRenderers = ((const StParamRendDevice* )myActiveDevParam.access())->getRenderers();
    size_t anActiveDev = size_t(myActiveDevParam->getValue());
    for(size_t aRendId = 0; aRendId < aRenderers.size(); ++aRendId) {
        const StStereoDeviceInfoList& aDevices = aRenderers[aRendId].getDeviceList();
        for(size_t aDevId = 0; aDevId < aDevices.size(); ++aDevId, ++aDevGlobalId) {
            aMenu->addItem(aDevices[aDevId].getName(), myActiveDevParam, int32_t(aDevGlobalId));
            if(aDevGlobalId == anActiveDev) {
                myActiveDevice = aDevices[aDevId].getName();
            }
        }
    }
    return aMenu;
}

bool StGLDevicesMenu::isDeviceChanged() {
    StSDOptionsList_t* anOptions = getSharedInfo();
    if(anOptions != NULL && myActiveDeviceId != anOptions->curDeviceId) {
        myActiveDeviceId = anOptions->curDeviceId;
        size_t aDevId = (myActiveDeviceId >= 0) ? size_t(myActiveDeviceId) : 0;
        StString anActiveRendererPath(anOptions->curRendererPath);

        const StArrayList<StRendererInfo>& aRenderers = ((const StParamRendDevice* )myActiveDevParam.access())->getRenderers();
        for(size_t aRendId = 0; aRendId < aRenderers.size(); ++aRendId) {
            const StRendererInfo& aRenderer = aRenderers[aRendId];
        #if(defined(_WIN32) || defined(__WIN32__))
            if(aRenderer.getPath().isEqualsIgnoreCase(anActiveRendererPath)) {
        #else
            if(aRenderer.getPath() == anActiveRendererPath) {
        #endif
                const StStereoDeviceInfoList& aDevList = aRenderer.getDeviceList();
                myActiveDevice = (aDevId >= aDevList.size()) ? aDevList.getLast().getName() : aDevList.getValue(aDevId).getName();
                break;
            }
        }
        return true;
    }
    return false;
}

void StGLDevicesMenu::stglUpdate(const StPointD_t& theCursorZo) {
    if(myParentItem != NULL && isDeviceChanged()) {
        myParentItem->setText(myActiveDevice);
        // update menu representation
        myParentItem->getParentMenu()->stglUpdateSubmenuLayout();
    }
    StGLWidget::stglUpdate(theCursorZo);
}

void StGLDevicesMenu::doAboutRenderer(const size_t ) {
    // read info from the active StRenderer plugin
    StSDOptionsList_t* anOptions = getSharedInfo();
    if(anOptions == NULL) {
        return;
    }
    StString anActiveRendererPath(anOptions->curRendererPath);
    StString anAboutText;

    const StArrayList<StRendererInfo>& aRenderers = ((const StParamRendDevice* )myActiveDevParam.access())->getRenderers();
    for(size_t aRendId = 0; aRendId < aRenderers.size(); ++aRendId) {
        const StRendererInfo& aRenderer = aRenderers[aRendId];
    #if(defined(_WIN32) || defined(__WIN32__))
        if(aRenderer.getPath().isEqualsIgnoreCase(anActiveRendererPath)) {
    #else
        if(aRenderer.getPath() == anActiveRendererPath) {
    #endif
            anAboutText = aRenderer.getAboutString();
            break;
        }
    }

    if(anAboutText.isEmpty()) {
        anAboutText = StString() + "Plugin '" + anActiveRendererPath + "' doesn't provide description";
    }

    StGLMessageBox* aDialog = new StGLMessageBox(getParent(), anAboutText, 512, 256);
    aDialog->setVisibility(true, true);
    aDialog->stglInit();
    aDialog->signals.onClickLeft.connect(aDialog,  &StGLMessageBox::doKillSelf);
    aDialog->signals.onClickRight.connect(aDialog, &StGLMessageBox::doKillSelf);
}
