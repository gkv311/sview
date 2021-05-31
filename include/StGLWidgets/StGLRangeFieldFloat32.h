/**
 * StGLWidgets, small C++ toolkit for writing GUI using OpenGL.
 * Copyright © 2013-2017 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#ifndef __StGLRangeFieldFloat32_h_
#define __StGLRangeFieldFloat32_h_

#include <StSettings/StFloat32Param.h>
#include <StGLWidgets/StGLSeekBar.h>

class StGLTextArea;

/**
 * This is radio button for float value.
 */
class StGLRangeFieldFloat32 : public StGLSeekBar {

        public:

    enum RangeStyle {
        RangeStyle_PlusMinus,
        RangeStyle_Seekbar,
    };

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
                                       int theLeft = 0, int theTop = 0,
                                       StGLCorner theCorner = StGLCorner(ST_VCORNER_TOP, ST_HCORNER_LEFT),
                                       RangeStyle theStyle = RangeStyle_PlusMinus,
                                       int theMargin = 0);

    ST_CPPEXPORT virtual ~StGLRangeFieldFloat32();
    ST_CPPEXPORT virtual bool stglInit() ST_ATTR_OVERRIDE;
    ST_CPPEXPORT virtual void stglResize() ST_ATTR_OVERRIDE;
    ST_CPPEXPORT virtual void stglDraw(unsigned int theView) ST_ATTR_OVERRIDE;
    ST_CPPEXPORT virtual bool doScroll(const StScrollEvent& theEvent) ST_ATTR_OVERRIDE;

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

    ST_CPPEXPORT void doSeekClick(const int    theMouseBtn,
                                  const double theValue);

        private:

    ST_LOCAL void onValueChange(const float theValue);

        private:

    StHandle<StFloat32Param> myTrackValue; //!< handle to tracked value
    StGLVec3                 myColors[FieldColorNb];
    StGLTextArea*            myValueText;  //!< text area
    StString                 myFormat;     //!< value format
    RangeStyle               myRangeStyle;

};

#endif // __StGLRangeFieldFloat32_h_
