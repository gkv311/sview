/**
 * Copyright © 2011-2015 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#ifndef __StGLCheckboxTextured_h_
#define __StGLCheckboxTextured_h_

#include <StGLWidgets/StGLTextureButton.h>
#include <StSettings/StParam.h>

/**
 * The textured button with two faces bound to boolean parameter.
 */
class StGLCheckboxTextured : public StGLTextureButton {

        public:

    /**
     * Main constructor.
     */
    ST_CPPEXPORT StGLCheckboxTextured(StGLWidget* theParent,
                                      const StHandle<StBoolParam>& theTrackedValue,
                                      const StString& theTextureOffPath,
                                      const StString& theTextureOnPath,
                                      const int theLeft, const int theTop,
                                      const StGLCorner theCorner = StGLCorner(ST_VCORNER_TOP, ST_HCORNER_LEFT));

    /**
     * Destructor.
     */
    ST_CPPEXPORT virtual ~StGLCheckboxTextured();
    ST_CPPEXPORT virtual void stglUpdate(const StPointD_t& theCursorZo);

    /**
     * Return opacity scale for OFF value, 0.5f by default.
     */
    ST_LOCAL float getOffOpacity() const {
        return myOffOpacity;
    }

    /**
     * Setup opacity scale for OFF value.
     */
    ST_LOCAL void setOffOpacity(const float theValue) {
        myOffOpacity = theValue;
    }

        private: //!< callback Slots (private overriders)

    ST_LOCAL void doClick(const size_t );

        private:

    StHandle<StBoolParam> myTrackValue; //!< handle to tracked value
    float                 myOffOpacity; //!< opacity scale for button in OFF state

};

#endif //__StGLCheckboxTextured_h_
