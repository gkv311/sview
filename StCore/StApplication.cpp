/**
 * StCore, window system independent C++ toolkit for writing OpenGL applications.
 * Copyright © 2009-2017 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#include <StCore/StApplication.h>

#include <StCore/StWindow.h>
#include <StCore/StSearchMonitors.h>
#include <StSettings/StSettings.h>
#include <StGL/StGLEnums.h>
#include <StGL/StGLContext.h>
#include <StGLStereo/StFormatEnum.h>
#include <StFile/StFileNode.h>
#include <StVersion.h>

#include "StEventsBuffer.h"

namespace {

    static const StCString ST_SETTING_RENDERER_AUTO = stCString("rendererPluginAuto");
    static const StCString ST_SETTING_RENDERER      = stCString("rendererPlugin");
    static const StCString ST_SETTING_AUTO_VALUE    = stCString("Auto");
    static const StCString ST_SETTING_DEF_DRAWER    = stCString("defaultDrawer");

    /**
     * Auxiliary parameter.
     */
    class StDefaultDrawerParam : public StBoolParamNamed {

            public:

        /**
         * Main constructor.
         */
        StDefaultDrawerParam(const StApplication* theApp, const StString& theDrawer, const StString& theTitle)
        : StBoolParamNamed(theApp->isDefaultDrawer(theDrawer), StString(), theTitle),
          myApp(theApp), myAppName(theDrawer) {}

        /**
         * Return list of available translations.
         */
        virtual bool setValue(const bool theValue) ST_ATTR_OVERRIDE {
            const bool aResult = StBoolParamNamed::setValue(theValue);
            myApp->setDefaultDrawer(theValue ? myAppName : StString());
            return aResult;
        }

            private:

        const StApplication* myApp;
        StString myAppName;

    };

}

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

StApplication::StApplication(const StHandle<StResourceManager>& theResMgr,
                             const StNativeWin_t                theParentWin,
                             const StHandle<StOpenInfo>&        theOpenInfo)
: myResMgr(theResMgr),
  myMsgQueue(new StMsgQueue()),
  myEventsBuffer(new StEventsBuffer()),
  myWinParent(theParentWin),
  myRendId(ST_SETTING_AUTO_VALUE),
  myExitCode(0),
  myGlDebug(false),
  myIsOpened(false),
  myToQuit(false),
  myToRecreateMenu(false) {
    stApplicationInit(theOpenInfo);
}

