/**
 * Copyright © 2013-2015 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#ifndef __StGLRangeFieldFloat32_h_
#define __StGLRangeFieldFloat32_h_

#include <StSettings/StFloat32Param.h>
#include <StGLWidgets/StGLWidget.h>

class StGLTextArea;

/**
 * This is radio button for float value.
 */
class StGLRangeFieldFloat32 : public StGLWidget {

        public:

    enum FieldColor {
        FieldColor_Default,
        FieldColor_Positive,
        FieldColor_Negative,
        FieldColorNb,
    };

        public: //! @name overriders

    /**
     * Main constructor.
     */
    ST_CPPEXPORT StGLRangeFieldFloat32(StGLWidget* theParent,
                                       const StHandle<StFloat32Param>& theTrackedValue,
                                       const int theLeft = 0, const int theTop = 0,
                                       const StGLCorner theCorner = StGLCorner(ST_VCORNER_TOP, ST_HCORNER_LEFT));

    ST_CPPEXPORT virtual ~StGLRangeFieldFloat32();
    ST_CPPEXPORT virtual bool stglInit();

        public:

    ST_LOCAL inline void setColor(const FieldColor theType,
                                  const StGLVec3&  theColor) {
        myColors[theType] = theColor;
    }

    ST_LOCAL inline void setFormat(const StCString& theFormat) {
        myFormat = theFormat;
    }

    ST_CPPEXPORT void doResetValue(const size_t );
    ST_CPPEXPORT void doDecrement(const size_t );
    ST_CPPEXPORT void doIncrement(const size_t );

        private:

    ST_LOCAL void onValueChange(const float theValue);
    ST_LOCAL void doMouseUnclick(const int theBtnId);

        private:

    StHandle<StFloat32Param> myTrackValue; //!< handle to tracked value
    StGLVec3                 myColors[FieldColorNb];
    StGLTextArea*            myValueText;  //!< text area
    StString                 myFormat;     //!< value format

};

#endif // __StGLRangeFieldFloat32_h_
