/**
 * Copyright © 2012 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#include <StGL/StGLTextFormatter.h>

#include <StGL/StGLVertexBuffer.h>

/**
 * Auxiliary function to translate rectangles by the vector.
 */
inline void move(StArray<StGLTile>& theRects,
                 const StGLVec2&    theMoveVec,
                 size_t             theCharFrom,
                 const size_t       theCharTo) {
    for(; theCharFrom <= theCharTo; ++theCharFrom) {
        StGLRect& aRect = theRects[theCharFrom].px;
        aRect.move(theMoveVec);
    }
}

/**
 * Auxiliary function to translate rectangles in horizontal direction.
 */
inline void moveX(StArray<StGLTile>& theRects,
                  const GLfloat      theMoveVec,
                  size_t             theCharFrom,
                  const size_t       theCharTo) {
    for(; theCharFrom <= theCharTo; ++theCharFrom) {
        StGLRect& aRect = theRects[theCharFrom].px;
        aRect.moveX(theMoveVec);
    }
}

/**
 * Auxiliary function to translate rectangles in vertical direction.
 */
inline void moveY(StArray<StGLTile>& theRects,
                  const GLfloat      theMoveVec,
                  size_t             theCharFrom,
                  const size_t       theCharTo) {
    for(; theCharFrom <= theCharTo; ++theCharFrom) {
        StGLRect& aRect = theRects[theCharFrom].px;
        aRect.moveY(theMoveVec);
    }
}

StGLTextFormatter::StGLTextFormatter()
: myAlignX(ST_ALIGN_X_LEFT),
  myAlignY(ST_ALIGN_Y_TOP),
  //
  myPen(0.0f, 0.0f),
  myRectsNb(0),
  myLineSpacing(0.0f),
  myAscender(0.0f),
  //
  myLinesNb(0),
  myRectLineStart(0),
  myRectWordStart(0),
  myPenCurrLine(0.0f),
  myAlignWidth(0.0f),
  myLineLeft(0.0f),
  myMoveVec(0.0f, 0.0f) {
    //
}

void StGLTextFormatter::setupAlignment(const StGLTextFormatter::StAlignX theAlignX,
                                       const StGLTextFormatter::StAlignY theAlignY) {
    myAlignX = theAlignX;
    myAlignY = theAlignY;
}

void StGLTextFormatter::reset() {
    myIsFormatted = false;
    myString.clear();
    myPen.x() = myPen.y() = 0.0f;
    myRectsNb  = 0;
    myLineSpacing = myAscender = 0.0f;
    myRects.clear(); /// TODO - clear without setting each rectangle to default value
}

/**
 * Apply floor to vector components.
 * @param  theVec - vector to change (by reference!)
 * @return modified vector
 */
inline StGLVec2& floor(StGLVec2& theVec) {
    theVec.x() = std::floor(theVec.x());
    theVec.y() = std::floor(theVec.y());
    return theVec;
}

