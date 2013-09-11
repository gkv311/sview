/**
 * Copyright © 2012-2013 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#include <StGL/StGLFont.h>

#include <StGLCore/StGLCore11.h>
#include <StGL/StGLContext.h>
#include <StGL/StGLFrameBuffer.h>

#include <StStrings/StLogger.h>
#include <stAssert.h>

StGLFont::StGLFont(StHandle<StFTFont>& theFont)
: myFont(theFont),
  myAscender(0.0f),
  myLineSpacing(0.0f),
  myTileSizeX(0),
  myTileSizeY(0),
  myLastTileId(size_t(-1)),
  myTextureFormat(GL_ALPHA8) {
    stMemSet(&myLastTilePx, 0, sizeof(myLastTilePx));
}

StGLFont::~StGLFont() {
    ST_ASSERT(myTextures.isEmpty(), "~StGLFont() with unreleased GL resources");
}

void StGLFont::release(StGLContext& theCtx) {
    for(size_t anIter = 0; anIter < myFbos.size(); ++anIter) {
        StHandle<StGLFrameBuffer>& aFbo = myFbos.changeValue(anIter);
        aFbo->release(theCtx);
        aFbo.nullify();
    }
    for(size_t anIter = 0; anIter < myTextures.size(); ++anIter) {
        StHandle<StGLTexture>& aTexture = myTextures.changeValue(anIter);
        aTexture->release(theCtx);
        aTexture.nullify();
    }
    myTextures.clear();
    myFbos.clear();
}

bool StGLFont::stglInit(StGLContext& theCtx) {
    release(theCtx);
    if(myFont.isNull() || !myFont->isValid()) {
        return false;
    }

    myAscender    = myFont->getAscender();
    myLineSpacing = myFont->getLineSpacing();
    myTileSizeX   = myFont->getGlyphMaxSizeX();
    myTileSizeY   = myFont->getGlyphMaxSizeY();

    myLastTileId = size_t(-1);
    return createTexture(theCtx);
}

bool StGLFont::createTexture(StGLContext& theCtx) {
    const GLint aMaxSize = theCtx.getMaxTextureSize();

    GLint aGlyphsNb = myFont->getGlyphsNumber() - GLint(myLastTileId) + 1;

    const GLsizei aTextureSizeX = getPowerOfTwo(aGlyphsNb * myTileSizeX, aMaxSize);
    const size_t  aTilesPerRow  = aTextureSizeX / myTileSizeX;
    const GLsizei aTextureSizeY = getPowerOfTwo(GLint((aGlyphsNb / aTilesPerRow) + 1) * myTileSizeY, aMaxSize);

    stMemZero(&myLastTilePx, sizeof(myLastTilePx));
    myLastTilePx.bottom() = myTileSizeY;

    myTextures.add(new StGLTexture(myTextureFormat));
    myFbos.add(new StGLFrameBuffer());
    StHandle<StGLTexture>&     aTexture = myTextures[myTextures.size() - 1];
    StHandle<StGLFrameBuffer>& aFbo     = myFbos    [myTextures.size() - 1];
    if(!aTexture->initTrash(theCtx, aTextureSizeX, aTextureSizeY)) {
        return false;
    }
    aTexture->bind(theCtx);
    theCtx.core11fwd->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    theCtx.core11fwd->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    aTexture->unbind(theCtx);

    // destruction of temporary FBO produces broken texture on Catalyst drivers for unknown reason
    //StGLFrameBuffer::clearTexture(theCtx, aTexture);
    if(aFbo->init(theCtx, aTexture, false)) {
        aFbo->clearTexture(theCtx);
    }

    return true;
}

bool StGLFont::renderGlyph(StGLContext&    theCtx,
                           const stUtf32_t theChar) {
    if(!myFont->renderGlyph(theChar)) {
        return false;
    }

    StHandle<StGLTexture>& aTexture = myTextures[myTextures.size() - 1];

    const StImagePlane& anImg = myFont->getGlyphImage();
    const size_t aTileId = myLastTileId + 1;
    myLastTilePx.left()  = myLastTilePx.right() + 3;
    myLastTilePx.right() = myLastTilePx.left() + (int )anImg.getSizeX();
    if(myLastTilePx.right() >= aTexture->getSizeX()) {
        myLastTilePx.left()    = 0;
        myLastTilePx.right()   = (int )anImg.getSizeX();
        myLastTilePx.top()    += myTileSizeY;
        myLastTilePx.bottom() += myTileSizeY;

        if(myLastTilePx.bottom() >= aTexture->getSizeY()) {
            if(!createTexture(theCtx)) {
                return false;
            }
            return renderGlyph(theCtx, theChar);
        }
    }

    /// TODO
    aTexture->bind(theCtx);
    theCtx.core11fwd->glPixelStorei(GL_UNPACK_LSB_FIRST,  GL_FALSE);
    theCtx.core11fwd->glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
    theCtx.core11fwd->glPixelStorei(GL_UNPACK_ALIGNMENT,  1);

    theCtx.core11fwd->glTexSubImage2D(GL_TEXTURE_2D, 0,
                                      myLastTilePx.left(), myLastTilePx.top(), (GLsizei )anImg.getSizeX(), (GLsizei )anImg.getSizeY(),
                                      GL_ALPHA, GL_UNSIGNED_BYTE, anImg.getData());

    StGLTile aTile;
    aTile.uv.left()   = GLfloat(myLastTilePx.left())                   / GLfloat(aTexture->getSizeX());
    aTile.uv.right()  = GLfloat(myLastTilePx.right())                  / GLfloat(aTexture->getSizeX());
    aTile.uv.top()    = GLfloat(myLastTilePx.top())                    / GLfloat(aTexture->getSizeY());
    aTile.uv.bottom() = GLfloat(myLastTilePx.top() + anImg.getSizeY()) / GLfloat(aTexture->getSizeY());
    aTile.texture     = aTexture->getTextureId();
    myFont->getGlyphRect(aTile.px);

    myLastTileId = aTileId;
    myTiles.add(aTile);
    return true;
}

void StGLFont::renderGlyph(StGLContext&    theCtx,
                           const stUtf32_t theUChar,
                           const stUtf32_t theUCharNext,
                           StGLTile&       theGlyph,
                           StGLVec2&       thePen) {
    std::map<stUtf32_t, size_t>::iterator aTileIter = myGlyphMap.find(theUChar);
    size_t aTileId;
    if(aTileIter == myGlyphMap.end()) {
        if(renderGlyph(theCtx, theUChar)) {
            aTileId = myLastTileId;
        } else {
            thePen.x() += myFont->getAdvanceX(theUChar, theUCharNext);
            return;
        }
        myGlyphMap[theUChar] = aTileId;
    } else {
        aTileId = aTileIter->second;
    }

    const StGLTile& aTile = myTiles[aTileId];
    StGLRect aRect(thePen.y() + aTile.px.top(),  thePen.y() + aTile.px.bottom(),
                   thePen.x() + aTile.px.left(), thePen.x() + aTile.px.right());
    theGlyph.px      = aRect;
    theGlyph.uv      = aTile.uv;
    theGlyph.texture = aTile.texture;

    thePen.x() += myFont->getAdvanceX(theUChar, theUCharNext);
}
