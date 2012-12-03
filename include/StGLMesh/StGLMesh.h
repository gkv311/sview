/**
 * Copyright © 2011-2012 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#ifndef __StGLMesh_h_
#define __StGLMesh_h_

#include <StGL/StGLMatrix.h>
#include <StGL/StGLProgram.h>
#include <StGL/StGLVertexBuffer.h>

#include "StBndSphere.h"

/**
 * Define the standard GLSL program interface to render the mesh.
 * This program contains:
 *  - projection matrix (mandatory);
 *  - model view matrix (mandatory);
 *  - vertex attribute (mandatory);
 *  - normal per vertex attribute (optional);
 *  - texture coordinates per vertex attribute (optional);
 *  - color per vertex attribute (optional).
 * Notice that variables names are not defined and you should define
 * variables locations in the program yourself (override link() method).
 */
class ST_LOCAL StGLMeshProgram : public StGLProgram {

        public:

    StGLMeshProgram(const StString& theTitle);

    /**
     * Returns the attribute location for specified VBO type.
     * @param theVBOType (StGLVBOType ) - VBO type;
     * @return variable location in the GLSL program.
     */
    StGLVarLocation getLocation(StGLVBOType theVBOType) const {
        switch(theVBOType) {
            case ST_VBO_VERTEX: return atrVVertexLoc;
            case ST_VBO_NORMAL: return atrVNormalLoc;
            case ST_VBO_TCOORD: return atrVTCoordLoc;
            case ST_VBO_COLORS: return atrVColorsLoc;
            default: return StGLVarLocation();
        }
    }

    /**
     * Setup the projection matrix.
     */
    void setProjMat(StGLContext&      theCtx,
                    const StGLMatrix& theProjMat);

    /**
     * Setup the model view matrix.
     */
    void setModelMat(StGLContext&      theCtx,
                     const StGLMatrix& theModelMat);

        protected: //! @name Uniforms

    StGLVarLocation  uniProjMatLoc; //!< projection matrix
    StGLVarLocation uniModelMatLoc; //!< model view matrix

        protected: //! @name Vertex shader attributes

    StGLVarLocation  atrVVertexLoc; //!< vertices
    StGLVarLocation  atrVNormalLoc; //!< normals
    StGLVarLocation  atrVTCoordLoc; //!< texture coordinates
    StGLVarLocation  atrVColorsLoc; //!< colors (per vertex)

};

/**
 * Rendering options.
 * This is a DRAFT list.
 */
typedef enum tagStGLFillMode {
    ST_FILL_WIREFRAME,
    ST_FILL_SHADING,
    ST_FILL_MESH,
    ST_FILL_SHADED_MESH,
} StGLFillMode;

/**
 * Class represents the drawable mesh object.
 * In general this object should be used like this:
 *  - create / define object (constructor or dedicated method with options);
 *  - prepare the mesh itself in RAM (virtual method computeMesh());
 *  - initialize OpenGL buffers, transfer mesh from RAM to GPU memory (virtual method initVBOs());
 *  - (optionally) release mesh from RAM (its already has copy in GPU memory);
 *  - draw mesh (virtual method draw());
 *  - release the object (virtual method clear()).
 */
class ST_LOCAL StGLMesh : public StGLResource {

        public:

    StGLMesh(const GLenum thePrimitives);

    virtual ~StGLMesh();

    /**
     * Invalidate current mesh buffers.
     */
    virtual void release(StGLContext& theCtx);

    /**
     * Compute the mesh (in RAM) for this object.
     * Notice that overrider should compute bounding sphere on this step.
     * @return true if mesh is not empty (at least has vertices).
     */
    virtual bool computeMesh();

    /**
     * Transfer the mesh from RAM to the GPU memory.
     * If mesh is empty computeMesh() will be called automatically!
     * @return true if VBOs are initialized.
     */
    virtual bool initVBOs(StGLContext& theCtx);

    /**
     * Access to VBO buffer using its enumeration id.
     */
    StGLVertexBuffer* changeVBO(StGLVBOType theVBOType) {
        switch(theVBOType) {
            case ST_VBO_VERTEX: return &myVertexBuf;
            case ST_VBO_NORMAL: return &myNormalBuf;
            case ST_VBO_TCOORD: return &myTCoordBuf;
            case ST_VBO_COLORS: return &myColorsBuf;
            case ST_VBO_INDEX:  return &myIndexBuf;
            default: return NULL;
        }
    }

    /**
     * @param theProgram (const StGLMeshProgram& ) - GLSL program;
     * @return false if some of enabled GLSL program attribute has no appropriate VBO.
     */
    bool check(const StGLMeshProgram& theProgram) const;

