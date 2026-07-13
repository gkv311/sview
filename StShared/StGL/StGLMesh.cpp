/**
 * Copyright © 2011-2012 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#include <StGLMesh/StGLMesh.h>

#include <StGLCore/StGLCore20.h>
#include <StGL/StGLContext.h>

#include <stAssert.h>

StGLMeshProgram::StGLMeshProgram(const StString& theTitle)
: StGLProgram(theTitle) {
    //
}

void StGLMeshProgram::setProjMat(StGLContext&      theCtx,
                                 const StGLMatrix& theProjMat) {
    theCtx.core20fwd->glUniformMatrix4fv(uniProjMatLoc, 1, GL_FALSE, theProjMat);
}

/**
 * Setup the model view matrix.
 */
void StGLMeshProgram::setModelMat(StGLContext&      theCtx,
                                  const StGLMatrix& theModelMat) {
    theCtx.core20fwd->glUniformMatrix4fv(uniModelMatLoc, 1, GL_FALSE, theModelMat);
}

void StGLMesh::drawKernel(StGLContext& theCtx) const {
    if(myIndexBuf.isValid()) {
        // has indices
        myIndexBuf.bind(theCtx);
        theCtx.core11fwd->glDrawElements(myPrimitives, GLsizei(myIndexBuf.getElemsCount()), myIndexBuf.getDataType(), NULL);
        myIndexBuf.unbind(theCtx);
    } else {
        // just draw all vertices
        theCtx.core11fwd->glDrawArrays(myPrimitives, 0, static_cast<GLsizei>(myVertexBuf.getElemsCount()));
    }
}

void StGLMesh::bind(StGLContext&           theCtx,
                    const StGLMeshProgram& theProgram) const {
    myVertexBuf.bindVertexAttrib(theCtx, theProgram.getLocation(ST_VBO_VERTEX));
    myNormalBuf.bindVertexAttrib(theCtx, theProgram.getLocation(ST_VBO_NORMAL));
    myTCoordBuf.bindVertexAttrib(theCtx, theProgram.getLocation(ST_VBO_TCOORD));
    myColorsBuf.bindVertexAttrib(theCtx, theProgram.getLocation(ST_VBO_COLORS));
}

void StGLMesh::unbind(StGLContext&           theCtx,
                      const StGLMeshProgram& theProgram) const {
    myColorsBuf.unBindVertexAttrib(theCtx, theProgram.getLocation(ST_VBO_COLORS));
    myTCoordBuf.unBindVertexAttrib(theCtx, theProgram.getLocation(ST_VBO_TCOORD));
    myNormalBuf.unBindVertexAttrib(theCtx, theProgram.getLocation(ST_VBO_NORMAL));
    myVertexBuf.unBindVertexAttrib(theCtx, theProgram.getLocation(ST_VBO_VERTEX));
}

void StGLMesh::bindFixed(StGLContext& theCtx) const {
#if defined(GL_ES_VERSION_2_0)
    (void )theCtx;
#else
    if(myVertexBuf.isValid()) {
        myVertexBuf.bind(theCtx);
        theCtx.core11->glEnableClientState(GL_VERTEX_ARRAY);
        theCtx.core11->glVertexPointer(GLint(myVertexBuf.getElemSize()), myVertexBuf.getDataType(), 0, NULL);
    } else {
        theCtx.core11->glDisableClientState(GL_VERTEX_ARRAY);
    }
    if(myNormalBuf.isValid()) {
        //ST_ASSERT(myNormalBuf.getElemSize() == 3,
        // "StGLMesh::bindFixed() - normal buffer has wrong number of components");
        myNormalBuf.bind(theCtx);
        theCtx.core11->glEnableClientState(GL_NORMAL_ARRAY);
        theCtx.core11->glNormalPointer(myNormalBuf.getDataType(), 0, NULL);
    } else {
        theCtx.core11->glDisableClientState(GL_NORMAL_ARRAY);
    }
    if(myTCoordBuf.isValid()) {
        myTCoordBuf.bind(theCtx);
        theCtx.core11->glEnableClientState(GL_TEXTURE_COORD_ARRAY);
        theCtx.core11->glTexCoordPointer(GLint(myTCoordBuf.getElemSize()), myTCoordBuf.getDataType(), 0, NULL);
    } else {
        theCtx.core11->glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    }
    if(myColorsBuf.isValid()) {
        myColorsBuf.bind(theCtx);
        //ST_ASSERT(myColorsBuf.getElemSize() == 3 || myColorsBuf.getElemSize() == 4,
        // "StGLMesh::bindFixed() - color buffer has wrong number of components");
        theCtx.core11->glEnableClientState(GL_COLOR_ARRAY);
        theCtx.core11->glColorPointer(GLint(myColorsBuf.getElemSize()), myColorsBuf.getDataType(), 0, NULL);
    } else {
        theCtx.core11->glDisableClientState(GL_COLOR_ARRAY);
    }
#endif
}

