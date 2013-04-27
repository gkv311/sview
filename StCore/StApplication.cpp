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

#include <StCore/StApplication.h>

#include <StCore/StWindow.h>
#include <StSettings/StSettings.h>
#include <StGL/StGLEnums.h>
#include <StGL/StGLContext.h>
#include <StGLStereo/StFormatEnum.h>
#include <StFile/StFileNode.h>
#include <StVersion.h>

namespace {
    const char ST_SETTING_RENDERER_AUTO[] = "rendererPluginAuto";
    const char ST_SETTING_RENDERER[]      = "rendererPlugin";
    const char ST_SETTING_AUTO_VALUE[]    = "Auto";
};

void StApplication::doChangeDevice(const int32_t theValue) {
    if(myWindow.isNull() || !myIsOpened
    || theValue < 0
    || size_t(theValue) >= myDevices.size()) {
        return;
    }

    mySwitchTo.nullify();
    const StHandle<StOutDevice>& aDev = myDevices[theValue];
    for(size_t aRendIter = 0; aRendIter < myRenderers.size(); ++aRendIter) {
        StHandle<StWindow>& aRend = myRenderers[aRendIter];
        if(aDev->PluginId == aRend->getRendererId()) {
            if(aRend->setDevice(aDev->DeviceId)
            || aRend != myWindow) {
                mySwitchTo = aRend;
            }
            break;
        }
    }
}

StApplication::StApplication(const StNativeWin_t         theParentWin,
                             const StHandle<StOpenInfo>& theOpenInfo)
: myWinParent(theParentWin),
  myRendId(ST_SETTING_AUTO_VALUE),
  myExitCode(0),
  myIsOpened(false),
  myToQuit(false) {
    myGlobalSettings = new StSettings("sview");
    params.ActiveDevice = new StEnumParam(0, "Change device");
    params.ActiveDevice->signals.onChanged.connect(this, &StApplication::doChangeDevice);

    bool isOutModeAuto = true; // AUTO by default
    myGlobalSettings->loadBool(ST_SETTING_RENDERER_AUTO, isOutModeAuto);
    if(!isOutModeAuto) {
        myGlobalSettings->loadString(ST_SETTING_RENDERER, myRendId);
    }

    stMemSet(myMessages, 0, sizeof(myMessages));

    // add additional paths
#ifdef _WIN32
    // requires Windows XP with SP1 or higher
    StStringUtfWide aStCoreFolder = StProcess::getStCoreFolder().toUtfWide();
    SetDllDirectoryW(aStCoreFolder.toCString());
#endif
    myOpenFileInfo = theOpenInfo;
    if(myOpenFileInfo.isNull()) {
        myOpenFileInfo = parseProcessArguments();
    }
    if(myOpenFileInfo.isNull()) {
        myOpenFileInfo = new StOpenInfo();
    }

    const StArgumentsMap anArgs = myOpenFileInfo->getArgumentsMap();
    const StString ARGUMENT_PLUGIN_OUT        = "out";
    const StString ARGUMENT_PLUGIN_OUT_DEVICE = "outDevice";
    StArgument anArgRenderer = anArgs[ARGUMENT_PLUGIN_OUT];
    StArgument anArgDevice   = anArgs[ARGUMENT_PLUGIN_OUT_DEVICE];
    if(anArgRenderer.isValid()) {
        myRendId = anArgRenderer.getValue();
    }
    //if(anArgDevice.isValid()) {
    //    aDevice = anArgDevice.getValue();
    //}
}

StApplication::~StApplication() {
    //
}

StString StApplication::getAboutString() const {
    return "No about information";
}

bool StApplication::closingDown() const {
    return !myIsOpened;
}

bool StApplication::open(const StOpenInfo& theOpenInfo) {
    *myOpenFileInfo = theOpenInfo;
    return open();
}

