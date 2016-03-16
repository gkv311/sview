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
#include <TDocStd_Document.hxx>
#include <XCAFApp_Application.hxx>

#include <StStrings/StString.h>
#include <StFile/StMIMEList.h>
#include <StGL/StPlayList.h>
#include <StGLMesh/StGLMesh.h>
#include <StSlots/StSignal.h>
#include <StThreads/StThread.h>

class StLangMap;
class StThread;

class StCADDocument {

        public:

    ST_LOCAL StCADDocument();
    ST_LOCAL void reset();

    const Handle(XCAFApp_Application)& getXCAFApp() const { return myXCAFApp; }
    const Handle(TDocStd_Document)&    getXCAFDoc() const { return myXCAFDoc; }
    Handle(TDocStd_Document)&          changeXCAFDoc()    { return myXCAFDoc; }

        protected:

    Handle(XCAFApp_Application) myXCAFApp;
    Handle(TDocStd_Document)    myXCAFDoc;

};

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
                                     StHandle<StCADDocument>& theDoc);

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
    StHandle<StCADDocument>  myDoc;
    NCollection_Sequence<Handle(AIS_InteractiveObject)> myPrsList;
    Graphic3d_MaterialAspect myDefaultMat;
    StMutex              myResultLock;
    volatile bool        myIsLoaded;
    volatile bool        myToQuit;

};

#endif //__StCADLoader_h_
