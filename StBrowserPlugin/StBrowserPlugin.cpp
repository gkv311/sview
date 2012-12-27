/**
 * Copyright © 2009-2012 Kirill Gavrilov <kirill@sview.ru>
 *
 * StBrowserPlugin NPAPI plugin is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * StBrowserPlugin NPAPI plugin is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this library.
 * If not, see <http://www.gnu.org/licenses/>.
 */

#include "StBrowserPlugin.h"
#include "StBrPluginInfo.h"

#include <StCore/StApplication.h>
#include <StFile/StFileNode.h>
#include <StFile/StFolder.h>
#include <StGLStereo/StFormatEnum.h>

#include <StTemplates/StHandle.h>
#include <StTemplates/StArrayList.h>

extern NPNetscapeFuncs NPNFuncs;

template<>
inline void StArray< StHandle<StRendererPlugin> >::sort() {}

namespace {
    static bool HAS_LOGGER_ID = StLogger::IdentifyModule("StBrowserPlugin");

    static bool isStCoreInitSuccess = false; // we use global init flag to go around stupid Mozilla plugin load behaviour
};

/**
 * Function called only on Linux!
 * On Win32 you should set MIME types in resource file.
 */
char* NPP_GetMIMEDescription() {
    return (char* )ST_NPAPI_MIME;
}

/**
 * Function called only on Linux!
 */
NPError NS_PluginGetValue(NPPVariable aVariable, void* aValue) {
    NPError err = NPERR_NO_ERROR;
    switch(aVariable) {
        case NPPVpluginNameString:
            *((char** )aValue) = (char* )ST_BROWSER_PLUGIN_NAME;
            break;
        case NPPVpluginDescriptionString:
            *((char** )aValue) = (char* )ST_BROWSER_PLUGIN_DESC;
            break;
        default:
            err = NPERR_INVALID_PARAM;
            break;
    }
    return err;
}

/**
 * General plugin initialization.
 * Called ONCE for all plugin's instances!
 */
NPError NS_PluginInitialize() {
    if(isStCoreInitSuccess) {
        return NPERR_NO_ERROR;
    }

    // Firstly INIT core library!
    if(StCore::INIT() != STERROR_LIBNOERROR) {
        // TODO (Kirill Gavrilov#9) we got this error message 3 times (!) in Mozilla Firefox 3.5.4
        // and 1 time (as expected) in Opera 10
        stError("StCore library not available!\nMake sure you install sView correctly.");
        // don't seduce yourself! Mozilla will continue to load and instaniate your plugin :|
        isStCoreInitSuccess = false;
        return NPERR_MODULE_LOAD_FAILED_ERROR;
    }

    isStCoreInitSuccess = true;
    return NPERR_NO_ERROR;
}

/**
 * General plugin shutdown.
 * Called when the last plugin's instance were closed.
 */
void NS_PluginShutdown() {
    StCore::FREE();
}

/**
 * Construction of our plugin instance object.
 */
NSPluginBase* NS_NewPluginInstance(NSPluginCreateData* aCreateDataStruct) {
    if(!isStCoreInitSuccess || aCreateDataStruct == NULL) {
        return NULL;
    }
    StBrowserPlugin* plugin = new StBrowserPlugin(aCreateDataStruct);
    return plugin;
}

/**
 * Destruction of our plugin instance object.
 */
void NS_DestroyPluginInstance(NSPluginBase* aPlugin) {
    if(aPlugin != NULL) {
        delete (StBrowserPlugin* )aPlugin;
    }
}

/**
 * Here plugin could provide more information to the browser about himself.
 */
#if(defined(__linux__) || defined(__linux))
NPError StBrowserPlugin::getValue(NPPVariable variable, void* value) {
    NPError err = NPERR_NO_ERROR;
    switch(variable) {
        /// TODO (Kirill Gavrilov#9) set XEmbed support as true (needed on modern browsers without Xt-interface)
        case NPPVpluginNeedsXEmbed:
            //*((PRBool* )value) = PR_TRUE;
            *((int* )value) = 1;
            break;
        default:
            err = NPERR_GENERIC_ERROR;
    }
    return err;
}
#endif