void StGLTextFormatter::getResult(StArrayList<GLuint>&                               theTextures,
                                  StArrayList< StHandle <StArrayList <StGLVec2> > >& theVertsPerTexture,
                                  StArrayList< StHandle <StArrayList <StGLVec2> > >& theTCrdsPerTexture) const {
    StGLVec2 aVec(0.0f, 0.0f);
    theTextures.clear();
    theVertsPerTexture.clear();
    theTCrdsPerTexture.clear();
    for(size_t aRectIter = 0; aRectIter < myRectsNb; ++aRectIter) {
        const StGLRect& aRect    = myRects[aRectIter].px;
        const StGLRect& aRectUV  = myRects[aRectIter].uv;
        const GLuint    aTexture = myRects[aRectIter].texture;

        size_t aListId = 0;
        for(aListId = 0; aListId < theTextures.size(); ++aListId) {
            if(theTextures[aListId] == aTexture) {
                break;
            }
        }
        if(aListId >= theTextures.size()) {
            theTextures.add(aTexture);
            theVertsPerTexture.add(new StArrayList<StGLVec2>());
            theTCrdsPerTexture.add(new StArrayList<StGLVec2>());
        }

        StArrayList<StGLVec2>& aVerts = *theVertsPerTexture.changeValue(aListId);
        StArrayList<StGLVec2>& aTCrds = *theTCrdsPerTexture.changeValue(aListId);

        // apply floor on position to avoid blurring issues
        // due to cross-pixel coordinates
        aVerts.add(floor(aRect.getBottomLeft(aVec)));
        aVerts.add(floor(aRect.getTopLeft(aVec)));
        aVerts.add(floor(aRect.getTopRight(aVec)));
        aTCrds.add(aRectUV.getBottomLeft(aVec));
        aTCrds.add(aRectUV.getTopLeft(aVec));
        aTCrds.add(aRectUV.getTopRight(aVec));

        aVerts.add(floor(aRect.getBottomLeft(aVec)));
        aVerts.add(floor(aRect.getTopRight(aVec)));
        aVerts.add(floor(aRect.getBottomRight(aVec)));
        aTCrds.add(aRectUV.getBottomLeft(aVec));
        aTCrds.add(aRectUV.getTopRight(aVec));
        aTCrds.add(aRectUV.getBottomRight(aVec));
    }
}

void StGLTextFormatter::getResult(StGLContext&                                theCtx,
                                  StArrayList<GLuint>&                        theTextures,
                                  StArrayList< StHandle <StGLVertexBuffer> >& theVertsPerTexture,
                                  StArrayList< StHandle <StGLVertexBuffer> >& theTCrdsPerTexture) const {
    StArrayList< StHandle <StArrayList <StGLVec2> > > aVertsPerTexture;
    StArrayList< StHandle <StArrayList <StGLVec2> > > aTCrdsPerTexture;
    getResult(theTextures, aVertsPerTexture, aTCrdsPerTexture);

    if(theVertsPerTexture.size() != theTextures.size()) {
        for(size_t aTextureIter = 0; aTextureIter < theVertsPerTexture.size(); ++aTextureIter) {
            theVertsPerTexture[aTextureIter]->release(theCtx);
            theTCrdsPerTexture[aTextureIter]->release(theCtx);
        }
        theVertsPerTexture.clear();
        theTCrdsPerTexture.clear();

        while(theVertsPerTexture.size() < theTextures.size()) {
            StHandle <StGLVertexBuffer> aVertsVbo = new StGLVertexBuffer();
            StHandle <StGLVertexBuffer> aTcrdsVbo = new StGLVertexBuffer();
            theVertsPerTexture.add(aVertsVbo);
            theTCrdsPerTexture.add(aTcrdsVbo);
            aVertsVbo->init(theCtx);
            aTcrdsVbo->init(theCtx);
        }
    }

    for(size_t aTextureIter = 0; aTextureIter < theTextures.size(); ++aTextureIter) {
        const StArrayList<StGLVec2>& aVerts = *aVertsPerTexture[aTextureIter];
        const StArrayList<StGLVec2>& aTCrds = *aTCrdsPerTexture[aTextureIter];
        theVertsPerTexture[aTextureIter]->init(theCtx, aVerts);
        theTCrdsPerTexture[aTextureIter]->init(theCtx, aTCrds);
    }
}

void StGLTextFormatter::append(StGLContext&    theCtx,
                               const StString& theString,
                               StGLFont&       theFont) {
    myAscender    = stMax(myAscender,    theFont.getAscender());
    myLineSpacing = stMax(myLineSpacing, theFont.getLineSpacing());

    if(theString.isEmpty()) {
        return;
    }

    myString += theString;
    ///myVerts.initArray(theString.getLength()); /// TODO resize
    ///myTCrds.initArray(theString.getLength());

    // first pass - render all symbols using associated font on single ZERO baseline
    StGLTile aTile;
    for(StUtf8Iter anIter = theString.iterator(); *anIter != 0;) {
        const stUtf32_t aCharThis =   *anIter;
        const stUtf32_t aCharNext = *++anIter;

        if(aCharThis == '\x0D') {
            continue; // ignore CR
        } else if(aCharThis == '\x0A') {
            /// if(theWidth <= 0.0f) myAlignWidth =
            continue; // will be processed on second pass
        } else if(aCharThis == ' ') {
            myPen.x() += theFont.getAdvanceX(aCharThis, aCharNext);
            continue;
        }

        theFont.renderGlyph(theCtx,
                            aCharThis, aCharNext,
                            aTile, myPen);
        myRects.add(aTile);

        ++myRectsNb;
    }
}