bool StApplication::open() {
    if(!myWindow.isNull()) {
        return true;
    }

    if(!mySwitchTo.isNull()) {
        myRendId = mySwitchTo->getRendererId();
        myWindow = mySwitchTo;
        mySwitchTo.nullify();
        myGlobalSettings->saveString(ST_SETTING_RENDERER,      myRendId);
        myGlobalSettings->saveBool  (ST_SETTING_RENDERER_AUTO, false);
    } else {
        if(myRenderers.isEmpty()) {
            myWindow = new StWindow(myWinParent);
        } else {
            bool isAuto = myRendId.isEqualsIgnoreCase(ST_SETTING_AUTO_VALUE);
            if(!isAuto) {
                for(size_t anIter = 0; anIter < myRenderers.size(); ++anIter) {
                    StHandle<StWindow> aWin = myRenderers[anIter];
                    if(myRendId == aWin->getRendererId()) {
                        myWindow = aWin;
                        myGlobalSettings->saveString(ST_SETTING_RENDERER,      myRendId);
                        myGlobalSettings->saveBool  (ST_SETTING_RENDERER_AUTO, isAuto);
                        break;
                    }
                }

                if(myWindow.isNull()) {
                    stError(StString("Output with id '" + myRendId + "' is not found."));
                    isAuto = true;
                }
            }

            if(isAuto) {
                myGlobalSettings->saveString(ST_SETTING_RENDERER,      ST_SETTING_AUTO_VALUE);
                myGlobalSettings->saveBool  (ST_SETTING_RENDERER_AUTO, isAuto);
                myRenderers.sort(); // sort by priority
                myWindow = myRenderers[0];
            }
        }
        myWindow->setTitle(myTitle);
    }

    // synchronize devices enumeration
    const StString aPluginId = myWindow->getRendererId();
    const StString aDeviceId = myWindow->getDeviceId();
    for(size_t aDevIter = 0; aDevIter < myDevices.size(); ++aDevIter) {
        const StHandle<StOutDevice>& aDev = myDevices[aDevIter];
        if(aPluginId == aDev->PluginId
        && aDeviceId == aDev->DeviceId) {
            params.ActiveDevice->setValue(aDevIter);
            break;
        }
    }

    myIsOpened = myWindow->create();
    if(myIsOpened) {
        myWindow->signals.onRedraw.connect(this, &StApplication::stglDraw);
    }

    return myIsOpened;
}

void StApplication::addRenderer(const StHandle<StWindow>& theRenderer) {
    if(theRenderer.isNull()) {
        return;
    }

    myRenderers.add(theRenderer);
    size_t aDevIter = myDevices.size();
    theRenderer->getDevices(myDevices);
    for(; aDevIter < myDevices.size(); ++aDevIter) {
        params.ActiveDevice->changeValues().add(myDevices[aDevIter]->Name);
    }
}

void StApplication::stglDraw(unsigned int ) {
    //
}

const StHandle<StWindow>& StApplication::getMainWindow() const {
    return myWindow;
}

int StApplication::exec() {
    if(!myIsOpened) {
        open();
    }

    for(; !myWindow.isNull() && myIsOpened;) {
        processEvents();
    }
    return myExitCode;
}

void StApplication::exit(const int theExitCode) {
    myExitCode = theExitCode;
    myToQuit   = true;
}

void StApplication::processEvents(const StMessage_t* ) {
    //
}

bool StApplication::resetDevice() {
    return false;
}

