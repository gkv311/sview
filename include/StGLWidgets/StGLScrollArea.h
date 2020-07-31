/**
 * StGLWidgets, small C++ toolkit for writing GUI using OpenGL.
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
    ST_CPPEXPORT virtual bool stglInit() ST_ATTR_OVERRIDE;
    ST_CPPEXPORT virtual void stglResize() ST_ATTR_OVERRIDE;
    ST_CPPEXPORT virtual void stglDraw(unsigned int theView) ST_ATTR_OVERRIDE;
    ST_CPPEXPORT virtual void stglUpdate(const StPointD_t& theCursorZo,
                                         bool theIsPreciseInput) ST_ATTR_OVERRIDE;
    ST_CPPEXPORT virtual bool tryClick  (const StClickEvent& theEvent, bool& isItemClicked)   ST_ATTR_OVERRIDE;
    ST_CPPEXPORT virtual bool tryUnClick(const StClickEvent& theEvent, bool& isItemUnclicked) ST_ATTR_OVERRIDE;
    ST_CPPEXPORT virtual bool doScroll(const StScrollEvent& theEvent) ST_ATTR_OVERRIDE;

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
     * Returns TRUE if dragging has been confirmed.
     */
    ST_LOCAL bool hasDragged() const { return myHasDragged; }

    /**
     * Scroll (vertically) content.
     * @param theDelta scroll delta
     * @return true if scrolling has been done
     */
    ST_CPPEXPORT bool doScroll(const int  theDelta,
                               const bool theIsFling = false);

        protected:

    StGLVertexBuffer myBarVertBuf; //!< vertices buffer
    StGLVec4         myBarColor;   //!< color of scroll bar

    bool       myIsLeftClick; //!< flag to perform dragging - some item has been clicked (but not yet unclicked)
    bool       myHasDragged;  //!< indicates that dragging has been confirmed
    StPointD_t myClickPntZo;  //!< remembered mouse click position
    StTimer    myDragTimer;   //!< timer between dragging animation
    double     myDragYDelta;  //!< last dragged distance
    int        myDragYCumul;  //!< cumulative dragged distance
    double     myFlingAccel;  //!< (positive) fling acceleration
    StTimer    myFlingTimer;  //!< timer for dragging inertia
    double     myFlingYSpeed; //!< the dragging velocity for inertial scrolling
    int        myFlingYDone;  //!< already animated inertial scrolling
    float      myScrollYAccum;//!< accumulated scroll event value

};

#endif // __StGLScrollArea_h_
