/**
 * StGLWidgets, small C++ toolkit for writing GUI using OpenGL.
 * Copyright © 2011-2015 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#ifndef __StGLMenuCheckbox_h_
#define __StGLMenuCheckbox_h_

#include <StGLWidgets/StGLMenuItem.h>

// forward declarations
class StBoolParam;
class StGLCheckbox;

class StGLMenuCheckbox : public StGLMenuItem {

        public:

    ST_CPPEXPORT StGLMenuCheckbox(StGLMenu* theParent,
                                  const StHandle<StBoolParam>& theTrackedValue);

    /**
     * Return checkbox widget.
     */
    StGLCheckbox* getCheckbox() { return myCheckbox; }

        private: //!< callback Slots (private overriders)

    ST_LOCAL void doItemClick(const size_t );

        private:

    StGLCheckbox* myCheckbox; //!< fast-link to the child widget

};

#endif //__StGLMenuCheckbox_h_
