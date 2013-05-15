/**
 * This source is a part of sView program.
 *
 * Copyright © Kirill Gavrilov, 2011-2013
 */

#ifdef ST_HAVE_STCONFIG
    #include <stconfig.conf>
#endif

#ifdef ST_HAVE_OCCT
    #include <BRep_Tool.hxx>
    #include <BRep_Builder.hxx>
    #include <BRepTools.hxx>          // BREP reader
    #include <BRepBuilderAPI_Sewing.hxx>
    #include <BRepMesh.hxx>
    #include <BRepLProp_SLProps.hxx>
    #include <IGESControl_Reader.hxx> // IGES reader
    #include <STEPControl_Reader.hxx> // STEP reader
    #include <XSControl_WorkSession.hxx>
    #include <ShapeFix_Shape.hxx>
    #include <TopoDS.hxx>
    #include <TopoDS_Shape.hxx>
    #include <TopExp_Explorer.hxx>
#endif

#include "StCADLoader.h"
#include "StCADPluginInfo.h"
#include "StCADModel.h"
#include "StMeshFileOBJ.h" // OBJ reader

#include <StStrings/StLangMap.h>
#include <StThreads/StThread.h>

const StString StCADLoader::ST_CAD_MIME_STRING(ST_CAD_PLUGIN_MIME_CHAR);
const StMIMEList StCADLoader::ST_CAD_MIME_LIST(StCADLoader::ST_CAD_MIME_STRING);
const StArrayList<StString> StCADLoader::ST_CAD_EXTENSIONS_LIST(StCADLoader::ST_CAD_MIME_LIST.getExtensionsList());

static SV_THREAD_FUNCTION threadFunction(void* theLoader) {
    StCADLoader* aCADLoader = (StCADLoader* )theLoader;
    aCADLoader->mainLoop();
    return SV_THREAD_RETURN 0;
}

StCADLoader::StCADLoader(const StHandle<StLangMap>& theLangMap)
: myLangMap(theLangMap),
  myPlayList(ST_CAD_EXTENSIONS_LIST, 1),
  myEvLoadNext(false),
  myIsLoaded(false),
  myToQuit(false) {
    //
    /// force thread-safe OCCT memory management
#ifdef ST_HAVE_OCCT
    Standard::SetReentrant(Standard_True);
#endif
    myThread = new StThread(threadFunction, (void* )this);
}

StCADLoader::~StCADLoader() {
    myToQuit = true;
    myEvLoadNext.set(); // stop the thread
    myThread->wait();
    myThread.nullify();
    ///ST_DEBUG_LOG_AT("Destructor done");
}

static StString formatError(const StString& theFilePath,
                            const StString& theLibDescr) {
    StString aFileName, aFolderName;
    StFileNode::getFolderAndFile(theFilePath, aFolderName, aFileName);
    ST_ERROR_LOG("Can not load CAD model from file \"" + theFilePath + "\" (" + theLibDescr + ')');
    return StString("Can not load CAD model from file\n\"") + aFileName + "\"\n" + theLibDescr;
}

#ifdef ST_HAVE_OCCT
TopoDS_Shape StCADLoader::loadIGES(const StString& theFileToLoadPath) {
    TopoDS_Shape aShape;
    IGESControl_Reader aReader;
    IFSelect_ReturnStatus aReadStatus = IFSelect_RetFail;
    StString aFilePath = StFileNode::getCompatibleName(theFileToLoadPath);

    try {
        aReadStatus = aReader.ReadFile(aFilePath.toCString());
    } catch(Standard_Failure) {
        signals.onError(formatError(theFileToLoadPath, "IGES reader, computation error"));
        return aShape;
    }
    if(aReadStatus != IFSelect_RetDone) {
        signals.onError(formatError(theFileToLoadPath, "IGES reader, bad file format"));
        return aShape;
    }

    // now perform the translation
    aReader.TransferRoots();
    if(aReader.NbShapes() <= 0) {
        Handle(XSControl_WorkSession) aWorkSession = new XSControl_WorkSession();
        aWorkSession->SelectNorm("IGES");
        aReader.SetWS(aWorkSession, Standard_True);
        aReader.SetReadVisible(Standard_False);
        aReader.TransferRoots();
    }
    if(aReader.NbShapes() <= 0) {
        signals.onError(formatError(theFileToLoadPath, "No shapes found in the IGES file"));
        return aShape;
    }
    TopoDS_Shape anImportedShape = aReader.OneShape();

    // apply sewing on the imported shape
    BRepBuilderAPI_Sewing aTool(0.0);
    aTool.SetNonManifoldMode(Standard_False);
    aTool.SetFloatingEdgesMode(Standard_True);
    aTool.Load(anImportedShape);
    aTool.Perform();
    TopoDS_Shape aSewedShape = aTool.SewedShape();

    if(aSewedShape.IsNull()) {
        signals.onError(formatError(theFileToLoadPath, "Sewing result is empty"));
        return aShape;
    }
    if(aSewedShape.IsSame(anImportedShape)) {
        aShape = anImportedShape;
    } else {
        // apply shape healing
        ShapeFix_Shape aShapeFixer(aSewedShape);
        aShapeFixer.FixSolidMode() = 1;
        aShapeFixer.FixFreeShellMode() = 1;
        aShapeFixer.FixFreeFaceMode() = 1;
        aShapeFixer.FixFreeWireMode() = 0;
        aShapeFixer.FixSameParameterMode() = 0;
        aShapeFixer.FixVertexPositionMode() = 0;
        aShape = aShapeFixer.Perform() ? aShapeFixer.Shape() : aSewedShape;
    }
    return aShape;
}