void StGLMesh::unbindFixed(StGLContext& theCtx) const {
#if defined(GL_ES_VERSION_2_0)
    (void )theCtx;
#else
    if(myColorsBuf.isValid()) {
        myColorsBuf.unbind(theCtx);
        theCtx.core11->glDisableClientState(GL_COLOR_ARRAY);
    }
    if(myTCoordBuf.isValid()) {
        myTCoordBuf.unbind(theCtx);
        theCtx.core11->glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    }
    if(myNormalBuf.isValid()) {
        myNormalBuf.unbind(theCtx);
        theCtx.core11->glDisableClientState(GL_NORMAL_ARRAY);
    }
    if(myVertexBuf.isValid()) {
        myVertexBuf.unbind(theCtx);
        theCtx.core11->glDisableClientState(GL_VERTEX_ARRAY);
    }
#endif
}

StGLMesh::StGLMesh(const GLenum thePrimitives)
: myBndSphere(),
  myVertices(1),
  myNormals(1),
  myTCoords(1),
  myColors(1),
  myIndices(1),
  myPrimitives(thePrimitives) {
    //
}

StGLMesh::~StGLMesh() {
    ST_ASSERT(!myVertexBuf.isValid()
           && !myNormalBuf.isValid()
           && !myTCoordBuf.isValid()
           && !myColorsBuf.isValid()
           && !myIndexBuf.isValid(),
              "~StGLMesh() with unreleased GL resources");
}

void StGLMesh::release(StGLContext& theCtx) {
    clearRAM();
    clearVRAM(theCtx);
}

bool StGLMesh::computeMesh() {
    // dummy - we can not compute mesh from nowhere
    // however in general alien mesh could be set just as is
    if(!myVertices.isEmpty()) {
        if(myBndSphere.isVoid()) {
            myBndSphere.enlarge(myVertices);
        }
        return true;
    }
    return false;
};

bool StGLMesh::initVBOs(StGLContext& theCtx) {
    // reset all current VBOs
    clearVRAM(theCtx);
    if(myVertices.isEmpty() && !computeMesh()) {
        // no vertices - invalid mesh
        return false;
    }
    if(myVertices.isEmpty()) {
        // no vertices - invalid mesh
        return false;
    }
    bool isOK = myVertexBuf.init(theCtx, myVertices);
    if(!myNormals.isEmpty() && myNormals.size() == myVertices.size()) {
        isOK = isOK && myNormalBuf.init(theCtx, myNormals);
    }
    if(!myTCoords.isEmpty() && myTCoords.size() == myVertices.size()) {
        isOK = isOK && myTCoordBuf.init(theCtx, myTCoords);
    }
    if(!myColors.isEmpty() && myColors.size() == myVertices.size()) {
        isOK = isOK && myColorsBuf.init(theCtx, myColors);
    }
    if(!myIndices.isEmpty()) {
        isOK = isOK && myIndexBuf.init(theCtx, myIndices);
    }
    return isOK;

};

