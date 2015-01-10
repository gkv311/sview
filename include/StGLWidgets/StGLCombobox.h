/**
 * Copyright © 2015 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#ifndef __StGLCombobox_h_
#define __StGLCombobox_h_

#include <StGLWidgets/StGLButton.h>
#include <StSettings/StEnumParam.h>

/**
 * Simple combobox widget.
 */
class StGLCombobox : public StGLButton {

        public:

    /**
     * Main constructor.
     */
    ST_CPPEXPORT StGLCombobox(StGLWidget* theParent,
                              const int   theLeft,
                              const int   theTop,
                              const StHandle<StEnumParam>& theParam);

    /**
     * Destructor.
     */
    virtual ~StGLCombobox();

        private: //! @name callback Slots (private overriders)

    ST_LOCAL void doShowList(const size_t );
    ST_LOCAL void doValueChanged(const int32_t );

        protected:

    StHandle<StEnumParam> myParam; //!< enumeration parameter

};

#endif // __StGLCombobox_h_
