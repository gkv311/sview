/**
 * Copyright © 2011-2012 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#ifndef __StGLCheckbox_h_
#define __StGLCheckbox_h_

#include <StGLWidgets/StGLShare.h>
#include <StGLWidgets/StGLWidget.h>
#include <StGL/StGLVertexBuffer.h>
#include <StSettings/StParam.h>

// forward declarations
class StGLMenuProgram;

/**
 * Widget represents classical checkbox.
 * Initialized with handle to tracked boolean property
 * thus multiple widgets can show one property without complex sync routines.
 */
class ST_LOCAL StGLCheckbox : public StGLWidget {

        public: //! @name overriders

    /**
     * Main constructor.
     * @param theParent (StGLWidget* ) - parent widget;
     * @param theTrackedValue (const StHandle<StBoolParam>& ) - tracked boolean value.
     */
    StGLCheckbox(StGLWidget* theParent,
                 const StHandle<StBoolParam>& theTrackedValue,
                 const int theLeft = 32, const int theTop = 32,
                 const StGLCorner theCorner = StGLCorner(ST_VCORNER_TOP, ST_HCORNER_LEFT));

    virtual ~StGLCheckbox();

    virtual const StString& getClassName();
    virtual void stglResize(const StRectI_t& theWinRectPx);
    virtual bool stglInit();
    virtual void stglDraw(unsigned int view);
    virtual bool tryClick(const StPointD_t& cursorZo, const int& mouseBtn, bool& isItemClicked);
    virtual bool tryUnClick(const StPointD_t& cursorZo, const int& mouseBtn, bool& isItemUnclicked);

        public:

    /**
     * Switch this checkbox.
     */
    void reverseValue();

        private: //! @name callback Slots (private overriders)

    void doMouseUnclick(const int theBtnId);

        private:

    /**
     * Auxiliary method.
     */
    void stglResize();

        private:

    StHandle<StBoolParam>      myTrackValue; //!< handle to tracked value
    StGLShare<StGLMenuProgram> myProgram;    //!< shared program
    StGLVertexBuffer           myVertBuf;    //!< vertices buffer

};

#endif //__StGLCheckbox_h_