StBrowserPlugin::StBrowserPlugin(NSPluginCreateData* theCreateDataStruct)
: NSPluginBase(),
  nppInstance(theCreateDataStruct->instance),
  myParentWin((StNativeWin_t )NULL),
  myStApp(),
  myOpenInfo(),
  myToLoadFull(false),
  myToQuit(false) {
    StArgumentsMap aDrawerArgs;
    for(int aParamId = 0; aParamId < theCreateDataStruct->argc; ++aParamId) {
        StString aParamName  = StString(theCreateDataStruct->argn[aParamId]);
        StString aParamValue = StString(theCreateDataStruct->argv[aParamId]);

        StArgument stArg(aParamName, aParamValue);
        aDrawerArgs.add(stArg);

        if(aParamName.isEqualsIgnoreCase("data-prv-url")) {
            myPreviewUrl = aParamValue;
            myPreviewUrlUtf8.fromUrl(aParamValue);
        }
    }
    const StString ST_ASTERIX = '*';
    StMIME stMIME(StString(theCreateDataStruct->type), ST_ASTERIX, ST_ASTERIX);
    myOpenInfo.setMIME(stMIME);

    const StString ST_SETTING_SRCFORMAT    = "srcFormat";
    const StString ST_SETTING_COMPRESS     = "toCompress";
    const StString ST_SETTING_ESCAPENOQUIT = "escNoQuit";
    const StMIME ST_MIME_X_JPS("image/x-jps", ST_ASTERIX, ST_ASTERIX);
    const StMIME ST_MIME_JPS  ("image/jps",   ST_ASTERIX, ST_ASTERIX);
    const StMIME ST_MIME_X_PNS("image/x-pns", ST_ASTERIX, ST_ASTERIX);
    const StMIME ST_MIME_PNS  ("image/pns",   ST_ASTERIX, ST_ASTERIX);
    StArgument anArgSrcFormat = aDrawerArgs[ST_SETTING_SRCFORMAT];
    if(!anArgSrcFormat.isValid()) {
        anArgSrcFormat.setKey(ST_SETTING_SRCFORMAT);
        if(stMIME == ST_MIME_X_JPS || stMIME == ST_MIME_JPS || stMIME == ST_MIME_X_PNS || stMIME == ST_MIME_PNS) {
            anArgSrcFormat.setValue(st::formatToString(ST_V_SRC_SIDE_BY_SIDE));
            aDrawerArgs.add(anArgSrcFormat);
        }
    }
    aDrawerArgs.add(StArgument(ST_SETTING_COMPRESS,     true)); // optimize memory usage
    aDrawerArgs.add(StArgument(ST_SETTING_ESCAPENOQUIT, true)); // do not close plugin instance by Escape key
    myOpenInfo.setArgumentsMap(aDrawerArgs);
}

StBrowserPlugin::~StBrowserPlugin() {
    if(!myThread.isNull()) {
        myToQuit = true;
        myThread->wait();
        myThread.nullify();
    }

    // remove temporary files
    for(size_t anIter = 0; anIter < myTmpFiles.size(); ++anIter) {
        StFileNode::removeFile(myTmpFiles.getValue(anIter));
    }
}

void StBrowserPlugin::stWindowLoop() {

    myStApp = new StApplication();
    if(!myStApp->create(myParentWin)) {
        myStApp.nullify();
        return;
    }

    // Load image viewer plugin
    StString imageViewerPath(StProcess::getStCoreFolder() + StCore::getDrawersDir() + SYS_FS_SPLITTER + "StImageViewer" + ST_DLIB_SUFFIX);
    StOpenInfo aCreateInfo;
    aCreateInfo.setMIME(StDrawerInfo::DRAWER_MIME().toString());
    aCreateInfo.setPath(imageViewerPath);
    if(!myStApp->open(aCreateInfo)) {
        myStApp.nullify();
        return;
    }

    bool isFileOpened = false;
    bool isFullscreen = false;
    bool isFullLoaded = false;
    for(;;) {
        if(!myStApp->isOpened()) {
            myStApp.nullify();
            return;
        }

        if(myToQuit) {
            StOpenInfo anOpenInfoClose;
            anOpenInfoClose.setMIME(StDrawerInfo::CLOSE_MIME().toString());
            myStApp->open(anOpenInfoClose);
        } else if(!isFileOpened) {
            // load the image
            StHandle<StString> aPrvPath = myPreviewPath;
            if(myPreviewUrl.isEmpty()) {
                StHandle<StString> aFullPath = myFullPath;
                if(!aFullPath.isNull()) {
                    myOpenInfo.setPath(*aFullPath);
                    myStApp->open(myOpenInfo);
                    isFileOpened = true;
                }
            } else if(!aPrvPath.isNull()) {
                myOpenInfo.setPath(*aPrvPath);
                myStApp->open(myOpenInfo);
                isFileOpened = true;
            }
        }

        myStApp->callback();

        if(myStApp->isFullscreen()) {
            StHandle<StString> aFullPath = myFullPath;
            if(!isFullscreen && !aFullPath.isNull()) {
                myOpenInfo.setPath(*aFullPath);
                myStApp->open(myOpenInfo);
                isFullscreen = true;
            } else if(!isFullLoaded && NPNFuncs.pluginthreadasynccall != NULL) {
                NPNFuncs.pluginthreadasynccall(nppInstance, StBrowserPlugin::doLoadFullSize, this);
                isFullLoaded = true;
            }
        } else if(isFullscreen) {
            StHandle<StString> aPrvPath = myPreviewPath;
            if(!aPrvPath.isNull()) {
                myOpenInfo.setPath(*aPrvPath);
                myStApp->open(myOpenInfo);
                isFullscreen = false;
            }
        }
    }
}

