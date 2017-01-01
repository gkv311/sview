/**
 * This source is a part of sView program.
 *
 * Copyright © Kirill Gavrilov, 2016
 */

#ifndef __StGLMaterial_h_
#define __StGLMaterial_h_

#include "StAssetTexture.h"

#include <StStrings/StString.h>
#include <StGL/StGLVec.h>

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
    Handle(StAssetTexture) Texture;
    StString Name;

        public:

    float  Shine()        const { return Params.x(); }
    float& ChangeShine()        { return Params.x(); }

    float  Transparency() const { return Params.y(); }
    float& ChangeTransparency() { return Params.y(); }

    StGLMaterial()
    : DiffuseColor (0.8f, 0.8f, 0.8f, 1.0f),
      AmbientColor (0.1f, 0.1f, 0.1f, 1.0f),
      SpecularColor(0.2f, 0.2f, 0.2f, 1.0f),
      EmissiveColor(0.0f, 0.0f, 0.0f, 1.0f),
      Params(0.039f, 0.0f, 0.0f, 0.0f) {}

        public:

    /**
     * Check this vector with another material for equality.
     */
    bool isEqual(const StGLMaterial& theOther) const {
        return DiffuseColor  == theOther.DiffuseColor
            && AmbientColor  == theOther.AmbientColor
            && SpecularColor == theOther.SpecularColor
            && EmissiveColor == theOther.EmissiveColor
            && Params        == theOther.Params
            && StAssetTexture::IsEqual(Texture, theOther.Texture);
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

    /**
     * Compute hash code for data map.
     */
    static int HashCode(const Handle(StGLMaterial)& theKey,
                        const int theUpper) {
        if(theKey.IsNull()) {
            return 0;
        }

        int aHashCode = ::HashCode(::HashCodes((Standard_CString )theKey.get(), sizeof(StGLVec4) * 5), theUpper);
        if(!theKey->Texture.IsNull()) {
            aHashCode = aHashCode ^ ::HashCode(theKey->Texture->GetId(), theUpper);
        }
        return ::HashCode(aHashCode, theUpper);
    }

    /**
     * Compare two materials
     */
    static bool IsEqual(const Handle(StGLMaterial)& theKey1,
                        const Handle(StGLMaterial)& theKey2) {
        if(theKey1 == theKey2) {
            return true;
        }

        return !theKey1.IsNull()
            && !theKey2.IsNull()
            && theKey1->isEqual(*theKey2);
    }

};

#endif // __StGLMaterial_h_
