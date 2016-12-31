/**
 * This source is a part of sView program.
 *
 * Copyright © Kirill Gavrilov, 2011-2016
 */

#ifdef ST_HAVE_STCONFIG
    #include <stconfig.conf>
#endif

#include "StCADLoader.h"
#include "StCADPluginInfo.h"
#include "StAssetPresentation.h"
#include "StAssetImportShape.h"
#include "StAssetNodeIterator.h"

#include <StStrings/StLangMap.h>
#include <StFile/StRawFile.h>

const StString StCADLoader::ST_CAD_MIME_STRING(ST_CAD_PLUGIN_MIME_CHAR);
const StMIMEList StCADLoader::ST_CAD_MIME_LIST(StCADLoader::ST_CAD_MIME_STRING);
const StArrayList<StString> StCADLoader::ST_CAD_EXTENSIONS_LIST(StCADLoader::ST_CAD_MIME_LIST.getExtensionsList());

StCADLoader::StCADLoader(const StHandle<StLangMap>&  theLangMap,
                         const StHandle<StPlayList>& thePlayList,
                         const bool                  theToStartThread)
: myLangMap(theLangMap),
  myPlayList(thePlayList),
  myEvLoadNext(false),
  myDefaultMat(Graphic3d_NOM_SILVER),
  myIsLoaded(false),
  myToQuit(false) {
    myPlayList->setExtensions(ST_CAD_EXTENSIONS_LIST);
    if(theToStartThread) {
        myThread = new StThread(threadFunction, (void* )this);
    }
}

StCADLoader::~StCADLoader() {
    myToQuit = true;
    myEvLoadNext.set(); // stop the thread
    myThread->wait();
    myThread.nullify();
    ///ST_DEBUG_LOG_AT("Destructor done");
}

bool StCADLoader::loadModel(const StHandle<StFileNode>& theSource) {
    const StMIME stMIMEType = theSource->getMIME();
    const StString aFileToLoadPath = theSource->getPath();
    const StString anExt = !stMIMEType.isEmpty() ? stMIMEType.getExtension() : StFileNode::getExtension(aFileToLoadPath);
    StAssetImportShape::FileFormat aShapeFormat = StAssetImportShape::FileFormat_UNKNOWN;
    {
      StRawFile aRawFile;
      if(aRawFile.readFile(aFileToLoadPath, -1, 2048)) {
          aShapeFormat = StAssetImportShape::probeFormatFromHeader((const char* )aRawFile.getBuffer(), anExt);
      }
    }

    StHandle<StAssetImportShape> aShapeImport = new StAssetImportShape();
    aShapeImport->signals.onError.connect(this, &StCADLoader::doOnErrorRedirect);

    myDoc = new StAssetDocument();
    NCollection_Sequence<Handle(AIS_InteractiveObject)> aPrsList;
    if(aShapeImport->load(myDoc, aFileToLoadPath, aShapeFormat)) {
        Handle(StAssetPresentation) aShapePrs = new StAssetPresentation();
        for(StAssetNodeIterator aMeshNodeIter(myDoc, StDocNodeType_Mesh); aMeshNodeIter.more(); aMeshNodeIter.next()) {
            Handle(StDocMeshNode) aMeshNode = Handle(StDocMeshNode)::DownCast(aMeshNodeIter.value());
            aShapePrs->AddMeshNode(aMeshNode, aMeshNodeIter.location());
        }
        aPrsList.Append(aShapePrs);
    }

    // setup new output shape
    myResultLock.lock();
        const bool isEmpty = aPrsList.IsEmpty();
        myPrsList.Assign(aPrsList);
        aPrsList.Clear();
        myIsLoaded = true;
    myResultLock.unlock();
    return !isEmpty;
}

bool StCADLoader::getNextDoc(NCollection_Sequence<Handle(AIS_InteractiveObject)>& thePrsList,
                             Handle(StAssetDocument)& theDoc) {
    if(!myResultLock.tryLock()) {
        return false;
    }

    bool hasNewShape = false;
    if(myIsLoaded) {
        thePrsList.Append(myPrsList);
        theDoc = myDoc;
        myPrsList.Clear();
        myDoc.Nullify();
        hasNewShape = true;
        myIsLoaded = false;
    }
    myResultLock.unlock();
    return hasNewShape;
}

void StCADLoader::mainLoop() {
    StHandle<StFileNode> aFileToLoad;
    StHandle<StStereoParams> aFileParams;

    StAssetImportShape::initStatic();

    for(;;) {
        myEvLoadNext.wait();
        if(myToQuit) {
            // exit the loop
            return;
        } else {
            // load next model (set as current in playlist)
            myEvLoadNext.reset();
            if(myPlayList->getCurrentFile(aFileToLoad, aFileParams)) {
                loadModel(aFileToLoad);
            }
        }
    }
}
