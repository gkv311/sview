/**
 * Copyright © 2011 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#ifndef __StGLRadioButton_h_
#define __StGLRadioButton_h_

#include <StGLWidgets/StGLShare.h>
#include <StGLWidgets/StGLWidget.h>
#include <StGL/StGLVertexBuffer.h>
#include <StSettings/StParam.h>

// forward declarations
class StGLMenuProgram;

/**
 * Widget represents one classical radio button.
 * Radio button initialized with handle to tracked property
 * and value associated to this item.
 * Thus multiple widgets can control one property without complex sync routines.
 */
class ST_LOCAL StGLRadioButton : public StGLWidget {

        private:

    StHandle<StInt32Param>     myTrackValue; //!< handle to tracked value
    StGLShare<StGLMenuProgram> myProgram;    //!< shared program
    StGLVertexBuffer           myVertBuf;    //!< vertices buffer
    int32_t                    myValueOn;    //!< value to turn radio button on

        private:

    /**
     * Auxiliary method.
     */
    void stglResize();

        public: //!< overriders

    /**
     * Main constructor.
     */
    StGLRadioButton(StGLWidget* theParent,
                    const StHandle<StInt32Param>& theTrackedValue,
                    const int32_t theOnValue,
                    const int theLeft = 32, const int theTop = 32,
                    const StGLCorner theCorner = StGLCorner(ST_VCORNER_TOP, ST_HCORNER_LEFT));
    virtual ~StGLRadioButton();
    virtual const StString& getClassName();
    virtual void stglResize(const StRectI_t& theWinRectPx);
    virtual bool stglInit();
    virtual void stglDraw(unsigned int view);
    virtual bool tryClick(const StPointD_t& cursorZo, const int& mouseBtn, bool& isItemClicked);
    virtual bool tryUnClick(const StPointD_t& cursorZo, const int& mouseBtn, bool& isItemUnclicked);

        public:

    /**
     * Return true if radio button is in active state.
     */
    virtual bool isActiveState() const;

    /**
     * Set this radio button on.
     */
    virtual void setValue();

        private: //!< callback Slots (private overriders)

    void doMouseUnclick(const int theBtnId);

};

#endif //__StGLRadioButton_h_
