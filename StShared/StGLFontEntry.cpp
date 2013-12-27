/**
 * Copyright © 2012-2013 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#include <StGL/StGLFontEntry.h>

#include <StGLCore/StGLCore11.h>
#include <StGL/StGLContext.h>
#include <StGL/StGLFrameBuffer.h>

#include <StStrings/StLogger.h>
#include <stAssert.h>

StGLFontEntry::StGLFontEntry(const StHandle<StFTFont>& theFont)
: myFont(theFont),
  myAscender(0.0f),
  myLineSpacing(0.0f),
  myTileSizeX(0),
  myTileSizeY(0),
  myLastTileId(size_t(-1)) {
    stMemZero(&myLastTilePx, sizeof(myLastTilePx));
}

StGLFontEntry::~StGLFontEntry() {
    ST_ASSERT(myTextures.isEmpty(), "~StGLFontEntry() with unreleased GL resources");
}

void StGLFontEntry::release(StGLContext& theCtx) {
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

    myAscender    = 0.0f;
    myLineSpacing = 0.0f;
    myTileSizeX   = 0;
    myTileSizeY   = 0;
    stMemZero(&myLastTilePx, sizeof(myLastTilePx));
    myTiles.clear();
    myGlyphMap.clear();
    myLastTileId = size_t(-1);
}

bool StGLFontEntry::stglInit(StGLContext&       theCtx,
                             const unsigned int thePointSize,
                             const unsigned int theResolution,
                             const bool         theToCreateTexture) {
    release(theCtx);
    if(!myFont->init(thePointSize, theResolution)) {
        return false;
    }

    return stglInit(theCtx, theToCreateTexture);
}

bool StGLFontEntry::stglInit(StGLContext& theCtx,
                             const bool   theToCreateTexture) {
    release(theCtx);
    if(myFont.isNull() || !myFont->isValid()) {
        return false;
    }

    myAscender    = myFont->getAscender();
    myLineSpacing = myFont->getLineSpacing();
    myTileSizeX   = myFont->getGlyphMaxSizeX();
    myTileSizeY   = myFont->getGlyphMaxSizeY();

    myLastTileId = size_t(-1);
    return !theToCreateTexture
         || createTexture(theCtx);
}

bool StGLFontEntry::createTexture(StGLContext& theCtx) {
    const GLint aMaxSize = theCtx.getMaxTextureSize();

    GLint aGlyphsNb = myFont->getGlyphsNumber() - GLint(myLastTileId) + 1;

    const GLsizei aTextureSizeX = getPowerOfTwo(aGlyphsNb * myTileSizeX, aMaxSize);
    const size_t  aTilesPerRow  = aTextureSizeX / myTileSizeX;
    GLsizei aTextureSizeY = stMin(getEvenNumber(GLint((aGlyphsNb / aTilesPerRow) + 1) * myTileSizeY), aMaxSize);
    if(!theCtx.arbNPTW) {
        aTextureSizeY = getPowerOfTwo(aTextureSizeY, aMaxSize);
    }

    stMemZero(&myLastTilePx, sizeof(myLastTilePx));
    myLastTilePx.bottom() = myTileSizeY;

    myTextures.add(new StGLTexture(theCtx.arbTexRG ? GL_R8 : GL_ALPHA8));
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
    } else {
        ST_ERROR_LOG("Fail to bind " + (theCtx.arbTexRG ? "GL_R8" : "GL_ALPHA8") + " texture to FBO!");
    }

    return true;
}

bool StGLFontEntry::renderGlyph(StGLContext&    theCtx,
                                const stUtf32_t theChar,
                                const bool      theToForce) {
    if(!myFont->renderGlyph(theChar)) {
        if(!theToForce
        || !myFont->renderGlyphNotdef()) {
            return false;
        }
    }

    if(myTextures.isEmpty()
    && !createTexture(theCtx)) {
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
            return renderGlyph(theCtx, theChar, theToForce);
        }
    }

    /// TODO
    aTexture->bind(theCtx);
    theCtx.core11fwd->glPixelStorei(GL_UNPACK_LSB_FIRST,  GL_FALSE);
    theCtx.core11fwd->glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
    theCtx.core11fwd->glPixelStorei(GL_UNPACK_ALIGNMENT,  1);

    theCtx.core11fwd->glTexSubImage2D(GL_TEXTURE_2D, 0,
                                      myLastTilePx.left(), myLastTilePx.top(), (GLsizei )anImg.getSizeX(), (GLsizei )anImg.getSizeY(),
                                      theCtx.arbTexRG ? GL_RED : GL_ALPHA,
                                      GL_UNSIGNED_BYTE, anImg.getData());

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

bool StGLFontEntry::renderGlyph(StGLContext&    theCtx,
                                const bool      theToDrawUndef,
                                const stUtf32_t theUChar,
                                const stUtf32_t theUCharNext,
                                StGLTile&       theGlyph,
                                StGLVec2&       thePen) {
    std::unordered_map<stUtf32_t, size_t>::iterator aTileIter = myGlyphMap.find(theUChar);
    size_t aTileId;
    if(aTileIter != myGlyphMap.end()) {
        aTileId = aTileIter->second;
    } else if(renderGlyph(theCtx, theUChar, false)) {
        aTileId = myLastTileId;
        myGlyphMap[theUChar] = aTileId;
    } else if(!theToDrawUndef) {
        return false;
    } else {
        aTileIter = myGlyphMap.find(0);
        if(aTileIter != myGlyphMap.end()) {
            aTileId = aTileIter->second;
        } else if(renderGlyph(theCtx, theUChar, true)) {
            aTileId = myLastTileId;
            myGlyphMap[theUChar] = aTileId;
        } else {
            thePen.x() += myFont->getAdvanceX(theUChar, theUCharNext);
            return false;
        }
    }

    const StGLTile& aTile = myTiles[aTileId];
    StGLRect aRect(thePen.y() + aTile.px.top(),  thePen.y() + aTile.px.bottom(),
                   thePen.x() + aTile.px.left(), thePen.x() + aTile.px.right());
    theGlyph.px      = aRect;
    theGlyph.uv      = aTile.uv;
    theGlyph.texture = aTile.texture;

    thePen.x() += myFont->getAdvanceX(theUChar, theUCharNext);
    return true;
}
