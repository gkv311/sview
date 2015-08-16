/**
 * Copyright © 2013-2015 Kirill Gavrilov <kirill@sview.ru>
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
    ST_CPPEXPORT virtual bool tryClick  (const StPointD_t& theCursorZo, const int& theMouseBtn, bool& isItemClicked);
    ST_CPPEXPORT virtual bool tryUnClick(const StPointD_t& theCursorZo, const int& theMouseBtn, bool& isItemUnclicked);

        public:

    /**
     * @return true if content doesn't fit into scroll area
     */
    ST_LOCAL inline bool isScrollable() const {
        const StGLWidget* aContent = myChildren.getStart();
        return aContent != NULL
            && aContent->getRectPx().height() > getRectPx().height();
    }

    /**
     * Scroll (vertically) content.
     * @param theDelta scroll delta
     * @return true if scrolling has been done
     */
    ST_CPPEXPORT bool doScroll(const int  theDelta,
                               const bool theIsFling = false);

        protected:

    StPointD_t myClickPntZo;  //!< remembered mouse click position
    StTimer    myDragTimer;   //!< timer between dragging animation
    double     myDragYDelta;  //!< last dragged distance
    double     myFlingAccel;  //!< (positive) fling acceleration
    StTimer    myFlingTimer;  //!< timer for dragging inertia
    double     myFlingYSpeed; //!< the dragging velocity for inertial scrolling
    int        myFlingYDone;  //!< already animated inertial scrolling

};

#endif // __StGLScrollArea_h_