    /**
     * Draw the mesh using GLSL program.
     * GLSL program should be bound before / unbound after!
     * Notice that drawable primitives are undefined and should be specified
     * in class inheritances.
     * @param theProgram (const StGLMeshProgram& ) - GLSL program;
     */
    void draw(StGLContext&           theCtx,
              const StGLMeshProgram& theProgram) const;

    /**
     * @deprecated Draw the mesh using deprecated fixed pipeline.
     */
    void drawFixed(StGLContext& theCtx) const;

    /**
     * Minimal bounding sphere computed for mesh.
     */
    const StBndSphere& getBndSphere() const {
        return myBndSphere;
    }

    /**
     * Access to the vertices array stored in RAM.
     * The main reason to access this array outside the class - perform boundary container computations.
     * Notice that this buffer may be empty even for non empty mesh if you force clean up memory
     * after VBO creation to eliminate RAM usage.
     */
    const StArrayList<StGLVec3>& getVertices() const {
        return myVertices;
    }

    StArrayList<StGLVec3>& changeVertices() {
        return myVertices;
    }

    /**
     * Access to the normals array stored in RAM.
     * The main reason to access this array outside the class - debug rendering of normals themselves.
     * Notice that this buffer may be empty even for non empty mesh if you force clean up memory
     * after VBO creation to eliminate RAM usage.
     */
    const StArrayList<StGLVec3>& getNormals() const {
        return myNormals;
    }

    StArrayList<StGLVec3>& changeNormals() {
        return myNormals;
    }

    StArrayList<GLuint>& changeIndices() {
        return myIndices;
    }

    /**
     * Deallocate vertices and normals arrays to eliminate RAM usage.
     * Doesn't touch VBO buffers.
     */
    virtual void clearRAM();

    /**
     * Remove VBO buffers from GPU memory (OpenGL objects).
     */
    virtual void clearVRAM(StGLContext& theCtx);

    /**
     * Auxiliary method to compute normals for each vertex.
     * This method should be used only for alien mesh when
     * normals are not available. If triangulation has duplicated nodes
     * (geometrically, not indices to same vertex)
     * you will see lighting glitches on joints due to simple computation algorithm.
     * @param theDelta (size_t ) - delta between triangles start node,
     *        should be 3 for GL_TRIANGLES and 1 for GL_TRIANGLES_STRIP;
     * @return true id something was computed.
     */
    bool computeNormals(size_t theDelta = 3);

        protected:

    /**
     * Draw GL primitives itself. Should be redefined in class inheritances.
     */
    virtual void drawKernel(StGLContext& theCtx) const;

    /**
     * Bind buffers to shader program attributes.
     * @param theProgram (const StGLMeshProgram& ) - GLSL program;
     */
    void bind(StGLContext&           theCtx,
              const StGLMeshProgram& theProgram) const;

    /**
     * Unbind buffers to shader program attributes.
     * @param theProgram (const StGLMeshProgram& ) - GLSL program;
     */
    void unbind(StGLContext&           theCtx,
                const StGLMeshProgram& theProgram) const;

        protected:

    /**
     * @deprecated Bind fixed pipeline attributes.
     */
    void bindFixed(StGLContext& theCtx) const;

    /**
     * @deprecated Unbind fixed pipeline attributes.
     */
    void unbindFixed(StGLContext& theCtx) const;

        protected:

    StBndSphere           myBndSphere;  //!< minimal boundary sphere computed for mesh
    StArrayList<StGLVec3> myVertices;   //!< vertices array (stored in RAM)
    StArrayList<StGLVec3> myNormals;    //!< normals array  (stored in RAM)
    StArrayList<StGLVec2> myTCoords;    //!< texture coordinates array (stored in RAM)
    StArrayList<StGLVec4> myColors;     //!< colors array   (stored in RAM)
    StArrayList<GLuint>   myIndices;    //!< indices array  (stored in RAM)
    StGLVertexBuffer      myVertexBuf;  //!< vertices VBO   (stored in VRAM)
    StGLVertexBuffer      myNormalBuf;  //!< normals VBO    (stored in VRAM)
    StGLVertexBuffer      myTCoordBuf;  //!< texture coordinates VBO (stored in VRAM)
    StGLVertexBuffer      myColorsBuf;  //!< colors (per vertex) VBO (stored in VRAM)
    StGLIndexBuffer       myIndexBuf;   //!< indices VBO    (stored in VRAM)

    GLenum                myPrimitives; //!< GL primitives

};

#endif //__StGLMesh_h_
