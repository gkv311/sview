/**
 * This source is a part of sView program.
 *
 * Copyright © Kirill Gavrilov, 2016
 */

#include "StImageOcct.h"

#include "StAssetTexture.h"

#include <StFile/StMIME.h>

Handle(Image_PixMap) StAssetTexture::GetImage() const {
    Handle(Image_PixMap) anImage;
    {
        Handle(StImageOcct) anStImage = new StImageOcct();
        if(anStImage->Load(myImageUri, StMIME())) {
            anImage = anStImage;
        }
    }
    //anImage = Graphic3d_Texture2Dmanual::GetImage();

    if(anImage.IsNull()) {
        return Handle(Image_PixMap)();
    }
    return anImage;
}
