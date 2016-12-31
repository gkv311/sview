/**
 * This source is a part of sView program.
 *
 * Copyright © Kirill Gavrilov, 2011-2016
 */

#include "StAssetImportShape.h"

#include <StStrings/StLogger.h>

#include <BRep_Builder.hxx>
#include <BRepLProp_SLProps.hxx>
#include <BRepMesh_IncrementalMesh.hxx>
#include <BRepTools.hxx>
#include <BinTools.hxx>
#include <BinXCAFDrivers.hxx>
#include <IGESCAFControl_Reader.hxx>
#include <IGESControl_Controller.hxx>
#include <Interface_Static.hxx>
#include <Prs3d.hxx>
#include <STEPCAFControl_Reader.hxx>
#include <STEPControl_Controller.hxx>
#include <TDF_Tool.hxx>
#include <TDF_ChildIterator.hxx>
#include <TDataStd_Name.hxx>
#include <TDocStd_Document.hxx>
#include <Transfer_TransientProcess.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <XCAFApp_Application.hxx>
#include <XCAFDoc_ColorTool.hxx>
#include <XCAFDoc_DocumentTool.hxx>
#include <XCAFDoc_ShapeTool.hxx>
#include <XCAFPrs_Style.hxx>
#include <XCAFPrs.hxx>
#include <XSControl_TransferReader.hxx>
#include <XSControl_WorkSession.hxx>

namespace {
    static StString formatError(const StString& theFilePath,
                                const StString& theLibDescr) {
        StString aFileName, aFolderName;
        StFileNode::getFolderAndFile(theFilePath, aFolderName, aFileName);
        ST_ERROR_LOG("Can not load model from file \"" + theFilePath + "\" (" + theLibDescr + ')');
        return StString("Can not load model from file\n\"") + aFileName + "\"\n" + theLibDescr;
    }

    // Check if specified data stream starts with specified header.
    #define findFileHeader(theData, theHeader) (::strncmp(theData, theHeader, sizeof(theHeader) - 1) == 0)
}

void StAssetImportShape::initStatic() {
    STEPControl_Controller::Init();
    IGESControl_Controller::Init();
    // STEP import
    Interface_Static::SetIVal("read.step.assembly.level", 2);
    Interface_Static::SetIVal("read.step.product.mode", 0);
    Interface_Static::SetIVal("read.surfacecurve.mode", 0);
    Interface_Static::SetIVal("read.step.shape.repr", 1);
    Interface_Static::SetIVal("read.step.product.context", 1);
    Interface_Static::SetIVal("read.step.shape.aspect", true);
    Interface_Static::SetIVal("read.step.shape.relationship", true);
    // STEP export
    Interface_Static::SetCVal("write.step.schema", "AP214IS");
    Interface_Static::SetIVal("write.surfacecurve.mode", 1);
    Interface_Static::SetIVal("write.step.unit", 6); // meters
    // IGES import
    Interface_Static::SetIVal("read.iges.bspline.continuity", 0);
    Interface_Static::SetIVal("read.surfacecurve.mode", 0);
    Interface_Static::SetIVal("read.iges.onlyvisible", 0);
    // IGES export
    Interface_Static::SetIVal("write.iges.unit", 6); // meters
    Interface_Static::SetIVal("write.iges.brep.mode", 0); // V5_1
}

StAssetImportShape::FileFormat StAssetImportShape::probeFormatFromHeader(const char* theHeader,
                                                                         const StString& theExt) {
    // suspect STEP if string "ISO-10303-21" is present
    if(findFileHeader(theHeader, "ISO-10303-21")) {
        // double-check by presence of "FILE_SHEMA" statement
        if(::strstr(theHeader, "FILE_SCHEMA") != NULL) {
            return FileFormat_STEP;
        }
    } else if(findFileHeader(theHeader, "DBRep_DrawableShape")
           || findFileHeader(theHeader, "CASCADE Topology V1")) {
        return FileFormat_BREP;
    } else if(findFileHeader(theHeader, "\nOpen CASCADE Topology V3")) {
        return FileFormat_BINBREP;
    } else if(findFileHeader(theHeader, "BINFILE") && theExt.isEqualsIgnoreCase(stCString("xbf"))) {
        return FileFormat_XBF;
    }

    if(theHeader[72] == 'S') {
        const char *aPtr = theHeader + 73;
        while(aPtr < theHeader + 80 && (*aPtr == ' ' || *aPtr == '0')) {
            ++aPtr;
        }
        if(*aPtr == '1' && !::isalnum ((unsigned char )*++aPtr)) {
            return FileFormat_IGES;
        }
    }
    return FileFormat_UNKNOWN;
}

