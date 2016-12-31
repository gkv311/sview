/**
 * This source is a part of sView program.
 *
 * Copyright © Kirill Gavrilov, 2011-2016
 */

#ifndef __StCADLoader_h_
#define __StCADLoader_h_

#if defined(_WIN32)
    #include <windows.h>
#endif

#include <AIS_InteractiveObject.hxx>
#include <NCollection_Sequence.hxx>

#include <StStrings/StString.h>
#include <StFile/StMIMEList.h>
#include <StGL/StPlayList.h>
#include <StGLMesh/StGLMesh.h>
#include <StSlots/StSignal.h>
#include <StThreads/StThread.h>

#include "StAssetDocument.h"

class StLangMap;
class StThread;

class StCADLoader {

        public:

    static const StString ST_CAD_MIME_STRING;
    static const StMIMEList ST_CAD_MIME_LIST;
    static const StArrayList<StString> ST_CAD_EXTENSIONS_LIST;

    ST_LOCAL StCADLoader(const StHandle<StLangMap>&  theLangMap,
                         const StHandle<StPlayList>& thePlayList,
                         const bool                  theToStartThread = true);
    ST_LOCAL virtual ~StCADLoader();

    ST_LOCAL void mainLoop();

    ST_LOCAL void doLoadNext() {
        myEvLoadNext.set();
    }

    ST_LOCAL virtual bool getNextDoc(NCollection_Sequence<Handle(AIS_InteractiveObject)>& thePrsList,
                                     Handle(StAssetDocument)& theDoc);

        public:  //!< Signals

    struct {
        /**
         * Emit callback Slot on model load error.
         * @param theUserData error description
         */
        StSignal<void (const StCString& )> onError;
    } signals;

        protected:

    ST_LOCAL virtual bool loadModel(const StHandle<StFileNode>& theSource);

    /**
     * Just redirect callback slot.
     */
    ST_LOCAL void doOnErrorRedirect(const StCString& theMsgText) {
        signals.onError(theMsgText);
    }

    static SV_THREAD_FUNCTION threadFunction(void* theLoader) {
        StCADLoader* aCADLoader = (StCADLoader* )theLoader;
        aCADLoader->mainLoop();
        return SV_THREAD_RETURN 0;
    }

        protected:

    StHandle<StThread>   myThread;
    StHandle<StLangMap>  myLangMap;
    StHandle<StPlayList> myPlayList;
    StCondition          myEvLoadNext;
    Handle(StAssetDocument) myDoc;
    NCollection_Sequence<Handle(AIS_InteractiveObject)> myPrsList;
    Graphic3d_MaterialAspect myDefaultMat;
    StMutex              myResultLock;
    volatile bool        myIsLoaded;
    volatile bool        myToQuit;

};

#endif //__StCADLoader_h_
