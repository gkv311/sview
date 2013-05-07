/**
 * This source is a part of sView program.
 *
 * Copyright © Kirill Gavrilov, 2011-2013
 */

#ifndef __StCADLoader_h_
#define __StCADLoader_h_

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

    ST_LOCAL StCADLoader(const StHandle<StLangMap>& theLangMap);
    ST_LOCAL ~StCADLoader();

    ST_LOCAL void mainLoop();

    ST_LOCAL void doRelease() {
        signals.onError.disconnect();
    }

    ST_LOCAL void doLoadNext() {
        myEvLoadNext.set();
    }

    ST_LOCAL bool getNextShape(StHandle<StGLMesh>& theMesh);

    ST_LOCAL StPlayList& getPlayList() {
        return myPlayList;
    }

        public:  //!< Signals

    struct {
        /**
         * Emit callback Slot on model load error.
         * @param theUserData (const StString& ) - error description.
         */
        StSignal<void (const StString& )> onError;
    } signals;

        private:

#ifdef ST_HAVE_OCCT
    ST_LOCAL TopoDS_Shape loadIGES(const StString& theFileToLoadPath);
    ST_LOCAL TopoDS_Shape loadSTEP(const StString& theFileToLoadPath);
#endif
    ST_LOCAL StHandle<StGLMesh> loadOBJ(const StString& theFileToLoadPath);
    ST_LOCAL bool loadModel(const StHandle<StFileNode>& theSource);
    ST_LOCAL bool computeMesh();

        private:

    StHandle<StThread>  myThread;
    StHandle<StLangMap> myLangMap;
    StPlayList          myPlayList;
    StCondition         myEvLoadNext;
    StHandle<StGLMesh>  myMesh;
    StMutex             myShapeLock;
    volatile bool       myIsLoaded;
    volatile bool       myToQuit;

};

#endif //__StCADLoader_h_