StAssetImportShape::FileFormat StAssetImportShape::probeFormatFromExtension(const StString& theExt) {
    if(theExt.isEqualsIgnoreCase(stCString("igs")) || theExt.isEqualsIgnoreCase(stCString("iges"))) {
        return FileFormat_IGES;
    } else if(theExt.isEqualsIgnoreCase(stCString("stp")) || theExt.isEqualsIgnoreCase(stCString("step"))) {
        return FileFormat_STEP;
    } else if(theExt.isEqualsIgnoreCase(stCString("brep")) || theExt.isEqualsIgnoreCase(stCString("rle"))) {
        return FileFormat_BREP;
    } else if(theExt.isEqualsIgnoreCase(stCString("xbf"))) {
        return FileFormat_XBF;
    }
    return FileFormat_UNKNOWN;
}

StAssetImportShape::StAssetImportShape()
: myXCAFApp(new TDocStd_Application()) {
    BinXCAFDrivers::DefineFormat(myXCAFApp);
    //StdLDrivers::DefineFormat(myXCAFApp);
    //BinLDrivers::DefineFormat(myXCAFApp);
    //XmlLDrivers::DefineFormat(myXCAFApp);
    //StdDrivers::DefineFormat(myXCAFApp);
    //BinDrivers::DefineFormat(myXCAFApp);
    //XmlDrivers::DefineFormat(myXCAFApp);
    reset();
}

void StAssetImportShape::reset() {
    // close old document
    if(!myXCAFDoc.IsNull()) {
        if(myXCAFDoc->HasOpenCommand()) {
            myXCAFDoc->AbortCommand();
        }

        myXCAFDoc->Main().Root().ForgetAllAttributes(Standard_True);
        myXCAFApp->Close(myXCAFDoc);
        myXCAFDoc.Nullify();
    }

    // create new document
    myXCAFApp->NewDocument(TCollection_ExtendedString("BinXCAF"), myXCAFDoc);
    if(!myXCAFDoc.IsNull()) {
        // Set the maximum number of available "undo" actions
        myXCAFDoc->SetUndoLimit(10);
    }
}

void StAssetImportShape::clearSession(const Handle(XSControl_WorkSession)& theWorkSession) {
    if(theWorkSession.IsNull()) {
        return;
    }

    // at first clear transient process
    Handle(Transfer_TransientProcess) aMapReader = theWorkSession->TransferReader()->TransientProcess();
    if(!aMapReader.IsNull()) {
        aMapReader->Clear();
    }

    // at second clear transfer reader
    Handle(XSControl_TransferReader) aTransferReader = theWorkSession->TransferReader();
    if(!aTransferReader.IsNull()) {
        aTransferReader->Clear(-1);
    }
}

