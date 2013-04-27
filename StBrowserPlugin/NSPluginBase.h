#ifndef __NSPluginBase_h_
#define __NSPluginBase_h_

#include <stTypes.h>
#include "npplat.h"

struct NSPluginCreateData {
    NPP instance;
    NPMIMEType type;
    uint16_t mode;
    int16_t argc;
    char** argn;
    char** argv;
    NPSavedData* saved;
};

class NSPluginBase {

        public:

    // these three methods must be implemented in the derived
    // class platform specific way
    ST_LOCAL virtual bool init(NPWindow* aWindow) = 0;
    ST_LOCAL virtual bool isInitialized() = 0;

    // implement all or part of those methods in the derived
    // class as needed
    ST_LOCAL virtual NPError setWindow(NPWindow* /*npWindow*/)                         { return NPERR_NO_ERROR; }
    ST_LOCAL virtual NPError streamNew(NPMIMEType /*type*/, NPStream* /*stream*/,
                                       NPBool /*seekable*/, uint16_t* /*stype*/)       { return NPERR_NO_ERROR; }
    ST_LOCAL virtual NPError streamDel(NPStream* /*stream*/, NPError /*reason*/)       { return NPERR_NO_ERROR; }
    ST_LOCAL virtual void    streamAsFile(NPStream* /*stream*/, const char* /*fname*/) { return; }
    ST_LOCAL virtual int32_t streamWriteReady(NPStream* /*stream*/)                    { return 0x0fffffff; }
    ST_LOCAL virtual int32_t streamWrite(NPStream* /*stream*/, int32_t /*offset*/,
                                         int32_t len, void* /*buffer*/)                { return len; }
    ST_LOCAL virtual void    print(NPPrint* /*printInfo*/)                             { return; }
    ST_LOCAL virtual uint16_t handleEvent(void* /*event*/)                             { return 0; }
    ST_LOCAL virtual void    urlNotify(const char* /*url*/, NPReason /*reason*/,
                                       void* /*notifyData*/)                           { return; }
    ST_LOCAL virtual NPError getValue(NPPVariable /*variable*/, void* /*value*/)       { return NPERR_NO_ERROR; }
    ST_LOCAL virtual NPError setValue(NPNVariable /*variable*/, void* /*value*/)       { return NPERR_NO_ERROR; }

};

// functions that should be implemented for each specific plugin

// creation and destruction of the object of the derived class
ST_LOCAL NSPluginBase* NS_NewPluginInstance(NSPluginCreateData* aCreateDataStruct);
ST_LOCAL void NS_DestroyPluginInstance(NSPluginBase* aPlugin);

// global plugin initialization and shutdown
ST_LOCAL NPError NS_PluginInitialize();
ST_LOCAL void NS_PluginShutdown();

// global to get plugins name & description
ST_LOCAL NPError NS_PluginGetValue(NPPVariable aVariable, void* aValue);

#endif //__NSPluginBase_h_
