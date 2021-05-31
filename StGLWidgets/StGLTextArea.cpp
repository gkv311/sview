/**
 * StGLWidgets, small C++ toolkit for writing GUI using OpenGL.
 * Copyright © 2009-2020 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#include <StGLWidgets/StGLTextArea.h>

#include <StGLWidgets/StGLRootWidget.h>
#include <StGLWidgets/StGLTextProgram.h>
#include <StGLWidgets/StGLTextBorderProgram.h>

#include <StGL/StGLContext.h>
#include <StGL/StGLProgram.h>
#include <StGLCore/StGLCore20.h>

#include <StFile/StFileNode.h>
#include <StThreads/StProcess.h>

StGLTextArea::StGLTextArea(StGLWidget* theParent,
                           const int theLeft, const int theTop,
                           const StGLCorner theCorner,
                           const int theWidth, const int theHeight,
                           const FontSize theSize)
: StGLWidget(theParent, theLeft, theTop, theCorner, theWidth, theHeight),
  mySize(theSize),
  myTextColor(0.0f, 0.0f, 0.0f, 1.0f),
  myShadowColor(0.0f, 0.0f, 0.0f, 1.0f),
  myBackColor(0.365f, 0.722f, 1.0f, 1.0f),
  myBorderColor(0.0f, 0.0f, 0.0f, 1.0f),
  myTextDX(0.0f),
  myTextWidth(-1.0f),
  myToRecompute(true),
  myToShowBorder(false),
  myToDrawShadow(false),
  myIsInitialized(false) {
    myFont = myRoot->getFontManager()->findCreate(StFTFont::Typeface_SansSerif, getFontSize());
}

StGLTextArea::~StGLTextArea() {
    StGLContext& aCtx = getContext();

    for(size_t anIter = 0; anIter < myTextVertBuf.size(); ++anIter) {
        myTextVertBuf[anIter]->release(aCtx);
        myTextTCrdBuf[anIter]->release(aCtx);
    }

    myBorderIVertBuf.release(aCtx);
    myBorderOVertBuf.release(aCtx);
}

const StString& StGLTextArea::getText() const {
    return myText;
}

bool StGLTextArea::setText(const StString& theText) {
    if(myText != theText) {
        myText = theText;
        myToRecompute = true;
        return true;
    }
    return false;
}

void StGLTextArea::setTextWidth(const int theWidth) {
    myTextWidth = (GLfloat )theWidth;
    myToRecompute = true;
}

void StGLTextArea::computeTextWidthFake(const StString& theText,
                                        int&            theWidth,
                                        int&            theHeight) {
    theWidth  = myRoot->scale(int(10 * (theText.getLength() + 2)));
    theHeight = myRoot->scale(16);
}

void StGLTextArea::computeTextWidth(const StString& theText,
                                    const GLfloat   theWidthMax,
                                    int&            theWidth,
                                    int&            theHeight) {
    if(myFont.isNull()) {
        computeTextWidthFake(theText, theWidth, theHeight);
        return;
    }
    StHandle<StGLFontEntry>& aFontGenEntry = myFont->changeFont();
    if(aFontGenEntry.isNull()) {
        computeTextWidthFake(theText, theWidth, theHeight);
        return;
    }
    StHandle<StFTFont>& aFontGen = aFontGenEntry->getFont();
    if(aFontGen.isNull() || !aFontGen->isValid()) {
        computeTextWidthFake(theText, theWidth, theHeight);
        return;
    }

    GLfloat aWidth    = 0.0f;
    GLfloat aWidthMax = 0.0f;
    size_t  aCharsInLine = 0;
    size_t  aNbLines     = 1;
    aFontGen->setActiveStyle(myFormatter.getDefaultStyle());
    for(StUtf8Iter anIter = theText.iterator(); *anIter != 0;) {
        const stUtf32_t aCharThis =   *anIter;
        const stUtf32_t aCharNext = *++anIter;

        if(aCharThis == '\x0D') {
            continue; // ignore CR
        } else if(aCharThis == '\x0A') {
            aWidthMax = stMax(aWidthMax, aWidth);
            aWidth = 0.0f;
            ++aNbLines;
            aCharsInLine = 0;
            continue; // will be processed on second pass
        } else if(aCharThis == ' ') {
            aWidth += aFontGen->getAdvanceX(aCharThis, aCharNext);
            continue;
        }

        const StFTFont::Subset   aSubset = StFTFont::subset(aCharThis);
        StHandle<StGLFontEntry>& anEntry = myFont->changeFont(aSubset);
        GLfloat anAdvance = 0.0f;
        if(!anEntry.isNull() && !anEntry->getFont().isNull()) {
            anAdvance = anEntry->getFont()->getAdvanceX(aCharThis, aCharNext);
        } else {
            anAdvance = aFontGen->getAdvanceX(aCharThis, aCharNext);
        }

        aWidth += anAdvance;
        if(theWidthMax > 0.0f
        && aWidth > theWidthMax) {
            if(aCharsInLine != 0) {
                aWidthMax = stMax(aWidthMax, aWidth - anAdvance);
                aWidth    = anAdvance;
            } else {
                aWidthMax = stMax(aWidthMax, aWidth);
                aWidth    = 0.0f;
            }
            ++aNbLines;
        }
    }
    aWidthMax = stMax(aWidthMax, aWidth);
    theWidth  = int(aWidthMax + 1.5f);
    theHeight = int(GLfloat(aNbLines) * aFontGen->getLineSpacing() + 0.5f);
}

bool StGLTextArea::stglInitAutoHeightWidth(const int theMaxWidth) {
    changeRectPx().right() = getRectPx().left() + theMaxWidth; // compute width from text
    if(!stglInit()) {
        return false;
    }
    changeRectPx().bottom() = getRectPx().top() + getTextHeight();
    if(theMaxWidth > 0) {
        changeRectPx().right() = getRectPx().left() + GLint(myFormatter.getMaxLineWidth() + 2.5f);
        myTextWidth = (GLfloat )getRectPx().width();
        myToRecompute = true;
    } else {
        changeRectPx().right() = getRectPx().left() + getTextWidth();
    }
    return true;
}

bool StGLTextArea::stglInit() {
    StGLContext& aCtx = getContext();
    if(myIsInitialized) {
        if(isVisible()) {
            formatText(aCtx);
        }
        return true;
    }

    // initialize GL resources for the font
    if(myFont.isNull()) {
        return false; // critical error
    } else if(!myFont->wasInitialized()) {
        if( myFont->changeFont().isNull()
        ||  myFont->changeFont()->getFont().isNull()
        || !myFont->changeFont()->getFont()->isValid()) {
            return false; // critical error
        } else if(!myFont->stglInit(aCtx)) {
            aCtx.pushError(StString("Could not initialize OpenGL resources for font"));
            return false;
        }
    } else if(!myFont->changeFont()->isValid()) {
        return false;
    }

    myTextWidth = (GLfloat )getRectPx().width();
    myIsInitialized = true;

    myBorderIVertBuf.init(aCtx);
    myBorderOVertBuf.init(aCtx);

    if(isVisible()) {
        formatText(aCtx);
    }

    return StGLWidget::stglInit();
}

void StGLTextArea::recomputeBorder(StGLContext& theCtx) {
    GLfloat aTextAreaW = GLfloat(getRectPx().width());
    GLfloat aMarg      = GLfloat(myRoot->scale(3));
    const GLfloat quadVerticesInner[4 * 4] = {
        aMarg + aTextAreaW, myTextBndBox.top()    + aMarg, 0.0f, 1.0f, // top-right
        aMarg + aTextAreaW, myTextBndBox.bottom() - aMarg, 0.0f, 1.0f, // bottom-right
                    -aMarg, myTextBndBox.top()    + aMarg, 0.0f, 1.0f, // top-left
                    -aMarg, myTextBndBox.bottom() - aMarg, 0.0f, 1.0f  // bottom-left
    };
    aMarg += 1.0f;
    const GLfloat quadVerticesOuter[4 * 4] = {
        aMarg + aTextAreaW, myTextBndBox.top()    + aMarg, 0.0f, 1.0f, // top-right
        aMarg + aTextAreaW, myTextBndBox.bottom() - aMarg, 0.0f, 1.0f, // bottom-right
                    -aMarg, myTextBndBox.top()    + aMarg, 0.0f, 1.0f, // top-left
                    -aMarg, myTextBndBox.bottom() - aMarg, 0.0f, 1.0f  // bottom-left
    };
    myBorderIVertBuf.init(theCtx, 4, 4, quadVerticesInner);
    myBorderOVertBuf.init(theCtx, 4, 4, quadVerticesOuter);
}

void StGLTextArea::formatText(StGLContext& theCtx) {
    if(myToRecompute) {
        myFormatter.reset();
        myFormatter.append(theCtx, myText, *myFont);
        myFormatter.format(myTextWidth, GLfloat(getRectPx().height()));
        myFormatter.getResult(theCtx, myTexturesList, myTextVertBuf, myTextTCrdBuf);
        myFormatter.getBndBox(myTextBndBox);
        if(myToShowBorder) {
            recomputeBorder(theCtx);
        }
        myToRecompute = false;
    }
}

void StGLTextArea::drawText(StGLContext& theCtx) {
    theCtx.core20fwd->glActiveTexture(GL_TEXTURE0);
    StGLTextProgram& aProgram = myRoot->getTextProgram();
    for(size_t aTextureIter = 0; aTextureIter < myTexturesList.size(); ++aTextureIter) {
        if(!myTextVertBuf[aTextureIter]->isValid() || myTextVertBuf[aTextureIter]->getElemsCount() < 1) {
            continue;
        }

        theCtx.core20fwd->glBindTexture(GL_TEXTURE_2D, myTexturesList[aTextureIter]);

        myTextVertBuf[aTextureIter]->bindVertexAttrib(theCtx, aProgram.getVVertexLoc());
        myTextTCrdBuf[aTextureIter]->bindVertexAttrib(theCtx, aProgram.getVTexCoordLoc());

        theCtx.core20fwd->glDrawArrays(GL_TRIANGLES, 0, GLsizei(myTextVertBuf[aTextureIter]->getElemsCount()));

        myTextTCrdBuf[aTextureIter]->unBindVertexAttrib(theCtx, aProgram.getVTexCoordLoc());
        myTextVertBuf[aTextureIter]->unBindVertexAttrib(theCtx, aProgram.getVVertexLoc());
    }
    theCtx.core20fwd->glBindTexture(GL_TEXTURE_2D, 0);
}

void StGLTextArea::stglDraw(unsigned int theView) {
    if(!myIsInitialized || !isVisible()) {
        return;
    }

    StGLContext& aCtx = getContext();
    StGLVec4 aTextColor = myTextColor;
    aTextColor.a()   *= myOpacity;
    myShadowColor.a() = myBackColor.a() = myBorderColor.a() = myOpacity;
    formatText(aCtx);

    StRectI_t aTextRectPx = getRectPx();
    aTextRectPx.left()   += myMargins.left;
    aTextRectPx.right()  -= myMargins.right;
    aTextRectPx.top()    += myMargins.top;
    aTextRectPx.bottom() -= myMargins.bottom;
    StRectD_t aTextRectGl = getRoot()->getRectGl(getAbsolute(aTextRectPx));

    // size corrector for FTGL
    StRectD_t aZParams; getCamera()->getZParams(aZParams);
    const GLfloat aSizeOut = 2.0f * GLfloat(aZParams.top()) / GLfloat(getRoot()->getRootFullSizeY());

    aCtx.core20fwd->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    aCtx.core20fwd->glEnable(GL_BLEND);

    // draw borders
    if(myToShowBorder) {
        if(myBorderOVertBuf.getElemsCount() == 0) {
            recomputeBorder(aCtx);
        }

        StGLTextBorderProgram& aBorderProgram = myRoot->getTextBorderProgram();
        aBorderProgram.use(aCtx);
        aBorderProgram.setDisplacement(aCtx,
                                       StGLVec3(GLfloat(aTextRectGl.left()) + getRoot()->getScreenDispX() + myTextDX,
                                                GLfloat(aTextRectGl.top()), 0.0f),
                                       aSizeOut);

        aBorderProgram.setColor(aCtx, myBorderColor);
        myBorderOVertBuf.bindVertexAttrib(aCtx, aBorderProgram.getVVertexLoc());
        aCtx.core20fwd->glDrawArrays(GL_TRIANGLE_STRIP, 0, GLsizei(myBorderOVertBuf.getElemsCount()));
        myBorderOVertBuf.unBindVertexAttrib(aCtx, aBorderProgram.getVVertexLoc());

        aBorderProgram.setColor(aCtx, myBackColor);
        myBorderIVertBuf.bindVertexAttrib(aCtx, aBorderProgram.getVVertexLoc());
        aCtx.core20fwd->glDrawArrays(GL_TRIANGLE_STRIP, 0, GLsizei(myBorderIVertBuf.getElemsCount()));
        myBorderIVertBuf.unBindVertexAttrib(aCtx, aBorderProgram.getVVertexLoc());
        aBorderProgram.unuse(aCtx);
    }

    // draw text
    aCtx.core20fwd->glActiveTexture(GL_TEXTURE0); // our shader is bound to first texture unit
    StGLTextProgram& aTextProgram = myRoot->getTextProgram();
    aTextProgram.use(aCtx);
        aTextProgram.setDisplacement(aCtx,
                                     StGLVec3(GLfloat(aTextRectGl.left()) + getRoot()->getScreenDispX() + myTextDX,
                                              GLfloat(aTextRectGl.top()), 0.0f),
                                     aSizeOut);
        aTextProgram.setColor(aCtx, myToDrawShadow ? myShadowColor : aTextColor);
        drawText(aCtx);

        if(myToDrawShadow) {
            aTextRectPx.left() -= 1;
            aTextRectPx.top()  -= 1;
            aTextRectGl = getRoot()->getRectGl(getAbsolute(aTextRectPx));

            aTextProgram.setDisplacement(aCtx,
                                         StGLVec3(GLfloat(aTextRectGl.left()) + getRoot()->getScreenDispX() + myTextDX,
                                                  GLfloat(aTextRectGl.top()), 0.0f),
                                         aSizeOut);
            aTextProgram.setColor(aCtx, aTextColor);

            drawText(aCtx);
        }

    aTextProgram.unuse(aCtx);

    aCtx.core20fwd->glDisable(GL_BLEND);

    StGLWidget::stglDraw(theView);
}
