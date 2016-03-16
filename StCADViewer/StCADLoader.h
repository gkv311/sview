/**
 * This source is a part of sView program.
 *
 * Copyright © Kirill Gavrilov, 2011-2016
 */

#ifndef __StCADLoader_h_
#define __StCADLoader_h_

#include <AIS_InteractiveObject.hxx>
#include <NCollection_Sequence.hxx>
#include <TopoDS_Shape.hxx>

#include <StStrings/StString.h>
#include <StFile/StMIMEList.h>
#include <StGL/StPlayList.h>
#include <StGLMesh/StGLMesh.h>
#include <StSlots/StSignal.h>

class StLangMap;
class StThread;

class StCADLoader {

        public:

    static const StString ST_CAD_MIME_STRING;
    static const StMIMEList ST_CAD_MIME_LIST;
    static const StArrayList<StString> ST_CAD_EXTENSIONS_LIST;

    ST_LOCAL StCADLoader(const StHandle<StLangMap>&  theLangMap,
                         const StHandle<StPlayList>& thePlayList);
    ST_LOCAL virtual ~StCADLoader();

    ST_LOCAL void mainLoop();

    ST_LOCAL void doLoadNext() {
        myEvLoadNext.set();
    }

    ST_LOCAL virtual bool getNextResult(NCollection_Sequence<Handle(AIS_InteractiveObject)>& thePrsList);

        public:  //!< Signals

    struct {
        /**
         * Emit callback Slot on model load error.
         * @param theUserData error description
         */
        StSignal<void (const StCString& )> onError;
    } signals;

        protected:

    ST_LOCAL TopoDS_Shape loadIGES(const StString& theFileToLoadPath);
    ST_LOCAL TopoDS_Shape loadSTEP(const StString& theFileToLoadPath);

    ST_LOCAL virtual bool loadModel(const StHandle<StFileNode>& theSource);
    ST_LOCAL virtual bool computeMesh(const TopoDS_Shape& theShape);

        protected:

    StHandle<StThread>   myThread;
    StHandle<StLangMap>  myLangMap;
    StHandle<StPlayList> myPlayList;
    StCondition          myEvLoadNext;
    NCollection_Sequence<Handle(AIS_InteractiveObject)> myPrsList;
    StMutex              myResultLock;
    volatile bool        myIsLoaded;
    volatile bool        myToQuit;

};

#endif //__StCADLoader_h_
