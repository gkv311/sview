/**
 * Copyright © 2012-2017 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#ifndef __StGLTextFormatter_h_
#define __StGLTextFormatter_h_

#include <StGL/StGLFont.h>

#include <vector>

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

    enum Parser {
        Parser_PlainText, //!< ignore any tags - prints text as is
        Parser_LiteHTML,  //!< process minimal set of HTML tags but print unknown tags as is (save for unknown source)
    };

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
     * @return default font style
     */
    ST_LOCAL StFTFont::Style getDefaultStyle() const {
        return myDefStyle;
    }

    /**
     * Setup default font style.
     */
    ST_LOCAL void setDefaultStyle(const StFTFont::Style theStyle) {
        myDefStyle = theStyle;
    }

    /**
     * @return active parser
     */
    ST_LOCAL StGLTextFormatter::Parser getParser() const {
        return myParser;
    }

    /**
     * Setup parser.
     */
    ST_LOCAL void setupParser(const StGLTextFormatter::Parser theParser) {
        myParser = theParser;
    }

    /**
     * Reset current progress.
     */
    ST_CPPEXPORT void reset();

    /**
     * Render specified text to inner buffer.
     */
    ST_CPPEXPORT void append(StGLContext&    theCtx,
                             const StString& theString,
                             StGLFont&       theFont);

    /**
     * Render specified text to inner buffer.
     */
    ST_CPPEXPORT void append(StGLContext&          theCtx,
                             const StCString&      theString,
                             const StFTFont::Style theStyle,
                             StGLFont&             theFont);

    /**
     * Process minimal set of formatting tags from HTML.
     */
    ST_CPPEXPORT void appendHTML(StGLContext&    theCtx,
                                 const StString& theString,
                                 StGLFont&       theFont);

    /**
     * Perform formatting on the buffered text.
     * Should not be called more than once after initialization!
     */
    ST_CPPEXPORT void format(const GLfloat theWidth,
                             const GLfloat theHeight);

    /**
     * Retrieve formatting results.
     */
    ST_CPPEXPORT void getResult(std::vector<GLuint>&                               theTextures,
                                std::vector< StHandle < std::vector<StGLVec2> > >& theVertsPerTexture,
                                std::vector< StHandle < std::vector<StGLVec2> > >& theTCrdsPerTexture) const;

    /**
     * Retrieve formatting results.
     */
    ST_CPPEXPORT void getResult(StGLContext&                                theCtx,
                                std::vector<GLuint>&                        theTextures,
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
     * @return maximum width of formatted text (<= getResultWidth())
     */
    inline GLfloat getMaxLineWidth() const {
        return myTextWidth;
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

    /**
     * Flip left to right order.
     */
    ST_CPPEXPORT void flipLeftRight(size_t theCharFrom, size_t theCharTo);

        protected: //! @name configuration

    StAlignX              myAlignX;        //!< horizontal alignment style
    StAlignY              myAlignY;        //!< vertical   alignment style
    Parser                myParser;        //!< parser configuration
    StFTFont::Style       myDefStyle;      //!< default font style

        protected: //! @name input data

    StString              myString;        //!< currently rendered text
    StGLVec2              myPen;           //!< current pen position
    std::vector<StGLTile> myRects;         //!< glyphs rectangles
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
    GLfloat               myTextWidth;     //!< maximum text width (<= myAlignWidth)
    GLfloat               myLineLeft;      //!< left x position of first glyph on line before formatting applied
    GLfloat               myBndTop;
    StGLVec2              myMoveVec;       //!< local variable

};

#endif // __StGLTextFormatter_h_
