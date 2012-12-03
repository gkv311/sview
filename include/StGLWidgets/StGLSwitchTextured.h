/**
 * Copyright © 2011 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#ifndef __StGLSwitchTextured_h_
#define __StGLSwitchTextured_h_

#include <StGLWidgets/StGLWidget.h>
#include <StSettings/StParam.h>

/**
 * This class represents a switch between values shown as image.
 * It behaves as clickable iterator - each click switch to the next value in cycle.
 */
class ST_LOCAL StGLSwitchTextured : public StGLWidget {

        private:

    StHandle<StInt32Param> myTrackValue; //!< handle to tracked value
    StArrayList<int32_t>   mySkipValues; //!< values to skip on click

        public:

    /**
     * Main constructor.
     */
    StGLSwitchTextured(StGLWidget* theParent,
                       const StHandle<StInt32Param>& theTrackedValue,
                       const int theLeft, const int theTop,
                       const StGLCorner theCorner = StGLCorner(ST_VCORNER_TOP, ST_HCORNER_LEFT));

    /**
     * Destructor.
     */
    virtual ~StGLSwitchTextured();

    virtual const StString& getClassName();
    virtual void setVisibility(bool isVisible, bool isForce);
    virtual bool stglInit();

    /**
     * Overrider that shows only active value in the switch.
     */
    virtual void stglDraw(unsigned int theView);

    /**
     * Overrider that blocks children's clicking functionality and switch the values in cycle.
     */
    virtual bool tryClick(const StPointD_t& theCursorZo, const int& theMouseBtn, bool& isItemClicked);
    virtual bool tryUnClick(const StPointD_t& theCursorZo, const int& theMouseBtn, bool& isItemUnclicked);

    /**
     * Append available value.
     */
    void addItem(const int32_t   theValueOn,
                 const StString& theTexturePath,
                 bool            theToSkip = false);

};

#endif //__StGLSwitchTextured_h_
