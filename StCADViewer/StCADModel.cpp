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
#include <BRepAdaptor_Surface.hxx>
#include <Bnd_Box.hxx>
#include <BRepBndLib.hxx>
#include <BRepTools.hxx>
#include <BRepLProp_SLProps.hxx>
#include <BRepMesh_IncrementalMesh.hxx>
#include <Poly_Array1OfTriangle.hxx>
#include <Poly_Triangulation.hxx>
#include <Precision.hxx>
#include <TopoDS.hxx>
#include <TopExp_Explorer.hxx>
#include <TColgp_Array1OfPnt.hxx>
#include <TColgp_Array1OfPnt2d.hxx>
#include <gp_Trsf.hxx>
#include <gp_Pln.hxx>
#include <gp_Sphere.hxx>
#include <Geom_Surface.hxx>
#include <GeomLProp_SLProps.hxx>

#include <StGLCore/StGLCore20.h>
#include "StCADModel.h"
#include <StGLMesh/StBndBox.h>
#include <StThreads/StTimer.h>

/**
 * Compute boundary box approximation.
 * If triangulation is available - it will be based on it.
 */
StBndBox getApproxBndBox(const TopoDS_Shape& theShape) {
    if(theShape.IsNull()) {
        return StBndBox();
    }

    Bnd_Box aBndBox;
    BRepBndLib::Add(theShape, aBndBox);
    if(aBndBox.IsVoid()) {
        return StBndBox();
    }

    Standard_Real aXmin, aYmin, aZmin, aXmax, aYmax, aZmax;
    aBndBox.Get(aXmin, aYmin, aZmin, aXmax, aYmax, aZmax);
    return StBndBox(StGLVec3(GLfloat(aXmin), GLfloat(aYmin), GLfloat(aZmin)),
                    StGLVec3(GLfloat(aXmax), GLfloat(aYmax), GLfloat(aZmax)));
}

StCADModel::StCADModel(const TopoDS_Shape& theShape)
: StGLMesh(GL_TRIANGLES),
  myShape(theShape),
  myDeflection(0.004),
  myApproxDiag(0.0) {
    // compute deflection relative to the model dimensions
    StBndBox anApproxBndBox = getApproxBndBox(theShape);
    Standard_Real aDeflRelative = 0.004;
    myApproxDiag = (Standard_Real )(anApproxBndBox.getMax() - anApproxBndBox.getMin()).modulus();
    if(myApproxDiag > gp::Resolution()) {
        myDeflection = myApproxDiag * aDeflRelative;
    }
}

StCADModel::~StCADModel() {
    //
}