bool StAssetImportShape::load(const Handle(StDocNode)& theParentNode,
                              const StString& theFile,
                              const FileFormat theFormat) {
    switch(theFormat) {
        case FileFormat_STEP: {
            if(!loadSTEP(theFile)) {
                return false;
            }
            break;
        }
        case FileFormat_IGES: {
            if(!loadIGES(theFile)) {
                return false;
            }
            break;
        }
        case FileFormat_BREP:
        case FileFormat_BINBREP: {
            BRep_Builder aBuilder;
            TopoDS_Shape aShape;
            if(theFormat == FileFormat_BINBREP
            && !BinTools::Read(aShape, theFile.toCString())) {
                signals.onError(formatError(theFile, "BREP loading failed"));
                return false;
            } else if(!BRepTools::Read(aShape, theFile.toCString(), aBuilder)) {
                signals.onError(formatError(theFile, "BREP loading failed"));
                return false;
            }

            if(aShape.IsNull()) {
                signals.onError(formatError(theFile, "No shapes found in the BREP file"));
                return false;
            }

            Handle(XCAFDoc_ShapeTool) aShapeTool = XCAFDoc_DocumentTool::ShapeTool (myXCAFDoc->Main());
            aShapeTool->AddShape(aShape);
            break;
        }
        case FileFormat_XBF: {
            if(myXCAFApp->Open(TCollection_ExtendedString(theFile.toCString(), true), myXCAFDoc) != PCDM_RS_OK) {
                signals.onError(formatError(theFile, "XBF loading failed"));
                return false;
            }
            break;
        }
        case FileFormat_UNKNOWN: {
            signals.onError(formatError(theFile, "Format doesn't supported"));
            return false;
        }
    }

    TDF_LabelSequence aLabels;
    Handle(XCAFDoc_ShapeTool) aShapeTool = XCAFDoc_DocumentTool::ShapeTool (myXCAFDoc->Main());
    Handle(XCAFDoc_ColorTool) aColorTool = XCAFDoc_DocumentTool::ColorTool(myXCAFDoc->Main());
    aShapeTool->GetFreeShapes(aLabels);
    if(aLabels.IsEmpty()) {
        signals.onError(formatError(theFile, "Empty document!"));
        return false;
    }

    // perform meshing explicitly
    TopoDS_Compound aCompound;
    BRep_Builder    aBuildTool;
    aBuildTool.MakeCompound(aCompound);
    for(TDF_LabelSequence::Iterator aLabIter(aLabels); aLabIter.More(); aLabIter.Next()) {
        TopoDS_Shape     aShape;
        const TDF_Label& aLabel = aLabIter.Value();
        if(XCAFDoc_ShapeTool::GetShape(aLabel, aShape)) {
            aBuildTool.Add(aCompound, aShape);
        }
    }

    Handle(Prs3d_Drawer) aDrawer = new Prs3d_Drawer();
    Standard_Real aDeflection = Prs3d::GetDeflection(aCompound, aDrawer);
    if(!BRepTools::Triangulation(aCompound, aDeflection)) {
        BRepMesh_IncrementalMesh anAlgo;
        anAlgo.ChangeParameters().Deflection = aDeflection;
        anAlgo.ChangeParameters().Angle      = aDrawer->HLRAngle();
        anAlgo.ChangeParameters().InParallel = true;
        anAlgo.SetShape(aCompound);
        anAlgo.Perform();
    }

    XCAFPrs_Style aDefStyle;
    aDefStyle.SetColorSurf(Quantity_NOC_GRAY65);
    aDefStyle.SetColorCurv(Quantity_NOC_GRAY65);
    for(TDF_LabelSequence::Iterator aLabIter(aLabels); aLabIter.More(); aLabIter.Next()) {
        const TDF_Label& aLabel = aLabIter.Value();
        TopLoc_Location aTrsf = XCAFDoc_ShapeTool::GetLocation(aLabel);
        addNodeRecursive(theParentNode, *aColorTool, aLabel, aTrsf, aDefStyle);
    }
    return true;
}

void StAssetImportShape::addNodeRecursive(const Handle(StDocNode)& theParentTreeItem,
                                          XCAFDoc_ColorTool&       theColorTool,
                                          const TDF_Label&         theLabel,
                                          const TopLoc_Location&   theParentTrsf,
                                          const XCAFPrs_Style&     theParentStyle) {
    TDF_Label aRefLabel = theLabel;
    if(XCAFDoc_ShapeTool::IsReference(theLabel)) {
        XCAFDoc_ShapeTool::GetReferredShape(theLabel, aRefLabel);
    }

    Handle(TDataStd_Name) aNodeName;
    TCollection_AsciiString aName;
    if(aRefLabel.FindAttribute(TDataStd_Name::GetID(), aNodeName)) {
        aName = TCollection_AsciiString(aNodeName->Get());
    }

    XCAFPrs_Style aDefStyle = theParentStyle;
    Quantity_Color aColor;
    if(theColorTool.GetColor(aRefLabel, XCAFDoc_ColorGen, aColor)) {
        aDefStyle.SetColorCurv(aColor);
        aDefStyle.SetColorSurf(aColor);
    }
    if(theColorTool.GetColor(aRefLabel, XCAFDoc_ColorSurf, aColor)) {
        aDefStyle.SetColorSurf(aColor);
    }
    if(theColorTool.GetColor(aRefLabel, XCAFDoc_ColorCurv, aColor)) {
        aDefStyle.SetColorCurv(aColor);
    }

    Handle(StDocObjectNode) aChildTreeItem = new StDocObjectNode();
    aChildTreeItem->setNodeName(aName.ToCString());
    aChildTreeItem->setNodeTransformation(theParentTrsf.Transformation());
    theParentTreeItem->ChangeChildren().Append(aChildTreeItem);
    if(!XCAFDoc_ShapeTool::IsAssembly(aRefLabel)) {
        addMeshNode(aChildTreeItem, aRefLabel, aDefStyle);
        return;
    }

    for(TDF_ChildIterator aChildIter(aRefLabel); aChildIter.More(); aChildIter.Next()) {
        TDF_Label aLabel = aChildIter.Value();
        if(!aLabel.IsNull()
        && (aLabel.HasAttribute() || aLabel.HasChild())) {
            const TopLoc_Location aTrsf = XCAFDoc_ShapeTool::GetLocation(aLabel);
            addNodeRecursive(aChildTreeItem, theColorTool, aLabel, aTrsf, aDefStyle);
        }
    }
}

