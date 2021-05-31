/**
 * StGLWidgets, small C++ toolkit for writing GUI using OpenGL.
 * Copyright © 2011-2015 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
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