void StGLTextFormatter::newLine(const size_t theLastRect) {
    if(myRectLineStart >= myRectsNb) {
        ++myLinesNb;
        myPenCurrLine -= myLineSpacing;
        return;
    }

    myMoveVec.y() = myPenCurrLine;
    switch(myAlignX) {
        default:
        case ST_ALIGN_X_LEFT: {
            myMoveVec.x() = 0.0f         - myRects[myRectLineStart].px.left();
            break;
        }
        case ST_ALIGN_X_RIGHT: {
            myMoveVec.x() = myAlignWidth - myRects[theLastRect].px.right();
            break;
        }
        case ST_ALIGN_X_CENTER: {
            myMoveVec.x() = 0.0f         - myRects[myRectLineStart].px.left();
            const GLfloat aLineWidth = myRects[theLastRect].px.right() - myRects[myRectLineStart].px.left();
            myMoveVec.x() += 0.5f * (myAlignWidth - aLineWidth);
            break;
        }
    }

    move(myRects, myMoveVec, myRectLineStart, theLastRect);

    ++myLinesNb;
    myPenCurrLine -= myLineSpacing;
    myRectLineStart = myRectWordStart = theLastRect + 1;
    if(myRectLineStart < myRectsNb) {
        myLineLeft = myRects[myRectLineStart].px.left();
    }
}

void StGLTextFormatter::format(const GLfloat theWidth,
                               const GLfloat theHeight) {
    if(myRectsNb == 0 || myIsFormatted) {
        return;
    }

    myIsFormatted = true;
    myLinesNb = myRectLineStart = myRectWordStart = 0;
    myLineLeft   = 0.0f;
    myBndTop     = 0.0f;
    myAlignWidth = theWidth;
    myMoveVec.x() = myMoveVec.y() = 0.0f;

    // split text into lines and apply horizontal alignment
    myPenCurrLine = -myAscender;
    size_t aRectIter = 0;
    for(StUtf8Iter anIter = myString.iterator(); *anIter != 0; ++anIter) {
        const stUtf32_t aCharThis = *anIter;
        if(aCharThis == '\x0D') {
            continue; // ignore CR
        } else if(aCharThis == '\x0A') {
            // new line symbol
            const size_t aLastRect = aRectIter - 1; // last rect on current line
            if(myAlignWidth <= 0.0f && aRectIter != 0) {
                myAlignWidth = myRects[aLastRect].px.right();
                ///toCorrectXAlignment = true;
            }
            newLine(aLastRect);
            continue;
        } else if(aCharThis == ' ') {
            myRectWordStart = aRectIter;
            continue;
        }

        GLfloat aWidth = myRects[aRectIter].px.right() - myLineLeft;
        if(theWidth > 0.0f && aWidth >= theWidth) {
            // force next line
            size_t aLastRect = myRectWordStart - 1; // last rect on current line
            if(myRectWordStart == myRectLineStart)
            {
                aLastRect = aRectIter - 1;
            }

            newLine(aLastRect);
        }

        ++aRectIter;
    }

    // move last line
    newLine(myRectsNb - 1);

    // apply vertical alignment style
    if(myAlignY == ST_ALIGN_Y_BOTTOM && theHeight > 0.0f) {
        myBndTop = -myAscender - theHeight - myPenCurrLine;
        moveY(myRects, myBndTop, 0, myRectsNb - 1);
    } else if(myAlignY == ST_ALIGN_Y_CENTER && theHeight > 0.0f) {
        myBndTop = 0.5f * (myLineSpacing * GLfloat(myLinesNb) - theHeight);
        moveY(myRects, myBndTop, 0, myRectsNb - 1);
    }
}
