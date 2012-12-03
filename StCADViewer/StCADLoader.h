/**
 * This source is a part of sView program.
 *
 * Copyright © Kirill Gavrilov, 2011
 */

#ifndef __StCADLoader_h_
#define __StCADLoader_h_

#include <StStrings/StString.h>
#include <StFile/StMIMEList.h>
#include <StGL/StPlayList.h>
#include <StGLMesh/StGLMesh.h>
#include <StSlots/StSignal.h>

class StLangMap;

class ST_LOCAL StCADLoader {

        private:

    StHandle<StThread>   myThread; // main loop thread
    StHandle<StLangMap> myLangMap;
    StPlayList         myPlayList; // play list
    StEvent          myEvLoadNext;
    StHandle<StGLMesh>     myMesh;
    StMutex           myShapeLock;
    volatile bool      myIsLoaded;
    volatile bool        myToQuit;

        private:

    TopoDS_Shape loadIGES(const StString& theFileToLoadPath);
    TopoDS_Shape loadSTEP(const StString& theFileToLoadPath);
    StHandle<StGLMesh> loadOBJ(const StString& theFileToLoadPath);
    bool loadModel(const StHandle<StFileNode>& theSource);
    bool computeMesh();

        public:

    static const StString ST_CAD_MIME_STRING;
    static const StMIMEList ST_CAD_MIME_LIST;
    static const StArrayList<StString> ST_CAD_EXTENSIONS_LIST;

    StCADLoader(const StHandle<StLangMap>& theLangMap);
    ~StCADLoader();

    void mainLoop();

    void doLoadNext() {
        myEvLoadNext.set();
    }

    bool getNextShape(StHandle<StGLMesh>& theMesh);

    StPlayList& getPlayList() {
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

};

#endif //__StCADLoader_h_
