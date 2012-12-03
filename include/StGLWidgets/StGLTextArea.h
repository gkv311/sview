/**
 * Copyright © 2009-2012 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#ifndef __StGLTextArea_h_
#define __StGLTextArea_h_

#include <StTemplates/StArrayList.h>

#include <StGL/StGLVertexBuffer.h>
#include <StGL/StGLVec.h>
#include <StGL/StGLTextFormatter.h>
#include <StGLWidgets/StGLShare.h>
#include <StGLWidgets/StGLWidget.h>

/**
 * Class implements basic text rendering widget.
 */
class ST_LOCAL StGLTextArea : public StGLWidget {

        public:

    typedef enum {
        SIZE_NORMAL,
        SIZE_BIG,
        SIZE_SMALL,
    } FontSize;

        public:

    StGLTextArea(StGLWidget* theParent,
                 const int theLeft = 32, const int theTop = 32,
                 const StGLCorner theCorner = StGLCorner(ST_VCORNER_TOP, ST_HCORNER_LEFT),
                 const int theWidth = 256, const int theHeight = 32, bool theToCutView = false,
                 const FontSize theSize = StGLTextArea::SIZE_NORMAL);

    virtual ~StGLTextArea();

    virtual const StString& getClassName();

    /**
     * @return currently drawn text.
     */
    const StString& getText() const;

    /**
     * @param theText - new text to draw
     */
    void setText(const StString& theText);

    /**
     * Setup alignment style.
     * @param theAlignX - horizontal alignment
     * @param theAlignY - vertical   alignment
     */
    void setupAlignment(const StGLTextFormatter::StAlignX theAlignX,
                        const StGLTextFormatter::StAlignY theAlignY) {
        myFormatter.setupAlignment(theAlignX, theAlignY);
    }

    /**
     * @param theToShow - to show border with background or not
     */
    void setBorder(const bool theToShow) {
        myToShowBorder = theToShow;
    }

    /**
     * @param theColor- border color
     */
    void setBorderColor(const StGLVec3& theColor) {
        myBorderColor.r() = theColor.r();
        myBorderColor.g() = theColor.g();
        myBorderColor.b() = theColor.b();
    }

    /**
     * @param theColor- background color
     */
    void setBackColor(const StGLVec3& theColor) {
        myBackColor.r() = theColor.r();
        myBackColor.g() = theColor.g();
        myBackColor.b() = theColor.b();
    }

    /**
     * @param theColor- text color
     */
    void setTextColor(const StGLVec3& theColor) {
        myTextColor.r() = theColor.r();
        myTextColor.g() = theColor.g();
        myTextColor.b() = theColor.b();
    }

    /**
     * @param theWidth - text width restriction to force newline (-1 means no restriction)
     */
    void setTextWidth(const int theWidth);

    const int getFontSize() const {
        switch(mySize) {
            case SIZE_SMALL:
                return 12;
            case SIZE_BIG:
                return 28;
            case SIZE_NORMAL:
            default:
                return 16;
        }
    }

    virtual bool stglInit();
    virtual void stglDraw(unsigned int theView);

        private:

    void drawText(StGLContext& theCtx);

    void recomputeBorder(StGLContext& theCtx);

        private:

    StArrayList<GLuint>                       myTexturesList;
    StArrayList< StHandle<StGLVertexBuffer> > myTextVertBuf;
    StArrayList< StHandle<StGLVertexBuffer> > myTextTCrdBuf;

    StGLVertexBuffer     myBorderIVertBuf;
    StGLVertexBuffer     myBorderOVertBuf;

    class StTextProgram;
    class StBorderProgram;
    StGLShare<StTextProgram>   myTextProgram;
    StGLShare<StBorderProgram> myBorderProgram;

        protected:

    StGLShare<StGLFont>  myFont;          //!< used font
    StGLTextFormatter    myFormatter;     //!< text formatter
    StString             myText;          //!< text
    FontSize             mySize;          //!< font size
    StGLVec4             myTextColor;     //!< text   color
    StGLVec4             myShadowColor;   //!< shadow color
    StGLVec4             myBackColor;     //!< text area color
    StGLVec4             myBorderColor;   //!< text area border color

    int                  myMarginLeft;
    int                  myMarginRight;
    int                  myMarginTop;
    int                  myMarginBottom;
    StGLRect             myTextBndBox;    //!< text boundary box

    GLfloat              myTextWidth;     //!< text width limit

    bool                 myToRecompute;   //!< flag indicates that text VBOs should be recomputed
    bool                 myToCutView;
    bool                 myToShowBorder;  //!< to show text area border
    bool                 myToDrawShadow;  //!< to render text shadow
    bool                 myIsInitialized;

};

#endif //__StGLTextArea_h_
