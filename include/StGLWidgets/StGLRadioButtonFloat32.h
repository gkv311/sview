/**
 * StGLWidgets, small C++ toolkit for writing GUI using OpenGL.
 * Copyright © 2011-2014 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#ifndef __StGLRadioButtonFloat32_h_
#define __StGLRadioButtonFloat32_h_

#include <StSettings/StFloat32Param.h>
#include <StGLWidgets/StGLRadioButton.h>

/**
 * This is radio button for float value.
 */
class StGLRadioButtonFloat32 : public StGLRadioButton {

        public: //!< overriders

    /**
     * Main constructor.
     */
    ST_CPPEXPORT StGLRadioButtonFloat32(StGLWidget* theParent,
                                        const StHandle<StFloat32Param>& theTrackedValue,
                                        const float theOnValue,
                                        const int theLeft = 32, const int theTop = 32,
                                        const StGLCorner theCorner = StGLCorner(ST_VCORNER_TOP, ST_HCORNER_LEFT));

    ST_CPPEXPORT virtual ~StGLRadioButtonFloat32();

        public:

    /**
     * Return true if radio button is in active state.
     */
    ST_CPPEXPORT virtual bool isActiveState() const ST_ATTR_OVERRIDE;

    /**
     * Set this radio button on.
     */
    ST_CPPEXPORT virtual void setValue() ST_ATTR_OVERRIDE;

        private:

    StHandle<StFloat32Param> myTrackValue; //!< handle to tracked value
    float                    myValueOn;    //!< value to turn radio button on

};

#endif //__StGLRadioButtonFloat32_h_
