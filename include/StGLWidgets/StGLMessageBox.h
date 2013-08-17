/**
 * Copyright © 2009-2013 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#ifndef __StGLMessageBox_h_
#define __StGLMessageBox_h_

#include <StGLWidgets/StGLTextArea.h>
#include <StGLWidgets/StGLMenuProgram.h>

class StGLButton;
class StGLScrollArea;

/**
 * Simple widget to show message text.
 */
class StGLMessageBox : public StGLWidget {

        public:

    ST_CPPEXPORT StGLMessageBox(StGLWidget*     theParent,
                                const StString& theText);
    ST_CPPEXPORT StGLMessageBox(StGLWidget*     theParent,
                                const StString& theText,
                                const int       theWidth,
                                const int       theHeight);
    ST_CPPEXPORT virtual ~StGLMessageBox();
    ST_CPPEXPORT virtual bool stglInit();
    ST_CPPEXPORT virtual void stglResize();
    ST_CPPEXPORT virtual void stglDraw(unsigned int theView);
    ST_CPPEXPORT virtual void setVisibility(bool isVisible, bool isForce);
    ST_CPPEXPORT virtual bool tryClick(const StPointD_t& theCursorZo, const int& theMouseBtn, bool& isItemClicked);
    ST_CPPEXPORT virtual bool tryUnClick(const StPointD_t& theCursorZo, const int& theMouseBtn, bool& isItemUnclicked);

        public:

    /**
     * @return child widget which holds content of this message box
     */
    ST_LOCAL inline StGLScrollArea* getContent() const {
        return myContent;
    }

    /**
     * Set content to the plain text.
     */
    ST_CPPEXPORT void setText(const StString& theText);

    /**
     * Append button to this message box.
     */
    ST_CPPEXPORT StGLButton* addButton(const StString& theTitle,
                                       const bool      theIsDefault = false,
                                       const int       theWidth = 0);

    ST_CPPEXPORT virtual bool doKeyDown(const StKeyEvent& theEvent);

        private:   //! @name callback Slots (private overriders)

    ST_LOCAL void doMouseUnclick(const int theBtnId);

    ST_LOCAL void create(const StString& theText,
                         const int       theWidth,
                         const int       theHeight);

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

        private:   //! @name private fields

    StGLScrollArea*   myContent;    //!< content widget
    StGLWidget*       myBtnPanel;   //!< panel for buttons
    StGLButton*       myDefaultBtn; //!< default button to redirect Enter
    StGLMenuProgram   myProgram;    //!< GLSL program
    StGLVertexBuffer  myVertexBuf;  //!< vertices VBO
    int               myButtonsNb;  //!< number of buttons added to this message box

};

#endif // __StGLMessageBox_h_
