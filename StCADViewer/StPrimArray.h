/**
 * This source is a part of sView program.
 *
 * Copyright © Kirill Gavrilov, 2016
 */

#ifndef __StPrimArray_h_
#define __StPrimArray_h_

#include "StGLMaterial.h"

#include <gp_Trsf.hxx>

#include <vector>

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

#endif // __StPrimArray_h_
