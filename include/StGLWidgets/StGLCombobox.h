/**
 * StGLWidgets, small C++ toolkit for writing GUI using OpenGL.
 * Copyright © 2015-2020 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#ifndef __StGLCombobox_h_
#define __StGLCombobox_h_

#include <StGLWidgets/StGLButton.h>
#include <StSettings/StEnumParam.h>

class StGLMessageBox;

/**
 * Simple combobox widget.
 */
class StGLCombobox : public StGLButton {

        public:

    class ListBuilder {

            public:

        ST_CPPEXPORT ListBuilder(StGLWidget* theParent);

        ST_LOCAL StGLMenu* getMenu() { return myMenu; }

        ST_CPPEXPORT void display();

            private:

        StGLMessageBox* myBack;
        StGLMenu*       myMenu;

    };

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
