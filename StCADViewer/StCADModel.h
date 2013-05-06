/**
 * This source is a part of sView program.
 *
 * Copyright © Kirill Gavrilov, 2011-2013
 */

#ifndef __StCADModel_h_
#define __StCADModel_h_

#ifdef ST_HAVE_STCONFIG
    #include <stconfig.conf>
#endif

#ifdef ST_HAVE_OCCT

// OCCT stuff
#include <TopoDS_Shape.hxx>

#include <StGLMesh/StGLMesh.h>
#include <StGLMesh/StBndSphere.h>

class TopoDS_Face;
class TColgp_Array1OfPnt2d;

class ST_LOCAL StCADModel : public StGLMesh {

        private:

    TopoDS_Shape  myShape;      //!< original shape
    Standard_Real myDeflection; //!< deflection for triangulation
    Standard_Real myApproxDiag; //!< boundary box approximation diagonal

        private:

    bool computeNormals(const TopoDS_Face& theFace, const TColgp_Array1OfPnt2d& theUVNodes);

        public:

    StCADModel(const TopoDS_Shape& theShape);
    virtual ~StCADModel();

    virtual bool computeMesh();

};

#endif // ST_HAVE_OCCT
#endif // __StCADModel_h_