bool StAssetImportShape::addMeshNode(const Handle(StDocNode)& theParentTreeItem,
                                     const TDF_Label&         theShapeLabel,
                                     const XCAFPrs_Style&     theParentStyle) {
    if(theShapeLabel.IsNull()) {
        return false;
    }

    TopoDS_Shape aShape;
    if(!XCAFDoc_ShapeTool::GetShape(theShapeLabel, aShape)
    || aShape.IsNull()) {
        return false;
    }

    XCAFPrs_DataMapOfShapeStyle aStyles1, aStyles2;
    {
        TopLoc_Location aDummyLoc;
        XCAFPrs::CollectStyleSettings(theShapeLabel, aDummyLoc, aStyles1);
        for(XCAFPrs_DataMapOfShapeStyle::Iterator aStyleIter(aStyles1); aStyleIter.More(); aStyleIter.Next()) {
            if(aStyleIter.Key().ShapeType() < TopAbs_FACE) {
                for(TopExp_Explorer aFaceIter(aStyleIter.Key(), TopAbs_FACE); aFaceIter.More(); aFaceIter.Next()) {
                    if(!aStyles1.IsBound(aFaceIter.Current())) {
                        aStyles2.Bind(aFaceIter.Current(), aStyleIter.Value());
                    }
                }
            }
        }
    }

    TopLoc_Location aFaceLoc;
    Handle(StDocMeshNode) aMeshNode = new StDocMeshNode();
    theParentTreeItem->ChangeChildren().Append(aMeshNode);
    BRepLProp_SLProps anSLProps(1, 1e-12);
    BRepAdaptor_Surface aFaceAdaptor;
    for(TopExp_Explorer aFaceIter(aShape, TopAbs_FACE); aFaceIter.More(); aFaceIter.Next()) {
        const TopoDS_Face& aFace = TopoDS::Face(aFaceIter.Current());
        const Handle(Poly_Triangulation)& aPolyTri = BRep_Tool::Triangulation(aFace, aFaceLoc);
        if(aPolyTri.IsNull()
        || aPolyTri->NbTriangles() < 1) {
            continue;
        }

        Handle(StPrimArray) aPrimAttribs = new StPrimArray();
        const TColgp_Array1OfPnt& aNodes = aPolyTri->Nodes();
        const int aNbNodes = aNodes.Size();
        aPrimAttribs->Trsf = aFaceLoc.Transformation();
        aPrimAttribs->Positions.resize(aNbNodes);
        aPrimAttribs->Normals  .resize(aNbNodes);
        aPrimAttribs->Indices  .resize(aPolyTri->NbTriangles() * 3);
        {
            const int aNodeLower = aNodes.Lower();
            const int aNodeUpper = aNodes.Upper();
            for(int aNodeIter = aNodeLower; aNodeIter <= aNodeUpper; ++aNodeIter) {
                const gp_Pnt& aSrcPos = aNodes.Value(aNodeIter);
                StGLVec3& aPos = aPrimAttribs->Positions[aNodeIter - aNodeLower];
                aPos.x() = (float )aSrcPos.X();
                aPos.y() = (float )aSrcPos.Y();
                aPos.z() = (float )aSrcPos.Z();
            }
        }

        const bool isMirrored = aPrimAttribs->Trsf.Form() != gp_Identity
                             && aPrimAttribs->Trsf.VectorialPart().Determinant() < 0.0;
        const bool isReversed = aFace.Orientation() == TopAbs_REVERSED;
        const bool toSwapIndices = isReversed ^ isMirrored;
        {
            const Poly_Array1OfTriangle& aTriangles = aPolyTri->Triangles();
            const int aNodeLower = aNodes.Lower();
            const int aTriLower = aTriangles.Lower();
            const int aTriUpper = aTriangles.Upper();
            int aTriIndices[3] = {0, 0, 0};
            int anIndexIter = 0;
            for(int aTriIter = aTriLower; aTriIter <= aTriUpper; ++aTriIter, anIndexIter += 3) {
                aTriangles.Value(aTriIter).Get(aTriIndices[0], aTriIndices[1], aTriIndices[2]);
                if(toSwapIndices) {
                    std::swap(aTriIndices[1], aTriIndices[2]);
                }
                aPrimAttribs->Indices[anIndexIter + 0] = aTriIndices[0] - aNodeLower;
                aPrimAttribs->Indices[anIndexIter + 1] = aTriIndices[1] - aNodeLower;
                aPrimAttribs->Indices[anIndexIter + 2] = aTriIndices[2] - aNodeLower;
            }
        }

        if(aPolyTri->HasNormals()
        && (aPolyTri->Normals().Size() / 3) == aPolyTri->Nodes().Size()) {
            const TShort_Array1OfShortReal& aNormals = aPolyTri->Normals();
            const int aNormLower = aNormals.Lower();
            const int aNormUpper = aNormals.Upper();
            for(int aNodeIter = 0; aNodeIter < aNbNodes; ++aNodeIter) {
                StGLVec3& aNorm = aPrimAttribs->Normals[aNodeIter];
                aNorm.x() = aNormals.Value(aNormLower + aNodeIter * 3);
                aNorm.y() = aNormals.Value(aNormLower + aNodeIter * 3 + 1);
                aNorm.z() = aNormals.Value(aNormLower + aNodeIter * 3 + 2);
                if(aNorm.modulus() != 0.0f) {
                    aNorm.normalize();
                    if(isReversed) {
                        aNorm = -aNorm;
                    }
                } else {
                    aNorm.x() = 0.0f;
                    aNorm.y() = 0.0f;
                    aNorm.z() = 1.0f;
                }
            }
        } else if(aPolyTri->HasUVNodes()
               && aPolyTri->UVNodes().Size() == aPolyTri->Nodes().Size()) {
            TopoDS_Face aFaceFwd = TopoDS::Face(aFace.Oriented(TopAbs_FORWARD));
            aFaceFwd.Location(TopLoc_Location());
            aFaceAdaptor.Initialize(aFaceFwd, false);
            anSLProps.SetSurface(aFaceAdaptor);

            const TColgp_Array1OfPnt2d& anUVNodes = aPolyTri->UVNodes();
            const int aNodeLower = anUVNodes.Lower();
            const int aNodeUpper = anUVNodes.Upper();
            for(int aNodeIter = aNodeLower; aNodeIter <= aNodeUpper; ++aNodeIter) {
                StGLVec3& aNorm = aPrimAttribs->Normals[aNodeIter - aNodeLower];
                const gp_Pnt2d& anUV = anUVNodes.Value(aNodeIter);
                anSLProps.SetParameters(anUV.X(), anUV.Y());
                if(anSLProps.IsNormalDefined()) {
                    gp_Dir aSurfNorm = anSLProps.Normal();
                    if(isReversed) {
                        aSurfNorm.Reverse();
                    }
                    aNorm.x() = (float )aSurfNorm.X();
                    aNorm.y() = (float )aSurfNorm.Y();
                    aNorm.z() = (float )aSurfNorm.Z();
                } else {
                    aNorm.x() = 0.0f;
                    aNorm.y() = 0.0f;
                    aNorm.z() = 1.0f;
                }
            }
        } else {
            // reconstruct missing normals
            aPrimAttribs->reconstructNormals();
        }

        XCAFPrs_Style aStyle = theParentStyle;
        if(!aStyles1.Find(aFace, aStyle)) {
            aStyles2.Find(aFace, aStyle);
        }

        const Graphic3d_Vec3 aColor = aStyle.GetColorSurf();
        aPrimAttribs->Material = new StGLMaterial();
        aPrimAttribs->Material->DiffuseColor.r() = aColor.r();
        aPrimAttribs->Material->DiffuseColor.g() = aColor.g();
        aPrimAttribs->Material->DiffuseColor.b() = aColor.b();
        aPrimAttribs->Material->AmbientColor.r() = aColor.r() * 0.25f;
        aPrimAttribs->Material->AmbientColor.g() = aColor.g() * 0.25f;
        aPrimAttribs->Material->AmbientColor.b() = aColor.b() * 0.25f;
        aPrimAttribs->Material->SpecularColor = StGLVec4(0.95f, 0.93f, 0.88f, 1.0f);
        aPrimAttribs->Material->EmissiveColor = StGLVec4(0.0f,  0.0f,  0.0f,  0.0f);
        aPrimAttribs->Material->ChangeShine() = 0.75f;

        aMeshNode->ChangePrimitiveArrays().Append(aPrimAttribs);
    }

    return true;
}

