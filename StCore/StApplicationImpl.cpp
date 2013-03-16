/**
 * Copyright © 2009-2012 Kirill Gavrilov <kirill@sview.ru>
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

#include "StApplicationImpl.h"

#include <StCore/StCore.h>
#include <StCore/StWindowInterface.h>
#include <StSettings/StSettings.h>
#include <StStrings/stConsole.h>
#include <StGL/StGLEnums.h>
#include <StGL/StGLContext.h>
#include <StGLStereo/StFormatEnum.h>
#include <StFile/StFileNode.h>

// Exported class-methods wrappers in StCore library (here!)
ST_EXPORT stBool_t StWindow_getValue(StWindowInterface* theInst, const size_t& , size_t* );
ST_EXPORT stBool_t StWindow_isFullScreen(StWindowInterface* theInst);
ST_EXPORT StRenderersArray_t* StCore_getStRenderers(const stBool_t theToDetectPriority);

namespace {

    void getRenderersList(StArrayList<StRendererInfo>& theList,
                          const bool                   theToDetectPriority) {
        theList.clear();
        StRenderersArray_t* anArray = StCore_getStRenderers(theToDetectPriority);
        if(anArray == NULL || anArray->count == 0) {
            return;
        }

        for(size_t aRendIter = 0; aRendIter < anArray->count; ++aRendIter) {
            theList.add(StRendererInfo(anArray->array[aRendIter]));
        }
    }

};

StApplicationImpl::StApplicationImpl()
: myParamDeviceInt(StRendererInfo::DEVICE_AUTO),
  myNativeWinParent((StNativeWin_t )NULL),
  myIsOpened(false),
  myIsFullscreen(false),
  myToQuit(false) {
    stMemSet(myMessages, 0, sizeof(myMessages));

    // add additional paths
#if(defined(_WIN32) || defined(__WIN32__))
    // requires Windows XP with SP1 or higher!
    StStringUtfWide aStCoreFolder = StProcess::getStCoreFolder().toUtfWide();
    SetDllDirectoryW(aStCoreFolder.toCString());
#endif
    parseProcessArguments();
}

StApplicationImpl::~StApplicationImpl() {
    //
}

bool StApplicationImpl::chooseRendererPlugin() {
    const StString ST_SETTING_RENDERER_AUTO = "rendererPluginAuto";
    const StString ST_SETTING_RENDERER      = "rendererPlugin";
    const StString ST_SETTING_AUTO_VALUE    = "Auto";

    StSettings aSettings("sview");
    bool isOutModeAuto = true; // AUTO by default

    if(myParamRenderer.isEqualsIgnoreCase(ST_SETTING_AUTO_VALUE)) {
        // force autodetection
        aSettings.saveString(ST_SETTING_RENDERER, ST_SETTING_AUTO_VALUE);
        aSettings.saveBool(ST_SETTING_RENDERER_AUTO, isOutModeAuto);
    } else if(!myParamRenderer.isEmpty()) {
        // renderer module selected by program attribute
        if(!myParamRenderer.isContains(SYS_FS_SPLITTER)) {
            // got the title
            myParamRenderer = myStCorePath + StCore::getRenderersDir() + SYS_FS_SPLITTER + myParamRenderer + ST_DLIB_SUFFIX;
        }
        StRendererInfo aSetRendererInfo(myParamRenderer, false);
        if(aSetRendererInfo.isValid() && myRenderer.InitLibrary(aSetRendererInfo.getPath())) {
            // save plugin info
            StArrayList<StRendererInfo> aRendList;
            getRenderersList(aRendList, false);
            if(aRendList.contains(aSetRendererInfo)) {
                aSettings.saveString(ST_SETTING_RENDERER, aSetRendererInfo.getTitle());
            } else {
                aSettings.saveString(ST_SETTING_RENDERER, aSetRendererInfo.getPath());
            }
            isOutModeAuto = false;
            aSettings.saveBool(ST_SETTING_RENDERER_AUTO, isOutModeAuto);
            return true;
        } else {
            stError(StString("Failed to load StRenderer Plugin:\n'") + aSetRendererInfo.getPath() + '\'');
        }
    }

    // load saved parameter
    aSettings.loadBool(ST_SETTING_RENDERER_AUTO, isOutModeAuto);
    if(!isOutModeAuto) {
        aSettings.loadString(ST_SETTING_RENDERER, myParamRenderer);
        if(myParamRenderer.isEqualsIgnoreCase(ST_SETTING_AUTO_VALUE)) {
            // still auto in data!
            isOutModeAuto = true;
            aSettings.saveBool(ST_SETTING_RENDERER_AUTO, isOutModeAuto);
        } else {
            if(!myParamRenderer.isContains(SYS_FS_SPLITTER)) {
                // got the title
                myParamRenderer = myStCorePath + StCore::getRenderersDir() + SYS_FS_SPLITTER + myParamRenderer + ST_DLIB_SUFFIX;
            }
            StRendererInfo aSetRendererInfo(myParamRenderer, false);
            if(aSetRendererInfo.isValid() && myRenderer.InitLibrary(aSetRendererInfo.getPath())) {
                // save plugin info
                StArrayList<StRendererInfo> aRendList;
                getRenderersList(aRendList, false);
                if(aRendList.contains(aSetRendererInfo)) {
                    aSettings.saveString(ST_SETTING_RENDERER, aSetRendererInfo.getTitle());
                } else {
                    aSettings.saveString(ST_SETTING_RENDERER, aSetRendererInfo.getPath());
                }
                aSettings.saveBool(ST_SETTING_RENDERER_AUTO, isOutModeAuto);
                return true;
            } else {
                stError(StString("Failed to load StRenderer Plugin:\n'") + aSetRendererInfo.getPath() + '\'');
            }
        }
    }
    aSettings.saveString(ST_SETTING_RENDERER, ST_SETTING_AUTO_VALUE);
    aSettings.saveInt32(ST_SETTING_RENDERER_AUTO, isOutModeAuto);

    StArrayList<StRendererInfo> aRendList;
    getRenderersList(aRendList, true);
    aRendList.sort();
    for(size_t aRendId = aRendList.size() - 1; aRendId < size_t(-1); --aRendId) {
        if(myRenderer.InitLibrary(aRendList[aRendId].getPath())) {
            return true;
        }
    }
    return false;
}

bool StApplicationImpl::create(const StNativeWin_t theNativeWinParent) {
    if(!chooseRendererPlugin()) {
        stError("StRenderer plugin (for stereo-device support) not available!");
        return false;
    }

    myNativeWinParent = theNativeWinParent;

    // TODO (Kirill Gavrilov#9) try open another renderer if initialization failes
    myRenderer.Instantiate();
    bool aResult = myRenderer.init(myParamDeviceInt, myNativeWinParent);
    ST_DEBUG_LOG(StGLContext::stglInfo());
    return aResult;
}

bool StApplicationImpl::open(const StOpenInfo& theOpenInfo) {
    if(theOpenInfo.getMIME() == StDrawerInfo::CLOSE_MIME()) {
        if(isOpened()) {
            myRenderer.open(theOpenInfo);
            myToQuit = true;
        }
        return true;
    } else if(theOpenInfo.getMIME() == StDrawerInfo::DRAWER_MIME()) {
        myDrawerInfo = theOpenInfo;
        myIsOpened   = myRenderer.open(theOpenInfo);
    } else if(!theOpenInfo.isEmpty()) {
        myOpenFileInfo = theOpenInfo;
        myIsOpened     = myRenderer.open(theOpenInfo);
    } else {

        // use stored info (previous opened file or command line arguments)
        ST_DEBUG_LOG_AT("use command line arguments");
        if(!myDrawerInfo.isEmpty()) {
            if(!myRenderer.open(myDrawerInfo)) {
                myIsOpened = false;
                return myIsOpened;
            }
        } else if(!myShowHelpString.isEmpty()) {
            // just show help
            st::cout << myShowHelpString;
            stInfo(myShowHelpString);
            myIsOpened = false;
            return myIsOpened;
        }

        myIsOpened = myOpenFileInfo.isEmpty() || myRenderer.open(myOpenFileInfo);
    }
    return myIsOpened;
}

void StApplicationImpl::callback(StMessage_t* theMessages) {
    if(!isOpened()) {
        return; // nothing to do
    }
    StMessage_t* aUsedMessages = theMessages;
    // if application need not parse callback events - use internal buffer
    if(theMessages == NULL) {
        aUsedMessages = myMessages;
    }

    if(myToQuit) {
        // force Render to quit
        aUsedMessages[0].uin = StMessageList::MSG_EXIT;
        aUsedMessages[1].uin = StMessageList::MSG_NULL;
        // be sure Render plugin process quit correctly
        myRenderer.callback(aUsedMessages);
        myIsOpened = false;
        myToQuit   = false;
        return;
    }

    // common callback call
    myRenderer.callback(aUsedMessages);
    for(size_t i = 0; aUsedMessages[i].uin != StMessageList::MSG_NULL; ++i) {
        switch(aUsedMessages[i].uin) {
            case StMessageList::MSG_EXIT: {
                // TODO (Kirill Gavrilov#1)
                myIsOpened = false;
                return;
            }
            case StMessageList::MSG_DEVICE_INFO: {
                int newDeviceId(StRendererInfo::DEVICE_AUTO);

                size_t ptrToStruct = 0;
                if(!StWindow_getValue(myRenderer.getStWindow()->getLibImpl(), ST_WIN_DATAKEYS_RENDERER, &ptrToStruct) || (ptrToStruct == 0)) {
                    break;
                }
                StSDOptionsList_t* optionsStruct = (StSDOptionsList_t* )ptrToStruct;
                StString newPluginPath(optionsStruct->curRendererPath);
                newDeviceId = optionsStruct->curDeviceId;

                // force Renderer to quit, plugin should disable this message if parsed it himself
                StOpenInfo stCloseInfo;
                stCloseInfo.setMIME(StDrawerInfo::CLOSE_MIME());
                if(isOpened()) {
                    myRenderer.open(stCloseInfo);
                }
                aUsedMessages[0].uin = StMessageList::MSG_EXIT;
                aUsedMessages[1].uin = StMessageList::MSG_NULL;

                // be sure Render plugin process quit correctly
                myRenderer.callback(aUsedMessages);
                myIsOpened = false;
                myToQuit   = false;

                // open new renderer
                myParamRenderer  = newPluginPath;
                myParamDeviceInt = newDeviceId;
                myRenderer.Destruct();

                if(!chooseRendererPlugin()) {
                    stError("StRenderer plugin (for stereo-device support) not available!");
                    return;
                }

                myRenderer.Instantiate();
                if(myRenderer.init(myParamDeviceInt, myNativeWinParent)) {
                    /// TODO (Kirill Gavrilov#5) get up-to-date opened file info
                    open(StOpenInfo());
                }
                return;
            }
        }
    }

    // draw iteration
    myRenderer.stglDraw(ST_DRAW_BOTH);
    myIsFullscreen = StWindow_isFullScreen(myRenderer.getStWindow()->getLibImpl());
}

static StString getAboutString(const StString& theProgramPath) {
    /// TODO (Kirill Gavrilov#9) call plugins for full help list
    StString anAboutString =
        StString("sView ") + StVersionInfo::getSDKVersionString() + '\n'
        + "Copyright (C) 2007-2013 Kirill Gavrilov (kirill@sview.ru).\n"
        + "Usage: " + theProgramPath + " [options] - file\n"
        + "Available options:\n"
          "  --fullscreen         Open fullscreen\n"
          "  --slideshow          Start slideshow\n"
          "  --in=DRAWER_PATH     Open StDrawer plugin (predefined values: image, video)\n"
          "  --out=RENDERER_PATH  Open StRenderer plugin (auto, StOutAnaglyph, StOutDual,...)\n"
          "  --imageLib=IMGLIB    Setup 3rd-party library for image processing (FFmpeg, FreeImage, DevIL)\n"
          "  --viewMode=MODE      View mode (flat, sphere)\n"
          "  --srcFormat=FORMAT   Setup source format:\n"
          "                       "
            + st::formatToString(ST_V_SRC_AUTODETECT)    + ", "
            + st::formatToString(ST_V_SRC_MONO)          + ", "
            + st::formatToString(ST_V_SRC_SIDE_BY_SIDE)  + ", "
            + st::formatToString(ST_V_SRC_PARALLEL_PAIR) + ",\n"
        + "                       "
            + st::formatToString(ST_V_SRC_OVER_UNDER_LR) + ", "
            + st::formatToString(ST_V_SRC_OVER_UNDER_RL) + ", "
            + st::formatToString(ST_V_SRC_ROW_INTERLACE) + ", "
            + st::formatToString(ST_V_SRC_PAGE_FLIP)     + '\n';
    return anAboutString;
}

void StApplicationImpl::parseProcessArguments() {
    StArrayList<StString> stArguments = StProcess::getArguments();
    StArgumentsMap stDrawerArgs;
    StArgumentsMap stOpenFileArgs;
    size_t filesCount = 0;
    myStCorePath = StProcess::getStCoreFolder();
    bool isFilesSection = false;
    const StString ARGUMENT_FILES_SECTION     = '-';
    const StString ARGUMENT_ANY               = "--";
    const StString ARGUMENT_HELP              = "help";
    const StString ARGUMENT_PLUGIN_IN         = "in";
    const StString ARGUMENT_PLUGIN_OUT        = "out";
    const StString ARGUMENT_PLUGIN_OUT_DEVICE = "outDevice";
    const StString ARGUMENT_FILE              = "file";
    const StString ARGUMENT_LEFT_VIEW         = "left";
    const StString ARGUMENT_RIGHT_VIEW        = "right";
    // parse extra parameters
    for(size_t p = 1; p < stArguments.size(); ++p) {
        StString param = stArguments[p];
        ///ST_DEBUG_LOG("param= '" + param + "'");
        if(isFilesSection) {
            // file name
            StString filePath = StProcess::getAbsolutePath(param);
            stOpenFileArgs.add(StArgument(ARGUMENT_FILE + filesCount++, filePath));
            if(!myOpenFileInfo.hasPath()) {
                // first file determines MIME type (needed to autoselect Drawer plugin)
                myOpenFileInfo.setPath(filePath);
            }
        } else if(param == ARGUMENT_FILES_SECTION) {
            isFilesSection = true;
        } else if(param.isStartsWith(ARGUMENT_ANY)) {
            // argument
            StArgument stArg; stArg.parseString(param.subString(2, param.getLength())); // cut suffix --

            if(stArg.getKey().isEqualsIgnoreCase(ARGUMENT_HELP)) {
                myShowHelpString = getAboutString(stArguments[0]);
            } else if(stArg.getKey().isEqualsIgnoreCase(ARGUMENT_PLUGIN_OUT)) {
                myParamRenderer = stArg.getValue();
                ST_DEBUG_LOG("sView argument, Renderer plugin= '" + myParamRenderer + '\'');
            } else if(stArg.getKey().isEqualsIgnoreCase(ARGUMENT_PLUGIN_OUT_DEVICE)) {
                ST_DEBUG_LOG("sView argument, StereoDevice= '" + stArg.getValue() + '\'');
                if(stUtfTools::isInteger(stArg.getValue())) {
                    myParamDeviceInt = std::atoi(stArg.getValue().toCString());
                } else {
                    myParamDeviceString = stArg.getValue();
                }
            } else if(stArg.getKey().isEqualsIgnoreCase(ARGUMENT_PLUGIN_IN)) {
                // predefined plugins links
                const StString IMAGE_VIEWER("image");
                const StString VIDEO_PLAYER("video");
                if(stArg.getValue().isEqualsIgnoreCase(IMAGE_VIEWER)) {
                    stArg.changeValue() = myStCorePath + StCore::getDrawersDir() + SYS_FS_SPLITTER + "StImageViewer" + ST_DLIB_SUFFIX;
                } else if(stArg.getValue().isEqualsIgnoreCase(VIDEO_PLAYER)) {
                    stArg.changeValue() = myStCorePath + StCore::getDrawersDir() + SYS_FS_SPLITTER + "StMoviePlayer" + ST_DLIB_SUFFIX;
                } else {
                    StString ext = StFileNode::getExtension(stArg.getValue());
                    if(ext != StString(ST_DLIB_EXTENSION)) {
                        stArg.changeValue() += StString(ST_DLIB_SUFFIX);
                    }
                    if(StFileNode::isRelativePath(stArg.getValue())) {
                        stArg.changeValue() = myStCorePath + StCore::getDrawersDir() + SYS_FS_SPLITTER + stArg.getValue();
                    }
                }
                ST_DEBUG_LOG("sView argument, Drawer plugin= '" + stArg.getValue() + '\'');
                myDrawerInfo.setMIME(StDrawerInfo::DRAWER_MIME());
                myDrawerInfo.setPath(stArg.getValue());
            } else if(stArg.getKey().isEqualsIgnoreCase(ARGUMENT_LEFT_VIEW)) {
                // left view
                stArg.setValue(StProcess::getAbsolutePath(stArg.getValue()));
                stOpenFileArgs.add(stArg);
                myOpenFileInfo.setPath(stArg.getValue()); // left file always determines MIME type
            } else if(stArg.getKey().isEqualsIgnoreCase(ARGUMENT_RIGHT_VIEW)) {
                // right view
                stArg.setValue(StProcess::getAbsolutePath(stArg.getValue()));
                stOpenFileArgs.add(stArg);
                if(!myOpenFileInfo.hasPath()) {
                    myOpenFileInfo.setPath(stArg.getValue());
                }
            } else {
                // pass argument unchanged to the Drawer
                stDrawerArgs.add(stArg);
            }
        } else {
            // file name
            StString filePath = StProcess::getAbsolutePath(param);
            stOpenFileArgs.add(StArgument(ARGUMENT_FILE + filesCount++, filePath));
            if(!myOpenFileInfo.hasPath()) {
                // first file determines MIME type (needed to autoselect Drawer plugin)
                myOpenFileInfo.setPath(filePath);
            }
        }
    }

    if(!myDrawerInfo.isEmpty()) {
        myDrawerInfo.setArgumentsMap(stDrawerArgs);
    } else {
        // if no Drawer selected - pass all arguments to open file structure
        for(size_t anArgId = 0; anArgId < stDrawerArgs.size(); ++anArgId) {
            stOpenFileArgs.add(stDrawerArgs.getFromIndex(anArgId));
        }
    }
    myOpenFileInfo.setArgumentsMap(stOpenFileArgs);

    // there are NO default sView plugin to load now!
    if(myDrawerInfo.isEmpty() && myOpenFileInfo.isEmpty()) {
        myShowHelpString = getAboutString(stArguments.size() > 0 ? stArguments[0] : StString("sView"));
    }
}

// Exported class-methods wpappers.
ST_EXPORT StApplicationInterface* StApplication_new() {
    return new StApplicationImpl();
}

ST_EXPORT void StApplication_del(StApplicationInterface* theInst) {
    delete (StApplicationImpl* )theInst;
}

ST_EXPORT stBool_t StApplication_isOpened(StApplicationInterface* theInst) {
    return ((StApplicationImpl* )theInst)->isOpened();
}

ST_EXPORT stBool_t StApplication_isFullscreen(StApplicationInterface* theInst) {
    return ((StApplicationImpl* )theInst)->isFullscreen();
}

ST_EXPORT stBool_t StApplication_create(StApplicationInterface* theInst, const StNativeWin_t theNativeWinParent) {
    return ((StApplicationImpl* )theInst)->create(theNativeWinParent);
}

ST_EXPORT stBool_t StApplication_open(StApplicationInterface* theInst, const StOpenInfo_t* stOpenInfo) {
    return ((StApplicationImpl* )theInst)->open(StOpenInfo(stOpenInfo));
}

ST_EXPORT void StApplication_callback(StApplicationInterface* theInst, StMessage_t* theMessages) {
    ((StApplicationImpl* )theInst)->callback(theMessages);
}
