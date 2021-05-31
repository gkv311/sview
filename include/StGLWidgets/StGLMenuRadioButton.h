/**
 * StGLWidgets, small C++ toolkit for writing GUI using OpenGL.
 * Copyright © 2011-2015 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
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
class StGLMenuRadioButton : public StGLMenuItem {

        public:

    ST_CPPEXPORT StGLMenuRadioButton(StGLMenu* theParent,
                                     const StHandle<StInt32Param>& theTrackedValue,
                                     const int32_t theOnValue);

    ST_CPPEXPORT StGLMenuRadioButton(StGLMenu* theParent,
                                     const StHandle<StFloat32Param>& theTrackedValue,
                                     const float theOnValue);

        private: //!< callback Slots (private overriders)

    ST_LOCAL void doItemClick(const size_t );

        private:

    StGLRadioButton* myRadio; //!< fast-link to the child widget

};

#endif //__StGLMenuRadioButton_h_
