/**
 * StGLWidgets, small C++ toolkit for writing GUI using OpenGL.
 * Copyright © 2011-2014 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#ifndef __StGLRadioButtonTextured_h_
#define __StGLRadioButtonTextured_h_

#include <StGLWidgets/StGLTextureButton.h>
#include <StSettings/StParam.h>

class StGLRadioButtonTextured : public StGLTextureButton {

        public:

    /**
     * Main constructor.
     */
    ST_CPPEXPORT StGLRadioButtonTextured(StGLWidget* theParent,
                                         const StHandle<StInt32Param>& theTrackedValue, const int32_t theValueOn,
                                         const StString& theTexturePath,
                                         const int theLeft, const int theTop,
                                         const StGLCorner theCorner = StGLCorner(ST_VCORNER_TOP, ST_HCORNER_LEFT));

    /**
     * Destructor.
     */
    ST_CPPEXPORT virtual ~StGLRadioButtonTextured();

    inline int32_t getValueOn() const {
        return myValueOn;
    }

        private:   //!< callback Slots (private overriders)

    ST_LOCAL void doClick(const size_t );

        private:

    StHandle<StInt32Param> myTrackValue; //!< handle to tracked value
    int32_t                   myValueOn; //!< value to turn radio button on

};

#endif //__StGLTextureRadioButton_h_
