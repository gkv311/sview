/**
 * StGLWidgets, small C++ toolkit for writing GUI using OpenGL.
 * Copyright © 2013-2015 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#ifndef __StGLFpsLabel_h_
#define __StGLFpsLabel_h_

#include <StGLWidgets/StGLTextArea.h>
#include <StGLWidgets/StGLMenuProgram.h>

/**
 * Widget for displaying diagnostic information
 * (frame rate, buffers state, etc.).
 */
class StGLFpsLabel : public StGLTextArea {

        public:  //! @name StGLTextArea overrides

    ST_CPPEXPORT StGLFpsLabel(StGLWidget* theParent);
    ST_CPPEXPORT virtual ~StGLFpsLabel();

        public:

    ST_CPPEXPORT void update(const bool      theIsStereo,
                             const double    theTargetFps,
                             const StString& theExtraInfo);

    ST_LOCAL double& changePlayFps()         { return myPlayFps; }
    ST_LOCAL int&    changePlayQueued()      { return myPlayQueued; }
    ST_LOCAL int&    changePlayQueueLength() { return myPlayQueueLen; }

        public:  //! @name Signals

    struct {
        /**
         * Emit callback Slot on button click.
         * @param theUserData user predefined data
         */
        StSignal<void (const size_t )> onBtnClick;
    } signals;

        private: //! @name callback Slots (private overriders)

    ST_LOCAL void doMouseUnclick(const int theBtnId);

        private:

    double       myPlayFps;      //!< video decoding FPS
    int          myPlayQueued;   //!< queued frames
    int          myPlayQueueLen; //!< queue length
    StTimer      myTimer;        //!< FPS timer
    unsigned int myCounter;      //!< frames counter

};

#endif // __StGLFpsLabel_h_
