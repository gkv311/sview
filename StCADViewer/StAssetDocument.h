/**
 * This source is a part of sView program.
 *
 * Copyright © Kirill Gavrilov, 2016
 */

#ifndef __StAssetDocument_h_
#define __StAssetDocument_h_

#include <gp_Trsf.hxx>
#include <NCollection_Vector.hxx>
#include <NCollection_Sequence.hxx>

#include <StStrings/StString.h>
#include <StGL/StGLVec.h>

#include <vector>

/**
 * Material definition.
 */
class StGLMaterial : public Standard_Transient {

    DEFINE_STANDARD_RTTI_INLINE(StGLMaterial, Standard_Transient)

        public:

    StGLVec4 DiffuseColor;
    StGLVec4 AmbientColor;
    StGLVec4 SpecularColor;
    StGLVec4 EmissiveColor;
    StGLVec4 Params;

        public:

    float  Shine()        const { return Params.x(); }
    float& ChangeShine()        { return Params.x(); }

    float  Transparency() const { return Params.y(); }
    float& ChangeTransparency() { return Params.y(); }

        public:

    /**
     * Check this vector with another material for equality.
     */
    bool isEqual(const StGLMaterial& theOther) const {
        return DiffuseColor  == theOther.DiffuseColor
            && AmbientColor  == theOther.AmbientColor
            && SpecularColor == theOther.SpecularColor
            && EmissiveColor == theOther.EmissiveColor
            && Params        == theOther.Params;
    }

    /**
     * Check this vector with another vector for equality.
     */
    bool operator==(const StGLMaterial& theOther)       { return isEqual(theOther); }
    bool operator==(const StGLMaterial& theOther) const { return isEqual(theOther); }

    /**
     * Check this vector with another vector for non-equality.
     */
    bool operator!=(const StGLMaterial& theOther)       { return !isEqual(theOther); }
    bool operator!=(const StGLMaterial& theOther) const { return !isEqual(theOther); }

        public:

    static int HashCode(const Handle(StGLMaterial)& theKey,
                        const int theUpper) {
        return !theKey.IsNull()
             ? ::HashCode(::HashCodes((Standard_CString )theKey.get(), sizeof(StGLVec4) * 5), theUpper)
             : 0;
    }

    static bool IsEqual(const Handle(StGLMaterial)& theKey1,
                        const Handle(StGLMaterial)& theKey2)
    {
        if(theKey1 == theKey2) {
            return true;
        }

        return !theKey1.IsNull()
            && !theKey2.IsNull()
            && theKey1->isEqual(*theKey2);
    }

};

/**
 * Definition of primitive array within the document.
 */
class StPrimArray : public Standard_Transient {

    DEFINE_STANDARD_RTTI_INLINE(StPrimArray, Standard_Transient)

        public:

    std::vector<StGLVec3> Positions;
    std::vector<StGLVec3> Normals;
    std::vector<StGLVec2> TexCoords0;
    std::vector<GLuint>   Indices;
    Handle(StGLMaterial)  Material;
    gp_Trsf               Trsf;

        public:

    StPrimArray() {}

        public:

