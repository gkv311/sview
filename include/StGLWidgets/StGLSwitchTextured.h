/**
 * StGLWidgets, small C++ toolkit for writing GUI using OpenGL.
 * Copyright © 2011-2014 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#ifndef __StGLSwitchTextured_h_
#define __StGLSwitchTextured_h_

#include <StGLWidgets/StGLWidget.h>
#include <StSettings/StParam.h>

/**
 * This class represents a switch between values shown as image.
 * It behaves as clickable iterator - each click switch to the next value in cycle.
 */
class StGLSwitchTextured : public StGLWidget {

        public:

    /**
     * Main constructor.
     */
    ST_CPPEXPORT StGLSwitchTextured(StGLWidget* theParent,
                                    const StHandle<StInt32Param>& theTrackedValue,
                                    const int theLeft, const int theTop,
                                    const StGLCorner theCorner = StGLCorner(ST_VCORNER_TOP, ST_HCORNER_LEFT));

    /**
     * Destructor.
     */
    ST_CPPEXPORT virtual ~StGLSwitchTextured();

    ST_CPPEXPORT virtual bool stglInit() ST_ATTR_OVERRIDE;

    /**
     * Overrider that shows only active value in the switch.
     */
    ST_CPPEXPORT virtual void stglDraw(unsigned int theView) ST_ATTR_OVERRIDE;

    /**
     * Overrider that blocks children's clicking functionality and switch the values in cycle.
     */
    ST_CPPEXPORT virtual bool tryClick  (const StClickEvent& theEvent, bool& theIsItemClicked)   ST_ATTR_OVERRIDE;
    ST_CPPEXPORT virtual bool tryUnClick(const StClickEvent& theEvent, bool& theIsItemUnclicked) ST_ATTR_OVERRIDE;

    /**
     * Append available value.
     */
    ST_CPPEXPORT void addItem(const int32_t   theValueOn,
                              const StString& theTexturePath,
                              bool            theToSkip = false);

        private:

    StHandle<StInt32Param> myTrackValue; //!< handle to tracked value
    StArrayList<int32_t>   mySkipValues; //!< values to skip on click

};

#endif //__StGLSwitchTextured_h_