bool StAssetImportShape::loadIGES(const StString& theFileToLoadPath) {
    IGESCAFControl_Reader aReader;
    Handle(XSControl_WorkSession) aWS = aReader.WS();
    try {
        // read model from file
        {
            {
                Handle(Transfer_TransientProcess) aMapReader = aWS->TransferReader()->TransientProcess();
                if(!aMapReader.IsNull()) {
                    //aMapReader->SetProgress(theProgress);
                }
            }

            if(!aReader.ReadFile(theFileToLoadPath.toCString())) {
                signals.onError(formatError(theFileToLoadPath, "IGES reader, reading failed"));
                clearSession (aWS);
                return false;
            }
        }
        // translate model into document
        {
          if(!aWS.IsNull()) {
              Handle(Transfer_TransientProcess) aMapReader = aWS->TransferReader()->TransientProcess();
              if(!aMapReader.IsNull()) {
                  //aMapReader->SetProgress(theProgress);
              }
          }
          {
              if(!aReader.Transfer(myXCAFDoc)) {
                  signals.onError(formatError(theFileToLoadPath, "IGES reader, shape translation failed"));
                  clearSession(aWS);
                  return false;
              }
              clearSession(aWS);
          }
        }
    } catch(Standard_Failure theFailure) {
        signals.onError(formatError(theFileToLoadPath, StString() + "IGES reader, exception raised\n[" + theFailure.GetMessageString() + "]"));
        return false;
    }
    return true;
}