bool StCADModel::computeNormals(const TopoDS_Face& theFace, const TColgp_Array1OfPnt2d& theUVNodes) {
    bool isReversed = theFace.Orientation() == TopAbs_REVERSED;
    // notice that TopoDS_Face contains 2 locations!
    // BRep_Tool give us result location = TopDS_Shape::Location() * BRep_TFace.Location()
    // this redundant annoying location will be probably removed in next version
    // but now we should care about this difference with another TopoDS_Shape inheritances
    TopLoc_Location aLoc;
    Handle(Geom_Surface) aSurf = BRep_Tool::Surface(theFace, aLoc);
    gp_Trsf aTrsf(aLoc);
    GeomAdaptor_Surface aSurfAdaptor(aSurf);
    switch(aSurfAdaptor.GetType()) {
        case GeomAbs_Plane: {
            // compute just one normal and duplicated it
            gp_Pnt aPnt; gp_Vec aD1U, aD1V;
            aSurf->D1(0.0, 0.0, aPnt, aD1U, aD1V);
            gp_Dir aDir = (aD1U.Crossed(aD1V)).XYZ();
            aDir.Transform(aTrsf);

            if(isReversed) {
                aDir.Reverse();
            }
            StGLVec3 aNormal(GLfloat(aDir.X()), GLfloat(aDir.Y()), GLfloat(aDir.Z()));
            for(Standard_Integer aNodeId = theUVNodes.Lower(); aNodeId <= theUVNodes.Upper(); ++aNodeId) {
                myNormals.add(aNormal);
            }
            return true;
        }
        case GeomAbs_Sphere: {
            // sphere has peculiar properties on its poles
            const gp_Pnt aLoc = aSurfAdaptor.Sphere().Location().Transformed(aTrsf);
            GLfloat aRadInv = 1.0f / GLfloat(aSurfAdaptor.Sphere().Radius());
            if(isReversed) {
                aRadInv = -aRadInv;
            }
            gp_Pnt aPnt;
            for(Standard_Integer aNodeId = theUVNodes.Lower(); aNodeId <= theUVNodes.Upper(); ++aNodeId) {
                gp_Pnt2d aUVPnt = theUVNodes(aNodeId);
                aSurf->D0(aUVPnt.X(), aUVPnt.Y(), aPnt);
                aPnt = aPnt.Transformed(aTrsf);
                myNormals.add(StGLVec3(GLfloat(aPnt.X() - aLoc.X()) * aRadInv,
                                       GLfloat(aPnt.Y() - aLoc.Y()) * aRadInv,
                                       GLfloat(aPnt.Z() - aLoc.Z()) * aRadInv));
            }
            return true;
        }
        default: {
            // general case, use special tool
            gp_Dir aNormal;
            GeomLProp_SLProps aProps(aSurf,
                                     1, // needed to get normal
                                     1e-12);
            for(Standard_Integer aNodeId = theUVNodes.Lower(); aNodeId <= theUVNodes.Upper(); ++aNodeId) {
                gp_Pnt2d aUVPnt = theUVNodes(aNodeId);
                aProps.SetParameters(aUVPnt.X(), aUVPnt.Y());
                if(!aProps.IsNormalDefined()) {
                    ST_DEBUG_LOG("GeomLProp_SLProps failer!");
                    myNormals.add(StGLVec3::DZ());
                    continue;
                }
                aNormal = aProps.Normal().Transformed(aTrsf);
                if(isReversed) {
                    aNormal.Reverse();
                }
                myNormals.add(StGLVec3(GLfloat(aNormal.X()),
                                       GLfloat(aNormal.Y()),
                                       GLfloat(aNormal.Z())));
            }
            return true;
        }
    }
}

