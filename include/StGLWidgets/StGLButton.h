/**
 * Copyright © 2013 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#ifndef __StGLButton_h_
#define __StGLButton_h_

#include <StGLWidgets/StGLMenu.h>

/**
 * Button with text widget.
 */
class StGLButton : public StGLMenu {

        public:

    ST_CPPEXPORT StGLButton(StGLWidget*     theParent,
                            const int       theLeft,
                            const int       theTop,
                            const StString& theText);

    ST_CPPEXPORT virtual ~StGLButton();

    ST_CPPEXPORT virtual bool stglInit();

    /**
     * @return button width
     */
    ST_CPPEXPORT int getWidth() const;

    /**
     * Setup button width.
     */
    ST_CPPEXPORT void setWidth(const int theWidth);

    /**
     * Setup button height.
     */
    ST_CPPEXPORT void setHeight(const int theHeight);

    ST_CPPEXPORT int getHeight(const int theHeight);

        public:  //!< @name Signals

    struct {
        /**
         * Emit callback Slot on button click.
         * @param theUserData (const size_t ) - user predefined data.
         */
        StSignal<void (const size_t )> onBtnClick;
    } signals;

        private: //!< @name callback Slots (private overriders)

    ST_LOCAL void doItemClick(const size_t );

};

#endif // __StGLButton_h_
