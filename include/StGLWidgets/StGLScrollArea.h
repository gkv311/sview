/**
 * Copyright © 2013 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#ifndef __StGLScrollArea_h_
#define __StGLScrollArea_h_

#include <StGLWidgets/StGLTextArea.h>

/**
 * Widget to display arbitrary number of objects inside specified area.
 * Clipping all children by own rectangle (activates scissor box)
 * and displays scrolling bars when needed.
 */
class StGLScrollArea : public StGLWidget {

        public:

    /**
     * Main constructor.
     */
    ST_CPPEXPORT StGLScrollArea(StGLWidget*      theParent,
                                const int        theLeft,  const int theTop,
                                const StGLCorner theCorner,
                                const int        theWidth, const int theHeight);
    ST_CPPEXPORT virtual ~StGLScrollArea();
    ST_CPPEXPORT virtual bool stglInit();
    ST_CPPEXPORT virtual void stglResize();
    ST_CPPEXPORT virtual void stglDraw(unsigned int theView);

        public:

    ST_CPPEXPORT void doScroll(const int theDir);

        private:   //! @name callback Slots (private overriders)

    ST_LOCAL void doMouseUnclick(const int theBtnId);

};

#endif // __StGLScrollArea_h_
