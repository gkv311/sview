/**
 * Copyright © 2012-2013 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#ifndef __StGLTextFormatter_h_
#define __StGLTextFormatter_h_

#include <StGL/StGLFont.h>

class StGLVertexBuffer;

template<> inline void StArray< StHandle <StArrayList <StGLVec2> > >::sort() {}
template<> inline void StArray< StHandle <StGLVertexBuffer> >::sort() {}

/**
 * This class intended to prepare formatted text.
 */
class StGLTextFormatter {

        public:

    /**
     * Horizontal alignment styles.
     */
    typedef enum {
        ST_ALIGN_X_LEFT,
        ST_ALIGN_X_CENTER,
        ST_ALIGN_X_RIGHT,
        ST_ALIGN_X_JUSTIFY,
    } StAlignX;

    /**
     * Vertical alignment styles.
     */
    typedef enum {
        ST_ALIGN_Y_TOP,
        ST_ALIGN_Y_CENTER,
        ST_ALIGN_Y_BOTTOM,
    } StAlignY;

        public:

    /**
     * Default constructor.
     */
    ST_CPPEXPORT StGLTextFormatter();

    /**
     * Setup alignment style.
     */
    ST_CPPEXPORT void setupAlignment(const StGLTextFormatter::StAlignX theAlignX,
                                     const StGLTextFormatter::StAlignY theAlignY);

    /**
     * Reset current progress.
     */
    ST_CPPEXPORT void reset();

    /**
     * Render specified text to inner buffer.
     */
    ST_CPPEXPORT void append(StGLContext&    theCtx,
                             const StString& theString,
                             StGLFontEntry&  theFont);

    /**
     * Perform formatting on the buffered text.
     * Should not be called more than once after initialization!
     */
    ST_CPPEXPORT void format(const GLfloat theWidth,
                             const GLfloat theHeight);

    /**
     * Retrieve formatting results.
     */
    ST_CPPEXPORT void getResult(StArrayList<GLuint>&                               theTextures,
                                StArrayList< StHandle <StArrayList <StGLVec2> > >& theVertsPerTexture,
                                StArrayList< StHandle <StArrayList <StGLVec2> > >& theTCrdsPerTexture) const;

    /**
     * Retrieve formatting results.
     */
    ST_CPPEXPORT void getResult(StGLContext&                                theCtx,
                                StArrayList<GLuint>&                        theTextures,
                                StArrayList< StHandle <StGLVertexBuffer> >& theVertsPerTexture,
                                StArrayList< StHandle <StGLVertexBuffer> >& theTCrdsPerTexture) const;

    /**
     * @return width of formatted text.
     */
    inline GLfloat getResultWidth() const {
        return myAlignWidth;
    }

    /**
     * @return height of formatted text.
     */
    inline GLfloat getResultHeight() const {
        return myLineSpacing * GLfloat(myLinesNb);
    }

    /**
     * @param bounding box.
     */
    inline void getBndBox(StGLRect& theBndBox) const {
        theBndBox.left()   = 0.0f;
        theBndBox.right()  = theBndBox.left() + myAlignWidth;
        theBndBox.top()    = myBndTop;
        theBndBox.bottom() = theBndBox.top() - myLineSpacing * GLfloat(myLinesNb);
    }

        protected: //! @name class auxiliary methods

    /**
     * Move glyphs on the current line to correct position.
     */
    ST_CPPEXPORT void newLine(const size_t theLastRect);

        protected: //! @name configuration

    StAlignX              myAlignX;        //!< horizontal alignment style
    StAlignY              myAlignY;        //!< vertical   alignment style

        protected: //! @name input data

    StString              myString;        //!< currently rendered text
    StGLVec2              myPen;           //!< current pen position
    StArrayList<StGLTile> myRects;         //!< glyphs rectangles
    size_t                myRectsNb;       //!< rectangles number
    GLfloat               myLineSpacing;   //!< line spacing (computed as maximum of all fonts involved in text formatting)
    GLfloat               myAscender;      //!<
    bool                  myIsFormatted;   //!< formatting state

        protected: //! @name temporary variables for formatting routines

    size_t                myLinesNb;       //!< overall (new)lines number (including splitting by width limit)
    size_t                myRectLineStart; //!< id of first rectangle on the current line
    size_t                myRectWordStart; //!< id of first rectangle in the current word
    GLfloat               myPenCurrLine;   //!< current baseline position
    GLfloat               myAlignWidth;    //!< line width used for horizontal alignment
    GLfloat               myLineLeft;      //!< left x position of first glyph on line before formatting applied
    GLfloat               myBndTop;
    StGLVec2              myMoveVec;       //!< local variable

};

#endif // __StGLTextFormatter_h_