    /**
     * Generate normals from triangles.
     * Considers the normals are initialized by ZEROs.
     */
    void reconstructNormals() {
        const size_t aNbNodes = Positions.size();
        if(Normals.size() != Positions.size()) {
            Normals.resize(aNbNodes);
        }

        const size_t aNbTris = Indices.size() / 3;
        int anElem[3] = {0, 0, 0};
        for(size_t aTriIter = 0; aTriIter < aNbTris; ++aTriIter) {
            anElem[0] = Indices[aTriIter * 3 + 0];
            anElem[1] = Indices[aTriIter * 3 + 1];
            anElem[2] = Indices[aTriIter * 3 + 2];
            const StGLVec3& aNode0 = Positions[anElem[0]];
            const StGLVec3& aNode1 = Positions[anElem[1]];
            const StGLVec3& aNode2 = Positions[anElem[2]];

            const StGLVec3 aVec01 = aNode1 - aNode0;
            const StGLVec3 aVec02 = aNode2 - aNode0;
            const StGLVec3 aTriNorm = StGLVec3::cross(aVec01, aVec02);
            for(int aNodeIter = 0; aNodeIter < 3; ++aNodeIter) {
                const int aNodeIndex = anElem[aNodeIter];
                StGLVec3& aNorm = Normals[aNodeIndex];
                aNorm += aTriNorm;
            }
        }

        // normalize
        for(int aNodeIter = 0; aNodeIter < aNbNodes; ++aNodeIter) {
            StGLVec3& aNorm = Normals[aNodeIter];
            aNorm.normalize();
        }
    }

};

/**
 * Type of the document node.
 */
enum StDocNodeType {
    StDocNodeType_Node,   //!< general node type containing child of any type
    StDocNodeType_Object, //!< object node type
    StDocNodeType_Mesh,   //!< mesh node
    StDocNodeType_Unknown //!< invalid node type
};

/**
 * Basic document node.
 */
class StDocNode : public Standard_Transient {

    DEFINE_STANDARD_RTTI_INLINE(StDocNode, Standard_Transient)

        public:

    /**
     * Return node type.
     */
    StDocNodeType nodeType() const { return myNodeType; }

    /**
     * Return node name.
     */
    const StString& nodeName() const { return myNodeName; }

    /**
     * Assign node name.
     */
    void setNodeName(const StString& theName) { myNodeName = theName; }

    /**
     * Return node local transformation (relative to the parent node).
     */
    const gp_Trsf& nodeTransformation() const { return myTrsf; }

    /**
     * Assign node local transformation.
     */
    void setNodeTransformation(const gp_Trsf& theTrsf) { myTrsf = theTrsf; }

    /**
     * Access child nodes.
     */
    const NCollection_Sequence<Handle(StDocNode)>& Children() const { return myChildren; }

    /**
     * Modify child nodes.
     */
    NCollection_Sequence<Handle(StDocNode)>& ChangeChildren() { return myChildren; }

        protected:

    /**
     * Protected constructor.
     */
    StDocNode(StDocNodeType theNodeType) : myNodeType(theNodeType) {}

        protected:

    NCollection_Sequence<Handle(StDocNode)> myChildren; //!< child nodes
    StString      myNodeName; //!< node name
    StDocNodeType myNodeType; //!< node type
    gp_Trsf       myTrsf;     //!< node local transformation

};

/**
 * The document node holding object(s).
 */
class StDocObjectNode : public StDocNode {

    DEFINE_STANDARD_RTTI_INLINE(StDocObjectNode, StDocNode)

        public:

    /**
     * Empty constructor.
     */
    StDocObjectNode() : StDocNode(StDocNodeType_Object) {}

};

/**
 * The document node holding Mesh object (list of primitive arrays).
 */
class StDocMeshNode : public StDocNode {

    DEFINE_STANDARD_RTTI_INLINE(StDocMeshNode, StDocNode)

        public:

    /**
     * Empty constructor.
     */
    StDocMeshNode() : StDocNode(StDocNodeType_Mesh) {}

    /**
     * Access primitive arrays.
     */
    const NCollection_Sequence<Handle(StPrimArray)>& PrimitiveArrays() const { return myPrimArrays; }

    /**
     * Modify primitive arrays.
     */
    NCollection_Sequence<Handle(StPrimArray)>& ChangePrimitiveArrays() { return myPrimArrays; }

        protected:

    NCollection_Sequence<Handle(StPrimArray)> myPrimArrays; //!< primitive arrays

};

/**
 * Document definition.
 */
class StAssetDocument : public StDocObjectNode {

        public:

    ST_LOCAL StAssetDocument();

};

#endif //__StAssetDocument_h_
