/**
 * Copyright © 2011 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#ifndef __StGLMenuRadioButton_h_
#define __StGLMenuRadioButton_h_

#include <StGLWidgets/StGLMenuItem.h>

// forward declarations
class StInt32Param;
class StFloat32Param;
class StGLRadioButton;

/**
 * Simple menu item widget with radio button item on it.
 */
class ST_LOCAL StGLMenuRadioButton : public StGLMenuItem {

        private:

    StGLRadioButton* myRadio; //!< fast-link to the child widget

        public:

    StGLMenuRadioButton(StGLMenu* theParent,
                        const StHandle<StInt32Param>& theTrackedValue,
                        const int32_t theOnValue);

    StGLMenuRadioButton(StGLMenu* theParent,
                        const StHandle<StFloat32Param>& theTrackedValue,
                        const float theOnValue);

    virtual const StString& getClassName();
    virtual void setVisibility(bool isVisible, bool isForce);

        private: //!< callback Slots (private overriders)

    void doItemClick(const size_t );

};

#endif //__StGLMenuRadioButton_h_
