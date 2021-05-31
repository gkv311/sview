/**
 * StGLWidgets, small C++ toolkit for writing GUI using OpenGL.
 * Copyright © 2011-2015 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#ifndef __StGLCheckbox_h_
#define __StGLCheckbox_h_

#include <StGLWidgets/StGLTextureButton.h>
#include <StSettings/StParam.h>

/**
 * Widget represents classical checkbox.
 * Initialized with handle to tracked boolean property
 * thus multiple widgets can show one property without complex sync routines.
 */
class StGLCheckbox : public StGLTextureButton {

        public: //! @name overriders

    /**
     * Main constructor.
     * @param theParent       parent widget
     * @param theTrackedValue tracked boolean value
     */
    ST_CPPEXPORT StGLCheckbox(StGLWidget* theParent,
                              const StHandle<StBoolParam>& theTrackedValue,
                              const int theLeft = 32, const int theTop = 32,
                              const StGLCorner theCorner = StGLCorner(ST_VCORNER_TOP, ST_HCORNER_LEFT));

    ST_CPPEXPORT virtual ~StGLCheckbox();

    ST_CPPEXPORT virtual void stglResize() ST_ATTR_OVERRIDE;
    ST_CPPEXPORT virtual bool stglInit() ST_ATTR_OVERRIDE;
    ST_CPPEXPORT virtual void stglDraw(unsigned int theView) ST_ATTR_OVERRIDE;

        public:

    /**
     * Switch this checkbox.
     */
    ST_CPPEXPORT void reverseValue();

        private: //! @name callback Slots (private overriders)

    ST_LOCAL void doMouseUnclick(const int theBtnId);

        private:

    StHandle<StBoolParam> myTrackValue; //!< handle to tracked value
    StGLVertexBuffer      myVertBuf;    //!< vertices buffer

};

#endif //__StGLCheckbox_h_
