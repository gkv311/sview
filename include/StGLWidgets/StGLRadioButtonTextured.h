/**
 * Copyright © 2011 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#ifndef __StGLRadioButtonTextured_h_
#define __StGLRadioButtonTextured_h_

#include <StGLWidgets/StGLTextureButton.h>
#include <StSettings/StParam.h>

class ST_LOCAL StGLRadioButtonTextured : public StGLTextureButton {

        private:

    StHandle<StInt32Param> myTrackValue; //!< handle to tracked value
    int32_t                   myValueOn; //!< value to turn radio button on

        public:

    /**
     * Main constructor.
     */
    StGLRadioButtonTextured(StGLWidget* theParent,
                            const StHandle<StInt32Param>& theTrackedValue, const int32_t theValueOn,
                            const StString& theTexturePath,
                            const int theLeft, const int theTop,
                            const StGLCorner theCorner = StGLCorner(ST_VCORNER_TOP, ST_HCORNER_LEFT));

    /**
     * Destructor.
     */
    virtual ~StGLRadioButtonTextured();

    virtual const StString& getClassName();

    int32_t getValueOn() const {
        return myValueOn;
    }

        private:   //!< callback Slots (private overriders)

    void doClick(const size_t );

};

#endif //__StGLTextureRadioButton_h_