bool StAssetImportShape::loadSTEP(const StString& theFileToLoadPath) {
    STEPCAFControl_Reader aReader;
    Handle(XSControl_WorkSession) aWS = aReader.Reader().WS();
    try {
        // read model from file
        {
            {
                Handle(Transfer_TransientProcess) aMapReader = aWS->TransferReader()->TransientProcess();
                if(!aMapReader.IsNull()) {
                    //aMapReader->SetProgress(theProgress);
                }
            }

            if(!aReader.ReadFile(theFileToLoadPath.toCString())) {
                signals.onError(formatError(theFileToLoadPath, "STEP reader, reading failed"));
                clearSession(aWS);
                return false;
            }
        }

        // translate model into document
        {
            {
              Handle(Transfer_TransientProcess) aMapReader = aWS->TransferReader()->TransientProcess();
              if(!aMapReader.IsNull()) {
                  //aMapReader->SetProgress(theProgress);
              }
            }

            {
              if(!aReader.Transfer(myXCAFDoc)) {
                  signals.onError(formatError(theFileToLoadPath, "STEP reader, shape translation failed"));
                  clearSession(aWS);
                  return false;
              }
            }

            clearSession(aWS);
        }
    } catch(Standard_Failure theFailure) {
        signals.onError(formatError(theFileToLoadPath, StString() + "STEP reader, exception raised\n[" + theFailure.GetMessageString() + "]"));
        return false;
    }
    return true;
}
