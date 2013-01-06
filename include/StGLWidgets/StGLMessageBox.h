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

/**
 * Simple widget to show message text.
 */
class ST_LOCAL StGLMessageBox : public StGLWidget {

        public:

    StGLMessageBox(StGLWidget*     theParent,
                   const StString& theText,
                   const int       theWidth  = 384,
                   const int       theHeight = 128);
    virtual ~StGLMessageBox();
    virtual bool stglInit();
    virtual void stglResize();
    virtual void stglResize(const StRectI_t& theWinRectPx);
    virtual void stglDraw(unsigned int theView);
    virtual void setVisibility(bool isVisible, bool isForce);
    virtual bool tryClick(const StPointD_t& theCursorZo, const int& theMouseBtn, bool& isItemClicked);
    virtual bool tryUnClick(const StPointD_t& theCursorZo, const int& theMouseBtn, bool& isItemUnclicked);

        private:   //! @name callback Slots (private overriders)

    void doMouseUnclick(const int theBtnId);

        public:    //! @name Signals

    struct {
        /**
         * @param theUserData (const size_t ) - user predefined data.
         */
        StSignal<void (const size_t )> onClickLeft;
        StSignal<void (const size_t )> onClickRight;
    } signals;

        public:    //! @name callback Slots

    void doKillSelf(const size_t );

        private:   //! @name private fields

    StGLTextArea*     myTextArea;  //!< text widget
    StGLMenuProgram   myProgram;   //!< GLSL program
    StGLVertexBuffer  myVertexBuf; //!< vertices VBO

};

#endif //__StGLMessageBox_h_
