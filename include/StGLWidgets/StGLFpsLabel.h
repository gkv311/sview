/**
 * Copyright © 2013 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#ifndef __StGLFpsLabel_h_
#define __StGLFpsLabel_h_

#include <StGLWidgets/StGLTextArea.h>
#include <StGLWidgets/StGLMenuProgram.h>

/**
 * FPS widget.
 */
class StGLFpsLabel : public StGLTextArea {

        public:  //!< StGLTextArea overrides

    ST_CPPEXPORT StGLFpsLabel(StGLWidget* theParent);
    ST_CPPEXPORT virtual ~StGLFpsLabel();

        public:

    ST_CPPEXPORT void update(const bool   theIsStereo,
                             const double theTargetFps);

        public:  //!< Signals

    struct {
        /**
         * Emit callback Slot on button click.
         * @param theUserData (const size_t ) - user predefined data.
         */
        StSignal<void (const size_t )> onBtnClick;
    } signals;

        private: //!< callback Slots (private overriders)

    ST_LOCAL void doMouseUnclick(const int theBtnId);

        private:

    StTimer      myTimer;   //!< FPS timer
    unsigned int myCounter; //!< frames counter

};

#endif // __StGLFpsLabel_h_
