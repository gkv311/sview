/**
 * StGLWidgets, small C++ toolkit for writing GUI using OpenGL.
 * Copyright © 2009-2016 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#ifndef __StGLMessageBox_h_
#define __StGLMessageBox_h_

#include <StGLWidgets/StGLTextArea.h>

class StGLButton;
class StGLScrollArea;

/**
 * Simple widget to show message text.
 */
class StGLMessageBox : public StGLWidget {

        public:

    /**
     * Main constructor creating message box of size 384 x 200 (clipped by root widget size).
     */
    ST_CPPEXPORT StGLMessageBox(StGLWidget*     theParent,
                                const StString& theTitle,
                                const StString& theText);

    /**
     * Main constructor creating message box of specified size (clipped by root widget size).
     */
    ST_CPPEXPORT StGLMessageBox(StGLWidget*     theParent,
                                const StString& theTitle,
                                const StString& theText,
                                const int       theWidth,
                                const int       theHeight);

    /**
     * Destructor.
     */
    ST_CPPEXPORT virtual ~StGLMessageBox();
    ST_CPPEXPORT virtual bool stglInit() ST_ATTR_OVERRIDE;
    ST_CPPEXPORT virtual void stglResize() ST_ATTR_OVERRIDE;
    ST_CPPEXPORT virtual void stglDraw(unsigned int theView) ST_ATTR_OVERRIDE;
    ST_CPPEXPORT virtual bool tryClick  (const StClickEvent& theEvent, bool& theIsItemClicked)   ST_ATTR_OVERRIDE;
    ST_CPPEXPORT virtual bool tryUnClick(const StClickEvent& theEvent, bool& theIsItemUnclicked) ST_ATTR_OVERRIDE;
    ST_CPPEXPORT virtual bool doScroll(const StScrollEvent& theEvent) ST_ATTR_OVERRIDE;

        public:

    /**
     * @return child widget which holds content of this message box
     */
    ST_LOCAL StGLScrollArea* getContent() const {
        return myContent;
    }

    /**
     * Return true if message box should be destroyed on first mouse click.
     */
    ST_LOCAL bool isContextual() const {
        return myIsContextual;
    }

    /**
     * Setup flag to automatically destroy the message box on first mouse click.
     */
    ST_LOCAL void setContextual(const bool theIsContextual) {
        myIsContextual = theIsContextual;
    }

    /**
     * Set message box title.
     */
    ST_CPPEXPORT void setTitle(const StString& theTitle);

    /**
     * Set content to the plain text (previous content will be discarded).
     */
    ST_CPPEXPORT void setText(const StString& theText);

    /**
     * Append button to this message box.
     */
    ST_CPPEXPORT StGLButton* addButton(const StString& theTitle,
                                       const bool      theIsDefault = false,
                                       const int       theWidth = 0);

    ST_CPPEXPORT virtual bool doKeyDown(const StKeyEvent& theEvent) ST_ATTR_OVERRIDE;

    ST_LOCAL int getMarginLeft()   const { return myMarginLeft; }
    ST_LOCAL int getMarginRight()  const { return myMarginRight; }
    ST_LOCAL int getMarginTop()    const { return myMarginTop; }
    ST_LOCAL int getMarginBottom() const { return myMarginBottom; }

        protected:

    /**
     * Protected empty constructor (does not call create()).
     */
    ST_CPPEXPORT StGLMessageBox(StGLWidget* theParent);

    /**
     * Initializes the layout of the widget.
     * Should be called once, in constructor.
     */
    ST_CPPEXPORT void create(const StString& theTitle,
                             const StString& theText,
                             const int  theWidth,
                             const int  theHeight,
                             const bool theHasButtons = true);

        private:   //! @name callback Slots (private overriders)

    ST_LOCAL void doMouseUnclick(const int theBtnId);

    /**
     * Move focus to the next button.
     * @param theDir determine direction (1 forward, -1 backward)
     * @return true if next button is available
     */
    ST_LOCAL bool doNextButton(const int theDir);

        public:    //! @name Signals

    struct {
        /**
         * @param theUserData (const size_t ) - user predefined data.
         */
        StSignal<void (const size_t )> onClickLeft;
        StSignal<void (const size_t )> onClickRight;
    } signals;

        public:    //! @name callback Slots

    ST_CPPEXPORT void doKillSelf(const size_t );

        protected:   //! @name private fields

    StGLScrollArea*   myContent;      //!< content widget
    StGLTextArea*     myTitle;        //!< window title
    StGLWidget*       myBtnPanel;     //!< panel for buttons
    StGLButton*       myDefaultBtn;   //!< default button to redirect Enter
    StGLVertexBuffer  myVertexBuf;    //!< vertices VBO
    int               myButtonsNb;    //!< number of buttons added to this message box


    int               myMarginLeft;   //!< margins to content
    int               myMarginRight;
    int               myMarginTop;
    int               myMarginBottom;
    int               myMinSizeY;     //!< minimal height of the message box
    bool              myToAdjustY;    //!< flag to automatically adjust height to fit content / window
    bool              myIsContextual; //!< flag to automatically destroy the message box on first mouse click

};

#endif // __StGLMessageBox_h_
