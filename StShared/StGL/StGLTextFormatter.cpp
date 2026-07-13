/**
 * Copyright © 2012-2018 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#include <StGL/StGLTextFormatter.h>

#include <StGL/StGLVertexBuffer.h>

#include <algorithm>

/**
 * Auxiliary function to translate rectangles by the vector.
 */
inline void move(std::vector<StGLTile>& theRects,
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
inline void moveX(std::vector<StGLTile>& theRects,
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
inline void moveY(std::vector<StGLTile>& theRects,
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
  myParser(Parser_LiteHTML),
  myDefStyle(StFTFont::Style_Regular),
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
  myTextWidth(0.0f),
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

void StGLTextFormatter::getResult(std::vector<GLuint>&                               theTextures,
                                  std::vector< StHandle <std::vector <StGLVec2> > >& theVertsPerTexture,
                                  std::vector< StHandle <std::vector <StGLVec2> > >& theTCrdsPerTexture) const {
    StGLVec2 aVec(0.0f, 0.0f);
    theTextures.clear();
    theVertsPerTexture.clear();
    theTCrdsPerTexture.clear();

    std::vector<size_t> aNbRectPerTexture;
    for(size_t aRectIter = 0; aRectIter < myRectsNb; ++aRectIter) {
        const GLuint aTexture = myRects[aRectIter].texture;

        size_t aListId = 0;
        for(aListId = 0; aListId < theTextures.size(); ++aListId) {
            if(theTextures[aListId] == aTexture) {
                break;
            }
        }
        if(aListId >= theTextures.size()) {
            theTextures.push_back(aTexture);
            theVertsPerTexture.push_back(new std::vector<StGLVec2>());
            theTCrdsPerTexture.push_back(new std::vector<StGLVec2>());
            aNbRectPerTexture.push_back(0);
        }

        ++aNbRectPerTexture[aListId];
    }

    for(size_t aTexIter = 0; aTexIter < theTextures.size(); ++aTexIter) {
        const size_t aNbTexTris = aNbRectPerTexture[aTexIter] * 6;
        std::vector<StGLVec2>& aVerts = *theVertsPerTexture[aTexIter];
        std::vector<StGLVec2>& aTCrds = *theTCrdsPerTexture[aTexIter];
        aVerts.reserve(aNbTexTris);
        aTCrds.reserve(aNbTexTris);
    }

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

        std::vector<StGLVec2>& aVerts = *theVertsPerTexture[aListId];
        std::vector<StGLVec2>& aTCrds = *theTCrdsPerTexture[aListId];

        // apply floor on position to avoid blurring issues
        // due to cross-pixel coordinates
        aVerts.push_back(floor(aRect.getBottomLeft(aVec)));
        aVerts.push_back(floor(aRect.getTopLeft(aVec)));
        aVerts.push_back(floor(aRect.getTopRight(aVec)));
        aTCrds.push_back(aRectUV.getBottomLeft(aVec));
        aTCrds.push_back(aRectUV.getTopLeft(aVec));
        aTCrds.push_back(aRectUV.getTopRight(aVec));

        aVerts.push_back(floor(aRect.getBottomLeft(aVec)));
        aVerts.push_back(floor(aRect.getTopRight(aVec)));
        aVerts.push_back(floor(aRect.getBottomRight(aVec)));
        aTCrds.push_back(aRectUV.getBottomLeft(aVec));
        aTCrds.push_back(aRectUV.getTopRight(aVec));
        aTCrds.push_back(aRectUV.getBottomRight(aVec));
    }
}

void StGLTextFormatter::getResult(StGLContext&                                theCtx,
                                  std::vector<GLuint>&                        theTextures,
                                  StArrayList< StHandle <StGLVertexBuffer> >& theVertsPerTexture,
                                  StArrayList< StHandle <StGLVertexBuffer> >& theTCrdsPerTexture) const {
    std::vector< StHandle < std::vector<StGLVec2> > > aVertsPerTexture;
    std::vector< StHandle < std::vector<StGLVec2> > > aTCrdsPerTexture;
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
        const std::vector<StGLVec2>& aVerts = *aVertsPerTexture[aTextureIter];
        const std::vector<StGLVec2>& aTCrds = *aTCrdsPerTexture[aTextureIter];
        theVertsPerTexture[aTextureIter]->init(theCtx, aVerts);
        theTCrdsPerTexture[aTextureIter]->init(theCtx, aTCrds);
    }
}

void StGLTextFormatter::append(StGLContext&    theCtx,
                               const StString& theString,
                               StGLFont&       theFont) {
    if(myParser == Parser_LiteHTML) {
        appendHTML(theCtx, theString, theFont);
        return;
    }
    append(theCtx, theString, myDefStyle, theFont);
}

void StGLTextFormatter::append(StGLContext&          theCtx,
                               const StCString&      theString,
                               const StFTFont::Style theStyle,
                               StGLFont&             theFont) {
    if(theFont.getFont().isNull()) {
        return;
    }

    theFont.setActiveStyle(theStyle);
    myAscender    = stMax(myAscender,    theFont.getFont()->getAscender());
    myLineSpacing = stMax(myLineSpacing, theFont.getFont()->getLineSpacing());
    if(theString.isEmpty()) {
        return;
    }

    myString += theString;

    // first pass - render all symbols using associated font on single ZERO baseline
    StGLTile aTile;
    for(StUtf8Iter anIter = theString.iterator(); *anIter != 0 && anIter.getIndex() < theString.Length;) {
        const stUtf32_t aCharThis =   *anIter;
        const stUtf32_t aCharNext = *++anIter;

        if(aCharThis == '\x0D') {
            continue; // ignore CR
        } else if(aCharThis == '\x0A') {
            /// if(theWidth <= 0.0f) myAlignWidth =
            continue; // will be processed on second pass
        } else if(aCharThis == ' ') {
            myPen.x() += theFont.changeFont()->getAdvanceX(aCharThis, aCharNext);
            continue;
        }

        theFont.renderGlyph(theCtx,
                            aCharThis, aCharNext,
                            aTile, myPen);
        myRects.push_back(aTile);

        ++myRectsNb;
    }
}

enum CtrlTag {
    CtrlTag_UNKNOWN,
    CtrlTag_Italic,
    CtrlTag_Bold,
};

void StGLTextFormatter::appendHTML(StGLContext&    theCtx,
                                   const StString& theString,
                                   StGLFont&       theFont) {
    if(theFont.getFont().isNull()) {
        return;
    }

    myAscender    = stMax(myAscender,    theFont.getFont()->getAscender());
    myLineSpacing = stMax(myLineSpacing, theFont.getFont()->getLineSpacing());
    if(theString.isEmpty()) {
        return;
    }

    StUtf8Iter anIter = theString.iterator();
    size_t          aStartId  = anIter.getIndex();
    const stUtf8_t* aStartPtr = anIter.getBufferHere();
    StFTFont::Style aStyle    = myDefStyle;
    for(; *anIter != 0;) {
        const stUtf8_t* anEndPtr  =    anIter.getBufferHere();
        const size_t    aCurrId   =    anIter.getIndex();
        const stUtf32_t aCharThis =   *anIter;
        const stUtf32_t aCharNext = *++anIter;
        if(aCharThis != '<') {
            continue;
        }
        bool isClose = false;
        if(aCharNext == '/') {
            isClose = true;
            ++anIter;
        }

        CtrlTag aTag = CtrlTag_UNKNOWN;
        if(stAreEqual(anIter.getBufferHere(), "I>", 2)
        || stAreEqual(anIter.getBufferHere(), "i>", 2)) {
            aTag    = CtrlTag_Italic;
            anIter += 2;
        } else if(stAreEqual(anIter.getBufferHere(), "B>", 2)
               || stAreEqual(anIter.getBufferHere(), "b>", 2)) {
            aTag    = CtrlTag_Bold;
            anIter += 2;
        } else {
            // ignore unknown tags
            continue;
        }

        if(anEndPtr != aStartPtr) {
            const size_t    aSubSize   = size_t(anEndPtr - aStartPtr);
            const size_t    aSubLen    = aCurrId - aStartId;
            const StCString aSubString = stStringExtConstr(aStartPtr, aSubSize, aSubLen);
            append(theCtx, aSubString, aStyle, theFont);
        }
        aStartId  = anIter.getIndex();
        aStartPtr = anIter.getBufferHere();

        switch(aTag) {
            case CtrlTag_Italic: {
                if(isClose) {
                    if(aStyle == StFTFont::Style_BoldItalic) {
                        aStyle = StFTFont::Style_Bold;
                    } else if(aStyle == StFTFont::Style_Italic) {
                        aStyle = StFTFont::Style_Regular;
                    }
                } else {
                    if(aStyle == StFTFont::Style_Bold) {
                        aStyle = StFTFont::Style_BoldItalic;
                    } else if(aStyle == StFTFont::Style_Regular) {
                        aStyle = StFTFont::Style_Italic;
                    }
                }
                break;
            }
            case CtrlTag_Bold: {
                if(isClose) {
                    if(aStyle == StFTFont::Style_BoldItalic) {
                        aStyle = StFTFont::Style_Italic;
                    } else if(aStyle == StFTFont::Style_Bold) {
                        aStyle = StFTFont::Style_Regular;
                    }
                } else {
                    if(aStyle == StFTFont::Style_Italic) {
                        aStyle = StFTFont::Style_BoldItalic;
                    } else if(aStyle == StFTFont::Style_Regular) {
                        aStyle = StFTFont::Style_Bold;
                    }
                }
                break;
            }
            default: break;
        }
    }

    const stUtf8_t* anEndPtr = theString.String + theString.Size;
    if(anEndPtr != aStartPtr) {
        const size_t    aSubSize   = size_t(anEndPtr - aStartPtr);
        const size_t    aSubLen    = theString.Length - aStartId;
        const StCString aSubString = stStringExtConstr(aStartPtr, aSubSize, aSubLen);
        append(theCtx, aSubString, aStyle, theFont);
    }
}

void StGLTextFormatter::newLine(const size_t theLastRect) {
    if(myRectLineStart >= myRectsNb
    || theLastRect == size_t(-1)) {
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

void StGLTextFormatter::flipLeftRight(size_t theCharFrom, size_t theCharTo) {
    if(theCharFrom == theCharTo
    || theCharTo == size_t(-1)) {
        return;
    }

    float aRight = myRects[theCharTo].px.right();
    for(size_t aCharIter = theCharFrom; aCharIter < theCharTo; ++aCharIter) {
        StGLRect&       aRect1 = myRects[aCharIter + 0].px;
        const StGLRect& aRect2 = myRects[aCharIter + 1].px;
        float aStepX = aRect2.left() - aRect1.left();
        aRect1.moveRightTo(aRight);
        aRight -= aStepX;
    }

    StGLRect& aRectLast = myRects[theCharTo].px;
    aRectLast.moveRightTo(aRight);

    std::reverse (myRects.begin() + theCharFrom, (theCharTo + 1) == myRects.size() ? myRects.end() : myRects.begin() + theCharTo + 1);
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
    myTextWidth  = 0.0f;
    myAlignWidth = theWidth;
    myMoveVec.x() = myMoveVec.y() = 0.0f;

    // split text into lines and apply horizontal alignment
    myPenCurrLine = -myAscender;
    size_t aRectIter = 0;
    size_t aFlipLower = 0, aFlipInnerLower = 0;
    bool isRight2Left = false, isLeft2RightInner = false;
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
            if(isRight2Left) {
                if(isLeft2RightInner) {
                    flipLeftRight(aFlipInnerLower, aLastRect);
                    isLeft2RightInner = false;
                }
                flipLeftRight(aFlipLower, aLastRect);
                aFlipLower = aLastRect + 1;
            }
            newLine(aLastRect);
            continue;
        } else if(aCharThis == ' ') {
            myRectWordStart = aRectIter;
            continue;
        }

        if(StFTFont::isRightToLeft(aCharThis)) {
            if(!isRight2Left) {
                isRight2Left = true;
                aFlipLower = aRectIter;
            } else if(isLeft2RightInner) {
                flipLeftRight(aFlipInnerLower, aRectIter - 1);
                isLeft2RightInner = false;
            }
        } else if(isRight2Left) {
            if((aCharThis >= '0' && aCharThis <= '9')
            || (isLeft2RightInner && (aCharThis == '.' || aCharThis == ','))) {
                if(!isLeft2RightInner) {
                    isLeft2RightInner = true;
                    aFlipInnerLower = aRectIter;
                }
            } else {
                isRight2Left = false;
                flipLeftRight(aFlipLower, aRectIter - 1);
                aFlipLower = aRectIter;
            }
        }

        GLfloat aWidth = myRects[aRectIter].px.right() - myLineLeft;
        myTextWidth = stMax(myTextWidth, aWidth);
        if(theWidth > 0.0f && aWidth >= theWidth) {
            // force next line
            size_t aLastRect = myRectWordStart - 1; // last rect on current line
            if(myRectWordStart == myRectLineStart) {
                aLastRect = aRectIter - 1;
            }

            if(isRight2Left) {
                if(isLeft2RightInner) {
                    flipLeftRight(aFlipInnerLower, aLastRect);
                    isLeft2RightInner = false;
                }
                flipLeftRight(aFlipLower, aLastRect);
                aFlipLower = aLastRect + 1;
            }
            newLine(aLastRect);
        }

        ++aRectIter;
    }

    // move last line
    if(myRectsNb != 0) {
        myTextWidth = stMax(myTextWidth, myRects[myRectsNb - 1].px.right() - myLineLeft);
    }
    if(isRight2Left) {
        if(isLeft2RightInner) {
            flipLeftRight(aFlipInnerLower, myRectsNb - 1);
            isLeft2RightInner = false;
        }
        flipLeftRight(aFlipLower, myRectsNb - 1);
    }
    newLine(myRectsNb - 1);
    if(myRectsNb != 0
    && myAlignWidth <= 0.0f) {
        myAlignWidth = myRects[myRectsNb - 1].px.right();
    }
    if(myTextWidth > myAlignWidth) {
        //myTextWidth = myAlignWidth;
    }

    // apply vertical alignment style
    if(myAlignY == ST_ALIGN_Y_BOTTOM && theHeight > 0.0f) {
        myBndTop = -myAscender - theHeight - myPenCurrLine;
        moveY(myRects, myBndTop, 0, myRectsNb - 1);
    } else if(myAlignY == ST_ALIGN_Y_CENTER && theHeight > 0.0f) {
        myBndTop = 0.5f * (myLineSpacing * GLfloat(myLinesNb) - theHeight);
        moveY(myRects, myBndTop, 0, myRectsNb - 1);
    }
}
