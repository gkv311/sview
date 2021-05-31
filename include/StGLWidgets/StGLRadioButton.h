/**
 * StGLWidgets, small C++ toolkit for writing GUI using OpenGL.
 * Copyright © 2011-2015 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#ifndef __StGLRadioButton_h_
#define __StGLRadioButton_h_

#include <StGLWidgets/StGLTextureButton.h>
#include <StSettings/StParam.h>

/**
 * Widget represents one classical radio button.
 * Radio button initialized with handle to tracked property
 * and value associated to this item.
 * Thus multiple widgets can control one property without complex sync routines.
 */
class StGLRadioButton : public StGLTextureButton {

        public: //!< overriders

    /**
     * Main constructor.
     */
    ST_CPPEXPORT StGLRadioButton(StGLWidget* theParent,
                                 const StHandle<StInt32Param>& theTrackedValue,
                                 const int32_t theOnValue,
                                 const int theLeft = 32, const int theTop = 32,
                                 const StGLCorner theCorner = StGLCorner(ST_VCORNER_TOP, ST_HCORNER_LEFT));
    ST_CPPEXPORT virtual ~StGLRadioButton();
    ST_CPPEXPORT virtual void stglResize() ST_ATTR_OVERRIDE;
    ST_CPPEXPORT virtual bool stglInit() ST_ATTR_OVERRIDE;
    ST_CPPEXPORT virtual void stglDraw(unsigned int theView) ST_ATTR_OVERRIDE;

        public:

    /**
     * Return true if radio button is in active state.
     */
    ST_CPPEXPORT virtual bool isActiveState() const;

    /**
     * Set this radio button on.
     */
    ST_CPPEXPORT virtual void setValue();

        private: //!< callback Slots (private overriders)

    ST_LOCAL void doMouseUnclick(const int theBtnId);

        private:

    StHandle<StInt32Param>     myTrackValue; //!< handle to tracked value
    StGLVertexBuffer           myVertBuf;    //!< vertices buffer
    int32_t                    myValueOn;    //!< value to turn radio button on

};

#endif //__StGLRadioButton_h_
