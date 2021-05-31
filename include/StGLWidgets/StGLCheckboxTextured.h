/**
 * StGLWidgets, small C++ toolkit for writing GUI using OpenGL.
 * Copyright © 2011-2015 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
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
    ST_CPPEXPORT virtual void stglUpdate(const StPointD_t& theCursorZo,
                                         bool theIsPreciseInput) ST_ATTR_OVERRIDE;

    /**
     * Return tracked value.
     */
    ST_LOCAL StHandle<StBoolParam>& getTrackedValue() {
        return myTrackValue;
    }

    /**
     * Return opacity scale for FALSE value, 0.5f by default.
     */
    ST_LOCAL float getFalseOpacity() const {
        return myFalseOpacity;
    }

    /**
     * Setup opacity scale for FALSE value.
     */
    ST_LOCAL void setFalseOpacity(const float theValue) {
        myFalseOpacity = theValue;
    }

    /**
     * Return opacity scale for TRUE value, 1.0f by default.
     */
    ST_LOCAL float getTrueOpacity() const {
        return myTrueOpacity;
    }

    /**
     * Setup opacity scale for TRUE value.
     */
    ST_LOCAL void setTrueOpacity(const float theValue) {
        myTrueOpacity = theValue;
    }

        private: //!< callback Slots (private overriders)

    ST_LOCAL void doClick(const size_t );

        private:

    StHandle<StBoolParam> myTrackValue;   //!< handle to tracked value
    float                 myFalseOpacity; //!< opacity scale for button in FALSE state
    float                 myTrueOpacity;  //!< opacity scale for button in TRUE  state

};

#endif //__StGLCheckboxTextured_h_