void StApplication::stApplicationInit(const StHandle<StOpenInfo>& theOpenInfo) {
    if(myResMgr.isNull()) {
        myResMgr = new StResourceManager();
    }
#ifdef ST_DEBUG_GL
    myGlDebug = true;
#endif
    StSettings aGlobalSettings(myResMgr, "sview");
    params.ActiveDevice = new StEnumParam(0, stCString("activeDevice"), stCString("Change device"));
    params.ActiveDevice->signals.onChanged.connect(this, &StApplication::doChangeDevice);

    params.VSyncMode = new StEnumParam(0, stCString("vsyncMode"), stCString("VSync mode"));
    params.VSyncMode->changeValues().add("Off");
    params.VSyncMode->changeValues().add("On");
    params.VSyncMode->changeValues().add("Mixed");

    bool isOutModeAuto = true; // AUTO by default
    aGlobalSettings.loadBool(ST_SETTING_RENDERER_AUTO, isOutModeAuto);
    if(!isOutModeAuto) {
        aGlobalSettings.loadString(ST_SETTING_RENDERER, myRendId);
    }

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
    const StString ARGUMENT_GLDEBUG           = "gldebug";
    StArgument anArgRenderer = anArgs[ARGUMENT_PLUGIN_OUT];
    StArgument anArgDevice   = anArgs[ARGUMENT_PLUGIN_OUT_DEVICE];
    StArgument anArgGlDebug  = anArgs[ARGUMENT_GLDEBUG];
    if(anArgRenderer.isValid()) {
        myRendId = anArgRenderer.getValue();
    }
    //if(anArgDevice.isValid()) {
    //    aDevice = anArgDevice.getValue();
    //}
    if(anArgGlDebug.isValid()) {
        myGlDebug = true;
    }
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

    StSettings aGlobalSettings(myResMgr, "sview");
    if(!mySwitchTo.isNull()) {
        myRendId = mySwitchTo->getRendererId();
        myWindow = mySwitchTo;
        mySwitchTo.nullify();
        aGlobalSettings.saveString(ST_SETTING_RENDERER,      myRendId);
        aGlobalSettings.saveBool  (ST_SETTING_RENDERER_AUTO, false);
    } else {
        if(myRenderers.isEmpty()) {
            myWindow = new StWindow(myResMgr, myWinParent);
            myWindow->setMessagesQueue(myMsgQueue);
            myWindow->params.VSyncMode = params.VSyncMode;
        } else {
            bool isAuto = myRendId.isEqualsIgnoreCase(ST_SETTING_AUTO_VALUE);
            if(!isAuto) {
                for(size_t anIter = 0; anIter < myRenderers.size(); ++anIter) {
                    StHandle<StWindow> aWin = myRenderers[anIter];
                    if(myRendId == aWin->getRendererId()) {
                        myWindow = aWin;
                        aGlobalSettings.saveString(ST_SETTING_RENDERER,      myRendId);
                        aGlobalSettings.saveBool  (ST_SETTING_RENDERER_AUTO, isAuto);
                        break;
                    }
                }

                if(myWindow.isNull()) {
                    stError(StString("Output with id '" + myRendId + "' is not found."));
                    isAuto = true;
                }
            }

            if(isAuto) {
                // autodetection
                aGlobalSettings.saveString(ST_SETTING_RENDERER,      ST_SETTING_AUTO_VALUE);
                aGlobalSettings.saveBool  (ST_SETTING_RENDERER_AUTO, isAuto);
                myWindow = myRenderers[0];
                if(!myDevices.isEmpty()) {
                    StHandle<StOutDevice> aBestDev = myDevices[0];
                    for(size_t aDevIter = 0; aDevIter < myDevices.size(); ++aDevIter) {
                        const StHandle<StOutDevice>& aDev = myDevices[aDevIter];
                        if(aDev->Priority > aBestDev->Priority) {
                            aBestDev = aDev;
                        }
                    }
                    for(size_t anIter = 0; anIter < myRenderers.size(); ++anIter) {
                        const StHandle<StWindow>& aWin = myRenderers[anIter];
                        if(aBestDev->PluginId == aWin->getRendererId()) {
                            myWindow = aWin;
                            myWindow->setDevice(aBestDev->DeviceId);
                            break;
                        }
                    }
                }
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
            params.ActiveDevice->setValue((int32_t )aDevIter);
            break;
        }
    }

    // setup GL options before window creation
    const StWinAttr anAttribs[] = {
        StWinAttr_GlDebug, (StWinAttr )myGlDebug,
        StWinAttr_NULL
    };
    myWindow->setAttributes(anAttribs);

    myIsOpened = myWindow->create();
    if(myIsOpened) {
        // connect slots
        myWindow->signals.onRedraw    = stSlot(this, &StApplication::doDrawProxy);
        myWindow->signals.onClose     = stSlot(this, &StApplication::doClose);
        myWindow->signals.onPause     = stSlot(this, &StApplication::doPause);
        myWindow->signals.onResize    = stSlot(this, &StApplication::doResize);
        myWindow->signals.onAction    = stSlot(this, &StApplication::doAction);
        myWindow->signals.onKeyDown   = stSlot(this, &StApplication::doKeyDown);
        myWindow->signals.onKeyUp     = stSlot(this, &StApplication::doKeyUp);
        myWindow->signals.onKeyHold   = stSlot(this, &StApplication::doKeyHold);
        myWindow->signals.onMouseDown = stSlot(this, &StApplication::doMouseDown);
        myWindow->signals.onMouseUp   = stSlot(this, &StApplication::doMouseUp);
        myWindow->signals.onTouch     = stSlot(this, &StApplication::doTouch);
        myWindow->signals.onGesture   = stSlot(this, &StApplication::doGesture);
        myWindow->signals.onScroll    = stSlot(this, &StApplication::doScroll);
        myWindow->signals.onFileDrop  = stSlot(this, &StApplication::doFileDrop);
        myWindow->signals.onNavigate  = stSlot(this, &StApplication::doNavigate);
    }

    return myIsOpened;
}

void StApplication::addRenderer(const StHandle<StWindow>& theRenderer) {
    if(theRenderer.isNull()) {
        return;
    }

    StHandle<StWindow> aRenderer = theRenderer;
    aRenderer->params.VSyncMode = params.VSyncMode; // share VSync mode between renderers
    aRenderer->setMessagesQueue(myMsgQueue);
    myRenderers.add(aRenderer);
    size_t aDevIter = myDevices.size();
    aRenderer->getDevices(myDevices);
    for(; aDevIter < myDevices.size(); ++aDevIter) {
        params.ActiveDevice->changeValues().add(myDevices[aDevIter]->Name);
    }
}

void StApplication::beforeDraw() {
    //
}

void StApplication::doDrawProxy(unsigned int theView) {
    stglDraw(!myWindow.isNull() && myWindow->isStereoOutput() ? theView : ST_DRAW_MONO);
}

void StApplication::stglDraw(unsigned int ) {
    //
}

const StHandle<StWindow>& StApplication::getMainWindow() const {
    return myWindow;
}

bool StApplication::isActive() const {
    return !myWindow.isNull()
        && myWindow->isActive();
}

const StHandle<StMsgQueue>& StApplication::getMessagesQueue() const {
    return myMsgQueue;
}

int StApplication::exec() {
    if(!myIsOpened) {
        if(!open()) {
            return 1;
        }
    }

    if(!myWindow.isNull()) {
        // just debug output Monitors' configuration
        const StSearchMonitors& aMonitors = myWindow->getMonitors();
        for(size_t aMonIter = 0; aMonIter < aMonitors.size(); ++aMonIter) {
            ST_DEBUG_LOG(aMonitors[aMonIter].toString());
        }
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

bool StApplication::resetDevice() {
    return false;
}

void StApplication::doClose(const StCloseEvent& ) {
    exit(0);
}

void StApplication::doPause(const StPauseEvent& ) {
    if(!myWindow.isNull()) {
        myWindow->beforeClose();
    }
}

const StHandle<StAction>& StApplication::getAction(const int theActionId) {
    return myActions[theActionId];
    //return myActions.at(theActionId);
}

int StApplication::getActionIdFromName(const StString& theActionName) const {
    StString aNameLower = theActionName;
    aNameLower.toLowerCase();
    const std::string aName(aNameLower.toCString());
    std::map< std::string, int >::const_iterator anAction = myActionLookup.find(aName);
    return anAction != myActionLookup.end()
         ? anAction->second
         : -1;
}

void StApplication::addAction(const int                 theActionId,
                              const StHandle<StAction>& theAction) {
    myActions[theActionId] = theAction;
    if(!theAction.isNull()) {
        StString aNameLower = theAction->getName();
        aNameLower.toLowerCase();
        const std::string aName(aNameLower.toCString());
        myActionLookup[aName] = theActionId;
    }
}

void StApplication::addAction(const int           theActionId,
                              StHandle<StAction>& theAction,
                              const unsigned int  theHotKey1,
                              const unsigned int  theHotKey2) {
    theAction->setDefaultHotKey1(theHotKey1);
    theAction->setDefaultHotKey2(theHotKey2);
    addAction(theActionId, theAction);
}

StHandle<StAction> StApplication::getActionForKey(unsigned int theHKey) const {
    std::map< unsigned int, StHandle<StAction> >::const_iterator anAction = myKeyActions.find(theHKey);
    return anAction != myKeyActions.end()
         ? anAction->second
         : StHandle<StAction>();
}

void StApplication::registerHotKeys() {
    myKeyActions.clear();
    StKeysState* aKeysState = !myWindow.isNull() ? &myWindow->changeKeysState() : NULL;
    if(aKeysState != NULL) {
        aKeysState->resetRegisteredKeys();
    }
    for(std::map< int, StHandle<StAction> >::iterator anIter = myActions.begin();
        anIter != myActions.end(); ++anIter) {
        const StHandle<StAction>& anAction = anIter->second;
        for(int aHotIter = 0; aHotIter < 2; ++aHotIter) {
            const unsigned int aHotKey = anAction->getHotKey(aHotIter);
            if(aHotKey == 0) {
                continue;
            }

            StHandle<StAction> anOldAction = getActionForKey(aHotKey);
            if(!anOldAction.isNull()) {
                for(int anOldHotIter = 0; anOldHotIter < 2; ++anOldHotIter) {
                    if(anOldAction->getHotKey(anOldHotIter) == aHotKey) {
                        anOldAction->setHotKey(anOldHotIter, 0);
                    }
                }
            }
            myKeyActions[aHotKey] = anAction;
            if(aKeysState != NULL) {
                const StVirtKey aVirtKey = getBaseKeyFromHotKey(aHotKey);
                aKeysState->registerKey(aVirtKey);
            }
        }
    }
}

void StApplication::invokeAction(const int    theActionId,
                                 const double theProgress) {
    StEvent anEvent;
    anEvent.Type            = stEvent_Action;
    anEvent.Action.ActionId = theActionId;
    anEvent.Action.Progress = theProgress;
    myEventsBuffer->append(anEvent);
}

void StApplication::doAction(const StActionEvent& theEvent) {
    std::map< int, StHandle<StAction> >::iterator anAction = myActions.find(theEvent.ActionId);
    if(anAction != myActions.end()) {
        anAction->second->doTrigger((const StEvent* )&theEvent);
    }
}

void StApplication::doKeyDown(const StKeyEvent& theEvent) {
    std::map< unsigned int, StHandle<StAction> >::iterator anAction = myKeyActions.find(theEvent.VKey | theEvent.Flags);
    if(anAction != myKeyActions.end()
    && !anAction->second->isHoldKey()) {
        anAction->second->doTrigger((const StEvent* )&theEvent);
    }
}

void StApplication::doKeyHold(const StKeyEvent& theEvent) {
    std::map< unsigned int, StHandle<StAction> >::iterator anAction = myKeyActions.find(theEvent.VKey | theEvent.Flags);
    if(anAction != myKeyActions.end()
    && anAction->second->isHoldKey()) {
        anAction->second->doTrigger((const StEvent* )&theEvent);
    }
}

void StApplication::doKeyUp    (const StKeyEvent&    ) {}

void StApplication::doResize   (const StSizeEvent&   ) {}
void StApplication::doMouseDown(const StClickEvent&  ) {}
void StApplication::doMouseUp  (const StClickEvent&  ) {}
void StApplication::doTouch    (const StTouchEvent&  ) {}
void StApplication::doGesture  (const StGestureEvent&) {}
void StApplication::doScroll   (const StScrollEvent& ) {}
void StApplication::doFileDrop (const StDNDropEvent& ) {}
void StApplication::doNavigate (const StNavigEvent&  ) {}

void StApplication::processEvents() {
    if(myWindow.isNull() || !myIsOpened) {
        return; // nothing to do
    }

    if(myToQuit) {
        // force Render to quit
        myWindow->beforeClose();
        myWindow->close();
        myIsOpened = false;
        myToQuit   = false;
        return;
    }

    // common callback call
    myWindow->processEvents();

    // application-specific queued events
    myEventsBuffer->swapBuffers();
    for(size_t anEventIter = 0; anEventIter < myEventsBuffer->getSize(); ++anEventIter) {
        StEvent& anEvent = myEventsBuffer->changeEvent(anEventIter);
        if(anEvent.Type == stEvent_Action) {
            doAction(anEvent.Action);
        }
    }

    // draw iteration
    beforeDraw();
    myWindow->stglDraw();

    const StString aDevice = myWindow->getDeviceId();
    const int32_t  aDevNum = params.ActiveDevice->getValue();
    if(!mySwitchTo.isNull()) {
        if(!resetDevice()) {
            myToQuit = true;
        }
        mySwitchTo.nullify();
    } else if(myWindow->isLostDevice()) {
        mySwitchTo = myWindow;
        if(!resetDevice()) {
            myToQuit = true;
        }
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
    const StString ARGUMENT_DEMO              = "demo";
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
            } else if(anArg.getKey().isEqualsIgnoreCase(ARGUMENT_DEMO)) {
                StString aFilePath = StProcess::getAbsolutePath(anArg.getValue());
                anArg.setValue(aFilePath);
                anInfo->setPath(aFilePath);
                anOpenFileArgs.add(anArg);
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

bool StApplication::isDefaultDrawer(const StString& theDrawer) const {
    StSettings aGlobalSettings(myResMgr, "sview");
    StString aDefDrawer;
    aGlobalSettings.loadString(ST_SETTING_DEF_DRAWER, aDefDrawer);
    return theDrawer == aDefDrawer;
}

void StApplication::setDefaultDrawer(const StString& theDrawer) const {
    StSettings aGlobalSettings(myResMgr, "sview");
    aGlobalSettings.saveString(ST_SETTING_DEF_DRAWER, theDrawer);
}

bool StApplication::readDefaultDrawer(StHandle<StOpenInfo>& theInfo) {
    StHandle<StResourceManager> aResMgr = new StResourceManager();
    StSettings aGlobalSettings(aResMgr, "sview");
    StString aDefDrawer;
    aGlobalSettings.loadString(ST_SETTING_DEF_DRAWER, aDefDrawer);
    if(aDefDrawer.isEmpty()) {
        return false;
    }

    if(theInfo.isNull()) {
        theInfo = new StOpenInfo();
    }
    StArgumentsMap anArgs = theInfo->getArgumentsMap();
    anArgs.add(StArgument("in", aDefDrawer));
    theInfo->setArgumentsMap(anArgs);
    return true;
}

StHandle<StBoolParamNamed> StApplication::createDefaultDrawerParam(const StString& theDrawer,
                                                                   const StString& theTitle) const {
    return new StDefaultDrawerParam(this, theDrawer, theTitle);
}

void StApplication::doChangeLanguage(const int32_t ) {
    myToRecreateMenu = true;
    myLangMap->resetReloaded();

    for(size_t aRendIter = 0; aRendIter < myRenderers.size(); ++aRendIter) {
        StHandle<StWindow>& aRend = myRenderers[aRendIter];
        aRend->doChangeLanguage();
    }

    for(size_t aDevIter = 0; aDevIter < myDevices.size(); ++aDevIter) {
        params.ActiveDevice->defineOption((int32_t )aDevIter, myDevices[aDevIter]->Name);
    }
}

bool StApplication::doExitOnEscape(StApplication::ActionOnEscape theAction) {
    const bool isFullscreen = myWindow->hasFullscreenMode()
                           && myWindow->isFullScreen();
    switch(theAction) {
        case ActionOnEscape_Nothing: {
            return false;
        }
        case ActionOnEscape_ExitOneClick: {
            StApplication::exit(0);
            return true;
        }
        case ActionOnEscape_ExitDoubleClick: {
            if(isFullscreen) {
                return false;
            }

            const double aTimeSec = myExitTimer.getElapsedTimeInSec();
            if(myExitTimer.isOn()
            && aTimeSec < 0.5) {
                StApplication::exit(0);
                return true;
            }
            myExitTimer.restart();
            return false;
        }
        case ActionOnEscape_ExitOneClickWindowed: {
            if(isFullscreen) {
                return false;
            }
            StApplication::exit(0);
            return true;
        }
    }
    return false;
}
