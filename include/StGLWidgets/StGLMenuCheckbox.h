/**
 * Copyright © 2011 Kirill Gavrilov <kirill@sview.ru>
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

class ST_LOCAL StGLMenuCheckbox : public StGLMenuItem {

        private:

    StGLCheckbox* myCheckbox; //!< fast-link to the child widget

        public:

    StGLMenuCheckbox(StGLMenu* theParent,
                     const StHandle<StBoolParam>& theTrackedValue);

    virtual const StString& getClassName();
    virtual void setVisibility(bool isVisible, bool isForce);

        private: //!< callback Slots (private overriders)

    void doItemClick(const size_t );

};

#endif //__StGLMenuCheckbox_h_
