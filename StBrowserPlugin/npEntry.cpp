/**
 * Main plugin entry point implementation - exports from the plugin library
 * Copyright © 2009 Kirill Gavrilov <kirill@sview.ru>
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

#include "npplat.h"
#include "NSPluginBase.h"

NPNetscapeFuncs NPNFuncs;

extern "C" NPError OSCALL NP_Shutdown() {
    NS_PluginShutdown();
    return NPERR_NO_ERROR;
}

static NPError fillPluginFunctionTable(NPPluginFuncs* aNPPFuncs) {
    if(aNPPFuncs == NULL) {
        return NPERR_INVALID_FUNCTABLE_ERROR;
    }

    // Set up the plugin function table that Netscape will use to call us.
    aNPPFuncs->version       = (NP_VERSION_MAJOR << 8) | NP_VERSION_MINOR;
    aNPPFuncs->newp          = NPP_New;
    aNPPFuncs->destroy       = NPP_Destroy;
    aNPPFuncs->setwindow     = NPP_SetWindow;
    aNPPFuncs->newstream     = NPP_NewStream;
    aNPPFuncs->destroystream = NPP_DestroyStream;
    aNPPFuncs->asfile        = NPP_StreamAsFile;
    aNPPFuncs->writeready    = NPP_WriteReady;
    aNPPFuncs->write         = NPP_Write;
    aNPPFuncs->print         = NPP_Print;
    aNPPFuncs->event         = NPP_HandleEvent;
    aNPPFuncs->urlnotify     = NPP_URLNotify;
    aNPPFuncs->getvalue      = NPP_GetValue;
    aNPPFuncs->setvalue      = NPP_SetValue;
    return NPERR_NO_ERROR;
}

#if defined(_WIN32) || defined(__linux__) || defined(__linux)
static NPError fillNetscapeFunctionTable(NPNetscapeFuncs* aNPNFuncs) {
    if(aNPNFuncs == NULL) {
        return NPERR_INVALID_FUNCTABLE_ERROR;
    }

    if(HIBYTE(aNPNFuncs->version) > NP_VERSION_MAJOR) {
        return NPERR_INCOMPATIBLE_VERSION_ERROR;
    }

    if(aNPNFuncs->size < sizeof(NPNetscapeFuncs)) {
        return NPERR_INVALID_FUNCTABLE_ERROR;
    }

    NPNFuncs.size             = aNPNFuncs->size;
    NPNFuncs.version          = aNPNFuncs->version;
    NPNFuncs.geturlnotify     = aNPNFuncs->geturlnotify;
    NPNFuncs.geturl           = aNPNFuncs->geturl;
    NPNFuncs.posturlnotify    = aNPNFuncs->posturlnotify;
    NPNFuncs.posturl          = aNPNFuncs->posturl;
    NPNFuncs.requestread      = aNPNFuncs->requestread;
    NPNFuncs.newstream        = aNPNFuncs->newstream;
    NPNFuncs.write            = aNPNFuncs->write;
    NPNFuncs.destroystream    = aNPNFuncs->destroystream;
    NPNFuncs.status           = aNPNFuncs->status;
    NPNFuncs.uagent           = aNPNFuncs->uagent;
    NPNFuncs.memalloc         = aNPNFuncs->memalloc;
    NPNFuncs.memfree          = aNPNFuncs->memfree;
    NPNFuncs.memflush         = aNPNFuncs->memflush;
    NPNFuncs.reloadplugins    = aNPNFuncs->reloadplugins;
    NPNFuncs.getvalue         = aNPNFuncs->getvalue;
    NPNFuncs.setvalue         = aNPNFuncs->setvalue;
    NPNFuncs.invalidaterect   = aNPNFuncs->invalidaterect;
    NPNFuncs.invalidateregion = aNPNFuncs->invalidateregion;
    NPNFuncs.forceredraw      = aNPNFuncs->forceredraw;
    NPNFuncs.pluginthreadasynccall = aNPNFuncs->pluginthreadasynccall;

    return NPERR_NO_ERROR;
}
#endif

#if defined(_WIN32)
    /**
     * Some exports are different on different platforms
     */
    extern "C" NPError OSCALL NP_Initialize(NPNetscapeFuncs* aNPNFuncs) {
        NPError rv = fillNetscapeFunctionTable(aNPNFuncs);
        if(rv != NPERR_NO_ERROR) {
            return rv;
        }
        return NS_PluginInitialize();
    }
#elif(defined(__linux__) || defined(__linux))
    NPError NP_Initialize(NPNetscapeFuncs* aNPNFuncs, NPPluginFuncs* aNPPFuncs) {
        NPError rv = fillNetscapeFunctionTable(aNPNFuncs);
        if(rv != NPERR_NO_ERROR) {
            return rv;
        }

        rv = fillPluginFunctionTable(aNPPFuncs);
        if(rv != NPERR_NO_ERROR) {
            return rv;
        }

        return NS_PluginInitialize();
    }
#endif

extern "C" NPError OSCALL NP_GetEntryPoints(NPPluginFuncs* aNPPFuncs) {
    return fillPluginFunctionTable(aNPPFuncs);
}

extern "C" char* OSCALL NP_GetMIMEDescription(void) {
    return NPP_GetMIMEDescription();
}

NPError OSCALL NP_GetValue(void* , NPPVariable aVariable, void* aValue) {
    return NS_PluginGetValue(aVariable, aValue);
}

#ifdef XP_MAC
    short gResFile; // Refnum of the plugin's resource file

    NPError Private_Initialize(void) {
        NPError rv = NS_PluginInitialize();
        return rv;
    }

    void Private_Shutdown(void) {
        NS_PluginShutdown();
        __destroy_global_chain();
    }

    NPError main(NPNetscapeFuncs* aNPNFuncs, NPPluginFuncs* aNPPFuncs, NPP_ShutdownUPP* aUnloadUpp) {
        NPError rv = NPERR_NO_ERROR;

        if(!aUnloadUpp) {
            rv = NPERR_INVALID_FUNCTABLE_ERROR;
        }

        if(rv == NPERR_NO_ERROR) {
            rv = fillNetscapeFunctionTable(aNPNFuncs);
        }

        if(rv == NPERR_NO_ERROR) {
            // defer static constructors until the global functions are initialized.
            __InitCode__();
            rv = fillPluginFunctionTable(aNPPFuncs);
        }

        *aUnloadUpp = NewNPP_ShutdownProc(Private_Shutdown);
        gResFile = CurResFile();
        rv = Private_Initialize();

        return rv;
    }
#endif // XP_MAC