bool StGLMesh::check(const StGLMeshProgram& theProgram) const {
    return !(theProgram.getLocation(ST_VBO_VERTEX).isValid() && !myVertexBuf.isValid())
        && !(theProgram.getLocation(ST_VBO_NORMAL).isValid() && !myNormalBuf.isValid())
        && !(theProgram.getLocation(ST_VBO_TCOORD).isValid() && !myTCoordBuf.isValid())
        && !(theProgram.getLocation(ST_VBO_COLORS).isValid() && !myColorsBuf.isValid());
}

void StGLMesh::draw(StGLContext&           theCtx,
                    const StGLMeshProgram& theProgram) const {
    bind(theCtx, theProgram);
    drawKernel(theCtx);
    unbind(theCtx, theProgram);
}

void StGLMesh::drawFixed(StGLContext& theCtx) const {
    bindFixed(theCtx);
    drawKernel(theCtx);
    unbindFixed(theCtx);
}

void StGLMesh::clearRAM() {
    myBndSphere.reset(); // should we?
    myVertices.initList(1);
    myNormals.initList(1);
    myTCoords.initList(1);
    myColors.initList(1);
    myIndices.initList(1);
}

void StGLMesh::clearVRAM(StGLContext& theCtx) {
    myVertexBuf.release(theCtx);
    myNormalBuf.release(theCtx);
    myTCoordBuf.release(theCtx);
    myColorsBuf.release(theCtx);
    myIndexBuf.release(theCtx);
}

bool StGLMesh::computeNormals(size_t theDelta) {
    ST_ASSERT(theDelta > 0, "StGLMesh::computeNormals() - wrong delta");
    myNormals.initArray(myVertices.size());
    if(myVertices.isEmpty()) {
        return false;
    }

    // iterate over each triangle
    // for each node we compute summary of normals for all triangles where this node used
    // normals are NOT normalized per triangle - this allows to interpolate result normal
    // with respect to each triangle dimensions
    GLuint aV1, aV2, aV3;
    StGLVec3 aNorm;
    if(myIndices.size() >= 3) {
        size_t aLimit = myIndices.size() - 3;
        for(size_t anIndexId = 0; anIndexId <= aLimit; anIndexId += theDelta) {
            aV1 = myIndices[anIndexId];
            aV2 = myIndices[anIndexId + 1];
            aV3 = myIndices[anIndexId + 2];
            const StGLVec3& aVert1 = myVertices[aV1];
            const StGLVec3& aVert2 = myVertices[aV2];
            const StGLVec3& aVert3 = myVertices[aV3];
            aNorm = StGLVec3::cross(aVert2 - aVert1, aVert3 - aVert1);
            myNormals.changeValue(aV1) += aNorm;
            myNormals.changeValue(aV2) += aNorm;
            myNormals.changeValue(aV3) += aNorm;
        }
    } else if(myVertices.size() >= 3) {
        size_t aLimit = myVertices.size() - 3;
        for(size_t aVertId = 0; aVertId <= aLimit; aVertId += theDelta) {
            aV1 = GLuint(aVertId);
            aV2 = GLuint(aVertId + 1);
            aV3 = GLuint(aVertId + 2);
            const StGLVec3& aVert1 = myVertices[aV1];
            const StGLVec3& aVert2 = myVertices[aV2];
            const StGLVec3& aVert3 = myVertices[aV3];
            aNorm = StGLVec3::cross(aVert2 - aVert1, aVert3 - aVert1);
            myNormals.changeValue(aV1) += aNorm;
            myNormals.changeValue(aV2) += aNorm;
            myNormals.changeValue(aV3) += aNorm;
        }
    } else {
        return false;
    }

    // normalize normals (important for OpenGL)
    for(size_t aNormId = 0; aNormId < myNormals.size(); ++aNormId) {
        myNormals.changeValue(aNormId).normalize();
    }
    return true;
}
