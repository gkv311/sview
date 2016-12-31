/**
 * This source is a part of sView program.
 *
 * Copyright © Kirill Gavrilov, 2016
 */

#ifndef __StAssetPresentation_h_
#define __StAssetPresentation_h_

#include "StAssetDocument.h"

#include <AIS_InteractiveObject.hxx>

/**
 * Document node with cumulative transformation (including parent nodes).
 */
struct StDocLocatedMeshNode {
    Handle(StDocMeshNode) Mesh;
    gp_Trsf               Trsf;

    StDocLocatedMeshNode(const Handle(StDocMeshNode)& theMesh, const gp_Trsf& theTrsf) : Mesh(theMesh), Trsf(theTrsf) {}
    StDocLocatedMeshNode() {}
};

/**
 * Custom interactive object for mesh data.
 */
class StAssetPresentation : public AIS_InteractiveObject {

    DEFINE_STANDARD_RTTI_INLINE(StAssetPresentation, AIS_InteractiveObject)

        public:

    StAssetPresentation() {
        SetDisplayMode(0);
    }

    virtual bool AcceptDisplayMode(const int theMode) const Standard_OVERRIDE { return theMode == 0; }

    //! Redefined method to compute presentation.
    ST_LOCAL virtual void Compute(const Handle(PrsMgr_PresentationManager3d)& thePrsMgr,
                                  const Handle(Prs3d_Presentation)& thePrs,
                                  const int theMode) Standard_OVERRIDE;

    //! Compute selection.
    ST_LOCAL virtual void ComputeSelection(const Handle(SelectMgr_Selection)& theSelection,
                                           const int theMode) Standard_OVERRIDE;

    void AddMeshNode(const Handle(StDocMeshNode)& theNode,
                     const gp_Trsf& theTrsf) { myDocNodes.Append(StDocLocatedMeshNode(theNode, theTrsf)); }

        protected:

    NCollection_Sequence<StDocLocatedMeshNode> myDocNodes;

};

#endif // __StAssetPresentation_h_