TopoDS_Shape StCADLoader::loadSTEP(const StString& theFileToLoadPath) {
    STEPControl_Reader aReader;
    IFSelect_ReturnStatus aReadStatus = IFSelect_RetFail;
    StString aFilePath = StFileNode::getCompatibleName(theFileToLoadPath);
    try {
        aReadStatus = aReader.ReadFile(aFilePath.toCString());
    } catch(Standard_Failure) {
        signals.onError(formatError(theFileToLoadPath, "STEP reader, computation error"));
        return TopoDS_Shape();
    }
    if(aReadStatus != IFSelect_RetDone) {
        signals.onError(formatError(theFileToLoadPath, "STEP reader, bad file format"));
        return TopoDS_Shape();
    } else if(aReader.NbRootsForTransfer() <= 0) {
        signals.onError(formatError(theFileToLoadPath, "STEP reader, shape is empty"));
        return TopoDS_Shape();
    }
    // now perform the translation
    aReader.TransferRoots();
    return aReader.OneShape();
}
#endif

StHandle<StGLMesh> StCADLoader::loadOBJ(const StString& theFileToLoadPath) {
    StMeshFileOBJ anOBJReader;
    if(!anOBJReader.load(theFileToLoadPath)) {
        /// description?
        return StHandle<StGLMesh>();
    }
    StHandle<StGLMesh> aMesh = anOBJReader.getResult();
    return aMesh;
}

bool StCADLoader::loadModel(const StHandle<StFileNode>& theSource) {
    const StMIME stMIMEType = theSource->getMIME();
    const StString aFileToLoadPath = theSource->getPath();
    const StString anExt = !stMIMEType.isEmpty() ? stMIMEType.getExtension() : StFileNode::getExtension(aFileToLoadPath);

    StHandle<StGLMesh> aMesh;
#ifdef ST_HAVE_OCCT
    TopoDS_Shape aShape;
    if(anExt.isEqualsIgnoreCase(stCString(ST_IGS_EXT)) || anExt.isEqualsIgnoreCase(stCString(ST_IGES_EXT))) {
        aShape = loadIGES(aFileToLoadPath);
    } else if(anExt.isEqualsIgnoreCase(stCString(ST_STP_EXT)) || anExt.isEqualsIgnoreCase(stCString(ST_STEP_EXT))) {
        aShape = loadSTEP(aFileToLoadPath);
    } else if(anExt.isEqualsIgnoreCase(stCString(ST_BREP_EXT)) || anExt.isEqualsIgnoreCase(stCString(ST_RLE_EXT))) {
        StString aFilePath = StFileNode::getCompatibleName(aFileToLoadPath);
        BRep_Builder aBuilder;
        if(!BRepTools::Read(aShape, aFilePath.toCString(), aBuilder)) {
            signals.onError(formatError(aFileToLoadPath, "BREP loading failed"));
        } else if(aShape.IsNull()) {
            signals.onError(formatError(aFileToLoadPath, "No shapes found in the BREP file"));
        }
    } else
#endif
    if(anExt.isEqualsIgnoreCase(stCString(ST_OBJ_EXT))) {
        ///signals.onError(formatError(aFileToLoadPath, "OBJ import not yet implemented!"));
        aMesh = StCADLoader::loadOBJ(aFileToLoadPath);
    } else {
        signals.onError(formatError(aFileToLoadPath, "Format doesn't supported"));
    }
#ifdef ST_HAVE_OCCT
    bool hasShape = !aShape.IsNull();
#endif

    // setup new output shape
    myShapeLock.lock();
    #ifdef ST_HAVE_OCCT
        if(!aShape.IsNull()) {
            myMesh = new StCADModel(aShape);
            computeMesh();
            aShape.Nullify();
        } else {
            myMesh = aMesh;
        }
    #else
        myMesh = aMesh;
    #endif
        myIsLoaded = true;
    myShapeLock.unlock();

#ifdef ST_HAVE_OCCT
    return hasShape;
#else
    return false;
#endif
}

bool StCADLoader::computeMesh() {
    /// TODO (Kirill Gavrilov#9)
    if(!myMesh.isNull()) {
        return myMesh->computeMesh();
    }
    return false;
}

bool StCADLoader::getNextShape(StHandle<StGLMesh>& theMesh) {
    bool hasNewShape = false;
    if(myShapeLock.tryLock()) {
        if(myIsLoaded) {
            theMesh = myMesh;
            myMesh.nullify();
            hasNewShape = true;
            myIsLoaded = false;
        }
        myShapeLock.unlock();
    }
    return hasNewShape;
}

void StCADLoader::mainLoop() {
    StHandle<StFileNode> aFileToLoad;
    StHandle<StStereoParams> aFileParams;
    for(;;) {
        myEvLoadNext.wait();
        if(myToQuit) {
            // exit the loop
            return;
        } else {
            // load next model (set as current in playlist)
            myEvLoadNext.reset();
            if(myPlayList.getCurrentFile(aFileToLoad, aFileParams)) {
                loadModel(aFileToLoad);
            }
        }
    }
}
