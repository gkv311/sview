/**
 * This source is a part of sView program.
 *
 * Copyright © Kirill Gavrilov, 2016
 */

#include "StAssetPresentation.h"

#include <Graphic3d_ArrayOfTriangles.hxx>
#include <NCollection_IndexedDataMap.hxx>

/**
 * Auxiliary structure for grouping primitive arrays by common material.
 */
struct StLocatedPrimArray {
    Handle(StPrimArray) PrimArray;
    gp_Trsf             NodeTrsf;

    StLocatedPrimArray() {}
    StLocatedPrimArray(const Handle(StPrimArray)& thePrimArray,
                       const gp_Trsf& theNodeTrsf) : PrimArray(thePrimArray), NodeTrsf(theNodeTrsf) {}
};

/**
 * Auxiliary structure for grouping primitive arrays by common material.
 */
struct StPrsPart {
    NCollection_Sequence<StLocatedPrimArray> PrimArrays;
    size_t NbNodes;
    size_t NbTris;

    StPrsPart() : NbNodes(0), NbTris(0) {}
};

void StAssetPresentation::Compute (const Handle(PrsMgr_PresentationManager3d)& thePrsMgr,
                                   const Handle(Prs3d_Presentation)& thePrs,
                                   const int theMode) {
    (void )thePrsMgr;
    if(theMode != 0) {
        return;
    }

    const StPrsPart anEmptyPart;
    NCollection_IndexedDataMap<Handle(StGLMaterial), StPrsPart, StGLMaterial> aStyleMap;

    for(NCollection_Sequence<StDocLocatedMeshNode>::Iterator aDocNodeIter(myDocNodes); aDocNodeIter.More(); aDocNodeIter.Next()) {
        const Handle(StDocMeshNode)& aDocNode = aDocNodeIter.Value().Mesh;
        const gp_Trsf& aMeshTrsf = aDocNodeIter.Value().Trsf;
        for(NCollection_Sequence<Handle(StPrimArray)>::Iterator aPrimIter(aDocNode->PrimitiveArrays()); aPrimIter.More(); aPrimIter.Next()) {
            const Handle(StPrimArray)& aPrims = aPrimIter.Value();
            const int anIndex = aStyleMap.Add(aPrims->Material, anEmptyPart);
            StPrsPart& aPrsPart = aStyleMap.ChangeFromIndex(anIndex);
            aPrsPart.NbNodes += aPrims->Positions.size();
            aPrsPart.NbTris  += aPrims->Indices.size() / 3;
            aPrsPart.PrimArrays.Append(StLocatedPrimArray(aPrims, aMeshTrsf));
        }
    }

    for(NCollection_IndexedDataMap<Handle(StGLMaterial), StPrsPart, StGLMaterial>::Iterator aStyleIter(aStyleMap); aStyleIter.More(); aStyleIter.Next()) {
        const StPrsPart& aPrsPart = aStyleIter.Value();
        Handle(Graphic3d_ArrayOfTriangles) aTris = new Graphic3d_ArrayOfTriangles(int(aPrsPart.NbNodes), int(aPrsPart.NbTris * 3), true);
        for(NCollection_Sequence<StLocatedPrimArray>::Iterator aPrimIter(aPrsPart.PrimArrays); aPrimIter.More(); aPrimIter.Next()) {
            const Handle(StPrimArray)& aPrims = aPrimIter.Value().PrimArray;
            const gp_Trsf& aMeshTrsf = aPrimIter.Value().NodeTrsf;

            const int aLowerVertex = aTris->VertexNumber() + 1;

            const size_t aNbPrimNodes = aPrims->Positions.size();
            const gp_Trsf aTrsf = aMeshTrsf * aPrims->Trsf;
            if(aTrsf.Form() != gp_Identity) {
                for(size_t aNodeIter = 0; aNodeIter < aNbPrimNodes; ++aNodeIter) {
                    const StGLVec3& aPos = aPrims->Positions[aNodeIter];
                    StGLVec3 aNorm = aPrims->Normals[aNodeIter];
                    if(aNorm.modulus() != 0.0f) {
                        gp_Dir aNormTrsf(aNorm.x(), aNorm.y(), aNorm.z());
                        aNormTrsf.Transform(aTrsf);
                        aNorm.x() = (float )aNormTrsf.X();
                        aNorm.y() = (float )aNormTrsf.Y();
                        aNorm.z() = (float )aNormTrsf.Z();
                    }

                    gp_Pnt aPosTrsf(aPos.x(),  aPos.y(),  aPos.z());
                    aPosTrsf.Transform(aTrsf);
                    aTris->AddVertex((float )aPosTrsf.X(), (float )aPosTrsf.Y(), (float )aPosTrsf.Z(),
                                     aNorm.x(), aNorm.y(), aNorm.z());
                }
            } else {
                for(size_t aNodeIter = 0; aNodeIter < aNbPrimNodes; ++aNodeIter) {
                    const StGLVec3& aPos  = aPrims->Positions[aNodeIter];
                    const StGLVec3& aNorm = aPrims->Normals  [aNodeIter];
                    aTris->AddVertex(aPos.x(),  aPos.y(),  aPos.z(),
                                     aNorm.x(), aNorm.y(), aNorm.z());
                }
            }

            const size_t aNbPrimIndices = aPrims->Indices.size();
            for(size_t anIndexIter = 0; anIndexIter < aNbPrimIndices; ++anIndexIter) {
                aTris->AddEdge(aLowerVertex + aPrims->Indices[anIndexIter]);
            }
        }

        const Handle(Graphic3d_Group) aGroup = thePrs->NewGroup();
        Graphic3d_MaterialAspect aMat(Graphic3d_NOM_SILVER);
        const Handle(StGLMaterial)& anStMat = aStyleIter.Key();
        if(!anStMat.IsNull()) {
            aMat = Graphic3d_MaterialAspect();
            aMat.SetDiffuse (1.0f);
            aMat.SetAmbient (1.0f);
            aMat.SetSpecular(1.0f);
            aMat.SetEmissive(1.0f);
            aMat.SetMaterialType (Graphic3d_MATERIAL_PHYSIC);
            aMat.SetDiffuseColor (Quantity_Color(anStMat->DiffuseColor.r(),  anStMat->DiffuseColor.g(),  anStMat->DiffuseColor.b(),  Quantity_TOC_RGB));
            aMat.SetAmbientColor (Quantity_Color(anStMat->AmbientColor.r(),  anStMat->AmbientColor.g(),  anStMat->AmbientColor.b(),  Quantity_TOC_RGB));
            aMat.SetSpecularColor(Quantity_Color(anStMat->SpecularColor.r(), anStMat->SpecularColor.g(), anStMat->SpecularColor.b(), Quantity_TOC_RGB));
            aMat.SetEmissiveColor(Quantity_Color(anStMat->EmissiveColor.r(), anStMat->EmissiveColor.g(), anStMat->EmissiveColor.b(), Quantity_TOC_RGB));
            aMat.SetShininess(anStMat->Shine());
        }

        Handle(Graphic3d_AspectFillArea3d) anAspect = new Graphic3d_AspectFillArea3d(Aspect_IS_SOLID, aMat.Color(), aMat.Color(), Aspect_TOL_EMPTY, 1.0, aMat, aMat);
        aGroup->SetGroupPrimitivesAspect(anAspect);
        aGroup->AddPrimitiveArray(aTris);
    }
}

void StAssetPresentation::ComputeSelection (const Handle(SelectMgr_Selection)& ,
                                            const int ) {}
