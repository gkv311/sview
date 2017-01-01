/**
 * This source is a part of sView program.
 *
 * Copyright © Kirill Gavrilov, 2016
 */

#ifndef __StAssetDocument_h_
#define __StAssetDocument_h_

#include "StPrimArray.h"

#include <NCollection_Vector.hxx>
#include <NCollection_Sequence.hxx>

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
