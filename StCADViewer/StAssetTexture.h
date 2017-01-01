/**
 * This source is a part of sView program.
 *
 * Copyright © Kirill Gavrilov, 2016
 */

#ifndef __StAssetTexture_h_
#define __StAssetTexture_h_

#include <Graphic3d_Texture2Dmanual.hxx>

#include <StStrings/StString.h>

/**
 * Image texture provide with equality check.
 */
class StAssetTexture : public Graphic3d_Texture2Dmanual {

    DEFINE_STANDARD_RTTI_INLINE(StAssetTexture, Graphic3d_Texture2Dmanual)

        public:

    /**
     * Constructor.
     */
    StAssetTexture(const StString& theUri)
    : Graphic3d_Texture2Dmanual(theUri.toCString()),
      myImageUri(theUri) {
        if(!theUri.isEmpty()) {
            myTexId = TCollection_AsciiString("texture://") + theUri.toCString();
        }
    }

    /**
     * Image getter.
     */
    ST_LOCAL virtual Handle(Image_PixMap) GetImage() const Standard_OVERRIDE;

    /**
     * Compare with another texture.
     */
    virtual bool isEqual(const StAssetTexture& theOther) const {
        return myImageUri == theOther.myImageUri;
    }

        public:

    /**
     * Compare to textures.
     */
    static bool IsEqual(const Handle(StAssetTexture)& theTex1,
                        const Handle(StAssetTexture)& theTex2) {
        if(theTex1 == theTex2) {
            return true;
        }

        return !theTex1.IsNull()
            && !theTex2.IsNull()
            &&  theTex1->isEqual(*theTex2);
    }

        protected:

    StString myImageUri;

};

#endif // __StAssetTexture_h_
