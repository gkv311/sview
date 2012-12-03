/**
 * Copyright © 2011 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#ifndef __StGLRadioButtonFloat32_h_
#define __StGLRadioButtonFloat32_h_

#include <StSettings/StFloat32Param.h>
#include <StGLWidgets/StGLRadioButton.h>

/**
 * This is radio button for float value.
 */
class ST_LOCAL StGLRadioButtonFloat32 : public StGLRadioButton {

        private:

    StHandle<StFloat32Param> myTrackValue; //!< handle to tracked value
    float                    myValueOn;    //!< value to turn radio button on

        public: //!< overriders

    /**
     * Main constructor.
     */
    StGLRadioButtonFloat32(StGLWidget* theParent,
                           const StHandle<StFloat32Param>& theTrackedValue,
                           const float theOnValue,
                           const int theLeft = 32, const int theTop = 32,
                           const StGLCorner theCorner = StGLCorner(ST_VCORNER_TOP, ST_HCORNER_LEFT));

    virtual ~StGLRadioButtonFloat32();

    virtual const StString& getClassName();

        public:

    /**
     * Return true if radio button is in active state.
     */
    virtual bool isActiveState() const;

    /**
     * Set this radio button on.
     */
    virtual void setValue();

};

#endif //__StGLRadioButtonFloat32_h_
