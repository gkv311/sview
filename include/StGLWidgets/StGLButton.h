/**
 * StGLWidgets, small C++ toolkit for writing GUI using OpenGL.
 * Copyright © 2013-2015 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
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

    ST_CPPEXPORT virtual bool stglInit() ST_ATTR_OVERRIDE;
    ST_CPPEXPORT virtual bool doScroll(const StScrollEvent& theEvent) ST_ATTR_OVERRIDE;

    /**
     * Change button text.
     */
    ST_CPPEXPORT void setLabel(const StString& theLabel);

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

    ST_CPPEXPORT void setFocus(const bool theValue);

    /**
     * Estimate maximum button width for specified text.
     */
    ST_CPPEXPORT int computeWidth(const StString& theText);

    /**
     * Return the underlying menu item.
     */
    ST_LOCAL StGLMenuItem* getMenuItem() {
        return (StGLMenuItem* )getChildren()->getStart();
    }

        public:  //! @name Signals

    struct {
        /**
         * Emit callback Slot on button click.
         * @param theUserData (const size_t ) - user predefined data.
         */
        StSignal<void (const size_t )> onBtnClick;
    } signals;

        private: //! @name callback Slots (private overriders)

    ST_LOCAL void doItemClick(const size_t );

        protected:

    ST_LOCAL StGLMenuItem* addItem(const StString& theLabel);

};

#endif // __StGLButton_h_
