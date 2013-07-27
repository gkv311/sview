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

        public:  //! @name StGLTextArea overrides

    ST_CPPEXPORT StGLFpsLabel(StGLWidget* theParent);
    ST_CPPEXPORT virtual ~StGLFpsLabel();

        public:

    ST_CPPEXPORT void update(const bool   theIsStereo,
                             const double theTargetFps);

    ST_LOCAL inline double& changePlayFps() {
        return myPlayFps;
    }

    ST_LOCAL inline int& changePlayQueued() {
        return myPlayQueued;
    }

    ST_LOCAL inline int& changePlayQueueLength() {
        return myPlayQueueLen;
    }

        public:  //! @name Signals

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

    double       myPlayFps;      //!< video decoding FPS
    int          myPlayQueued;   //!< queued frames
    int          myPlayQueueLen; //!< queue length
    StTimer      myTimer;        //!< FPS timer
    unsigned int myCounter;      //!< frames counter

};

#endif // __StGLFpsLabel_h_