bool StCADModel::computeMesh() {
    // reset current mesh
    clearRAM();
    if(myShape.IsNull()) {
        return false;
    }

    TopLoc_Location aLoc;
    size_t aFacesCount = 0;
    size_t aVertCount = 0; // a nodes number
    size_t anIndCount = 0; // an indices number
    const Standard_Real anEps2 = Precision::Confusion() * Precision::Confusion();

    Standard_Real aDeflection = myDeflection;
    // 12.0 * Standard_PI / 180.0;
    // 20.0 * Standard_PI / 180.0;
    Standard_Real anAngular = 20.0 * M_PI / 180.0;
    const bool toComputeMesh = !BRepTools::Triangulation(myShape, myDeflection);
    StTimer aTimer(true);
    if(toComputeMesh) {
        // perform incremental mesh
        BRepMesh_IncrementalMesh aMesher(myShape, aDeflection, Standard_False, anAngular);
    }

    // If computed triangulation has greater tolerance - get the maximal value
    // to prevent unnecessary and useless recomputation of triangulation next time
    for(TopExp_Explorer aFaceExp(myShape, TopAbs_FACE); aFaceExp.More(); aFaceExp.Next()) {
        const TopoDS_Face& aFace = TopoDS::Face(aFaceExp.Current());
        const Handle(Poly_Triangulation)& aTri = BRep_Tool::Triangulation(aFace, aLoc);
        if(aTri.IsNull() || !aTri->HasUVNodes()) {
            continue;
        }
        aDeflection = stMax(aTri->Deflection(), aDeflection); // find deflection maximum

        // collect information
        anIndCount += 3 * size_t(aTri->Triangles().Length());
        aVertCount += size_t(aTri->Nodes().Length());
    }
    myDeflection = aDeflection;

    if(toComputeMesh) {
        ST_DEBUG_LOG("Triangulation computed in "
                   + aTimer.getElapsedTimeInMilliSec() + " msec\n"
                   + " with deflection " + aDeflection + " (requested " + myDeflection + ") "
                   + " and angle " + (anAngular * 180.0 / M_PI) + " degrees "
                   + "; Indices= " + anIndCount + "; Vertices= " + aVertCount); ///
    } else {
        ST_DEBUG_LOG("Triangulation reused\n"
                   + " with deflection " + aDeflection + " (requested " + myDeflection + ") "
                   + " and angle " + (anAngular * 180.0 / M_PI) + " degrees "
                   + "; Indices= " + anIndCount + "; Vertices= " + aVertCount); ///
    }

    // initialize the whole memory block at once
    myIndices.initList(anIndCount);
    myVertices.initList(aVertCount);
    myNormals.initList(aVertCount);

    Standard_Integer aV1, aV2, aV3;
    gp_Trsf aTrsf;
    // iterate over all faces
    for(TopExp_Explorer aFaceExp(myShape, TopAbs_FACE); aFaceExp.More(); aFaceExp.Next(), ++aFacesCount) {
        const TopoDS_Face& aFace = TopoDS::Face(aFaceExp.Current());
        const bool isReversed = (aFace.Orientation() == TopAbs_REVERSED);
        const Handle(Poly_Triangulation)& aTri = BRep_Tool::Triangulation(aFace, aLoc);
        aTrsf = gp_Trsf(aLoc);
        if(aTri.IsNull() || !aTri->HasUVNodes()) {
            continue;
        }

        const Poly_Array1OfTriangle& aTriangles = aTri->Triangles();
        const TColgp_Array1OfPnt&    aNodes     = aTri->Nodes();
        const TColgp_Array1OfPnt2d&  aUVNodes   = aTri->UVNodes();

        // collect indices
        Standard_Integer aNodesNbBefore = Standard_Integer(myVertices.size());
        for(Standard_Integer aTriangleId = aTriangles.Lower(); aTriangleId <= aTriangles.Upper(); ++aTriangleId) {
            aTriangles.Value(aTriangleId).Get(aV1, aV2, aV3);
            // check that triangle is not degenerative
            if(((aNodes(aV3).XYZ() -
                 aNodes(aV1).XYZ()) ^
                (aNodes(aV2).XYZ() -
                 aNodes(aV1).XYZ())).SquareModulus() > anEps2) {
                aV1 += (aNodesNbBefore - aNodes.Lower());
                aV2 += (aNodesNbBefore - aNodes.Lower());
                aV3 += (aNodesNbBefore - aNodes.Lower());
                myIndices.add(GLuint(aV1));
                if(isReversed) {
                    // this is significant only if front/back face materials are different
                    myIndices.add(GLuint(aV3));
                    myIndices.add(GLuint(aV2));
                } else {
                    myIndices.add(GLuint(aV2));
                    myIndices.add(GLuint(aV3));
                }
            }
        }

        // collect nodes
        for(Standard_Integer aNodeId = aNodes.Lower(); aNodeId <= aNodes.Upper(); ++aNodeId) {
            const gp_Pnt aNode = aNodes.Value(aNodeId).Transformed(aTrsf);
            myVertices.add(StGLVec3(GLfloat(aNode.X()), GLfloat(aNode.Y()), GLfloat(aNode.Z())));
        }

        // collect surface normals
        computeNormals(aFace, aUVNodes);
    }

    ST_DEBUG_LOG("Mesh info: nodes= " + myVertices.size()
               + "; normals= " + myNormals.size()
               + "; triangles= " + (myIndices.size() / 3)
               + "; faces= " + aFacesCount);

    // compute bounding sphere
    myBndSphere.enlarge(myVertices);
    return myVertices.size() >= 3;
}

#endif // ST_HAVE_OCCT