static SV_THREAD_FUNCTION stThreadFunction(void* theParam) {
    StBrowserPlugin* aBrPlugin = (StBrowserPlugin* )theParam;
    aBrPlugin->stWindowLoop();
    return SV_THREAD_RETURN 0;
}

bool StBrowserPlugin::init(NPWindow* npWindow) {
    if(npWindow == NULL || npWindow->window == NULL) {
        return false;
    }

    myParentWin = (StNativeWin_t )npWindow->window;
    myThread = new StThread(stThreadFunction, this); // starts out plugin main loop in another thread

    if(!myPreviewUrl.isEmpty() && NPNFuncs.geturl != NULL) {
        NPNFuncs.geturl(nppInstance, myPreviewUrl.toCString(), NULL);
    }

    return true;
}

bool StBrowserPlugin::isInitialized() {
    return !myThread.isNull();
}

const char* StBrowserPlugin::getVersion() {
    return NPN_UserAgent(nppInstance);
}

void StBrowserPlugin::doLoadFullSize(void* thePlugin) {
    StBrowserPlugin* aPlugin = (StBrowserPlugin* )thePlugin;
    if(aPlugin == NULL) {
        return;
    }

    aPlugin->doLoadFullSize();
}

void StBrowserPlugin::doLoadFullSize() {
    if(!myFullUrl.isEmpty() && NPNFuncs.geturl != NULL) {
        myToLoadFull = true;
        NPNFuncs.geturl(nppInstance, myFullUrl.toCString(), NULL);
    }
}

NPError StBrowserPlugin::streamNew(NPMIMEType ,
                                   NPStream* theStream,
                                   NPBool ,
                                   uint16_t* theStreamType) {
    *theStreamType = NP_ASFILEONLY;
    // here we got MIME from server header (not object MIME-type!)
    // because jps/pns/mpo are actually jpeg/png files - we ignore returned MIME here
    //myOpenInfo.setMIME(StString(mimeString) + ST_TEXT(":*:*"));

    if(myToLoadFull) {
        // load full-size image only when switched to fullscreen
        return NPERR_NO_ERROR;
    }

    // notice that some browsers (Chromium) returns NOT the same string as requested by NPNFuncs.geturl()!
    // instead here we got URL with decoded Unicode characters
    const StString anUrl((theStream->url != NULL) ? theStream->url : "");
    if(myPreviewUrl.isEmpty()
    || anUrl.isEndsWith(myPreviewUrl)
    || anUrl.isEndsWith(myPreviewUrlUtf8)) {
        return NPERR_NO_ERROR;
    }
    myFullUrl = anUrl;

    // block wrong streams
    return NPERR_INVALID_URL;
}

void StBrowserPlugin::streamAsFile(NPStream*   theStream,
                                   const char* theFileName) {
    if(theFileName == NULL) {
        ///ST_DEBUG_LOG("streamAsFile ERROR");
        return;
    }

    const StString anUrl((theStream->url != NULL) ? theStream->url : "");
    const bool isPreview = !myPreviewUrl.isEmpty()
                        && (anUrl.isEndsWith(myPreviewUrl) || anUrl.isEndsWith(myPreviewUrlUtf8));
    StString aFileName = StString(theFileName);
    StString aFolder, aDummy;
    StFileNode::getFolderAndFile(aFileName, aFolder, aDummy);
    if(aFileName.isStartsWith(StProcess::getTempFolder())) {
        // Some browsers (Safari) returns file copies in temporary folder
        // and imidiatly remove it after function execution.
        // sView load image async, so we need to copy file until it will be read.
        StString aFileNameNew = aFileName + ".sView.tmp";
        if(StFileNode::moveFile(aFileName, aFileNameNew)) {
            aFileName = aFileNameNew;
            myTmpFiles.add(aFileName);
        }
    }

    if(isPreview) {
        myPreviewPath = new StString(aFileName);
    } else {
        myFullPath    = new StString(aFileName);
    }
}
