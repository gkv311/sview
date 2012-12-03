#ifndef __NSPluginBase_h_
#define __NSPluginBase_h_

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
    virtual bool init(NPWindow* aWindow) = 0;
    virtual bool isInitialized() = 0;

    // implement all or part of those methods in the derived
    // class as needed
    virtual NPError setWindow(NPWindow* /*npWindow*/)                         { return NPERR_NO_ERROR; }
    virtual NPError streamNew(NPMIMEType /*type*/, NPStream* /*stream*/,
                              NPBool /*seekable*/, uint16_t* /*stype*/)       { return NPERR_NO_ERROR; }
    virtual NPError streamDel(NPStream* /*stream*/, NPError /*reason*/)       { return NPERR_NO_ERROR; }
    virtual void    streamAsFile(NPStream* /*stream*/, const char* /*fname*/) { return; }
    virtual int32_t streamWriteReady(NPStream* /*stream*/)                    { return 0x0fffffff; }
    virtual int32_t streamWrite(NPStream* /*stream*/, int32_t /*offset*/,
                                int32_t len, void* /*buffer*/)                { return len; }
    virtual void    print(NPPrint* /*printInfo*/)                             { return; }
    virtual uint16_t handleEvent(void* /*event*/)                             { return 0; }
    virtual void    urlNotify(const char* /*url*/, NPReason /*reason*/,
                              void* /*notifyData*/)                           { return; }
    virtual NPError getValue(NPPVariable /*variable*/, void* /*value*/)       { return NPERR_NO_ERROR; }
    virtual NPError setValue(NPNVariable /*variable*/, void* /*value*/)       { return NPERR_NO_ERROR; }

};

// functions that should be implemented for each specific plugin

// creation and destruction of the object of the derived class
NSPluginBase* NS_NewPluginInstance(NSPluginCreateData* aCreateDataStruct);
void NS_DestroyPluginInstance(NSPluginBase* aPlugin);

// global plugin initialization and shutdown
NPError NS_PluginInitialize();
void NS_PluginShutdown();

// global to get plugins name & description
NPError NS_PluginGetValue(NPPVariable aVariable, void* aValue);

#endif //__NSPluginBase_h_
