/**
 * Implementation of plugin entry points (NPP_*)
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

#include "NSPluginBase.h"

/**
 * Here the plugin creates a plugin instance object which
 * will be associated with this newly created NPP instance and
 * will do all the necessary job
 */
NPError NPP_New(NPMIMEType pluginType, NPP instance, uint16_t mode, int16_t argc, char* argn[], char* argv[], NPSavedData* saved) {
    if(instance == NULL) {
        return NPERR_INVALID_INSTANCE_ERROR;
    }

    // create a new plugin instance object
    // initialization will be done when the associated window is ready
    NSPluginCreateData ds;

    ds.instance = instance;
    ds.type     = pluginType;
    ds.mode     = mode;
    ds.argc     = argc;
    ds.argn     = argn;
    ds.argv     = argv;
    ds.saved    = saved;

    NSPluginBase* plugin = NS_NewPluginInstance(&ds);
    if(plugin == NULL) {
        return NPERR_OUT_OF_MEMORY_ERROR;
    }

    // associate the plugin instance object with NPP instance
    instance->pdata = (void* )plugin;
    return NPERR_NO_ERROR;
}

// here is the place to clean up and destroy the NSPluginInstance object
NPError NPP_Destroy(NPP instance, NPSavedData** /*save*/) {
    if(instance == NULL) {
        return NPERR_INVALID_INSTANCE_ERROR;
    }

    NSPluginBase* plugin = (NSPluginBase* )instance->pdata;
    if(plugin != NULL) {
        NS_DestroyPluginInstance(plugin);
    }
    return NPERR_NO_ERROR;
}

#include <StStrings/StLogger.h>

/**
 * During this call we know when the plugin window is ready or
 * is about to be destroyed so we can do some gui specific
 * initialization and shutdown
 */
NPError NPP_SetWindow(NPP instance, NPWindow* pNPWindow) {

    if(instance == NULL) {
        return NPERR_INVALID_INSTANCE_ERROR;
    }

    if(pNPWindow == NULL) {
        return NPERR_GENERIC_ERROR;
    }

    NSPluginBase* plugin = (NSPluginBase* )instance->pdata;

    if(plugin == NULL) {
        return NPERR_GENERIC_ERROR;
    }

    // window just created
    if(!plugin->isInitialized() && pNPWindow->window != NULL) {
        if(!plugin->init(pNPWindow)) {
            NS_DestroyPluginInstance(plugin);
            return NPERR_MODULE_LOAD_FAILED_ERROR;
        }
    }

    // window goes away
    if(pNPWindow->window == NULL && plugin->isInitialized()) {
        return plugin->setWindow(pNPWindow);
    }

    // window resized?
    if(plugin->isInitialized() && pNPWindow->window != NULL) {
        return plugin->setWindow(pNPWindow);
    }

    // this should not happen, nothing to do
    if(pNPWindow->window == NULL && !plugin->isInitialized()) {
        return plugin->setWindow(pNPWindow);
    }

    return NPERR_NO_ERROR;
}

NPError NPP_NewStream(NPP instance, NPMIMEType type, NPStream* stream, NPBool seekable, uint16_t* stype) {
    if(instance == NULL) {
        return NPERR_INVALID_INSTANCE_ERROR;
    }

    NSPluginBase* plugin = (NSPluginBase* )instance->pdata;
    if(plugin == NULL) {
        return NPERR_GENERIC_ERROR;
    }

    return plugin->streamNew(type, stream, seekable, stype);
}

int32_t NPP_WriteReady(NPP instance, NPStream* stream) {
    if(instance == NULL) {
        return 0x0fffffff;
    }

    NSPluginBase* plugin = (NSPluginBase* )instance->pdata;
    if(plugin == NULL) {
        return 0x0fffffff;
    }

    return plugin->streamWriteReady(stream);
}

int32_t NPP_Write(NPP instance, NPStream* stream, int32_t offset, int32_t len, void* buffer) {
    if(instance == NULL) {
        return len;
    }

    NSPluginBase* plugin = (NSPluginBase* )instance->pdata;
    if(plugin == NULL) {
        return len;
    }

    return plugin->streamWrite(stream, offset, len, buffer);
}

NPError NPP_DestroyStream(NPP instance, NPStream* stream, NPError reason) {
    if(instance == NULL) {
        return NPERR_INVALID_INSTANCE_ERROR;
    }

    NSPluginBase* plugin = (NSPluginBase* )instance->pdata;
    if(plugin == NULL) {
        return NPERR_GENERIC_ERROR;
    }

    return plugin->streamDel(stream, reason);
}

void NPP_StreamAsFile(NPP instance, NPStream* stream, const char* fname) {
    if(instance == NULL) {
        return;
    }

    NSPluginBase* plugin = (NSPluginBase* )instance->pdata;
    if(plugin == NULL) {
        return;
    }

    plugin->streamAsFile(stream, fname);
}

void NPP_Print(NPP instance, NPPrint* printInfo) {
    if(instance == NULL) {
        return;
    }

    NSPluginBase* plugin = (NSPluginBase* )instance->pdata;
    if(plugin == NULL) {
        return;
    }

    plugin->print(printInfo);
}

void NPP_URLNotify(NPP instance, const char* url, NPReason reason, void* notifyData) {
    if(instance == NULL) {
        return;
    }

    NSPluginBase* plugin = (NSPluginBase* )instance->pdata;
    if(plugin == NULL) {
        return;
    }

    plugin->urlNotify(url, reason, notifyData);
}

NPError NPP_GetValue(NPP instance, NPPVariable variable, void* value) {
    if(instance == NULL) {
        return NPERR_INVALID_INSTANCE_ERROR;
    }

    NSPluginBase* plugin = (NSPluginBase* )instance->pdata;
    if(plugin == NULL) {
        return NPERR_GENERIC_ERROR;
    }

    return plugin->getValue(variable, value);
}

NPError NPP_SetValue(NPP instance, NPNVariable variable, void* value) {
    if(instance == NULL) {
        return NPERR_INVALID_INSTANCE_ERROR;
    }

    NSPluginBase* plugin = (NSPluginBase* )instance->pdata;
    if(plugin == NULL) {
        return NPERR_GENERIC_ERROR;
    }

    return plugin->setValue(variable, value);
}

int16_t NPP_HandleEvent(NPP instance, void* event) {
    if(instance == NULL) {
        return 0;
    }

    NSPluginBase* plugin = (NSPluginBase* )instance->pdata;
    if(plugin == NULL) {
        return 0;
    }

    return plugin->handleEvent(event);
}