void StApplication::processEvents() {
    if(myWindow.isNull() || !myIsOpened) {
        return; // nothing to do
    }

    if(myToQuit) {
        // force Render to quit
        myMessages[0].uin = StMessageList::MSG_EXIT;
        myMessages[1].uin = StMessageList::MSG_NULL;
        // be sure Render plugin process quit correctly
        myWindow->processEvents(myMessages);
        myWindow->close();
        myIsOpened = false;
        myToQuit   = false;
        return;
    }

    // common callback call
    myMessages[0].uin = StMessageList::MSG_NULL;
    myWindow->processEvents(myMessages);
    processEvents(myMessages);

    // draw iteration
    myWindow->stglDraw();

    const StString aDevice = myWindow->getDeviceId();
    const int32_t  aDevNum = params.ActiveDevice->getValue();
    if(!mySwitchTo.isNull()) {
        resetDevice();
        mySwitchTo.nullify();
    } else if(myWindow->isLostDevice()) {
        mySwitchTo = myWindow;
        resetDevice();
        mySwitchTo.nullify();
    } else if(aDevNum >= 0
           && size_t(aDevNum) < myDevices.size()
           && aDevice != myDevices[aDevNum]->DeviceId) {
        // device was changed by renderer itself - synchronize value
        const StString aPlugin = myWindow->getRendererId();
        for(size_t aDevIter = 0; aDevIter < myDevices.size(); ++aDevIter) {
            const StHandle<StOutDevice>& anOutDev = myDevices[aDevIter];
            if(aPlugin == anOutDev->PluginId
            && aDevice == anOutDev->DeviceId) {
                params.ActiveDevice->setValue((int32_t )aDevIter);
                break;
            }
        }
    }
}

StHandle<StOpenInfo> StApplication::parseProcessArguments() {
    StHandle<StOpenInfo> anInfo = new StOpenInfo();

    StArrayList<StString> anArguments = StProcess::getArguments();
    StArgumentsMap anOpenFileArgs;
    size_t aFilesCount = 0;
    bool isFilesSection = false;
    const StString ARGUMENT_FILES_SECTION     = '-';
    const StString ARGUMENT_ANY               = "--";
    const StString ARGUMENT_HELP              = "help";
    const StString ARGUMENT_FILE              = "file";
    const StString ARGUMENT_LEFT_VIEW         = "left";
    const StString ARGUMENT_RIGHT_VIEW        = "right";
    // parse extra parameters
    for(size_t aParamIter = 1; aParamIter < anArguments.size(); ++aParamIter) {
        StString aParam = anArguments[aParamIter];
        ///ST_DEBUG_LOG("aParam= '" + aParam + "'");
        if(isFilesSection) {
            // file name
            StString aFilePath = StProcess::getAbsolutePath(aParam);
            anOpenFileArgs.add(StArgument(ARGUMENT_FILE + aFilesCount++, aFilePath));
            if(!anInfo->hasPath()) {
                // first file determines MIME type (needed to autoselect Drawer plugin)
                anInfo->setPath(aFilePath);
            }
        } else if(aParam == ARGUMENT_FILES_SECTION) {
            isFilesSection = true;
        } else if(aParam.isStartsWith(ARGUMENT_ANY)) {
            // argument
            StArgument anArg; anArg.parseString(aParam.subString(2, aParam.getLength())); // cut suffix --

            if(anArg.getKey().isEqualsIgnoreCase(ARGUMENT_HELP)) {
                return NULL;
            } else if(anArg.getKey().isEqualsIgnoreCase(ARGUMENT_LEFT_VIEW)) {
                // left view
                anArg.setValue(StProcess::getAbsolutePath(anArg.getValue()));
                anOpenFileArgs.add(anArg);
                anInfo->setPath(anArg.getValue()); // left file always determines MIME type
            } else if(anArg.getKey().isEqualsIgnoreCase(ARGUMENT_RIGHT_VIEW)) {
                // right view
                anArg.setValue(StProcess::getAbsolutePath(anArg.getValue()));
                anOpenFileArgs.add(anArg);
                if(!anInfo->hasPath()) {
                    anInfo->setPath(anArg.getValue());
                }
            } else {
                // pass argument unchanged
                anOpenFileArgs.add(anArg);
            }
        } else {
            // file name
            StString aFilePath = StProcess::getAbsolutePath(aParam);
            anOpenFileArgs.add(StArgument(ARGUMENT_FILE + aFilesCount++, aFilePath));
            if(!anInfo->hasPath()) {
                // first file determines MIME type (needed to autoselect Drawer plugin)
                anInfo->setPath(aFilePath);
            }
        }
    }

    anInfo->setArgumentsMap(anOpenFileArgs);
    return anInfo;
}
